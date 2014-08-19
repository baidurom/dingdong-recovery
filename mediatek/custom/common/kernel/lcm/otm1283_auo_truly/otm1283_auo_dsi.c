/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include <mach/mt_pm_ldo.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define LCM_ID       (0x69)
#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

#define LCM_OTM1283_ID									0x1283

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
#define GPIO_LCD_RST_EN      GPIO131

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
//#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

    {0xff,3,{0x12,0x80,0x01}},   
    {0x00,1,{0x80}}, 
    {0xff,2,{0x12,0x80}},
    {0x00,1,{0xb8}},
    {0xf5,2,{0x0c,0x12}},
    {0x00,1,{0x90}},
    {0xc5,8,{0x10,0x6F,0x02,0x88,0x1D,0x15,0x00,0x04}},
    {0x00,1,{0xa0}},
    {0xc5,8,{0x10,0x6F,0x02,0x88,0x1D,0x15,0x00,0x04}},
    {0x00,1,{0x80}},
    {0xc5,8,{0x20,0x01,0x00,0xb0,0xb0,0x00,0x04,0x00}},
    {0x00,1,{0x00}},             
    {0xd8,4,{0x58,0x00,0x58,0x00}},
    {0x00,1,{0x84}},             
    {0xff,2,{0x10,0x02}},//Set VCOM
 /*   {0x00,1,{0x00}},             
    {0xd9,1,{0x94}},//Set VCOM
    {0x00,1,{0x80}}, 
    {0xff,2,{0x00,0x00}},
    {0x00,1,{0x80}}, 
    {0xff,3,{0x00,0x00,0x00}},  
    {REGFLAG_DELAY, 120, {}},
*/  
    {0x11,1,{0x00}},
    {REGFLAG_DELAY, 120, {}},
    {0x29,1,{0x00}},  
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},

    // Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
