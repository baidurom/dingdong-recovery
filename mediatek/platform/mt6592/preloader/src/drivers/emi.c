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

#include <typedefs.h>
#include <platform.h>
#include <emi.h>
#include <dramc.h>
#include <platform.h>
#include <pll.h>
#include "wdt.h"
#include "custom_emi.h"
#include <emi_hw.h>

//#define GATING_CLOCK_CONTROL_DVT
// Macro definition.
/* These PLL config is defined in pll.h */
//#define DDRPHY_3PLL_MODE
#define REAL_MEMPLL_MDL
/* To define DDRTYPE for bring up */
#define DDRTYPE_LPDDR2	
//#define DDRTYPE_LPDDR3
//#define DDRTYPE_DDR3

#if 0
#define APMIXED_BASE 			(0x10209000)
#ifdef AP_PLL_CON1
#undef AP_PLL_CON1
#endif
#ifdef AP_PLL_CON2
#undef AP_PLL_CON2
#endif
#define AP_PLL_CON1	((volatile unsigned int *)(APMIXED_BASE+0x0004))
#define AP_PLL_CON2	((volatile unsigned int *)(APMIXED_BASE+0x0008))
#endif
#define print 
#define DDR_1333 1 //Science LPDDR3 Run in 1333MHz.
#define VcHV_VmHV  0
#define VcLV_VmLV  0
#define VcNV_VmNV  1
#define VcHV_VmLV  0
#define VcLV_VmHV  0

#define ENABLE_DFS
static int enable_combo_dis = 0;
#define REPAIR_SRAM
#ifdef REPAIR_SRAM
extern int repair_sram(void);
#endif

extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];

#ifdef ENABLE_DFS
extern char *opt_gw_coarse_value0, *opt_gw_fine_value0;
extern char *opt_gw_coarse_value1, *opt_gw_fine_value1;
#endif

//#define DERATING_ENABLE
#ifdef DERATING_ENABLE
void derating_enable(void);
#endif


#define REXTDN_ENABLE
#ifdef REXTDN_ENABLE
void ett_rextdn_sw_calibration(void);
unsigned int drvp, drvn;
#endif
int DDR_type;

#define COMBO_MCP
#ifndef COMBO_MCP
#ifdef DDRTYPE_LPDDR2
#define    EMI_CONA_VAL         0x0002A3AE   
#define    DRAMC_ACTIM_VAL      0x44584493   
#define    DRAMC_GDDR3CTL1_VAL  0x01000000   
#define    DRAMC_CONF1_VAL      0xF0048483   
#define    DRAMC_DDR2CTL_VAL    0xA00632D1  
#define    DRAMC_MODE_REG_63    0x0000003F   
#define    DRAMC_MODE_REG_10    0x00FF000A   
#define    DRAMC_MODE_REG_1     0x00C30001   
#define    DRAMC_MODE_REG_2     0x00060002    
#define    DRAMC_MODE_REG_3     0x00020003   
#define    DRAMC_TEST2_3_VAL    0xBF000401    
#define    DRAMC_CONF2_VAL      0x0340633F    
#define    DRAMC_PD_CTRL_VAL    0xd1642342    
#define    DRAMC_ACTIM1_VAL     0x01000510    
#define    DRAMC_ACTIM05T_VAL   0x04002600    
#define    DRAMC_MISCTL0_VAL    0x07800000    

#define DRAMC_DRVCTL0_VAL 0xAA22AA22  
#define DRAMC_DRVCTL1_VAL 0xAA22AA22  
#define DRAMC_PADCTL3_VAL 0x0       
#define DRAMC_DQODLY_VAL 0xEEEEEEEE    
#define CMD_ADDR_OUTPUT_DLY 0x0       
#define CLK_OUTPUT_DLY 0x0         

#endif

#ifdef DDRTYPE_LPDDR3
//H9TQ65A8JTMCUR
#define    EMI_CONA_VAL         0x2A3AE
#define    DRAMC_ACTIM_VAL      0x44584493
#define    DRAMC_GDDR3CTL1_VAL  0x01000000
#define    DRAMC_CONF1_VAL      0xF0048483
#define    DRAMC_DDR2CTL_VAL    0xA00632D1
#define    DRAMC_MODE_REG_63    0x0000003F
#define    DRAMC_MODE_REG_10    0x00FF000A
#define    DRAMC_MODE_REG_1     0x00C30001
#define    DRAMC_MODE_REG_2     0x00060002
#define    DRAMC_MODE_REG_3     0x00020003
#define    DRAMC_TEST2_3_VAL    0xBF080401
#define    DRAMC_CONF2_VAL      0x0340633F
#define    DRAMC_PD_CTRL_VAL    0xd1642342
#define    DRAMC_ACTIM1_VAL     0x11000510
#define    DRAMC_ACTIM05T_VAL   0x04002600
#define    DRAMC_MISCTL0_VAL    0x17800000

#define DRAMC_PADCTL3_VAL 0x0
#define DRAMC_DQODLY_VAL 0xEEEEEEEE
#define CMD_ADDR_OUTPUT_DLY 0x0
#define CLK_OUTPUT_DLY 0x0
#define DRAMC_DRVCTL0_VAL 0xAA22AA22
#define DRAMC_DRVCTL1_VAL 0xAA22AA22



#endif 

