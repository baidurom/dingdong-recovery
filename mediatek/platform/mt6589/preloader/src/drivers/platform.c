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
#include "boot_device.h"
#include "mtk_nand.h"
#include "mmc_common_inter.h"

#include "uart.h"
#include "mtk_nand_core.h"
#include "mtk_pll.h"
#include "mtk_nand_core.h"
#include "mt_i2c.h"
#include "mt_rtc.h"
#include "mt_emi.h"
#include "mtk_pmic_6320.h"
#include "mt6577_cpu_power.h"
#include "mtk_wdt.h"
#include "ram_console.h"
#include "cust_sec_ctrl.h"
#include "gpio.h"
#include "mt_pmic_wrap_init.h"
#include "mtk_key.h"
/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD "[PLFM]"

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/
unsigned int sys_stack[CFG_SYS_STACK_SZ >> 2];
const unsigned int sys_stack_sz = CFG_SYS_STACK_SZ;
boot_mode_t g_boot_mode;
boot_dev_t g_boot_dev;
meta_com_t g_meta_com_type = META_UNKNOWN_COM;
u32 g_meta_com_id = 0;
boot_reason_t g_boot_reason;
ulong g_boot_time;
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
kal_bool kpoc_flag = false;
#endif

/*============================================================================*/
/* EXTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static u32 boot_device_init(void)
{
#if CFG_LEGACY_USB_DOWNLOAD
    #if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    print("[%s] boot device: SDMMC\n", MOD);

    // basic device function
    g_boot_dev.dev_init = mmc_init_device;
    g_boot_dev.dev_read_data = mmc_read_data;
    g_boot_dev.dev_write_data = mmc_write_data;
    g_boot_dev.dev_erase_data = mmc_erase_data;    
    g_boot_dev.dev_wait_ready = mmc_wait_ready;     
    g_boot_dev.dev_find_safe_block = mmc_find_safe_block;             

    // for checksum calculation
    g_boot_dev.dev_chksum_per_file = mmc_chksum_per_file;             
    g_boot_dev.dev_cal_chksum_body = mmc_chksum_body;

    #else
    print("[%s] boot device: NAND\n", MOD);

    // basic device function
    g_boot_dev.dev_init = nand_init_device;
    g_boot_dev.dev_read_data = nand_read_data;
    g_boot_dev.dev_write_data = nand_write_data;
    g_boot_dev.dev_erase_data = nand_erase_data;    
    g_boot_dev.dev_wait_ready = nand_wait_ready;     
    g_boot_dev.dev_find_safe_block = nand_find_safe_block;             

    // for checksum calculation
    g_boot_dev.dev_chksum_per_file = nand_chksum_per_file;             
    g_boot_dev.dev_cal_chksum_body = nand_chksum_body;
    #endif

    /* perform device init */
    return (u32)g_boot_dev.dev_init();
#else
    #if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    return (u32)mmc_init_device();
    #else
    return (u32)nand_init_device();
    #endif
#endif
}

int usb_accessory_in(void)
{
#if !CFG_FPGA_PLATFORM
    int exist = 0;

    if (PMIC_CHRDET_EXIST == pmic_IsUsbCableIn()) {
        exist = 1;
        #if !CFG_USBIF_COMPLIANCE
        /* enable charging current as early as possible to avoid can't enter 
         * following battery charging flow when low battery
         */
        platform_set_chrg_cur(450);
        #endif
    }
    return exist;
#else
    return 1;
#endif
}

int usb_cable_in(void)
{
#if !CFG_FPGA_PLATFORM
    int exist = 0;
    CHARGER_TYPE ret;

    if ((g_boot_reason == BR_USB) || usb_accessory_in()) {
        ret = mt_charger_type_detection();
        if (ret == PMIC_STANDARD_HOST || ret == PMIC_CHARGING_HOST) {
            print("\n%s USB cable in\n", MOD);
            mt_usb_phy_poweron();
            mt_usb_phy_savecurrent();

            /* enable pmic hw charger detection */
            #if CFG_BATTERY_DETECT
            if (hw_check_battery())
                pl_hw_ulc_det();
            #endif

            exist = 1;
        } else if (ret == PMIC_NONSTANDARD_CHARGER || ret == PMIC_STANDARD_CHARGER) {
            #if CFG_USBIF_COMPLIANCE
            platform_set_chrg_cur(450);
            #endif
        }
    }
    return exist;
#else
    mt_usb_phy_poweron();
    mt_usb_phy_savecurrent();
    return 1;
#endif
}

