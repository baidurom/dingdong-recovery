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

#ifndef __LCD_REG_H__
#define __LCD_REG_H__

#include <stddef.h>
#include "disp_drv_platform.h"
#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    unsigned RUN        : 1;
    unsigned WAIT_CMDQ  : 1;
    unsigned WAIT_HTT   : 1;
    unsigned WAIT_SYNC  : 1;
    unsigned BUSY       : 1;
    unsigned RDMA       : 1;
	unsigned SMI        : 1;
    unsigned rsv_7      : 9;
} LCD_REG_STATUS, *PLCD_REG_STATUS;


typedef struct
{
    unsigned COMPLETED      : 1;
    unsigned CMDQ_COMPLETED : 1;
    unsigned HTT            : 1;
    unsigned SYNC           : 1;
    unsigned rsv_4          : 12;
} LCD_REG_INTERRUPT, *PLCD_REG_INTERRUPT;

 
typedef struct
{
	unsigned RESET     : 1;
    unsigned rsv_1     : 14;
    unsigned START     : 1;
} LCD_REG_START, *PLCD_REG_START;

typedef struct
{
    unsigned SIZE_0       : 3;
    unsigned THREE_WIRE_0 : 1;
    unsigned SDI_0        : 1;
    unsigned FIRST_POL_0    : 1;
    unsigned SCK_DEF_0    : 1;
    unsigned DIV2_0       : 1;
	unsigned SIZE_1       : 3;
    unsigned THREE_WIRE_1 : 1;
    unsigned SDI_1        : 1;
    unsigned FIRST_POL_1    : 1;
    unsigned SCK_DEF_1    : 1;
    unsigned DIV2_1       : 1;
    unsigned rsv_16       : 8;
	unsigned HW_CS        : 1;
	unsigned rsv_25       : 7;
} LCD_REG_SCNF, *PLCD_REG_SCNF;

typedef struct
{
    unsigned WST        : 6;
    unsigned rsv_6      : 2;
    unsigned C2WS       : 4;
    unsigned C2WH       : 4;
    unsigned RLT        : 6;
    unsigned rsv_22     : 2;
    unsigned C2RS       : 4;
    unsigned C2RH       : 4;
} LCD_REG_PCNF, *PLCD_REG_PCNF;

typedef struct
{
    unsigned ENABLE     : 1;
    unsigned EDGE_SEL   : 1;
    unsigned MODE       : 1;
	unsigned rsv_3      : 12;
    unsigned SW_TE      : 1;
    unsigned rsv_16     : 16;
} LCD_REG_TECON, *PLCD_REG_TECON;

typedef struct
{
    unsigned PCNF0_DW   : 3;
    unsigned rsv_3      : 1;
    unsigned PCNF1_DW   : 3;
    unsigned rsv_7      : 1;
    unsigned PCNF2_DW   : 3;
    unsigned rsv_11     : 5;
	unsigned PCNF0_CHW  : 4;
	unsigned PCNF1_CHW  : 4;
	unsigned PCNF2_CHW  : 4;
	unsigned rsv_28     : 4;
} LCD_REG_PCNFDW, *PLCD_REG_PCNFDW;

typedef struct
{
    UINT16 WIDTH;
    UINT16 HEIGHT;
} LCD_REG_SIZE, *PLCD_REG_SIZE;

typedef struct
{
	unsigned rsv_0   :4;
	unsigned addr    :4;
	unsigned rsv_8   :8;
} LCD_REG_CMD_ADDR, *PLCD_REG_CMD_ADDR;

typedef struct
{
	unsigned rsv_0   :4;
	unsigned addr    :4;
	unsigned rsv_8   :8;
} LCD_REG_DAT_ADDR, *PLCD_REG_DAT_ADDR;


typedef struct
{
    unsigned RGB_ORDER      : 1;
    unsigned BYTE_ORDER     : 1;
    unsigned PADDING        : 1;
    unsigned DATA_FMT       : 3;
    unsigned IF_FMT         : 2;
    unsigned COMMAND        : 5;
    unsigned rsv_13         : 2;
    unsigned ENC            : 1;
    unsigned rsv_16         : 8;
    unsigned SEND_RES_MODE  : 1;
    unsigned IF_24          : 1;
	unsigned rsv_6          : 6;
}LCD_REG_WROI_CON, *PLCD_REG_WROI_CON;

