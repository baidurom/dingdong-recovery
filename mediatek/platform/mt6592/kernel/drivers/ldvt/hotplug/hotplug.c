#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cnt32_to_63.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <mach/mt_spm_mtcmos.h>

//#include <asm/uaccess.h>
//#include <asm/tcm.h>
//#include <mach/timer.h>
//#include <mach/irqs.h>
//

/*
    This is a Kernel driver used by user space. 
    This driver will using the interface of the GPT production driver.  
    We implement some IOCTLs: 
     0) UP CPU1
     1) DOWN CPU1
     2) UP CPU2
     3) DOWN CPU2
     4) UP CPU3
     5) DOWN CPU3
     6) UP CPU4
     7) DOWN CPU4
     8) UP CPU5
     9) DOWN CPU6
    10) UP CPU6
    11) DOWN CPU6
    12) UP CPU7
    13) DOWN CPU7
    14) STRESS 1 UP DOWN CPUS
    15) STRESS 2 UP DOWN CPUS
    16) UP DBG0
    17) DOWN DBG0
    18) UP DBG1
    19) DOWN DBG1
*/

#define hotplugname                             "uvvp_hotplug"

/*IOCTL code Define*/
#define UVVP_HOTPLUG_UP_CPU1                    _IOW('k', 0, int)
#define UVVP_HOTPLUG_DOWN_CPU1                  _IOW('k', 1, int)
#define UVVP_HOTPLUG_UP_CPU2                    _IOW('k', 2, int)
#define UVVP_HOTPLUG_DOWN_CPU2                  _IOW('k', 3, int)
#define UVVP_HOTPLUG_UP_CPU3                    _IOW('k', 4, int)
#define UVVP_HOTPLUG_DOWN_CPU3                  _IOW('k', 5, int)
#define UVVP_HOTPLUG_UP_CPU4                    _IOW('k', 6, int)
#define UVVP_HOTPLUG_DOWN_CPU4                  _IOW('k', 7, int)
#define UVVP_HOTPLUG_UP_CPU5                    _IOW('k', 8, int)
#define UVVP_HOTPLUG_DOWN_CPU5                  _IOW('k', 9, int)
#define UVVP_HOTPLUG_UP_CPU6                    _IOW('k', 10, int)
#define UVVP_HOTPLUG_DOWN_CPU6                  _IOW('k', 11, int)
#define UVVP_HOTPLUG_UP_CPU7                    _IOW('k', 12, int)
#define UVVP_HOTPLUG_DOWN_CPU7                  _IOW('k', 13, int)
#define UVVP_HOTPLUG_STRESS_1_UP_DOWN_CPUS      _IOW('k', 14, int)
#define UVVP_HOTPLUG_STRESS_2_UP_DOWN_CPUS      _IOW('k', 15, int)
#define UVVP_HOTPLUG_UP_DBG0                    _IOW('k', 16, int)
#define UVVP_HOTPLUG_DOWN_DBG0                  _IOW('k', 17, int)
#define UVVP_HOTPLUG_UP_DBG1                    _IOW('k', 18, int)
#define UVVP_HOTPLUG_DOWN_DBG1                  _IOW('k', 19, int)

//#define STRESS_TEST_1_COUNT                     1000
#define STRESS_TEST_1_COUNT                     1
//#define STRESS_TEST_1_DELAY_MS                  2000
#define STRESS_TEST_1_DELAY_MS                  111

//#define STRESS_TEST_2_COUNT                     30000
#define STRESS_TEST_2_COUNT                     2
#define STRESS_TEST_2_DELAY_MS                  111         //mt6589:  11 ms ok
                                                            //mt6582:  11 ms not random enough
                                                            //        111 ms ok

