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


#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#ifndef FALSE
    #define FALSE 0
#endif
#define GPIO_LCD_RST_EN      GPIO131

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#define LCM_DSI_CMD_MODE									0
#define LCM_ID_HX8394 0x0094
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
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table {
    unsigned cmd;
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
    {0xb9,3,{0xff,0x83,0x94}},   
	{REGFLAG_DELAY,1,{}},
	{0xc7,4,{0x00, 0x10, 0x00, 0x10}},	 
    {REGFLAG_DELAY,1,{}},

    {0xbc,1,{0x07}}, 
	{REGFLAG_DELAY,1,{}},
	{0xba,1,{0x13}}, 
    {REGFLAG_DELAY,1,{}},

    {0xb1,15,{0x7c,0x01,0x27,0x07,0x01,0x10,0x10,0x21,0x29,0x29,0x29,0x50,0x12,0x00,0x00}},
    {REGFLAG_DELAY,1,{}},
	{0xb2,6,{0x00, 0xC8, 0x09, 0x05, 0x00, 0x71}},
	{REGFLAG_DELAY,1,{}},

    {0xcc,1,{0x09}},
    {REGFLAG_DELAY,1,{}},
    
    {0xd5,16,{0x00,0x00,0x00,0x00,0x0A,0x00,0x01,0x00,0x00,0x00,0x33,0x00,0x23,0x45,0x67,0x01}},
    {REGFLAG_DELAY,1,{}},
	{0xd5,16,{0x01,0x23,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x99,0x99,0x99,0x88,0x88,0x99,0x88}},
	{REGFLAG_DELAY,1,{}},
	{0xd5,16,{0x54,0x32,0x10,0x76,0x32,0x10,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x99,0x99,0x99}},
	{REGFLAG_DELAY,1,{}},
	{0xd5,4,{0x88,0x88,0x88,0x99}},
	{REGFLAG_DELAY,1,{}},

	{0xb4,22,{0x80, 0x08, 0x32, 0x10, 0x00, 0x32, 0x15, 0x08, 0x32, 0x12, 0x20, 0x33, 0x05, 0x4C, 0x05, 0x37, 0x05, 0x3F, 0x1E, 0x5F, 0x5F, 0x06}},
	{REGFLAG_DELAY,1,{}},

	{0xb6,1,{0x00}},
	{REGFLAG_DELAY,1,{}},
	
    {0xe0,34,{0x01, 0x05, 0x07, 0x25, 0x35, 0x3F, 0x0B, 0x32, 0x04, 0x09, 0x0E, 0x10, 0x13, 0x10, 0x14, 0x16, 0x1B, 0x01, 0x05, 0x07, 0x25, 0x35, 0x3F, 0x0B, 0x32, 0x04, 0x09, 0x0E, 0x10, 0x13, 0x10, 0x14, 0x16, 0x1B}},
    {REGFLAG_DELAY,1,{}},

	{0xbf,3,{0x06,0x00,0x10}},   
	{REGFLAG_DELAY,1,{}},

//    {0xc7,3,{0x00,0x30,0x00}},   
//    {REGFLAG_DELAY,1,{}},
//    {0xc9,9,{0x0f,0x00,0x1e,0x1e,0x00,0x00,0x00,0x01,0x3e}},
//    {REGFLAG_DELAY,1,{}},

    {0x11,0,{0x00}},
    {REGFLAG_DELAY, 200, {}},
    
	{0x29,0,{0x00}},
    {REGFLAG_DELAY, 50, {}},
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
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 50, {}},
    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

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

        //params->dsi.mode   = BURST_VDO_MODE; 
        params->dsi.mode   = SYNC_EVENT_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
		params->dsi.word_count=720*3;
		params->dsi.vertical_sync_active				= 4;//2//
		params->dsi.vertical_backporch					= 20;//8
		params->dsi.vertical_frontporch					= 20;//6
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		params->dsi.horizontal_sync_active				= 20;//86
		params->dsi.horizontal_backporch				= 50;//55
		params->dsi.horizontal_frontporch				= 50;//55	
		params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
		

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.pll_select=1;
	    params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_227_5;//this value must be in MTK suggested table
		
}

