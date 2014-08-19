#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>    //udelay

#include <mach/mt_typedefs.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/hotplug.h>
#include <mach/mt_clkmgr.h>


/**************************************
 * for ARM CA7 Errata
 **************************************/
//#define HAS_ARM_CA7_ERRATA_802022_SW_WORKAROUND


/**************************************
 * for CPU MTCMOS
 **************************************/
/*
 * regiser bit difinition
 */
/* SPM_MP0_FC1_PWR_CON */
/* SPM_MP0_FC2_PWR_CON */
/* SPM_MP0_FC3_PWR_CON */
/* SPM_MP0_DBG_PWR_CON */
/* SPM_MP1_FC0_PWR_CON */
/* SPM_MP1_FC1_PWR_CON */
/* SPM_MP1_FC2_PWR_CON */
/* SPM_MP1_FC3_PWR_CON */
/* SPM_MP1_DBG_PWR_CON */
/* SPM_MP1_CPU_PWR_CON */
#define SRAM_ISOINT_B       (1U << 6)
#define SRAM_CKISO          (1U << 5)
#define PWR_CLK_DIS         (1U << 4)
#define PWR_ON_S            (1U << 3)
#define PWR_ON              (1U << 2)
#define PWR_ISO             (1U << 1)
#define PWR_RST_B           (1U << 0)

/* SPM_MP0_FC1_L1_PDN */
/* SPM_MP0_FC2_L1_PDN */
/* SPM_MP0_FC3_L1_PDN */
/* SPM_MP1_FC0_L1_PDN */
/* SPM_MP1_FC1_L1_PDN */
/* SPM_MP1_FC2_L1_PDN */
/* SPM_MP1_FC3_L1_PDN */
#define L1_PDN_ACK          (1U << 8)
#define L1_PDN              (1U << 0)
/* SPM_MP1_L2_DAT_PDN */
#define L2_SRAM_PDN_ACK     (1U << 8)
#define L2_SRAM_PDN         (1U << 0)
/* SPM_MP1_L2_DAT_SLEEP_B */
#define L2_SRAM_SLEEP_B_ACK (1U << 8)
#define L2_SRAM_SLEEP_B     (1U << 0)

/* SPM_PWR_STATUS */
/* SPM_PWR_STATUS_S */
#define MP1_DBG             (1U << 20)
#define MP1_FC0             (1U << 19)
#define MP1_FC1             (1U << 18)
#define MP1_FC2             (1U << 17)
#define MP1_FC3             (1U << 16)
#define MP1_CPU             (1U << 15)
#define MP0_DBG             (1U << 14)
#define MP0_FC1             (1U << 11)
#define MP0_FC2             (1U << 10)
#define MP0_FC3             (1U <<  9)

/* SPM_SLEEP_TIMER_STA */
#define MP1_CPU3_STANDBYWFI (1U << 23)
#define MP1_CPU2_STANDBYWFI (1U << 22)
#define MP1_CPU1_STANDBYWFI (1U << 21)
#define MP1_CPU0_STANDBYWFI (1U << 20)
#define MP0_CPU3_STANDBYWFI (1U << 19)
#define MP0_CPU2_STANDBYWFI (1U << 18)
#define MP0_CPU1_STANDBYWFI (1U << 17)


static DEFINE_SPINLOCK(spm_cpu_lock);


void spm_mtcmos_cpu_lock(unsigned long *flags)
{
    spin_lock_irqsave(&spm_cpu_lock, *flags);
}

void spm_mtcmos_cpu_unlock(unsigned long *flags)
{
    spin_unlock_irqrestore(&spm_cpu_lock, *flags);
}

int spm_mtcmos_ctrl_cpu0(int state, int chkWfiBeforePdn)
{
    if (state == STA_POWER_DOWN) {
        
    } else {    /* STA_POWER_ON */
        
    }

    return 0;
}

