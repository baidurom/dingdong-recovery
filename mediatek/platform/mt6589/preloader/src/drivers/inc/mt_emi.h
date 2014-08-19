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

#ifndef MT_EMI_H
#define MT_EMI_H

extern void mt6516_set_emi (void);
extern void mt6516_256M_mem_setting (void);
int get_dram_rank_nr (void);
int get_dram_rank_size (int dram_rank_size[]);
#define DDR1  1
#define DDR2  2
#define DDR3  3
typedef struct
{
    int   sub_version;            // sub_version: 0x1 for new version
    int  type;                /* 0x0000 : Invalid
                                 0x0001 : Discrete DDR1
                                 0x0002 : Discrete DDR2
                                 0x0003 : Discrete DDR3
                                 0x0101 : MCP(NAND+DDR1)
                                 0x0102 : MCP(NAND+DDR2)
                                 0x0103 : MCP(NAND+DDR3)
                                 0x0201 : MCP(eMMC+DDR1)
                                 0x0202 : MCP(eMMC+DDR2)
                                 0x0203 : MCP(eMMC+DDR3)
                              */
    int   id_length;              // EMMC and NAND ID checking length
    int   fw_id_length;              // FW ID checking length
    char  ID[16];
    char  fw_id[8];               // To save fw id
    int   EMI_CONA_VAL;           //@0x3000
    int   DRAMC_DRVCTL0_VAL;      //@0x40B8               -> customized TX I/O driving
    int   DRAMC_DRVCTL1_VAL;      //@0x40BC               -> customized TX I/O driving
    int   DRAMC_DLE_VAL;          //40E4[4]:407c[6:4]     -> customized
    int   DRAMC_ACTIM_VAL;        //@0x4000
    int   DRAMC_GDDR3CTL1_VAL;    //@0x40F4
    int   DRAMC_CONF1_VAL;        //@0x4004
    int   DRAMC_DDR2CTL_VAL;      //@0x407C 
    int   DRAMC_TEST2_3_VAL;      //@0x4044
    int   DRAMC_CONF2_VAL;        //@0x4008
    int   DRAMC_PD_CTRL_VAL;      //@0x41DC
    int   DRAMC_PADCTL3_VAL;      //@0x4014               -> customized TX DQS delay
    int   DRAMC_DQODLY_VAL;       //@0x4200~0x420C        -> customized TX DQ delay
    int   DRAMC_ADDR_OUTPUT_DLY;  // for E1 DDR2 only
    int   DRAMC_CLK_OUTPUT_DLY;   // for E1 DDR2 only
    int   DRAMC_ACTIM1_VAL;       //@0x41E8
    int   DRAMC_MISCTL0_VAL;      //@0x40FC
    int   DRAM_RANK_SIZE[4];
    int   MMD;           // MMD info
    int   reserved[8];

    union
    {
        struct
        {
            int   DDR2_MODE_REG_1;
            int   DDR2_MODE_REG_2;
            int   DDR2_MODE_REG_3;
            int   DDR2_MODE_REG_10;
            int   DDR2_MODE_REG_63;
            int   DDR2_MODE_REG_5;
        };
        struct
        {
            int   DDR1_MODE_REG;
            int   DDR1_EXT_MODE_REG;
        };
        struct
        {
            int   DDR3_MODE_REG0;
            int   DDR3_MODE_REG1;
            int   DDR3_MODE_REG2;
            int   DDR3_MODE_REG3;
        };
    };
} EMI_SETTINGS;
int mt_get_dram_type (void); 
/* 0: invalid */
/* 1: mDDR1 */
/* 2: mDDR2 */
/* 3: mDDR3 */
#endif
