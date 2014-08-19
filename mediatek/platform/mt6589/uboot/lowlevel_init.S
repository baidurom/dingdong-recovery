#include <config.h>
#include <version.h>

_TEXT_BASE:
	.word	TEXT_BASE	/* sdram load addr from config.mk */

.globl lowlevel_init
lowlevel_init:
	ldr	sp,	SRAM_STACK
	str	ip,	[sp]	/* stash old link register */

	/* Enable branch prediction and icache */
	mrc	p15, 0, ip, c1, c0, 0
	orr	ip, ip, #0x1800
	mcr	p15, 0, ip, c1, c0, 0
	
	mov	ip,	lr	/* save link reg across call */
	bl	system_init	/* go setup pll,mux,memory */
	ldr	ip,	[sp]	/* restore save ip */
	mov	lr,	ip	/* restore link reg */

	/* back to arch calling code */
	mov	pc,	lr

	/* the literal pools origin */
	.ltorg

SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK
