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
//#ifdef BUILD_LK
#if 1
#define ENABLE_DSI_INTERRUPT 0

#include <platform/disp_drv_platform.h>

#ifdef FIX_BUILD_WARNING_0616
#include <platform/ddp_path.h>
#endif
#else

#define ENABLE_DSI_INTERRUPT 1

#include <linux/delay.h>
#include <disp_drv_log.h>
#include <linux/time.h>
#include <linux/string.h>

#include "disp_drv_platform.h"

#include "lcd_reg.h"
#include "lcd_drv.h"

#include "dsi_reg.h"
#include "dsi_drv.h"
#endif

#if ENABLE_DSI_INTERRUPT
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <mach/irqs.h>
#include "mtkfb.h"
static wait_queue_head_t _dsi_wait_queue;
static wait_queue_head_t _dsi_dcs_read_wait_queue;
#endif

/*
#define PLL_BASE			(0xF0060000)
#define DSI_PHY_BASE		(0xF0060B00)
#define DSI_BASE            	(0xF0140000)
*/

#if !(defined(CONFIG_MT6589_FPGA) || defined(BUILD_LK))
//#define DSI_MIPI_API
#endif

#include <platform/sync_write.h>
#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif

#ifdef FIX_BUILD_WARNING_0616
/*****************************************************************************/
/* fix warning: dereferencing type-punned pointer will break strict-aliasing */
/*              rules [-Wstrict-aliasing]                                    */
/*****************************************************************************/
#define DSI_OUTREG32_R(type, addr2, addr1) DISP_OUTREG32_R(type, addr2, addr1)
#define DSI_OUTREG32_V(type, addr2, var) DISP_OUTREG32_V(type, addr2, var)
#define DSI_OUTREGBIT(TYPE,REG,bit,value) DISP_OUTREGBIT(TYPE,REG,bit,value)
#define DSI_INREG32(type,addr) DISP_INREG32(type,addr)
#endif

#ifdef FIX_BUILD_WARNING_0616
#include <platform/mt_gpt.h>
#include <string.h>
#endif

static PDSI_REGS const DSI_REG = (PDSI_REGS)(DSI_BASE);
static PDSI_PHY_REGS const DSI_PHY_REG = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE);
static PDSI_CMDQ_REGS const DSI_CMDQ_REG = (PDSI_CMDQ_REGS)(DSI_BASE+0x180);
static PLCD_REGS const LCD_REG = (PLCD_REGS)(LCD_BASE);
static PDSI_VM_CMDQ_REGS const DSI_VM_CMD_REG = (PDSI_VM_CMDQ_REGS)(DSI_BASE + 0x134);

typedef struct
{
	DSI_REGS regBackup;
	unsigned int cmdq_size;
	DSI_CMDQ_REGS cmdqBackup;
	unsigned int bit_time_ns;
	unsigned int vfp_period_us;
	unsigned int vsa_vs_period_us;
	unsigned int vsa_hs_period_us;
	unsigned int vsa_ve_period_us;
	unsigned int vbp_period_us;
    void (*pIntCallback)(DISP_INTERRUPT_EVENTS);
} DSI_CONTEXT;

static BOOL s_isDsiPowerOn = FALSE;
static DSI_CONTEXT _dsiContext;
static volatile bool isTeSetting = false;
static volatile bool dsiTeEnable = false;
static volatile bool dsiTeExtEnable = false;
extern LCM_PARAMS *lcm_params;
// PLL_clock * 10
#define PLL_TABLE_NUM  (26)
int LCM_DSI_6572_PLL_CLOCK_List[PLL_TABLE_NUM+1]={0,1000,1040,1250,1300,1500,1560,1750,1820,2000,2080,2250,
				2340,2500,2600,2750,2860,3000,3120,3250,3500,3750,4000,4250,4500,4750,5000};

DSI_PLL_CONFIG pll_config[PLL_TABLE_NUM+1] =
		{{0,0,0,0,0,0,0},
		{2,0,0x3D89D89D,1,0x1B1,0x745,0x745},
		{2,0,0x40000000,1,0x1B1,0x790,0x790},
		{1,0,0x26762762,1,0x1B1,0x48B,0x48B},
		{1,0,0x28000000,1,0x1B1,0x4BA,0x4BA},
		{1,0,0x2E276276,1,0x1B1,0x574,0x574},
		{1,0,0x30000000,1,0x1B1,0x5AC,0x5AC},
		{1,0,0x35D89D89,1,0x1B1,0x65D,0x65D},
		{1,0,0x38000000,1,0x1B1,0x69E,0x69E},
		{1,0,0x3D89D89D,1,0x1B1,0x745,0x745},
		{1,0,0x40000000,1,0x1B1,0x790,0x790},
		{1,0,0x453B13B1,1,0x1B1,0x82E,0x82E},
		{1,0,0x48000000,1,0x1B1,0x882,0x882},
		{0,0,0x26762762,1,0x1B1,0x48B,0x48B},
		{0,0,0x28000000,1,0x1B1,0x4BA,0x4BA},
		{0,0,0x2A4EC4EC,1,0x1B1,0x500,0x500},
		{0,0,0x2C000000,1,0x1B1,0x533,0x533},
		{0,0,0x2E276276,1,0x1B1,0x574,0x574},
		{0,0,0x30000000,1,0x1B1,0x5AC,0x5AC},
		{0,0,0x32000000,1,0x1B1,0x5E8,0x5E8},
		{0,0,0x35D89D89,1,0x1B1,0x65D,0x65D},
		{0,0,0x39B13B13,1,0x1B1,0x6D1,0x6D1},
		{0,0,0x3D89D89D,1,0x1B1,0x745,0x745},
		{0,0,0x41627627,1,0x1B1,0x7BA,0x7BA},
		{0,0,0x453B13B1,1,0x1B1,0x82E,0x82E},
		{0,0,0x4913B13B,1,0x1B1,0x8A2,0x8A2},
		{0,0,0x4CEC4EC4,1,0x1B1,0x917,0x917},
};
#ifndef BUILD_LK

DEFINE_SPINLOCK(g_handle_esd_lock);

static BOOL dsi_esd_recovery = false;
static BOOL dsi_noncont_clk_enabled = false;
static unsigned int dsi_noncont_clk_period = 1;
static BOOL dsi_int_te_enabled = false;
static unsigned int dsi_int_te_period = 1;
static unsigned int dsi_dpi_isr_count = 0;
unsigned long g_handle_esd_flag;

#endif

#ifdef BUILD_LK
static long int get_current_time_us(void)
{
    return 0;       ///TODO: fix me
}
#else
static long int get_current_time_us(void)
{
    struct timeval t;
    do_gettimeofday(&t);
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}
#endif
#if 0
unsigned int custom_pll_clock_remap(int input_mipi_clock)
{
   int i;
   unsigned int ret = 0;
   int mipi_clock=10*input_mipi_clock;
   printk("DSI: custom_pll_clock_remap,mipi clock be %d!!!\n", mipi_clock);

	if(mipi_clock == 0)return 0;

   if((mipi_clock>0)&&(mipi_clock<LCM_DSI_6572_PLL_CLOCK_List[1]))
//  	   lcm_params->dsi.PLL_CLOCK=1;
	   return 1;

   if(mipi_clock>LCM_DSI_6572_PLL_CLOCK_List[PLL_TABLE_NUM])
//  	   lcm_params->dsi.PLL_CLOCK=50;
	   return PLL_TABLE_NUM;

	for(i=1;i<PLL_TABLE_NUM+1;i++)
   	{
		ASSERT(LCM_DSI_6572_PLL_CLOCK_List[i] < LCM_DSI_6572_PLL_CLOCK_List[i+1]);
   		if((mipi_clock>=LCM_DSI_6572_PLL_CLOCK_List[i])&&(mipi_clock<=LCM_DSI_6572_PLL_CLOCK_List[i+1]))
   	  	{
   	  		if((mipi_clock-LCM_DSI_6572_PLL_CLOCK_List[i])<=(LCM_DSI_6572_PLL_CLOCK_List[i+1]-mipi_clock))
   	  	  	{
//   	  	  	  	    lcm_params->dsi.PLL_CLOCK=i+1;
				ret = i;
				break;
			}
			else
			{
//			  	    lcm_params->dsi.PLL_CLOCK=i+2;
				ret = i+1;
				break;
			}
		}
	}
	printk("custom_pll_clock_remap,remap clock is %d!!!\n", ret);
	return ret;
}
#endif
static void lcm_mdelay(UINT32 ms)
{
	udelay(1000 * ms);
}

#if ENABLE_DSI_INTERRUPT
static irqreturn_t _DSI_InterruptHandler(int irq, void *dev_id)
{
    DSI_INT_STATUS_REG status = DSI_REG->DSI_INTSTA;

    if (status.RD_RDY)
    {
        ///write clear RD_RDY interrupt
        //DSI_REG->DSI_INTSTA.RD_RDY = 1;
        OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
        /// write clear RD_RDY interrupt must be before DSI_RACK
        /// because CMD_DONE will raise after DSI_RACK,
        /// so write clear RD_RDY after that will clear CMD_DONE too

		do
        {
            ///send read ACK
            //DSI_REG->DSI_RACK.DSI_RACK = 1;
			DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
        } while(DSI_REG->DSI_STA.BUSY);

		wake_up_interruptible(&_dsi_dcs_read_wait_queue);
        if(_dsiContext.pIntCallback)
            _dsiContext.pIntCallback(DISP_DSI_READ_RDY_INT);
    }

    if (status.CMD_DONE)
    {
        //DSI_REG->DSI_INTSTA.CMD_DONE = 1;
        OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
    	// Go back to LP mode.
    	DSI_clk_HS_mode(0);
        wake_up_interruptible(&_dsi_wait_queue);
        if(_dsiContext.pIntCallback)
            _dsiContext.pIntCallback(DISP_DSI_CMD_DONE_INT);
    }

    return IRQ_HANDLED;
}
#endif



static BOOL _IsEngineBusy(void)
{
	DSI_INT_STATUS_REG status;
	//arch_clean_invalidate_cache_range(*(unsigned int*)(&DSI_REG->DSI_STA), 32);

	status = DSI_REG->DSI_INTSTA;

	if (status.BUSY)
		return TRUE;

	return FALSE;
}

#if 0
static BOOL _IsCMDQBusy(void)
{
	DSI_INT_STATUS_REG INT_status;

	INT_status=DSI_REG->DSI_INTSTA;

	if (!INT_status.CMD_DONE)
	{
		DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI CMDQ status BUSY !!\n");

		return TRUE;
	}

	return FALSE;
}
#endif

static void _WaitForEngineNotBusy(void)
{
    int timeOut;
#if ENABLE_DSI_INTERRUPT
    long int time;
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
#endif

	if (DSI_REG->DSI_MODE_CTRL.MODE)
		return ;

	timeOut = 400;

#if ENABLE_DSI_INTERRUPT
	time = get_current_time_us();

    if (in_interrupt())
    {
        // perform busy waiting if in interrupt context
		while(_IsEngineBusy()) {
			msleep(1);
			if (--timeOut < 0)	{

				DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!(Wait %d us)\n", get_current_time_us() - time);
				DSI_DumpRegisters();
				DSI_Reset();
				dsiTeEnable = false;//disable TE
                dsiTeExtEnable = false;//disable TE
				break;
			}
		}
    }
    else
    {
        while (DSI_REG->DSI_INTSTA.BUSY || DSI_REG->DSI_INTSTA.CMD_DONE)
        {
            long ret = wait_event_interruptible_timeout(_dsi_wait_queue,
                                                        !_IsEngineBusy() && !(DSI_REG->DSI_INTSTA.CMD_DONE),
                                                        WAIT_TIMEOUT);
            if (0 == ret) {
                DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine not busy timeout!!!\n");
				DSI_DumpRegisters();
				DSI_Reset();
				dsiTeEnable = false;//disable TE
                dsiTeExtEnable = false;//disable TE
            }
        }
    }
#else

	while(_IsEngineBusy()) {
		udelay(100);
		/*printk("xuecheng, dsi wait\n");*/
		if (--timeOut < 0) {
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", " Wait for DSI engine not busy timeout!!!\n");
			DSI_DumpRegisters();
			DSI_Reset();
			dsiTeEnable = false;//disable TE
            dsiTeExtEnable = false;//disable TE
			break;
		}
	}
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA, 0x0);
#else
	OUTREG32(&DSI_REG->DSI_INTSTA, 0x0);
#endif
#endif
}

static void _BackupDSIRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);

    //memcpy((void*)&(_dsiContext.regBackup), (void*)DSI_BASE, sizeof(DSI_REGS));
#ifdef FIX_BUILD_WARNING_0616
    DSI_OUTREG32_R(PDSI_INT_ENABLE_REG,&regs->DSI_INTEN, &DSI_REG->DSI_INTEN);
    DSI_OUTREG32_R(PDSI_MODE_CTRL_REG,&regs->DSI_MODE_CTRL, &DSI_REG->DSI_MODE_CTRL);
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&regs->DSI_TXRX_CTRL, &DSI_REG->DSI_TXRX_CTRL);
    DSI_OUTREG32_R(PDSI_PSCTRL_REG,&regs->DSI_PSCTRL, &DSI_REG->DSI_PSCTRL);

    DSI_OUTREG32_R(PDSI_VSA_NL_REG,&regs->DSI_VSA_NL, &DSI_REG->DSI_VSA_NL);
    DSI_OUTREG32_R(PDSI_VBP_NL_REG,&regs->DSI_VBP_NL, &DSI_REG->DSI_VBP_NL);
    DSI_OUTREG32_R(PDSI_VFP_NL_REG,&regs->DSI_VFP_NL, &DSI_REG->DSI_VFP_NL);
    DSI_OUTREG32_R(PDSI_VACT_NL_REG,&regs->DSI_VACT_NL, &DSI_REG->DSI_VACT_NL);

    DSI_OUTREG32_R(PDSI_HSA_WC_REG,&regs->DSI_HSA_WC, &DSI_REG->DSI_HSA_WC);
    DSI_OUTREG32_R(PDSI_HBP_WC_REG,&regs->DSI_HBP_WC, &DSI_REG->DSI_HBP_WC);
    DSI_OUTREG32_R(PDSI_HFP_WC_REG,&regs->DSI_HFP_WC, &DSI_REG->DSI_HFP_WC);
    DSI_OUTREG32_R(PDSI_BLLP_WC_REG,&regs->DSI_BLLP_WC, &DSI_REG->DSI_BLLP_WC);
