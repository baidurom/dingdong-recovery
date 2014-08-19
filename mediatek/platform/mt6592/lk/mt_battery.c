#include <target/board.h>
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#define CFG_POWER_CHARGING
#endif
#ifdef CFG_POWER_CHARGING
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/boot_mode.h>
#include <platform/mt_gpt.h>
#include <platform/mt_rtc.h>
#include <platform/mt_disp_drv.h>
#include <platform/mtk_wdt.h>
#include <platform/mtk_key.h>
#include <platform/mt_logo.h>
#include <platform/mt_leds.h>
#include <printf.h>
#include <sys/types.h>
#include <target/cust_battery.h>

#undef printf


/*****************************************************************************
 *  Type define
 ****************************************************************************/
#define BATTERY_LOWVOL_THRESOLD             3350  //liguowei@wind-mobi.com 20131111


/*****************************************************************************
 *  Global Variable
 ****************************************************************************/
bool g_boot_reason_change = false;


/*****************************************************************************
 *  Externl Variable
 ****************************************************************************/
extern bool g_boot_menu;

#ifdef MTK_FAN5405_SUPPORT
extern void fan5405_hw_init(void);
extern void fan5405_turn_on_charging(void);
extern void fan5405_dump_register(void);
#endif

#ifdef MTK_BQ24196_SUPPORT
extern void bq24196_hw_init(void);
extern void bq24196_charging_enable(kal_uint32 bEnable);
extern void bq24196_dump_register(void);
extern kal_uint32 bq24196_get_chrg_stat(void);
#endif

void kick_charger_wdt(void)
{
    upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
    upmu_set_rg_chrwdt_wr(1); 			  // CHRWDT_WR
    upmu_set_rg_chrwdt_int_en(1);         // CHRWDT_INT_EN
    upmu_set_rg_chrwdt_en(1);             // CHRWDT_EN
    upmu_set_rg_chrwdt_flag_wr(1);        // CHRWDT_WR
}

kal_bool is_low_battery(kal_uint32 val)
{
    #ifdef MTK_BQ24196_SUPPORT
    kal_uint32 bq24196_chrg_status;
    
    if(0 == val)
        val = get_i_sense_volt(5);
    #endif
    
    if (val < BATTERY_LOWVOL_THRESOLD)
    {
        dprintf(INFO, "%s, TRUE\n", __FUNCTION__);
        return KAL_TRUE;
    }
    else
    {
        #ifdef MTK_BQ24196_SUPPORT
        bq24196_chrg_status = bq24196_get_chrg_stat();
        dprintf(INFO, "bq24196_chrg_status = %d\n", bq24196_chrg_status);
    
        if(bq24196_chrg_status == 0x1) //Pre-charge
        {
            dprintf(INFO, "%s, battery protect TRUE\n", __FUNCTION__);
            return KAL_TRUE;
        }  
        #endif
    }
    
    dprintf(INFO, "%s, FALSE\n", __FUNCTION__);
    return KAL_FALSE;
}

void pchr_turn_on_charging (void)
{
	upmu_set_rg_usbdl_rst(1);		//force leave USBDL mode
	
	kick_charger_wdt();
	
    upmu_set_rg_cs_vth(0xC);    	// CS_VTH, 450mA            
    upmu_set_rg_csdac_en(1);                // CSDAC_EN
    upmu_set_rg_chr_en(1);                  // CHR_EN  

#ifdef MTK_FAN5405_SUPPORT
    fan5405_hw_init();
    fan5405_turn_on_charging();
    fan5405_dump_register();
#endif

#ifdef MTK_BQ24196_SUPPORT
    bq24196_hw_init();
    bq24196_charging_enable(0);  //disable charging with power path
    bq24196_dump_register();
#endif
}


void mt65xx_bat_init(void)
{    
		kal_int32 bat_vol;
		
		// Low Battery Safety Booting
		
		bat_vol = get_bat_sense_volt(1);

		#ifdef MTK_BQ24196_SUPPORT
		bat_vol = get_i_sense_volt(5);
		#endif

		dprintf(INFO, "[mt65xx_bat_init] check VBAT=%d mV with %d mV\n", bat_vol, BATTERY_LOWVOL_THRESOLD);
		
		pchr_turn_on_charging();

		if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && (upmu_get_pwrkey_deb()==0) ) {
				dprintf(INFO, "[mt65xx_bat_init] KPOC+PWRKEY => change boot mode\n");		
		
				g_boot_reason_change = true;
		}
		rtc_boot_check(false);

	#ifndef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
    //if (bat_vol < BATTERY_LOWVOL_THRESOLD)
    if (is_low_battery(bat_vol))
    {
        if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && upmu_is_chr_det() == KAL_TRUE)
        {
            dprintf(INFO, "[%s] Kernel Low Battery Power Off Charging Mode\n", __func__);
            g_boot_mode = LOW_POWER_OFF_CHARGING_BOOT;
            return;
        }
        else
        {
            dprintf(INFO, "[BATTERY] battery voltage(%dmV) <= CLV ! Can not Boot Linux Kernel !! \n\r",bat_vol);
#ifndef NO_POWER_OFF
            mt6575_power_off();
#endif			
            while(1)
            {
                dprintf(INFO, "If you see the log, please check with RTC power off API\n\r");
            }
        }
    }
	#endif
    return;
}

#else

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <printf.h>

void mt65xx_bat_init(void)
{
    dprintf(INFO, "[BATTERY] Skip mt65xx_bat_init !!\n\r");
    dprintf(INFO, "[BATTERY] If you want to enable power off charging, \n\r");
    dprintf(INFO, "[BATTERY] Please #define CFG_POWER_CHARGING!!\n\r");
}

#endif
