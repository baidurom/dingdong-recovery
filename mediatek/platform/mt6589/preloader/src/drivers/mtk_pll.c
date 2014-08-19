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

#include "typedefs.h"
#include "platform.h"

#include "mt6589.h"
#include "mtk_pll.h"
#include "mtk_timer.h"

unsigned int mt_get_bus_freq(void)
{
    kal_uint32 bus_clk = 26000;

    #if !CFG_FPGA_PLATFORM

    kal_uint32 mainpll_con0, mainpll_con1, main_diff;
    kal_uint32 clk_cfg_0, output_freq;

    clk_cfg_0 = DRV_Reg32(CLK_CFG_0);

    mainpll_con0 = DRV_Reg32(MAINPLL_CON0);
    mainpll_con1 = DRV_Reg32(MAINPLL_CON1);

    main_diff = ((mainpll_con1 >> 12) - 0x8009A) / 2;

    if ((mainpll_con0 & 0xFF) == 0x01)
    {
        output_freq = 1001 + (main_diff * 13); // Mhz
    }
    else // if (mainpll_con0 & 0xFF) == 0x41)
    {
        output_freq = 1001 + (main_diff * 13) / 2; // Mhz
    }

    if ((clk_cfg_0 & 0x7) == 1) // SYSPLL_D3 = MAINPLL / 3 / 2
    {
        bus_clk = ((output_freq * 1000) / 3) / 2;
    }
    else if ((clk_cfg_0 & 0x7) == 2) // SYSPLL_D4 = MAINPLL / 2 / 4
    {
        bus_clk = ((output_freq * 1000) / 2) / 4;
    }
    else if ((clk_cfg_0 & 0x7) == 3) // SYSPLL_D6 = MAINPLL /2 / 6
    {
        bus_clk = ((output_freq * 1000) / 2) / 6;
    }
    else if ((clk_cfg_0 & 0x7) == 4) // UNIVPLL_D5 = UNIVPLL / 5
    {
        bus_clk = (1248 * 1000) / 5;
    }
    else if ((clk_cfg_0 & 0x7) == 5) // UNIVPLL2_D2 = UNIVPLL / 3 / 2
    {
        bus_clk = (1248 * 1000) / 3 / 2;
    }
    else // CLKSQ
    {
        bus_clk = 26 * 1000;
    }

    #endif

    return bus_clk; // Khz
}

#define DRAMC_WRITE_REG(val,offset) \
        do { \
            (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) = (unsigned int)(val); \
            (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) = (unsigned int)(val); \
        } while(0)

/***********************
 * MEMPLL Configuration
 ***********************/

#define r_bias_en_stb_time            (0x00000000 << 24)
#define r_bias_lpf_en_stb_time        (0x00000000 << 16)
#define r_mempll_en_stb_time          (0x00000000 << 8)
#define r_dmall_ck_en_stb_time        (0x00000000 << 0)

#define r_dds_en_stb_time             (0x00000000 << 24)
#define r_div_en_stb_time             (0x00000000 << 16)
#define r_dmpll2_ck_en_stb_time       (0x00000000 << 8)
#define r_iso_en_stb_time             (0x00000000 << 0)

#define r_bias_en_stb_dis             (0x00000001 << 28)
#define r_bias_en_src_sel             (0x00000001 << 24)
#define r_bias_lpf_en_stb_dis         (0x00000001 << 20)
#define r_bias_lpf_en_src_sel         (0x00000001 << 16)
#define r_mempll4_en_stb_dis          (0x00000001 << 15)
#define r_mempll3_en_stb_dis          (0x00000001 << 14)
#define r_mempll2_en_stb_dis          (0x00000001 << 13)
#define r_mempll_en_stb_dis           (0x00000001 << 12)
#define r_mempll4_en_src_sel          (0x00000001 << 11)
#define r_mempll3_en_src_sel          (0x00000001 << 10)
#define r_mempll2_en_src_sel          (0x00000001 << 9)
#define r_mempll_en_src_sel           (0x00000001 << 8)
#define r_dmall_ck_en_stb_dis         (0x00000001 << 4)
#define r_dmall_ck_en_src_sel         (0x00000001 << 0)

#define r_dds_en_stb_dis              (0x00000001 << 28)
#define r_dds_en_src_sel              (0x00000001 << 24)
#define r_div_en_stb_dis              (0x00000001 << 20)
#define r_div_en_src_sel              (0x00000001 << 16)
#define r_dmpll2_ck_en_stb_dis        (0x00000001 << 12)
#define r_dmpll2_ck_en_src_sel        (0x00000001 << 8)
#define r_iso_en_stb_dis              (0x00000001 << 4)
#define r_iso_en_src_sel              (0x00000001 << 0)

#define r_dmbyp_pll4                  (0x00000001 << 0)
#define r_dmbyp_pll3                  (0x00000001 << 1)
#define r_dm1pll_sync_mode            (0x00000001 << 2)
#define r_dmall_ck_en                 (0x00000001 << 4)
#define r_dmpll2_clk_en               (0x00000001 << 5)

#ifdef MEMPLL_CLK_500 //DDR1000
    unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
    unsigned int pllc1_blp                     = 0x00000001 << 12;
    unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
    unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x0000004e << 25;
    unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x00d89d89 <<  1;
    unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
