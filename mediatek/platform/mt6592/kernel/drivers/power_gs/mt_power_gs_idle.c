#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *AP_CG_gs_idle;
extern unsigned int AP_CG_gs_idle_len;

extern unsigned int *AP_DCM_gs_idle;
extern unsigned int AP_DCM_gs_idle_len;

extern unsigned int *PMIC23_LDO_BUCK_gs_idle;
extern unsigned int PMIC23_LDO_BUCK_gs_idle_len;

extern unsigned int *CHG33_CHG_BUCK_gs_idle;
extern unsigned int CHG33_CHG_BUCK_gs_idle_len;

void mt_power_gs_dump_idle(void)
{
    mt_power_gs_compare("Idle",                             \
                        AP_CG_gs_idle, AP_CG_gs_idle_len,   \
                        AP_DCM_gs_idle, AP_DCM_gs_idle_len, \
                        PMIC23_LDO_BUCK_gs_idle, PMIC23_LDO_BUCK_gs_idle_len, \
                        CHG33_CHG_BUCK_gs_idle, CHG33_CHG_BUCK_gs_idle_len);
}

static int dump_idle_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : idle\n");

    mt_power_gs_dump_idle();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_idle_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_idle_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_idle", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_idle_read;
        }
    }

    return 0;
}

module_init(mt_power_gs_idle_init);
module_exit(mt_power_gs_idle_exit);

MODULE_DESCRIPTION("MT Power Golden Setting - idle");