/*
static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xB9,	3,	{0xFF, 0x83, 0x69}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
//	{0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/
#if 0
static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);				

				if (cmd != 0xFF && cmd != 0x2C && cmd != 0x3C) {
					//#if defined(BUILD_UBOOT)
					//	printf("[DISP] - uboot - REG_R(0x%x) = 0x%x. \n", cmd, table[i].para_list[0]);
					//#endif
					while(read_reg(cmd) != table[i].para_list[0]);		
				}
       	}
		
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		//params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//params->dsi.LANE_NUM				= LCM_THREE_LANE;
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

#if (LCM_DSI_CMD_MODE)
		params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
#else
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
#endif
	

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	
#if 0
		
		params->dsi.vertical_sync_active				= 2;  //---3
		params->dsi.vertical_backporch					= 10; //---14
		params->dsi.vertical_frontporch					= 10;  //----8
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 2;  //----2
		params->dsi.horizontal_backporch				= 40; //----28
		params->dsi.horizontal_frontporch				= 40; //----50
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#endif

                params->dsi.vertical_sync_active                                = 8;  //---3
                params->dsi.vertical_backporch                                  = 16; //---14
                params->dsi.vertical_frontporch                                 = 16;  //----8
                params->dsi.vertical_active_line                                = FRAME_HEIGHT;

                params->dsi.horizontal_sync_active                              = 10;  //----2
                params->dsi.horizontal_backporch                                = 24;//24; //----28
                params->dsi.horizontal_frontporch                               = 24; //----50
                params->dsi.horizontal_active_pixel                             = FRAME_WIDTH;

        	//params->dsi.HS_PRPR=6;
                //params->dsi.LPX=8; 
		//params->dsi.HS_PRPR=5;
		//params->dsi.HS_TRAIL=13;

		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{
	unsigned int data_array[16];
#ifdef BUILD_LK
        upmu_set_rg_vgp6_vosel(6);
        upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3300, "LCM");
#endif
    MDELAY(10);

    mt_set_gpio_mode(GPIO74, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO74, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO74, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(10);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
    MDELAY(50);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(120);

#if 1
	data_array[0]=0x00043902;
	data_array[1]=0x018312ff;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
	
        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
	
        data_array[0]=0x00033902;
	data_array[1]=0x008312ff;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009200;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x000230ff;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a600;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x00000bb3;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009400;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000002f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000ba00;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000003f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000a3902;
	data_array[1]=0x006400c0;//cmd mode
	data_array[2]=0x64001010;//cmd mode
	data_array[3]=0x00001010;//cmd mode
	dsi_set_cmdq(data_array, 4, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00073902;
	data_array[1]=0x005c00c0;//cmd mode
	data_array[2]=0x00040001;//cmd mode
	dsi_set_cmdq(data_array, 3, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a200;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00043902;
	data_array[1]=0x000001c0;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000b300;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x005000c0;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000055c1;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000049c4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x041005c4;//cmd mode
	data_array[2]=0x11150502;//cmd mode
	data_array[3]=0x02071005;//cmd mode
	data_array[4]=0x00111505;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x000000c4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000bb00;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000080c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x005009c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000c000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00073902;
	data_array[1]=0xf3af01c5;//cmd mode
	data_array[2]=0x008382b0;//cmd mode
	dsi_set_cmdq(data_array, 3, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x00b0b0d8;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000057d9;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000082c4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x00b804c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
#if 0
        data_array[0]=0x00023902;
	data_array[1]=0x00008200;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x000000f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008200;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000000f4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
#endif
        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000040d0;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00033902;
	data_array[1]=0x000000d1;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000000c4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009800;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000010c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
         data_array[0]=0x00023902;
	data_array[1]=0x00008200;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000002c4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008300;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008500;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008700;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008900;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00008b00;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009500;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009700;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00009900;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a300;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a500;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000a700;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000015f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000ab00;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000018f5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000b100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000e3902;
	data_array[1]=0x000015f5;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x15020000;//cmd mode
	data_array[4]=0x00001508;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x0000b400;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000c0c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
#if 0
        data_array[0]=0x00023902;
	data_array[1]=0x00009b00;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009d00;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000a000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000a200;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b300;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b500;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b700;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b900;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000ffcb;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000000c4;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009800;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000010c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b100;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000e3902;
	data_array[1]=0x000015f5;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x15020000;//cmd mode
	data_array[4]=0x00001508;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b400;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x0000c0c5;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
#endif
        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000c3902;
	data_array[1]=0x000000cb;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 4, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x000000cb;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x00000000;//cmd mode
	data_array[4]=0x00ff00ff;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000a000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0xff00ffcb;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x00000000;//cmd mode
	data_array[4]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x000000cb;//cmd mode
	data_array[2]=0x00ff00ff;//cmd mode
	data_array[3]=0x00ff00ff;//cmd mode
	data_array[4]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000c000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x000000cb;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x05000505;//cmd mode
	data_array[4]=0x05050505;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000d000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x050505cb;//cmd mode
	data_array[2]=0x00000505;//cmd mode
	data_array[3]=0x00000000;//cmd mode
	data_array[4]=0x05000000;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000e000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x050005cb;//cmd mode
	data_array[2]=0x05050505;//cmd mode
	data_array[3]=0x05050505;//cmd mode
	data_array[4]=0x00000505;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000f000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000c3902;
	data_array[1]=0xffffffcb;//cmd mode
	data_array[2]=0xffffffff;//cmd mode
	data_array[3]=0xffffffff;//cmd mode
	dsi_set_cmdq(data_array, 4, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x000000cc;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x0d000705;//cmd mode
	data_array[4]=0x110b0f09;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x171315cc;//cmd mode
	data_array[2]=0x00000301;//cmd mode
	data_array[3]=0x00000000;//cmd mode
	data_array[4]=0x06000000;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000a000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x0e0008cc;//cmd mode
	data_array[2]=0x120c100a;//cmd mode
	data_array[3]=0x02181416;//cmd mode
	data_array[4]=0x00000004;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x000000cc;//cmd mode
	data_array[2]=0x00000000;//cmd mode
	data_array[3]=0x0c000204;//cmd mode
	data_array[4]=0x180e0a10;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000c000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x121614cc;//cmd mode
	data_array[2]=0x00000608;//cmd mode
	data_array[3]=0x00000000;//cmd mode
	data_array[4]=0x03000000;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000d000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00103902;
	data_array[1]=0x0b0001cc;//cmd mode
	data_array[2]=0x170d090f;//cmd mode
	data_array[3]=0x07111513;//cmd mode
	data_array[4]=0x00000005;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000d3902;
	data_array[1]=0x280387ce;//cmd mode
	data_array[2]=0x85280386;//cmd mode
	data_array[3]=0x03842803;//cmd mode
	data_array[4]=0x00000028;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000d3902;
	data_array[1]=0x28fc34ce;//cmd mode
	data_array[2]=0x3428fd34;//cmd mode
	data_array[3]=0xff3428fe;//cmd mode
	data_array[4]=0x00000028;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000a000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050738ce;//cmd mode
	data_array[2]=0x00280000;//cmd mode
	data_array[3]=0x01050638;//cmd mode
	data_array[4]=0x00002800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050538ce;//cmd mode
	data_array[2]=0x00280002;//cmd mode
	data_array[3]=0x03050438;//cmd mode
	data_array[4]=0x00002800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000c000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050338ce;//cmd mode
	data_array[2]=0x00280004;//cmd mode
	data_array[3]=0x05050238;//cmd mode
	data_array[4]=0x00002800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000d000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050138ce;//cmd mode
	data_array[2]=0x00280006;//cmd mode
	data_array[3]=0x07050038;//cmd mode
	data_array[4]=0x00002800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00008000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x040778cf;//cmd mode
	data_array[2]=0x281800ff;//cmd mode
	data_array[3]=0x00050678;//cmd mode
	data_array[4]=0x00281800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00009000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050578cf;//cmd mode
	data_array[2]=0x28180001;//cmd mode
	data_array[3]=0x02050478;//cmd mode
	data_array[4]=0x00281800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000a000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050070cf;//cmd mode
	data_array[2]=0x28180000;//cmd mode
	data_array[3]=0x01050170;//cmd mode
	data_array[4]=0x00281800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000b000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000f3902;
	data_array[1]=0x050270cf;//cmd mode
	data_array[2]=0x28180002;//cmd mode
	data_array[3]=0x03050370;//cmd mode
	data_array[4]=0x00281800;//cmd mode
	dsi_set_cmdq(data_array, 5, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000c000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x000c3902;
	data_array[1]=0x200101cf;//cmd mode
	data_array[2]=0x01000020;//cmd mode
	data_array[3]=0x08030081;//cmd mode
	dsi_set_cmdq(data_array, 4, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00113902;
	data_array[1]=0x110f00e1;//cmd mode
	data_array[2]=0x0c110913;//cmd mode
	data_array[3]=0x0e05020c;//cmd mode
	data_array[4]=0x2531150d;//cmd mode
	data_array[5]=0x0000000f;//cmd mode
	dsi_set_cmdq(data_array, 6, 1);

        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00113902;
	data_array[1]=0x110f00e2;//cmd mode
	data_array[2]=0x0c110913;//cmd mode
	data_array[3]=0x0e05020c;//cmd mode
	data_array[4]=0x2531150d;//cmd mode
	data_array[5]=0x0000000f;//cmd mode
	dsi_set_cmdq(data_array, 6, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00233902;
	data_array[1]=0x444440ec;//cmd mode
	data_array[2]=0x44444444;//cmd mode
	data_array[3]=0x44444444;//cmd mode
	data_array[4]=0x44444444;//cmd mode
	data_array[5]=0x44444444;//cmd mode
	data_array[6]=0x44444444;//cmd mode
	data_array[7]=0x44444444;//cmd mode
	data_array[8]=0x44444444;//cmd mode
	data_array[9]=0x00004444;//cmd mode
	dsi_set_cmdq(data_array, 10, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00233902;
	data_array[1]=0x444440ed;//cmd mode
	data_array[2]=0x44444444;//cmd mode
	data_array[3]=0x44444444;//cmd mode
	data_array[4]=0x44444444;//cmd mode
	data_array[5]=0x44444444;//cmd mode
	data_array[6]=0x44444444;//cmd mode
	data_array[7]=0x44444444;//cmd mode
	data_array[8]=0x44444444;//cmd mode
	data_array[9]=0x00001111;//cmd mode
	dsi_set_cmdq(data_array, 10, 1);
        
        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00233902;
	data_array[1]=0x444440ee;//cmd mode
	data_array[2]=0x44444444;//cmd mode
	data_array[3]=0x44444444;//cmd mode
	data_array[4]=0x44444444;//cmd mode
	data_array[5]=0x44444444;//cmd mode
	data_array[6]=0x44444444;//cmd mode
	data_array[7]=0x44444444;//cmd mode
	data_array[8]=0x44444444;//cmd mode
	data_array[9]=0x00004444;//cmd mode
	dsi_set_cmdq(data_array, 10, 1);
#if 0
        data_array[0]=0x00023902;
	data_array[1]=0x0000c700;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00023902;
	data_array[1]=0x000002cf;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
#endif
        data_array[0]=0x00023902;
	data_array[1]=0x00000000;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
        data_array[0]=0x00043902;
	data_array[1]=0xffffffff;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

    //ADD   CE
    data_array[0]=0x00023902;
    data_array[1]=0x0000A000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x000D3902;
    data_array[1]=0x01CD01D6;//cmd mode
    data_array[2]=0x01CD01CD;//cmd mode
    data_array[3]=0x01CD01CD;//cmd mode
    data_array[4]=0x000000CD;//cmd mode
    dsi_set_cmdq(data_array, 5, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000B000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x000D3902;
    data_array[1]=0x01CD01D6;//cmd mode
    data_array[2]=0x01CD01CD;//cmd mode
    data_array[3]=0x01CD01CD;//cmd mode
    data_array[4]=0x000000CD;//cmd mode
    dsi_set_cmdq(data_array, 5, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000C000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x000D3902;
    data_array[1]=0x891189D6;//cmd mode
    data_array[2]=0x89891189;//cmd mode
    data_array[3]=0x11898911;//cmd mode
    data_array[4]=0x00000089;//cmd mode
    dsi_set_cmdq(data_array, 5, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000D000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00073902;
    data_array[1]=0x891189D6;//cmd mode
    data_array[2]=0x00891189;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000E000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x000D3902;
    data_array[1]=0x441144D6;//cmd mode
    data_array[2]=0x44441144;//cmd mode
    data_array[3]=0x11444411;//cmd mode
    data_array[4]=0x00000044;//cmd mode
    dsi_set_cmdq(data_array, 5, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000F000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00073902;
    data_array[1]=0x441144D6;//cmd mode
    data_array[2]=0x00441144;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0]=0x00043902;
    data_array[1]=0xffffffff;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    //ADD  CE
    data_array[0]=0x00023902;
    data_array[1]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00009055;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

        data_array[0]=0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
        MDELAY(120);

	data_array[0]=0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
		 MDELAY(200);
#endif	
//	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    unsigned int data_array[16];

    data_array[0]=0x00280500; // Display Off
    dsi_set_cmdq(&data_array, 1, 1);
    MDELAY(120);
    data_array[0] = 0x00100500; // Sleep In
    dsi_set_cmdq(&data_array, 1, 1);
    MDELAY(120);
#ifdef BUILD_LK
    upmu_set_rg_vgp6_en(0);
#else
    hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
    // do nothing in LK
    // do nothing in uboot
#else
    //hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif
    lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}

#if 0
static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_UBOOT
        if(lcm_esd_test)
        {
            lcm_esd_test = FALSE;
            return TRUE;
        }

        /// please notice: the max return packet size is 1
        /// if you want to change it, you can refer to the following marked code
        /// but read_reg currently only support read no more than 4 bytes....
        /// if you need to read more, please let BinHan knows.
        /*
                unsigned int data_array[16];
                unsigned int max_return_size = 1;
                
                data_array[0]= 0x00003700 | (max_return_size << 16);    
                
                dsi_set_cmdq(&data_array, 1, 1);
        */

        if(read_reg(0xB6) == 0x42)
        {
            return FALSE;
        }
        else
        {            
            return TRUE;
        }
