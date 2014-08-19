/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <dev/uart.h>
#include <arch/arm/mmu.h>
#include <arch/ops.h>

#include <string.h>
#include <stdlib.h>
#include <platform/mmc_core.h>
#include <platform/mt_disp_drv.h>
#include <platform/mtk_wdt.h>

#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <mt_partition.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#include <video.h>
#include <target/board.h>
#include <platform/mt_logo.h>
#include <platform/mt_gpio.h>
#include <platform/mtk_key.h>
#include <platform/mt_pmic_wrap_init.h>
#include <platform/mt_rtc.h>
#include <platform/env.h>
#include <platform/mt_leds.h>
#include <platform/mt_typedefs.h>


#ifdef LK_DL_CHECK
/*block if check dl fail*/
#undef LK_DL_CHECK_BLOCK_LEVEL
#endif

#ifdef DUMMY_AP
#define PART_MAX 10
part_hdr_t part_info_temp[PART_MAX];
#endif

extern void platform_early_init_timer();
extern int sec_func_init(int dev_type);
extern int sec_usbdl_enabled (void);
extern int sec_usbdl_verify_da(u8 *data_buf, u32 data_len, u8 *sig_buf, u32 sig_len);
extern void platform_deinit_interrupts(void);
void platform_uninit(void);
void config_shared_SRAM_size(void);
extern  void mtk_wdt_disable(void);

#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
extern kal_bool is_low_battery(kal_uint32 val);
extern kal_uint32 charging_get_charger_type(void *data);
#endif

/* Transparent to DRAM customize */
int g_nr_bank;
int g_rank_size[4];
unsigned int g_fb_base;
unsigned int g_fb_size;
BOOT_ARGUMENT *g_boot_arg;
BOOT_ARGUMENT boot_addr;
BI_DRAM bi_dram[MAX_NR_BANK];

//xuecheng, for 82 bring up
//#define DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
extern void jump_da(u32 addr, u32 arg1, u32 arg2);
//#define DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
#define REPAIR_SRAM_ENABLE_LK_FOR_82_BRINGUP
#ifdef REPAIR_SRAM_ENABLE_LK_FOR_82_BRINGUP
extern int repair_sram(void);
#endif

extern void isink0_init(void);
extern U32 pmic_init (void);
extern int mboot_common_load_logo(unsigned long logo_addr, char* filename);
int dram_init(void)
{
    int i;
    unsigned int dram_rank_num;

#ifndef MTK_EMMC_SUPPORT
    g_nr_bank = 2;
        bi_dram[0].start = RIL_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = 256*1024*1024;
        bi_dram[1].start = bi_dram[0].size + DRAM_PHY_ADDR;;
        bi_dram[1].size = 256*1024*1024;
        g_rank_size[0]= bi_dram[0].size;
        g_rank_size[1]= bi_dram[1].size;
#else
    /* Get parameters from pre-loader. Get as early as possible
     * The address of BOOT_ARGUMENT_LOCATION will be used by Linux later
     * So copy the parameters from BOOT_ARGUMENT_LOCATION to LK's memory region 
     */
    g_boot_arg = &boot_addr;
    memcpy(g_boot_arg,(void *) BOOT_ARGUMENT_LOCATION, sizeof(BOOT_ARGUMENT));

    dram_rank_num = g_boot_arg->dram_rank_num;    
    g_nr_bank = dram_rank_num;
#ifdef MACH_FPGA
 g_nr_bank = 2;
#endif

    for (i = 0; i < g_nr_bank; i++)
    {
        g_rank_size[i] = g_boot_arg->dram_rank_size[i];
    }

    if (g_nr_bank == 1)
    {
        bi_dram[0].start = RIL_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = g_rank_size[0] - RIL_SIZE;
    } else if (g_nr_bank == 2)
    {
        bi_dram[0].start = RIL_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = g_rank_size[0] - RIL_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];

#ifdef MACH_FPGA
        g_nr_bank = 2;
        bi_dram[0].start = RIL_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = 256*1024*1024;
        bi_dram[1].start = bi_dram[0].size + DRAM_PHY_ADDR;;
        bi_dram[1].size = 256*1024*1024;
        g_rank_size[0]= bi_dram[0].size;
        g_rank_size[1]= bi_dram[1].size;
#endif


    } else if (g_nr_bank == 3)
    {
        bi_dram[0].start = RIL_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = g_rank_size[0] - RIL_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;;
        bi_dram[1].size = g_rank_size[1];
        bi_dram[2].start = bi_dram[1].start + bi_dram[1].size;
        bi_dram[2].size = g_rank_size[2];
    } else if (g_nr_bank == 4)
    {
        bi_dram[0].start = RIL_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = g_rank_size[0] - RIL_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;;
        bi_dram[1].size = g_rank_size[1];
        bi_dram[2].start = bi_dram[1].start + bi_dram[1].size;
        bi_dram[2].size = g_rank_size[2];
        bi_dram[3].start = bi_dram[2].start + bi_dram[2].size;
        bi_dram[3].size = g_rank_size[3];
    } else
    {
        //dprintf(CRITICAL, "[LK ERROR] DRAM bank number is not correct!!!");
        while (1) ;
    }