#endif
//#define FLASH_TOOL_PRELOADER
//#define DUMP_DRAMC_REGS
//#define HARDCODE_DRAM_SETTING
#ifdef HARDCODE_DRAM_SETTING
#define NUM_EMI_RECORD_HARD_CODE (13)
#define emi_settings emi_settings_hard_code
EMI_SETTINGS emi_settings_hard_code[] =
{
        //H9TCNNNBLDMMPR
        {
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3FE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048783,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x03406340,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x40000000,0x40000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
        } ,
        //EDBA232B1MA
        {
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3FE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048783,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x40000000,0x40000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000003,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
        } ,
        //K4PAG304EB_FGC1
        {
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3FE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048783,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x20000000,0x20000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000001,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
        } ,
        //H9TP32A8JDACPR_KGM
        {
                0x0,            /* sub_version */
                0x0202,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x90,0x01,0x4A,0x48,0x34,0x47,0x31,0x64,0x04,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3AE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x20000000,0x20000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
        } ,
        //KMK7U000VM_B309
        {
                0x0,            /* sub_version */
                0x0202,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x15,0x01,0x00,0x4B,0x37,0x55,0x30,0x30,0x4D,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3AE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x03406340,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x00002230,             /* DRAMC_ACTIM05T_VAL*/
                {0x20000000,0x20000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000001,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
        } ,
        //H9TQ65A8JTMCUR_KTM
        {
                0x0,            /* sub_version */
                0x0203,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x90,0x01,0x4A,0x48,0x38,0x47,0x32,0x64,0x04,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3AE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x11000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x20000000,0x20000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0X00C30001,             /* LPDDR3_MODE_REG1 */
                0X00060002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000006,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
        } ,
        //H9TQ18ABJTMCUR
        {
                0x0,            /* sub_version */
                0x0203,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x90,0x01,0x4A,0x48,0x41,0x47,0x34,0x64,0x04,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002F3AE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF0D0401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1643842,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x11000D20,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x40000000,0x40000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0X00C30001,             /* LPDDR3_MODE_REG1 */
                0X00060002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000006,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
        } ,
        {
            //LPDDR2 512MB
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x0000212E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0x51642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
            },
            {
        //LPDDR3 512MB
                0x0,            /* sub_version */
                0x0003,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x0000212E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0x51642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x11000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00C30001,             /* LPDDR3_MODE_REG1 */
                0x00060002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000006,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
             },
		{
        //PCDDR3 1GB
                0x0,            /* sub_version */
                0x0004,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x00003122,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x33584562,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF07484A2,             /* DRAMC_CONF1_VAL */
                0x918E11C1,             /* DRAMC_DDR2CTL_VAL */
                0xBF850401,             /* DRAMC_TEST2_3_VAL */
                0x03046960,             /* DRAMC_CONF2_VAL */
                0x51972842,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x00000640,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x00000000,             /* DRAMC_ACTIM05T_VAL*/
                {0x40000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00001941,             /* LPDDR3_MODE_REG1 */
                0x00002000,             /* LPDDR3_MODE_REG2 */
                0x00004008,             /* LPDDR3_MODE_REG3 */
                0x00006000,             /* LPDDR3_MODE_REG5 */
                0x00000024,             /* LPDDR3_MODE_REG10 */
                0x00000000,             /* LPDDR3_MODE_REG63 */
             },
        //KMR4Z0001M_B802
        {
                0x0,            /* sub_version */
                0x0203,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x15,0x01,0x00,0x52,0x34,0x5A,0x31,0x4D,0x42,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3FE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048783,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF0D0401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1643842,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x11000D20,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x40000000,0x40000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0X00C30001,             /* LPDDR3_MODE_REG1 */
                0X00060002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000001,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
             },

        //KMK7X000VM-B314
        {
                0x0,            /* sub_version */
                0x0202,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x15,0x01,0x00,0x4B,0x37,0x55,0x30,0x30,0x4D,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A3AE,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x03406340,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x00002230,             /* DRAMC_ACTIM05T_VAL*/
                {0x20000000,0x20000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000001,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
        }, 
        //H9TP32A6DDMCPR
        {
                0x0,            /* sub_version */
                0x0202,         /* TYPE */
                9,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x90,0x01,0x4A,0x4e,0x49,0x58,0x20,0x00,0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* NAND_EMMC_ID */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* FW_ID */
                0x0002A36E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048583,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0xD1642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0x88888888,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x20000000,0x10000000,0,0},            /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},          /* reserved 10 */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
             }
        };
#endif

/* select the corresponding memory devices */
extern int dramc_calib(void);

#if 0
/*
 * init_dram: Do initialization for LPDDR.
 */
static void init_lpddr1(EMI_SETTINGS *emi_setting) 
{
    return;
}
#endif
EMI_SETTINGS emi_setting_default_lpddr2 =
{

        //default
                0x0,            /* sub_version */
                0x0002,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x0000212E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632D1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0x51642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0xEEEEEEEE,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x01000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00C30001,             /* LPDDR2_MODE_REG1 */
                0x00060002,             /* LPDDR2_MODE_REG2 */
                0x00020003,             /* LPDDR2_MODE_REG3 */
                0x00000006,             /* LPDDR2_MODE_REG5 */
                0x00FF000A,             /* LPDDR2_MODE_REG10 */
                0x0000003F,             /* LPDDR2_MODE_REG63 */
};
EMI_SETTINGS emi_setting_default_lpddr3 =
{

        //default
                0x0,            /* sub_version */
                0x0003,         /* TYPE */
                0,              /* EMMC ID/FW ID checking length */
                0,              /* FW length */
                {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},              /* NAND_EMMC_ID */
                {0x00,0x0,0x0,0x0,0x0,0x0,0x0,0x0},             /* FW_ID */
                0x0000212E,             /* EMI_CONA_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL0_VAL */
                0xAA00AA00,             /* DRAMC_DRVCTL1_VAL */
                0x44584493,             /* DRAMC_ACTIM_VAL */
                0x01000000,             /* DRAMC_GDDR3CTL1_VAL */
                0xF0048683,             /* DRAMC_CONF1_VAL */
                0xA00632F1,             /* DRAMC_DDR2CTL_VAL */
                0xBF080401,             /* DRAMC_TEST2_3_VAL */
                0x0340633F,             /* DRAMC_CONF2_VAL */
                0x51642342,             /* DRAMC_PD_CTRL_VAL */
                0x00008888,             /* DRAMC_PADCTL3_VAL */
                0xEEEEEEEE,             /* DRAMC_DQODLY_VAL */
                0x00000000,             /* DRAMC_ADDR_OUTPUT_DLY */
                0x00000000,             /* DRAMC_CLK_OUTPUT_DLY */
                0x11000510,             /* DRAMC_ACTIM1_VAL*/
                0x07800000,             /* DRAMC_MISCTL0_VAL*/
                0x04002600,             /* DRAMC_ACTIM05T_VAL*/
                {0x10000000,0,0,0},                 /* DRAM RANK SIZE */
                {0,0,0,0,0,0,0,0,0,0},              /* reserved 10*4 bytes */
                0x00C30001,             /* LPDDR3_MODE_REG1 */
                0x00060002,             /* LPDDR3_MODE_REG2 */
                0x00020003,             /* LPDDR3_MODE_REG3 */
                0x00000006,             /* LPDDR3_MODE_REG5 */
                0x00FF000A,             /* LPDDR3_MODE_REG10 */
                0x0000003F,             /* LPDDR3_MODE_REG63 */
};
static int mt_get_dram_type_for_dis(void)
{
    int i;
    int type = 2;
    type = (emi_settings[0].type & 0xF);
    for (i = 0 ; i < num_of_emi_records; i++)
    {
      //print("[EMI][%d] type%d\n",i,type);
      if (type != (emi_settings[0].type & 0xF))
      {
          print("It's not allow to combine two type dram when combo discrete dram enable\n");
          ASSERT(0);
          break;
      }
    }
    return type;
}

unsigned int DRAM_MRR(int MRR_num)
{
    unsigned int MRR_value = 0x0;
    unsigned int bak_1e8;
    unsigned int MRR_DQ_map[3];
    unsigned int dram_type;
    bak_1e8 = DRAMC_READ_REG(0x01E8);

    dram_type = mt_get_dram_type_for_dis();

    if (TYPE_LPDDR2 == dram_type)
    {
        MRR_DQ_map[0] = 0x000A0B09;
        MRR_DQ_map[1] = 0x000E0C08;
        MRR_DQ_map[2] = 0x00000D0F;
    }
    else if (TYPE_LPDDR3 == dram_type)
    {
        MRR_DQ_map[0] = 0x00080B09;
        MRR_DQ_map[1] = 0x000F0D0A;
        MRR_DQ_map[2] = 0x00000E0C;
    }

    //DRAMC_WRITE_CLEAR(0x00320000,0x1E8);
    //Setup DQ swap for bit[2:0]
    DRAMC_WRITE_REG(MRR_DQ_map[0],0x1F4);

    //Set MRSMA
    DRAMC_WRITE_REG(MRR_num,0x088);

    /*Enable MRR read for reading LPDDR2 refresh rate*/
    DRAMC_WRITE_SET(0x00010000,0x1E8);
    print("Enable MRR read: 0x1E8:%x\n",DRAMC_READ_REG(0x01E8));
    gpt_busy_wait_us(1000);//Wait > 1000000us
    MRR_value = (DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8;
    print("0x1f4:%x,refresh_rate[10:8]:%d\n",DRAMC_READ_REG(0x01F4),(DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8);

    //Setup DQ swap for bit[5:3]
    DRAMC_WRITE_REG(MRR_DQ_map[1],0x1F4);

    gpt_busy_wait_us(1000);//Wait > 1000000us
    MRR_value |= ((DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8) << 0x3;
    print("0x1f4:%x,refresh_rate[10:8]:%d\n",DRAMC_READ_REG(0x01F4),(DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8);
    //Setup DQ swap for bit[9:6]
    DRAMC_WRITE_REG(MRR_DQ_map[2],0x1F4);

    gpt_busy_wait_us(1000);//Wait > 1000000us
    MRR_value |= (((DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8) << 0x6);
    print("0x1f4:%x,refresh_rate[10:8]:%d\n",DRAMC_READ_REG(0x01F4),(DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8);
    //print("MRR_value:%x\n",MRR_value);
    /*Disable MRR read for reading LPDDR2 refresh rate*/
    DRAMC_WRITE_REG(bak_1e8,0x1E8);
    print("Disable MRR read: 0x1E8:%x\n",DRAMC_READ_REG(0x01E8));

    /* reset infra*/
    print("Before infra reset, 0x3B8:%x,0x10001034:%x\n",(((DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8) ),*(volatile unsigned *)(0x10001034));
    *(volatile unsigned *)(0x10001034) |= (1 << 0x2);
    *(volatile unsigned *)(0x10001034) &= ~(1 << 0x2);

    *(volatile unsigned *)(0x100040E4) |= (1 << 0x2);
    *(volatile unsigned *)(0x100040E4) &= ~(1 << 0x2);
    print("After infra reset, 0x3B8:%x,0x10001034:%x\n",(((DRAMC_READ_REG(0x03B8) & 0x000000300) >> 8) ),*(volatile unsigned *)(0x10001034));


    return MRR_value;
}

#if 0//PoP derating function enable

void derating_enable(void)
{
    unsigned int TRRD_DERATE = 0x03 <<28;
    unsigned int TRPAB_DERATE = 0x01 << 24;
    unsigned int  TRP_DERATE = 0x05 << 20;
    unsigned int  TRAS_DERATE = 0x05 <<16;
    unsigned int  TRC_DERATE = 0x0A << 8;
    unsigned int  TRCD_DERATE = 0x05 <<4;
    unsigned int  val1=0;
    /* setup derating AC timing */
    *((volatile unsigned *)(DRAMC0_BASE + 0x01f0)) = TRRD_DERATE | TRPAB_DERATE | TRP_DERATE | TRAS_DERATE | TRC_DERATE | TRCD_DERATE ;
    DRAMC_WRITE_REG(0x00080b09,0x1F4); //orignal: 0x3, pinmux, need to port

    //Set MRSMA
    DRAMC_WRITE_REG(0x4,0x088);

    /*Enable MRR read for reading LPDDR2 refresh rate*/
    val1 = (DRAMC_READ_REG(0x1E8) | (0x00010000));
    DRAMC_WRITE_REG(val1,0x1E8);
    print("[EMI] MRR 0x3B8:%x\n",(((DRAMC_READ_REG(0x03B8) & 0x000000700) >> 8) ));

    print("[EMI] Enable derating function\n");
    /*Enable derating function*/
    val1 = (DRAMC_READ_REG(0x1F0) | (0x00000001));
    DRAMC_WRITE_REG(val1,0x1F0);

    /*Enable derating interrupt*/
   // val1 = (DRAMC_READ_REG(0x0F4) | (0x08000000));
    //DRAMC_WRITE_REG(val1,0x0F4);

}

#endif












#define TOPRGU_BASE                 RGU_BASE
#define UNLOCK_KEY_MODE (0x22000000)
#define UNLOCK_KEY_SWRST (0x00001209)
#define UNLOCK_KEY_DEBUG_CTL (0x59000000)
#define TIMEOUT 3
extern u32 g_ddr_reserve_enable;
extern u32 g_ddr_reserve_success;


// EMI address definition.
#define EMI_CONA 		((volatile unsigned int *)(EMI_BASE+0x000))
#define EMI_CONB		((volatile unsigned int *)(EMI_BASE+0x008))
#define EMI_CONC		((volatile unsigned int *)(EMI_BASE+0x010))
#define EMI_COND		((volatile unsigned int *)(EMI_BASE+0x018))
#define EMI_CONE		((volatile unsigned int *)(EMI_BASE+0x020))
#define EMI_CONF                ((volatile unsigned int *)(EMI_BASE+0x028))
#define EMI_CONG		((volatile unsigned int *)(EMI_BASE+0x030))
#define EMI_CONH		((volatile unsigned int *)(EMI_BASE+0x038))
#define EMI_CONM		((volatile unsigned int *)(EMI_BASE+0x060))
#define EMI_DFTB		((volatile unsigned int *)(EMI_BASE+0x0E8))		// TESTB
#define EMI_DFTD		((volatile unsigned int *)(EMI_BASE+0x0F8))		// TESTD
#define EMI_SLCT		((volatile unsigned int *)(EMI_BASE+0x158))
#define EMI_ARBA		((volatile unsigned int *)(EMI_BASE+0x100))
#define EMI_ARBB		((volatile unsigned int *)(EMI_BASE+0x108))
#define EMI_ARBC		((volatile unsigned int *)(EMI_BASE+0x110))
#define EMI_ARBD		((volatile unsigned int *)(EMI_BASE+0x118))
#define EMI_ARBE		((volatile unsigned int *)(EMI_BASE+0x120))
#define EMI_ARBF		((volatile unsigned int *)(EMI_BASE+0x128))
#define EMI_ARBG		((volatile unsigned int *)(EMI_BASE+0x130))
#define EMI_ARBI		((volatile unsigned int *)(EMI_BASE+0x140))
#define EMI_BMEN		((volatile unsigned int *)(EMI_BASE+0x400))

typedef volatile unsigned int *V_UINT32P;
/*
 * init_lpddr2: Do initialization for LPDDR2.
 */
static void init_lpddr2(EMI_SETTINGS *emi_setting) 
{
        // The following settings are from Yi-Chih.
        /* TINFO="Star mm_cosim_emi_config" */
        //----------------EMI Setting--------------------
        *((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
        *((V_UINT32P)(EMI_BASE   +0x0140))=0x20406188;//0x12202488;   // 83 for low latency
        *((V_UINT32P)(EMI_BASE   +0x0144))=0x20406188;//0x12202488; //new add
        *((V_UINT32P)(EMI_BASE   +0x0100))=0x40105806;//0x40105808; //m0 cpu ori:0x8020783f
        *((V_UINT32P)(EMI_BASE   +0x0108))=0x808050ea;//0xa0a05028; //m1 ori:0x80200000 // ultra can over limit
        *((V_UINT32P)(EMI_BASE   +0x0110))=0xffff50c4;//0xa0a05001; //m2 arm9
        *((V_UINT32P)(EMI_BASE   +0x0118))=0x0810d84a;//0x030fd80d; // ???????????????????????????????????????????????????????????//m3 mdmcu ori:0x80807809 ori:0x07007010 bit[12]:enable bw limiter
        *((V_UINT32P)(EMI_BASE   +0x0120))=0x40405042; //m4 fcore ori:0x8080781a
        // *((V_UINT32P)(EMI_BASE   +0x0128))=0xa0a05001; //m5 peri
        // *((V_UINT32P)(EMI_BASE   +0x0130))=0xa0a05028; //m6 vcodec ori:0x8080381a
        *((V_UINT32P)(EMI_BASE   +0x0148))=0x9719595e;//0323 chg, ori :0x00462f2f
        *((V_UINT32P)(EMI_BASE   +0x014c))=0x9719595e; // new add
        //-----------------------------------
        //-- modified by liya for EMI address mapping setting
        *EMI_CONA =emi_setting->EMI_CONA_VAL;  //8Gb, 4Gb per rank

        //*((V_UINT32P)(EMI_BASE   +0x0000))=0x1130112e;
        //-----------------------------------

        *((V_UINT32P)(EMI_BASE   +0x00f8))=0x00000000;
        *((V_UINT32P)(EMI_BASE   +0x0400))=0x007f0001;
        *((V_UINT32P)(EMI_BASE   +0x0008))=0x05121624;// Chuan:0x181e1e2b Edwin:0x12161620
        *((V_UINT32P)(EMI_BASE   +0x0010))=0x04070506;// Chuan:0x18181818 Edwin:0x12121212
        *((V_UINT32P)(EMI_BASE   +0x0018))=0x05121624;// Chuan:0x1e262b33 Edwin:0x161c2026
        *((V_UINT32P)(EMI_BASE   +0x0020))=0x04070506;// Chuan:0x1818181b Edwin:0x12121214
        *((V_UINT32P)(EMI_BASE   +0x0030))=0x04050516;
        *((V_UINT32P)(EMI_BASE   +0x0038))=0x04050516;
        *((V_UINT32P)(EMI_BASE   +0x0158))=0x00010800;// ???????????????????????0x08090800;
        *((V_UINT32P)(EMI_BASE   +0x0150))=0x6470fc79;
        *((V_UINT32P)(EMI_BASE   +0x0154))=0x6470fc79;
        *((V_UINT32P)(EMI_BASE   +0x00e8))=0x00000100;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
        //============== Defer WR threthold
        *((V_UINT32P)(EMI_BASE   +0x00f0))=0x38470000;
        //============== Reserve bufer
        //
        //MDMCU don't always ULTRA, but small age
        *((V_UINT32P)(EMI_BASE   +0x0078))=0x23110003;// ???????????0x00030F4d;
        // Turn on M1 Ultra and all port DRAMC hi enable
        *((V_UINT32P)(EMI_BASE   +0x0158))=0x1f011f00;// ???????????????????????0x08090800;
        // RFF)_PBC_MASK
        *((V_UINT32P)(EMI_BASE   +0x00e8))=0x00001004;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
        // Page hit is high
        *((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
        *((V_UINT32P)(EMI_BASE   +0x00d0))=0x84848c84;//R/8 W/8 outstanding
        *((V_UINT32P)(EMI_BASE   +0x00d8))=0x00480084;//R/8 W/8 outstanding
        //===========END===========================================
       //---- add for real chip--------------------------//

#ifdef REXTDN_ENABLE
        *((V_UINT32P)(DRAMC0_BASE + 0x00b4)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));   
        *((V_UINT32P)(DDRPHY_BASE + 0x00b4)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DRAMC0_BASE + 0x00b8)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));   
        *((V_UINT32P)(DDRPHY_BASE + 0x00b8)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DRAMC0_BASE + 0x00bc)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));   
        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
#else
        // TX driving 0x0a
        *((V_UINT32P)(DRAMC0_BASE + 0x00b4)) = 0xAA22AA22;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
        *((V_UINT32P)(DDRPHY_BASE + 0x00b4)) = 0xAA22AA22;
        *((V_UINT32P)(DRAMC0_BASE + 0x00b8)) = 0xAA22AA22;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
        *((V_UINT32P)(DDRPHY_BASE + 0x00b8)) = 0xAA22AA22;
        *((V_UINT32P)(DRAMC0_BASE + 0x00bc)) = 0xAA22AA22;   //[31:28]CLK DRVP/[27:24]CLK DRVN;[15:12]CMD DRVP/[11:8]CMD DRVN;, default: 0xaa22aa22
        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) = 0xAA22AA22;
#endif
        // Pre-emphasis ON. Should be set after MEMPLL init.
        *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= 0x00003f00;
        *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= 0x00003f00;
#ifdef DRAMC_ASYNC
        *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = 0x07900000 ;  //ASYNC // Edward: [23] PBC_ARB_E1T=1 [16]MODE18V=0in 6582
        *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = 0x07900000 ;  //ASYNC
#else
        *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL ;
        *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL ;
#endif

        *((V_UINT32P)(DRAMC0_BASE + 0x0048)) = 0x0001110d;
        *((V_UINT32P)(DDRPHY_BASE + 0x0048)) = 0x0001110d;

        *((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x00500900;    //pinmux // // Edward : 100b for LPDDR2/3, 110b for DDR3x16x2
        *((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x00500900;    //pinmux

        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x008c)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x008c)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x0090)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0090)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x80000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x80000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x00dc)) = 0x83004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00dc)) = 0x83004004;

        //*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x19004004;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x19004004;
#ifdef SYSTEM_26M //Edwin
            *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;
            *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080022;
        *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080022;
#else
        #ifdef DDRPHY_3PLL_MODE
            *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1c004004;
            *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1c004004;
        #else
            *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
            *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
        #endif

        #ifdef DDRPHY_3PLL_MODE
            *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080011;
            *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080011;
        #else
            *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080033;
            *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080033;
        #endif
#endif

    *((V_UINT32P)(DRAMC0_BASE + 0x00f0)) = 0xc0000000;  // Edward: [31]DQ4BMUX=1 in SBS. [30]S2=1, Edward : 100b for LPDDR2/3, 110b for DDR3x16x2
    *((V_UINT32P)(DDRPHY_BASE + 0x00f0)) = 0xc0000000;

    //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01000000;
    //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01000000;
    #ifdef DRAMC_ASYNC
      //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x11100000;
      //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x11100000;
       *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;      // PHYSYNCM, default to 0
      *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #else
      //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01100000;
      //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01100000;
       *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
      *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #endif

    #ifdef PHYSYNC_MODE
     *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) |= 0x10000000;
      *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) |= 0x10000000;

    #endif

    *((V_UINT32P)(DRAMC0_BASE + 0x0168)) = 0x00000080;
    *((V_UINT32P)(DDRPHY_BASE + 0x0168)) = 0x00000080;


    *((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x00700900;
    *((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x00700900;
   *((V_UINT32P)(DRAMC0_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;  // Edward: MATYPE[9:8] is for test engine use, set to 0 is safe.
    *((V_UINT32P)(DDRPHY_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;
    //*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xc00646d1;
    //*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xc00646d1;
    #ifdef DRAMC_ASYNC
      *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xa00632f1;    //DLE //Edward:TR2W[15:12]=3h TRTP[10:8]=2h, DATLAT[6:4]=7h
      *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xa00632f1;
    #else
          *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL ;
          *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL ;
    #endif

    *((V_UINT32P)(DRAMC0_BASE + 0x0028)) = 0xf1200f01;
    *((V_UINT32P)(DDRPHY_BASE + 0x0028)) = 0xf1200f01;

    *((V_UINT32P)(DRAMC0_BASE + 0x01e0)) = 0x3001ebff;
    *((V_UINT32P)(DDRPHY_BASE + 0x01e0)) = 0x3001ebff;

    *((V_UINT32P)(DRAMC0_BASE + 0x0158)) = 0xf0f0f0f0;  // Edward: 4 bit swap enable 0xf0f0f0f0. disable 0xff00ff00.
    *((V_UINT32P)(DDRPHY_BASE + 0x0158)) = 0xf0f0f0f0;

    *((V_UINT32P)(DRAMC0_BASE + 0x0400)) = 0x00111100;
    *((V_UINT32P)(DDRPHY_BASE + 0x0400)) = 0x00111100;

    *((V_UINT32P)(DRAMC0_BASE + 0x0404)) = 0x00000002;
    *((V_UINT32P)(DDRPHY_BASE + 0x0404)) = 0x00000002;

    *((V_UINT32P)(DRAMC0_BASE + 0x0408)) = 0x00222222;
    *((V_UINT32P)(DDRPHY_BASE + 0x0408)) = 0x00222222;

    *((V_UINT32P)(DRAMC0_BASE + 0x040c)) = 0x33330000;
    *((V_UINT32P)(DDRPHY_BASE + 0x040c)) = 0x33330000;

    *((V_UINT32P)(DRAMC0_BASE + 0x0410)) = 0x33330000;
    *((V_UINT32P)(DDRPHY_BASE + 0x0410)) = 0x33330000;
   *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x0b052311;          // Edward: Modify XRTR2W[14:12] to 2 & XRTR2R[10:8] to 3
    *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x0b052311;

    //*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000004;
    //*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000004;
    *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL
    *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL

        __asm__ __volatile__ ("dsb" : : : "memory");    // Edward : add this according to 6589.
        gpt_busy_wait_us(200);//Wait > 200us              // Edward : add this according to 6589.

    *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_63;
    *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_63;

    *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(10);//Wait > 10us                              // Edward : add this according to DRAM spec, should wait at least 10us if not checking DAI.

    *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;


    *((V_UINT32P)(DRAMC0_BASE + 0x0110)) &= (~0x7);   // Edward : Add this to disable two rank support for ZQ calibration tempority.
    *((V_UINT32P)(DDRPHY_BASE + 0x0110)) &= (~0x7);


    *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;
    *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;

    *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);            //Wait > 1us. Edward : Add this because tZQINIT min value is 1us.

    *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        // Edward : Add this for dual ranks support.
        if ( *(V_UINT32P)(EMI_CONA)& 0x20000)  {
                //for chip select 1: ZQ calibration
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) |= (0x8);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) |= (0x8);

                *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;
                *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_10;

                *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
                *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

                __asm__ __volatile__ ("dsb" : : : "memory");
                gpt_busy_wait_us(1);//Wait > 100us

                *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
                *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

                //swap back
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) &= (~0x8);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) &= (~0x8);
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) |= (0x1);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) |= (0x1);
        }
       *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_1;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_1;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                        // Edward : 6589 has this delay. Seems no need.

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_2;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_2;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                        // Edward : 6589 has this delay. Seems no need.

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_3;    //Dram Driving : 34.3-ohm  //Edward: modify default driving to 40ohm (1->2)?
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR2_MODE_REG_3;    //Dram Driving : 34.3-ohm

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                // Edward : 6589 has this delay. Seems no need.

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00001100;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

       // Edward : add two rank enable here.
        if ( *(V_UINT32P)(EMI_CONA)& 0x20000) {
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112391;      // Edward: Modify XRTR2W & XRTR2R.
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112391;
        } else {
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112390;
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112390;
        }

        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL

        *((V_UINT32P)(DRAMC0_BASE + 0x01ec)) = 0x00000001;    // Edward : Add this to enable dual scheduler. Should enable this for all DDR type.
        *((V_UINT32P)(DDRPHY_BASE + 0x01ec)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x0084)) = 0x00000a56;
        *((V_UINT32P)(DDRPHY_BASE + 0x0084)) = 0x00000a56;

        *((V_UINT32P)(DRAMC0_BASE + 0x000c)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x000c)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;;
        *((V_UINT32P)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;;

        // Edward : AC_TIME_0.5T control (new for 6582)
        *((V_UINT32P)(DRAMC0_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;   //TWTR_M[26]=1,TFAW[13]=1,TWR_M[10]=1,TRAS[9]=1,TRP[7]=1,TRCD[6]=1,TRC[0]=1, TXP_05T =1

        *((V_UINT32P)(DRAMC0_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;            // Edward : Modify DMPGTIM to 3f.
        *((V_UINT32P)(DDRPHY_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;            // Edward : TRFC=18h, TRFC[19:16]=8h

        *((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //82M need to enable R_DMREFRDIS // Edward : Why need to set [26]R_DMREFRDIS = 1 here? Should not enable according to SY.
        *((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //82M need to enable R_DMREFRDIS
       //333Mhz
        //*((V_UINT32P)(DRAMC0_BASE + 0x0004)) = 0xf00487c3;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0000)) = 0x55d84608;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0x28000401;
        //*((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = 0x00001010;

        *((V_UINT32P)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;

        *((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x00f8)) = 0xedcb000f;
        *((V_UINT32P)(DDRPHY_BASE + 0x00f8)) = 0xedcb000f;

        *((V_UINT32P)(DRAMC0_BASE + 0x0020)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0020)) = 0x00000000;
#ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0x8c6e8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0x8c6e8008<<0);
        #else
            *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfc6f8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc6f8008<<0); 
        #endif
#else
        #ifdef DDRPHY_3PLL_MODE
           // Edward: Add 2PLL.
            #ifdef DDRPHY_2PLL
                  *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfc7e8008<<0);
                  *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc7e8008<<0);
            #else
                *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0x8c7e8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating // Edward: [3] is to save unused DDR3 I/O.
                 *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0x8c7e8008<<0);
            #endif
        #else
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc7f8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc7f8008<<0);
        #endif
#endif

        //*((V_UINT32P)(DRAMC0_BASE + 0x01dc)) = 0x51622842;  //enable DDRPHY dynamic clk gating
        //*((V_UINT32P)(DDRPHY_BASE + 0x01dc)) = 0x51622842;  //enable DDRPHY dynamic clk gating
        *((V_UINT32P)(DRAMC0_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //enable DDRPHY dynamic clk gating & comb // Edward : [23:16]REFCNT_FR_CLK =64h, TXREFCNT[15:8]=23h
        *((V_UINT32P)(DDRPHY_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //enable DDRPHY dynamic clk gating & comb

        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112381;      //dsiable R_DMMRS2RK, this bit enable will impact MRR result // Edward : Modify [14:12] XRTR2W 1->2 [10:8] XRTR2R 1->3
        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112381;      //dsiable R_DMMRS2RK, this bit enable will impact MRR result
        // Before calibration, set default settings to avoid DLE calibration  fail. The following setting is for 1066Mbps.
        // After bring-up or HQA, please set optimized default here.
#if 1
        // GW coarse tune 14 (fh -> 1111b -> 11b, 10b)
        //*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = (*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) & 0xf8ffffff) | (0x03<<24);
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = (*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) & 0xf8ffffff) | (0x03<<24);

        //*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = (*((V_UINT32P)(DRAMC0_BASE + 0x0124)) & 0xffffff00) | 0xaa;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = (*((V_UINT32P)(DDRPHY_BASE + 0x0124)) & 0xffffff00) | 0xaa;

        // GW fine tune 56 (38h)
        //*((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x38383838;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x38383838;

        //*((V_UINT32P)(DRAMC0_BASE + 0x0098)) = 0x38383838;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0098)) = 0x38383838;

        //  DLE 7
//        *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = (*((V_UINT32P)(DRAMC0_BASE + 0x007c)) & 0xFFFFFF8F) | ((6 & 0x07) <<4);
//        *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = (*((V_UINT32P)(DDRPHY_BASE + 0x007c)) & 0xFFFFFF8F) |  ((6 & 0x07) <<4);

//        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = (*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) & 0xFFFFFFEF) | (((6 >> 3) & 0x01) << 4);
//        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) =  (*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) & 0xFFFFFFEF) | (((6 >> 3) & 0x01) << 4);
      // RX DQ, DQS input delay
        *((V_UINT32P)(DRAMC0_BASE + 0x0210)) = 0x05050101;
        *((V_UINT32P)(DDRPHY_BASE + 0x0210)) = 0x05050101;
        *((V_UINT32P)(DRAMC0_BASE + 0x0214)) = 0x00040307;
        *((V_UINT32P)(DDRPHY_BASE + 0x0214)) = 0x00040307;
        *((V_UINT32P)(DRAMC0_BASE + 0x0218)) = 0x01030503;
        *((V_UINT32P)(DDRPHY_BASE + 0x0218)) = 0x01030503;
        *((V_UINT32P)(DRAMC0_BASE + 0x021c)) = 0x03020100;
        *((V_UINT32P)(DDRPHY_BASE + 0x021c)) = 0x03020100;
        *((V_UINT32P)(DRAMC0_BASE + 0x0220)) = 0x05070001;
        *((V_UINT32P)(DDRPHY_BASE + 0x0220)) = 0x05070001;
        *((V_UINT32P)(DRAMC0_BASE + 0x0224)) = 0x03040602;
        *((V_UINT32P)(DDRPHY_BASE + 0x0224)) = 0x03040602;
        *((V_UINT32P)(DRAMC0_BASE + 0x0228)) = 0x03030405;
        *((V_UINT32P)(DDRPHY_BASE + 0x0228)) = 0x03030405;
        *((V_UINT32P)(DRAMC0_BASE + 0x022c)) = 0x04040004;
        *((V_UINT32P)(DDRPHY_BASE + 0x022c)) = 0x04040004;

        *((V_UINT32P)(DRAMC0_BASE + 0x0018)) = 0x1F221E22;
        *((V_UINT32P)(DDRPHY_BASE + 0x0018)) = 0x1F221E22;
        *((V_UINT32P)(DRAMC0_BASE + 0x001c)) = 0x1F221E22;
        *((V_UINT32P)(DDRPHY_BASE + 0x001c)) = 0x1F221E22;

        // TX DQ, DQS output delay
        *((V_UINT32P)(DRAMC0_BASE + 0x0014)) = 0x00008888;   // DQS output delay
        *((V_UINT32P)(DDRPHY_BASE + 0x0014)) = 0x00008888;

        *((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x00008888;   // DQM output delay
        *((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x00008888;

        *((V_UINT32P)(DRAMC0_BASE + 0x0200)) = 0x88888888;   // DQ
        *((V_UINT32P)(DDRPHY_BASE + 0x0200)) = 0x88888888;
        *((V_UINT32P)(DRAMC0_BASE + 0x0204)) = 0x88888888;
        *((V_UINT32P)(DDRPHY_BASE + 0x0204)) = 0x88888888;
        *((V_UINT32P)(DRAMC0_BASE + 0x0208)) = 0x88888888;
        *((V_UINT32P)(DDRPHY_BASE + 0x0208)) = 0x88888888;
        *((V_UINT32P)(DRAMC0_BASE + 0x020c)) = 0x88888888;
        *((V_UINT32P)(DDRPHY_BASE + 0x020c)) = 0x88888888;

#endif
	
}

unsigned int ddr3_1333_enable = 0x00; //1: enable, 0 : disable
#define DRAM_IS_2_RANK_MODE()	(*((volatile unsigned int*)(EMI_CONA)) & 0x20000)
static void init_ddr3(EMI_SETTINGS *emi_setting) 
{
#ifdef DDR3_16B
	print("mt6592 ddr3 x16 init : %d\n", ddr3_1333_enable);
#else
	print("mt6592 ddr3 x32 init : %d\n", ddr3_1333_enable);
#endif

	// The following settings are from Yi-Chih.
	/* TINFO="Star mm_cosim_emi_config" */
	//----------------EMI Setting--------------------
	*((volatile unsigned int *)(EMI_BASE + 0x0060))=0x0000051f;
	*((volatile unsigned int *)(EMI_BASE + 0x0140))=0x20406188;//0x12202488;   // 83 for low latency
	*((volatile unsigned int *)(EMI_BASE + 0x0144))=0x20406188;//0x12202488; //new add
	*((volatile unsigned int *)(EMI_BASE + 0x0100))=0x40105906;//0x40105808; //m0 cpu ori:0x8020783f
	*((volatile unsigned int *)(EMI_BASE + 0x0108))=0x808050ea;//0xa0a05028; //m1 ori:0x80200000 // ultra can over limit
	*((volatile unsigned int *)(EMI_BASE + 0x0110))=0xffff50c4;//0xa0a05001; //m2 arm9
	*((volatile unsigned int *)(EMI_BASE + 0x0118))=0x0810d84a;//0x030fd80d; // ???????????????????????????????????????????????????????????//m3 mdmcu ori:0x80807809 ori:0x07007010 bit[12]:enable bw limiter
	*((volatile unsigned int *)(EMI_BASE + 0x0120))=0x40405042; //m4 fcore ori:0x8080781a
	// *((volatile unsigned int *)(EMI_BASE +0x0128))=0xa0a05001; //m5 peri
	// *((volatile unsigned int *)(EMI_BASE +0x0130))=0xa0a05028; //m6 vcodec ori:0x8080381a
	*((volatile unsigned int *)(EMI_BASE + 0x0148))=0x9719595e;//0323 chg, ori :0x00462f2f
	*((volatile unsigned int *)(EMI_BASE + 0x014c))=0x9719595e; // new add
	//-----------------------------------
	//-- modified by liya for EMI address mapping setting
#ifdef DDR3_16B
	*(volatile unsigned int*)(EMI_CONA) = 0x00003120;			// Edward 4Gb(256bx16) settings (may use  Hynix H5TC4G64AFR), 8 banks/15 rows/10 columns.	
#else
	*(volatile unsigned int*)(EMI_CONA) = emi_setting->EMI_CONA_VAL;//0x00003122;			// Edward: 4Gb(256bx16)x2 settings (may use  Hynix H5TC4G64AFR), 8 banks/15 rows/10 columns.	
#endif
	//-----------------------------------
	*((volatile unsigned int *)(EMI_BASE + 0x00f8))=0x00000000;
	*((volatile unsigned int *)(EMI_BASE + 0x0400))=0x007f0001;
	if(ddr3_1333_enable)
	{//1333MHz, default use
	*((volatile unsigned int *)(EMI_BASE   +0x0008))=0x0b1b2737;// Chuan:0x181e1e2b Edwin:0x12161620
	*((volatile unsigned int *)(EMI_BASE   +0x0010))=0x08170717;// Chuan:0x18181818 Edwin:0x12121212
	*((volatile unsigned int *)(EMI_BASE   +0x0018))=0x0b1b2737;// Chuan:0x1e262b33 Edwin:0x161c2026
	*((volatile unsigned int *)(EMI_BASE   +0x0020))=0x08170717;// Chuan:0x1818181b Edwin:0x12121214
	*((volatile unsigned int *)(EMI_BASE   +0x0030))=0x2828272b;
	*((volatile unsigned int *)(EMI_BASE   +0x0038))=0x2828272b;
	}
	else
	{//1066MHz, bringup use
	*((volatile unsigned int *)(EMI_BASE   +0x0008))=0x05121624;// Chuan:0x181e1e2b Edwin:0x12161620
	*((volatile unsigned int *)(EMI_BASE   +0x0010))=0x04070506;// Chuan:0x18181818 Edwin:0x12121212
	*((volatile unsigned int *)(EMI_BASE   +0x0018))=0x05121624;// Chuan:0x1e262b33 Edwin:0x161c2026
	*((volatile unsigned int *)(EMI_BASE   +0x0020))=0x04070506;// Chuan:0x1818181b Edwin:0x12121214
	*((volatile unsigned int *)(EMI_BASE   +0x0030))=0x27272625;
	*((volatile unsigned int *)(EMI_BASE   +0x0038))=0x27272625;
	}
	*((volatile unsigned int *)(EMI_BASE + 0x0158))=0x00010800;// ???????????????????????0x08090800; 
	*((volatile unsigned int *)(EMI_BASE + 0x0150))=0x6470fc79;
	*((volatile unsigned int *)(EMI_BASE + 0x0154))=0x6470fc79;
	//============== Defer WR threthold
	*((volatile unsigned int *)(EMI_BASE + 0x00f0))=0x38470000;
	//============== Reserve bufer
	//
	//MDMCU don't always ULTRA, but small age
	*((volatile unsigned int *)(EMI_BASE   +0x0078))=0x23110c03;// ???????????0x00030F4d; 
	// Turn on M1 Ultra and all port DRAMC hi enable
	*((volatile unsigned int *)(EMI_BASE + 0x0158))=0x1f011f00;// ???????????????????????0x08090800; 
	// RFF)_PBC_MASK
#ifdef DDR3_16B
	*((volatile unsigned int *)(EMI_BASE + 0x00e8))=0x00000004;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
#else    
	*((volatile unsigned int *)(EMI_BASE + 0x00e8))= emi_setting->PCDDR3_MODE_REG4;//0x00000024; //Edward : 92 PCDDR3O32 bit ,n}32byte_wrap mode(0xe8[5])
#endif    
	// Page hit is high
	*((volatile unsigned int *)(EMI_BASE + 0x0060))=0x0000051f;
	*((volatile unsigned int *)(EMI_BASE + 0x00d0))=0x84848c84;//R/8 W/8 outstanding
	*((volatile unsigned int *)(EMI_BASE + 0x00d8))=0x00480084;//R/8 W/8 outstanding
	//===========END===========================================
#ifdef DRAMC_ASYNC
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07900000;   //ASYNC  //Edward: TXP[30:28]=0, PBC_ARB_E1T[23]=1 & MODE18V[16]=0.
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07900000;   //ASYNC
#else
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07800000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00FC)) = emi_setting->DRAMC_MISCTL0_VAL;//0x07800000;
#endif
    //default cross-talk pattern
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0048) = 0x1e01110d;    // Edward: tZQCS[31:24]=0x1e
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0048) = 0x1e01110d;      // cross-talk pattern  

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F0) = 0xC0000000;   //PINMUX2 ==> 1 // Edward: DQ4BMUX[31]=1  [30]S2=1,
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F0) = 0xC0000000;
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00D8) = 0x80100900;   //PINMUX(1,0) ==> (0,1) // Edward : 101b for DDR3x8x4, 110b for DDR3x16x2, 111b for DDR3x32
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00D8) = 0x80100900;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00E4) = 0x000000a3;	//CKEBYCTL
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00E4) = 0x000000a3;
	// Edward : After reset enable, need 500us before CKE high. (Reset low needs 200us.)
	 __asm__ __volatile__ ("dsb" : : : "memory");
	gpt_busy_wait_us(500);

	*(volatile unsigned int *)(DRAMC0_BASE + 0x008C) = 0x00000000;    //[20]DQ16COM1=0, [3]DQCMD=0, WTLATC=000, only DDR3 use 
	*(volatile unsigned int *)(DDRPHY_BASE + 0x008C) = 0x00000000;    //[20]DQ16COM1=0, [3]DQCMD=0, WTLATC=000, only DDR3 use

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0090) = 0x00000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0090) = 0x00000000;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0094) = 0x80000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0094) = 0x80000000;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00DC) = 0x83080080;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00DC) = 0x83080080;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00E0) = 0x13080080;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00E0) = 0x13080080;

