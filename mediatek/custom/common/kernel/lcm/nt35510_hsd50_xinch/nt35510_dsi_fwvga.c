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
#include <linux/kernel.h>
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

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define REGFLAG_DELAY             							(0XFEF)
#define REGFLAG_END_OF_TABLE      							(0xFFF)	// END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#define LCM_ID       (0x55)
#define LCM_ID1       (0xC1)
#define LCM_ID2       (0x80)
#define GPIO_LCD_RST_EN      (GPIO131)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
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

	//LV2 Page 1 enable
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
	{REGFLAG_DELAY, 1, {}},

	//AVDD Set AVDD 5.2V
	{0xB0,3,{0x0D,0x0D,0x0D}},
	{REGFLAG_DELAY, 1, {}},

	{0xB6,3,{0x44,0x44,0x44}},
	{REGFLAG_DELAY, 10, {}},

	//AVEE  -5.2V
	{0xB1,3,{0x0d,0x0d,0x0d}},
	{REGFLAG_DELAY, 1, {}},

	{0xB7,3,{0x34,0x34,0x34}},
	{REGFLAG_DELAY, 1, {}},

	//VCL  -2.5V
	{0xB2,3,{0x00,0x00,0x00}},
	{REGFLAG_DELAY, 1, {}},

	{0xB8,3,{0x24,0x24,0x24}},
	{REGFLAG_DELAY, 1, {}},

	//VGH 15V  (Free pump)
	{0xBF,1,{0x01}},
	{REGFLAG_DELAY, 1, {}},

	{0xB3,3,{0x0f,0x0f,0x0f}},
	{REGFLAG_DELAY, 1, {}},

	{0xB9,3,{0x34,0x34,0x34}},
	{REGFLAG_DELAY, 1, {}},

	//VGL_REG -10V
	{0xB5,3,{0x08,0x08,0x08}},
	{REGFLAG_DELAY, 1, {}},

	{0xC2,1,{0x03}},
	{REGFLAG_DELAY, 1, {}},

	{0xBA,3,{0x34,0x34,0x34}},
	{REGFLAG_DELAY, 1, {}},

	//VGMP/VGSP 4.5V/0V
	{0xBC,3,{0x00,0x78,0x00}},
	{REGFLAG_DELAY, 1, {}},

	//VGMN/VGSN -4.5V/0V
	{0xBD,3,{0x00,0x78,0x00}},
	{REGFLAG_DELAY, 1, {}},

	//VCOM  -1.25V
	{0xBE,2,{0x00,0x67}},
	{REGFLAG_DELAY, 1, {}},

	//Gamma Setting
	{0xD1,52,{0x00,0x2a,0x00,0x2b,0x00,0x2f,0x00,0x3c,0x00,0x4c,
	                0x00,0x75,0x00,0x9f,0x00,0xe1,0x01,0x12,0x01,0x56,
	                0x01,0x85,0x01,0xca,0x01,0xfd,0x01,0xfe,0x02,0x2c,
	                0x02,0x58,0x02,0x70,0x02,0x8b,0x02,0x9c,0x02,0xb1,
	                0x02,0xbe,0x02,0xd0,0x02,0xdc,0x02,0xec,0x03,0x09,
	                0x03,0x2e}},
	{REGFLAG_DELAY, 1, {}},

	{0xD2,52,{0x00,0x2a,0x00,0x2b,0x00,0x2f,0x00,0x3c,0x00,0x4c,
	                0x00,0x75,0x00,0x9f,0x00,0xe1,0x01,0x12,0x01,0x56,
	                0x01,0x85,0x01,0xca,0x01,0xfd,0x01,0xfe,0x02,0x2c,
	                0x02,0x58,0x02,0x70,0x02,0x8b,0x02,0x9c,0x02,0xb1,
	                0x02,0xbe,0x02,0xd0,0x02,0xdc,0x02,0xec,0x03,0x09,
	                0x03,0x2e}},
	{REGFLAG_DELAY, 1, {}},

	{0xD3,52,{0x00,0x2a,0x00,0x2b,0x00,0x2f,0x00,0x3c,0x00,0x4c,
	                0x00,0x75,0x00,0x9f,0x00,0xe1,0x01,0x12,0x01,0x56,
	                0x01,0x85,0x01,0xca,0x01,0xfd,0x01,0xfe,0x02,0x2c,
	                0x02,0x58,0x02,0x70,0x02,0x8b,0x02,0x9c,0x02,0xb1,
	                0x02,0xbe,0x02,0xd0,0x02,0xdc,0x02,0xec,0x03,0x09,
	                0x03,0x2e}},
	{REGFLAG_DELAY, 1, {}},

	{0xD4,52,{0x00,0x2a,0x00,0x2b,0x00,0x2f,0x00,0x3c,0x00,0x4c,
	                0x00,0x75,0x00,0x9f,0x00,0xe1,0x01,0x12,0x01,0x56,
	                0x01,0x85,0x01,0xca,0x01,0xfd,0x01,0xfe,0x02,0x2c,
	                0x02,0x58,0x02,0x70,0x02,0x8b,0x02,0x9c,0x02,0xb1,
	                0x02,0xbe,0x02,0xd0,0x02,0xdc,0x02,0xec,0x03,0x09,
	                0x03,0x2e}},
	{REGFLAG_DELAY, 1, {}},

	{0xD5,52,{0x00,0x2a,0x00,0x2b,0x00,0x2f,0x00,0x3c,0x00,0x4c,
	                0x00,0x75,0x00,0x9f,0x00,0xe1,0x01,0x12,0x01,0x56,
	                0x01,0x85,0x01,0xca,0x01,0xfd,0x01,0xfe,0x02,0x2c,
	                0x02,0x58,0x02,0x70,0x02,0x8b,0x02,0x9c,0x02,0xb1,
	                0x02,0xbe,0x02,0xd0,0x02,0xdc,0x02,0xec,0x03,0x09,
	                0x03,0x2e}},
	{REGFLAG_DELAY, 1, {}},

	{0xD6,52,{0x00,0x2a,0x00,0x2b,0x00,0x2f,0x00,0x3c,0x00,0x4c,
	                0x00,0x75,0x00,0x9f,0x00,0xe1,0x01,0x12,0x01,0x56,
	                0x01,0x85,0x01,0xca,0x01,0xfd,0x01,0xfe,0x02,0x2c,
	                0x02,0x58,0x02,0x70,0x02,0x8b,0x02,0x9c,0x02,0xb1,
	                0x02,0xbe,0x02,0xd0,0x02,0xdc,0x02,0xec,0x03,0x09,
	                0x03,0x2e}},
	{REGFLAG_DELAY, 1, {}},

	//LV2 Page 0 enable
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
	{REGFLAG_DELAY, 1, {}},

	//480x854
	{0xB5,1,{0x6B}},
	{REGFLAG_DELAY, 1, {}},

	//Display control
	{0xB1,2,{0xCC,0x00}},
	{REGFLAG_DELAY, 1, {}},

	//Source hold time
	{0xB6,1,{0x05}},
	{REGFLAG_DELAY, 1, {}},

	//Gate EQ control
	{0xB7,2,{0x70,0x70}},
	{REGFLAG_DELAY, 1, {}},

	//Source EQ control (Mode 2)
	{0xB8,4,{0x01,0x03,0x03,0x03}},
	{REGFLAG_DELAY, 1, {}},

	//Inversion mode  (2-dot)
	{0xBC,3,{0x02,0x00,0x00}},
	{REGFLAG_DELAY, 1, {}},

	//Frame rate
	{0xBD,5,{0x01,0x6c,0x1e,0x1d,0x00}},
	{REGFLAG_DELAY, 1, {}},

	//Timing control 4H w/ 4-delay
	{0xC9,5,{0xD0,0x02,0x50,0x50,0x50}},
	{REGFLAG_DELAY, 1, {}},

	{0x36,1,{0x00}},
	{REGFLAG_DELAY, 1, {}},

	{0x35,1,{0x00}},
	{REGFLAG_DELAY, 1, {}},

        {0x11,  0,      {0x00}},
        {REGFLAG_DELAY, 120, {}},
        {0x29,  0,      {0x00}},
        {REGFLAG_DELAY, 20, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {

		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count,
					table[i].para_list, force_update);
			MDELAY(2);
		}
	}
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS * util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS * params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

