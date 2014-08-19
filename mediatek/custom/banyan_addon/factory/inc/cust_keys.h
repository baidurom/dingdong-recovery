/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef FTM_CUST_KEYS_H
#define FTM_CUST_KEYS_H

#include <linux/input.h>

#include <cust_lcd.h>
#include <cust_font.h>

/* available keys (Linux keycodes) */
#define KEY_CALL	KEY_SEND
#define KEY_ENDCALL	KEY_END
#undef KEY_OK
#define KEY_OK		KEY_REPLY
#define KEY_FOCUS	KEY_HP
#define KEY_AT		KEY_EMAIL
#define KEY_POUND	228
//#define KEY_STAR	227
#define KEY_DEL 	KEY_BACKSPACE
#define KEY_SYM		KEY_COMPOSE
/* KEY_HOME */
/* KEY_BACK */
/* KEY_VOLUMEDOWN */
/* KEY_VOLUMEUP */
/* KEY_MUTE */
/* KEY_MENU */
/* KEY_UP */
/* KEY_DOWN */
/* KEY_LEFT */
/* KEY_RIGHT */
/* KEY_CAMERA */
/* KEY_POWER */
/* KEY_TAB */
/* KEY_ENTER */
/* KEY_LEFTSHIFT */
/* KEY_COMMA */
/* KEY_DOT */
/* KEY_SLASH */
/* KEY_LEFTALT */
/* KEY_RIGHTALT */
/* KEY_SPACE */
/* KEY_SEARCH */
/* KEY_0 ~ KEY_9 */
/* KEY_A ~ KEY_Z */

#define KEYS_NUM_COLS	2
#define KEYS_COL_WIDTH	(CUST_LCD_AVAIL_WIDTH / KEYS_NUM_COLS)
#define KEYS_COL_LEN	(KEYS_COL_WIDTH / CHAR_WIDTH)

#define KEYS_COL_SPACE	2	/* chars */
#define KEYS_NAME_LEN	(KEYS_COL_LEN - KEYS_COL_SPACE)

struct key {
	int code;
	char name[KEYS_NAME_LEN + 1];	/* recommend: max 6 chars */
};

#define KEYS_PWRKEY_MAP		{ KEY_ENDCALL, "PwrEnd" }

#define DEFINE_KEYS_KEYMAP(x)		\
struct key x[] = {			\
	KEYS_PWRKEY_MAP,		\
	{ KEY_MENU,       "Menu" },	\
	{ KEY_HOME,       "Home" },	\
	{ KEY_BACK,       "Back" },	\
	{ KEY_UP,         "Up" },	\
	{ KEY_DOWN,       "Down" },	\
	{ KEY_VOLUMEUP,   "VLUp" },	\
	{ KEY_VOLUMEDOWN, "VLDown" },	\
}

#define CUST_KEY_UP		KEY_UP
#define CUST_KEY_VOLUP		KEY_VOLUMEUP
#define CUST_KEY_DOWN		KEY_DOWN
#define CUST_KEY_VOLDOWN	KEY_VOLUMEDOWN
#define CUST_KEY_LEFT		KEY_MENU
#define CUST_KEY_CENTER		KEY_HOME
#define CUST_KEY_RIGHT		KEY_BACK

#define CUST_KEY_CONFIRM	KEY_HOME
#define CUST_KEY_BACK		KEY_BACK

#endif /* FTM_CUST_KEYS_H */