#else
    OUTREG32(&regs->DSI_INTEN, AS_UINT32(&DSI_REG->DSI_INTEN));
    OUTREG32(&regs->DSI_MODE_CTRL, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    OUTREG32(&regs->DSI_TXRX_CTRL, AS_UINT32(&DSI_REG->DSI_TXRX_CTRL));
    OUTREG32(&regs->DSI_PSCTRL, AS_UINT32(&DSI_REG->DSI_PSCTRL));

    OUTREG32(&regs->DSI_VSA_NL, AS_UINT32(&DSI_REG->DSI_VSA_NL));
    OUTREG32(&regs->DSI_VBP_NL, AS_UINT32(&DSI_REG->DSI_VBP_NL));
    OUTREG32(&regs->DSI_VFP_NL, AS_UINT32(&DSI_REG->DSI_VFP_NL));
    OUTREG32(&regs->DSI_VACT_NL, AS_UINT32(&DSI_REG->DSI_VACT_NL));

    OUTREG32(&regs->DSI_HSA_WC, AS_UINT32(&DSI_REG->DSI_HSA_WC));
    OUTREG32(&regs->DSI_HBP_WC, AS_UINT32(&DSI_REG->DSI_HBP_WC));
    OUTREG32(&regs->DSI_HFP_WC, AS_UINT32(&DSI_REG->DSI_HFP_WC));
    OUTREG32(&regs->DSI_BLLP_WC, AS_UINT32(&DSI_REG->DSI_BLLP_WC));
#endif
    OUTREG32(&regs->DSI_HSTX_CKL_WC, AS_UINT32(&DSI_REG->DSI_HSTX_CKL_WC));

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_R(PDSI_MEM_CONTI_REG,&regs->DSI_MEM_CONTI, &DSI_REG->DSI_MEM_CONTI);
#else
	OUTREG32(&regs->DSI_MEM_CONTI, AS_UINT32(&DSI_REG->DSI_MEM_CONTI));
#endif

    OUTREG32(&regs->DSI_PHY_TIMECON0, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON0));
    OUTREG32(&regs->DSI_PHY_TIMECON1, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON1));
    OUTREG32(&regs->DSI_PHY_TIMECON2, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON2));
    OUTREG32(&regs->DSI_PHY_TIMECON3, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON3));

    DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG,&regs->DSI_VM_CMD_CON, &DSI_REG->DSI_VM_CMD_CON);

}

static void _RestoreDSIRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);

#ifdef FIX_BUILD_WARNING_0616
    DSI_OUTREG32_R(PDSI_INT_ENABLE_REG,&DSI_REG->DSI_INTEN, &regs->DSI_INTEN);
    DSI_OUTREG32_R(PDSI_MODE_CTRL_REG,&DSI_REG->DSI_MODE_CTRL, &regs->DSI_MODE_CTRL);
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &regs->DSI_TXRX_CTRL);
    DSI_OUTREG32_R(PDSI_PSCTRL_REG,&DSI_REG->DSI_PSCTRL, &regs->DSI_PSCTRL);

    DSI_OUTREG32_R(PDSI_VSA_NL_REG,&DSI_REG->DSI_VSA_NL, &regs->DSI_VSA_NL);
    DSI_OUTREG32_R(PDSI_VBP_NL_REG,&DSI_REG->DSI_VBP_NL, &regs->DSI_VBP_NL);
    DSI_OUTREG32_R(PDSI_VFP_NL_REG,&DSI_REG->DSI_VFP_NL, &regs->DSI_VFP_NL);
    DSI_OUTREG32_R(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, &regs->DSI_VACT_NL);

    DSI_OUTREG32_R(PDSI_HSA_WC_REG,&DSI_REG->DSI_HSA_WC, &regs->DSI_HSA_WC);
    DSI_OUTREG32_R(PDSI_HBP_WC_REG,&DSI_REG->DSI_HBP_WC, &regs->DSI_HBP_WC);
    DSI_OUTREG32_R(PDSI_HFP_WC_REG,&DSI_REG->DSI_HFP_WC, &regs->DSI_HFP_WC);
    DSI_OUTREG32_R(PDSI_BLLP_WC_REG,&DSI_REG->DSI_BLLP_WC, &regs->DSI_BLLP_WC);
#else
    OUTREG32(&DSI_REG->DSI_INTEN, AS_UINT32(&regs->DSI_INTEN));
    OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&regs->DSI_MODE_CTRL));
    OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&regs->DSI_TXRX_CTRL));
    OUTREG32(&DSI_REG->DSI_PSCTRL, AS_UINT32(&regs->DSI_PSCTRL));

    OUTREG32(&DSI_REG->DSI_VSA_NL, AS_UINT32(&regs->DSI_VSA_NL));
    OUTREG32(&DSI_REG->DSI_VBP_NL, AS_UINT32(&regs->DSI_VBP_NL));
    OUTREG32(&DSI_REG->DSI_VFP_NL, AS_UINT32(&regs->DSI_VFP_NL));
    OUTREG32(&DSI_REG->DSI_VACT_NL, AS_UINT32(&regs->DSI_VACT_NL));

    OUTREG32(&DSI_REG->DSI_HSA_WC, AS_UINT32(&regs->DSI_HSA_WC));
    OUTREG32(&DSI_REG->DSI_HBP_WC, AS_UINT32(&regs->DSI_HBP_WC));
    OUTREG32(&DSI_REG->DSI_HFP_WC, AS_UINT32(&regs->DSI_HFP_WC));
    OUTREG32(&DSI_REG->DSI_BLLP_WC, AS_UINT32(&regs->DSI_BLLP_WC));
#endif

    OUTREG32(&DSI_REG->DSI_HSTX_CKL_WC, AS_UINT32(&regs->DSI_HSTX_CKL_WC));

#ifdef FIX_BUILD_WARNING_0616
    DSI_OUTREG32_R(PDSI_MEM_CONTI_REG,&DSI_REG->DSI_MEM_CONTI, &regs->DSI_MEM_CONTI);
#else
	OUTREG32(&DSI_REG->DSI_MEM_CONTI, AS_UINT32(&regs->DSI_MEM_CONTI));
#endif
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON0, AS_UINT32(&regs->DSI_PHY_TIMECON0));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON1, AS_UINT32(&regs->DSI_PHY_TIMECON1));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON2, AS_UINT32(&regs->DSI_PHY_TIMECON2));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON3, AS_UINT32(&regs->DSI_PHY_TIMECON3));

    DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG,&DSI_REG->DSI_VM_CMD_CON, &regs->DSI_VM_CMD_CON);

}

static void _ResetBackupedDSIRegisterValues(void)
{
    DSI_REGS *regs = &_dsiContext.regBackup;
    memset((void*)regs, 0, sizeof(DSI_REGS));
}

static DSI_STATUS DSI_TE_Setting(void)
{
    //return DSI_STATUS_OK;
    if(isTeSetting)
    {
        return DSI_STATUS_OK;
    }

    if(lcm_params->dsi.mode == CMD_MODE && lcm_params->dsi.lcm_ext_te_enable == TRUE)
    {
        //Enable EXT TE
		dsiTeEnable = false;
		dsiTeExtEnable = true;
    }
    else
    {
        //Enable BTA TE
		dsiTeEnable = true;
		dsiTeExtEnable = false;
    }

    isTeSetting = true;

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Init(BOOL isDsiPoweredOn)
{
    DSI_STATUS ret = DSI_STATUS_OK;

    memset(&_dsiContext, 0, sizeof(_dsiContext));

	OUTREG32(DISPSYS_BASE + 0x4C, 0x0);

    if (isDsiPoweredOn) {
        _BackupDSIRegisters();
    } else {
        _ResetBackupedDSIRegisterValues();
    }
	ret = DSI_PowerOn();
	DSI_TE_Setting();

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_MEM_CONTI_REG,&DSI_REG->DSI_MEM_CONTI, DSI_WMEM_CONTI);
	/* External TE Setting */
	DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,EXT_TE,1);
#else
	OUTREG32(&DSI_REG->DSI_MEM_CONTI, DSI_WMEM_CONTI);
	/* External TE Setting */
	OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,EXT_TE,1);
#endif

    ASSERT(ret == DSI_STATUS_OK);
/*
    mipitx_con0=DSI_PHY_REG->MIPITX_CON0;
    mipitx_con1=DSI_PHY_REG->MIPITX_CON1;
    mipitx_con3=DSI_PHY_REG->MIPITX_CON3;
    mipitx_con6=DSI_PHY_REG->MIPITX_CON6;
    mipitx_con8=DSI_PHY_REG->MIPITX_CON8;
    mipitx_con9=DSI_PHY_REG->MIPITX_CON9;
*/
#if ENABLE_DSI_INTERRUPT
    init_waitqueue_head(&_dsi_wait_queue);
    init_waitqueue_head(&_dsi_dcs_read_wait_queue);
    if (request_irq(MT6589_DSI_IRQ_ID,
        _DSI_InterruptHandler, IRQF_TRIGGER_LOW, MTKFB_DRIVER, NULL) < 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DSI", "fail to request DSI irq\n");
        return DSI_STATUS_ERROR;
    }
		//mt65xx_irq_unmask(MT6577_DSI_IRQ_ID);
    //DSI_REG->DSI_INTEN.CMD_DONE=1;
    //DSI_REG->DSI_INTEN.RD_RDY=1;
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
    OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
#endif

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Deinit(void)
{
    DSI_STATUS ret = DSI_PowerOff();

    ASSERT(ret == DSI_STATUS_OK);

    return DSI_STATUS_OK;
}

#ifdef BUILD_LK
DSI_STATUS DSI_PowerOn(void)
{
    if (!s_isDsiPowerOn)
    {
        MASKREG32(0x14000118, 0x3, 0x3);
        _RestoreDSIRegisters();
        s_isDsiPowerOn = TRUE;
	    printf("\n[DISP] - DSI_PowerOn. 0x%8x\n", INREG32(0x14000110));
    }

    return DSI_STATUS_OK;

}


DSI_STATUS DSI_PowerOff(void)
{
	if (s_isDsiPowerOn)
		{
#ifndef FIX_BUILD_WARNING_0616
			BOOL ret = TRUE;
#endif
			_BackupDSIRegisters();

			MASKREG32(0x14000114, 0x3, 0x3);
			printf("[DISP] - DSI_PowerOff. 0x%8x\n", INREG32(0x14000110));

			s_isDsiPowerOn = FALSE;
		}

		return DSI_STATUS_OK;

}

#else
DSI_STATUS DSI_PowerOn(void)
{
#ifndef CONFIG_MT6589_FPGA
    if (!s_isDsiPowerOn)
    {
#if 0   // FIXME
        //BOOL ret = hwEnableClock(MT65XX_PDN_MM_DSI, "DSI");
        BOOL ret = enable_clock(MT65XX_PDN_MM_DSI, "DSI");
        ASSERT(!ret);
#endif
        _RestoreDSIRegisters();
        s_isDsiPowerOn = TRUE;
    }
#endif
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOff(void)
{
#ifndef CONFIG_MT6589_FPGA
    if (s_isDsiPowerOn)
    {
        BOOL ret = TRUE;
        _WaitForEngineNotBusy();
        _BackupDSIRegisters();
#if 0   // FIXME
        //ret = hwDisableClock(MT65XX_PDN_MM_DSI, "DSI");
        ret = disable_clock(MT65XX_PDN_MM_DSI, "DSI");
        ASSERT(!ret);
#endif
        s_isDsiPowerOn = FALSE;
    }
#endif
    return DSI_STATUS_OK;
}
#endif

DSI_STATUS DSI_WaitForNotBusy(void)
{
    _WaitForEngineNotBusy();

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_EnableClk(void)
{
	_WaitForEngineNotBusy();

	//DSI_REG->DSI_START.DSI_START=0;
	//DSI_REG->DSI_START.DSI_START=1;
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
#else
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
#endif

    return DSI_STATUS_OK;
}

static void DSI_BackUpCmdQ(void)
{
	unsigned int i;
	DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);
#ifdef FIX_BUILD_WARNING_0616
	_dsiContext.cmdq_size = DSI_INREG32(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE);
#else
	_dsiContext.cmdq_size = AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE);
#endif

	for (i=0; i<_dsiContext.cmdq_size; i++)
		OUTREG32(&regs->data[i], AS_UINT32(&DSI_CMDQ_REG->data[i]));
}


static void DSI_RestoreCmdQ(void)
{
	unsigned int i;
	DSI_CMDQ_REGS *regs = &(_dsiContext.cmdqBackup);

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, _dsiContext.cmdq_size);
#else
	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, AS_UINT32(&_dsiContext.cmdq_size));
#endif

	for (i=0; i<_dsiContext.cmdq_size; i++)
		OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32(&regs->data[i]));
}

void DSI_WaitBtaTE(void)
{
	DSI_T0_INS t0;
#if ENABLE_DSI_INTERRUPT
	long ret;
	static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
#else
	long int dsi_wait_time = 0;
#endif

	if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
#ifdef FIX_BUILD_WARNING_0616
	{
		return;
	}
#else
		return DSI_STATUS_OK;
#endif

	_WaitForEngineNotBusy();

	DSI_clk_HS_mode(0);
	// backup command queue setting.
	DSI_BackUpCmdQ();

	t0.CONFG = 0x20;		///TE
	t0.Data0 = 0;
	t0.Data_ID = 0;
	t0.Data1 = 0;

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
#else
	OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

	//DSI_REG->DSI_START.DSI_START=0;
	//DSI_REG->DSI_START.DSI_START=1;
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
#endif
#if ENABLE_DSI_INTERRUPT

	ret = wait_event_interruptible_timeout(_dsi_wait_bta_te,
											!_IsEngineBusy(),
											WAIT_TIMEOUT);

	if (0 == ret) {
		DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for _dsi_wait_bta_te(DSI_INTSTA.TE_RDY) ready timeout!!!\n");

		// Set DSI_RACK to let DSI idle
		//DSI_REG->DSI_RACK.DSI_RACK = 1;
		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_DumpRegisters();
		///do necessary reset here
		DSI_Reset();
		dsiTeEnable = false;//disable TE
		return;
	}

	// After setting DSI_RACK, it needs to wait for CMD_DONE interrupt.
	_WaitForEngineNotBusy();

#else

	while(DSI_REG->DSI_INTSTA.TE_RDY == 0)	// polling TE_RDY
	{
	    udelay(100);//sleep 50us
		dsi_wait_time++;
		if(dsi_wait_time > 40000)
		{
			DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for TE_RDY timeout!!!\n");

			// Set DSI_RACK to let DSI idle
//			DSI_REG->DSI_RACK.DSI_RACK = 1;
			DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
			DSI_DumpRegisters();

			//do necessary reset here
			DSI_Reset();
			dsiTeEnable = false;//disable TE
			break;
		}
	}
	dsi_wait_time = 0;
	// Write clear RD_RDY
//	DSI_REG->DSI_INTSTA.TE_RDY = 0;
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,TE_RDY,0);
	// Set DSI_RACK to let DSI idle
//	DSI_REG->DSI_RACK.DSI_RACK = 1;
	DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
	if(!dsiTeEnable){
		DSI_RestoreCmdQ();
		DSI_LP_Reset();
		return;
	}
	while(DSI_REG->DSI_INTSTA.CMD_DONE == 0)	// polling CMD_DONE
	{
	    udelay(100);//sleep 50us
		dsi_wait_time++;
		if(dsi_wait_time > 40000)
		{
			DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for CMD_DONE timeout!!!\n");

			// Set DSI_RACK to let DSI idle
//			DSI_REG->DSI_RACK.DSI_RACK = 1;
			DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
			DSI_DumpRegisters();

			///do necessary reset here
			DSI_Reset();
			dsiTeEnable = false;//disable TE
			break;
		}
	}

	// Write clear CMD_DONE
