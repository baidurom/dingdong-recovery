/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <linux/jiffies.h>

#include <asm/system.h>
#include <asm/uaccess.h>

#include "mach/mt_freqhopping.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_cpufreq.h"
#include "mach/sync_write.h"

#include "mach/mt6333.h"
#include <mach/emi_bwl.h>

/* PMIC external buck */
#include <mach/pmic_sw.h>
#include <cust_pmic.h>

/**************************************************
* enable for MEMPLL OD
***************************************************/
//#define CPUFREQ_MEMPLL_OD

#ifdef CPUFREQ_MEMPLL_OD
#include <mach/mt_dramc.h>
#endif
extern kal_uint32 bat_get_ui_percentage(void);
/**************************************************
* enable for DVFS random test
***************************************************/
//#define MT_DVFS_RANDOM_TEST

/**************************************************
* Define for bring up
***************************************************/
//#define MT_DVFS_BRINGUP

/**************************************************
* Define for FF TT SS voltage test
***************************************************/
#define MT_DVFS_FFTT_TEST
//#define MT_OFFICIAL_TURBO
 
/**************************************************
* If MT6333 supported, VPROC could support lower than 1.0V
***************************************************/
#if defined(IS_VCORE_USE_6333VCORE)
#define MT_DVFS_LOW_VOLTAGE_SUPPORT
#endif

/**************************************************
* PTPOD downgrade frequency
***************************************************/
#define PTPOD_DOWNGRADE_FREQ

/**************************************************
* Low battery percentage protect
***************************************************/
#define PTPOD_LOW_BATT_PERCENTAGE_FREQ

/**************************************************
* Turbo mode in highest frequency
***************************************************/
#define CPUFREQ_HIGHEST_TURBO_MODE

/**************************************************
* enable this option to use hopping control
***************************************************/
#define MT_CPUFREQ_FHCTL
#define FHCTL_CHANGE_FREQ (1000000) //KHz /* If cross 1GHz when DFS, not used FHCTL. */

/**************************************************
* CPU possible number
***************************************************/
#define MT_CPUFREQ_POSSIBLE_CPU   8

/**************************************************
* Define register write function
***************************************************/
#define mt_cpufreq_reg_write(val, addr)        mt65xx_reg_sync_writel((val), ((void *)addr))

/***************************
* debug message
****************************/
#define dprintk(fmt, args...)                                       \
do {                                                                \
    if (mt_cpufreq_debug) {                                         \
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", fmt, ##args);   \
    }                                                               \
} while(0)

#define Socprintk(fmt, args...)                                       \
do {																\
	if (mt_socfreq_debug) { 										\
		xlog_printk(ANDROID_LOG_INFO, "Power/SOC", fmt, ##args);	\
	}																\
} while(0)

#define Soc2printk(fmt, args...)                                       \
	do {																\
		if (mt_socfreq_debug_lv2) { 										\
			xlog_printk(ANDROID_LOG_INFO, "Power/SOC", fmt, ##args);	\
		}																\
} while(0)

#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend mt_cpufreq_early_suspend_handler =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 200,
    .suspend = NULL,
    .resume  = NULL,
};
#endif

#define DVFS_F0_1   (2002000)   // KHz
//#define DVFS_F0_2   (1599000)   // KHz
//#define DVFS_F0_3   (1495000)   // KHz
//#define DVFS_F0_4   (1391000)   // KHz
#define DVFS_F0     (1664000)   // KHz
#define DVFS_F1     (1495000)   // KHz
#define DVFS_F2     (1365000)   // KHz
#define DVFS_F3     (1248000)   // KHz
#define DVFS_F4     (1144000)   // KHz
#define DVFS_F5     ( 845000)   // KHz
#define DVFS_F6     ( 728000)   // KHz

#if defined(HQA_LV_1_09V)
    #define DVFS_V0     (1200)  // mV
    #define DVFS_V1     (1150)  // mV
    #define DVFS_V2     (1090)  // mV
    #define DVFS_V3     (1090)  // mV
#elif defined(HQA_NV_1_15V)
    #define DVFS_V0     (1260)  // mV
    #define DVFS_V1     (1200)  // mV
    #define DVFS_V2     (1150)  // mV
    #define DVFS_V3     (1050)  // mV /*Not used */
#elif defined(HQA_HV_1_21V)
    #define DVFS_V0     (1320)  // mV
    #define DVFS_V1     (1210)  // mV
    #define DVFS_V2     (1150)  // mV /*Not used */
    #define DVFS_V3     (1050)  // mV /*Not used */
#else /* Normal case */
    #define DVFS_V0_ptp0     (1150)  // mV
    #define DVFS_V1_1_ptp1   (1120)  // mV, for 1.365MHz
    #define DVFS_V1_0_ptp1   (1100)  // mV, for 1.66MHz in turbo mode
    #define DVFS_V1_ptp1     (1090)  // mV, for 1.66MHz in normal mode
    #define DVFS_V2_ptp2     (1060)  // mV
    #define DVFS_V3_0_ptp3   (1040)  // mV, for 1.365MHz
    #define DVFS_V3_ptp3     (1030)  // mV
    #define DVFS_V4_ptp4     (1000)  // mV
    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    #define DVFS_V5_ptp5     ( 950)  // mV
    #define DVFS_V6_ptp6     ( 900)  // mV
    #else
	#define DVFS_V4_ptp5	 (1000)  // mV
	#define DVFS_V4_ptp6	 (1000)  // mV
    #endif

    #define DVFS_VCORE_1p1     (1125)  // mV
    #define DVFS_VCORE_1p0     (1000)  // mV
    #define DVFS_VCORE_1p05    (1050)  // mV
    #define DVFS_VCORE_1p075    (1075)  // mV

    #define DVFS_SOC_freq_hp    (700)  // 
    #define DVFS_SOC_freq_lp    (500)  // 

	/* PMIC external buck */
	#define DVFS_V0_fan53555     (1150)  // mV
	#define DVFS_V1_1_fan53555	 (1120)  // mV, for 1.365MHz
	#define DVFS_V1_0_fan53555	 (1100)  // mV, for 1.66MHz in turbo mode
	#define DVFS_V1_fan53555	 (1090)  // mV, for 1.66MHz in normal mode
	#define DVFS_V2_fan53555	 (1060)  // mV
	#define DVFS_V3_0_fan53555	 (1040)  // mV, for 1.365MHz
	#define DVFS_V3_fan53555	 (1030)  // mV
	#define DVFS_V4_fan53555	 (1000)  // mV
	#define DVFS_V5_fan53555	 ( 950)  // mV
	#define DVFS_V6_fan53555	 ( 900)  // mV
#endif

/*********************
* GPU Frequency List
**********************/
#define GPU_DVFS_F0     (700000)   // KHz
#define GPU_DVFS_F0_1   (600000)   // KHz
#define GPU_DVFS_F1     (500000)   // KHz
#define GPU_DVFS_F2     (250000)   // KHz

/*********************
* MM Frequency List
**********************/
#define MM_DVFS_F0     (400000)   // KHz
#define MM_DVFS_F1     (295000)   // KHz

/*********************
* DDR Frequency List
**********************/
#ifdef CPUFREQ_MEMPLL_OD
#define DDR_DVFS_F0_1   (367000)   // KHz
#endif
#define DDR_DVFS_F0     (333000)   // KHz
#define DDR_DVFS_F1     (266000)   // KHz

/*********************
* Turbo mode
**********************/
#ifdef CPUFREQ_HIGHEST_TURBO_MODE
#define TURBO_MODE_LV_0     (0)   // KHz
#define TURBO_MODE_LV_1     (26000)   // KHz
#define TURBO_MODE_LV_2     (52000)   // KHz
#define TURBO_MODE_LV_3     (78000)   // KHz
#define TURBO_MODE_LV_4     (104000)   // KHz
#define TURBO_MODE_LV_5     (130000)   // KHz
#define TURBO_MODE_LV_6     (156000)   // KHz
#define TURBO_MODE_LV_7     (182000)   // KHz

#define CPUFREQ_TURBO_MODE_CPU_NUMBER 5
#endif

/*****************************************
* PMIC settle time, should not be changed
******************************************/
#define PMIC_SETTLE_TIME (40) // us

/*****************************************
* PLL settle time, should not be changed
******************************************/
#define PLL_SETTLE_TIME (30) // us

/***********************************************
* RMAP DOWN TIMES to postpone frequency degrade
************************************************/
#define RAMP_DOWN_TIMES (2)

/**********************************
* Available Clock Source for CPU
***********************************/
#define TOP_CKMUXSEL_CLKSQ   0x0
#define TOP_CKMUXSEL_ARMPLL  0x1
#define TOP_CKMUXSEL_MAINPLL 0x2
#define TOP_CKMUXSEL_UNIVPLL 0x3


#define MAX_SPM_PMIC_TBL     (9)
#define SOC_DVFS_IGNORE_REQUEST   (0xFF)

/**************************************************
* enable DVFS function
***************************************************/
static int g_dvfs_disable_count = 0;
static int g_soc_dvfs_disable_count = 0;

static unsigned int g_cur_freq;
//static unsigned int g_cur_cpufreq_volt;
static unsigned int g_limited_max_ncpu;
static unsigned int g_limited_max_freq;
static unsigned int g_limited_min_freq;
static unsigned int g_limited_freq_by_hevc = 0;
static unsigned int g_cpufreq_get_ptp_level = 0;
static unsigned int g_max_freq_by_ptp = DVFS_F0; /* default 1.66GHz */
#if defined(CONFIG_THERMAL_LIMIT_TEST)
static unsigned int g_limited_load_for_cpu_power_test = 0;
static unsigned int g_limited_max_cpu_power;
#endif
static unsigned int g_cpu_power_table_num = 0;
static unsigned int g_cur_cpufreq_OPPidx;
static unsigned int g_cur_socfreq_OPPidx;
static unsigned int g_cur_socfreq_gpu_freq = GPU_DVFS_F1; /* pre-loader 500MHz */
static unsigned int g_cur_socfreq_mm_freq = MM_DVFS_F1; /* pre-loader 295MHz */
static unsigned int g_cur_socfreq_ddr_freq = DDR_DVFS_F1; /* default setting in mt_socfreq_ddr_detection() */
//static unsigned int g_cur_socfreq_default_gpu_freq = GPU_DVFS_F1; /* pre-loader 500MHz */
//static unsigned int g_cur_socfreq_default_mm_freq = MM_DVFS_F1; /* pre-loader 295MHz */
static unsigned int g_cur_socfreq_default_ddr_freq = DDR_DVFS_F1; /* default setting in mt_socfreq_ddr_detection() */
static unsigned int g_cur_socfreq_keep_default_state = 0;

#define MT_CPUFREQ_POWER_LIMITED_MAX_NUM 10

#define MT_CPUFREQ_THERMAL_LIMITED_INDEX        0
#define MT_CPUFREQ_LBAT_VOLT_LIMITED_INDEX      1
#define MT_CPUFREQ_LBAT_VOLUME_LIMITED_INDEX    2

static unsigned int mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_POWER_LIMITED_MAX_NUM] = {0};
static unsigned int mt_cpufreq_limited_gpu_power_array[MT_CPUFREQ_POWER_LIMITED_MAX_NUM] = {0};

static unsigned int mt_cpufreq_thermal_limited_cpu_power = 0;
static unsigned int mt_cpufreq_thermal_limited_gpu_power = 0;

static unsigned int mt_cpufreq_lbat_volt_limited_level = 0;
static unsigned int mt_cpufreq_lbat_volt_limited_cpu_power = 0;
static unsigned int mt_cpufreq_lbat_volt_limited_gpu_power = 0;

static unsigned int mt_cpufreq_lbat_volt_limited_cpu_power_0 = 0;
static unsigned int mt_cpufreq_lbat_volt_limited_cpu_power_1 = 1900;
static unsigned int mt_cpufreq_lbat_volt_limited_cpu_power_2 = 1049;

static unsigned int mt_cpufreq_lbat_volt_limited_gpu_power_0 = 0;
static unsigned int mt_cpufreq_lbat_volt_limited_gpu_power_1 = 776;
static unsigned int mt_cpufreq_lbat_volt_limited_gpu_power_2 = 776;

static unsigned int mt_cpufreq_lbat_volume_limited_cpu_power = 0;
static unsigned int mt_cpufreq_lbat_volume_limited_gpu_power = 0;

static unsigned int mt_cpufreq_lbat_volume_limited_cpu_power_0 = 0;
static unsigned int mt_cpufreq_lbat_volume_limited_cpu_power_1 = 1049; // TODO: this must modified whenever OPP table changes to fix 4x1.365

static unsigned int mt_cpufreq_lbat_volume_limited_gpu_power_0 = 0;
static unsigned int mt_cpufreq_lbat_volume_limited_gpu_power_1 = 776; //fix GPU 500Mhz

/* Enable/disable low battery percentage protect */
static unsigned int mt_cpufreq_lbat_volume_enable = 1;

/* Enable/disable battery voltage drop level protect */
static unsigned int mt_cpufreq_lbat_volt_drop_enable = 1;

/* Enable/disable downgrade freq for ptpod */
static unsigned int mt_cpufreq_downgrade_freq_for_ptpod_enable = 1;


static int g_ramp_down_count = 0;

static bool mt_cpufreq_debug = false;
static bool mt_socfreq_debug = false;
static bool mt_socfreq_debug_lv2 = false;
static bool mt_cpufreq_ready = false;

static bool mt_cpufreq_hotplug_notify_ready = false;

/*  TOBEDONE */
static bool mt_cpufreq_pause = false;
static bool mt_socfreq_pause = false;
static bool mt_socfreq_allowed_enable = false;
static bool mt_socfreq_ddr_allowed_enable = false;
static bool mt_cpufreq_ptpod_disable = false;
//static bool mt_cpufreq_ptpod_voltage_down = false;
//static bool mt_cpufreq_max_freq_overdrive = false;
static bool mt_cpufreq_limit_max_freq_early_suspend = false;
static bool mt_cpufreq_earlysuspend_allow_deepidle_control_vproc = false;
static bool mt_cpufreq_freq_table_allocated = false;
static bool mt_cpufreq_freq_set_initial = false;

/* pmic volt by PTP-OD */
static unsigned int mt_cpufreq_pmic_volt[8] = {0};
static unsigned int mt_cpufreq_spm_volt[8] = {0};

#ifdef MT_DVFS_PTPOD_TEST
static unsigned int mt_cpufreq_ptpod_test[8] = {0};
#endif

static unsigned int mt_num_possible_cpus;

static unsigned int g_cur_soc_volt = DVFS_VCORE_1p0;  // Default VORE 1.0V  
#ifdef CPUFREQ_MEMPLL_OD
static unsigned int g_default_soc_volt = 0x30;  // Default VORE 1.0V 
#endif
static unsigned int g_cur_vcore = 0;
static unsigned int g_soc_final_mask = 0;
static unsigned int g_soc_mmpll_final_mask = 0;
static unsigned int g_soc_vencpll_final_mask = 0;
static unsigned int g_soc_mempll_final_mask = 0;
static unsigned int g_soc_volt_enable_state = 0;
static unsigned int g_soc_gpu_enable_state = 0;
static unsigned int g_soc_mm_enable_state = 0;
static unsigned int g_soc_ddr_enable_state = 0;
static unsigned int g_soc_DRAM_Type;
static unsigned int g_soc_api_call_test = 0;

static unsigned int g_soc_fixed_volt_state = 0;
static unsigned int g_soc_fixed_volt_enable = 0;
static unsigned int g_soc_fixed_mmpll_state = 0;
static unsigned int g_soc_fixed_mmpll_enable = 0;
static unsigned int g_soc_fixed_vencpll_state = 0;
static unsigned int g_soc_fixed_vencpll_enable = 0;
static unsigned int g_soc_fixed_mempll_state = 0;
static unsigned int g_soc_fixed_mempll_enable = 0;

static unsigned int g_soc_avs_type = 0;
#ifdef CPUFREQ_MEMPLL_OD
static int g_ddr_frequency_type = 0;
#endif

static unsigned int mt6592_soc_dvfs_config[SOC_DVFS_TYPE_NUM] = {0};
static unsigned int mt6592_soc_mmpll_dvfs_config[SOC_DVFS_TYPE_NUM] = {0};
static unsigned int mt6592_soc_vencpll_dvfs_config[SOC_DVFS_TYPE_NUM] = {0};
static unsigned int mt6592_soc_mempll_dvfs_config[SOC_DVFS_TYPE_NUM] = {0};

#ifdef CPUFREQ_HIGHEST_TURBO_MODE
static unsigned int g_cur_freq_highest;
static unsigned int g_cpufreq_turbo_mode_efuse_on_off = 1; /* bit[10], 0: on, 1:off  */
static unsigned int g_cpufreq_turbo_mode_efuse_4_core = 0;
static unsigned int g_cpufreq_turbo_mode_efuse_2_core = 0;
static unsigned int g_cur_freq_target_online_cpu = 0;
static unsigned int g_prev_freq_target_online_cpu = 0;
static unsigned int g_mt_cpufreq_hotplug_notify_change = 0;
#endif


/* PMIC external buck */
static unsigned int pmic_external_buck_used = 0;
static DEFINE_MUTEX(mt_cpufreq_mutex);

static DEFINE_SPINLOCK(mt_cpufreq_lock);
#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
static DEFINE_MUTEX(mt_sochp_mutex);
#endif

static DEFINE_MUTEX(mt_cpufreq_cpu_power_mutex);
static DEFINE_MUTEX(mt_sochp_gpu_power_mutex);
#ifdef PTPOD_DOWNGRADE_FREQ
static DEFINE_MUTEX(mt_cpufreq_downgrade_freq_mutex);
#endif

/***************************
* Operate Point Definition
****************************/
#define OP(khz, volt)       \
{                           \
    .cpufreq_khz = khz,     \
    .cpufreq_volt = volt,   \
}

struct mt_cpu_freq_info
{
    unsigned int cpufreq_khz;
    unsigned int cpufreq_volt;
};

struct mt_dvfs_vcore_tbl_info
{
    unsigned int dvfs_khz;
    unsigned int dvfs_vcore;
    unsigned int tbl_idx;
};

struct mt_cpu_power_info
{
    unsigned int cpufreq_khz;
    unsigned int cpufreq_ncpu;
    unsigned int cpufreq_power;
};

struct mt_module_info {
	//char module_name[12];
	SOC_DVFS_TYPE_ENUM type;
	unsigned int sochp_enable;
	struct list_head link;
};

/* PMIC external buck */
#define OPEXT(vproc, ext_vproc)       \
{                           \
    .cpufreq_vproc = vproc,     \
    .cpufreq_ext_vproc = ext_vproc,   \
}

struct mt_cpu_ext_buck_vproc_info
{
    unsigned int cpufreq_vproc;
    unsigned int cpufreq_ext_vproc;
};

/***************************
* MT6592 E1 DVFS Table
****************************/
#if defined(HQA_LV_1_09V)
static struct mt_cpu_freq_info mt6592_freqs_e1[] = {
    OP(DVFS_F0, DVFS_V0),
    OP(DVFS_F1, DVFS_V0),
    OP(DVFS_F2, DVFS_V1),
    OP(DVFS_F3, DVFS_V1),
    OP(DVFS_F4, DVFS_V2),
};
#elif defined(HQA_NV_1_15V)
static struct mt_cpu_freq_info mt6592_freqs_e1[] = {
    OP(DVFS_F0, DVFS_V0),
    OP(DVFS_F1, DVFS_V1),
    OP(DVFS_F2, DVFS_V2),
    OP(DVFS_F3, DVFS_V2),
    OP(DVFS_F4, DVFS_V2),
};
#elif defined(HQA_HV_1_21V)
static struct mt_cpu_freq_info mt6592_freqs_e1[] = {
    OP(DVFS_F0, DVFS_V0),
    OP(DVFS_F1, DVFS_V1),
    OP(DVFS_F2, DVFS_V1),
    OP(DVFS_F3, DVFS_V1),
    OP(DVFS_F4, DVFS_V1),
};
#else /* Normal case */

/* PMIC MT6323, VPROC */
static struct mt_cpu_freq_info mt6592_freqs_e1[] = {
    OP(DVFS_F0, DVFS_V0_ptp0),
    OP(DVFS_F1, DVFS_V1_ptp1),
    OP(DVFS_F2, DVFS_V2_ptp2),
    OP(DVFS_F3, DVFS_V3_ptp3),
    OP(DVFS_F4, DVFS_V4_ptp4),
    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    OP(DVFS_F5, DVFS_V5_ptp5),
    OP(DVFS_F6, DVFS_V6_ptp6),
    #else
    OP(DVFS_F5, DVFS_V4_ptp5),
    OP(DVFS_F6, DVFS_V4_ptp6),
    #endif
};

/* PMIC external buck (fan53555), VPROC */
static struct mt_cpu_freq_info mt6592_freqs_fan53555_e1[] = {
    OP(DVFS_F0, DVFS_V0_fan53555),
    OP(DVFS_F1, DVFS_V1_fan53555),
    OP(DVFS_F2, DVFS_V2_fan53555),
    OP(DVFS_F3, DVFS_V3_fan53555),
    OP(DVFS_F4, DVFS_V4_fan53555),
    OP(DVFS_F5, DVFS_V5_fan53555),
    OP(DVFS_F6, DVFS_V6_fan53555),
};

#endif

/* PMIC MT6323, VPROC */
static struct mt_cpu_freq_info mt6592_freqs_e1_1[] = {
    OP(DVFS_F0_1, DVFS_V0_ptp0),
    OP(DVFS_F0, DVFS_V1_0_ptp1),
    OP(DVFS_F1, DVFS_V1_ptp1),
    OP(DVFS_F2, DVFS_V2_ptp2),
    OP(DVFS_F4, DVFS_V4_ptp4),
	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    OP(DVFS_F5, DVFS_V5_ptp5),
    OP(DVFS_F6, DVFS_V6_ptp6),
	#else
    OP(DVFS_F5, DVFS_V4_ptp5),
    OP(DVFS_F6, DVFS_V4_ptp6),
	#endif
};

/* PMIC external buck (fan53555), VPROC */
static struct mt_cpu_freq_info mt6592_freqs_fan53555_e1_1[] = {
    OP(DVFS_F0_1, DVFS_V0_fan53555),
    OP(DVFS_F0, DVFS_V1_0_fan53555),
    OP(DVFS_F1, DVFS_V1_fan53555),
    OP(DVFS_F2, DVFS_V2_fan53555),
    OP(DVFS_F4, DVFS_V4_fan53555),
    OP(DVFS_F5, DVFS_V5_fan53555),
    OP(DVFS_F6, DVFS_V6_fan53555),
};

/* PMIC MT6323, VPROC */
static struct mt_cpu_freq_info mt6592_freqs_e1_2[] = {
    OP(DVFS_F2, DVFS_V0_ptp0),
    OP(DVFS_F3, DVFS_V1_1_ptp1),
    OP(DVFS_F4, DVFS_V1_ptp1),
    OP(DVFS_F5, DVFS_V3_0_ptp3),
    OP(DVFS_F6, DVFS_V4_ptp4),
};

/* PMIC external buck (fan53555), VPROC */
static struct mt_cpu_freq_info mt6592_freqs_fan53555_e1_2[] = {
    OP(DVFS_F2, DVFS_V0_fan53555),
    OP(DVFS_F3, DVFS_V1_1_fan53555),
    OP(DVFS_F4, DVFS_V1_fan53555),
    OP(DVFS_F5, DVFS_V3_0_fan53555),
    OP(DVFS_F6, DVFS_V4_fan53555),
};


/* PMIC external buck (fan53555) */
static struct mt_cpu_ext_buck_vproc_info mt6592_fan53555_vproc_map[] = {
    OPEXT(90000, 90000),
    OPEXT(90625, 91000),
    OPEXT(91250, 92000),
    OPEXT(91875, 92000),
    OPEXT(92500, 93000),
    OPEXT(93125, 94000),
    OPEXT(93750, 94000),
    OPEXT(94375, 95000),
    OPEXT(95000, 95000),
    OPEXT(95625, 96000),
    
    OPEXT(96250, 97000),
    OPEXT(96875, 97000),
    OPEXT(97500, 98000),
    OPEXT(98125, 99000),
	OPEXT(98750, 99000),
	OPEXT(99375, 100000),
	OPEXT(100000, 100000),
	OPEXT(100625, 101000),
	OPEXT(101250, 102000),
	OPEXT(101875, 102000),

    OPEXT(102500, 103000),
    OPEXT(103125, 104000),
    OPEXT(103750, 104000),
    OPEXT(104375, 105000),
	OPEXT(105000, 105000),
	OPEXT(105625, 106000),
	OPEXT(106250, 107000),
	OPEXT(106875, 107000),
	OPEXT(107500, 108000),
	OPEXT(108125, 109000),

    OPEXT(108750, 109000),
    OPEXT(109375, 110000),
    OPEXT(110000, 110000),
    OPEXT(110625, 111000),
	OPEXT(111250, 112000),
	OPEXT(111875, 112000),
	OPEXT(112500, 113000),
	OPEXT(113125, 114000),
	OPEXT(113750, 114000),
	OPEXT(114375, 115000),

    OPEXT(115000, 115000),
    OPEXT(115625, 116000),
    OPEXT(116250, 117000),
    OPEXT(116875, 117000),
	OPEXT(117500, 118000),
	OPEXT(118125, 119000),
	OPEXT(118750, 119000),
	OPEXT(119375, 120000),
	OPEXT(120000, 120000),
	OPEXT(120625, 121000),

    OPEXT(121250, 122000),
    OPEXT(121875, 122000),
    OPEXT(122500, 123000),
    OPEXT(123125, 124000),
	OPEXT(123750, 124000),
	OPEXT(124375, 125000),
	OPEXT(125000, 125000),
	OPEXT(125625, 126000),
	OPEXT(126250, 127000),
	OPEXT(126875, 127000),

    OPEXT(127500, 128000),
    OPEXT(128125, 129000),
    OPEXT(128750, 129000),
    OPEXT(129375, 130000),
	OPEXT(130000, 130000),
};


static unsigned int mt_cpu_freqs_num;
static struct mt_cpu_freq_info *mt_cpu_freqs = NULL;
static struct cpufreq_frequency_table *mt_cpu_freqs_table;
static struct mt_cpu_power_info *mt_cpu_power = NULL;
//static struct mt_module_info info_ori;
#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
#else
static struct mt_dvfs_vcore_tbl_info dvfs_vcore_config[MAX_SPM_PMIC_TBL];
#endif

/*********************
* GPU power info structure
**********************/
struct mt_gpufreq_info
{
	unsigned int gpufreq_khz;
	unsigned int gpufreq_volt;
};

struct mt_gpufreq_power_info
{
    unsigned int gpufreq_khz;
    unsigned int gpufreq_power;
};

static unsigned int g_gpu_limited_max_id = 0;
static unsigned int g_gpu_limited_previous = 0;
static unsigned int mt_gpufreqs_num = 0;
static struct mt_gpufreq_info *mt_gpufreqs = NULL;
static struct mt_gpufreq_power_info *mt_gpufreqs_power = NULL;

/***************************
* MT6592 GPU Power Table
****************************/
static struct mt_gpufreq_power_info mt_gpufreqs_golden_power[] = {
    {.gpufreq_khz = GPU_DVFS_F0, .gpufreq_power = 1222},
	{.gpufreq_khz = GPU_DVFS_F0_1, .gpufreq_power = 1070},
    {.gpufreq_khz = GPU_DVFS_F1, .gpufreq_power = 776},
    {.gpufreq_khz = GPU_DVFS_F2, .gpufreq_power = 411},
};

/******************************
* Internal Function Declaration
*******************************/
static int mt_cpufreq_volt_set(unsigned int newOPPidx);
#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
#else
static unsigned int mt_get_max_vcore(unsigned int cpu_opp_idx, unsigned int soc_opp_idx);
#endif


/******************************
* Extern Function Declaration
*******************************/
extern int spm_dvfs_ctrl_volt(u32 value);
extern int mtk_cpufreq_register(struct mt_cpu_power_info *freqs, int num);
extern int mtk_gpufreq_register(struct mt_gpufreq_power_info *freqs, int num);
#ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
extern void hp_limited_cpu_num(int num);
#endif
extern u32 PTP_get_ptp_level(void);

extern unsigned int mt_get_cpu_freq(void);

extern unsigned int mt_get_mfgclk_freq(void);
extern unsigned int mt_get_mmclk_freq(void);
extern unsigned int mt_get_emi_freq(void);

extern int mt_dfs_mmpll(unsigned int target_freq);
extern int mt_dfs_vencpll(unsigned int target_freq);

extern int DFS_Detection(void);

extern unsigned int get_devinfo_with_index(unsigned int index);

/* PMIC External Buck */
extern void ext_buck_init(void); // note : done once at pmic_mt_probe() for set GPIO48&49 to GPIO mode and output low
extern int ext_buck_vosel(unsigned long val); // val=mV
extern int is_ext_buck_sw_ready(void);
extern int is_ext_buck_exist(void);
extern int is_fan53555_exist(void);
extern int is_ncp6335_exist(void);

#ifdef PTPOD_DOWNGRADE_FREQ
extern void (*cpufreq_freq_check)(void);
extern int ptp_status(void);
extern int mtktscpu_get_Tj_temp(void);
#endif

/***********************************************
* MT6592 E1 Raw Data: 1.3Ghz @ 1.15V @ TT 125C
************************************************/
#define P_MCU_L         (1243)   // MCU Leakage Power
#define P_MCU_T         (2900)  // MCU Total Power
#define P_CA7_L         (110)    // CA7 Leakage Power
#define P_CA7_T         (305)   // Single CA7 Core Power

#define P_MCL99_105C_L  (1243)   // MCL99 Leakage Power @ 105C
#define P_MCL99_25C_L   (93)    // MCL99 Leakage Power @ 25C
#define P_MCL50_105C_L  (587)   // MCL50 Leakage Power @ 105C
#define P_MCL50_25C_L   (35)    // MCL50 Leakage Power @ 25C

#define T_105           (105)   // Temperature 105C
#define T_65            (65)    // Temperature 65C
#define T_25            (25)    // Temperature 25C

#define P_MCU_D ((P_MCU_T - P_MCU_L) - 8 * (P_CA7_T - P_CA7_L)) // MCU dynamic power except of CA7 cores

#define P_TOTAL_CORE_L ((P_MCL99_105C_L  * 27049) / 100000) // Total leakage at T_65
#define P_EACH_CORE_L  ((P_TOTAL_CORE_L * ((P_CA7_L * 1000) / P_MCU_L)) / 1000) // 1 core leakage at T_65

#define P_CA7_D_1_CORE ((P_CA7_T - P_CA7_L) * 1) // CA7 dynamic power for 1 cores turned on
#define P_CA7_D_2_CORE ((P_CA7_T - P_CA7_L) * 2) // CA7 dynamic power for 2 cores turned on
#define P_CA7_D_3_CORE ((P_CA7_T - P_CA7_L) * 3) // CA7 dynamic power for 3 cores turned on
#define P_CA7_D_4_CORE ((P_CA7_T - P_CA7_L) * 4) // CA7 dynamic power for 4 cores turned on
#define P_CA7_D_5_CORE ((P_CA7_T - P_CA7_L) * 5) // CA7 dynamic power for 5 cores turned on
#define P_CA7_D_6_CORE ((P_CA7_T - P_CA7_L) * 6) // CA7 dynamic power for 6 cores turned on
#define P_CA7_D_7_CORE ((P_CA7_T - P_CA7_L) * 7) // CA7 dynamic power for 7 cores turned on
#define P_CA7_D_8_CORE ((P_CA7_T - P_CA7_L) * 8) // CA7 dynamic power for 8 cores turned on

#define A_1_CORE (P_MCU_D + P_CA7_D_1_CORE) // MCU dynamic power for 1 cores turned on
#define A_2_CORE (P_MCU_D + P_CA7_D_2_CORE) // MCU dynamic power for 2 cores turned on
#define A_3_CORE (P_MCU_D + P_CA7_D_3_CORE) // MCU dynamic power for 3 cores turned on
#define A_4_CORE (P_MCU_D + P_CA7_D_4_CORE) // MCU dynamic power for 4 cores turned on
#define A_5_CORE (P_MCU_D + P_CA7_D_5_CORE) // MCU dynamic power for 5 cores turned on
#define A_6_CORE (P_MCU_D + P_CA7_D_6_CORE) // MCU dynamic power for 6 cores turned on
#define A_7_CORE (P_MCU_D + P_CA7_D_7_CORE) // MCU dynamic power for 7 cores turned on
#define A_8_CORE (P_MCU_D + P_CA7_D_8_CORE) // MCU dynamic power for 8 cores turned on


/*************************************************************************************
* Check SOC Efuse
**************************************************************************************/
static unsigned int mt_soc_check_efuse(void)
{
    unsigned int soc_avs = 0, ret = 0;

    soc_avs = get_devinfo_with_index(15) & 0x1;

    if (soc_avs == 0x1)
    {
        ret = 1; // 1.075V
    }
    else
    {
        ret = 0; // 1.125V
    }

    return ret;
}

#ifdef CPUFREQ_MEMPLL_OD
/*************************************************************************************
* Check default frequency and voltage
**************************************************************************************/
static unsigned int mt_socfreq_default_freq_volt_check(void)
{
    unsigned int ret = 0, default_volt = 0, default_freq = 0;

	/* Read default frequency and voltage */
	#if 1
	default_volt = get_DRAM_default_voltage();
	default_freq = get_DRAM_default_freq();
	#endif
	
	/* Check default voltage */
    switch (default_volt)
    {
        case 0:
	    	g_cur_soc_volt = DVFS_VCORE_1p1;
			g_default_soc_volt = 0x44;
			g_cur_vcore = DVFS_VCORE_1p1;
	        g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;			
            break;
        case 1:
	    	g_cur_soc_volt = DVFS_VCORE_1p05;
			g_default_soc_volt = 0x38;
			g_cur_vcore = DVFS_VCORE_1p05;
	        g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;			
            break;
        case 2:
	    	g_cur_soc_volt = DVFS_VCORE_1p0;
			g_default_soc_volt = 0x30;
			g_cur_vcore = DVFS_VCORE_1p0;
	        g_cur_socfreq_OPPidx = 8;
			g_soc_volt_enable_state = 0;			
            break;
		case 3:
			g_cur_soc_volt = DVFS_VCORE_1p075;
			g_default_soc_volt = 0x3C;
			g_cur_vcore = DVFS_VCORE_1p075;
			g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;			
			break;
        default:
	    	g_cur_soc_volt = DVFS_VCORE_1p1;
			g_default_soc_volt = 0x44;
			g_cur_vcore = DVFS_VCORE_1p1;
	        g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;			
            break;
    }

	xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "default_volt = %d\n", default_volt);

	/* Check default frequency */
    switch (default_freq)
    {
        case 0:
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F0_1;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F0_1;
			g_soc_ddr_enable_state = 1;			
            break;
        case 1:
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F0;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
			g_soc_ddr_enable_state = 1;			
            break;
        case 2:
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F1;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F1;
			g_soc_ddr_enable_state = 0;		
            break;
        default:
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F0_1;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F0_1;
			g_soc_ddr_enable_state = 1;			
            break;
    }

	xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "default_freq = %d\n", default_freq);
	
    return ret;
}
#endif