#endif
    return 0;
}

/*******************************************************
 * Routine: memory_size
 * Description: return DRAM size to LCM driver
 ******************************************************/
u32 memory_size(void)
{
    int nr_bank = g_nr_bank;
    int i, size = 0;

    for (i = 0; i < nr_bank; i++)
        size += bi_dram[i].size;
    size += RIL_SIZE;

    return size;
}

void sw_env()
{
//    int dl_status = 0;

#ifdef LK_DL_CHECK
#ifdef MTK_EMMC_SUPPORT
    dl_status = mmc_get_dl_info();
    dprintf(INFO,"mt65xx_sw_env--dl_status: %d\n", dl_status);
    if (dl_status != 0)
    {
        video_printf("=> TOOL DL image Fail!\n");
        dprintf(INFO,"TOOL DL image Fail\n");
#ifdef LK_DL_CHECK_BLOCK_LEVEL
        dprintf(INFO,"uboot is blocking by dl info\n");
        while (1) ;
#endif
    }
#endif
#endif

#ifndef USER_BUILD
    switch (g_boot_mode)
    {
      case META_BOOT:
          video_printf(" => META MODE\n");
          break;
      case FACTORY_BOOT:
          video_printf(" => FACTORY MODE\n");
          break;
      case RECOVERY_BOOT:
          video_printf(" => RECOVERY MODE\n");
          break;
      case SW_REBOOT:
          //video_printf(" => SW RESET\n");
          break;
      case NORMAL_BOOT:
          //if(g_boot_arg->boot_reason != BR_RTC && get_env("hibboot") != NULL && atoi(get_env("hibboot")) == 1)
          if(get_env("hibboot") != NULL && atoi(get_env("hibboot")) == 1)
              video_printf(" => HIBERNATION BOOT\n");
          else
              video_printf(" => NORMAL BOOT\n");
          break;
      case ADVMETA_BOOT:
          video_printf(" => ADVANCED META MODE\n");
          break;
      case ATE_FACTORY_BOOT:
          video_printf(" => ATE FACTORY MODE\n");
          break;
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	case KERNEL_POWER_OFF_CHARGING_BOOT:
		video_printf(" => POWER OFF CHARGING MODE\n");
		break; 
	case LOW_POWER_OFF_CHARGING_BOOT:
		video_printf(" => LOW POWER OFF CHARGING MODE\n");
		break; 
#endif
      case ALARM_BOOT:
          video_printf(" => ALARM BOOT\n");
          break;
      case FASTBOOT:
          video_printf(" => FASTBOOT mode...\n");
          break;
      default:
          video_printf(" => UNKNOWN BOOT\n");
    }
    return;
#endif

#ifdef USER_BUILD
    if(g_boot_mode == FASTBOOT)
        video_printf(" => FASTBOOT mode...\n");
#endif
}

