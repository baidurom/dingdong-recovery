#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>  
#include <linux/aee.h>

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_idle.h>
#include <mach/mt_dormant.h>
#include <mach/mt_gpt.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_spm_sleep.h>
#include <mach/env.h> 
#include <mach/hotplug.h>

#include <asm/hardware/gic.h>

//#include <mach/wd_api.h>

#ifdef SPM_SODI_ENABLED

#define SPM_SODI_DEBUG 0
#define SPM_MCDI_BYPASS_SYSPWREQ 1


//DEFINE_SPINLOCK(spm_sodi_lock);

s32 gSpm_Sodi_Disable_Counter = 0;
bool gSpm_IsLcmVideoMode = TRUE;
u32 gSPM_SODI_EN = 0;  // flag for idle task


#define PCM_WDT_TIMEOUT         (30 * 32768)    /* 30s */
#define PCM_TIMER_MAX_FOR_WDT   (0xffffffff - PCM_WDT_TIMEOUT)


#define mcdi_wfi_with_sync()                         \
do {                                            \
    isb();                                      \
    dsb();                                      \
    __asm__ __volatile__("wfi" : : : "memory"); \
} while (0)



#define WAKE_SRC_FOR_SODI \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CCIF_MD |      \
     WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_AFE |                 \
     WAKE_SRC_SYSPWREQ | WAKE_SRC_MD_WDT | WAKE_SRC_CONN_WDT | WAKE_SRC_CONN | WAKE_SRC_THERM| WAKE_SRC_MP0_CPU0_IRQ)


   
#define WAKE_SRC_FOR_MCDI                     \
    (WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | \
     WAKE_SRC_MP0_CPU0_IRQ | WAKE_SRC_MP0_CPU1_IRQ | WAKE_SRC_MP0_CPU2_IRQ | WAKE_SRC_MP0_CPU3_IRQ | \
     WAKE_SRC_MP1_CPU0_IRQ | WAKE_SRC_MP1_CPU1_IRQ | WAKE_SRC_MP1_CPU2_IRQ | WAKE_SRC_MP1_CPU3_IRQ)


// ==========================================
// PCM code for SODI (Screen On Deep Idle)  pcm_sodi_v0.5_20131002
//
// core 0 : GPT 4
// ==========================================
static u32 __pcm_sodidle[] = {
    0x88000000, 0xfffffffb, 0x89c00007, 0xfffffffd, 0x89c00007, 0xfffbfeff,
    0x1950001f, 0x10006360, 0x02401409, 0xa9c00007, 0x00010400, 0x1b00001f,
    0xbfffe7ff, 0xf0000000, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x17c07c1f,
    0x8880000c, 0x3fffe7ff, 0xd8000702, 0x17c07c1f, 0x8980000d, 0x00000010,
    0xd8200466, 0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x1b00001f, 0x3fffe7ff,
    0x17c07c1f, 0x8880000c, 0x3fffe7ff, 0xd8000702, 0x17c07c1f, 0x1b00001f,
    0xffffffff, 0x17c07c1f, 0x8880000c, 0x40000000, 0xd8000782, 0x17c07c1f,
    0x89c00007, 0xfffeffff, 0xa1d40407, 0x1b80001f, 0x20000008, 0xa1d90407,
    0xc0c00ba0, 0x17c07c1f, 0xd8000703, 0x17c07c1f, 0xa8000000, 0x00000004,
    0xd0000780, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0xd00007c0, 0x17c07c1f,
    0x1b00001f, 0x7fffe7ff, 0xf0000000, 0x17c07c1f, 0xd800090a, 0x17c07c1f,
    0xe2e00036, 0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f,
    0xd8200a0a, 0x17c07c1f, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f,
    0x20000020, 0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xd8000aca, 0x17c07c1f,
    0xe2e0006d, 0xe2e0002d, 0xd8200b6a, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e,
    0xe2e00032, 0xf0000000, 0x17c07c1f, 0x89c00007, 0xfffffbff, 0xa9c00007,
    0x00000200, 0x1950001f, 0x100041dc, 0xba008005, 0xff00ffff, 0x000a0000,
    0xe1000008, 0xa1d08407, 0x1b80001f, 0x2000000c, 0x80eab401, 0x1a00001f,
    0x10006814, 0xe2000003, 0xba008005, 0xff00ffff, 0x00640000, 0xe1000008,
    0xf0000000, 0x17c07c1f, 0x1a00001f, 0x10006604, 0xd8000f43, 0x17c07c1f,
    0xd8200f43, 0x17c07c1f, 0xf0000000, 0x17c07c1f, 0x1a10001f, 0x10002058,
    0x1a80001f, 0x10002058, 0xaa000008, 0x80000000, 0xe2800008, 0xf0000000,
    0x17c07c1f, 0xa1d40407, 0x1b80001f, 0x20000008, 0xa1d90407, 0xf0000000,
    0x17c07c1f, 0x1a90001f, 0x10006b00, 0xd82012aa, 0x17c07c1f, 0xe2e0000d,
    0xe2e0000c, 0xe2e0001c, 0xe2e0001e, 0xe2e00016, 0xe2e00012, 0xf0000000,
    0x17c07c1f, 0x1a90001f, 0x10006b00, 0xd820148a, 0x17c07c1f, 0x1212841f,
    0xe2e00016, 0x1380201f, 0xe2e0001e, 0x1380201f, 0xe2e0001c, 0x1380201f,
    0xe2e0000c, 0xe2e0000d, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001, 0x1a50001f, 0x10006610,
    0x82472401, 0x1a00001f, 0x10006b00, 0xe2000009, 0x12407c1f, 0x1b00001f,
    0x3fffe7ff, 0x1b80001f, 0xd00f0000, 0x8880000c, 0x3fffe7ff, 0xd80055c2,
    0x17c07c1f, 0x1950001f, 0x10006400, 0x80d70405, 0xd8004883, 0x17c07c1f,
    0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200, 0xc0c00a40, 0x12807c1f,
    0xe8208000, 0x1000625c, 0x00000001, 0x1b80001f, 0x20000080, 0xc0c00a40,
    0x1280041f, 0x18c0001f, 0x10006204, 0xc0c01160, 0x17c07c1f, 0x18c0001f,
    0x10006208, 0xc0c00a40, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000000,
    0x1b80001f, 0x20000080, 0xc0c00a40, 0x1280041f, 0x18c0001f, 0x10006290,
    0xc0c00a40, 0x1280041f, 0xa9c00007, 0x00000080, 0x1b80001f, 0x20000030,
    0xc0c00f80, 0x17c07c1f, 0xa8000000, 0x00000002, 0xa8000000, 0x00000200,
    0xa8000000, 0x00800000, 0xa8000000, 0x00020000, 0x1890001f, 0x10006400,
    0x80868801, 0xd82049e2, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f,
    0xd0100000, 0xd0004f00, 0x17c07c1f, 0x1b00001f, 0xffffffff, 0x8880000c,
    0x3fffe7ff, 0xd8004f02, 0x17c07c1f, 0x8880000c, 0x40000000, 0xd80049e2,
    0x17c07c1f, 0x89c00007, 0xfffeffff, 0xc0c010a0, 0x17c07c1f, 0xc0c00ba0,
    0x17c07c1f, 0xd8004da3, 0x17c07c1f, 0xa8000000, 0x00000004, 0xe8208000,
    0x10006310, 0x0b160038, 0x1b00001f, 0x7fffe7ff, 0x1b80001f, 0x90100000,
    0xe8208000, 0x10006310, 0x0b160008, 0x88000000, 0xfffffffb, 0x89c00007,
    0xfffffffd, 0x89c00007, 0xfffbfeff, 0x1950001f, 0x10006360, 0x02401409,
    0xa9c00007, 0x00010400, 0x80d70405, 0xd80055c3, 0x17c07c1f, 0x88000000,
    0xfffdffff, 0x1b80001f, 0x20000300, 0x88000000, 0xff7fffff, 0x1b80001f,
    0x20000300, 0x88000000, 0xfffffdff, 0x88000000, 0xfffffffd, 0x1b80001f,
    0x2000079e, 0x89c00007, 0xffffff7f, 0x80d70405, 0xd80055c3, 0x17c07c1f,
    0x18c0001f, 0x10006290, 0x1212841f, 0xc0c00800, 0x1280041f, 0x18c0001f,
    0x10006208, 0x1212841f, 0xc0c00800, 0x12807c1f, 0xe8208000, 0x10006248,
    0x00000001, 0x1b80001f, 0x20000080, 0xc0c00800, 0x1280041f, 0x18c0001f,
    0x10006204, 0xc0c012e0, 0x17c07c1f, 0x18c0001f, 0x10006200, 0xc0c00800,
    0x12807c1f, 0xe8208000, 0x1000625c, 0x00000000, 0x1b80001f, 0x20000080,
    0xc0c00800, 0x1280041f, 0x19c0001f, 0x00015820, 0x10007c1f, 0x80cab001,
    0x808cb401, 0x80800c02, 0xd82056e2, 0x17c07c1f, 0xa1d78407, 0x12c0241f,
    0x1240301f, 0x1a00001f, 0x10006b00, 0xe200001f, 0xe8208000, 0x10006b30,
    0x00000000, 0xe8208000, 0x100063e0, 0x00000001, 0x1b00001f, 0x00200000,
    0x1b80001f, 0x80001000, 0x809c840d, 0xd8205842, 0x17c07c1f, 0xa1d78407,
    0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402, 0xe0c00002,
    0x1b80001f, 0x00001000, 0xf0000000, 0x17c07c1f

};

