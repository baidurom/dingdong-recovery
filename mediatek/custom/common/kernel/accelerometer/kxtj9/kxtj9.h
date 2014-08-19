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

/* linux/drivers/hwmon/kxud91026.h
 *
 * (C) Copyright 2008 
 * MediaTek <www.mediatek.com>
 *
 * KXTJ9 driver for MT6516
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __KXTJ9_H
#define __KXTJ9_H
/******************************************************************************
 * Function Configuration
******************************************************************************/
//#define KXTJ9_TEST_MODE
/******************************************************************************
 * Definition
******************************************************************************/
#define KXT_TAG					"<KXTJ9> "
#define KXT_DEV_NAME			"KXTJ9"
#define KXT_FUN(f)				printk(KXT_TAG"%s\n", __FUNCTION__)
#define KXT_ERR(fmt, args...)	printk(KXT_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define KXT_LOG(fmt, args...)	printk(KXT_TAG fmt, ##args)
#define KXT_VER(fmt, args...)   ((void)0)
#define KXTJ9_WR_SLAVE_ADDR	    (0x1E)
#define KXTJ9_AXES_NUM		    3
#define KXTJ9_DATA_LEN          3
#define KXTJ9_ADDR_MAX          0x5F
#define KXTJ9_AXIS_X			0
#define KXTJ9_AXIS_Y			1
#define KXTJ9_AXIS_Z			2
/*position in the sysfs attribute array*/
#define KXTJ9_ATTR_X_OFFSET	    0
#define KXTJ9_ATTR_Y_OFFSET	    1
#define KXTJ9_ATTR_Z_OFFSET	    2
#define KXTJ9_ATTR_DATA		    3
#define KXTJ9_ATTR_REGS         4
/*In MT6516, KXTJ9 is connected to I2C Controller 3: SCL2, SDA2*/
#define KXTJ9_I2C_ID	        2 

/*Register definition*/
#define KXTJ9_REG_XOUT              0x06
#define KXTJ9_REG_YOUT              0x13
#define KXTJ9_REG_ZOUT              0x14
#define KXTJ9_REG_CTRL_REG1         0x1B
#define KXTJ9_REG_CTRL_REG2         0x1D
#define KXTJ9_REG_INT_CTRL_REG1     0x1E

#define KXTJ9_SENSITIVITY       1024
#define KXTJ9_DATA_COUNT(X)     ((X) >> 2)
#define KXTJ9_DATA_TO_G(X)      (1000 * ((X)-KXTJ9_0G_OFFSET)/KXTJ9_SENSITIVITY)

/*default value*/
#define KXTJ9_BUFSIZE				256


#endif /*__KXTJ9_H*/