void platform_vusb_on(void)
{
#if !CFG_FPGA_PLATFORM
    U32 ret=0;

    ret=pmic_config_interface( (U32)(DIGLDO_CON2),
                               (U32)(1),
                               (U32)(PMIC_RG_VUSB_EN_MASK),
                               (U32)(PMIC_RG_VUSB_EN_SHIFT)
	                         );
    if(ret!=0)
    {
        print("[platform_vusb_on] Fail\n");
    }
    else
    {
        print("[platform_vusb_on] PASS\n");
    }    
#endif
    return;
}

void platform_set_boot_args()
{
#if CFG_BOOT_ARGUMENT
    boot_arg_t *bootarg = (boot_arg_t*)BOOT_ARGUMENT_ADDR;

    bootarg->magic = BOOT_ARGUMENT_MAGIC;
    bootarg->mode  = g_boot_mode;
    bootarg->e_flag = sp_check_platform();
    bootarg->log_port = CFG_UART_LOG;
    bootarg->log_baudrate = CFG_LOG_BAUDRATE;
    bootarg->log_enable = (u8)log_status();
    bootarg->dram_rank_num = get_dram_rank_nr();
    get_dram_rank_size(bootarg->dram_rank_size);
    bootarg->boot_reason = g_boot_reason;
    bootarg->meta_com_type = (u32)g_meta_com_type;
    bootarg->meta_com_id = g_meta_com_id;
    bootarg->boot_time = get_timer(g_boot_time);

    bootarg->da_info.addr = 0;
    bootarg->da_info.arg1 = 0;
    bootarg->da_info.arg2 = 0;

    print("\n%s boot reason: %d\n", MOD, g_boot_reason);
    print("%s boot mode: %d\n", MOD, g_boot_mode);
    print("%s META COM%d: %d\n", MOD, bootarg->meta_com_id, bootarg->meta_com_type);
    print("%s <0x%x>: 0x%x\n", MOD, &bootarg->e_flag, bootarg->e_flag);
    print("%s boot time: %dms\n", MOD, bootarg->boot_time);
#endif

}

void platform_set_dl_boot_args(da_info_t *da_info)
{
#if CFG_BOOT_ARGUMENT
    boot_arg_t *bootarg = (boot_arg_t*)BOOT_ARGUMENT_ADDR;

    bootarg->da_info.addr = da_info->addr;
    bootarg->da_info.arg1 = da_info->arg1;
    bootarg->da_info.arg2 = da_info->arg2;
#endif

    return;
}

void platform_wdt_all_kick(void)
{
    /* kick watchdog to avoid cpu reset */
    mtk_wdt_restart();

#if !CFG_FPGA_PLATFORM
    /* kick PMIC watchdog to keep charging */
    pl_kick_chr_wdt();
#endif
}

void platform_wdt_kick(void)
{
    /* kick hardware watchdog */
    mtk_wdt_restart();
}

#if CFG_DT_MD_DOWNLOAD
void platform_modem_download(void)
{
    print("[%s] modem download...\n", MOD);

    while (1) {
        platform_wdt_all_kick();
    }
}
#endif

#if CFG_EMERGENCY_DL_SUPPORT
void platform_safe_mode(int en, u32 timeout)
{
    U32 usbdlreg = 0;

    /* if anything is wrong and caused wdt reset, enter bootrom download mode */
    timeout = !timeout ? USBDL_TIMEOUT_MAX : timeout / 1000;
    timeout <<= 2;
    timeout &= USBDL_TIMEOUT_MASK; /* usbdl timeout cannot exceed max value */

    usbdlreg |= timeout;
    if (en)
	usbdlreg |= USBDL_BIT_EN;
    else
	usbdlreg &= ~USBDL_BIT_EN;
    usbdlreg &= ~USBDL_PL;
        
    DRV_WriteReg32(SRAMROM_USBDL, usbdlreg);

    return;
}

