#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *AP_CG_gs_dpidle;
extern unsigned int AP_CG_gs_dpidle_len;

extern unsigned int *AP_DCM_gs_dpidle;
extern unsigned int AP_DCM_gs_dpidle_len;

extern unsigned int *PMIC23_LDO_BUCK_gs_dpidle;
extern unsigned int PMIC23_LDO_BUCK_gs_dpidle_len;

extern unsigned int *CHG33_CHG_BUCK_gs_dpidle;
extern unsigned int CHG33_CHG_BUCK_gs_dpidle_len;

void mt_power_gs_dump_dpidle(void)
{
    mt_power_gs_compare("DPIdle",                               \
                        AP_CG_gs_dpidle, AP_CG_gs_dpidle_len,   \
                        AP_DCM_gs_dpidle, AP_DCM_gs_dpidle_len, \
                        PMIC23_LDO_BUCK_gs_dpidle, PMIC23_LDO_BUCK_gs_dpidle_len, \
                        CHG33_CHG_BUCK_gs_dpidle, CHG33_CHG_BUCK_gs_dpidle_len);
}

static int dump_dpidle_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : dpidle\n");

    mt_power_gs_dump_dpidle();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_dpidle_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_dpidle_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_dpidle", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_dpidle_read;
        }
    }

    return 0;
}

module_init(mt_power_gs_dpidle_init);
module_exit(mt_power_gs_dpidle_exit);

MODULE_DESCRIPTION("MT Power Golden Setting - dpidle");