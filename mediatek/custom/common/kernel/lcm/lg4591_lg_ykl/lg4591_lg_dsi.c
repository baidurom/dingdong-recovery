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

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID_LG4591                                      0x4591
#define GPIO_LCD_RST_EN      GPIO131

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
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)



static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};

#if 0
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

	//set Password
	{0xB9,	3,	{0xFF,0x83,0x94}},
	{REGFLAG_DELAY, 10, {}},

	//set MIPI 4 Lane
	{0xBA, 17, {0x13,0x82,0x00,0x16,0xC6,0x00,0x08,0xFF,0x0F,0x24,0x03,0x21,0x24,0x25,0x20,0x00,0x00}},
	{REGFLAG_DELAY, 10, {}},

	//set Power
	{0xB1, 15, {0x7C,0x00,0x24,0x09,0x01,0x11,0x11,0x36,0x3E,0x26,0x26,0x57,0x0A,0x01,0xE6}},
	{REGFLAG_DELAY, 10, {}},

	//set CYC
	{0xB4, 18, {0x00,0x00,0x00,0x05,0x06,0x41,0x42,0x02,0x41,0x42,0x43,0x47,0x19,0x58,0x60,0x08,0x85,0x10}},
	{REGFLAG_DELAY, 10, {}},

	//set GIP
	{0xD5, 24, {0x4C,0x01,0x07,0x01,0xCD,0x23,0xEF,0x45,0x67,0x89,0xAB,0x11,0x00,0xDC,0x10,0xFE,0x32,0xBA,0x98,0x76,0x54,0x00,0x11,0x40}},
	{REGFLAG_DELAY, 10, {}},

	//set Display related register
	{0xB2, 6, {0x0F,0xC8,0x04,0x04,0x00,0x81}},
	{REGFLAG_DELAY, 10, {}},

	//set Vcom
	{0xB6, 1, {0x25}},
	{REGFLAG_DELAY, 10, {}},

	//set TCOM Option
	{0xC7, 2, {0x00,0x20}},
	{REGFLAG_DELAY, 10, {}},

	//set panel
	{0xCC, 1, {0x09}},
	{REGFLAG_DELAY, 10, {}},

	{0x11,	0,	{}},     //sleep out
	{REGFLAG_DELAY, 200, {}},

	//display on
	{0x29, 0, {}},
	{REGFLAG_DELAY, 50, {}},

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

        data_array[0] = 0x00062902;                          
        data_array[1] = 0x800043E0;
        data_array[2] = 0x00000000;
        dsi_set_cmdq(data_array, 3, 1);
        //MDELAY(1);

        data_array[0] = 0x00062902;                          
        data_array[1] = 0x40201cB5;
        data_array[2] = 0x00000000;
        //data_array[3] = 0x03240FFF;
        //data_array[4] = 0x20252421;
        //data_array[5] = 0x00000000;
        dsi_set_cmdq(data_array, 3, 1);
        //MDELAY(1);


        data_array[0] = 0x00062902;                          
        data_array[1] = 0x0F1601B6; 
        data_array[2] = 0x00001316;
        //data_array[3] = 0x26263E36;
        //data_array[4] = 0xE6010A57;
        dsi_set_cmdq(data_array, 3, 1); 
        //MDELAY(1);
#if 1
        data_array[0] = 0x00032902;                          
        data_array[1] = 0x000000C0; 
        //data_array[2] = 0x42410605; 
        //data_array[3] = 0x43424102;
        //data_array[4] = 0x60581947;
        //data_array[5] = 0x00108508;
        dsi_set_cmdq(data_array, 2, 1);
        //MDELAY(1);

        data_array[0] = 0x00C12300;                          
        //data_array[1] = 0x000000C1; 
        //data_array[2] = 0xEF23CD00;
        //data_array[3] = 0xAB896745;
        //data_array[4] = 0x10DC0011;
        //data_array[5] = 0x98BA32FE;
        //data_array[6] = 0x11005476;
        //data_array[7] = 0x00000040;
        dsi_set_cmdq(data_array, 1, 1); 
        //MDELAY(1);

        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x100901C3; 
        data_array[2] = 0x40660012;
        data_array[3] = 0x00000032; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        data_array[0] = 0x00062902;                          
        data_array[1] = 0x182422C4;
        data_array[2] = 0x00005218;
        dsi_set_cmdq(data_array, 3, 1);
        data_array[0] = 0x00032902;                          
        data_array[1] = 0x004042C6;
        dsi_set_cmdq(data_array, 2, 1);
        //MDELAY(1);
