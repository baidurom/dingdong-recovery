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

#ifdef BUILD_UBOOT
#define ENABLE_DPI_INTERRUPT        0
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#include <asm/arch/disp_drv_platform.h>
#else

#define ENABLE_DPI_INTERRUPT        0
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#if ENABLE_DPI_REFRESH_RATE_LOG && !ENABLE_DPI_INTERRUPT
#error "ENABLE_DPI_REFRESH_RATE_LOG should be also ENABLE_DPI_INTERRUPT"
#endif

#if defined(MTK_HDMI_SUPPORT) && !ENABLE_DPI_INTERRUPT
#error "enable MTK_HDMI_SUPPORT should be also ENABLE_DPI_INTERRUPT"
#endif

#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/io.h>
#include <disp_drv_log.h>

#include "disp_drv_platform.h"

#include "dpi_reg.h"
#include "dsi_reg.h"
#include "dpi_drv.h"
#include "lcd_drv.h"

#if ENABLE_DPI_INTERRUPT
#include <linux/interrupt.h>
#include <linux/wait.h>

#include <mach/irqs.h>
#include "mtkfb.h"
#endif

#include <linux/module.h>
#endif

static PDPI_REGS const DPI_REG = (PDPI_REGS)(DPI_BASE);
static PDSI_PHY_REGS const DSI_PHY_REG_DPI = (PDSI_PHY_REGS)(MIPI_CONFIG_BASE + 0x800);
static UINT32 const PLL_SOURCE = APMIXEDSYS_BASE + 0x44;
static BOOL s_isDpiPowerOn = FALSE;
static DPI_REGS regBackup;
static void (*dpiIntCallback)(DISP_INTERRUPT_EVENTS);

#define DPI_REG_OFFSET(r)       offsetof(DPI_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))

#if !(defined(CONFIG_MT6589_FPGA) || defined(BUILD_UBOOT))
#define DPI_MIPI_API
#endif

const UINT32 BACKUP_DPI_REG_OFFSETS[] =
{
    DPI_REG_OFFSET(INT_ENABLE),
    DPI_REG_OFFSET(SIZE),
    DPI_REG_OFFSET(CLK_CNTL),

    DPI_REG_OFFSET(TGEN_HWIDTH),
    DPI_REG_OFFSET(TGEN_HPORCH),

	DPI_REG_OFFSET(TGEN_VWIDTH_LODD),
    DPI_REG_OFFSET(TGEN_VPORCH_LODD),

    DPI_REG_OFFSET(CNTL),
};

static void _BackupDPIRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;
    
    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i])));
    }
}

static void _RestoreDPIRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i])));
    }
}

static void _ResetBackupedDPIRegisterValues(void)
{
    DPI_REGS *regs = &regBackup;
    memset((void*)regs, 0, sizeof(DPI_REGS));

    OUTREG32(&regs->CLK_CNTL, 0x00000101);
}


#if ENABLE_DPI_REFRESH_RATE_LOG
static void _DPI_LogRefreshRate(DPI_REG_INTERRUPT status)
{
    static unsigned long prevUs = 0xFFFFFFFF;

    if (status.VSYNC)
    {
        struct timeval curr;
        do_gettimeofday(&curr);

        if (prevUs < curr.tv_usec)
        {
            DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "Receive 1 vsync in %lu us\n", 
                   curr.tv_usec - prevUs);
        }
        prevUs = curr.tv_usec;
    }
}
#else
#define _DPI_LogRefreshRate(x)  do {} while(0)
#endif


#if ENABLE_DPI_INTERRUPT
static irqreturn_t _DPI_InterruptHandler(int irq, void *dev_id)
{   
    static int counter = 0;
    DPI_REG_INTERRUPT status = DPI_REG->INT_STATUS;
//    if (status.FIFO_EMPTY) ++ counter;

    if(status.VSYNC)
    {
        if(dpiIntCallback)
           dpiIntCallback(DISP_DPI_VSYNC_INT);
    }

    if (status.VSYNC && counter) {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "[Error] DPI FIFO is empty, "
               "received %d times interrupt !!!\n", counter);
        counter = 0;
    }

    _DPI_LogRefreshRate(status);
	OUTREG32(&DPI_REG->INT_STATUS, 0);
    return IRQ_HANDLED;
}
#endif


