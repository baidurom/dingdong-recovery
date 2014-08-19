/* drivers/hwmon/mt6516/amit/TMD2771.c - TMD2771 ALS/PS driver
 *
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
//#include "tmd2771.h"
/******************************************************************************
 * configuration
*******************************************************************************/
/*----------------------------------------------------------------------------*/
#define touch_DEV_NAME     "touch"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO fmt, ##args)
/******************************************************************************
 * extern functions
*******************************************************************************/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static struct touch_priv *g_touch_ptr = NULL;
static struct platform_driver touch_alsps_driver;

/*----------------------------------------------------------------------------*/
typedef enum {
    CMC_TRC_PS_DATA = 0x0002,
    CMC_TRC_IOCTL   = 0x0008,
    CMC_TRC_I2C     = 0x0010,
    CMC_TRC_CVT_PS  = 0x0040,
    CMC_TRC_DEBUG   = 0x8000,
} CMC_TRC;
/*----------------------------------------------------------------------------*/
typedef enum {
    CMC_BIT_PS     = 2,
} CMC_BIT;
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
struct touch_priv {


    /*misc*/
    atomic_t    trace;
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;


    /*data*/
    u8          ps;
    u8          _align;
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/

    /*early suspend*/
};
/*----------------------------------------------------------------------------*/

static struct touch_priv *touch_obj = NULL;
static int touch_get_ps_value(u16 ps);
static int touch_read_ps(u8 *data);
/*----------------------------------------------------------------------------*/
static bool PS_VAL = 1;
/*----------------------------------------------------------------------------*/
extern void (* tpd_ps_enable)(bool on);

int touch_set_ps(bool val)
{
    int err;
    hwm_sensor_data sensor_data;
    memset(&sensor_data, 0, sizeof(sensor_data));

    sensor_data.values[0] = val;
    sensor_data.value_divide = 1;
    sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
    //let up layer to know
    if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
    {
        APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
    }
    //map and store data to hwm_sensor_data
    PS_VAL = val ;
    return 0;
}
/*----------------------------------------------------------------------------*/
int touch_read_ps(u8 *data)
{
    *data = PS_VAL;
    return 0;
}
/*----------------------------------------------------------------------------*/
int touch_write_ps(u8 cmd)
{
	u8 buf = cmd;
	int ret = 0;

	return 0;
}
/*----------------------------------------------------------------------------*/
int touch_write_ps_thd(u8 thd)
{
	u8 buf = thd;
	int ret = 0;

	return 0;
}
/*----------------------------------------------------------------------------*/
static int touch_enable_ps(int enable)
{
	if(tpd_ps_enable == NULL)
	{
		APS_ERR("ERROR: Not implement touch ps function!\n");
		return 0;
	}

	if(enable)
	{
		(*tpd_ps_enable)(1);
	}
	else
	{
		(*tpd_ps_enable)(0);
	}


	return 0;
}
/*----------------------------------------------------------------------------*/
/******************************************************************************
 * Sysfs attributes
*******************************************************************************/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static ssize_t touch_show_ps(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	if(!touch_obj)
	{
		APS_ERR("touch_obj is null!!\n");
		return 0;
	}

	if((res = touch_read_ps(&touch_obj->ps)))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
	else
	{
		return snprintf(buf, PAGE_SIZE, "%d\n", touch_obj->ps);
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(ps,      S_IWUSR | S_IRUGO, touch_show_ps,    NULL);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *touch_attr_list[] = {
    &driver_attr_ps,
};

/*----------------------------------------------------------------------------*/
static int touch_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(touch_attr_list)/sizeof(touch_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, touch_attr_list[idx])))
		{
			APS_ERR("driver_create_file (%s) = %d\n", touch_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int touch_delete_attr(struct device_driver *driver)
{
    int idx ,err = 0;
    int num = (int)(sizeof(touch_attr_list)/sizeof(touch_attr_list[0]));

    if (!driver)
        return -EINVAL;

    for (idx = 0; idx < num; idx++)
    {
        driver_remove_file(driver, touch_attr_list[idx]);
    }

    return err;
}
/******************************************************************************
 * Function Configuration
******************************************************************************/
/*----------------------------------------------------------------------------*/
static int touch_get_ps_value(u16 ps)
{

    return ps;
}
/******************************************************************************
 * Function Configuration
******************************************************************************/
static int touch_open(struct inode *inode, struct file *file)
{

	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int touch_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static int touch_unlocked_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct touch_priv *obj = touch_obj;
	int err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;

	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = touch_enable_ps(1)))
				{
					APS_ERR("enable ps fail: %ld\n", err);
					goto err_out;
				}

			}
			else
			{
				if((err = touch_enable_ps(0)))
				{
					APS_ERR("disable ps fail: %ld\n", err);
					goto err_out;
				}

			}
			break;

		case ALSPS_GET_PS_MODE:
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:
			if((err = touch_read_ps(&obj->ps)))
			{
				goto err_out;
			}

			dat = touch_get_ps_value(obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_RAW_DATA:
			if((err = touch_read_ps(&obj->ps)))
			{
				goto err_out;
			}

			dat = obj->ps;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;
}
/*----------------------------------------------------------------------------*/
static struct file_operations touch_fops = {
	.owner = THIS_MODULE,
	.open = touch_open,
	.release = touch_release,
	//.ioctl = touch_ioctl,
	.unlocked_ioctl = touch_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice touch_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &touch_fops,
};
/*----------------------------------------------------------------------------*/

int touch_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct touch_priv *obj = (struct touch_priv *)self;

	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(value)
				{
					if((err = touch_enable_ps(1)))
					{
						APS_ERR("enable ps fail: %d\n", err);
						return -1;
					}
				}
				else
				{
					if((err = touch_enable_ps(0)))
					{
						APS_ERR("disable ps fail: %d\n", err);
						return -1;
					}
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;

				if((err = touch_read_ps(&obj->ps)))
				{
					err = -1;;
				}
				else
				{
					sensor_data->values[0] = touch_get_ps_value(obj->ps);
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				}
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}

/*----------------------------------------------------------------------------*/
static int touch_probe(struct platform_device *pdev)
{

    struct hwmsen_object obj_ps;
    int err = 0;
    if(!(touch_obj = kzalloc(sizeof(touch_obj), GFP_KERNEL)))
    {
        err = -ENOMEM;
    }
    memset(touch_obj, 0, sizeof(touch_obj));

    if((err = misc_register(&touch_device)))
    {
        APS_ERR("touch_device register failed\n");
    }

    obj_ps.self = touch_obj;
    obj_ps.polling = 0;
    obj_ps.sensor_operate = touch_ps_operate;
    if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
    {
        APS_ERR("attach fail = %d\n", err);
    }
    touch_create_attr(&touch_alsps_driver.driver);
    APS_LOG("%s: OK\n", __func__);

    return 0;
}
/*----------------------------------------------------------------------------*/
static int touch_remove(struct platform_device *pdev)
{
	return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver touch_alsps_driver = {
	.probe      = touch_probe,
	.remove     = touch_remove,
	.driver     = {
		.name  = "als_ps",
	}
};
/*----------------------------------------------------------------------------*/
static int __init touch_init(void)
{
	if(platform_driver_register(&touch_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit touch_exit(void)
{
	platform_driver_unregister(&touch_alsps_driver);
}
/*----------------------------------------------------------------------------*/
module_init(touch_init);
module_exit(touch_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("zhangqingzhan");
MODULE_DESCRIPTION("touch ALS/PS driver");
MODULE_LICENSE("GPL");
