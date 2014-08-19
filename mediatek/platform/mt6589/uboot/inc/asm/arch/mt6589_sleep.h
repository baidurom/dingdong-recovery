#include <common.h>

#include <asm/io.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/mt65xx_typedefs.h>

/* extern function */
extern void sc_mod_init(void);
extern void sc_mod_exit(void);
extern void gpt_one_shot_irq(unsigned int ms);
extern void mtk_sleep(u32 timeout, kal_bool en_deep_idle);
