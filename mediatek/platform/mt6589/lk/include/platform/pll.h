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

#ifndef PLL_H
#define PLL_H

#include <platform/mt_reg_base.h>
#include <platform/mt_typedefs.h>

#define AP_PLL_CON0         (0x10209000)
#define AP_PLL_CON1         (0x10209004)
#define AP_PLL_CON2         (0x10209008)
#define PLL_HP_CON0         (0x10209014)
#define PLL_TEST_CON0       (0x10209054)
#define PLL_TEST_CON1       (0x10209058)
#define ARMPLL_CON0         (0x10209200)
#define ARMPLL_CON1         (0x10209204)
#define ARMPLL_CON2         (0x10209208)
#define ARMPLL_PWR_CON0     (0x10209218)
#define MAINPLL_CON0        (0x1020921C)
#define MAINPLL_CON1        (0x10209220)
#define MAINPLL_CON2        (0x10209224)
#define MAINPLL_PWR_CON0    (0x10209234)
#define UNIVPLL_CON0        (0x10209238)
#define MMPLL_CON0          (0x10209240)
#define ISPPLL_CON0         (0x10209248)
#define MSDCPLL_CON0        (0x10209250)
#define MSDCPLL_CON1        (0x10209254)
#define MSDCPLL_CON2        (0x10209258)
#define MSDCPLL_PWR_CON0    (0x10209268)
#define TVDPLL_CON0         (0x1020926C)
#define TVDPLL_CON1         (0x10209270)
#define TVDPLL_CON2         (0x10209274)
#define TVDPLL_CON3         (0x10209278)
#define TVDPLL_PWR_CON0     (0x10209284)
#define LVDSPLL_CON0        (0x10209288)
#define LVDSPLL_CON1        (0x1020928C)
#define LVDSPLL_CON2        (0x10209290)
#define LVDSPLL_CON3        (0x10209294)
#define LVDSPLL_PWR_CON0    (0x102092A0)
#define AP_AUXADC_CON0      (0x10209400)
#define AP_AUXADC_CON1      (0x10209404)
#define TS_CON0             (0x10209600)
#define TS_CON1             (0x10209604)
#define AP_BB_CON0          (0x10209800)
#define AP_ABIST_MON_CON0   (0x10209E00)
#define AP_ABIST_MON_CON1   (0x10209E04)
#define AP_ABIST_MON_CON2   (0x10209E08)
#define AP_ABIST_MON_CON3   (0x10209E0C)

#define CLK_CFG_0           (0x10000140)
#define CLK_CFG_1           (0x10000144)
#define CLK_CFG_2           (0x10000148)
#define CLK_CFG_3           (0x1000014C)
#define CLK_CFG_4           (0x10000150)
#define CLK_CFG_5           (0x10000154)
#define CLK_CFG_6           (0x10000158)
#define CLK_CFG_7           (0x1000015C)
#define CLK_MISC_CFG_2      (0x10000160)
#define CLK_CFG_8           (0x10000164)

#define TOP_CKMUXSEL        (0x10001000)
#define TOP_CKDIV1          (0x10001008)
#define TOP_DCMCTL          (0x10001008)

extern unsigned int mtk_get_bus_freq(void);

#endif