DPI_STATUS DPI_Init(BOOL isDpiPoweredOn)
{
    DPI_REG_CNTL cntl;

    if (isDpiPoweredOn) {
        _BackupDPIRegisters();
    } else {
        _ResetBackupedDPIRegisterValues();
    }

    DPI_PowerOn();
	
	OUTREG32(DPI_BASE+ 0x64, 0x400);//
	OUTREG32(DPI_BASE+ 0x6C, 0x400);//
	OUTREG32(DPI_BASE+ 0x74, 0x400);//
	OUTREG32(DPI_BASE+ 0x8C, 0x0FFF0000);//
	OUTREG32(DPI_BASE+ 0x90, 0x0FFF0000);//
#if ENABLE_DPI_INTERRUPT
    if (request_irq(MT6589_DPI_IRQ_ID,
        _DPI_InterruptHandler, IRQF_TRIGGER_LOW, "mtkdpi", NULL) < 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "[ERROR] fail to request DPI irq\n"); 
        return DPI_STATUS_ERROR;
    }

    {
        DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
        enInt.VSYNC = 1;
        OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
    }
#endif
	LCD_W2M_NeedLimiteSpeed(TRUE);
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_Init);

DPI_STATUS DPI_Deinit(void)
{
    DPI_DisableClk();
    DPI_PowerOff();

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_Deinit);

void DPI_mipi_switch(bool on)
{
	if(on)
	{
	// may call enable_mipi(), but do this in DPI_Init_PLL
	}
	else
	{
#ifdef DPI_MIPI_API 
		disable_mipi(MT65XX_MIPI_TX, "DPI");
#endif
	}	
}