int spm_mtcmos_ctrl_cpu1(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN)
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP0_CPU1_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP0_FC1_L1_PDN, spm_read(SPM_MP0_FC1_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP0_FC1_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP0_FC1_PWR_CON, (spm_read(SPM_MP0_FC1_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_FC1) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP0_FC1) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
    } 
    else /* STA_POWER_ON */
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_FC1) != MP0_FC1) | ((spm_read(SPM_PWR_STATUS_S) & MP0_FC1) != MP0_FC1));
        
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP0_FC1_L1_PDN, spm_read(SPM_MP0_FC1_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP0_FC1_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpu2(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP0_CPU2_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP0_FC2_L1_PDN, spm_read(SPM_MP0_FC2_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP0_FC2_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP0_FC2_PWR_CON, (spm_read(SPM_MP0_FC2_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_FC2) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP0_FC2) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
    } 
    else /* STA_POWER_ON */
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_FC2) != MP0_FC2) | ((spm_read(SPM_PWR_STATUS_S) & MP0_FC2) != MP0_FC2));
        
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP0_FC2_L1_PDN, spm_read(SPM_MP0_FC2_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP0_FC2_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpu3(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP0_CPU3_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP0_FC3_L1_PDN, spm_read(SPM_MP0_FC3_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP0_FC3_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP0_FC3_PWR_CON, (spm_read(SPM_MP0_FC3_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_FC3) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP0_FC3) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
    } 
    else /* STA_POWER_ON */
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_FC3) != MP0_FC3) | ((spm_read(SPM_PWR_STATUS_S) & MP0_FC3) != MP0_FC3));
        
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP0_FC3_L1_PDN, spm_read(SPM_MP0_FC3_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP0_FC3_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpu4(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP1_CPU0_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC0_L1_PDN, spm_read(SPM_MP1_FC0_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP1_FC0_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP1_FC0_PWR_CON, (spm_read(SPM_MP1_FC0_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC0) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC0) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
        
        if (!(spm_read(SPM_PWR_STATUS) & (MP1_FC1 | MP1_FC2 | MP1_FC3)) && 
            !(spm_read(SPM_PWR_STATUS_S) & (MP1_FC1 | MP1_FC2 | MP1_FC3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } 
    else /* STA_POWER_ON */
    {
        if (!(spm_read(SPM_PWR_STATUS) & MP1_CPU) &&
            !(spm_read(SPM_PWR_STATUS_S) & MP1_CPU))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC0) != MP1_FC0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC0) != MP1_FC0));
        
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP1_FC0_L1_PDN, spm_read(SPM_MP1_FC0_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP1_FC0_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpu5(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN)
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP1_CPU1_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC1_L1_PDN, spm_read(SPM_MP1_FC1_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP1_FC1_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP1_FC1_PWR_CON, (spm_read(SPM_MP1_FC1_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC1) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC1) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
        
        if (!(spm_read(SPM_PWR_STATUS) & (MP1_FC0 | MP1_FC2 | MP1_FC3)) && 
            !(spm_read(SPM_PWR_STATUS_S) & (MP1_FC0 | MP1_FC2 | MP1_FC3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } 
    else /* STA_POWER_ON */
    {
        if (!(spm_read(SPM_PWR_STATUS) & MP1_CPU) &&
            !(spm_read(SPM_PWR_STATUS_S) & MP1_CPU))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC1) != MP1_FC1) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC1) != MP1_FC1));
        
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP1_FC1_L1_PDN, spm_read(SPM_MP1_FC1_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP1_FC1_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpu6(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP1_CPU2_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC2_L1_PDN, spm_read(SPM_MP1_FC2_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP1_FC2_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP1_FC2_PWR_CON, (spm_read(SPM_MP1_FC2_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC2) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC2) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
        
        if (!(spm_read(SPM_PWR_STATUS) & (MP1_FC0 | MP1_FC1 | MP1_FC3)) && 
            !(spm_read(SPM_PWR_STATUS_S) & (MP1_FC0 | MP1_FC1 | MP1_FC3)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } 
    else /* STA_POWER_ON */
    {
        if (!(spm_read(SPM_PWR_STATUS) & MP1_CPU) &&
            !(spm_read(SPM_PWR_STATUS_S) & MP1_CPU))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC2) != MP1_FC2) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC2) != MP1_FC2));
        
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP1_FC2_L1_PDN, spm_read(SPM_MP1_FC2_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP1_FC2_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpu7(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        if (chkWfiBeforePdn)
            while ((spm_read(SPM_SLEEP_TIMER_STA) & MP1_CPU3_STANDBYWFI) == 0);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC3_L1_PDN, spm_read(SPM_MP1_FC3_L1_PDN) | L1_PDN);
        while ((spm_read(SPM_MP1_FC3_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
        
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP1_FC3_PWR_CON, (spm_read(SPM_MP1_FC3_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC3) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC3) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
        
        if (!(spm_read(SPM_PWR_STATUS) & (MP1_FC0 | MP1_FC1 | MP1_FC2)) && 
            !(spm_read(SPM_PWR_STATUS_S) & (MP1_FC0 | MP1_FC1 | MP1_FC2)))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
    } 
    else /* STA_POWER_ON */
    {
        if (!(spm_read(SPM_PWR_STATUS) & MP1_CPU) &&
            !(spm_read(SPM_PWR_STATUS_S) & MP1_CPU))
            spm_mtcmos_ctrl_cpusys1(state, chkWfiBeforePdn);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_FC3) != MP1_FC3) | ((spm_read(SPM_PWR_STATUS_S) & MP1_FC3) != MP1_FC3));
        
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~PWR_CLK_DIS);
        
        spm_write(SPM_MP1_FC3_L1_PDN, spm_read(SPM_MP1_FC3_L1_PDN) & ~L1_PDN);
        while ((spm_read(SPM_MP1_FC3_L1_PDN) & L1_PDN_ACK) != 0);
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_dbg0(int state)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP0_DBG_PWR_CON, (spm_read(SPM_MP0_DBG_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_DBG) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP0_DBG) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
    } 
    else /* STA_POWER_ON */
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP0_DBG) != MP0_DBG) | ((spm_read(SPM_PWR_STATUS_S) & MP0_DBG) != MP0_DBG));
        
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) & ~SRAM_CKISO);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP0_DBG_PWR_CON, spm_read(SPM_MP0_DBG_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_dbg1(int state)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP1_DBG_PWR_CON, (spm_read(SPM_MP1_DBG_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_DBG) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_DBG) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
    } 
    else /* STA_POWER_ON */
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_DBG) != MP1_DBG) | ((spm_read(SPM_PWR_STATUS_S) & MP1_DBG) != MP1_DBG));
        
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) & ~SRAM_CKISO);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP1_DBG_PWR_CON, spm_read(SPM_MP1_DBG_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
    }
    
    return 0;
}