#define SODI_PCM_PC_0      0
#define SODI_PCM_PC_1      15
    
static const pcm_desc_t pcm_sodidle = {
    .base   = __pcm_sodidle,
    .size   = 724,
    .vec0   = EVENT_VEC(30, 1, 0, SODI_PCM_PC_0),       /* AP-wake event */
    .vec1   = EVENT_VEC(31, 1, 0, SODI_PCM_PC_1),       /* AP-sleep event */
};

/**********************************************************
 * PCM code for MCDI (v0.10 @ 2013-09-30)
 **********************************************************/
static const u32 spm_pcm_mcdi[] = {
    0x1840001f, 0x00000001, 0x11407c1f, 0xe8208000, 0x10006310, 0x0b160008,
    0x1b00001f, 0x3da28091, 0x1b80001f, 0xd0010000, 0xc0803100, 0x17c07c1f,
    0x60a07c05, 0x88900002, 0x10006814, 0x1910001f, 0x10006400, 0x81271002,
    0x80801001, 0xd8002c22, 0x17c07c1f, 0x1a50001f, 0x10006f00, 0x1a10001f,
    0x10006720, 0x82082001, 0x80c02408, 0x81201403, 0xd82004e4, 0x17c07c1f,
    0x89c00007, 0xffffefff, 0x1a40001f, 0x10006200, 0x1a80001f, 0x1000625c,
    0xc2402fc0, 0x17c07c1f, 0xa1400405, 0x1a50001f, 0x10006f04, 0x1a10001f,
    0x10006720, 0x8208a001, 0x80c02408, 0x81209403, 0xd82006e4, 0x17c07c1f,
    0x1a40001f, 0x10006218, 0x1a80001f, 0x10006264, 0xc2402fc0, 0x17c07c1f,
    0xa1508405, 0x1a50001f, 0x10006f08, 0x1a10001f, 0x10006720, 0x82092001,
    0x80c02408, 0x81211403, 0xd82008e4, 0x17c07c1f, 0x1a40001f, 0x1000621c,
    0x1a80001f, 0x1000626c, 0xc2402fc0, 0x17c07c1f, 0xa1510405, 0x1a50001f,
    0x10006f0c, 0x1a10001f, 0x10006720, 0x8209a001, 0x80c02408, 0x81219403,
    0xd8200ae4, 0x17c07c1f, 0x1a40001f, 0x10006220, 0x1a80001f, 0x10006274,
    0xc2402fc0, 0x17c07c1f, 0xa1518405, 0x1a50001f, 0x10006f10, 0x1a10001f,
    0x10006720, 0x820a2001, 0x80c02408, 0x81221403, 0xd8200ce4, 0x17c07c1f,
    0x1a40001f, 0x100062a0, 0x1a80001f, 0x100062c0, 0xc2402fc0, 0x17c07c1f,
    0xa1520405, 0x1a50001f, 0x10006f14, 0x1a10001f, 0x10006720, 0x820aa001,
    0x80c02408, 0x81229403, 0xd8200ee4, 0x17c07c1f, 0x1a40001f, 0x100062a4,
    0x1a80001f, 0x100062c4, 0xc2402fc0, 0x17c07c1f, 0xa1528405, 0x1a50001f,
    0x10006f18, 0x1a10001f, 0x10006720, 0x820b2001, 0x80c02408, 0x81231403,
    0xd82010e4, 0x17c07c1f, 0x1a40001f, 0x100062a8, 0x1a80001f, 0x100062c8,
    0xc2402fc0, 0x17c07c1f, 0xa1530405, 0x1a50001f, 0x10006f1c, 0x1a10001f,
    0x10006720, 0x820ba001, 0x80c02408, 0x81239403, 0xd82012e4, 0x17c07c1f,
    0x1a40001f, 0x100062ac, 0x1a80001f, 0x100062cc, 0xc2402fc0, 0x17c07c1f,
    0xa1538405, 0xd8202bac, 0x17c07c1f, 0x11807c1f, 0x81001401, 0xd82016a4,
    0x17c07c1f, 0x810d3001, 0xb1003081, 0xb10c3081, 0xd82016a4, 0x17c07c1f,
    0x1a40001f, 0x10006200, 0x1a80001f, 0x1000625c, 0xc2402d80, 0x17c07c1f,
    0xa9c00007, 0x00001000, 0x89400005, 0xfffffffe, 0xe8208000, 0x10006f00,
    0x00000000, 0xe8208000, 0x10006b30, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000001, 0x81009401, 0xd82019a4, 0x17c07c1f, 0x810db001, 0xb1003081,
    0xd82019a4, 0x17c07c1f, 0x1a40001f, 0x10006218, 0x1a80001f, 0x10006264,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xfffffffd, 0xe8208000, 0x10006f04,
    0x00000000, 0xe8208000, 0x10006b34, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000002, 0x81011401, 0xd8201ca4, 0x17c07c1f, 0x810e3001, 0xb1003081,
    0xd8201ca4, 0x17c07c1f, 0x1a40001f, 0x1000621c, 0x1a80001f, 0x1000626c,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xfffffffb, 0xe8208000, 0x10006f08,
    0x00000000, 0xe8208000, 0x10006b38, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000004, 0x81019401, 0xd8201fa4, 0x17c07c1f, 0x810eb001, 0xb1003081,
    0xd8201fa4, 0x17c07c1f, 0x1a40001f, 0x10006220, 0x1a80001f, 0x10006274,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xfffffff7, 0xe8208000, 0x10006f0c,
    0x00000000, 0xe8208000, 0x10006b3c, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000008, 0x81021401, 0xd82022a4, 0x17c07c1f, 0x8103b001, 0xb1003081,
    0xd82022a4, 0x17c07c1f, 0x1a40001f, 0x100062a0, 0x1a80001f, 0x100062c0,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xffffffef, 0xe8208000, 0x10006f10,
    0x00000000, 0xe8208000, 0x10006b40, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000010, 0x81029401, 0xd82025a4, 0x17c07c1f, 0x8107b001, 0xb1003081,
    0xd82025a4, 0x17c07c1f, 0x1a40001f, 0x100062a4, 0x1a80001f, 0x100062c4,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xffffffdf, 0xe8208000, 0x10006f14,
    0x00000000, 0xe8208000, 0x10006b44, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000020, 0x81031401, 0xd82028a4, 0x17c07c1f, 0x8108b001, 0xb1003081,
    0xd82028a4, 0x17c07c1f, 0x1a40001f, 0x100062a8, 0x1a80001f, 0x100062c8,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xffffffbf, 0xe8208000, 0x10006f18,
    0x00000000, 0xe8208000, 0x10006b48, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000040, 0x81039401, 0xd8202ba4, 0x17c07c1f, 0x810bb001, 0xb1003081,
    0xd8202ba4, 0x17c07c1f, 0x1a40001f, 0x100062ac, 0x1a80001f, 0x100062cc,
    0xc2402d80, 0x17c07c1f, 0x89400005, 0xffffff7f, 0xe8208000, 0x10006f1c,
    0x00000000, 0xe8208000, 0x10006b4c, 0x00000000, 0xe8208000, 0x100063e0,
    0x00000080, 0xd00000c0, 0x17c07c1f, 0xd00000c0, 0x17c07c1f, 0x19c0001f,
    0x00015820, 0x10007c1f, 0x1ac0001f, 0x55aa55aa, 0x1940001f, 0xaa55aa55,
    0x1b80001f, 0x00001000, 0xf0000000, 0x17c07c1f, 0x1212841f, 0xe2e00036,
    0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xe2a00000,
    0x1b80001f, 0x20000080, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f,
    0x20000020, 0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xe2e0006d, 0xe2e0002d,
    0xe2a00001, 0x1b80001f, 0x20000080, 0xe2e0002f, 0xe2e0003e, 0xe2e00032,
    0xf0000000, 0x17c07c1f, 0x80cab001, 0x808cb401, 0x80800c02, 0xd8203202,
    0x17c07c1f, 0xa1d78407, 0x1b80001f, 0x10001000, 0xf0000000, 0x17c07c1f

};
static const pcm_desc_t pcm_mcdi = {
    .base   = spm_pcm_mcdi,
    .size   = 402,
    /*no event vector for MCDI*/
};

