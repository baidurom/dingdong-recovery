#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mt6575_irq.h>

#define GIC_ICDISER0    (GIC_DIST_BASE + 0x100)
#define GIC_ICDISER1    (GIC_DIST_BASE + 0x104)
#define GIC_ICDISER2    (GIC_DIST_BASE + 0x108)
#define GIC_ICDISER3    (GIC_DIST_BASE + 0x10C)
#define GIC_ICDISER4    (GIC_DIST_BASE + 0x110)

#define GIC_ICDICER0    (GIC_DIST_BASE + 0x180)
#define GIC_ICDICER1    (GIC_DIST_BASE + 0x184)
#define GIC_ICDICER2    (GIC_DIST_BASE + 0x188)
#define GIC_ICDICER3    (GIC_DIST_BASE + 0x18C)
#define GIC_ICDICER4    (GIC_DIST_BASE + 0x190)

/*
 * mt6575_irq_mask: mask one IRQ
 * @irq: IRQ line of the IRQ to mask
 */
void mt6575_irq_mask(unsigned int irq)
{
    unsigned int mask = 1 << (irq % 32);

    DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_ENABLE_CLEAR + irq / 32 * 4, mask);
}

/*
 * mt6575_irq_unmask: unmask one IRQ
 * @irq: IRQ line of the IRQ to unmask
 */
void mt6575_irq_unmask(unsigned int irq)
{
	unsigned int mask = 1 << (irq % 32);
    
  DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_ENABLE_SET + irq / 32 * 4, mask);
}

/*
 * mt6575_irq_mask_all: mask all IRQ lines. (This is ONLY used for the idle current measurement by the factory mode.)
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 */
int mt6575_irq_mask_all(struct mtk_irq_mask *mask)
{
    unsigned long flags;

    if (mask) {

        mask->mask0 = DRV_Reg32(GIC_ICDISER0);
        mask->mask1 = DRV_Reg32(GIC_ICDISER1);
        mask->mask2 = DRV_Reg32(GIC_ICDISER2);
        mask->mask3 = DRV_Reg32(GIC_ICDISER3);
        mask->mask4 = DRV_Reg32(GIC_ICDISER4);

        DRV_WriteReg32(GIC_ICDICER0, 0xFFFFFFFF);
        DRV_WriteReg32(GIC_ICDICER1, 0xFFFFFFFF);
        DRV_WriteReg32(GIC_ICDICER2, 0xFFFFFFFF);
        DRV_WriteReg32(GIC_ICDICER3, 0xFFFFFFFF);
        DRV_WriteReg32(GIC_ICDICER4, 0xFFFFFFFF);
        dsb();

        mask->header = IRQ_MASK_HEADER;
        mask->footer = IRQ_MASK_FOOTER;

        return 0;
    } else {
        return -1;
    }
}

int mt6575_irq_is_active(const unsigned int irq)
{
    const unsigned int iActive = DRV_Reg32(GIC_DIST_BASE + 0x200 + irq / 32 * 4);

    return iActive & (1 << (irq % 32)) ? 1 : 0;
}

/*
 * mt6575_irq_mask_restore: restore all IRQ lines' masks. (This is ONLY used for the idle current measurement by the factory mode.)
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 */
int mt6575_irq_mask_restore(struct mtk_irq_mask *mask)
{
    unsigned long flags;

    if (!mask) {
        return -1;
    }
    if (mask->header != IRQ_MASK_HEADER) {
        return -1;
    }
    if (mask->footer != IRQ_MASK_FOOTER) {
        return -1;
    }

    DRV_WriteReg32(GIC_ICDISER0,mask->mask0);
    DRV_WriteReg32(GIC_ICDISER1,mask->mask1);
    DRV_WriteReg32(GIC_ICDISER2,mask->mask2);
    DRV_WriteReg32(GIC_ICDISER3,mask->mask3);
    DRV_WriteReg32(GIC_ICDISER4,mask->mask4);
    dsb();
    

    return 0;
}

static void mt6575_gic_cpu_init(void)
{
    DRV_WriteReg32(GIC_CPU_BASE + GIC_CPU_PRIMASK, 0xF0);
    DRV_WriteReg32(GIC_CPU_BASE + GIC_CPU_CTRL, 1);
}

void mt6575_irq_set_sens(unsigned int irq, unsigned int sens)
{
    unsigned long flags;
    unsigned int config;

    if (sens == MT65xx_EDGE_SENSITIVE) {
        config = DRV_Reg32(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4);
        config |= (0x2 << (irq % 16) * 2);
        DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4, config);
    }else {
        config = DRV_Reg32(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4);
        config &= ~(0x2 << (irq % 16) * 2);
        DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_CONFIG + (irq / 16) * 4, config);
    }
}

void mt6575_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
    unsigned int offset;
    unsigned int reg_index;
    unsigned int value;

    // peripheral device's IRQ line is using GIC's SPI, and line ID >= GIC_PRIVATE_SIGNALS
    if (irq < GIC_PRIVATE_SIGNALS) {
        printf("The Interrupt ID < 32, please check!");
        return;
    }

    offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
    reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;
    if (polarity == 0) {
        value = DRV_Reg32(INT_POL_CTL0 + (reg_index * 4));
        value |= (1 << offset); // always invert the incoming IRQ's polarity
        DRV_WriteReg32((INT_POL_CTL0 + (reg_index * 4)), value);
    }else {
        value = DRV_Reg32(INT_POL_CTL0 + (reg_index * 4));
        value &= ~(0x1 << offset);
        DRV_WriteReg32(INT_POL_CTL0 + (reg_index * 4), value);
    }
}

static void mt6575_gic_dist_init(void)
{
    unsigned int i;
    unsigned int cpumask = 1 << 0;

    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;

	  DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_CTRL, 0);

#if 0   // Discard using GIC register to count IRQ#
    /*
     * Find out how many interrupts are supported.
     */
    max_irq = DRV_Reg32(GIC_DIST_BASE + GIC_DIST_CTR) & 0x1F;
    max_irq = (max_irq + 1) * 32;

    /*
     * The GIC only supports up to 1020 interrupt sources.
     * Limit this to either the architected maximum, or the
     * platform maximum.
     */
    if (max_irq > max(1020, NR_IRQS)) {
        max_irq = max(1020, NR_IRQS);
    }
#endif

    /*
     * Set all global interrupts to be level triggered, active low.
     */
    for (i = 32; i < (MT6575_NR_SPI + 32); i += 16) {
        DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_CONFIG + i * 4 / 16, 0);
    }

    /*
     * Set all global interrupts to this CPU only.
     */
    for (i = 32; i < (MT6575_NR_SPI + 32); i += 4) {
        DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_TARGET + i * 4 / 4, cpumask);
    }

    /*
     * Set priority on all interrupts.
     */
    for (i = 0; i < NR_MT6575_IRQ_LINE; i += 4) {
        DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_PRI + i * 4 / 4, 0xA0A0A0A0);
    }

	  /*
	  * Disable all interrupts.
	  */
	  for (i = 0; i < NR_MT6575_IRQ_LINE; i += 32) {
        DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_ENABLE_CLEAR + i * 4 / 32, 0xFFFFFFFF); 
    }

    DRV_WriteReg32(GIC_DIST_BASE + GIC_DIST_CTRL, 1);
}

void mt6575_irq_ack(unsigned int irq)
{
    DRV_WriteReg32(GIC_CPU_BASE + GIC_CPU_EOI, irq);
}

void mt6575_init_irq(void)
{
    mt6575_gic_dist_init();
    mt6575_gic_cpu_init();
}