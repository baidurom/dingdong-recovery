#ifndef _MT_SPM_IDLE_
#define _MT_SPM_IDLE_

#include <linux/kernel.h>

#define SPM_SODI_ENABLED
//#define SPM_MCDI_FUNC

#define clc_emerg(fmt, args...)     printk(KERN_EMERG "[CLC] " fmt, ##args)
#define clc_alert(fmt, args...)     printk(KERN_ALERT "[CLC] " fmt, ##args)
#define clc_crit(fmt, args...)      printk(KERN_CRIT "[CLC] " fmt, ##args)
#define clc_error(fmt, args...)     printk(KERN_ERR "[CLC] " fmt, ##args)
#define clc_warning(fmt, args...)   printk(KERN_WARNING "[CLC] " fmt, ##args)
#define clc_notice(fmt, args...)    printk(KERN_NOTICE "[CLC] " fmt, ##args)
#define clc_info(fmt, args...)      printk(KERN_INFO "[CLC] " fmt, ##args)
#define clc_debug(fmt, args...)     printk(KERN_DEBUG "[CLC] " fmt, ##args)

#ifdef SPM_SODI_ENABLED
void spm_go_to_sodi(bool cpu_pdn);
void spm_sodi_lcm_video_mode(bool IsLcmVideoMode);
void spm_disable_sodi(void);
void spm_enable_sodi(void);
#endif //SPM_SODI_ENABLED

#ifdef SPM_MCDI_FUNC
void spm_hot_plug_in_before(u32 target_core);
void spm_hot_plug_out_after(u32 target_core);
int spm_mcdi_init(void);
void spm_mcdi_wakeup_all_cores(void);
#endif

#endif