#define SPM_MP_CORE0_AUX SPM_PCM_EVENT_VECTOR2
#define SPM_MP_CORE1_AUX SPM_PCM_EVENT_VECTOR3
#define SPM_MP_CORE2_AUX SPM_PCM_EVENT_VECTOR4
#define SPM_MP_CORE3_AUX SPM_PCM_EVENT_VECTOR5
#define SPM_MP_CORE4_AUX SPM_PCM_EVENT_VECTOR6
#define SPM_MP_CORE5_AUX SPM_PCM_EVENT_VECTOR7
#define SPM_MP_CORE6_AUX SPM_PCM_RESERVE
#define SPM_MP_CORE7_AUX SPM_PCM_WDT_TIMER_VAL

#define SPM_PCM_HOTPLUG          (1U << 16)
#define SPM_PCM_IPI              (1U << 17)


#define spm_error2(fmt, args...)    \
do {                                \
    aee_sram_printk(fmt, ##args);   \
    spm_error(fmt, ##args);         \
} while (0)


typedef struct {
    u32 debug_reg;      /* PCM_REG_DATA_INI */
    u32 r12;            /* PCM_REG12_DATA */
    u32 raw_sta;        /* SLEEP_ISR_RAW_STA */
    u32 cpu_wake;       /* SLEEP_CPU_WAKEUP_EVENT */
    u32 timer_out;      /* PCM_TIMER_OUT */
    u32 event_reg;      /* PCM_EVENT_REG_STA */
    u32 isr;            /* SLEEP_ISR_STATUS */
    u32 r13;            /* PCM_REG13_DATA */
} wake_status_t;


//extern void mtk_wdt_suspend(void);
//extern void mtk_wdt_resume(void);
extern int mt_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt_irq_mask_restore(struct mtk_irq_mask *mask);
//extern int mt_SPI_mask_all(struct mtk_irq_mask *mask);
//extern int mt_SPI_mask_restore(struct mtk_irq_mask *mask);
//extern int mt_PPI_mask_all(struct mtk_irq_mask *mask);
//extern int mt_PPI_mask_restore(struct mtk_irq_mask *mask);
//extern void mt_irq_mask_for_sleep(unsigned int irq);
extern void mt_irq_unmask_for_sleep(unsigned int irq);
extern void mt_cirq_enable(void);
extern void mt_cirq_disable(void);
extern void mt_cirq_clone_gic(void);
extern void mt_cirq_flush(void);
//extern void mt_cirq_mask(unsigned int cirq_num);
//extern void mt_cirq_mask_all(void);

extern void soidle_before_wfi(int cpu);
extern void soidle_after_wfi(int cpu);

void __attribute__((weak)) soidle_before_wfi(int cpu)
{
}

void __attribute__((weak)) soidle_after_wfi(int cpu)
{
}

extern spinlock_t spm_lock;

#if SPM_SODI_DEBUG
static void spm_sodi_dump_regs(void)
{
    /* SPM register */
    clc_notice("SPM_MP0_CPU0_IRQ_MASK   0x%x = 0x%x\n", SPM_MP0_CPU0_IRQ_MASK, spm_read(SPM_MP0_CPU0_IRQ_MASK));
    clc_notice("SPM_MP0_CPU1_IRQ_MASK   0x%x = 0x%x\n", SPM_MP0_CPU1_IRQ_MASK, spm_read(SPM_MP0_CPU1_IRQ_MASK));    
    clc_notice("SPM_MP0_CPU2_IRQ_MASK   0x%x = 0x%x\n", SPM_MP0_CPU2_IRQ_MASK, spm_read(SPM_MP0_CPU2_IRQ_MASK));
    clc_notice("SPM_MP0_CPU3_IRQ_MASK   0x%x = 0x%x\n", SPM_MP0_CPU3_IRQ_MASK, spm_read(SPM_MP0_CPU3_IRQ_MASK));
    clc_notice("SPM_MP1_CPU0_IRQ_MASK   0x%x = 0x%x\n", SPM_MP1_CPU0_IRQ_MASK, spm_read(SPM_MP1_CPU0_IRQ_MASK));
    clc_notice("SPM_MP1_CPU1_IRQ_MASK   0x%x = 0x%x\n", SPM_MP1_CPU1_IRQ_MASK, spm_read(SPM_MP1_CPU1_IRQ_MASK));
    clc_notice("SPM_MP1_CPU2_IRQ_MASK   0x%x = 0x%x\n", SPM_MP1_CPU2_IRQ_MASK, spm_read(SPM_MP1_CPU2_IRQ_MASK));
    clc_notice("SPM_MP1_CPU3_IRQ_MASK   0x%x = 0x%x\n", SPM_MP1_CPU3_IRQ_MASK, spm_read(SPM_MP1_CPU3_IRQ_MASK));    
#if 0    
    clc_notice("POWER_ON_VAL0   0x%x = 0x%x\n", SPM_POWER_ON_VAL0          , spm_read(SPM_POWER_ON_VAL0));
    clc_notice("POWER_ON_VAL1   0x%x = 0x%x\n", SPM_POWER_ON_VAL1          , spm_read(SPM_POWER_ON_VAL1));
    clc_notice("PCM_PWR_IO_EN   0x%x = 0x%x\n", SPM_PCM_PWR_IO_EN          , spm_read(SPM_PCM_PWR_IO_EN));
    clc_notice("CLK_CON         0x%x = 0x%x\n", SPM_CLK_CON                , spm_read(SPM_CLK_CON));
    clc_notice("AP_DVFS_CON     0x%x = 0x%x\n", SPM_AP_DVFS_CON_SET        , spm_read(SPM_AP_DVFS_CON_SET));
    clc_notice("PWR_STATUS      0x%x = 0x%x\n", SPM_PWR_STATUS             , spm_read(SPM_PWR_STATUS));
    clc_notice("PWR_STATUS_S    0x%x = 0x%x\n", SPM_PWR_STATUS_S           , spm_read(SPM_PWR_STATUS_S));
    clc_notice("SLEEP_TIMER_STA 0x%x = 0x%x\n", SPM_SLEEP_TIMER_STA        , spm_read(SPM_SLEEP_TIMER_STA));
    clc_notice("WAKE_EVENT_MASK 0x%x = 0x%x\n", SPM_SLEEP_WAKEUP_EVENT_MASK, spm_read(SPM_SLEEP_WAKEUP_EVENT_MASK));
    clc_notice("SPM_SLEEP_CPU_WAKEUP_EVENT 0x%x = 0x%x\n", SPM_SLEEP_CPU_WAKEUP_EVENT, spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT));
    clc_notice("SPM_PCM_RESERVE   0x%x = 0x%x\n", SPM_PCM_RESERVE          , spm_read(SPM_PCM_RESERVE));  
    clc_notice("SPM_AP_STANBY_CON   0x%x = 0x%x\n", SPM_AP_STANBY_CON          , spm_read(SPM_AP_STANBY_CON));  
    clc_notice("SPM_PCM_TIMER_OUT   0x%x = 0x%x\n", SPM_PCM_TIMER_OUT          , spm_read(SPM_PCM_TIMER_OUT));
    clc_notice("SPM_PCM_CON1   0x%x = 0x%x\n", SPM_PCM_CON1          , spm_read(SPM_PCM_CON1));
#endif    
    
    // PCM register
    clc_notice("PCM_REG0_DATA   0x%x = 0x%x\n", SPM_PCM_REG0_DATA          , spm_read(SPM_PCM_REG0_DATA));
    clc_notice("PCM_REG1_DATA   0x%x = 0x%x\n", SPM_PCM_REG1_DATA          , spm_read(SPM_PCM_REG1_DATA));
    clc_notice("PCM_REG2_DATA   0x%x = 0x%x\n", SPM_PCM_REG2_DATA          , spm_read(SPM_PCM_REG2_DATA));
    clc_notice("PCM_REG3_DATA   0x%x = 0x%x\n", SPM_PCM_REG3_DATA          , spm_read(SPM_PCM_REG3_DATA));
    clc_notice("PCM_REG4_DATA   0x%x = 0x%x\n", SPM_PCM_REG4_DATA          , spm_read(SPM_PCM_REG4_DATA));
    clc_notice("PCM_REG5_DATA   0x%x = 0x%x\n", SPM_PCM_REG5_DATA          , spm_read(SPM_PCM_REG5_DATA));
    clc_notice("PCM_REG6_DATA   0x%x = 0x%x\n", SPM_PCM_REG6_DATA          , spm_read(SPM_PCM_REG6_DATA));
    clc_notice("PCM_REG7_DATA   0x%x = 0x%x\n", SPM_PCM_REG7_DATA          , spm_read(SPM_PCM_REG7_DATA));
    clc_notice("PCM_REG8_DATA   0x%x = 0x%x\n", SPM_PCM_REG8_DATA          , spm_read(SPM_PCM_REG8_DATA));
    clc_notice("PCM_REG9_DATA   0x%x = 0x%x\n", SPM_PCM_REG9_DATA          , spm_read(SPM_PCM_REG9_DATA));
    clc_notice("PCM_REG10_DATA   0x%x = 0x%x\n", SPM_PCM_REG10_DATA          , spm_read(SPM_PCM_REG10_DATA));
    clc_notice("PCM_REG11_DATA   0x%x = 0x%x\n", SPM_PCM_REG11_DATA          , spm_read(SPM_PCM_REG11_DATA));
    clc_notice("PCM_REG12_DATA   0x%x = 0x%x\n", SPM_PCM_REG12_DATA          , spm_read(SPM_PCM_REG12_DATA));
    clc_notice("PCM_REG13_DATA   0x%x = 0x%x\n", SPM_PCM_REG13_DATA          , spm_read(SPM_PCM_REG13_DATA));
    clc_notice("PCM_REG14_DATA   0x%x = 0x%x\n", SPM_PCM_REG14_DATA          , spm_read(SPM_PCM_REG14_DATA));
    clc_notice("PCM_REG15_DATA   0x%x = 0x%x\n", SPM_PCM_REG15_DATA          , spm_read(SPM_PCM_REG15_DATA));  

    clc_notice("SPM_MP0_FC0_PWR_CON   0x%x = 0x%x\n", SPM_MP0_FC0_PWR_CON, spm_read(SPM_MP0_FC0_PWR_CON));    
    clc_notice("SPM_MP0_FC1_PWR_CON   0x%x = 0x%x\n", SPM_MP0_FC1_PWR_CON, spm_read(SPM_MP0_FC1_PWR_CON));    
    clc_notice("SPM_MP0_FC2_PWR_CON   0x%x = 0x%x\n", SPM_MP0_FC2_PWR_CON, spm_read(SPM_MP0_FC2_PWR_CON));    
    clc_notice("SPM_MP0_FC3_PWR_CON   0x%x = 0x%x\n", SPM_MP0_FC3_PWR_CON, spm_read(SPM_MP0_FC3_PWR_CON));    
    clc_notice("SPM_MP1_FC0_PWR_CON   0x%x = 0x%x\n", SPM_MP1_FC0_PWR_CON, spm_read(SPM_MP1_FC0_PWR_CON));    
    clc_notice("SPM_MP1_FC1_PWR_CON   0x%x = 0x%x\n", SPM_MP1_FC1_PWR_CON, spm_read(SPM_MP1_FC1_PWR_CON));    
    clc_notice("SPM_MP1_FC2_PWR_CON   0x%x = 0x%x\n", SPM_MP1_FC2_PWR_CON, spm_read(SPM_MP1_FC2_PWR_CON));    
    clc_notice("SPM_MP1_FC3_PWR_CON   0x%x = 0x%x\n", SPM_MP1_FC3_PWR_CON, spm_read(SPM_MP1_FC3_PWR_CON));    

    clc_notice("CLK_CON         0x%x = 0x%x\n", SPM_CLK_CON                , spm_read(SPM_CLK_CON));
    clc_notice("SPM_PCM_CON0   0x%x = 0x%x\n", SPM_PCM_CON0          , spm_read(SPM_PCM_CON0));
    clc_notice("SPM_PCM_CON1   0x%x = 0x%x\n", SPM_PCM_CON1          , spm_read(SPM_PCM_CON1));
    
    clc_notice("SPM_PCM_MP_CORE0_AUX   0x%x = 0x%x\n", SPM_PCM_EVENT_VECTOR2  , spm_read(SPM_PCM_EVENT_VECTOR2));
    clc_notice("SPM_PCM_MP_CORE1_AUX   0x%x = 0x%x\n", SPM_PCM_EVENT_VECTOR3  , spm_read(SPM_PCM_EVENT_VECTOR3));
    clc_notice("SPM_PCM_MP_CORE2_AUX   0x%x = 0x%x\n", SPM_PCM_EVENT_VECTOR4  , spm_read(SPM_PCM_EVENT_VECTOR4));
    clc_notice("SPM_PCM_MP_CORE3_AUX   0x%x = 0x%x\n", SPM_PCM_EVENT_VECTOR5  , spm_read(SPM_PCM_EVENT_VECTOR5));
    clc_notice("SPM_PCM_MP_CORE4_AUX   0x%x = 0x%x\n", SPM_PCM_EVENT_VECTOR6  , spm_read(SPM_PCM_EVENT_VECTOR6));
    clc_notice("SPM_PCM_MP_CORE5_AUX   0x%x = 0x%x\n", SPM_PCM_EVENT_VECTOR7  , spm_read(SPM_PCM_EVENT_VECTOR7));
    clc_notice("SPM_PCM_MP_CORE6_AUX   0x%x = 0x%x\n", SPM_PCM_RESERVE  , spm_read(SPM_PCM_RESERVE));
    clc_notice("SPM_PCM_MP_CORE7_AUX   0x%x = 0x%x\n", SPM_PCM_WDT_TIMER_VAL  , spm_read(SPM_PCM_WDT_TIMER_VAL));

    clc_notice("SPM_MP0_CORE0_WFI_SEL   0x%x = 0x%x\n", SPM_MP0_CORE0_WFI_SEL  , spm_read(SPM_MP0_CORE0_WFI_SEL));
    clc_notice("SPM_MP0_CORE1_WFI_SEL   0x%x = 0x%x\n", SPM_MP0_CORE1_WFI_SEL  , spm_read(SPM_MP0_CORE1_WFI_SEL));
    clc_notice("SPM_MP0_CORE2_WFI_SEL   0x%x = 0x%x\n", SPM_MP0_CORE2_WFI_SEL  , spm_read(SPM_MP0_CORE2_WFI_SEL));
    clc_notice("SPM_MP0_CORE3_WFI_SEL   0x%x = 0x%x\n", SPM_MP0_CORE3_WFI_SEL  , spm_read(SPM_MP0_CORE3_WFI_SEL));
    clc_notice("SPM_MP1_CORE0_WFI_SEL   0x%x = 0x%x\n", SPM_MP1_CORE0_WFI_SEL  , spm_read(SPM_MP1_CORE0_WFI_SEL));
    clc_notice("SPM_MP1_CORE1_WFI_SEL   0x%x = 0x%x\n", SPM_MP1_CORE1_WFI_SEL  , spm_read(SPM_MP1_CORE1_WFI_SEL));
    clc_notice("SPM_MP1_CORE2_WFI_SEL   0x%x = 0x%x\n", SPM_MP1_CORE2_WFI_SEL  , spm_read(SPM_MP1_CORE2_WFI_SEL));
    clc_notice("SPM_MP1_CORE3_WFI_SEL   0x%x = 0x%x\n", SPM_MP1_CORE3_WFI_SEL  , spm_read(SPM_MP1_CORE3_WFI_SEL));

    clc_notice("SPM_SLEEP_TIMER_STA   0x%x = 0x%x\n", SPM_SLEEP_TIMER_STA  , spm_read(SPM_SLEEP_TIMER_STA));
    clc_notice("SPM_PWR_STATUS   0x%x = 0x%x\n", SPM_PWR_STATUS  , spm_read(SPM_PWR_STATUS));
    clc_notice("SPM_PWR_STATUS_S   0x%x = 0x%x\n", SPM_PWR_STATUS_S  , spm_read(SPM_PWR_STATUS_S));

    clc_notice("SPM_MP0_FC0_PWR_CON   0x%x = 0x%x\n", SPM_MP0_FC0_PWR_CON  , spm_read(SPM_MP0_FC0_PWR_CON));
    clc_notice("SPM_MP0_DBG_PWR_CON   0x%x = 0x%x\n", SPM_MP0_DBG_PWR_CON  , spm_read(SPM_MP0_DBG_PWR_CON));    
    clc_notice("SPM_MP0_CPU_PWR_CON   0x%x = 0x%x\n", SPM_MP0_CPU_PWR_CON  , spm_read(SPM_MP0_CPU_PWR_CON));    
 
}
#endif

static void spm_direct_disable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp |= (0x1<<13);
    
    spm_write(SPM_CLK_CON, clc_temp);  
}