// enable tearing-free
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;

	params->dsi.mode = CMD_MODE;

// DSI
/* Command mode setting */
	params->dsi.LANE_NUM = LCM_TWO_LANE;
//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=FRAME_WIDTH*3;

// Highly depends on LCD driver capability.
	params->dsi.packet_size = 256;

// Bit rate calculation
// 1 Every lane speed
	params->dsi.pll_div1=0; 	// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=1; 	// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div =33;	// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

//*************Enable CMD2 Page1  *******************//
	data_array[0] = 0x00053902;
	data_array[1] = 0x2555AAFF;
	data_array[2] = 0x00000001;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00093902;
	data_array[1] = 0x000201F8;
	data_array[2] = 0x00133320;
	data_array[3] = 0x00000048;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x00063902;
	data_array[1] = 0x52AA55F0;
	data_array[2] = 0x00000108;
	dsi_set_cmdq(data_array, 3, 1);


//AVDD: 5.0V
	data_array[0] = 0x00043902;
	data_array[1] = 0x0D0D0DB0;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x343434B6;
	dsi_set_cmdq(data_array, 2, 1);//44

//AVEE: -5.0V
	data_array[0] = 0x00043902;
	data_array[1] = 0x0D0D0DB1;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x343434B7;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x000000B2;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x242424B8;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001BF;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x0F0F0FB3;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x343434B9;
	dsi_set_cmdq(data_array, 2, 1);

