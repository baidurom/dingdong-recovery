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

#include <linux/string.h>
#ifdef BUILD_UBOOT
#include <asm/arch/mt6575_gpio.h>
#else
#include <mach/mt6575_gpio.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  			(540)
#define FRAME_HEIGHT 			(960)
#define LCM_OTM9608A_AUO45_YKL_ID       (0x9608)

#define REGFLAG_DELAY             	(0XFE)
#define REGFLAG_END_OF_TABLE      	(0x100)	// END OF REGISTERS MARKER

#if defined(BUILD_UBOOT)
#define LCM_DEBUG(fmt,arg...)  printf("[Eric][uboot]""[%s]"fmt"\n",__func__,##arg)
#else
#define LCM_DEBUG(fmt,arg...)  printk("[Eric][kernel]""[%s]"fmt"\n",__func__,##arg)
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    		(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 			(lcm_util.udelay(n))
#define MDELAY(n) 			(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	(lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update)		(lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd)						(lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums)			(lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg						(lcm_util.dsi_read_reg())
#define read_reg_v2(cmd, buffer, buffer_size)                   (lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

static struct LCM_setting_table {
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
	{0xFF, 3, {0x96, 0x08, 0x01}},

	{0x00, 1, {0x80}},
	{0xFF, 2, {0x96, 0x08}},

	{0x00, 1, {0x00}},
	{0xA0, 1, {0x00}},

	{0x00, 1, {0x80}},
	{0xB3, 5, {0x00, 0x00, 0x20, 0x00,
		   0x00}},

	{0x00, 1, {0xC0}},
	{0xB3, 1, {0x09}},

	{0x00, 1, {0x80}},
	{0xC0, 9, {0x00, 0x48, 0x00, 0x10,
		   0x10, 0x00, 0x48, 0x10,
		   0x10}},

	{0x00, 1, {0x92}},
	{0xC0, 4, {0x00, 0x0B, 0x00, 0x13}},

	{0x00, 1, {0xA2}},
	{0xC0, 3, {0x0C, 0x05, 0x02}},

	{0x00, 1, {0xB3}},
	{0xC0, 2, {0x00, 0x10}},

	{0x00, 1, {0xB5}},
	{0xC0, 1, {0x48}},

	{0x00, 1, {0x81}},
	{0xC1, 1, {0x66}},	//44

	{0x00, 1, {0x80}},
	{0xC4, 2, {0x00, 0x84}},

	{0x00, 1, {0x88}},
	{0xC4, 1, {0x40}},	//C0

	{0x00, 1, {0xA0}},
	{0xC4, 8, {0x33, 0x09, 0x90, 0x2B,
		   0x33, 0x09, 0x90, 0x54}},

	{0x00, 1, {0x90}},
	{0xC5, 7, {0x96, 0x79, 0x01, 0x79,
		   0x33, 0x33, 0x34}},

	{0x00, 1, {0xA0}},
	{0xC5, 7, {0x96, 0x79, 0x00, 0x79,
		   0x33, 0x33, 0x34}},

	{0x00, 1, {0x00}},
	{0xD8, 2, {0x67, 0x67}},

	{0x00, 1, {0x00}},
	{0xD9, 1, {0x68}},

	{0x00, 1, {0x00}},
	{0xD0, 1, {0x40}},

	{0x00, 1, {0x00}},
	{0xD1, 2, {0x00, 0x00}},

	{0x00, 1, {0xA6}},
	{0xB3, 1, {0x27}},

	{0x00, 1, {0xA0}},
	{0xB3, 1, {0x10}},

	{0x00, 1, {0xF0}},
	{0xCB, 10, {0xFF, 0xFF, 0xFF, 0xFF,
		    0xFF, 0xFF, 0xFF, 0xFF,
		    0xFF, 0xFF}},

	{0x00, 1, {0x80}},
	{0xCE, 12, {0x82, 0x01, 0x0E, 0x81,
		    0x01, 0x0E, 0x00, 0x0F,
		    0x00, 0x00, 0x0F, 0x00}},

	{0x00, 1, {0x90}},
	{0xCE, 14, {0x13, 0xBF, 0x0E, 0x13,
		    0xC0, 0x0E, 0xF0, 0x00,
		    0x00, 0xF0, 0x00, 0x00,
		    0x00, 0x00}},

	{0x00, 1, {0xA0}},
	{0xCE, 14, {0x18, 0x00, 0x03, 0xBD,
		    0x00, 0x0E, 0x00, 0x10,
		    0x00, 0x03, 0xBE, 0x00,
		    0x0E, 0x00}},

	{0x00, 1, {0xB0}},
	{0xCE, 14, {0x10, 0x01, 0x03, 0xBF,
		    0x00, 0x0E, 0x00, 0x10,
		    0x02, 0x03, 0xC0, 0x00,
		    0x0E, 0x00}},

	{0x00, 1, {0xC0}},
	{0xCE, 14, {0x38, 0x02, 0x03, 0xC1,
		    0x00, 0x09, 0x05, 0x38,
		    0x01, 0x03, 0xC2, 0x00,
		    0x09, 0x05}},

	{0x00, 1, {0xD0}},
	{0xCE, 14, {0x30, 0x01, 0x03, 0xBD,
		    0x00, 0x09, 0x05, 0x30,
		    0x02, 0x03, 0xBE, 0x00,
		    0x09, 0x05}},

	{0x00, 1, {0xC0}},
	{0xCF, 10, {0x01, 0x01, 0x00, 0x00,
		    0x00, 0x00, 0x01, 0x02,
		    0x00, 0x00}},

	{0x00, 1, {0x00}},
	{0xE1, 16, {0x05, 0x0B, 0x10, 0x0C,
		    0x05, 0x0D, 0x0B, 0x0A,
		    0x04, 0x07, 0x0D, 0x07,
		    0x0E, 0x13, 0x0E, 0x00}},

	{0x00, 1, {0x00}},
	{0xE2, 16, {0x05, 0x0B, 0x10, 0x0C,
		    0x05, 0x0D, 0x0B, 0x0A,
		    0x04, 0x07, 0x0D, 0x07,
		    0x0E, 0x13, 0x0E, 0x00}},

	{0x00, 1, {0x00}},
	{0xFF, 3, {0xFF, 0xFF, 0xFF}},
	{REGFLAG_DELAY, 20, {}},
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},

	// Display ON
	{0x2C, 1, {0x00}},
	{0x13, 1, {0x00}},
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
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
			break;
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

	// Highly depends on LCD driver capability.
	params->dsi.packet_size = 256;

	// Video mode setting
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.word_count = FRAME_WIDTH * 3;
	params->dsi.vertical_sync_active = 1;
	params->dsi.vertical_backporch = 1;
	params->dsi.vertical_frontporch = 1;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.line_byte = 2180;	// 2256 = 752*3
	params->dsi.horizontal_sync_active_byte = 26;
	params->dsi.horizontal_backporch_byte = 12;
	params->dsi.horizontal_frontporch_byte = 12;
	params->dsi.rgb_byte = (FRAME_WIDTH * 3 + 6);

	params->dsi.horizontal_sync_active_word_count = 20;
	params->dsi.horizontal_backporch_word_count = 200;
	params->dsi.horizontal_frontporch_word_count = 200;

	// Bit rate calculation
	params->dsi.pll_div1 = 38;	// fref=26MHz, fvco=fref*(div1+1)       (div1=0~63, fvco=500MHZ~1GHz)
	params->dsi.pll_div2 = 1;	// div2=0~15: fout=fvo/(2*div2)

}


static void lcm_init(void)
{
	LCM_DEBUG("enter init!");
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(60);

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
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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
	unsigned char buffer[5];
	unsigned int array[16];
	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(50);

	array[0] = 0x00053700;	// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);
	id = ((buffer[2] << 8) | buffer[3]);	//we only need ID
#if defined(BUILD_UBOOT)
	printf
	    ("%s, id1 = 0x%x,id2 = 0x%x,id3 = 0x%x,id4 = 0x%x,id5 = 0x%x\n",
	     __func__, buffer[0], buffer[1], buffer[2], buffer[3],
	     buffer[4]);
#endif
	return (LCM_OTM9608A_AUO45_YKL_ID == id) ? 1 : 0;
}



LCM_DRIVER otm9608a_dsi_lcm_drv = {
	.name = "otm9608a_auo45_ykl",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.update = lcm_update,
	.compare_id = lcm_compare_id,
};
