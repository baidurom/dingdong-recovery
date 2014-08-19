/* MXC622X motion sensor driver
 *
 *
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

#if 0
#ifdef MT6516
#include <mach/mt6516_devs.h>
#include <mach/mt6516_typedefs.h>
#include <mach/mt6516_gpio.h>
#include <mach/mt6516_pll.h>
#endif

#ifdef MT6573
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_pll.h>
#endif

#ifdef MT6575
#include <mach/mt6575_devs.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_gpio.h>
#include <mach/mt6575_pm_ldo.h>
#endif
#ifdef MT6577
#include <mach/mt6577_devs.h>
#include <mach/mt6577_typedefs.h>
#include <mach/mt6577_gpio.h>
#include <mach/mt6577_pm_ldo.h>
#endif
#endif
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "mxc622x.h"
#include <linux/hwmsen_helper.h>

/*-------------------------MT6516&MT6573 define-------------------------------*/
#define SW_SIMULATE_I2C	1			//if use GPIO simulate I2C, define 1,
									// if use hardware I2C, define 0
#if SW_SIMULATE_I2C
////////////////////////////////////////////////////////////////////////////
#define GPIO_ACCELE_SCL_PIN GPIO175
#define GPIO_ACCELE_SDA_PIN GPIO174

static int accele_gpio_sda = GPIO_ACCELE_SDA_PIN;
static int accele_gpio_scl = GPIO_ACCELE_SCL_PIN;


#define ACC_NDELAY(x)  ndelay((x)*100)

//start setup time, >= 0.6 us, 1.2us
#define	DELAY_LOOP_START_SETUP  12
//start hold time, >= 0.6 us, 1.2us
#define	DELAY_LOOP_START_HOLD  12
//LOW period of SCL, >= 1.3 us, 2.6us
#define	DELAY_LOOP_SCL_LOW  26
//HIGH period of SCL, >= 0.6 us, 1.2us
#define	DELAY_LOOP_SCL_HIGH   12
//data hold time, <= 0.9 us, 1.8us
#define	DELAY_LOOP_DATA_HOLD   18
//data setup time, >= 0.1 us, 2.6 - 1.2 = 1.4us
#define	DELAY_LOOP_DATA_SETUP  (DELAY_LOOP_SCL_LOW - DELAY_LOOP_DATA_HOLD)
//stop setup time, >= 0.6 us, 1.2us
#define	DELAY_LOOP_STOP_SETUP  DELAY_LOOP_START_SETUP
//bus free time between STOP and START, >= 1.3 us, 2.6us
#define	DELAY_LOOP_BUS_FREE  26


#define ACC_SET_I2C_CLK_OUTPUT			mt_set_gpio_dir(accele_gpio_scl,GPIO_DIR_OUT)
#define ACC_SET_I2C_CLK_INPUT			mt_set_gpio_dir(accele_gpio_scl,GPIO_DIR_IN)
#define ACC_SET_I2C_CLK_HIGH			mt_set_gpio_out(accele_gpio_scl,GPIO_OUT_ONE)
#define ACC_SET_I2C_CLK_LOW				mt_set_gpio_out(accele_gpio_scl,GPIO_OUT_ZERO)

#define ACC_SET_I2C_DATA_OUTPUT			mt_set_gpio_dir(accele_gpio_sda,GPIO_DIR_OUT)
#define ACC_SET_I2C_DATA_INPUT			mt_set_gpio_dir(accele_gpio_sda,GPIO_DIR_IN)
#define ACC_SET_I2C_DATA_HIGH			mt_set_gpio_out(accele_gpio_sda,GPIO_OUT_ONE)
#define ACC_SET_I2C_DATA_LOW			mt_set_gpio_out(accele_gpio_sda,GPIO_OUT_ZERO)
#define ACC_GET_I2C_DATA_BIT			mt_get_gpio_in(accele_gpio_sda)


/*
* macro function for i2c start/restart
*/
#define memsic_acc_i2c_start()	\
{	\
	ACC_SET_I2C_DATA_OUTPUT;	\
	ACC_SET_I2C_CLK_OUTPUT; 	\
	ACC_SET_I2C_DATA_HIGH;		\
	ACC_SET_I2C_CLK_HIGH;		\
	ACC_NDELAY(DELAY_LOOP_START_SETUP);	\
	ACC_SET_I2C_DATA_LOW;	\
	ACC_NDELAY(DELAY_LOOP_START_HOLD);	\
	ACC_SET_I2C_CLK_LOW;	\
}

/*
* macro function for i2c write
*/
#define memsic_acc_i2c_write(data)	\
{	\
	kal_int8 i;	\
	ACC_SET_I2C_DATA_OUTPUT;	\
	ACC_SET_I2C_CLK_LOW;	\
	for (i=7;i>=0;i--)	\
	{	\
		ACC_NDELAY(DELAY_LOOP_DATA_HOLD);		\
		if ((data) & (1<<i))	\
		{	\
			ACC_SET_I2C_DATA_HIGH;	\
		}	\
		else	\
		{	\
			ACC_SET_I2C_DATA_LOW;	\
		}	\
		ACC_NDELAY(DELAY_LOOP_DATA_SETUP);		\
		ACC_SET_I2C_CLK_HIGH;	\
		ACC_NDELAY(DELAY_LOOP_SCL_HIGH);		\
		ACC_SET_I2C_CLK_LOW;	\
	}	\
}

