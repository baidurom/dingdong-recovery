/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/* linux/drivers/hwmon/kxtj9.c
 *
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 *
 * KXTJ9 driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#ifdef MT6575
#include <mach/mt6575_devs.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_gpio.h>
#include <mach/mt6575_pm_ldo.h>
#endif
#ifdef MT6589
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "kxtj9.h"
#include <linux/hwmsen_helper.h>


#ifdef MT6516
#define POWER_NONE_MACRO MT6516_POWER_NONE
#endif

#ifdef MT6573
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif

#ifdef MT6575
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif

#ifdef MT6589
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif
/******************************************************************************
 * structure/enumeration
*******************************************************************************/
struct kxtj9_object{
	struct i2c_client	    client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;


    /*software calibration: the unit follows standard format*/
    s16                     cali_sw[KXTJ9_AXES_NUM+1];

    /*misc*/
    atomic_t                enable;
    atomic_t                suspend;
    atomic_t                trace;

    /*data*/
    u8                      data[KXTJ9_DATA_LEN+1];

};
/*----------------------------------------------------------------------------*/
#define KXTJ9_TRC_GET_DATA  0x0001


/*----------------------------------------------------------------------------*/
#define DEBUG 1
/*----------------------------------------------------------------------------*/
#define KXTJ9_AXIS_X          0
#define KXTJ9_AXIS_Y          1
#define KXTJ9_AXIS_Z          2
#define KXTJ9_AXES_NUM        3
#define KXTJ9_DEV_NAME        "KXTJ9"
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id kxtj9_i2c_id[] = {{KXTJ9_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_kxtj9={ I2C_BOARD_INFO("KXTJ9", (KXTJ9_WR_SLAVE_ADDR >> 1))};

/*the adapter id will be available in customization*/
//static unsigned short kxtj9_force[] = {0x00, KXTJ9_WR_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const kxtj9_forces[] = { kxtj9_force, NULL };
//static struct i2c_client_address_data kxtj9_addr_data = { .forces = kxtj9_forces,};

/*----------------------------------------------------------------------------*/
static int kxtj9_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int kxtj9_i2c_remove(struct i2c_client *client);
//static int kxtj9_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int  kxtj9_local_init(void);
static int kxtj9_remove(void);

static int kxtj9_init_flag =-1; // 0<==>OK -1 <==> fail
static struct sensor_init_info kxtj9_init_info = {
	.name = "kxtj9",
	.init = kxtj9_local_init,
	.uninit = kxtj9_remove,
};
/*----------------------------------------------------------------------------*/
typedef enum {
    ADX_TRC_FILTER  = 0x01,
    ADX_TRC_RAWDATA = 0x02,
    ADX_TRC_IOCTL   = 0x04,
    ADX_TRC_CALI	= 0X08,
    ADX_TRC_INFO	= 0X10,
} ADX_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};

/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][KXTJ9_AXES_NUM];
    int sum[KXTJ9_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct kxtj9_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;

    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[KXTJ9_AXES_NUM+1];

    /*data*/
    s8                      offset[KXTJ9_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[KXTJ9_AXES_NUM+1];

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver kxtj9_i2c_driver = {
    .driver = {
 //       .owner          = THIS_MODULE,
        .name           = KXTJ9_DEV_NAME,
    },
	.probe      		= kxtj9_i2c_probe,
	.remove    			= kxtj9_i2c_remove,
//	.detect				= kxtj9_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)
    .suspend            = kxtj9_suspend,
    .resume             = kxtj9_resume,
#endif
	.id_table = kxtj9_i2c_id,
//	.address_data = &kxtj9_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *kxtj9_i2c_client = NULL;
static struct kxtj9_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = false;
static GSENSOR_VECTOR3D gsensor_gain, gsensor_offset;


/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)



/*--------------------Gsensor power control function----------------------------------*/
static void KXTJ9_power(struct acc_hw *hw, unsigned int on)
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "KXTJ9"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "KXTJ9"))
			{
				GSE_ERR("power off fail!!\n");
			}
		}
	}
	power_on = on;
}

