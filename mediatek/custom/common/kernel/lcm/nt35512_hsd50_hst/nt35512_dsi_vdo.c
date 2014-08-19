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

#define GPIO_LCD_RST_EN      (GPIO131)
#define LCM_NT35512_HSD_HST_ID (0x5512)
// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (480)
#define FRAME_HEIGHT (854)
#define LCM_DSI_CMD_MODE	(0)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY (0xFEE)
#define REGFLAG_END_OF_TABLE (0xFDD)	// END OF REGISTERS MARKER

static unsigned int lcm_esd_test = FALSE;	///only for ESD test
// ---------------------------------------------------------------------------
// Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) (lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update) (lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd) (lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums) (lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg											(lcm_util.dsi_read_reg())
#define read_reg_v2(cmd, buffer, buffer_size) (lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xf0, 5, {0x55, 0xaa, 0x52, 0x08, 0x01}},
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

//LV2 Page 1 enable
	{0xF0, 5, {0x55, 0xAA, 0x52, 0x08,
		   0x01}},

//AVDD Set AVDD 5.2V
	{0xB0, 3, {0x0D, 0x0D, 0x0D}},

//AVDD ratio
	{0xB6, 3, {0x44, 0x44, 0x44}},

//AVEE  -5.2V
	{0xB1, 3, {0x0D, 0x0D, 0x0D}},

//AVEE ratio
	{0xB7, 3, {0x34, 0x34, 0x34}},

//VCL  -2.5V
	{0xB2, 3, {0x00, 0x00, 0x00}},

//VCL ratio
	{0xB8, 3, {0x34, 0x34, 0x34}},

//VGH 15V  (Free pump)
	{0xBF, 1, {0x01}},
	{0xB3, 3, {0x0F, 0x0F, 0x0F}},

//VGH ratio
	{0xB9, 3, {0x34, 0x34, 0x34}},

//VGL_REG -10V
	{0xB5, 3, {0x08, 0x08, 0x08}},

	{0xC2, 1, {0x03}},

//VGLX ratio
	{0xBA, 3, {0x24, 0x24, 0x24}},

//VGMP/VGSP 4.5V/0V
	{0xBC, 3, {0x00, 0x78, 0x00}},

//VGMN/VGSN -4.5V/0V
	{0xBD, 3, {0x00, 0x78, 0x00}},

//VCOM  -1.575V
	{0xBE, 2, {0x00, 0x66}},

//Gamma Setting
	{0xD1, 52, {0x00, 0x08, 0x00, 0x09,
		    0x00, 0x11, 0x00, 0x24,
		    0x00, 0x3A, 0x00, 0x69,
		    0x00, 0x95, 0x00, 0xDB,
		    0x01, 0x0E, 0x01, 0x54,
		    0x01, 0x85, 0x01, 0xC8,
		    0x01, 0xFC, 0x01, 0xFD,
		    0x02, 0x29, 0x02, 0x56,
		    0x02, 0x6F, 0x02, 0x8C,
		    0x02, 0x9E, 0x02, 0xB4,
		    0x02, 0xC2, 0x02, 0xD6,
		    0x02, 0xE4, 0x02, 0xF9,
		    0x03, 0x25, 0x03, 0x8E}},

	{0xD2, 52, {0x00, 0x08, 0x00, 0x09,
		    0x00, 0x11, 0x00, 0x24,
		    0x00, 0x3A, 0x00, 0x69,
		    0x00, 0x95, 0x00, 0xDB,
		    0x01, 0x0E, 0x01, 0x54,
		    0x01, 0x85, 0x01, 0xC8,
		    0x01, 0xFC, 0x01, 0xFD,
		    0x02, 0x29, 0x02, 0x56,
		    0x02, 0x6F, 0x02, 0x8C,
		    0x02, 0x9E, 0x02, 0xB4,
		    0x02, 0xC2, 0x02, 0xD6,
		    0x02, 0xE4, 0x02, 0xF9,
		    0x03, 0x25, 0x03, 0x8E}},

	{0xD3, 52, {0x00, 0x08, 0x00, 0x09,
		    0x00, 0x11, 0x00, 0x24,
		    0x00, 0x3A, 0x00, 0x69,
		    0x00, 0x95, 0x00, 0xDB,
		    0x01, 0x0E, 0x01, 0x54,
		    0x01, 0x85, 0x01, 0xC8,
		    0x01, 0xFC, 0x01, 0xFD,
		    0x02, 0x29, 0x02, 0x56,
		    0x02, 0x6F, 0x02, 0x8C,
		    0x02, 0x9E, 0x02, 0xB4,
		    0x02, 0xC2, 0x02, 0xD6,
		    0x02, 0xE4, 0x02, 0xF9,
		    0x03, 0x25, 0x03, 0x8E}},

	{0xD4, 52, {0x00, 0x08, 0x00, 0x09,
		    0x00, 0x11, 0x00, 0x24,
		    0x00, 0x3A, 0x00, 0x69,
		    0x00, 0x95, 0x00, 0xDB,
		    0x01, 0x0E, 0x01, 0x54,
		    0x01, 0x85, 0x01, 0xC8,
		    0x01, 0xFC, 0x01, 0xFD,
		    0x02, 0x29, 0x02, 0x56,
		    0x02, 0x6F, 0x02, 0x8C,
		    0x02, 0x9E, 0x02, 0xB4,
		    0x02, 0xC2, 0x02, 0xD6,
		    0x02, 0xE4, 0x02, 0xF9,
		    0x03, 0x25, 0x03, 0x8E}},

	{0xD5, 52, {0x00, 0x08, 0x00, 0x09,
		    0x00, 0x11, 0x00, 0x24,
		    0x00, 0x3A, 0x00, 0x69,
		    0x00, 0x95, 0x00, 0xDB,
		    0x01, 0x0E, 0x01, 0x54,
		    0x01, 0x85, 0x01, 0xC8,
		    0x01, 0xFC, 0x01, 0xFD,
		    0x02, 0x29, 0x02, 0x56,
		    0x02, 0x6F, 0x02, 0x8C,
		    0x02, 0x9E, 0x02, 0xB4,
		    0x02, 0xC2, 0x02, 0xD6,
		    0x02, 0xE4, 0x02, 0xF9,
		    0x03, 0x25, 0x03, 0x8E}},

	{0xD6, 52, {0x00, 0x08, 0x00, 0x09,
		    0x00, 0x11, 0x00, 0x24,
		    0x00, 0x3A, 0x00, 0x69,
		    0x00, 0x95, 0x00, 0xDB,
		    0x01, 0x0E, 0x01, 0x54,
		    0x01, 0x85, 0x01, 0xC8,
		    0x01, 0xFC, 0x01, 0xFD,
		    0x02, 0x29, 0x02, 0x56,
		    0x02, 0x6F, 0x02, 0x8C,
		    0x02, 0x9E, 0x02, 0xB4,
		    0x02, 0xC2, 0x02, 0xD6,
		    0x02, 0xE4, 0x02, 0xF9,
		    0x03, 0x25, 0x03, 0x8E}},