int spm_mtcmos_ctrl_cpusys1(int state, int chkWfiBeforePdn)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    if (state == STA_POWER_DOWN) 
    {
        if (chkWfiBeforePdn)
            while((spm_read(MP1_CA7_MISC_CONFIG) & STANDBYWFIL2) == 0);
        
        //if (((spm_read(SPM_PWR_STATUS) & MP1_DBG) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_DBG) != 0))
        spm_mtcmos_ctrl_dbg1(state);
        
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~SRAM_ISOINT_B);
    #ifdef HAS_ARM_CA7_ERRATA_802022_SW_WORKAROUND
        spm_write(SPM_MP1_L2_DAT_PDN, spm_read(SPM_MP1_L2_DAT_PDN) | L2_SRAM_PDN);
        while ((spm_read(SPM_MP1_L2_DAT_PDN) & L2_SRAM_PDN_ACK) != L2_SRAM_PDN_ACK);
    #else
        spm_write(SPM_MP1_L2_DAT_SLEEP_B, spm_read(SPM_MP1_L2_DAT_SLEEP_B) & ~L2_SRAM_SLEEP_B);
        while ((spm_read(SPM_MP1_L2_DAT_SLEEP_B) & L2_SRAM_SLEEP_B_ACK) != 0);
    #endif
        
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | PWR_ISO);
        spm_write(SPM_MP1_CPU_PWR_CON, (spm_read(SPM_MP1_CPU_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
        
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~PWR_ON);
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_CPU) != 0) | ((spm_read(SPM_PWR_STATUS_S) & MP1_CPU) != 0));
        
        spm_mtcmos_cpu_unlock(&flags);
    } 
    else /* STA_POWER_ON */
    {
        spm_mtcmos_cpu_lock(&flags);
        
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | PWR_ON);
        udelay(1);
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | PWR_ON_S);
        while (((spm_read(SPM_PWR_STATUS) & MP1_CPU) != MP1_CPU) | ((spm_read(SPM_PWR_STATUS_S) & MP1_CPU) != MP1_CPU));
        
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~PWR_CLK_DIS);
        
    #ifdef HAS_ARM_CA7_ERRATA_802022_SW_WORKAROUND
        spm_write(SPM_MP1_L2_DAT_PDN, spm_read(SPM_MP1_L2_DAT_PDN) & ~L2_SRAM_PDN);
        while ((spm_read(SPM_MP1_L2_DAT_PDN) & L2_SRAM_PDN_ACK) != 0);
    #else
        spm_write(SPM_MP1_L2_DAT_SLEEP_B, spm_read(SPM_MP1_L2_DAT_SLEEP_B) | L2_SRAM_SLEEP_B);
        while ((spm_read(SPM_MP1_L2_DAT_SLEEP_B) & L2_SRAM_SLEEP_B_ACK) != L2_SRAM_SLEEP_B_ACK);
    #endif
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~SRAM_CKISO);
        
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | PWR_RST_B);
        
        spm_mtcmos_cpu_unlock(&flags);
        
        spm_mtcmos_ctrl_dbg1(state);
    }
    
    return 0;
}