//#VGLX:-13V
	data_array[0] = 0x00043902;
	data_array[1] = 0x080808B5;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x242424BA;
	dsi_set_cmdq(data_array, 2, 1);//34

	data_array[0] = 0x00043902;
	data_array[1] = 0x000003C2;
	dsi_set_cmdq(data_array, 2, 1);

//#VGMP:4.7V  /VGSP:0V
	data_array[0] = 0x00043902;
	data_array[1] = 0x007800BC;
	dsi_set_cmdq(data_array, 2, 1);

//#VGMN:-4.7V /VGSN:0V
	data_array[0] = 0x00043902;
	data_array[1] = 0x007800BD;
	dsi_set_cmdq(data_array, 2, 1);

//##VCOM  Setting
	data_array[0] = 0x00033902;
	data_array[1] = 0x005500BE;//55 59 64
	dsi_set_cmdq(data_array, 2, 1);//5B

//# R+
	data_array[0] = 0x00353902;
	data_array[1] = 0x001F00D1;
	data_array[2] = 0x00340020;
	data_array[3] = 0x00750055;
	data_array[4] = 0x01CF00A9;
	data_array[5] = 0x0133010B;
	data_array[6] = 0x019E0171;
	data_array[7] = 0x020E02DE;
	data_array[8] = 0x023C0210;
	data_array[9] = 0x027F0267;
	data_array[10] = 0x02AE029C;
	data_array[11] = 0x02D302C5;
	data_array[12] = 0x03F402E6;
	data_array[13] = 0x03390308;
	data_array[14] = 0x000000FA;
	dsi_set_cmdq(data_array, 15, 1);

//#G +
	data_array[0] = 0x00353902;
	data_array[1] = 0x001F00D2;
	data_array[2] = 0x00340020;
	data_array[3] = 0x00750055;
	data_array[4] = 0x01CF00A9;
	data_array[5] = 0x0133010B;
	data_array[6] = 0x019E0171;
	data_array[7] = 0x020E02DE;
	data_array[8] = 0x023C0210;
	data_array[9] = 0x027F0267;
	data_array[10] = 0x02AE029C;
	data_array[11] = 0x02D302C5;
	data_array[12] = 0x03F402E6;
	data_array[13] = 0x03390308;
	data_array[14] = 0x000000FA;
	dsi_set_cmdq(data_array, 15, 1);

//#B +
	data_array[0] = 0x00353902;
	data_array[1] = 0x001F00D3;
	data_array[2] = 0x00340020;
	data_array[3] = 0x00750055;
	data_array[4] = 0x01CF00A9;
	data_array[5] = 0x0133010B;
	data_array[6] = 0x019E0171;
	data_array[7] = 0x020E02DE;
	data_array[8] = 0x023C0210;
	data_array[9] = 0x027F0267;
	data_array[10] = 0x02AE029C;
	data_array[11] = 0x02D302C5;
	data_array[12] = 0x03F402E6;
	data_array[13] = 0x03390308;
	data_array[14] = 0x000000FA;
	dsi_set_cmdq(data_array, 15, 1);

