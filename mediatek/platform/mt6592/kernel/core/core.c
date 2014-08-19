#include <linux/pm.h>
#include <linux/bug.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/smp_scu.h>
#include <asm/page.h>
#include <mach/mt_reg_base.h>
#include <mach/irqs.h>

extern void arm_machine_restart(char mode, const char *cmd);
extern struct sys_timer mt_timer;
extern void mt_fixup(struct tag *tags, char **cmdline, struct meminfo *mi);
extern void mt_reserve(void);
void __init mt_init(void)
{


}

static struct map_desc mt_io_desc[] __initdata = 
{
    {
        /*#define INFRA_BASE (0xF0000000)*/
        .virtual = INFRA_BASE, 
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(INFRA_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /*#define AP_DMA_BASE (0xF1000000) to 0x11000000*/
        .virtual = AP_DMA_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(AP_DMA_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
        /*#define G3D_CONFIG_BASE (0xF3000000) to 0x13000000*/
    {
        .virtual = G3D_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(G3D_CONFIG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /*#define DISPSYS_BASE (0xF4000000) to 0x14000000*/
        .virtual = DISPSYS_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(DISPSYS_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /*#define IMGSYS_CONFG_BASE (0xF5000000) to 0x15000000*/
        .virtual = IMGSYS_CONFG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(IMGSYS_CONFG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /*#define VDEC_GCON_BASE (0xF6000000) to 0x16000000*/
        .virtual = VDEC_GCON_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(VDEC_GCON_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /*#define MJCSYS_CONFIG_BASE (0xF7000000) to 0x17000000*/
        .virtual = MJCSYS_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MJCSYS_CONFIG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /*#define CONN_BTSYS_PKV_BASE (0xF8000000) to 0x18000000*/
        .virtual = CONN_BTSYS_PKV_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(CONN_BTSYS_PKV_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /* virtual 0xF9000000, physical 0x00100000 */
        .virtual = INTER_SRAM,
        .pfn = __phys_to_pfn(0x00100000),
        .length = SZ_64K+SZ_64K+SZ_64K,
        .type = MT_MEMORY_NONCACHED
    },
    {
        /* virtual 0xF2000000, physical 0x00200000 */
        .virtual = SYSRAM_BASE,
        .pfn = __phys_to_pfn(0x00200000),
        .length = SZ_128K,
        .type = MT_MEMORY_NONCACHED
    },
    {
        /* virtual 0xFA000000, physical 0x08000000 */
        .virtual = DEVINFO_BASE,
        .pfn = __phys_to_pfn(0x08000000),
        .length = SZ_64K,
        .type = MT_DEVICE
    },

    
};

void __init mt_map_io(void)
{
    iotable_init(mt_io_desc, ARRAY_SIZE(mt_io_desc));
}

#ifdef MTK_TABLET_PLATFORM
MACHINE_START(MT6592, MTK_TABLET_PLATFORM)
#else
MACHINE_START(MT6592, "MT6592")
#endif
    .atag_offset    = 0x00000100,
    .map_io         = mt_map_io,
    .init_irq       = mt_init_irq,
    .timer          = &mt_timer,
    .init_machine   = mt_init,
    .fixup          = mt_fixup,
    .restart        = arm_machine_restart,
    .reserve        = mt_reserve,
MACHINE_END
