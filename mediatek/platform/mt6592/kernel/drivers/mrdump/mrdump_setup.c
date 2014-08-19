#include <asm/page.h>
#include <asm/io.h>
#include <mach/mt_reg_base.h>
#include <mach/wd_api.h>
#include <linux/memblock.h>
#include <linux/mrdump.h>

#define MRDUMP_CB_ADDR 0x81F00000
#define MRDUMP_CB_SIZE 0x1000

/* fill the address of ramdisk */
#define RDISK_ADDR             (INFRA_BASE + 0x202060)
/* fill 0x504D454D, magic number */
#define RDISK_EN_MAGIC_REG     (INFRA_BASE + 0x202064)


#define LK_LOAD_ADDR 0x81E00000
#define LK_LOAD_SIZE 0x100000

#define PL_RDISK_ADDR  0xF9010D00
#define PL_RDISK_START 0x00110D00

#include "mrdump_pl.c"

static void mrdump_hw_enable(bool enabled)
{
	struct wd_api *wd = NULL;

	if (get_wd_api(&wd) == -1) {
		printk(KERN_ERR "%s: Can't get wd api\n", __FUNCTION__);
		return;
	}
	
	if (enabled) {
		memcpy((void *) PL_RDISK_ADDR, mrdump_pl_GFH_bin, mrdump_pl_GFH_bin_len);
		writel(PL_RDISK_START, RDISK_ADDR);
		writel(0x504D454D, RDISK_EN_MAGIC_REG);
	}
	wd->wd_dram_reserved_mode(enabled);
}

void mrdump_reserve_memory(void)
{
	struct mrdump_control_block *cblock = NULL;

	/* We must reserved the lk block, can we pass it from lk? */    
	memblock_reserve(LK_LOAD_ADDR, LK_LOAD_SIZE);

	memblock_reserve(MRDUMP_CB_ADDR, MRDUMP_CB_SIZE);
	cblock = (struct mrdump_control_block *)__va(MRDUMP_CB_ADDR);

	mrdump_platform_init(cblock, mrdump_hw_enable);
}
