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

#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/switch.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/time.h>

#include <linux/string.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/irqs.h>

#include "hall.h"
#include <mach/mt_boot.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>

//#define GPIO_HALL_EINT_PIN GPIO116	//move to dct
//#define CUST_EINT_HALL_NUM 11		//move to dct

int hall_cur_eint_state = HALL_FAR;

extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

static struct workqueue_struct * hall_eint_workqueue = NULL;
static struct work_struct hall_eint_work;

static struct switch_dev hall_data;

void hall_eint_work_callback(struct work_struct *work)
{
	HALL_FUNC();
    mt65xx_eint_mask(CUST_EINT_HALL_NUM);
	if(hall_cur_eint_state == HALL_NEAR)
	{
		HALL_DEBUG("HALL_NEAR\n");
        switch_set_state((struct switch_dev *)&hall_data, HALL_NEAR);
	}
	else
	{
		HALL_DEBUG("HALL_FAR\n");
        switch_set_state((struct switch_dev *)&hall_data, HALL_FAR);
	}
    mt65xx_eint_unmask(CUST_EINT_HALL_NUM);
}

void hall_eint_func(void)
{
	int ret;
	
	HALL_FUNC();
	if(hall_cur_eint_state ==  HALL_FAR ) 
	{
		mt65xx_eint_set_polarity(CUST_EINT_HALL_NUM, !(CUST_EINT_ACCDET_POLARITY));
		hall_cur_eint_state = HALL_NEAR;
	}
	else
	{
		mt65xx_eint_set_polarity(CUST_EINT_HALL_NUM, (CUST_EINT_ACCDET_POLARITY));
		hall_cur_eint_state = HALL_FAR;
	}

	
	ret = queue_work(hall_eint_workqueue, &hall_eint_work); 
}

static inline int hall_setup_eint(void)
{
	HALL_FUNC();
	
	mt_set_gpio_mode(GPIO_HALL_EINT_PIN, GPIO_HALL_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_HALL_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_HALL_EINT_PIN, GPIO_PULL_DISABLE);

    mt65xx_eint_set_sens(CUST_EINT_HALL_NUM, CUST_EINT_HALL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_HALL_NUM, CUST_EINT_HALL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_HALL_NUM, CUST_EINT_HALL_DEBOUNCE_EN, CUST_EINT_HALL_POLARITY, hall_eint_func, 0);
	mt65xx_eint_unmask(CUST_EINT_HALL_NUM);  

	return 0;
}

static int hall_probe(struct platform_device *dev)
{
	int ret = 0;
	
	HALL_FUNC();

	hall_data.name = "hall";
	hall_data.index = 0;
	hall_data.state = HALL_FAR;

	ret = switch_dev_register(&hall_data);
	if(ret)
	{
		HALL_DEBUG("switch_dev_register return %d\n", ret);
	}

	hall_eint_workqueue = create_singlethread_workqueue("hall_eint");
	INIT_WORK(&hall_eint_work, hall_eint_work_callback);

	hall_setup_eint();
	
	return 0;
}

static int hall_remove(struct platform_device *dev)
{
	HALL_FUNC();

	destroy_workqueue(hall_eint_workqueue);
	switch_dev_unregister(&hall_data);

	return 0;
}

static struct platform_driver hall_driver = {
	.probe = hall_probe,
	.suspend = NULL,
	.resume = NULL,
	.remove = hall_remove,
	.driver = {
		.name = "hall_driver",
	},
};

static int hall_mod_init(void)
{
	int ret = 0;

	HALL_FUNC();
	
	if(platform_driver_register(&hall_driver) != 0)
	{
		HALL_DEBUG("unable to register hall driver\n");
		return -1;
	}
	
	return 0;
}

static void hall_mod_exit(void)
{
	HALL_FUNC();

	platform_driver_unregister(&hall_driver);
}

module_init(hall_mod_init);
module_exit(hall_mod_exit);

MODULE_DESCRIPTION("Vanzo Hall driver");
MODULE_AUTHOR("AL <lubaoquan@vanzotec.com>");
MODULE_LICENSE("GPL");