#endif
}

static unsigned int lcm_esd_recover(void)
{
    unsigned char para = 0;

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);
	  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
	  push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
    dsi_set_cmdq_V2(0x35, 1, &para, 1);     ///enable TE
    MDELAY(10);

    return TRUE;
}
#endif
static unsigned int lcm_compare_id(void)
{
	unsigned int id0,id1, id2, id3,id4;
	unsigned char buffer[5];
	unsigned int array[5];
#ifdef BUILD_LK
        upmu_set_rg_vgp6_vosel(6);
        upmu_set_rg_vgp6_en(1);
#else
        hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif

        mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
        MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(100);

	// Set Maximum return byte = 1
	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
        
        read_reg_v2(0xA1, buffer, 5);
        id0 = buffer[0];
        id1 = buffer[1];
        id2 = buffer[2];
        id3 = buffer[3];
        id4 = buffer[4];

#if defined(BUILD_LK)
	printf("%s, Module ID = {%x, %x, %x, %x, %x} \n", __func__, id0, id1, id2,id3,id4);
#else
	printk("%s, Module ID = {%x, %x, %x, %x,%x} \n", __func__,  id0,id1, id2, id3,id4);
#endif

    return (LCM_OTM1283_ID==((id2 << 8) | id3) )?1:0;
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1283_auo_lcm_drv = 
{
    .name			= "otm1283_AUO",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
	//.set_backlight	= lcm_setbacklight,
//	.set_pwm        = lcm_setpwm,
//	.get_pwm        = lcm_getpwm,
	//.esd_check   = lcm_esd_check,
    //.esd_recover   = lcm_esd_recover,
	//.compare_id    = lcm_compare_id,
#endif
};