/*
* macro function for i2c read
*/
#define memsic_acc_i2c_read(data)	\
{	\
	kal_int8 i;	\
	(data) = 0;	\
	ACC_SET_I2C_CLK_LOW;	\
	ACC_SET_I2C_DATA_INPUT;	\
	for (i=7;i>=0;i--)	\
	{	\
		ACC_NDELAY(DELAY_LOOP_SCL_LOW);		\
		ACC_SET_I2C_CLK_HIGH;	\
		ACC_NDELAY(DELAY_LOOP_SCL_HIGH);		\
		if (ACC_GET_I2C_DATA_BIT)	\
		{	\
			(data) |= (1<<i);	\
		}	\
		ACC_SET_I2C_CLK_LOW;	\
	}	\
}

/*
* macro function for i2c slave acknowledge
*/
#define memsic_acc_i2c_slave_ack(ack)	\
{	\
	ACC_SET_I2C_CLK_LOW;	\
	ACC_SET_I2C_DATA_INPUT;	\
	ACC_NDELAY(DELAY_LOOP_SCL_LOW);		\
	ACC_SET_I2C_CLK_HIGH;	\
	ACC_NDELAY(DELAY_LOOP_SCL_HIGH);		\
	(ack) = ACC_GET_I2C_DATA_BIT;	\
	ACC_SET_I2C_CLK_LOW;	\
}

/*
* macro function for master acknowledge
*/
#define memsic_acc_i2c_master_ack()	\
{	\
	ACC_SET_I2C_CLK_LOW;	\
	ACC_SET_I2C_DATA_OUTPUT;	\
	ACC_NDELAY(DELAY_LOOP_DATA_HOLD);		\
	ACC_SET_I2C_DATA_LOW;	\
	ACC_NDELAY(DELAY_LOOP_DATA_SETUP);		\
	ACC_SET_I2C_CLK_HIGH;	\
	ACC_NDELAY(DELAY_LOOP_SCL_HIGH);		\
	ACC_SET_I2C_CLK_LOW;	\
}

/*
* macro function for master non-acknowledge
*/
#define memsic_acc_i2c_master_nack()	\
{	\
	ACC_SET_I2C_CLK_LOW;	\
	ACC_SET_I2C_DATA_OUTPUT;	\
	ACC_NDELAY(DELAY_LOOP_DATA_HOLD);		\
	ACC_SET_I2C_DATA_HIGH;	\
	ACC_NDELAY(DELAY_LOOP_DATA_SETUP);		\
	ACC_SET_I2C_CLK_HIGH;	\
	ACC_NDELAY(DELAY_LOOP_SCL_HIGH);		\
	ACC_SET_I2C_CLK_LOW;	\
}

/*
* macro function for i2c stop
*/
#define memsic_acc_i2c_stop()	\
{	\
	ACC_SET_I2C_CLK_LOW;	\
	ACC_SET_I2C_DATA_OUTPUT;	\
	ACC_SET_I2C_DATA_LOW;	\
	ACC_NDELAY(DELAY_LOOP_SCL_LOW);		\
	ACC_SET_I2C_CLK_HIGH;	\
	ACC_NDELAY(DELAY_LOOP_STOP_SETUP);		\
	ACC_SET_I2C_DATA_HIGH;	\
}


static int acc_i2c_putbyte(char byte)
{
	char ack = 1;

	memsic_acc_i2c_write(byte);
	memsic_acc_i2c_slave_ack(ack);

	return ack;
}

static char acc_i2c_getbyte(int LastByte)
{
	char data = 0;

	memsic_acc_i2c_read(data);
	//printk("[ -------- ACC MXC622X --------] %s: raw data = %d\n", __FUNCTION__, data);

	if(LastByte == 1){
		memsic_acc_i2c_master_nack();
	}
	else{
		memsic_acc_i2c_master_ack();
	}

	return data;
}

static void acc_i2c_write_bytes(int slave_addr,char *data,int len)
{
	int i;

	mt_set_gpio_mode(accele_gpio_scl,GPIO_MODE_00);
	mt_set_gpio_mode(accele_gpio_sda,GPIO_MODE_00);
	mt_set_gpio_out(accele_gpio_scl,GPIO_OUT_ONE);
	mt_set_gpio_out(accele_gpio_sda,GPIO_OUT_ONE);

	memsic_acc_i2c_start();
	if(acc_i2c_putbyte(slave_addr)){
		printk("[ACCESS: mxc622x] %s put address no ack!\n", __FUNCTION__);
		memsic_acc_i2c_stop();
		return;
	}

	for(i=0;i<len;i++)
	{
		if(acc_i2c_putbyte(data[i])){
			printk("[ACCESS: mxc622x] %s put data no ack!\n", __FUNCTION__);
			memsic_acc_i2c_stop();
			return;
		}
	}

	memsic_acc_i2c_stop();
}