//LV2 Page 0 enable
	{0xF0, 5, {0x55, 0xAA, 0x52, 0x08,
		   0x00}},

//480x854
	{0xB5, 1, {0x6B}},

//Display control
	{0xB1, 2, {0xFC, 0x00}},

//Source hold time
	{0xB6, 1, {0x05}},

//Gate EQ control
	{0xB7, 2, {0x70, 0x70}},

//Source EQ control (Mode 2)
	{0xB8, 4, {0x01, 0x03, 0x03, 0x03}},

//Inversion mode  (2-dot)
	{0xBC, 3, {0x00, 0x00, 0x00}},

//Frame rate
	{0xBD, 5, {0x01, 0x6C, 0x1E, 0x1D,
		   0x00}},

//Timing control 4H w/ 4-delay
	{0xC9, 5, {0xD0, 0x02, 0x50, 0x50,
		   0x50}},

	{0x36, 1, {0x00}},
	{0x35, 1, {0x00}},

	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	// Sleep Mode On
	{0x10, 1, {0x00}},
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
			MDELAY(2);
		}
	}
}

// ---------------------------------------------------------------------------
// LCM Driver Implementations
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

	params->dsi.mode = BURST_VDO_MODE;
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;


	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 40;	//20
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 4;
	params->dsi.horizontal_backporch = 80;
	params->dsi.horizontal_frontporch = 80;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.pll_div1 = 0;	// div1=0,1,2,3;div1_real=1,2,4,4
	params->dsi.pll_div2 = 1;	// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div = 15;	//32;  //38;  // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

static void lcm_init(void)
{
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
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(120);

	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) /
		   sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	data_array[0] = 0x00002200;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(2);
	data_array[0] = 0x00280500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00100500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(50);
}

static void lcm_resume(void)
{
	unsigned int data_array[16];
	data_array[0] = 0x00110500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00290500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00003200;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(1);
}


static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[5];
	unsigned int array[16];

	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(50);

	push_table(lcm_compare_id_setting,
		   sizeof(lcm_compare_id_setting) /
		   sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00033700;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(5);
	read_reg_v2(0xC5, buffer, 3);
	id = ((buffer[0] << 8) | buffer[1]);	//we only need ID
#if defined(BUILD_LK)
	printf
	    (" [nt35512_auo445_ykl] %s, id = 0x%x,buffer[0] = 0x%x,buffer[1] = 0x%x buffer[2] = 0x%x\n",
	     __func__, id, buffer[0], buffer[1], buffer[2]);
#endif
	return (LCM_NT35512_HSD_HST_ID == id) ? 1 : 0;
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
	char buffer[3];
	int array[4];

	if (lcm_esd_test) {
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0A, buffer, 1);
	if (buffer[0] == 0x9C) {
		return FALSE;
	} else {
		return TRUE;
	}
#endif
}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	return TRUE;
}

LCM_DRIVER nt35512_dsi_lcm_drv = {
	.name = "nt35512_hsd50_hst",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id = lcm_compare_id,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	//.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,
};