//	DSI_REG->DSI_INTSTA.CMD_DONE = 0;
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,0);

#endif
	// restore command queue setting.
	DSI_RestoreCmdQ();
	DSI_LP_Reset();
}

void DSI_WaitExternalTE(void)
{
	UINT32 time0, time1;
#ifndef FIX_BUILD_WARNING_0616
   	DSI_T0_INS t0;
#endif
#if ENABLE_DSI_INTERRUPT
    long ret;
    static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
#else
    long int dsi_wait_time = 0;
#endif

    if(DSI_REG->DSI_MODE_CTRL.MODE != CMD_MODE)
        return;
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,1);
	DSI_OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EDGE,0);
#else
	OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,1);
	OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EDGE,0);
#endif
#if ENABLE_DSI_INTERRUPT

    //can not go to this line

#else

	time0 = gpt4_tick2time_us(gpt4_get_current_tick());
    while(DSI_REG->DSI_INTSTA.EXT_TE == 0)	// polling EXT_TE
    {
        udelay(100);//sleep 50us
        dsi_wait_time++;
        if(dsi_wait_time > 40000)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for EXT_TE timeout!!!\n");
			DSI_OUTREGBIT(DSI_TXRX_CTRL_REG,DSI_REG->DSI_TXRX_CTRL,EXT_TE_EN,0);

            DSI_DumpRegisters();

            //do necessary reset here
            DSI_Reset();
            dsiTeExtEnable = false;//disable TE
            break;
        }
    }

	  time1 = gpt4_tick2time_us(gpt4_get_current_tick());
	  printf("[wwy]polling ext te time =%d , DSI_INTSTA = 0x%08x\n", time1 - time0,INREG32(DSI_BASE+0xC));

#ifdef FIX_BUILD_WARNING_0616
	// Write clear EXT_TE
	DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,EXT_TE,0);
#else
	// Write clear EXT_TE
	OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,EXT_TE,0);
#endif
    if(!dsiTeExtEnable){
        DSI_LP_Reset();
        return;
    }

    #endif

    DSI_LP_Reset();
}

DSI_STATUS DSI_WaitVsync()
{
	Wait_Rdma_Start(0);

	return DSI_STATUS_OK;
}
DSI_STATUS DSI_RegUpdate(void)
{
	UINT32 dsi_wait_time = 0;
	printk("[wwy] enter DSI_RegUpdate\n");
	//MASKREG32(0x14011000, 0x1, 0x1); //Enable DISP MUTEX0
	//MASKREG32(0x14011004, 0x1, 0x0);
	while((INREG32(0x1400E004)&0x1) != 0x1) // polling DISP MUTEX0
	{
	    printk("[wwy] DSI_RegUpdate dsi_wait_time = %d\n",dsi_wait_time);
		udelay(50);//sleep 50us
		dsi_wait_time++;
		if(dsi_wait_time > 40000){
			DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", "Wait for DISP MUTEX0 IRQ timeout!!!\n");
			break;
		}
	}
	MASKREG32(0x1400E004, 0x1, 0x0);
	printk("[wwy] end DSI_RegUpdate\n");
	//mdelay(500);//sleep 50us
	return DSI_STATUS_OK;
}
DSI_STATUS DSI_StartTransfer(BOOL needStartDSI)
{
	disp_path_get_mutex();
	LCD_ConfigOVL();
    if (dsiTeEnable)
        DSI_WaitBtaTE();

    if (dsiTeExtEnable)
    {
        DSI_WaitExternalTE();
    }
	if(needStartDSI)
	{
		DSI_clk_HS_mode(1);
		if(1 == lcm_params->dsi.ufoe_enable)
			UFOE_Start();
		DSI_EnableClk();
	}

	disp_path_release_mutex();
#ifdef FIX_BUILD_WARNING_0616
	return DSI_STATUS_OK;
#endif
}

DSI_STATUS DSI_DisableClk(void)
{
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
#else
	//DSI_REG->DSI_START.DSI_START=0;
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
#endif
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Reset(void)
{
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
	lcm_mdelay(5);
	DSI_OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
#else
	//DSI_REG->DSI_COM_CTRL.DSI_RESET = 1;
	OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
	lcm_mdelay(5);
	//DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;
	OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
#endif

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_LP_Reset(void)
{
#if 0
	_WaitForEngineNotBusy();
	OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
	OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
#endif
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_SetMode(unsigned int mode)
{
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,MODE,mode);
#else
	//DSI_REG->DSI_MODE_CTRL.MODE = mode;
	OUTREGBIT(DSI_MODE_CTRL_REG,DSI_REG->DSI_MODE_CTRL,MODE,mode);
#endif
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_DSI_INTERRUPT
    switch(eventID)
    {
        case DISP_DSI_READ_RDY_INT:
            //DSI_REG->DSI_INTEN.RD_RDY = 1;
            OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
            break;
        case DISP_DSI_CMD_DONE_INT:
            //DSI_REG->DSI_INTEN.CMD_DONE = 1;
            OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);
            break;
        default:
            return DSI_STATUS_ERROR;
    }

    return DSI_STATUS_OK;
#else
    ///TODO: warning log here
    return DSI_STATUS_ERROR;
#endif
}


DSI_STATUS DSI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    _dsiContext.pIntCallback = pCB;

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_handle_TE(void)
{

	unsigned int data_array;

	//data_array=0x00351504;
	//DSI_set_cmdq(&data_array, 1, 1);

	//lcm_mdelay(10);

	// RACT
	//data_array=1;
	//OUTREG32(&DSI_REG->DSI_RACK, data_array);

	// TE + BTA
	data_array=0x24;
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI_handle_TE TE + BTA !! \n");
	OUTREG32(&DSI_CMDQ_REG->data, data_array);

	//DSI_CMDQ_REG->data.byte0=0x24;
	//DSI_CMDQ_REG->data.byte1=0;
	//DSI_CMDQ_REG->data.byte2=0;
	//DSI_CMDQ_REG->data.byte3=0;

	//DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE=1;
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREGBIT(DSI_CMDQ_CTRL_REG,DSI_REG->DSI_CMDQ_SIZE,CMDQ_SIZE,1);

	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
#else
	OUTREGBIT(DSI_CMDQ_CTRL_REG,DSI_REG->DSI_CMDQ_SIZE,CMDQ_SIZE,1);

	//DSI_REG->DSI_START.DSI_START=0;
	//DSI_REG->DSI_START.DSI_START=1;
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,0);
	OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,DSI_START,1);
#endif
	// wait TE Trigger status
//	do
//	{
		lcm_mdelay(10);
#ifdef FIX_BUILD_WARNING_0616
		data_array=DSI_INREG32(PDSI_INT_STATUS_REG,&DSI_REG->DSI_INTSTA);
#else
		data_array=INREG32(&DSI_REG->DSI_INTSTA);
#endif
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI INT state : %x !! \n", data_array);

#ifdef FIX_BUILD_WARNING_0616
		data_array=DSI_INREG32(PDSI_TRIG_STA_REG,&DSI_REG->DSI_TRIG_STA);
#else
		data_array=INREG32(&DSI_REG->DSI_TRIG_STA);
#endif
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI TRIG TE status check : %x !! \n", data_array);
//	} while(!(data_array&0x4));

	// RACT
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] DSI Set RACT !! \n");
	data_array=1;
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_RACK_REG,&DSI_REG->DSI_RACK, data_array);
#else
	OUTREG32(&DSI_REG->DSI_RACK, data_array);
#endif
	return DSI_STATUS_OK;

}
void DSI_Set_VM_CMD(LCM_PARAMS *lcm_params)
{
	DSI_OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,TS_VFP_EN,1);
	DSI_OUTREGBIT(DSI_VM_CMD_CON_REG,DSI_REG->DSI_VM_CMD_CON,VM_CMD_EN,1);
	return;
}
#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))
//unsigned int dsi_cycle_time;

void DSI_Config_VDO_Timing(LCM_PARAMS *lcm_params)
{
	unsigned int line_byte;
#ifdef FIX_BUILD_WARNING_0616
	unsigned int horizontal_sync_active_byte = 0;
#else
	unsigned int horizontal_sync_active_byte;
#endif
	unsigned int horizontal_backporch_byte;
	unsigned int horizontal_frontporch_byte;
	unsigned int horizontal_bllp_byte;
	unsigned int dsiTmpBufBpp;

	#define LINE_PERIOD_US				(8 * line_byte * _dsiContext.bit_time_ns / 1000)

	if(lcm_params->dsi.data_format.format == LCM_DSI_FORMAT_RGB565)
		dsiTmpBufBpp = 2;
	else
		dsiTmpBufBpp = 3;

	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[dsi_drv.c] LK config VDO Timing: %d\n", __LINE__);
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_VSA_NL_REG,&DSI_REG->DSI_VSA_NL, lcm_params->dsi.vertical_sync_active);
	DSI_OUTREG32_V(PDSI_VBP_NL_REG,&DSI_REG->DSI_VBP_NL, lcm_params->dsi.vertical_backporch);
	DSI_OUTREG32_V(PDSI_VFP_NL_REG,&DSI_REG->DSI_VFP_NL, lcm_params->dsi.vertical_frontporch);
	DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, lcm_params->dsi.vertical_active_line);
#else
	OUTREG32(&DSI_REG->DSI_VSA_NL, lcm_params->dsi.vertical_sync_active);
	OUTREG32(&DSI_REG->DSI_VBP_NL, lcm_params->dsi.vertical_backporch);
	OUTREG32(&DSI_REG->DSI_VFP_NL, lcm_params->dsi.vertical_frontporch);
	OUTREG32(&DSI_REG->DSI_VACT_NL, lcm_params->dsi.vertical_active_line);
#endif

	line_byte							=	(lcm_params->dsi.horizontal_sync_active \
											+ lcm_params->dsi.horizontal_backporch \
											+ lcm_params->dsi.horizontal_frontporch \
											+ lcm_params->dsi.horizontal_active_pixel) * dsiTmpBufBpp;

	if (lcm_params->dsi.mode == SYNC_EVENT_VDO_MODE || lcm_params->dsi.mode == BURST_VDO_MODE ){
		ASSERT((lcm_params->dsi.horizontal_backporch + lcm_params->dsi.horizontal_sync_active) * dsiTmpBufBpp> 9);
		horizontal_backporch_byte		=	((lcm_params->dsi.horizontal_backporch + lcm_params->dsi.horizontal_sync_active)* dsiTmpBufBpp - 10);
	}
	else{
		ASSERT(lcm_params->dsi.horizontal_sync_active * dsiTmpBufBpp > 9);
		horizontal_sync_active_byte 		=	(lcm_params->dsi.horizontal_sync_active * dsiTmpBufBpp - 10);

		ASSERT(lcm_params->dsi.horizontal_backporch * dsiTmpBufBpp > 9);
		horizontal_backporch_byte		=	(lcm_params->dsi.horizontal_backporch * dsiTmpBufBpp - 10);
	}

	ASSERT(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp > 11);
	horizontal_frontporch_byte			=	(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp - 12);
	horizontal_bllp_byte				=	(lcm_params->dsi.horizontal_bllp * dsiTmpBufBpp);
//	ASSERT(lcm_params->dsi.horizontal_frontporch * dsiTmpBufBpp > ((300/dsi_cycle_time) * lcm_params->dsi.LANE_NUM));
//	horizontal_frontporch_byte -= ((300/dsi_cycle_time) * lcm_params->dsi.LANE_NUM);

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_HSA_WC_REG,&DSI_REG->DSI_HSA_WC, ALIGN_TO((horizontal_sync_active_byte), 4));
	DSI_OUTREG32_V(PDSI_HBP_WC_REG,&DSI_REG->DSI_HBP_WC, ALIGN_TO((horizontal_backporch_byte), 4));
	DSI_OUTREG32_V(PDSI_HFP_WC_REG,&DSI_REG->DSI_HFP_WC, ALIGN_TO((horizontal_frontporch_byte), 4));
	DSI_OUTREG32_V(PDSI_BLLP_WC_REG,&DSI_REG->DSI_BLLP_WC, ALIGN_TO((horizontal_bllp_byte), 4));
#else
	OUTREG32(&DSI_REG->DSI_HSA_WC, ALIGN_TO((horizontal_sync_active_byte), 4));
	OUTREG32(&DSI_REG->DSI_HBP_WC, ALIGN_TO((horizontal_backporch_byte), 4));
	OUTREG32(&DSI_REG->DSI_HFP_WC, ALIGN_TO((horizontal_frontporch_byte), 4));
	OUTREG32(&DSI_REG->DSI_BLLP_WC, ALIGN_TO((horizontal_bllp_byte), 4));
#endif
	_dsiContext.vfp_period_us 		= LINE_PERIOD_US * lcm_params->dsi.vertical_frontporch / 1000;
	_dsiContext.vsa_vs_period_us	= LINE_PERIOD_US * 1 / 1000;
	_dsiContext.vsa_hs_period_us	= LINE_PERIOD_US * (lcm_params->dsi.vertical_sync_active - 2) / 1000;
	_dsiContext.vsa_ve_period_us	= LINE_PERIOD_US * 1 / 1000;
	_dsiContext.vbp_period_us		= LINE_PERIOD_US * lcm_params->dsi.vertical_backporch / 1000;

	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - video timing, mode = %d \n", lcm_params->dsi.mode);
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VSA : %d %d(us)\n", DSI_REG->DSI_VSA_NL, (_dsiContext.vsa_vs_period_us+_dsiContext.vsa_hs_period_us+_dsiContext.vsa_ve_period_us));
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VBP : %d %d(us)\n", DSI_REG->DSI_VBP_NL, _dsiContext.vbp_period_us);
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VFP : %d %d(us)\n", DSI_REG->DSI_VFP_NL, _dsiContext.vfp_period_us);
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - VACT: %d \n", DSI_REG->DSI_VACT_NL);
}