/*----------------------------------------------------------------------------*/
static int kxtj9_write_calibration(struct kxtj9_i2c_data *obj, s16 dat[KXTJ9_AXES_NUM])
{
    obj->cali_sw[KXTJ9_AXIS_X] = obj->cvt.sign[KXTJ9_AXIS_X]*dat[obj->cvt.map[KXTJ9_AXIS_X]];
    obj->cali_sw[KXTJ9_AXIS_Y] = obj->cvt.sign[KXTJ9_AXIS_Y]*dat[obj->cvt.map[KXTJ9_AXIS_Y]];
    obj->cali_sw[KXTJ9_AXIS_Z] = obj->cvt.sign[KXTJ9_AXIS_Z]*dat[obj->cvt.map[KXTJ9_AXIS_Z]];
    return 0;
}


/*----------------------------------------------------------------------------*/
static int KXTJ9_ResetCalibration(struct i2c_client *client)
{
	struct kxtj9_i2c_data *obj = i2c_get_clientdata(client);

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ9_ReadCalibration(struct i2c_client *client, int dat[KXTJ9_AXES_NUM])
{
    struct kxtj9_i2c_data *obj = i2c_get_clientdata(client);

    dat[obj->cvt.map[KXTJ9_AXIS_X]] = obj->cvt.sign[KXTJ9_AXIS_X]*obj->cali_sw[KXTJ9_AXIS_X];
    dat[obj->cvt.map[KXTJ9_AXIS_Y]] = obj->cvt.sign[KXTJ9_AXIS_Y]*obj->cali_sw[KXTJ9_AXIS_Y];
    dat[obj->cvt.map[KXTJ9_AXIS_Z]] = obj->cvt.sign[KXTJ9_AXIS_Z]*obj->cali_sw[KXTJ9_AXIS_Z];

    return 0;
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int KXTJ9_WriteCalibration(struct i2c_client *client, int dat[KXTJ9_AXES_NUM])
{
	struct kxtj9_i2c_data *obj = i2c_get_clientdata(client);
	int err = 0;
	int cali[KXTJ9_AXES_NUM];


	GSE_FUN();
	if(!obj || ! dat)
	{
		GSE_ERR("null ptr!!\n");
		return -EINVAL;
	}
	else
	{
		s16 cali[KXTJ9_AXES_NUM];
		cali[obj->cvt.map[KXTJ9_AXIS_X]] = obj->cvt.sign[KXTJ9_AXIS_X]*obj->cali_sw[KXTJ9_AXIS_X];
		cali[obj->cvt.map[KXTJ9_AXIS_Y]] = obj->cvt.sign[KXTJ9_AXIS_Y]*obj->cali_sw[KXTJ9_AXIS_Y];
		cali[obj->cvt.map[KXTJ9_AXIS_Z]] = obj->cvt.sign[KXTJ9_AXIS_Z]*obj->cali_sw[KXTJ9_AXIS_Z];
		cali[KXTJ9_AXIS_X] += dat[KXTJ9_AXIS_X];
		cali[KXTJ9_AXIS_Y] += dat[KXTJ9_AXIS_Y];
		cali[KXTJ9_AXIS_Z] += dat[KXTJ9_AXIS_Z];
		return kxtj9_write_calibration(obj, cali);
	}

	return err;
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int KXTJ9_SetPowerMode(struct i2c_client *client, bool enable)
{
	u8 databuf[2];
	int res = 0;
	u8 addr = KXTJ9_REG_CTRL_REG1;
	struct kxtj9_i2c_data *obj = i2c_get_clientdata(client);

	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status is newest!\n");
		return 0;
	}
#if 0
	if(hwmsen_read_block(client, addr, databuf, 0x01))
	{
		GSE_ERR("read power ctl register err!\n");
		return -1;
	}
#endif
        databuf[0] = addr;
        client->addr = (client->addr & I2C_MASK_FLAG) | I2C_WR_FLAG | I2C_RS_FLAG;

        i2c_master_send(client, &databuf[0], ((1<<8)|1));

        client->addr = (client->addr & I2C_MASK_FLAG);


        databuf[0] &= ~(1 << 7);

	if(enable == TRUE)
	{
		databuf[0] |= (1 << 7);
	}
	else
	{
		// do nothing
	}
	databuf[1] = databuf[0];
	databuf[0] = KXTJ9_REG_CTRL_REG1;

        res = i2c_master_send(client, databuf, 0x2);

	if(res < 0)
	{
		GSE_ERR("set power mode failed!\n");
		return -1;
	}
	else if(atomic_read(&obj->trace) & ADX_TRC_INFO)
	{
		GSE_LOG("set power mode ok %d!\n", databuf[1]);
	}

	sensor_power = enable;
        return 0;
}

/*----------------------------------------------------------------------------*/
static int kxtj9_init_hw(struct i2c_client* client)
{
	int err = 0;
	GSE_LOG("sensor test: kxtj9_init_hw function!\n");
	if(!client)
	{
		return -EINVAL;
	}
	if((err = hwmsen_write_byte(client, KXTJ9_REG_CTRL_REG1, 0xc0)))
	{
		GSE_ERR("write KXTJ9_REG_CTRL_REG3 fail!!\n");
		return err;
	}
	if((err = hwmsen_write_byte(client, KXTJ9_REG_CTRL_REG2, 0x00)))
	{
		GSE_ERR("write KXTJ9_REG_CTRL_REG3 fail!!\n");
		return err;
	}
        /*disable all interrupt*/
        if(err = hwmsen_write_byte(client, KXTJ9_REG_INT_CTRL_REG1, 0x00))
        {
            GSE_ERR("write KXTJ9_REG_INT_CTRL_REG2 fail!!\n");
            return err;
        }
	return err;
}

/*----------------------------------------------------------------------------*/
static int KXTJ9_Init_client(struct i2c_client *client, int reset_cali)
{

	gsensor_offset.x = gsensor_offset.y = gsensor_offset.z = 2048;
	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = KXTJ9_SENSITIVITY;

	return kxtj9_init_hw(client);
}
/*----------------------------------------------------------------------------*/
static int KXTJ9_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}

	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "KXTJ9 Chip");
	return 0;
}