/************************************************
* SOC return to default setting
*************************************************/
static void mt_socfreq_return_default(unsigned int state)
{
	int i=0;
	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	#else
	unsigned int new_soc_idx, new_vcore_idx;
	#endif
	
	if(state == 0)
	{
		g_cur_socfreq_keep_default_state = 0;
		Socprintk("mt_socfreq_return_default: cancel default \n");
	}
	else
	{
		g_cur_socfreq_keep_default_state = 1;
		Socprintk("mt_socfreq_return_default: return default \n");

		#ifdef CPUFREQ_MEMPLL_OD
		if((g_cur_socfreq_default_ddr_freq == DDR_DVFS_F0)||(g_cur_socfreq_default_ddr_freq == DDR_DVFS_F0_1))
		#else
		if(g_cur_socfreq_default_ddr_freq == DDR_DVFS_F0)
		#endif
		{
			/* Set VCORE up */
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			g_soc_volt_enable_state = 1;	
                        
			#ifdef CPUFREQ_MEMPLL_OD
			DFS_phase_mt6333_config_interface(g_default_soc_volt); /* DVFS_VCORE_1p1 = 1.125V */
			#else
			if(g_soc_avs_type == 0)
			{
				mt6333_set_vcore_vosel_on(0x44); /* DVFS_VCORE_1p1 = 1.125V */
			}
			else
			{
				mt6333_set_vcore_vosel_on(0x3C); /* DVFS_VCORE_1p1 = 1.075V */
			}
			#endif
			/* delay 40us + volt step * 0.5 us = 40 + (125/6.25)*0.5 = 50us */
			udelay(50);
			g_cur_soc_volt = DVFS_VCORE_1p1;
			g_cur_vcore = DVFS_VCORE_1p1;
			g_cur_socfreq_OPPidx = 7;
				
			Socprintk("mt_socfreq_return_default:Set VCORE up\n");	
			
			#else

			g_soc_volt_enable_state = 1;
				
			new_soc_idx = 7;

			if(is_ext_buck_exist() == 0)
			{
				new_vcore_idx = mt_get_max_vcore(g_cur_cpufreq_OPPidx, new_soc_idx);
			}
			else
			{
				new_vcore_idx = 7;
			}
					
			Socprintk("mt_socfreq_return_default: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);
				
			/* Scaling voltage and frequency up */
			mt_cpufreq_volt_set(new_vcore_idx);
			udelay(PMIC_SETTLE_TIME);
			
			g_cur_soc_volt = DVFS_VCORE_1p1;
			g_cur_vcore = dvfs_vcore_config[new_vcore_idx].dvfs_vcore;
			g_cur_socfreq_OPPidx = new_soc_idx;

			#endif
			
			/* Set MMPLL 500MHz */
			mt_dfs_mmpll(1796000); /* MMPLL 500MHz*/
			g_cur_socfreq_gpu_freq = GPU_DVFS_F1;
			g_soc_gpu_enable_state = 0;

			Socprintk("mt_socfreq_return_default: Set MMPLL freq down\n");

			/* Set VENCPLL 295MHz */
			mt_dfs_vencpll(1183000); /* VENCPLL 295MHz*/
			g_cur_socfreq_mm_freq = MM_DVFS_F1;		
			g_soc_mm_enable_state = 0;
			
			Socprintk("mt_soc_mt_socfreq_return_defaultdvfs: Set VENCPLL freq down\n");

			#ifdef CPUFREQ_MEMPLL_OD
			/* Set MEMPLL */
			if((g_ddr_frequency_type == 1)||(g_ddr_frequency_type == 2))
			{
				mt_fh_dram_overclock(367); /* MEMPLL 367MHz*/
				g_cur_socfreq_ddr_freq = DDR_DVFS_F0_1;
			}
			else
			{
				mt_fh_dram_overclock(333); /* MEMPLL 333MHz*/
				g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
			}
			#else
			/* Set MEMPLL 333MHz */
			mt_fh_dram_overclock(333); /* MEMPLL 333MHz*/		
			g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
			#endif

			g_soc_ddr_enable_state = 1;
			
			Socprintk("mt_socfreq_return_default: Set MEMPLL freq up\n");
		}
		else if(g_cur_socfreq_default_ddr_freq == DDR_DVFS_F1)
		{
			/* Set MMPLL 500MHz */
			mt_dfs_mmpll(1796000); /* MMPLL 500MHz*/
			g_cur_socfreq_gpu_freq = GPU_DVFS_F1;
			g_soc_gpu_enable_state = 0;

			Socprintk("mt_socfreq_return_default: Set MMPLL freq down\n");

			/* Set VENCPLL 295MHz */
			mt_dfs_vencpll(1183000); /* VENCPLL 295MHz*/
			g_cur_socfreq_mm_freq = MM_DVFS_F1;		
			g_soc_mm_enable_state = 0;
			
			Socprintk("mt_socfreq_return_default: Set VENCPLL freq down\n");

			/* Set MEMPLL 266MHz */
			mt_fh_dram_overclock(266); /* MEMPLL 266MHz*/
			g_cur_socfreq_ddr_freq = DDR_DVFS_F1;
			g_soc_ddr_enable_state = 0;
			Socprintk("mt_socfreq_return_default: Set MEMPLL freq down\n");

			/* Set VCORE down */
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			g_soc_volt_enable_state = 0;

			#ifdef CPUFREQ_MEMPLL_OD	
			DFS_phase_mt6333_config_interface(g_default_soc_volt); /* DVFS_VCORE_1p0 = 1.00V */
			#else
			mt6333_set_vcore_vosel_on(0x30); /* DVFS_VCORE_1p0 = 1.00V */
			#endif
			g_cur_soc_volt = DVFS_VCORE_1p0;
			g_cur_vcore = DVFS_VCORE_1p0;
	        g_cur_socfreq_OPPidx = 8;
			
			Socprintk("mt_socfreq_return_default: Set VCORE down\n");	

			#else

			g_soc_volt_enable_state = 0;
				
			new_soc_idx = 8;

			if(is_ext_buck_exist() == 0)
			{
				new_vcore_idx = mt_get_max_vcore(g_cur_cpufreq_OPPidx, new_soc_idx);
			}
			else
			{
				new_vcore_idx = 8;
			}
				
			Socprintk("mt_socfreq_return_default: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);
			
			mt_cpufreq_volt_set(new_vcore_idx);
			
			g_cur_soc_volt = DVFS_VCORE_1p0;
			g_cur_vcore = dvfs_vcore_config[new_vcore_idx].dvfs_vcore;
			g_cur_socfreq_OPPidx = new_soc_idx;

			#endif
		}

		for (i = 0; i < SOC_DVFS_TYPE_NUM; i++)
        {
        	mt6592_soc_dvfs_config[i] = 0;
			mt6592_soc_mmpll_dvfs_config[i] = 0;
			mt6592_soc_vencpll_dvfs_config[i] = 0;
            Socprintk("mt_socfreq_return_default: reset mt6592_soc_dvfs_config[%d] = %d\n", i, mt6592_soc_dvfs_config[i]);
			
		}
		g_soc_final_mask = 0;
		g_soc_mmpll_final_mask = 0;
		g_soc_vencpll_final_mask = 0;
	}
	
}

/************************************************
* SOC fixed frew or volt check for test
*************************************************/
static unsigned int mt_soc_volt_fixed_check(unsigned int enable)
{
	if(g_soc_fixed_volt_state == 1)
	{
		if(g_soc_fixed_volt_enable == 1)
		{
			enable = 1;
		}
		else
		{
			enable = 0;
		}

		Socprintk("mt_soc_volt_fixed_check 1: ask enable = %d\n", enable);
	}

	if(((g_soc_fixed_mmpll_state == 1) && (g_soc_fixed_mmpll_enable == 1))
		|| ((g_soc_fixed_vencpll_state == 1) && (g_soc_fixed_vencpll_enable == 1))
		|| ((g_soc_fixed_mempll_state == 1) && (g_soc_fixed_mempll_enable == 1)))
	{
		enable = 1;

		Socprintk("mt_soc_volt_fixed_check 2: ask enable = %d\n", enable);
	}
	
	return enable;
}

static unsigned int mt_soc_mmpll_fixed_check(unsigned int mmpll_enable)
{
	if(g_soc_fixed_mmpll_state == 1)
	{
		if(g_soc_fixed_mmpll_enable == 1)
		{
			mmpll_enable = 1;
		}
		else
		{
			mmpll_enable = 0;
		}

		Socprintk("mt_soc_mmpll_fixed_check: ask mmpll_enable = %d\n", mmpll_enable);
	}

	return mmpll_enable;
}

static unsigned int mt_soc_vencpll_fixed_check(unsigned int vencpll_enable)
{
	if(g_soc_fixed_vencpll_state == 1)
	{
		if(g_soc_fixed_vencpll_enable == 1)
		{
			vencpll_enable = 1;
		}
		else
		{
			vencpll_enable = 0;
		}

		Socprintk("mt_soc_vencpll_fixed_check: ask vencpll_enable = %d\n", vencpll_enable);
	}

	return vencpll_enable;
}

static unsigned int mt_soc_mempll_fixed_check(unsigned int mempll_enable)
{
	if(g_soc_fixed_mempll_state == 1)
	{
		if(g_soc_fixed_mempll_enable == 1)
		{
			mempll_enable = 1;
		}
		else
		{
			mempll_enable = 0;
		}

		Socprintk("mt_soc_mempll_fixed_check: ask mempll_enable = %d\n", mempll_enable);
	}
	
	return mempll_enable;
}

/************************************************
* GPU frequency adjust interface for thermal protect
*************************************************/
unsigned int mt_socfreq_get_cur_gpufreq(void)
{
	return g_cur_socfreq_gpu_freq;
}
EXPORT_SYMBOL(mt_socfreq_get_cur_gpufreq);


/************************************************
* DDR DFS detection
*************************************************/
static void mt_socfreq_ddr_detection(void)
{
	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	#else
	unsigned int new_soc_idx, new_vcore_idx;
	#endif

    /*  TOBEDONE */
	mt_num_possible_cpus = num_possible_cpus();
	g_soc_DRAM_Type = get_ddr_type();

	#ifdef CPUFREQ_MEMPLL_OD
	g_ddr_frequency_type = DFS_Detection();

	mt_socfreq_default_freq_volt_check();
	#endif

	/* Check if support DDR DFS, only LPDDR3 may support DFS. */
	#ifdef CPUFREQ_MEMPLL_OD
	if(g_ddr_frequency_type < 0)
	#else
	if(DFS_Detection() < 0)
	#endif
    {
    	
		mt_socfreq_ddr_allowed_enable = false; 	/* === DDR can not DFS ===*/

		if(g_soc_DRAM_Type == LPDDR3) /* 3 */
		{
			mt_socfreq_allowed_enable = false;	/* === SOC can not DFS ===*/
			
			/* 1333M / 1.125V set in pre-loader */

			/* LPDDR3 without DFS/ DDR3, initial set GPU/MM freq high */
			/* set MMPLL freq */
			if(mt_num_possible_cpus == 8)
			{
				mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
				g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
			}
			else if(mt_num_possible_cpus == 4)
			{
				mt_dfs_mmpll(2405000); /* MMPLL 601.25MHz*/
				g_cur_socfreq_gpu_freq = GPU_DVFS_F0_1;
			}
			else
			{
				mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
				g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
			}
			g_soc_gpu_enable_state = 1;

			/* Set VENCPLL freq */
			mt_dfs_vencpll(1599000); /* VENCPLL 400MHz*/
			g_cur_socfreq_mm_freq = MM_DVFS_F0;
			g_soc_mm_enable_state = 1;

			#ifdef CPUFREQ_MEMPLL_OD
			#else
			/* MEMPLL freq */
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F0;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
			g_soc_ddr_enable_state = 1;
			
			/* VCORE voltage */
			if(g_soc_avs_type == 0)
			{
	    		g_cur_soc_volt = DVFS_VCORE_1p1;
				g_cur_vcore = DVFS_VCORE_1p1;
	        	g_cur_socfreq_OPPidx = 7;
				g_soc_volt_enable_state = 1;
			}
			else
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				#ifdef CPUFREQ_MEMPLL_OD	
				DFS_phase_mt6333_config_interface(0x3C); /* DVFS_VCORE_1p0 = 1.075V */
				#else
				mt6333_set_vcore_vosel_on(0x3C); /* DVFS_VCORE_1p0 = 1.075V */
				#endif
				g_cur_soc_volt = DVFS_VCORE_1p075;
				g_cur_vcore = DVFS_VCORE_1p075;
				g_cur_socfreq_OPPidx = 7;
				g_soc_volt_enable_state = 1;
				xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_socfreq_ddr_detection: By soc-avs, VCORE highest 1.075V\n");
				#else

				new_soc_idx = 7;
				new_vcore_idx = new_soc_idx; // Set VCORE to 1.075V as default
						
				Socprintk("mt_socfreq_ddr_detection: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);
					
				/* Scaling voltage and frequency up */
				mt_cpufreq_volt_set(new_vcore_idx);
				udelay(PMIC_SETTLE_TIME);
				
	    		g_cur_soc_volt = DVFS_VCORE_1p075;
				g_cur_vcore = DVFS_VCORE_1p075;
	        	g_cur_socfreq_OPPidx = 7;
				g_soc_volt_enable_state = 1;

				xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_socfreq_ddr_detection: By soc-avs, VCORE highest 1.075V\n");
				#endif
			}
			#endif
		}
		else if (g_soc_DRAM_Type == LPDDR2) /* 0 */
		{
			mt_socfreq_allowed_enable = true;	/* === SOC can DFS === */
			
			/* 1066M / 1.0V set in pre-loader */

			#ifdef CPUFREQ_MEMPLL_OD
			#else
			/* MEMPLL freq */
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F1;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F1;
			g_soc_ddr_enable_state = 0;
			
			/* VCORE voltage */
    		g_cur_soc_volt = DVFS_VCORE_1p0;
			g_cur_vcore = DVFS_VCORE_1p0;
        	g_cur_socfreq_OPPidx = 8;	
			g_soc_volt_enable_state = 0;
			#endif
		}
		else /* 1, 2 */
		{
			mt_socfreq_allowed_enable = false;	/* === SOC can not DFS ===*/
			
			/* 1333M / 1.125V set in pre-loader */

			/* LPDDR3 without DFS/ DDR3, initial set GPU/MM freq high */
			/* set MMPLL freq */
			if(mt_num_possible_cpus == 8)
			{
				mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
				g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
			}
			else if(mt_num_possible_cpus == 4)
			{
				mt_dfs_mmpll(2405000); /* MMPLL 601.25MHz*/
				g_cur_socfreq_gpu_freq = GPU_DVFS_F0_1;
			}
			else
			{
				mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
				g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
			}
			g_soc_gpu_enable_state = 1;

			/* Set VENCPLL freq */
			mt_dfs_vencpll(1599000); /* VENCPLL 400MHz*/
			g_cur_socfreq_mm_freq = MM_DVFS_F0;
			g_soc_mm_enable_state = 1;

			#ifdef CPUFREQ_MEMPLL_OD
			#else
			/* MEMPLL freq */
			g_cur_socfreq_default_ddr_freq = DDR_DVFS_F0;
			g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
			g_soc_ddr_enable_state = 1;
			
			/* VCORE voltage */
			if(g_soc_avs_type == 0)
			{
	    		g_cur_soc_volt = DVFS_VCORE_1p1;
				g_cur_vcore = DVFS_VCORE_1p1;
	        	g_cur_socfreq_OPPidx = 7;
				g_soc_volt_enable_state = 1;
			}
			else
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				
				#ifdef CPUFREQ_MEMPLL_OD
				DFS_phase_mt6333_config_interface(0x3C); /* DVFS_VCORE_1p0 = 1.075V */
				#else
				mt6333_set_vcore_vosel_on(0x3C); /* DVFS_VCORE_1p0 = 1.075V */
				#endif
				g_cur_soc_volt = DVFS_VCORE_1p075;
				g_cur_vcore = DVFS_VCORE_1p075;
				g_cur_socfreq_OPPidx = 7;
				g_soc_volt_enable_state = 1;
				xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_socfreq_ddr_detection: By soc-avs, VCORE highest 1.075V\n");

				#else
				
				new_soc_idx = 7;
				new_vcore_idx = new_soc_idx; // Set VCORE to 1.075V as default
						
				Socprintk("mt_socfreq_ddr_detection: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);
					
				/* Scaling voltage and frequency up */
				mt_cpufreq_volt_set(new_vcore_idx);
				udelay(PMIC_SETTLE_TIME);
				
	    		g_cur_soc_volt = DVFS_VCORE_1p075;
				g_cur_vcore = DVFS_VCORE_1p075;
	        	g_cur_socfreq_OPPidx = 7;
				g_soc_volt_enable_state = 1;

				xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_socfreq_ddr_detection: By soc-avs, VCORE highest 1.075V\n");

				#endif
			}
			#endif
		}
        
		xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "DFS_Detection() < 0, g_soc_DRAM_Type = %d\n", g_soc_DRAM_Type);
	}
	else
	{	
		mt_socfreq_ddr_allowed_enable = true;	/* === DDR can DFS === */
		mt_socfreq_allowed_enable = true;		/* === SOC can DFS === */

		/* 1333M / 1.125V set in pre-loader*/
		
		#ifdef CPUFREQ_MEMPLL_OD
		#else
		/* MEMPLL freq */
		g_cur_socfreq_default_ddr_freq = DDR_DVFS_F0;
		g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
		g_soc_ddr_enable_state = 1;
		
		/* VCORE voltage */
		if(g_soc_avs_type == 0)
		{
	    	g_cur_soc_volt = DVFS_VCORE_1p1;
			g_cur_vcore = DVFS_VCORE_1p1;
	        g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;
		}
		else
		{
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			#ifdef CPUFREQ_MEMPLL_OD
			DFS_phase_mt6333_config_interface(0x3C); /* DVFS_VCORE_1p0 = 1.075V */
			#else
			mt6333_set_vcore_vosel_on(0x3C); /* DVFS_VCORE_1p0 = 1.075V */
 			#endif
			g_cur_soc_volt = DVFS_VCORE_1p075;
			g_cur_vcore = DVFS_VCORE_1p075;
			g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;
			xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_socfreq_ddr_detection: By soc-avs, VCORE highest 1.075V\n");
			
			#else
			
			new_soc_idx = 7;
			new_vcore_idx = new_soc_idx; // Set VCORE to 1.075V as default
					
			Socprintk("mt_socfreq_ddr_detection: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);
				
			/* Scaling voltage and frequency up */
			mt_cpufreq_volt_set(new_vcore_idx);
			udelay(PMIC_SETTLE_TIME);
			
			g_cur_soc_volt = DVFS_VCORE_1p075;
			g_cur_vcore = DVFS_VCORE_1p075;
			g_cur_socfreq_OPPidx = 7;
			g_soc_volt_enable_state = 1;
			
			xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_socfreq_ddr_detection: By soc-avs, VCORE highest 1.075V\n");
			
			#endif
		}

		#endif

		xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "DFS_Detection() > 0, g_soc_DRAM_Type = %d\n", g_soc_DRAM_Type);
	}

}

/************************************************
* GPU frequency adjust interface for thermal protect
*************************************************/
void mt_gpufreq_power_protect(void)
{
    int i = 0;
    unsigned int limited_freq = 0;
    unsigned int limited_power = 0;

	mutex_lock(&mt_sochp_gpu_power_mutex);

    if (mt_gpufreqs_num == 0)
    {
    	mutex_unlock(&mt_sochp_gpu_power_mutex);
        return;
    }

    for (i = 0; i < MT_CPUFREQ_POWER_LIMITED_MAX_NUM; i++)
    {
        if (mt_cpufreq_limited_gpu_power_array[i] != 0 && limited_power == 0)
        {
            limited_power = mt_cpufreq_limited_gpu_power_array[i];
        }
        else if (mt_cpufreq_limited_gpu_power_array[i] != 0 && limited_power != 0)
        {
            if (mt_cpufreq_limited_gpu_power_array[i] < limited_power)
            {
                limited_power = mt_cpufreq_limited_gpu_power_array[i];
            }
        }
    }

    for (i = 0; i < MT_CPUFREQ_POWER_LIMITED_MAX_NUM; i++)
    {
        dprintk("mt_gpufreq_power_protect: mt_cpufreq_limited_gpu_power_array[%d] = %d\n ", i, mt_cpufreq_limited_gpu_power_array[i]);
    }

    if (limited_power == 0)
    {
        g_gpu_limited_max_id = 0;
    }
    else
    {
        g_gpu_limited_max_id = mt_gpufreqs_num - 1;

        for (i = 0; i < ARRAY_SIZE(mt_gpufreqs_golden_power); i++)
        {
            if (mt_gpufreqs_golden_power[i].gpufreq_power <= limited_power)
            {
                limited_freq = mt_gpufreqs_golden_power[i].gpufreq_khz;
                break;
            }
        }

        for (i = 0; i < mt_gpufreqs_num; i++)
        {
            if (mt_gpufreqs[i].gpufreq_khz <= limited_freq)
            {
                g_gpu_limited_max_id = i;
                break;
            }
        }
    }

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "limit gpu frequency upper bound to id = %d, frequency = %d, limited_power = %d\n", g_gpu_limited_max_id, mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz, limited_power);
    
	mutex_unlock(&mt_sochp_gpu_power_mutex);
	
    return;
}
EXPORT_SYMBOL(mt_gpufreq_power_protect);


/************************************************
* GPU frequency adjust interface for thermal protect
*************************************************/
void mt_gpufreq_thermal_protect(unsigned int limited_power)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_gpufreq_thermal_protect: limited_power = %d\n", limited_power);
    mt_cpufreq_thermal_limited_gpu_power = limited_power;
    mt_cpufreq_limited_gpu_power_array[MT_CPUFREQ_THERMAL_LIMITED_INDEX] = mt_cpufreq_thermal_limited_gpu_power;
    mt_gpufreq_power_protect();
}
EXPORT_SYMBOL(mt_gpufreq_thermal_protect);

/************************************************
* GPU frequency adjust interface for lbat_volt protect
*************************************************/
void mt_gpufreq_lbat_volt_protect(unsigned int limited_power)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_gpufreq_lbat_volt_protect: limited_power = %d\n", limited_power);
    mt_cpufreq_lbat_volt_limited_gpu_power = limited_power;
    mt_cpufreq_limited_gpu_power_array[MT_CPUFREQ_LBAT_VOLT_LIMITED_INDEX] = mt_cpufreq_lbat_volt_limited_gpu_power;
    mt_gpufreq_power_protect();
}
EXPORT_SYMBOL(mt_gpufreq_lbat_volt_protect);

/************************************************
* GPU frequency adjust interface for lbat_volume protect
*************************************************/
void mt_gpufreq_lbat_volume_protect(unsigned int limited_power)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_gpufreq_lbat_volume_protect: limited_power = %d\n", limited_power);
    mt_cpufreq_lbat_volume_limited_gpu_power = limited_power;
    mt_cpufreq_limited_gpu_power_array[MT_CPUFREQ_LBAT_VOLUME_LIMITED_INDEX] = mt_cpufreq_lbat_volume_limited_gpu_power;
    mt_gpufreq_power_protect();
}
EXPORT_SYMBOL(mt_gpufreq_lbat_volume_protect);

/***********************************************
* Setup GPU power table
************************************************/
static void mt_setup_gpufreqs_power_table(int num)
{
    int i = 0, j = 0;

    mt_gpufreqs_power = kzalloc((num) * sizeof(struct mt_gpufreq_power_info), GFP_KERNEL);
    if (mt_gpufreqs_power == NULL)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/SOC_DVFS", "GPU power table memory allocation fail\n");
        return;
    }

    for (i = 0; i < num; i++) {
        mt_gpufreqs_power[i].gpufreq_khz = mt_gpufreqs[i].gpufreq_khz;

        for (j = 0; j < ARRAY_SIZE(mt_gpufreqs_golden_power); j++)
        {
            if (mt_gpufreqs[i].gpufreq_khz == mt_gpufreqs_golden_power[j].gpufreq_khz)
            {
                mt_gpufreqs_power[i].gpufreq_power = mt_gpufreqs_golden_power[j].gpufreq_power;
                break;
            }
        }

        xlog_printk(ANDROID_LOG_INFO, "Power/SOC_DVFS", "mt_gpufreqs_power[%d].gpufreq_khz = %u\n", i, mt_gpufreqs_power[i].gpufreq_khz);
        xlog_printk(ANDROID_LOG_INFO, "Power/SOC_DVFS", "mt_gpufreqs_power[%d].gpufreq_power = %u\n", i, mt_gpufreqs_power[i].gpufreq_power);
    }

    #ifdef CONFIG_THERMAL
    mtk_gpufreq_register(mt_gpufreqs_power, num);
    #endif
}

/***********************************************
* Setup GPU frequency table
************************************************/
static int mt_setup_gpufreqs_table(void)
{
    unsigned int ncpus;
		
    mt_gpufreqs_num = 3;
	ncpus = num_possible_cpus();
	
    mt_gpufreqs = kzalloc((mt_gpufreqs_num) * sizeof(struct mt_gpufreq_info), GFP_KERNEL);
    if (mt_gpufreqs == NULL)
        return -ENOMEM;

    if(ncpus == 8)
    {
    	/* MT6592 */
    	mt_gpufreqs[0].gpufreq_khz = GPU_DVFS_F0;
    	mt_gpufreqs[0].gpufreq_volt = DVFS_VCORE_1p1;
    	mt_gpufreqs[1].gpufreq_khz = GPU_DVFS_F1;
    	mt_gpufreqs[1].gpufreq_volt = DVFS_VCORE_1p0;
    	mt_gpufreqs[2].gpufreq_khz = GPU_DVFS_F2;
    	mt_gpufreqs[2].gpufreq_volt = DVFS_VCORE_1p0;    
    }
	else if(ncpus == 4)
	{
		/* MT6588 */
    	mt_gpufreqs[0].gpufreq_khz = GPU_DVFS_F0_1;
    	mt_gpufreqs[0].gpufreq_volt = DVFS_VCORE_1p1;
    	mt_gpufreqs[1].gpufreq_khz = GPU_DVFS_F1;
    	mt_gpufreqs[1].gpufreq_volt = DVFS_VCORE_1p0;
    	mt_gpufreqs[2].gpufreq_khz = GPU_DVFS_F2;
    	mt_gpufreqs[2].gpufreq_volt = DVFS_VCORE_1p0;    
	}
	else
	{
    	mt_gpufreqs[0].gpufreq_khz = GPU_DVFS_F0;
    	mt_gpufreqs[0].gpufreq_volt = DVFS_VCORE_1p1;
    	mt_gpufreqs[1].gpufreq_khz = GPU_DVFS_F1;
    	mt_gpufreqs[1].gpufreq_volt = DVFS_VCORE_1p0;
    	mt_gpufreqs[2].gpufreq_khz = GPU_DVFS_F2;
    	mt_gpufreqs[2].gpufreq_volt = DVFS_VCORE_1p0;  
	}
	
    g_gpu_limited_max_id = 0;

    if(mt_gpufreqs_power == NULL)
        mt_setup_gpufreqs_power_table(mt_gpufreqs_num);

    return 0;
}


#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
#else
/*************************************************************************************
* Get max vcore between cpu and soc
**************************************************************************************/
static unsigned int mt_get_max_vcore(unsigned int cpu_opp_idx, unsigned int soc_opp_idx)
{
	unsigned int CpuVolt = 0, SocVolt = 0;

	if(is_ext_buck_exist() == 0)
	{	
		/* Convert to CPU ptpod voltage */
		CpuVolt = (700 * 100) + (mt_cpufreq_pmic_volt[cpu_opp_idx] * 625);
		/* Convert to SOC voltage */
		SocVolt = dvfs_vcore_config[soc_opp_idx].dvfs_vcore * 100;

		dprintk("mt_get_max_vcore: CpuVolt =%d, SocVolt = %d\n", CpuVolt, SocVolt);

		#if 1
		
		/* If SOC could not support DVFS, VCORE could not be scaled lower than 1.125V */
		if(mt_socfreq_allowed_enable == false)
		{
			dprintk("mt_get_max_vcore: SOC could not support DVFS, VCORE could not be scaled lower than 1.125V \n");
		}

		#else
		/* If SOC could not support DVFS, VCORE could not be scaled lower than 1.125V */
		if(mt_socfreq_allowed_enable == false)
		{
			dprintk("mt_get_max_vcore: mt_socfreq_allowed_enable = %d \n", mt_socfreq_allowed_enable);
			dprintk("mt_get_max_vcore: SOC could not support DVFS, VCORE could not be scaled, the only fix VORE in 1.125V \n");
			return soc_opp_idx;
		}
		#endif
		
	    if(CpuVolt > SocVolt)
	    {
	        dprintk("mt_get_max_vcore: CPU VCORE > SOC VCORE \n");
			return cpu_opp_idx;
	    }
		else if(CpuVolt < SocVolt)
	    {
	        dprintk("mt_get_max_vcore: CPU VCORE < SOC VCORE \n");
			return soc_opp_idx;
	    }
		else
		{
			#if 1
			
	        dprintk("mt_get_max_vcore: CPU VCORE = SOC VCORE\n");
	        return cpu_opp_idx;		
			
			#else
	        if(dvfs_vcore_config[soc_opp_idx].dvfs_vcore == DVFS_VCORE_1p1)
	        {
	            dprintk("mt_get_max_vcore: CPU VCORE = SOC VCORE, SOC VCORE = %d \n", DVFS_VCORE_1p1);
				return soc_opp_idx;
	        }
	        else if(dvfs_vcore_config[soc_opp_idx].dvfs_vcore == DVFS_VCORE_1p0)
	        {
	            dprintk("mt_get_max_vcore: CPU VCORE = SOC VCORE, SOC VCORE = %d \n", DVFS_VCORE_1p0);
	            return cpu_opp_idx;
	        }
			else
			{
	            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_get_max_vcore: None defined case!!! soc_volt = %d\n", dvfs_vcore_config[soc_opp_idx].dvfs_vcore);
	            return soc_opp_idx;
			}
			#endif
		}
	}
	else
	{
		return soc_opp_idx;
	}
}
#endif

