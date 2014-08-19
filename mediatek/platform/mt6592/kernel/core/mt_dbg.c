#include <linux/device.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#ifdef CONFIG_SMP
#include <mach/hotplug.h>
#include <linux/cpu.h>
#endif

#define UNLOCK_KEY 0xC5ACCE55
#define HDBGEN (1 << 14)
#define MDBGEN (1 << 15)
#define DBGLAR 0xF0170FB0
#define DBGOSLAR 0xF0170300
#define DBGDSCR 0xF0170088
#define DBGLAR2 0xF0190FB0
#define DBGOSLAR2 0xF0190300
#define DBGDSCR2 0xF0190088
#define DBGWVR_BASE 0xF0170180
#define DBGWCR_BASE 0xF01701C0
#define DBGBVR_BASE 0xF0170100
#define DBGBCR_BASE 0xF0170140
#define DBGWVR_BASE2 0xF0190180
#define DBGWCR_BASE2 0xF01901C0
#define DBGBVR_BASE2 0xF0190100
#define DBGBCR_BASE2 0xF0190140

#define DBGWFAR 0xF0170018
#define MAX_NR_WATCH_POINT 4
#define MAX_NR_BREAK_POINT 6
#define NUM_CPU 4   // # of cpu in a cluster
extern void save_dbg_regs(unsigned int data[]);
extern void restore_dbg_regs(unsigned int data[]);
extern unsigned int get_cluster_core_count(void);

void save_dbg_regs(unsigned int data[])
{
    //register unsigned int cpu_id;
    int i;
    //__asm__ __volatile__ ("MRC   p15, 0, %0, c0, c0, 5" :"=r"(cpu_id) );
    //cpu_id &= 0xf;

    // actually only cpu0 will execute this function

    //data[0] = *(volatile unsigned int *)DBGDSCR;
    data[0] = readl(DBGDSCR);
    for(i = 0; i < MAX_NR_WATCH_POINT; i++) {
        //data[i*2+1] = *(((volatile unsigned int *)(DBGWVR_BASE + cpu_id * 0x2000)) + i);
        data[i*2+1] = readl(DBGWVR_BASE + i * sizeof(unsigned int *));
        //data[i*2+2] = *(((volatile unsigned int *)(DBGWCR_BASE + cpu_id * 0x2000)) + i);
        data[i*2+2] = readl(DBGWCR_BASE + i * sizeof(unsigned int *));
    }

    for(i = 0; i < MAX_NR_BREAK_POINT; i++) {
        //data[i*2+9] = *(((volatile unsigned int *)(DBGBVR_BASE + cpu_id * 0x2000)) + i);
        data[i*2+9] = readl(DBGBVR_BASE + i * sizeof(unsigned int *));
        //data[i*2+10] = *(((volatile unsigned int *)(DBGBCR_BASE+ cpu_id * 0x2000)) + i);
        data[i*2+10] = readl(DBGBCR_BASE + i * sizeof(unsigned int *));
    }
}