/*----------------------------------------------------------------------------*/
static int kxtj9_read_data(struct i2c_client *client, u8 data[KXTJ9_AXES_NUM])
{
	u8 addr = KXTJ9_REG_XOUT;
	int err = 0;
        data[0] = addr;
#if 0
	if(!client)
	{
		err = -EINVAL;
	}
	else if(err = hwmsen_read_block(client, addr, data, 6))
	{
		GSE_ERR("error: %d\n", err);
	}
#endif
         client->addr = (client->addr & I2C_MASK_FLAG) | I2C_WR_FLAG | I2C_RS_FLAG;

         err = i2c_master_send(client, &data[0], ((6<<8)|1));

         client->addr = (client->addr & I2C_MASK_FLAG);

	return 0;
}


/******************************************************************************
 * Functions
******************************************************************************/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int kxtj9_to_standard_format(struct kxtj9_i2c_data *obj, u8 dat[6],
                                    s16 out[KXTJ9_AXES_NUM])
{

	/*convert original KXTJ9 data t*/
    if(dat[KXTJ9_AXIS_X*2+1] & 0x80)
        out[KXTJ9_AXIS_X] = (((dat[KXTJ9_AXIS_X*2] >> 4) & 0x0f) | (dat[KXTJ9_AXIS_X*2 + 1] << 4) - 4096);
    else
        out[KXTJ9_AXIS_X] = (((dat[KXTJ9_AXIS_X*2] >> 4) & 0x0f) | (dat[KXTJ9_AXIS_X*2 + 1] << 4));
    if(dat[KXTJ9_AXIS_Y*2+1] & 0x80)
        out[KXTJ9_AXIS_Y] = (((dat[KXTJ9_AXIS_Y*2] >> 4) & 0x0f) | (dat[KXTJ9_AXIS_Y*2 + 1] << 4) - 4096);
    else
        out[KXTJ9_AXIS_Y] = (((dat[KXTJ9_AXIS_Y*2] >> 4) & 0x0f) | (dat[KXTJ9_AXIS_Y*2 + 1] << 4));
    if(dat[KXTJ9_AXIS_Z*2+1] & 0x80)
        out[KXTJ9_AXIS_Z] = (((dat[KXTJ9_AXIS_Y*2] >> 4) & 0x0f) | (dat[KXTJ9_AXIS_Z*2 + 1] << 4) - 4096);
    else
        out[KXTJ9_AXIS_Z] = (((dat[KXTJ9_AXIS_Y*2] >> 4) & 0x0f) | (dat[KXTJ9_AXIS_Z*2 + 1] << 4));
	return 0;
}