#ifdef DDR3_16B
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F0) = 0xC0000000;   //pinmux2=1   // Edward: DQ4BMUX[31]=1  [30]S2=1,
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F0) = 0xC0000000;   //pinmux2=1
#else
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F0) = 0xC0000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F0) = 0xC0000000;
#endif //DDR3_16B
	
	*(volatile unsigned int *)(DRAMC0_BASE + 0x00F4) = emi_setting->DRAMC_GDDR3CTL1_VAL;//0x01000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00F4) = emi_setting->DRAMC_GDDR3CTL1_VAL;//0x01000000;
	
    #ifdef PHYSYNC_MODE
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00F4)) |= 0x10000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00F4)) |= 0x10000000;    
    #endif

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0168) = 0x00000080;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0168) = 0x00000080;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0130) = 0x30000000;	 //dram_clk_en0 & 1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0130) = 0x30000000;	 //dram_clk_en0 & 1
	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);									// Edward : add this because 6589 has this.

	*(volatile unsigned int *)(DRAMC0_BASE + 0x00D8) = 0x80300900;   //PINMUX (1,0) ==> (0,1)  // Edward : 101b for DDR3x8x4, 110b for DDR3x16x2, 111b for DDR3x32
	*(volatile unsigned int *)(DDRPHY_BASE + 0x00D8) = 0x80300900;