void platform_init_mmu_mappings(void)
{
  /* configure available RAM banks */
  dram_init();
  
/* Enable D-cache  */
#if 1
  unsigned int addr;
//  unsigned int i = 0;
  unsigned int dram_size = 0;

  dram_size = memory_size();

  for (addr = 0; addr < dram_size; addr += (1024*1024))
  {
    /*virtual to physical 1-1 mapping*/
    arm_mmu_map_section(bi_dram[0].start+addr,bi_dram[0].start+addr, MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE | MMU_MEMORY_AP_READ_WRITE);
  }
#endif  
}
//#define PMIC_WRAP_PORTING //only for lk early porting
void platform_early_init(void)
{
#ifdef LK_PROFILING
    unsigned int time_led_init;
    unsigned int time_pmic6329_init;
    unsigned int time_platform_early_init;
    unsigned int time_repair_sram;
    unsigned int time_display_early_init;
    unsigned int time_wdt_early_init;
    time_platform_early_init = get_timer(0);
#endif

    platform_init_interrupts();
    platform_early_init_timer();
#ifndef MACH_FPGA
    mt_gpio_set_default();
#endif
    /* initialize the uart */
    uart_init_early();
    #ifdef REPAIR_SRAM_ENABLE_LK_FOR_82_BRINGUP
    #ifdef LK_PROFILING
    time_repair_sram = get_timer(0);
    #endif
    int repair_ret;
    repair_ret = repair_sram();
    if(repair_ret != 0)
    {
        dprintf(CRITICAL,"Sram repair failed %d\n", repair_ret);
        while(1);
    }
    #ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- Repair SRAM takes %d ms -------- \n",(int) get_timer(time_repair_sram));
    #endif
    #endif
    //i2c_v1_init();
    #ifdef LK_PROFILING
    time_wdt_early_init = get_timer(0);
    #endif
    mtk_wdt_init();
    #ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- WDT Init  takes %d ms -------- \n", (int)get_timer(time_wdt_early_init));
    #endif
// WDT will be triggered when uncompressing linux image on FPGA
#ifdef MACH_FPGA
    mtk_wdt_disable();
#endif

#ifdef DUMMY_AP
   memcpy(&part_info_temp,(unsigned int*)g_boot_arg->part_info,(sizeof(part_hdr_t)*((unsigned int)g_boot_arg->part_num)));
   g_boot_arg->part_info=&part_info_temp;
#endif


/* initialize the frame buffet information */
//FIXME: Disable for MT6582 FPGA Ealry Porting  
#ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP 
    #ifdef LK_PROFILING
    time_display_early_init = get_timer(0);
    #endif
    g_fb_size = mt_disp_get_vram_size();
    g_fb_base = memory_size() - g_fb_size + DRAM_PHY_ADDR;
    dprintf(INFO, "FB base = 0x%x, FB size = %d\n", g_fb_base, g_fb_size);
    #ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- Display eraly init takes %d ms -------- \n", (int)get_timer(time_display_early_init));
    #endif
#endif


#ifdef LK_PROFILING
    time_led_init = get_timer(0);
#endif

#ifndef MACH_FPGA
    leds_init();
#endif

#ifdef LK_PROFILING
      dprintf(INFO,"[PROFILE] ------- led init takes %d ms -------- \n", (int)get_timer(time_led_init));
#endif

    isink0_init();              //turn on PMIC6329 isink0



//#ifdef MACH_FPGA  
   // pwrap_init_lk();
    //pwrap_init_for_early_porting();
//#endif

#ifdef LK_PROFILING
    time_pmic6329_init = get_timer(0);
#endif

    pmic_init();

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- pmic_init takes %d ms -------- \n", (int)get_timer(time_pmic6329_init));
    dprintf(INFO,"[PROFILE] ------- platform_early_init takes %d ms -------- \n", (int)get_timer(time_platform_early_init));
#endif
}

extern void mt65xx_bat_init(void); 
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)