void DSI_PHY_clk_setting(LCM_PARAMS *lcm_params)
{
	unsigned int data_Rate = lcm_params->dsi.PLL_CLOCK*2;
	unsigned int txdiv,pcw;
//	unsigned int fmod = 30;//Fmod = 30KHz by default
	unsigned int delta1 = 5;//Delta1 is SSC range, default is 0%~-5%
	unsigned int pdelta1;
	u32 m_hw_res3;
	u32 temp1, temp2,temp3,temp4,temp5;

	m_hw_res3 = DSI_INREG32(u32*,0x10206180);
	temp1 = (m_hw_res3 >> 28) & 0xF;
	temp2 = (m_hw_res3 >> 24) & 0xF;
	temp3 = (m_hw_res3 >> 20) & 0xF;
	temp4 = (m_hw_res3 >> 16) & 0xF;
	temp5 = (m_hw_res3 >> 12) & 0xF;

	printk("DSI_PHY_clk_adjusting: efuse register 0x10206180=0x%x, bit28_31=0x%x, bit24_27=0x%x, bit20_23=0x%x,bit16_19=0x%x,bit12_15=0x%x\n", 
				m_hw_res3, temp1, temp2,temp3,temp4,temp5);
	printk("before efuse adjust, DSI_CLK_REG = 0x%x, DSI_DAT0_REG=0x%x,DSI_DAT1_REG=0x%x,DSI_DAT2_REG=0x%x,DSI_DAT3_REG=0x%x\n", 
		DSI_INREG32(PMIPITX_DSI0_CLOCK_LANE_REG,&DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE0_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE1_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE2_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE3_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3));

	DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_IMP_CAL_CODE,8);
	DSI_OUTREGBIT(MIPITX_DSI_TOP_CON_REG,DSI_PHY_REG->MIPITX_DSI_TOP_CON,RG_DSI_LNT_HS_BIAS_EN,1);

	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V032_SEL,4);
	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V04_SEL,4);
	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V072_SEL,4);
	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V10_SEL,4);
	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_V12_SEL,4);
	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CKEN,1);
	DSI_OUTREGBIT(MIPITX_DSI_BG_CON_REG,DSI_PHY_REG->MIPITX_DSI_BG_CON,RG_DSI_BG_CORE_EN,1);

	mdelay(10);

	DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_CKG_LDOOUT_EN,1);
	DSI_OUTREGBIT(MIPITX_DSI0_CON_REG,DSI_PHY_REG->MIPITX_DSI0_CON,RG_DSI0_LDOCORE_EN,1);

	DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_PWR_ON,1);
	DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,1);
	mdelay(10);

	DSI_OUTREGBIT(MIPITX_DSI_PLL_PWR_REG,DSI_PHY_REG->MIPITX_DSI_PLL_PWR,DA_DSI0_MPPLL_SDM_ISO_EN,0);

	DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PREDIV,0);
	DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_POSDIV,0);

	if(0!=data_Rate){//if lcm_params->dsi.PLL_CLOCK=0, use other method
		if(data_Rate > 1250){
			printk("[dsi_drv.c error]Data Rate exceed limitation\n");
			ASSERT(0);
		}
		else if(data_Rate >= 500)
			txdiv = 1;
		else if(data_Rate >= 250)
			txdiv = 2;
		else if(data_Rate >= 125)
			txdiv = 4;
		else if(data_Rate > 62)
			txdiv = 8;
		else if(data_Rate >= 50)
			txdiv = 16;
		else{
			printk("[dsi_drv.c Error]: dataRate is too low,%d!!!\n", __LINE__);
			ASSERT(0);
		}
		//PLL txdiv config
		switch(txdiv)
		{
			case 1:
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,0);
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,0);
				break;
			case 2:
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,1);
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,0);
				break;
			case 4:
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,2);
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,0);
				break;
			case 8:
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,2);
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,1);
				break;
			case 16:
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,2);
				DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,2);
				break;

			default:
				break;
		}

		// PLL PCW config
		/*
		PCW bit 24~30 = floor(pcw)
		PCW bit 16~23 = (pcw - floor(pcw))*256
		PCW bit 8~15 = (pcw*256 - floor(pcw)*256)*256
		PCW bit 8~15 = (pcw*256*256 - floor(pcw)*256*256)*256
		*/
		//	pcw = data_Rate*4*txdiv/(26*2);//Post DIV =4, so need data_Rate*4
		pcw = data_Rate*txdiv/13;

		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_H,(pcw & 0x7F));
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_16_23,((256*(data_Rate*txdiv%13)/13) & 0xFF));
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_8_15,((256*(256*(data_Rate*txdiv%13)%13)/13) & 0xFF));
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_0_7,((256*(256*(256*(data_Rate*txdiv%13)%13)%13)/13) & 0xFF));

		//SSC config
//		pmod = ROUND(1000*26MHz/fmod/2);fmod default is 30Khz, and this value not be changed
//		pmod = 433.33;
		if(1 != lcm_params->dsi.ssc_disable){
			DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PH_INIT,1);
			DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_PRD,0x1B1);//PRD=ROUND(pmod) = 433;
			if(0 != lcm_params->dsi.ssc_range)
				delta1 = lcm_params->dsi.ssc_range;
			ASSERT(delta1<=8);
			pdelta1 = (delta1*data_Rate*txdiv*262144+281664)/563329;
			DSI_OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA,pdelta1);
			DSI_OUTREGBIT(MIPITX_DSI_PLL_CON3_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON3,RG_DSI0_MPPLL_SDM_SSC_DELTA1,pdelta1);
			DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,1);
			printk("[dsi_drv.c] PLL config:data_rate=%d,txdiv=%d,pcw=%d,delta1=%d,pdelta1=0x%x\n",
				data_Rate,txdiv,DSI_INREG32(PMIPITX_DSI_PLL_CON2_REG,&DSI_PHY_REG->MIPITX_DSI_PLL_CON2),delta1,pdelta1);
		}
	}
	else{
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV0,lcm_params->dsi.pll_div1);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_TXDIV1,lcm_params->dsi.pll_div2);

		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_H,((lcm_params->dsi.fbk_div)<< 2));
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_16_23,0);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_8_15,0);
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON2_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON2,RG_DSI0_MPPLL_SDM_PCW_0_7,0);

		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_FRA_EN,0);
	}

	DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_RT_CODE,(temp1 == 0)?0x8:temp1);
	DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_PHI_SEL,0x1);
	DSI_OUTREGBIT(MIPITX_DSI0_CLOCK_LANE_REG,DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE,RG_DSI0_LNTC_LDOOUT_EN,1);
	if(lcm_params->dsi.LANE_NUM > 0)
	{
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_RT_CODE,(temp2 == 0)?0x8:temp2);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE0_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0,RG_DSI0_LNT0_LDOOUT_EN,1);
	}

	if(lcm_params->dsi.LANE_NUM > 1)
	{
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_RT_CODE,(temp3 == 0)?0x8:temp3);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE1_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1,RG_DSI0_LNT1_LDOOUT_EN,1);
	}

	if(lcm_params->dsi.LANE_NUM > 2)
	{
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_RT_CODE,(temp4 == 0)?0x8:temp4);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE2_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2,RG_DSI0_LNT2_LDOOUT_EN,1);
	}

	if(lcm_params->dsi.LANE_NUM > 3)
	{
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE3_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3,RG_DSI0_LNT3_RT_CODE,(temp5 == 0)?0x8:temp5);
		DSI_OUTREGBIT(MIPITX_DSI0_DATA_LANE3_REG,DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3,RG_DSI0_LNT3_LDOOUT_EN,1);
	}
	printk("after efuse adjust, DSI_CLK_REG = 0x%x, DSI_DAT0_REG=0x%x,DSI_DAT1_REG=0x%x,DSI_DAT2_REG=0x%x,DSI_DAT3_REG=0x%x\n",
		DSI_INREG32(PMIPITX_DSI0_CLOCK_LANE_REG,&DSI_PHY_REG->MIPITX_DSI0_CLOCK_LANE),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE0_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE0),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE1_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE1),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE2_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE2),
		DSI_INREG32(PMIPITX_DSI0_DATA_LANE3_REG,&DSI_PHY_REG->MIPITX_DSI0_DATA_LANE3));

	DSI_OUTREGBIT(MIPITX_DSI_PLL_CON0_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON0,RG_DSI0_MPPLL_PLL_EN,1);
	mdelay(1);
	if((0 != data_Rate) && (1 != lcm_params->dsi.ssc_disable))
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_EN,1);
	else
		DSI_OUTREGBIT(MIPITX_DSI_PLL_CON1_REG,DSI_PHY_REG->MIPITX_DSI_PLL_CON1,RG_DSI0_MPPLL_SDM_SSC_EN,0);

	// default POSDIV by 4
	DSI_OUTREGBIT(MIPITX_DSI_PLL_TOP_REG,DSI_PHY_REG->MIPITX_DSI_PLL_TOP,RG_MPPLL_PRESERVE_L,3);
}

#ifdef BUILD_LK
void DSI_PHY_clk_switch(BOOL on)
{

}
#else
void DSI_PHY_clk_switch(BOOL on)
{
#if 0
	if(on)
	{
#ifdef DSI_MIPI_API
		enable_mipi(MT65XX_MIPI_TX, "DSI");
#endif
		mipitx_con0.PLL_EN=1;
		DSI_PHY_REG->MIPITX_CON0=mipitx_con0;
	}
	else
	{
		mipitx_con0=DSI_PHY_REG->MIPITX_CON0;
		mipitx_con1=DSI_PHY_REG->MIPITX_CON1;
		mipitx_con3=DSI_PHY_REG->MIPITX_CON3;
		mipitx_con6=DSI_PHY_REG->MIPITX_CON6;
		mipitx_con8=DSI_PHY_REG->MIPITX_CON8;
		mipitx_con9=DSI_PHY_REG->MIPITX_CON9;
#ifdef DSI_MIPI_API
		disable_mipi(MT65XX_MIPI_TX, "DSI");
#else
		OUTREG32(&DSI_PHY_REG->MIPITX_CON0, 0);
		OUTREG32(&DSI_PHY_REG->MIPITX_CON6, 0);
		OUTREG32(&DSI_PHY_REG->MIPITX_CON9, 0);
#endif
	}
#endif
}
#endif

void DSI_PHY_TIMCONFIG(LCM_PARAMS *lcm_params)
{
	DSI_PHY_TIMCON0_REG timcon0;
	DSI_PHY_TIMCON1_REG timcon1;
	DSI_PHY_TIMCON2_REG timcon2;
	DSI_PHY_TIMCON3_REG timcon3;
	unsigned int div1 = 0;
	unsigned int div2 = 0;
	unsigned int pre_div = 0;
	unsigned int post_div = 0;
	unsigned int fbk_sel = 0;
	unsigned int fbk_div = 0;
	unsigned int lane_no = lcm_params->dsi.LANE_NUM;

	//	unsigned int div2_real;
	unsigned int cycle_time;
	unsigned int ui;
	unsigned int hs_trail_m, hs_trail_n;

	if(0 != lcm_params->dsi.PLL_CLOCK){
		ui= 1000/(lcm_params->dsi.PLL_CLOCK*2)+0x01;
		cycle_time=8000/(lcm_params->dsi.PLL_CLOCK*2)+0x01;
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - _DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). , lane# = %d \n", cycle_time, ui, lane_no);
	}
	else{
		div1 = lcm_params->dsi.pll_div1;
		div2 = lcm_params->dsi.pll_div2;
		fbk_div = lcm_params->dsi.fbk_div;
   switch(div1)
   {
	  case 0:
		 div1 = 1;
		 break;

	  case 1:
		 div1 = 2;
		 break;

	  case 2:
	  case 3:
		 div1 = 4;
		 break;

	  default:
		 printk("div1 should be less than 4!!\n");
		 div1 = 4;
		 break;
   }

   switch(div2)
   {
	  case 0:
		 div2 = 1;
		 break;

	  case 1:
		 div2 = 2;
		 break;

	  case 2:
	  case 3:
		 div2 = 4;
		 break;

	  default:
		 printk("div2 should be less than 4!!\n");
		 div2 = 4;
		 break;
   }

   switch(pre_div)
   {
	  case 0:
		 pre_div = 1;
		 break;

	  case 1:
		 pre_div = 2;
		 break;

	  case 2:
	  case 3:
		 pre_div = 4;
		 break;

	  default:
		 printk("pre_div should be less than 4!!\n");
		 pre_div = 4;
		 break;
   }

   switch(post_div)
   {
	  case 0:
		 post_div = 1;
		 break;

	  case 1:
		 post_div = 2;
		 break;

	  case 2:
	  case 3:
		 post_div = 4;
		 break;

	  default:
		 printk("post_div should be less than 4!!\n");
		 post_div = 4;
		 break;
   }

   switch(fbk_sel)
   {
	  case 0:
		 fbk_sel = 1;
		 break;

	  case 1:
		 fbk_sel = 2;
		 break;

	  case 2:
	  case 3:
		 fbk_sel = 4;
		 break;

	  default:
		 printk("fbk_sel should be less than 4!!\n");
		 fbk_sel = 4;
		 break;
   }
	cycle_time=(1000*4*div2*div1)/(fbk_div*26)+0x01;

	ui=(1000*div2*div1)/(fbk_div*26*0x2)+0x01;
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, Cycle Time = %d(ns), Unit Interval = %d(ns). div1 = %d, div2 = %d, fbk_div = %d, lane# = %d \n", cycle_time, ui, div1, div2, fbk_div, lane_no);
	}

   //	div2_real=div2 ? div2*0x02 : 0x1;
   //cycle_time = (1000 * div2 * div1 * pre_div * post_div)/ (fbk_sel * (fbk_div+0x01) * 26) + 1;
   //ui = (1000 * div2 * div1 * pre_div * post_div)/ (fbk_sel * (fbk_div+0x01) * 26 * 2) + 1;
