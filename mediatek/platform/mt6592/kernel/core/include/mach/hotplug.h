#ifndef _HOTPLUG
#define _HOTPLUG

#include <linux/xlog.h>
#include <linux/kernel.h>   //printk
#include <asm/atomic.h>
#include <mach/mt_reg_base.h>
#include <mach/sync_write.h>


/* log */
#define HOTPLUG_LOG_NONE                                0
#define HOTPLUG_LOG_WITH_XLOG                           1
#define HOTPLUG_LOG_WITH_PRINTK                         2

#define HOTPLUG_LOG_PRINT                               HOTPLUG_LOG_WITH_PRINTK

#if (HOTPLUG_LOG_PRINT == HOTPLUG_LOG_NONE)
#define HOTPLUG_INFO(fmt, args...)                    
#elif (HOTPLUG_LOG_PRINT == HOTPLUG_LOG_WITH_XLOG)
#define HOTPLUG_INFO(fmt, args...)                      xlog_printk(ANDROID_LOG_INFO, "Power/hotplug", fmt, ##args)
#elif (HOTPLUG_LOG_PRINT == HOTPLUG_LOG_WITH_PRINTK)
#define HOTPLUG_INFO(fmt, args...)                      printk("[Power/hotplug] "fmt, ##args)
#endif


/* profilling */
//#define CONFIG_HOTPLUG_PROFILING                        
#define CONFIG_HOTPLUG_PROFILING_COUNT                  100


/* register address */
#define BOOT_ADDR                                       (INFRACFG_AO_BASE + 0x800)

#define CCI400_STATUS                                   (CCI400_BASE + 0x000C)
#define CHANGE_PENDING                                  (1U << 0)

//cluster 1
#define CCI400_SI3_BASE                                 (CCI400_BASE + 0x4000)
#define CCI400_SI3_SNOOP_CONTROL                        (CCI400_SI3_BASE)
//cluster 0
#define CCI400_SI4_BASE                                 (CCI400_BASE + 0x5000)
#define CCI400_SI4_SNOOP_CONTROL                        (CCI400_SI4_BASE)
#define DVM_MSG_REQ                                     (1U << 1)
#define SNOOP_REQ                                       (1U << 0)

#define MP0_AXI_CONFIG                                  (MCUSYS_CFGREG_BASE + 0x0018)
#define MP1_AXI_CONFIG                                  (MCUSYS_CFGREG_BASE + 0x0118)
#define ACINACTM                                        (1U << 4)

#define MP1_CA7_MISC_CONFIG                             (MCUSYS_CFGREG_BASE + 0x0130)
#define STANDBYWFIL2                                    (1U << 28)

#define MP0_DBG_CTRL                                    (MCUSYS_CFGREG_BASE + 0x0038)
#define MP0_DBG_FLAG                                    (MCUSYS_CFGREG_BASE + 0x003C)
#define MP1_DBG_CTRL                                    (MCUSYS_CFGREG_BASE + 0x0138)
#define MP1_DBG_FLAG                                    (MCUSYS_CFGREG_BASE + 0x013C)


/* register read/write */
#define REG_READ(addr)                                  (*(volatile u32 *)(addr))
#define REG_WRITE(addr, value)                          mt65xx_reg_sync_writel(value, addr)


/* power on/off cpu*/
#define CONFIG_HOTPLUG_WITH_POWER_CTRL


/* global variable */
extern volatile int pen_release;
extern atomic_t hotplug_cpu_count;

#ifdef CONFIG_MTK_SCHED_TRACERS
DECLARE_PER_CPU(u64, last_event_ts);
#endif

#endif //enf of #ifndef _HOTPLUG
