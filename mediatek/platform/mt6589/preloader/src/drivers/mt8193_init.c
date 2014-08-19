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

#include "typedefs.h"
#include "platform.h"
#include "uart.h"
#include "meta.h"
#include "mt8193.h"
#include "mt_i2c.h"


int mt8193_pllgp_en()
{
    printf("mt8193_pllgp_en() enter\n");

    /* PLL1 setup of 75MHZ. AD_PLLGP_CLK=450MHZ*/

    CKGEN_WRITE32(REG_RW_PLL_GPANACFG0, 0xAD300982);

    CKGEN_WRITE32(REG_RW_PLL_GPANACFG3, 0x80008000);

    CKGEN_WRITE32(REG_RW_PLL_GPANACFG2, 0x2500000);

    printf("mt8193_pllgp_en() exit\n");

    return 0;
}

int mt8193_vopll_en()
{
    printf("mt8193_pllgp_en() enter\n");

    /* PLL2 setup of 75MHZ. AD_PLLGP_CLK=450MHZ*/

    IO_WRITE32(0, 0x44c, 0x1);

    // CKGEN_WRITE32(REG_RW_PLL_GPANACFG0, 0xAD300982);

    CKGEN_WRITE32(REG_RW_LVDS_ANACFG2, 0x32215000);

    CKGEN_WRITE32(REG_RW_LVDS_ANACFG3, 0x410c0);

    CKGEN_WRITE32(REG_RW_LVDS_ANACFG4, 0x300);

    printf("mt8193_pllgp_en() exit\n");

    return 0;
}

int mt8193_i2c_init(void)
{   
    u32 ret_code;
    
    printf("mt8193_i2c_init() enter\n");    

    /* Sset I2C speed mode */
    ret_code = mt_i2c_set_speed(I2C2, I2C_CLK_RATE, ST_MODE, MAX_ST_MODE_SPEED);
    if( ret_code !=  I2C_OK)
    {
        printf("[mt8193_i2c_init] mt_i2c_set_speed error (%d)\n", ret_code);
        return ret_code;
    }

    printf("mt8193_i2c_init() exit\n"); 

    return (0);
}

#define NFI_MLC 0
#if NFI_MLC

#define RG_NFI_PWR_RST_B (0x110)
#define RG_NFI_PWR_CTRL  (0x114)

void close_8193_NFI(void)
{
	/*
	1.	Power off digital:
		RG_NFI_PWR_CTRL[2] = 1       // disable clock
		RG_NFI_PWR_CTRL[0] = 1       // en iso   
		RG_NFI_PWR_RST_B[0] = 0     // en reset    
		RG_NFI_PWR_CTRL[1] = 0       // turn off power

	2.	Power off analog
	      ADDR:0x134C BIT[1] =0
	      ADDR:0x1354  BIT[20] =0
	*/
	u32 value = 0;
	printf("Close 8193 NFI Digital\n");

	//power off digital
	value = IO_READ32(0,RG_NFI_PWR_CTRL);
	IO_WRITE32(0,RG_NFI_PWR_CTRL,(value | 0x4));
	//IO_WRITE32(0,RG_NFI_PWR_CTRL,(value | 0x5));
	value = IO_READ32(0,RG_NFI_PWR_CTRL);
	IO_WRITE32(0,RG_NFI_PWR_CTRL,(value | 0x1));
	value = IO_READ32(0,RG_NFI_PWR_RST_B);
	IO_WRITE32(0,RG_NFI_PWR_RST_B,(value & 0xFFFFFFFE));
	value = IO_READ32(0,RG_NFI_PWR_CTRL);
	IO_WRITE32(0,RG_NFI_PWR_CTRL,(value & 0xFFFFFFFD));

	printf("Close 8193 NFI Analog\n");
	//power off analog
	value = IO_READ32(0,0x134C);
	IO_WRITE32(0,0x134C,(value & 0xFFFFFFFD));

	//can not close power share power
	//value = IO_READ32(0,0x1354);
	//IO_WRITE32(0,0x1354,(value & 0xFFEFFFFF));
	
	printf("Close 8193 NFI OK\n");
}
#endif

