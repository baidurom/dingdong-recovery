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

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)

#define REGFLAG_DELAY             							0xEE
#define REGFLAG_END_OF_TABLE      							0xEF   // END OF REGISTERS MARKER

#define GPIO_LCD_RST_EN      GPIO131
#define LCM_DSI_CMD_MODE									1
#define LCM_NT35516_ID       (0x5516)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
static struct LCM_setting_table lcm_compare_id_setting[] = {
    // Display off sequence
    {0xf0,  5,      {0x55,0xaa,0x52,0x08,0x01}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_initialization_setting[] = {

         /*
         Note :

         Data ID will depends on the following rule.

                 count of parameters > 1 => Data ID = 0x39
                 count of parameters = 1 => Data ID = 0x15
                 count of parameters = 0 => Data ID = 0x05

         Structure Format :

         {DCS command, count of parameters, {parameter list}}
         {REGFLAG_DELAY, milliseconds of time, {}},

         ...

         Setting ending by predefined flag

         {REGFLAG_END_OF_TABLE, 0x00, {}}
         */

	{0xFF,4,{0xAA,0x55,0x25,0x01}}, //P4
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}}, //P5   Page 0.
	{0xB1,1,{0xEC}}, //P1    Video Mode Enable,Gate Scan direction.
	{0xB8,4,{0x01,0x02,0x02,0x02}}, //P4   EQS3=1us
	{0xBC,3,{0x05,0x05,0x05}}, //P3   Zig-Zag patial mode
	{0xB7,2,{0x00,0x00}}, //P2    GREQ_CKx=GFEQ_CKx=0us
	{0xC8,18,{0x01,0x00,0x46,0x64,
			0x46,0x64,0x46,0x64,
			0x46,0x64,0x64,0x3C,
			0x3C,0x64,0x64,0x3C,
			0x3C,0x64}}, //P18  delay time for V_END_R signal
	{0xBD,5,{0x01,0x41,0x08,0x40,0x01}}, //P3  VGMN=-0.5V
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}}, //P5  Page 1.
	{0xB0,3,{0x05,0x05,0x05}}, //P3
	{0xB6,3,{0x44,0x44,0x44}}, //P3   2.5xVDDB
	{0xB1,3,{0x05,0x05,0x05}}, //P3
	{0xB7,3,{0x34,0x34,0x34}}, //P3
	{0xB3,3,{0x13,0x13,0x13}}, //P3
	{0xB4,3,{0x0A,0x0A,0x0A}}, //P3
	{0xBC,3,{0x00,0x90,0x11}}, //P3     VGMP=+0.5V
	{0xBD,3,{0x00,0x90,0x11}}, //P3  VGMN=-0.5V
	{0xBE,1,{0x51}}, //P1  VCOM=-1.3V
	{0xD1,16,{0x00,0x17,0x00,0x24,
			0x00,0x3D,0x00,0x52,
			0x00,0x66,0x00,0x86,
			0x00,0xA0,0x00,0xCC}}, //P16  V23
	{0xD2,16,{0x00,0xF1,0x01,0x26,
			0x01,0x4E,0x01,0x8C,
			0x01,0xBC,0x01,0xBE,
			0x01,0xE7,0x02,0x0E}}, //P16  V192
	{0xD3,16,{0x02,0x22,0x02,0x3C,
			0x02,0x4F,0x02,0x71,
			0x02,0x90,0x02,0xC6,
			0x02,0xF1,0x03,0x3A}}, //P16  V252
	{0xD4,4,{0x03,0xB5,0x03,0xC1}}, //P4   V255
	{0xD5,16,{0x00,0x17,0x00,0x24,
			0x00,0x3D,0x00,0x52,
			0x00,0x66,0x00,0x86,
			0x00,0xA0,0x00,0xCC}}, //P16  V23
	{0xD6,16,{0x00,0xF1,0x01,0x26,
			0x01,0x4E,0x01,0x8C,
			0x01,0xBC,0x01,0xBE,
			0x01,0xE7,0x02,0x0E}}, //P16  V192
	{0xD7,16,{0x02,0x22,0x02,0x3C,
			0x02,0x4F,0x02,0x71,
			0x02,0x90,0x02,0xC6,
			0x02,0xF1,0x03,0x3A}}, //P16   V252
	{0xD8,4,{0x03,0xB5,0x03,0xC1}}, //P4  V255
	{0xD9,16,{0x00,0x17,0x00,0x24,
			0x00,0x3D,0x00,0x52,
			0x00,0x66,0x00,0x86,
			0x00,0xA0,0x00,0xCC}}, //P16  V23
	{0xDD,16,{0x00,0xF1,0x01,0x26,
			0x01,0x4E,0x01,0x8C,
			0x01,0xBC,0x01,0xBE,
			0x01,0xE7,0x02,0x0E}}, //P16   V192
	{0xDE,16,{0x02,0x22,0x02,0x3C,
			0x02,0x4F,0x02,0x71,
			0x02,0x90,0x02,0xC6,
			0x02,0xF1,0x03,0x3A}}, //P16   V252
	{0xDF,4,{0x03,0xB5,0x03,0xC1}}, //P4   V255
	{0xE0,16,{0x00,0x17,0x00,0x24,
			0x00,0x3D,0x00,0x52,
			0x00,0x66,0x00,0x86,
			0x00,0xA0,0x00,0xCC}}, //P16   V23
	{0xE1,16,{0x00,0xF1,0x01,0x26,
			0x01,0x4E,0x01,0x8C,
			0x01,0xBC,0x01,0xBE,
			0x01,0xE7,0x02,0x0E}}, //P16  V192
	{0xE2,16,{0x02,0x22,0x02,0x3C,
			0x02,0x4F,0x02,0x71,
			0x02,0x90,0x02,0xC6,
			0x02,0xF1,0x03,0x3A}}, //P16  V252
	{0xE3,4,{0x03,0xB5,0x03,0xC1}}, //P4  V255
	{0xE4,16,{0x00,0x17,0x00,0x24,
			0x00,0x3D,0x00,0x52,
			0x00,0x66,0x00,0x86,
			0x00,0xA0,0x00,0xCC}}, //P16  V23
	{0xE5,16,{0x00,0xF1,0x01,0x26,
			0x01,0x4E,0x01,0x8C,
			0x01,0xBC,0x01,0xBE,
			0x01,0xE7,0x02,0x0E}}, //P16  V192
	{0xE6,16,{0x02,0x22,0x02,0x3C,
			0x02,0x4F,0x02,0x71,
			0x02,0x90,0x02,0xC6,
			0x02,0xF1,0x03,0x3A}}, //P16  V252
	{0xE7,4,{0x03,0xB5,0x03,0xC1}}, //P4  V255
	{0xE8,16,{0x00,0x17,0x00,0x24,
			0x00,0x3D,0x00,0x52,
			0x00,0x66,0x00,0x86,
			0x00,0xA0,0x00,0xCC}}, //P16  V23
	{0xE9,16,{0x00,0xF1,0x01,0x26,
			0x01,0x4E,0x01,0x8C,
			0x01,0xBC,0x01,0xBE,
			0x01,0xE7,0x02,0x0E}}, //P16  V192
	{0xEA,16,{0x02,0x22,0x02,0x3C,
			0x02,0x4F,0x02,0x71,
			0x02,0x90,0x02,0xC6,
			0x02,0xF1,0x03,0x3A}}, //P16  V252
	{0xEB,4,{0x03,0xB5,0x03,0xC1}}, //P4  V255
        {0x4c,  1,      {0x11}},
	{0x11,1,{0x00}}, //Tearing Effect On        Sleep Out
	{REGFLAG_DELAY, 120, {}},

	{0x29,1,{0x00}}, //Display On   Display On.
	{REGFLAG_DELAY, 40, {}},
	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	//{0x11, 0, {0x00}},
	// LCM driver IC specifies 15ms needed after sleep out. But we need more delay time to make sure latest RAM data has been refreshed to screen.
       // {REGFLAG_DELAY, 100, {}},
    // Display ON
	//{0x29, 0, {0x00}},
       // {REGFLAG_DELAY, 80, {}},
	{0x28, 0, {0x00}},
        {REGFLAG_DELAY, 35, {}},
	{0x29, 0, {0x00}},
        {REGFLAG_DELAY, 35, {}},
	{0x28, 0, {0x00}},
        {REGFLAG_DELAY, 35, {}},
	{0x29, 0, {0x00}},
        {REGFLAG_DELAY, 35, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
        {REGFLAG_DELAY, 35, {}},
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

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

			// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
		params->dsi.compatibility_for_nvk=1;

		params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage


		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;


                params->dsi.vertical_sync_active                                = 2;  //---3
                params->dsi.vertical_backporch                                  = 10; //---14
                params->dsi.vertical_frontporch                                 = 44;  //----8
                params->dsi.vertical_active_line                                = FRAME_HEIGHT;

                params->dsi.horizontal_sync_active                              = 2;  //----2
                params->dsi.horizontal_backporch                                = 60; //----28
                params->dsi.horizontal_frontporch                               = 20; //----50
                params->dsi.horizontal_active_pixel                             = FRAME_WIDTH;


		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =14;//18;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
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

        mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
        MDELAY(50);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(120);
#if 0
	data_array[0]=0x00053902;
	data_array[1]=0x2555aaff;
	data_array[2]=0x00000001;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00063902;
	data_array[1]=0x52aa55f0;
	data_array[2]=0x00000008;
	dsi_set_cmdq(&data_array,3,1);

        data_array[0]=0x00023902;
	data_array[1]=0x0000ecb1;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00053902;
	data_array[1]=0x020201b8;
	data_array[2]=0x00000002;
	dsi_set_cmdq(&data_array,3,1);

        data_array[0]=0x00043902;
	data_array[1]=0x050505bc;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00033902;
	data_array[1]=0x000000b7;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00133902;
	data_array[1]=0x460001c8;
	data_array[2]=0x46644664;
	data_array[3]=0x64644664;
	data_array[4]=0x64643c3c;
	data_array[5]=0x00643c3c;
	dsi_set_cmdq(&data_array,6,1);

        data_array[0]=0x00063902;
	data_array[1]=0x084101bd;
	data_array[2]=0x00000140;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00063902;
	data_array[1]=0x52aa55f0;
	data_array[2]=0x00000108;
	dsi_set_cmdq(&data_array,3,1);

        data_array[0]=0x00043902;
	data_array[1]=0x050505b0;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x444444b6;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x050505b1;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x343434b7;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x131313b3;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x0a0a0ab4;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x119000bc;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00043902;
	data_array[1]=0x119000bd;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00023902;
	data_array[1]=0x000051be;
	dsi_set_cmdq(&data_array,2,1);

//#Gamma Setting
	data_array[0]=0x00113902;
	data_array[1]=0x001700D1;
	data_array[2]=0x003d0024;
	data_array[3]=0x00660052;
	data_array[4]=0x00a00086;
	data_array[5]=0x000000cc;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x01f100D2;
	data_array[2]=0x014e0126;
	data_array[3]=0x01bc018c;
	data_array[4]=0x02e701be;
	data_array[5]=0x0000000e;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x022202D3;//yufeng
	data_array[2]=0x024f023c;
	data_array[3]=0x02900271;
	data_array[4]=0x03F102c6;
	data_array[5]=0x0000003a;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00053902;
	data_array[1]=0x03b503D4;
	data_array[2]=0x000000c1;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00113902;
	data_array[1]=0x001700D5;
	data_array[2]=0x003d0024;
	data_array[3]=0x00660052;
	data_array[4]=0x00a00086;
	data_array[5]=0x000000cc;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x01f100D6;
	data_array[2]=0x014e0126;
	data_array[3]=0x01bc018c;
	data_array[4]=0x02e701be;
	data_array[5]=0x0000000e;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x022202D7;//yufeng
	data_array[2]=0x024f023c;
	data_array[3]=0x02900271;
	data_array[4]=0x03F102c6;
	data_array[5]=0x0000003a;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00053902;
	data_array[1]=0x03b503D8;
	data_array[2]=0x000000c1;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00113902;
	data_array[1]=0x001700D9;
	data_array[2]=0x003d0024;
	data_array[3]=0x00660052;
	data_array[4]=0x00a00086;
	data_array[5]=0x000000cc;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x01f100DD;
	data_array[2]=0x014e0126;
	data_array[3]=0x01bc018c;
	data_array[4]=0x02e701be;
	data_array[5]=0x0000000e;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x022202DE;//yufeng
	data_array[2]=0x024f023c;
	data_array[3]=0x02900271;
	data_array[4]=0x03F102c6;
	data_array[5]=0x0000003a;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00053902;
	data_array[1]=0x03b503DF;
	data_array[2]=0x000000c1;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00113902;
	data_array[1]=0x001700E0;
	data_array[2]=0x003d0024;
	data_array[3]=0x00660052;
	data_array[4]=0x00a00086;
	data_array[5]=0x000000cc;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x01f100E1;
	data_array[2]=0x014e0126;
	data_array[3]=0x01bc018c;
	data_array[4]=0x02e701be;
	data_array[5]=0x0000000e;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x022202E2;//yufeng
	data_array[2]=0x024f023c;
	data_array[3]=0x02900271;
	data_array[4]=0x03F102c6;
	data_array[5]=0x0000003a;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00053902;
	data_array[1]=0x03b503E3;
	data_array[2]=0x000000c1;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00113902;
	data_array[1]=0x001700E4;
	data_array[2]=0x003d0024;
	data_array[3]=0x00660052;
	data_array[4]=0x00a00086;
	data_array[5]=0x000000cc;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x01f100E5;
	data_array[2]=0x014e0126;
	data_array[3]=0x01bc018c;
	data_array[4]=0x02e701be;
	data_array[5]=0x0000000e;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x022202E6;//yufeng
	data_array[2]=0x024f023c;
	data_array[3]=0x02900271;
	data_array[4]=0x03F102c6;
	data_array[5]=0x0000003a;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00053902;
	data_array[1]=0x03b503E7;
	data_array[2]=0x000000c1;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00113902;
	data_array[1]=0x001700E8;
	data_array[2]=0x003d0024;
	data_array[3]=0x00660052;
	data_array[4]=0x00a00086;
	data_array[5]=0x000000cc;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x01f100E9;
	data_array[2]=0x014e0126;
	data_array[3]=0x01bc018c;
	data_array[4]=0x02e701be;
	data_array[5]=0x0000000e;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00113902;
	data_array[1]=0x022202EA;//yufeng
	data_array[2]=0x024f023c;
	data_array[3]=0x02900271;
	data_array[4]=0x03F102c6;
	data_array[5]=0x0000003a;
	dsi_set_cmdq(&data_array,6,1);

	data_array[0]=0x00053902;
	data_array[1]=0x03b503EB;
	data_array[2]=0x000000c1;
	dsi_set_cmdq(&data_array,3,1);

	data_array[0]=0x00023902;
	data_array[1]=0x0000114C;
	dsi_set_cmdq(&data_array,2,1);

        data_array[0]=0x00351500;
	dsi_set_cmdq(&data_array,1,1);

	data_array[0]=0x00291500;
	dsi_set_cmdq(&data_array,1,1);
	MDELAY(40);

	data_array[0]=0x00110500;
	dsi_set_cmdq(&data_array,1,1);
	MDELAY(120);
#endif
push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

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
	dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);

