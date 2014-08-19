#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/aee.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_spm_idle.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_power_gs.h>

#include <mach/mt_pmic_wrap.h>
#include <mach/pmic_mt6323_sw.h>
#include <mach/mt6333.h>
#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>

#define gs_read(addr) (*(volatile u32 *)(addr))

struct proc_dir_entry *mt_power_gs_dir = NULL;

static kal_uint16 gs6323_pmic_read(kal_uint16 reg)
{
    kal_uint32 ret = 0;
    kal_uint32 reg_val = 0;

    ret = pmic_read_interface(reg, &reg_val, 0xFFFF, 0x0);

    return (kal_uint16)reg_val;
}

static kal_uint8 gs6333_pmic_read(kal_uint8 reg)
{
    kal_uint32 ret = 0;
    kal_uint8 reg_val = 0;

    ret = mt6333_read_interface(reg, &reg_val, 0xFF, 0x0);

    return (kal_uint8)reg_val;
}

static void mt_power_gs_compare_pll(void)
{
    if (pll_is_on(MSDCPLL)) {
        printk("MSDCPLL: %s\n", pll_is_on(MSDCPLL) ?  "on" : "off");
    }

    if (subsys_is_on(SYS_MD1)) {
        printk("SYS_MD1: %s\n", subsys_is_on(SYS_MD1) ? "on" : "off");
    }

    if (subsys_is_on(SYS_CONN)) {
        printk("SYS_CONN: %s\n", subsys_is_on(SYS_CONN) ? "on" : "off");
    }

    if (subsys_is_on(SYS_DIS)) {
        printk("SYS_DIS: %s\n", subsys_is_on(SYS_DIS) ? "on" : "off");
    }

    if (subsys_is_on(SYS_MFG)) {
        printk("SYS_MFG: %s\n", subsys_is_on(SYS_MFG) ? "on" : "off");
    }

    if (subsys_is_on(SYS_ISP)) {
        printk("SYS_ISP: %s\n", subsys_is_on(SYS_ISP) ? "on" : "off");
    }

    if (subsys_is_on(SYS_VDE)) {
        printk("SYS_VDE: %s\n", subsys_is_on(SYS_VDE) ? "on" : "off");
    }
}

void mt_power_gs_diff_output(unsigned int val1, unsigned int val2)
{
    int i = 0;
    unsigned int diff = val1 ^ val2;

    while (diff != 0)
    {
        if ((diff % 2) != 0) printk("%d ", i);
        diff /= 2;
        i++;
    }
    printk("\n");
}

void mt_power_gs_compare(char *scenario, \
                         unsigned int *ap_cg_gs, unsigned int ap_cg_gs_len, \
                         unsigned int *ap_dcm_gs, unsigned int ap_dcm_gs_len, \
                         unsigned int *pmic_ldo_buck_gs, unsigned int pmic_ldo_buck_gs_len, \
                         unsigned int *pmic_chg_buck_gs, unsigned int pmic_chg_buck_gs_len)
{
    unsigned int i, val1, val2;

    // AP CG
    for (i = 0; i < ap_cg_gs_len; i += 3)
    {
        aee_sram_printk("%d\n", i);
        val1 = gs_read(ap_cg_gs[i]) & ap_cg_gs[i + 1];
        val2 = ap_cg_gs[i + 2] & ap_cg_gs[i + 1];
        if (val1 != val2)
        {
            printk("%s - AP CG - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, ap_cg_gs[i], gs_read(ap_cg_gs[i]), ap_cg_gs[i + 1], ap_cg_gs[i + 2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }

    // AP DCM
    for (i = 0; i < ap_dcm_gs_len; i += 3)
    {
        aee_sram_printk("%d\n", i);
        val1 = gs_read(ap_dcm_gs[i]) & ap_dcm_gs[i + 1];
        val2 = ap_dcm_gs[i + 2] & ap_dcm_gs[i + 1];
        if (val1 != val2)
        {
            printk("%s - AP DCM - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, ap_dcm_gs[i], gs_read(ap_dcm_gs[i]), ap_dcm_gs[i + 1], ap_dcm_gs[i + 2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }

    // LDO BUCK
    for (i = 0; i < pmic_ldo_buck_gs_len; i += 3)
    {
        aee_sram_printk("%d\n", i);
        val1 = gs6323_pmic_read(pmic_ldo_buck_gs[i]) & pmic_ldo_buck_gs[i + 1];
        val2 = pmic_ldo_buck_gs[i + 2] & pmic_ldo_buck_gs[i + 1];
        if (val1 != val2)
        {
            printk("%s - LDO BUCK - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, pmic_ldo_buck_gs[i], gs6323_pmic_read(pmic_ldo_buck_gs[i]), pmic_ldo_buck_gs[i + 1], pmic_ldo_buck_gs[i + 2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }

    // CHG BUCK
    for (i = 0; i < pmic_chg_buck_gs_len; i += 3)
    {
        aee_sram_printk("%d\n", i);
        val1 = gs6333_pmic_read(pmic_chg_buck_gs[i]) & pmic_chg_buck_gs[i + 1];
        val2 = pmic_chg_buck_gs[i + 2] & pmic_chg_buck_gs[i + 1];
        if (val1 != val2)
        {
            printk("%s - CHG BUCK - 0x%x - 0x%x - 0x%x - 0x%x - ", \
                    scenario, pmic_chg_buck_gs[i], gs6333_pmic_read(pmic_chg_buck_gs[i]), pmic_chg_buck_gs[i + 1], pmic_chg_buck_gs[i + 2]);
            mt_power_gs_diff_output(val1, val2);
        }
    }

    mt_power_gs_compare_pll();
}
EXPORT_SYMBOL(mt_power_gs_compare);

static void __exit mt_power_gs_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_init(void)
{
    mt_power_gs_dir = proc_mkdir("mt_power_gs", NULL);
    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }

    return 0;
}

module_init(mt_power_gs_init);
module_exit(mt_power_gs_exit);

MODULE_DESCRIPTION("MT Low Power Golden Setting");