#else
    #ifdef MEMPLL_CLK_400 //DDR800
        unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
        unsigned int pllc1_blp                     = 0x00000001 << 12;
        unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
        unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x0000004e << 25;
        unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x00d89d89 <<  1;
        unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
    #else
        #ifdef MEMPLL_CLK_250 //DDR500
            unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
            unsigned int pllc1_blp                     = 0x00000001 << 12;
            unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
            unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x0000004e << 25;
            unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x00d89d89 <<  1;
            unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
        #else
            #ifdef MEMPLL_CLK_266 //DDR533
                unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
                unsigned int pllc1_blp                     = 0x00000001 << 12;
                unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
                unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x00000046 << 25;
                unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x000aaaaa <<  1;
                unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
            #else
                #ifdef MEMPLL_CLK_150 //DDR300
                    unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
                    unsigned int pllc1_blp                     = 0x00000001 << 12;
                    unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
                    unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x0000004e << 25;
                    unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x00d89d89 <<  1;
                    unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
                #else //DDR1172.6
                    #ifdef MEMPLL_CLK_586
                        unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
                        unsigned int pllc1_blp                     = 0x00000001 << 12;
                        unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
                        unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x0000004d << 25;
                        unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x000bbbbb <<  1;
                        unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
                    #else //DDR1066
                        unsigned int pllc1_prediv_1_0_n1           = 0x00000000 << 16;
                        unsigned int pllc1_blp                     = 0x00000001 << 12;
                        unsigned int pllc1_fbksel_1_0_n3           = 0x00000000 << 26;
                        unsigned int pllc1_dmss_pcw_ncpo_30_24_n4  = 0x0000004c << 25;
                        unsigned int pllc1_dmss_pcw_ncpo_23_0_n4   = 0x0068ba2e <<  1;
                        unsigned int pllc1_postdiv_1_0_n5          = 0x00000000 << 14;
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif

#define pllc1_dmss_pcw_ncpo_chg       (0x00000001 << 0)
#define pllc1_mempll_div_en           (0x00000001 <<24)
#define pllc1_mempll_div_6_0          (0x00000052 <<25)
#ifdef BYPASS_MEMPLL1
    #define pllc1_mempll_refck_en     (0x00000000 <<13)
#else
    #define pllc1_mempll_refck_en     (0x00000001 <<13)
#endif
#define pllc1_mempll_top_reserve_2_0  (0x00000000 <<16)
#define pllc1_mempll_bias_en          (0x00000001 <<14)
#define pllc1_mempll_bias_lpf_en      (0x00000001 <<15)
#define pllc1_mempll_en               (0x00000001 << 2)
#define pllc1_dmss_ncpo_en            (0x00000001 << 4)
#define pllc1_dmss_fifo_start_man     (0x00000001 <<11)
#define pllc1_mempll_dds_en           (0x00000001 <<25)

#define mempll2_prediv_1_0            (0x00000000 << 0)
#define mempll2_vco_div_sel           (0x00000000 <<29)

#ifdef MEMPLL_CLK_500 //DDR1000
    unsigned int mempll2_m4pdiv_1_0    = (0x00000001 << 10);
    #ifdef DDRPHY_3PLL_MODE
        unsigned int mempll2_fbdiv_6_0 = (0x00000009 <<  2);
    #else  //DDRPHY_3PLL_MODE
        unsigned int mempll2_fbdiv_6_0 = (0x00000050 <<  2);
    #endif //DDRPHY_3PLL_MODE
#else
    #ifdef MEMPLL_CLK_400 //DDR800
        unsigned int mempll2_m4pdiv_1_0    = (0x00000001 << 10);
        #ifdef DDRPHY_3PLL_MODE
            unsigned int mempll2_fbdiv_6_0 = (0x00000007 <<  2);
        #else //DDRPHY_3PLL_MODE
            unsigned int mempll2_fbdiv_6_0 = (0x00000040 <<  2);
        #endif //DDRPHY_3PLL_MODE
    #else
        #ifdef MEMPLL_CLK_250 //DDR500
        unsigned int mempll2_m4pdiv_1_0        = (0x00000002 << 10);
            #ifdef DDRPHY_3PLL_MODE
                unsigned int mempll2_fbdiv_6_0 = (0x00000004 <<  2);
            #else //DDRPHY_3PLL_MODE
                unsigned int mempll2_fbdiv_6_0 = (0x00000050 <<  2);
            #endif //DDRPHY_3PLL_MODE
        #else
            #ifdef MEMPLL_CLK_266 //DDR533
                unsigned int mempll2_m4pdiv_1_0    = (0x00000002 << 10);
                #ifdef DDRPHY_3PLL_MODE
                    unsigned int mempll2_fbdiv_6_0 = (0x00000005 <<  2);
                #else //DDRPHY_3PLL_MODE
                    unsigned int mempll2_fbdiv_6_0 = (0x00000060 <<  2);
                #endif //DDRPHY_3PLL_MODE
            #else //DDR1066
                #ifdef MEMPLL_CLK_150 //DDR300
                    unsigned int mempll2_m4pdiv_1_0    = (0x00000002 << 10);
                    #ifdef DDRPHY_3PLL_MODE
                        unsigned int mempll2_fbdiv_6_0 = (0x00000002 <<  2);
                    #else //DDRPHY_3PLL_MODE
                        unsigned int mempll2_fbdiv_6_0 = (0x00000060 <<  2);
                    #endif //DDRPHY_3PLL_MODE
                #else
                    #ifdef MEMPLL_CLK_586 //DDD1172.6
                        unsigned int mempll2_m4pdiv_1_0    = (0x00000001 << 10);
                        #ifdef DDRPHY_3PLL_MODE
                            unsigned int mempll2_fbdiv_6_0 = (0x0000000b <<  2);
                        #else //DDRPHY_3PLL_MODE
                            unsigned int mempll2_fbdiv_6_0 = (0x00000058 <<  2);
                        #endif //DDRPHY_3PLL_MODE
                    #else //DDR1066
                        unsigned int mempll2_m4pdiv_1_0    = (0x00000001 << 10);
                        #ifdef DDRPHY_3PLL_MODE
                            unsigned int mempll2_fbdiv_6_0 = (0x0000000a <<  2);
                        #else //DDRPHY_3PLL_MODE
                            unsigned int mempll2_fbdiv_6_0 = (0x00000058 <<  2);
                        #endif //DDRPHY_3PLL_MODE
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif

#ifdef DDRPHY_3PLL_MODE
    unsigned int mempll2_fb_mck_sel = (0x00000001 << 9);
#else  //DDRPHY_3PLL_MODE
    unsigned int mempll2_fb_mck_sel = (0x00000000 << 9);
#endif //DDRPHY_3PLL_MODE

#define mempll2_fbksel_1_0     (0x00000000 << 10)
#define mempll2_posdiv_1_0     (0x00000000 << 30)
#define mempll2_ref_dl_4_0     (0x00000000 << 27)
#define mempll2_fb_dl_4_0      (0x00000000 << 22)
#define mempll2_en             (0x00000001 << 18)

#define mempll3_prediv_1_0     (0x00000000 <<  0)
#define mempll3_vco_div_sel    (0x00000000 << 29)

#ifdef MEMPLL_CLK_500 //DDR1000
        unsigned int mempll3_m4pdiv_1_0 = (0x00000001 << 10);
    #ifdef DDRPHY_3PLL_MODE
        unsigned int mempll3_fbdiv_6_0  = (0x00000009 <<  2);
    #else //DDRPHY_3PLL_MODE
        unsigned int mempll3_fbdiv_6_0  = (0x00000050 <<  2);
    #endif //DDRPHY_3PLL_MODE
#else
    #ifdef MEMPLL_CLK_400 //DDR800
        unsigned int mempll3_m4pdiv_1_0    = (0x00000001 << 10);
        #ifdef DDRPHY_3PLL_MODE
            unsigned int mempll3_fbdiv_6_0 = (0x00000007 <<  2);
        #else //DDRPHY_3PLL_MODE
            unsigned int mempll3_fbdiv_6_0 = (0x00000040 <<  2);
        #endif //DDRPHY_3PLL_MODE
    #else
        #ifdef MEMPLL_CLK_250 //DDR500
            unsigned int mempll3_m4pdiv_1_0    = (0x00000002 << 10);
            #ifdef DDRPHY_3PLL_MODE
                unsigned int mempll3_fbdiv_6_0 = (0x00000004 <<  2);
            #else //DDRPHY_3PLL_MODE
                unsigned int mempll3_fbdiv_6_0 = (0x00000050 <<  2);
            #endif //DDRPHY_3PLL_MODE
        #else
            #ifdef MEMPLL_CLK_266 //DDR533
                unsigned int mempll3_m4pdiv_1_0    = (0x00000002 << 10);
                #ifdef DDRPHY_3PLL_MODE
                    unsigned int mempll3_fbdiv_6_0 = (0x00000005 <<  2);
                #else //DDRPHY_3PLL_MODE
                    unsigned int mempll3_fbdiv_6_0 = (0x00000060 <<  2);
                #endif //DDRPHY_3PLL_MODE
            #else
                #ifdef MEMPLL_CLK_150 //DDR300
                    unsigned int mempll3_m4pdiv_1_0    = (0x00000002 << 10);
                    #ifdef DDRPHY_3PLL_MODE
                        unsigned int mempll3_fbdiv_6_0 = (0x00000002 <<  2);
                    #else //DDRPHY_3PLL_MODE
                        unsigned int mempll3_fbdiv_6_0 = (0x00000060 <<  2);
                    #endif //DDRPHY_3PLL_MODE
                #else
                    #ifdef MEMPLL_CLK_586 //DDD1172.6
                        unsigned int mempll3_m4pdiv_1_0    = (0x00000001 << 10);
                        #ifdef DDRPHY_3PLL_MODE
                            unsigned int mempll3_fbdiv_6_0 = (0x0000000b <<  2);
                        #else //DDRPHY_3PLL_MODE
                            unsigned int mempll3_fbdiv_6_0 = (0x00000058 <<  2);
                        #endif //DDRPHY_3PLL_MODE
                    #else //DDR1066
                        unsigned int mempll3_m4pdiv_1_0    = (0x00000001 << 10);
                        #ifdef DDRPHY_3PLL_MODE
                            unsigned int mempll3_fbdiv_6_0 = (0x0000000a <<  2);
                        #else //DDRPHY_3PLL_MODE
                            unsigned int mempll3_fbdiv_6_0 = (0x00000058 <<  2);
                        #endif //DDRPHY_3PLL_MODE
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif

#ifdef DDRPHY_3PLL_MODE
    unsigned int mempll3_fb_mck_sel = (0x00000001 << 9);
#else  //DDRPHY_3PLL_MODE
    unsigned int mempll3_fb_mck_sel = (0x00000000 << 9);
#endif //DDRPHY_3PLL_MODE

#define mempll3_fbksel_1_0   (0x00000000 << 10)
#define mempll3_posdiv_1_0   (0x00000000 << 30)
#define mempll3_ref_dl_4_0   (0x00000000 << 27)
#define mempll3_fb_dl_4_0    (0x00000000 << 22)
#define mempll3_en           (0x00000001 << 18)