#ifdef DDR3_16B
	if(ddr3_1333_enable)
	{//1333MHz, default use
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0004) = 0xf0748662;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0004) = 0xf0748662;	 // Edward : MATYPE[9:8] set to 0 for safe.
	}
	else
	{//1066MHz, bringup use
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0004) = 0xf07486a2;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0004) = 0xf07486a2;   // Edward : MATYPE[9:8] set to 0 for safe.
	}
#else
	if(ddr3_1333_enable)
	{//1333MHz, default use
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0004) = emi_setting->DRAMC_CONF1_VAL;//0xf0748663;	//DM64BIT=1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0004) = emi_setting->DRAMC_CONF1_VAL;//0xf0748663;
	}
	else
	{//1066MHz, bringup use
	*(volatile unsigned int *)(DRAMC0_BASE + 0x0004) = 0xf07486a3;  //DM64BIT=1
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0004) = 0xf07486a3;
	}
#endif

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0124) = 0x80000011;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0124) = 0x80000011;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0094) = 0x40404040;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0094) = 0x40404040;
	
	*(volatile unsigned int *)(DRAMC0_BASE + 0x01C0) = 0x00000000;    // Edward: Should not enable HW gating & GW limit here.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x01C0) = 0x00000000;	
	
	if(ddr3_1333_enable)
	{//1333MHz, default use
	*(volatile unsigned int *)(DRAMC0_BASE + 0x007C) = emi_setting->DRAMC_DDR2CTL_VAL;//0xa18e21c1;	  // Edward: TR2W[15:12]=3->1. Disable WOEN and ROEN.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x007C) = emi_setting->DRAMC_DDR2CTL_VAL;//0xa18e21c1;
	}
	else
	{//1066MHz, bringup use
	*(volatile unsigned int *)(DRAMC0_BASE + 0x007C) = 0x918e21c1;    // Edward: TR2W[15:12]=3->1. Disable WOEN and ROEN.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x007C) = 0x918e21c1;
	}

#ifdef DDR3_16B
	*(volatile unsigned int *)(DRAMC0_BASE + 0x008C) = 0x00100008;    //[20]DQ16COM1=1, [3]dqcmd=1, WTLATC=000, only pinmux1 use  // Edward : Only in 6582 case. In 6592, DDR3 does not use this setting.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x008C) = 0x00100008;	  //[20]DQ16COM1=1, [3]dqcmd=1, WTLATC=000, only pinmux1 use
#endif

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0028) = 0xF1200f01;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0028) = 0xF1200f01;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x0158) = 0xf0f0f0f0;	//Edward: 4 bits swap 0xf0f0f0f0, disable 0xff00ff00.
	*(volatile unsigned int *)(DDRPHY_BASE + 0x0158) = 0xf0f0f0f0;

	*(volatile unsigned int *)(DRAMC0_BASE + 0x01E0) = 0x08000000;
	*(volatile unsigned int *)(DDRPHY_BASE + 0x01E0) = 0x08000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x00E4)) = 0x000000a7;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00E4)) = 0x000000a7;
	//-------------------------------------------------------------
	#ifdef REXTDN_ENABLE
	drvp = drvp + 2;
	drvn = drvn + 2;
	if(drvp > 0x0F) drvp = 0x0F;
	if(drvn > 0x0F) drvn = 0x0F;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00BC)) = 0x00000000 | ((drvp<<28) + (drvn<<24) + ((drvp - 2)<<12) + ((drvn - 2)<<8));
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00BC)) = 0x00000000 | ((drvp<<28) + (drvn<<24) + ((drvp - 2)<<12) + ((drvn - 2)<<8));
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00B8)) = 0x00000000 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00B8)) = 0x00000000 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00B4)) = 0x00000000 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00B4)) = 0x00000000 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
	#else
	//CLK, CMD driving
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00BC)) = emi_setting->DRAMC_DRVCTL1_VAL;//0xaa00aa00;//0xaa00aa00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00BC)) = emi_setting->DRAMC_DRVCTL1_VAL;//0xaa00aa00;//0xaa00aa00;
	
	//DQS driving
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00B8)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;//0xaa00aa00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00B8)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;//0xaa00aa00;
	
	//DQ driving
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00B4)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;//0xaa00aa00;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00B4)) = emi_setting->DRAMC_DRVCTL0_VAL;//0xaa00aa00;//0xaa00aa00;
	#endif	
	
	//Pre-emphassis enable
	//*((volatile unsigned int *)(DRAMC0_BASE + 0x0640)) |= 0x3F00;
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x0640)) |= 0x3F00;
	
	//CLK delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x000C)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x000C)) = 0x00000000;	

	//CMD/ADDR delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01A8)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01A8)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01AC)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01AC)) = 0x00000000;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01B0)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01B0)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01B4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01B4)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01B8)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01B8)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01BC)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01BC)) = 0x00000000;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01C0)) &= 0xE0E0FFFF;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01C0)) &= 0xE0E0FFFF;
	delay_a_while(1000);
	
	if(!DRAM_IS_2_RANK_MODE())
	{//single rank mode
		*((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051100;
		*((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051100;
	}
	else
	{//dual rank mode
		*((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051111;
		*((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051111;
	}
	//-------------------------------------------------------------	
	//MRS2 -> MRS3 -> MRS1 - > MRS0
	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(2000);				// Edward : add this because 6589 has this. 

	if(ddr3_1333_enable)
	{//1333MHz, default use
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG2;//0x00004010;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG2;//0x00004010;
	}
	else
	{//1066MHz, bringup use
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x00004008;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x00004008;
	}

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG3;//0x00006000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG3;//0x00006000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG1;//0x00002000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG1;//0x00002000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
	
	if(ddr3_1333_enable)
	{//1333MHz, default use
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG0;//0x00001B51;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->PCDDR3_MODE_REG0;//0x00001B51;
	}
	else
	{//1066MHz, bringup use
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x00001941;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x00001941;
	}

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
	//-------------------------------------------------------------	
	//ZQ calibration
	#if 0	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x00000400;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x00000400;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000010;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000010;

	 __asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

	#else
	//for chip select 0: ZQ calibration
	*((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x7);
	*((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x7);
	
	*((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = 0x00000400;
	*((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = 0x00000400;
	
	*((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000010;
	*((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000010;
	
	__asm__ __volatile__ ("dsb" : : : "memory");
	delay_a_while(1000);
	
	*((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
	if (DRAM_IS_2_RANK_MODE())
	{
		//for chip select 1: ZQ calibration
		*((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x8);
		*((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x8);

		*((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = 0x00000400;
		*((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = 0x00000400;

		*((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000010;
		*((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000010;

		__asm__ __volatile__ ("dsb" : : : "memory");
		delay_a_while(1000);
		*((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
		*((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
		//swap back
		*((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x8);
		*((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x8);
		*((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x1);
		*((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x1);
	}

	#endif
	__asm__ __volatile__ ("dsb" : : : "memory");
	 delay_a_while(1000);		// Edward : Add this for ZQinit=Max(512nCK, 640ns) .
	//-------------------------------------------------------------	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00001100;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x00E4)) = 0x000007a3;  //CKEBYCTL
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00E4)) = 0x000007a3;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01ec)) = 0x00000001;  // Edward : Add this to enable dual scheduler according to Cl and SY.
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01ec)) = 0x00000001;    

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E0)) = 0x08000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E0)) = 0x08000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x0000ffff;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x0000ffff;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E4)) = 0x00000020;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E4)) = 0x00000020;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E4)) = 0x00000000;
		
	if(ddr3_1333_enable)
	{//1333MHz, default use
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;//0x337A4684;   // Edward: TFAW[23:20]=6->5h
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;//0x337A4684;
	
	// Edward : AC_TIME_0.5T control (new for 6582)
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;//0x040014E1;  //TWTR_M[26]=1, TRTW[24]=1, TFAW[13]=1
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;//0x040014E1;   
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf890401;  
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;//0xbf890401;  // Edward: : TRFC=45h, TRFC[19:16]=5h
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E8)) = emi_setting->DRAMC_ACTIM1_VAL;//0x00000650;  // Edward: TRFC_BIT7_4[7:4]=4h
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E8)) = emi_setting->DRAMC_ACTIM1_VAL;//0x00000650;
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;//0x03046978;  // Edward: REDFCNT=41h. But not use.
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;//0x03046978;
	}
	else
	{//1066MHz, bringup use
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0000)) = 0x33584562;   // Edward: TFAW[23:20]=6->5h
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0000)) = 0x33584562;

	// Edward : AC_TIME_0.5T control (new for 6582)
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01f8)) = 0x05002000;  //TWTR_M[26]=1, TRTW[24]=1, TFAW[13]=1
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01f8)) = 0x05002000;   

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0044)) = 0xbf850401;  
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0044)) = 0xbf850401;  // Edward: : TRFC=45h, TRFC[19:16]=5h

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01E8)) = 0x00000640;  // Edward: TRFC_BIT7_4[7:4]=4h
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01E8)) = 0x00000640;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0008)) = 0x0304693f;  // Edward: REDFCNT=41h. But not use.
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0008)) = 0x0304693f;
	}
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0010)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0010)) = 0x00000000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x00F8)) = 0xedcb000f;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00F8)) = 0xedcb000f;

	//*((volatile unsigned int *)(DRAMC0_BASE + 0x00FC)) = 0x27010000;
	//*((volatile unsigned int *)(DDRPHY_BASE + 0x00FC)) = 0x27010000;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x01D8)) = 0x00c80008;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01D8)) = 0x00c80008;

    // Edward : Added according to Cl comment for power saving.
    #ifdef DDRPHY_3PLL_MODE
        #ifdef DDRPHY_2PLL
              *((volatile unsigned int *)(DRAMC0_BASE + 0x0640)) |= (0xfc7e8000<<0);  
        *((volatile unsigned int *)(DDRPHY_BASE + 0x0640)) |= (0xfc7e8000<<0);		
        #else
         	*((volatile unsigned int *)(DRAMC0_BASE + 0x0640)) |= (0x8c7e8000<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating // Edward: [3] is to save unused DDR3 I/O. how about other bits?
		*((volatile unsigned int *)(DDRPHY_BASE + 0x0640)) |= (0x8c7e8000<<0);		//enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating // Edward: [3] is to save unused DDR3 I/O. how about other bits?
        #endif             	
    #else
        *((volatile unsigned int *)(DRAMC0_BASE + 0x0640)) |= (0xfc7f8000<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
        *((volatile unsigned int *)(DDRPHY_BASE + 0x0640)) |= (0xfc7f8000<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
    #endif
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;//0xd1972842;    //Edward : need to enable DDRPHY dynamic clk gating and FR.
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;//0xd1972842;    // REFFRERUN[24]=1, REFCNT_FR_CLK[23:16]=64h, TXREFCNT[15:8]=NA (Fix by HW)

	#if 1
	// Before calibration, set default settings to avoid some problems happened during calibration.
	// GW coarse tune 13 (Dh -> 01101b -> 011b, 01b) ()
	*((volatile unsigned int *)(DRAMC0_BASE + 0x00e0)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x00e0)) & 0xf8ffffff) | (0x03<<24);
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00e0)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x00e0)) & 0xf8ffffff) | (0x03<<24);
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0124)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x0124)) & 0xffffff00) | 0x11;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0124)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x0124)) & 0xffffff00) | 0x11;

	// GW fine tune 56 (38h)
	//rank0 fine tune
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0094)) = 0x38383838;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0094)) = 0x38383838;	
	//rank1 fine tune
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0098)) = 0x38383838;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0098)) = 0x38383838;		

	//DLE 6
	*((volatile unsigned int *)(DRAMC0_BASE + 0x007c)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x007c)) & 0xFFFFFF8F) | ((6 & 0x07) <<4);
	*((volatile unsigned int *)(DDRPHY_BASE + 0x007c)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x007c)) & 0xFFFFFF8F) | ((6 & 0x07) <<4);

	*((volatile unsigned int *)(DRAMC0_BASE + 0x00e4)) = (*((volatile unsigned int *)(DRAMC0_BASE + 0x00e4)) & 0xFFFFFFEF) | (((6 >> 3) & 0x01) << 4);
	*((volatile unsigned int *)(DDRPHY_BASE + 0x00e4)) = (*((volatile unsigned int *)(DDRPHY_BASE + 0x00e4)) & 0xFFFFFFEF) | (((6 >> 3) & 0x01) << 4);

	// RX DQ, DQS input delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0018)) = 0x1B1B1B1B;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0018)) = 0x1B1B1B1B;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x001c)) = 0x1B1B1B1B;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x001c)) = 0x1B1B1B1B;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x0210)) = 0x05030002;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0210)) = 0x05030002;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0214)) = 0x00010104;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0214)) = 0x00010104;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0218)) = 0x00020202;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0218)) = 0x00020202;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x021c)) = 0x03030003;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x021c)) = 0x03030003;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0220)) = 0x02020000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0220)) = 0x02020000;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0224)) = 0x01030202;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0224)) = 0x01030202;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0228)) = 0x01030502;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0228)) = 0x01030502;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x022c)) = 0x04030002;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x022c)) = 0x04030002;	
	
	// TX DQ, DQS output delay
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;	
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0010)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0010)) = emi_setting->DRAMC_PADCTL3_VAL;//0x00008888;	
	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0200)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0200)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0204)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0204)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0208)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0208)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DRAMC0_BASE + 0x020c)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;	
	*((volatile unsigned int *)(DDRPHY_BASE + 0x020c)) = emi_setting->DRAMC_DQODLY_VAL;//0x88888888;
	
	#endif
	
	print("mt6592 ddr3 init finish!\n");
	
}

