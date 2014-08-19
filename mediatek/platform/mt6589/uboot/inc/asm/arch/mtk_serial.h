#ifndef ___MTK_SERIAL_H__
#define ___MTK_SERIAL_H__

#include <asm/arch/mt65xx.h>

typedef enum
{
	UART1 = UART1_BASE,
	UART2 = UART2_BASE,
	UART3 = UART3_BASE,
	UART4 = UART4_BASE
} MTK_UART;

extern void mtk_serial_init(void);
extern int serial_nonblock_getc(void);
extern void mtk_serial_set_current_uart(MTK_UART uart_base);

#endif /* __MT65XX_SERIAL_H__ */

