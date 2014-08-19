#include <linux/init.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/localtimer.h>
#include <asm/fiq_glue.h>
#include <mach/mt_reg_base.h>
#include <mach/smp.h>
#include <mach/sync_write.h>
#include <mach/hotplug.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_spm_idle.h>
#include <mach/wd_api.h>

#ifdef CONFIG_MTK_SCHED_TRACERS
#include <trace/events/mtk_events.h>
#include "kernel/trace/trace.h"
#endif

#define SLAVE1_MAGIC_REG (SRAMROM_BASE+0x38)
#define SLAVE2_MAGIC_REG (SRAMROM_BASE+0x38)
#define SLAVE3_MAGIC_REG (SRAMROM_BASE+0x38)
#define SLAVE4_MAGIC_REG (SRAMROM_BASE+0x3C)
#define SLAVE5_MAGIC_REG (SRAMROM_BASE+0x3C)
#define SLAVE6_MAGIC_REG (SRAMROM_BASE+0x3C)
#define SLAVE7_MAGIC_REG (SRAMROM_BASE+0x3C)

#define SLAVE1_MAGIC_NUM 0x534C4131
#define SLAVE2_MAGIC_NUM 0x4C415332
#define SLAVE3_MAGIC_NUM 0x41534C33
#define SLAVE4_MAGIC_NUM 0x534C4134
#define SLAVE5_MAGIC_NUM 0x4C415335
#define SLAVE6_MAGIC_NUM 0x41534C36
#define SLAVE7_MAGIC_NUM 0x534C4137

#define SLAVE_JUMP_REG  (SRAMROM_BASE+0x34)


extern void mt_secondary_startup(void);
extern void irq_raise_softirq(const struct cpumask *mask, unsigned int irq);
extern void mt_gic_secondary_init(void);
extern u32 get_devinfo_with_index(u32 index);
extern unsigned int mt_cpufreq_hotplug_notify(unsigned int ncpu);


extern unsigned int irq_total_secondary_cpus;
static unsigned int is_secondary_cpu_first_boot;
static DEFINE_SPINLOCK(boot_lock);
/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen".
 */
volatile int pen_release = -1;


/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void __cpuinit write_pen_release(int val)
{
    pen_release = val;
    smp_wmb();
    __cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
    outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}


int get_core_count(void){

    unsigned int cores = 0;
//#if defined(CONFIG_MTK_FPGA)
#if 1
    cores = get_devinfo_with_index(3);
    HOTPLUG_INFO("get_devinfo_with_index(3): 0x%08x\n", cores);
    //if bits 15:12 == 1111b, turn off cluster 1
    if ((cores & 0x0000F000) == 0x0000F000)
        cores = 4;
    else
        cores = 8;
#else
    asm volatile(
    "MRC p15, 1, %0, c9, c0, 2\n"
    : "=r" (cores)
    :
    : "cc"
    );

    cores = cores >> 24;
    cores += 1;    
#endif
    printk("return get_core_count:%d\n",cores);
    return cores;  
}