static void init_lpddr3(EMI_SETTINGS *emi_setting) {
        // The following settings are from Yi-Chih.
        /* TINFO="Star mm_cosim_emi_config" */
        //----------------EMI Setting--------------------
        *((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
        *((V_UINT32P)(EMI_BASE   +0x0140))=0x10203278;//0x12202488;   // 83 for low latency
        *((V_UINT32P)(EMI_BASE   +0x0144))=0x10203278;//0x12202488; //new add
        *((V_UINT32P)(EMI_BASE   +0x0100))=0x40105806;//0x40105808; //m0 cpu ori:0x8020783f
        *((V_UINT32P)(EMI_BASE   +0x0108))=0x808050ea;//0xa0a05028; //m1 ori:0x80200000 // ultra can over limit
        *((V_UINT32P)(EMI_BASE   +0x0110))=0xffff50c4;//0xa0a05001; //m2 arm9
        *((V_UINT32P)(EMI_BASE   +0x0118))=0x0810d84a;//0x030fd80d; // ???????????????????????????????????????????????????????????//m3 mdmcu ori:0x80807809 ori:0x07007010 bit[12]:enable bw limiter
        *((V_UINT32P)(EMI_BASE   +0x0120))=0x40405042; //m4 fcore ori:0x8080781a
        // *((V_UINT32P)(EMI_BASE   +0x0128))=0xa0a05001; //m5 peri
        // *((V_UINT32P)(EMI_BASE   +0x0130))=0xa0a05028; //m6 vcodec ori:0x8080381a
        *((V_UINT32P)(EMI_BASE   +0x0148))=0x9719595e;//0323 chg, ori :0x00462f2f
        *((V_UINT32P)(EMI_BASE   +0x014c))=0x9719595e; // new add
        //-----------------------------------
        //-- modified by liya for EMI address mapping setting
        //*EMI_CONA = 0x31303132;
        //*EMI_CONA = 0x0002A3AE;               // Edward : Use 8Gb (2 ranks settings)
        *EMI_CONA = emi_setting->EMI_CONA_VAL;         // Edward : Use 8Gb (2 ranks settings)
        //*((V_UINT32P)(EMI_BASE   +0x0000))=0x1130112e;
        //-----------------------------------
        *((V_UINT32P)(EMI_BASE   +0x00f8))=0x00000000;
        *((V_UINT32P)(EMI_BASE   +0x0400))=0x007f0001;

        *((V_UINT32P)(EMI_BASE   +0x0008))=0x0b1b2737;// Chuan:0x181e1e2b Edwin:0x12161620
        *((V_UINT32P)(EMI_BASE   +0x0010))=0x08170717;// Chuan:0x18181818 Edwin:0x12121212
        *((V_UINT32P)(EMI_BASE   +0x0018))=0x0b1b2737;// Chuan:0x1e262b33 Edwin:0x161c2026
        *((V_UINT32P)(EMI_BASE   +0x0020))=0x08170717;// Chuan:0x1818181b Edwin:0x12121214
        *((V_UINT32P)(EMI_BASE   +0x0030))=0x2828272b;
        *((V_UINT32P)(EMI_BASE   +0x0038))=0x2828272b;

        *((V_UINT32P)(EMI_BASE   +0x0158))=0x00010800;// ???????????????????????0x08090800;
        *((V_UINT32P)(EMI_BASE   +0x0150))=0x6470fc79;
        *((V_UINT32P)(EMI_BASE   +0x0154))=0x6470fc79;
        *((V_UINT32P)(EMI_BASE   +0x00e8))=0x00000100;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
        //============== Defer WR threthold
        *((V_UINT32P)(EMI_BASE   +0x00f0))=0x38470000;
        //============== Reserve bufer
        //
        //MDMCU don't always ULTRA, but small age
//        *((V_UINT32P)(EMI_BASE   +0x0078))=0x23110003;// ???????????0x00030F4d;
        *((V_UINT32P)(EMI_BASE   +0x0078))=0x23110C03; 
       // Turn on M1 Ultra and all port DRAMC hi enable
        *((V_UINT32P)(EMI_BASE   +0x0158))=0x1f011f00;// ???????????????????????0x08090800;
        // RFF)_PBC_MASK
        *((V_UINT32P)(EMI_BASE   +0x00e8))=0x00001024;//0x00070714; // EMI_TESTB[4]=1 -> pre-ultra= 1/2 starvation
        // Page hit is high
        *((V_UINT32P)(EMI_BASE   +0x0060))=0x0000051f;
        *((V_UINT32P)(EMI_BASE   +0x00d0))=0x84848c84;//R/8 W/8 outstanding
        *((V_UINT32P)(EMI_BASE   +0x00d8))=0x00480084;//R/8 W/8 outstanding
        //===========END===========================================

#ifdef REXTDN_ENABLE
        *((V_UINT32P)(DRAMC0_BASE + 0x00b4)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DDRPHY_BASE + 0x00b4)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DRAMC0_BASE + 0x00b8)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DDRPHY_BASE + 0x00b8)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DRAMC0_BASE + 0x00bc)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) = 0x00220022 | ((drvp<<28) + (drvn<<24) + (drvp<<12) + (drvn<<8));
#else
        // TX driving 0x0a
        *((V_UINT32P)(DRAMC0_BASE + 0x00b4)) = 0xAA22AA22;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
        *((V_UINT32P)(DDRPHY_BASE + 0x00b4)) = 0xAA22AA22;
        *((V_UINT32P)(DRAMC0_BASE + 0x00b8)) = 0xAA22AA22;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
        *((V_UINT32P)(DDRPHY_BASE + 0x00b8)) = 0xAA22AA22;
        *((V_UINT32P)(DRAMC0_BASE + 0x00bc)) = 0xAA22AA22;   //[31:28]CLK DRVP/[27:24]CLK DRVN;[15:12]CMD DRVP/[11:8]CMD DRVN;, default: 0xaa22aa22
        *((V_UINT32P)(DDRPHY_BASE + 0x00bc)) = 0xAA22AA22;
#endif
        // Pre-emphasis ON. Should be set after MEMPLL init.
        *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= 0x00003f00;
        *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= 0x00003f00;

        #ifdef DRAMC_ASYNC
            *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) =  emi_setting->DRAMC_MISCTL0_VAL | (1<<20);
            *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) =  emi_setting->DRAMC_MISCTL0_VAL | (1<<20);
 
        #else
            *((V_UINT32P)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
            *((V_UINT32P)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
        #endif

        *((V_UINT32P)(DRAMC0_BASE + 0x0048)) = 0x0001110d;
        *((V_UINT32P)(DDRPHY_BASE + 0x0048)) = 0x0001110d;

        *((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x00500900; //Edward: 100b for LPDDR2/3, 110b for DDR3x16x2
        *((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x00500900;

        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x008c)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x008c)) = 0x00000001;

        *((V_UINT32P)(DRAMC0_BASE + 0x0090)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0090)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x80000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x80000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x00dc)) = 0x83004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00dc)) = 0x83004004;

        //*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x19004004;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x19004004;
#ifdef SYSTEM_26M //Edwin
        *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080022;
        *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080022;
#else
        *((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0124)) = 0xaa080033;
        *((V_UINT32P)(DDRPHY_BASE + 0x0124)) = 0xaa080033;