typedef struct {
    unsigned MAX_BURST          : 3;
    unsigned rsv_3              : 1;
    unsigned THROTTLE_EN        : 1;
    unsigned rsv_5              : 11;
    unsigned THROTTLE_PERIOD    : 16;
} LCD_REG_SMICON;

typedef struct
{
    unsigned DB_B        : 2;
    unsigned rsv_2       : 2;
    unsigned DB_G        : 2;
    unsigned rsv_6       : 2;
    unsigned DB_R        : 2;
    unsigned rsv_10      : 2;
    unsigned LFSR_B_SEED : 4;
    unsigned LFSR_G_SEED : 4;
    unsigned LFSR_R_SEED : 4;
    unsigned rsv_48      : 8;
} LCD_REG_DITHER_CON, *PLCD_REG_DITHER_CON;

typedef struct 
{
	unsigned WR_2ND		: 4;
	unsigned WR_1ST		: 4;
	unsigned RD_2ND		: 4;
	unsigned RD_1ST		: 4;
	unsigned CSH		: 4;
	unsigned CSS		: 4;
	unsigned rsv_24		: 8;
} LCD_REG_SIF_TIMING, *PLCD_REG_SIF_TIMING;

typedef struct
{
	unsigned CS0		: 1;
	unsigned CS1		: 1;
	unsigned rsv30		:30;
} LCD_REG_SIF_CS, *PLCD_REG_SIF_CS;

typedef struct 
{
	unsigned TIME_OUT	: 12;
	unsigned rsv_12		: 4;
	unsigned COUNT		: 12;
	unsigned rsv_28		: 4;
} LCD_REG_CALC_HTT, *PLCD_REG_CALC_HTT;

typedef struct 
{
	unsigned HTT		: 10;
	unsigned rsv_10		: 6;
	unsigned VTT		: 12;
	unsigned rsv_28		: 4;
} LCD_REG_SYNC_LCM_SIZE, *PLCD_REG_SYNC_LCM_SIZE;


typedef struct 
{
	unsigned WAITLINE	: 12;
	unsigned rsv_12		: 4;
	unsigned SCANLINE	: 12;
	unsigned rsv_28		: 4;
} LCD_REG_SYNC_CNT, *PLCD_REG_SYNC_CNT;

typedef struct 
{
	unsigned rsv_0      : 16;
	unsigned SWAP	 	: 1;
	unsigned ERR	    : 1;
	unsigned DITHER		: 1;
	unsigned MM_MODE	: 1;
	unsigned CFMT	    : 3;
	unsigned rsv_23     : 3;
	unsigned CSWAP		: 1;
	unsigned rsv_27     : 5;
} LCD_SRC_CON, *PLCD_SRC_CON;

typedef struct// this is tricky solution for 8320 build error, not real usage
{
    unsigned DC_DSI      : 1;
    unsigned BYTE_SWAP   : 1;
    unsigned RGB_SWAP    : 1;
    unsigned PAD_MSB     : 1;
    unsigned CLR_FMT     : 3;
    unsigned rsv_7       : 9;
	unsigned PACKET_SIZE : 9;
	unsigned rsv_25      : 7;
} LCD_REG_DSI_DC, *PLCD_REG_DSI_DC;

typedef struct
{
    LCD_REG_STATUS				STATUS;				// 1000
//    UINT16                    	rsv_0002;           // 1002
    LCD_REG_INTERRUPT         	INT_ENABLE;         // 1004
//    UINT16                    	rsv_0006;           // 1006
    LCD_REG_INTERRUPT         	INT_STATUS;         // 1008
//    UINT16                    	rsv_000A;           // 100A
    LCD_REG_START             	START;              // 100C
//    UINT16                    	rsv_000E;           // 100E
    UINT32                    	RESET;              // 1010
    UINT32                    	rsv_0014[2];        // 1014..1018
	LCD_REG_SIF_TIMING		  	SIF_TIMING[2];	  	// 101C..1020
    UINT32                    	rsv_0024;        	// 1024
	LCD_REG_SCNF			  	SERIAL_CFG;		  	// 1028
	LCD_REG_SIF_CS			  	SIF_CS;			  	// 102C
    LCD_REG_PCNF              	PARALLEL_CFG[3];    // 1030..1038
    LCD_REG_PCNFDW            	PARALLEL_DW;        // 103C
    LCD_REG_TECON             	TEARING_CFG;        // 1040
	LCD_REG_CALC_HTT			CALC_HTT;			// 1044
	LCD_REG_SYNC_LCM_SIZE		SYNC_LCM_SIZE;		// 1048
	LCD_REG_SYNC_CNT			SYNC_CNT;			// 104C
    LCD_REG_SMICON            	SMI_CON;            // 1050
    UINT32                    	rsv_0054[3];        // 1054..105C
    LCD_REG_WROI_CON          	WROI_CONTROL;       // 1060
    LCD_REG_CMD_ADDR           	WROI_CMD_ADDR;      // 1064
//    UINT16                      rsv_008A;
    LCD_REG_DAT_ADDR           	WROI_DATA_ADDR;     // 1068
//    UINT16                      rsv_008E;
    LCD_REG_SIZE              	WROI_SIZE;          // 106C
    LCD_SRC_CON                 SRC_CON;            // 1070
    UINT32                      SRC_ADD;            // 1074
    UINT32                      SRC_PITCH;          // 1078
    LCD_REG_DSI_DC              DS_DSI_CON;        	// 107C // this is tricky solution for 8320 build error, not real usage
    LCD_REG_DITHER_CON        	DITHER_CON;         // 1080
    UINT32                    	rsv_01F4[3];        // 1084..108C
    UINT32                    	ULTRA_CON;          // 1090
    UINT32                    	CONSUME_RATE;       // 1094
    UINT32                    	DBI_ULTRA_TH;       // 1098
    UINT32                    	GMC_ULTRA_TH;       // 109C
    UINT32         	            rsv_00A0[728];           // 10A0..1BFC
	UINT32						CMDQ[32];		    // 1C00..1C7C
	UINT32						rsv_1D00[160];		// 1C80.1EFC
    UINT32                    	PCMD0;             // 1F00
    UINT32                    	rsv_1F04[7];	    // 1F04..1F1C
    UINT32                    	PCMD1;             // 1F20 
    UINT32                    	rsv_1F24[7];  		// 1F24..1F3C
    UINT32                    	PCMD2;             // 1F40 
    UINT32                    	rsv_1F44[15];	   	// 1F44..1F7C
    UINT32                    	SCMD0;             // 1F80
    UINT32                    	rsv_1F84[7];   		// 1F84..1F9C
    UINT32                    	SCMD1;             // 1FA0
    UINT32                    	rsv_1FA4[7];    	// 1FA4..1FBC
} volatile LCD_REGS, *PLCD_REGS;

#ifndef BUILD_UBOOT
STATIC_ASSERT(0x0000 == offsetof(LCD_REGS, STATUS));
STATIC_ASSERT(0x0004 == offsetof(LCD_REGS, INT_ENABLE));
STATIC_ASSERT(0x0028 == offsetof(LCD_REGS, SERIAL_CFG));
STATIC_ASSERT(0x0030 == offsetof(LCD_REGS, PARALLEL_CFG));
STATIC_ASSERT(0x0040 == offsetof(LCD_REGS, TEARING_CFG));

STATIC_ASSERT(0x0080 == offsetof(LCD_REGS, DITHER_CON));
STATIC_ASSERT(0x0C00 == offsetof(LCD_REGS, CMDQ));

STATIC_ASSERT((0xF00) == offsetof(LCD_REGS, PCMD0));
STATIC_ASSERT((0xF80) == offsetof(LCD_REGS, SCMD0));

STATIC_ASSERT(0xFC0 == sizeof(LCD_REGS));
#endif
#define LCD_A0_LOW_OFFSET  (0x0)
#define LCD_A0_HIGH_OFFSET (0x10)

#ifdef __cplusplus
}
#endif

#endif // __LCD_REG_H__
