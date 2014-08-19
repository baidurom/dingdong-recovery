#ifndef __MTK_KEY_H__
#define __MTK_KEY_H__

#include <asm/arch/mt65xx.h>

#define KP_STA		(KP_BASE + 0x0000)
#define KP_MEM1		(KP_BASE + 0x0004)
#define KP_MEM2		(KP_BASE + 0x0008)
#define KP_MEM3		(KP_BASE + 0x000c)
#define KP_MEM4		(KP_BASE + 0x0010)
#define KP_MEM5		(KP_BASE + 0x0014)
#define KP_DEBOUNCE	(KP_BASE + 0x0018)

#define KPD_NUM_MEMS	5
#define KPD_MEM5_BITS	8

#define KPD_NUM_KEYS	72	/* 4 * 16 + KPD_MEM5_BITS */

void set_kpd_pmic_mode();

extern bool mt6575_detect_key(unsigned short key);
extern bool mt6575_detect_pmic_just_rst(void);

#endif /* __MT6575_KEY_H__ */