static void spm_direct_enable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp &= 0xffffdfff; // ~(0x1<<13);

    spm_write(SPM_CLK_CON, clc_temp);  
}


static void spm_reset_and_init_pcm(bool IsMcdi)
{
    u32 con1;

    /* reset PCM */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY);

    /* init PCM_CON0 (disable event vector) */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);

    /* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
    con1 = spm_read(SPM_PCM_CON1) & (CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
    if(IsMcdi==TRUE)
        spm_write(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_SPM_SRAM_ISO_B |
                                CON1_SPM_SRAM_SLP_B | CON1_MIF_APBEN);        
    else
        spm_write(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_SPM_SRAM_ISO_B |
                                CON1_SPM_SRAM_SLP_B | CON1_IM_NONRP_EN | CON1_MIF_APBEN);
}


static void spm_kick_im_to_fetch(const pcm_desc_t *pcmdesc)
{
    u32 ptr, len, con0;

    /* tell IM where is PCM code (use slave mode if code existed) */
    ptr = spm_get_base_phys(pcmdesc->base);
    len = pcmdesc->size - 1;
    if (spm_read(SPM_PCM_IM_PTR) != ptr || spm_read(SPM_PCM_IM_LEN) != len) {
        spm_write(SPM_PCM_IM_PTR, ptr);
        spm_write(SPM_PCM_IM_LEN, len);
    } else {
        spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_IM_SLAVE);
    }

    /* kick IM to fetch (only toggle IM_KICK) */
    con0 = spm_read(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}


