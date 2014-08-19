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

#include <asm/arch/mt6575_rtc.h>
#include <asm/arch/mt6575_irq.h>
#include <asm/arch/mt6575_sleep.h>
#include <asm/arch/mt6575_pmic6329.h>

#define MT6575_SC_DEBUG false

#define SC_CLK_CON_SRCLKEN      (1U << 1)
#define SC_CLK_CON_BYPASSDBG    (1U << 8)
#define SC_CLK_CON_CLKREQ2MD    (1U << 25)
#define SC_CLK_CON_3GPLLON      (1U << 31)

#define SC_TIMER_CON_EN         (1U << 0)
#define SC_TIMER_CON_DOWN       (1U << 1)
#define SC_TIMER_CON_DBG        (1U << 2)

#define SC_TIMER_CMD_PAUSE      (1U << 0)
#define SC_TIMER_CMD_WR         (1U << 2)
#define SC_TIMER_CMD_CNTWR      (1U << 13)
#define SC_TIMER_CMD_CONWR      (1U << 15)
#define SC_TIMER_CMD_KEY        (0x6575 << 16)

#define SC_TIMER_STA_CMDCPL     (1U << 1)

/* typical values */
#define SC_SYSCLK_SETTLE        (160)   /* T 32K */
#define SC_PLL_SETTLE           (3)     /* T 32K */
#define SC_PLL2_SETTLE          (3)     /* T 32K */

#define SC_CLK_SETTLE_TYPICAL   ((SC_PLL2_SETTLE << 24) |   \
                                 (SC_PLL_SETTLE << 16) |    \
                                  SC_SYSCLK_SETTLE)

#define SC_CLK_SETTLE_26MON     ((SC_PLL2_SETTLE << 24) |   \
                                 (SC_PLL_SETTLE << 16) |    \
                                  1)

static u32 sc_wake_src = (
    WAKE_SRC_GPT 
);