void restore_dbg_regs(unsigned int data[])
{
    //register unsigned int cpu_id;
    int i;
    //__asm__ __volatile__ ("MRC   p15, 0, %0, c0, c0, 5" :"=r"(cpu_id) );
    //cpu_id &= 0xf;

    // actually only cpu0 will execute this function
    
    //*(volatile unsigned int *)(DBGLAR   + cpu_id * 0x2000) = UNLOCK_KEY;
    mt_reg_sync_writel(UNLOCK_KEY, DBGLAR);
    //*(volatile unsigned int *)(DBGOSLAR + cpu_id * 0x2000) = ~UNLOCK_KEY;
    mt_reg_sync_writel(~UNLOCK_KEY, DBGOSLAR);
    //*(volatile unsigned int *)(DBGDSCR  + cpu_id * 0x2000) = data[0];
    mt_reg_sync_writel(data[0], DBGDSCR);

    for(i = 0; i < MAX_NR_WATCH_POINT; i++) {
        //*(((volatile unsigned int *)(DBGWVR_BASE + cpu_id * 0x2000)) + i) = data[i*2+1];
        mt_reg_sync_writel(data[i*2+1], DBGWVR_BASE + i * sizeof(unsigned int *));
        //*(((volatile unsigned int *)(DBGWCR_BASE + cpu_id * 0x2000)) + i) = data[i*2+2];
        mt_reg_sync_writel(data[i*2+2], DBGWCR_BASE + i * sizeof(unsigned int *));
    } 
        
    for(i = 0; i < MAX_NR_BREAK_POINT; i++) {
        //*(((volatile unsigned int *)(DBGBVR_BASE + cpu_id * 0x2000)) + i) = data[i*2+9];
        mt_reg_sync_writel(data[i*2+9], DBGBVR_BASE + i * sizeof(unsigned int *));
        //*(((volatile unsigned int *)(DBGBCR_BASE + cpu_id * 0x2000)) + i) = data[i*2+10];
        mt_reg_sync_writel(data[i*2+10], DBGBCR_BASE + i * sizeof(unsigned int *));
    }
}