/*************************************************************************************
* SOC DVFS
**************************************************************************************/
static unsigned int mt_soc_type_enable_check(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{
	unsigned int dummy_request = 0;
			
	if(type == SOC_DVFS_TYPE_GPU_HP)
	{
		if((mt6592_soc_dvfs_config[type] == 0) && (sochp_enable == 0))
		{
			if(g_gpu_limited_max_id != 0)
			{
				Socprintk("mt_soc_type_enable_check: thermal limit gpu freq, g_gpu_limited_max_id = %d\n", g_gpu_limited_max_id);
			}
			else
			{
				if(g_gpu_limited_previous == 0)
				{
					Socprintk("mt_soc_type_enable_check: type: %d, previous state: %d, sochp_enable: %d, dummy and return.\n", type, mt6592_soc_dvfs_config[type], sochp_enable);
					dummy_request = 1;
				}
				else
				{
					Socprintk("mt_soc_type_enable_check: thermal unlimit gpu freq, g_gpu_limited_max_id = %d\n", g_gpu_limited_max_id);
				}
			}
		}
	}

	g_gpu_limited_previous = g_gpu_limited_max_id;
	
	return dummy_request;
}

static unsigned int mt_soc_opp_mask(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{
		unsigned int Final_Mask = 0;
		int i=0;
		
        mt6592_soc_dvfs_config[type] = sochp_enable;
		
		for (i = 0; i < SOC_DVFS_TYPE_NUM; i++)
        {
            Soc2printk("mt_soc_opp_mask: mt6592_soc_dvfs_config[%d] = %d\n", i, mt6592_soc_dvfs_config[i]);
			Final_Mask |= mt6592_soc_dvfs_config[i];
		}

		g_soc_final_mask = Final_Mask;
		//Socprintk("mt_soc_opp_mask: g_soc_final_mask = %d\n", g_soc_final_mask);

		return g_soc_final_mask;
}

static unsigned int mt_soc_mmpll_opp_mask(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{
		unsigned int Final_Mask = 0;
		int i=0;
		
		if((type == SOC_DVFS_TYPE_GPU_HP) 
			||(type == SOC_DVFS_TYPE_PAUSE))
		{
        	mt6592_soc_mmpll_dvfs_config[type] = sochp_enable;
		}
		
		for (i = 0; i < SOC_DVFS_TYPE_NUM; i++)
        {
            //Socprintk("mt_soc_mmpll_opp_mask: mt6592_soc_mmpll_dvfs_config[%d] = %d\n", i, mt6592_soc_mmpll_dvfs_config[i]);
			Final_Mask |= mt6592_soc_mmpll_dvfs_config[i];
		}

		g_soc_mmpll_final_mask = Final_Mask;
		//Socprintk("mt_soc_mmpll_opp_mask: g_soc_mmpll_final_mask = %d\n", g_soc_mmpll_final_mask);

		return g_soc_mmpll_final_mask;
}

static unsigned int mt_soc_vencpll_opp_mask(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{
		unsigned int Final_Mask = 0;
		int i=0;
		
		if((type == SOC_DVFS_TYPE_ZSD) 
			||(type == SOC_DVFS_TYPE_DISPLAY)
			||(type == SOC_DVFS_TYPE_PAUSE))
		{
        	mt6592_soc_vencpll_dvfs_config[type] = sochp_enable;
		}
		
		for (i = 0; i < SOC_DVFS_TYPE_NUM; i++)
        {
            //Socprintk("mt_soc_vencpll_opp_mask: mt6592_soc_dvfs_config[%d] = %d\n", i, mt6592_soc_dvfs_config[i]);
			Final_Mask |= mt6592_soc_vencpll_dvfs_config[i];
		}

		g_soc_vencpll_final_mask = Final_Mask;
		//Socprintk("mt_soc_vencpll_opp_mask: g_soc_vencpll_final_mask = %d\n", g_soc_vencpll_final_mask);

		return g_soc_vencpll_final_mask;
}

static unsigned int mt_soc_mempll_opp_mask(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{
		unsigned int Final_Mask = 0;
		int i=0;
		
        mt6592_soc_mempll_dvfs_config[type] = sochp_enable;
		
		for (i = 0; i < SOC_DVFS_TYPE_NUM; i++)
        {
            //Socprintk("mt_soc_mempll_opp_mask: mt6592_soc_mempll_dvfs_config[%d] = %d\n", i, mt6592_soc_mempll_dvfs_config[i]);
			Final_Mask |= mt6592_soc_mempll_dvfs_config[i];
		}

		g_soc_mempll_final_mask = Final_Mask;
		//Socprintk("mt_soc_mempll_opp_mask: g_soc_mempll_final_mask = %d\n", g_soc_mempll_final_mask);

		return g_soc_mempll_final_mask;
}


#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
#else
unsigned int mt_soc_vcore_freq_sub(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{
	unsigned int new_soc_idx, new_vcore_idx;
    unsigned int enable;
	unsigned int mmpll_enable;
	unsigned int vencpll_enable;
	unsigned int mempll_enable;
	unsigned int dummy_check = 0;
	
	g_soc_api_call_test = 1;

	/* Dummy request check */
	dummy_check = mt_soc_type_enable_check(type, sochp_enable);
	if(dummy_check == 1)
	{
		return 0;
	}

	enable = mt_soc_opp_mask(type, sochp_enable);
	mmpll_enable = mt_soc_mmpll_opp_mask(type, sochp_enable);
	vencpll_enable = mt_soc_vencpll_opp_mask(type, sochp_enable);
	mempll_enable = mt_soc_mempll_opp_mask(type, sochp_enable);
	Socprintk("mt_soc_dvfs: g_soc_final_mask = %d, g_soc_mmpll_final_mask = %d\n", g_soc_final_mask, g_soc_mmpll_final_mask);
	Socprintk("mt_soc_dvfs: g_soc_vencpll_final_mask = %d, g_soc_mempll_final_mask = %d\n", g_soc_vencpll_final_mask, g_soc_mempll_final_mask);

	/* Fixed freq/volt check */ 
	enable = mt_soc_volt_fixed_check(enable);
	mmpll_enable = mt_soc_mmpll_fixed_check(mmpll_enable);
	vencpll_enable = mt_soc_vencpll_fixed_check(vencpll_enable);
	mempll_enable = mt_soc_mempll_fixed_check(mempll_enable);

	/* Set VCORE voltage up if anyone request high. */
	if(enable == 1)
	{
		if(mt_socfreq_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs:Not allow to DVS up, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
		}
		else
		{
			if(g_soc_volt_enable_state == 1)
			{
				Socprintk("mt_soc_dvfs: volt already high, g_soc_volt_enable_state = %d\n", g_soc_volt_enable_state);
			}
			else
			{
				g_soc_volt_enable_state = 1;
					
				new_soc_idx = 7;

				if(is_ext_buck_exist() == 0)
				{
					new_vcore_idx = mt_get_max_vcore(g_cur_cpufreq_OPPidx, new_soc_idx);	
				}
				else
				{
					new_vcore_idx = 7;
				}
				
				Socprintk("mt_soc_dvfs: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);

				if(g_soc_avs_type == 0)
				{
					/* Scaling voltage and frequency up */
					mt_cpufreq_volt_set(new_vcore_idx);
					udelay(PMIC_SETTLE_TIME);

					g_cur_soc_volt = DVFS_VCORE_1p1;
					g_cur_vcore = dvfs_vcore_config[new_vcore_idx].dvfs_vcore;
					g_cur_socfreq_OPPidx = new_soc_idx;

					Socprintk("mt_soc_dvfs:Set VCORE up, g_cur_vcore = %d\n", dvfs_vcore_config[new_vcore_idx].dvfs_vcore);
					//Socprintk("mt_soc_dvfs: Set VCORE = %d\n", dvfs_vcore_config[new_vcore_idx].dvfs_vcore);
				}
				else
				{
					/* Scaling voltage and frequency up */
					mt_cpufreq_volt_set(new_vcore_idx);
					udelay(PMIC_SETTLE_TIME);

					g_cur_soc_volt = DVFS_VCORE_1p075;
					g_cur_vcore = dvfs_vcore_config[new_vcore_idx].dvfs_vcore;
					g_cur_socfreq_OPPidx = new_soc_idx;

					Socprintk("mt_soc_dvfs:Set VCORE up, g_cur_vcore = %d\n", dvfs_vcore_config[new_vcore_idx].dvfs_vcore);
				}
			}

		}
		
		//Socprintk("mt_soc_dvfs: g_cur_soc_volt = %d mv\n", g_cur_soc_volt);
	}

	/* Set GPU(MMPLL) frequency */
	if((type == SOC_DVFS_TYPE_GPU_HP)
		||(type == SOC_DVFS_TYPE_PAUSE)
		||(type == SOC_DVFS_TYPE_FIXED))
	{
		/* Thermal limit GPU frequency */
		if(g_gpu_limited_max_id != 0)
		{
			Socprintk("mt_soc_dvfs: thermal limit GPU freq, mt_gpufreqs[%d].gpufreq_khz = %d\n", g_gpu_limited_max_id, mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz);
			
			if(mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz == g_cur_socfreq_gpu_freq)
			{
				Socprintk("mt_soc_dvfs: thermal limit GPU freq == cur GPU freq = %d\n", g_cur_socfreq_gpu_freq);
			}
			else
			{
				if(mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz == GPU_DVFS_F1)
				{
					mt_dfs_mmpll(1796000); /* MMPLL 500MHz*/
					g_cur_socfreq_gpu_freq = GPU_DVFS_F1;
				}
				else if(mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz == GPU_DVFS_F2)
				{
					mt_dfs_mmpll(1014000); /* MMPLL 250MHz */
					g_cur_socfreq_gpu_freq = GPU_DVFS_F2;
				}
				else
				{
					Socprintk("mt_soc_dvfs: No thermal limit GPU freq mach, mt_gpufreqs[%d].gpufreq_khz = %d\n", g_gpu_limited_max_id, mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz);
				}
			}

			g_soc_gpu_enable_state = 2; /* When thermal unlimit, set GPU freq by request no matter what. */
		}
		else
		{
			if(mt_socfreq_allowed_enable == false)
			{
				if(g_soc_gpu_enable_state == 2)
				{
					/* LPDDR3 without DFS / DDR3 */
					/* If thermal have limited GPU freq, when thermal unlimited, reset to high freq. */
					if(mt_num_possible_cpus == 8)
					{
						mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
					}
					else if(mt_num_possible_cpus == 4)
					{
						mt_dfs_mmpll(2405000); /* MMPLL 601.25MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F0_1;
					}
					else
					{
						mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
					}
					
					g_soc_gpu_enable_state = 1;
					
					Socprintk("mt_soc_dvfs: LPDDR3 without DFS / DDR3\n");
					Socprintk("mt_soc_dvfs: If thermal have limited GPU freq, when thermal unlimited, reset to high freq.\n");
				}
				
				Socprintk("mt_soc_dvfs:Not allow to GPU DFS, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
			}
			else
			{
				if(mmpll_enable == 1)
				{
					if(g_soc_gpu_enable_state == 1)
					{
						Socprintk("mt_soc_dvfs: gpu already high, g_soc_gpu_enable_state = %d\n", g_soc_gpu_enable_state);
					}
					else
					{
						if(mt_num_possible_cpus == 8)
						{
							mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
							g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
						}
						else if(mt_num_possible_cpus == 4)
						{
							mt_dfs_mmpll(2405000); /* MMPLL 601.25MHz*/
							g_cur_socfreq_gpu_freq = GPU_DVFS_F0_1;
						}
						else
						{
							mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
							g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
						}

						g_soc_gpu_enable_state = 1;

						Socprintk("mt_soc_dvfs: Set MMPLL freq up, g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
					}
				}
				else
				{
					if(g_soc_gpu_enable_state == 0)
					{
						Socprintk("mt_soc_dvfs: gpu already low, g_soc_gpu_enable_state = %d\n", g_soc_gpu_enable_state);
					}
					else
					{
						mt_dfs_mmpll(1796000); /* MMPLL 500MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F1;

						g_soc_gpu_enable_state = 0;

						Socprintk("mt_soc_dvfs: Set MMPLL freq down, g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
					}
				}
			}
		}

		//Socprintk("mt_soc_dvfs: g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
	}

	/* Set MM(VENCPLL) frequency */
	if((type == SOC_DVFS_TYPE_ZSD)
		||(type == SOC_DVFS_TYPE_DISPLAY)
		||(type == SOC_DVFS_TYPE_PAUSE)
		||(type == SOC_DVFS_TYPE_FIXED))
	{
		if(mt_socfreq_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs:Not allow to MM DFS, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
		}
		else
		{
			if(vencpll_enable == 1)
			{
				if(g_soc_mm_enable_state == 1)
				{
					Socprintk("mt_soc_dvfs: mm already high, g_soc_mm_enable_state = %d\n", g_soc_mm_enable_state);
				}
				else
				{
					mt_dfs_vencpll(1599000); /* VENCPLL 400MHz*/
					g_cur_socfreq_mm_freq = MM_DVFS_F0;
					
					g_soc_mm_enable_state = 1;

					Socprintk("mt_soc_dvfs: Set VENCPLL freq up, g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
				}
			}
			else
			{
				if(g_soc_mm_enable_state == 0)
				{
					Socprintk("mt_soc_dvfs: mm already low, g_soc_mm_enable_state = %d\n", g_soc_mm_enable_state);
				}
				else
				{
					mt_dfs_vencpll(1183000); /* VENCPLL 295MHz*/
					g_cur_socfreq_mm_freq = MM_DVFS_F1;
					
					g_soc_mm_enable_state = 0;

					Socprintk("mt_soc_dvfs: Set VENCPLL freq down, g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
				}
			}
		}

		//Socprintk("mt_soc_dvfs: g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
	}

	/* Set DDR(MEMPLL) frequency */
	if((type >= SOC_DVFS_TYPE_VENC) && (type < SOC_DVFS_TYPE_NUM))
	{
		if(mt_socfreq_ddr_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs: This DDR not support DFS\n");
		}
		else
		{
			if(mempll_enable == 1)
			{
				if(g_soc_ddr_enable_state == 1)
				{
					Socprintk("mt_soc_dvfs: ddr already high, g_soc_ddr_enable_state = %d\n", g_soc_ddr_enable_state);
				}
				else
				{
					#ifdef CPUFREQ_MEMPLL_OD
					if((g_ddr_frequency_type == 1)||(g_ddr_frequency_type == 2))
					{
						mt_fh_dram_overclock(367); /* MEMPLL 367MHz*/
						g_cur_socfreq_ddr_freq = DDR_DVFS_F0_1;
					}
					else
					{
						mt_fh_dram_overclock(333); /* MEMPLL 333MHz*/
						g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
					}
					#else
					mt_fh_dram_overclock(333); /* MEMPLL 333MHz*/
						
					g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
					#endif

					g_soc_ddr_enable_state = 1;
					Socprintk("mt_soc_dvfs: Set MEMPLL freq up, g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
				}
			}
			else
			{
				if(g_soc_ddr_enable_state == 0)
				{
					Socprintk("mt_soc_dvfs: ddr already low, g_soc_ddr_enable_state = %d\n", g_soc_ddr_enable_state);
				}
				else
				{
					mt_fh_dram_overclock(266); /* MEMPLL 266MHz*/
						
					g_cur_socfreq_ddr_freq = DDR_DVFS_F1;
					g_soc_ddr_enable_state = 0;
					Socprintk("mt_soc_dvfs: Set MEMPLL freq down, g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
				}
			}
		}
		
		//Socprintk("mt_soc_dvfs: g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
	}

	/* Set VCORE voltage down only if all request low. */
	if(enable == 0)
	{
		if(mt_socfreq_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs:Not allow to DVS down, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
		}
		else
		{
			/* Set VCORE voltage */
			if(g_soc_volt_enable_state == 0)
			{
				Socprintk("mt_soc_dvfs: volt already low, g_soc_volt_enable_state = %d\n", g_soc_volt_enable_state);
			}
			else
			{
				g_soc_volt_enable_state = 0;
					
				new_soc_idx = 8;

				if(is_ext_buck_exist() == 0)
				{
					new_vcore_idx = mt_get_max_vcore(g_cur_cpufreq_OPPidx, new_soc_idx);
				}
				else
				{
					new_vcore_idx = 8;
				}
				
				Socprintk("mt_soc_dvfs: new_vcore_idx = %d, g_cur_cpufreq_OPPidx = %d, new_soc_idx = %d\n", new_vcore_idx, g_cur_cpufreq_OPPidx, new_soc_idx);

				mt_cpufreq_volt_set(new_vcore_idx);
				
				g_cur_soc_volt = DVFS_VCORE_1p0;
				g_cur_vcore = dvfs_vcore_config[new_vcore_idx].dvfs_vcore;
				g_cur_socfreq_OPPidx = new_soc_idx;
				
				Socprintk("mt_soc_dvfs: Set VCORE down, g_cur_vcore =  %d\n", dvfs_vcore_config[new_vcore_idx].dvfs_vcore);
				//Socprintk("mt_soc_dvfs: Set VCORE down\n");
			}
		}
		
		//Socprintk("mt_soc_dvfs: g_cur_soc_volt = %d mv\n", g_cur_soc_volt);
	}

	return 0;
}
#endif

unsigned int mt_soc_dvfs(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable)
{	
	/*	TOBEDONE */
    /* Efuse   */
#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    unsigned int enable;
	unsigned int mmpll_enable;
	unsigned int vencpll_enable;
	unsigned int mempll_enable;
	unsigned int dummy_check = 0;
	
	Socprintk("mt_soc_dvfs: type = %d, sochp_enable = %d\n", type, sochp_enable);
		
	if(g_cur_socfreq_keep_default_state == 1)
	{
		Socprintk("mt_soc_dvfs: Return request because command of return DEFAULT setting\n");
		return 0;
	}
	
	mutex_lock(&mt_sochp_mutex);

	g_soc_api_call_test = 1;

	/* Dummy request check */
	dummy_check = mt_soc_type_enable_check(type, sochp_enable);
	if(dummy_check == 1)
	{
		mutex_unlock(&mt_sochp_mutex);
		return 0;
	}
	
    enable = mt_soc_opp_mask(type, sochp_enable);
	mmpll_enable = mt_soc_mmpll_opp_mask(type, sochp_enable);
	vencpll_enable = mt_soc_vencpll_opp_mask(type, sochp_enable);
	mempll_enable = mt_soc_mempll_opp_mask(type, sochp_enable);
	Socprintk("mt_soc_dvfs: g_soc_final_mask = %d, g_soc_mmpll_final_mask = %d\n", g_soc_final_mask, g_soc_mmpll_final_mask);
	Socprintk("mt_soc_dvfs: g_soc_vencpll_final_mask = %d, g_soc_mempll_final_mask = %d\n", g_soc_vencpll_final_mask, g_soc_mempll_final_mask);
	
	/* Fixed freq/volt check */	
	enable = mt_soc_volt_fixed_check(enable);
	mmpll_enable = mt_soc_mmpll_fixed_check(mmpll_enable);
	vencpll_enable = mt_soc_vencpll_fixed_check(vencpll_enable);
	mempll_enable = mt_soc_mempll_fixed_check(mempll_enable);
	
	/* Set VCORE voltage up if anyone request high. */
    if(enable == 1)
    {
		if(mt_socfreq_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs:Not allow to DVS up, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
		}
		else
		{
	        if(g_soc_volt_enable_state == 1)
	        {
	            Socprintk("mt_soc_dvfs: volt already high, g_soc_volt_enable_state = %d\n", g_soc_volt_enable_state);
			}
			else
			{
				g_soc_volt_enable_state = 1;

				if(g_soc_avs_type == 0)
				{
					#ifdef CPUFREQ_MEMPLL_OD
					DFS_phase_mt6333_config_interface(0x44); /* DVFS_VCORE_1p1 = 1.125V */
					#else
					mt6333_set_vcore_vosel_on(0x44); /* DVFS_VCORE_1p1 = 1.125V */
					#endif
					/* delay 40us + volt step * 0.5 us = 40 + (125/6.25)*0.5 = 50us */
					udelay(50);
				}
				else
				{
					#ifdef CPUFREQ_MEMPLL_OD
					DFS_phase_mt6333_config_interface(0x3C); /* DVFS_VCORE_1p075 = 1.075V */
					#else
					mt6333_set_vcore_vosel_on(0x3C); /* DVFS_VCORE_1p05 = 1.075V */
					#endif
					/* delay 40us + volt step * 0.5 us = 40 + (125/6.25)*0.5 = 50us */
					udelay(50);

					Socprintk("mt_soc_dvfs:By soc-avs, VCORE 1.075V\n");
				}

				Socprintk("mt_soc_dvfs:Set mt6333 VCORE up, g_cur_soc_volt = %d mv\n", g_cur_soc_volt);
			}

			if(g_soc_avs_type == 0)
			{
				g_cur_soc_volt = DVFS_VCORE_1p1;
				g_cur_vcore = DVFS_VCORE_1p1;
		        g_cur_socfreq_OPPidx = 7;
			}
			else
			{
				g_cur_soc_volt = DVFS_VCORE_1p075;
				g_cur_vcore = DVFS_VCORE_1p075;
		        g_cur_socfreq_OPPidx = 7;			
			}
		}
		//Socprintk("mt_soc_dvfs: g_cur_soc_volt = %d mv\n", g_cur_soc_volt);
	}

	/* Set GPU(MMPLL) frequency */
	if((type == SOC_DVFS_TYPE_GPU_HP)
		||(type == SOC_DVFS_TYPE_PAUSE)
		||(type == SOC_DVFS_TYPE_FIXED))
	{
		/* Thermal limit GPU frequency */
		if(g_gpu_limited_max_id != 0)
		{
			Socprintk("mt_soc_dvfs: thermal limit GPU freq, mt_gpufreqs[%d].gpufreq_khz = %d\n", g_gpu_limited_max_id, mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz);
			
			if(mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz == g_cur_socfreq_gpu_freq)
			{
				Socprintk("mt_soc_dvfs: thermal limit GPU freq == cur GPU freq = %d\n", g_cur_socfreq_gpu_freq);
			}
			else
			{
				if(mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz == GPU_DVFS_F1)
				{
					mt_dfs_mmpll(1796000); /* MMPLL 500MHz*/
					g_cur_socfreq_gpu_freq = GPU_DVFS_F1;
				}
				else if(mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz == GPU_DVFS_F2)
				{
					mt_dfs_mmpll(1014000); /* MMPLL 250MHz */
					g_cur_socfreq_gpu_freq = GPU_DVFS_F2;
				}
				else
				{
					Socprintk("mt_soc_dvfs: No thermal limit GPU freq mach, mt_gpufreqs[%d].gpufreq_khz = %d\n", g_gpu_limited_max_id, mt_gpufreqs[g_gpu_limited_max_id].gpufreq_khz);
				}
			}

			g_soc_gpu_enable_state = 2; /* When thermal unlimit, set GPU freq by request no matter what. */
		}
		else
		{
			if(mt_socfreq_allowed_enable == false)
			{
				if(g_soc_gpu_enable_state == 2)
				{
					/* LPDDR3 without DFS / DDR3 */
					/* If thermal have limited GPU freq, when thermal unlimited, reset to high freq. */
					if(mt_num_possible_cpus == 8)
					{
						mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
					}
					else if(mt_num_possible_cpus == 4)
					{
						mt_dfs_mmpll(2405000); /* MMPLL 601.25MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F0_1;
					}
					else
					{
						mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
					}
					
					g_soc_gpu_enable_state = 1;
					
					Socprintk("mt_soc_dvfs: LPDDR3 without DFS / DDR3\n");
					Socprintk("mt_soc_dvfs: If thermal have limited GPU freq, when thermal unlimited, reset to high freq.\n");
				}
				
				Socprintk("mt_soc_dvfs:Not allow to GPU DFS, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
			}
			else
			{
				if(mmpll_enable == 1)
				{
					if(g_soc_gpu_enable_state == 1)
					{
						Socprintk("mt_soc_dvfs: gpu already high, g_soc_gpu_enable_state = %d\n", g_soc_gpu_enable_state);
					}
					else
					{
						if(mt_num_possible_cpus == 8)
						{
							mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
							g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
						}
						else if(mt_num_possible_cpus == 4)
						{
							mt_dfs_mmpll(2405000); /* MMPLL 601.25MHz*/
							g_cur_socfreq_gpu_freq = GPU_DVFS_F0_1;
						}
						else
						{
							mt_dfs_mmpll(2782000); /* MMPLL 700MHz*/
							g_cur_socfreq_gpu_freq = GPU_DVFS_F0;
						}

						g_soc_gpu_enable_state = 1;

						Socprintk("mt_soc_dvfs: Set MMPLL freq up, g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
					}
				}
				else
				{
					if(g_soc_gpu_enable_state == 0)
					{
						Socprintk("mt_soc_dvfs: gpu already low, g_soc_gpu_enable_state = %d\n", g_soc_gpu_enable_state);
					}
					else
					{
						mt_dfs_mmpll(1796000); /* MMPLL 500MHz*/
						g_cur_socfreq_gpu_freq = GPU_DVFS_F1;

						g_soc_gpu_enable_state = 0;

						Socprintk("mt_soc_dvfs: Set MMPLL freq down, g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
					}
				}
			}
		}

		//Socprintk("mt_soc_dvfs: g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
	}

	/* Set MM(VENCPLL) frequency */
	if((type == SOC_DVFS_TYPE_ZSD)
		||(type == SOC_DVFS_TYPE_DISPLAY)
		||(type == SOC_DVFS_TYPE_PAUSE)
		||(type == SOC_DVFS_TYPE_FIXED))
	{
		if(mt_socfreq_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs:Not allow to MM DFS, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
		}
		else
		{
			if(vencpll_enable == 1)
			{
				if(g_soc_mm_enable_state == 1)
				{
					Socprintk("mt_soc_dvfs: mm already high, g_soc_mm_enable_state = %d\n", g_soc_mm_enable_state);
				}
				else
				{
					mt_dfs_vencpll(1599000); /* VENCPLL 400MHz*/
					g_cur_socfreq_mm_freq = MM_DVFS_F0;
					
					g_soc_mm_enable_state = 1;

					Socprintk("mt_soc_dvfs: Set VENCPLL freq up, g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
				}
			}
			else
			{
				if(g_soc_mm_enable_state == 0)
				{
					Socprintk("mt_soc_dvfs: mm already low, g_soc_mm_enable_state = %d\n", g_soc_mm_enable_state);
				}
				else
				{
					mt_dfs_vencpll(1183000); /* VENCPLL 295MHz*/
					g_cur_socfreq_mm_freq = MM_DVFS_F1;
					
					g_soc_mm_enable_state = 0;

					Socprintk("mt_soc_dvfs: Set VENCPLL freq down, g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
				}
			}
		}

		//Socprintk("mt_soc_dvfs: g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
	}

	/* Set DDR(MEMPLL) frequency */
	if((type >= SOC_DVFS_TYPE_VENC) && (type < SOC_DVFS_TYPE_NUM))
	{
		if(mt_socfreq_ddr_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs: This DDR not support DFS\n");
		}
		else
		{
			if(mempll_enable == 1)
			{
				if(g_soc_ddr_enable_state == 1)
				{
					Socprintk("mt_soc_dvfs: ddr already high, g_soc_ddr_enable_state = %d\n", g_soc_ddr_enable_state);
				}
				else
				{
					#ifdef CPUFREQ_MEMPLL_OD
					if((g_ddr_frequency_type == 1)||(g_ddr_frequency_type == 2))
					{
						mt_fh_dram_overclock(367); /* MEMPLL 367MHz*/
						g_cur_socfreq_ddr_freq = DDR_DVFS_F0_1;
					}
					else
					{
						mt_fh_dram_overclock(333); /* MEMPLL 333MHz*/
						g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
					}
					#else
					mt_fh_dram_overclock(333); /* MEMPLL 333MHz*/
						
					g_cur_socfreq_ddr_freq = DDR_DVFS_F0;
					#endif

					g_soc_ddr_enable_state = 1;
					Socprintk("mt_soc_dvfs: Set MEMPLL freq up, g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
				}
			}
			else
			{
				if(g_soc_ddr_enable_state == 0)
				{
					Socprintk("mt_soc_dvfs: ddr already low, g_soc_ddr_enable_state = %d\n", g_soc_ddr_enable_state);
				}
				else
				{
					mt_fh_dram_overclock(266); /* MEMPLL 266MHz*/
						
					g_cur_socfreq_ddr_freq = DDR_DVFS_F1;
					g_soc_ddr_enable_state = 0;
					Socprintk("mt_soc_dvfs: Set MEMPLL freq down, g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
				}
			}
		}
		
		//Socprintk("mt_soc_dvfs: g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
	}

	/* Set VCORE voltage down only if all request low. */
    if(enable == 0)
    {
		if(mt_socfreq_allowed_enable == false)
		{
			Socprintk("mt_soc_dvfs:Not allow to DVS down, mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
		}
		else
		{
	    	/* Set VCORE voltage */
	        if(g_soc_volt_enable_state == 0)
	        {
	            Socprintk("mt_soc_dvfs: volt already low, g_soc_volt_enable_state = %d\n", g_soc_volt_enable_state);
			}
			else
			{
				g_soc_volt_enable_state = 0;
				
				#ifdef CPUFREQ_MEMPLL_OD	
				DFS_phase_mt6333_config_interface(0x30); /* DVFS_VCORE_1p0 = 1.00V */
				#else
				mt6333_set_vcore_vosel_on(0x30); /* DVFS_VCORE_1p0 = 1.00V */
				#endif

				Socprintk("mt_soc_dvfs: Set VCORE down, g_cur_soc_volt = %d\n", g_cur_soc_volt);
			}

			g_cur_soc_volt = DVFS_VCORE_1p0;
			g_cur_vcore = DVFS_VCORE_1p0;
	        g_cur_socfreq_OPPidx = 8;

		}
		
		//Socprintk("mt_soc_dvfs: mt6333 g_cur_soc_volt = %d mv\n", g_cur_soc_volt);
	}

    mutex_unlock(&mt_sochp_mutex);
#else

    unsigned long flags;

	Socprintk("mt_soc_dvfs: type = %d, sochp_enable = %d\n", type, sochp_enable);
	
	if(g_cur_socfreq_keep_default_state == 1)
	{
		Socprintk("mt_soc_dvfs: Return request because command of return DEFAULT setting\n");
		return 0;
	}
	
	/* PMIC External Buck */
	if(is_ext_buck_exist()==0)
    {
    	spin_lock_irqsave(&mt_cpufreq_lock, flags);

		mt_soc_vcore_freq_sub(type, sochp_enable);

		spin_unlock_irqrestore(&mt_cpufreq_lock, flags);
	}
	else
	{
		mutex_lock(&mt_cpufreq_mutex);

		mt_soc_vcore_freq_sub(type, sochp_enable);

		mutex_unlock(&mt_cpufreq_mutex);
	}
	
#endif

    return 0;
}
EXPORT_SYMBOL(mt_soc_dvfs);


/*************************************************************************************
* Return vproc for ptpod
**************************************************************************************/
extern int ncp6335_read_byte(kal_uint8 cmd, kal_uint8 *returnData);
extern int fan53555_read_byte(kal_uint8 cmd, kal_uint8 *returnData);
extern S32 pwrap_read( U32  adr, U32 *rdata );

#ifdef CPUFREQ_HIGHEST_TURBO_MODE

/*************************************************************************************
* Check turbo mode Efuse
**************************************************************************************/
static unsigned int mt_cpufreq_turbo_mode_check_efuse(void)
{
    unsigned int turbo_mode = 0, on_off = 0, turbo_4_core = 0, turbo_2_core = 0, ret = 0;

    turbo_mode = get_devinfo_with_index(15);

	on_off = ((turbo_mode >> 10) & 0x1); /* bit[10], 0: on, 1:off  */
	turbo_4_core = ((turbo_mode >> 7) & 0x7); /* bit[9:7], 000: 0, 001: 26MHz, 010: 52MHz, 011: 78MHz, 100: 104MHz, 																			  
																 101: 130MHz, 110: 156MHz, 111: 182MHz */
	turbo_2_core = ((turbo_mode >> 4) & 0x7); /* bit[6:4], 000: 104MHz, 001: 26MHz, 010: 52MHz, 011: 78MHz, 100: 104MHz,
																 101: 130MHz, 110: 156MHz, 111: 182MHz */

	xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq_turbo_mode_check_efuse: on_off = %d, turbo_4_core = %d, turbo_2_core = %d\n", on_off, turbo_4_core, turbo_2_core);
																	 	
	g_cpufreq_turbo_mode_efuse_on_off = on_off;
	xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "g_cpufreq_turbo_mode_efuse_on_off = %d\n", g_cpufreq_turbo_mode_efuse_on_off);
	
    switch (turbo_4_core)
    {
        case 0:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_0; // 000: 0
            break;
        case 1:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_1;
            break;
        case 2:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_2;
            break;
        case 3:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_3;
            break;
        case 4:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_4; // 100: 104MHz
            break;
        case 5:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_5;
            break;
        case 6:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_6;
            break;
        case 7:
			g_cpufreq_turbo_mode_efuse_4_core = TURBO_MODE_LV_7;
            break;
        default:
            break;
    }

	xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "g_cpufreq_turbo_mode_efuse_4_core = %d\n", g_cpufreq_turbo_mode_efuse_4_core);
	
    switch (turbo_2_core)
    {
        case 0:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_4; // 000: 104MHz
            break;
        case 1:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_1;
            break;
        case 2:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_2;
            break;
        case 3:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_3;
            break;
        case 4:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_0; // 100: 0MHz
            break;
        case 5:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_5;
            break;
        case 6:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_6;
            break;
        case 7:
			g_cpufreq_turbo_mode_efuse_2_core = TURBO_MODE_LV_7;
            break;
        default:
            break;
    }

	xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "g_cpufreq_turbo_mode_efuse_2_core = %d\n", g_cpufreq_turbo_mode_efuse_2_core);

	#ifdef CPUFREQ_HIGHEST_TURBO_MODE_OFF
	g_cpufreq_turbo_mode_efuse_on_off = 1;
	g_cpufreq_turbo_mode_efuse_4_core = 0;
	g_cpufreq_turbo_mode_efuse_2_core = 0;
	#endif
	
    return ret;
}
#endif

unsigned int mt_cpufreq_cur_vproc(void)
{
	kal_uint8 reg_val=0;
	u32 volt_val=0;
	u32 rdata = 0;
	
	if(is_ext_buck_exist() == 0)
	{
		pwrap_read((u32)0x0220, &rdata);
		rdata = (((rdata * 625) / 100) + 700); // (700mv + n * 6.25mv)
		return rdata;
	}
	else
	{
		if(is_fan53555_exist() == 1)
		{
			fan53555_read_byte(0x01, &reg_val); // 1step=10mV
		
			volt_val = (u32)reg_val & 0x3F;
			volt_val = volt_val*1000 + 60000;
		}
		else if(is_ncp6335_exist() == 1)
		{
			ncp6335_read_byte(0x10, &reg_val); // 1step=6.25mV
								
			volt_val = (u32)reg_val & 0x7F;
			volt_val = volt_val*625 + 60000; 				   
		}
		else
		{
			//error
			dprintk("mt_cpufreq_cur_vproc: Not support\n");
		}
		
		return (volt_val/100); // ex. 1000.25mV => return value is 100025
	}
}
EXPORT_SYMBOL(mt_cpufreq_cur_vproc);


/* PMIC external buck */
/************************************************
* External buck VPROC mapping
*************************************************/
static unsigned int mt_cpufreq_ext_buck_vproc_mapping(unsigned int vproc)
{
	int i = 0, array_size = 0, ext_vproc = 0, found = 0;

	if(is_ext_buck_exist() == 0)
	{
		ext_vproc = vproc;
	}
	else
	{
		if(is_fan53555_exist() == 1)
		{
			array_size = ARRAY_SIZE(mt6592_fan53555_vproc_map);
				
			for (i = 0; i < array_size; i++)
			{
				if (vproc == mt6592_fan53555_vproc_map[i].cpufreq_vproc)
				{
					ext_vproc = mt6592_fan53555_vproc_map[i].cpufreq_ext_vproc;
					found = 1;
					dprintk("mt_cpufreq_ext_buck_vproc_mapping: vproc = %d, mt6592_fan53555_vproc_map[%d].cpufreq_ext_vproc = %d\n", vproc, i, mt6592_fan53555_vproc_map[i].cpufreq_ext_vproc);
					break;
				}
			}

			if(found == 0)
			{
				ext_vproc = vproc;
				dprintk("mt_cpufreq_ext_buck_vproc_mapping: Not found correspond voltage, vproc = %d\n", vproc);
			}
		}
		else if(is_ncp6335_exist() == 1)
		{
			ext_vproc = vproc;
			dprintk("mt_cpufreq_ext_buck_vproc_mapping: vproc = %d, ncp6335\n", vproc);
		}
		else
		{
			ext_vproc = vproc;
		}
	}
				
	return ext_vproc;
}

/*************************************************************************************
* Only if dvfs enter earlysuspend and set 1.1GHz/1.15V, deep idle could control VPROC.
**************************************************************************************/
bool mt_cpufreq_earlysuspend_status_get(void)
{
    return mt_cpufreq_earlysuspend_allow_deepidle_control_vproc;
}
EXPORT_SYMBOL(mt_cpufreq_earlysuspend_status_get);

/************************************************
* Limited max frequency in 1.15GHz when early suspend 
*************************************************/
static unsigned int mt_cpufreq_limit_max_freq_by_early_suspend(void)
{
    struct cpufreq_policy *policy;

    policy = cpufreq_cpu_get(0);

    if (!policy)
        goto no_policy;

    cpufreq_driver_target(policy, DVFS_F4, CPUFREQ_RELATION_L);

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq limited max freq by early suspend %d\n", DVFS_F4);

    cpufreq_cpu_put(policy);

no_policy:
    return g_cur_freq;
}

#ifdef PTPOD_DOWNGRADE_FREQ
#include "mach/mt_thermal.h"
#include "mach/mt_ptp.h"

static bool mt_cpufreq_downgrade_freq_for_ptpod = false;

static int mt_cpufreq_over_max_cpu = 8;

static unsigned int mt_cpufreq_downgrade_freq_step_idx_1 = 1; // downgrade frequency 1 level, 45C < x < 65C
static unsigned int mt_cpufreq_downgrade_freq_step_idx_2 = 1; // downgrade frequency 1 level, 65C < x
static unsigned int mt_cpufreq_downgrade_freq_step_idx_limit = 1;
static unsigned int mt_cpufreq_downgrade_freq = 0;

static int mt_cpufreq_ptpod_temperature_limit_1 = 45000; //45000; //45C
static int mt_cpufreq_ptpod_temperature_limit_2 = 65000; //65000; //65C

static int mt_cpufreq_ptpod_temperature_delta_1 = 5000; // tempature delta 5C
static int mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = 5; // 5:5 , 45C < x < 65C, delta < 5C
static int mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = 6; // 6:4, 45C < x < 65C, delta > 5C
static int mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = 8; // 8:2, 65C < x, delta < 5C
static int mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = 9; // 9:1, 65C < x, delta > 5C
static int mt_cpufreq_ptpod_temperature_delta_counter = 0; 
static int mt_cpufreq_ptpod_temperature_delta_counter_limit = 10; //300ms

static unsigned int mt_cpufreq_downgrade_freq_counter = 0;
static unsigned int mt_cpufreq_downgrade_freq_counter_return = 0;
static unsigned int mt_cpufreq_downgrade_freq_counter_limit = 0; // downgrade frequency time ratio 0 (default)
static unsigned int mt_cpufreq_downgrade_freq_counter_return_limit = 10; // return frequency time ratio 10 (default)

//static unsigned int mt_cpufreq_pre_online_cpu = 0;
//static unsigned int mt_cpufreq_pre_freq = 0;

static unsigned int mt_cpufreq_downgrade_freq_temp_prev = 0;

int mt_cpufreq_lbat_volume_protect(void);
#endif

void mt_cpufreq_downgrade_freq_check(void)
{
    struct cpufreq_policy *policy;
    unsigned int temp_0 = 0;
    unsigned int delta_temp_0 = 0;

    mutex_lock(&mt_cpufreq_downgrade_freq_mutex);

    /* Low Battery Volume Protection */
    if (mt_cpufreq_lbat_volume_protect())
    {
        mt_cpufreq_ptpod_temperature_delta_counter = 0;
		
        mt_cpufreq_downgrade_freq_counter = 0;
        mt_cpufreq_downgrade_freq_counter_return = 0;

        mt_cpufreq_downgrade_freq_counter_limit = 0;
        mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;

        /* Release frequency limit */ 
        mt_cpufreq_downgrade_freq_for_ptpod = false;

        mt_cpufreq_downgrade_freq_temp_prev = temp_0;

        mutex_unlock(&mt_cpufreq_downgrade_freq_mutex);
        return;
    }

	if(mt_cpufreq_downgrade_freq_for_ptpod_enable == 0)
		return;
	
    //dprintk("mt_cpufreq_downgrade_freq_check: ============================\n");
    if(g_max_freq_by_ptp < DVFS_F0)
    {
        mt_cpufreq_ptpod_temperature_delta_counter = 0;

        mt_cpufreq_downgrade_freq_counter = 0;
        mt_cpufreq_downgrade_freq_counter_return = 0;

        mt_cpufreq_downgrade_freq_counter_limit = 0;
        mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;

        /* Release frequency limit */ 
        mt_cpufreq_downgrade_freq_for_ptpod = false;

        mt_cpufreq_downgrade_freq_temp_prev = temp_0;

        mutex_unlock(&mt_cpufreq_downgrade_freq_mutex);
        return;
    }

    // Get temperature
    if(ptp_status() == 1)
    {
        temp_0 = (((DRV_Reg32(PTP_TEMP) & 0xff)) + 25) * 1000;
    }
    else
    {
        temp_0 = (unsigned int)mtktscpu_get_Tj_temp();
    }

    if(temp_0 < 0 || temp_0 > 125000)
    {
        dprintk("mt_cpufreq_downgrade_freq_check: temp_0 < 0 || temp_0 > 125000\n");
        mutex_unlock(&mt_cpufreq_downgrade_freq_mutex);
        return;
    }

    // Temperature do not need to downgrade frequency 
    if(temp_0 <= mt_cpufreq_ptpod_temperature_limit_1)
    {
        mt_cpufreq_ptpod_temperature_delta_counter = 0;

        mt_cpufreq_downgrade_freq_counter = 0;
        mt_cpufreq_downgrade_freq_counter_return = 0;

        mt_cpufreq_downgrade_freq_counter_limit = 0;
        mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;

        /* Release frequency limit */ 
        mt_cpufreq_downgrade_freq_for_ptpod = false;

        //dprintk("mt_cpufreq_downgrade_freq_check: temp_0 <= 45, mt_cpufreq_ptpod_temperature_delta_counter = %d\n", mt_cpufreq_ptpod_temperature_delta_counter);
        //dprintk("mt_cpufreq_downgrade_freq_check: temp_0 <= 45, mt_cpufreq_downgrade_freq_counter = %d\n", mt_cpufreq_downgrade_freq_counter);
        //dprintk("mt_cpufreq_downgrade_freq_check: temp_0 <= 45, mt_cpufreq_downgrade_freq_counter_return = %d\n", mt_cpufreq_downgrade_freq_counter_return);

        //dprintk("mt_cpufreq_downgrade_freq_check: temp_0 <= 45, mt_cpufreq_downgrade_freq_counter_limit = %d\n", mt_cpufreq_downgrade_freq_counter_limit);
        //dprintk("mt_cpufreq_downgrade_freq_check: temp_0 <= 45, mt_cpufreq_downgrade_freq_counter_return_limit = %d\n", mt_cpufreq_downgrade_freq_counter_return_limit);

        mt_cpufreq_downgrade_freq_temp_prev = temp_0;

        mutex_unlock(&mt_cpufreq_downgrade_freq_mutex);
        return;
    }

    // Calculate temperature delta
    if(mt_cpufreq_ptpod_temperature_delta_counter >= mt_cpufreq_ptpod_temperature_delta_counter_limit)
    {	
        if(temp_0 >= mt_cpufreq_downgrade_freq_temp_prev)
        {
            delta_temp_0 = temp_0 - mt_cpufreq_downgrade_freq_temp_prev;
        }
        else
        {
        delta_temp_0 = mt_cpufreq_downgrade_freq_temp_prev - temp_0;
        }

        dprintk("mt_cpufreq_downgrade_freq_check: temp_0 = %d, delta_temp_0 = %d\n", temp_0, delta_temp_0);

        mt_cpufreq_ptpod_temperature_delta_counter = 0;
        mt_cpufreq_ptpod_temperature_delta_counter++;
        mt_cpufreq_downgrade_freq_counter = 0;
        mt_cpufreq_downgrade_freq_counter_return = 0;

        // Different temperature segment has different downgrade frequency time ratio
        if((temp_0 > mt_cpufreq_ptpod_temperature_limit_1) 
            && (temp_0 <= mt_cpufreq_ptpod_temperature_limit_2))
        {
            if(delta_temp_0 <= mt_cpufreq_ptpod_temperature_delta_1)
            {
                mt_cpufreq_downgrade_freq_counter_limit = mt_cpufreq_ptpod_temperature_delta_1_ratio_1;
                mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;
            }
            else
            {
                mt_cpufreq_downgrade_freq_counter_limit = mt_cpufreq_ptpod_temperature_delta_2_ratio_1;
                mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;		
            }

            mt_cpufreq_downgrade_freq_step_idx_limit = mt_cpufreq_downgrade_freq_step_idx_1;
        }
        else
        {
            if(delta_temp_0 <= mt_cpufreq_ptpod_temperature_delta_1)
            {
                mt_cpufreq_downgrade_freq_counter_limit = mt_cpufreq_ptpod_temperature_delta_1_ratio_2;
                mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;
            }
            else
            {
                mt_cpufreq_downgrade_freq_counter_limit = mt_cpufreq_ptpod_temperature_delta_2_ratio_2;
                mt_cpufreq_downgrade_freq_counter_return_limit = mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit;		
            }

            mt_cpufreq_downgrade_freq_step_idx_limit = mt_cpufreq_downgrade_freq_step_idx_2;
        }

        mt_cpufreq_downgrade_freq_temp_prev = temp_0;

        dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_downgrade_freq_counter_limit = %d\n", mt_cpufreq_downgrade_freq_counter_limit);
        dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_downgrade_freq_counter_return_limit = %d\n", mt_cpufreq_downgrade_freq_counter_return_limit);
    }
    else
    {
        mt_cpufreq_ptpod_temperature_delta_counter++;
    }

    // Check if need to downgrade frequency
    if (num_online_cpus() >= mt_cpufreq_over_max_cpu)
    {
        if (mt_cpufreq_downgrade_freq_counter < mt_cpufreq_downgrade_freq_counter_limit)
        {
            mt_cpufreq_downgrade_freq_counter++;
            dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_downgrade_freq_counter_limit = %d\n", mt_cpufreq_downgrade_freq_counter_limit);
            dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_downgrade_freq_counter = %d\n", mt_cpufreq_downgrade_freq_counter);

            mt_cpufreq_downgrade_freq = mt_cpu_freqs[0+mt_cpufreq_downgrade_freq_step_idx_limit].cpufreq_khz;

            mt_cpufreq_downgrade_freq_for_ptpod  = true;

            dprintk("mt_cpufreq_downgrade_freq_check: freq limit, mt_cpufreq_downgrade_freq_for_ptpod = %d\n", mt_cpufreq_downgrade_freq_for_ptpod);

            /* force cpufreq thread executed next time */
            if(g_cur_freq > mt_cpufreq_downgrade_freq)
            {
                policy = cpufreq_cpu_get(0);

                if (!policy)
                    goto no_policy;

                cpufreq_driver_target(policy, mt_cpufreq_downgrade_freq, CPUFREQ_RELATION_L);

                cpufreq_cpu_put(policy);
            }
        }
        else
        {
            /* Release frequency limit */ 
            mt_cpufreq_downgrade_freq_for_ptpod = false;

            if(mt_cpufreq_downgrade_freq_counter_return < mt_cpufreq_downgrade_freq_counter_return_limit)
            {
                mt_cpufreq_downgrade_freq_counter_return++;
            }

            if((mt_cpufreq_downgrade_freq_counter >= mt_cpufreq_downgrade_freq_counter_limit)
                && (mt_cpufreq_downgrade_freq_counter_return >= mt_cpufreq_downgrade_freq_counter_return_limit))
            {
                mt_cpufreq_downgrade_freq_counter = 0;
                mt_cpufreq_downgrade_freq_counter_return = 0;
            }
        }
    }
    else
    {
        /* Reset downgrade frequency counter counter if needed */
        if(mt_cpufreq_downgrade_freq_counter < mt_cpufreq_downgrade_freq_counter_limit)
        {
            mt_cpufreq_downgrade_freq_counter++;
        }
        else
        {
            if(mt_cpufreq_downgrade_freq_counter_return < mt_cpufreq_downgrade_freq_counter_return_limit)
            {
                mt_cpufreq_downgrade_freq_counter_return++;
            }
        }

        if((mt_cpufreq_downgrade_freq_counter >= mt_cpufreq_downgrade_freq_counter_limit)
            && (mt_cpufreq_downgrade_freq_counter_return >= mt_cpufreq_downgrade_freq_counter_return_limit))
        {
            mt_cpufreq_downgrade_freq_counter = 0;
            mt_cpufreq_downgrade_freq_counter_return = 0;
        }

        /* Release frequency limit */ 
        mt_cpufreq_downgrade_freq_for_ptpod = false;
    }

    dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_downgrade_freq_counter = %d\n", mt_cpufreq_downgrade_freq_counter);
    dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_downgrade_freq_counter_return = %d\n", mt_cpufreq_downgrade_freq_counter_return);
    dprintk("mt_cpufreq_downgrade_freq_check: mt_cpufreq_ptpod_temperature_delta_counter = %d\n", mt_cpufreq_ptpod_temperature_delta_counter);

no_policy:
    mutex_unlock(&mt_cpufreq_downgrade_freq_mutex);
    return;
}
EXPORT_SYMBOL(mt_cpufreq_downgrade_freq_check);


/* Set voltage because PTP-OD modified voltage table by PMIC wrapper */
unsigned int mt_cpufreq_voltage_set_by_ptpod(unsigned int pmic_volt[], unsigned int array_size)
{
    int i;
    int ret = 1;
    unsigned long flags;
    unsigned int PMIC_WRAP_DVFS_WDATA_array[8] = {PMIC_WRAP_DVFS_WDATA0, PMIC_WRAP_DVFS_WDATA1, PMIC_WRAP_DVFS_WDATA2,
                                                  PMIC_WRAP_DVFS_WDATA3, PMIC_WRAP_DVFS_WDATA4, PMIC_WRAP_DVFS_WDATA5,
                                                  PMIC_WRAP_DVFS_WDATA6, PMIC_WRAP_DVFS_WDATA7};
	/* PMIC External Buck */
	unsigned int RegtoVolt;

    if(array_size > (sizeof(mt_cpufreq_pmic_volt)/4))
    {
        dprintk("mt_cpufreq_voltage_set_by_ptpod: ERROR!array_size is invalide, array_size = %d\n", array_size);
    }

	/* PMIC external buck */
	if(is_ext_buck_exist() == 0)
	{
		spin_lock_irqsave(&mt_cpufreq_lock, flags);
		
	    /* Update voltage setting by PTPOD request. */
	    for (i = 0; i < array_size; i++)
	    {
	        #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT

	        if ((mt_cpufreq_earlysuspend_allow_deepidle_control_vproc == true) && (i == 6))
	        {
	            /* early suspend need to sync 4th volt to 6th volt for deep idle resume */
				if(g_cpufreq_get_ptp_level == 0)
	            	mt_cpufreq_reg_write(pmic_volt[4], PMIC_WRAP_DVFS_WDATA_array[6]);
				else if(g_cpufreq_get_ptp_level == 1)
					mt_cpufreq_reg_write(pmic_volt[4], PMIC_WRAP_DVFS_WDATA_array[6]);
				else if(g_cpufreq_get_ptp_level == 2)
					mt_cpufreq_reg_write(pmic_volt[2], PMIC_WRAP_DVFS_WDATA_array[6]);
				else
					mt_cpufreq_reg_write(pmic_volt[4], PMIC_WRAP_DVFS_WDATA_array[6]);
	        }
	        else
	        {			
	        	mt_cpufreq_reg_write(pmic_volt[i], PMIC_WRAP_DVFS_WDATA_array[i]);
	        }
			
	        mt_cpufreq_pmic_volt[i] = pmic_volt[i];
			dprintk("mt_cpufreq_voltage_set_by_ptpod: pmic_volt[%d] = %d\n", i, pmic_volt[i]);
			
	        #else

			/* For MT6323 only, VMIN is 1.0V, VMAX is 1.15V */
			if(pmic_volt[i] < 0x30)
	        {
	            pmic_volt[i] = 0x30;
	            dprintk("mt_cpufreq_voltage_set_by_ptpod: PTPOD voltage < 1.0V, force pmic_volt[%d] = %d\n", i, pmic_volt[i]);
	        }
			else if(pmic_volt[i] > 0x48) /* DVFS_VCORE_1p1 = 1.15V */
	        {
	            pmic_volt[i] = 0x48; /* DVFS_VCORE_1p1 = 1.15V */
	            dprintk("mt_cpufreq_voltage_set_by_ptpod: PTPOD voltage > 1.15V, force pmic_volt[%d] = %d\n", i, pmic_volt[i]);
	        }
			else
	        {
	            dprintk("mt_cpufreq_voltage_set_by_ptpod: 1.0V =< PTPOD voltage =< 1.15V, pmic_volt[%d] = %d\n", i, pmic_volt[i]);
			}

	        if ((mt_cpufreq_earlysuspend_allow_deepidle_control_vproc == true) && (i == 6))
	        {
	            /* early suspend need to sync 4th volt to 6th volt for deep idle resume */
				if(g_cpufreq_get_ptp_level == 0)
	            	mt_cpufreq_reg_write(pmic_volt[4], PMIC_WRAP_DVFS_WDATA_array[6]);
				else if(g_cpufreq_get_ptp_level == 1)
					mt_cpufreq_reg_write(pmic_volt[4], PMIC_WRAP_DVFS_WDATA_array[6]);
				else if(g_cpufreq_get_ptp_level == 2)
					mt_cpufreq_reg_write(pmic_volt[2], PMIC_WRAP_DVFS_WDATA_array[6]);
				else
					mt_cpufreq_reg_write(pmic_volt[4], PMIC_WRAP_DVFS_WDATA_array[6]);
	        }
	        else
	        {	
	        	mt_cpufreq_reg_write(pmic_volt[i], PMIC_WRAP_DVFS_WDATA_array[i]);
	        }
			
	        mt_cpufreq_pmic_volt[i] = pmic_volt[i];
			dprintk("mt_cpufreq_voltage_set_by_ptpod: pmic_volt[%d] = %d\n", i, pmic_volt[i]);
			
	        #endif
	    }

	    /* For SPM voltage setting in deep idle.*/
	    /* Need to sync PMIC_WRAP_DVFS_WDATA in mt_cpufreq_pdrv_probe() */
	    if(g_cpufreq_get_ptp_level == 0)
	    {
	        #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
	        mt_cpufreq_spm_volt[6] = pmic_volt[4];
			
	        #else
			
	        if((pmic_volt[3] <= 0x48) && (pmic_volt[3] >= 0x30)) /* VCORE VMAX <= 1.15V, VMIN >= 1.0V */
	        {
	            mt_cpufreq_spm_volt[6] = pmic_volt[4];
	        }
			
	        #endif
	    }
	    else if(g_cpufreq_get_ptp_level == 1)  
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
	        mt_cpufreq_spm_volt[6] = pmic_volt[4];
			
			#else
			
	        if((pmic_volt[3] <= 0x48) && (pmic_volt[3] >= 0x30)) /* VCORE VMAX <= 1.15V, VMIN >= 1.0V */
	        {
	            mt_cpufreq_spm_volt[6] = pmic_volt[4];
	        }
			
			#endif
	    }
	    else if(g_cpufreq_get_ptp_level == 2)  
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
	        mt_cpufreq_spm_volt[6] = pmic_volt[2];
			
			#else
			
	        if((pmic_volt[3] <= 0x48) && (pmic_volt[3] >= 0x30)) /* VCORE VMAX <= 1.15V, VMIN >= 1.0V */
	        {
	            mt_cpufreq_spm_volt[6] = pmic_volt[2];
	        }
			
			#endif
	    }
	    else
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
	        mt_cpufreq_spm_volt[6] = pmic_volt[4];
			
			#else
			
	        if((pmic_volt[3] <= 0x48) && (pmic_volt[3] >= 0x30)) /* VCORE VMAX <= 1.15V, VMIN >= 1.0V */
	        {
	            mt_cpufreq_spm_volt[6] = pmic_volt[4];
	        }
			
			#endif
	    }
		
	    for (i = 0; i < (sizeof(mt_cpufreq_pmic_volt)/4); i++)
	    {
	        dprintk("mt_cpufreq_pmic_volt[%d] = %x\n", i, mt_cpufreq_pmic_volt[i]);
			dprintk("mt_cpufreq_spm_volt[%d] = %x\n", i, mt_cpufreq_spm_volt[i]);
	    }

		ret = mt_cpufreq_volt_set(g_cur_cpufreq_OPPidx);
		if(ret == 0)
		{
			dprintk("mt_cpufreq_voltage_set_by_ptpod: I2C return fail, ret = %d\n", ret);
		}
		
		spin_unlock_irqrestore(&mt_cpufreq_lock, flags);
	}
	else
	{
		mutex_lock(&mt_cpufreq_mutex);
		
		if(is_fan53555_exist() == 1)
		{
	    	for (i = 0; i < array_size; i++)
	    	{
				RegtoVolt = (700 * 100) + (pmic_volt[i] * 625);
				
				mt_cpufreq_pmic_volt[i] = mt_cpufreq_ext_buck_vproc_mapping(RegtoVolt);

				dprintk("mt_cpufreq_voltage_set_by_ptpod: fan53555 mt_cpufreq_pmic_volt[%d] = %d\n", i, mt_cpufreq_pmic_volt[i]);
	    	}		
		}
		else if(is_ncp6335_exist() == 1)
		{
	    	for (i = 0; i < array_size; i++)
	    	{
				RegtoVolt = (700 * 100) + (pmic_volt[i] * 625);
				
				mt_cpufreq_pmic_volt[i] = mt_cpufreq_ext_buck_vproc_mapping(RegtoVolt);

				dprintk("mt_cpufreq_voltage_set_by_ptpod: ncp6335 mt_cpufreq_pmic_volt[%d] = %d\n", i, mt_cpufreq_pmic_volt[i]);
	    	}
		}
		else
		{
	    	for (i = 0; i < array_size; i++)
	    	{
				RegtoVolt = (700 * 100) + (pmic_volt[i] * 625);
				
				mt_cpufreq_pmic_volt[i] = mt_cpufreq_ext_buck_vproc_mapping(RegtoVolt);

				dprintk("mt_cpufreq_voltage_set_by_ptpod: not suppot, mt_cpufreq_pmic_volt[%d] = %d\n", i, mt_cpufreq_pmic_volt[i]);
	    	}
		}		

		ret = mt_cpufreq_volt_set(g_cur_cpufreq_OPPidx);
		if(ret == 0)
		{
			dprintk("mt_cpufreq_voltage_set_by_ptpod: I2C return fail, ret = %d\n", ret);
		}
		
		mutex_unlock(&mt_cpufreq_mutex);
	}
	
    return 0;
}
EXPORT_SYMBOL(mt_cpufreq_voltage_set_by_ptpod);

/* Look for MAX frequency in number of DVS. */
unsigned int mt_cpufreq_max_frequency_by_DVS(unsigned int num)
{
    #if 1

    if(num < mt_cpu_freqs_num)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PTPOD:num = %d, frequency= %d\n", num, mt_cpu_freqs[num].cpufreq_khz);
        return mt_cpu_freqs[num].cpufreq_khz;
    }

	#else
    int voltage_change_num = 0;
	int i = 0;

    /* Assume mt6592_freqs_e1 voltage will be put in order, and freq will be put from high to low.*/
    if(num == voltage_change_num)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PTPOD0:num = %d, frequency= %d\n", num, mt_cpu_freqs[0].cpufreq_khz);
        return mt_cpu_freqs[0].cpufreq_khz;	
    }
	
    for (i = 1; i < mt_cpu_freqs_num; i++)
    {
        if(mt_cpu_freqs[i].cpufreq_volt != mt_cpu_freqs[i-1].cpufreq_volt)
            voltage_change_num++;
		
        if(num == voltage_change_num)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PTPOD1:num = %d, frequency= %d\n", num, mt_cpu_freqs[i].cpufreq_khz);
			return mt_cpu_freqs[i].cpufreq_khz;
        }
    }
	#endif
	
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "PTPOD:num = %d, NOT found! return 0!\n", num);
    return 0;
}
EXPORT_SYMBOL(mt_cpufreq_max_frequency_by_DVS);


static void mt_cpufreq_power_calculation(int index, int ncpu)
{
    int multi = 0, p_dynamic = 0, p_leakage = 0, freq_ratio = 0, volt_square_ratio = 0;
    int possiblecpu = 0;

    possiblecpu = num_possible_cpus();
	
    volt_square_ratio = (((mt_cpu_freqs[index].cpufreq_volt * 100) / 1000) * ((mt_cpu_freqs[index].cpufreq_volt * 100) / 1000)) / 100;
    freq_ratio = (mt_cpu_freqs[index].cpufreq_khz / 1700);
    dprintk("freq_ratio = %d, volt_square_ratio %d\n", freq_ratio, volt_square_ratio);
	
    multi = ((mt_cpu_freqs[index].cpufreq_volt * 100) / 1000) * ((mt_cpu_freqs[index].cpufreq_volt * 100) / 1000) * ((mt_cpu_freqs[index].cpufreq_volt * 100) / 1000);

    switch (ncpu)
    {
        case 0:
            // 1 core
            p_dynamic = (((A_1_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 7 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 1:
            // 2 core
            p_dynamic = (((A_2_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 6 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 2:
            // 3 core
            p_dynamic = (((A_3_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 5 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 3:
            // 4 core
            p_dynamic = (((A_4_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 4 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 4:
            // 5 core
            p_dynamic = (((A_5_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 3 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 5:
            // 6 core
            p_dynamic = (((A_6_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 2 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 6:
            // 7 core
            p_dynamic = (((A_7_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 1 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        case 7:
            // 8 core
            p_dynamic = (((A_8_CORE * freq_ratio) / 1000) * volt_square_ratio) / 100;
            p_leakage = ((P_TOTAL_CORE_L - 0 * P_EACH_CORE_L) * (multi)) / (100 * 100 * 100);
            dprintk("p_dynamic = %d, p_leakage = %d\n", p_dynamic, p_leakage);
            break;
        default:
            break;
    }
	
	mt_cpu_power[index * possiblecpu + ncpu].cpufreq_ncpu = ncpu + 1;
	mt_cpu_power[index * possiblecpu + ncpu].cpufreq_khz = mt_cpu_freqs[index].cpufreq_khz;
	mt_cpu_power[index * possiblecpu + ncpu].cpufreq_power = p_dynamic + p_leakage;

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpu_power[%d]: cpufreq_ncpu = %d, cpufreq_khz = %d, cpufreq_power = %d\n", (index * possiblecpu + ncpu), 
                mt_cpu_power[index * possiblecpu + ncpu].cpufreq_ncpu,
                mt_cpu_power[index * possiblecpu + ncpu].cpufreq_khz,
                mt_cpu_power[index * possiblecpu + ncpu].cpufreq_power);

}


static unsigned int mt_cpufreq_power_table_config[8] = {0,0,1,0,1,0,1,0};

static void mt_setup_power_table(int num)
{
    int i = 0, j = 0;
    struct mt_cpu_power_info temp_power_info;
    int possiblecpu = 0;
    unsigned int mt_cpufreq_power_efficiency[20][10];
	unsigned int mt_cpufreq_power_efficiency_num = 0;
	
    dprintk("P_MCU_D = %d\n", P_MCU_D);  

    dprintk("P_CA7_D_1_CORE = %d, P_CA7_D_2_CORE = %d, P_CA7_D_3_CORE = %d, P_CA7_D_4_CORE = %d\n", 
             P_CA7_D_1_CORE, P_CA7_D_2_CORE, P_CA7_D_3_CORE, P_CA7_D_4_CORE);

    dprintk("P_TOTAL_CORE_L = %d, P_EACH_CORE_L = %d\n", 
             P_TOTAL_CORE_L, P_EACH_CORE_L);

    dprintk("A_1_CORE = %d, A_2_CORE = %d, A_3_CORE = %d, A_4_CORE = %d\n", 
             A_1_CORE, A_2_CORE, A_3_CORE, A_4_CORE);

    possiblecpu = num_possible_cpus();

    memset( (void *)mt_cpufreq_power_efficiency, 0, sizeof(unsigned int)*20*10 );
	
    mt_cpu_power = kzalloc((num * possiblecpu) * sizeof(struct mt_cpu_power_info), GFP_KERNEL);

    /* Init power table to 0 */
    for (i = 0; i < num; i++)
    {
        for (j = 0; j < possiblecpu; j++)
        {
            mt_cpu_power[i * possiblecpu + j].cpufreq_ncpu = 0;
            mt_cpu_power[i * possiblecpu + j].cpufreq_khz = 0;
            mt_cpu_power[i * possiblecpu + j].cpufreq_power = 0;   
        }
    }

#if 1
    /* Setup power efficiency array */
    for (i = 0; i < possiblecpu; i++)
    {
		if(mt_cpufreq_power_table_config[i] == 1)
		{
			mt_cpufreq_power_efficiency_num++;
		}     
    }

    for (i = 0; i < num; i++)
    {
        for (j = 0; j < possiblecpu; j++)
        {
            if(mt_cpufreq_power_table_config[j] == 1)
            {
                mt_cpufreq_power_efficiency[i][j] = 1;
            } 
        }
    }

	g_cpu_power_table_num = (num * possiblecpu) - (num * mt_cpufreq_power_efficiency_num); /* Need to check, if condition num change.*/
#endif


    /* Calculate power and fill in power table */		
    for (i = 0; i < num; i++)
    {
        for (j = 0; j < possiblecpu; j++)
        {
            if(mt_cpufreq_power_efficiency[i][j] == 0)
                mt_cpufreq_power_calculation(i, j);
        }
    }

    /* Sort power table */
    for (i = (num * possiblecpu - 1); i > 0; i--)
    {
        for (j = 1; j <= i; j++)
        {
            if (mt_cpu_power[j - 1].cpufreq_power < mt_cpu_power[j].cpufreq_power)
            {
                temp_power_info.cpufreq_khz = mt_cpu_power[j - 1].cpufreq_khz;
                temp_power_info.cpufreq_ncpu = mt_cpu_power[j - 1].cpufreq_ncpu;
                temp_power_info.cpufreq_power = mt_cpu_power[j - 1].cpufreq_power;

                mt_cpu_power[j - 1].cpufreq_khz = mt_cpu_power[j].cpufreq_khz;
                mt_cpu_power[j - 1].cpufreq_ncpu = mt_cpu_power[j].cpufreq_ncpu;
                mt_cpu_power[j - 1].cpufreq_power = mt_cpu_power[j].cpufreq_power;

                mt_cpu_power[j].cpufreq_khz = temp_power_info.cpufreq_khz;
                mt_cpu_power[j].cpufreq_ncpu = temp_power_info.cpufreq_ncpu;
                mt_cpu_power[j].cpufreq_power = temp_power_info.cpufreq_power;
            }
        }
    }

    for (i = 0; i < (num * possiblecpu); i++)
    {
        dprintk("mt_cpu_power[%d].cpufreq_khz = %d, ", i, mt_cpu_power[i].cpufreq_khz);
        dprintk("mt_cpu_power[%d].cpufreq_ncpu = %d, ", i, mt_cpu_power[i].cpufreq_ncpu);
        dprintk("mt_cpu_power[%d].cpufreq_power = %d\n", i, mt_cpu_power[i].cpufreq_power);
    }

    #ifdef CONFIG_THERMAL
        mtk_cpufreq_register(mt_cpu_power, g_cpu_power_table_num);
    #endif
}

/***********************************************
* register frequency table to cpufreq subsystem
************************************************/
static int mt_setup_freqs_table(struct cpufreq_policy *policy, struct mt_cpu_freq_info *freqs, int num)
{
    struct cpufreq_frequency_table *table;
    int i, ret;

    if(mt_cpufreq_freq_table_allocated == false)
    {
        table = kzalloc((num + 1) * sizeof(*table), GFP_KERNEL);
        if (table == NULL)
            return -ENOMEM;

        for (i = 0; i < num; i++) {
            table[i].index = i;
            table[i].frequency = freqs[i].cpufreq_khz;
        }
        table[num].index = i;
        table[num].frequency = CPUFREQ_TABLE_END;

        mt_cpu_freqs = freqs;
        mt_cpu_freqs_num = num;
        mt_cpu_freqs_table = table;
	
        mt_cpufreq_freq_table_allocated = true;
    }

    ret = cpufreq_frequency_table_cpuinfo(policy, mt_cpu_freqs_table);
    if (!ret)
        cpufreq_frequency_table_get_attr(mt_cpu_freqs_table, policy->cpu);

    if (mt_cpu_power == NULL)
        mt_setup_power_table(num);

    return 0;
}

/*****************************
* set SOC DVFS status
******************************/
int mt_socfreq_state_set(int enabled)
{
    if (enabled)
    {
        if (!mt_socfreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "socfreq already enabled\n");
            return 0;
        }

        /*************
        * enable SOC DVFS
        **************/
        g_soc_dvfs_disable_count--;
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "enable SOC DVFS: g_soc_dvfs_disable_count = %d\n", g_soc_dvfs_disable_count);

        /***********************************************
        * enable SOC DVFS if no any module still disable it
        ************************************************/
        if (g_soc_dvfs_disable_count <= 0)
        {
            mt_socfreq_pause = false;
			mt_soc_dvfs(SOC_DVFS_TYPE_PAUSE, 0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "someone still disable socfreq, cannot enable it\n");
        }
    }
    else
    {
        /**************
        * disable SOC DVFS
        ***************/
        g_soc_dvfs_disable_count++;
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "disable SOC DVFS: g_dvfs_disable_count = %d\n", g_soc_dvfs_disable_count);

        if (mt_socfreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "socfreq already disabled\n");
            return 0;
        }

        mt_socfreq_pause = true;
		mt_soc_dvfs(SOC_DVFS_TYPE_PAUSE, 1);
    }

    return 0;
}
EXPORT_SYMBOL(mt_socfreq_state_set);

/*****************************
* set CPU DVFS status
******************************/
int mt_cpufreq_state_set(int enabled)
{
    if (enabled)
    {
        if (!mt_cpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "cpufreq already enabled\n");
            return 0;
        }

        /*************
        * enable DVFS
        **************/
        g_dvfs_disable_count--;
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "enable DVFS: g_dvfs_disable_count = %d\n", g_dvfs_disable_count);

        /***********************************************
        * enable DVFS if no any module still disable it
        ************************************************/
        if (g_dvfs_disable_count <= 0)
        {
            mt_cpufreq_pause = false;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "someone still disable cpufreq, cannot enable it\n");
        }
    }
    else
    {
        /**************
        * disable DVFS
        ***************/
        g_dvfs_disable_count++;
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "disable DVFS: g_dvfs_disable_count = %d\n", g_dvfs_disable_count);

        if (mt_cpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "cpufreq already disabled\n");
            return 0;
        }

        mt_cpufreq_pause = true;
    }

    return 0;
}
EXPORT_SYMBOL(mt_cpufreq_state_set);

static int mt_cpufreq_verify(struct cpufreq_policy *policy)
{
    dprintk("call mt_cpufreq_verify!\n");
    return cpufreq_frequency_table_verify(policy, mt_cpu_freqs_table);
}

static unsigned int mt_cpufreq_get(unsigned int cpu)
{
    dprintk("call mt_cpufreq_get: %d!\n", g_cur_freq);
    return g_cur_freq;
}

static void mt_cpu_clock_switch(unsigned int sel)
{
    unsigned int ckmuxsel = 0;

    ckmuxsel = DRV_Reg32(TOP_CKMUXSEL) & ~0xC;

    switch (sel)
    {
        case TOP_CKMUXSEL_CLKSQ:
            mt_cpufreq_reg_write((ckmuxsel | 0x00), TOP_CKMUXSEL);
            break;
        case TOP_CKMUXSEL_ARMPLL:
            mt_cpufreq_reg_write((ckmuxsel | 0x04), TOP_CKMUXSEL);
            break;
        case TOP_CKMUXSEL_MAINPLL:
            mt_cpufreq_reg_write((ckmuxsel | 0x08), TOP_CKMUXSEL);
            break;
        case TOP_CKMUXSEL_UNIVPLL:
            mt_cpufreq_reg_write((ckmuxsel | 0x0C), TOP_CKMUXSEL);
            break;
        default:
            break;
    }
}

/* Need sync with mt_cpufreq_volt_to_pmic_wrap(), mt_cpufreq_pdrv_probe() */
static int mt_cpufreq_volt_set(unsigned int newOPPidx)
{
	int ret = 1;
	
	if(g_cpufreq_get_ptp_level == 0)
	{
		switch (newOPPidx)
		{
			case 0:
	            if(is_ext_buck_exist()==0)
	            {
	                dprintk("switch to DVFS_V0_ptp0: %d mV\n", DVFS_V0_ptp0);
	                spm_dvfs_ctrl_volt(0);
	            } 
	            else
	            {
	                dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 1:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V1_ptp1: %d mV\n", DVFS_V1_ptp1);
	                spm_dvfs_ctrl_volt(1);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
				    ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 2:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V2_ptp2: %d mV\n", DVFS_V2_ptp2);
	                spm_dvfs_ctrl_volt(2);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 3:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V3_ptp3: %d mV\n", DVFS_V3_ptp3);
	                spm_dvfs_ctrl_volt(3);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 4:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp4: %d mV\n", DVFS_V4_ptp4);
	                spm_dvfs_ctrl_volt(4);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;

			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V5_ptp5: %d mV\n", DVFS_V5_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V6_ptp6: %d mV\n", DVFS_V6_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			
			#else
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp5: %d mV\n", DVFS_V4_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp6: %d mV\n", DVFS_V4_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
				
			#endif


			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			case 7:
				dprintk("switch to DVFS_VCORE_1p1: %d mV\n", DVFS_VCORE_1p1);
	            spm_dvfs_ctrl_volt(7);
				break;
			case 8:
				if(is_ext_buck_exist() == 0)
				{
					dprintk("switch to DVFS_VCORE_1p0: should not happened\n");
				}
				else
				{
					dprintk("switch to DVFS_VCORE_1p0: %d\n", DVFS_V4_ptp4);
					/* PTPOD would not set mt6323 register, so it is as default 1.0V */
					spm_dvfs_ctrl_volt(4);
				}
				break;
			#endif
			
			default:
				break;
		}
	}
	else if(g_cpufreq_get_ptp_level == 1)
	{
		switch (newOPPidx)
		{
			case 0:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V0_ptp0: %d mV\n", DVFS_V0_ptp0);
	                spm_dvfs_ctrl_volt(0);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 1:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V1_0_ptp1: %d mV\n", DVFS_V1_0_ptp1);
	                spm_dvfs_ctrl_volt(1);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 2:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V2_ptp2: %d mV\n", DVFS_V2_ptp2);
	                spm_dvfs_ctrl_volt(2);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 3:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V3_ptp3: %d mV\n", DVFS_V3_ptp3);
	                spm_dvfs_ctrl_volt(3);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 4:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp4: %d mV\n", DVFS_V4_ptp4);
	                spm_dvfs_ctrl_volt(4);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;

			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V5_ptp5: %d mV\n", DVFS_V5_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V6_ptp6: %d mV\n", DVFS_V6_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			
			#else
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp5: %d mV\n", DVFS_V4_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp6: %d mV\n", DVFS_V4_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
				
			#endif


			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			case 7:
				dprintk("switch to DVFS_VCORE_1p1: %d mV\n", DVFS_VCORE_1p1);
	            spm_dvfs_ctrl_volt(7);
				break;
			case 8:
				if(is_ext_buck_exist() == 0)
				{
					dprintk("switch to DVFS_VCORE_1p0: should not happened\n");
				}
				else
				{
					dprintk("switch to DVFS_VCORE_1p0: %d\n", DVFS_V4_ptp4);
					/* PTPOD would not set mt6323 register, so it is as default 1.0V */
					spm_dvfs_ctrl_volt(4);				
				}
				break;
			#endif
			
			default:
				break;
		}
	}
	else if(g_cpufreq_get_ptp_level == 2)  
	{
		switch (newOPPidx)
		{
			case 0:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to : %d mV\n", DVFS_V0_ptp0);
	                spm_dvfs_ctrl_volt(0);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 1:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to : %d mV\n", DVFS_V1_1_ptp1);
	                spm_dvfs_ctrl_volt(1);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 2:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to : %d mV\n", DVFS_V1_ptp1);
	                spm_dvfs_ctrl_volt(2);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 3:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to : %d mV\n", DVFS_V3_0_ptp3);
	                spm_dvfs_ctrl_volt(3);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 4:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to : %d mV\n", DVFS_V4_ptp4);
	                spm_dvfs_ctrl_volt(4);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;

			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V5_ptp5: %d mV\n", DVFS_V5_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V6_ptp6: %d mV\n", DVFS_V6_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			
			#else
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp5: %d mV\n", DVFS_V4_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp6: %d mV\n", DVFS_V4_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
				
			#endif


			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			case 7:
				dprintk("switch to DVFS_VCORE_1p1: %d mV\n", DVFS_VCORE_1p1);
	            spm_dvfs_ctrl_volt(7);
				break;
			case 8:
				if(is_ext_buck_exist() == 0)
				{
					dprintk("switch to DVFS_VCORE_1p0: should not happened\n");
				}
				else
				{
					dprintk("switch to DVFS_VCORE_1p0: %d\n", DVFS_V4_ptp4);
					/* PTPOD would not set mt6323 register, so it is as default 1.0V */
					spm_dvfs_ctrl_volt(4);				
				}
				break;
			#endif
			
			default:
				break;
		}
	}
	else
	{
		switch (newOPPidx)
		{
			case 0:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V0_ptp0: %d mV\n", DVFS_V0_ptp0);
	                spm_dvfs_ctrl_volt(0);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 1:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V1_ptp1: %d mV\n", DVFS_V1_ptp1);
	                spm_dvfs_ctrl_volt(1);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 2:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V2_ptp2: %d mV\n", DVFS_V2_ptp2);
	                spm_dvfs_ctrl_volt(2);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret= ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 3:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V3_ptp3: %d mV\n", DVFS_V3_ptp3);
	                spm_dvfs_ctrl_volt(3);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 4:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp4: %d mV\n", DVFS_V4_ptp4);
	                spm_dvfs_ctrl_volt(4);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;

			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V5_ptp5: %d mV\n", DVFS_V5_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V6_ptp6: %d mV\n", DVFS_V6_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			
			#else
			
			case 5:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp5: %d mV\n", DVFS_V4_ptp5);
	                spm_dvfs_ctrl_volt(5);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
			case 6:
	            if(is_ext_buck_exist()==0)
	            {
	            	dprintk("switch to DVFS_V4_ptp6: %d mV\n", DVFS_V4_ptp6);
	                spm_dvfs_ctrl_volt(6);
	            }
	            else
	            {
	            	dprintk("external buck %d : mt_cpufreq_pmic_volt[%d]: %d mV\n", pmic_external_buck_used, newOPPidx, mt_cpufreq_pmic_volt[newOPPidx]);
	                ret = ext_buck_vosel(mt_cpufreq_pmic_volt[newOPPidx]);
					return ret;
	            }
				break;
				
			#endif


			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			case 7:
				dprintk("switch to DVFS_VCORE_1p1: %d mV\n", DVFS_VCORE_1p1);
	            spm_dvfs_ctrl_volt(7);
				break;
			case 8:
				if(is_ext_buck_exist() == 0)
				{
					dprintk("switch to DVFS_VCORE_1p0: should not happened\n");
				}
				else
				{
					dprintk("switch to DVFS_VCORE_1p0: %d\n", DVFS_V4_ptp4);
					/* PTPOD would not set mt6323 register, so it is as default 1.0V */
					spm_dvfs_ctrl_volt(4);				
				}
				break;
			#endif
			
			default:
				break;

		}
	}

    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	#else
    g_cur_vcore = dvfs_vcore_config[newOPPidx].dvfs_vcore;
    #endif

	return ret;
}

/* Set freq to max in probe function */
static int mt_cpufreq_set_initial(unsigned int freq_new, unsigned int new_cpu_idx)
{
	unsigned int armpll = 0;
    int ret = 1;
	
    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    #else
	unsigned int new_vcore_idx;

	if(is_ext_buck_exist() == 0)
	{
		new_vcore_idx = mt_get_max_vcore(new_cpu_idx, g_cur_socfreq_OPPidx);
	}
	else
	{
		new_vcore_idx = new_cpu_idx;
	}
	
    #endif

	
	if (freq_new >= 1001000)
	{
		armpll = 0x8009A000;
		armpll = armpll + ((freq_new - 1001000) / 13000) * 0x2000;
	}
	else if (freq_new >= 793000)
	{
		armpll = 0x810F4000;
		armpll = armpll + ((freq_new - 793000) / 6500) * 0x2000;
	}
	else
	{
		dprintk("mt_cpufreq_set: ERROR frequency can not < 793MHz, freq_new = %d !\n", freq_new);
		return 0;
	}
	
	/* YP comment no need to call enable/disable pll. mainpll will always on until suspend. */
	//enable_pll(MAINPLL, "CPU_DVFS");
    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	
	ret = mt_cpufreq_volt_set(new_cpu_idx);
	if(ret == 0)
	{
		dprintk("mt_cpufreq_set: I2C return fail, could not set freq up, ret = %d\n", ret);
		return 0;
	}
	udelay(PMIC_SETTLE_TIME);
	
    #else
	
	ret = mt_cpufreq_volt_set(new_vcore_idx);
	if(ret == 0)
	{
		dprintk("mt_cpufreq_set: I2C return fail, could not set freq up, ret = %d\n", ret);
		return 0;
	}
	udelay(PMIC_SETTLE_TIME);
	
    #endif

	
    mt_cpufreq_reg_write(0x0A, TOP_CKDIV1_CPU);
    mt_cpu_clock_switch(TOP_CKMUXSEL_MAINPLL);
	
    mt_cpufreq_reg_write(armpll, ARMPLL_CON1);
	
    mb();
	udelay(PLL_SETTLE_TIME);
	
    mt_cpu_clock_switch(TOP_CKMUXSEL_ARMPLL);
    mt_cpufreq_reg_write(0x00, TOP_CKDIV1_CPU);

	//disable_pll(MAINPLL, "CPU_DVFS");

	return 1;
}

/*****************************************
* frequency ramp up and ramp down handler
******************************************/
/***********************************************************
* [note]
* 1. frequency ramp up need to wait voltage settle
* 2. frequency ramp down do not need to wait voltage settle
************************************************************/
static void mt_cpufreq_set(unsigned int freq_old, unsigned int freq_new, unsigned int target_volt, unsigned int new_cpu_idx)
{
	int ret = 1, set_init_ret = 0;
	unsigned int freq_new_keep;
		
	#ifdef CPUFREQ_HIGHEST_TURBO_MODE
	int online_cpu = 0;
	#endif
	
	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT

	#else
	unsigned int new_vcore_idx;

	//cur_vcore = mt_get_cur_vcore();
	if(is_ext_buck_exist() == 0)
	{
		new_vcore_idx = mt_get_max_vcore(new_cpu_idx, g_cur_socfreq_OPPidx);
	}
	else
	{
		new_vcore_idx = new_cpu_idx;
	}
	
	#endif

	freq_new_keep = freq_new;
	
	/* PMIC External Buck */
	if(is_ext_buck_sw_ready() == 0)
	{
		xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "ext buck sw not ready\n");
		return;
	}

	/* Set max freq in probe fail, need to set to max freq again */
	if(mt_cpufreq_freq_set_initial == false)
	{
		xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "Set max freq in probe fail, need to set to max freq again!\n");
		set_init_ret = mt_cpufreq_set_initial(g_max_freq_by_ptp, 0);
		if(set_init_ret == 1)
		{
			g_cur_freq = g_max_freq_by_ptp;  /* Default set to max */
			//g_cur_cpufreq_volt = DVFS_V0_ptp0; /* Default set to max */
			g_cur_cpufreq_OPPidx = 0; /*  Default set to max */
			mt_cpufreq_freq_set_initial = true;
		}
		else
		{
			xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "Set max freq not success!\n");
		}
		return;
	}
	
	if (freq_new > freq_old)
	{
	    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
		
	    ret = mt_cpufreq_volt_set(new_cpu_idx);
		if(ret == 0)
		{
			dprintk("mt_cpufreq_set: I2C return fail, could not set volt up, ret = %d\n", ret);
			return;
		}
	    udelay(PMIC_SETTLE_TIME);
		
	    #else
		
	    ret = mt_cpufreq_volt_set(new_vcore_idx);
		if(ret == 0)
		{
			dprintk("mt_cpufreq_set: I2C return fail, could not set volt up, ret = %d\n", ret);
			return;
		}
	    udelay(PMIC_SETTLE_TIME);
		
		#endif

		#ifdef CPUFREQ_HIGHEST_TURBO_MODE
		if((g_max_freq_by_ptp < DVFS_F0)||(g_cpufreq_turbo_mode_efuse_on_off == 1))
		{		
		}
		else
		{
			if(freq_new == mt_cpu_freqs[0].cpufreq_khz)
			{
				online_cpu = num_online_cpus();
				if((online_cpu == 3)||(online_cpu == 4))
				{
					freq_new = freq_new + g_cpufreq_turbo_mode_efuse_4_core;
				}
				else if((online_cpu == 1)||(online_cpu == 2))
				{
					freq_new = freq_new + g_cpufreq_turbo_mode_efuse_2_core;
				}

				g_cur_freq_highest = freq_new;

				dprintk("mt_cpufreq_set: [turbo mode] scaling up, freq_old = %d, freq_new = %d\n", freq_old, freq_new);
			}
		}
		#endif
			
		if((freq_new > FHCTL_CHANGE_FREQ) && (freq_old > FHCTL_CHANGE_FREQ))
		{
			mt_dfs_armpll(freq_old, freq_new);
		}
		else if((freq_new < FHCTL_CHANGE_FREQ) && (freq_old < FHCTL_CHANGE_FREQ)) /* This will not happen in real case. */
		{
	        //mt_cpufreq_reg_write(0x0A, TOP_CKDIV1_CPU); /* Already div 2 */
	        mt_dfs_armpll((freq_old * 2), (freq_new * 2));
		}
		else
		{
	        mt_dfs_armpll((freq_old * 2), freq_new);
	        mt_cpufreq_reg_write(0x00, TOP_CKDIV1_CPU);
		}
		
		dprintk("=== FHCTL: freq_new = %d > freq_old = %d ===\n", freq_new, freq_old);
		//dprintk("=== FHCTL: freq meter = %d ===\n", mt_get_cpu_freq());
	}
	#ifdef CPUFREQ_HIGHEST_TURBO_MODE
	else if(freq_new == freq_old)
	{
		if((g_max_freq_by_ptp < DVFS_F0)||(g_cpufreq_turbo_mode_efuse_on_off == 1))
		{		
		}
		else
		{
			if(freq_new == mt_cpu_freqs[0].cpufreq_khz)
			{
				online_cpu = num_online_cpus();
				if((online_cpu == 3)||(online_cpu == 4))
				{
					freq_new = freq_new + g_cpufreq_turbo_mode_efuse_4_core;
				}
				else if((online_cpu == 1)||(online_cpu == 2))
				{
					freq_new = freq_new + g_cpufreq_turbo_mode_efuse_2_core;
				}

				if (freq_new == g_cur_freq_highest)
				{
					dprintk("mt_cpufreq_set: [turbo mode] equal, freq_new == g_cur_freq_highest = %d, return\n", g_cur_freq_highest);
					return;
				}
				
				freq_old = g_cur_freq_highest;
				
				mt_dfs_armpll(freq_old, freq_new);
				
				g_cur_freq_highest = freq_new;

				dprintk("mt_cpufreq_set: [turbo mode] equal, freq_old = %d, freq_new = %d\n", freq_old, freq_new);
			}
		}	
	}
	#endif
	else
	{
		#ifdef CPUFREQ_HIGHEST_TURBO_MODE
		if((g_max_freq_by_ptp < DVFS_F0)||(g_cpufreq_turbo_mode_efuse_on_off == 1))
		{		
		}
		else
		{
			if(freq_old == mt_cpu_freqs[0].cpufreq_khz)
			{
				freq_old = g_cur_freq_highest;

				dprintk("mt_cpufreq_set: [turbo mode] scaling down, freq_old = %d, freq_new = %d\n", freq_old, freq_new);
			}
		}
		#endif

		if((freq_new > FHCTL_CHANGE_FREQ) && (freq_old > FHCTL_CHANGE_FREQ))
		{
			mt_dfs_armpll(freq_old, freq_new);
		}
		else if((freq_new < FHCTL_CHANGE_FREQ) && (freq_old < FHCTL_CHANGE_FREQ))
		{
	        //mt_cpufreq_reg_write(0x0A, TOP_CKDIV1_CPU); /* Already div 2 */
	        mt_dfs_armpll((freq_old * 2), (freq_new * 2));
		}
		else
		{
	        mt_cpufreq_reg_write(0x0A, TOP_CKDIV1_CPU);
	        mt_dfs_armpll(freq_old, (freq_new * 2));
		}
		
	    dprintk("=== FHCTL: freq_new = %d < freq_old = %d ===\n", freq_new, freq_old);
	    //dprintk("=== FHCTL: freq meter = %d ===\n", mt_get_cpu_freq());

	    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT		
		ret = mt_cpufreq_volt_set(new_cpu_idx);
		if(ret == 0)
		{
			g_cur_freq = freq_new_keep;
			g_cur_cpufreq_OPPidx = new_cpu_idx;
			dprintk("mt_cpufreq_set: I2C return fail, could not set volt down, ret = %d\n", ret);
			return;
		}
	    #else
	    ret = mt_cpufreq_volt_set(new_vcore_idx);
		if(ret == 0)
		{
			g_cur_freq = freq_new_keep;
			g_cur_cpufreq_OPPidx = new_cpu_idx;
			dprintk("mt_cpufreq_set: I2C return fail, could not set volt down, ret = %d\n", ret);
			return;
		}
	    #endif
	}


    g_cur_freq = freq_new_keep;
	//g_cur_cpufreq_volt = target_volt;
	g_cur_cpufreq_OPPidx = new_cpu_idx;
	
    dprintk("ARMPLL_CON0 = 0x%x, ARMPLL_CON1 = 0x%x, g_cur_freq = %d\n", DRV_Reg32(ARMPLL_CON0), DRV_Reg32(ARMPLL_CON1), g_cur_freq);
}

/**************************************
* check if maximum frequency is needed
***************************************/
static int mt_cpufreq_keep_max_freq(unsigned int freq_old, unsigned int freq_new)
{
    if (mt_cpufreq_pause)
        return 1;

    /* check if system is going to ramp down */
    if (freq_new < freq_old)
        g_ramp_down_count++;
    else
        g_ramp_down_count = 0;

    if (g_ramp_down_count < RAMP_DOWN_TIMES)
        return 1;

    return 0;
}

#ifdef MT_DVFS_RANDOM_TEST
static int mt_cpufreq_idx_get(int num)
{
    int random = 0, mult = 0, idx;
    random = jiffies & 0xF;

    while (1)
    {
        if ((mult * num) >= random)
        {
            idx = (mult * num) - random;
            break;
        }
        mult++;
    }
    return idx;
}
#endif

static unsigned int mt_cpufreq_cpu_power_limited_verify(unsigned int target_freq)
{
    int i = 0, index = 0, online_cpu = 0;
	int found = 0;
	int check_limit = 0;
	
    online_cpu = num_online_cpus();
	
	dprintk("mt_cpufreq_cpu_power_limited_verify: begin, target_freq = %d, online_cpu = %d\n", target_freq, online_cpu);

	for (i = 0; i < MT_CPUFREQ_POWER_LIMITED_MAX_NUM; i++)
	{
		if (mt_cpufreq_limited_cpu_power_array[i] != 0)
		{
			check_limit = 1;
		}
	}
	
    if(check_limit == 0)
        return target_freq;
	
    for (i = 0; i < (mt_cpu_freqs_num * MT_CPUFREQ_POSSIBLE_CPU); i++)
    {
        if (mt_cpu_power[i].cpufreq_ncpu == g_limited_max_ncpu && mt_cpu_power[i].cpufreq_khz == g_limited_max_freq)
        {
            index = i;
            break;
        }
    }

	dprintk("mt_cpufreq_cpu_power_limited_verify: index = %d, g_limited_max_ncpu = %d, g_limited_max_freq = %d\n", index, g_limited_max_ncpu, g_limited_max_freq);

    /* For MT6592 power table, ther is no 3,5,7 cpu case. */
	if((online_cpu == 3) || (online_cpu == 5) || (online_cpu == 7))
	{
		online_cpu = online_cpu + 1;
		dprintk("mt_cpufreq_cpu_power_limited_verify: online cpu 3,5,7 not in power table, increase 1, online_cpu = %d\n", online_cpu);
	}
		
    for (index = i; index < (mt_cpu_freqs_num * MT_CPUFREQ_POSSIBLE_CPU); index++)
    {
        if (mt_cpu_power[index].cpufreq_ncpu == online_cpu)
        {
            if (target_freq >= mt_cpu_power[index].cpufreq_khz)
            {
                dprintk("mt_cpufreq_cpu_power_limited_verify: Found freq => index = %d, target_freq = %d, online_cpu = %d\n", index, mt_cpu_power[index].cpufreq_khz, online_cpu);
                target_freq = mt_cpu_power[index].cpufreq_khz;
				found = 1;
                break;
            }
        }
    }

	if(found == 0)
	{
		target_freq = g_limited_max_freq;
		dprintk("mt_cpufreq_cpu_power_limited_verify: Not found freq, set to g_limited_max_freq = %d\n", g_limited_max_freq);
	}
	
	dprintk("mt_cpufreq_cpu_power_limited_verify: End, target_freq = %d, , online_cpu = %d\n", target_freq, online_cpu);
    return target_freq;
}

/**********************************
* cpufreq target callback function
***********************************/
/*************************************************
* [note]
* 1. handle frequency change request
* 2. call mt_cpufreq_set to set target frequency
**************************************************/
static int mt_cpufreq_target(struct cpufreq_policy *policy, unsigned int target_freq, unsigned int relation)
{
    int i, idx, newOPPidx;
    unsigned int cpu;
    unsigned long flags;

    struct mt_cpu_freq_info next;
    struct cpufreq_freqs freqs;

    if (!mt_cpufreq_ready)
        return -ENOSYS;

    if (policy->cpu >= num_possible_cpus())
        return -EINVAL;

    /******************************
    * look up the target frequency
    *******************************/
    if (cpufreq_frequency_table_target(policy, mt_cpu_freqs_table, target_freq, relation, &idx))
    {
        return -EINVAL;
    }

    #ifdef MT_DVFS_RANDOM_TEST
    idx = mt_cpufreq_idx_get(7);
    #endif

	/* PMIC external buck */
	if(is_ext_buck_exist() == 0)
	{
	    if(g_cpufreq_get_ptp_level == 0)
	        next.cpufreq_khz = mt6592_freqs_e1[idx].cpufreq_khz;
	    else if(g_cpufreq_get_ptp_level == 1)
	        next.cpufreq_khz = mt6592_freqs_e1_1[idx].cpufreq_khz;
	    else if(g_cpufreq_get_ptp_level == 2)
	        next.cpufreq_khz = mt6592_freqs_e1_2[idx].cpufreq_khz;
	    else
	        next.cpufreq_khz = mt6592_freqs_e1[idx].cpufreq_khz;
	}
	else
	{
		if(is_fan53555_exist() == 1)
		{
			if(g_cpufreq_get_ptp_level == 0)
				next.cpufreq_khz = mt6592_freqs_fan53555_e1[idx].cpufreq_khz;
			else if(g_cpufreq_get_ptp_level == 1)
				next.cpufreq_khz = mt6592_freqs_fan53555_e1_1[idx].cpufreq_khz;
			else if(g_cpufreq_get_ptp_level == 2)
				next.cpufreq_khz = mt6592_freqs_fan53555_e1_2[idx].cpufreq_khz;
			else
				next.cpufreq_khz = mt6592_freqs_fan53555_e1[idx].cpufreq_khz;
		}
		else if(is_ncp6335_exist() == 1)
		{
		    if(g_cpufreq_get_ptp_level == 0)
		        next.cpufreq_khz = mt6592_freqs_e1[idx].cpufreq_khz;
		    else if(g_cpufreq_get_ptp_level == 1)
		        next.cpufreq_khz = mt6592_freqs_e1_1[idx].cpufreq_khz;
		    else if(g_cpufreq_get_ptp_level == 2)
		        next.cpufreq_khz = mt6592_freqs_e1_2[idx].cpufreq_khz;
		    else
		        next.cpufreq_khz = mt6592_freqs_e1[idx].cpufreq_khz;		
		}
		else
		{
		    if(g_cpufreq_get_ptp_level == 0)
		        next.cpufreq_khz = mt6592_freqs_e1[idx].cpufreq_khz;
		    else if(g_cpufreq_get_ptp_level == 1)
		        next.cpufreq_khz = mt6592_freqs_e1_1[idx].cpufreq_khz;
		    else if(g_cpufreq_get_ptp_level == 2)
		        next.cpufreq_khz = mt6592_freqs_e1_2[idx].cpufreq_khz;
		    else
		        next.cpufreq_khz = mt6592_freqs_e1[idx].cpufreq_khz;		

			dprintk("external buck not support\n");
		}
	}

    #ifdef MT_DVFS_RANDOM_TEST
    dprintk("idx = %d, freqs.old = %d, freqs.new = %d\n", idx, policy->cur, next.cpufreq_khz);
    #endif

    freqs.old = policy->cur;
    freqs.new = next.cpufreq_khz;
    freqs.cpu = policy->cpu;

    #ifndef MT_DVFS_RANDOM_TEST
    if (mt_cpufreq_keep_max_freq(freqs.old, freqs.new))
    {
        freqs.new = policy->max;
    }

    /**************************************
    * Search for min freq of HEVC needed
    ***************************************/
	if(g_limited_freq_by_hevc != 0)
	{
	    for (i = (mt_cpu_freqs_num - 1); i >= 0; i--)
	    {
	        if (mt_cpu_freqs[i].cpufreq_khz >= g_limited_freq_by_hevc)
	        {
				freqs.new = mt_cpu_freqs[i].cpufreq_khz;
				dprintk("HEVC limit freq! freqs.new = %d\n", freqs.new, i, mt_cpu_freqs[i].cpufreq_khz);
	            break;
	        }
	    }
	}
	
	#ifdef PTPOD_DOWNGRADE_FREQ
	/************************************************
	* DVFS keep at second freq when downgrade freq for ptpod
	*************************************************/
	if (mt_cpufreq_downgrade_freq_for_ptpod == true)
	{
		if(freqs.new > mt_cpufreq_downgrade_freq)
		{
			freqs.new = mt_cpufreq_downgrade_freq;
			dprintk("Downgrade for PTPOD! limit freq <= %d, freqs.new = %d\n", mt_cpufreq_downgrade_freq, freqs.new);
		}	
	}
	#endif
	
	/*  TOBEDONE */
    #ifdef MT_DVFS_BRINGUP
	freqs.new = DVFS_F1;
    #endif

    /**************************************
    * Search for thermal limited freq
    ***************************************/
    freqs.new = mt_cpufreq_cpu_power_limited_verify(freqs.new);

    if (freqs.new < g_limited_min_freq)
    {
        dprintk("cannot switch CPU frequency to %d Mhz due to voltage limitation\n", g_limited_min_freq / 1000);
        freqs.new = g_limited_min_freq;
    }
    #endif

    /************************************************
    * DVFS keep at 1.15Ghz/1.0V when PTPOD initial 
    *************************************************/
    if (mt_cpufreq_ptpod_disable)
    {
    	if(g_cpufreq_get_ptp_level == 0)
        	freqs.new = DVFS_F4;
		else if(g_cpufreq_get_ptp_level == 1)
			freqs.new = DVFS_F4;
		else if(g_cpufreq_get_ptp_level == 2)
			freqs.new = DVFS_F6;
		else
			freqs.new = DVFS_F4;
		
        dprintk("PTPOD, freqs.new = %d\n", freqs.new);
    }

    /************************************************
    * DVFS keep at 1.15GHz/1.0V in earlysuspend when max freq overdrive.
    *************************************************/
    if(mt_cpufreq_limit_max_freq_early_suspend == true)
    {
        freqs.new = DVFS_F4;
        dprintk("mt_cpufreq_limit_max_freq_early_suspend, freqs.new = %d\n", freqs.new);
    }

    #if 0
    /************************************************
    * If MT6333 not support and ISP_VDEC on,
    * DVFS can only higher than 1.05Ghz/1.15V when 4 online cpu, for power consumption.
    *************************************************/
    #ifndef MT_DVFS_LOW_VOLTAGE_SUPPORT
    if((num_online_cpus() < 4) && (freqs.new > DVFS_F2))
    {
        if(isp_vdec_on_off() == true)
        {
            dprintk("Limited frequency, because num_online_cpus() = %d, freqs.new = %d\n", num_online_cpus(), freqs.new);
            freqs.new = DVFS_F2;
        }
    }
    #endif
    #endif

    /************************************************
    * target frequency == existing frequency, skip it
    *************************************************/
    if (freqs.old == freqs.new)
    {
    	#ifdef CPUFREQ_HIGHEST_TURBO_MODE

		if((g_max_freq_by_ptp < DVFS_F0)||(g_cpufreq_turbo_mode_efuse_on_off == 1))
		{
			dprintk("CPU frequency from %d MHz to %d MHz (skipped) due to same frequency\n", freqs.old / 1000, freqs.new / 1000);
			return 0;		
		}
		else
		{
			if((freqs.new == mt_cpu_freqs[0].cpufreq_khz) && (g_mt_cpufreq_hotplug_notify_change == 1))
			{
				dprintk("Highest CPU frequency and cpu number from %d to %d \n", g_prev_freq_target_online_cpu, g_cur_freq_target_online_cpu);
			}
			else
			{
				dprintk("CPU frequency from %d MHz to %d MHz (skipped) due to same frequency\n", freqs.old / 1000, freqs.new / 1000);
				return 0;
			}
		}
		
		#else
		
        dprintk("CPU frequency from %d MHz to %d MHz (skipped) due to same frequency\n", freqs.old / 1000, freqs.new / 1000);
        return 0;
		
		#endif
    }
	
    /**************************************
    * search for the corresponding voltage
    ***************************************/
    next.cpufreq_volt = 0;

    for (i = 0; i < mt_cpu_freqs_num; i++)
    {
        dprintk("freqs.new = %d, mt_cpu_freqs[%d].cpufreq_khz = %d\n", freqs.new, i, mt_cpu_freqs[i].cpufreq_khz);
        if (freqs.new == mt_cpu_freqs[i].cpufreq_khz)
        {
            next.cpufreq_volt = mt_cpu_freqs[i].cpufreq_volt;
			newOPPidx = i;
            dprintk("next.cpufreq_volt = %d, mt_cpu_freqs[%d].cpufreq_volt = %d\n", next.cpufreq_volt, i, mt_cpu_freqs[i].cpufreq_volt);
            break;
        }
    }

    if (next.cpufreq_volt == 0)
    {
        dprintk("Error!! Cannot find corresponding voltage at %d Mhz\n", freqs.new / 1000);
        return 0;
    }

    for_each_online_cpu(cpu)
    {
        freqs.cpu = cpu;
        cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
    }

    /* PMIC External Buck */
	if(is_ext_buck_exist()==0)
    {
		spin_lock_irqsave(&mt_cpufreq_lock, flags);  
		
		/******************************
		* set to the target freeuency
		*******************************/
		mt_cpufreq_set(freqs.old, freqs.new, next.cpufreq_volt, newOPPidx);

		spin_unlock_irqrestore(&mt_cpufreq_lock, flags);
	}
	else
	{
    	mutex_lock(&mt_cpufreq_mutex);

		/******************************
		* set to the target freeuency
		*******************************/
		mt_cpufreq_set(freqs.old, freqs.new, next.cpufreq_volt, newOPPidx);
	
		mutex_unlock(&mt_cpufreq_mutex);
	}

    for_each_online_cpu(cpu)
    {
        freqs.cpu = cpu;
        cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
    }

    return 0;
}

/*********************************************************
* set up frequency table and register to cpufreq subsystem
**********************************************************/
static int mt_cpufreq_init(struct cpufreq_policy *policy)
{
    int ret = -EINVAL;

    if (policy->cpu >= num_possible_cpus())
        return -EINVAL;

    policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
    cpumask_setall(policy->cpus);

    /*******************************************************
    * 1 us, assumed, will be overwrited by min_sampling_rate
    ********************************************************/
    policy->cpuinfo.transition_latency = 1000;

    /*********************************************
    * set default policy and cpuinfo, unit : Khz
    **********************************************/
    policy->cpuinfo.max_freq = g_max_freq_by_ptp;
    policy->cpuinfo.min_freq = DVFS_F6;

    policy->cur = g_max_freq_by_ptp;  /* Default set to max */
    policy->max = g_max_freq_by_ptp;
    policy->min = DVFS_F6;

    if(mt_gpufreqs == NULL)
        mt_setup_gpufreqs_table();

	/* PMIC external buck */
	if(is_ext_buck_exist() == 0)
	{
	    if(g_cpufreq_get_ptp_level == 0)
	        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1));
	    else if(g_cpufreq_get_ptp_level == 1)
	        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1_1));
	    else if(g_cpufreq_get_ptp_level == 2)
	        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1_2));
	    else
	        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1));
	}
	else
	{
		if(is_fan53555_exist() == 1)
		{
		    if(g_cpufreq_get_ptp_level == 0)
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_fan53555_e1));
		    else if(g_cpufreq_get_ptp_level == 1)
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_fan53555_e1_1));
		    else if(g_cpufreq_get_ptp_level == 2)
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_fan53555_e1_2));
		    else
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_fan53555_e1));		
		}
		else if(is_ncp6335_exist() == 1)
		{
		    if(g_cpufreq_get_ptp_level == 0)
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1));
		    else if(g_cpufreq_get_ptp_level == 1)
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1_1));
		    else if(g_cpufreq_get_ptp_level == 2)
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1_2));
		    else
		        ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1));		
		}
		else
		{
			if(g_cpufreq_get_ptp_level == 0)
				ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1));
			else if(g_cpufreq_get_ptp_level == 1)
				ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1_1));
			else if(g_cpufreq_get_ptp_level == 2)
				ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1_2));
			else
				ret = mt_setup_freqs_table(policy, ARRAY_AND_SIZE(mt6592_freqs_e1));
		}
	}

    if (ret) {
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "failed to setup frequency table\n");
        return ret;
    }
	
    return 0;
}