static void slp_dump_pm_regs(void)
{
    /* PLL registers */
    
    printf("CLKSQ_CON0           0x%x = 0x%x\n", CLKSQ_CON0             , DRV_Reg16(CLKSQ_CON0));
    printf("PLL_CON1             0x%x = 0x%x\n", PLL_CON1               , DRV_Reg16(PLL_CON1));
    printf("PLL_CON2             0x%x = 0x%x\n", PLL_CON2               , DRV_Reg16(PLL_CON2));
    printf("ARMPLL_CON0          0x%x = 0x%x\n", ARMPLL_CON0            , DRV_Reg16(ARMPLL_CON0));
    printf("MAINPLL_CON0         0x%x = 0x%x\n", MAINPLL_CON0           , DRV_Reg16(MAINPLL_CON0));
    printf("IPLL_CON0            0x%x = 0x%x\n", IPLL_CON0              , DRV_Reg16(IPLL_CON0));
    printf("UPLL_CON0            0x%x = 0x%x\n", UPLL_CON0              , DRV_Reg16(UPLL_CON0));
    printf("MDPLL_CON0           0x%x = 0x%x\n", MDPLL_CON0             , DRV_Reg16(MDPLL_CON0));
    printf("WPLL_CON0            0x%x = 0x%x\n", WPLL_CON0              , DRV_Reg16(WPLL_CON0));
    printf("AUDPLL_CON0          0x%x = 0x%x\n", AUDPLL_CON0            , DRV_Reg16(AUDPLL_CON0));
    printf("MEMPLL_CON0          0x%x = 0x%x\n", MEMPLL_CON0            , DRV_Reg16(MEMPLL_CON0));
    
    /* TOPCKGEN/PERICFG registers */
    
    printf("TOP_CKMUXSEL         0x%x = 0x%x\n", TOP_CKMUXSEL           , DRV_Reg32(TOP_CKMUXSEL));
    printf("TOP_DCMCTL           0x%x = 0x%x\n", TOP_DCMCTL             , DRV_Reg32(TOP_DCMCTL));
    printf("TOP_MISC             0x%x = 0x%x\n", TOP_MISC               , DRV_Reg32(TOP_MISC));
    printf("TOP_CKCTL            0x%x = 0x%x\n", TOP_CKCTL              , DRV_Reg32(TOP_CKCTL));
    printf("GLOBALCON_PDN0       0x%x = 0x%x\n", PERI_GLOBALCON_PDN0    , DRV_Reg32(PERI_GLOBALCON_PDN0));
    printf("GLOBALCON_PDN1       0x%x = 0x%x\n", PERI_GLOBALCON_PDN1    , DRV_Reg32(PERI_GLOBALCON_PDN1));
    printf("GLOBALCON_DCMCTL     0x%x = 0x%x\n", PERI_GLOBALCON_DCMCTL  , DRV_Reg32(PERI_GLOBALCON_DCMCTL));
    
    /* SLPCTRL registers */
    
    printf("SC_CLK_SETTLE        0x%x = 0x%x\n", SC_CLK_SETTLE          , DRV_Reg32(SC_CLK_SETTLE));
    printf("SC_PWR_SETTLE        0x%x = 0x%x\n", SC_PWR_SETTLE          , DRV_Reg32(SC_PWR_SETTLE));
    printf("SC_PWR_CON0          0x%x = 0x%x\n", SC_PWR_CON0            , DRV_Reg32(SC_PWR_CON0));
    printf("SC_PWR_CON1          0x%x = 0x%x\n", SC_PWR_CON1            , DRV_Reg32(SC_PWR_CON1));
    printf("SC_PWR_CON2          0x%x = 0x%x\n", SC_PWR_CON2            , DRV_Reg32(SC_PWR_CON2));
    printf("SC_PWR_CON3          0x%x = 0x%x\n", SC_PWR_CON3            , DRV_Reg32(SC_PWR_CON3));
    printf("SC_PWR_CON4          0x%x = 0x%x\n", SC_PWR_CON4            , DRV_Reg32(SC_PWR_CON4));
    printf("SC_PWR_CON5          0x%x = 0x%x\n", SC_PWR_CON5            , DRV_Reg32(SC_PWR_CON5));
    printf("SC_PWR_CON6          0x%x = 0x%x\n", SC_PWR_CON6            , DRV_Reg32(SC_PWR_CON6));
    printf("SC_PWR_CON7          0x%x = 0x%x\n", SC_PWR_CON7            , DRV_Reg32(SC_PWR_CON7));
    printf("SC_PWR_CON8          0x%x = 0x%x\n", SC_PWR_CON8            , DRV_Reg32(SC_PWR_CON8));
    printf("SC_PWR_CON9          0x%x = 0x%x\n", SC_PWR_CON9            , DRV_Reg32(SC_PWR_CON9));
    printf("SC_CLK_CON           0x%x = 0x%x\n", SC_CLK_CON             , DRV_Reg32(SC_CLK_CON));
    printf("SC_MD_CLK_CON        0x%x = 0x%x\n", SC_MD_CLK_CON          , DRV_Reg32(SC_MD_CLK_CON));
    printf("SC_MD_INTF_CON       0x%x = 0x%x\n", SC_MD_INTF_CON         , DRV_Reg32(SC_MD_INTF_CON));
    printf("SC_MD_INTF_STS       0x%x = 0x%x\n", SC_MD_INTF_STS         , DRV_Reg32(SC_MD_INTF_STS));
    printf("SC_TMR_PWR           0x%x = 0x%x\n", SC_TMR_PWR             , DRV_Reg32(SC_TMR_PWR));
    printf("SC_DBG_CON           0x%x = 0x%x\n", SC_DBG_CON             , DRV_Reg32(SC_DBG_CON));
    printf("SC_PERI_CON          0x%x = 0x%x\n", SC_PERI_CON            , DRV_Reg32(SC_PERI_CON));
    printf("SC_STATE             0x%x = 0x%x\n", SC_STATE               , DRV_Reg32(SC_STATE));
    printf("SC_PWR_STA           0x%x = 0x%x\n", SC_PWR_STA             , DRV_Reg32(SC_PWR_STA));
    printf("SC_APMCU_PWRCTL      0x%x = 0x%x\n", SC_APMCU_PWRCTL        , DRV_Reg32(SC_APMCU_PWRCTL));
    printf("SC_AP_DVFS_CON       0x%x = 0x%x\n", SC_AP_DVFS_CON         , DRV_Reg32(SC_AP_DVFS_CON));
    printf("SC_AP_STANBY_EXT     0x%x = 0x%x\n", SC_AP_STANBY_EXT       , DRV_Reg32(SC_AP_STANBY_EXT));
    printf("SC_TIMER_CON         0x%x = 0x%x\n", SC_TIMER_CON           , DRV_Reg32(SC_TIMER_CON));
    printf("SC_TIMER_CMD         0x%x = 0x%x\n", SC_TIMER_CMD           , DRV_Reg32(SC_TIMER_CMD));
    printf("SC_TIMER_STA         0x%x = 0x%x\n", SC_TIMER_STA           , DRV_Reg32(SC_TIMER_STA));
    printf("SC_FINAL_PAUSE       0x%x = 0x%x\n", SC_FINAL_PAUSE         , DRV_Reg32(SC_FINAL_PAUSE));
    printf("SC_PAUSE             0x%x = 0x%x\n", SC_PAUSE               , DRV_Reg32(SC_PAUSE));
    printf("SC_DBG_WAKEUP        0x%x = 0x%x\n", SC_DBG_WAKEUP          , DRV_Reg32(SC_DBG_WAKEUP));
    printf("SC_WAKEUP_SRC        0x%x = 0x%x\n", SC_WAKEUP_SRC          , DRV_Reg32(SC_WAKEUP_SRC));
    printf("SC_WAKEUP_EVENT_MASK 0x%x = 0x%x\n", SC_WAKEUP_EVENT_MASK   , DRV_Reg32(SC_WAKEUP_EVENT_MASK));
    printf("SC_ISR_MASK          0x%x = 0x%x\n", SC_ISR_MASK            , DRV_Reg32(SC_ISR_MASK));
    printf("SC_ISR_STATUS        0x%x = 0x%x\n", SC_ISR_STATUS          , DRV_Reg32(SC_ISR_STATUS));
}