/*----------------------------------------------------------------------------*/
static ssize_t KXTJ9_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{

	struct kxtj9_i2c_data *obj = (struct kxtj9_i2c_data*)i2c_get_clientdata(client);
	s16 output[KXTJ9_AXES_NUM];
	u8 data[6];
	int res = 0;

	if(sensor_power == FALSE)
	{
		res = KXTJ9_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on kxtj9 error %d!\n", res);
		}
	}

	if(!obj || !buf)
	{
		GSE_ERR("null pointer");
		return -EINVAL;
	}


	if((res = kxtj9_read_data(obj->client, data)))
	{
		GSE_ERR("read data failed!!");
		return -EINVAL;
	}

	/*convert to mg: 1000 = 1g*/
	if((res = kxtj9_to_standard_format(obj, data, output)))
	{
		GSE_ERR("convert to standard format fail:%d\n",res);
		return -EINVAL;
	}

	obj->data[KXTJ9_AXIS_X] = output[KXTJ9_AXIS_X] + obj->cali_sw[KXTJ9_AXIS_X];
	obj->data[KXTJ9_AXIS_Y] = output[KXTJ9_AXIS_Y] + obj->cali_sw[KXTJ9_AXIS_Y];
	obj->data[KXTJ9_AXIS_Z] = output[KXTJ9_AXIS_Z] + obj->cali_sw[KXTJ9_AXIS_Z];

	/*remap coordinate*/
	output[obj->cvt.map[KXTJ9_AXIS_X]] = obj->cvt.sign[KXTJ9_AXIS_X]*obj->data[KXTJ9_AXIS_X];
	output[obj->cvt.map[KXTJ9_AXIS_Y]] = obj->cvt.sign[KXTJ9_AXIS_Y]*obj->data[KXTJ9_AXIS_Y];
	output[obj->cvt.map[KXTJ9_AXIS_Z]] = obj->cvt.sign[KXTJ9_AXIS_Z]*obj->data[KXTJ9_AXIS_Z];

	//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[KXTJ9_AXIS_X], acc[KXTJ9_AXIS_Y], acc[KXTJ9_AXIS_Z]);

	//Out put the mg
	output[KXTJ9_AXIS_X] = output[KXTJ9_AXIS_X] * GRAVITY_EARTH_1000 / KXTJ9_SENSITIVITY;
	output[KXTJ9_AXIS_Y] = output[KXTJ9_AXIS_Y] * GRAVITY_EARTH_1000 / KXTJ9_SENSITIVITY;
	output[KXTJ9_AXIS_Z] = output[KXTJ9_AXIS_Z] * GRAVITY_EARTH_1000 / KXTJ9_SENSITIVITY;

	if(atomic_read(&obj->trace) & KXTJ9_TRC_GET_DATA)
	{
		GSE_LOG("%d (0x%08X, 0x%08X, 0x%08X) -> (%5d, %5d, %5d)\n", KXTJ9_SENSITIVITY,
			obj->data[KXTJ9_AXIS_X], obj->data[KXTJ9_AXIS_Y], obj->data[KXTJ9_AXIS_Z],
			output[KXTJ9_AXIS_X],output[KXTJ9_AXIS_Y],output[KXTJ9_AXIS_Z]);
	}

	sprintf(buf, "%04x %04x %04x", output[KXTJ9_AXIS_X], output[KXTJ9_AXIS_Y], output[KXTJ9_AXIS_Z]);
    return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ9_ReadRawData(struct i2c_client *client, char *buf)
{
	char buff[KXTJ9_BUFSIZE];
	s16 data[3];


	KXTJ9_ReadSensorData(client, buff, KXTJ9_BUFSIZE);
	sscanf(buff, "%x %x %x", data[0], data[1],data[2]);

	data[0] = data[0] * KXTJ9_SENSITIVITY / GRAVITY_EARTH_1000;
	data[1] = data[1] * KXTJ9_SENSITIVITY / GRAVITY_EARTH_1000;
	data[2] = data[2] * KXTJ9_SENSITIVITY / GRAVITY_EARTH_1000;

	sprintf(buf, "%04x %04x %04x", data[0],data[1],data[2]);
	return 0;
}