#define mempll4_prediv_1_0   (0x00000000 <<  0)
#define mempll4_vco_div_sel  (0x00000000 << 29)

#ifdef MEMPLL_CLK_500 //DDR1000
    unsigned int mempll4_m4pdiv_1_0    = (0x00000001 << 10);
    #ifdef DDRPHY_3PLL_MODE
        unsigned int mempll4_fbdiv_6_0 = (0x00000009 <<  2);
    #else //DDRPHY_3PLL_MODE
        unsigned int mempll4_fbdiv_6_0 = (0x00000050 <<  2);
    #endif //DDRPHY_3PLL_MODE
#else
    #ifdef MEMPLL_CLK_400 //DDR800
        unsigned int mempll4_m4pdiv_1_0    = (0x00000001 << 10);
        #ifdef DDRPHY_3PLL_MODE
            unsigned int mempll4_fbdiv_6_0 = (0x00000007 <<  2);
        #else //DDRPHY_3PLL_MODE
            unsigned int mempll4_fbdiv_6_0 = (0x00000040 <<  2);
        #endif //DDRPHY_3PLL_MODE
    #else
        #ifdef MEMPLL_CLK_250 //DDR500
            unsigned int mempll4_m4pdiv_1_0    = (0x00000002 << 10);
            #ifdef DDRPHY_3PLL_MODE
                unsigned int mempll4_fbdiv_6_0 = (0x00000004 <<  2);
            #else //DDRPHY_3PLL_MODE
                unsigned int mempll4_fbdiv_6_0 = (0x00000050 <<  2);
            #endif //DDRPHY_3PLL_MODE
        #else
            #ifdef MEMPLL_CLK_266 //DDR533
                unsigned int mempll4_m4pdiv_1_0    = (0x00000002 << 10);
                #ifdef DDRPHY_3PLL_MODE
                    unsigned int mempll4_fbdiv_6_0 = (0x00000005 <<  2);
                #else //DDRPHY_3PLL_MODE
                    unsigned int mempll4_fbdiv_6_0 = (0x00000060 <<  2);
                #endif //DDRPHY_3PLL_MODE
            #else //DDR1066
                #ifdef MEMPLL_CLK_150 //DDR300
                    unsigned int mempll4_m4pdiv_1_0    = (0x00000002 << 10);
                    #ifdef DDRPHY_3PLL_MODE
                        unsigned int mempll4_fbdiv_6_0 = (0x00000002 <<  2);
                    #else //DDRPHY_3PLL_MODE
                        unsigned int mempll4_fbdiv_6_0 = (0x00000060 <<  2);
                    #endif //DDRPHY_3PLL_MODE
                #else
                    #ifdef MEMPLL_CLK_586 //DDD1172.6
                        unsigned int mempll4_m4pdiv_1_0    = (0x00000001 << 10);
                        #ifdef DDRPHY_3PLL_MODE
                            unsigned int mempll4_fbdiv_6_0 = (0x0000000b <<  2);
                        #else //DDRPHY_3PLL_MODE
                            unsigned int mempll4_fbdiv_6_0 = (0x00000058 <<  2);
                        #endif //DDRPHY_3PLL_MODE
                    #else //DDR1066
                        unsigned int mempll4_m4pdiv_1_0    = (0x00000001 << 10);
                        #ifdef DDRPHY_3PLL_MODE
                            unsigned int mempll4_fbdiv_6_0 = (0x0000000a <<  2);
                        #else //DDRPHY_3PLL_MODE
                            unsigned int mempll4_fbdiv_6_0 = (0x00000058 <<  2);
                        #endif //DDRPHY_3PLL_MODE
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif

#ifdef DDRPHY_3PLL_MODE
    unsigned int mempll4_fb_mck_sel = (0x00000001 << 9);
#else  //DDRPHY_3PLL_MODE
    unsigned int mempll4_fb_mck_sel = (0x00000000 << 9);
#endif //DDRPHY_3PLL_MODE

#define mempll4_fbksel_1_0  (0x00000000 << 10)
#define mempll4_posdiv_1_0  (0x00000000 << 30)
#define mempll4_ref_dl_4_0  (0x00000000 << 27)
#define mempll4_fb_dl_4_0   (0x00000000 << 22)
#define mempll4_en          (0x00000001 << 18)