int spm_mtcmos_reset_cpusys1(void)
{
    unsigned long flags;
    
    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    
    spm_mtcmos_cpu_lock(&flags);
    
    spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) & ~PWR_RST_B);
    spm_write(SPM_MP1_CPU_PWR_CON, spm_read(SPM_MP1_CPU_PWR_CON) | PWR_RST_B);
    
    spm_mtcmos_cpu_unlock(&flags);
    
    return 0;
}

void spm_mtcmos_ctrl_cpusys1_init_1st_bring_up(int state)
{

    if (state == STA_POWER_DOWN) 
    {
        spm_mtcmos_ctrl_cpu7(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu6(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu5(STA_POWER_DOWN, 0);
        spm_mtcmos_ctrl_cpu4(STA_POWER_DOWN, 0);
    } 
    else /* STA_POWER_ON */
    {
        //XXX: reset cpusys1 one more time
        spm_mtcmos_reset_cpusys1();
        
        spm_mtcmos_ctrl_cpu4(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_cpu5(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_cpu6(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_cpu7(STA_POWER_ON, 1);
        spm_mtcmos_ctrl_dbg1(STA_POWER_ON);
    }
    
    //XXX: backup old code for reference
    //unsigned long flags;
    //
    ///* enable register control */
    //spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    //
    //spm_mtcmos_cpu_lock(&flags);
    //
    //spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) & ~PWR_CLK_DIS);
    //spm_write(SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON) | PWR_RST_B);
    //spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) & ~PWR_CLK_DIS);
    //spm_write(SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON) | PWR_RST_B);
    //spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) & ~PWR_CLK_DIS);
    //spm_write(SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON) | PWR_RST_B);
    //spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) & ~PWR_CLK_DIS);
    //spm_write(SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON) | PWR_RST_B);
    //
    //spm_mtcmos_cpu_unlock(&flags);
    //
    //spm_mtcmos_ctrl_dbg1(STA_POWER_ON);
}

bool spm_cpusys0_can_power_down(void)
{
    return !(spm_read(SPM_PWR_STATUS) & (MP1_DBG | MP1_FC0 | MP1_FC1 | MP1_FC2 | MP1_FC3 | MP1_CPU | MP0_FC1 | MP0_FC2 | MP0_FC3)) &&
           !(spm_read(SPM_PWR_STATUS_S) & (MP1_DBG | MP1_FC0 | MP1_FC1 | MP1_FC2 | MP1_FC3 | MP1_CPU | MP0_FC1 | MP0_FC2 | MP0_FC3));
}


/**************************************
 * for non-CPU MTCMOS
 **************************************/
static DEFINE_SPINLOCK(spm_noncpu_lock);

#if 0
void spm_mtcmos_noncpu_lock(unsigned long *flags)
{
    spin_lock_irqsave(&spm_noncpu_lock, *flags);
}

void spm_mtcmos_noncpu_unlock(unsigned long *flags)
{
    spin_unlock_irqrestore(&spm_noncpu_lock, *flags);
}
#else
#define spm_mtcmos_noncpu_lock(flags)   \
do {    \
    spin_lock_irqsave(&spm_noncpu_lock, flags);  \
} while (0)

#define spm_mtcmos_noncpu_unlock(flags) \
do {    \
    spin_unlock_irqrestore(&spm_noncpu_lock, flags);    \
} while (0)

#endif

#define MJC_PWR_STA_MASK    (0x1 << 21) 
#define VDE_PWR_STA_MASK    (0x1 << 7)
#define IFR_PWR_STA_MASK    (0x1 << 6)
#define ISP_PWR_STA_MASK    (0x1 << 5)
#define DIS_PWR_STA_MASK    (0x1 << 3)
#define MFG_PWR_STA_MASK    (0x1 << 4)
#define DPY_PWR_STA_MASK    (0x1 << 2)
#define CONN_PWR_STA_MASK   (0x1 << 1)
#define MD1_PWR_STA_MASK    (0x1 << 0)

#if 0
#define PWR_RST_B           (0x1 << 0)
#define PWR_ISO             (0x1 << 1)
#define PWR_ON              (0x1 << 2)
#define PWR_ON_S            (0x1 << 3)
#define PWR_CLK_DIS         (0x1 << 4)
#endif

#define SRAM_PDN            (0xf << 8)
#define MD_SRAM_PDN         (0x1 << 8)

#define VDE_SRAM_ACK        (0x1 << 12)
#define IFR_SRAM_ACK        (0xf << 12)
#define ISP_SRAM_ACK        (0x3 << 12)
#define DIS_SRAM_ACK        (0xf << 12)
#define MFG_SRAM_ACK        (0x1 << 12)

#define MD1_PROT_MASK     0x00B8
#define CONN_PROT_MASK    0x0104
#define DISP_PROT_MASK    0x0002//0x0042


int spm_mtcmos_ctrl_vdec(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_VDE_PWR_CON) & VDE_SRAM_ACK) != VDE_SRAM_ACK) {
        }

        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_VDE_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_VDE_PWR_CON, val);

        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & VDE_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & VDE_PWR_STA_MASK) 
                || (spm_read(SPM_PWR_STATUS_S) & VDE_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | PWR_ON);
        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & VDE_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_S) & VDE_PWR_STA_MASK)) {
        }