//	data_array[0]= 0x00290508;
//	dsi_set_cmdq(&data_array, 1, 1);
	//MDELAY(1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);
	//MDELAY(1);

}

static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    unsigned int data_array[16];

    data_array[0]=0x00280500; // Display Off
    dsi_set_cmdq(&data_array, 1, 1);
    MDELAY(100);

    data_array[0] = 0x00100500; // Sleep In
    dsi_set_cmdq(&data_array, 1, 1);
    MDELAY(100);

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
    unsigned int id = 0;
    unsigned char buffer[5];
    unsigned int array[5];
#ifdef BUILD_LK
    upmu_set_rg_vgp6_vosel(6);
    upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif

	//Do reset here
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);

	SET_RESET_PIN(1);
	MDELAY(10);

        push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);

        array[0] = 0x00033700;// read id return two byte,version and id
        dsi_set_cmdq(array, 1, 1);
        read_reg_v2(0xc5, buffer, 3);
        id = ((buffer[0] << 8) | buffer[1]); //we only need ID
#if defined(BUILD_LK)
        printf("%s, id1 = 0x%x,id2 = 0x%x,id3 = 0x%x\n", __func__, buffer[0],buffer[1],buffer[2]);
#endif
        return (LCM_NT35516_ID == id)?1:0;

}


LCM_DRIVER nt35516_auo53_ykl_drv=
{
	.name		= "nt35516_auo53_ykl",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.compare_id    = lcm_compare_id,
	.update         = lcm_update,
#endif
};