//#R -
	data_array[0] = 0x00353902;
	data_array[1] = 0x001F00D4;
	data_array[2] = 0x00340020;
	data_array[3] = 0x00750055;
	data_array[4] = 0x01CF00A9;
	data_array[5] = 0x0133010B;
	data_array[6] = 0x019E0171;
	data_array[7] = 0x020E02DE;
	data_array[8] = 0x023C0210;
	data_array[9] = 0x027F0267;
	data_array[10] = 0x02AE029C;
	data_array[11] = 0x02D302C5;
	data_array[12] = 0x03F402E6;
	data_array[13] = 0x03390308;
	data_array[14] = 0x000000FA;
	dsi_set_cmdq(data_array, 15, 1);

//#G -
	data_array[0] = 0x00353902;
	data_array[1] = 0x001F00D5;
	data_array[2] = 0x00340020;
	data_array[3] = 0x00750055;
	data_array[4] = 0x01CF00A9;
	data_array[5] = 0x0133010B;
	data_array[6] = 0x019E0171;
	data_array[7] = 0x020E02DE;
	data_array[8] = 0x023C0210;
	data_array[9] = 0x027F0267;
	data_array[10] = 0x02AE029C;
	data_array[11] = 0x02D302C5;
	data_array[12] = 0x03F402E6;
	data_array[13] = 0x03390308;
	data_array[14] = 0x000000FA;
	dsi_set_cmdq(data_array, 15, 1);

//#B -
	data_array[0] = 0x00353902;
	data_array[1] = 0x001F00D6;
	data_array[2] = 0x00340020;
	data_array[3] = 0x00750055;
	data_array[4] = 0x01CF00A9;
	data_array[5] = 0x0133010B;
	data_array[6] = 0x019E0171;
	data_array[7] = 0x020E02DE;
	data_array[8] = 0x023C0210;
	data_array[9] = 0x027F0267;
	data_array[10] = 0x02AE029C;
	data_array[11] = 0x02D302C5;
	data_array[12] = 0x03F402E6;
	data_array[13] = 0x03390308;
	data_array[14] = 0x000000FA;
	dsi_set_cmdq(data_array, 15, 1);

//#######  ENABLE PAGE 0  ############
	data_array[0] = 0x00063902;
	data_array[1] = 0x52AA55F0;
	data_array[2] = 0x00000008;
	dsi_set_cmdq(data_array, 3, 1);
//###################################

	data_array[0] = 0x6BB51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00006BB5;
	dsi_set_cmdq(data_array, 2, 1);

//#Display option control-> video mode
	data_array[0] = 0x00033902;
	data_array[1] = 0x0000CCB1;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000005B6;//05->07
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00033902;
	data_array[1] = 0x007070B7;//70->75
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00053902;
	data_array[1] = 0x050501B8;//03
	data_array[2] = 0x00000005;
	dsi_set_cmdq(data_array, 3, 1);

//#Gate EQ for rising edge
	data_array[0] = 0x00043902;
	data_array[1] = 0x000000BC;
	dsi_set_cmdq(data_array, 2, 1);

//#Source EQ
	data_array[0] = 0x00063902;
	data_array[1] = 0x1C8401BD;
	data_array[2] = 0x0000001C;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00063902;
	data_array[1] = 0x5002D0C9;
	data_array[2] = 0x00005050;
	dsi_set_cmdq(data_array, 3, 1);

	//data_array[0] = 0x00033902;
	//data_array[1] = 0x00FA0044;
	//dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00351500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000773A;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);

}

static void lcm_init(void)
{
#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif
MDELAY(120);

#if 0
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);
#else
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(120);
#endif
	//init_lcm_registers();
	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) /
		   sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00280500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(100);

	data_array[0] = 0x00100500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
}

static void lcm_update(unsigned int x, unsigned int y,
		       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] =
	    (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 0);

	data_array[0] = 0x00053902;
	data_array[1] =
	    (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 0);

	data_array[0] = 0x002c3909;

	dsi_set_cmdq(data_array, 1, 0);

}


static void lcm_resume(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(150);

}

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0, id2 = 0;
	unsigned char buffer[2];
	unsigned int data_array[16];

#if 0
	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);
#else
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(120);
#endif

//*************Enable CMD2 Page1  *******************//
	data_array[0] = 0x00063902;
	data_array[1] = 0x52AA55F0;
	data_array[2] = 0x00000108;
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(10);

	data_array[0] = 0x00023700;	// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xC5, buffer, 2);
	id = buffer[0];		//we only need ID
	id2 = buffer[1];	//we test buffer 1

	return (LCM_ID == id) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------

LCM_DRIVER nt35510_lcm_drv = {
	.name = "nt35510_hsd50_xinchuang",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.update = lcm_update
};