void platform_emergency_download(u32 timeout)
{
    /* enter download mode */
    print("%s emergency download mode(timeout: %ds).\n", MOD, timeout / 1000);
    platform_safe_mode(1, timeout);

#if !CFG_FPGA_PLATFORM
    mtk_arch_reset(0); /* don't bypass power key */
#endif
    
    while(1);
}
#else
void platform_safe_mode(int en, u32 timeout){}
#define platform_emergency_download(x)  do{}while(0)
#endif


int platform_get_mcp_id(u8 *id, u32 len, u32 *fw_id_len)
{
    int ret = -1;

    memset(id, 0, len);
    
#if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    ret = mmc_get_device_id(id, len,fw_id_len);
#else
    ret = nand_get_device_id(id, len);
#endif

    return ret;
}

void platform_set_chrg_cur(int ma)
{
    hw_set_cc(ma);
}

static boot_reason_t platform_boot_status(void)
{  
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	ulong begin = get_timer(0);
	do  {
		if (rtc_boot_check()) {
			print("%s RTC boot!\n", MOD);
			return BR_RTC;
		}
		if(!kpoc_flag)
			break;
	} while (get_timer(begin) < 1000 && kpoc_flag);
#else
    if (rtc_boot_check()) {
        print("%s RTC boot!\n", MOD);
        return BR_RTC;
    }

#endif
    if (mtk_wdt_boot_check() == WDT_NORMAL_REBOOT) {
        print("%s WDT normal boot!\n", MOD);
        return BR_WDT;
    } else if (mtk_wdt_boot_check() == WDT_BY_PASS_PWK_REBOOT){
        print("%s WDT reboot bypass power key!\n", MOD);
        return BR_WDT_BY_PASS_PWK;
    }
#if !CFG_FPGA_PLATFORM
    /* check power key */
    if (mtk_detect_key(8)) {
        print("%s Power key boot!\n", MOD);
        rtc_mark_bypass_pwrkey();
        return BR_POWER_KEY;
    }
#endif

#ifndef EVB_PLATFORM
    if (usb_accessory_in()) {
        print("%s USB/charger boot!\n", MOD);
        return BR_USB;
    }

    print("%s Unknown boot!\n", MOD);
    pl_power_off();
    /* should nerver be reached */
#endif

    print("%s Power key boot!\n", MOD);

    return BR_POWER_KEY;
}

#if CFG_LOAD_DSP_ROM || CFG_LOAD_MD_ROM
int platform_is_three_g(void)
{
    u32 tmp = sp_check_platform();

    return (tmp & 0x1) ? 0 : 1;
}
#endif

chip_ver_t platform_chip_ver(void)
{
    #if 0
    unsigned int hw_subcode = DRV_Reg32(APHW_SUBCODE);
    unsigned int sw_ver = DRV_Reg32(APSW_VER);
    #endif

    /* mt6589/mt6583 share the same hw_code/hw_sub_code/hw_version/sw_version currently */

    chip_ver_t ver = CHIP_6583_E1;

    return ver;
}

// ------------------------------------------------ 
// detect download mode
// ------------------------------------------------ 

bool platform_com_wait_forever_check(void)
{
#ifdef USBDL_DETECT_VIA_KEY
    /* check download key */
    if (TRUE == mtk_detect_key(COM_WAIT_KEY)) {
        print("%s COM handshake timeout force disable: Key\n", MOD);
        return TRUE;
    }
#endif

#ifdef USBDL_DETECT_VIA_AT_COMMAND
    /* check WDT_NONRST_REG */
    if (MTK_WDT_NONRST_DL == (INREG32(MTK_WDT_NONRST_REG) & MTK_WDT_NONRST_DL)) {
	print("%s COM handshake timeout force disable: AT Cmd\n", MOD);	
	CLRREG32(MTK_WDT_NONRST_REG, MTK_WDT_NONRST_DL);
	return TRUE;
    }
#endif

    return FALSE;
}