#endif

        *((V_UINT32P)(DRAMC0_BASE + 0x00f0)) = 0xC0000000; // Edward: DQ4BMUX=1 for SBS. [30]S2=1, Edward : 100b for LPDDR2/3, 110b for DDR3x16x2
        *((V_UINT32P)(DDRPHY_BASE + 0x00f0)) = 0xC0000000;

        //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01000000;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01000000;
    #ifdef DRAMC_ASYNC
        //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x11100000;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x11100000;
         *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;      // PHYSYNCM, default to 0
        *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #else
        //*((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = 0x01100000;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = 0x01100000;
         *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #endif

    #ifdef PHYSYNC_MODE
        *((V_UINT32P)(DRAMC0_BASE + 0x00f4)) |= 0x10000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x00f4)) |= 0x10000000;
    #endif
        *((V_UINT32P)(DRAMC0_BASE + 0x0168)) = 0x00000080;
        *((V_UINT32P)(DDRPHY_BASE + 0x0168)) = 0x00000080;

        *((V_UINT32P)(DRAMC0_BASE + 0x00d8)) = 0x00700900;   // Edward : 100b for LPDDR2/3, 110b for DDR3x16x2
        *((V_UINT32P)(DDRPHY_BASE + 0x00d8)) = 0x00700900;

        *((V_UINT32P)(DRAMC0_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL; // Edward: MATYPE[8:9]=0 for safe.
        *((V_UINT32P)(DDRPHY_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;

        //*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = 0xc00646d1;
        //*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = 0xc00646d1;
            #ifdef DRAMC_ASYNC
              *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL | 0x1<<5;
              *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL | 0x1<<5;
            #else
              *((V_UINT32P)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;
              *((V_UINT32P)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;

    #endif

        *((V_UINT32P)(DRAMC0_BASE + 0x0028)) = 0xf1200f01;
        *((V_UINT32P)(DDRPHY_BASE + 0x0028)) = 0xf1200f01;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e0)) = 0x2001ebff;    //LPDDR2EN set to 0
        *((V_UINT32P)(DDRPHY_BASE + 0x01e0)) = 0x2001ebff;    //LPDDR2EN set to 0

        *((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //LPDDR3EN set to 1
        *((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //LPDDR3EN set to 1

        *((V_UINT32P)(DRAMC0_BASE + 0x0158)) = 0xf0f0f0f0;      // Edward: 4 bit swap enable 0xf0f0f0f0, disable 0xff00ff00
        *((V_UINT32P)(DDRPHY_BASE + 0x0158)) = 0xf0f0f0f0;

        *((V_UINT32P)(DRAMC0_BASE + 0x0400)) = 0x00111100;
        *((V_UINT32P)(DDRPHY_BASE + 0x0400)) = 0x00111100;

        *((V_UINT32P)(DRAMC0_BASE + 0x0404)) = 0x00000002;
        *((V_UINT32P)(DDRPHY_BASE + 0x0404)) = 0x00000002;

        *((V_UINT32P)(DRAMC0_BASE + 0x0408)) = 0x00222222;
        *((V_UINT32P)(DDRPHY_BASE + 0x0408)) = 0x00222222;

        *((V_UINT32P)(DRAMC0_BASE + 0x040c)) = 0x33330000;
        *((V_UINT32P)(DDRPHY_BASE + 0x040c)) = 0x33330000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0410)) = 0x33330000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0410)) = 0x33330000;
        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x0b052311;              //Edward: cross rank timing.
        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x0b052311;
        //*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000004;
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000004;
        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL

        __asm__ __volatile__ ("dsb" : : : "memory");    // Edward : add this according to 6589.
        gpt_busy_wait_us(200);//Wait > 200us              // Edward : add this according to 6589.

        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_63;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_63;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(10);//Wait > 10us                              // Edward : add this according to DRAM spec, should wait at least 10us if not checking DAI.

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) &= (~0x7); // Edward : Add this to disable  two ranks support for ZQ calibration tempority.
        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) &= (~0x7);

        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);            //Wait > 1us. Edward : Add this because tZQINIT min value is 1us.

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        // Edward : Add this for dual ranks support.
        if ( *(V_UINT32P)(EMI_CONA)& 0x20000)  {
                //for chip select 1: ZQ calibration
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) |= (0x8);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) |= (0x8);

                *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;
                *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_10;

                *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
                *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

                __asm__ __volatile__ ("dsb" : : : "memory");
              gpt_busy_wait_us(1);//Wait > 1us

                *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
                *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

                //swap back
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) &= (~0x8);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) &= (~0x8);
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) |= (0x1);
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) |= (0x1);
        }

        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_1;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_1;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                        // Edward : 6589 has this delay. Seems no need.

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_2;
        *((V_UINT32P)(DDRPHY_BASE + 0x0088)) = emi_setting->LPDDR3_MODE_REG_2;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

         __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us                        // Edward : 6589 has this delay. Seems no need.
        *((V_UINT32P)(DRAMC0_BASE + 0x01e4)) = 0x00001100;
        *((V_UINT32P)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

        // Edward : add two rank enable here.
        if ( *(V_UINT32P)(EMI_CONA)& 0x20000)  {
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112391;
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112391;
        } else {
                *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112390;
                *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112390;
        }

        *((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
        *((V_UINT32P)(DDRPHY_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
        *((V_UINT32P)(DRAMC0_BASE + 0x01ec)) = 0x00000001;    // Edward : Add this to enable dual scheduler according to CC Wen. Should enable this for all DDR type.
        *((V_UINT32P)(DDRPHY_BASE + 0x01ec)) = 0x00000001;
        *((V_UINT32P)(DRAMC0_BASE + 0x0084)) = 0x00000a56;
        *((V_UINT32P)(DDRPHY_BASE + 0x0084)) = 0x00000a56;

        *((V_UINT32P)(DRAMC0_BASE + 0x000c)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x000c)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL; // Edward : tRAS[3:0] 3h, TRC[7:4] ah, TWTR[11:8] 4h,  TWR[19:16] 8h,
        *((V_UINT32P)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;  // Edward : TFAW[23:20] 5h, TRP[27:24] 5h, TRCD[31:28] 5h

        // Edward : AC_TIME_0.5T control (new for 6582)
        *((V_UINT32P)(DRAMC0_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x01f8)) = emi_setting->DRAMC_ACTIM05T_VAL;   //TWTR_M[26]=1,TFAW[13]=1,TWR_M[10]=1,TRAS[9]=1,TRP[7]=1,TRCD[6]=1,TRC[0]=1

        *((V_UINT32P)(DRAMC0_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;
        *((V_UINT32P)(DDRPHY_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;

        *((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //LPDDR3EN set to 1
        *((V_UINT32P)(DDRPHY_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;    //LPDDR3EN set to 1

        //333Mhz
        //*((V_UINT32P)(DRAMC0_BASE + 0x0004)) = 0xf00487c3;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0000)) = 0x55d84608;
        //*((V_UINT32P)(DRAMC0_BASE + 0x0044)) = 0x28000401;
        //*((V_UINT32P)(DRAMC0_BASE + 0x01e8)) = 0x00001010;

        *((V_UINT32P)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;   // REFCNT[7:0]=41h although not use this. Instead, use FR clock.
        *((V_UINT32P)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;

        *((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x00000000;

        *((V_UINT32P)(DRAMC0_BASE + 0x00f8)) = 0xedcb000f;
        *((V_UINT32P)(DDRPHY_BASE + 0x00f8)) = 0xedcb000f;

        *((V_UINT32P)(DRAMC0_BASE + 0x0020)) = 0x00000000;
        *((V_UINT32P)(DDRPHY_BASE + 0x0020)) = 0x00000000;

        // Edward: The following code will overwrite PLL code setting?
        //*((V_UINT32P)(DRAMC0_BASE + 0x0640)) = 0xffc00033;    //enable DDRPHY dynamic clk gating
        //*((V_UINT32P)(DDRPHY_BASE + 0x0640)) = 0xffc00033;    //enable DDRPHY dynamic clk gating
        // *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= 0xffc00008;    //enable DDRPHY dynamic clk gating
        //*((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= 0xffc00008;    //enable DDRPHY dynamic clk gating

#ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0x8c6e8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0x8c6e8008<<0);
        #else
            *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfc6f8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc6f8008<<0);
        #endif
#else
        #ifdef DDRPHY_3PLL_MODE
            #ifdef DDRPHY_2PLL
                  *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfc7e8008<<0);
                  *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc7e8008<<0);
            #else
                *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0x8c7e8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
                *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0x8c7e8008<<0);
            #endif
        #else
            *((V_UINT32P)(DRAMC0_BASE + 0x0640)) |= (0xfc7f8008<<0);      //enable DDRPHY dynamic clk gating, // disable C_PHY_M_CK dynamic gating
            *((V_UINT32P)(DDRPHY_BASE + 0x0640)) |= (0xfc7f8008<<0);
        #endif
#endif


        *((V_UINT32P)(DRAMC0_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //enable DDRPHY dynamic clk gating
        *((V_UINT32P)(DDRPHY_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //enable DDRPHY dynamic clk gating

        *((V_UINT32P)(DRAMC0_BASE + 0x0110)) = 0x00112381;      //dsiable R_DMMRS2RK, this bit enable will impact MRR result // Edward : Modify [14:12] XRTR2W 1->2 [10:8] XRTR2R 1->3
        *((V_UINT32P)(DDRPHY_BASE + 0x0110)) = 0x00112381;      //dsiable R_DMMRS2RK, this bit enable will impact MRR result

#if 1
        // Before calibration, set default settings to avoid DLE calibration  fail. The following setting is for 1066Mbps.
        // After bring-up or HQA, please set optimized default here.

        // GW coarse tune 18/12h (fh -> 10010b -> 100b, 10b)
        //*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) = (*((V_UINT32P)(DRAMC0_BASE + 0x00e0)) & 0xf8ffffff) | (0x04<<24);
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) = (*((V_UINT32P)(DDRPHY_BASE + 0x00e0)) & 0xf8ffffff) | (0x04<<24);

        //*((V_UINT32P)(DRAMC0_BASE + 0x0124)) = (*((V_UINT32P)(DRAMC0_BASE + 0x0124)) & 0xffffff00) | 0xaa;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0124)) = (*((V_UINT32P)(DDRPHY_BASE + 0x0124)) & 0xffffff00) | 0xaa;

        // GW fine tune 56 (38h)
        //*((V_UINT32P)(DRAMC0_BASE + 0x0094)) = 0x28282828;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0094)) = 0x28282828;

        //*((V_UINT32P)(DRAMC0_BASE + 0x0098)) = 0x28282828;
        //*((V_UINT32P)(DDRPHY_BASE + 0x0098)) = 0x28282828;

        //  DLE 8
        //*((V_UINT32P)(DRAMC0_BASE + 0x007c)) = (*((V_UINT32P)(DRAMC0_BASE + 0x007c)) & 0xFFFFFF8F) | ((8 & 0x07) <<4);
        //*((V_UINT32P)(DDRPHY_BASE + 0x007c)) = (*((V_UINT32P)(DDRPHY_BASE + 0x007c)) & 0xFFFFFF8F) |  ((8 & 0x07) <<4);

        //*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) = (*((V_UINT32P)(DRAMC0_BASE + 0x00e4)) & 0xFFFFFFEF) | (((8 >> 3) & 0x01) << 4);
        //*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) =  (*((V_UINT32P)(DDRPHY_BASE + 0x00e4)) & 0xFFFFFFEF) | (((8 >> 3) & 0x01) << 4);

        // RX DQ, DQS input delay
        *((V_UINT32P)(DRAMC0_BASE + 0x0210)) = 0x4050003;
        *((V_UINT32P)(DDRPHY_BASE + 0x0210)) = 0x4050003;
        *((V_UINT32P)(DRAMC0_BASE + 0x0214)) = 0x60305;
        *((V_UINT32P)(DDRPHY_BASE + 0x0214)) = 0x60305;
        *((V_UINT32P)(DRAMC0_BASE + 0x0218)) = 0x1010405;
        *((V_UINT32P)(DDRPHY_BASE + 0x0218)) = 0x1010405;
        *((V_UINT32P)(DRAMC0_BASE + 0x021c)) = 0x5030004;
        *((V_UINT32P)(DDRPHY_BASE + 0x021c)) = 0x5030004;
        *((V_UINT32P)(DRAMC0_BASE + 0x0220)) = 0x4040001;
        *((V_UINT32P)(DDRPHY_BASE + 0x0220)) = 0x4040001;
        *((V_UINT32P)(DRAMC0_BASE + 0x0224)) = 0x1040304;
        *((V_UINT32P)(DDRPHY_BASE + 0x0224)) = 0x1040304;
        *((V_UINT32P)(DRAMC0_BASE + 0x0228)) = 0x1030404;
        *((V_UINT32P)(DDRPHY_BASE + 0x0228)) = 0x1030404;
        *((V_UINT32P)(DRAMC0_BASE + 0x022c)) = 0x4040004;
        *((V_UINT32P)(DDRPHY_BASE + 0x022c)) = 0x4040004;

        *((V_UINT32P)(DRAMC0_BASE + 0x0018)) = 0x1A1C1A1D;
        *((V_UINT32P)(DDRPHY_BASE + 0x0018)) = 0x1A1C1A1D;
        *((V_UINT32P)(DRAMC0_BASE + 0x001c)) = 0x1A1C1A1D;
        *((V_UINT32P)(DDRPHY_BASE + 0x001c)) = 0x1A1C1A1D;

        // TX DQ, DQS output delay
        *((V_UINT32P)(DRAMC0_BASE + 0x0014)) = 0x8888;   // DQS output delay
        *((V_UINT32P)(DDRPHY_BASE + 0x0014)) = 0x8888;

        *((V_UINT32P)(DRAMC0_BASE + 0x0010)) = 0x8888;   // DQM output delay
        *((V_UINT32P)(DDRPHY_BASE + 0x0010)) = 0x8888;

        *((V_UINT32P)(DRAMC0_BASE + 0x0200)) = 0x88888888;   // DQ
        *((V_UINT32P)(DDRPHY_BASE + 0x0200)) = 0x88888888;
        *((V_UINT32P)(DRAMC0_BASE + 0x0204)) = 0x88888888;
        *((V_UINT32P)(DDRPHY_BASE + 0x0204)) = 0x88888888;
        *((V_UINT32P)(DRAMC0_BASE + 0x0208)) = 0x99998888;
        *((V_UINT32P)(DDRPHY_BASE + 0x0208)) = 0x99998888;
        *((V_UINT32P)(DRAMC0_BASE + 0x020c)) = 0x99998888;
        *((V_UINT32P)(DDRPHY_BASE + 0x020c)) = 0x99998888;


#endif




}


static char id[22];
static int emmc_nand_id_len=16;
static int fw_id_len;
static int mt_get_mdl_number (void)
{
    static int found = 0;
    static int mdl_number = -1;
    int i;
    int j;
    int has_emmc_nand = 0;
    int discrete_dram_num = 0;
    int mcp_dram_num = 0;

    unsigned int mode_reg_5;
    unsigned int dram_type;
    unsigned int index;
#ifdef HARDCODE_DRAM_SETTING
#ifdef FLASH_TOOL_PRELOADER
//   print("[Check] Flash tool EMI Setting determination\n");
  num_of_emi_records = NUM_EMI_RECORD_HARD_CODE ; 
  platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
   if ((memcmp(id, emi_settings[5].ID, emi_settings[5].id_length) == 0 )||( memcmp(id, emi_settings[6].ID, emi_settings[6].id_length) == 0 ) )
     {
       index=8; 
 //      print("[Check] Flash tool mt_get_mdl_number 0x%x\n",index);
       return index;
     }
   else
     {
       index=7;
   //    print("[Check] Flash tool mt_get_mdl_number 0x%x\n",index);
       return index;
     }

#else
  num_of_emi_records = NUM_EMI_RECORD_HARD_CODE ;
#ifdef H9TCNNNBLDMMPR
 index=0; 
#endif

#ifdef EDBA232B1MA
  index=1;
#endif


#ifdef K4PAG304EB_FGC1
  index=2;
#endif

#ifdef H9TP32A8JDACPR_KGM
  index=3;
#endif

#ifdef KMK7U000VM_B309
  index=4;
#endif

#ifdef H9TQ65A8JTMCUR_KTM
  index=5;
#endif

#ifdef H9TQ18ABJTMCUR
 index=6;
#endif

#ifdef COMMON_DDR3_1GB
index=9;
#endif


#ifdef KMR4Z0001M_B802
index=10;
#endif

#ifdef KMK7X000VM_B314
index=11;
#endif

#ifdef H9TP32A6DDMCPR
index=12;
#endif
platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
//print("[Check]mt_get_mdl_number 0x%x\n",index);
return index;
#endif


#else
    if (!(found))
    {
        int result=0;
        platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
        for (i = 0 ; i < num_of_emi_records; i++)
        {
            if ((emi_settings[i].type & 0x0F00) == 0x0000) 
            {
                discrete_dram_num ++; 
            }
            else
            {
                mcp_dram_num ++; 
            }
        }

        /*If the number >=2  &&
         * one of them is discrete DRAM
         * enable combo discrete dram parse flow
         * */
        if ((discrete_dram_num > 0) && (num_of_emi_records >= 2))
        {
            /* if we enable combo discrete dram
             * check all dram are all same type and not DDR3
             * */
            enable_combo_dis = 1;
            dram_type = emi_settings[0].type & 0x000F;
            for (i = 0 ; i < num_of_emi_records; i++)
            {
                if (dram_type != (emi_settings[i].type & 0x000F))
                {
                    printf("[EMI] Combo discrete dram only support when combo lists are all same dram type.");
                    ASSERT(0);
                }
                if ((emi_settings[i].type & 0x000F) == TYPE_PCDDR3) 
                {
                    // has PCDDR3, disable combo discrete drame, no need to check others setting 
                    enable_combo_dis = 0; 
                    break;
                }
                dram_type = emi_settings[i].type & 0x000F;
            }
            
        } 
        printf("[EMI] mcp_dram_num:%d,discrete_dram_num:%d,enable_combo_dis:%d\r\n",mcp_dram_num,discrete_dram_num,enable_combo_dis);
        /*
         *
         * 0. if there is only one discrete dram, use index=0 emi setting and boot it.
         * */
        if ((0 == mcp_dram_num) && (1 == discrete_dram_num))
        {
            mdl_number = 0;
            found = 1;
            return mdl_number;
        }
            

        /* 1.
         * if there is MCP dram in the list, we try to find emi setting by emmc ID
         * */
        if (mcp_dram_num > 0)
        {
        result = platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);
    
        for (i = 0; i < num_of_emi_records; i++)
        {
            if (emi_settings[i].type != 0)
            {
                if ((emi_settings[i].type & 0xF00) != 0x000)
                {
                    if (result == 0)
                    {   /* valid ID */

                        if ((emi_settings[i].type & 0xF00) == 0x100)
                        {
                            /* NAND */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0){
                                memset(id + emi_settings[i].id_length, 0, sizeof(id) - emi_settings[i].id_length);                                
                                mdl_number = i;
                                found = 1;
                                break; /* found */
                            }
                        }
                        else
                        {
                            
                            /* eMMC */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0)
                            {
#if 1
                                printf("fw id len:%d\n",emi_settings[i].fw_id_length);
                                if (emi_settings[i].fw_id_length > 0)
                                {
                                    char fw_id[6];
                                    memset(fw_id, 0, sizeof(fw_id));
                                    memcpy(fw_id,id+emmc_nand_id_len,fw_id_len);
                                    for (j = 0; j < fw_id_len;j ++){
                                        printf("0x%x, 0x%x ",fw_id[j],emi_settings[i].fw_id[j]); 
                                    }
                                    if(memcmp(fw_id,emi_settings[i].fw_id,fw_id_len) == 0)
                                    {
                                        mdl_number = i;
                                        found = 1;
                                        break; /* found */
                                    }
                                    else
                                    {
                                        printf("[EMI] fw id match failed\n");
                                    }
                                }
                                else
                                {
                                    mdl_number = i;
                                    found = 1;
                                    break; /* found */
                                }
#else
                                    mdl_number = i;
                                    found = 1;
                                    break; /* found */
#endif
                            }
                            else{
                                  printf("[EMI] index(%d) emmc id match failed\n",i);
                            }
                            
                        }
                    }
                }
            }
        }
        }
#if 1
        /* 2. find emi setting by MODE register 5
         * */
        // if we have found the index from by eMMC ID checking, we can boot android by the setting
        // if not, we try by vendor ID
        if ((0 == found) && (1 == enable_combo_dis))
        {
            unsigned int manu_id;
            /* DDR reserve mode no need to enable memory & test */
            if((g_ddr_reserve_enable==1) && (g_ddr_reserve_success==1))
            {
                unsigned int emi_cona;
                emi_cona = *(volatile unsigned *)(EMI_CONA);
                print("[DDR Reserve mode] EMI dummy read CONA = 0x%x in mt_get_mdl_number()\n", emi_cona);

                /*
                 * NOTE:
                 * in DDR reserve mode, the DRAM access abnormal after DRAM_MRR(), so
                 * we use 0x10004100[7:0] to store DRAM's vendor ID, skip DRAM_MRR() after reboot;
                 * 0x10004100[7:0] is the dummy register.
                 * */
                print("[EMI] 0x10004100:%x\n\r",*(volatile unsigned *)(0x10004100));
                manu_id = (*(volatile unsigned *)(0x10004100) & 0xff);
            }
        else{
            EMI_SETTINGS *emi_set;
            //print_DBG_info();
            //print("-->%x,%x,%x\n",emi_set->DRAMC_ACTIM_VAL,emi_set->sub_version,emi_set->fw_id_length); 
            //print("-->%x,%x,%x\n",emi_setting_default.DRAMC_ACTIM_VAL,emi_setting_default.sub_version,emi_setting_default.fw_id_length); 
            dram_type = mt_get_dram_type_for_dis();
            if (TYPE_LPDDR2 == dram_type)
            {
                print("[EMI] LPDDR2 discrete dram init\r\n");
                emi_set = &emi_setting_default_lpddr2;
                init_lpddr2(emi_set);
            }
            else if (TYPE_LPDDR3 == dram_type)
            {
                print("[EMI] LPDDR3 discrete dram init\r\n");
                emi_set = &emi_setting_default_lpddr3;
                init_lpddr3(emi_set);
            }
            //print_DBG_info();
            if (dramc_calib() < 0) {
                print("[EMI] Default EMI setting DRAMC calibration failed\n\r");
            } else {
                print("[EMI] Default EMI setting DRAMC calibration passed\n\r");
            }

            manu_id = DRAM_MRR(0x5);
          }
            print("[EMI]MR5:%x\n",manu_id);
            //try to find discrete dram by DDR2_MODE_REG5(vendor ID)
            for (i = 0; i < num_of_emi_records; i++)
            {
                if (TYPE_LPDDR2 == dram_type)
                    mode_reg_5 = emi_settings[i].LPDDR2_MODE_REG_5; 
                else if (TYPE_LPDDR3 == dram_type)
                    mode_reg_5 = emi_settings[i].LPDDR3_MODE_REG_5; 
                print("emi_settings[i].MODE_REG_5:%x,emi_settings[i].type:%x\n",mode_reg_5,emi_settings[i].type);
                //only check discrete dram type
                if ((emi_settings[i].type & 0x0F00) == 0x0000) 
                {
                    //support for compol discrete dram 
                    if ((mode_reg_5 == manu_id) )
                    {
                        mdl_number = i;
                        found = 1;
                        break; 
                    }
                }
            }
        }
#endif
        printf("found:%d,i:%d\n",found,i);    
    }
    

    return mdl_number;
#endif
}


int mt_get_dram_type (void)
{

    int n;
   /* if combo discrete is enabled, the dram_type is LPDDR2 or LPDDR4, depend on the emi_setting list*/
    if ( 1 == enable_combo_dis)
    return mt_get_dram_type_for_dis();

    n = mt_get_mdl_number ();

    if (n < 0  || n >= num_of_emi_records)
    {
        return 0; /* invalid */
    }

    return (emi_settings[n].type & 0xF);

}

int get_dram_rank_nr (void)
{

    int index;
    int emi_cona;
    index = mt_get_mdl_number ();
    if (index < 0 || index >=  num_of_emi_records)
    {
        return -1;
    }

#ifdef COMBO_MCP
    emi_cona = emi_settings[index].EMI_CONA_VAL;
#else
    emi_cona = EMI_CONA_VAL;
#if CFG_FPGA_PLATFORM
    return 1;
#endif
#endif
    return (emi_cona & 0x20000) ? 2 : 1;

}

int get_dram_rank_size (int dram_rank_size[])
{
 int index,/* bits,*/ rank_nr, i;
    //int emi_cona;
#if 1
    index = mt_get_mdl_number ();
    
    if (index < 0 || index >=  num_of_emi_records)
    {
        return;
    }

    rank_nr = get_dram_rank_nr();

    for(i = 0; i < rank_nr; i++){
        dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];

        printf("%d:dram_rank_size:%x\n",i,dram_rank_size[i]);
    }

    return;
#endif

   

}
#ifdef DUMP_DRAMC_REGS
void print_DBG_info(){
    unsigned int addr = 0x0;
        printf("=====================DBG=====================\n");
    for(addr = 0x0; addr < 0x650; addr +=4){
    printf("addr:%x, value:%x\n",addr, DRAMC_READ_REG(addr));
}
    printf("=============================================\n");
}
#endif
int DFS_ability_detection(void)
{

  int rank0_coarse;
  int rank1_coarse;
  int n;
  int DDR_TYPE;
  DDR_TYPE=DDR_type;
  if (DDR_TYPE == TYPE_LPDDR3){
    rank0_coarse = atoi(opt_gw_coarse_value0);
    rank1_coarse = atoi(opt_gw_coarse_value1);
    printf("atoi rank 0 coarse value %d, rank1 coarse value %d\n",rank0_coarse,rank0_coarse);
    if(rank0_coarse <=20 && rank1_coarse <=20){
       printf("DFS could be enable\n");
       *((V_UINT32P)(DRAMC0_BASE + 0xf4)) = *((V_UINT32P)(DRAMC0_BASE + 0xf4)) | (0x1 << 15);
       *((V_UINT32P)(DDRPHY_BASE + 0xf4)) = *((V_UINT32P)(DDRPHY_BASE + 0xf4)) | (0x1 << 15);
       return 1;        
    }
    else{
       printf("DFS could not be enable\n");
       *((V_UINT32P)(DRAMC0_BASE + 0xf4)) = *((V_UINT32P)(DRAMC0_BASE + 0xf4)) & ~(0x1 << 15);
       *((V_UINT32P)(DDRPHY_BASE + 0xf4)) = *((V_UINT32P)(DDRPHY_BASE + 0xf4)) & ~(0x1 << 15);
       return 0;
    }
   }
  else
  { 
      printf("LPDDR2 could enable DFS\n");
      return 0;
  }

return 0;
}

#define Vcore_HV_LPPDR2  (0x40)   //1.1
#define Vcore_NV_LPPDR2  (0x30)   //1.0
#define Vcore_LV_LPPDR2  (0x20)  //0.9

#define Vcore_HV_LPPDR3  (0x50)   //1.2
#define Vcore_NV_LPPDR3  (0x44)   //1.125
#define Vcore_LV_LPPDR3  (0x30)   //1.0

#define Vcore_HV_PCDDR3  (0x48)   //1.15
#define Vcore_NV_PCDDR3  (0x44)   //1.125
#define Vcore_LV_PCDDR3  (0x30)   //1.0

#define Vmem_HV_LPDDR2 (0x60) //1.3
#define Vmem_NV_LPDDR2 (0x54) //1.22
#define Vmem_LV_LPDDR2 (0x47) //1.14

#define Vmem_HV_LPDDR3 (0x60) //1.3
#define Vmem_NV_LPDDR3 (0x54) //1.22
#define Vmem_LV_LPDDR3 (0x4A) //1.16

#define Vmem_HV_PCDDR3 (0x68 + 0x0B) //1.418V
#define Vmem_NV_PCDDR3 (0x68 + 0x02) //1.350V
#define Vmem_LV_PCDDR3 (0x68 - 0x0A) //1.283V

void DRAM_Vcore_adjust(int nAdjust)
{
    unsigned int OldVcore1 = 0;
    unsigned int OldVcore2 = 0;
        switch(nAdjust) {
        case 1 :        // HV. Vcore 1.1V
                mt6333_config_interface(0x6B, Vcore_HV_LPPDR2, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_HV_LPPDR2, 0x7F,0);
                break;
        case 2 :        // NV. Vcore 1.0V
                mt6333_config_interface(0x6B, Vcore_NV_LPPDR2, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_NV_LPPDR2, 0x7F,0);
                break;
        case 3 :        // LV. Vcore 0.9V
                mt6333_config_interface(0x6B, Vcore_LV_LPPDR2, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_LV_LPPDR2, 0x7F,0);
                break;
        case 4 :        // HHV. Vcore 1.2V
                mt6333_config_interface(0x6B, Vcore_HV_LPPDR3, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_HV_LPPDR3, 0x7F,0);
                break;
        case 5 :        // HV. Vcore 1.1V
                mt6333_config_interface(0x6B, Vcore_NV_LPPDR3, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_NV_LPPDR3, 0x7F,0);
                break;
        case 6 :        // NV. Vcore 1.0V
                mt6333_config_interface(0x6B, Vcore_LV_LPPDR3, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_LV_LPPDR3, 0x7F,0);
                break;
        case 7 :        // HV. Vcore 1.15V
                mt6333_config_interface(0x6B, Vcore_HV_PCDDR3, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_HV_PCDDR3, 0x7F,0);
                break;
        case 8 :        // NV. Vcore 1.125V
                mt6333_config_interface(0x6B, Vcore_NV_PCDDR3, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_NV_PCDDR3, 0x7F,0);
                break;
        case 9 :        // LV. Vcore 1.0V
                mt6333_config_interface(0x6B, Vcore_LV_PCDDR3, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_LV_PCDDR3, 0x7F,0);
                break;
        default :
                mt6333_config_interface(0x6B, Vcore_NV_LPPDR2, 0x7F,0);
                mt6333_config_interface(0x6C, Vcore_NV_LPPDR2, 0x7F,0);
                break;
        }
}

void DRAM_Vmem_adjust(int nAdjust)
{
    unsigned int OldVmem1 = 0;
    unsigned int OldVmem2 = 0;
        switch(nAdjust) {
        case 1 :        // HV. Vmem 1.375V
                mt6333_config_interface(0x81, Vmem_HV_LPDDR2, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_HV_LPDDR2, 0x7F,0);
                break;
        case 2 :        // NV. Vmem 1.22V
                mt6333_config_interface(0x81, Vmem_NV_LPDDR2, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_NV_LPDDR2, 0x7F,0);
                break;
        case 3 :        // LV. Vmem 1.14V
                mt6333_config_interface(0x81, Vmem_LV_LPDDR2, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_LV_LPDDR2, 0x7F,0);
                break;
        case 4 :        // LV. Vmem 1.16V
                mt6333_config_interface(0x81, Vmem_HV_LPDDR3, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_HV_LPDDR3, 0x7F,0);
                break;
        case 5 :        // PCDDR3 HV. Vmem 1.418V
                mt6333_config_interface(0x81, Vmem_NV_LPDDR3, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_NV_LPDDR3, 0x7F,0);
                break;
        case 6 :        // PCDDR3 NV. Vmem 1.350V
                mt6333_config_interface(0x81, Vmem_LV_LPDDR3, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_LV_LPDDR3, 0x7F,0);
                break;
		case 7 :		// PCDDR3 HV. Vmem 1.418V
                mt6333_config_interface(0x81, Vmem_HV_PCDDR3, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_HV_PCDDR3, 0x7F,0);
                break;
        case 8 :        // PCDDR3 NV. Vmem 1.350V
                mt6333_config_interface(0x81, Vmem_NV_PCDDR3, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_NV_PCDDR3, 0x7F,0);
                break;
        case 9 :        // PCDDR3 LV. Vmem 1.283V
				mt6333_config_interface(0x81, Vmem_LV_PCDDR3, 0x7F,0);
				mt6333_config_interface(0x82, Vmem_LV_PCDDR3, 0x7F,0);
				break;	
        default :
                mt6333_config_interface(0x81, Vmem_LV_LPDDR2, 0x7F,0);
                mt6333_config_interface(0x82, Vmem_LV_LPDDR2, 0x7F,0);
                break;
        }
}

/*
* mt_set_emi: Set up EMI/DRAMC.
*/
#if CFG_FPGA_PLATFORM
void mt_set_emi (void)
{
}
#else
void mt_set_emi (void)
{
    int index = 0;
    unsigned int val1,val2/*, temp1, temp2*/;
    EMI_SETTINGS *emi_set;
    unsigned int manu_id;
    *EMI_CONF = 0x004210000; // Enable EMI Address Scramble 


#ifdef COMBO_MCP

DDR_type=mt_get_dram_type();
print ("mt_get_dram_type() 0x%x\n",DDR_type);
switch (DDR_type)
{
    case TYPE_mDDR:
        print("[EMI] DDR1\r\n");
        break;
    case TYPE_LPDDR2:
        print("[EMI] LPDDR2\r\n");
        break;
    case TYPE_LPDDR3:
        print("[EMI] LPDDR3\r\n");
        break;
    case TYPE_PCDDR3:
        print("[EMI] PCDDR3\r\n");
        break;
    default:
        print("[EMI] unknown dram type:%d\r\n",mt_get_dram_type());
        break;

}
index = mt_get_mdl_number ();
print("[Check]mt_get_mdl_number 0x%x\n",index);
print("[EMI] eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8],id[9],id[10],id[11],id[12],id[13],id[14],id[15]);
if (index < 0 || index >=  num_of_emi_records)
{
    print("[EMI] setting failed 0x%x\r\n", index);
    return;
}

print("[EMI] MDL number = %d\r\n", index);
emi_set = &emi_settings[index];
print("[EMI] emi_set eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", emi_set->ID[0], emi_set->ID[1], emi_set->ID[2], emi_set->ID[3], emi_set->ID[4], emi_set->ID[5], emi_set->ID[6], emi_set->ID[7], emi_set->ID[8],emi_set->ID[9],emi_set->ID[10],emi_set->ID[11],emi_set->ID[12],emi_set->ID[13],emi_set->ID[14],emi_set->ID[15]);
if ((emi_set->type & 0xF) == TYPE_LPDDR2)
{
    #ifdef MTK_MT6333_SUPPORT
    #if VcHV_VmHV
    DRAM_Vmem_adjust(1);
    DRAM_Vcore_adjust(1);
    pmic_config_interface(0x558, 0xB, 0xF,8);
    print("[EMI] LPDDR2 Vcore = 1.1 Vmem = 1.3V \r\n");
    #elif VcLV_VmLV
    DRAM_Vcore_adjust(3);
    DRAM_Vmem_adjust(3);
    pmic_config_interface(0x558, 0x6, 0xF,8);//-120mV
    print("[EMI] LPDDR2 Vcore = 0.9 Vmem = 1.14V \r\n");
    #elif VcHV_VmLV
    DRAM_Vcore_adjust(1);
    DRAM_Vmem_adjust(3);
    //pmic_config_interface(0x558, 0x5, 0xF,8);//-100mV
    pmic_config_interface(0x558, 0x6, 0xF,8);//-120mV
    print("[EMI] LPDDR2 Vcore = 1.1 Vmem = 1.14V \r\n"); 
    #elif VcLV_VmHV
    DRAM_Vcore_adjust(3);
    DRAM_Vmem_adjust(1);
    pmic_config_interface(0x558, 0xB, 0xF,8);
    print("[EMI] LPDDR2 Vcore = 0.9 Vmem = 1.3V \r\n");
    #else
    DRAM_Vmem_adjust(2);
    DRAM_Vcore_adjust(2);
    print("[EMI] LPDDR2 Vcore = 1.0 Vmem = 1.22V \r\n");
    #endif    
    #endif
    mt_mempll_init(1066,1);
    #ifdef REXTDN_ENABLE
    ett_rextdn_sw_calibration();
    #endif
    init_lpddr2(emi_set);
}
else if ((emi_set->type & 0xF) == TYPE_LPDDR3)
{
    //emi_set = &emi_setting_default_lpddr3;
    #ifdef MTK_MT6333_SUPPORT
    #if VcHV_VmHV
    DRAM_Vcore_adjust(4);
    DRAM_Vmem_adjust(4);
    pmic_config_interface(0x558, 0xB, 0xF,8);
    print("[EMI] LPDDR3 Vcore = 1.2 Vmem = 1.3V \r\n");
    #elif VcLV_VmLV
    DRAM_Vmem_adjust(6);
    DRAM_Vcore_adjust(6);
    pmic_config_interface(0x558, 0x6, 0xF,8);//-120mV
    print("[EMI] LPDDR3 Vcore = 1.0 Vmem = 1.16V \r\n");
    #elif VcHV_VmLV
    DRAM_Vcore_adjust(4);
    DRAM_Vmem_adjust(6);
    pmic_config_interface(0x558, 0x6, 0xF,8);//-120mV
    print("[EMI] LPDDR3 Vcore = 1.15 Vmem = 1.16V \r\n");
    #elif VcLV_VmHV
    DRAM_Vcore_adjust(6);
    DRAM_Vmem_adjust(4);
    pmic_config_interface(0x558, 0xB, 0xF,8);
    print("[EMI] LPDDR3 Vcore = 1.0 Vmem = 1.3V \r\n");
    #else
    if((*((volatile unsigned int *)(0x10206174))& 0x1)==1)
    {
      mt6333_config_interface(0x6B, 0x3c, 0x7F,0);   //Vcore 1.05V
      mt6333_config_interface(0x6C, 0x3c, 0x7F,0);
      DRAM_Vmem_adjust(5);             //Vmem 1.22V
      print("[EMI] LPDDR3 Vcore = 1.075V Vmem = 1.22V \r\n");
    }
    else
    {
      mt6333_config_interface(0x6B, 0x44, 0x7F,0);  // Vcore 1.125V
      mt6333_config_interface(0x6C, 0x44, 0x7F,0);
      DRAM_Vmem_adjust(5);           //Vmem 1.22V
      print("[EMI] LPDDR3 Vcore = 1.125V Vmem = 1.22V \r\n");
    }
    #endif    
    #endif
    #ifndef DRAM_1066
    mt_mempll_init(1333,1);
    #else
    mt_mempll_init(1066,1);
    #endif
    #ifdef REXTDN_ENABLE
    ett_rextdn_sw_calibration();
    #endif
    init_lpddr3(emi_set);


}
else if ((emi_set->type & 0xF) == TYPE_PCDDR3)
{//fy for 92 bring up
	//----------------------------------------
	//1.default Vcore 1.125V , Vmem 1.350V, HV/LV only for HQA stress test
	#ifdef MTK_MT6333_SUPPORT
	#if VcHV_VmHV
	DRAM_Vcore_adjust(7);
	DRAM_Vmem_adjust(7);
	print("[EMI] PCDDR3 Vcore = 1.15V, Vmem = 1.418V\r\n");
	#elif VcLV_VmLV
	DRAM_Vcore_adjust(9);
	DRAM_Vmem_adjust(9);
	print("[EMI] PCDDR3 Vcore = 1.0V, Vmem = 1.283V\r\n");
	#else
	DRAM_Vcore_adjust(8);//Because PCDDR3 power on at 1333MHZ,we set vcore to 1.125V
	DRAM_Vmem_adjust(8);
	print("[EMI] PCDDR3 Vcore = 1.125V, Vmem = 1.350V\r\n");
	#endif
	#endif
	//2.mempll 1066MHz --> 1333MHz
	int dram_clock = 1333;
	print("[EMI] PCDDR3 DRAM Clock = %d MHz\r\n", dram_clock);
	mt_mempll_init(dram_clock, 1);
	if(dram_clock == 1333)	ddr3_1333_enable = 0x01;
	else					ddr3_1333_enable = 0x00;
	//3.REXTDN Calibration
	#ifdef REXTDN_ENABLE
	print("[EMI] PCDDR3 RXTDN Calibration:\r\n");
    ett_rextdn_sw_calibration();
	#endif	
	//----------------------------------------
	//4.DDR3 init
    init_ddr3(emi_set);
	//----------------------------------------
}
else
{
    print("The DRAM type is not supported");
    ASSERT(0);
}
//    print_DBG_info();
#else
    #ifdef DDRTYPE_LPDDR2	
    init_lpddr2(emi_set);
    #endif

    #ifdef DDRTYPE_DDR3
    init_ddr3(emi_set);
    #endif

    #ifdef DDRTYPE_LPDDR3	
    init_lpddr3(emi_set);
    #endif

#endif
#if 0
	/* save original PLL setting */
	temp1 = DRV_Reg32(AP_PLL_CON1);
	temp2 = DRV_Reg32(AP_PLL_CON2);
	temp1 = temp1 & 0xFF00FC0C;
	temp2 = temp2 & 0xFFFFFFF8;
	
	//SWITCH FROM SW TO HW SPM SLEEP CONTROL
	DRV_WriteReg32(AP_PLL_CON1, 0x0);
	DRV_WriteReg32(AP_PLL_CON2, 0x0);
	//*AP_PLL_CON1 = 0x0;
	//*AP_PLL_CON2 = 0x0;
#endif
	// Ungate dram transaction blockage mechanism after Dram Init.
	// If bit 10 of EMI_CONM is not 1, any transaction to EMI will be ignored.
	*EMI_CONM |= (1<<10);

	//DRAM Translate Off
	// Edward : After discussing with CC Wen, need to test different patterns like following  during calibration to see which one is worse in the future. If possible, do two is good.
	// 					TESTXTALKPAT(REG.48[16])		TESTAUDPAT (REG.44[7]) 
	// Audio pattern				0							1
	// Cross-talk					1							0

#ifndef PHYSYNC_MODE
        // not PHYSYNC mode.
        *((V_UINT32P)(DRAMC0_BASE + 0x015c)) |= 0x80000000;    //15c.31 set to 1 in sync mode, 0 in PHYsync mode
        *((V_UINT32P)(DDRPHY_BASE + 0x015c)) |= 0x80000000;    //15c.31 set to 1 in sync mode, 0 in PHYsync mode
#endif

#ifdef DUMP_DRAMC_REGS
    print_DBG_info();
#endif
    if (dramc_calib() < 0) {
        print("[EMI] DRAMC calibration failed\n\r");
    } else {
        print("[EMI] DRAMC calibration passed\n\r");
    }
    if ( *(volatile unsigned *)(EMI_CONA)& 0x20000) /* check dual rank support */ {
                unsigned int val1, val2;
                val1 = *((V_UINT32P)(DRAMC0_BASE + 0xe0)) & 0x07000000;
#ifdef PHYSYNC_MODE
                // 2PLL, 3PLL
                val1 = ((val1 >> 24)+2) <<16;
#else
                // 1PLL
                val1 = ((val1 >> 24)+1) <<16;
#endif
                val2 = *((V_UINT32P)(DRAMC0_BASE + 0x1c4)) & 0xFFF0FFFF | val1;
                *((V_UINT32P)(DRAMC0_BASE + 0x1c4)) = val2;
    }
   


        // Edward : The following setting is after calibration.

        // Edward : power-down CMP. (After REXTDN)
        *((V_UINT32P)(DRAMC0_BASE + 0x0630)) |= (1<<20);
        *((V_UINT32P)(DDRPHY_BASE + 0x0630)) |= (1<<20);


        // Edward : enable DQS GW HW enable[31].  (Tracking value apply to 2 ranks[30] need to be set to 1 when if DFS is applied.)
        *((V_UINT32P)(DRAMC0_BASE + 0x01c0)) |= (0x80000000);           // Edward : Need to make sure bit 13 is 0.
        *((V_UINT32P)(DDRPHY_BASE + 0x01c0)) |= (0x80000000);

        // Edward : Add following based on CC Wen request.
        //      * WFLUSHEN (REG.1EC[14]): 1
        //      * R_D/RWHPRICTL (REG.1EC[13]): 0
        //      * R_DMEMILLATEN (REG.1EC[11]): 1
        //      R/W aging (REG.1EC[10]) : 1
        //    R/W low latency (REG.1EC[9]) : 1
        //    R/W high priority (REG.1EC[8]) : 1
        //      R/W out of order enable (REG.1EC[4]) : 1
        *((V_UINT32P)(DRAMC0_BASE + 0x01ec)) |= (0x4f10);       // Edward : Need to make sure bit 13 is 0.
        *((V_UINT32P)(DDRPHY_BASE + 0x01ec)) |= (0x4f10); 

#if 0
    /* revert PLL setting */
    DRV_WriteReg32(AP_PLL_CON1, temp1);
    DRV_WriteReg32(AP_PLL_CON2, temp2);
#endif

	#if 1
	if (mt_get_dram_type() == TYPE_PCDDR3)
	{//A15 enable: EMI_CONA bit17 1, TESTB bit7 1
		if(*((volatile unsigned int *)(EMI_BASE + 0x00e8)) & 0x80) //TESTB bit7
		{
			*(volatile unsigned int*)(EMI_CONA) |= 0x20000; //bit17 2 rank enable for A15
		}
	}
	#endif
#ifdef DUMP_DRAMC_REGS	
    print_DBG_info();
#endif
    //pmic_voltage_read(1);//check the pmic setting
    //print("[MEM]mt_get_dram_type:%d\n",mt_get_dram_type());
    /* FIXME: modem exception occurs if set the max pending count */
    /* To Repair SRAM */
    #if defined(CTP) || defined(SLT)
    #ifdef REPAIR_SRAM
    int repair_ret;
    repair_ret = repair_sram();
    if(repair_ret != 0){
        printf("Sram repair failed %d\n", repair_ret);
        while(1);
    }
    printf("Sram repaired ok\n");
    #endif
    #endif

#ifdef GATING_CLOCK_CONTROL_DVT
    *((V_UINT32P)(DRAMC0_BASE + 0x01dc)) &= ~(0x1 << 31);       // Edward : Need to make sure bit 13 is 0.
    *((V_UINT32P)(DDRPHY_BASE + 0x01dc)) &= ~(0x1 << 30);
#endif
    //print("MR5:%x,MR8:%x\n",DRAM_MRR(0x5),DRAM_MRR(0x8)); 

#ifdef DERATING_ENABLE 
    derating_enable();
#endif

}
#endif

#if 0
void in_sref()
{
    volatile unsigned int tmp;
    tmp = DRAMC_READ_REG(DRAMC0_BASE+0x4);
    if(tmp & (0x1 << 26))
        print("Enable sref %x\n", tmp);
    else
        print("Disable sref %x\n", tmp);
    tmp = DRAMC_READ_REG(DRAMC0_BASE+0x3B8);
    if(tmp & (0x1 << 16))
        print("In sref %x\n", tmp);
    else
        print("Not in sref %x\n", tmp);
    tmp = READ_REG(TOPRGU_BASE+0x40);
    if(tmp & 0x10000)
        print("success finish ddr reserved flow. %x\n", tmp);
    else
        print("failed finish ddr reserved flow. %x\n", tmp);
}
#endif
void release_dram()
{
#ifdef DDR_RESERVE_MODE  
    int counter = TIMEOUT;
    rgu_release_rg_dramc_conf_iso();
    rgu_release_rg_dramc_iso();
    rgu_release_rg_dramc_sref();
    while(counter)
    {
      if(rgu_is_dram_slf() == 0) /* expect to exit dram-self-refresh */
        break;
      counter--;
    }
    if(counter == 0)
    {
      if(g_ddr_reserve_enable==1 && g_ddr_reserve_success==1)
      {
        print("[DDR Reserve] release dram from self-refresh FAIL!\n");
        g_ddr_reserve_success = 0;
      }
    }
#endif    
}

void check_ddr_reserve_status()
{
#ifdef DDR_RESERVE_MODE  
    int counter = TIMEOUT;
    if(rgu_is_reserve_ddr_enabled())
    {
      g_ddr_reserve_enable = 1;
      if(rgu_is_reserve_ddr_mode_success())
      {
        while(counter)
        {
          if(rgu_is_dram_slf())
          {
            g_ddr_reserve_success = 1;
            break;
          }
          counter--;
        }
        if(counter == 0)
        {
          print("[DDR Reserve] ddr reserve mode success but DRAM not in self-refresh!\n");
          g_ddr_reserve_success = 0;
        }
      }
    else
      {
        print("[DDR Reserve] ddr reserve mode FAIL!\n");
        g_ddr_reserve_success = 0;
      }
    }
    else
    {
      print("[DDR Reserve] ddr reserve mode not be enabled yet\n");
      g_ddr_reserve_enable = 0;
    }
    
    /* release dram, no matter success or failed */
    release_dram();
#endif    
}
#ifdef REXTDN_ENABLE
void ett_rextdn_sw_calibration(){
    unsigned int drvp_sel, drvn_sel;
    unsigned int tmp;
    unsigned int sel;

    print("Start REXTDN SW calibration...\n");
    
//    for(sel = 0; sel < 4; sel ++){ 
    for(sel = 0; sel < 1; sel ++){ 
        
    /* 1.initialization */
            //DRAMC_WRITE_REG(0x0,DRAMC_DLLSEL) ;
            /* set DRVP_SEL as 0, set DRVN_SEL as 0 */
            //DRAMC_WRITE_REG(0x0,0x100) ;
            //dbg_print("SEL:%d\n",DRAMC_READ_REG(0x100));
            drvp_sel = (sel>>2)&0x3;
            drvn_sel = (sel>>0)&0x3;

            /* disable power down mode */
            tmp = (DRAMC_READ_REG(0x1e4)&0xffffdfff);
            DRAMC_WRITE_REG(tmp,0x1e4) ;
            print("PD 0x1e4[13]:%xh\n",DRAMC_READ_REG(0x1e4));
    /* 2.DRVP calibration */
            /* setup SEL,*/
            tmp = (DRAMC_READ_REG(0x100)&0xfffcffff)|(drvp_sel << 16);
            DRAMC_WRITE_REG(tmp,0x100) ;
            print("1.INTREF_SEL:0x100[17:16]:%xh\n",DRAMC_READ_REG(0x100));
            /*enable P drive calibration*/
            tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xfffdf0f0|0x000500f0);
            //tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xfffef0f0|0x000600f0);
            DRAMC_WRITE_REG(tmp,DRAMC_DLLSEL) ;
            print("2.enable P drive (initial settings),DRAMC_DLLSEL:%xh\n",DRAMC_READ_REG(DRAMC_DLLSEL));
            /*Start P drive calibration*/
            for(drvp = 0 ; drvp <=15; drvp ++){
                tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xffff0fff)|(drvp << 12);
                DRAMC_WRITE_REG(tmp,DRAMC_DLLSEL); 
                print("2.1.DRAMC_DLLSEL, CMPDRVP 0x0c0[15:12]:%xh\n",tmp);
                //delay_a_while(10000000);
                //gpt_busy_wait_us(1000);
                gpt_busy_wait_ms(1);

                /* check the 3dc[31] from 0 to 1 */
                print("2.2.CMPOT:0x3dc[31]:%xh\n", DRAMC_READ_REG(0x3dc));
                if ((DRAMC_READ_REG(0x3dc) >> 31)  == 1){
                    print("P drive:%d\n",drvp);
                    break;
                }
            }
            if (drvp == 16)
                print("No valid P drive");
    /* 3.DRVN calibration */
            /*enable N drive calibration*/
            /* setup SEL,*/
            tmp = (DRAMC_READ_REG(0x100)&0xfffcffff)|(drvn_sel << 16);
            DRAMC_WRITE_REG(tmp,0x100) ;
            print("3.INTREF_SEL:0x100[17:16]:%xh\n",DRAMC_READ_REG(0x100));
            /* Enable N drive calibration */
            tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xfffbffff)|(0x0002000F);
            //tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xfffbffff)|(0x0001000F);
            DRAMC_WRITE_REG(tmp,DRAMC_DLLSEL) ;
            print("4.enable N drive (initial settings),DRAMC_DLLSEL:%xh\n",tmp);
            
            /*Start N drive calibration*/
            for(drvn = 0 ; drvn <=15; drvn ++){
                tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xfffff0ff)|(drvn<<8);
                DRAMC_WRITE_REG(tmp,DRAMC_DLLSEL); 
                print("4.1.DRAMC_DLLSEL, CMPDRVN 0x0c0[11:8]:%xh\n",tmp);
                //delay_a_while(10000000);
                //gpt_busy_wait_us(1000);
                gpt_busy_wait_ms(1);
                
                /* check the 3dc[31] from 1 to 0 */
                print("4.2.CMPOT:0x3dc[31]:%xh\n", DRAMC_READ_REG(0x3dc));
                if ((DRAMC_READ_REG(0x3dc) >> 31)  == 0){
                    /* fixup the drvn by minus 1 */
                    if (drvn > 0)
                       drvn--;
                    print("N drive:%d\n",drvn);
                    break;
                }
            }
            if (drvn == 16) {
                drvn = 15;
                print("No valid N drive");
            }

    }
    print("drvp=0x%x,drvn=0x%x\n",drvp,drvn);
    return;

}
#endif


