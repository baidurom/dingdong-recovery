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

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include <mach/mt_pm_ldo.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  			(480)
#define FRAME_HEIGHT 			(854)
#define LCM_OTM8009A_HSD45_JQ_ID	(0x8009)

#define REGFLAG_DELAY             				(0XFFE)
#define REGFLAG_END_OF_TABLE      				(0xFFF)	// END OF REGISTERS MARKER

#define GPIO_LCD_RST_EN      GPIO131
#define LCM_DSI_CMD_MODE					(0)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    					(lcm_util.set_reset_pin((v)))
#define SET_RESET_PIN(v)   					(lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(n, v) 					(lcm_util.set_gpio_out((n), (v)))
#define read_reg_v2(cmd, buffer, buffer_size) 			(lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

#define UDELAY(n) 						(lcm_util.udelay(n))
#define MDELAY(n) 						(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	(lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update)		(lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd)						(lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums)			(lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg						(lcm_util.dsi_read_reg())


static struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xff, 3, {0x80, 0x09, 0x01}},

	{0x00, 1, {0x80}},	//Address shift
	{0xff, 2, {0x80, 0x09}},

	{0x00, 1, {0x03}},	//Address shift
	{0xFF, 1, {0x01}},
	//gamma DC

	{0x00, 1, {0xb4}},	//Address shift
	{0xc0, 1, {0x50}},

	{0x00, 1, {0x82}},	//Address shift
	{0xc5, 1, {0xa3}},

	{0x00, 1, {0x90}},	//Address shift
	{0xC5, 2, {0x96, 0x76}},

	{0x00, 1, {0x00}},	//Address shift
	{0xD8, 2, {0x75, 0x73}},

	{0x00, 1, {0x00}},	//Address shift
	{0xd9, 1, {0x40}},

	{0x00, 1, {0x81}},	//Address shift
	{0xc1, 1, {0x66}},

	{0x00, 1, {0xa1}},	//Address shift
	{0xc1, 1, {0x08}},

	{0x00, 1, {0x89}},	//Address shift
	{0xc4, 1, {0x08}},

	{0x00, 1, {0xa2}},	//Address shift
	{0xc0, 3, {0x1b, 0x00, 0x02}},
	//-----------------------------------------------

	{0x00, 1, {0x81}},	//Address shift
	{0xC4, 1, {0x83}},

	{0x00, 1, {0x92}},	//Address shift
	{0xC5, 1, {0x01}},	//(01)

	{0x00, 1, {0xb1}},	//Address shift
	{0xC5, 1, {0xa9}},	//(01)
	//--------------------854x480---------------------------------

	{0x00, 1, {0x92}},	//Address shift
	{0xb3, 1, {0x45}},

	{0x00, 1, {0x90}},	//Address shift
	{0xb3, 1, {0x02}},

	{0x00, 1, {0x80}},	//Address shift
	{0xc0, 5, {0x00, 0x58, 0x00, 0x14, 0x16}},

	{0x00, 1, {0x80}},	//Address shift
	{0xC4, 1, {0x30}},	//(01)
	//--------------------854x480---------------------------------
	//--------------------------------------------------------------------------------
	//              initial setting 2 < tcon_goa_wave >  480x854
	//--------------------------------------------------------------------------------
	// C09x : mck_shift1/mck_shift2/mck_shift3

	{0x00, 1, {0x90}},	//Address shift
	{0xC0, 6, {0x00, 0x44, 0x00, 0x00,
		   0x00, 0x03}},

	// C1Ax : hs_shift/vs_shift
	{0x00, 1, {0xa6}},	//Address shift
	{0xC1, 3, {0x00, 0x00, 0x00}},

	//--------------------------------------------------------------------------------
	//              initial setting 2 < tcon_goa_wave >
	//--------------------------------------------------------------------------------
	// CE8x : vst1, vst2, vst3, vst4
	{0x00, 1, {0x80}},	//Address shift
	{0xCE, 12, {0x87, 0x03, 0x00, 0x85,
		    0x03, 0x00, 0x86, 0x03,
		    0x00, 0x84, 0x03, 0x00}},

	//CEAx : clka1, clka2
	{0x00, 1, {0xa0}},	//Address shift
	{0xCE, 14, {0x38, 0x03, 0x03, 0x58,
		    0x00, 0x00, 0x00, 0x38,
		    0x02, 0x03, 0x59, 0x00,
		    0x00, 0x00}},

	//CEBx : clka3, clka4
	{0x00, 1, {0xb0}},	//Address shift
	{0xCE, 14, {0x38, 0x01, 0x03, 0x5a,
		    0x00, 0x00, 0x00, 0x38,
		    0x00, 0x03, 0x5b, 0x00,
		    0x00, 0x00}},

	//CECx : clkb1, clkb2
	{0x00, 1, {0xc0}},	//Address shift
	{0xCE, 14, {0x30, 0x00, 0x03, 0x5c,
		    0x00, 0x00, 0x00, 0x30,
		    0x01, 0x03, 0x5d, 0x00,
		    0x00, 0x00}},

	//CEDx : clkb3, clkb4
	{0x00, 1, {0xd0}},	//Address shift
	{0xCE, 14, {0x30, 0x02, 0x03, 0x5e,
		    0x00, 0x00, 0x00, 0x30,
		    0x03, 0x03, 0x5f, 0x00,
		    0x00, 0x00}},

	//CFCx :
	{0x00, 1, {0xc7}},	//Address shift
	{0xCf, 1, {0x00}},

	{0x00, 1, {0xc9}},	//Address shift
	{0xCf, 1, {0x00}},

	//--------------------------------------------------------------------------------
	//              initial setting 3 < Panel setting >
	//--------------------------------------------------------------------------------
	// cbcx
	{0x00, 1, {0xc4}},	//Address shift
	{0xCB, 6, {0x04, 0x04, 0x04, 0x04,
		   0x04, 0x04}},

	// cbdx
	{0x00, 1, {0xd9}},	//Address shift
	{0xCB, 6, {0x04, 0x04, 0x04, 0x04,
		   0x04, 0x04}},

	// cc8x
	{0x00, 1, {0x84}},	//Address shift
	{0xCC, 6, {0x0c, 0x0a, 0x10, 0x0e,
		   0x03, 0x04}},

	// cc9x
	{0x00, 1, {0x9e}},	//Address shift
	{0xcc, 1, {0x0b}},

	// ccax
	{0x00, 1, {0xa0}},	//Address shift
	{0xCC, 5, {0x09, 0x0f, 0x0d, 0x01,
		   0x02}},

	// ccbx
	{0x00, 1, {0xb4}},	//Address shift
	{0xCC, 6, {0x0d, 0x0f, 0x09, 0x0b,
		   0x02, 0x01}},

	// cccx
	{0x00, 1, {0xce}},	//Address shift
	{0xCC, 1, {0x0e}},

	// ccdx
	{0x00, 1, {0xd0}},	//Address shift
	{0xCC, 5, {0x10, 0x0a, 0x0c, 0x04,
		   0x03}},

	///============================================================================
	///============================================================================
	{0x00, 1, {0x00}},	//Address shift
	{0x3a, 1, {0x77}},
	{0x35,1,{0x00}},

	{0x00, 1, {0x00}},	//Address shift
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{0x00, 1, {0x00}},	//Address shift
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}

};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},



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

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	// Highly depends on LCD driver capability.
	// Video mode setting
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	
	params->dsi.vertical_sync_active = 8;
	params->dsi.vertical_backporch = 18;
	params->dsi.vertical_frontporch = 18;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch = 37;
	params->dsi.horizontal_frontporch = 37;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	// Bit rate calculation
	//1 Every lane speed
	params->dsi.pll_div1=0; 	// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=1; 	// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div =13;	// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}