static struct freq_attr *mt_cpufreq_attr[] = {
    &cpufreq_freq_attr_scaling_available_freqs,
    NULL,
};

static struct cpufreq_driver mt_cpufreq_driver = {
    .verify = mt_cpufreq_verify,
    .target = mt_cpufreq_target,
    .init   = mt_cpufreq_init,
    .get    = mt_cpufreq_get,
    .name   = "mt-cpufreq",
    .attr	= mt_cpufreq_attr,
};

/*********************************
* early suspend callback function
**********************************/
void mt_cpufreq_early_suspend(struct early_suspend *h)
{
	int i = 0;
	
    #ifndef MT_DVFS_RANDOM_TEST

    mt_cpufreq_state_set(0);

    mt_cpufreq_limit_max_freq_early_suspend = true;
    mt_cpufreq_limit_max_freq_by_early_suspend();

	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	
	/* Switch for SPM usage */
	mt_cpufreq_reg_write(mt_cpufreq_spm_volt[6], PMIC_WRAP_DVFS_WDATA6);
	mt_cpufreq_reg_write(mt_cpufreq_spm_volt[7], PMIC_WRAP_DVFS_WDATA7);
	
	#else
	if(g_soc_final_mask == 0)
	{
		/* Switch for SPM usage */
		mt_cpufreq_reg_write(mt_cpufreq_spm_volt[6], PMIC_WRAP_DVFS_WDATA6);
		mt_cpufreq_reg_write(mt_cpufreq_spm_volt[7], PMIC_WRAP_DVFS_WDATA7);	
	}
	#endif
	
    /* Deep idle could control vproc now. */
    mt_cpufreq_earlysuspend_allow_deepidle_control_vproc = true;
    #endif

	for (i = 0; i < SOC_DVFS_TYPE_NUM; i++)
	{
		Soc2printk("mt_cpufreq_early_suspend: mt6592_soc_dvfs_config[%d] = %d\n", i, mt6592_soc_dvfs_config[i]);
	}

    return;
}