#endif

        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) | PWR_RST_B);

        spm_write(SPM_VDE_PWR_CON, spm_read(SPM_VDE_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_VDE_PWR_CON) & VDE_SRAM_ACK)) {
        }

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & VDE_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_isp(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_ISP_PWR_CON) & ISP_SRAM_ACK) != ISP_SRAM_ACK) {
        }

        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_ISP_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_ISP_PWR_CON, val);

        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & ISP_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & ISP_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_S) & ISP_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | PWR_ON);
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & ISP_PWR_STA_MASK)
                || !(spm_read(SPM_PWR_STATUS_S) & ISP_PWR_STA_MASK)) {
        }
#endif
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) | PWR_RST_B);

        spm_write(SPM_ISP_PWR_CON, spm_read(SPM_ISP_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_ISP_PWR_CON) & ISP_SRAM_ACK)) {
        }

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & ISP_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}
#if 0
int spm_mtcmos_ctrl_disp(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) | DISP_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & DISP_PROT_MASK) != DISP_PROT_MASK) {
        }
        
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | SRAM_PDN);
#if 0
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) != DIS_SRAM_ACK) {
        }
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_DIS_PWR_CON);
        //val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        val = val | PWR_CLK_DIS;
        spm_write(SPM_DIS_PWR_CON, val);

        //spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) { 
            err = 1;
        }
#else
        //while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
        //        || (spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)) {
        //}
#endif
    } else {    /* STA_POWER_ON */
        //spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON);
        //spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        //while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) 
        //        || !(spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)) {
        //}
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO);
        //spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN);

#if 0
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
        }
#endif

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) & ~DISP_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & DISP_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

#else

int spm_mtcmos_ctrl_disp(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) | DISP_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & DISP_PROT_MASK) != DISP_PROT_MASK) {
        }
        
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | SRAM_PDN);
#if 0
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) != DIS_SRAM_ACK) {
        }
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_DIS_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        //val = val | PWR_CLK_DIS;
        spm_write(SPM_DIS_PWR_CON, val);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)) {
        }
#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN);

#if 0
        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
        }
#endif

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) & ~DISP_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & DISP_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}
#endif

int spm_mtcmos_ctrl_mfg(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
//        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) | MFG_PROT_MASK);
//        while ((spm_read(TOPAXI_PROT_STA1) & MFG_PROT_MASK) != MFG_PROT_MASK) {
//        }

//        spm_write(TOPAXI_SI0_CTL, spm_read(TOPAXI_SI0_CTL) & ~MFG_SI0_MASK);

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_MFG_PWR_CON) & MFG_SRAM_ACK) != MFG_SRAM_ACK) {
        }

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_MFG_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_MFG_PWR_CON, val);

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK) 
                || (spm_read(SPM_PWR_STATUS_S) & MFG_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK) || 
                !(spm_read(SPM_PWR_STATUS_S) & MFG_PWR_STA_MASK)) {
        }
