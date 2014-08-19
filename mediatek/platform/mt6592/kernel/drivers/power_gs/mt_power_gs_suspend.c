#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *AP_CG_gs_suspend;
extern unsigned int AP_CG_gs_suspend_len;

extern unsigned int *AP_DCM_gs_suspend;
extern unsigned int AP_DCM_gs_suspend_len;

extern unsigned int *PMIC23_LDO_BUCK_gs_suspend;
extern unsigned int PMIC23_LDO_BUCK_gs_suspend_len;

extern unsigned int *CHG33_CHG_BUCK_gs_suspend;
extern unsigned int CHG33_CHG_BUCK_gs_suspend_len;

void mt_power_gs_dump_suspend(void)
{
    mt_power_gs_compare("Suspend",                                \
                        AP_CG_gs_suspend, AP_CG_gs_suspend_len,   \
                        AP_DCM_gs_suspend, AP_DCM_gs_suspend_len, \
                        PMIC23_LDO_BUCK_gs_suspend, PMIC23_LDO_BUCK_gs_suspend_len, \
                        CHG33_CHG_BUCK_gs_suspend, CHG33_CHG_BUCK_gs_suspend_len);
}

static int dump_suspend_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : suspend\n");

    mt_power_gs_dump_suspend();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_suspend_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_suspend_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_suspend", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_suspend_read;
        }
    }

    return 0;
}

module_init(mt_power_gs_suspend_init);
module_exit(mt_power_gs_suspend_exit);

MODULE_DESCRIPTION("MT Power Golden Setting - Suspend");