static void lcm_init(void)
{
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

	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) /
		   sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting,
		   sizeof(lcm_deep_sleep_mode_in_setting) /
		   sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(0);
}


static void lcm_resume(void)
{
	lcm_init();
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
	data_array[3] = 0x00053902;
	data_array[4] =
	    (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[5] = (y1_LSB);
	data_array[6] = 0x002c3909;
	dsi_set_cmdq(&data_array, 7, 0);
}


static unsigned int lcm_compare_id()
{
	unsigned int id = 0;
	unsigned char buffer[6], buffer1[2];
	unsigned int array[16];

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
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(100);

	array[0] = 0x00043902;
	array[1] = 0x010980ff;
	array[2] = 0x80001500;
	array[3] = 0x00033902;
	array[4] = 0x010980ff;
	dsi_set_cmdq(array, 5, 1);
	MDELAY(10);
	array[0] = 0x00053700;	// set return byte number
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xA1, &buffer, 5);
	array[0] = 0x00013700;	// set return byte number
	dsi_set_cmdq(array, 1, 1);
	array[0] = 0x50001500;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(1);
	read_reg_v2(0xF8, &buffer1, 1);
	id = (buffer[2] << 8) | buffer[3] | buffer1[0];
#if defined(BUILD_LK)
	printf
	    ("%s, buffer[0]=0x%x,buffer[1]=0x%x,buffer[2]=0x%x,buffer[3]=0x%x buffer[4]=0x%x buffer1[0]=0x%x id = 0x%x\n",
	     __func__, buffer[0], buffer[1], buffer[2], buffer[3],
	     buffer[4], buffer1[0], id);
#endif
	return (LCM_OTM8009A_HSD45_JQ_ID == id) ? 1 : 0;
}

LCM_DRIVER otm8009a_dsi_lcm_drv = {
	.name = "otm8009a_hsd53_jingtai",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
#endif
	.compare_id = lcm_compare_id,
};