void __cpuinit platform_secondary_init(unsigned int cpu)
{
    struct wd_api *wd_api = NULL;

    printk(KERN_INFO "Slave cpu init\n");
    HOTPLUG_INFO("platform_secondary_init, cpu: %d\n", cpu);

    mt_gic_secondary_init();

    /*
     * let the primary processor know we're out of the
     * pen, then head off into the C entry point
     */
    write_pen_release(-1);

    get_wd_api(&wd_api);
    if (wd_api)
        wd_api->wd_cpu_hot_plug_on_notify(cpu);

    fiq_glue_resume();

#ifdef SPM_MCDI_FUNC
    spm_hot_plug_in_before(cpu);
#endif

#ifdef CONFIG_MTK_SCHED_TRACERS
    trace_cpu_hotplug(cpu, 1, per_cpu(last_event_ts, cpu));
    per_cpu(last_event_ts, cpu) = ns2usecs(ftrace_now(cpu));
#endif

    /*
     * Synchronise with the boot thread.
     */
    spin_lock(&boot_lock);
    spin_unlock(&boot_lock);
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    unsigned long timeout;

    printk(KERN_CRIT "Boot slave CPU\n");

    atomic_inc(&hotplug_cpu_count);

    /*
     * Set synchronisation state between this boot processor
     * and the secondary one
     */
    spin_lock(&boot_lock);

    HOTPLUG_INFO("boot_secondary, cpu: %d\n", cpu);
    /*
     * The secondary processor is waiting to be released from
     * the holding pen - release it, then wait for it to flag
     * that it has been released by resetting pen_release.
     *
     * Note that "pen_release" is the hardware CPU ID, whereas
     * "cpu" is Linux's internal ID.
     */
    /*
     * This is really belt and braces; we hold unintended secondary
     * CPUs in the holding pen until we're ready for them.  However,
     * since we haven't sent them a soft interrupt, they shouldn't
     * be there.
     */
    write_pen_release(cpu);

    switch(cpu)
    {
        case 1:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE1_MAGIC_NUM, SLAVE1_MAGIC_REG);
                HOTPLUG_INFO("SLAVE1_MAGIC_NUM:%x\n", SLAVE1_MAGIC_NUM);
            
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu1(STA_POWER_ON, 1);
            }
        #endif
            break;
        case 2:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE2_MAGIC_NUM, SLAVE2_MAGIC_REG);
                HOTPLUG_INFO("SLAVE2_MAGIC_NUM:%x\n", SLAVE2_MAGIC_NUM);
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu2(STA_POWER_ON, 1);
            }
        #endif
            break;
        case 3:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE3_MAGIC_NUM, SLAVE3_MAGIC_REG);
                HOTPLUG_INFO("SLAVE3_MAGIC_NUM:%x\n", SLAVE3_MAGIC_NUM);
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu3(STA_POWER_ON, 1);
            }
        #endif
            break;
        case 4:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE4_MAGIC_NUM, SLAVE4_MAGIC_REG);
                HOTPLUG_INFO("SLAVE4_MAGIC_NUM:%x\n", SLAVE4_MAGIC_NUM);
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu4(STA_POWER_ON, 1);
            }
        #endif
            break;

        case 5:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE5_MAGIC_NUM, SLAVE5_MAGIC_REG);
                HOTPLUG_INFO("SLAVE5_MAGIC_NUM:%x\n", SLAVE5_MAGIC_NUM);
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                if ((cpu_online(4) == 0) && (cpu_online(6) == 0) && (cpu_online(7) == 0))
                {
                    HOTPLUG_INFO("up CPU%d fail, please up CPU4 first\n", cpu);
                    spin_unlock(&boot_lock);
                    return -ENOSYS;
                }
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu5(STA_POWER_ON, 1);
            }
        #endif
            break;
        case 6:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE6_MAGIC_NUM, SLAVE6_MAGIC_REG);
                HOTPLUG_INFO("SLAVE6_MAGIC_NUM:%x\n", SLAVE6_MAGIC_NUM);
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                if ((cpu_online(4) == 0) && (cpu_online(5) == 0) && (cpu_online(7) == 0))
                {
                    HOTPLUG_INFO("up CPU%d fail, please up CPU4 first\n", cpu);
                    spin_unlock(&boot_lock);
                    return -ENOSYS;
                }
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu6(STA_POWER_ON, 1);
            }
        #endif
            break;

        case 7:
            if (is_secondary_cpu_first_boot)
            {
                --is_secondary_cpu_first_boot;
                mt65xx_reg_sync_writel(SLAVE7_MAGIC_NUM, SLAVE7_MAGIC_REG);
                HOTPLUG_INFO("SLAVE7_MAGIC_NUM:%x\n", SLAVE7_MAGIC_NUM);
            }
        #ifdef CONFIG_HOTPLUG_WITH_POWER_CTRL
            else
            {
                if ((cpu_online(4) == 0) && (cpu_online(5) == 0) && (cpu_online(6) == 0))
                {
                    HOTPLUG_INFO("up CPU%d fail, please up CPU4 first\n", cpu);
                    spin_unlock(&boot_lock);
                    return -ENOSYS;
                }
                mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOT_ADDR);
                spm_mtcmos_ctrl_cpu7(STA_POWER_ON, 1);
            }
        #endif
            break;


        default:
            break;

    }

    smp_cross_call(cpumask_of(cpu));

    timeout = jiffies + (1 * HZ);
    while (time_before(jiffies, timeout)) {
        smp_rmb();
        if (pen_release == -1)
            break;

        udelay(10);
    }
    
    /*
     * Now the secondary core is starting up let it run its
     * calibrations, then wait for it to finish
     */
    spin_unlock(&boot_lock);

    if (pen_release == -1)
    {
        mt_cpufreq_hotplug_notify(num_online_cpus());
        return 0;
    }
    else
    {
        if (cpu < 4)
        {
            mt65xx_reg_sync_writel(cpu + 8, MP0_DBG_CTRL);
            printk(KERN_EMERG "CPU%u, MP0_DBG_CTRL: 0x%08x, MP0_DBG_FLAG: 0x%08x\n", cpu, *(volatile u32 *)(MP0_DBG_CTRL), *(volatile u32 *)(MP0_DBG_FLAG));
        }
        else
        {
            mt65xx_reg_sync_writel(cpu - 4 + 8, MP1_DBG_CTRL);
            printk(KERN_EMERG "CPU%u, MP1_DBG_CTRL: 0x%08x, MP1_DBG_FLAG: 0x%08x\n", cpu, *(volatile u32 *)(MP1_DBG_CTRL), *(volatile u32 *)(MP1_DBG_FLAG));
        }
        on_each_cpu((smp_call_func_t)dump_stack, NULL, 0);
        return -ENOSYS;
    }
}

void __init smp_init_cpus(void)
{
    unsigned int i = 0, ncores;
    
    /* Enable CA7 snoop function */
    REG_WRITE(MP0_AXI_CONFIG, REG_READ(MP0_AXI_CONFIG) & ~ACINACTM);
    
    /* Enables DVM */
    asm volatile(
        "MRC p15, 0, %0, c1, c0, 1\n"
        "BIC %0, %0, #1 << 15\n"        /* DDVM: bit15 */
        "MCR p15, 0, %0, c1, c0, 1\n"
        : "+r"(i)
        :
        : "cc"
    );
    
    /* Enable snoop requests and DVM message requests*/
    REG_WRITE(CCI400_SI4_SNOOP_CONTROL, REG_READ(CCI400_SI4_SNOOP_CONTROL) | (SNOOP_REQ | DVM_MSG_REQ));
    while (REG_READ(CCI400_STATUS) & CHANGE_PENDING);
    
    ncores = get_core_count();
    if (ncores > NR_CPUS) {
        printk(KERN_WARNING
               "L2CTLR core count (%d) > NR_CPUS (%d)\n", ncores, NR_CPUS);
        printk(KERN_WARNING
               "set nr_cores to NR_CPUS (%d)\n", NR_CPUS);
        ncores = NR_CPUS;
    }

    for (i = 0; i < ncores; i++)
        set_cpu_possible(i, true);

    irq_total_secondary_cpus = num_possible_cpus() - 1;
    is_secondary_cpu_first_boot = num_possible_cpus() - 1;

    set_smp_cross_call(irq_raise_softirq);
    
    if (ncores > 4)
        spm_mtcmos_ctrl_cpusys1_init_1st_bring_up(STA_POWER_ON);
    else
        spm_mtcmos_ctrl_cpusys1_init_1st_bring_up(STA_POWER_DOWN);
}

void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
    int i;

    for (i = 0; i < max_cpus; i++)
        set_cpu_present(i, true);


    /* write the address of slave startup into the system-wide flags register */
    mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), SLAVE_JUMP_REG);
  
}