void platform_pre_init(void)
{
    u32 i2c_ret, pmic_ret;
    u32 pwrap_ret=0,i=0;
    int pll_ret;
     /* 2012/11/27
     * Sten
     * Add MT6589 MCI downsizer workaround start*/
    *(volatile unsigned int*)(0x10001200) |= (0x1);
     /* Add MT6589 MCI downsizer workaround end*/

    /* init timer */
    mtk_timer_init();

    /* init boot time */
    g_boot_time = get_timer(0);

#if 0 /* FIXME */
    /*
     * NoteXXX: CPU 1 may not be reset clearly after power-ON.
     *          Need to apply a S/W workaround to manualy reset it first.
     */
    {
	U32 val;
	val = DRV_Reg32(0xC0009010);
	DRV_WriteReg32(0xC0009010, val | 0x2);
	gpt_busy_wait_us(10);
	DRV_WriteReg32(0xC0009010, val & ~0x2);
	gpt_busy_wait_us(10);
    }
#ifndef SLT_BOOTLOADER        
    /* power off cpu1 for power saving */
    power_off_cpu1();
#endif
#endif

    /* init pll */
    pll_ret = mt_pll_init();

    /*GPIO init*/
    mt_gpio_init();

    /* init uart baudrate when pll on */
    mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);

    if (pll_ret != 0)
        print("MEMPLL 3PLL mode calibration fail\n");

    /* init pmic i2c interface and pmic */
    i2c_ret = i2c_v1_init();
    //retry 3 times for pmic wrapper init
    pwrap_init_preloader();

    pmic_ret = pmic6320_init();

//enable long press reboot function***************
#ifndef EVB_PLATFORM
#ifdef KPD_PMIC_LPRST_TD
	#ifdef ONEKEY_REBOOT_NORMAL_MODE_PL
			pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
			pmic_config_interface(GPIO_SMT_CON3,0x01, PMIC_RG_HOMEKEY_PUEN_MASK, PMIC_RG_HOMEKEY_PUEN_SHIFT);//pull up homekey pin of PMIC for 89 project
			pmic_config_interface(TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, PMIC_RG_PWRKEY_RST_TD_MASK, PMIC_RG_PWRKEY_RST_TD_SHIFT);
	#endif

	#ifdef TWOKEY_REBOOT_NORMAL_MODE_PL
			pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);//pmic package function for long press reboot function setting//
			pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);;//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
			pmic_config_interface(GPIO_SMT_CON3,0x01, PMIC_RG_HOMEKEY_PUEN_MASK, PMIC_RG_HOMEKEY_PUEN_SHIFT);//pull up homekey pin of PMIC for 89 project
			pmic_config_interface(TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, PMIC_RG_PWRKEY_RST_TD_MASK, PMIC_RG_PWRKEY_RST_TD_SHIFT);
	#endif
#endif
#endif
//************************************************


    print("%s Init I2C: %s(%d)\n", MOD, i2c_ret ? "FAIL" : "OK", i2c_ret);   
    print("%s Init PWRAP: %s(%d)\n", MOD, pwrap_ret ? "FAIL" : "OK", pwrap_ret);
    print("%s Init PMIC: %s(%d)\n", MOD, pmic_ret ? "FAIL" : "OK", pmic_ret);

    print("%s chip[%x]\n", MOD, platform_chip_ver());
}


#ifdef MTK_MT8193_SUPPORT
extern int mt8193_init(void);
#endif


void platform_init(void)
{
    u32 ret, tmp;
    boot_reason_t reason;
	
    /* init watch dog, will enable AP watch dog */
    mtk_wdt_init();
    /*init kpd PMIC mode support*/
    set_kpd_pmic_mode();


#if CFG_MDWDT_DISABLE 
    /* no need to disable MD WDT, the code here is for backup reason */

    /* disable MD0 watch dog. */
    DRV_WriteReg32(0x20050000, 0x2200); 

    /* disable MD1 watch dog. */
    DRV_WriteReg32(0x30050020, 0x2200); 
#endif

//ALPS00427972, implement the analog register formula
    //Set the calibration after power on
    //Add here for eFuse, chip version checking -> analog register calibration
    mt_usb_calibraion();
//ALPS00427972, implement the analog register formula

    /* make usb11 phy enter savecurrent mode */
    mt_usb11_phy_savecurrent();

#if 1 /* FIXME */
    g_boot_reason = reason = platform_boot_status();

    if (reason == BR_RTC || reason == BR_POWER_KEY || reason == BR_USB || reason == BR_WDT || reason == BR_WDT_BY_PASS_PWK)
        rtc_bbpu_power_on();
#else

    g_boot_reason = BR_POWER_KEY;

#endif

    enable_PMIC_kpd_clock();
#if CFG_EMERGENCY_DL_SUPPORT
    /* check if to enter emergency download mode */
    if (mtk_detect_dl_keys()) {
        platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    }
#endif

    /* init memory */
    mt_mem_init();

#ifdef MTK_MT8193_SUPPORT   
	mt8193_init();
#endif

    /* init device storeage */
    ret = boot_device_init();
    print("%s Init Boot Device: %s(%d)\n", MOD, ret ? "FAIL" : "OK", ret);

#if CFG_REBOOT_TEST
    mtk_wdt_sw_reset();
    while(1);
#endif

}