/*******************************
* late resume callback function
********************************/
void mt_cpufreq_late_resume(struct early_suspend *h)
{
    #ifndef MT_DVFS_RANDOM_TEST
    /* Deep idle could NOT control vproc now. */
    mt_cpufreq_earlysuspend_allow_deepidle_control_vproc = false;

	/* Switch back for CPU usage */
	mt_cpufreq_reg_write(mt_cpufreq_pmic_volt[6], PMIC_WRAP_DVFS_WDATA6);
	mt_cpufreq_reg_write(mt_cpufreq_pmic_volt[7], PMIC_WRAP_DVFS_WDATA7);

    mt_cpufreq_limit_max_freq_early_suspend = false;

    mt_cpufreq_state_set(1);

    #endif

    return;
}

/************************************************
* API to switch back default voltage setting for PTPOD disabled
*************************************************/
void mt_cpufreq_return_default_DVS_by_ptpod(void)
{
	/* PMIC external buck */
	if(is_ext_buck_exist() == 0)
	{
		pmic_external_buck_used = 0;
			
	    if(g_cpufreq_get_ptp_level == 0)
	    {
	        #if defined(HQA_LV_1_09V)
	            mt_cpufreq_reg_write(0x50, PMIC_WRAP_DVFS_WDATA0); // 1.20V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA1); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x3D, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC
	            mt_cpufreq_reg_write(0x38, PMIC_WRAP_DVFS_WDATA3); // 1.05V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA4); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA5); // 1.15V VPROC, for spm control in deep idle
	            mt_cpufreq_reg_write(0x38, PMIC_WRAP_DVFS_WDATA6); // 1.05V VPROC, for spm control in deep idle
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA7); // 1.15V VPROC, for spm control in deep idle
			
	            /* For PTP-OD */
	            mt_cpufreq_pmic_volt[0] = 0x50; // 1.20V VPROC
	            mt_cpufreq_pmic_volt[1] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[2] = 0x3D; // 1.09V VPROC
	            mt_cpufreq_pmic_volt[3] = 0x38; // 1.05V VPROC
	            mt_cpufreq_pmic_volt[4] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[5] = 0x48; // 1.15V VPROC, for spm control in deep idle
	            mt_cpufreq_pmic_volt[6] = 0x38; // 1.05V VPROC, for spm control in deep idle
	            mt_cpufreq_pmic_volt[7] = 0x48; // 1.15V VPROC, for spm control in deep idle
	        #elif defined(HQA_NV_1_15V)
	            mt_cpufreq_reg_write(0x5A, PMIC_WRAP_DVFS_WDATA0); // 1.26V VPROC
	            mt_cpufreq_reg_write(0x50, PMIC_WRAP_DVFS_WDATA1); // 1.20V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA2); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x38, PMIC_WRAP_DVFS_WDATA3); // 1.05V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA4); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA5); // 1.15V VPROC, for spm control in deep idle
	            mt_cpufreq_reg_write(0x38, PMIC_WRAP_DVFS_WDATA6); // 1.05V VPROC, for spm control in deep idle
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA7); // 1.15V VPROC, for spm control in deep idle
			
	            /* For PTP-OD */
	            mt_cpufreq_pmic_volt[0] = 0x5A; // 1.26V VPROC
	            mt_cpufreq_pmic_volt[1] = 0x50; // 1.20V VPROC
	            mt_cpufreq_pmic_volt[2] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[3] = 0x38; // 1.05V VPROC
	            mt_cpufreq_pmic_volt[4] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[5] = 0x48; // 1.15V VPROC, for spm control in deep idle
	            mt_cpufreq_pmic_volt[6] = 0x38; // 1.05V VPROC, for spm control in deep idle
	            mt_cpufreq_pmic_volt[7] = 0x48; // 1.15V VPROC, for spm control in deep idle
	        #elif defined(HQA_HV_1_21V)
	            mt_cpufreq_reg_write(0x64, PMIC_WRAP_DVFS_WDATA0); // 1.32V VPROC
	            mt_cpufreq_reg_write(0x52, PMIC_WRAP_DVFS_WDATA1); // 1.20V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA2); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x38, PMIC_WRAP_DVFS_WDATA3); // 1.05V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA4); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA5); // 1.15V VPROC, for spm control in deep idle
	            mt_cpufreq_reg_write(0x38, PMIC_WRAP_DVFS_WDATA6); // 1.05V VPROC, for spm control in deep idle
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA7); // 1.15V VPROC, for spm control in deep idle
			
	            /* For PTP-OD */
	            mt_cpufreq_pmic_volt[0] = 0x64; // 1.32V VPROC
	            mt_cpufreq_pmic_volt[1] = 0x52; // 1.20V VPROC
	            mt_cpufreq_pmic_volt[2] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[3] = 0x38; // 1.05V VPROC
	            mt_cpufreq_pmic_volt[4] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[5] = 0x48; // 1.15V VPROC, for spm control in deep idle
	            mt_cpufreq_pmic_volt[6] = 0x38; // 1.05V VPROC, for spm control in deep idle
	            mt_cpufreq_pmic_volt[7] = 0x48; // 1.15V VPROC, for spm control in deep idle
			#else /* Normal case */
			
	            #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09V VPROC (1.09375)
	            mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.06V VPROC (1.0625)
	            mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03V VPROC (1.03125)
	            mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
	            mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
	            mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC
	            if(g_soc_avs_type == 0)
	            	mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
				else
					mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
				
	            /* For PTP-OD */
	            mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09V VPROC (1.09375)
	            mt_cpufreq_pmic_volt[2] = 0x3A; // 1.06V VPROC (1.0625)
	            mt_cpufreq_pmic_volt[3] = 0x35; // 1.03V VPROC (1.03125)
	            mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
	            mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
	            mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC
	            if(g_soc_avs_type == 0)
	            	mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
				else
					mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance

				mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
				mt_cpufreq_spm_volt[7] = 0x0; // 0.70V VPROC, for spm control in deep idle

	            #else
	            mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
	            mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09V VPROC (1.09375)
	            mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.06V VPROC (1.0625)
	            mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03V VPROC (1.03125)
	            mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
	            mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
	            mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
	            if(g_soc_avs_type == 0)
	            	mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
				else
					mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
				
	            /* For PTP-OD */
	            mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
	            mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09V VPROC (1.09375)
	            mt_cpufreq_pmic_volt[2] = 0x3A; // 1.06V VPROC (1.0625)
	            mt_cpufreq_pmic_volt[3] = 0x35; // 1.03V VPROC (1.03125)
	            mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
	            mt_cpufreq_pmic_volt[5] = 0x30; // 1.00V VPROC
	            mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC
	            if(g_soc_avs_type == 0)
	            	mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
				else
					mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
					
				mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
				mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle

	            #endif
				
	        #endif
	    }
	    else if(g_cpufreq_get_ptp_level == 1)
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x40, PMIC_WRAP_DVFS_WDATA1); // 1.10V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA3); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
			mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
			
			/* For PTP-OD */
			mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
			mt_cpufreq_pmic_volt[1] = 0x40; // 1.10V VPROC
			mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09V VPROC (1.09375)
			mt_cpufreq_pmic_volt[3] = 0x3A; // 1.06V VPROC (1.0625)
			mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
			mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x0; // 0.70V VPROC, for spm control in deep idle
			
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x40, PMIC_WRAP_DVFS_WDATA1); // 1.10V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA3); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
			
			/* For PTP-OD */
			mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
			mt_cpufreq_pmic_volt[1] = 0x40; // 1.10V VPROC
			mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09V VPROC (1.09375)
			mt_cpufreq_pmic_volt[3] = 0x3A; // 1.06V VPROC (1.0625)
			mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[5] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
			
			#endif

	    }
	    else if(g_cpufreq_get_ptp_level == 2)
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x43, PMIC_WRAP_DVFS_WDATA1); // 1.12V VPROC (1.11875)
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x37, PMIC_WRAP_DVFS_WDATA3); // 1.04V VPROC (1.04375)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
			mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
			
			/* For PTP-OD */
			mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
			mt_cpufreq_pmic_volt[1] = 0x43; // 1.12V VPROC (1.11875)
			mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09V VPROC (1.09375)
			mt_cpufreq_pmic_volt[3] = 0x37; // 1.04V VPROC (1.04375)
			mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
			mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x0; // 0.70V VPROC, for spm control in deep idle
			
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x43, PMIC_WRAP_DVFS_WDATA1); // 1.12V VPROC (1.11875)
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x37, PMIC_WRAP_DVFS_WDATA3); // 1.04V VPROC (1.04375)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
			
			/* For PTP-OD */
			mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
			mt_cpufreq_pmic_volt[1] = 0x43; // 1.12V VPROC (1.11875)
			mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09V VPROC (1.09375)
			mt_cpufreq_pmic_volt[3] = 0x37; // 1.04V VPROC (1.04375)
			mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[5] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
			
			#endif

	    }
	    else
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03V VPROC (1.03125)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
			mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
			
			/* For PTP-OD */
			mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
			mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09V VPROC (1.09375)
			mt_cpufreq_pmic_volt[2] = 0x3A; // 1.06V VPROC (1.0625)
			mt_cpufreq_pmic_volt[3] = 0x35; // 1.03V VPROC (1.03125)
			mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
			mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x0; // 0.70V VPROC, for spm control in deep idle
			
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03V VPROC (1.03125)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance
			
			/* For PTP-OD */
			mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
			mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09V VPROC (1.09375)
			mt_cpufreq_pmic_volt[2] = 0x3A; // 1.06V VPROC (1.0625)
			mt_cpufreq_pmic_volt[3] = 0x35; // 1.03V VPROC (1.03125)
			mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[5] = 0x30; // 1.00V VPROC
			mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
			
			#endif

	    }
	}
	else
	{
		if(g_cpufreq_get_ptp_level == 0)
		{
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03V VPROC (1.03125)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance

			/* For PTP-OD */
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle

			#endif
		}
	    else if(g_cpufreq_get_ptp_level == 1)
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x40, PMIC_WRAP_DVFS_WDATA1); // 1.10V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA3); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance

			/* For PTP-OD */
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle

			#endif
		}
	    else if(g_cpufreq_get_ptp_level == 2)
	    {
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x43, PMIC_WRAP_DVFS_WDATA1); // 1.12V VPROC (1.11875)
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x37, PMIC_WRAP_DVFS_WDATA3); // 1.04V VPROC (1.04375)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance

			/* For PTP-OD */
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle

			#endif
		}
		else
		{
			#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
			#else
			mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
			mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09V VPROC (1.09375)
			mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.06V VPROC (1.0625)
			mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03V VPROC (1.03125)
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA5); // 1.00V VPROC
			mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA6); // 1.00V VPROC
			if(g_soc_avs_type == 0)
				mt_cpufreq_reg_write(0x44, PMIC_WRAP_DVFS_WDATA7); // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA7); // 1.075V VPROC, for SOC DVFS high performance

			/* For PTP-OD */
			if(g_soc_avs_type == 0)
				mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			else
				mt_cpufreq_pmic_volt[7] = 0x3C; // 1.075V VPROC, for SOC DVFS high performance
			
			mt_cpufreq_spm_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
			mt_cpufreq_spm_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle

			#endif
		}
		
		if(is_fan53555_exist() == 1)
		{
			pmic_external_buck_used = 1;
			
			if(g_cpufreq_get_ptp_level == 0)
	    	{
		        #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
		        /* For PTP-OD */
		        mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
		        mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
		        mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
		        mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
		        mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
		        mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
		        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
		        //mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance

		        #else
					
		        /* For PTP-OD */
		        mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
		        mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
		        mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
		        mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
		        mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
		        mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
		        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
		        //mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
		            
		        #endif	
		    }
			else if(g_cpufreq_get_ptp_level == 1)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
				mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
				mt_cpufreq_pmic_volt[3] = 106000; // 1.06V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
					
				#else
						
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
				mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
				mt_cpufreq_pmic_volt[3] = 106000; // 1.06V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
					
				#endif	
			}
			else if(g_cpufreq_get_ptp_level == 2)
	    	{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
		        /* For PTP-OD */
		        mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
		        mt_cpufreq_pmic_volt[1] = 112000; // 1.12V VPROC
		        mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
		        mt_cpufreq_pmic_volt[3] = 104000; // 1.04V VPROC
		        mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
		        mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
		        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
		        //mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance

				#else
					
		        /* For PTP-OD */
		        mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
		        mt_cpufreq_pmic_volt[1] = 112000; // 1.12V VPROC
		        mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
		        mt_cpufreq_pmic_volt[3] = 104000; // 1.04V VPROC
		        mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
		        mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
		        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
		        //mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
		            
				#endif	
		    }
			else
	    	{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
		        /* For PTP-OD */
		        mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
		        mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
		        mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
		        mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
		        mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
		        mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
		        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
		        //mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance

				#else
					
		        /* For PTP-OD */
		        mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
		        mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
		        mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
		        mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
		        mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
		        mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
		        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
		        //mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
		            
				#endif	
		    }
		}
		else if(is_ncp6335_exist() == 1)
		{
			pmic_external_buck_used = 2;
			
			if(g_cpufreq_get_ptp_level == 0)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#else
				
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#endif	
			}
			else if(g_cpufreq_get_ptp_level == 1)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
				
				#else
					
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
				
				#endif	
			}
			else if(g_cpufreq_get_ptp_level == 2)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#else
				
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#endif	
			}
			else
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#else
				
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#endif	
			}
		}
		else
		{
			pmic_external_buck_used = 0xFF;
			
			if(g_cpufreq_get_ptp_level == 0)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#else
				
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#endif	
			}
			else if(g_cpufreq_get_ptp_level == 1)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
				
				#else
					
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
				
				#endif	
			}
			else if(g_cpufreq_get_ptp_level == 2)
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#else
				
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
				mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#endif	
			}
			else
			{
				#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#else
				
				/* For PTP-OD */
				mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
				mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
				mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
				mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
				mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
				mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
				mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC
				//mt_cpufreq_pmic_volt[7] = 0x44; // 1.125V VPROC, for SOC DVFS high performance
			
				#endif	
			}
		}
	}

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq return default DVS by ptpod\n");
}
EXPORT_SYMBOL(mt_cpufreq_return_default_DVS_by_ptpod);