static void lcm_init(void)
{
	unsigned int data_array[16];
#ifdef BUILD_LK
        upmu_set_rg_vgp6_vosel(6);
        upmu_set_rg_vgp6_en(1);
#else
        hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif


        mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
        MDELAY(1);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(50);
		
#if 1
		   			
			 data_array[0]=0x00043902;
			 data_array[1]=0x9483ffb9;
			 dsi_set_cmdq(data_array, 2, 1);
	 
			 data_array[0]=0x00053902;
			 data_array[1]=0x001000c7;
			 data_array[2]=0x00000010;
			 dsi_set_cmdq(data_array, 3, 1);
			 
			data_array[0]=0x00023902;
			 data_array[1]=0x000007bc;
			 dsi_set_cmdq(data_array, 2, 1);
	 
	 		data_array[0]=0x00023902;
			 data_array[1]=0x000013ba;
			 dsi_set_cmdq(data_array, 2, 1);
			 
			 data_array[0]=0x00103902;
			 data_array[1]=0x070001b1;
			 data_array[2]=0x0e0e0181;
			 data_array[3]=0x22223832;
			 data_array[4]=0x00000250;
			 dsi_set_cmdq(data_array, 5, 1);
	 
 
			 data_array[0]=0x00173902;
			 data_array[1]=0x320880b4;
			 data_array[2]=0x15320010;
			 data_array[3]=0x20123208;
			 data_array[4]=0x054c0533;
			 data_array[5]=0x1e3f0537;
			 data_array[6]=0x00065f5f;
			 dsi_set_cmdq(data_array, 7, 1);
	 
			 data_array[0]=0x00073902;
			 data_array[1]=0x09c800b2;
			 data_array[2]=0x00710005;
			 dsi_set_cmdq(data_array, 3, 1);

			 data_array[0]=0x00023902;
			 data_array[1]=0x000009cc;
			 dsi_set_cmdq(data_array, 2, 1);

			 data_array[0]=0x00000500;
			 dsi_set_cmdq(data_array, 1, 1);
	 
			 data_array[0]=0x00353902;
			 data_array[1]=0x000000d5;
			 data_array[2]=0x01000a00;
			 data_array[3]=0x33000000;
			 data_array[4]=0x67452300;
			 data_array[5]=0x88230101;
			 data_array[6]=0x88888888;
			 data_array[7]=0x99998888;
			 data_array[8]=0x99888899;
			 data_array[9]=0x10325488;
			 data_array[10]=0x88103276;
			 data_array[11]=0x88888888;
			 data_array[12]=0x99998888;
			 data_array[13]=0x88888899;
			 data_array[14]=0x00000099;
			 dsi_set_cmdq(data_array, 15, 1);
	 
			 data_array[0]=0x00043902;
			 data_array[1]=0x800004c6;
			 dsi_set_cmdq(data_array, 2, 1);
	  
			 data_array[0]=0x00043902;
			 data_array[1]=0x100006bf;
			 dsi_set_cmdq(data_array, 2, 1);
	 
			 data_array[0]=0x00233902;
			 data_array[1]=0x0c0a06e0;
			 data_array[2]=0x1f3f3525;
			 data_array[3]=0x0e090446;
			 data_array[4]=0x14101310;
			 data_array[5]=0x0a061b16;
			 data_array[6]=0x3f35250c;
			 data_array[7]=0x0904461f;
			 data_array[8]=0x1013100e;
			 data_array[9]=0x001b1614;
			 dsi_set_cmdq(data_array, 10, 1);
	 
			 data_array[0]=0x00023902;
			 data_array[1]=0x000000b6;
			 dsi_set_cmdq(data_array, 2, 1);
	 
			 data_array[0]=0x00023902;
			 data_array[1]=0x000001e6;
			 dsi_set_cmdq(data_array, 2, 1);
	 
			 data_array[0]=0x00110500;
			 dsi_set_cmdq(data_array, 1, 1);
			 MDELAY(120);
	 
			 data_array[0]=0x00290500;
			 dsi_set_cmdq(data_array, 1, 1);
			 MDELAY(20);
#endif

     //   push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    unsigned int data_array[16];

    data_array[0]=0x00280500; // Display Off
    dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);

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
#else
#endif
    lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_compare_id(void)
{
    unsigned int id,id0,id1, id2, id3,id4;
    unsigned char buffer[5];
    unsigned int array[5];
#ifdef BUILD_LK
    upmu_set_rg_vgp6_vosel(6);
    upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms

    array[0]=0x00043902;
    array[1]=0x9483FFB9;// page enable
    dsi_set_cmdq(&array, 2, 1);
    MDELAY(10);

    array[0]=0x00023902;
    array[1]=0x000013ba;
    dsi_set_cmdq(&array, 2, 1);
    MDELAY(10);

    array[0] = 0x00023700;// return byte number
    dsi_set_cmdq(&array, 1, 1);
    MDELAY(10);

    read_reg_v2(0xF4, buffer, 2);
    id = buffer[0]; 

    return (LCM_ID_HX8394 == id)?1:0;
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8394_lg47_truly_lcm_drv = 
{
    .name			= "hx8394_lg47",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};