void mt_mempll_init(void)
{
    unsigned int temp;

    /****************************************
     * (1) Setup DDRPHY operation mode
     ****************************************/

    #ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            temp = (0x00000020 | r_dmpll2_clk_en); // 3PLL mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #else
            temp = (0x00000003); // 1PLL async mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #endif
    #else
        #ifdef DDRPHY_3PLL_MODE
            temp = (0x00000020 | r_dmpll2_clk_en); // 3PLL sync mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #else
            temp = (0x00000007); // 1PLL sync mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #endif
    #endif

    /*******************************************************************************************
     * (2) Setup MEMPLL operation case & frequency, May set according to dram type & frequency
     *******************************************************************************************/

    temp = r_bias_en_stb_time | r_bias_lpf_en_stb_time | r_mempll_en_stb_time | r_dmall_ck_en_stb_time;
    DRAMC_WRITE_REG(temp, (0x0170 << 2));

    temp = r_dds_en_stb_time | r_div_en_stb_time | r_dmpll2_ck_en_stb_time | r_iso_en_stb_time;
    DRAMC_WRITE_REG(temp, (0x0171 << 2));

    temp = r_bias_en_stb_dis | r_bias_en_src_sel | r_bias_lpf_en_stb_dis | r_bias_lpf_en_src_sel | r_mempll4_en_stb_dis | r_mempll3_en_stb_dis |
           r_mempll2_en_stb_dis | r_mempll_en_stb_dis| r_mempll4_en_src_sel | r_mempll3_en_src_sel | r_mempll2_en_src_sel | r_mempll_en_src_sel |
           r_dmall_ck_en_stb_dis | r_dmall_ck_en_src_sel;
    DRAMC_WRITE_REG(temp, (0x0172 << 2));

    temp = r_dds_en_stb_dis | r_dds_en_src_sel | r_div_en_stb_dis| r_div_en_src_sel | r_dmpll2_ck_en_stb_dis | r_dmpll2_ck_en_src_sel |
           r_iso_en_stb_dis | r_iso_en_src_sel | r_dmbyp_pll4 | r_dmbyp_pll3 | r_dm1pll_sync_mode | r_dmall_ck_en | r_dmpll2_clk_en;
    DRAMC_WRITE_REG(temp, (0x0173 << 2));

    temp = pllc1_prediv_1_0_n1  | pllc1_blp | pllc1_fbksel_1_0_n3 | pllc1_postdiv_1_0_n5;
    DRAMC_WRITE_REG(temp, (0x0180 << 2));

    temp = pllc1_mempll_div_6_0 | pllc1_mempll_refck_en;
    DRAMC_WRITE_REG(temp, (0x0181 << 2));

    temp = mempll2_vco_div_sel | mempll2_m4pdiv_1_0 | mempll2_fb_mck_sel | mempll2_posdiv_1_0;
    DRAMC_WRITE_REG(temp, (0x0183 << 2));

    temp = pllc1_mempll_top_reserve_2_0 | mempll2_prediv_1_0 | mempll2_fbdiv_6_0 | mempll2_fbksel_1_0;
    DRAMC_WRITE_REG(temp, (0x0182 << 2));

    temp = mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel | mempll3_posdiv_1_0;
    DRAMC_WRITE_REG(temp, (0x0185 << 2));

    temp = mempll2_ref_dl_4_0 | mempll2_fb_dl_4_0 | mempll3_prediv_1_0 | mempll3_fbdiv_6_0 | mempll3_fbksel_1_0;
    DRAMC_WRITE_REG(temp, (0x0184 << 2));

    temp = mempll4_vco_div_sel | mempll4_m4pdiv_1_0 | mempll4_fb_mck_sel | mempll4_posdiv_1_0;
    DRAMC_WRITE_REG(temp, (0x0187 << 2));

    temp = mempll3_fb_dl_4_0 | mempll3_ref_dl_4_0 | mempll4_prediv_1_0 | mempll4_fbdiv_6_0 | mempll4_fbksel_1_0;
    DRAMC_WRITE_REG(temp, (0x0186 << 2));

    temp = mempll4_ref_dl_4_0 | mempll4_fb_dl_4_0;
    DRAMC_WRITE_REG(temp, (0x0188 << 2));

    temp = pllc1_dmss_pcw_ncpo_23_0_n4 | pllc1_dmss_pcw_ncpo_30_24_n4  | pllc1_dmss_pcw_ncpo_chg;
    DRAMC_WRITE_REG(temp, (0x0189 << 2));

    DRAMC_WRITE_REG(0x003F0000, (0x018C << 2)); // SDM PLL tie high default value

    /*************************************
     * (3) Setup MEMPLL power on sequence
     *************************************/

    gpt_busy_wait_us(5); // min delay is 2us

    temp = pllc1_mempll_div_6_0 | pllc1_mempll_refck_en | (pllc1_mempll_bias_en );
    DRAMC_WRITE_REG(temp, (0x0181 << 2));

    gpt_busy_wait_us(5); // min delay is 1us

    temp = pllc1_mempll_div_6_0 | pllc1_mempll_refck_en | (pllc1_mempll_bias_en | pllc1_mempll_bias_lpf_en);
    DRAMC_WRITE_REG(temp, (0x0181 << 2));

    gpt_busy_wait_ms(5); // min delay is 1ms

    temp = pllc1_prediv_1_0_n1  | pllc1_blp | pllc1_fbksel_1_0_n3 | pllc1_postdiv_1_0_n5  | (pllc1_mempll_en);
    DRAMC_WRITE_REG(temp, (0x0180 << 2));

    gpt_busy_wait_us(50); // min delay is 20us

    temp = 0x10000000;
    DRAMC_WRITE_REG(temp, (0x018a << 2));

    gpt_busy_wait_us(5); // min delay is 2us

    temp = 0x18000000;
    DRAMC_WRITE_REG(temp, (0x018a << 2));

    temp = mempll4_ref_dl_4_0 | mempll4_fb_dl_4_0 | (pllc1_dmss_ncpo_en | pllc1_dmss_fifo_start_man);
    DRAMC_WRITE_REG(temp, (0x0188 << 2));

    gpt_busy_wait_us(5); // min delay is 2us

    temp = pllc1_mempll_dds_en | (pllc1_prediv_1_0_n1  | pllc1_blp | pllc1_fbksel_1_0_n3 | pllc1_postdiv_1_0_n5  | (pllc1_mempll_en));
    DRAMC_WRITE_REG(temp, (0x0180 << 2));

    temp = pllc1_mempll_div_en | (pllc1_mempll_div_6_0 | pllc1_mempll_refck_en | (pllc1_mempll_bias_en | pllc1_mempll_bias_lpf_en));
    DRAMC_WRITE_REG(temp, (0x0181 << 2));

    gpt_busy_wait_us(50); // min delay is 23us

    temp = mempll2_en | mempll2_vco_div_sel | mempll2_m4pdiv_1_0 | mempll2_fb_mck_sel | mempll2_posdiv_1_0;
    DRAMC_WRITE_REG(temp, (0x0183 << 2));

    temp = mempll3_en | mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel | mempll3_posdiv_1_0;
    DRAMC_WRITE_REG(temp, (0x0185 << 2));

    temp = mempll4_en | mempll4_vco_div_sel | mempll4_m4pdiv_1_0 | mempll4_fb_mck_sel | mempll4_posdiv_1_0;
    DRAMC_WRITE_REG(temp, (0x0187 << 2));

    gpt_busy_wait_us(30); // min delay is 23us

    #ifdef DRAMC_ASYNC
        #ifdef DDRPHY_3PLL_MODE
            temp = 0x00003f20 | r_dmpll2_clk_en | r_dmall_ck_en; // 3PLL mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #else
            temp = 0x00003f03 | r_dmpll2_clk_en | r_dmall_ck_en; // 1PLL async mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #endif
    #else
        #ifdef DDRPHY_3PLL_MODE
            temp = 0x00003f20 | r_dmpll2_clk_en | r_dmall_ck_en; // 3PLL sync mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #else
            temp = 0x00003f07 | r_dmpll2_clk_en | r_dmall_ck_en; // 1PLL sync mode
            DRAMC_WRITE_REG(temp, (0x0190 << 2));
        #endif
    #endif

    /*****************************************************
     * MEMPLL control switch to SPM for ISO_EN zero delay
     *****************************************************/

    #ifdef DDRPHY_3PLL_MODE
        temp = 0x00101010; // [0]ISO_EN_SRC=0, [22]DIV_EN_SC_SRC=0 (pll2off), [20]DIV_EN_DLY=1 (no delay), [16]DIV_EN_SRC=0, [8]PLL2_CK_EN_SRC=1(1pll), [8]PLL2_CK_EN_SRC=0(3pll)
        DRAMC_WRITE_REG(temp, (0x0173 << 2));
    #else
        temp = 0x00101010; // [0]ISO_EN_SRC=0, [22]DIV_EN_SC_SRC=0 (pll2off), [20]DIV_EN_DLY=1 (no delay), [16]DIV_EN_SRC=0, [8]PLL2_CK_EN_SRC=1(1pll), [8]PLL2_CK_EN_SRC=0(3pll)
        DRAMC_WRITE_REG(temp, (0x0173 << 2));
    #endif

    /***************************************************
     * Setting for MEMPLL_EN control by sc_mempll3_off
     ***************************************************/

    temp = 0x0000F010; // [24]BIAS_EN_SRC=0, [16]BIAS_LPF_EN_SRC=0, [8]MEMPLL_EN, [9][10][11]MEMPLL2,3,4_EN_SRC, [0]ALL_CK_EN_SRC=0
    DRAMC_WRITE_REG(temp, (0x0172 << 2));

    temp = 0x00021B96; // Setting for delay time, BIAS_LPF_EN delay time=2T
    DRAMC_WRITE_REG(temp, (0x0170 << 2));
}