#define NS_TO_CYCLE(n, c)	((n) / (c))

   hs_trail_m=1;
   hs_trail_n= (lcm_params->dsi.HS_TRAIL == 0) ? NS_TO_CYCLE(((hs_trail_m * 0x4) + 0x60), cycle_time) : lcm_params->dsi.HS_TRAIL;
   // +3 is recommended from designer becauase of HW latency
   timcon0.HS_TRAIL = ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n) + 0x0a;

   timcon0.HS_PRPR	= (lcm_params->dsi.HS_PRPR == 0) ? NS_TO_CYCLE((0x40 + 0x5 * ui), cycle_time) : lcm_params->dsi.HS_PRPR;
   // HS_PRPR can't be 1.
   if (timcon0.HS_PRPR == 0)
	  timcon0.HS_PRPR = 1;

   timcon0.HS_ZERO	= (lcm_params->dsi.HS_ZERO == 0) ? NS_TO_CYCLE((0xC8 + 0x0a * ui), cycle_time) : lcm_params->dsi.HS_ZERO;
   if (timcon0.HS_ZERO > timcon0.HS_PRPR)
	  timcon0.HS_ZERO -= timcon0.HS_PRPR;

   timcon0.LPX		= (lcm_params->dsi.LPX == 0) ? NS_TO_CYCLE(0x50, cycle_time) : lcm_params->dsi.LPX;
   if(timcon0.LPX == 0)
	  timcon0.LPX = 1;

   //	timcon1.TA_SACK 	= (lcm_params->dsi.TA_SACK == 0) ? 1 : lcm_params->dsi.TA_SACK;
   timcon1.TA_GET		= (lcm_params->dsi.TA_GET == 0) ? (0x5 * timcon0.LPX) : lcm_params->dsi.TA_GET;
   timcon1.TA_SURE	= (lcm_params->dsi.TA_SURE == 0) ? (0x3 * timcon0.LPX / 0x2) : lcm_params->dsi.TA_SURE;
   timcon1.TA_GO		= (lcm_params->dsi.TA_GO == 0) ? (0x4 * timcon0.LPX) : lcm_params->dsi.TA_GO;
   // --------------------------------------------------------------
   //  NT35510 need fine tune timing
   //  Data_hs_exit = 60 ns + 128UI
   //  Clk_post = 60 ns + 128 UI.
   // --------------------------------------------------------------
   timcon1.DA_HS_EXIT  = (lcm_params->dsi.DA_HS_EXIT == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.DA_HS_EXIT;

   timcon2.CLK_TRAIL	= ((lcm_params->dsi.CLK_TRAIL == 0) ? NS_TO_CYCLE(0x64, cycle_time) : lcm_params->dsi.CLK_TRAIL) + 0x0a;
   // CLK_TRAIL can't be 1.
   if (timcon2.CLK_TRAIL < 2)
	  timcon2.CLK_TRAIL = 2;

   //	timcon2.LPX_WAIT	= (lcm_params->dsi.LPX_WAIT == 0) ? 1 : lcm_params->dsi.LPX_WAIT;
   timcon2.CONT_DET 	= lcm_params->dsi.CONT_DET;
   timcon2.CLK_ZERO = (lcm_params->dsi.CLK_ZERO == 0) ? NS_TO_CYCLE(0x190, cycle_time) : lcm_params->dsi.CLK_ZERO;

   timcon3.CLK_HS_PRPR	= (lcm_params->dsi.CLK_HS_PRPR == 0) ? NS_TO_CYCLE(0x40, cycle_time) : lcm_params->dsi.CLK_HS_PRPR;
   if(timcon3.CLK_HS_PRPR == 0)
	  timcon3.CLK_HS_PRPR = 1;
   timcon3.CLK_HS_EXIT= (lcm_params->dsi.CLK_HS_EXIT == 0) ? (2 * timcon0.LPX) : lcm_params->dsi.CLK_HS_EXIT;
   timcon3.CLK_HS_POST= (lcm_params->dsi.CLK_HS_POST == 0) ? NS_TO_CYCLE((0x3c + 0x80 * ui), cycle_time) : lcm_params->dsi.CLK_HS_POST;

   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] - kernel - DSI_PHY_TIMCONFIG, HS_TRAIL = %d, HS_ZERO = %d, HS_PRPR = %d, LPX = %d, TA_GET = %d, TA_SURE = %d, TA_GO = %d, CLK_TRAIL = %d, CLK_ZERO = %d, CLK_HS_PRPR = %d \n", \
   timcon0.HS_TRAIL, timcon0.HS_ZERO, timcon0.HS_PRPR, timcon0.LPX, timcon1.TA_GET, timcon1.TA_SURE, timcon1.TA_GO, timcon2.CLK_TRAIL, timcon2.CLK_ZERO, timcon3.CLK_HS_PRPR);

   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,LPX,timcon0.LPX);
   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_PRPR,timcon0.HS_PRPR);
   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_ZERO,timcon0.HS_ZERO);
   OUTREGBIT(DSI_PHY_TIMCON0_REG,DSI_REG->DSI_PHY_TIMECON0,HS_TRAIL,timcon0.HS_TRAIL);

   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GO,timcon1.TA_GO);
   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_SURE,timcon1.TA_SURE);
   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,TA_GET,timcon1.TA_GET);
   OUTREGBIT(DSI_PHY_TIMCON1_REG,DSI_REG->DSI_PHY_TIMECON1,DA_HS_EXIT,timcon1.DA_HS_EXIT);

   OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CONT_DET,timcon2.CONT_DET);
   OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_ZERO,timcon2.CLK_ZERO);
   OUTREGBIT(DSI_PHY_TIMCON2_REG,DSI_REG->DSI_PHY_TIMECON2,CLK_TRAIL,timcon2.CLK_TRAIL);

   OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_PRPR,timcon3.CLK_HS_PRPR);
   OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_POST,timcon3.CLK_HS_POST);
   OUTREGBIT(DSI_PHY_TIMCON3_REG,DSI_REG->DSI_PHY_TIMECON3,CLK_HS_EXIT,timcon3.CLK_HS_EXIT);
	printk("%s, 0x%08x,0x%08x,0x%08x,0x%08x\n", __func__, INREG32(DSI_BASE+0x110),INREG32(DSI_BASE+0x114),INREG32(DSI_BASE+0x118),INREG32(DSI_BASE+0x11c));
}



void DSI_clk_ULP_mode(BOOL enter)
{
	DSI_PHY_LCCON_REG tmp_reg1;
	//DSI_PHY_REG_ANACON0	tmp_reg2;

	tmp_reg1=DSI_REG->DSI_PHY_LCCON;
	//tmp_reg2=DSI_PHY_REG->ANACON0;

	if(enter) {

		tmp_reg1.LC_HS_TX_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		tmp_reg1.LC_ULPM_EN=1;

#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		//tmp_reg2.PLL_EN=0;
		//OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&tmp_reg2));

	}
	else {

		//tmp_reg2.PLL_EN=1;
		//OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&tmp_reg2));
		lcm_mdelay(1);
		tmp_reg1.LC_ULPM_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		tmp_reg1.LC_WAKEUP_EN=1;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		tmp_reg1.LC_WAKEUP_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);

	}
}


void DSI_clk_HS_mode(BOOL enter)
{
	DSI_PHY_LCCON_REG tmp_reg1 = DSI_REG->DSI_PHY_LCCON;

	if(enter && !DSI_clk_HS_state()) {
		tmp_reg1.LC_HS_TX_EN=1;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		//lcm_mdelay(1);
	}
	else if (!enter && DSI_clk_HS_state()) {
		tmp_reg1.LC_HS_TX_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LCCON_REG,&DSI_REG->DSI_PHY_LCCON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
#endif
		//lcm_mdelay(1);

	}
}


void DSI_Continuous_HS(void)
{
    DSI_TXRX_CTRL_REG tmp_reg = DSI_REG->DSI_TXRX_CTRL;

    tmp_reg.HSTX_CKLP_EN = 0;
    DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &tmp_reg);
}


BOOL DSI_clk_HS_state(void)
{
	return DSI_REG->DSI_PHY_LCCON.LC_HS_TX_EN ? TRUE : FALSE;
}


void DSI_lane0_ULP_mode(BOOL enter)
{
	DSI_PHY_LD0CON_REG tmp_reg1;

	tmp_reg1=DSI_REG->DSI_PHY_LD0CON;

	if(enter) {
		// suspend
		tmp_reg1.L0_HS_TX_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		tmp_reg1.L0_ULPM_EN=1;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
	}
	else {
		// resume
		tmp_reg1.L0_ULPM_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		tmp_reg1.L0_WAKEUP_EN=1;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
		tmp_reg1.L0_WAKEUP_EN=0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_PHY_LD0CON_REG,&DSI_REG->DSI_PHY_LD0CON, &tmp_reg1);
#else
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
#endif
		lcm_mdelay(1);
	}
}

#ifndef BUILD_LK

// called by DPI ISR
void DSI_handle_esd_recovery(void)
{
#ifndef MT65XX_NEW_DISP
	long int dsi_current_time;
	unsigned int state;

	dsi_dpi_isr_count++;

	if (dsi_noncont_clk_enabled && (dsi_dpi_isr_count % dsi_noncont_clk_period)==0) {
		//DSI_handle_noncont_clk();
		{
			if (!DSI_REG->DSI_MODE_CTRL.MODE)
				return ;

			dsi_current_time = get_current_time_us();

			state = DSI_REG->DSI_STATE_DBG3.TCON_STATE;

			switch(state)
			{
				case DSI_VDO_VSA_VS_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_HS_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_vs_period_us)
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_HS_STATE, _dsiContext.vsa_vs_period_us);
							return ;
						}
					}
					break;

				case DSI_VDO_VSA_HS_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VE_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_hs_period_us)
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VE_STATE, _dsiContext.vsa_hs_period_us);
							return ;
						}
					}
					break;

				case DSI_VDO_VSA_VE_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VBP_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_ve_period_us)
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VBP_STATE, _dsiContext.vsa_ve_period_us);
							return ;
						}
					}
					break;

				case DSI_VDO_VBP_STATE:
					xlog_printk(ANDROID_LOG_WARN, "DSI", "Can't do clock switch in DSI_VDO_VBP_STATE !!!\n");
					break;

				case DSI_VDO_VACT_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VFP_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VFP_STATE, _dsiContext.vfp_period_us );
							return ;
						}
					}
					break;

				case DSI_VDO_VFP_STATE:
					while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VS_STATE)
					{
						if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
						{
							xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VS_STATE, _dsiContext.vfp_period_us );
							return ;
						}
					}
					break;

				default :
					xlog_printk(ANDROID_LOG_ERROR, "DSI", "invalid state = %x \n", state);
					return ;
			}

			// Clock switch HS->LP->HS
			if(DSI_REG->DSI_STATE_DBG0.CTL_STATE_0 == 0x1)
			{
				spin_lock_irqsave(&g_handle_esd_lock, g_handle_esd_flag);

				DSI_clk_HS_mode(0);
				DSI_clk_HS_mode(1);

				spin_unlock_irqrestore(&g_handle_esd_lock, g_handle_esd_flag);

			}

			return ;
		}
	}

	if (dsi_int_te_enabled && (dsi_dpi_isr_count % dsi_int_te_period)==0) {
		spin_lock_irqsave(&g_handle_esd_lock, g_handle_esd_flag);

		//mt65xx_irq_mask(MT6575_DSI_IRQ_ID);
		disable_irq(MT_DSI_IRQ_ID);

		dsi_esd_recovery = DSI_handle_int_TE();

		//mt65xx_irq_unmask(MT6575_DSI_IRQ_ID);
		enable_irq(MT_DSI_IRQ_ID);

		spin_unlock_irqrestore(&g_handle_esd_lock, g_handle_esd_flag);
	}
#endif
}


// called by "esd_recovery_kthread"
BOOL DSI_esd_check(void)
{
#ifndef MT65XX_NEW_DISP
	BOOL result = false;

	if(dsi_esd_recovery)
		result = true;
	else
		result = false;

	dsi_esd_recovery = false;

	return result;
#endif
}


void DSI_set_int_TE(BOOL enable, unsigned int period)
{
#ifndef MT65XX_NEW_DISP
	dsi_int_te_enabled = enable;
	if(period<1)
		period = 1;
	dsi_int_te_period = period;
	dsi_dpi_isr_count = 0;
#endif
}


// called by DPI ISR.
BOOL DSI_handle_int_TE(void)
{
#ifndef MT65XX_NEW_DISP
	DSI_T0_INS t0;
	long int dsi_current_time;

	if (!DSI_REG->DSI_MODE_CTRL.MODE)
		return false;

	dsi_current_time = get_current_time_us();

	if(DSI_REG->DSI_STATE_DBG3.TCON_STATE == DSI_VDO_VFP_STATE)
	{
		udelay(_dsiContext.vfp_period_us / 2);

		if ((DSI_REG->DSI_STATE_DBG3.TCON_STATE == DSI_VDO_VFP_STATE) && DSI_REG->DSI_STATE_DBG0.CTL_STATE_0 == 0x1)
		{
			// Can't do int. TE check while INUSE FB number is not 0 because later disable/enable DPI will set INUSE FB to number 0.
			if(DPI_REG->STATUS.FB_INUSE != 0)
				return false;

			DSI_clk_HS_mode(0);

			//DSI_REG->DSI_COM_CTRL.DSI_RESET = 1;
			OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,1);
			DPI_DisableClk();
			DSI_SetMode(CMD_MODE);
			//DSI_REG->DSI_COM_CTRL.DSI_RESET = 0;
			OUTREGBIT(DSI_COM_CTRL_REG,DSI_REG->DSI_COM_CTRL,DSI_RESET,0);
			//DSI_Reset();

			t0.CONFG = 0x20;		///TE
			t0.Data0 = 0;
			t0.Data_ID = 0;
			t0.Data1 = 0;

			OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

			// Enable RD_RDY INT for polling it's status later
			//DSI_REG->DSI_INTEN.RD_RDY =  1;
			OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);

			DSI_EnableClk();

	        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  // polling RD_RDY
	        {
				if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for internal TE time-out for %d (us)!!!\n", _dsiContext.vfp_period_us);

					///do necessary reset here
					//DSI_REG->DSI_RACK.DSI_RACK = 1;
					OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
					DSI_Reset();

					return true;
				}
	        }

			// Write clear RD_RDY
			//DSI_REG->DSI_INTSTA.RD_RDY = 1;
			//DSI_REG->DSI_RACK.DSI_RACK = 1;
			OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
			OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
			// Write clear CMD_DONE
			//DSI_REG->DSI_INTSTA.CMD_DONE = 1;
			OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);

			// Restart video mode. (with VSA ahead)
			DSI_SetMode(SYNC_PULSE_VDO_MODE);
			DSI_clk_HS_mode(1);
			DPI_EnableClk();
			DSI_EnableClk();
		}

	}
#endif
	return false;

}


void DSI_set_noncont_clk(BOOL enable, unsigned int period)
{
#ifndef MT65XX_NEW_DISP
	dsi_noncont_clk_enabled = enable;
	if(period<1)
		period = 1;
	dsi_noncont_clk_period = period;
	dsi_dpi_isr_count = 0;
#endif
}


// called by DPI ISR.
void DSI_handle_noncont_clk(void)
{
#ifndef MT65XX_NEW_DISP
	unsigned int state;
	long int dsi_current_time;

	if (!DSI_REG->DSI_MODE_CTRL.MODE)
		return ;

	state = DSI_REG->DSI_STATE_DBG3.TCON_STATE;

	dsi_current_time = get_current_time_us();

	switch(state)
	{
		case DSI_VDO_VSA_VS_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_HS_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_vs_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_HS_STATE, _dsiContext.vsa_vs_period_us);
					return ;
				}
			}
			break;

		case DSI_VDO_VSA_HS_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VE_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_hs_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VE_STATE, _dsiContext.vsa_hs_period_us);
					return ;
				}
			}
			break;

		case DSI_VDO_VSA_VE_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VBP_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vsa_ve_period_us)
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VBP_STATE, _dsiContext.vsa_ve_period_us);
					return ;
				}
			}
			break;

		case DSI_VDO_VBP_STATE:
			xlog_printk(ANDROID_LOG_WARN, "DSI", "Can't do clock switch in DSI_VDO_VBP_STATE !!!\n");
			break;

		case DSI_VDO_VACT_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VFP_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VFP_STATE, _dsiContext.vfp_period_us );
					return ;
				}
			}
			break;

		case DSI_VDO_VFP_STATE:
			while(DSI_REG->DSI_STATE_DBG3.TCON_STATE != DSI_VDO_VSA_VS_STATE)
			{
				if(get_current_time_us() - dsi_current_time > _dsiContext.vfp_period_us )
				{
					xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for %x state timeout %d (us)!!!\n", DSI_VDO_VSA_VS_STATE, _dsiContext.vfp_period_us );
					return ;
				}
			}
			break;

		default :
			xlog_printk(ANDROID_LOG_ERROR, "DSI", "invalid state = %x \n", state);
			return ;
	}

	// Clock switch HS->LP->HS
	DSI_clk_HS_mode(0);
	udelay(1);
	DSI_clk_HS_mode(1);