static void spm_init_pcm_register(bool IsMCDI)
{
    if(IsMCDI==FALSE)
    {
        /* init r0 with POWER_ON_VAL0 */
        spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL0));
        spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
        spm_write(SPM_PCM_PWR_IO_EN, 0);


        /* init r7 with POWER_ON_VAL1 */
        spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL1));
        spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
        spm_write(SPM_PCM_PWR_IO_EN, 0);
    }

    /* clear REG_DATA_INI for PCM after init rX */
    spm_write(SPM_PCM_REG_DATA_INI, 0);
}


static void spm_init_event_vector(const pcm_desc_t *pcmdesc)
{
    /* init event vector register */
    spm_write(SPM_PCM_EVENT_VECTOR0, pcmdesc->vec0);
    spm_write(SPM_PCM_EVENT_VECTOR1, pcmdesc->vec1);
#if 0    
    spm_write(SPM_PCM_EVENT_VECTOR2, pcmdesc->vec2);
    spm_write(SPM_PCM_EVENT_VECTOR3, pcmdesc->vec3);
    spm_write(SPM_PCM_EVENT_VECTOR4, pcmdesc->vec4);
    spm_write(SPM_PCM_EVENT_VECTOR5, pcmdesc->vec5);
    spm_write(SPM_PCM_EVENT_VECTOR6, pcmdesc->vec6);
    spm_write(SPM_PCM_EVENT_VECTOR7, pcmdesc->vec7);
#endif
    /* event vector will be enabled by PCM itself */
}


static void spm_set_ap_standbywfi(bool IsMcdi)
{

    if (IsMcdi == TRUE)
    {
        spm_write(SPM_AP_STANBY_CON, (0 << 19) |  /* mask MD*/
                                     (0 << 20) |  /* mask CONN */
                                     (0 << 16) |  /* mask DISP */
                                     (0 << 17) |  /* mask MFG */
                                     (0 << 6) |   /* mask SCU idle */
                                     (0 << 5) |   /* mask L2C idle */
                                     (0U << 4));  /* Reduce OR */   
        
    }
    else
    {
        spm_write(SPM_AP_STANBY_CON, (1 << 19) |  /* unmask MD*/
                                     (1 << 20) |  /* unmask CONN */
                                     (1 << 16) |    /* unmask DISP */
                                     (0 << 17) |    /* mask MFG */
                                     (0 << 6) |     /* check SCU idle */
                                     (0 << 5) |     /* check L2C idle */
                                     (1U << 4));    /* Reduce AND */
    }
 

}


/*
 * timer_val: PCM timer value (0 = disable)
 * wake_src : WAKE_SRC_XXX
 */
static void spm_set_wakeup_event(u32 wake_src,bool IsMcdi)
{
    u32 isr;
    
    /* unmask wakeup source */
#if SPM_MCDI_BYPASS_SYSPWREQ
    wake_src &= ~WAKE_SRC_SYSPWREQ;     /* make 26M off when attach ICE */
#endif
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~wake_src);

    /* unmask SPM ISR (keep TWAM setting) */
    isr = spm_read(SPM_SLEEP_ISR_MASK) & ISR_TWAM;
    spm_write(SPM_SLEEP_ISR_MASK, isr | ISRM_PCM_IRQ_AUX);
    
    if (IsMcdi == FALSE)
    {
        //mask CPU0 IRQ
        spm_write(SPM_MP0_CPU0_IRQ_MASK,0x1);
    }
}


static void spm_kick_pcm_to_run(bool cpu_pdn, bool infra_pdn, bool IsMcdi)
{
    u32 clk, con0;

    if(IsMcdi==FALSE)
    {

        #if 0
        /*enable SODI*/
        //spm_direct_enable_sodi();

        if (TRUE == gSpm_IsLcmVideoMode)
        {
            /*Disable MEM self re-fresh for video mode LCM*/
            spm_direct_disable_sodi();
        }
        /*else, controlled by LCM driver*/
        #endif        
        
        /* keep CPU or INFRA/DDRPHY power if needed and lock INFRA DCM */
        clk = spm_read(SPM_CLK_CON) & ~(CC_DISABLE_DORM_PWR | CC_DISABLE_INFRA_PWR);
        if (!cpu_pdn)
            clk |= CC_DISABLE_DORM_PWR;
        if (!infra_pdn)
            clk |= CC_DISABLE_INFRA_PWR;
        spm_write(SPM_CLK_CON, clk | CC_LOCK_INFRA_DCM);

        /* init pause request mask for PCM */
        spm_write(SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);

        /* enable r0 and r7 to control power */
        spm_write(SPM_PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

        /* SRCLKENA: r7 (PWR_IO_EN[7]=1) */
        spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) | CC_SRCLKENA_MASK);
    }

    /* kick PCM to run (only toggle PCM_KICK) */
    con0 = spm_read(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}


static void spm_trigger_wfi_for_mcdi(bool cpu_pdn)
{
    if (cpu_pdn) {
        if (!cpu_power_down(DORMANT_MODE,0)) {
            switch_to_amp();
            mcdi_wfi_with_sync();
        }
        switch_to_smp();
        cpu_check_dormant_abort();
    } else {
        mcdi_wfi_with_sync();
    }
}


static void spm_get_wakeup_status(wake_status_t *wakesta)
{
    /* get PC value if PCM assert (pause abort) */
    wakesta->debug_reg = spm_read(SPM_PCM_REG_DATA_INI);

    /* get wakeup event */
    wakesta->r12 = spm_read(SPM_PCM_REG9_DATA);     /* r9 = r12 for pcm_normal */
    wakesta->raw_sta = spm_read(SPM_SLEEP_ISR_RAW_STA);
    wakesta->cpu_wake = spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT);

    /* get sleep time */
    wakesta->timer_out = spm_read(SPM_PCM_TIMER_OUT);

    /* get special pattern (0xf0000 or 0x10000) if sleep abort */
    wakesta->event_reg = spm_read(SPM_PCM_EVENT_REG_STA);

    /* get ISR status */
    wakesta->isr = spm_read(SPM_SLEEP_ISR_STATUS);

    /* get MD/CONN and co-clock status */
    wakesta->r13 = spm_read(SPM_PCM_REG13_DATA);
}


static void spm_clean_after_wakeup(void)
{
    /* PCM has cleared uart_clk_off_req and now clear it in POWER_ON_VAL1 */
    spm_write(SPM_POWER_ON_VAL1, spm_read(SPM_POWER_ON_VAL1) & ~R7_UART_CLK_OFF_REQ);

    /* SRCLKENA: POWER_ON_VAL1|r7 (PWR_IO_EN[7]=1) */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_SRCLKENA_MASK);

    /* re-enable POWER_ON_VAL0/1 to control power */
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* unlock INFRA DCM */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_LOCK_INFRA_DCM);

    /* clean CPU wakeup event (pause abort) */
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0);

    /* clean wakeup event raw status (except THERM) */
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~WAKE_SRC_THERM);

    /* clean ISR status (except TWAM) */
    spm_write(SPM_SLEEP_ISR_MASK, spm_read(SPM_SLEEP_ISR_MASK) | ISRM_ALL_EXC_TWAM);
    spm_write(SPM_SLEEP_ISR_STATUS, ISRC_ALL_EXC_TWAM);
    spm_write(SPM_PCM_SW_INT_CLEAR, PCM_SW_INT0);
}