int kernel_charging_boot(void)
{
	if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_TRUE)
	{
		dprintf(INFO,"[%s] Kernel Power Off Charging with Charger/Usb \n", __func__);
		return  1;
	}
	else if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_FALSE)
	{
		dprintf(INFO,"[%s] Kernel Power Off Charging without Charger/Usb \n", __func__);
		return -1;
	}
	else
		return 0;
}
#endif

//#define MTK_EMMC_SUPPORT_TEST 1
#ifdef MTK_EMMC_SUPPORT_TEST
#define LK_MMC_TEST_SIZE   256*512
unsigned char g_mmc_buf[LK_MMC_TEST_SIZE + 1];
#endif

void platform_init(void)
{

#ifdef MACH_FPGA
   mtk_timer_init();
#endif
#ifdef LK_PROFILING
    unsigned int time_nand_emmc;
    unsigned int time_load_logo;
    unsigned int time_bat_init;
    unsigned int time_backlight;
    unsigned int time_show_logo;
    unsigned int time_boot_mode;
    unsigned int time_sw_env;
    unsigned int time_platform_init;
    unsigned int time_env;
    unsigned int time_disp_init;
    unsigned int time_security_init;
    unsigned int time_RTC_boot_Check;
    time_platform_init = get_timer(0);
    time_nand_emmc = get_timer(0);
#endif

#ifdef MTK_EMMC_SUPPORT_TEST
    unsigned int i;
#endif
    dprintf(CRITICAL, "platform_init()\n");

#ifdef DUMMY_AP
	dummy_ap_entry();
#endif

//FIX ME FPGA early porting
#ifdef MTK_EMMC_SUPPORT
    mmc_legacy_init(1);
#else
#ifndef MACH_FPGA
    nand_init();
    nand_driver_test();
   
#endif
#endif
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if((g_boot_arg->boot_reason == BR_USB) && (upmu_is_chr_det() == KAL_FALSE))
	{
		dprintf(INFO,"[%s] Unplugged Charger/Usb between Pre-loader and Uboot in Kernel Charging Mode, Power Off \n", __func__);
		mt6575_power_off();
	}
#endif
#ifdef MTK_EMMC_SUPPORT_TEST
    /* write 0x40000000(1G) with 256 block(0x5a) */
    dprintf(INFO,"[LK][%s:%d]emmc simp test start\n", __func__, __LINE__);
    memset(g_mmc_buf, 0x5a, LK_MMC_TEST_SIZE);

    for (i = 0; i < LK_MMC_TEST_SIZE; i++){
        g_mmc_buf[i] = 0x5a;
    }

    /* 0 is for emmc */
    mmc_block_write(0, 0x40000000/512, LK_MMC_TEST_SIZE/512, g_mmc_buf);
    
    /* read */
     for (i = 0; i < LK_MMC_TEST_SIZE; i++){
        g_mmc_buf[i] = 0x0;
    }  
    mmc_block_read(0, 0x40000000/512, LK_MMC_TEST_SIZE/512, g_mmc_buf);

    /* compara */
    for (i = 0; i < LK_MMC_TEST_SIZE; i++){
        if (g_mmc_buf[i] != 0x5a){
            dprintf(INFO,"[LK][%s:%d]mmc simple r/w test failed(%d is %d, not 0x5a)\n", __func__, __LINE__, i, g_mmc_buf[i]);
            break;
        }
    }

    dprintf(INFO,"[LK][%s:%d]emmc simp test end\n", __func__, __LINE__);
#endif

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- NAND/EMMC init takes %d ms -------- \n", (int)get_timer(time_nand_emmc));
    time_env = get_timer(0);
#endif
	env_init();
	print_env();
#ifdef LK_PROFILING
	dprintf(INFO,"[PROFILE] ------- ENV init takes %d ms -------- \n", (int)get_timer(time_env));
#endif

#ifdef LK_PROFILING
		time_disp_init = get_timer(0);
#endif

	//FIXME: Disable for MT6582 FPGA Ealry Porting
 #ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
	 mt_disp_init((void *)g_fb_base);
 #endif 
#ifdef LK_PROFILING
	 dprintf(INFO,"[PROFILE] ------- disp init takes %d ms -------- \n", (int)get_timer(time_disp_init));
#endif


#ifdef LK_PROFILING
                time_load_logo = get_timer(0);
#endif

#ifdef CONFIG_CFB_CONSOLE
		//FIXME: Disable for MT6582 FPGA Ealry Porting
    #ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
		drv_video_init();
    #endif
#endif

    //#endif
    #ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
    mboot_common_load_logo((unsigned long)mt_get_logo_db_addr(), "logo");
    dprintf(INFO, "Show BLACK_PICTURE\n");
    #endif
#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
    if (!is_low_battery(0))
    {
#endif
    //FIXME: Disable for MT6582 FPGA Ealry Porting
    #ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
    mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
    mt_disp_power(TRUE);
    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    mt_disp_wait_idle();    
    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    mt_disp_wait_idle();
    mt_disp_power(1);           //power on display related modules
    #endif
#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
    }
#endif 

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- load_logo takes %d ms -------- \n", (int)get_timer(time_load_logo));
#endif

    #ifdef LK_PROFILING
    time_backlight = get_timer(0);
    #endif
    /*for kpd pmic mode setting*/
    set_kpd_pmic_mode();

#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
    if (!is_low_battery(0))
    {
#endif
    //FIXME: Disable for MT6582 FPGA Ealry Porting
    #ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
    mt65xx_backlight_on();
    #endif
#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
    }
#endif 

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- backlight takes %d ms -------- \n", (int)get_timer(time_backlight));
#endif

#ifdef LK_PROFILING
    time_boot_mode = get_timer(0);
#endif


#ifndef MACH_FPGA
    boot_mode_select();
#endif

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- boot mode select takes %d ms -------- \n", (int)get_timer(time_boot_mode));
#endif


#ifdef MTK_SECURITY_SW_SUPPORT    

    /* initialize security library */
#ifdef MTK_EMMC_SUPPORT
    #ifdef LK_PROFILING
        time_security_init = get_timer(0);
    #endif
    #ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        sec_func_init(3);
    #else
        sec_func_init(1);
    #endif
#else
    sec_func_init(0);
#endif
    #ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- Security init takes %d ms -------- \n", (int)get_timer(time_security_init));
    #endif
#endif


    /*Show download logo & message on screen */
    if (g_boot_arg->boot_mode == DOWNLOAD_BOOT)
    {
	dprintf(CRITICAL,"[LK] boot mode is DOWNLOAD_BOOT\n");
	
#ifdef MTK_SECURITY_SW_SUPPORT    
	/* verify da before jumping to da*/
	if (sec_usbdl_enabled()) {
	    u8  *da_addr =(u8*) g_boot_arg->da_info.addr;
	    u32 da_len   = g_boot_arg->da_info.len;
	    u32 sig_len  = g_boot_arg->da_info.sig_len;
	    u8  *sig_addr = (unsigned char *)da_addr + (da_len - sig_len);

	    if (da_len == 0 || sig_len == 0) {
		dprintf(INFO,"[LK] da argument is invalid\n");
		dprintf(INFO,"da_addr = 0x%x\n",(int) da_addr);
		dprintf(INFO,"da_len  = 0x%x\n", da_len);
		dprintf(INFO,"sig_len = 0x%x\n", sig_len);
	    }

	    if (sec_usbdl_verify_da(da_addr, (da_len - sig_len), sig_addr, sig_len)) {
		/* da verify fail */    
                video_printf(" => Not authenticated tool, download stop...\n");
		while(1); /* fix me, should not be infinite loop in lk */
	    }
	}
	else 
#endif
	{
	    dprintf(INFO," DA verification disabled...\n");
	}
        mt_disp_show_boot_logo();
        video_printf(" => Downloading...\n");
        mt65xx_backlight_on();
        mtk_wdt_disable();//Disable wdt before jump to DA
        platform_uninit();
#ifdef HAVE_CACHE_PL310
        l2_disable();
#endif

#ifdef ENABLE_L2_SHARING
        config_shared_SRAM_size();
#endif
        arch_disable_cache(UCACHE);
        arch_disable_mmu();
        jump_da(g_boot_arg->da_info.addr, g_boot_arg->da_info.arg1, g_boot_arg->da_info.arg2);
    }

#ifdef LK_PROFILING
    time_bat_init = get_timer(0);
#endif


    mt65xx_bat_init();

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- battery init takes %d ms -------- \n", (int)get_timer(time_bat_init));
#endif

#ifndef CFG_POWER_CHARGING
    #ifdef LK_PROFILING
    time_RTC_boot_Check = get_timer(0);
    #endif
    /* NOTE: if define CFG_POWER_CHARGING, will rtc_boot_check() in mt65xx_bat_init() */
    rtc_boot_check(false);
    #ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- RTC boot check Init  takes %d ms -------- \n", (int)get_timer(time_RTC_boot_Check));
    #endif

#endif

#ifdef LK_PROFILING
    time_show_logo = get_timer(0);
#endif

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if(kernel_charging_boot() == 1)
	{         
		#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
		CHARGER_TYPE CHR_Type_num = CHARGER_UNKNOWN;
		charging_get_charger_type(&CHR_Type_num);
		if ((g_boot_mode != LOW_POWER_OFF_CHARGING_BOOT) ||
		((CHR_Type_num != STANDARD_HOST) && (CHR_Type_num != NONSTANDARD_CHARGER)))
		{
			dprintf(INFO, "[PROFILE] ------- g_boot_mode = %d -------- \n", g_boot_mode);
		#endif			
		mt_disp_power(TRUE);
		mt_disp_show_low_battery();	
		mt_disp_wait_idle();		
		mt65xx_leds_brightness_set(6, 110);	
		#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
		}
		#endif
	}   
	else if(g_boot_mode != KERNEL_POWER_OFF_CHARGING_BOOT && g_boot_mode != LOW_POWER_OFF_CHARGING_BOOT)
	{
#ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
		if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
		{
			mt_disp_show_boot_logo();
		}
#endif     
	}
#else
#ifndef DISABLE_DISPLAY_IN_LK_FOR_82_BRINGUP
	if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
	{
		mt_disp_show_boot_logo();
	}
#endif     
#endif

#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- show logo takes %d ms -------- \n", (int)get_timer(time_show_logo));
    time_sw_env= get_timer(0);
#endif

#ifndef MACH_FPGA
    sw_env();
#endif
#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- sw_env takes %d ms -------- \n", (int)get_timer(time_sw_env));
    dprintf(INFO,"[PROFILE] ------- platform_init takes %d ms -------- \n", (int)get_timer(time_platform_init));
#endif
}