static void slp_pmic_init(void)
{
    if (get_chip_eco_ver() == CHIP_E1) 
    {
        /* 1.10V DVS_VOL_00 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CON8_OFFSET), 
                              (kal_uint8)(0x10),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS00_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS00_SHIFT)
                              );
        
        /* 1.25V DVS_VOL_01 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CON9_OFFSET), 
                              (kal_uint8)(0x16),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS01_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS01_SHIFT)
                              );
        
        /* 1.10V DVS_VOL_10 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CONA_OFFSET), 
                              (kal_uint8)(0x10),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS10_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS10_SHIFT)
                              );
        
        /* 1.25V DVS_VOL_11 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CONB_OFFSET), 
                              (kal_uint8)(0x16),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS11_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS11_SHIFT)
                              );
    }
    else
    {
        /* 0.90V DVS_VOL_00 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CON8_OFFSET), 
                              (kal_uint8)(0x08),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS00_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS00_SHIFT)
                              );
        
        /* 1.00V DVS_VOL_01 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CON9_OFFSET), 
                              (kal_uint8)(0x0C),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS01_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS01_SHIFT)
                              );
        
        /* 1.10V DVS_VOL_10 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CONA_OFFSET), 
                              (kal_uint8)(0x10),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS10_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS10_SHIFT)
                              );
        
        /* 1.25V DVS_VOL_11 */
        pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CONB_OFFSET), 
                              (kal_uint8)(0x16),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS11_MASK),
                              (kal_uint8)(BANK_0_BUCK_VOSEL_DVS11_SHIFT)
                              );    
    }
    
    /* VPROC = 0.9V in sleep mode */
    pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CON6_OFFSET), 
                          (kal_uint8)(0x08),
                          (kal_uint8)(BANK_0_BUCK_VOSEL_SRCLKEN0_MASK),
                          (kal_uint8)(BANK_0_BUCK_VOSEL_SRCLKEN0_SHIFT)
                          );
    
    pmic_config_interface((kal_uint8)(BANK0_VPROC_CON0+BUCK_CONC_OFFSET), 
                          (kal_uint8)(0x03),
                          (kal_uint8)(BANK_0_RG_BUCK_CTRL_MASK),
                          (kal_uint8)(BANK_0_RG_BUCK_CTRL_SHIFT)
                          );
    
    /* VCORE = 0.9V in sleep mode */
    pmic_config_interface((kal_uint8)(BANK0_VCORE_CON0+BUCK_CON5_OFFSET), 
                          (kal_uint8)(0x08),
                          (kal_uint8)(BANK_0_RG_BUCK_VOSEL_MASK),
                          (kal_uint8)(BANK_0_RG_BUCK_VOSEL_SHIFT)
                          );
                            
    pmic_config_interface((kal_uint8)(BANK0_VCORE_CON0+BUCK_CON7_OFFSET), 
                          (kal_uint8)(0x01),
                          (kal_uint8)(BANK_0_RG_VCORE_CTRL_MASK),
                          (kal_uint8)(BANK_0_RG_VCORE_CTRL_SHIFT)
                          );
	
    /* VM12_INT = 0.9V in sleep mode */
    pmic_config_interface(0x8b, 0x08, 0x1f, 0x0);
	
    pmic_config_interface((kal_uint8)(BANK0_DIGLDO_CON9), 
                          (kal_uint8)(0x08),
                          (kal_uint8)(BANK_0_VM12_INT_SLEEP_MASK),
                          (kal_uint8)(BANK_0_VM12_INT_SLEEP_SHIFT)
                          );
	
    pmic_config_interface((kal_uint8)(BANK0_DIGLDO_COND), 
                          (kal_uint8)(0x01),
                          (kal_uint8)(BANK_0_VM12_INT_CTRL_SEL_MASK),
                          (kal_uint8)(BANK_0_VM12_INT_CTRL_SEL_SHIFT)
                          );
                            
                            
    pmic_config_interface(0x8C, 0x10, 0x1F, 0x0);   // VM12_INT_LOW_BOUND
    
    pmic_config_interface(0x90, 0x1, 0x1, 0x0);     // VM12_INT_LP_SEL HW control
    
    pmic_config_interface(0x85, 0x1, 0x1, 0x0);     // VM12_1_LP_SEL HW control
    
    pmic_config_interface(0x89, 0x1, 0x1, 0x0);     // VM12_2_LP_SEL HW control
    
    pmic_config_interface(0xA9, 0x1, 0x1, 0x0);     // VMC_LP_SEL HW control
    
    pmic_config_interface(0xAD, 0x1, 0x1, 0x0);     // VMCH_LP_SEL HW control
    
    pmic_config_interface(0xC6, 0x1, 0x1, 0x0);     // VA1_LP_SEL HW control
    
    pmic_config_interface(0xC1, 0x1, 0x1, 0x1);     // VTCXO_ON_CTRL HW control
    
    
    /* ARMPLL DIV2 500.5Mhz */
    DRV_WriteReg32(TOP_CKDIV1, 0xA);
    
    /* set VPROC to a lower DVS voltage */
    DRV_WriteReg32(SC_AP_DVFS_CON, 0x02);
}