#ifndef BULID_UBOOT
extern UINT32 FB_Addr;
#endif
DPI_STATUS DPI_Init_PLL(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2)
{
    unsigned int reg_value = 0;
#if 0
    MIPITX_CFG0_REG con0 = DSI_PHY_REG_DPI->MIPITX_CON0;
    MIPITX_CFG1_REG con1 = DSI_PHY_REG_DPI->MIPITX_CON1;
#ifdef DPI_MIPI_API 
	enable_mipi(MT65XX_MIPI_TX, "DPI");
#endif
    #ifdef BUILD_UBOOT
	OUTREG16(0xc2080858, 0x8000);
	OUTREG16(0xc20a3824, 0x4008);
	MASKREG16(0xc20a380c, 0x000c, 0x0000); //default value is 0x7008, but should be 0x7000
	#else
	OUTREG16(0xf2080858, 0x8000); //??
	OUTREG16(0xf20a3824, 0x4008);
	MASKREG16(0xf20a380c, 0x000c, 0x0000); //default value is 0x7008, but should be 0x7000
	MASKREG16(PLL_SOURCE, 0x0010, 0x0010); //
	#endif
    con1.RG_PLL_DIV1 = mipi_pll_clk_div1;
    con1.RG_PLL_DIV2 = mipi_pll_clk_div2;

	con0.PLL_CLKR_EN = 1;
	con0.PLL_EN = 1;
	con0.RG_DPI_EN = 1;

    // Set to DSI_PHY_REG
    
    OUTREG32(&DSI_PHY_REG_DPI->MIPITX_CON0, AS_UINT32(&con0));
    OUTREG32(&DSI_PHY_REG_DPI->MIPITX_CON1, AS_UINT32(&con1));
#else
#ifndef BUILD_UBOOT
	OUTREG32(DISPSYS_BASE + 0x34, 0x02);//set RDMA0 to DPI
#if 0
	//CG
	OUTREG32(DISPSYS_BASE + 0x104, 0xffffffff);//set CG
	OUTREG32(DISPSYS_BASE + 0x108, 0xffffffff);
	OUTREG32(DISPSYS_BASE + 0x114, 0xffffffff);
	OUTREG32(DISPSYS_BASE + 0x118, 0xffffffff);
#endif

	OUTREG32(DISP_MUTEX_BASE + 0x28, 0x01);//reset mutex
	OUTREG32(DISP_MUTEX_BASE + 0x28, 0);
	
	OUTREG32(DISP_MUTEX_BASE + 0x2c, 0x80); //rdma0 is in the mutex
	OUTREG32(DISP_MUTEX_BASE + 0x30, 0x2);  //dpi0 is the dst

	OUTREG32(DISP_MUTEX_BASE + 0x24, 0x1);  //lock mutex0
	
	OUTREG32(DISP_MUTEX_BASE + 0, 0x1);  //lock mutex0
	// OUTREG32(MUTEX_BASE + 0x4, 0x1);  //lock mutex0

	while((INREG32(DISP_MUTEX_BASE + 0x24)&0x02)!=0x02){} // polling until mutex lock complete
	
	OUTREG32(DISPSYS_BASE + 0xC08, 0x01);//select DPI pin
	OUTREG32(DISPSYS_BASE + 0x60, 0x8);// DPI src clock
	
	//RDMA0 setting
	OUTREG32(RDMA0_BASE + 0x10, 0); //stop rdma0
	
	// width, height and format
	OUTREG32(RDMA0_BASE + 0x14, DISP_GetScreenWidth());
	OUTREG32(RDMA0_BASE + 0x18, DISP_GetScreenHeight());
	
	OUTREG32(RDMA0_BASE + 0x24, 0x40);//input format ARGB888
#ifndef BUILD_UBOOT
	OUTREG32(RDMA0_BASE + 0x28, FB_Addr);//input addr
#else
	OUTREG32(RDMA0_BASE + 0x28, mt65xx_get_fb_addr());//input addr
#endif
	OUTREG32(RDMA0_BASE + 0x2C, DISP_GetScreenWidth()*2);//input pitch
	
	OUTREG32(RDMA0_BASE + 0x30, 0x10101010);
	OUTREG32(RDMA0_BASE + 0x40, 0x80f00008);
	
	OUTREG32(RDMA0_BASE + 0x50, 0);
	OUTREG32(RDMA0_BASE + 0x54, 0);
	OUTREG32(RDMA0_BASE + 0x58, 0);
	OUTREG32(RDMA0_BASE + 0x5C, 0);
	OUTREG32(RDMA0_BASE + 0x60, 0);
	OUTREG32(RDMA0_BASE + 0x64, 0);
	OUTREG32(RDMA0_BASE + 0x68, 0);
	OUTREG32(RDMA0_BASE + 0x6C, 0);
	OUTREG32(RDMA0_BASE + 0x70, 0);
	OUTREG32(RDMA0_BASE + 0x74, 0);
	OUTREG32(RDMA0_BASE + 0x78, 0);
	OUTREG32(RDMA0_BASE + 0x7C, 0);
	OUTREG32(RDMA0_BASE + 0x80, 0);
	OUTREG32(RDMA0_BASE + 0x84, 0);
	OUTREG32(RDMA0_BASE + 0x88, 0);
	OUTREG32(RDMA0_BASE + 0x8C, 0);
	
	//start rdma
	OUTREG32(RDMA0_BASE + 0x10, 0x03); //start + memory mode
    
    // dump register
    
    //release mutex0
    // OUTREG32(MUTEX_BASE + 0x24, 0);
    // while((INREG32(MUTEX_BASE + 0x24)&&0x02)!=0){} // polling until mutex lock complete
#endif
#endif
	return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_Init_PLL);

DPI_STATUS DPI_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
	DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "DPI_Set_DrivingCurrent not implement for 6575");
	return DPI_STATUS_OK;
}