void platform_uninit(void)
{
    leds_deinit();
    platform_deinit_interrupts();
}

#ifdef ENABLE_L2_SHARING
#define L2C_SIZE_CFG_OFF 5
/* config L2 cache and sram to its size */
void config_L2_size(void)
{
        volatile unsigned int cache_cfg;
        /* set L2C size to 128KB */
        cache_cfg = DRV_Reg(MCUSYS_CFGREG_BASE);
        cache_cfg &= (~0x7) << L2C_SIZE_CFG_OFF;
        cache_cfg |= 0x3 << L2C_SIZE_CFG_OFF;
        DRV_WriteReg(MCUSYS_CFGREG_BASE, cache_cfg);  
}
#endif


#ifdef ENABLE_L2_SHARING
/* config SRAM back from L2 cache for DA relocation */
void config_shared_SRAM_size(void)
{
        volatile unsigned int cache_cfg;
        /* set L2C size to 256KB */
        cache_cfg = DRV_Reg(MCUSYS_CFGREG_BASE);
        cache_cfg &= (~0x7) << L2C_SIZE_CFG_OFF;
        cache_cfg |= 0x1 << L2C_SIZE_CFG_OFF;
        DRV_WriteReg(MCUSYS_CFGREG_BASE, cache_cfg);
}
#endif