void set_NFI_PIN_GPIO(void)
{
	u32 value = 0;
	printf("set_CKGEN_NFI_PIN_GPIO\n");

#if 1
	//
	value = CKGEN_READ32(0x204);
	value &= 0xFFE00000;
	value |= 0x49249;
	CKGEN_WRITE32(0x204,value);

	value = CKGEN_READ32(0x21C);
	value &= 0xC7FFFFFF;
	value |= 0x08000000;
	CKGEN_WRITE32(0x21C,value);

	value = CKGEN_READ32(0x220);
	value &= 0xFFFC0000;
	value |= 0x1249B;
	CKGEN_WRITE32(0x220,value);

	value = CKGEN_READ32(0x258);
	value |= 0x7F;
	CKGEN_WRITE32(0x258,value);

	value = CKGEN_READ32(0x260);
	value |= 0xFF;
	CKGEN_WRITE32(0x260,value);
#else 

  CKGEN_WRITE32(0x204,0x00049249);

  CKGEN_WRITE32(0x21c,0x08018493);
  CKGEN_WRITE32(0x220,0x0001249b);

 
  CKGEN_WRITE32(0x258,0xffe7ffff);
  CKGEN_WRITE32(0x260,0xffffffff);
 
  CKGEN_WRITE32(0x254,0x00000082);
  CKGEN_WRITE32(0x204,0x00049249);
  CKGEN_WRITE32(0x208,0x12000000);
  CKGEN_WRITE32(0x20c,0x1b6d3012);
  CKGEN_WRITE32(0x210,0x124926db);
  
  CKGEN_WRITE32(0x218,0x1b692490);
  CKGEN_WRITE32(0x21c,0x08018493);
  CKGEN_WRITE32(0x220,0x0001249b);
  CKGEN_WRITE32(0x254,0x00000182);
  CKGEN_WRITE32(0x10c,0x00000007);
  
  CKGEN_WRITE32(0x108,0x00000000);
  CKGEN_WRITE32(0x10c,0x00000005);
  CKGEN_WRITE32(0x104,0x00000005);
  CKGEN_WRITE32(0x114,0x00000007);
  CKGEN_WRITE32(0x110,0x00000000);
  
  CKGEN_WRITE32(0x114,0x00000005);
  CKGEN_WRITE32(0x334,0x00000094);
  CKGEN_WRITE32(0x33c,0x2d300880);
  
  CKGEN_WRITE32(0x354,0x00400000);
  CKGEN_WRITE32(0x35c,0xad300880);
  CKGEN_WRITE32(0x258,0xffe7ffff);
  CKGEN_WRITE32(0x25c,0xffffffff);
  CKGEN_WRITE32(0x260,0xffffffff);
#endif

	printf("set_CKGEN_NFI_PIN_GPIO\n");
}


int mt8193_init(void)
{
    printf("mt8193_init() enter\n");

	u32 u4Tmp = 0;

    mt8193_i2c_init();

	
	u4Tmp = CKGEN_READ32(REG_RW_LVDSWRAP_CTRL1);
	u4Tmp |= (CKGEN_LVDSWRAP_CTRL1_NFIPLL_MON_EN | CKGEN_LVDSWRAP_CTRL1_DCXO_POR_MON_EN);
	CKGEN_WRITE32(REG_RW_LVDSWRAP_CTRL1, u4Tmp);
	  
	/* close pad_int trapping function*/
	u4Tmp = 0x0;
	CKGEN_WRITE32(REG_RW_PMUX7, u4Tmp);


#if 0
    /*  dcxo enable */
	u4Tmp = CKGEN_READ32(REG_RW_CKMISC_CTRL);
	u4Tmp &= (~CKGEN_CKMISC_CTRL_DCXO_MODE_EN);
	CKGEN_WRITE32(REG_RW_CKMISC_CTRL, u4Tmp);
#endif
	  
    mt8193_pllgp_en();

    mt8193_vopll_en();

	#if NFI_MLC
	close_8193_NFI();
	#endif

	set_NFI_PIN_GPIO();
      
	printf("mt8193_init() exit\n");
	  

    return (0);
}

