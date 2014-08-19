#ifndef _MT_CPUFREQ_H
#define _MT_CPUFREQ_H

#include <linux/module.h>

/*********************
* SOC Efuse Register
**********************/
#define M_HW_RES4    (0xF0206174)

/*********************
* Clock Mux Register
**********************/
#define TOP_CKMUXSEL    (0xF0001000)
#define TOP_CKDIV1_CPU  (0xF0001008)

/****************************
* PMIC Wrapper DVFS Register
*****************************/
#define PWRAP_BASE              (0xF000D000)
#define PMIC_WRAP_DVFS_ADR0     (PWRAP_BASE + 0xE4)
#define PMIC_WRAP_DVFS_WDATA0   (PWRAP_BASE + 0xE8)
#define PMIC_WRAP_DVFS_ADR1     (PWRAP_BASE + 0xEC)
#define PMIC_WRAP_DVFS_WDATA1   (PWRAP_BASE + 0xF0)
#define PMIC_WRAP_DVFS_ADR2     (PWRAP_BASE + 0xF4)
#define PMIC_WRAP_DVFS_WDATA2   (PWRAP_BASE + 0xF8)
#define PMIC_WRAP_DVFS_ADR3     (PWRAP_BASE + 0xFC)
#define PMIC_WRAP_DVFS_WDATA3   (PWRAP_BASE + 0x100)
#define PMIC_WRAP_DVFS_ADR4     (PWRAP_BASE + 0x104)
#define PMIC_WRAP_DVFS_WDATA4   (PWRAP_BASE + 0x108)
#define PMIC_WRAP_DVFS_ADR5     (PWRAP_BASE + 0x10C)
#define PMIC_WRAP_DVFS_WDATA5   (PWRAP_BASE + 0x110)
#define PMIC_WRAP_DVFS_ADR6     (PWRAP_BASE + 0x114)
#define PMIC_WRAP_DVFS_WDATA6   (PWRAP_BASE + 0x118)
#define PMIC_WRAP_DVFS_ADR7     (PWRAP_BASE + 0x11C)
#define PMIC_WRAP_DVFS_WDATA7   (PWRAP_BASE + 0x120)

/****************************
* SOC DVFS request 
*****************************/
typedef enum
{
    SOC_DVFS_TYPE_VENC = 0,
    SOC_DVFS_TYPE_ZSD,
    SOC_DVFS_TYPE_GPU_HP,
    SOC_DVFS_TYPE_WIFI_DISPLAY,
    SOC_DVFS_TYPE_DISPLAY,
    SOC_DVFS_TYPE_PAUSE,
    SOC_DVFS_TYPE_FIXED,
    SOC_DVFS_TYPE_NUM
}SOC_DVFS_TYPE_ENUM;

/*****************
* extern function 
******************/
extern int mt_cpufreq_state_set(int enabled);
extern void mt_cpufreq_thermal_protect(unsigned int limited_power);
void mt_cpufreq_enable_by_ptpod(void);
unsigned int mt_cpufreq_disable_by_ptpod(void);
extern unsigned int mt_cpufreq_max_frequency_by_DVS(unsigned int num);
void mt_cpufreq_return_default_DVS_by_ptpod(void);
extern bool mt_cpufreq_earlysuspend_status_get(void);
extern unsigned int mt_soc_dvfs(SOC_DVFS_TYPE_ENUM type, unsigned int sochp_enable);
#endif