#ifdef BUILD_UBOOT
DPI_STATUS DPI_PowerOn()
{
    if (!s_isDpiPowerOn)
    {
#if 1   // FIXME
        MASKREG32(0xC2080028, 0x10, 0x10);
#endif        
        _RestoreDPIRegisters();
        s_isDpiPowerOn = TRUE;
    }

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_PowerOff()
{
    if (s_isDpiPowerOn)
    {
        BOOL ret = TRUE;
        _BackupDPIRegisters();
#if 1   // FIXME
        MASKREG32(0xC2080018, 0x10, 0x10);
#endif        
        ASSERT(ret);
        s_isDpiPowerOn = FALSE;
    }

    return DPI_STATUS_OK;
}

#else

DPI_STATUS DPI_PowerOn()
{
#ifndef CONFIG_MT6589_FPGA
    if (!s_isDpiPowerOn)
    {
#if 1   // FIXME
        int ret = enable_clock(MT65XX_PDN_MM_DPI, "DPI");
		if(1 == ret)
		{
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
		}
#endif        
        _RestoreDPIRegisters();
        s_isDpiPowerOn = TRUE;
    }
#endif
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_PowerOff()
{
#ifndef CONFIG_MT6589_FPGA
    if (s_isDpiPowerOn)
    {
        int ret = TRUE;
        _BackupDPIRegisters();
#if 1   // FIXME
        ret = disable_clock(MT65XX_PDN_MM_DPI, "DPI");
		if(1 == ret)
		{
			DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "power manager API return FALSE\n");
		}
#endif        
        s_isDpiPowerOn = FALSE;
    }
#endif
    return DPI_STATUS_OK;
}
#endif
EXPORT_SYMBOL(DPI_PowerOn);

EXPORT_SYMBOL(DPI_PowerOff);

DPI_STATUS DPI_EnableClk()
{
	DPI_REG_EN en = DPI_REG->DPI_EN;
    en.EN = 1;
    OUTREG32(&DPI_REG->DPI_EN, AS_UINT32(&en));
   //release mutex0
#ifndef BUILD_UBOOT
    OUTREG32(DISP_MUTEX_BASE + 0x24, 0);
    while((INREG32(DISP_MUTEX_BASE + 0x24)&0x02)!=0){} // polling until mutex lock complete
#endif
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_EnableClk);

DPI_STATUS DPI_DisableClk()
{
    DPI_REG_EN en = DPI_REG->DPI_EN;
    en.EN = 0;
    OUTREG32(&DPI_REG->DPI_EN, AS_UINT32(&en));

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_DisableClk);

DPI_STATUS DPI_EnableSeqOutput(BOOL enable)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_EnableSeqOutput);

DPI_STATUS DPI_SetRGBOrder(DPI_RGB_ORDER input, DPI_RGB_ORDER output)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_SetRGBOrder);

DPI_STATUS DPI_ConfigPixelClk(DPI_POLARITY polarity, UINT32 divisor, UINT32 duty)
{
    DPI_REG_CLKCNTL ctrl;

    ASSERT(divisor >= 2);
    ASSERT(duty > 0 && duty < divisor);
    
    ctrl.POLARITY = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    ctrl.DIVISOR = divisor - 1;
    ctrl.DUTY = duty;

    OUTREG32(&DPI_REG->CLK_CNTL, AS_UINT32(&ctrl));

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigPixelClk);


DPI_STATUS DPI_ConfigDataEnable(DPI_POLARITY polarity)
{
/*
    DPI_REG_TGEN_HCNTL hctrl = DPI_REG->TGEN_HCNTL;
    hctrl.DE_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    OUTREG32(&DPI_REG->TGEN_HCNTL, AS_UINT32(&hctrl));
 */  
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigDataEnable);

DPI_STATUS DPI_ConfigVsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
    DPI_REG_TGEN_VWIDTH_LODD vwidth_lodd  = DPI_REG->TGEN_VWIDTH_LODD;
	DPI_REG_TGEN_VPORCH_LODD vporch_lodd  = DPI_REG->TGEN_VPORCH_LODD;

    vwidth_lodd.VPW_LODD = pulseWidth;
    vporch_lodd.VBP_LODD= backPorch;
    vporch_lodd.VFP_LODD= frontPorch;

    OUTREG32(&DPI_REG->TGEN_VWIDTH_LODD, AS_UINT32(&vwidth_lodd));
	OUTREG32(&DPI_REG->TGEN_VPORCH_LODD, AS_UINT32(&vporch_lodd));
    
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigVsync);