static wake_reason_t spm_output_wake_reason(const wake_status_t *wakesta)
{
    if (wakesta->debug_reg != 0) {
        spm_error2("PCM ASSERT AND PC = %u (0x%x)(0x%x)\n",
                   wakesta->debug_reg, wakesta->r13, wakesta->event_reg);
        return WR_PCM_ASSERT;
    }

    return WR_WAKE_SRC;
}


void spm_go_to_sodi(bool cpu_pdn)
{
    wake_status_t wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    const pcm_desc_t *pcmdesc = &pcm_sodidle;
    const bool isMCDI = 0;
    
    spin_lock_irqsave(&spm_lock, flags);

    gSPM_SODI_EN =1;

    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM_IRQ_ID);
    
    mt_cirq_clone_gic();
    mt_cirq_enable();

    spm_reset_and_init_pcm(isMCDI);

    spm_kick_im_to_fetch(pcmdesc);

    spm_init_pcm_register(isMCDI);

    spm_init_event_vector(pcmdesc);

    spm_set_ap_standbywfi(FALSE);

    spm_set_wakeup_event(WAKE_SRC_FOR_SODI,isMCDI);
    
    spm_kick_pcm_to_run(cpu_pdn, false, isMCDI);     /* keep INFRA/DDRPHY power */

#if SPM_SODI_DEBUG
    //clc_notice("============SODI Before============\n");
    //spm_sodi_dump_regs(); //dump debug info
#endif
    
    soidle_before_wfi(0);

    spm_trigger_wfi_for_mcdi(cpu_pdn);

    soidle_after_wfi(0);

#if SPM_SODI_DEBUG
    //clc_notice("============SODI After=============\n");
    //spm_sodi_dump_regs();//dump debug info
#endif
    spm_get_wakeup_status(&wakesta);

    spm_clean_after_wakeup();

    wr = spm_output_wake_reason(&wakesta);

    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);

    gSPM_SODI_EN =0;
        
    spin_unlock_irqrestore(&spm_lock, flags);
   
    //return wr;

}


void spm_sodi_lcm_video_mode(bool IsLcmVideoMode)
{
    gSpm_IsLcmVideoMode = IsLcmVideoMode;

#if SPM_SODI_DEBUG    
    printk("spm_sodi_lcm_video_mode() : gSpm_IsLcmVideoMode = %x\n", gSpm_IsLcmVideoMode);    
#endif
    
}

void spm_disable_sodi(void)
{
    //spin_lock(&spm_sodi_lock);

    gSpm_Sodi_Disable_Counter++;
    
#if SPM_SODI_DEBUG    
    printk("spm_disable_sodi() : spm_sodi_disable_counter = 0x%x\n", gSpm_Sodi_Disable_Counter);    
#endif

    if(gSpm_Sodi_Disable_Counter > 0)
    {
        spm_direct_disable_sodi();
    }

    //spin_unlock(&spm_sodi_lock);
}

void spm_enable_sodi(void)
{
    //spin_lock(&spm_sodi_lock);

    gSpm_Sodi_Disable_Counter--;
    
#if SPM_SODI_DEBUG    
    printk("spm_enable_sodi() : spm_sodi_disable_counter = 0x%x\n", gSpm_Sodi_Disable_Counter);    
#endif    

    if(gSpm_Sodi_Disable_Counter <= 0)
    {
        spm_direct_enable_sodi();
    }

    //spin_unlock(&spm_sodi_lock);
}
#endif //SPM_SODI_ENABLED


#ifdef SPM_MCDI_FUNC
bool SPM_MCDI_Enable = 0;
bool SPM_MCDI_isKICK = 0;
unsigned int g_is_mcdi_wfi;

static void spm_mcdi_init_core_mux(void)
{
    int i = 0;
    // Init SPM_MP_COREn_AUX
    spm_write(SPM_MP_CORE0_AUX, spm_read(SPM_MP_CORE0_AUX)&0xfe00ffff);
    spm_write(SPM_MP_CORE1_AUX, spm_read(SPM_MP_CORE1_AUX)&0xfe00ffff);
    spm_write(SPM_MP_CORE2_AUX, spm_read(SPM_MP_CORE2_AUX)&0xfe00ffff); 
    spm_write(SPM_MP_CORE3_AUX, spm_read(SPM_MP_CORE3_AUX)&0xfe00ffff);
    spm_write(SPM_MP_CORE4_AUX, spm_read(SPM_MP_CORE4_AUX)&0xfe00ffff);
    spm_write(SPM_MP_CORE5_AUX, spm_read(SPM_MP_CORE5_AUX)&0xfe00ffff);
    spm_write(SPM_MP_CORE6_AUX, spm_read(SPM_MP_CORE6_AUX)&0xfe00ffff);
    spm_write(SPM_MP_CORE7_AUX, spm_read(SPM_MP_CORE7_AUX)&0xfe00ffff);
    
    for (i = (num_possible_cpus() - 1); i > 0; i--)
    {
        if (cpu_online(i)==0)
        {
            switch(i)
            {
                case 0://core0
                    spm_write(SPM_MP_CORE0_AUX, spm_read(SPM_MP_CORE0_AUX)|SPM_PCM_HOTPLUG);
                    break;               
                case 1://core1
                    spm_write(SPM_MP_CORE1_AUX, spm_read(SPM_MP_CORE1_AUX)|SPM_PCM_HOTPLUG);
                    break;
                case 2://core2
                    spm_write(SPM_MP_CORE2_AUX, spm_read(SPM_MP_CORE2_AUX)|SPM_PCM_HOTPLUG); 
                    break;
                case 3://core3
                    spm_write(SPM_MP_CORE3_AUX, spm_read(SPM_MP_CORE3_AUX)|SPM_PCM_HOTPLUG); 
                    break;
                case 4://core4
                    spm_write(SPM_MP_CORE4_AUX, spm_read(SPM_MP_CORE4_AUX)|SPM_PCM_HOTPLUG);
                    break;               
                case 5://core5
                    spm_write(SPM_MP_CORE5_AUX, spm_read(SPM_MP_CORE5_AUX)|SPM_PCM_HOTPLUG);
                    break;
                case 6://core6
                    spm_write(SPM_MP_CORE6_AUX, spm_read(SPM_MP_CORE6_AUX)|SPM_PCM_HOTPLUG); 
                    break;
                case 7://core7
                    spm_write(SPM_MP_CORE7_AUX, spm_read(SPM_MP_CORE7_AUX)|SPM_PCM_HOTPLUG); 
                    break;

                default: break;                    
            //cpu_down(i);
            }
        }
    }
    

}

wake_reason_t spm_go_to_mcdi(bool cpu_pdn)
{
#if 1 //REMOVE_MCDI_TEST
    wake_status_t wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    const pcm_desc_t *pcmdesc = &pcm_mcdi;
    const bool pcmwdt_en = false;
    const bool isMCDI = TRUE;
    u32 clc_temp;


    spin_lock_irqsave(&spm_lock, flags);
    g_is_mcdi_wfi = 0;

    /*Init WFI SEL*/
    spm_write(SPM_MP0_CORE0_WFI_SEL, 0); 
    spm_write(SPM_MP0_CORE1_WFI_SEL, 0);                      
    spm_write(SPM_MP0_CORE2_WFI_SEL, 0);                     
    spm_write(SPM_MP0_CORE3_WFI_SEL, 0);
    spm_write(SPM_MP1_CORE0_WFI_SEL, 0); 
    spm_write(SPM_MP1_CORE1_WFI_SEL, 0);                      
    spm_write(SPM_MP1_CORE2_WFI_SEL, 0);                     
    spm_write(SPM_MP1_CORE3_WFI_SEL, 0);    

    spm_reset_and_init_pcm(isMCDI);

    spm_kick_im_to_fetch(pcmdesc);

    spm_init_pcm_register(isMCDI);

    //spm_init_event_vector(pcmdesc);//remove event vector from MCDI

    spm_set_ap_standbywfi(isMCDI);

    spm_set_wakeup_event(WAKE_SRC_FOR_MCDI,isMCDI);

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp |= (0x1<<13);//Disable SODI
    
    spm_write(SPM_CLK_CON, clc_temp);     

    spm_kick_pcm_to_run(cpu_pdn, false, isMCDI);     /* keep INFRA/DDRPHY power */

    SPM_MCDI_isKICK = 1;

    spin_unlock_irqrestore(&spm_lock, flags);

    printk("spm_go_to_mcdi()\n");

    return wr;
#endif    
}
unsigned int g_SPM_MCDI_Abnormal_WakeUp = 0;