#endif
        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x641101D0; 
        data_array[2] = 0x51061835;
        data_array[3] = 0x00000232; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x641420D1; 
        data_array[2] = 0x71050134;
        data_array[3] = 0x00000433; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x641101D2; 
        data_array[2] = 0x51061835;
        data_array[3] = 0x00000232; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x641420D3; 
        data_array[2] = 0x71050134;
        data_array[3] = 0x00000433; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x641101D4; 
        data_array[2] = 0x51061835;
        data_array[3] = 0x00000232; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        data_array[0] = 0x000A2902;                          
        data_array[1] = 0x641420D5; 
        data_array[2] = 0x71050134;
        data_array[3] = 0x00000433; 
        dsi_set_cmdq(data_array, 4, 1);
        //MDELAY(1);

        //data_array[0] = 0x08362300;                          
        //data_array[1] = 0x00000836; 
        //dsi_set_cmdq(data_array, 1, 1);
        //MDELAY(1);

#if 0
        data_array[0] = 0x00F92300;                          
        //data_array[1] = 0x000000F9; 
        dsi_set_cmdq(data_array, 1, 1);
        //MDELAY(1);

        //data_array[0] = 0x00B32300;                          
        //data_array[1] = 0x000002C2; 
        //dsi_set_cmdq(data_array, 1, 1);

        data_array[0] = 0x02C22300;                          
        //data_array[1] = 0x000002C2; 
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(10);

        data_array[0] = 0x06C22300;                          
        //data_array[1] = 0x000006C2; 
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(10);

        data_array[0] = 0x4EC22300;                          
        //data_array[1] = 0x00004EC2; 
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(80);
#endif
        data_array[0] = 0x00110500; // Sleep Out
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(50);

        data_array[0] = 0x00F92300;                          
        //data_array[1] = 0x000000F9; 
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(10);

        data_array[0] = 0x00290500; // Display On
        dsi_set_cmdq(data_array, 1, 1); 
	//MDELAY(10);
}


#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {}},
    {REGFLAG_DELAY, 200, {}},

    // Display ON
	{0x29, 0, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {}},

    // Sleep Mode On
	{0x10, 0, {}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
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

#endif
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
	//	params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	    params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
#endif

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;




		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting
		params->dsi.intermediat_buffer_num = 0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;


		params->dsi.word_count=720*3;

		params->dsi.vertical_sync_active= 2;
		params->dsi.vertical_backporch= 8;
		params->dsi.vertical_frontporch= 10;
		params->dsi.vertical_active_line=FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 10;  //
		params->dsi.horizontal_backporch				= 120; //
		params->dsi.horizontal_frontporch				= 20; //
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		//params->dsi.HS_PRPR=6;
		//params->dsi.LPX=8;
		//params->dsi.HS_PRPR=5;
		//params->dsi.HS_TRAIL=13;


		// Bit rate calculation
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =20;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

static void lcm_init(void)
{
#ifdef BUILD_LK
        upmu_set_rg_vgp6_vosel(6);
        upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif

    mt_set_gpio_mode(GPIO74, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO74, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO74, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(10);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
    MDELAY(10);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(120);

	init_lcm_registers();

	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	unsigned int data_array[16];


	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
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

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(&data_array, 1, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);

}

static unsigned int lcm_compare_id(void)
{
		unsigned int id=0;
		unsigned char buffer[8];
		unsigned int array[16];

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
                MDELAY(100);

		array[0] = 0x00083700;// return byte number
		dsi_set_cmdq(&array, 1, 1);
		MDELAY(10);

		read_reg_v2(0xa1, buffer, 8);
		id = buffer[6]<<8 | buffer[7];

#ifdef BUILD_LK
		printf("=====>for test %s, id = 0x%08x\n", __func__, id);
#else
		printk("=====>fot test %s, id = 0x%08x\n", __func__, id);
#endif

		return (LCM_ID_LG4591 == id)?1:0;

}


LCM_DRIVER lg4591_auo_lcm_drv =
{
    .name			= "lg4591_dsi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
        .update         = lcm_update
#endif
    };