/************************************************
* DVFS enable API for PTPOD
*************************************************/
void mt_cpufreq_enable_by_ptpod(void)
{
    mt_cpufreq_ptpod_disable = false;
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq enabled by ptpod\n");
}
EXPORT_SYMBOL(mt_cpufreq_enable_by_ptpod);

/************************************************
* DVFS disable API for PTPOD
*************************************************/
unsigned int mt_cpufreq_disable_by_ptpod(void)
{
    struct cpufreq_policy *policy;

    mt_cpufreq_ptpod_disable = true;

    policy = cpufreq_cpu_get(0);

    if (!policy)
        goto no_policy;

	if(g_cpufreq_get_ptp_level == 0)
    	cpufreq_driver_target(policy, DVFS_F4, CPUFREQ_RELATION_L);
	else if(g_cpufreq_get_ptp_level == 1)
		cpufreq_driver_target(policy, DVFS_F4, CPUFREQ_RELATION_L);
	else if(g_cpufreq_get_ptp_level == 2)
		cpufreq_driver_target(policy, DVFS_F6, CPUFREQ_RELATION_L);
	else
		cpufreq_driver_target(policy, DVFS_F4, CPUFREQ_RELATION_L);
		
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq disabled by ptpod, limited freq. at %d\n", DVFS_F3);

    cpufreq_cpu_put(policy);

no_policy:
    return g_cur_freq;
}
EXPORT_SYMBOL(mt_cpufreq_disable_by_ptpod);


/************************************************
* Houtplug notify
*************************************************/
unsigned int mt_cpufreq_hotplug_notify(unsigned int ncpu)
{
#ifdef CPUFREQ_HIGHEST_TURBO_MODE

    struct cpufreq_policy *policy;

	if(g_max_freq_by_ptp < DVFS_F0)
	{
		dprintk("mt_cpufreq_hotplug_notify: not support\n");
		return 0;		
	}

	/* If efuse not support, return. */
	if(g_cpufreq_turbo_mode_efuse_on_off == 1)
		return 0;

	/* If dvfs not ready, return. */
	if(mt_cpufreq_hotplug_notify_ready == false)
		return 0;

	dprintk("mt_cpufreq_hotplug_notify: [turbo mode] cpu number = %d\n", ncpu);
	
	/* If hotplug notify first time, update previous cpunumber. */
	g_cur_freq_target_online_cpu = ncpu;
	if(g_prev_freq_target_online_cpu == 0)
	{
		g_prev_freq_target_online_cpu = g_cur_freq_target_online_cpu;
	}

	dprintk("mt_cpufreq_hotplug_notify: [turbo mode] prev cpu = %d, cur cpu = %d\n", g_prev_freq_target_online_cpu, g_cur_freq_target_online_cpu);
	
	/* If cpu number change check if need to scale highest frequency 0/52/104 .*/
	if(((g_prev_freq_target_online_cpu <= 2)&&(g_cur_freq_target_online_cpu >= 3))
		|| ((g_prev_freq_target_online_cpu >= 3)&&(g_cur_freq_target_online_cpu <= 2))
		|| ((g_prev_freq_target_online_cpu <= 4)&&(g_cur_freq_target_online_cpu >= 5))
		|| ((g_prev_freq_target_online_cpu >= 5)&&(g_cur_freq_target_online_cpu <= 4)))
	{
		g_mt_cpufreq_hotplug_notify_change = 1;
	}
	else
	{
		g_mt_cpufreq_hotplug_notify_change = 0;
	}

	dprintk("mt_cpufreq_hotplug_notify: [turbo mode] g_mt_cpufreq_hotplug_notify_change = %d\n", g_mt_cpufreq_hotplug_notify_change);
	dprintk("mt_cpufreq_hotplug_notify: [turbo mode] g_cur_freq = %d\n", g_cur_freq);
	
	/* Trigger to set highest frequency. */
	if(g_max_freq_by_ptp < DVFS_F0)
	{
		g_prev_freq_target_online_cpu = g_cur_freq_target_online_cpu;
		dprintk("mt_cpufreq_hotplug_notify: not support\n");
		return 0;		
	}
	else
	{
		if(g_mt_cpufreq_hotplug_notify_change == 1)
		{
			if(g_cur_freq == mt_cpu_freqs[0].cpufreq_khz)
			{
			    policy = cpufreq_cpu_get(0);

			    if (!policy)
			        goto no_policy;

				dprintk("mt_cpufreq_hotplug_notify: [turbo mode] call cpufreq_driver_target()\n");
				
			    cpufreq_driver_target(policy, mt_cpu_freqs[0].cpufreq_khz, CPUFREQ_RELATION_L);

			    cpufreq_cpu_put(policy);
			}	

			/* Reset state */
			g_mt_cpufreq_hotplug_notify_change = 0;
		}
	}
	
no_policy:
	/* Update previous cpu number */
	g_prev_freq_target_online_cpu = g_cur_freq_target_online_cpu;

    return 0;
	
#else
	return 0;
#endif	
}
EXPORT_SYMBOL(mt_cpufreq_hotplug_notify);

void mt_cpufreq_cpu_power_protect(void)
{
    int i = 0, ncpu = 0, found = 0;
    unsigned int limited_power = 0;

    struct cpufreq_policy *policy;

	mutex_lock(&mt_cpufreq_cpu_power_mutex);

    for (i = 0; i < MT_CPUFREQ_POWER_LIMITED_MAX_NUM; i++)
    {
        if (mt_cpufreq_limited_cpu_power_array[i] != 0 && limited_power == 0)
        {
            limited_power = mt_cpufreq_limited_cpu_power_array[i];
        }
        else if (mt_cpufreq_limited_cpu_power_array[i] != 0 && limited_power != 0)
        {
            if (mt_cpufreq_limited_cpu_power_array[i] < limited_power)
            {
                limited_power = mt_cpufreq_limited_cpu_power_array[i];
            }
        }
    }

    for (i = 0; i < MT_CPUFREQ_POWER_LIMITED_MAX_NUM; i++)
    {
        dprintk("mt_cpufreq_cpu_power_protect: mt_cpufreq_limited_cpu_power_array[%d] = %d\n ", i, mt_cpufreq_limited_cpu_power_array[i]);
    }

    /* apply cpu power protection */

    policy = cpufreq_cpu_get(0);

    if (!policy)
        goto no_policy;

    ncpu = num_possible_cpus();

    if (limited_power == 0)
    {
        g_limited_max_ncpu = num_possible_cpus();
        g_limited_max_freq = g_max_freq_by_ptp;

        cpufreq_driver_target(policy, g_limited_max_freq, CPUFREQ_RELATION_L);
        #ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
        hp_limited_cpu_num(g_limited_max_ncpu);
        #endif

        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "thermal limit g_limited_max_freq = %d, g_limited_max_ncpu = %d, limited_power = %d\n", g_limited_max_freq, g_limited_max_ncpu, limited_power);
    }
    else
    {
        while (ncpu)
        {
            for (i = 0; i < (mt_cpu_freqs_num * MT_CPUFREQ_POSSIBLE_CPU); i++)
            {
                if (mt_cpu_power[i].cpufreq_ncpu == ncpu)
                {
                    if (mt_cpu_power[i].cpufreq_power <= limited_power)
                    {
                        g_limited_max_ncpu = mt_cpu_power[i].cpufreq_ncpu;
                        g_limited_max_freq = mt_cpu_power[i].cpufreq_khz;
                        #if defined(CONFIG_THERMAL_LIMIT_TEST)
                        g_limited_max_cpu_power = mt_cpu_power[i].cpufreq_power;
                        #endif
						
                        found = 1;
                        break;
                    }
                }
            }

            if (found)
                break;

            ncpu--;
        }

        if (!found)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "Not found suitable DVFS OPP, limit to lowest OPP!\n");
            g_limited_max_ncpu = mt_cpu_power[g_cpu_power_table_num - 1].cpufreq_ncpu;
            g_limited_max_freq = mt_cpu_power[g_cpu_power_table_num - 1].cpufreq_khz;
            #if defined(CONFIG_THERMAL_LIMIT_TEST)
            g_limited_max_cpu_power = mt_cpu_power[g_cpu_power_table_num - 1].cpufreq_power;
            #endif
        }

        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "thermal limit g_limited_max_freq = %d, g_limited_max_ncpu = %d, limited_power = %d\n", g_limited_max_freq, g_limited_max_ncpu, limited_power);

        #ifdef CONFIG_CPU_FREQ_GOV_HOTPLUG
        hp_limited_cpu_num(g_limited_max_ncpu);
        #endif

        cpufreq_driver_target(policy, g_limited_max_freq, CPUFREQ_RELATION_L);

    }

    cpufreq_cpu_put(policy);

no_policy:
	mutex_unlock(&mt_cpufreq_cpu_power_mutex);
    return;
}

/************************************************
* cpu power protection for thermal
*************************************************/
/******************************************************
* parameter: limited_power
*******************************************************/
void mt_cpufreq_thermal_protect(unsigned int limited_power)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq_thermal_protect: limited_power = %d\n", limited_power);

    mt_cpufreq_thermal_limited_cpu_power = limited_power;
    mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_THERMAL_LIMITED_INDEX] = mt_cpufreq_thermal_limited_cpu_power;
    mt_cpufreq_cpu_power_protect();
}
EXPORT_SYMBOL(mt_cpufreq_thermal_protect);

/************************************************
* cpu power protection for lbat_volt
*************************************************/
/******************************************************
* parameter: low_battery_level
*******************************************************/
void mt_cpufreq_lbat_volt_protect(LOW_BATTERY_LEVEL low_battery_level)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mt_cpufreq_lbat_volt_protect: low_battery_level = %d\n", low_battery_level);

	if(mt_cpufreq_lbat_volt_drop_enable == 0)
		return;
	
    mt_cpufreq_lbat_volt_limited_level = low_battery_level;

	//is_low_battery=1:need limit HW, is_low_battery=0:no limit
	//3.25V HW issue int and is_low_battery=1, 3.5V HW issue int and is_low_battery=0
    if (low_battery_level == LOW_BATTERY_LEVEL_1)
    {
        if (mt_cpufreq_lbat_volt_limited_cpu_power != mt_cpufreq_lbat_volt_limited_cpu_power_1)
        {
            mt_cpufreq_lbat_volt_limited_cpu_power = mt_cpufreq_lbat_volt_limited_cpu_power_1;
            mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_LBAT_VOLT_LIMITED_INDEX] = mt_cpufreq_lbat_volt_limited_cpu_power;
            mt_cpufreq_cpu_power_protect(); // TODO: this must modified whenever OPP table changes to fix 8x1.365

            mt_gpufreq_lbat_volt_protect(mt_cpufreq_lbat_volt_limited_gpu_power_1); //fix GPU 500Mhz
        }
    }
    else if(low_battery_level == LOW_BATTERY_LEVEL_2) //2nd LV trigger CPU Limit to under 4X 1.36G
    {
        if (mt_cpufreq_lbat_volt_limited_cpu_power != mt_cpufreq_lbat_volt_limited_cpu_power_2)
        {
            mt_cpufreq_lbat_volt_limited_cpu_power = mt_cpufreq_lbat_volt_limited_cpu_power_2;
            mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_LBAT_VOLT_LIMITED_INDEX] = mt_cpufreq_lbat_volt_limited_cpu_power;
            mt_cpufreq_cpu_power_protect();

            mt_gpufreq_lbat_volt_protect(mt_cpufreq_lbat_volt_limited_gpu_power_2); //fix GPU 500Mhz
        }
    }
    else //unlimit cpu and gpu
    {
        if (mt_cpufreq_lbat_volt_limited_cpu_power != mt_cpufreq_lbat_volt_limited_cpu_power_0)
        {
            mt_cpufreq_lbat_volt_limited_cpu_power = mt_cpufreq_lbat_volt_limited_cpu_power_0;
            mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_LBAT_VOLT_LIMITED_INDEX] = mt_cpufreq_lbat_volt_limited_cpu_power;
            mt_cpufreq_cpu_power_protect();

            mt_gpufreq_lbat_volt_protect(mt_cpufreq_lbat_volt_limited_gpu_power_0); //fix GPU 500Mhz
        }
    }
}