void sc_mod_exit(void)
{
    /* set VPROC to the max. DVS voltage */
    DRV_WriteReg32(SC_AP_DVFS_CON, 0x03);
    
    gpt_busy_wait_us(PMIC_SETTLE_TIME);

    /* ARMPLL DIV0 1001Mhz */
    DRV_WriteReg32(TOP_CKDIV1, 0x0);
}

void sc_mod_init(void)
{
    mt6575_init_irq();
    
    gpt_irq_init();
    
    mt6575_irq_set_sens(MT6575_SLEEP_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt6575_irq_set_polarity(MT6575_SLEEP_IRQ_ID, MT65xx_POLARITY_LOW);
    
    power_saving_init();
    
    slp_pmic_init();
    
    DRV_WriteReg32(CA9_SCU_CONTROL, (DRV_Reg32(CA9_SCU_CONTROL) | (1 << 5))); // enable_scu_standby
    DRV_WriteReg32(CA9_SCU_CONTROL, (DRV_Reg32(CA9_SCU_CONTROL) | (1 << 6))); // enable_scu_ic_standby
    
    /* make AP control whole-chip sleep mode */
    DRV_WriteReg32(SC_MD_INTF_CON, 0x363f);
    
    if (get_chip_eco_ver() == CHIP_E1) {
        /* to workaround AP+MD sleep/wakeup issue */
        DRV_WriteReg32(SC_CLK_SETTLE, (3 << 24) | (3 << 16) | 100);
        DRV_WriteReg32(SC_MD_INTF_CON, DRV_Reg32(SC_MD_INTF_CON) | (1U << 2));
        DRV_WriteReg32(SC_MD_INTF_CON, DRV_Reg32(SC_MD_INTF_CON) & ~(1U << 10));
    } else {
        /* typical values can cover MD's RM_SYSCLK_SETTLE = 3 ~ 5 ms */
        DRV_WriteReg32(SC_CLK_SETTLE, SC_CLK_SETTLE_TYPICAL);
    }
    
    /* MTCMOS power-on sequence: CPUSYS(2) -> CPU(0) -> NEON(3) -> DBG(1) */
    DRV_WriteReg32(SC_PWR_CON8, (1 << 20) | (3 << 16) | (0 << 12) | (2 << 8));
    DRV_WriteReg32(SC_PWR_CON9, 0x000f);
    
    /* bit 17: DVFS_SEL[0] = 1 depends on AP_SRCLKEN (from E2) */
    /* bit 7: STANDBYWFI = STANDBYWFI_CPU & L2CC_IDLE & SCU_IDLE (from E2) */
    /* bit 4: enable SRCLKENAI pin */
    DRV_WriteReg32(SC_CLK_CON, (1U << 17) | (1U << 7) | (1U << 4));
    
    /* if bus clock source is from 3GPLL, make AP be able to control 3GPLL */
    if ((DRV_Reg32(TOP_CKMUXSEL) & 0x3) == 0x2)
        DRV_WriteReg32(SC_CLK_CON, DRV_Reg32(SC_CLK_CON) | SC_CLK_CON_3GPLLON);
    
    /* set peripheral control to handshake mode */
    DRV_WriteReg32(SC_PERI_CON, 0x0081);
    
    DRV_WriteReg32(SC_PAUSE, 0);
    
    /* mask all wakeup sources */
    DRV_WriteReg32(SC_WAKEUP_EVENT_MASK, 0xffffffff);
    
    /* for cleaning wakeup source status */
    DRV_WriteReg32(SC_DBG_WAKEUP, 0);
    
    DRV_WriteReg32(SC_TIMER_CON, SC_TIMER_CON_DBG | SC_TIMER_CON_EN);
    DRV_WriteReg32(SC_TIMER_CMD, SC_TIMER_CMD_KEY | SC_TIMER_CMD_CONWR |
                                 SC_TIMER_CMD_CNTWR | SC_TIMER_CMD_WR);
    while (!(DRV_Reg32(SC_TIMER_STA) & SC_TIMER_STA_CMDCPL));
    
    DRV_WriteReg32(SC_ISR_MASK, 0x0007);
    DRV_WriteReg32(SC_ISR_STATUS, 0x0007);  /* write 1 clear */
    
    printf("[BATTERY] sc_hw_init : Done\r\n");
}

static void sc_go_to_sleep(u32 timeout, kal_bool en_deep_idle)
{
    struct mtk_irq_mask mask;
    
    unsigned int i, wakesrc, irq, clk_mux, clksettle;
    
    __asm__ __volatile__("cpsid i @ arch_local_irq_disable" : : : "memory", "cc"); // set i bit to disable interrupt
    
    mt6575_irq_mask_all(&mask); // mask all interrupts
    
    gpt_irq_ack();
    
    if (en_deep_idle)
    {
        mt6575_irq_unmask(MT6575_SLEEP_IRQ_ID);
    }
    else
    {
        mt6575_irq_unmask(MT6575_GPT_IRQ_ID);
    }
    
    gpt_one_shot_irq(timeout);
	
    if (en_deep_idle)
    {
        /* keep SRCLKENA high (26M on) when in sleep mode */
        DRV_WriteReg32(SC_CLK_CON, DRV_Reg32(SC_CLK_CON) | SC_CLK_CON_SRCLKEN);
        
        /* set SYSCLK settle time to 1T 32K */
        clksettle = DRV_Reg32(SC_CLK_SETTLE);
        DRV_WriteReg32(SC_CLK_SETTLE, SC_CLK_SETTLE_26MON);
	    
        DRV_WriteReg32(SC_PAUSE, 0xffffffff);
        
        /* unmask wakeup sources */
        DRV_WriteReg32(SC_WAKEUP_EVENT_MASK, ~sc_wake_src);
        
        /* unmask Pause Interrupt and Pause Abort */
        DRV_WriteReg32(SC_ISR_MASK, 0x0001);
        
        DRV_WriteReg32(SC_TIMER_CON, SC_TIMER_CON_EN);
        DRV_WriteReg32(SC_TIMER_CMD, SC_TIMER_CMD_KEY | SC_TIMER_CMD_CONWR |
                                     SC_TIMER_CMD_CNTWR | SC_TIMER_CMD_WR);
        while (!(DRV_Reg32(SC_TIMER_STA) & SC_TIMER_STA_CMDCPL));
        
        DRV_WriteReg32(SC_TIMER_CMD, SC_TIMER_CMD_KEY | SC_TIMER_CMD_PAUSE);
        while (!(DRV_Reg32(SC_TIMER_STA) & SC_TIMER_STA_CMDCPL));
    }
    
    __asm__ __volatile__("dsb" : : : "memory");
    __asm__ __volatile__("wfi"); // enter wfi
    
    if (en_deep_idle)
    {
        /* delay to make sure ISR_STATUS can be cleared later */
        udelay(100);
        
        /* for cleaning wakeup source status */
        DRV_WriteReg32(SC_DBG_WAKEUP, 0);
        
        DRV_WriteReg32(SC_TIMER_CON, SC_TIMER_CON_DBG | SC_TIMER_CON_EN);
        DRV_WriteReg32(SC_TIMER_CMD, SC_TIMER_CMD_KEY | SC_TIMER_CMD_CONWR |
                                     SC_TIMER_CMD_WR);
        while (!(DRV_Reg32(SC_TIMER_STA) & SC_TIMER_STA_CMDCPL));
        
        DRV_WriteReg32(SC_ISR_MASK, 0x0007);
        DRV_WriteReg32(SC_ISR_STATUS, 0x0007);	/* write 1 clear */
        
        /* restore SC_CLK_SETTLE and SC_CLK_CON */
        DRV_WriteReg32(SC_CLK_SETTLE, clksettle);
        DRV_WriteReg32(SC_CLK_CON, DRV_Reg32(SC_CLK_CON) & ~SC_CLK_CON_SRCLKEN);
    }
    
    gpt_irq_ack();
    mt6575_irq_ack(MT6575_GPT_IRQ_ID);
    
    if (en_deep_idle)
    {
        mt6575_irq_ack(MT6575_SLEEP_IRQ_ID);
    }
    
    mt6575_irq_mask_restore(&mask); // restore all interrupts
    
    __asm__ __volatile__("cpsie i @ arch_local_irq_enable" : : : "memory", "cc"); // clear i bit to enable interrupt
}

void mt6575_sleep(u32 timeout, kal_bool en_deep_idle)
{
    unsigned int topmisc, pdn0;
    
    printf("enter mt6575_sleep, timeout = %d\n", timeout);
    
    #if MT6575_SC_DEBUG
        slp_dump_pm_regs();
    #endif
    
    pdn0 = DRV_Reg32(PERI_GLOBALCON_PDN0);
    if (get_chip_eco_ver() == CHIP_E1) {
        /* power on UART0/1/2/3 to workaround handshake mode issue */
        DRV_WriteReg32(PERI_GLOBALCON_PDN0, pdn0 & ~(0xf << 24));
    }
    
    /* keep CA9 clock when entering WFI mode in sleep */
    topmisc = DRV_Reg32(TOP_MISC);
    DRV_WriteReg32(TOP_MISC, topmisc & ~(1U << 0));
    
    if (get_chip_eco_ver() != CHIP_E1) {
        if (en_deep_idle) {
            DRV_WriteReg32(TOPCKGEN_CON3, DRV_Reg32(TOPCKGEN_CON3) & 0x7FFF);
            DRV_WriteReg32(MDPLL_CON0, DRV_Reg32(MDPLL_CON0) | 0x1);
        }
    }
    
    rtc_writeif_lock();
    
    sc_go_to_sleep(timeout, en_deep_idle);
    
    rtc_writeif_unlock();
    
    if (get_chip_eco_ver() != CHIP_E1) {
        if (en_deep_idle) {
            DRV_WriteReg32(MDPLL_CON0, DRV_Reg32(MDPLL_CON0) & 0xFFFE);
            udelay(20);
            DRV_WriteReg32(TOPCKGEN_CON3, DRV_Reg32(TOPCKGEN_CON3) | 0x8000);
        }
    }
    
    /* restore TOP_MISC (and PERI_GLOBALCON_PDN0) */
    DRV_WriteReg32(TOP_MISC, topmisc);
    
    if (get_chip_eco_ver() == CHIP_E1)
        DRV_WriteReg32(PERI_GLOBALCON_PDN0, pdn0);
    
    return 0;
}

enum mtcmos_offset 
{
    PWR_RST_B = 0,
    PWR_ISO,
    PWR_ON,
    PWR_MEM_OFF,
    PWR_CLK_DIS,
    PWR_MEM_SLPB,
    PWR_REQ_EN,
    PWR_CTRL,
};

enum subsys_name
{
    CA9_SUBSYS = 0,
    CA9MP_SUBSYS,
    CPUSYS_SUBSYS,
    NEON_SUBSYS,
    MM1_SUBSYS,
    MM2_SUBSYS,
};

void dcm_enable(void)
{
    unsigned int temp;
    
    // CA9_DCM
    DRV_WriteReg32(TOP_CA9DCMFSEL, 0x07000000);
    DRV_WriteReg32(TOP_DCMCTL, 0x6);
    
    // TOPAXI_DCM
    DRV_WriteReg32(INFRA_DCMFSEL, 0x001F0F07);
    DRV_WriteReg32(INFRA_DCMCTL, ((0x1 << 8) | (0x1)));
    
    // EMI_DCM
    temp = DRV_Reg32(0xF00041DC);
    temp |= (0x3 << 24);
    DRV_WriteReg32(0xF00041DC, temp);
    
    DRV_WriteReg32(INFRA_MISC, 0x1);
    DRV_WriteReg32(INFRA_DCMCTL, ((0x1 << 8) | (0x1)));
    
    // PERI_DCM
    DRV_WriteReg32(PERI_GLOBALCON_DCMFSEL, 0x001F0F07);
    DRV_WriteReg32(PERI_GLOBALCON_DCMCTL, (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | (0x1 << 0));
    
    // MM1_DCM
    DRV_WriteReg32(SMI_LARB0_CON_SET, (0x1 << 11));
    DRV_WriteReg32(SMI_LARB1_CON_SET, (0x1 << 11));
    DRV_WriteReg32(SMI_LARB2_CON_SET, (0x1 << 11));
    DRV_WriteReg32(SMI_LARB3_CON_SET, (0x1 << 11));
    
    // MM2_DCM
    DRV_WriteReg32(MMSYS2_DCMFSEL, 0x001F0F07);
    DRV_WriteReg32(MMSYS2_DCMCTL, ((0x1 << 8) | (0x1 << 0)));
}

void power_saving_init(void)
{
    unsigned int reg_val = 0;
    
    dcm_enable();
    
    /* Power Controlled by SW */
    DRV_SetReg32(SC_PWR_CON5, (0x1 << PWR_CTRL));
    
    /* pull up PWR_ISO */
    DRV_SetReg32(SC_PWR_CON5, (0x1 << PWR_ISO));
    
    /* pull down PWR_RST_B, pull up PWR_MEM_OFF and PWR_CLK_DIS */
    reg_val = DRV_Reg32(SC_PWR_CON5);
    reg_val = (reg_val & ~(0x1 << PWR_RST_B)) | (0x1 << PWR_MEM_OFF) | (0x1 << PWR_CLK_DIS);
    DRV_WriteReg32(SC_PWR_CON5, reg_val);
    
    /* pull down PWR_ON */
    DRV_ClrReg32(SC_PWR_CON5, (0x1 << PWR_ON));
    
    /* wait for PWR_STA update */
    while ((DRV_Reg32(SC_PWR_STA) & (0x1 << MM2_SUBSYS))) {
    }
}