#endif
}
#endif

DSI_STATUS DSI_EnableVM_CMD(void)
{
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,0);
	DSI_OUTREGBIT(DSI_START_REG,DSI_REG->DSI_START,VM_CMD_START,1);
    return DSI_STATUS_OK;
}

void DSI_set_cmdq_V2(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update)
{
#ifdef FIX_BUILD_WARNING_0616
	UINT32 i;
	UINT32 goto_addr, mask_para, set_para;
	DSI_T0_INS t0;
	DSI_T2_INS t2;
#else
	UINT32 i, layer, layer_state, lane_num;
	UINT32 goto_addr, mask_para, set_para;
	UINT32 fbPhysAddr, fbVirAddr;
	DSI_T0_INS t0;
	DSI_T1_INS t1;
	DSI_T2_INS t2;
#endif
	if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
		DSI_VM_CMD_CON_REG vm_cmdq;
		//OUTREG32(&vm_cmdq, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));
		DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &vm_cmdq, &DSI_REG->DSI_VM_CMD_CON);
		printf("set cmdq in VDO mode in set_cmdq_V2\n");
		if (cmd < 0xB0)
		{
			if (count > 1)
			{
				vm_cmdq.LONG_PKT = 1;
				vm_cmdq.CM_DATA_ID = DSI_DCS_LONG_PACKET_ID;
				vm_cmdq.CM_DATA_0 = count+1;
				//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
				DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);

				goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (cmd<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);

				for(i=0; i<count; i++)
				{
					goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
					mask_para = (0xFF<<((goto_addr&0x3)*8));
					set_para = (para_list[i]<<((goto_addr&0x3)*8));
					MASKREG32(goto_addr&(~0x3), mask_para, set_para);
				}
			}
			else
			{
				vm_cmdq.LONG_PKT = 0;
				vm_cmdq.CM_DATA_0 = cmd;
				if (count)
				{
					vm_cmdq.CM_DATA_ID = DSI_DCS_SHORT_PACKET_ID_1;
					vm_cmdq.CM_DATA_1 = para_list[0];
				}
				else
				{
					vm_cmdq.CM_DATA_ID = DSI_DCS_SHORT_PACKET_ID_0;
					vm_cmdq.CM_DATA_1 = 0;
				}
				//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
				DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
			}
		}
		else{
			if (count > 1)
			{
				vm_cmdq.LONG_PKT = 1;
				vm_cmdq.CM_DATA_ID = DSI_GERNERIC_LONG_PACKET_ID;
				vm_cmdq.CM_DATA_0 = count+1;
				//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
				DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);

				goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (cmd<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);

				for(i=0; i<count; i++)
				{
					goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
					mask_para = (0xFF<<((goto_addr&0x3)*8));
					set_para = (para_list[i]<<((goto_addr&0x3)*8));
					MASKREG32(goto_addr&(~0x3), mask_para, set_para);
				}
			}
			else
			{
				vm_cmdq.LONG_PKT = 0;
				vm_cmdq.CM_DATA_0 = cmd;
				if (count)
				{
					vm_cmdq.CM_DATA_ID = DSI_GERNERIC_SHORT_PACKET_ID_2;
					vm_cmdq.CM_DATA_1 = para_list[0];
				}
				else
				{
					vm_cmdq.CM_DATA_ID = DSI_GERNERIC_SHORT_PACKET_ID_1;
					vm_cmdq.CM_DATA_1 = 0;
				}
				//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
				DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
			}
		}
		//start DSI VM CMDQ
		if(force_update){
			DSI_EnableVM_CMD();
		}
	}
	else{
    _WaitForEngineNotBusy();
	if (cmd < 0xB0)
	{
		if (count > 1)
		{
			t2.CONFG = 2;
			t2.Data_ID = DSI_DCS_LONG_PACKET_ID;
			t2.WC16 = count+1;

#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t2);
#else
		OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t2));
#endif
			goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
			mask_para = (0xFF<<((goto_addr&0x3)*8));
			set_para = (cmd<<((goto_addr&0x3)*8));
			MASKREG32(goto_addr&(~0x3), mask_para, set_para);

			for(i=0; i<count; i++)
			{
				goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (para_list[i]<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);
			}

#ifdef FIX_BUILD_WARNING_0616
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
#else
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
#endif
		}
		else
		{
			t0.CONFG = 0;
			t0.Data0 = cmd;
			if (count)
			{
				t0.Data_ID = DSI_DCS_SHORT_PACKET_ID_1;
				t0.Data1 = para_list[0];
			}
			else
			{
				t0.Data_ID = DSI_DCS_SHORT_PACKET_ID_0;
				t0.Data1 = 0;
			}
#ifdef FIX_BUILD_WARNING_0616
			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
#else
			OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
#endif
		}
	}
	else
	{
		if (count > 1)
		{
			t2.CONFG = 2;
			t2.Data_ID = DSI_GERNERIC_LONG_PACKET_ID;
			t2.WC16 = count+1;

#ifdef FIX_BUILD_WARNING_0616
			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t2);
#else
			OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t2));
#endif

			goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
			mask_para = (0xFF<<((goto_addr&0x3)*8));
			set_para = (cmd<<((goto_addr&0x3)*8));
			MASKREG32(goto_addr&(~0x3), mask_para, set_para);

			for(i=0; i<count; i++)
			{
				goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (para_list[i]<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);
			}
#ifdef FIX_BUILD_WARNING_0616
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
#else
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
#endif

		}
		else
		{
			t0.CONFG = 0;
			t0.Data0 = cmd;
			if (count)
			{
				t0.Data_ID = DSI_GERNERIC_SHORT_PACKET_ID_2;
				t0.Data1 = para_list[0];
			}
			else
			{
				t0.Data_ID = DSI_GERNERIC_SHORT_PACKET_ID_1;
				t0.Data1 = 0;
			}
#ifdef FIX_BUILD_WARNING_0616
			DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
			DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
#else
			OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
			OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
#endif
		}
	}

	//for (i = 0; i < AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE); i++)
    //    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V2. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));

		if(force_update)
			DSI_EnableClk();
	}

}

void DSI_set_cmdq_V3(LCM_setting_table_V3 *para_tbl, unsigned int size, unsigned char force_update)
{
#ifdef FIX_BUILD_WARNING_0616
	UINT32 i;
	UINT32 goto_addr, mask_para, set_para;
	DSI_T0_INS t0;
	DSI_T2_INS t2;
#else
	UINT32 i, layer, layer_state, lane_num;
	UINT32 goto_addr, mask_para, set_para;
	UINT32 fbPhysAddr, fbVirAddr;
	DSI_T0_INS t0;
	DSI_T1_INS t1;
	DSI_T2_INS t2;
#endif
	UINT32 index = 0;

	unsigned char data_id, cmd, count;
	unsigned char *para_list;

	do {
		data_id = para_tbl[index].id;
		cmd = para_tbl[index].cmd;
		count = para_tbl[index].count;
		para_list = para_tbl[index].para_list;

		if (data_id == REGFLAG_ESCAPE_ID && cmd == REGFLAG_DELAY_MS_V3)
		{
			udelay(1000*count);
			DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. Delay %d (ms) \n", index, count);

			continue;
		}
		if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
			DSI_VM_CMD_CON_REG vm_cmdq;
			//OUTREG32(&vm_cmdq, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));
			DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &vm_cmdq, &DSI_REG->DSI_VM_CMD_CON);
			printk("set cmdq in VDO mode\n");
			if (count > 1)
			{
				vm_cmdq.LONG_PKT = 1;
				vm_cmdq.CM_DATA_ID = data_id;
				vm_cmdq.CM_DATA_0 = count+1;
				//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
				DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);

				goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte0);
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (cmd<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);

				for(i=0; i<count; i++)
				{
					goto_addr = (UINT32)(&DSI_VM_CMD_REG->data[0].byte1) + i;
					mask_para = (0xFF<<((goto_addr&0x3)*8));
					set_para = (para_list[i]<<((goto_addr&0x3)*8));
					MASKREG32(goto_addr&(~0x3), mask_para, set_para);
				}
			}
			else
			{
				vm_cmdq.LONG_PKT = 0;
				vm_cmdq.CM_DATA_0 = cmd;
				if (count)
				{
					vm_cmdq.CM_DATA_ID = data_id;
					vm_cmdq.CM_DATA_1 = para_list[0];
				}
				else
				{
					vm_cmdq.CM_DATA_ID = data_id;
					vm_cmdq.CM_DATA_1 = 0;
				}
				//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
				DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
			}
		//start DSI VM CMDQ
			if(force_update){
				DSI_EnableVM_CMD();
			}
		}
		else{
		_WaitForEngineNotBusy();
		{
			//for(i = 0; i < sizeof(DSI_CMDQ_REG->data0) / sizeof(DSI_CMDQ); i++)
			//	OUTREG32(&DSI_CMDQ_REG->data0[i], 0);
			//memset(&DSI_CMDQ_REG->data[0], 0, sizeof(DSI_CMDQ_REG->data[0]));
			//OUTREG32(&DSI_CMDQ_REG->data[0], 0);
			DSI_OUTREG32_V(PDSI_CMDQ, &DSI_CMDQ_REG->data[0], 0);

			if (count > 1)
			{
				t2.CONFG = 2;
				t2.Data_ID = data_id;
				t2.WC16 = count+1;
#ifdef FIX_BUILD_WARNING_0616
				DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t2);
#else
				OUTREG32(&DSI_CMDQ_REG->data[0].byte0, AS_UINT32(&t2));
#endif
				goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte0);
				mask_para = (0xFF<<((goto_addr&0x3)*8));
				set_para = (cmd<<((goto_addr&0x3)*8));
				MASKREG32(goto_addr&(~0x3), mask_para, set_para);

				for(i=0; i<count; i++)
				{
					goto_addr = (UINT32)(&DSI_CMDQ_REG->data[1].byte1) + i;
					mask_para = (0xFF<<((goto_addr&0x3)*8));
					set_para = (para_list[i]<<((goto_addr&0x3)*8));
					MASKREG32(goto_addr&(~0x3), mask_para, set_para);
				}
#ifdef FIX_BUILD_WARNING_0616
				DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
#else
				OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 2+(count)/4);
#endif
			}
			else
			{
				t0.CONFG = 0;
				t0.Data0 = cmd;
				if (count)
				{
					t0.Data_ID = data_id;
					t0.Data1 = para_list[0];
				}
				else
				{
					t0.Data_ID = data_id;
					t0.Data1 = 0;
				}
#ifdef FIX_BUILD_WARNING_0616
				DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
				DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
#else
				OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
				OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
#endif
			}

			//for (i = 0; i < AS_UINT32(&DSI_REG->DSI_CMDQ_SIZE); i++)
			//	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_set_cmdq_V3[%d]. DSI_CMDQ+%04x : 0x%08x\n", index, i*4, INREG32(DSI_BASE + 0x180 + i*4));

			if(force_update)
				DSI_EnableClk();
		}
		}

	} while (++index < size);

}

void DSI_set_cmdq(unsigned int *pdata, unsigned int queue_size, unsigned char force_update)
{
	UINT32 i;

	if (0 != DSI_REG->DSI_MODE_CTRL.MODE){//not in cmd mode
		DSI_VM_CMD_CON_REG vm_cmdq;
		//OUTREG32(&vm_cmdq, AS_UINT32(&DSI_REG->DSI_VM_CMD_CON));
		DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &vm_cmdq, &DSI_REG->DSI_VM_CMD_CON);
		printk("set cmdq in VDO mode\n");
		if(queue_size > 1){//long packet
			unsigned int i = 0;
			vm_cmdq.LONG_PKT = 1;
			vm_cmdq.CM_DATA_ID = ((pdata[0] >> 8) & 0xFF);
			vm_cmdq.CM_DATA_0 = ((pdata[0] >> 16) & 0xFF);
			//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
			DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
			for(i=0;i<queue_size-1;i++)
			{
				//OUTREG32(&DSI_VM_CMD_REG->data[i], AS_UINT32((pdata+i+1)));
				DSI_OUTREG32_V(PDSI_VM_CMDQ, &DSI_VM_CMD_REG->data[i], AS_UINT32(pdata+i+1));
			}
		}
		else{
			vm_cmdq.LONG_PKT = 0;
			vm_cmdq.CM_DATA_ID = ((pdata[0] >> 8) & 0xFF);
			vm_cmdq.CM_DATA_0 = ((pdata[0] >> 16) & 0xFF);
			vm_cmdq.CM_DATA_1 = ((pdata[0] >> 24) & 0xFF);
			//OUTREG32(&DSI_REG->DSI_VM_CMD_CON, AS_UINT32(&vm_cmdq));
			DSI_OUTREG32_R(PDSI_VM_CMD_CON_REG, &DSI_REG->DSI_VM_CMD_CON, &vm_cmdq);
		}
		//start DSI VM CMDQ
		if(force_update){
			DSI_EnableVM_CMD();
		}
	}
	else{
	ASSERT(queue_size<=32);
    _WaitForEngineNotBusy();

	for(i=0; i<queue_size; i++)
	{
		//OUTREG32(&DSI_CMDQ_REG->data[i], AS_UINT32((pdata+i)));
		DSI_OUTREG32_V(PDSI_CMDQ, &DSI_CMDQ_REG->data[i], AS_UINT32(pdata+i));
	}

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, queue_size);
#else
	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, queue_size);
#endif
    //for (i = 0; i < queue_size; i++)
     //   printf("[DISP] - kernel - DSI_set_cmdq. DSI_CMDQ+%04x : 0x%08x\n", i*4, INREG32(DSI_BASE + 0x180 + i*4));

	if(force_update)
		DSI_EnableClk();
	}
}


DSI_STATUS DSI_Write_T0_INS(DSI_T0_INS *t0)
{
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t0));
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);
#else
	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);
#endif
	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T1_INS(DSI_T1_INS *t1)
{
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t1));
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);
#else
	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);
#endif
	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T2_INS(DSI_T2_INS *t2)
{
	unsigned int i;

	OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t2));

#ifdef FIX_BUILD_WARNING_0616
	for(i=0;i<(unsigned int)((t2->WC16-1)>>2)+1;i++)
	    OUTREG32(&DSI_CMDQ_REG->data[1+i], AS_UINT32((t2->pdata+i)));

	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, (((t2->WC16-1)>>2)+2));
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);
#else
	for(i=0;i<((t2->WC16-1)>>2)+1;i++)
	    OUTREG32(&DSI_CMDQ_REG->data[1+i], AS_UINT32((t2->pdata+i)));

	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, (((t2->WC16-1)>>2)+2));
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);
#endif
	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T3_INS(DSI_T3_INS *t3)
{
    OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(t3));

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
	DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);
#else
	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);