/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj9_i2c_client;
	char strbuf[KXTJ9_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	KXTJ9_ReadChipInfo(client, strbuf, KXTJ9_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}
/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj9_i2c_client;
	char strbuf[KXTJ9_BUFSIZE];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	KXTJ9_ReadSensorData(client, strbuf, KXTJ9_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}
/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj9_i2c_client;
	struct kxtj9_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);

	int err, len = 0;
	int tmp[KXTJ9_AXES_NUM];

	if((err = KXTJ9_ReadCalibration(client, tmp)))
	{
		return -EINVAL;
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1,
			obj->cali_sw[KXTJ9_AXIS_X], obj->cali_sw[KXTJ9_AXIS_Y], obj->cali_sw[KXTJ9_AXIS_Z]);

		return len;
    }
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = kxtj9_i2c_client;
	int err, x, y, z;
	int dat[KXTJ9_AXES_NUM];

	if(!strncmp(buf, "rst", 3))
	{
		if((err = KXTJ9_ResetCalibration(client)))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
		dat[KXTJ9_AXIS_X] = x;
		dat[KXTJ9_AXIS_Y] = y;
		dat[KXTJ9_AXIS_Z] = z;
		if((err = KXTJ9_WriteCalibration(client, dat)))
		{
			GSE_ERR("write calibration err = %d\n", err);
		}
	}
	else
	{
		GSE_ERR("invalid format\n");
	}

	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct kxtj9_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));
	return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct kxtj9_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}

	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	struct kxtj9_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n",
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;
}
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,             S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata,           S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *kxtj9_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
};

/*----------------------------------------------------------------------------*/
static int kxtj9_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(kxtj9_attr_list)/sizeof(kxtj9_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, kxtj9_attr_list[idx])))
		{
			GSE_ERR("driver_create_file (%s) = %d\n", kxtj9_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int kxtj9_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(kxtj9_attr_list)/sizeof(kxtj9_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}


	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, kxtj9_attr_list[idx]);
	}


	return err;
}

