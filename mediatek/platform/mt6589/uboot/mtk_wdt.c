/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>

#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mtk_wdt.h>

#if ENABLE_WDT_MODULE

static unsigned short timeout;
static unsigned short is_rgu_trigger_rst = 0;

static void mtk_wdt_disable(void)
{
    u16 tmp;

    tmp = DRV_Reg(MTK_WDT_MODE);
    tmp &= ~MTK_WDT_MODE_ENABLE;       /* disable watchdog */
    tmp |= (MTK_WDT_MODE_KEY);         /* need key then write is allowed */
    DRV_WriteReg(MTK_WDT_MODE,tmp);
}

static void mtk_wdt_reset(void)
{
    /* Watchdog Rest */
    DRV_WriteReg16(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY); 
    //DRV_WriteReg(MTK_WDT_MODE, (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_ENABLE));
    //DRV_WriteReg(MTK_WDT_LENGTH, MTK_WDT_LENGTH_KEY);
    DRV_WriteReg(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
}


unsigned int mtk_wdt_check_status(void)
{
    unsigned int status;

    status = DRV_Reg16(MTK_WDT_STATUS);

    return status;
}

static void mtk_wdt_mode_config(BOOL debug_en, 
                         BOOL irq, 
                         BOOL ext_en, 
                         BOOL ext_pol, 
                         BOOL wdt_en )
{
    unsigned short tmp;

    tmp = DRV_Reg16(MTK_WDT_MODE);
    tmp |= MTK_WDT_MODE_KEY;

    // Bit 0 : Whether enable watchdog or not
    if(wdt_en == TRUE)
        tmp |= MTK_WDT_MODE_ENABLE;
    else
        tmp &= ~MTK_WDT_MODE_ENABLE;

    // Bit 1 : Configure extern reset signal polarity.
    if(ext_pol == TRUE)
        tmp |= MTK_WDT_MODE_EXT_POL;
    else
        tmp &= ~MTK_WDT_MODE_EXT_POL;

    // Bit 2 : Whether enable external reset signal
    if(ext_en == TRUE)
        tmp |= MTK_WDT_MODE_EXTEN;
    else
        tmp &= ~MTK_WDT_MODE_EXTEN;

    // Bit 3 : Whether generating interrupt instead of reset signal
    if(irq == TRUE)
        tmp |= MTK_WDT_MODE_IRQ;
    else
        tmp &= ~MTK_WDT_MODE_IRQ;

    // Bit 6 : Whether enable debug module reset
    if(debug_en == TRUE)
        tmp |= MTK_WDT_MODE_DEBUG_EN;
    else
        tmp &= ~MTK_WDT_MODE_DEBUG_EN;

    DRV_WriteReg16(MTK_WDT_MODE,tmp);
}

void mtk_wdt_set_time_out_value(UINT16 value)
{
    /*
    * TimeOut = BitField 15:5
    * Key      = BitField  4:0 = 0x08
    */

    // sec * 32768 / 512 = sec * 64 = sec * 1 << 6
    timeout = (unsigned short)(value * ( 1 << 6) );
    timeout = timeout << 5; 
    DRV_WriteReg16(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY) );  
}

void mtk_wdt_restart(void)
{
    // Reset WatchDogTimer's counting value to time out value
    // ie., keepalive()

    DRV_WriteReg16(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
}

static void mtk_wdt_sw_reset(void)
{
    printf ("UB WDT SW RESET\n");
    //DRV_WriteReg16 (0x70025000, 0x2201);
    //DRV_WriteReg16 (0x70025008, 0x1971);
    //DRV_WriteReg16 (0x7002501C, 0x1209);
    mtk_wdt_reset();/* NOTE here, we change "reset count" to "set length" */

    // system will reset 

    while (1)
    {
        printf ("UB SW reset fail ... \n");
    };
}

static void mtk_wdt_hw_reset(void)
{
    printf("UB WDT_HW_Reset\n");

    // 1. set WDT timeout 1 secs, 1*64*512/32768 = 1sec
    mtk_wdt_set_time_out_value(1);

    // 2. enable WDT debug reset enable, generating irq disable, ext reset disable
    //    ext reset signal low, wdt enalbe
    mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);

    // 3. reset the watch dog timer to the value set in WDT_LENGTH register
    mtk_wdt_restart();

    // 4. system will reset
    while(1);
} 

/**
 * For Power off and power on reset, the INTERVAL default value is 0x7FF.
 * We set Interval[1:0] to different value to distinguish different stage.
 * Enter pre-loader, we will set it to 0x0
 * Enter u-boot, we will set it to 0x1
 * Enter kernel, we will set it to 0x2
 * And the default value is 0x3 which means reset from a power off and power on reset
 */
#define POWER_OFF_ON_MAGIC	(0x3)
#define PRE_LOADER_MAGIC	(0x0)
#define U_BOOT_MAGIC		(0x1)
#define KERNEL_MAGIC		(0x2)
#define MAGIC_NUM_MASK		(0x3)
/**
 * If the reset is trigger by RGU(Time out or SW trigger), we hope the system can boot up directly;
 * we DO NOT hope we must press power key to reboot system after reset.
 * This message should tell pre-loader and u-boot, and we use Interval[2] to store this information.
 * And this information will be cleared when enter kernel.
 */
#define IS_POWER_ON_RESET	(0x1<<2)
#define RGU_TRIGGER_RESET_MASK	(0x1<<2)
void mtk_wdt_init(void)
{
    unsigned short interval_val = DRV_Reg16(MTK_WDT_INTERVAL);
    mtk_wdt_mode_config(FALSE, FALSE, FALSE, FALSE, FALSE);
    printf("UB wdt init\n");

    /* Update interval register value and check reboot flag */
    if( (interval_val & RGU_TRIGGER_RESET_MASK) == IS_POWER_ON_RESET )
        is_rgu_trigger_rst = 0; // Power off and power on reset
    else
        is_rgu_trigger_rst = 1; // RGU trigger reset

    interval_val &= ~(RGU_TRIGGER_RESET_MASK|MAGIC_NUM_MASK);
    interval_val |= (U_BOOT_MAGIC|IS_POWER_ON_RESET);

    /* Write back INTERVAL REG */
    DRV_WriteReg(MTK_WDT_INTERVAL, interval_val);
}

BOOL mtk_is_rgu_trigger_reset(void)
{
    if(is_rgu_trigger_rst)
        return TRUE;
    return FALSE;
}

void mtk_arch_reset(char mode)
{
    printf("UB mtk_arch_reset\n");

    mtk_wdt_reset();

    while (1);
}
#else
void mtk_wdt_init(void)
{
    printf("UB WDT Dummy init called\n");
}
BOOL mtk_is_rgu_trigger_reset()
{
    printf("UB Dummy mtk_is_rgu_trigger_reset called\n");
    return FALSE;
}
void mtk_arch_reset(char mode)
{
    printf("UB WDT Dummy arch reset called\n");
}
#endif
