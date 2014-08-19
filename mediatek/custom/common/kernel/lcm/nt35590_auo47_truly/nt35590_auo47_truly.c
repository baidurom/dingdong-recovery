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

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_NT35590 (0x90)
#define GPIO_LCD_RST_EN      GPIO131

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

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

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#if defined(MTK_WFD_SUPPORT)
#define   LCM_DSI_CMD_MODE							1
#else
#define   LCM_DSI_CMD_MODE							1
#endif


static void init_lcm_registers(void)
{
	unsigned int data_array[16];


        data_array[0] = 0x00023902;//CMD1
        data_array[1] = 0x000000FF;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;//03 Video 08 command
#if (LCM_DSI_CMD_MODE)
        data_array[1] = 0x000008C2;
#else
        data_array[1] = 0x000003C2;
#endif
        dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0] = 0x00023902;//
        data_array[1] = 0x000003ba;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;//CMD2,Page0
        data_array[1] = 0x000001FF;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;//720*1280
        data_array[1] = 0x00004A00;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00004401; //4401
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00005402; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00005503; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00005504; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00003305; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00002206; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00005608; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00008f09; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000b30b; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000b30c; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00002f0d; //5402
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;//VGH=+8.6V
        data_array[1] = 0x0000330E;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00008611; //8611
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;//VCOMDC
        data_array[1] = 0x00000312;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;//VCOMDC
        data_array[1] = 0x00000a0f;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x000005FF;
        dsi_set_cmdq(data_array, 2, 1);
        
        data_array[0] = 0x00023902;
        data_array[1] = 0x000001FB;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902; /////////////LTPS
        data_array[1] = 0x00000001;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00008202;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00008203;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00008204;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00003005;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;//06
        data_array[1] = 0x00003306;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000107;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000008;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00003009;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000300A;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902; //0D
        data_array[1] = 0x0000110D;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00001e0E;

        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000080F;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00005310;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000011;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;//12
        data_array[1] = 0x00000012;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000114;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000015;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000516;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000417;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00007F19;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000ff1A;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000F1B;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000001C;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000001D; //101D
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000001E; //011E
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000071F;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000020;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000021;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00005522; //5522
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000D23;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000022d;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x00000183;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000589e;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;
        data_array[1] = 0x0000329f;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x000001a0;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x000010a2;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000abb;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000abc;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000128;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000022f;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000832;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000b833;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000136;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000037;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000043;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000214b;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000034c;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00002150;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000351;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00002158;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x00000359;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000215d;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;
        data_array[1] = 0x0000429f;
        dsi_set_cmdq(data_array, 2, 1);

        data_array[0] = 0x00023902;////CMD1
        data_array[1] = 0x000000FF;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;////CMD1
        data_array[1] = 0x000002ba;
        dsi_set_cmdq(data_array, 2, 1);
        data_array[0] = 0x00023902;////CMD1
        data_array[1] = 0x00000035;
        dsi_set_cmdq(data_array, 2, 1);

        /*******debug-----start********/
        data_array[0] = 0x00110500;
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(120);
        data_array[0] = 0x00290500;
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(10);
        data_array[0] = 0x002c0500;
        dsi_set_cmdq(data_array, 1, 1);
        MDELAY(10);
	/*******debug-----end********/

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
                params->dbi.te_mode                             = LCM_DBI_TE_MODE_DISABLED;
                params->dbi.te_edge_polarity            = LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
        #endif

		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 2;// 3    2
		params->dsi.vertical_backporch					= 20;// 20   1
		params->dsi.vertical_frontporch					= 6; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 6;// 50  2
		params->dsi.horizontal_backporch				= 12;
		params->dsi.horizontal_frontporch				= 80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8;

		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =45;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

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
           MDELAY(5);
           mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
           MDELAY(1);
           mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
           MDELAY(20);
           //init_lcm_registers();

           data_array[0] = 0x00023902;
           data_array[1] = 0x0000EEFF; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(2); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000826; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(2); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000026; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(2); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000FF; 				
           dsi_set_cmdq(data_array, 2, 1);
           mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
           MDELAY(10);
           mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
           MDELAY(80);
           
           data_array[0] = 0x00023902;//CMD1							
           data_array[1] = 0x000000FF; 				
           dsi_set_cmdq(data_array, 2, 1); 	

           data_array[0] = 0x00023902;//03 4lane  02 3lanes			   
           data_array[1] = 0x000003BA; 				
           dsi_set_cmdq(data_array, 2, 1);    

           data_array[0] = 0x00023902;//03 Video 08 command
