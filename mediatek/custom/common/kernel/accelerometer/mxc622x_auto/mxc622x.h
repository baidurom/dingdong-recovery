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

/*
 *
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 *
 * MXC622X driver for MT6516
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

#include <linux/ioctl.h>

extern struct acc_hw* mxc622x_get_cust_acc_hw(void); 
	
#define MXC622X_I2C_SLAVE_ADDR        0x2A

/* MXC622X register address */
#define MXC622X_REG_CTRL        0x04
#define MXC622X_REG_DATAX0        0x00

/* MXC622X control bit */
#define MXC622X_MEASURE_MODE      0x00    /* power on */
#define MXC622X_CTRL_PWRDN      0x80    /* power donw */

#define MXC622X_BUFSIZE				256

#if 0
/* Use 'm' as magic number */
#define MXC622X_IOM       'm'

/* IOCTLs for MXC622X device */
#define MXC622X_IOC_PWRON       _IO (MXC622X_IOM, 0x00)
#define MXC622X_IOC_PWRDN       _IO (MXC622X_IOM, 0x01)
#define MXC622X_IOC_READXYZ     _IOR(MXC622X_IOM, 0x05, int[3])
#define MXC622X_IOC_READSTATUS      _IOR(MXC622X_IOM, 0x07, int[3])
#define MXC622X_IOC_SETDETECTION    _IOW(MXC622X_IOM, 0x08, unsigned char)
#endif

/* MXC622X Register Map  (Please refer to MXC622X Specifications) */
#define MXC622X_REG_DEVID			0x00
#define MXC622X_REG_THRESH_TAP		0x1D
#define MXC622X_REG_OFSX			0x1E
#define MXC622X_REG_OFSY			0x1F
#define MXC622X_REG_OFSZ			0x20
#define MXC622X_REG_DUR				0x21
#define MXC622X_REG_THRESH_ACT		0x24
#define MXC622X_REG_THRESH_INACT	0x25
#define MXC622X_REG_TIME_INACT		0x26
#define MXC622X_REG_ACT_INACT_CTL	0x27
#define MXC622X_REG_THRESH_FF		0x28
#define MXC622X_REG_TIME_FF			0x29
#define MXC622X_REG_TAP_AXES		0x2A
#define MXC622X_REG_ACT_TAP_STATUS	0x2B
#define	MXC622X_REG_BW_RATE			0x2C
//#define MXC622X_REG_POWER_CTL		0x2D
#define MXC622X_REG_INT_ENABLE		0x2E
#define MXC622X_REG_INT_MAP			0x2F
#define MXC622X_REG_INT_SOURCE		0x30
#define MXC622X_REG_DATA_FORMAT		0x31
//#define MXC622X_REG_DATAX0			0x32
#define MXC622X_REG_FIFO_CTL		0x38
#define MXC622X_REG_FIFO_STATUS		0x39

#define MXC622X_FIXED_DEVID			0xE5
#define ADXL346_FIXED_DEVID			0xE6

#define MXC622X_BW_200HZ			0x0C
#define MXC622X_BW_100HZ			0x0B
#define MXC622X_BW_50HZ				0x0A

#define	MXC622X_FULLRANG_LSB		0XFF

//#define MXC622X_MEASURE_MODE		0x08
#define MXC622X_DATA_READY			0x80

#define MXC622X_FULL_RES			0x08
#define MXC622X_RANGE_2G			0x00
#define MXC622X_RANGE_4G			0x01
#define MXC622X_RANGE_8G			0x02
#define MXC622X_RANGE_16G			0x03
#define MXC622X_SELF_TEST           0x80

#define MXC622X_STREAM_MODE			0x80
#define MXC622X_SAMPLES_15			0x0F

#define MXC622X_FS_8G_LSB_G			64
#define MXC622X_FS_4G_LSB_G			128
#define MXC622X_FS_2G_LSB_G			256

#define MXC622X_LEFT_JUSTIFY		0x04
#define MXC622X_RIGHT_JUSTIFY		0x00

#define MXC622X_SUCCESS						0
#define MXC622X_ERR_I2C						-1
#define MXC622X_ERR_STATUS					-3
#define MXC622X_ERR_SETUP_FAILURE			-4
#define MXC622X_ERR_GETGSENSORDATA			-5
#define MXC622X_ERR_IDENTIFICATION			-6