static int acc_i2c_read_byte(int slave_addr, char *cmd, int len)
{
	char data;

	mt_set_gpio_mode(accele_gpio_scl,GPIO_MODE_00);
	mt_set_gpio_mode(accele_gpio_sda,GPIO_MODE_00);
	mt_set_gpio_out(accele_gpio_scl,GPIO_OUT_ONE);
	mt_set_gpio_out(accele_gpio_sda,GPIO_OUT_ONE);

	acc_i2c_write_bytes(slave_addr, cmd, len);

	memsic_acc_i2c_start();
	acc_i2c_putbyte(slave_addr+1);

	data = acc_i2c_getbyte(1); // only read one byte.
	memsic_acc_i2c_stop();

	return data;
}

static int acc_i2c_read_muti_bytes(int slave_addr, char *cmd, int cmdlen, char *data, int datalen)
{
	char tmp = 0;
	int i = 0;

	mt_set_gpio_mode(accele_gpio_scl,GPIO_MODE_00);
	mt_set_gpio_mode(accele_gpio_sda,GPIO_MODE_00);
	mt_set_gpio_out(accele_gpio_scl,GPIO_OUT_ONE);
	mt_set_gpio_out(accele_gpio_sda,GPIO_OUT_ONE);

	acc_i2c_write_bytes(slave_addr, cmd, cmdlen);

	memsic_acc_i2c_start();
	acc_i2c_putbyte(slave_addr+1);

	for(i = 0; i < datalen; i++)
	{
		if(i == (datalen - 1))
			tmp = acc_i2c_getbyte(1);
		else
			tmp = acc_i2c_getbyte(0);

		data[i] = tmp;
	}
	memsic_acc_i2c_stop();

	return i;
}
////////////////////////////////////////////////////////////////////////////
#endif