#endif

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_RST_B);

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_MFG_PWR_CON) & MFG_SRAM_ACK)) {
        }

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
//        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) & ~MFG_PROT_MASK);
//        while (spm_read(TOPAXI_PROT_STA1) & MFG_PROT_MASK) {
//        }
//        spm_write(TOPAXI_SI0_CTL, spm_read(TOPAXI_SI0_CTL) | MFG_SI0_MASK);
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_infra(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) | SRAM_PDN);

        while ((spm_read(SPM_IFR_PWR_CON) & IFR_SRAM_ACK) != IFR_SRAM_ACK) {
        }

        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_IFR_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_IFR_PWR_CON, val);

        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & IFR_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & IFR_PWR_STA_MASK) 
                || (spm_read(SPM_PWR_STATUS_S) & IFR_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) | PWR_ON);
        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & IFR_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_S) & IFR_PWR_STA_MASK)) {
        }
#endif

        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) | PWR_RST_B);

        spm_write(SPM_IFR_PWR_CON, spm_read(SPM_IFR_PWR_CON) & ~SRAM_PDN);

        while ((spm_read(SPM_IFR_PWR_CON) & IFR_SRAM_ACK)) {
        }

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & IFR_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_ddrphy(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_DPY_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_DPY_PWR_CON, val);

        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & DPY_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & DPY_PWR_STA_MASK) 
                || (spm_read(SPM_PWR_STATUS_S) & DPY_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) | PWR_ON);
        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & DPY_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_S) & DPY_PWR_STA_MASK)) {
        }
#endif

        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_DPY_PWR_CON, spm_read(SPM_DPY_PWR_CON) | PWR_RST_B);

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & DPY_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_mdsys1(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;
    int count = 0;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) | MD1_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & MD1_PROT_MASK) != MD1_PROT_MASK) {
            count++;
            if(count>1000)
                break;
        }

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | MD_SRAM_PDN);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_MD_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_MD_PWR_CON, val);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
                || (spm_read(SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK)) {
        }
#endif
    } else {    /* STA_POWER_ON */
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK)) {
        }
#endif

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_RST_B);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~MD_SRAM_PDN);

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) & ~MD1_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & MD1_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_connsys(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;
    int count = 0;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) | CONN_PROT_MASK);
        while ((spm_read(TOPAXI_PROT_STA1) & CONN_PROT_MASK) != CONN_PROT_MASK) {
            count++;
            if(count>1000)	
                break;	
        }

        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) | MD_SRAM_PDN);

        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_CONN_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_CONN_PWR_CON, val);

        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) & ~(PWR_ON | PWR_ON_S));

#if 0
        udelay(1); 
        if (spm_read(SPM_PWR_STATUS) & CONN_PWR_STA_MASK) { 
            err = 1;
        }
#else
        while ((spm_read(SPM_PWR_STATUS) & CONN_PWR_STA_MASK) 
                || (spm_read(SPM_PWR_STATUS_S) & CONN_PWR_STA_MASK)) {
        }
#endif
    } else {    
        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) | PWR_ON);
        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) | PWR_ON_S);
#if 0
        udelay(1);
#else
        while (!(spm_read(SPM_PWR_STATUS) & CONN_PWR_STA_MASK) 
                || !(spm_read(SPM_PWR_STATUS_S) & CONN_PWR_STA_MASK)) {
        }
#endif

        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) | PWR_RST_B);

        spm_write(SPM_CONN_PWR_CON, spm_read(SPM_CONN_PWR_CON) & ~MD_SRAM_PDN);

#if 0
        udelay(1); 
        if (!(spm_read(SPM_PWR_STATUS) & CONN_PWR_STA_MASK)) { 
            err = 1;
        }
#endif
        spm_write(TOPAXI_PROT_EN, spm_read(TOPAXI_PROT_EN) & ~CONN_PROT_MASK);
        while (spm_read(TOPAXI_PROT_STA1) & CONN_PROT_MASK) {
        }
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

