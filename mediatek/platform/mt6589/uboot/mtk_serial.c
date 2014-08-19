/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/mtk_uart.h>
#include <asm/arch/mtk_serial.h>

#ifdef CFG_META_MODE
#include <asm/arch/mt65xx_meta.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define ASSERT( expr ) if( !( expr ) ) { \
		printf("<ASSERT> %s:line %d %s\n",__FILE__,__LINE__,(int)(#expr)); }

#define UART_READ8(REG)             __raw_readb(REG)
#define UART_READ16(REG)            __raw_readw(REG)
#define UART_READ32(REG)            __raw_readl(REG)
#define UART_WRITE8(VAL, REG)       __raw_writeb(VAL, REG)
#define UART_WRITE16(VAL, REG)      __raw_writew(VAL, REG)
#define UART_WRITE32(VAL, REG)      __raw_writel(VAL, REG)

#define UART_SET_BITS(BS,REG)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define UART_CLR_BITS(BS,REG)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

#define UART_BASE(uart)					  (uart)

#define UART_RBR(uart)                    (UART_BASE(uart)+0x0)  /* Read only */
#define UART_THR(uart)                    (UART_BASE(uart)+0x0)  /* Write only */
#define UART_IER(uart)                    (UART_BASE(uart)+0x4)
#define UART_IIR(uart)                    (UART_BASE(uart)+0x8)  /* Read only */
#define UART_FCR(uart)                    (UART_BASE(uart)+0x8)  /* Write only */
#define UART_LCR(uart)                    (UART_BASE(uart)+0xc)
#define UART_MCR(uart)                    (UART_BASE(uart)+0x10)
#define UART_LSR(uart)                    (UART_BASE(uart)+0x14)
#define UART_MSR(uart)                    (UART_BASE(uart)+0x18)
#define UART_SCR(uart)                    (UART_BASE(uart)+0x1c)
#define UART_DLL(uart)                    (UART_BASE(uart)+0x0)  /* Only when LCR.DLAB = 1 */
#define UART_DLH(uart)                    (UART_BASE(uart)+0x4)  /* Only when LCR.DLAB = 1 */
#define UART_EFR(uart)                    (UART_BASE(uart)+0x8)  /* Only when LCR = 0xbf */
#define UART_XON1(uart)                   (UART_BASE(uart)+0x10) /* Only when LCR = 0xbf */
#define UART_XON2(uart)                   (UART_BASE(uart)+0x14) /* Only when LCR = 0xbf */
#define UART_XOFF1(uart)                  (UART_BASE(uart)+0x18) /* Only when LCR = 0xbf */
#define UART_XOFF2(uart)                  (UART_BASE(uart)+0x1c) /* Only when LCR = 0xbf */
#define UART_AUTOBAUD_EN(uart)            (UART_BASE(uart)+0x20)
#define UART_HIGHSPEED(uart)              (UART_BASE(uart)+0x24)
#define UART_SAMPLE_COUNT(uart)           (UART_BASE(uart)+0x28) 
#define UART_SAMPLE_POINT(uart)           (UART_BASE(uart)+0x2c) 
#define UART_AUTOBAUD_REG(uart)           (UART_BASE(uart)+0x30)
#define UART_RATE_FIX_AD(uart)            (UART_BASE(uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)        (UART_BASE(uart)+0x38)
#define UART_GUARD(uart)                  (UART_BASE(uart)+0x3c)
#define UART_ESCAPE_DAT(uart)             (UART_BASE(uart)+0x40)
#define UART_ESCAPE_EN(uart)              (UART_BASE(uart)+0x44)
#define UART_SLEEP_EN(uart)               (UART_BASE(uart)+0x48)
#define UART_VFIFO_EN(uart)               (UART_BASE(uart)+0x4c)
#define UART_RXTRI_AD(uart)               (UART_BASE(uart)+0x50)
/*------------------------------------------------------------------------------------------------*/
/* Configure section                                                                              */
/*------------------------------------------------------------------------------------------------*/
#ifdef CFG_MT6589_FPGA
#define UART_ON_FPGA
#endif
//#define __ENABLE_UART_LOG_SWITCH_FEATURE__

#ifdef UART_ON_FPGA
#define UART_SRC_CLK     12000000    /*for fpga-chip*/
#else
//#define UART_SRC_CLK     65000000    /*for real-chip*/
#define UART_SRC_CLK     49400000    /*for real-chip*/
#endif

int mtk_uart_power_on(MTK_UART uart)
{
#ifdef UART_ON_FPGA    /*for fpga-chip*/
    return 0;
#else
    /* UART Powr PDN and Reset*/
    #define AP_PERI_GLOBALCON_RST0 (PERI_CON_BASE+0x0)
    #define AP_PERI_GLOBALCON_PDN0 (PERI_CON_BASE+0x10)
    if (uart == UART1)
        UART_CLR_BITS(1 << 24, AP_PERI_GLOBALCON_PDN0); /* Power on UART1 */
    else if (uart == UART4)
        UART_CLR_BITS(1 << 27, AP_PERI_GLOBALCON_PDN0); /* Power on UART4 */
    return 0;
#endif    
}

/*------------------------------------------------------------------------------------------------*/
// output uart port
volatile unsigned int g_uart;
// output uart baudrate
unsigned int g_brg;
// get bus clock
extern unsigned int mtk_get_bus_freq(void);

void serial_setbrg()
{
    unsigned int byte,speed;
    unsigned int highspeed;
    unsigned int quot, divisor, remainder;
    //unsigned int ratefix; 
    unsigned int uartclk;
    //unsigned int highclk = (UART_SRC_CLK/(speed*4)) > 10 ? (1) : 0;
    unsigned short data, high_speed_div, sample_count, sample_point;
    unsigned int tmp_div;

    speed = g_brg;
#ifdef UART_ON_FPGA
    uartclk = UART_SRC_CLK;
#else
    uartclk = (unsigned int)(mtk_get_bus_freq()*1000/4);
#endif
    if (speed <= 115200 ) {
        highspeed = 0;
        quot = 16;
    } else {
        highspeed = 3;
        quot = 1;
    }

    if (highspeed < 3) { /*0~2*/
        /* Set divisor DLL and DLH  */             
        divisor   =  uartclk / (quot * speed);
        remainder =  uartclk % (quot * speed);
              
        if (remainder >= (quot / 2) * speed)
            divisor += 1;

        UART_WRITE16(highspeed, UART_HIGHSPEED(g_uart));
        byte = UART_READ32(UART_LCR(g_uart));     /* DLAB start */
        UART_WRITE32((byte | UART_LCR_DLAB), UART_LCR(g_uart));
        UART_WRITE32((divisor & 0x00ff), UART_DLL(g_uart));
        UART_WRITE32(((divisor >> 8)&0x00ff), UART_DLH(g_uart));
        UART_WRITE32(byte, UART_LCR(g_uart));     /* DLAB end */
    } else {
        data=(unsigned short)(uartclk/speed);
        high_speed_div = (data>>8) + 1; // divided by 256

        tmp_div=uartclk/(speed*high_speed_div);
        divisor =  (unsigned short)tmp_div;

        remainder = (uartclk)%(high_speed_div*speed);
        /*get (sample_count+1)*/
        if (remainder >= ((speed)*(high_speed_div))>>1)
            divisor =  (unsigned short)(tmp_div+1);
        else
            divisor =  (unsigned short)tmp_div;
        
        sample_count=divisor-1;
        
        /*get the sample point*/
        sample_point=(sample_count-1)>>1;
        
        /*configure register*/
        UART_WRITE32(highspeed, UART_HIGHSPEED(g_uart));
        
        byte = UART_READ32(UART_LCR(g_uart));     /* DLAB start */
        UART_WRITE32((byte | UART_LCR_DLAB), UART_LCR(g_uart));
        UART_WRITE32((high_speed_div & 0x00ff), UART_DLL(g_uart));
        UART_WRITE32(((high_speed_div >> 8)&0x00ff), UART_DLH(g_uart));
        UART_WRITE32(sample_count, UART_SAMPLE_COUNT(g_uart));
        UART_WRITE32(sample_point, UART_SAMPLE_POINT(g_uart));
        UART_WRITE32(byte, UART_LCR(g_uart));     /* DLAB end */
    }
}

// Shu-Hsin : add this non-blocking getc fucntion
int serial_nonblock_getc(void)
{
 	return (int)UART_READ32(UART_RBR(g_uart));
}

int serial_getc(void)
{
	while (!(UART_READ32(UART_LSR(g_uart)) & UART_LSR_DR)); 	
 	return (int)UART_READ32(UART_RBR(g_uart));
}

void serial_putc(const char c)
{
	while (!(UART_READ32(UART_LSR(g_uart)) & UART_LSR_THRE));
	
	if (c == '\n')
		UART_WRITE32((unsigned int)'\r', UART_THR(g_uart));

	UART_WRITE32((unsigned int)c, UART_THR(g_uart));
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc(void)
{
	return UART_READ32(UART_LSR(g_uart)) & UART_LSR_DR;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init(void)
{
	return 0;
}

void mtk_serial_set_current_uart(MTK_UART uart_base)
{
	switch(uart_base)
	{	
        case UART1 :
		g_uart = uart_base;
		break;
	case UART4 :
		g_uart = uart_base;
		break;
	default:
		ASSERT(0);
		break;
	}
}

static int get_uart_port_id(void);
void mtk_serial_init(void)
{

	gd->bd->bi_baudrate = CONFIG_BAUDRATE;

#ifdef CFG_META_MODE
	mtk_serial_set_current_uart(UART1);	
	mtk_uart_power_on(UART1);
	UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */	
	UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
	g_brg = CONFIG_META_BAUDRATE;
	serial_setbrg();         
#endif

	#ifdef __ENABLE_UART_LOG_SWITCH_FEATURE__
	if(get_uart_port_id() == 1){
		mtk_serial_set_current_uart(UART1);
		mtk_uart_power_on(UART1);
	}else{
		mtk_serial_set_current_uart(UART4);
		mtk_uart_power_on(UART4);
	}
	#else
	mtk_serial_set_current_uart(UART4);
	mtk_uart_power_on(UART4);
	#endif
#ifdef UART_ON_FPGA    
        mtk_serial_set_current_uart(UART1);
        mtk_uart_power_on(UART1);
#endif

	UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */	
	UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
	g_brg = CONFIG_BAUDRATE;
	serial_setbrg();
}


#ifdef __ENABLE_UART_LOG_SWITCH_FEATURE__

typedef struct {
	unsigned int  magic;
	unsigned char boot_mode;
	unsigned int  e_flag;
	unsigned int  log_port;
	unsigned int  log_baudrate;
	unsigned char log_enable;
	unsigned char reserved[3];
}boot_arg_t;

int get_uart_port_id(void)
{
	boot_arg_t *boot_arg;
	unsigned int mode = 0;
	unsigned int log_port;
	unsigned int log_enable;
	unsigned int  log_baudrate;

	boot_arg = (volatile BOOT_ARGUMENT *)(BOOT_ARGUMENT_LOCATION);

	mode = boot_arg->boot_mode &= 0x000000FF;
	log_port = boot_arg->log_port;
	log_enable = boot_arg->log_enable;
	log_baudrate = boot_arg->log_baudrate;
	
	if( (log_port == UART1)&&(log_enable != 0) )
		return 1;
	return 4;
}

static void change_uart_port(char * cmd_line, char new_val)
{
	int i;
	int len;
	char *ptr;
	if(NULL == cmd_line)
		return;

	len = strlen(cmd_line);
	ptr = cmd_line;

	i = strlen("ttyMT");
	if(len < i)
		return;
	len = len-i;

	for(i=0; i<=len; i++)
	{
		if(strncmp(ptr, "ttyMT", 5)==0)
		{
			ptr[5] = new_val; // Find and modify
			break;
		}
		ptr++;
	}
}
void custom_port_in_kernel(BOOTMODE boot_mode, char *command)
{
	if(get_uart_port_id() == 1){
		change_uart_port(command, '0');
	}
}

#else
void custom_port_in_kernel(BOOTMODE boot_mode, char *command)
{
	// Dummy function case
}

int get_uart_port_id(void)
{
	// Dummy function case
}
#endif