/*----------------------------------------------------------------------------*/
static int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	struct kxtj9_i2c_data *priv = (struct kxtj9_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[KXTJ9_BUFSIZE];

	//GSE_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
				{
					GSE_LOG("Gsensor device have updated!\n");
				}
				else
				{
					err = KXTJ9_SetPowerMode( priv->client, !sensor_power);
				}
			}
                        break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gsensor_data = (hwm_sensor_data *)buff_out;
				KXTJ9_ReadSensorData(priv->client, buff, KXTJ9_BUFSIZE);
				sscanf(buff, "%x %x %x", &gsensor_data->values[0],
					&gsensor_data->values[1], &gsensor_data->values[2]);
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				gsensor_data->value_divide = 1000;
			}
			break;
		default:
			GSE_ERR("gsensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}

/******************************************************************************
 * Function Configuration
******************************************************************************/
static int kxtj9_open(struct inode *inode, struct file *file)
{
	file->private_data = kxtj9_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int kxtj9_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int kxtj9_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
static long kxtj9_unlocked_ioctl(struct file *file, unsigned int cmd,
     unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct kxtj9_i2c_data *obj = (struct kxtj9_i2c_data*)i2c_get_clientdata(client);
	char strbuf[KXTJ9_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	long err = 0;
	int cali[3];

	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			KXTJ9_Init_client(client, 0);
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			KXTJ9_ReadChipInfo(client, strbuf, KXTJ9_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			KXTJ9_ReadSensorData(client, strbuf, KXTJ9_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			if(copy_to_user(data, &gsensor_gain, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_OFFSET:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			if(copy_to_user(data, &gsensor_offset, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			KXTJ9_ReadRawData(client, &strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}
			if(atomic_read(&obj->suspend))
			{
				GSE_ERR("Perform calibration in suspend state!!\n");
				err = -EINVAL;
			}
			else
			{
				cali[KXTJ9_AXIS_X] = sensor_data.x * KXTJ9_SENSITIVITY / GRAVITY_EARTH_1000;
				cali[KXTJ9_AXIS_Y] = sensor_data.y * KXTJ9_SENSITIVITY / GRAVITY_EARTH_1000;
				cali[KXTJ9_AXIS_Z] = sensor_data.z * KXTJ9_SENSITIVITY / GRAVITY_EARTH_1000;
				err = KXTJ9_WriteCalibration(client, cali);
			}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = KXTJ9_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			if((err = KXTJ9_ReadCalibration(client, cali)))
			{
				break;
			}

			sensor_data.x = cali[KXTJ9_AXIS_X] * GRAVITY_EARTH_1000 / KXTJ9_SENSITIVITY;
			sensor_data.y = cali[KXTJ9_AXIS_Y] * GRAVITY_EARTH_1000 / KXTJ9_SENSITIVITY;
			sensor_data.z = cali[KXTJ9_AXIS_Z] * GRAVITY_EARTH_1000 / KXTJ9_SENSITIVITY;
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}
			break;


		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;

	}

	return err;
}


/*----------------------------------------------------------------------------*/
static struct file_operations kxtj9_fops = {
//	.owner = THIS_MODULE,
	.open = kxtj9_open,
	.release = kxtj9_release,
	.unlocked_ioctl = kxtj9_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice kxtj9_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &kxtj9_fops,
};
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int kxtj9_suspend(struct i2c_client *client, pm_message_t msg)
{
	struct kxtj9_i2c_data *obj = i2c_get_clientdata(client);
	int err = 0;
	GSE_FUN();

	if(msg.event == PM_EVENT_SUSPEND)
	{
		if(obj == NULL)
		{
			GSE_ERR("null pointer!!\n");
			return -EINVAL;
		}
		atomic_set(&obj->suspend, 1);
		if((err = hwmsen_write_byte(client, KXTJ9_REG_CTRL_REG1, 0x00)))
		{
			GSE_ERR("write power control fail!!\n");
			return err;
		}
		//KXTJ9_power(obj->hw, 0);
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int kxtj9_resume(struct i2c_client *client)
{
	struct kxtj9_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	//KXTJ9_power(obj->hw, 1);
	if((err = KXTJ9_Init_client(client, 0)))
	{
		GSE_ERR("initialize client fail!!\n");
		//return err;
	}
	atomic_set(&obj->suspend, 0);

	return 0;
}
/*----------------------------------------------------------------------------*/
#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void kxtj9_early_suspend(struct early_suspend *h)
{
	struct kxtj9_i2c_data *obj = container_of(h, struct kxtj9_i2c_data, early_drv);
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1);
	if((err = KXTJ9_SetPowerMode(obj->client, false)))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}

	sensor_power = false;

	//KXTJ9_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void kxtj9_late_resume(struct early_suspend *h)
{
	struct kxtj9_i2c_data *obj = container_of(h, struct kxtj9_i2c_data, early_drv);
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

	//KXTJ9_power(obj->hw, 1);
	if((err = KXTJ9_Init_client(obj->client, 0)))
	{
		GSE_ERR("initialize client fail!!\n");
		return;
	}
	atomic_set(&obj->suspend, 0);
}
/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
/*
static int kxtj9_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
	strcpy(info->type, KXTJ9_DEV_NAME);
	return 0;
}
*/
/*----------------------------------------------------------------------------*/
static int kxtj9_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct kxtj9_i2c_data *obj;
	struct hwmsen_object sobj;
	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}

	memset(obj, 0, sizeof(struct kxtj9_i2c_data));

	obj->hw = kxtj9_get_cust_acc_hw();

	if((err = hwmsen_get_convert(obj->hw->direction, &obj->cvt)))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;

	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);

	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);

	kxtj9_i2c_client = new_client;
        //kxtj9_i2c_client->addr = (kxtj9_i2c_client->addr & I2C_MASK_FLAG) | I2C_ENEXT_FLAG;

	if((err = KXTJ9_Init_client(new_client, 1)))
	{
		goto exit_init_failed;
	}


	if((err = misc_register(&kxtj9_device)))
	{
		GSE_ERR("kxtj9_device register failed\n");
		goto exit_misc_device_register_failed;
	}

if(err = kxtj9_create_attr(&(kxtj9_init_info.platform_diver_addr->driver)))
	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	sobj.self = obj;
	sobj.polling = 1;
	sobj.sensor_operate = gsensor_operate;
	if((err = hwmsen_attach(ID_ACCELEROMETER, &sobj)))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_kfree;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = kxtj9_early_suspend,
	obj->early_drv.resume   = kxtj9_late_resume,
	register_early_suspend(&obj->early_drv);
#endif
kxtj9_init_flag = 0;
	GSE_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&kxtj9_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	kxtj9_init_flag = -1;
	GSE_ERR("%s: err = %d\n", __func__, err);
	return err;
}

/*----------------------------------------------------------------------------*/
static int kxtj9_i2c_remove(struct i2c_client *client)
{
	int err = 0;

	if(err = kxtj9_delete_attr(&(kxtj9_init_info.platform_diver_addr->driver)))
	{
		GSE_ERR("kxtj9_delete_attr fail: %d\n", err);
	}

	if((err = misc_deregister(&kxtj9_device)))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}

	if((err = hwmsen_detach(ID_ACCELEROMETER)))


	kxtj9_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int  kxtj9_local_init(void)
{
	struct acc_hw *hw = kxtj9_get_cust_acc_hw();
	GSE_FUN();

	KXTJ9_power(hw, 1);
	if(i2c_add_driver(&kxtj9_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	if(-1 == kxtj9_init_flag)
	{
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int kxtj9_remove()
{

    GSE_FUN();
//    KXTJ9_power(hw, 0);
    i2c_del_driver(&kxtj9_i2c_driver);
    return 0;
}

/*----------------------------------------------------------------------------*/
static int __init kxtj9_init(void)
{
	GSE_FUN();
	i2c_register_board_info(3, &i2c_kxtj9, 1);
	hwmsen_gsensor_add(&kxtj9_init_info);
}
/*----------------------------------------------------------------------------*/
static void __exit kxtj9_exit(void)
{
	GSE_FUN();
}
/*----------------------------------------------------------------------------*/
module_init(kxtj9_init);
module_exit(kxtj9_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KXTJ9 I2C driver");
MODULE_AUTHOR("Chunlei.Wang@mediatek.com");