int mt_cpufreq_lbat_volume_protect(void)
{
    unsigned int bat_ui_percentage, ret = 0;

	if(mt_cpufreq_lbat_volume_enable == 0)
		return 0;
		
	bat_ui_percentage = bat_get_ui_percentage();

    //dprintk("mt_cpufreq_lbat_volume_protect: bat_ui_percentage=%d, LOW_POWER_LIMIT_LEVEL_1 = %d\n", bat_ui_percentage, LOW_POWER_LIMIT_LEVEL_1);

    if (bat_ui_percentage <= LOW_POWER_LIMIT_LEVEL_1)
    {
        if (mt_cpufreq_lbat_volume_limited_cpu_power != mt_cpufreq_lbat_volume_limited_cpu_power_1)
        {
            mt_cpufreq_lbat_volume_limited_cpu_power = mt_cpufreq_lbat_volume_limited_cpu_power_1;
            mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_LBAT_VOLUME_LIMITED_INDEX] = mt_cpufreq_lbat_volume_limited_cpu_power;
            mt_cpufreq_cpu_power_protect();

            mt_gpufreq_lbat_volume_protect(mt_cpufreq_lbat_volume_limited_gpu_power_1);
        }
        ret = 1;
    }
    else
    {
        if (mt_cpufreq_lbat_volume_limited_cpu_power != mt_cpufreq_lbat_volume_limited_cpu_power_0)
        {
            mt_cpufreq_lbat_volume_limited_cpu_power = mt_cpufreq_lbat_volume_limited_cpu_power_0;
            mt_cpufreq_limited_cpu_power_array[MT_CPUFREQ_LBAT_VOLUME_LIMITED_INDEX] = mt_cpufreq_lbat_volume_limited_cpu_power;
            mt_cpufreq_cpu_power_protect();

            mt_gpufreq_lbat_volume_protect(mt_cpufreq_lbat_volume_limited_gpu_power_0);
        }
        ret = 0;
    }

    return ret;
}

#if defined(CONFIG_THERMAL_LIMIT_TEST)
unsigned int mt_cpufreq_thermal_test_limited_load(void)
{
    return g_limited_load_for_cpu_power_test;
}
EXPORT_SYMBOL(mt_cpufreq_thermal_test_limited_load);
#endif


/***************************
* show SOC DVFS info
****************************/
static int mt_socfreq_dump_info_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i = 0, len = 0;
    char *p = buf;

    for (i = 0; i < (SOC_DVFS_TYPE_NUM); i++)
    {
        p += sprintf(p, "mt6592_soc_dvfs_config[%d] = %d\n", i, mt6592_soc_dvfs_config[i]);
    }
    
    p += sprintf(p, "mt_socfreq_pause = %d\n", mt_socfreq_pause);
	p += sprintf(p, "g_cur_socfreq_keep_default_state = %d\n", g_cur_socfreq_keep_default_state);
	p += sprintf(p, "g_soc_api_call_test = %d\n", g_soc_api_call_test);
	p += sprintf(p, "mt_num_possible_cpus = %d\n", mt_num_possible_cpus);
	p += sprintf(p, "g_gpu_limited_max_id = %d\n", g_gpu_limited_max_id);
	p += sprintf(p, "g_soc_final_mask = %d\n", g_soc_final_mask);
	p += sprintf(p, "g_soc_mmpll_final_mask = %d\n", g_soc_mmpll_final_mask);
	p += sprintf(p, "g_soc_vencpll_final_mask = %d\n", g_soc_vencpll_final_mask);
	p += sprintf(p, "g_soc_mempll_final_mask = %d\n", g_soc_mempll_final_mask);
	p += sprintf(p, "g_soc_DRAM_Type = %d\n", g_soc_DRAM_Type);
	p += sprintf(p, "g_soc_avs_type = %d\n", g_soc_avs_type);
	#ifdef CPUFREQ_MEMPLL_OD
	p += sprintf(p, "g_ddr_frequency_type = %d\n", g_ddr_frequency_type);
	p += sprintf(p, "g_default_soc_volt = %d\n", g_default_soc_volt);
	p += sprintf(p, "g_cur_socfreq_default_ddr_freq = %d\n", g_cur_socfreq_default_ddr_freq);
	#endif
	p += sprintf(p, "mt_socfreq_allowed_enable = %d\n", mt_socfreq_allowed_enable);
	p += sprintf(p, "mt_socfreq_ddr_allowed_enable = %d\n", mt_socfreq_ddr_allowed_enable);
	p += sprintf(p, "g_cur_soc_volt = %d\n", g_cur_soc_volt);
	p += sprintf(p, "g_cur_socfreq_gpu_freq = %d\n", g_cur_socfreq_gpu_freq);
	p += sprintf(p, "g_cur_socfreq_mm_freq = %d\n", g_cur_socfreq_mm_freq);
	p += sprintf(p, "g_cur_socfreq_ddr_freq = %d\n", g_cur_socfreq_ddr_freq);
		
    len = p - buf;
    return len;
}

/***************************
* show current SOC DVFS stauts
****************************/
static int mt_socfreq_state_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (!mt_socfreq_pause)
        p += sprintf(p, "SOC DVFS enabled\n");
    else
        p += sprintf(p, "SOC DVFS disabled\n");

    len = p - buf;
    return len;
}

/************************************
* set SOC DVFS stauts by sysfs interface
*************************************/
static ssize_t mt_socfreq_state_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 1)
        {
           mt_socfreq_state_set(1);
        }
        else if (enabled == 0)
        {
           mt_socfreq_state_set(0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/***************************
* show current SOC DVFS stauts
****************************/
static int mt_socfreq_enable_hp_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (!g_soc_gpu_enable_state)
        p += sprintf(p, "SOC DVFS high performance set\n");
    else
        p += sprintf(p, "SOC DVFS high performance not set\n");

    len = p - buf;
    return len;
}

/************************************
* set SOC DVFS stauts by sysfs interface
*************************************/
static ssize_t mt_socfreq_enable_hp_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 1)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_VENC, 1);
        }
        else if (enabled == 2)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_VENC, 0);
        }
        else if (enabled == 3)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_ZSD, 1);
        }
        else if (enabled == 4)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_ZSD, 0);
        }
        else if (enabled == 5)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_GPU_HP, 1);
        }
        else if (enabled == 6)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_GPU_HP, 0);
        }
        else if (enabled == 7)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_WIFI_DISPLAY, 1);
        }
        else if (enabled == 8)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_WIFI_DISPLAY, 0);
        }
        else if (enabled == 9)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_PAUSE, 1);
        }
        else if (enabled == 10)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_PAUSE, 0);
        }
        else if (enabled == 11)
        {
           mt_socfreq_return_default(1);
        }
        else if (enabled == 12)
        {
           mt_socfreq_return_default(0);
        }
        else if (enabled == 13)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_DISPLAY, 1);
        }
        else if (enabled == 14)
        {
           mt_soc_dvfs(SOC_DVFS_TYPE_DISPLAY, 0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/************************************
* set SOC DVFS fixed test by sysfs interface
*************************************/
static int mt_socfreq_fixed_test_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;
	
	p += sprintf(p, "g_soc_fixed_volt_state = %d, g_soc_fixed_volt_enable = %d\n", g_soc_fixed_volt_state, g_soc_fixed_volt_enable);
	p += sprintf(p, "g_soc_fixed_mmpll_state = %d, g_soc_fixed_mmpll_enable = %d\n", g_soc_fixed_mmpll_state, g_soc_fixed_mmpll_enable);
	p += sprintf(p, "g_soc_fixed_vencpll_state = %d, g_soc_fixed_vencpll_enable = %d\n", g_soc_fixed_vencpll_state, g_soc_fixed_vencpll_enable);
	p += sprintf(p, "g_soc_fixed_mempll_state = %d, g_soc_fixed_mempll_enable = %d\n", g_soc_fixed_mempll_state, g_soc_fixed_mempll_enable);
	
	len = p - buf;
	return len;
}

static ssize_t mt_socfreq_fixed_test_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int pll_volt, state, enable = 0;

    if (sscanf(buffer, "%d %d %d", &pll_volt, &state, &enable) == 3)
    {
        if (pll_volt == 0) /* Voltage */
        {
        	if(state == 1)
        	{
                g_soc_fixed_volt_state = 1;
				
				if(enable == 1)
					g_soc_fixed_volt_enable = 1;
				else
					g_soc_fixed_volt_enable = 0;
        	}
			else
			{
				g_soc_fixed_volt_state = 0;
			}

			mt_soc_dvfs(SOC_DVFS_TYPE_FIXED, 0);
        }
        else if (pll_volt == 1) /* MMPLL */
        {
        	if(state == 1)
        	{
                g_soc_fixed_mmpll_state = 1;
				
				if(enable == 1)
					g_soc_fixed_mmpll_enable = 1;
				else
					g_soc_fixed_mmpll_enable = 0;
        	}
			else
			{
				g_soc_fixed_mmpll_state = 0;
			}

			mt_soc_dvfs(SOC_DVFS_TYPE_FIXED, 0);
        }
        else if (pll_volt == 2) /* VENCPLL */
        {
        	if(state == 1)
        	{
                g_soc_fixed_vencpll_state = 1;
				
				if(enable == 1)
					g_soc_fixed_vencpll_enable = 1;
				else
					g_soc_fixed_vencpll_enable = 0;
        	}
			else
			{
				g_soc_fixed_vencpll_state = 0;
			}

			mt_soc_dvfs(SOC_DVFS_TYPE_FIXED, 0);
        }
        else if (pll_volt == 3) /* MEMPLL */
        {
        	if(state == 1)
        	{
                g_soc_fixed_mempll_state = 1;
				
				if(enable == 1)
					g_soc_fixed_mempll_enable = 1;
				else
					g_soc_fixed_mempll_enable = 0;
        	}
			else
			{
				g_soc_fixed_mempll_state = 0;
			}

			mt_soc_dvfs(SOC_DVFS_TYPE_FIXED, 0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/****************************
* show current limited freq
*****************************/
static int mt_socfreq_limited_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_gpu_limited_max_id = %d\n", g_gpu_limited_max_id);

    len = p - buf;
    return len;
}

/**********************************
* limited power for thermal protect
***********************************/
static ssize_t mt_socfreq_limited_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int power = 0;

    if (sscanf(buffer, "%u", &power) == 1)
    {
        mt_gpufreq_thermal_protect(power);
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! please provide the maximum limited power\n");
    }

    return -EINVAL;
}

/***************************
* show current soc debug status
****************************/
static int mt_socfreq_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_socfreq_debug)
        p += sprintf(p, "socfreq debug enabled\n");
    else
        p += sprintf(p, "socfreq debug disabled\n");

    len = p - buf;
    return len;
}

/***********************
* enable soc debug message
************************/
static ssize_t mt_socfreq_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int debug = 0;

    if (sscanf(buffer, "%d", &debug) == 1)
    {
        if (debug == 0) 
        {
            mt_socfreq_debug = 0;
            return count;
        }
        else if (debug == 1)
        {
            mt_socfreq_debug = 1;
            return count;
        }
        else if (debug == 2)
        {
            mt_socfreq_debug = 1;
			mt_socfreq_debug_lv2 = 1;
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
    }

    return -EINVAL;
}

/***************************
* show gpu power info
****************************/
static int mt_socfreq_gpu_power_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i = 0, len = 0;
    char *p = buf;

    for (i = 0; i < (mt_gpufreqs_num); i++)
    {
        p += sprintf(p, "mt_gpufreqs_power[%d].gpufreq_khz = %d\n", i, mt_gpufreqs_power[i].gpufreq_khz);
        p += sprintf(p, "mt_gpufreqs_power[%d].gpufreq_power = %d\n", i, mt_gpufreqs_power[i].gpufreq_power);
    }

    p += sprintf(p, "done\n");

    len = p - buf;
    return len;
}

#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
extern kal_uint32 mt6333_get_reg_value(kal_uint32 reg);

/***************************
* show current soc voltage
****************************/
static int mt_socfreq_voltage_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "socfreq_voltage = 0x%x\n", mt6333_get_reg_value(0x6C));

    len = p - buf;
    return len;
}
#endif

/***************************
* show current DVFS stauts
****************************/
static int mt_cpufreq_state_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (!mt_cpufreq_pause)
        p += sprintf(p, "DVFS enabled\n");
    else
        p += sprintf(p, "DVFS disabled\n");

    len = p - buf;
    return len;
}

/************************************
* set DVFS stauts by sysfs interface
*************************************/
static ssize_t mt_cpufreq_state_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 1)
        {
            mt_cpufreq_state_set(1);
        }
        else if (enabled == 0)
        {
            mt_cpufreq_state_set(0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/****************************
* show current limited freq
*****************************/
static int mt_cpufreq_limited_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_limited_max_freq = %d, g_limited_max_ncpu = %d\n", g_limited_max_freq, g_limited_max_ncpu);

    len = p - buf;
    return len;
}

/**********************************
* limited power for thermal protect
***********************************/
static ssize_t mt_cpufreq_limited_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int power = 0;

    if (sscanf(buffer, "%u", &power) == 1)
    {
        mt_cpufreq_thermal_protect(power);
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! please provide the maximum limited power\n");
    }

    return -EINVAL;
}

#if defined(CONFIG_THERMAL_LIMIT_TEST)
/****************************
* show limited loading for thermal protect test
*****************************/
static int mt_cpufreq_limited_load_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_limited_load_for_cpu_power_test = %d\n", g_limited_load_for_cpu_power_test);
    p += sprintf(p, "mt_cpufreq_thermal_limited_cpu_power = %d\n", mt_cpufreq_thermal_limited_cpu_power);
    p += sprintf(p, "mt_cpufreq_lbat_volt_limited_cpu_power = %d\n", mt_cpufreq_lbat_volt_limited_cpu_power);
    p += sprintf(p, "mt_cpufreq_lbat_volt_limited_gpu_power = %d\n", mt_cpufreq_lbat_volt_limited_gpu_power);
    p += sprintf(p, "g_limited_max_cpu_power = %d\n", g_limited_max_cpu_power);
    p += sprintf(p, "g_cur_freq = %d, ncpu = %d\n", g_cur_freq, num_online_cpus());
	
    len = p - buf;
    return len;
}

/**********************************
* limited loading for thermal protect test
***********************************/
static ssize_t mt_cpufreq_limited_load_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int load = 0;

    if (sscanf(buffer, "%u", &load) == 1)
    {
        g_limited_load_for_cpu_power_test = load;
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! please provide the limited load\n");
    }

    return -EINVAL;
}
#endif

/***************************
* show current debug status
****************************/
static int mt_cpufreq_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_cpufreq_debug)
        p += sprintf(p, "cpufreq debug enabled\n");
    else
        p += sprintf(p, "cpufreq debug disabled\n");

	p += sprintf(p, "g_cpufreq_get_ptp_level = %d\n", g_cpufreq_get_ptp_level);
	p += sprintf(p, "pmic_external_buck_used = %d\n", pmic_external_buck_used);
	
    len = p - buf;
    return len;
}

/***********************
* enable debug message
************************/
static ssize_t mt_cpufreq_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int debug = 0;

    if (sscanf(buffer, "%d", &debug) == 1)
    {
        if (debug == 0) 
        {
            mt_cpufreq_debug = 0;
            return count;
        }
        else if (debug == 1)
        {
            mt_cpufreq_debug = 1;
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
    }

    return -EINVAL;
}

/***************************
* show cpufreq power info
****************************/
static int mt_cpufreq_power_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i = 0, len = 0;
    char *p = buf;

    for (i = 0; i < (mt_cpu_freqs_num * MT_CPUFREQ_POSSIBLE_CPU); i++)
    {
        p += sprintf(p, "mt_cpu_power[%d].cpufreq_khz = %d\n", i, mt_cpu_power[i].cpufreq_khz);
        p += sprintf(p, "mt_cpu_power[%d].cpufreq_ncpu = %d\n", i, mt_cpu_power[i].cpufreq_ncpu);
        p += sprintf(p, "mt_cpu_power[%d].cpufreq_power = %d\n", i, mt_cpu_power[i].cpufreq_power);
    }
	
    p += sprintf(p, "done\n");

    len = p - buf;
    return len;
}

#ifdef MT_DVFS_PTPOD_TEST
/***********************
* PTPOD test
************************/
static ssize_t mt_cpufreq_ptpod_test_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enable = 0;

    if (sscanf(buffer, "%d", &enable) == 1)
    {
        if (enable == 0) 
        {
            #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
            mt_cpufreq_pmic_volt[0] = 0x40; // 1.10V VPROC
            mt_cpufreq_pmic_volt[1] = 0x40; // 1.10V VPROC
            mt_cpufreq_pmic_volt[2] = 0x40; // 1.10V VPROC
            mt_cpufreq_pmic_volt[3] = 0x30; // 1.00V VPROC
            mt_cpufreq_pmic_volt[4] = 0x20; // 0.90V VPROC
            mt_cpufreq_pmic_volt[5] = 0x40; // 1.10V VPROC, for SOC DVFS high performance
            mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
            mt_cpufreq_pmic_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
            #else
            mt_cpufreq_pmic_volt[0] = 0x40; // 1.10V VPROC
            mt_cpufreq_pmic_volt[1] = 0x40; // 1.10V VPROC
            mt_cpufreq_pmic_volt[2] = 0x40; // 1.10V VPROC
            mt_cpufreq_pmic_volt[3] = 0x30; // 1.00V VPROC
            mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
            mt_cpufreq_pmic_volt[5] = 0x40; // 1.10V VPROC, for SOC DVFS high performance
            mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
            mt_cpufreq_pmic_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
            #endif

            mt_cpufreq_voltage_set_by_ptpod(mt_cpufreq_ptpod_test, 5);
			
            return count;
        }
        else if (enable == 1)
        {
            #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
            mt_cpufreq_pmic_volt[0] = 0x3F; // 1.10V VPROC
            mt_cpufreq_pmic_volt[1] = 0x3F; // 1.10V VPROC
            mt_cpufreq_pmic_volt[2] = 0x3F; // 1.10V VPROC
            mt_cpufreq_pmic_volt[3] = 0x3F; // 1.00V VPROC
            mt_cpufreq_pmic_volt[4] = 0x3F; // 0.90V VPROC
            mt_cpufreq_pmic_volt[5] = 0x40; // 1.10V VPROC, for SOC DVFS high performance
            mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
            mt_cpufreq_pmic_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
            #else
            mt_cpufreq_pmic_volt[0] = 0x3F; // 1.10V VPROC
            mt_cpufreq_pmic_volt[1] = 0x3F; // 1.10V VPROC
            mt_cpufreq_pmic_volt[2] = 0x3F; // 1.10V VPROC
            mt_cpufreq_pmic_volt[3] = 0x3F; // 1.00V VPROC
            mt_cpufreq_pmic_volt[4] = 0x3F; // 1.00V VPROC
            mt_cpufreq_pmic_volt[5] = 0x40; // 1.10V VPROC, for SOC DVFS high performance
            mt_cpufreq_pmic_volt[6] = 0x30; // 1.00V VPROC, for spm control in deep idle
            mt_cpufreq_pmic_volt[7] = 0x20; // 0.90V VPROC, for spm control in deep idle
            #endif

            mt_cpufreq_voltage_set_by_ptpod(mt_cpufreq_ptpod_test, 5);
			
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
    }

    return -EINVAL;
}
#endif

