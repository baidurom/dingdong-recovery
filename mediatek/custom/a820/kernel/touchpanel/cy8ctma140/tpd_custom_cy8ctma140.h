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

#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#define TPD_POWER_SOURCE         MT6575_POWER_VGP2
#define TPD_POWER_SOURCE_1800 MT65XX_POWER_LDO_VGP5
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

#define TPD_DELAY                (2*HZ/100)
//#define TPD_RES_X                480
//#define TPD_RES_Y                800
#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};

//#define TPD_HAVE_CALIBRATION
//#define TPD_HAVE_BUTTON
//#define TPD_HAVE_TREMBLE_ELIMINATION
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#if (defined(WVGA))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,870,60,50},{180,870,60,50},{300,870,60,50},{420,870,60,50}}

#elif (defined(FWVGA))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,920,60,50},{180,920,60,50},{300,920,60,50},{420,920,60,50}}

#elif (defined(QHD))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{85,1030,60,50},{185,1030,60,50},{350,1030,60,50},{500,1030,60,50}}

#elif (defined(HD))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{90,1350,60,50},{270,1350,60,50},{430,1350,60,50},{630,1350,60,50}}

#elif (defined(HVGA))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{40,530,60,50},{120,530,60,50},{200,530,60,50},{280,530,60,50}}

#elif (defined(LQHD))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{50,1030,60,50},{185,1030,60,50},{350,1030,60,50},{500,1030,60,50}}

#else

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,920,60,50},{180,920,60,50},{300,920,60,50},{420,920,60,50}}

#endif

#endif /* TOUCHPANEL_H__ */