u32 spm_leave_MCDI(void)
{
#if 1 //REMOVE_MCDI_TEST

    u32 spm_counter;
    u32 spm_core_pws, hotplug_out_core_id;
    unsigned long flags;

    /* Mask ARM i bit */
    local_irq_save(flags);
    
    spin_lock(&spm_lock);
    SPM_MCDI_isKICK = 0;

    /*disable MCDI Dormant*/
    spm_write(SPM_CLK_CON,spm_read(SPM_CLK_CON)|CC_DISABLE_DORM_PWR);
    
    // trigger cpu wake up event
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x1);

    spin_unlock(&spm_lock);   

#if 1
    while(1)
    {
        if(g_is_mcdi_wfi==0)
            break;
        if(spm_counter >= 100000)
        {
            printk("g_is_mcdi_wfi:%x\n",g_is_mcdi_wfi);
            spm_counter=0;
        }
        spm_counter++;
    }
    spm_counter = 0;
#endif    

    /*offload MCDI F/W*/
    spm_write(SPM_CLK_CON,spm_read(SPM_CLK_CON)&~CC_DISABLE_DORM_PWR);  

    // polling SPM_SLEEP_ISR_STATUS ===========================
    spm_counter = 0;


#if 1

    /* clean PCM timer event */
    spm_write(SPM_PCM_CON1, CON1_CFG_KEY | (spm_read(SPM_PCM_CON1) & ~CON1_PCM_TIMER_EN));

    //Polling MCDI return
    while((spm_read(SPM_PCM_REG11_DATA))!=0x55AA55AA)//or R5=0xAA55AA55
    {
        if(spm_counter >= 10000)
            BUG_ON(1);
        spm_counter++;
    }

    //spm_write(SPM_SLEEP_ISR_STATUS,0x8);

    // set cpu wake up event = 0
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x0);     

    /* clean ISR status */
    spm_write(SPM_SLEEP_ISR_MASK, 0xff0c);
    spm_write(SPM_SLEEP_ISR_STATUS, 0x000c);
    spm_write(SPM_PCM_SW_INT_CLEAR, 0x00ff);    /* PCM_SWINT_0 */

    //disable IO output for regiser 0 and 7
    spm_write(SPM_PCM_PWR_IO_EN, 0x0);  

    // print spm debug log ===================================
    //spm_mcdi_dump_regs();

    // clean wakeup event raw status
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, 0xffffffff);
#endif

    /* Un-Mask ARM i bit */
    local_irq_restore(flags);
    //printk("MCDI_EXIT : OK\n");
    return 0;
#endif    
}

extern void mcidle_before_wfi(int cpu);
extern void mcidle_after_wfi(int cpu);

extern void invalidate_unified_TLB(void);//individual core
extern void __enable_cache();


extern unsigned int mcdi_pdn_cnt[8];
extern int mcdi_xgpt_wakeup_cnt[8];
static void spm_mcdi_wfi_sel_enter(int core_id)
{
    /*SPM WFI Select by core number*/
    switch (core_id)
    {
        case 0 : 
            spm_write(SPM_MP0_CPU0_IRQ_MASK, 1);
            spm_write(SPM_MP0_CORE0_WFI_SEL, 1); 
            
            break;
        case 1 : 
            spm_write(SPM_MP0_CPU1_IRQ_MASK, 1);
            spm_write(SPM_MP0_CORE1_WFI_SEL, 1);             
            break;                     
        case 2 : 
            spm_write(SPM_MP0_CPU2_IRQ_MASK, 1);
            spm_write(SPM_MP0_CORE2_WFI_SEL, 1);             
            break;                     
        case 3 : 
            spm_write(SPM_MP0_CPU3_IRQ_MASK, 1);
            spm_write(SPM_MP0_CORE3_WFI_SEL, 1);             
            break;
        case 4 : 
            spm_write(SPM_MP1_CPU0_IRQ_MASK, 1);
            spm_write(SPM_MP1_CORE0_WFI_SEL, 1);             
            break;
        case 5 : 
            spm_write(SPM_MP1_CPU1_IRQ_MASK, 1);
            spm_write(SPM_MP1_CORE1_WFI_SEL, 1);             
            break;                     
        case 6 : 
            spm_write(SPM_MP1_CPU2_IRQ_MASK, 1);
            spm_write(SPM_MP1_CORE2_WFI_SEL, 1);            
            break;                     
        case 7 : 
            spm_write(SPM_MP1_CPU3_IRQ_MASK, 1);
            spm_write(SPM_MP1_CORE3_WFI_SEL, 1);            
            break;         
        default : break;
    }  

}

static void spm_mcdi_wfi_sel_leave(int core_id)
{
    /*SPM WFI Select by core number*/
    switch (core_id)
    {
        case 0 : 
            spm_write(SPM_MP0_CORE0_WFI_SEL, 0); 
            spm_write(SPM_MP0_CPU0_IRQ_MASK, 0);
            break;
        case 1 : 
            spm_write(SPM_MP0_CORE1_WFI_SEL, 0); 
            spm_write(SPM_MP0_CPU1_IRQ_MASK, 0);
            break;                     
        case 2 : 
            spm_write(SPM_MP0_CORE2_WFI_SEL, 0); 
            spm_write(SPM_MP0_CPU2_IRQ_MASK, 0);
            break;                     
        case 3 : 
            spm_write(SPM_MP0_CORE3_WFI_SEL, 0); 
            spm_write(SPM_MP0_CPU3_IRQ_MASK, 0);
            break;
        case 4 : 
            spm_write(SPM_MP1_CORE0_WFI_SEL, 0); 
            spm_write(SPM_MP1_CPU0_IRQ_MASK, 0);
            break;
        case 5 : 
            spm_write(SPM_MP1_CORE1_WFI_SEL, 0); 
            spm_write(SPM_MP1_CPU1_IRQ_MASK, 0);
            break;                     
        case 6 : 
            spm_write(SPM_MP1_CORE2_WFI_SEL, 0); 
            spm_write(SPM_MP1_CPU2_IRQ_MASK, 0);
            break;                     
        case 7 : 
            spm_write(SPM_MP1_CORE3_WFI_SEL, 0);
            spm_write(SPM_MP1_CPU3_IRQ_MASK, 0);
            break;         
        default : break;
    }  

}
//#define REMOVE_MCDI_TEST
bool spm_mcdi_wfi(int core_id)
{   

    //volatile u32 core_id;
    u32 clc_counter;
    u32 spm_val;
    bool ret = 0;
    unsigned long flags;

#if 0//core0 not pd
    if( core_id != 0 )
        return ret;
#endif    
#if 1 //ndef REMOVE_MCDI_TEST

    spin_lock_irqsave(&spm_lock, flags);
    if( SPM_MCDI_isKICK == 0 )
    {
        spin_unlock_irqrestore(&spm_lock, flags);
        return ret;    
    }

    g_is_mcdi_wfi=( g_is_mcdi_wfi | (1<<core_id) );
    spin_unlock_irqrestore(&spm_lock, flags);

    /*core wfi_sel & cpu mask*/
    spm_mcdi_wfi_sel_enter(core_id);  

    /*sync core1~n local timer to XGPT*/
    mcidle_before_wfi(core_id);
#endif

    if (!cpu_power_down(DORMANT_MODE,1)) 
    {
        switch_to_amp();  

        /* do not add code here */
        mcdi_wfi_with_sync();

    }

    /*check if MCDI abort by unkonw IRQ*/
    while((spm_read(SPM_SLEEP_ISR_STATUS)&(1<<(core_id+4)))==0)
    {
        g_SPM_MCDI_Abnormal_WakeUp|=(1<<core_id);
    }    

    switch_to_smp();

    invalidate_unified_TLB();

    __enable_cache();

    cpu_check_mcdi_dormant_abort();
#if 1//ndef REMOVE_MCDI_TEST
    /*clear core wfi_sel & cpu unmask*/
    spm_mcdi_wfi_sel_leave(core_id);  

    mcidle_after_wfi(core_id);    

    /*Clear SPM SW IRQ*/
    spm_write(SPM_PCM_SW_INT_CLEAR, (0x1<<core_id) );    /* PCM_SWINT_3 */
#endif

    ret = 1;

    spin_lock_irqsave(&spm_lock, flags);
    g_is_mcdi_wfi=( g_is_mcdi_wfi &~ (1<<core_id) );
    spin_unlock_irqrestore(&spm_lock, flags);


    //printk("mcdi: core_%d exit wfi, %d.\n", core_id,mcdi_pdn_cnt[core_id]);

    return ret;

}    