int spm_mtcmos_ctrl_mjc(int state)
{
    int err = 0;
    volatile unsigned int val;
    unsigned long flags;

    spm_mtcmos_noncpu_lock(flags);

    if (state == STA_POWER_DOWN) {

        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) | SRAM_PDN);
        spm_write(SPM_MJC_MEM_PDN1, spm_read(SPM_MJC_MEM_PDN1) | 0x1);
        //spm_write(SPM_MJC_MEM_PDN2, spm_read(SPM_MJC_MEM_PDN2) | 0x7F);
        spm_write(SPM_MJC_MEM_PDN2, spm_read(SPM_MJC_MEM_PDN2) | 0x0F);
        spm_write(SPM_MJC_MEM_PDN2, spm_read(SPM_MJC_MEM_PDN2) | 0x70);

        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) | PWR_ISO);

        val = spm_read(SPM_MJC_PWR_CON);
        val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
        spm_write(SPM_MJC_PWR_CON, val);

        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) & ~(PWR_ON | PWR_ON_S));


        while ((spm_read(SPM_PWR_STATUS) & MJC_PWR_STA_MASK) 
                || (spm_read(SPM_PWR_STATUS_S) & MJC_PWR_STA_MASK)) {
        }
    } else {    /* STA_POWER_ON */
    	spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) | PWR_ON);
        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) | PWR_ON_S);

        while (!(spm_read(SPM_PWR_STATUS) & MJC_PWR_STA_MASK) || 
                !(spm_read(SPM_PWR_STATUS_S) & MJC_PWR_STA_MASK)) {
        }

        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) | PWR_RST_B);

        spm_write(SPM_MJC_PWR_CON, spm_read(SPM_MJC_PWR_CON) & ~SRAM_PDN);
        spm_write(SPM_MJC_MEM_PDN1, spm_read(SPM_MJC_MEM_PDN1) & ~0x1);
        //spm_write(SPM_MJC_MEM_PDN2, spm_read(SPM_MJC_MEM_PDN2) & ~0x7F);
        spm_write(SPM_MJC_MEM_PDN2, spm_read(SPM_MJC_MEM_PDN2) & ~0x0F);
        spm_write(SPM_MJC_MEM_PDN2, spm_read(SPM_MJC_MEM_PDN2) & ~0x70);
    }

    spm_mtcmos_noncpu_unlock(flags);

    return err;
}

/**
 *test_spm_gpu_power_on - test whether gpu could be powered on 
 *
 *Returns 1 if power on operation succeed, 0 otherwise.
 */
int test_spm_gpu_power_on(void)
{
    int i;
    volatile unsigned int sta1, sta2;
    volatile unsigned int val;
    unsigned long flags;
 
    sta1 = spm_read(SPM_PWR_STATUS);
    sta2 = spm_read(SPM_PWR_STATUS_S);
    if (((sta1 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK) && 
            ((sta2 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK)) {
        printk("[%s]: test_spm_gpu_power_on already on, return: 1.\n", __func__);
        return 1;
    }

    spm_mtcmos_noncpu_lock(flags);

    val = spm_read(SPM_MFG_PWR_CON);
    BUG_ON(!(val & PWR_ISO));

    for(i = 0; i < 5; i++) {
    
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_S);

        udelay(5);

        sta1 = spm_read(SPM_PWR_STATUS);
        sta2 = spm_read(SPM_PWR_STATUS_S);
        if (((sta1 & MFG_PWR_STA_MASK) != MFG_PWR_STA_MASK) || 
                ((sta2 & MFG_PWR_STA_MASK) != MFG_PWR_STA_MASK)) {
            spm_mtcmos_noncpu_unlock(flags);
            printk("[%s]: test_spm_gpu_power_on return: 0.\n", __func__);
            return 0;
        }

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON | PWR_ON_S));

        sta1 = spm_read(SPM_PWR_STATUS);
        sta2 = spm_read(SPM_PWR_STATUS_S);
        if (((sta1 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK) || 
                ((sta2 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK)) {
            spm_mtcmos_noncpu_unlock(flags);
            printk("[%s]: test_spm_gpu_power_on return: 0.\n", __func__);
            return 0;
        }
        mdelay(1);
    }

    spm_mtcmos_noncpu_unlock(flags);

    printk("[%s]: test_spm_gpu_power_on return: 1.\n", __func__);
    return 1;
}

MODULE_DESCRIPTION("SPM-MTCMOS Driver v0.1");