#endif
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_TXRX_Control(BOOL cksm_en,
                                  BOOL ecc_en,
                                  unsigned char lane_num,
                                  unsigned char vc_num,
                                  BOOL null_packet_en,
                                  BOOL err_correction_en,
                                  BOOL dis_eotp_en,
								  BOOL hstx_cklp_en,
                                  unsigned int  max_return_size)
{
    DSI_TXRX_CTRL_REG tmp_reg;

    tmp_reg=DSI_REG->DSI_TXRX_CTRL;

    ///TODO: parameter checking
//    tmp_reg.CKSM_EN=cksm_en;
//    tmp_reg.ECC_EN=ecc_en;
	switch(lane_num)
	{
		case LCM_ONE_LANE:tmp_reg.LANE_NUM = 1;break;
		case LCM_TWO_LANE:tmp_reg.LANE_NUM = 3;break;
		case LCM_THREE_LANE:tmp_reg.LANE_NUM = 0x7;break;
		case LCM_FOUR_LANE:tmp_reg.LANE_NUM = 0xF;break;
	}
    tmp_reg.VC_NUM=vc_num;
//    tmp_reg.CORR_EN = err_correction_en;
    tmp_reg.DIS_EOT = dis_eotp_en;
    tmp_reg.NULL_EN = null_packet_en;
    tmp_reg.MAX_RTN_SIZE = max_return_size;
	tmp_reg.HSTX_CKLP_EN = hstx_cklp_en;
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_R(PDSI_TXRX_CTRL_REG,&DSI_REG->DSI_TXRX_CTRL, &tmp_reg);
#else
	OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&tmp_reg));
#endif
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PS_Control(unsigned int ps_type, unsigned int vact_line, unsigned int ps_wc)
{
    DSI_PSCTRL_REG tmp_reg;
	UINT32 tmp_hstx_cklp_wc;
    tmp_reg=DSI_REG->DSI_PSCTRL;

    ///TODO: parameter checking
    ASSERT(ps_type <= PACKED_PS_18BIT_RGB666);
	if(ps_type>LOOSELY_PS_18BIT_RGB666)
    	tmp_reg.DSI_PS_SEL=(5 - ps_type);
	else
		tmp_reg.DSI_PS_SEL=ps_type;
    tmp_reg.DSI_PS_WC=ps_wc;
    tmp_hstx_cklp_wc = ps_wc;

#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, AS_UINT32(&vact_line));
	DSI_OUTREG32_R(PDSI_PSCTRL_REG,&DSI_REG->DSI_PSCTRL, &tmp_reg);
#else
	OUTREG32(&DSI_REG->DSI_VACT_NL, AS_UINT32(&vact_line));
    OUTREG32(&DSI_REG->DSI_PSCTRL, AS_UINT32(&tmp_reg));
#endif
	OUTREG32(&DSI_REG->DSI_HSTX_CKL_WC, tmp_hstx_cklp_wc);
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_ConfigVDOTiming(unsigned int dsi_vsa_nl,
                                    unsigned int dsi_vbp_nl,
                                    unsigned int dsi_vfp_nl,
                                    unsigned int dsi_vact_nl,
                                    unsigned int dsi_line_nb,
                                    unsigned int dsi_hsa_nb,
                                    unsigned int dsi_hbp_nb,
                                    unsigned int dsi_hfp_nb,
                                    unsigned int dsi_rgb_nb,
                                    unsigned int dsi_hsa_wc,
                                    unsigned int dsi_hbp_wc,
                                    unsigned int dsi_hfp_wc)
{
    ///TODO: parameter checking
#ifdef FIX_BUILD_WARNING_0616
	DSI_OUTREG32_V(PDSI_VSA_NL_REG,&DSI_REG->DSI_VSA_NL, AS_UINT32(&dsi_vsa_nl));
	DSI_OUTREG32_V(PDSI_VBP_NL_REG,&DSI_REG->DSI_VBP_NL, AS_UINT32(&dsi_vbp_nl));
	DSI_OUTREG32_V(PDSI_VFP_NL_REG,&DSI_REG->DSI_VFP_NL, AS_UINT32(&dsi_vfp_nl));
	DSI_OUTREG32_V(PDSI_VACT_NL_REG,&DSI_REG->DSI_VACT_NL, AS_UINT32(&dsi_vact_nl));

	DSI_OUTREG32_V(PDSI_HSA_WC_REG,&DSI_REG->DSI_HSA_WC, AS_UINT32(&dsi_hsa_wc));
	DSI_OUTREG32_V(PDSI_HBP_WC_REG,&DSI_REG->DSI_HBP_WC, AS_UINT32(&dsi_hbp_wc));
	DSI_OUTREG32_V(PDSI_HFP_WC_REG,&DSI_REG->DSI_HFP_WC, AS_UINT32(&dsi_hfp_wc));
#else
	OUTREG32(&DSI_REG->DSI_VSA_NL, AS_UINT32(&dsi_vsa_nl));
	OUTREG32(&DSI_REG->DSI_VBP_NL, AS_UINT32(&dsi_vbp_nl));
	OUTREG32(&DSI_REG->DSI_VFP_NL, AS_UINT32(&dsi_vfp_nl));
	OUTREG32(&DSI_REG->DSI_VACT_NL, AS_UINT32(&dsi_vact_nl));
#if 0 //6583 not need these parameters
	OUTREG32(&DSI_REG->DSI_LINE_NB, AS_UINT32(&dsi_line_nb));
	OUTREG32(&DSI_REG->DSI_HSA_NB, AS_UINT32(&dsi_hsa_nb));
	OUTREG32(&DSI_REG->DSI_HBP_NB, AS_UINT32(&dsi_hbp_nb));
	OUTREG32(&DSI_REG->DSI_HFP_NB, AS_UINT32(&dsi_hfp_nb));
	OUTREG32(&DSI_REG->DSI_RGB_NB, AS_UINT32(&dsi_rgb_nb));
#endif
	OUTREG32(&DSI_REG->DSI_HSA_WC, AS_UINT32(&dsi_hsa_wc));
	OUTREG32(&DSI_REG->DSI_HBP_WC, AS_UINT32(&dsi_hbp_wc));
	OUTREG32(&DSI_REG->DSI_HFP_WC, AS_UINT32(&dsi_hfp_wc));
#endif
    return DSI_STATUS_OK;
}

void DSI_write_lcm_cmd(unsigned int cmd)
{
	DSI_T0_INS *t0_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=SHORT_PACKET_RW;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=LOW_POWER;
	CONFG_tmp.CL=CL_8BITS;
	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;

	t0_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t0_tmp->Data_ID= (cmd&0xFF);
	t0_tmp->Data0 = 0x0;
	t0_tmp->Data1 = 0x0;

	DSI_Write_T0_INS(t0_tmp);
}


void DSI_write_lcm_regs(unsigned int addr, unsigned int *para, unsigned int nums)
{
	DSI_T2_INS *t2_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=LONG_PACKET_W;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=LOW_POWER;
	CONFG_tmp.CL=CL_8BITS;
	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;

	t2_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t2_tmp->Data_ID = (addr&0xFF);
	t2_tmp->WC16 = nums;
	t2_tmp->pdata = para;

	DSI_Write_T2_INS(t2_tmp);

}

UINT32 DSI_dcs_read_lcm_reg(UINT8 cmd)
{
#ifdef FIX_BUILD_WARNING_0616
    UINT32 recv_data = 0;
#else
	UINT32 max_try_count = 5;
	UINT32 recv_data;
	UINT32 recv_data_cnt;
	unsigned int read_timeout_ms;
	unsigned char packet_type;
#endif
#if 0
    DSI_T0_INS t0;
#if ENABLE_DSI_INTERRUPT
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
    long ret;
#endif

	if (DSI_REG->DSI_MODE_CTRL.MODE)
		return 0;

    do
    {
       if(max_try_count == 0)
          return 0;

       max_try_count--;
       recv_data = 0;
       recv_data_cnt = 0;
       read_timeout_ms = 20;

       _WaitForEngineNotBusy();

       t0.CONFG = 0x04;        ///BTA
       t0.Data0 = cmd;
       t0.Data_ID = DSI_DCS_READ_PACKET_ID;
       t0.Data1 = 0;

       OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
       OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

       ///clear read ACK
       DSI_REG->DSI_RACK.DSI_RACK = 1;
       DSI_REG->DSI_INTSTA.RD_RDY = 1;
       DSI_REG->DSI_INTSTA.CMD_DONE = 1;
       DSI_REG->DSI_INTEN.RD_RDY =  1;
       DSI_REG->DSI_INTEN.CMD_DONE=  1;

       OUTREG32(&DSI_REG->DSI_START, 0);
       OUTREG32(&DSI_REG->DSI_START, 1);

       /// the following code is to
       /// 1: wait read ready
       /// 2: ack read ready
       /// 3: wait for CMDQ_DONE
       /// 3: read data
#if ENABLE_DSI_INTERRUPT
        ret = wait_event_interruptible_timeout(_dsi_dcs_read_wait_queue,
                                                       !_IsEngineBusy(),
                                                       WAIT_TIMEOUT);
        if (0 == ret) {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");

				DSI_DumpRegisters();

				///do necessary reset here
				DSI_REG->DSI_RACK.DSI_RACK = 1;
				DSI_Reset();

                return 0;
            }
#else
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Start polling DSI read ready!!!\n");
    #endif
        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
        {
            ///keep polling
            msleep(1);
            read_timeout_ms --;

            if(read_timeout_ms == 0)
            {
                DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Polling DSI read ready timeout!!!\n");
                DSI_DumpRegisters();

                ///do necessary reset here
                DSI_REG->DSI_RACK.DSI_RACK = 1;
                DSI_Reset();
                return 0;
            }
        }
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " End polling DSI read ready!!!\n");
    #endif

        DSI_REG->DSI_RACK.DSI_RACK = 1;

       while(DSI_REG->DSI_STA.BUF_UNDERRUN || DSI_REG->DSI_STA.ESC_ENTRY_ERR || DSI_REG->DSI_STA.LPDT_SYNC_ERR || DSI_REG->DSI_STA.CTRL_ERR || DSI_REG->DSI_STA.CONTENT_ERR)
       {
           ///DSI READ ACK HW bug workaround
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
           DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI is busy: 0x%x !!!\n", DSI_REG->DSI_STA.BUSY);
    #endif
           DSI_REG->DSI_RACK.DSI_RACK = 1;
       }


       ///clear interrupt status
       DSI_REG->DSI_INTSTA.RD_RDY = 1;
       ///STOP DSI
       OUTREG32(&DSI_REG->DSI_START, 0);

#endif

       DSI_REG->DSI_INTEN.RD_RDY =  0;

    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_STA : 0x%x \n", DSI_REG->DSI_RX_STA);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_SIZE : 0x%x \n", DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA0 : 0x%x \n", DSI_CMDQ_REG->data[0].byte0);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA1 : 0x%x \n", DSI_CMDQ_REG->data[0].byte1);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA2 : 0x%x \n", DSI_CMDQ_REG->data[0].byte2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA3 : 0x%x \n", DSI_CMDQ_REG->data[0].byte3);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE0 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE0);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE1 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE1);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE2 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE3 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE3);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE4 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE4);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE5 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE5);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE6 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE6);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA.BYTE7 : 0x%x \n", DSI_REG->DSI_RX_DATA.BYTE7);
    #endif
       packet_type = DSI_REG->DSI_RX_DATA.BYTE0;

    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI read packet_type is 0x%x \n",packet_type);
    #endif
       if(DSI_REG->DSI_RX_STA.LONG == 1)
       {
           recv_data_cnt = DSI_REG->DSI_RX_DATA.BYTE1 + DSI_REG->DSI_RX_DATA.BYTE2 * 16;
           if(recv_data_cnt > 4)
           {
    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
              DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet data  exceeds 4 bytes \n");
    #endif
              recv_data_cnt = 4;
           }
           memcpy((void*)&recv_data, (void*)&DSI_REG->DSI_RX_DATA.BYTE4, recv_data_cnt);
       }
       else
       {
           memcpy((void*)&recv_data, (void*)&DSI_REG->DSI_RX_DATA.BYTE1, 2);
       }

    #ifdef DSI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI read 0x%x data is 0x%x \n",cmd,  recv_data);
    #endif
   }while(packet_type != 0x1C && packet_type != 0x21 && packet_type != 0x22);
   /// here: we may receive a ACK packet which packet type is 0x02 (incdicates some error happened)
   /// therefore we try re-read again until no ACK packet
   /// But: if it is a good way to keep re-trying ???
#endif
   return recv_data;
}

/// return value: the data length we got
UINT32 DSI_dcs_read_lcm_reg_v2(UINT8 cmd, UINT8 *buffer, UINT8 buffer_size)
{
    UINT32 max_try_count = 5;
    UINT32 recv_data_cnt;
    unsigned int read_timeout_ms;
    unsigned char packet_type;
	DSI_RX_DATA_REG read_data0;
	DSI_RX_DATA_REG read_data1;
	DSI_RX_DATA_REG read_data2;
	DSI_RX_DATA_REG read_data3;
#if 1
    DSI_T0_INS t0;

#if ENABLE_DSI_INTERRUPT
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec
    long ret;
#endif
	if (DSI_REG->DSI_MODE_CTRL.MODE)
		return 0;

    if (buffer == NULL || buffer_size == 0)
        return 0;

do
    {
       if(max_try_count == 0)
	  return 0;
       max_try_count--;
       recv_data_cnt = 0;
       read_timeout_ms = 20;

       _WaitForEngineNotBusy();

       t0.CONFG = 0x04;        ///BTA
       t0.Data0 = cmd;
       if (buffer_size < 0x3)
           t0.Data_ID = DSI_DCS_READ_PACKET_ID;
       else
           t0.Data_ID = DSI_GERNERIC_READ_LONG_PACKET_ID;
       t0.Data1 = 0;
#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREG32_R(PDSI_CMDQ,&DSI_CMDQ_REG->data[0], &t0);
		DSI_OUTREG32_V(PDSI_CMDQ_CTRL_REG,&DSI_REG->DSI_CMDQ_SIZE, 1);

		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
		DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
		DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);

		DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
		DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 1);
#else
       OUTREG32(&DSI_CMDQ_REG->data[0], AS_UINT32(&t0));
       OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);

       ///clear read ACK
       //DSI_REG->DSI_RACK.DSI_RACK = 1;
       //DSI_REG->DSI_INTSTA.RD_RDY = 1;
       //DSI_REG->DSI_INTSTA.CMD_DONE = 1;
       //DSI_REG->DSI_INTEN.RD_RDY =  1;
       //DSI_REG->DSI_INTEN.CMD_DONE=  1;
       OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
       OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
	   OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,CMD_DONE,1);
	   OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
	   OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,CMD_DONE,1);



       OUTREG32(&DSI_REG->DSI_START, 0);
       OUTREG32(&DSI_REG->DSI_START, 1);
#endif
       /// the following code is to
       /// 1: wait read ready
       /// 2: ack read ready
       /// 3: wait for CMDQ_DONE
       /// 3: read data