DPI_STATUS DPI_ConfigHsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
    DPI_REG_TGEN_HPORCH hporch = DPI_REG->TGEN_HPORCH;

    DPI_REG->TGEN_HWIDTH = pulseWidth - 1;
    hporch.HBP = backPorch - 1;
    hporch.HFP = frontPorch - 1;

    OUTREG32(&DPI_REG->TGEN_HPORCH, AS_UINT32(&hporch));

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_ConfigHsync);

DPI_STATUS DPI_FBEnable(DPI_FB_ID id, BOOL enable)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBEnable);

DPI_STATUS DPI_FBSyncFlipWithLCD(BOOL enable)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSyncFlipWithLCD);


DPI_STATUS DPI_FBSetFormat(DPI_FB_FORMAT format)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSetFormat);

DPI_FB_FORMAT DPI_FBGetFormat(void)
{
    return 0;
}
EXPORT_SYMBOL(DPI_FBGetFormat);


DPI_STATUS DPI_FBSetSize(UINT32 width, UINT32 height)
{
    DPI_REG_SIZE size;
    size.WIDTH = width;
    size.HEIGHT = height;
    
    OUTREG32(&DPI_REG->SIZE, AS_UINT32(&size));

    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSetSize);

DPI_STATUS DPI_FBSetAddress(DPI_FB_ID id, UINT32 address)
{
    return DPI_STATUS_OK;
}    
EXPORT_SYMBOL(DPI_FBSetAddress);

DPI_STATUS DPI_FBSetPitch(DPI_FB_ID id, UINT32 pitchInByte)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_FBSetPitch);

DPI_STATUS DPI_SetFifoThreshold(UINT32 low, UINT32 high)
{
    return DPI_STATUS_OK;
}
EXPORT_SYMBOL(DPI_SetFifoThreshold);


DPI_STATUS DPI_DumpRegisters(void)
{
    UINT32 i;

    DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "---------- Start dump DPI registers ----------\n");
    
    for (i = 0; i < sizeof(DPI_REGS); i += 4)
    {
        DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "DPI+%04x : 0x%08x\n", i, INREG32(DPI_BASE + i));
    }

    return DPI_STATUS_OK;
}

EXPORT_SYMBOL(DPI_DumpRegisters);

UINT32 DPI_GetCurrentFB(void)
{
	return 0;
}
EXPORT_SYMBOL(DPI_GetCurrentFB);

