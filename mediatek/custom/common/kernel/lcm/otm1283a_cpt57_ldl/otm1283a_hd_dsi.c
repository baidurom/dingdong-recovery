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

#define FRAME_WIDTH  			(720)
#define FRAME_HEIGHT 			(1280)
#define LCM_OTM1283_ID			(0x1283)

#define REGFLAG_DELAY          		(0XFEE)
#define REGFLAG_END_OF_TABLE      	(0xFFFF)	// END OF REGISTERS MARKER

#ifndef FALSE
#define FALSE 0
#endif
#define GPIO_LCD_RST_EN      (GPIO131)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    		(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 					(lcm_util.udelay(n))
#define MDELAY(n) 					(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)        (lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update)		(lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd)						(lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums)			(lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg						(lcm_util.dsi_read_reg())
#define read_reg_v2(cmd, buffer, buffer_size)   		(lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

static struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {

	/*
	   Note :

	   Data ID will depends on the following rule.

	   count of parameters > 1      => Data ID = 0x39
	   count of parameters = 1      => Data ID = 0x15
	   count of parameters = 0      => Data ID = 0x05

	   Structure Format :

	   {DCS command, count of parameters, {parameter list}}
	   {REGFLAG_DELAY, milliseconds of time, {}},

	   ...

	   Setting ending by predefined flag

	   {REGFLAG_END_OF_TABLE, 0x00, {}}
	 */
	{0x00, 1, {0x00}},	//
	{0xff, 3, {0x12, 0x83, 0x01}},

	{0x00, 1, {0x80}},	//
	{0xff, 2, {0x12, 0x83}},

	{0x00, 1, {0x92}},	//
	{0xff, 2, {0x30, 0x02}},	// 1lane:00h/2lane:10h/3lane:20h/4lane:30h

	{0x00, 1, {0x80}},
	{0xc0, 9, {0x00, 0x64, 0x00, 0x10,
		   0x10, 0x00, 0x64, 0x10,
		   0x10}},

	{0x00, 1, {0x90}},
	{0xc0, 6, {0x00, 0x5C, 0x00, 0x01,
		   0x00, 0x04}},

	{0x00, 1, {0xb3}},
	{0xc0, 2, {0x00, 0x50}},

	{0x00, 1, {0x81}},
	{0xc1, 1, {0x55}},	//0x55

	{0x00, 1, {0xa0}},
	{0xc4, 14, {0x05, 0x10, 0x06, 0x02,
		    0x05, 0x15, 0x10, 0x05,
		    0x10, 0x07, 0x02, 0x05,
		    0x15, 0x10}},

	{0x00, 1, {0xb0}},
	{0xc4, 2, {0x00, 0x00}},

	{0x00, 1, {0x91}},
	{0xc5, 2, {0x46, 0x40}},

	{0x00, 1, {0x00}},
	{0xd8, 2, {0xBc, 0xBc}},

	{0x00, 1, {0x00}},
	{0xd9, 1, {0x44}},	//VCOMDC

	{0x00, 1, {0xb0}},
	{0xc5, 2, {0x04, 0xb8}},

	{0x00, 1, {0x00}},	////ID1
	{0xd0, 1, {0x40}},

	{0x00, 1, {0x00}},	//ID2 & ID3
	{0xd1, 2, {0x00, 0x00}},

	{0x00, 1, {0x80}},
	{0xcb, 11, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0x90}},
	{0xcb, 15, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0xa0}},
	{0xcb, 15, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0xb0}},
	{0xcb, 15, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0xc0}},
	{0xcb, 15, {0x05, 0x00, 0x05, 0x05,
		    0x05, 0x05, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0xd0}},
	{0xcb, 15, {0x00, 0x00, 0x00, 0x05,
		    0x00, 0x05, 0x05, 0x05,
		    0x00, 0x05, 0x05, 0x05,
		    0x05, 0x00, 0x00}},

	{0x00, 1, {0xe0}},
	{0xcb, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x05, 0x00,
		    0x05, 0x05}},

	{0x00, 1, {0xf0}},
	{0xcb, 11, {0xFF, 0xFF, 0xFF, 0xFF,
		    0xFF, 0xFF, 0xFF, 0xFF,
		    0xFF, 0xFF, 0xFF}},

	{0x00, 1, {0x80}},
	{0xcc, 15, {0x02, 0x00, 0x0a, 0x0e,
		    0x0c, 0x10, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0x90}},
	{0xcc, 15, {0x00, 0x00, 0x00, 0x06,
		    0x00, 0x2e, 0x2d, 0x01,
		    0x00, 0x09, 0x0d, 0x0b,
		    0x0f, 0x00, 0x00}},

	{0x00, 1, {0xa0}},
	{0xcc, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x05, 0x00,
		    0x2e, 0x2d}},

	{0x00, 1, {0xb0}},
	{0xcc, 15, {0x05, 0x00, 0x0f, 0x0b,
		    0x0d, 0x09, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0x00, 1, {0xc0}},
	{0xcc, 15, {0x00, 0x00, 0x00, 0x01,
		    0x00, 0x2d, 0x2e, 0x06,
		    0x00, 0x10, 0x0c, 0x0e,
		    0x0a, 0x00, 0x00}},

	{0x00, 1, {0xd0}},
	{0xcc, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x02, 0x00,
		    0x2d, 0x2e}},

	{0x00, 1, {0x80}},
	{0xce, 12, {0x87, 0x03, 0x18, 0x86,
		    0x03, 0x18, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00}},

	{0x00, 1, {0x90}},
	{0xce, 14, {0x34, 0xFe, 0x18, 0x34,
		    0xFf, 0x18, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00}},

	{0x00, 1, {0xa0}},
	{0xce, 14, {0x38, 0x03, 0x05, 0x00,
		    0x00, 0x18, 0x00, 0x38,
		    0x02, 0x05, 0x01, 0x00,
		    0x18, 0x00}},

	{0x00, 1, {0xb0}},
	{0xce, 14, {0x38, 0x01, 0x05, 0x02,
		    0x00, 0x18, 0x00, 0x38,
		    0x00, 0x05, 0x03, 0x00,
		    0x18, 0x00}},

	{0x00, 1, {0xc0}},
	{0xce, 14, {0x30, 0x00, 0x05, 0x04,
		    0x00, 0x18, 0x00, 0x30,
		    0x01, 0x05, 0x05, 0x00,
		    0x18, 0x00}},

	{0x00, 1, {0xd0}},
	{0xce, 14, {0x30, 0x02, 0x05, 0x06,
		    0x00, 0x18, 0x00, 0x30,
		    0x03, 0x05, 0x07, 0x00,
		    0x18, 0x00}},

	{0x00, 1, {0x80}},
	{0xcf, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00}},

	{0x00, 1, {0x90}},
	{0xcf, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00}},

	{0x00, 1, {0xa0}},
	{0xcf, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00}},

	{0x00, 1, {0xb0}},
	{0xcf, 14, {0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00}},

	{0x00, 1, {0xc0}},
	{0xcf, 11, {0x01, 0x01, 0x20, 0x20,
		    0x00, 0x00, 0x01, 0x81,
		    0x00, 0x03, 0x08}},

	{0x00, 1, {0x00}},
	{0xe1, 16, {0x00, 0x05, 0x09, 0x0d,
		    0x06, 0x0e, 0x0d, 0x0c,
		    0x02, 0x05, 0x11, 0x0f,
		    0x17, 0x25, 0x1b, 0x06}},

	{0x00, 1, {0x00}},
	{0xe2, 16, {0x00, 0x05, 0x09, 0x0d,
		    0x06, 0x0e, 0x0d, 0x0c,
		    0x02, 0x05, 0x11, 0x0f,
		    0x17, 0x25, 0x1b, 0x06}},

	{0x00, 1, {0xb5}},
	{0xc5, 6, {0x37, 0xf1, 0xfd, 0x37,
		   0xf1, 0xfd}},

	{0x00, 1, {0x81}},
	{0xc4, 1, {0x82}},

	{0x00, 1, {0xa4}},
	{0xc0, 1, {0x22}},

	{0x00, 1, {0xB2}},
	{0xF5, 2, {0x00, 0x00}},

	{0x00, 1, {0xB4}},
	{0xF5, 2, {0x00, 0x00}},

	{0x00, 1, {0x6}},
	{0xF5, 2, {0x00, 0x00}},

	{0x00, 1, {0xB8}},
	{0xF5, 2, {0x00, 0x00}},

	{0x00, 1, {0xB4}},	//timing setting
	{0xc5, 1, {0xC0}},

	{0x00, 1, {0xa0}},
	{0xd6, 12, {0x01, 0x0d, 0x01, 0x0d,
		    0x01, 0x80, 0x01, 0xc0,
		    0x01, 0xc0, 0x01, 0xc0}},

	{0x00, 1, {0xb0}},
	{0xd6, 12, {0x01, 0xc0, 0x01, 0xc0,
		    0x01, 0xc0, 0x01, 0xc0,
		    0x01, 0xc0, 0x01, 0xc0}},

	{0x00, 1, {0xc0}},
	{0xd6, 12, {0x44, 0x11, 0x09, 0x55,
		    0x11, 0x80, 0x80, 0x11,
		    0x80, 0x80, 0x11, 0x80}},

	{0x00, 1, {0xd0}},
	{0xd6, 6, {0x80, 0x11, 0x80, 0x80,
		   0x11, 0x80}},

	{0x00, 1, {0xe0}},
	{0xd6, 12, {0x22, 0x11, 0x40, 0x2b,
		    0x11, 0x40, 0x40, 0x11,
		    0x40, 0x40, 0x11, 0x40}},

	{0x00, 1, {0xf0}},
	{0xd6, 6, {0x40, 0x11, 0x40, 0x40,
		   0x11, 0x40}},

	{0x00, 1, {0x00}},
	{0x55, 1, {0xb0}},

	{0x00, 1, {0x00}},
	{0xff, 3, {0x00, 0x00, 0x00}},

	{0x3A, 1, {0x77}},	//

	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},

	{0x29, 1, {0x00}},	//  Display On
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


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
	{REGFLAG_DELAY, 100, {}},

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
		}

	}

}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

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
	params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;

	params->dsi.mode = SYNC_PULSE_VDO_MODE;	//BURST_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.intermediat_buffer_num = 0;	//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count = 720 * 3;

	params->dsi.vertical_sync_active = 3;	//2;  //---3
	params->dsi.vertical_backporch = 8;	//9; //---14
	params->dsi.vertical_frontporch = 8;	//44;  //----8
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 11;	//2;  //----2
	params->dsi.horizontal_backporch = 0x40;	//60; //----28
	params->dsi.horizontal_frontporch = 0x40;	//20; //----50
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	// Bit rate calculation
	//1 Every lane speed
	params->dsi.pll_div1 = 0;	// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2 = 1;	// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div = 14;	//12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

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
	MDELAY(20);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(120);

	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) /
		   sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00280500;	// Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00100500;	// Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(200);
#ifdef BUILD_LK
	upmu_set_rg_vgp6_en(0);
#else
	hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif
}


static void lcm_resume(void)
{
	lcm_init();
}


static unsigned int lcm_compare_id(void)
{
	unsigned int id0, id1, id2, id3, id4;
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
	printf("%s, Module ID = {%x, %x, %x, %x, %x} \n", __func__, id0,
	       id1, id2, id3, id4);
#else
	printk("%s, Module ID = {%x, %x, %x, %x,%x} \n", __func__, id0,
	       id1, id2, id3, id4);
#endif

	return (LCM_OTM1283_ID == ((id2 << 8) | id3)) ? 1 : 0;
}

//

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1283_auo_lcm_drv = {
	.name = "otm1283a_cpt57_ldl",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
};
