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

#ifndef __UFOE_REG_H__
#define __UFOE_REG_H__

#include <platform/mt_typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned ufoe_start                	: 1;
	unsigned ufoe_out_sel				: 1; 
    unsigned ufoe_bypass                : 1;
    unsigned rsv_3						: 5;
	unsigned ufoe_sw_reset				: 1;
	unsigned rsv_9						: 7;
	unsigned ufoe_dbg_sel				: 8;
	unsigned rsv_24						: 8;
} UFOE_START_REG, *PUFOE_START_REG;

typedef struct
{
    unsigned ufoe_fra_complete   	: 1;
	unsigned ufoe_fra_done		: 1; 
    unsigned ufoe_fra_underrun	: 1;
    unsigned rsv_3						: 29;
} UFOE_INTEN_REG, *PUFOE_INTEN_REG;

typedef struct
{
    unsigned ufoe_fra_complete   	: 1;
	unsigned ufoe_fra_done		: 1; 
    unsigned ufoe_fra_underrun	: 1;
    unsigned rsv_3						: 29;
} UFOE_INTSTA_REG, *PUFOE_INTSTA_REG;

typedef struct
{
    unsigned ufoe_crc_cen   	: 1;
	unsigned ufoe_crc_start		: 1; 
    unsigned ufoe_crc_clr		: 1;
    unsigned rsv_3				: 29;
} UFOE_CRC_REG, *PUFOE_CRC_REG;

typedef struct
{
    unsigned ufoe_crc_out_0   	: 16;
	unsigned ufoe_crc_rdy_0		: 1; 
    unsigned ufoe_engine_end	: 1;
    unsigned rsv_18				: 14;
} UFOE_R0_CRC_REG, *PUFOE_R0_CRC_REG;

typedef struct
{
	UINT32				UFOE_CFG_0B;//0x800
	UINT32				UFOE_CFG_1B;//0x804
	UINT32				rsv_808[446];//0x808~0xEFC
	UFOE_START_REG		UFOE_START;//0xF00
	UFOE_INTEN_REG		UFOE_INTEN;//0xF04
	UFOE_INTSTA_REG		UFOE_INTSTA;//0xF08
	UINT32				UFOE_DBUF;//0xF0C
	UINT32				rsv_F10;//0xF10
	UFOE_CRC_REG		UFOE_CRC;//0xF14
	UINT32				UFOE_SW_SCRATCH;//0xF18
	UINT32				rsv_F1C[3];
	UINT32				UFOE_CK_ON;//0xF28
	UINT32				rsv_F2C[9];
	UINT32				UFOE_FRA_WIDTH;//0xF50
	UINT32				UFOE_FRA_HEIGHT;//0xF54
	UINT32				rsv_F58[38];//0xF58~0xFEC
	UFOE_R0_CRC_REG		UFOE_R0_CRC;//0xFF0
} volatile UFOE_REGS, *PUFOE_REGS;

#ifndef BUILD_LK
STATIC_ASSERT(0x0700 == offsetof(UFOE_REGS, UFOE_START));
STATIC_ASSERT(0x0728 == offsetof(UFOE_REGS, UFOE_CK_ON));
STATIC_ASSERT(0x0750 == offsetof(UFOE_REGS, UFOE_FRA_WIDTH));
STATIC_ASSERT(0x07F0 == offsetof(UFOE_REGS, UFOE_R0_CRC));
#endif

#ifdef __cplusplus
}
#endif

#endif // __UFOE_REG_H__