int mt_mempll_cali(void)
{
    int one_count = 0, zero_count = 0;
    int pll2_done = 0, pll3_done = 0, pll4_done = 0, ret = 0;

    unsigned int temp = 0, pll2_dl = 0, pll3_dl = 0, pll4_dl = 0;

    /***********************************************
    * 1. Set jitter meter clock to internal FB path
    ************************************************/
    //temp = DRV_Reg32(0x1001160C);
    //DRV_WriteReg32(0x1001160C, temp & ~0x200); // 0x1001160C[9] = 0 PLL2

    //temp = DRV_Reg32(0x10011614);
    //DRV_WriteReg32(0x10011614, temp & ~0x200); // 0x10011614[9] = 0 PLL3

    //temp = DRV_Reg32(0x1001161C);
    //DRV_WriteReg32(0x1001161C, temp & ~0x200); // 0x1001161C[9] = 0 PLL4

    /***********************************************
    * 2. Set jitter meter count number
    ************************************************/

    temp = DRV_Reg32(0x100111CC) & 0x0000FFFF;
    DRV_WriteReg32(0x100111CC, (temp | 0x04000000)); // 0x100111CC[31:16] PLL2 0x400 = 1024 count

    temp = DRV_Reg32(0x100111D0) & 0x0000FFFF;
    DRV_WriteReg32(0x100111D0, (temp | 0x04000000)); // 0x100111D0[31:16] PLL3 0x400 = 1024 count

    temp = DRV_Reg32(0x100111D4) & 0x0000FFFF;
    DRV_WriteReg32(0x100111D4, (temp | 0x04000000)); // 0x100111D4[31:16] PLL4 0x400 = 1024 count

    while(1)
    { 
        /***********************************************
        * 3. Adjust delay chain tap number
        ************************************************/

        if (!pll2_done)
        {
            temp = DRV_Reg32(0x10011610) & ~0xF8000000;
            DRV_WriteReg32(0x10011610, (temp | (0x10 << 27 ))); // 0x10011610[31:27] PLL2 REF 0x10 fix

            temp = DRV_Reg32(0x10011610) & ~0x07C00000;
            DRV_WriteReg32(0x10011610, (temp | (pll2_dl << 22))); // 0x10011610[26:22] PLL2 FB inc 1
        }

        if (!pll3_done)
        {
            temp = DRV_Reg32(0x10011618) & ~0xF8000000;
            DRV_WriteReg32(0x10011618, (temp | (0x10 << 27 ))); // 0x10011618[31:27] PLL3 REF 0x10 fix

            temp = DRV_Reg32(0x10011618) & ~0x07C00000;
            DRV_WriteReg32(0x10011618, (temp | (pll3_dl << 22))); // 0x10011618[26:22] PLL3 FB inc 1
        }

        if (!pll4_done)
        {
            temp = DRV_Reg32(0x10011620) & ~0xF8000000;
            DRV_WriteReg32(0x10011620, (temp | (0x10 << 27 ))); // 0x10011620[31:27] PLL4 REF 0x10 fix

            temp = DRV_Reg32(0x10011620) & ~0x07C00000;
            DRV_WriteReg32(0x10011620, (temp | (pll4_dl << 22))); // 0x10011620[26:22] PLL4 FB inc 1
        }

        /***********************************************
        * 4. Enable jitter meter
        ************************************************/

        if (!pll2_done)
        {
            temp = DRV_Reg32(0x100111CC);
            DRV_WriteReg32(0x100111CC, temp | 0x1); // 0x100111CC[0]=1 PLL2
        }

        if (!pll3_done)
        {
            temp = DRV_Reg32(0x100111D0);
            DRV_WriteReg32(0x100111D0, temp | 0x1); // 0x100111D0[0]=1 PLL3
        }

        if (!pll4_done)
        {
            temp = DRV_Reg32(0x100111D4);
            DRV_WriteReg32(0x100111D4, temp | 0x1); // 0x100111D4[0]=1 PLL4
        }

        gpt_busy_wait_us(40); // wait for jitter meter complete

        /***********************************************
        * 5. Check jitter meter counter value
        ************************************************/

        if (!pll2_done)
        {
            one_count = DRV_Reg32(0x10011320) >> 16; // 0x10011320[31:16] PLL2 one count
            zero_count = DRV_Reg32(0x10011320) & 0x0000FFFF; // 0x10011320[15:0] PLL2 zero count

            if (zero_count > 512)
            {
                //print("[mt_pll_init] PLL2 FB_DL: 0x%x, 1/0 = %d/%d\n", ((DRV_Reg32(0x10011610)& 0x07C00000) >> 22), one_count, zero_count);
                pll2_done = 1;
            }
        }

        if (!pll3_done)
        {
            one_count = DRV_Reg32(0x10011324) >> 16; // 0x10011324[31:16] PLL3 one count
            zero_count = DRV_Reg32(0x10011324) & 0x0000FFFF; // 0x10011324[15:0] PLL3 zero count

            if (zero_count > 512)
            {
                //print("[mt_pll_init] PLL3 FB_DL: 0x%x, 1/0 = %d/%d\n", ((DRV_Reg32(0x10011618) & 0x07C00000) >> 22), one_count, zero_count);
                pll3_done = 1;
            }
        }

        if (!pll4_done)
        {
            one_count = DRV_Reg32(0x10011328) >> 16; // 0x10011328[31:16] PLL4 one count
            zero_count = DRV_Reg32(0x10011328) & 0x0000FFFF; // 0x10011328[15:0] PLL4 zero count

            if (zero_count > 512)
            {
                //print("[mt_pll_init] PLL4 FB_DL: 0x%x, 1/0 = %d/%d\n", ((DRV_Reg32(0x10011620) & 0x07C00000) >> 22), one_count, zero_count);
                pll4_done = 1;
            }
        }

        /***********************************************
        * 6. Reset jitter meter value
        ************************************************/

        if (!pll2_done)
        {
            pll2_dl++;
            temp = DRV_Reg32(0x100111CC);
            DRV_WriteReg32(0x100111CC, temp & ~0x1); // 0x100111CC[0]=0 PLL2
        }

        if (!pll3_done)
        {
            pll3_dl++;
            temp = DRV_Reg32(0x100111D0);
            DRV_WriteReg32(0x100111D0, temp & ~0x1); // 0x100111D0[0]=0 PLL3
        }

        if (!pll4_done)
        {
            pll4_dl++;
            temp = DRV_Reg32(0x100111D4);
            DRV_WriteReg32(0x100111D4, temp & ~0x1); // 0x100111D4[0]=0 PLL4
        }

        /*************************************************************
        * Then return to step 1 to adjust next delay chain tap value.
        * Until we have ~ 50% of one or zero count on jitter meter
        **************************************************************/

        if (pll2_done && pll3_done && pll4_done)
        {
            ret = 0;
            break;
        }

        if (pll2_dl >= 32 || pll3_dl >= 32 || pll4_dl >= 32)
        {
            ret = -1;
            break;
        }
    }

    #if 0
    if (ret != 0)
        print("[mt_pll_init] MEMPLL 3PLL mode calibration fail\n");
    #endif 

    /***********************************************
    * 7. Set jitter meter clock to external FB path
    ************************************************/

    temp = DRV_Reg32(0x1001160C);
    DRV_WriteReg32(0x1001160C, temp | 0x200); // 0x1001160C[9] = 1 PLL2

    temp = DRV_Reg32(0x10011614);
    DRV_WriteReg32(0x10011614, temp | 0x200); // 0x10011614[9] = 1 PLL3

    temp = DRV_Reg32(0x1001161C);
    DRV_WriteReg32(0x1001161C, temp | 0x200); // 0x1001161C[9] = 1 PLL4

    return ret;
}