#if ENABLE_DSI_INTERRUPT
       ret = wait_event_interruptible_timeout(_dsi_dcs_read_wait_queue,
                                                       !_IsEngineBusy(),
                                                       WAIT_TIMEOUT);
        if (0 == ret) {
            xlog_printk(ANDROID_LOG_WARN, "DSI", " Wait for DSI engine read ready timeout!!!\n");

				DSI_DumpRegisters();

				///do necessary reset here
				//DSI_REG->DSI_RACK.DSI_RACK = 1;
				OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
				DSI_Reset();

                return 0;
            }
#else
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Start polling DSI read ready!!!\n");
    #endif
        while(DSI_REG->DSI_INTSTA.RD_RDY == 0)  ///read clear
        {
            ///keep polling
            msleep(1);
            read_timeout_ms --;

            if(read_timeout_ms == 0)
            {
                DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " Polling DSI read ready timeout!!!\n");
                DSI_DumpRegisters();

                ///do necessary reset here
#ifdef FIX_BUILD_WARNING_0616
				DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
#else
				//DSI_REG->DSI_RACK.DSI_RACK = 1;
				OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
#endif
                DSI_Reset();
                return 0;
            }
        }
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " End polling DSI read ready!!!\n");
    #endif

#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);
		DSI_OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
		DSI_OUTREG32_V(PDSI_START_REG,&DSI_REG->DSI_START, 0);
#else
		//DSI_REG->DSI_RACK.DSI_RACK = 1;
        OUTREGBIT(DSI_RACK_REG,DSI_REG->DSI_RACK,DSI_RACK,1);

       ///clear interrupt status
       //DSI_REG->DSI_INTSTA.RD_RDY = 1;
       OUTREGBIT(DSI_INT_STATUS_REG,DSI_REG->DSI_INTSTA,RD_RDY,1);
       ///STOP DSI
       OUTREG32(&DSI_REG->DSI_START, 0);
#endif

#endif

#ifdef FIX_BUILD_WARNING_0616
		DSI_OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
#else
       //DSI_REG->DSI_INTEN.RD_RDY =  0;
       OUTREGBIT(DSI_INT_ENABLE_REG,DSI_REG->DSI_INTEN,RD_RDY,1);
#endif
	   OUTREG32(&read_data0, AS_UINT32(&DSI_REG->DSI_RX_DATA0));
	   OUTREG32(&read_data1, AS_UINT32(&DSI_REG->DSI_RX_DATA1));
	   OUTREG32(&read_data2, AS_UINT32(&DSI_REG->DSI_RX_DATA2));
	   OUTREG32(&read_data3, AS_UINT32(&DSI_REG->DSI_RX_DATA3));
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
    {
       unsigned int i;
//       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_STA : 0x%x \n", DSI_REG->DSI_RX_STA);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_SIZE : 0x%x \n", DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA0 : 0x%x \n", DSI_CMDQ_REG->data[0].byte0);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA1 : 0x%x \n", DSI_CMDQ_REG->data[0].byte1);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA2 : 0x%x \n", DSI_CMDQ_REG->data[0].byte2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_CMDQ_DATA3 : 0x%x \n", DSI_CMDQ_REG->data[0].byte3);
#if 1
	   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA0 : 0x%x \n", DSI_REG->DSI_RX_DATA0);
	   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA1 : 0x%x \n", DSI_REG->DSI_RX_DATA1);
	   DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA2 : 0x%x \n", DSI_REG->DSI_RX_DATA2);
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI_RX_DATA3 : 0x%x \n", DSI_REG->DSI_RX_DATA3);

       printf("read_data0, %x,%x,%x,%x\n", read_data0.byte0, read_data0.byte1, read_data0.byte2, read_data0.byte3);
	   printf("read_data1, %x,%x,%x,%x\n", read_data1.byte0, read_data1.byte1, read_data1.byte2, read_data1.byte3);
	   printf("read_data2, %x,%x,%x,%x\n", read_data2.byte0, read_data2.byte1, read_data2.byte2, read_data2.byte3);
	   printf("read_data3, %x,%x,%x,%x\n", read_data3.byte0, read_data3.byte1, read_data3.byte2, read_data3.byte3);
#endif
    }
    #endif

#if 1
	packet_type = read_data0.byte0;

    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
       DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", " DSI read packet_type is 0x%x \n",packet_type);
    #endif



	if(packet_type == 0x1A || packet_type == 0x1C)
    {
    	recv_data_cnt = read_data0.byte1 + read_data0.byte2 * 16;
		if(recv_data_cnt > 10)
        {
    #ifdef DDI_DRV_DEBUG_LOG_ENABLE
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet data  exceeds 4 bytes \n");
    #endif
            recv_data_cnt = 10;
         }

          if(recv_data_cnt > buffer_size)
          {
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
              DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet data  exceeds buffer size: %d\n", buffer_size);
#endif
              recv_data_cnt = buffer_size;
           }
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
          DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read long packet size: %d\n", recv_data_cnt);
#endif
           memcpy((void*)buffer, (void*)&read_data1, recv_data_cnt);
       }
       else
       {
             if(recv_data_cnt > buffer_size)
             {
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
                 DISP_LOG_PRINT(ANDROID_LOG_WARN, "DSI", " DSI read short packet data  exceeds buffer size: %d\n", buffer_size);
#endif
                 recv_data_cnt = buffer_size;
             }
           memcpy((void*)buffer,(void*)&read_data0.byte1, 2);
       }
#endif
   }while(packet_type != 0x1C && packet_type != 0x21 && packet_type != 0x22 && packet_type != 0x1A);
   /// here: we may receive a ACK packet which packet type is 0x02 (incdicates some error happened)
   /// therefore we try re-read again until no ACK packet
   /// But: if it is a good way to keep re-trying ???
#endif
   return recv_data_cnt;
}

UINT32 DSI_read_lcm_reg()
{
    return 0;
}


DSI_STATUS DSI_write_lcm_fb(unsigned int addr, BOOL long_length)
{
	DSI_T1_INS *t1_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=FB_WRITE;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=HIGH_SPEED;

	if(long_length)
		CONFG_tmp.CL=CL_16BITS;
	else
		CONFG_tmp.CL=CL_8BITS;

	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;


	t1_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t1_tmp->Data_ID= 0x39;
	t1_tmp->mem_start0 = (addr&0xFF);

	if(long_length)
		t1_tmp->mem_start1 = ((addr>>8)&0xFF);

	return DSI_Write_T1_INS(t1_tmp);


}


DSI_STATUS DSI_read_lcm_fb(void)
{
	// TBD
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_enable_MIPI_txio(BOOL en)
{
#if 0
    if(en)
    {
        *(volatile unsigned int *) (INFRACFG_BASE+0x890) |= 0x00000100;    // enable MIPI TX IO
    }
    else
    {
        *(volatile unsigned int *) (INFRACFG_BASE+0x890) &= ~0x00000100;   // disable MIPI TX IO
    }
#endif
	return DSI_STATUS_OK;
}


BOOL Need_Wait_ULPS(void)
{
#ifndef MT65XX_NEW_DISP
	if(((INREG32(DSI_BASE + 0x14C)>> 24) & 0xFF) != 0x04) {

#ifdef DDI_DRV_DEBUG_LOG_ENABLE
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[%s]:true \n", __func__);
#endif
		return TRUE;

	} else {

#ifdef DDI_DRV_DEBUG_LOG_ENABLE
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[%s]:false \n", __func__);
#endif
		return FALSE;

	}
#endif
#ifdef FIX_BUILD_WARNING_0616
	return FALSE;
#endif
}


DSI_STATUS Wait_ULPS_Mode(void)
{
#ifndef MT65XX_NEW_DISP
	DSI_PHY_LCCON_REG lccon_reg=DSI_REG->DSI_PHY_LCCON;
	DSI_PHY_LD0CON_REG ld0con=DSI_REG->DSI_PHY_LD0CON;

	lccon_reg.LC_ULPM_EN =1;
	ld0con.L0_ULPM_EN=1;
	OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
	OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));

#ifdef DDI_DRV_DEBUG_LOG_ENABLE
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[%s]:enter \n", __func__);
#endif

	while(((INREG32(DSI_BASE + 0x14C)>> 24) & 0xFF) != 0x04)
	{
		lcm_mdelay(5);
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI+%04x : 0x%08x \n", DSI_BASE, INREG32(DSI_BASE + 0x14C));
#endif
	}

#ifdef DDI_DRV_DEBUG_LOG_ENABLE
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[%s]:exit \n", __func__);
#endif
#endif
	return DSI_STATUS_OK;

}


DSI_STATUS Wait_WakeUp(void)
{
#ifndef MT65XX_NEW_DISP
	DSI_PHY_LCCON_REG lccon_reg=DSI_REG->DSI_PHY_LCCON;
	DSI_PHY_LD0CON_REG ld0con=DSI_REG->DSI_PHY_LD0CON;

#ifdef DDI_DRV_DEBUG_LOG_ENABLE
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[%s]:enter \n", __func__);
#endif

	lccon_reg.LC_ULPM_EN =0;
	ld0con.L0_ULPM_EN=0;
	OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
	OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));

	lcm_mdelay(1);//Wait 1ms for LCM Spec

	lccon_reg.LC_WAKEUP_EN =1;
	ld0con.L0_WAKEUP_EN=1;
	OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
	OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));

	while(((INREG32(DSI_BASE + 0x148)>> 8) & 0xFF) != 0x01)
	{
		lcm_mdelay(5);
#ifdef DDI_DRV_DEBUG_LOG_ENABLE
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[soso]DSI+%04x : 0x%08x \n", DSI_BASE, INREG32(DSI_BASE + 0x148));
#endif
	}

#ifdef DDI_DRV_DEBUG_LOG_ENABLE
	DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[%s]:exit \n", __func__);
#endif

	lccon_reg.LC_WAKEUP_EN =0;
	ld0con.L0_WAKEUP_EN=0;
	OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&lccon_reg));
	OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&ld0con));
#endif
	return DSI_STATUS_OK;

}

// -------------------- Retrieve Information --------------------

DSI_STATUS DSI_DumpRegisters(void)
{
    UINT32 i;
/*
description of dsi status
Bit	Value	Description
[0]	0x0001	Idle (wait for command)
[1]	0x0002	Reading command queue for header
[2]	0x0004	Sending type-0 command
[3]	0x0008	Waiting frame data from RDMA for type-1 command
[4]	0x0010	Sending type-1 command
[5]	0x0020	Sending type-2 command
[6]	0x0040	Reading command queue for data
[7]	0x0080	Sending type-3 command
[8]	0x0100	Sending BTA
[9]	0x0200	Waiting RX-read data
[10]	0x0400	Waiting SW RACK for RX-read data
[11]	0x0800	Waiting TE
[12]	0x1000	Get TE
[13]	0x2000	Waiting external TE
[14]	0x4000	Waiting SW RACK for TE

*/
static const char* DSI_DBG_STATUS_DESCRIPTION[] =
{
	"null",
	"Idle (wait for command)",
	"Reading command queue for header",
	"Sending type-0 command",
	"Waiting frame data from RDMA for type-1 command",
	"Sending type-1 command",
	"Sending type-2 command",
	"Reading command queue for data",
	"Sending type-3 command",
	"Sending BTA",
	"Waiting RX-read data ",
	"Waiting SW RACK for RX-read data",
	"Waiting TE",
	"Get TE ",
	"Waiting external TE",
	"Waiting SW RACK for TE",
};
	unsigned int DSI_DBG6_Status = (INREG32(DSI_BASE+0x160))&0xffff;
#ifndef FIX_BUILD_WARNING_0616
	unsigned int DSI_DBG6_Status_bak = DSI_DBG6_Status;
#endif
	int count=0;
	while(DSI_DBG6_Status){DSI_DBG6_Status>>=1; count++;}
	//while((1<<count) != DSI_DBG6_Status) count++;
	//count++;
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "---------- Start dump DSI registers ----------\n");
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_STATE_DBG6=0x%08x, count=%d, means: [%s]\n", DSI_DBG6_Status, count, DSI_DBG_STATUS_DESCRIPTION[count]);
    for (i = 0; i < sizeof(DSI_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI+%04x : 0x%08x\n", i, INREG32(DSI_BASE + i));
    }

    for (i = 0; i < sizeof(DSI_CMDQ_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_CMD+%04x(%p) : 0x%08x\n", i, (UINT32*)(DSI_BASE+0x180+i), INREG32((DSI_BASE+0x180+i)));
    }

    for (i = 0; i < sizeof(DSI_PHY_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "DSI_PHY+%04x(%p) : 0x%08x\n", i, (UINT32*)(MIPI_CONFIG_BASE+i), INREG32((MIPI_CONFIG_BASE+i)));
    }

    return DSI_STATUS_OK;
}


static LCM_PARAMS lcm_params_for_clk_setting;


DSI_STATUS DSI_FMDesense_Query(void)
{
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_FM_Desense(unsigned long freq)
{
    ///need check
    DSI_Change_CLK(freq);
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Reset_CLK(void)
{
    extern LCM_PARAMS *lcm_params;

    _WaitForEngineNotBusy();
	DSI_PHY_TIMCONFIG(lcm_params);
	DSI_PHY_clk_setting(lcm_params);
	return DSI_STATUS_OK;
}

DSI_STATUS DSI_Get_Default_CLK(unsigned int *clk)
{
    extern LCM_PARAMS *lcm_params;
	unsigned int div2_real = lcm_params->dsi.pll_div2 ? lcm_params->dsi.pll_div2 : 0x1;

    *clk = 13 * (lcm_params->dsi.pll_div1 + 1) / div2_real;
    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Get_Current_CLK(unsigned int *clk)
{

    return DSI_STATUS_OK;
}

DSI_STATUS DSI_Change_CLK(unsigned int clk)
{
    extern LCM_PARAMS *lcm_params;

    if(clk > 1000)
        return DSI_STATUS_ERROR;
    memcpy((void *)&lcm_params_for_clk_setting, (void *)lcm_params, sizeof(LCM_PARAMS));

    for(lcm_params_for_clk_setting.dsi.pll_div2 = 15; lcm_params_for_clk_setting.dsi.pll_div2 > 0; lcm_params_for_clk_setting.dsi.pll_div2--)
    {
        for(lcm_params_for_clk_setting.dsi.pll_div1 = 0; lcm_params_for_clk_setting.dsi.pll_div1 < 39; lcm_params_for_clk_setting.dsi.pll_div1++)
        {
            if((13 * (lcm_params_for_clk_setting.dsi.pll_div1 + 1) / lcm_params_for_clk_setting.dsi.pll_div2) >= clk)
                goto end;
        }
    }

    if(lcm_params_for_clk_setting.dsi.pll_div2 == 0)
    {
        for(lcm_params_for_clk_setting.dsi.pll_div1 = 0; lcm_params_for_clk_setting.dsi.pll_div1 < 39; lcm_params_for_clk_setting.dsi.pll_div1++)
        {
            if((26 * (lcm_params_for_clk_setting.dsi.pll_div1 + 1)) >= clk)
                goto end;
        }
    }

end:
    _WaitForEngineNotBusy();
	DSI_PHY_TIMCONFIG(&lcm_params_for_clk_setting);
	DSI_PHY_clk_setting(&lcm_params_for_clk_setting);

    return DSI_STATUS_OK;
}