// ==============================================================================
static int spm_mcdi_probe(struct platform_device *pdev)
{
    int hibboot = 0;
    hibboot = get_env("hibboot") == NULL ? 0 : simple_strtol(get_env("hibboot"), NULL, 10);

    // set SPM_MP_CORE0_AUX
    spm_mcdi_init_core_mux();

    //#if MCDI_KICK_PCM
    clc_notice("spm_mcdi_probe start.\n");        
    if (1 == hibboot)
    {
        clc_notice("[%s] skip spm_go_to_MCDI due to hib boot\n", __func__);
    }
    else
    {
        SPM_MCDI_Enable = 1;//Start MCDI
        //spm_go_to_mcdi(1);        

    }  
    //#endif
    
    return 0;
}
static void spm_mcdi_early_suspend(struct early_suspend *h) 
{
    clc_notice("spm_mcdi_early_suspend start.\n");
  
    SPM_MCDI_Enable = 0;
}

static void spm_mcdi_late_resume(struct early_suspend *h) 
{
    clc_notice("spm_mcdi_late_resume start.\n");

    SPM_MCDI_Enable = 1;
}



static struct platform_driver mtk_spm_mcdi_driver = {
    .remove     = NULL,
    .shutdown   = NULL,
    .probe      = spm_mcdi_probe,
    .suspend	= NULL,
    .resume		= NULL,
    .driver     = {
        .name = "mtk-spm-mcdi",
    },
};

static struct early_suspend mtk_spm_mcdi_early_suspend_driver =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 251,
    .suspend = spm_mcdi_early_suspend,
    .resume  = spm_mcdi_late_resume,
};

//static int __init spm_mcdi_init(void)
int  spm_mcdi_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;    
    struct proc_dir_entry *mt_mcdi_dir = NULL;
    int mcdi_err = 0;
#if 0
    mt_mcdi_dir = proc_mkdir("mcdi", NULL);
    if (!mt_mcdi_dir)
    {
        clc_notice("[%s]: mkdir /proc/mcdi failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("mcdi_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_mcdi_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = spm_mcdi_debug_read;
            mt_entry->write_proc = spm_mcdi_debug_write;
        }

        mt_entry = create_proc_entry("sodi_en", S_IRUGO | S_IWUSR | S_IWGRP, mt_mcdi_dir);
        if (mt_entry)
        {
            mt_entry->write_proc = spm_user_sodi_en;
        }
    }
#endif

    mcdi_err = platform_driver_register(&mtk_spm_mcdi_driver);
    
    if (mcdi_err)
    {
        clc_notice("spm mcdi driver callback register failed..\n");
        return mcdi_err;
    }


    clc_notice("spm mcdi driver callback register OK..\n");

    register_early_suspend(&mtk_spm_mcdi_early_suspend_driver);

    clc_notice("spm mcdi driver early suspend callback register OK..\n");
    
    return 0;

}

static void __exit spm_mcdi_exit(void)
{
    clc_notice("Exit SPM-MCDI\n\r");
}

/*
    CORE0_AUX  :  PCM_EVENT_VECTOR2[24:16]
    CORE1_AUX  :  PCM_EVENT_VECTOR3[24:16]
    CORE2_AUX  :  PCM_EVENT_VECTOR4[24:16]
    CORE3_AUX  :  PCM_EVENT_VECTOR5[24:16]
    CORE4_AUX  :  PCM_EVENT_VECTOR6[24:16]
    CORE5_AUX  :  PCM_EVENT_VECTOR7[24:16]
    CORE6_AUX  :  PCM_RESERVED[24:16]
    CORE7_AUX  :  { SLEEP_APMCU_PWRCTL[7:0],SLEEP_L2C_TAG_SLEEP[0] }


    Bit0 :  COREn  hot-plugged ?   1: hot-plugged ;  0: non hot-plugged
    Bit1 : Assert IPI to CORE0
    Bit2 : Assert IPI to CORE1
    Bit3 : Assert IPI to CORE2
    Bit4 : Assert IPI to CORE3
    Bit5 : Assert IPI to CORE4
    Bit6 : Assert IPI to CORE5
    Bit7 : Assert IPI to CORE6
    Bit8 : Assert IPI to CORE7

*/

void spm_check_core_status_before(u32 target_core)
{
  
}

void spm_check_core_status_after(u32 target_core)
{
    //check mtcmos power on & clear IPI bits move to PCM
}

void spm_hot_plug_in_before(u32 target_core)
{


    //spm_notice("spm_hot_plug_in_before()........ target_core = 0x%x\n", target_core);

    switch (target_core)
    {
        case 0 : spm_write(SPM_MP_CORE0_AUX, (spm_read(SPM_MP_CORE0_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 1 : spm_write(SPM_MP_CORE1_AUX, (spm_read(SPM_MP_CORE1_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 2 : spm_write(SPM_MP_CORE2_AUX, (spm_read(SPM_MP_CORE2_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 3 : spm_write(SPM_MP_CORE3_AUX, (spm_read(SPM_MP_CORE3_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 4 : spm_write(SPM_MP_CORE4_AUX, (spm_read(SPM_MP_CORE4_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 5 : spm_write(SPM_MP_CORE5_AUX, (spm_read(SPM_MP_CORE5_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 6 : spm_write(SPM_MP_CORE6_AUX, (spm_read(SPM_MP_CORE6_AUX) & (~SPM_PCM_HOTPLUG)));  break;
        case 7 : spm_write(SPM_MP_CORE7_AUX, (spm_read(SPM_MP_CORE7_AUX) & (~SPM_PCM_HOTPLUG)));  break;        
        default : break;
    }

}

void spm_hot_plug_out_after(u32 target_core)
{

    //spm_notice("spm_hot_plug_out_after()........ target_core = 0x%x\n", target_core);    

    switch (target_core)
    {
        case 0 : spm_write(SPM_MP_CORE0_AUX, (spm_read(SPM_MP_CORE0_AUX) | SPM_PCM_HOTPLUG));  break;                     
        case 1 : spm_write(SPM_MP_CORE1_AUX, (spm_read(SPM_MP_CORE1_AUX) | SPM_PCM_HOTPLUG));  break; 
        case 2 : spm_write(SPM_MP_CORE2_AUX, (spm_read(SPM_MP_CORE2_AUX) | SPM_PCM_HOTPLUG));  break;                        
        case 3 : spm_write(SPM_MP_CORE3_AUX, (spm_read(SPM_MP_CORE3_AUX) | SPM_PCM_HOTPLUG));  break;  
        case 4 : spm_write(SPM_MP_CORE4_AUX, (spm_read(SPM_MP_CORE4_AUX) | SPM_PCM_HOTPLUG));  break;                     
        case 5 : spm_write(SPM_MP_CORE5_AUX, (spm_read(SPM_MP_CORE5_AUX) | SPM_PCM_HOTPLUG));  break; 
        case 6 : spm_write(SPM_MP_CORE6_AUX, (spm_read(SPM_MP_CORE6_AUX) | SPM_PCM_HOTPLUG));  break;                        
        case 7 : spm_write(SPM_MP_CORE7_AUX, (spm_read(SPM_MP_CORE7_AUX) | SPM_PCM_HOTPLUG));  break;         
        default : break;
    }    

}

void spm_mcdi_wakeup_all_cores(void)
{
    unsigned int spm_counter = 0;
    /*disable MCDI Dormant*/
    spm_write(SPM_CLK_CON,spm_read(SPM_CLK_CON)|CC_DISABLE_DORM_PWR);
    
    // trigger cpu wake up event
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x1);


    while(1)
    {
        if(g_is_mcdi_wfi==0)
            break;
        if(spm_counter >= 100000)
        {
            printk("g_is_mcdi_wfi:%x\n",g_is_mcdi_wfi);
            spm_counter=0;
        }
        spm_counter++;
    }
    spm_counter = 0;
    // trigger cpu wake up event
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0x0);  
    /*disable MCDI Dormant*/
    spm_write(SPM_CLK_CON,spm_read(SPM_CLK_CON)&~CC_DISABLE_DORM_PWR);
    
}
#endif// SPM_MCDI_FUNC


MODULE_DESCRIPTION("SPM-Idle Driver v1.3");