#ifdef MT_DVFS_FFTT_TEST
/***********************
* FF TT SS voltage test
************************/
static ssize_t mt_cpufreq_fftt_test_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enable = 0;

    if (sscanf(buffer, "%d", &enable) == 1)
    {
        if (enable == 0) 
        {
        	/* PMIC external buck */
			if(is_ext_buck_exist() == 0)
			{
				if(g_cpufreq_get_ptp_level == 0)
			    {
					/* FF */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x2C, PMIC_WRAP_DVFS_WDATA0); // 0.975V VPROC
					mt_cpufreq_reg_write(0x25, PMIC_WRAP_DVFS_WDATA1); // 0.93125V VPROC
					mt_cpufreq_reg_write(0x24, PMIC_WRAP_DVFS_WDATA2); // 0.925V VPROC
					mt_cpufreq_reg_write(0x22, PMIC_WRAP_DVFS_WDATA3); // 0.9125V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA4); // 0.90V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA5); // 0.90V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x2C; // 0.975V VPROC
					mt_cpufreq_pmic_volt[1] = 0x25; // 0.93125V VPROC
					mt_cpufreq_pmic_volt[2] = 0x24; // 0.925V VPROC
					mt_cpufreq_pmic_volt[3] = 0x22; // 0.9125V VPROC
					mt_cpufreq_pmic_volt[4] = 0x20; // 0.90V VPROC
					mt_cpufreq_pmic_volt[5] = 0x20; // 0.90V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle					
					#else
					#endif
				}
				else if(g_cpufreq_get_ptp_level == 1)
				{
					/* FF */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x3C, PMIC_WRAP_DVFS_WDATA0); // 1.075V VPROC
					mt_cpufreq_reg_write(0x2F, PMIC_WRAP_DVFS_WDATA1); // 0.99375V VPROC
					mt_cpufreq_reg_write(0x25, PMIC_WRAP_DVFS_WDATA2); // 0.93125V VPROC
					mt_cpufreq_reg_write(0x24, PMIC_WRAP_DVFS_WDATA3); // 0.925V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA4); // 0.90V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA5); // 0.90V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x3C; // 1.075V VPROC
					mt_cpufreq_pmic_volt[1] = 0x2F; // 0.99375V VPROC
					mt_cpufreq_pmic_volt[2] = 0x25; // 0.93125V VPROC
					mt_cpufreq_pmic_volt[3] = 0x24; // 0.925V VPROC
					mt_cpufreq_pmic_volt[4] = 0x20; // 0.90V VPROC
					mt_cpufreq_pmic_volt[5] = 0x20; // 0.90V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle				
					#else
					#endif
				}
				else if(g_cpufreq_get_ptp_level == 2)
			    {
					/* FF */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT				
					#else
					#endif
				}
				else
			    {
					/* FF */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x2C, PMIC_WRAP_DVFS_WDATA0); // 0.975V VPROC
					mt_cpufreq_reg_write(0x25, PMIC_WRAP_DVFS_WDATA1); // 0.93125V VPROC
					mt_cpufreq_reg_write(0x24, PMIC_WRAP_DVFS_WDATA2); // 0.925V VPROC
					mt_cpufreq_reg_write(0x22, PMIC_WRAP_DVFS_WDATA3); // 0.9125V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA4); // 0.90V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA5); // 0.90V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x2C; // 0.975V VPROC
					mt_cpufreq_pmic_volt[1] = 0x25; // 0.93125V VPROC
					mt_cpufreq_pmic_volt[2] = 0x24; // 0.925V VPROC
					mt_cpufreq_pmic_volt[3] = 0x22; // 0.9125V VPROC
					mt_cpufreq_pmic_volt[4] = 0x20; // 0.90V VPROC
					mt_cpufreq_pmic_volt[5] = 0x20; // 0.90V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle					
					#else
					#endif
				}
			}
			else
			{
				if(is_fan53555_exist() == 1)
				{
					if(g_cpufreq_get_ptp_level == 0)
			    	{
			    		/* FF */
				        #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				        /* For PTP-OD */
				        mt_cpufreq_pmic_volt[0] = 98000; // 0.98V VPROC
				        mt_cpufreq_pmic_volt[1] = 93000; // 0.93V VPROC
				        mt_cpufreq_pmic_volt[2] = 92000; // 0.92V VPROC
				        mt_cpufreq_pmic_volt[3] = 91000; // 0.91V VPROC
				        mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

				        #else
				        /* For PTP-OD */
				        mt_cpufreq_pmic_volt[0] = 98000; // 0.98V VPROC
				        mt_cpufreq_pmic_volt[1] = 93000; // 0.93V VPROC
				        mt_cpufreq_pmic_volt[2] = 92000; // 0.92V VPROC
				        mt_cpufreq_pmic_volt[3] = 91000; // 0.91V VPROC
				        mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

				        #endif	
				    }
					else if(g_cpufreq_get_ptp_level == 1)
					{
						/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 108000; // 1.08V VPROC
						mt_cpufreq_pmic_volt[1] = 99000; // 0.99V VPROC
						mt_cpufreq_pmic_volt[2] = 93000; // 0.93V VPROC
						mt_cpufreq_pmic_volt[3] = 92000; // 0.92V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 108000; // 1.08V VPROC
						mt_cpufreq_pmic_volt[1] = 99000; // 0.99V VPROC
						mt_cpufreq_pmic_volt[2] = 93000; // 0.93V VPROC
						mt_cpufreq_pmic_volt[3] = 92000; // 0.92V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
					}		
					else if(g_cpufreq_get_ptp_level == 2)
			    	{
			    		/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						#else
						#endif	
				    }
					else
			    	{
			    		/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
				        /* For PTP-OD */
				        mt_cpufreq_pmic_volt[0] = 98000; // 0.98V VPROC
				        mt_cpufreq_pmic_volt[1] = 93000; // 0.93V VPROC
				        mt_cpufreq_pmic_volt[2] = 92000; // 0.92V VPROC
				        mt_cpufreq_pmic_volt[3] = 91000; // 0.91V VPROC
				        mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
				        /* For PTP-OD */
				        mt_cpufreq_pmic_volt[0] = 98000; // 0.98V VPROC
				        mt_cpufreq_pmic_volt[1] = 93000; // 0.93V VPROC
				        mt_cpufreq_pmic_volt[2] = 92000; // 0.92V VPROC
				        mt_cpufreq_pmic_volt[3] = 91000; // 0.91V VPROC
				        mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
				        mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
				}
				else if(is_ncp6335_exist() == 1)
				{
					if(g_cpufreq_get_ptp_level == 0)
			    	{
			    		/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 97500; // 0.975V VPROC
						mt_cpufreq_pmic_volt[1] = 93125; // 0.93125V VPROC
						mt_cpufreq_pmic_volt[2] = 92500; // 0.925V VPROC
						mt_cpufreq_pmic_volt[3] = 91250; // 0.9125V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 97500; // 0.975V VPROC
						mt_cpufreq_pmic_volt[1] = 93125; // 0.93125V VPROC
						mt_cpufreq_pmic_volt[2] = 92500; // 0.925V VPROC
						mt_cpufreq_pmic_volt[3] = 91250; // 0.9125V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else if(g_cpufreq_get_ptp_level == 1)
					{
						/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 107500; // 1.075V VPROC
						mt_cpufreq_pmic_volt[1] = 99375; // 0.99375V VPROC
						mt_cpufreq_pmic_volt[2] = 93125; // 0.93125V VPROC
						mt_cpufreq_pmic_volt[3] = 92500; // 0.925V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 107500; // 1.075V VPROC
						mt_cpufreq_pmic_volt[1] = 99375; // 0.99375V VPROC
						mt_cpufreq_pmic_volt[2] = 93125; // 0.93125V VPROC
						mt_cpufreq_pmic_volt[3] = 92500; // 0.925V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
					}		
					else if(g_cpufreq_get_ptp_level == 2)
			    	{
			    		/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						#else
						#endif	
				    }
					else
			    	{
			    		/* FF */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 97500; // 0.975V VPROC
						mt_cpufreq_pmic_volt[1] = 93125; // 0.93125V VPROC
						mt_cpufreq_pmic_volt[2] = 92500; // 0.925V VPROC
						mt_cpufreq_pmic_volt[3] = 91250; // 0.9125V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 97500; // 0.975V VPROC
						mt_cpufreq_pmic_volt[1] = 93125; // 0.93125V VPROC
						mt_cpufreq_pmic_volt[2] = 92500; // 0.925V VPROC
						mt_cpufreq_pmic_volt[3] = 91250; // 0.9125V VPROC
						mt_cpufreq_pmic_volt[4] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[5] = 90000; // 0.90V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
				}
			}
			
            return count;
        }
        else if (enable == 1)
        {
        	/* PMIC external buck */
			if(is_ext_buck_exist() == 0)
			{
				if(g_cpufreq_get_ptp_level == 0)
			    {
					/* TT = Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.0625V VPROC
					mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03125V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3A; // 1.0625V VPROC
					mt_cpufreq_pmic_volt[3] = 0x35; // 1.03125V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle		
					#else
					#endif
				}
				else if(g_cpufreq_get_ptp_level == 1)
				{
					/* TT = Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x40, PMIC_WRAP_DVFS_WDATA1); // 1.10V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA3); // 1.0625V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x40; // 1.10V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[3] = 0x3A; // 1.0625V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle	
					#else
					#endif
				}
				else if(g_cpufreq_get_ptp_level == 2)
			    {
					/* TT = Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x43, PMIC_WRAP_DVFS_WDATA1); // 1.11875V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x37, PMIC_WRAP_DVFS_WDATA3); // 1.04375V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x43; // 1.11875V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[3] = 0x37; // 1.04375V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle		
					#else
					#endif
				}
				else
			    {
					/* TT = Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.0625V VPROC
					mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03125V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3A; // 1.0625V VPROC
					mt_cpufreq_pmic_volt[3] = 0x35; // 1.03125V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle		
					#else
					#endif
				}
			}
			else
			{
				if(is_fan53555_exist() == 1)
				{
					if(g_cpufreq_get_ptp_level == 0)
			    	{
			    		/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else if(g_cpufreq_get_ptp_level == 1)
					{
						/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
					}		
					else if(g_cpufreq_get_ptp_level == 2)
			    	{
			    		/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 112000; // 1.12V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 104000; // 1.04V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 112000; // 1.12V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 104000; // 1.04V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else
			    	{
			    		/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
				}
				else if(is_ncp6335_exist() == 1)
				{
					if(g_cpufreq_get_ptp_level == 0)
			    	{
			    		/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else if(g_cpufreq_get_ptp_level == 1)
					{
						/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
					}	
					else if(g_cpufreq_get_ptp_level == 2)
			    	{
			    		/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else
			    	{
			    		/* TT = Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
				}
			}
			
            return count;
        }
        else if (enable == 2)
        {
        	/* PMIC external buck */
			if(is_ext_buck_exist() == 0)
			{
				if(g_cpufreq_get_ptp_level == 0)
			    {
					/* Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.0625V VPROC
					mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03125V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3A; // 1.0625V VPROC
					mt_cpufreq_pmic_volt[3] = 0x35; // 1.03125V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle		
					#else
					#endif
				}
				else if(g_cpufreq_get_ptp_level == 1)
				{
					/* Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x40, PMIC_WRAP_DVFS_WDATA1); // 1.10V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA3); // 1.0625V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x40; // 1.10V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[3] = 0x3A; // 1.0625V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle	
					#else
					#endif
				}
				else if(g_cpufreq_get_ptp_level == 2)
			    {
					/* Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x43, PMIC_WRAP_DVFS_WDATA1); // 1.11875V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA2); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x37, PMIC_WRAP_DVFS_WDATA3); // 1.04375V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x43; // 1.11875V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[3] = 0x37; // 1.04375V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle		
					#else
					#endif
				}
				else
			    {
					/* Initial */
					#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
					mt_cpufreq_reg_write(0x48, PMIC_WRAP_DVFS_WDATA0); // 1.15V VPROC
					mt_cpufreq_reg_write(0x3F, PMIC_WRAP_DVFS_WDATA1); // 1.09375V VPROC
					mt_cpufreq_reg_write(0x3A, PMIC_WRAP_DVFS_WDATA2); // 1.0625V VPROC
					mt_cpufreq_reg_write(0x35, PMIC_WRAP_DVFS_WDATA3); // 1.03125V VPROC
					mt_cpufreq_reg_write(0x30, PMIC_WRAP_DVFS_WDATA4); // 1.00V VPROC
					mt_cpufreq_reg_write(0x28, PMIC_WRAP_DVFS_WDATA5); // 0.95V VPROC
					mt_cpufreq_reg_write(0x20, PMIC_WRAP_DVFS_WDATA6); // 0.90V VPROC

					/* For PTP-OD */
					mt_cpufreq_pmic_volt[0] = 0x48; // 1.15V VPROC
					mt_cpufreq_pmic_volt[1] = 0x3F; // 1.09375V VPROC
					mt_cpufreq_pmic_volt[2] = 0x3A; // 1.0625V VPROC
					mt_cpufreq_pmic_volt[3] = 0x35; // 1.03125V VPROC
					mt_cpufreq_pmic_volt[4] = 0x30; // 1.00V VPROC
					mt_cpufreq_pmic_volt[5] = 0x28; // 0.95V VPROC
					mt_cpufreq_pmic_volt[6] = 0x20; // 0.90V VPROC

					mt_cpufreq_spm_volt[6] = mt_cpufreq_pmic_volt[4]; // 1.00V VPROC, for spm control in deep idle		
					#else
					#endif
				}
			}
			else
			{
				if(is_fan53555_exist() == 1)
				{
					if(g_cpufreq_get_ptp_level == 0)
			    	{
			    		/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else if(g_cpufreq_get_ptp_level == 1)
					{
						/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
					}			
					else if(g_cpufreq_get_ptp_level == 2)
			    	{
			    		/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 112000; // 1.12V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 104000; // 1.04V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 112000; // 1.12V VPROC
						mt_cpufreq_pmic_volt[2] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[3] = 104000; // 1.04V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else
			    	{
			    		/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.15V VPROC
						mt_cpufreq_pmic_volt[1] = 109000; // 1.09V VPROC
						mt_cpufreq_pmic_volt[2] = 106000; // 1.06V VPROC
						mt_cpufreq_pmic_volt[3] = 103000; // 1.03V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
				}
				else if(is_ncp6335_exist() == 1)
				{
					if(g_cpufreq_get_ptp_level == 0)
			    	{
			    		/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else if(g_cpufreq_get_ptp_level == 1)
					{
						/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 110000; // 1.10V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
					}		
					else if(g_cpufreq_get_ptp_level == 2)
			    	{
			    		/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 111875; // 1.11875V VPROC
						mt_cpufreq_pmic_volt[2] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[3] = 104375; // 1.04375V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
					else
			    	{
			    		/* Initial */
						#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#else
						/* For PTP-OD */
						mt_cpufreq_pmic_volt[0] = 115000; // 1.150V VPROC
						mt_cpufreq_pmic_volt[1] = 109375; // 1.09375V VPROC
						mt_cpufreq_pmic_volt[2] = 106250; // 1.0625V VPROC
						mt_cpufreq_pmic_volt[3] = 103125; // 1.03125V VPROC
						mt_cpufreq_pmic_volt[4] = 100000; // 1.00V VPROC
						mt_cpufreq_pmic_volt[5] = 95000; // 0.95V VPROC
						mt_cpufreq_pmic_volt[6] = 90000; // 0.90V VPROC

						#endif	
				    }
				}
			}
			
            return count;
        }

        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 or 2\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! should be 0 or 1 or 2\n");
    }

    return -EINVAL;
}
#endif

/**********************************
* limited freq for hevc bandwidth
***********************************/
static ssize_t mt_cpufreq_limited_by_hevc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int freq = 0;

    if (sscanf(buffer, "%u", &freq) == 1)
    {
        g_limited_freq_by_hevc = freq; //KHz
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "limited_by_hevc! g_limited_freq_by_hevc = %d\n", g_limited_freq_by_hevc);
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! please provide the min limit\n");
    }

    return -EINVAL;
}

static int mt_cpufreq_limited_by_hevc_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_limited_freq_by_hevc = %d\n", g_limited_freq_by_hevc);

    len = p - buf;
    return len;
}

static int mt_cpufreq_ptpod_freq_volt_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;
	int i = 0;
		
	for (i = 0; i < mt_cpu_freqs_num; i++)
	{
		p += sprintf(p, "default freq = %d, volt = %d\n", mt_cpu_freqs[i].cpufreq_khz, mt_cpu_freqs[i].cpufreq_volt);
	}
		
    len = p - buf;
    return len;
}

#ifdef PTPOD_DOWNGRADE_FREQ

static int mt_cpufreq_downgrade_freq_info_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_downgrade_freq_step_idx_1 = %d\n", mt_cpufreq_downgrade_freq_step_idx_1);
	p += sprintf(p, "mt_cpufreq_downgrade_freq_step_idx_2 = %d\n", mt_cpufreq_downgrade_freq_step_idx_2);
	p += sprintf(p, "mt_cpufreq_downgrade_freq_step_idx_limit = %d\n", mt_cpufreq_downgrade_freq_step_idx_limit);

	p += sprintf(p, "mt_cpufreq_ptpod_temperature_limit_1 = %d\n", mt_cpufreq_ptpod_temperature_limit_1);
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_limit_2 = %d\n", mt_cpufreq_ptpod_temperature_limit_2);
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_delta_1 = %d\n", mt_cpufreq_ptpod_temperature_delta_1);	
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = %d\n", mt_cpufreq_ptpod_temperature_delta_1_ratio_1);
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = %d\n", mt_cpufreq_ptpod_temperature_delta_2_ratio_1);
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = %d\n", mt_cpufreq_ptpod_temperature_delta_1_ratio_2);
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = %d\n", mt_cpufreq_ptpod_temperature_delta_2_ratio_2);
	p += sprintf(p, "mt_cpufreq_ptpod_temperature_delta_counter_limit = %d\n", mt_cpufreq_ptpod_temperature_delta_counter_limit);
	p += sprintf(p, "mt_cpufreq_downgrade_freq_counter_limit = %d\n", mt_cpufreq_downgrade_freq_counter_limit);
	p += sprintf(p, "mt_cpufreq_downgrade_freq_counter_return_limit = %d\n", mt_cpufreq_ptpod_temperature_delta_counter_limit - mt_cpufreq_downgrade_freq_counter_limit);
	p += sprintf(p, "mt_cpufreq_over_max_cpu = %d\n", mt_cpufreq_over_max_cpu);
	
    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_downgrade_freq_step_idx_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int step_idx_1 = 0, step_idx_2 = 0;

    if (sscanf(buffer, "%d %d", &step_idx_1, &step_idx_2) == 2)
    {
    	mt_cpufreq_downgrade_freq_step_idx_1 = step_idx_1;
		mt_cpufreq_downgrade_freq_step_idx_2 = step_idx_2;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}	

static ssize_t mt_cpufreq_ptpod_temperature_limit_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int dwn_temp_1 = 0, dwn_temp_2 = 0;

    if (sscanf(buffer, "%d %d", &dwn_temp_1, &dwn_temp_2) == 2)
    {
    	mt_cpufreq_ptpod_temperature_limit_1 = dwn_temp_1;
		mt_cpufreq_ptpod_temperature_limit_2 = dwn_temp_2;

		// Reset all counter
		mt_cpufreq_ptpod_temperature_delta_counter = 0;
		mt_cpufreq_downgrade_freq_counter = 0;
		mt_cpufreq_downgrade_freq_counter_return = 0;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static ssize_t mt_cpufreq_ptpod_temperature_delta_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int delta_1 = 0;

    if (sscanf(buffer, "%d", &delta_1) == 1)
    {
    	mt_cpufreq_ptpod_temperature_delta_1 = delta_1;

		// Reset all counter
		mt_cpufreq_ptpod_temperature_delta_counter = 0;
		mt_cpufreq_downgrade_freq_counter = 0;
		mt_cpufreq_downgrade_freq_counter_return = 0;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static ssize_t mt_cpufreq_ptpod_temperature_delta_ratio_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int delta_count = 0, ratio_1 = 0, ratio_2 = 0, ratio_3 = 0, ratio_4 = 0;

    if (sscanf(buffer, "%d %d %d %d %d", &delta_count, &ratio_1, &ratio_2, &ratio_3, &ratio_4) == 5)
    {
    	mt_cpufreq_ptpod_temperature_delta_counter_limit = delta_count;
    	mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = ratio_1;
		mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = ratio_2;
    	mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = ratio_3;
		mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = ratio_4;

		// Reset all counter
		mt_cpufreq_ptpod_temperature_delta_counter = 0;
		mt_cpufreq_downgrade_freq_counter = 0;
		mt_cpufreq_downgrade_freq_counter_return = 0;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static ssize_t mt_cpufreq_over_max_cpu_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int max_cpu = 0;

    if (sscanf(buffer, "%d", &max_cpu) == 1)
    {
    	mt_cpufreq_over_max_cpu = max_cpu;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}	

static int mt_cpufreq_downgrade_freq_enable_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_downgrade_freq_for_ptpod_enable = %d\n", mt_cpufreq_downgrade_freq_for_ptpod_enable);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_downgrade_freq_enable_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int on_off = 0;

    if (sscanf(buffer, "%d", &on_off) == 1)
    {
        mt_cpufreq_downgrade_freq_for_ptpod_enable = on_off;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}
#endif

static int mt_cpufreq_lbat_volt_limited_cpu_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_cpu_power_0 = %d\n", mt_cpufreq_lbat_volt_limited_cpu_power_0);
	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_cpu_power_1 = %d\n", mt_cpufreq_lbat_volt_limited_cpu_power_1);
	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_cpu_power_2 = %d\n", mt_cpufreq_lbat_volt_limited_cpu_power_2);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volt_limited_cpu_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int lbat_power_0 = 0, lbat_power_1 = 0, lbat_power_2 = 0;

    if (sscanf(buffer, "%d %d %d", &lbat_power_0, &lbat_power_1, &lbat_power_2) == 3)
    {
        mt_cpufreq_lbat_volt_limited_cpu_power_0 = lbat_power_0;
        mt_cpufreq_lbat_volt_limited_cpu_power_1 = lbat_power_1;
        mt_cpufreq_lbat_volt_limited_cpu_power_2 = lbat_power_2;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static int mt_cpufreq_lbat_volt_limited_gpu_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_gpu_power_0 = %d\n", mt_cpufreq_lbat_volt_limited_gpu_power_0);
	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_gpu_power_1 = %d\n", mt_cpufreq_lbat_volt_limited_gpu_power_1);
	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_gpu_power_2 = %d\n", mt_cpufreq_lbat_volt_limited_gpu_power_2);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volt_limited_gpu_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int lbat_power_0 = 0, lbat_power_1 = 0, lbat_power_2 = 0;

    if (sscanf(buffer, "%d %d %d", &lbat_power_0, &lbat_power_1, &lbat_power_2) == 3)
    {
        mt_cpufreq_lbat_volt_limited_gpu_power_0 = lbat_power_0;
        mt_cpufreq_lbat_volt_limited_gpu_power_1 = lbat_power_1;
        mt_cpufreq_lbat_volt_limited_gpu_power_2 = lbat_power_2;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static int mt_cpufreq_lbat_volume_limited_cpu_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volume_limited_cpu_power_0 = %d\n", mt_cpufreq_lbat_volume_limited_cpu_power_0);
	p += sprintf(p, "mt_cpufreq_lbat_volume_limited_cpu_power_1 = %d\n", mt_cpufreq_lbat_volume_limited_cpu_power_1);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volume_limited_cpu_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int lbat_power_0 = 0, lbat_power_1 = 0;

    if (sscanf(buffer, "%d %d", &lbat_power_0, &lbat_power_1) == 2)
    {
        mt_cpufreq_lbat_volume_limited_cpu_power_0 = lbat_power_0;
        mt_cpufreq_lbat_volume_limited_cpu_power_1 = lbat_power_1;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static int mt_cpufreq_lbat_volume_limited_gpu_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volume_limited_gpu_power_0 = %d\n", mt_cpufreq_lbat_volume_limited_gpu_power_0);
	p += sprintf(p, "mt_cpufreq_lbat_volume_limited_gpu_power_1 = %d\n", mt_cpufreq_lbat_volume_limited_gpu_power_1);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volume_limited_gpu_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int lbat_power_0 = 0, lbat_power_1 = 0;

    if (sscanf(buffer, "%d %d", &lbat_power_0, &lbat_power_1) == 2)
    {
        mt_cpufreq_lbat_volume_limited_gpu_power_0 = lbat_power_0;
        mt_cpufreq_lbat_volume_limited_gpu_power_1 = lbat_power_1;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static int mt_cpufreq_lbat_volt_limited_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volt_limited_level = %d\n", mt_cpufreq_lbat_volt_limited_level);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volt_limited_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int lbat_volt_level = 0;

    if (sscanf(buffer, "%d", &lbat_volt_level) == 1)
    {
        mt_cpufreq_lbat_volt_protect((LOW_BATTERY_LEVEL)lbat_volt_level);
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static int mt_cpufreq_lbat_volt_drop_enable_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volt_drop_enable = %d\n", mt_cpufreq_lbat_volt_drop_enable);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volt_drop_enable_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int on_off = 0;

    if (sscanf(buffer, "%d", &on_off) == 1)
    {
        mt_cpufreq_lbat_volt_drop_enable = on_off;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

static int mt_cpufreq_lbat_volume_enable_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "mt_cpufreq_lbat_volume_enable = %d\n", mt_cpufreq_lbat_volume_enable);

    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_lbat_volume_enable_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enable = 0;

    if (sscanf(buffer, "%d", &enable) == 1)
    {
        mt_cpufreq_lbat_volume_enable = enable;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

#ifdef CPUFREQ_HIGHEST_TURBO_MODE
static int mt_cpufreq_highest_turbo_mode_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

	p += sprintf(p, "g_cpufreq_turbo_mode_efuse_on_off = %d\n", g_cpufreq_turbo_mode_efuse_on_off);
	p += sprintf(p, "g_cpufreq_turbo_mode_efuse_4_core = %d\n", g_cpufreq_turbo_mode_efuse_4_core);
	p += sprintf(p, "g_cpufreq_turbo_mode_efuse_2_core = %d\n", g_cpufreq_turbo_mode_efuse_2_core);
	
    len = p - buf;
    return len;
}

static ssize_t mt_cpufreq_highest_turbo_mode_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int on_off = 0, freq_4core = 0, freq_2core = 0;

    if (sscanf(buffer, "%d %d %d", &on_off, &freq_4core, &freq_2core) == 3)
    {
        g_cpufreq_turbo_mode_efuse_on_off = on_off;
		g_cpufreq_turbo_mode_efuse_4_core = freq_4core;
		g_cpufreq_turbo_mode_efuse_2_core = freq_2core;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "bad argument!! argument should be oC\n");
    }

	return count;
}

#endif

/*******************************************
* cpufrqe platform driver callback function
********************************************/
static int mt_cpufreq_pdrv_probe(struct platform_device *pdev)
{
    int ret = 0, set_init_ret = 0;
	
	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    #else
	int i , pmic_idx = 0;
	#endif
	
	/* PMIC External Buck */
    if(is_ext_buck_sw_ready()==1)
    {
        ext_buck_init();
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "ext buck sw not ready\n");
    }
		
    #ifdef CONFIG_HAS_EARLYSUSPEND
    mt_cpufreq_early_suspend_handler.suspend = mt_cpufreq_early_suspend;
    mt_cpufreq_early_suspend_handler.resume = mt_cpufreq_late_resume;
    register_early_suspend(&mt_cpufreq_early_suspend_handler);
    #endif

    /************************************************
    * Check PTP level to define default max freq
    *************************************************/
    /*  TOBEDONE */
    g_cpufreq_get_ptp_level = PTP_get_ptp_level();
    g_soc_avs_type = mt_soc_check_efuse();
	#ifdef CPUFREQ_HIGHEST_TURBO_MODE
	mt_cpufreq_turbo_mode_check_efuse();
	#endif

	#if defined(MT_OFFICIAL_TURBO)
	g_cpufreq_get_ptp_level = 1;
	#endif
	
    if(g_cpufreq_get_ptp_level == 0)
        g_max_freq_by_ptp = DVFS_F0;
    else if(g_cpufreq_get_ptp_level == 1)
        g_max_freq_by_ptp = DVFS_F0_1;
    else if(g_cpufreq_get_ptp_level == 2)
        g_max_freq_by_ptp = DVFS_F2;
    else
        g_max_freq_by_ptp = DVFS_F0;

    xlog_printk(ANDROID_LOG_INFO, "Power/DVFS", "mediatek cpufreq initialized\n");

    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR0);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR1);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR2);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR3);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR4);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR5);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR6);
    mt_cpufreq_reg_write(0x0220, PMIC_WRAP_DVFS_ADR7);

	/* Set initial voltage setting */
	mt_cpufreq_return_default_DVS_by_ptpod();
	
	#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
	#else
	
	if(g_soc_avs_type == 0)
	{
		dvfs_vcore_config[pmic_idx].dvfs_vcore = DVFS_VCORE_1p1;
		dvfs_vcore_config[pmic_idx].dvfs_khz = DVFS_SOC_freq_hp;
		dvfs_vcore_config[pmic_idx+1].dvfs_vcore = DVFS_VCORE_1p0;
		dvfs_vcore_config[pmic_idx+1].dvfs_khz = DVFS_SOC_freq_lp;
	}
	else
	{
		dvfs_vcore_config[pmic_idx].dvfs_vcore = DVFS_VCORE_1p075;
		dvfs_vcore_config[pmic_idx].dvfs_khz = DVFS_SOC_freq_hp;
		dvfs_vcore_config[pmic_idx+1].dvfs_vcore = DVFS_VCORE_1p0;
		dvfs_vcore_config[pmic_idx+1].dvfs_khz = DVFS_SOC_freq_lp;
	}
	
	#endif
		
	mt_socfreq_ddr_detection();

	/* Set to max freq in probe function */
	set_init_ret = mt_cpufreq_set_initial(g_max_freq_by_ptp, 0);

	if(set_init_ret == 1)
	{
		g_cur_freq = g_max_freq_by_ptp;  /* Default set to max */
		//g_cur_cpufreq_volt = DVFS_V0_ptp0; /* Default set to max */
		g_cur_cpufreq_OPPidx = 0; /*  Default set to max */
		mt_cpufreq_freq_set_initial = true;
	}
	else
	{
		g_cur_freq = 845000;  /* Default 845000KHz in pre-loader */
		//g_cur_cpufreq_volt = DVFS_V5_ptp5; /* Default 0.95v in pmic init */
		g_cur_cpufreq_OPPidx = 5; /* Default 0.95v in pmic init */

		xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "mt_cpufreq_pdrv_probe: mt_cpufreq_set_initial return not success\n");
		xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "mt_cpufreq_pdrv_probe: keep to set to max freq in mt_cpufreq_set()\n");
	}
	
	g_limited_max_freq = g_max_freq_by_ptp;
	g_limited_min_freq = DVFS_F6;

    /************************************************
    * voltage scaling need to wait PMIC driver ready
    *************************************************/
    mt_cpufreq_ready = true;
		
    ret = cpufreq_register_driver(&mt_cpufreq_driver);

	mt_cpufreq_hotplug_notify_ready = true;
		
    #ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
    #else
	
    for(i = 0; i < mt_cpu_freqs_num; i++){
            dvfs_vcore_config[pmic_idx].dvfs_vcore = mt_cpu_freqs[i].cpufreq_volt;
            dvfs_vcore_config[pmic_idx].dvfs_khz = mt_cpu_freqs[i].cpufreq_khz;
            pmic_idx++;
    }

    for(i = 0; i < MAX_SPM_PMIC_TBL; i++){
        dvfs_vcore_config[i].tbl_idx = i;	
    }

	/*  This table is mapping to pmic wrapper VCORE table
        {DVFS_F0,          DVFS_V0_ptp0,   0}
        {DVFS_F1,          DVFS_V1_ptp1,   1}
        {DVFS_F2,          DVFS_V2_ptp2,   2}
        {DVFS_F3,          DVFS_V3_ptp3,   3}
        {DVFS_F4,          DVFS_V4_ptp4,   4}
        {DVFS_F5,          DVFS_V4_ptp5,   5}
        {DVFS_F6,          DVFS_V4_ptp6,   6}
        {DVFS_SOC_freq_hp, DVFS_VCORE_1p1, 7} or {DVFS_SOC_freq_hp, DVFS_VCORE_1p075, 7}
        {DVFS_SOC_freq_lp, DVFS_VCORE_1p0, 8}
	*/
	
    #endif

	#if defined(PTPOD_DOWNGRADE_FREQ) || defined(PTPOD_LOW_BATT_PERCENTAGE_FREQ)

	if(g_cpufreq_get_ptp_level == 0)
	{
		mt_cpufreq_ptpod_temperature_delta_counter_limit = 10; // 300ms
		mt_cpufreq_downgrade_freq_step_idx_1 = 1; // downgrade frequency 1 level, 45C < x < 65C
		mt_cpufreq_downgrade_freq_step_idx_2 = 1; // downgrade frequency 1 level, 65C < x
		mt_cpufreq_ptpod_temperature_limit_1 = 45000; //45000; //45C
		mt_cpufreq_ptpod_temperature_limit_2 = 65000; //65000; //65C
		mt_cpufreq_ptpod_temperature_delta_1 = 5000; // tempature delta 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = 0; // 0:10 , 45C < x < 65C, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = 5; // 5:5, 45C < x < 65C, delta > 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = 0; // 0:10, 65C < x, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = 8; // 8:2, 65C < x, delta > 5C
	}
	else if(g_cpufreq_get_ptp_level == 1)
	{
		mt_cpufreq_ptpod_temperature_delta_counter_limit = 20; // 600ms
		mt_cpufreq_downgrade_freq_step_idx_1 = 1; // downgrade frequency 1 level, 45C < x < 65C
		mt_cpufreq_downgrade_freq_step_idx_2 = 2; // downgrade frequency 2 level, 65C < x
		mt_cpufreq_ptpod_temperature_limit_1 = 45000; //45000; //45C
		mt_cpufreq_ptpod_temperature_limit_2 = 70000; //70000; //70C
		mt_cpufreq_ptpod_temperature_delta_1 = 5000; // tempature delta 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = 12; // 12:8 , 45C < x < 65C, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = 18; // 18:2, 45C < x < 65C, delta > 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = 10; // 10:10, 65C < x, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = 18; // 18:2, 65C < x, delta > 5C
	}
	else if(g_cpufreq_get_ptp_level == 2)
	{
		mt_cpufreq_ptpod_temperature_delta_counter_limit = 10; // 300ms
		mt_cpufreq_downgrade_freq_step_idx_1 = 1; // downgrade frequency 1 level, 45C < x < 65C
		mt_cpufreq_downgrade_freq_step_idx_2 = 1; // downgrade frequency 1 level, 65C < x
		mt_cpufreq_ptpod_temperature_limit_1 = 45000; //45000; //45C
		mt_cpufreq_ptpod_temperature_limit_2 = 65000; //65000; //65C
		mt_cpufreq_ptpod_temperature_delta_1 = 5000; // tempature delta 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = 0; // 0:10 , 45C < x < 65C, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = 5; // 5:5, 45C < x < 65C, delta > 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = 0; // 0:10, 65C < x, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = 8; // 8:2, 65C < x, delta > 5C
	}
	else
	{
		mt_cpufreq_ptpod_temperature_delta_counter_limit = 10; // 300ms
		mt_cpufreq_downgrade_freq_step_idx_1 = 1; // downgrade frequency 1 level, 45C < x < 65C
		mt_cpufreq_downgrade_freq_step_idx_2 = 1; // downgrade frequency 1 level, 65C < x
		mt_cpufreq_ptpod_temperature_limit_1 = 45000; //45000; //45C
		mt_cpufreq_ptpod_temperature_limit_2 = 65000; //65000; //65C
		mt_cpufreq_ptpod_temperature_delta_1 = 5000; // tempature delta 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_1 = 0; // 0:10 , 45C < x < 65C, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_1 = 5; // 5:5, 45C < x < 65C, delta > 5C
		mt_cpufreq_ptpod_temperature_delta_1_ratio_2 = 0; // 0:10, 65C < x, delta < 5C
		mt_cpufreq_ptpod_temperature_delta_2_ratio_2 = 8; // 8:2, 65C < x, delta > 5C
	}

	if(ptp_status() == 1)
	{
		mt_cpufreq_downgrade_freq_temp_prev = (((DRV_Reg32(PTP_TEMP) & 0xff)) + 25) * 1000;
	}
	else
	{
		mt_cpufreq_downgrade_freq_temp_prev = (unsigned int)mtktscpu_get_Tj_temp();
	}

	if(mt_cpufreq_downgrade_freq_temp_prev < 0 || mt_cpufreq_downgrade_freq_temp_prev > 125000)
	{
		xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "mt_cpufreq_pdrv_probe: temp < 0 || temp > 125000\n");
		mt_cpufreq_downgrade_freq_temp_prev = 45000;
	}
	
	cpufreq_freq_check = mt_cpufreq_downgrade_freq_check;
	#endif

	#ifdef CPUFREQ_HIGHEST_TURBO_MODE
	g_cur_freq_highest = g_max_freq_by_ptp;
	#endif
	
	register_low_battery_notify(&mt_cpufreq_lbat_volt_protect, LOW_BATTERY_PRIO_CPU);

	return ret;
}

/***************************************
* this function should never be called
****************************************/
static int mt_cpufreq_pdrv_remove(struct platform_device *pdev)
{
    return 0;
}

static unsigned int mt_cpufreq_dvfs_get_cpu_freq(void)
{
    unsigned int armpll = 0;
    unsigned int freq = 0;

    freq = DRV_Reg32(ARMPLL_CON1) & ~0x80000000;

    if ((freq >= 0x0009A000) && (freq <= 0x001B8000))
    {
        armpll = 0x0009A000;
        armpll = 1001000 + (((freq - armpll) / 0x2000) * 13000);
    }
    else if ((freq >= 0x010F4000) && (freq <= 0x01132000))
    {
        armpll = 0x010F4000;
        armpll = 793000 + (((freq - armpll) / 0x2000) * 6500);
    }
    else
    {
    	xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "Frequency is out of range, return max freq.\n");
		return g_max_freq_by_ptp;
    }

	xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "mt_cpufreq_dvfs_get_cpu_freq: ARMPLL_CON1 = 0x%x\n", DRV_Reg32(ARMPLL_CON1));

    return armpll; //KHz
}

static int mt_cpufreq_pm_restore_early(struct device *dev)
{
    //int ret;
	int i = 0;
		
	#if 1

	g_cur_freq = mt_cpufreq_dvfs_get_cpu_freq();

	for (i = 0; i < mt_cpu_freqs_num; i++)
    {
        if (g_cur_freq == mt_cpu_freqs[i].cpufreq_khz)
        {
			g_cur_cpufreq_OPPidx = i;
			xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "match g_cur_cpufreq_OPPidx: %d\n", g_cur_cpufreq_OPPidx);
            break;
        }
    }

	#ifdef CPUFREQ_HIGHEST_TURBO_MODE
	if (g_cur_freq > mt_cpu_freqs[0].cpufreq_khz)
	{
		g_cur_cpufreq_OPPidx = 0;
		xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "match, turob mode, g_cur_cpufreq_OPPidx: %d\n", g_cur_cpufreq_OPPidx);
	}		
	#endif

	xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "CPU freq SW/HW: %d/%d\n", g_cur_freq, mt_cpufreq_dvfs_get_cpu_freq());
	xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "g_cur_cpufreq_OPPidx: %d\n", g_cur_cpufreq_OPPidx);
	
	#else
    /* Set to max freq for hibernation flow (IPO-H) */
    ret = mt_cpufreq_set_initial(g_max_freq_by_ptp, 0);
    BUG_ON(ret != 1);

    g_cur_freq = g_max_freq_by_ptp;  /* Default set to max */
    g_cur_cpufreq_OPPidx = 0; /*  Default set to max */
    mt_cpufreq_freq_set_initial = true;

    pr_warn("[%s] CPU freq SW/HW: %u/%u\n", __func__, g_cur_freq, mt_get_cpu_freq());
	#endif
	
    return 0;
}

struct dev_pm_ops mt_cpufreq_pm_ops = {
    .suspend    = NULL,
    .resume     = NULL,
    .restore_early = mt_cpufreq_pm_restore_early,
};

static struct platform_driver mt_cpufreq_pdrv = {
    .probe      = mt_cpufreq_pdrv_probe,
    .remove     = mt_cpufreq_pdrv_remove,
    .driver     = {
        .name   = "mt-cpufreq",
        .pm     = &mt_cpufreq_pm_ops,
        .owner  = THIS_MODULE,
    },
};

/***********************************************************
* cpufreq initialization to register cpufreq platform driver
************************************************************/
static int __init mt_cpufreq_pdrv_init(void)
{
    int ret = 0;

    struct proc_dir_entry *mt_entry = NULL;
    struct proc_dir_entry *mt_cpufreq_dir = NULL;

    mt_cpufreq_dir = proc_mkdir("cpufreq", NULL);
    if (!mt_cpufreq_dir)
    {
        pr_err("[%s]: mkdir /proc/cpufreq failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("cpufreq_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_debug_read;
            mt_entry->write_proc = mt_cpufreq_debug_write;
        }

        mt_entry = create_proc_entry("cpufreq_limited_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_limited_power_read;
            mt_entry->write_proc = mt_cpufreq_limited_power_write;
        }

        mt_entry = create_proc_entry("cpufreq_state", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_state_read;
            mt_entry->write_proc = mt_cpufreq_state_write;
        }

        mt_entry = create_proc_entry("cpufreq_power_dump", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_power_dump_read;
        }

        mt_entry = create_proc_entry("socfreq_state", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_socfreq_state_read;
            mt_entry->write_proc = mt_socfreq_state_write;
        }

        mt_entry = create_proc_entry("socfreq_dump_info", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_socfreq_dump_info_read;
        }

        #if defined(CONFIG_THERMAL_LIMIT_TEST)
        mt_entry = create_proc_entry("cpufreq_limited_load", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_cpufreq_limited_load_read;
            mt_entry->write_proc = mt_cpufreq_limited_load_write;
        }
        #endif

        #ifdef MT_DVFS_PTPOD_TEST
        mt_entry = create_proc_entry("cpufreq_ptpod_test", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->write_proc = mt_cpufreq_ptpod_test_write;
        }
        #endif

		#ifdef MT_DVFS_FFTT_TEST
        mt_entry = create_proc_entry("cpufreq_fftt_test", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->write_proc = mt_cpufreq_fftt_test_write;
        }
		#endif

        mt_entry = create_proc_entry("cpufreq_limited_by_hevc", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
        	mt_entry->read_proc = mt_cpufreq_limited_by_hevc_read;
            mt_entry->write_proc = mt_cpufreq_limited_by_hevc_write;
        }

        mt_entry = create_proc_entry("cpufreq_ptpod_freq_volt", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
        	mt_entry->read_proc = mt_cpufreq_ptpod_freq_volt_read;
        }

        mt_entry = create_proc_entry("socfreq_limited_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_socfreq_limited_power_read;
            mt_entry->write_proc = mt_socfreq_limited_power_write;
        }

		mt_entry = create_proc_entry("socfreq_enable_hp", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->read_proc = mt_socfreq_enable_hp_read;
			mt_entry->write_proc = mt_socfreq_enable_hp_write;
		}

		mt_entry = create_proc_entry("socfreq_gpu_power_dump", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->read_proc = mt_socfreq_gpu_power_dump_read;
		}

		mt_entry = create_proc_entry("socfreq_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->read_proc = mt_socfreq_debug_read;
			mt_entry->write_proc = mt_socfreq_debug_write;
		}

		#ifdef MT_DVFS_LOW_VOLTAGE_SUPPORT
		mt_entry = create_proc_entry("socfreq_voltage", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->read_proc = mt_socfreq_voltage_read;
		}
		#endif

		mt_entry = create_proc_entry("socfreq_fixed_test", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->read_proc = mt_socfreq_fixed_test_read;
			mt_entry->write_proc = mt_socfreq_fixed_test_write;
		}		

		#ifdef PTPOD_DOWNGRADE_FREQ
		mt_entry = create_proc_entry("cpufreq_downgrade_freq_info", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->read_proc = mt_cpufreq_downgrade_freq_info_read;
		}

		mt_entry = create_proc_entry("cpufreq_downgrade_freq_step_idx", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->write_proc = mt_cpufreq_downgrade_freq_step_idx_write;
		}	

		mt_entry = create_proc_entry("cpufreq_ptpod_temperature_limit", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->write_proc = mt_cpufreq_ptpod_temperature_limit_write;
		}

		mt_entry = create_proc_entry("cpufreq_ptpod_temperature_delta", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->write_proc = mt_cpufreq_ptpod_temperature_delta_write;
		}

		mt_entry = create_proc_entry("cpufreq_ptpod_temperature_delta_ratio", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->write_proc = mt_cpufreq_ptpod_temperature_delta_ratio_write;
		}
		
		mt_entry = create_proc_entry("cpufreq_over_max_cpu", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
			mt_entry->write_proc = mt_cpufreq_over_max_cpu_write;
		}

        mt_entry = create_proc_entry("cpufreq_downgrade_freq_enable", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_downgrade_freq_enable_read;
			mt_entry->write_proc = mt_cpufreq_downgrade_freq_enable_write;
		}
		#endif

        mt_entry = create_proc_entry("cpufreq_lbat_volt_limited_cpu_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volt_limited_cpu_power_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volt_limited_cpu_power_write;
		}

        mt_entry = create_proc_entry("cpufreq_lbat_volt_limited_gpu_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volt_limited_gpu_power_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volt_limited_gpu_power_write;
		}

		mt_entry = create_proc_entry("cpufreq_lbat_volume_limited_cpu_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volume_limited_cpu_power_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volume_limited_cpu_power_write;
		}

        mt_entry = create_proc_entry("cpufreq_lbat_volume_limited_gpu_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volume_limited_gpu_power_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volume_limited_gpu_power_write;
		}

        mt_entry = create_proc_entry("cpufreq_lbat_volt_limited", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volt_limited_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volt_limited_write;
		}

        mt_entry = create_proc_entry("cpufreq_lbat_volt_drop_enable", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volt_drop_enable_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volt_drop_enable_write;
		}

        mt_entry = create_proc_entry("cpufreq_lbat_volume_enable", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_lbat_volume_enable_read;
			mt_entry->write_proc = mt_cpufreq_lbat_volume_enable_write;
		}

		#ifdef CPUFREQ_HIGHEST_TURBO_MODE
        mt_entry = create_proc_entry("cpufreq_highest_turbo_mode", S_IRUGO | S_IWUSR | S_IWGRP, mt_cpufreq_dir);
		if (mt_entry)
		{
		    mt_entry->read_proc = mt_cpufreq_highest_turbo_mode_read;
			mt_entry->write_proc = mt_cpufreq_highest_turbo_mode_write;
		}
		#endif
    }

    ret = platform_driver_register(&mt_cpufreq_pdrv);

    if (ret)
    {
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "failed to register cpufreq driver\n");
        return ret;
    }
    else
    {
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "cpufreq driver registration done\n");
        xlog_printk(ANDROID_LOG_ERROR, "Power/DVFS", "g_cpufreq_get_ptp_level = %d\n", g_cpufreq_get_ptp_level);
        return 0;
    }
}

static void __exit mt_cpufreq_pdrv_exit(void)
{
    cpufreq_unregister_driver(&mt_cpufreq_driver);
}


module_init(mt_cpufreq_pdrv_init);
module_exit(mt_cpufreq_pdrv_exit);

MODULE_DESCRIPTION("MediaTek CPU Frequency Scaling driver");
MODULE_LICENSE("GPL");