#if (LCM_DSI_CMD_MODE)
           data_array[1] = 0x000008C2; 
#else
           data_array[1] = 0x000003C2; 
#endif                
           dsi_set_cmdq(data_array, 2, 1);   
			
           data_array[0] = 0x00023902;//CMD2,Page0  
           data_array[1] = 0x000001FF; 				
           dsi_set_cmdq(data_array, 2, 1);   

           ////////////////////////
           ///////////////////////
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00004A00; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003301; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005302; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005503; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005504; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003305; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002206; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005608; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008F09; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007336; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009F0B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009F0C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002F0D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000240E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008311; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000312; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002C71; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000036F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000A0F; 				
           dsi_set_cmdq(data_array, 2, 1);		
           data_array[0] = 0x00023902;
           data_array[1] = 0x000005FF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000001; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008202; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008203; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008204; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003005; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003306; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000107; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000008; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00004609; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000460A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000B0D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001D0E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000080F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005310; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000011; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000012; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000114; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000015; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000516; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000017; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007F19; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000FF1A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000F1B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000001C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000001D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000001E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000071F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000020; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000021; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005522; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00004D23; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000022D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000128; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000022F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000183; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000589E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006A9F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001A0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000010A2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000ABB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000ABC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000832; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B833; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000136; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000037; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000043; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000214B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000034C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002150; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000351; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002158; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000359; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000215D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000035E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000006C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000006D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003301; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005302; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000075; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007D76; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000077; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008A78; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000079; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009C7A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000007B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B17C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000007D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000BF7E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000007F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CF80; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000081; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000DD82; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000083; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E884; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000085; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000F286; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000187; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001F88; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000189; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000418A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000018B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000788C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000018D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A58E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000018F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000EE90; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000291; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002992; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000293; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002A94; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000295; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005D96; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000297; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009398; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000299; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B89A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000029B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E79C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000039D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000079E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000039F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000037A0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000046A3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000056A5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000066A7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007AAA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003AB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000093AC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003AD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A3AE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003AF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B4B0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003B1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CBB2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007DB4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00008AB6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009CB8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B1BA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000BB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000BFBC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000BD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CFBE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000BF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000DDC0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000C1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E8C2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000C3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000F2C4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001FC6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000041C8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000078CA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001CB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A5CC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001CD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000EECE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002CF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000029D0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002AD2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005DD4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000093D6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B8D8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E7DA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003DB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000007DC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003DD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000037DE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003DF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000046E0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000056E2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000066E4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007AE6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000093E8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A3EA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003EB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B4EC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003ED; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CBEE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000EF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000EDF0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000F1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000F3F2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000F3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000FEF4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001F5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000009F6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001F7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000013F8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001F9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001DFA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002FF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000100; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002601; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000102; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002F03; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000104; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003705; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000106; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005607; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000108; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007009; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000010A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009D0B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000010C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000C20D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000010E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000FF0F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000210; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003111; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000212; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003213; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000214; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006015; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000216; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009417; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000218; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B519; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000021A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E31B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000031C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000031D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000031E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002D1F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000320; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003A21; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000322; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00004823; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000324; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005725; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000326; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006827; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000328; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007B29; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000032A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000902B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000032D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A02F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000330; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CB31; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000032; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000ED33; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000034; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000F335; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000036; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000FE37; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000138; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000939; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000013A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000133B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000013D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001D3F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000140; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002641; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000142; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002F43; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000144; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003745; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000146; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005647; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000148; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007049; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000014A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009D4B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000014C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000C24D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000014E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000FF4F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000250; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003151; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000252; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003253; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000254; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006055; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000256; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009458; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000259; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B55A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000025B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E35C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000035D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000035E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000035F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002D60; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000361; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003A62; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000363; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00004864; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000365; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005766; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000367; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006868; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000369; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007B6A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000036B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000906C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000036D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A06E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000036F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CB70; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000071; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001972; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000073; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003674; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000075; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005576; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000077; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00007078; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000079; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000837A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000007B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000997C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000007D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A87E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000007F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B780; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000081; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000C582; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000083; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000F784; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000185; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001E86; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000187; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006088; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000189; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000958A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000018B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E18C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000028D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000208E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000028F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00002390; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000291; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005992; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000293; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00009494; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000295; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B496; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000297; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E198; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000399; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000019A; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000039B; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000289C; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000039D; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000309E; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000039F; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000037A0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003BA3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000040A5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000050A7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003A9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006DAA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003AB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000080AC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003AD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CBAE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000AF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000019B0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000036B2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000055B4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000070B6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000083B8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000B9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000099BA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000BB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000A8BC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000BD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B7BE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000BF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000C5C0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000C1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000F7C2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00001EC4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000060C6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000095C8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001C9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E1CA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002CB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000020CC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002CD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000023CE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002CF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000059D0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000094D2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000B4D4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000002D5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000E1D6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003D7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001D8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003D9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000028DA; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003DB; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000030DC; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003DD; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000037DE; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003DF; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00003BE0; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E1; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000040E2; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E3; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000050E4; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E5; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x00006DE6; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E7; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000080E8; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x000003E9; 				
           dsi_set_cmdq(data_array, 2, 1);	
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000CBEA; 				
           dsi_set_cmdq(data_array, 2, 1);		

           ///////////////////////////
           data_array[0] = 0x00023902;////CMD2 page0
           data_array[1] = 0x000001FF; 				
           dsi_set_cmdq(data_array, 2, 1);    
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);    
           data_array[0] = 0x00023902;//CMD2,Page1
           data_array[1] = 0x000002FF; 				
           dsi_set_cmdq(data_array, 2, 1); 	  
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1);   
           data_array[0] = 0x00023902;//CMD2,Page1 
           data_array[1] = 0x000004FF; 				
           dsi_set_cmdq(data_array, 2, 1); 		
           data_array[0] = 0x00023902;
           data_array[1] = 0x000001FB; 				
           dsi_set_cmdq(data_array, 2, 1); 	  
           data_array[0] = 0x00023902;//CMD select 
           data_array[1] = 0x000000FF; 				
           dsi_set_cmdq(data_array, 2, 1); 	

           data_array[0] = 0x00110500; 			   
           dsi_set_cmdq(data_array, 1, 1); 
           MDELAY(120); 

           data_array[0] = 0x00023902;
           data_array[1] = 0x0000EEFF; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(1); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x00005012; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(1); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x00000213; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(1); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x0000606A; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(1); 
           data_array[0] = 0x00023902;
           data_array[1] = 0x000000FF; 				
           dsi_set_cmdq(data_array, 2, 1);
           MDELAY(1); 
           data_array[0] = 0x00023902; 	
           data_array[1] = 0x00000035; 		
           dsi_set_cmdq(data_array, 2, 1); 

           data_array[0] = 0x00290500; 			   
           dsi_set_cmdq(data_array, 1, 1); 
           MDELAY(5);
           data_array[0] = 0x002c0500;
           dsi_set_cmdq(data_array, 1, 1);
           MDELAY(1);
}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
        MDELAY(10);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
        MDELAY(80);
#ifdef BUILD_LK
        upmu_set_rg_vgp6_en(0);
#else
        hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif

}


static void lcm_resume(void)
{

	lcm_init();

    #ifdef BUILD_LK
	  printf("[LK]---cmd---nt35590----%s------\n",__func__);
    #else
	  printk("[KERNEL]---cmd---nt35590----%s------\n",__func__);
    #endif
}

#if (LCM_DSI_CMD_MODE)
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
#if 1
	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
#endif
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];
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


	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
    #ifdef BUILD_LK
		printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35590)
    	return 1;
    else
        return 0;


}


static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x36, buffer, 1);
	if(buffer[0]==0x90)
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
	lcm_init();
	lcm_resume();

	return TRUE;
}



LCM_DRIVER nt35590_auo47_truly_lcm_drv =
{
    .name			= "nt35590_auo47_truly",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
//	.esd_check = lcm_esd_check,
//	.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