DPI_STATUS DPI_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
#if 0
    unsigned int i = 0;
    unsigned char *fbv;
    unsigned int fbsize = 0;
    unsigned int dpi_fb_bpp = 0;
    unsigned int w,h;
	BOOL dpi_needPowerOff = FALSE;
	if(!s_isDpiPowerOn){
		DPI_PowerOn();
		dpi_needPowerOff = TRUE;
		LCD_WaitForNotBusy();
	    LCD_WaitDPIIndication(FALSE);
		LCD_FBReset();
    	LCD_StartTransfer(TRUE);
		LCD_WaitDPIIndication(TRUE);
	}

    if(pvbuf == 0 || bpp == 0)
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "DPI_Capture_Framebuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return DPI_STATUS_OK;
    }

    if(DPI_FBGetFormat() == DPI_FB_FORMAT_RGB565)
    {
        dpi_fb_bpp = 16;
    }
    else if(DPI_FBGetFormat() == DPI_FB_FORMAT_RGB888)
    {
        dpi_fb_bpp = 24;
    }
    else
    {
        DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "DPI_Capture_Framebuffer, ERROR, dpi_fb_bpp is wrong: %d\n", dpi_fb_bpp);
        return DPI_STATUS_OK;
    }

    w = DISP_GetScreenWidth();
    h = DISP_GetScreenHeight();
    fbsize = w*h*dpi_fb_bpp/8;
	if(dpi_needPowerOff)
    	fbv = (unsigned char*)ioremap_cached((unsigned int)DPI_REG->FB[0].ADDR, fbsize);
	else
    	fbv = (unsigned char*)ioremap_cached((unsigned int)DPI_REG->FB[DPI_GetCurrentFB()].ADDR, fbsize);
 
    DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "current fb count is %d\n", DPI_GetCurrentFB());

    if(bpp == 32 && dpi_fb_bpp == 24)
    {			
    	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned int*)(pvbuf+ (pix_count - i) * 4) = 0xff000000|fbv[i*3]|(fbv[i*3+1]<<8)|(fbv[i*3+2]<<16);
    		}
		}
		else{
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned int*)(pvbuf+i*4) = 0xff000000|fbv[i*3]|(fbv[i*3+1]<<8)|(fbv[i*3+2]<<16);
    		}
		}
    }
    else if(bpp == 32 && dpi_fb_bpp == 16)
    {
        unsigned int t;
		unsigned short* fbvt = (unsigned short*)fbv;
    	
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
			
    		for(i = 0;i < w*h; i++)
    		{
				t = fbvt[i];
            	*(unsigned int*)(pvbuf+ (pix_count - i) * 4) = 0xff000000|((t&0x001F)<<3)|((t&0x07E0)<<5)|((t&0xF800)<<8);
    		}
		}
		else{
        	for(i = 0;i < w*h; i++)
    		{
	    		t = fbvt[i];
            	*(unsigned int*)(pvbuf+i*4) = 0xff000000|((t&0x001F)<<3)|((t&0x07E0)<<5)|((t&0xF800)<<8);
    		}
		}
    }
    else if(bpp == 16 && dpi_fb_bpp == 16)
    {
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
			unsigned short* fbvt = (unsigned short*)fbv;
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned short*)(pvbuf+ (pix_count - i) * 2) = fbvt[i];
    		}
		}
		else
    		memcpy((void*)pvbuf, (void*)fbv, fbsize);
    }
    else if(bpp == 16 && dpi_fb_bpp == 24)
    {
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)){
			unsigned int pix_count = w * h - 1;
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned short*)(pvbuf+ (pix_count - i) * 2) = ((fbv[i*3+0]&0xF8)>>3)|
	            	                        				((fbv[i*3+1]&0xFC)<<3)|
														    ((fbv[i*3+2]&0xF8)<<8);
    		}
		}
		else{
    		for(i = 0;i < w*h; i++)
    		{
            	*(unsigned short*)(pvbuf+i*2) = ((fbv[i*3+0]&0xF8)>>3)|
	            	                        ((fbv[i*3+1]&0xFC)<<3)|
						    				((fbv[i*3+2]&0xF8)<<8);
    		}
		}
    }
    else
    {
    	DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "DPI_Capture_Framebuffer, bpp:%d & dpi_fb_bpp:%d is not supported now\n", bpp, dpi_fb_bpp);
    }

    iounmap(fbv);
    	
	if(dpi_needPowerOff){
		DPI_PowerOff();
	}
#endif

    return DPI_STATUS_OK;    
}

DPI_STATUS DPI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_DPI_INTERRUPT
    switch(eventID)
    {
        case DISP_DPI_VSYNC_INT:
            DPI_REG->INT_ENABLE.VSYNC = 1;
            break;
        default:
            return DPI_STATUS_ERROR;
    }
    
    return DPI_STATUS_OK;
#else
    ///TODO: warning log here
    return DPI_STATUS_ERROR;
#endif
}


DPI_STATUS DPI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
    dpiIntCallback = pCB;

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FMDesense_Query(void)
{
    return DPI_STATUS_ERROR;
}

DPI_STATUS DPI_FM_Desense(unsigned long freq)
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Reset_CLK(void)
{
	return DPI_STATUS_OK;
}

DPI_STATUS DPI_Get_Default_CLK(unsigned int *clk)
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Get_Current_CLK(unsigned int *clk)
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Change_CLK(unsigned int clk)
{
    return DPI_STATUS_OK;
}