void platform_post_init(void)
{
    struct ram_console_buffer *ram_console;

#if CFG_BATTERY_DETECT
    /* normal boot to check battery exists or not */
    if (g_boot_mode == NORMAL_BOOT && !hw_check_battery() && usb_accessory_in()) {
        print("%s Wait for battery inserted...\n", MOD);
        /* disable pmic pre-charging led */
        pl_close_pre_chr_led();
        /* enable force charging mode */
        pl_charging(1);
        do {
            mdelay(300);
            /* check battery exists or not */
            if (hw_check_battery())
                break;
            /* kick all watchdogs */
            platform_wdt_all_kick();
        } while(1);
        /* disable force charging mode */
        pl_charging(0);
    }
#endif

#if !CFG_FPGA_PLATFORM
    /* security check */
    sec_lib_read_secro();
    sec_boot_check();
    device_APC_dom_setup();
#endif

#if CFG_MDJTAG_SWITCH 
    unsigned int md_pwr_con;

    /* md0 default power on and clock on */
    /* md1 default power on and clock off */

    /* ungate md1 */
    /* rst_b = 0 */
    md_pwr_con = DRV_Reg32(0x10006280);
    md_pwr_con &= ~0x1;
    DRV_WriteReg32(0x10006280, md_pwr_con);

    /* enable clksq2 for md1 */
    DRV_WriteReg32(0x10209000, 0x00001137);
    udelay(200);
    DRV_WriteReg32(0x10209000, 0x0000113f);

    /* rst_b = 1 */
    md_pwr_con = DRV_Reg32(0x10006280);
    md_pwr_con |= 0x1;
    DRV_WriteReg32(0x10006280, md_pwr_con);

    /* switch to MD legacy JTAG */
    /* this step is not essentially required */
#endif

#if CFG_MDMETA_DETECT 
    if (g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT) {
	/* trigger md0 to enter meta mode */
        DRV_WriteReg32(0x20000010, 0x1);
	/* trigger md1 to enter meta mode */
        DRV_WriteReg32(0x30000010, 0x1);
    } else {
	/* md0 does not enter meta mode */
        DRV_WriteReg32(0x20000010, 0x0);
	/* md1 does not enter meta mode */
        DRV_WriteReg32(0x30000010, 0x0);
    }
#endif

#if CFG_RAM_CONSOLE
    ram_console = (struct ram_console_buffer *)RAM_CONSOLE_ADDR;

    if (ram_console->sig == RAM_CONSOLE_SIG) {
        print("%s ram_console->start=0x%x\n", MOD, ram_console->start);
        if (ram_console->start > RAM_CONSOLE_MAX_SIZE)
            ram_console->start = 0;

		ram_console->hw_status = g_rgu_status;

        print("%s ram_console(0x%x)=0x%x (boot reason)\n", MOD, 
            ram_console->hw_status, g_rgu_status);
		    }
#endif

    platform_set_boot_args();

    /* post init pll */
    mt_pll_post_init();
}

void platform_error_handler(void)
{
    /* if log is disabled, re-init log port and enable it */
    if (log_status() == 0) {
        mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);
        log_ctrl(1);
    }
    print("%s preloader fatal error...\n", MOD);
    /* enter emergency download mode */
    platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    while(1);
}

void platform_assert(char *file, int line, char *expr)
{   
    print("<ASSERT> %s:line %d %s\n", file, line, expr); 
    platform_error_handler();
}