/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_MXC622X 345
/*----------------------------------------------------------------------------*/
#define DEBUG 1
/*----------------------------------------------------------------------------*/
#define CONFIG_MXC622X_LOWPASS   /*apply low pass filter on output*/
/*----------------------------------------------------------------------------*/
#define MXC622X_AXIS_X          0
#define MXC622X_AXIS_Y          1
#define MXC622X_AXIS_Z          2
#define MXC622X_AXES_NUM        3
#define MXC622X_DATA_LEN        2
#define MXC622X_DEV_NAME        "MXC622X"
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id mxc622x_i2c_id[] = {{MXC622X_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_mxc622x={ I2C_BOARD_INFO("MXC622X", (MXC622X_I2C_SLAVE_ADDR >> 1))};
/*the adapter id will be available in customization*/
//static unsigned short mxc622x_force[] = {0x00, MXC622X_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const mxc622x_forces[] = { mxc622x_force, NULL };
//static struct i2c_client_address_data mxc622x_addr_data = { .forces = mxc622x_forces,};

/*----------------------------------------------------------------------------*/
static int mxc622x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int mxc622x_i2c_remove(struct i2c_client *client);
//static int mxc622x_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int mxc622x_suspend(struct i2c_client *client, pm_message_t msg) ;
static int mxc622x_resume(struct i2c_client *client);

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
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][MXC622X_AXES_NUM];
    int sum[MXC622X_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct mxc622x_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;

    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[MXC622X_AXES_NUM+1];

    /*data*/
    s8                      offset[MXC622X_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    char                     data[MXC622X_AXES_NUM+1];

#if defined(CONFIG_MXC622X_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif
};
/*----------------------------------------------------------------------------*/
#if SW_SIMULATE_I2C
struct early_suspend    acc_early_drv;
#endif
static struct i2c_driver mxc622x_i2c_driver = {
    .driver = {
  //    .owner          = THIS_MODULE,
        .name           = MXC622X_DEV_NAME,
    },
	.probe      		= mxc622x_i2c_probe,
	.remove    			= mxc622x_i2c_remove,
	//.detect				= mxc622x_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)
    .suspend            = mxc622x_suspend,
    .resume             = mxc622x_resume,
#endif
	.id_table = mxc622x_i2c_id,
	//.address_data = &mxc622x_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *mxc622x_i2c_client = NULL;
static struct platform_driver mxc622x_gsensor_driver;
static struct mxc622x_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = false;
static GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0};


/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/
static struct data_resolution mxc622x_data_resolution[] = {
 /*8 combination by {FULL_RES,RANGE}*/
    {{ 3, 9}, 256},   /*+/-2g  in 10-bit resolution:  3.9 mg/LSB*/
    {{ 7, 8}, 128},   /*+/-4g  in 10-bit resolution:  7.8 mg/LSB*/
    {{15, 6},  64},   /*+/-8g  in 10-bit resolution: 15.6 mg/LSB*/
    {{31, 2},  32},   /*+/-16g in 10-bit resolution: 31.2 mg/LSB*/
    {{ 3, 9}, 256},   /*+/-2g  in 10-bit resolution:  3.9 mg/LSB (full-resolution)*/
    {{ 3, 9}, 256},   /*+/-4g  in 11-bit resolution:  3.9 mg/LSB (full-resolution)*/
    {{ 3, 9}, 256},   /*+/-8g  in 12-bit resolution:  3.9 mg/LSB (full-resolution)*/
    {{ 3, 9}, 256},   /*+/-16g in 13-bit resolution:  3.9 mg/LSB (full-resolution)*/
};
/*----------------------------------------------------------------------------*/
static struct data_resolution mxc622x_offset_resolution = {{15, 6}, 64};

/*----------------------------------------------------------------------------*/
static int MXC622X_SetDataResolution(struct mxc622x_i2c_data *obj)
{
	int err;
	u8  dat, reso;

	if((err = hwmsen_read_byte(obj->client, MXC622X_REG_DATA_FORMAT, &dat)))
	{
		GSE_ERR("write data format fail!!\n");
		return err;
	}

	/*the data_reso is combined by 3 bits: {FULL_RES, DATA_RANGE}*/
	reso  = (dat & MXC622X_FULL_RES) ? (0x04) : (0x00);
	reso |= (dat & MXC622X_RANGE_16G);

	if(reso < sizeof(mxc622x_data_resolution)/sizeof(mxc622x_data_resolution[0]))
	{
		obj->reso = &mxc622x_data_resolution[reso];
		return 0;
	}
	else
	{
		return -EINVAL;
	}
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadData(struct i2c_client *client, char data[MXC622X_AXES_NUM])
{
	char addr = MXC622X_REG_DATAX0;
	char buf[MXC622X_DATA_LEN] = {0};
	int err = 0;
#if SW_SIMULATE_I2C
	if((MXC622X_DATA_LEN != acc_i2c_read_muti_bytes(MXC622X_I2C_SLAVE_ADDR, &addr, 1, buf, MXC622X_DATA_LEN)))
	{
		GSE_ERR("error: %d\n", err);
	}
#else
	struct mxc622x_i2c_data *priv = i2c_get_clientdata(client);

	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if(err = hwmsen_read_block(client, addr, buf, MXC622X_DATA_LEN))
	{
		GSE_ERR("error: %d\n", err);
	}
#endif
	else
	{
		data[MXC622X_AXIS_X] = (signed char)(buf[0]);
		data[MXC622X_AXIS_Y] = (signed char)(buf[1]);
		data[MXC622X_AXIS_Z] = 0;
		//printk("[+++++++ ACC MXC622X +++++++++++] %s: data[0] = %d, data[1] = %d\n", __FUNCTION__, buf[0], buf[1]);
	}

	return err;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadOffset(struct i2c_client *client, s8 ofs[MXC622X_AXES_NUM])
{
	int err;

	if((err = hwmsen_read_block(client, MXC622X_REG_OFSX, ofs, MXC622X_AXES_NUM)))
	{
		GSE_ERR("error: %d\n", err);
	}

	return err;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ResetCalibration(struct i2c_client *client)
{
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
	s8 ofs[MXC622X_AXES_NUM] = {0x00, 0x00, 0x00};
	int err;

	if((err = hwmsen_write_block(client, MXC622X_REG_OFSX, ofs, MXC622X_AXES_NUM)))
	{
		GSE_ERR("error: %d\n", err);
	}

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	return err;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadCalibration(struct i2c_client *client, int dat[MXC622X_AXES_NUM])
{
    struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
    int err;
    int mul;

    if ((err = MXC622X_ReadOffset(client, obj->offset))) {
        GSE_ERR("read offset fail, %d\n", err);
        return err;
    }

    mul = obj->reso->sensitivity/mxc622x_offset_resolution.sensitivity;

    dat[obj->cvt.map[MXC622X_AXIS_X]] = obj->cvt.sign[MXC622X_AXIS_X]*(obj->offset[MXC622X_AXIS_X]*mul + obj->cali_sw[MXC622X_AXIS_X]);
    dat[obj->cvt.map[MXC622X_AXIS_Y]] = obj->cvt.sign[MXC622X_AXIS_Y]*(obj->offset[MXC622X_AXIS_Y]*mul + obj->cali_sw[MXC622X_AXIS_Y]);
    dat[obj->cvt.map[MXC622X_AXIS_Z]] = obj->cvt.sign[MXC622X_AXIS_Z]*(obj->offset[MXC622X_AXIS_Z]*mul + obj->cali_sw[MXC622X_AXIS_Z]);

    return 0;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadCalibrationEx(struct i2c_client *client, int act[MXC622X_AXES_NUM], int raw[MXC622X_AXES_NUM])
{
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int mul;

	if((err = MXC622X_ReadOffset(client, obj->offset)))
	{
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	mul = obj->reso->sensitivity/mxc622x_offset_resolution.sensitivity;
	raw[MXC622X_AXIS_X] = obj->offset[MXC622X_AXIS_X]*mul + obj->cali_sw[MXC622X_AXIS_X];
	raw[MXC622X_AXIS_Y] = obj->offset[MXC622X_AXIS_Y]*mul + obj->cali_sw[MXC622X_AXIS_Y];
	raw[MXC622X_AXIS_Z] = obj->offset[MXC622X_AXIS_Z]*mul + obj->cali_sw[MXC622X_AXIS_Z];

	act[obj->cvt.map[MXC622X_AXIS_X]] = obj->cvt.sign[MXC622X_AXIS_X]*raw[MXC622X_AXIS_X];
	act[obj->cvt.map[MXC622X_AXIS_Y]] = obj->cvt.sign[MXC622X_AXIS_Y]*raw[MXC622X_AXIS_Y];
	act[obj->cvt.map[MXC622X_AXIS_Z]] = obj->cvt.sign[MXC622X_AXIS_Z]*raw[MXC622X_AXIS_Z];

	return 0;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_WriteCalibration(struct i2c_client *client, int dat[MXC622X_AXES_NUM])
{
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int cali[MXC622X_AXES_NUM], raw[MXC622X_AXES_NUM];
	int lsb = mxc622x_offset_resolution.sensitivity;
	int divisor = obj->reso->sensitivity/lsb;

	if((err = MXC622X_ReadCalibrationEx(client, cali, raw)))	/*offset will be updated in obj->offset*/
	{
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n",
		raw[MXC622X_AXIS_X], raw[MXC622X_AXIS_Y], raw[MXC622X_AXIS_Z],
		obj->offset[MXC622X_AXIS_X], obj->offset[MXC622X_AXIS_Y], obj->offset[MXC622X_AXIS_Z],
		obj->cali_sw[MXC622X_AXIS_X], obj->cali_sw[MXC622X_AXIS_Y], obj->cali_sw[MXC622X_AXIS_Z]);

	/*calculate the real offset expected by caller*/
	cali[MXC622X_AXIS_X] += dat[MXC622X_AXIS_X];
	cali[MXC622X_AXIS_Y] += dat[MXC622X_AXIS_Y];
	cali[MXC622X_AXIS_Z] += dat[MXC622X_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n",
		dat[MXC622X_AXIS_X], dat[MXC622X_AXIS_Y], dat[MXC622X_AXIS_Z]);

	obj->offset[MXC622X_AXIS_X] = (s8)(obj->cvt.sign[MXC622X_AXIS_X]*(cali[obj->cvt.map[MXC622X_AXIS_X]])/(divisor));
	obj->offset[MXC622X_AXIS_Y] = (s8)(obj->cvt.sign[MXC622X_AXIS_Y]*(cali[obj->cvt.map[MXC622X_AXIS_Y]])/(divisor));
	obj->offset[MXC622X_AXIS_Z] = (s8)(obj->cvt.sign[MXC622X_AXIS_Z]*(cali[obj->cvt.map[MXC622X_AXIS_Z]])/(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[MXC622X_AXIS_X] = obj->cvt.sign[MXC622X_AXIS_X]*(cali[obj->cvt.map[MXC622X_AXIS_X]])%(divisor);
	obj->cali_sw[MXC622X_AXIS_Y] = obj->cvt.sign[MXC622X_AXIS_Y]*(cali[obj->cvt.map[MXC622X_AXIS_Y]])%(divisor);
	obj->cali_sw[MXC622X_AXIS_Z] = obj->cvt.sign[MXC622X_AXIS_Z]*(cali[obj->cvt.map[MXC622X_AXIS_Z]])%(divisor);

	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n",
		obj->offset[MXC622X_AXIS_X]*divisor + obj->cali_sw[MXC622X_AXIS_X],
		obj->offset[MXC622X_AXIS_Y]*divisor + obj->cali_sw[MXC622X_AXIS_Y],
		obj->offset[MXC622X_AXIS_Z]*divisor + obj->cali_sw[MXC622X_AXIS_Z],
		obj->offset[MXC622X_AXIS_X], obj->offset[MXC622X_AXIS_Y], obj->offset[MXC622X_AXIS_Z],
		obj->cali_sw[MXC622X_AXIS_X], obj->cali_sw[MXC622X_AXIS_Y], obj->cali_sw[MXC622X_AXIS_Z]);

	if((err = hwmsen_write_block(obj->client, MXC622X_REG_OFSX, obj->offset, MXC622X_AXES_NUM)))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}

	return err;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[10];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);
	databuf[0] = MXC622X_REG_DEVID;

	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto exit_MXC622X_CheckDeviceID;
	}

	udelay(500);

	databuf[0] = 0x0;
	res = i2c_master_recv(client, databuf, 0x01);
	if(res <= 0)
	{
		goto exit_MXC622X_CheckDeviceID;
	}


	if(databuf[0]!=MXC622X_FIXED_DEVID)
	{
		return MXC622X_ERR_IDENTIFICATION;
	}

	exit_MXC622X_CheckDeviceID:
	if (res <= 0)
	{
		return MXC622X_ERR_I2C;
	}

	return MXC622X_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_SetPowerMode(struct i2c_client *client, bool enable)
{
	char databuf[2]={0};
	int res = 0;
	char addr = MXC622X_REG_CTRL;
#if (!SW_SIMULATE_I2C)
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
#endif

	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status is newest!\n");
		return MXC622X_SUCCESS;
	}

	databuf[1] &= ~MXC622X_MEASURE_MODE;

	if(enable == TRUE)
	{
		databuf[1] |= MXC622X_MEASURE_MODE;
	}
	else
	{
		databuf[1] |= MXC622X_CTRL_PWRDN;
	}

	databuf[0] = addr;

#if SW_SIMULATE_I2C
	acc_i2c_write_bytes(MXC622X_I2C_SLAVE_ADDR, databuf, 2);
#else
	if(err = hwmsen_write_byte(obj->client, addr, databuf[1]))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
#endif
	sensor_power = enable;

	return MXC622X_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[10];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);
	databuf[0] = MXC622X_REG_DATA_FORMAT;
	databuf[1] = dataformat;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MXC622X_ERR_I2C;
	}


	return MXC622X_SetDataResolution(obj);
}
/*----------------------------------------------------------------------------*/
static int MXC622X_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	u8 databuf[10];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);
	databuf[0] = MXC622X_REG_BW_RATE;
	databuf[1] = bwrate;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MXC622X_ERR_I2C;
	}

	return MXC622X_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_SetIntEnable(struct i2c_client *client, u8 intenable)
{
	u8 databuf[10];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);
	databuf[0] = MXC622X_REG_INT_ENABLE;
	databuf[1] = intenable;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MXC622X_ERR_I2C;
	}

	return MXC622X_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int mxc622x_gpio_config(void)
{
   //because we donot use EINT to support low power
   // config to GPIO input mode + PD

 /*   //set to GPIO_GSE_1_EINT_PIN
    mt_set_gpio_mode(GPIO_GSE_1_EINT_PIN, GPIO_GSE_1_EINT_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_GSE_1_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_GSE_1_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_GSE_1_EINT_PIN, GPIO_PULL_DOWN);
    //set to GPIO_GSE_2_EINT_PIN
	mt_set_gpio_mode(GPIO_GSE_2_EINT_PIN, GPIO_GSE_2_EINT_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_GSE_2_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_GSE_2_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_GSE_2_EINT_PIN, GPIO_PULL_DOWN);
	return 0;*/
}
static int mxc622x_init_client(struct i2c_client *client, int reset_cali)
{
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;

	return MXC622X_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
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

	sprintf(buf, "MXC622X Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
#if (!SW_SIMULATE_I2C)
	struct mxc622x_i2c_data *obj =  obj_i2c_data; //(struct mxc622x_i2c_data*)i2c_get_clientdata(client);
	client = obj->client;
#endif
	char databuf[MXC622X_AXES_NUM] = {0};
	int acc[MXC622X_AXES_NUM] = {0};
	int res = 0;

	memset(acc, 0, sizeof(u8)*MXC622X_AXES_NUM);

	if(NULL == buf)
	{
		return -1;
	}
#if (!SW_SIMULATE_I2C)
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}
#endif
	if(sensor_power == FALSE)
	{
#if SW_SIMULATE_I2C
		res = MXC622X_SetPowerMode(NULL, true);
#else
		res = MXC622X_SetPowerMode(client, true);
#endif
		if(res)
		{
			GSE_ERR("Power on mxc622x error %d!\n", res);
		}
		msleep(20);
	}


#if SW_SIMULATE_I2C
	if((MXC622X_ReadData(NULL, databuf)))
	{
		printk("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
		//Out put the mg
#if defined(VANZO_ACC_BOTTOM)
		acc[MXC622X_AXIS_X] = -((signed char)databuf[MXC622X_AXIS_X] * GRAVITY_EARTH_1000 / 64);
#else
		acc[MXC622X_AXIS_X] = (signed char)databuf[MXC622X_AXIS_X] * GRAVITY_EARTH_1000 / 64;
#endif
		acc[MXC622X_AXIS_Y] = (signed char)databuf[MXC622X_AXIS_Y] * GRAVITY_EARTH_1000 / 64;
		acc[MXC622X_AXIS_Z] = 32 * GRAVITY_EARTH_1000 / 64;
#else
	if(MXC622X_ReadData(client, obj->data))
	{
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
		/*remap coordinate*/
		acc[obj->cvt.map[MXC622X_AXIS_X]] = obj->cvt.sign[MXC622X_AXIS_X]*obj->data[MXC622X_AXIS_X];
		acc[obj->cvt.map[MXC622X_AXIS_Y]] = obj->cvt.sign[MXC622X_AXIS_Y]*obj->data[MXC622X_AXIS_Y];
		acc[obj->cvt.map[MXC622X_AXIS_Z]] = obj->cvt.sign[MXC622X_AXIS_Z]*obj->data[MXC622X_AXIS_Z];
		//printk("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[MXC622X_AXIS_X],obj->cvt.sign[MXC622X_AXIS_Y]);


		//GSE_LOG("Mapped gsensor data: %d, %d!\n", acc[MXC622X_AXIS_X], acc[MXC622X_AXIS_Y]);
		//Out put the mg
		acc[MXC622X_AXIS_X] = (signed char)acc[MXC622X_AXIS_X] * GRAVITY_EARTH_1000 / 64;
		acc[MXC622X_AXIS_Y] = (signed char)acc[MXC622X_AXIS_Y] * GRAVITY_EARTH_1000 / 64;
		acc[MXC622X_AXIS_Z] = 32 * GRAVITY_EARTH_1000 / 64;
#endif

	//	printk("acc : %d, %d, %d\n", acc[0], acc[1], acc[2]);
		sprintf(buf, "%04x %04x %04x", acc[MXC622X_AXIS_X], acc[MXC622X_AXIS_Y], acc[MXC622X_AXIS_Z]);
		/*
		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}*/
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static int MXC622X_ReadRawData(struct i2c_client *client, char *buf)
{
	struct mxc622x_i2c_data *obj = (struct mxc622x_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}

	if((res = MXC622X_ReadData(client, obj->data)))
	{
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "%04x %04x %04x", obj->data[MXC622X_AXIS_X],
			obj->data[MXC622X_AXIS_Y], obj->data[MXC622X_AXIS_Z]);

	}

	return 0;
}
/*----------------------------------------------------------------------------*/
int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value, sample_delay;
	struct mxc622x_i2c_data *priv = (struct mxc622x_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[MXC622X_BUFSIZE];

	//GSE_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
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
#if SW_SIMULATE_I2C
					err = MXC622X_SetPowerMode(NULL, !sensor_power);
#else
					err = MXC622X_SetPowerMode(priv->client, !sensor_power);
#endif
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
#if SW_SIMULATE_I2C
				err = MXC622X_ReadSensorData(NULL, buff, MXC622X_BUFSIZE);
#else
				err = MXC622X_ReadSensorData(priv->client, buff, MXC622X_BUFSIZE);
#endif
				gsensor_data->values[0]=gsensor_data->values[1]=gsensor_data->values[2] = 0;
				sscanf(buff, "%x %x %x", &gsensor_data->values[0], &gsensor_data->values[1], &gsensor_data->values[2]);
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				gsensor_data->value_divide = 1000;
				//printk("X :%d,Y: %d, Z:%d\n",gsensor_data->values[0],gsensor_data->values[1],gsensor_data->values[2]);
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
static int mxc622x_open(struct inode *inode, struct file *file)
{
/*	file->private_data = mxc622x_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
*/
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int mxc622x_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mxc622x_unlocked_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
       unsigned long arg)
{
//	struct i2c_client *client = (struct i2c_client*)file->private_data;
//	struct mxc622x_i2c_data *obj = NULL;
	char strbuf[MXC622X_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	int err = 0;
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
			//mxc622x_init_client(client, 0);
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			MXC622X_ReadChipInfo(NULL, strbuf, MXC622X_BUFSIZE);
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

			MXC622X_ReadSensorData(NULL, strbuf, MXC622X_BUFSIZE);
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

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			MXC622X_ReadRawData(NULL, strbuf);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
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
		//	if(atomic_read(&obj->suspend))
		//	{
		//		GSE_ERR("Perform calibration in suspend state!!\n");
		//		err = -EINVAL;
		//	}
		//	else
		//	{
		//		cali[MXC622X_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
		//		cali[MXC622X_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
		//		cali[MXC622X_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;
		//		err = MXC622X_WriteCalibration(NULL, cali);
		//	}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = MXC622X_ResetCalibration(NULL);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			if((err = MXC622X_ReadCalibration(NULL, cali)))
			{
				break;
			}

		//	sensor_data.x = cali[MXC622X_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		//	sensor_data.y = cali[MXC622X_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		//	sensor_data.z = cali[MXC622X_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
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
static struct file_operations mxc622x_fops = {
	//.owner = THIS_MODULE,
	.open = mxc622x_open,
	.release = mxc622x_release,
	.unlocked_ioctl = mxc622x_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice mxc622x_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &mxc622x_fops,
};
/*----------------------------------------------------------------------------*/
//#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int mxc622x_suspend(struct i2c_client *client, pm_message_t msg)
{
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
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

		if((err = MXC622X_SetPowerMode(obj->client, false)))
		{
			GSE_ERR("write power control fail!!\n");
			return err;
		}
		//MXC622X_power(obj->hw, 0);
		GSE_LOG("mxc622x_suspend ok\n");
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int mxc622x_resume(struct i2c_client *client)
{
	struct mxc622x_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	//MXC622X_power(obj->hw, 1);
	if((err = mxc622x_init_client(client, 0)))
	{
		GSE_ERR("initialize client fail!!\n");
		//return err;
	}
	atomic_set(&obj->suspend, 0);
	GSE_LOG("mxc622x_resume ok\n");

	return 0;
}
/*----------------------------------------------------------------------------*/
//#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void mxc622x_early_suspend(struct early_suspend *h)
{
 	int err;
#if (!SW_SIMULATE_I2C)
	struct mxc622x_i2c_data *obj = container_of(h, struct mxc622x_i2c_data, early_drv);
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1);
	if((err = MXC622X_SetPowerMode(obj->client, false)))
#else
	if((err = MXC622X_SetPowerMode(NULL, false)))
#endif
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}

	sensor_power = false;

	//MXC622X_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void mxc622x_late_resume(struct early_suspend *h)
{
	int err;
#if (!SW_SIMULATE_I2C)
	struct mxc622x_i2c_data *obj = container_of(h, struct mxc622x_i2c_data, early_drv);
	if((err = MXC622X_SetPowerMode(obj->client, true)))
#else
	if((err = MXC622X_SetPowerMode(NULL, true)))
#endif
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}

	sensor_power = true;
}
/*----------------------------------------------------------------------------*/
//#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
/*
static int mxc622x_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
	strcpy(info->type, MXC622X_DEV_NAME);
	return 0;
}
*/
/*----------------------------------------------------------------------------*/
static int mxc622x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
//	struct i2c_client *new_client;
	struct mxc622x_i2c_data *obj;
	struct hwmsen_object sobj;
	int err = 0;
//#if (!SW_SIMULATE_I2C)
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}

	memset(obj, 0, sizeof(struct mxc622x_i2c_data));

	obj->hw = get_cust_acc_hw();

	if((err = hwmsen_get_convert(obj->hw->direction, &obj->cvt)))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;
/*
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
*/
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);

#ifdef CONFIG_MXC622X_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}

	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}

#endif
/*
	mxc622x_i2c_client = new_client;

	if((err = mxc622x_init_client(new_client, 1)))
	{
		goto exit_init_failed;
	}
*/
//#endif

	if((err = misc_register(&mxc622x_device)))
	{
		GSE_ERR("mxc622x_device register failed\n");
		goto exit_misc_device_register_failed;
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
#if SW_SIMULATE_I2C
	acc_early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	acc_early_drv.suspend  = mxc622x_early_suspend,
	acc_early_drv.resume   = mxc622x_late_resume,
	register_early_suspend(&acc_early_drv);
#else
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = mxc622x_early_suspend,
	obj->early_drv.resume   = mxc622x_late_resume,
	register_early_suspend(&obj->early_drv);
#endif
#endif

	GSE_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&mxc622x_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
#if (!SW_SIMULATE_I2C)
	kfree(obj);
#endif
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);
	return err;
}

/*----------------------------------------------------------------------------*/
static int mxc622x_i2c_remove(struct i2c_client *client)
{
	int err = 0;
#if (!SW_SIMULATE_I2C)
	if((err = mxc622x_delete_attr(&mxc622x_gsensor_driver.driver)))
	{
		GSE_ERR("mxc622x_delete_attr fail: %d\n", err);
	}
#endif
	if((err = misc_deregister(&mxc622x_device)))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}

	if((err = hwmsen_detach(ID_ACCELEROMETER)))


	mxc622x_i2c_client = NULL;
#if (!SW_SIMULATE_I2C)
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mxc622x_probe(struct platform_device *pdev)
{
#if SW_SIMULATE_I2C
	mxc622x_i2c_probe(NULL, NULL);
#else
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();

	//MXC622X_power(hw, 1);
	//mxc622x_force[0] = hw->i2c_num;
	if(i2c_add_driver(&mxc622x_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mxc622x_remove(struct platform_device *pdev)
{
#if (!SW_SIMULATE_I2C)
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();
   // MXC622X_power(hw, 0);
    i2c_del_driver(&mxc622x_i2c_driver);
#endif
    return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver mxc622x_gsensor_driver = {
	.probe      = mxc622x_probe,
	.remove     = mxc622x_remove,
	.driver     = {
		.name  = "gsensor",
	//	.owner = THIS_MODULE,
	}
};

/*----------------------------------------------------------------------------*/
static int __init mxc622x_init(void)
{
	struct acc_hw* hw = get_cust_acc_hw();
	GSE_FUN();
	i2c_register_board_info(hw->i2c_num, &i2c_mxc622x, 1);
	if(platform_driver_register(&mxc622x_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit mxc622x_exit(void)
{
	GSE_FUN();
	platform_driver_unregister(&mxc622x_gsensor_driver);
}
/*----------------------------------------------------------------------------*/
module_init(mxc622x_init);
module_exit(mxc622x_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MXC622X I2C driver");
MODULE_AUTHOR("");
