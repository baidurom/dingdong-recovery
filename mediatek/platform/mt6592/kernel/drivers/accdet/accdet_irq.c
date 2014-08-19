#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/ctype.h>

#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/switch.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/input.h>
#include <linux/wakelock.h>
#include <linux/time.h>

#include <linux/string.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/irqs.h>
#include <mach/reg_accdet.h>
#include <accdet_custom.h>
#include <accdet_custom_def.h>

//#include <mach/mt_boot.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>

extern void accdet_workqueue_func(void);
extern inline void clear_accdet_interrupt(void);
extern void accdet_auxadc_switch_on(void);
extern U32 accdet_get_irq_state(void);
/*
static int debug_enable_irq = 1;

#define ACCDET_DEBUG_IRQ(format, args...) do{ \
	if(debug_enable_irq) \
	{\
		printk(KERN_ERR format,##args);\
	}\
}while(0)

*/

irqreturn_t accdet_handler_irq(int irq,void *dev_id)
{
/*
	//int ret = 0 ;
    
    //ACCDET_DEBUG_IRQ("[Accdet]accdet interrupt happen\n"); //decrease top-half ISR cost time
    //disable_irq_nosync(MT6577_ACCDET_IRQ_ID);
    clear_accdet_interrupt();
    accdet_workqueue_func(); 
*/
    return IRQ_HANDLED;

}

void accdet_set_interrupt(void)
{
	//mt6577_irq_set_sens(MT6577_ACCDET_IRQ_ID, MT65xx_EDGE_SENSITIVE);
	//mt_irq_set_sens(MT_ACCDET_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
	//mt_irq_set_polarity(MT_ACCDET_IRQ_ID, MT65xx_POLARITY_LOW);
}
void accdet_irq_register(void)
{
/*
	int ret;
	ret =  request_irq(MT_ACCDET_IRQ_ID, accdet_handler_irq, 0, "ACCDET", NULL);
	if(ret)
	{
		ACCDET_DEBUG_IRQ("[Accdet]accdet register interrupt error\n");
	}
*/
}

void accdet_irq_free(void)
{
/*
	free_irq(MT_ACCDET_IRQ_ID,NULL);
*/
}

int accdet_irq_handler(void)
{
	int i = 0;
	if(accdet_get_irq_state()) {
		clear_accdet_interrupt();
	}
	accdet_auxadc_switch_on();
    accdet_workqueue_func();  
	while(accdet_get_irq_state() && i<10) {
		i++;
		udelay(200);
	}
    return 1;
}