#ifdef CONFIG_SMP
static int __cpuinit
regs_hotplug_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
//        printk(KERN_ALERT "In hotplug callback\n");
	int i;
    unsigned int cpu = (unsigned int) hcpu;
    unsigned cluster_id = cpu / get_cluster_core_count();
    //printk("regs_hotplug_callback cpu = %d\n", cpu);
    switch (action) {
    case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
        // for cluster 0
        if(cluster_id == 0) {
            printk("cpu = %d, cluster = %d\n", cpu, cluster_id);
            //*(volatile unsigned int *)(DBGLAR + cpu *0x2000) = UNLOCK_KEY;
            mt_reg_sync_writel(UNLOCK_KEY, DBGLAR + cpu *0x2000);
            printk("after write UNLOCK to DBGLAR\n");
            //*(volatile unsigned int *)(DBGOSLAR + cpu * 0x2000) = ~UNLOCK_KEY;
            mt_reg_sync_writel(~UNLOCK_KEY, DBGOSLAR + cpu *0x2000);
            printk("after write ~UNLOCK to DBGOSLAR\n");
            //*(volatile unsigned int *)(DBGDSCR + cpu * 0x2000) |= *(volatile unsigned int *)(DBGDSCR);
            mt_reg_sync_writel(readl(DBGDSCR + cpu *0x2000) | readl(DBGDSCR), DBGDSCR + cpu *0x2000);
            printk("after write to DBGOSLAR: 0x%x\n", readl(DBGDSCR + cpu *0x2000));
			
            for(i = 0; i < MAX_NR_WATCH_POINT; i++) {
                //*(((volatile unsigned int *)(DBGWVR_BASE + cpu * 0x2000)) + i) = *(((volatile unsigned int *)DBGWVR_BASE) + i);
                mt_reg_sync_writel(readl(DBGWVR_BASE + i * sizeof(unsigned int*)), DBGWVR_BASE + cpu * 0x2000 + i * sizeof(unsigned int *));
                //*(((volatile unsigned int *)(DBGWCR_BASE + cpu * 0x2000)) + i) = *(((volatile unsigned int *)DBGWCR_BASE) + i);
                mt_reg_sync_writel(readl(DBGWCR_BASE + i * sizeof(unsigned int*)), DBGWCR_BASE + cpu * 0x2000 + i * sizeof(unsigned int *));
            }
		
            for(i = 0; i < MAX_NR_BREAK_POINT; i++) {
                //*(((volatile unsigned int *)(DBGBVR_BASE + cpu * 0x2000)) + i) = *(((volatile unsigned int *)DBGBVR_BASE) + i);
                mt_reg_sync_writel(readl(DBGBVR_BASE + i * sizeof(unsigned int*)), DBGBVR_BASE + cpu * 0x2000 + i * sizeof(unsigned int *));
                //*(((volatile unsigned int *)(DBGBCR_BASE + cpu * 0x2000)) + i) = *(((volatile unsigned int *)DBGBCR_BASE) + i);
                mt_reg_sync_writel(readl(DBGBCR_BASE + i * sizeof(unsigned int*)), DBGBCR_BASE + cpu * 0x2000 + i * sizeof(unsigned int *));
            }
            printk("after write to cpu = %d, cluster = %d\n", cpu, cluster_id);
        }
        else if(cluster_id == 1) {
            unsigned int cpu2 = cpu - NUM_CPU * cluster_id;
            printk("cpu2 = %d, cluster = %d\n", cpu2, cluster_id);
            //*(volatile unsigned int *)(DBGLAR2 + cpu * 0x2000) = UNLOCK_KEY;
            mt_reg_sync_writel(UNLOCK_KEY, DBGLAR2 + cpu2 *0x2000);
            printk("after write UNLOCK to DBGLAR\n");
            //*(volatile unsigned int *)(DBGOSLAR2 + cpu * 0x2000) = ~UNLOCK_KEY;
            mt_reg_sync_writel(~UNLOCK_KEY, DBGOSLAR2 + cpu2 *0x2000);
            printk("after write ~UNLOCK to DBGOSLAR\n");
            //*(volatile unsigned int *)(DBGDSCR2 + cpu * 0x2000) |= *(volatile unsigned int *)(DBGDSCR);
            mt_reg_sync_writel(readl(DBGDSCR2 + cpu2 *0x2000) | readl(DBGDSCR), DBGDSCR2 + cpu2 *0x2000);
            printk("after write to DBGOSLAR: 0x%x\n", readl(DBGDSCR2 + cpu2 *0x2000));
			
            for(i = 0; i < MAX_NR_WATCH_POINT; i++) {
                //*(((volatile unsigned int *)(DBGWVR_BASE2 + cpu2 * 0x2000)) + i) = *(((volatile unsigned int *)DBGWVR_BASE2) + i);
                mt_reg_sync_writel(readl(DBGWVR_BASE + i * sizeof(unsigned int*)), DBGWVR_BASE2 + cpu2 * 0x2000 + i * sizeof(unsigned int *));
                //*(((volatile unsigned int *)(DBGWCR_BASE2 + cpu2 * 0x2000)) + i) = *(((volatile unsigned int *)DBGWCR_BASE2) + i);
                mt_reg_sync_writel(readl(DBGWCR_BASE + i * sizeof(unsigned int*)), DBGWCR_BASE2 + cpu2 * 0x2000 + i * sizeof(unsigned int *));
            }
		
            for(i = 0; i < MAX_NR_BREAK_POINT; i++) {
                //*(((volatile unsigned int *)(DBGBVR_BASE2 + cpu2 * 0x2000)) + i) = *(((volatile unsigned int *)DBGBVR_BASE2) + i);
                mt_reg_sync_writel(readl(DBGBVR_BASE + i * sizeof(unsigned int*)), DBGBVR_BASE2 + cpu2 * 0x2000 + i * sizeof(unsigned int *));
                //*(((volatile unsigned int *)(DBGBCR_BASE2 + cpu2 * 0x2000)) + i) = *(((volatile unsigned int *)DBGBCR_BASE2) + i);
                mt_reg_sync_writel(readl(DBGBCR_BASE + i * sizeof(unsigned int*)), DBGBCR_BASE2 + cpu2 * 0x2000 + i * sizeof(unsigned int *));
            }
            printk("after write to cpu = %d, cluster = %d\n", cpu2, cluster_id);
        }
		
	break;

	default: 
	break;
    }

        return NOTIFY_OK;
}

static struct notifier_block __cpuinitdata cpu_nfb = {
        .notifier_call = regs_hotplug_callback
};

static int __init regs_backup(void)
{
    
    register_cpu_notifier(&cpu_nfb);

    return 0;
}

module_init(regs_backup);
#endif