void mt_pll_post_init(void)
{
    DRV_WriteReg32(INFRA_PDN_SET, 0x00008040);
    DRV_WriteReg32(PERI_PDN0_SET, 0x002C1BFC);
    DRV_WriteReg32(PERI_PDN1_SET, 0x00000008);
}

int mt_pll_init(void)
{
    #if !CFG_FPGA_PLATFORM

    int ret = 0;
    unsigned int temp;

    /* CLKSQ1, CLKSQ2 */
    DRV_WriteReg32(AP_PLL_CON0, 0x00001133); // enable CLKSQ1 low pass filter

    DRV_WriteReg32(AP_PLL_CON0, 0x00001137); // enable CLKSQ2

    gpt_busy_wait_us(200); // wait for CLKSQ2 ready (min delay is 100us)

    DRV_WriteReg32(AP_PLL_CON0, 0x0000113F); // enable CLKSQ2 low pass filter

    /* ARMPLL */
    temp = DRV_Reg32(ARMPLL_PWR_CON0);
    DRV_WriteReg32(ARMPLL_PWR_CON0, temp | 0x1);

    gpt_busy_wait_us(10); // wait for PWR ready (min delay is 2us)

    temp = DRV_Reg32(ARMPLL_PWR_CON0);
    DRV_WriteReg32(ARMPLL_PWR_CON0, temp & ~0x2);

    DRV_WriteReg32(ARMPLL_CON1, 0x8009A000);

    temp = DRV_Reg32(ARMPLL_CON0);
    DRV_WriteReg32(ARMPLL_CON0, temp | 0x1);

    gpt_busy_wait_us(40); // wait for ARMPLL stable (min delay is 20us)

    /* MAINPLL */
    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp | 0x1);

    gpt_busy_wait_us(10); // wait for PWR ready (min delay is 2us)

    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp & ~0x2);

    DRV_WriteReg32(MAINPLL_CON1, 0x800F8000);

    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x1);

    gpt_busy_wait_us(40); // wait for PLL stable (min delay is 20us)

    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x08000000); // DIV_RSTB = 1

    /* MSDCPLL */
    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp | 0x1);

    gpt_busy_wait_us(10); // wait for PWR ready (min delay is 2us)

    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp & ~0x2);

    DRV_WriteReg32(MSDCPLL_CON1, 0x800F0000); // MSDC 195Mhz

 	temp = DRV_Reg32(MSDCPLL_CON0);
    DRV_WriteReg32(MSDCPLL_CON0, temp | 0x1);

    gpt_busy_wait_us(40); // wait for PLL stable (min delay is 20us)

    /* UNIVPLL */
    DRV_WriteReg32(UNIVPLL_CON0, 0xF3003001); // turn on UNIVPLL

    gpt_busy_wait_us(40); // wait for PLL stable (min delay is 20us)

    DRV_WriteReg32(UNIVPLL_CON0, 0xFB003001); // turn on UNIVPLL DIV

    /* MMPLL */
    DRV_WriteReg32(MMPLL_CON0, 0xF0003701);

    gpt_busy_wait_us(40); // wait for PLL stable (min delay is 20us)

    DRV_WriteReg32(MMPLL_CON0, 0xF8003701);

    /* MEMPLL Init */
    mt_mempll_init();

    /* MEMPLL Calibration */
    #ifdef DDRPHY_3PLL_MODE
        ret = mt_mempll_cali();
    #endif

    temp = DRV_Reg32(TOP_DCMCTL);
    DRV_WriteReg32(TOP_DCMCTL, temp | 0x1); // enable infrasys DCM

    DRV_WriteReg32(CLK_MODE, 0x0); // output clock is enabled

    /* Clock Divider */
    DRV_WriteReg32(TOP_CKDIV1, 0x0); // CPU clock divide by 1

    /* Clock Mux */
    temp = DRV_Reg32(TOP_CKMUXSEL) & ~0xC;
    DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x4); // switch CPU clock to ARMPLL

    DRV_WriteReg32(CLK_CFG_0, 0x02070305);

    DRV_WriteReg32(CLK_CFG_1, 0x04020201);

    DRV_WriteReg32(CLK_CFG_2, 0x05050505);

    DRV_WriteReg32(CLK_CFG_3, 0x80808001);

    DRV_WriteReg32(CLK_CFG_4, 0x01040607);

    DRV_WriteReg32(CLK_CFG_6, 0x00010101);

    DRV_WriteReg32(CLK_CFG_7, 0x01010101);

    DRV_WriteReg32(CLK_CFG_8, 0x00010501);

    DRV_WriteReg32(DCM_CFG, 0x0);

    /* HW Control for CLKSQ1, CLKSQ2, ARMPLL, UNIVPLL. SW Conrol MAINPLL, MMPLL */
    DRV_WriteReg32(AP_PLL_CON3, 0x0);
    DRV_WriteReg32(AP_PLL_CON1, 0x020250a0);
    DRV_WriteReg32(AP_PLL_CON2, 0x0000000a);

    /* HW Control */
    temp = DRV_Reg32(CLK_CFG_5);
    DRV_WriteReg32(CLK_CFG_5, temp | 0x3FF);

    /* HW Control */
    temp = DRV_Reg32(CLK_MISC_CFG_2);
    DRV_WriteReg32(CLK_MISC_CFG_2, temp | 0x11);

    #endif

    return ret;
}