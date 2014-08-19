#include <linux/linkage.h>
#include <linux/init.h>

    __CPUINIT

CCI400_SI3_SNOOP_CONTROL:
.long 0x10394000
CCI400_STATUS:
.long 0x1039000C
MP1_AXI_CONFIG:
.long 0x10200118

ENTRY(mt_secondary_startup)
    mrc     p15, 0, r0, c0, c0, 5           @ MPIDR
    ubfx    r0, r0, #8, #4                  @ Cluster Id
    cmp     r0, #0                          @ Cluster 1
    beq     cluster0
    ldr     r2, MP1_AXI_CONFIG
    ldr     r1, [r2]
    bic     r1, r1, #0x10
    str     r1, [r2]
    ldr     r2, CCI400_SI3_SNOOP_CONTROL
    ldr     r1, [r2]
    orr     r1, r1, #0x3
    str     r1, [r2]
    ldr     r2, CCI400_STATUS
    b       1f
0:
    dsb
1:
    ldr     r1, [r2]
    tst     r1, #1
    bne     0b
    
cluster0:
    b secondary_startup
END(mt_secondary_startup)