//Let's use struct GPT_CONFIG for all IOCTL: 
static long uvvp_hotplug_ioctl(struct file *file,
                            unsigned int cmd, unsigned long arg)
{
    #ifdef Lv_debug
    printk("\r\n******** uvvp_hotplug_ioctl cmd[%d]********\r\n",cmd);
    #endif 
    
    /*
     * 20121101 marc.huang 
     * mark to fix build warning
     */
    //void __user *argp = (void __user *)arg;
    //int __user *p = argp;
    
    int i, j, k, cpu_index, cpu_count;

    switch (cmd) {
        default:
            return -1;

        case UVVP_HOTPLUG_UP_CPU1:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(1) ********\r\n");
            cpu_up(1);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU1:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(1) ********\r\n");
            cpu_down(1);
            return 0;
        
        case UVVP_HOTPLUG_UP_CPU2:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(2) ********\r\n");
            cpu_up(2);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU2:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(2) ********\r\n");
            cpu_down(2);
            return 0;
        
        case UVVP_HOTPLUG_UP_CPU3:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(3) ********\r\n");
            cpu_up(3);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU3:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(3) ********\r\n");
            cpu_down(3);
            return 0;
        
        case UVVP_HOTPLUG_UP_CPU4:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(4) ********\r\n");
            cpu_up(4);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU4:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(4) ********\r\n");
            cpu_down(4);
            return 0;
        
        case UVVP_HOTPLUG_UP_CPU5:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(4) ********\r\n");
            cpu_up(5);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU5:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(4) ********\r\n");
            cpu_down(5);
            return 0;
        
        case UVVP_HOTPLUG_UP_CPU6:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(6) ********\r\n");
            cpu_up(6);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU6:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(6) ********\r\n");
            cpu_down(6);
            return 0;
        
        case UVVP_HOTPLUG_UP_CPU7:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_up(7) ********\r\n");
            cpu_up(7);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_CPU7:
            printk("\r\n******** uvvp_hotplug_ioctl cpu_down(7) ********\r\n");
            cpu_down(7);
            return 0;
        
        case UVVP_HOTPLUG_UP_DBG0:
            printk("\r\n******** uvvp_hotplug_ioctl spm_mtcmos_ctrl_dbg0(STA_POWER_ON) ********\r\n");
            spm_mtcmos_ctrl_dbg0(STA_POWER_ON);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_DBG0:
            printk("\r\n******** uvvp_hotplug_ioctl spm_mtcmos_ctrl_dbg0(STA_POWER_DOWN) ********\r\n");
            spm_mtcmos_ctrl_dbg0(STA_POWER_DOWN);
            return 0;
        
        case UVVP_HOTPLUG_UP_DBG1:
            printk("\r\n******** uvvp_hotplug_ioctl spm_mtcmos_ctrl_dbg1(STA_POWER_ON) ********\r\n");
            spm_mtcmos_ctrl_dbg1(STA_POWER_ON);
            return 0;
        
        case UVVP_HOTPLUG_DOWN_DBG1:
            printk("\r\n******** uvvp_hotplug_ioctl spm_mtcmos_ctrl_dbg1(STA_POWER_DOWN) ********\r\n");
            spm_mtcmos_ctrl_dbg1(STA_POWER_DOWN);
            return 0;
        
        case UVVP_HOTPLUG_STRESS_1_UP_DOWN_CPUS:
            printk("\r\n******** uvvp_hotplug_ioctl stress_test_1 cpu_up/cpu_down(1/2/3/4/5/6/7) ********\r\n");
            
            //0. turn on all the cpus
            for (i = 1; i < 8; ++i)
                cpu_up(i);
            
            for (i = 0; i < STRESS_TEST_1_COUNT; ++i)
            {
                for (j = 1; j < 8; ++j) //cpu_count
                {
                    for (k = 1; k < 8; ++k) //index
                    {
                        cpu_index = k;
                        cpu_count = j;
                        while (cpu_count--)
                        {
                            cpu_down(cpu_index);
                            if (++cpu_index == 8)
                                cpu_index = 1;
                        }
                        msleep(STRESS_TEST_1_DELAY_MS);
                        
                        cpu_index = k;
                        cpu_count = j;
                        while (cpu_count--)
                        {
                            cpu_up(cpu_index);
                            if (++cpu_index == 8)
                                cpu_index = 1;
                        }
                        msleep(STRESS_TEST_1_DELAY_MS);
                    }
                }
            }
                      
            /*            
            //1. turn off 1 cpu at one time
            for (i = 0; i < STRESS_TEST_1_COUNT; ++i)
            {
                for (j = 1; j < 4; ++j)
                {
                    cpu_down(j);
                    msleep(STRESS_TEST_1_DELAY_MS);
                    cpu_up(j);
                    msleep(STRESS_TEST_1_DELAY_MS);
                }
            }
            
            //2. turn off 2 cpus at one time
            for (i = 0; i < STRESS_TEST_1_COUNT; ++i)
            {
                for (j = 1; j < 4; ++j)
                {
                    cpu_down(j);
                    cpu_down( ((j + 1 == 4) ? 1 : j + 1) );
                    msleep(STRESS_TEST_1_DELAY_MS);
                    cpu_up(j);
                    cpu_up( ((j + 1 == 4) ? 1 : j + 1) );
                    msleep(STRESS_TEST_1_DELAY_MS);
                }
            }
            
            //3. turn off 3 cpus at one time
            for (i = 0; i < STRESS_TEST_1_COUNT; ++i)
            {
                for (j = 1; j < 4; ++j)
                {
                    cpu_down(j);
                }
                msleep(STRESS_TEST_1_DELAY_MS);
                
                for (j = 1; j < 4; ++j)
                {
                    cpu_up(j);
                }
                msleep(STRESS_TEST_1_DELAY_MS);
            }
            */
            return 0;
            
        case UVVP_HOTPLUG_STRESS_2_UP_DOWN_CPUS:
            printk("\r\n******** uvvp_hotplug_ioctl stress_test_2 cpu_up/cpu_down(1/2/3) ********\r\n");
            
            for (i = 0; i < STRESS_TEST_2_COUNT; ++i)
            {
                j = jiffies % 7 + 1;
                if (cpu_online(j))
                {
                    printk("@@@@@ %8d: cpu_down(%d) @@@@@\n", i, j);
                    cpu_down(j);
                }
                else
                {
                    printk("@@@@@ %8d: cpu_up(%d) @@@@@\n", i, j);
                    cpu_up(j);
                }
                msleep(STRESS_TEST_2_DELAY_MS);
            }
            
            return 0;
        
    }

    return 0;    
}

static int uvvp_hotplug_open(struct inode *inode, struct file *file)
{
    return 0;
}


static struct file_operations uvvp_hotplug_fops = {
    .owner              = THIS_MODULE,

    .open               = uvvp_hotplug_open,
    .unlocked_ioctl     = uvvp_hotplug_ioctl,
    .compat_ioctl       = uvvp_hotplug_ioctl,
};

static struct miscdevice uvvp_hotplug_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = hotplugname,
    .fops = &uvvp_hotplug_fops,
};

static int __init uvvp_hotplug_init(void)
{
    misc_register(&uvvp_hotplug_dev);
    return 0;
}

static void __exit uvvp_hotplug_exit(void)
{

}

module_init(uvvp_hotplug_init);
module_exit(uvvp_hotplug_exit);


