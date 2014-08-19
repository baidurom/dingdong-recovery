/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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


#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>

#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "tpd_custom_gt818_818x_868.h"
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "tpd.h"
#include <cust_eint.h>
#include <linux/jiffies.h>

#ifndef TPD_NO_GPIO
#include "cust_gpio_usage.h"
#endif

extern struct tpd_device *tpd;

static int tpd_flag = 0;
static int tpd_halt = 0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

#ifdef TPD_HAVE_TOUCH_KEY
const u16 touch_key_array[] = { KEY_MENU, KEY_HOME, KEY_BACK, KEY_SEARCH };

#define TPD_TOUCH_KEY_NUM ( sizeof( touch_key_array )/sizeof( touch_key_array[0] ) )
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT] = TPD_WARP_END;
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static void gtp_reset_guitar(s32 ms);
static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client,
			  struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
				     kal_bool ACT_Polarity,
				     void (EINT_FUNC_PTR) (void),
				     kal_bool auto_umask);

#ifdef TPD_CREATE_WR_NODE
extern s32 init_wr_node(struct i2c_client *);
extern void uninit_wr_node(void);
#endif

extern s32 gt818_downloader(struct i2c_client *client);
extern s32 gt818x_868_downloader(struct i2c_client *client);
#define TPD_RESET_ISSUE_WORKAROUND
#define TPD_MAX_RESET_COUNT 3

#ifdef TPD_ESD_PROTECT
#define TPD_ESD_CHECK_CIRCLE        2000
static struct delayed_work gtp_esd_check_work;
static struct workqueue_struct *gtp_esd_check_workqueue = NULL;
static void gtp_esd_check_func(struct work_struct *);
#endif

#define TPD_CONFIG_REG_BASE           0x6A2
#define TPD_FREQ_CAL_RESULT           0x70F
#define TPD_SENSOR_ID_REG             0x710
#ifdef TOUCH_PS
#define NOT_ONLY_LARGETOUCH		0
#define TPD_NORMAL_LARGE_TOUCH_REG			0x06EA
#define TPD_PROXIMITY_REG        			0x071D
#define TPD_REFRESH_REG						0x070B
#endif
#define TPD_TOUCH_INFO_REG_BASE       0x712
#define TPD_POINT_INFO_REG_BASE       0x722
#define TPD_VERSION_INFO_REG          0x713
#define TPD_VERSION_BASIC_REG         0x717
#define TPD_KEY_INFO_REG_BASE         0x721
#define TPD_POWER_MODE_REG            0x692
#define TPD_HANDSHAKING_START_REG     0xFFF
#define TPD_HANDSHAKING_END_REG       0x8000
#define TPD_FREQ_REG                  0x1522
#define TPD_SOFT_RESET_MODE           0x01
#define TPD_POINT_INFO_LEN            8
#define TPD_MAX_POINTS                5
#define MAX_TRANSACTION_LENGTH        8
#define I2C_DEVICE_ADDRESS_LEN        2
#define I2C_MASTER_CLOCK              300


extern kal_bool upmu_is_chr_det(void);

#define MAX_I2C_TRANSFER_SIZE (MAX_TRANSACTION_LENGTH - I2C_DEVICE_ADDRESS_LEN)

#define GT818_CONFIG_PROC_FILE "gt818_config"
#define CONFIG_LEN (106)

struct tpd_info_t {
	u8 vendor_id_1;
	u8 vendor_id_2;
	u8 product_id_1;
	u8 product_id_2;
	u8 version_1;
	u8 version_2;
};

static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = { {"gt818b", 0}, {} };
static unsigned short force[] =
    { 0, 0xBA, I2C_CLIENT_END, I2C_CLIENT_END };
static const unsigned short *const forces[] = { force, NULL };

//static struct i2c_client_address_data addr_data = { .forces = forces,};
static struct i2c_board_info __initdata i2c_tpd =
    { I2C_BOARD_INFO("gt818b", (0xBA >> 1)) };
static struct i2c_driver tpd_i2c_driver = {
	.probe = tpd_i2c_probe,
	.remove = tpd_i2c_remove,
	.detect = tpd_i2c_detect,
	.driver.name = "gt818b",
	.id_table = tpd_i2c_id,
	.address_list = (const unsigned short *) forces,
};

struct tpd_info_t tpd_info;
u8 *cfg_data = NULL;
u8 *cfg_data_with_charger = NULL;
u8 chip_type = 0;
u8 gt868_div_2 = 0;

/* proc file system */
static int i2c_read_bytes(struct i2c_client *client, u16 addr, u8 * rxbuf,
			  int len);
static int i2c_write_bytes(struct i2c_client *client, u16 addr, u8 * txbuf,
			   int len);
static int i2c_write_dummy(struct i2c_client *client, u16 addr);
static struct proc_dir_entry *gt818_config_proc = NULL;

#define VELOCITY_CUSTOM_GT818B
#ifdef VELOCITY_CUSTOM_GT818B
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#ifndef TPD_VELOCITY_CUSTOM_X
#define TPD_VELOCITY_CUSTOM_X 10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
#define TPD_VELOCITY_CUSTOM_Y 10
#endif

// for magnify velocity********************************************
#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

int g_v_magnify_x = TPD_VELOCITY_CUSTOM_X;
int g_v_magnify_y = TPD_VELOCITY_CUSTOM_Y;
static int tpd_misc_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int tpd_misc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
			       unsigned long arg)
{
	void __user *data;

	long err = 0;

	if (_IOC_DIR(cmd) & _IOC_READ) {
		err =
		    !access_ok(VERIFY_WRITE, (void __user *) arg,
			       _IOC_SIZE(cmd));
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		err =
		    !access_ok(VERIFY_READ, (void __user *) arg,
			       _IOC_SIZE(cmd));
	}

	if (err) {
		printk("tpd: access error: %08X, (%2d, %2d)\n", cmd,
		       _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch (cmd) {
	case TPD_GET_VELOCITY_CUSTOM_X:
		data = (void __user *) arg;

		if (data == NULL) {
			err = -EINVAL;
			break;
		}

		if (copy_to_user
		    (data, &g_v_magnify_x, sizeof(g_v_magnify_x))) {
			err = -EFAULT;
			break;
		}

		break;

	case TPD_GET_VELOCITY_CUSTOM_Y:
		data = (void __user *) arg;

		if (data == NULL) {
			err = -EINVAL;
			break;
		}

		if (copy_to_user
		    (data, &g_v_magnify_y, sizeof(g_v_magnify_y))) {
			err = -EFAULT;
			break;
		}

		break;

	default:
		printk("tpd: unknown IOCTL: 0x%08x\n", cmd);
		err = -ENOIOCTLCMD;
		break;

	}

	return err;
}


static struct file_operations tpd_fops = {
//      .owner = THIS_MODULE,
	.open = tpd_misc_open,
	.release = tpd_misc_release,
	.unlocked_ioctl = tpd_unlocked_ioctl,
};

/*----------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch",
	.fops = &tpd_fops,
};

//**********************************************
#endif

#ifdef TOUCH_PS
extern int touch_set_ps(bool val);
static bool PS_STATUS = 0;

static void gt818_ps_enable(bool val)
{
	int ret = -1;
#if NOT_ONLY_LARGETOUCH
	u8 state;
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);
	ret = i2c_read_bytes(i2c_client, TPD_POWER_MODE_REG, &state, 1);

	if (val) {
		state |= 0x80;
	} else {
		state &= 0x7f;
	}
	ret = i2c_write_bytes(i2c_client, TPD_POWER_MODE_REG, &state, 1);
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
	PS_STATUS = val;
#else
	u8 config_refresh = 1;
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);

	if (val) {
		i2c_write_bytes(i2c_client, TPD_NORMAL_LARGE_TOUCH_REG,
				&cfg_data[104], 1);
	} else {
		i2c_write_bytes(i2c_client, TPD_NORMAL_LARGE_TOUCH_REG,
				&cfg_data[72], 1);
	}
	ret =
	    i2c_write_bytes(i2c_client, TPD_REFRESH_REG, &config_refresh,
			    1);
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
	PS_STATUS = val;
#endif
	return 0;
}
#endif

static int gt818_config_read_proc(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	char *ptr = page;
	char temp_data[CONFIG_LEN] = { 0 };
	int i;

	ptr += sprintf(ptr, "==== GT818 config init value====\n");

	for (i = 0; i < CONFIG_LEN; i++) {
		ptr += sprintf(ptr, "0x%02X ", cfg_data[i]);

		if (i % 8 == 7)
			ptr += sprintf(ptr, "\n");
	}

	ptr += sprintf(ptr, "\n");
	ptr += sprintf(ptr, "==== GT818 charger init config ====\n");

	for (i = 0; i < CONFIG_LEN; i++) {
		ptr += sprintf(ptr, "0x%02X ", cfg_data_with_charger[i]);

		if (i % 8 == 7)
			ptr += sprintf(ptr, "\n");
	}

	ptr += sprintf(ptr, "\n");
	ptr += sprintf(ptr, "==== GT818 config real value====\n");
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);
	i2c_read_bytes(i2c_client, TPD_CONFIG_REG_BASE, temp_data,
		       CONFIG_LEN);
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);

	for (i = 0; i < CONFIG_LEN; i++) {
		ptr += sprintf(ptr, "0x%02X ", temp_data[i]);

		if (i % 8 == 7)
			ptr += sprintf(ptr, "\n");
	}

	ptr += sprintf(ptr, "\n");
	*eof = 1;
	return (ptr - page);
}

static int gt818_config_write_proc(struct file *file, const char *buffer,
				   unsigned long count, void *data)
{
	kal_bool temp = upmu_is_chr_det();

	TPD_DEBUG("write count %ld\n", count);

	if (count != (CONFIG_LEN * 2)) {
		TPD_DEBUG("size not match [%d:%ld]\n", CONFIG_LEN * 2,
			  count);
		return -EFAULT;
	}

	if (copy_from_user(cfg_data, buffer, count / 2)) {
		TPD_DEBUG("copy from user fail\n");
		return -EFAULT;
	}

	if (copy_from_user
	    (cfg_data_with_charger, buffer + CONFIG_LEN, count / 2)) {
		TPD_DEBUG("copy from user fail\n");
		return -EFAULT;
	}

	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);

	if (temp)
		i2c_write_bytes(i2c_client, TPD_CONFIG_REG_BASE,
				cfg_data_with_charger, CONFIG_LEN);
	else
		i2c_write_bytes(i2c_client, TPD_CONFIG_REG_BASE, cfg_data,
				CONFIG_LEN);

	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
	return count;
}

static int i2c_read_bytes(struct i2c_client *client, u16 addr, u8 * rxbuf,
			  int len)
{
	u8 buffer[I2C_DEVICE_ADDRESS_LEN];
	u8 retry;
	u16 left = len;
	u16 offset = 0;

	struct i2c_msg msg[2] = {
		{
		 //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
		 .addr = ((client->addr & I2C_MASK_FLAG)),
		 .flags = 0,
		 .buf = buffer,
		 .len = I2C_DEVICE_ADDRESS_LEN,
		 .timing = I2C_MASTER_CLOCK},
		{
		 //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
		 .addr = ((client->addr & I2C_MASK_FLAG)),
		 .flags = I2C_M_RD,
		 .timing = I2C_MASTER_CLOCK},
	};

	if (rxbuf == NULL)
		return -1;

	TPD_DEBUG("i2c_read_bytes to device %02X address %04X len %d\n",
		  client->addr, addr, len);

	while (left > 0) {
		buffer[0] = ((addr + offset) >> 8) & 0xFF;
		buffer[1] = (addr + offset) & 0xFF;

		msg[1].buf = &rxbuf[offset];

		if (left > MAX_TRANSACTION_LENGTH) {
			msg[1].len = MAX_TRANSACTION_LENGTH;
			left -= MAX_TRANSACTION_LENGTH;
			offset += MAX_TRANSACTION_LENGTH;
		} else {
			msg[1].len = left;
			left = 0;
		}

		retry = 0;

		while (i2c_transfer(client->adapter, &msg[0], 2) != 2) {
			retry++;

			if (retry == 20) {
				TPD_DEBUG
				    ("I2C read 0x%X length=%d failed\n",
				     addr + offset, len);
				TPD_DMESG
				    ("I2C read 0x%X length=%d failed\n",
				     addr + offset, len);
				return -1;
			}
		}
	}

	return 0;
}

static int i2c_write_bytes(struct i2c_client *client, u16 addr, u8 * txbuf,
			   int len)
{
	u8 buffer[MAX_TRANSACTION_LENGTH];
	u16 left = len;
	u16 offset = 0;
	u8 retry = 0;

	struct i2c_msg msg = {
		//.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
		.addr = ((client->addr & I2C_MASK_FLAG)),
		.flags = 0,
		.buf = buffer,
		.timing = I2C_MASTER_CLOCK,
	};


	if (txbuf == NULL)
		return -1;

	TPD_DEBUG("i2c_write_bytes to device %02X address %04X len %d\n",
		  client->addr, addr, len);

	while (left > 0) {
		retry = 0;

		buffer[0] = ((addr + offset) >> 8) & 0xFF;
		buffer[1] = (addr + offset) & 0xFF;

		if (left > MAX_I2C_TRANSFER_SIZE) {
			memcpy(&buffer[I2C_DEVICE_ADDRESS_LEN],
			       &txbuf[offset], MAX_I2C_TRANSFER_SIZE);
			msg.len = MAX_TRANSACTION_LENGTH;
			left -= MAX_I2C_TRANSFER_SIZE;
			offset += MAX_I2C_TRANSFER_SIZE;
		} else {
			memcpy(&buffer[I2C_DEVICE_ADDRESS_LEN],
			       &txbuf[offset], left);
			msg.len = left + I2C_DEVICE_ADDRESS_LEN;
			left = 0;
		}

		TPD_DEBUG("byte left %d offset %d\n", left, offset);

		while (i2c_transfer(client->adapter, &msg, 1) != 1) {
			retry++;

			if (retry == 20) {
				TPD_DEBUG
				    ("I2C write 0x%X%X length=%d failed\n",
				     buffer[0], buffer[1], len);
				TPD_DMESG
				    ("I2C write 0x%X%X length=%d failed\n",
				     buffer[0], buffer[1], len);
				return -1;
			} else
				TPD_DEBUG
				    ("I2C write retry %d addr 0x%X%X\n",
				     retry, buffer[0], buffer[1]);

		}
	}

	return 0;
}

static int i2c_write_dummy(struct i2c_client *client, u16 addr)
{
	u8 buffer[MAX_TRANSACTION_LENGTH];
	s8 ret = -1;

	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.buf = buffer,
		.timing = I2C_MASTER_CLOCK,
		.len = 2
	};

	TPD_DEBUG("i2c_write_dummy to device %02X address %04X\n",
		  client->addr, addr);

	buffer[0] = (addr >> 8) & 0xFF;
	buffer[1] = (addr) & 0xFF;

	ret = i2c_transfer(client->adapter, &msg, 1);

	return ret;
}

/*******************************************************
Function:
	GTP read sensor id function.
Input:
	client:	i2c client private struct.
Output:
	sensor ID.
*******************************************************/
static u8 gtp_get_sensor_id(struct i2c_client *client)
{
	u8 buf[8] = { 0 };
	u8 sensor_id = 0;
	u8 i = 0;
	u8 count = 0;

	// step 1: setup sensorID port as input
	i2c_read_bytes(client, 0x1600, buf, 1);
	buf[0] &= 0xfd;
	i2c_write_bytes(client, 0x1600, buf, 1);
	// step2: setup SensorID as pullup, shutdown SensorID pulldown
	i2c_read_bytes(client, 0x1606, buf, 2);
	buf[0] |= 0x02;
	buf[1] &= 0xfd;
	i2c_write_bytes(client, 0x1606, buf, 2);
	msleep(1);
	// step3: read 0x1602, result and 0x02, test equal 0, repeat 200 times
	count = 0;
	for (i = 0; i < 200; i++) {
		i2c_read_bytes(client, 0x1602, buf, 1);
		buf[0] &= 0x02;
		if (buf[0] == 0) {
			++count;
		}
	}
	// if count greater than 100, then assign sensorid as 2
	if (count >= 100) {
		TPD_DMESG("count = %d", count);
		sensor_id = 2;
		goto SENSOR_ID_NONC;
	}
	// step4: setup SensorID as pulldown, shutdown SensorID pullup
	i2c_read_bytes(client, 0x1606, buf, 2);
	buf[0] &= 0xfd;
	buf[1] |= 0x02;
	i2c_write_bytes(client, 0x1606, buf, 2);
	msleep(1);
	count = 0;
	// step 5: do the same as step 3
	for (i = 0; i < 200; ++i) {
		i2c_read_bytes(client, 0x1602, buf, 1);
		buf[0] &= 0x02;
		if (buf[0] != 0) {
			++count;
		}
	}
	if (count >= 100) {
		TPD_DMESG("count = %d", count);
		sensor_id = 1;
		goto SENSOR_ID_NONC;
	}
	sensor_id = 0;
	goto SENSOR_ID_NC;
      SENSOR_ID_NONC:
	i2c_read_bytes(client, 0x1606, buf, 2);
	buf[0] &= 0xfd;
	buf[1] &= 0xfd;
	i2c_write_bytes(client, 0x1606, buf, 2);
      SENSOR_ID_NC:
	return sensor_id;
}

static int tpd_i2c_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	strcpy(info->type, "mtk-tpd");
	return 0;
}

static int tpd_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	char int_type = 0;
#ifdef TPD_RESET_ISSUE_WORKAROUND
	int reset_count = 0;

      reset_proc:
#ifdef TPD_POWER_SOURCE_CUSTOM
        hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
        hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif

	// set INT mode
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);

	// reset
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(1);

	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);

#else
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
	msleep(10);
	// set deep sleep off
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(10);
#endif

	memset(&tpd_info, 0, sizeof(struct tpd_info_t));
	i2c_write_dummy(client, TPD_HANDSHAKING_START_REG);
	err =
	    i2c_read_bytes(client, TPD_VERSION_INFO_REG, (u8 *) & tpd_info,
			   sizeof(struct tpd_info_t));
	i2c_write_dummy(client, TPD_HANDSHAKING_END_REG);

	if (err) {
		TPD_DMESG(TPD_DEVICE " fail to get tpd info %d\n", err);
#ifdef TPD_RESET_ISSUE_WORKAROUND

		if (reset_count < TPD_MAX_RESET_COUNT) {
			reset_count++;
			goto reset_proc;
		}
#endif
		return err;
	} else {
		TPD_DMESG("TPD info\n");
		TPD_DMESG("vendor %02X %02X\n", tpd_info.vendor_id_1,
			  tpd_info.vendor_id_2);
		TPD_DMESG("product %02X %02X\n", tpd_info.product_id_1,
			  tpd_info.product_id_2);
		TPD_DMESG("version %02X %02X\n", tpd_info.version_1,
			  tpd_info.version_2);

		if (tpd_info.product_id_2 == 0x18) {
			if (tpd_info.product_id_1 < 0xF0) {
				TPD_DMESG("Chip_type: GT818.\n");
				chip_type = GT818;
			} else {
				TPD_DMESG("Chip_type: GT818X.\n");
				chip_type = GT818X;
			}
		} else if (tpd_info.product_id_2 == 0x68) {
			chip_type = GT868;
		}
#ifdef TPD_AUTOUPDATE_FIRMWARE

		if (chip_type == GT818) {
			TPD_DMESG("GT818 update.\n");
			gt818_downloader(client);
		} else {
			TPD_DMESG("%s update.\n",
				  (chip_type ==
				   GT818X) ? "GT818X" : "GT868");
			gt818x_868_downloader(client);
		}
#endif
	}

#ifdef VELOCITY_CUSTOM_GT818B

	if ((err = misc_register(&tpd_misc_device))) {
		printk("mtk_tpd: tpd_misc_device register failed\n");
	}
#endif

	i2c_client = client;

	// Create proc file system
	gt818_config_proc =
	    create_proc_entry(GT818_CONFIG_PROC_FILE, 0666, NULL);

	if (gt818_config_proc == NULL) {
		TPD_DEBUG("create_proc_entry %s failed\n",
			  GT818_CONFIG_PROC_FILE);
	} else {
		gt818_config_proc->read_proc = gt818_config_read_proc;
		gt818_config_proc->write_proc = gt818_config_write_proc;
	}

#ifdef TPD_CREATE_WR_NODE
	init_wr_node(client);
#endif

	//check sensor id
	if (GT868 == chip_type) {
		//INT pin output low
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN,
				 GPIO_CTP_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
		msleep(1);
		//reset GT868
		gtp_reset_guitar(10);
		//get sensor id from GT868
		cfg_sensor_id = gtp_get_sensor_id(client);
		//INT pin output high
		mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
		msleep(1);
		//reset GT868
		gtp_reset_guitar(10);
		//INT pin as interrupt pin
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN,
				 GPIO_CTP_EINT_PIN_M_EINT);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	} else {
		err =
		    i2c_read_bytes(client, TPD_SENSOR_ID_REG,
				   &cfg_sensor_id, 1);

		if (err) {
			TPD_DMESG(TPD_DEVICE " fail to get sensor id %d\n",
				  err);
			cfg_sensor_id = 0;
		}
	}

	cfg_sensor_id &= 0x03;
	cfg_sensor_id = (cfg_sensor_id > 2) ? 2 : cfg_sensor_id;

	if (chip_type == GT818) {
		if ((!gt818_cfg_data_len[1]) && (!gt818_cfg_data_len[2])) {
			TPD_DMESG(TPD_DEVICE "Only One TP vendor.\n");
			cfg_sensor_id = 0;
		}

		TPD_DMESG(TPD_DEVICE " get sensor id: %d\n",
			  cfg_sensor_id);
		cfg_data = gt818_cfg_data_ptr[cfg_sensor_id];
		cfg_data_with_charger =
		    gt818_cfg_data_with_charger_ptr[cfg_sensor_id];
	} else if (chip_type == GT818X) {
		if ((!gt818x_cfg_data_len[1]) && (!gt818x_cfg_data_len[2])) {
			TPD_DMESG(TPD_DEVICE " Only One TP vendor.\n");
			cfg_sensor_id = 0;
		}

		TPD_DMESG(TPD_DEVICE " get sensor id: %d\n",
			  cfg_sensor_id);
		cfg_data = gt818x_cfg_data_ptr[cfg_sensor_id];
		cfg_data_with_charger =
		    gt818x_cfg_data_with_charger_ptr[cfg_sensor_id];
	} else {
		if ((!gt868_cfg_data_len[1]) && (!gt868_cfg_data_len[2])) {
			TPD_DMESG(TPD_DEVICE "Only One TP vendor.\n");
			cfg_sensor_id = 0;
		}

		TPD_DMESG(TPD_DEVICE " get sensor id: %d\n",
			  cfg_sensor_id);
		cfg_data = gt868_cfg_data_ptr[cfg_sensor_id];
		cfg_data_with_charger =
		    gt868_cfg_data_with_charger_ptr[cfg_sensor_id];
		gt868_div_2 = 0;
		if (0x0f == cfg_data[51]) {
			gt868_div_2 = 1;
		}
	}


	// setting resolution, RES_X, RES_Y
#ifdef RES_AUTO_CONFIG
	cfg_data[59] = cfg_data_with_charger[59] = (TPD_X_RES & 0xff);
	cfg_data[60] = cfg_data_with_charger[60] =
	    ((TPD_X_RES >> 8) & 0xff);
	cfg_data[61] = cfg_data_with_charger[61] = (TPD_Y_RES & 0xff);
	cfg_data[62] = cfg_data_with_charger[62] =
	    ((TPD_Y_RES >> 8) & 0xff);
#endif

	int_type = ((cfg_data[55] >> 3) & 0x01);
	i2c_write_dummy(client, TPD_HANDSHAKING_START_REG);
	err =
	    i2c_write_bytes(client, TPD_CONFIG_REG_BASE, cfg_data,
			    CONFIG_LEN);

	if (err) {
		TPD_DMESG(TPD_DEVICE " fail to write tpd cfg %d\n", err);
		return err;
	}

	i2c_write_dummy(client, TPD_HANDSHAKING_END_REG);

	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);

	if (IS_ERR(thread)) {
		err = PTR_ERR(thread);
		TPD_DMESG(TPD_DEVICE
			  " failed to create kernel thread: %d\n", err);
	}
#ifdef TPD_HAVE_TOUCH_KEY

	for (idx = 0; idx < TPD_TOUCH_KEY_NUM; idx++) {
		input_set_capability(tpd->dev, EV_KEY,
				     touch_key_array[idx]);
	}

#endif


#ifndef TPD_RESET_ISSUE_WORKAROUND
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(10);
#endif

	// set INT mode
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);

	msleep(50);

	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM,
			     CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM,
				    CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);

	if (int_type) {
		mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM,
					 CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN,
					 CUST_EINT_POLARITY_HIGH,
					 tpd_eint_interrupt_handler, 1);
	} else {
		mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM,
					 CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN,
					 CUST_EINT_POLARITY_LOW,
					 tpd_eint_interrupt_handler, 1);
	}

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

#ifndef TPD_RESET_ISSUE_WORKAROUND
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#endif

#ifdef TPD_ESD_PROTECT
	INIT_DELAYED_WORK(&gtp_esd_check_work, gtp_esd_check_func);
	gtp_esd_check_workqueue = create_workqueue("gtp_esd_check");
	queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work,
			   TPD_ESD_CHECK_CIRCLE);
#endif

	tpd_load_status = 1;

	return 0;
}

static void gtp_reset_guitar(s32 ms)
{
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(ms);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);
	return;
}

static void tpd_eint_interrupt_handler(void)
{
	TPD_DEBUG_PRINT_INT;
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
}

static int tpd_i2c_remove(struct i2c_client *client)
{
#ifdef TPD_CREATE_WR_NODE
	uninit_wr_node();
#endif

#ifdef TPD_ESD_PROTECT
	destroy_workqueue(gtp_esd_check_workqueue);
#endif

	return 0;
}

#ifdef TPD_ESD_PROTECT
static void force_reset_guitar(void)
{
	s32 i;
	s32 ret;

	TPD_DEBUG("force_reset_guitar\n");

	//Power off TP
	hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");
	msleep(30);
	//Power on TP
	hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
	msleep(30);

	for (i = 0; i < 5; i++) {
		//Reset Guitar
#if (!defined(DCT_Z1))
		mt_set_gpio_mode(GPIO_CTP_RST_PIN,
				 GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(20);
#else
		hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");
		msleep(10);
		//Power on TP
		hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
		msleep(20);
#endif

		//Send config
		ret =
		    i2c_write_bytes(i2c_client, TPD_CONFIG_REG_BASE,
				    cfg_data, CONFIG_LEN);

		if (ret < 0) {
			continue;
		}

		i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
		break;
	}

}

static void gtp_esd_check_func(struct work_struct *work)
{
	int i;
	int ret = -1;

	if (tpd_halt) {
		return;
	}

	for (i = 0; i < 3; i++) {
		ret = i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);

		if (ret > 0) {
			break;
		}
	}

	if (i >= 3) {
		force_reset_guitar();
	}

	if (!tpd_halt) {
		queue_delayed_work(gtp_esd_check_workqueue,
				   &gtp_esd_check_work,
				   TPD_ESD_CHECK_CIRCLE);
	}

	return;
}
#endif

static void tpd_down(int x, int y, int size, int id)
{
	if ((!size) && (!id)) {
		input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	} else {
		input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, size / 100);
		/* track id Start 0 */
		input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id - 1);
	}

	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);
	TPD_EM_PRINT(x, y, x, y, id - 1, 1);

	if (FACTORY_BOOT == get_boot_mode()
	    || RECOVERY_BOOT == get_boot_mode()) {
		tpd_button(x, y, 1);
	}
}

static void tpd_up(int x, int y, int id)
{
	// input_report_abs(tpd->dev, ABS_PRESSURE, 0);
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
	input_mt_sync(tpd->dev);
	TPD_EM_PRINT(x, y, x, y, id, 0);

	if (FACTORY_BOOT == get_boot_mode()
	    || RECOVERY_BOOT == get_boot_mode()) {
		tpd_button(x, y, 0);
	}
}

static int touch_event_handler(void *unused)
{
	struct sched_param param = {.sched_priority = RTPM_PRIO_TPD };
	int x, y, size, finger_num = 0;
	int wrap_x, wrap_y = 0;
	int id = 0;
	static u8 buffer[TPD_POINT_INFO_LEN * TPD_MAX_POINTS];
	static char buf_status;
#ifdef TOUCH_PS
	static u8 ps_status = 0;
#endif
	u8 cur_mask;
	int idx;
	u8 key = 0;

	static u8 last_finger_num = 0;
#if (defined(TPD_HAVE_TOUCH_KEY) || defined(TPD_HAVE_BUTTON))
	u8 key1 = 0, key2 = 0, key3 = 0, key4 = 0;
	static u8 key1_old = 0, key2_old = 0, key3_old = 0, key4_old = 0;
	static u8 last_key = 0;
	unsigned int count = 0;
#endif

#ifdef TPD_CONDITION_SWITCH
	u8 charger_plug = 0;
	u8 *cfg;
	u32 temp;
#endif

	sched_setscheduler(current, SCHED_RR, &param);

	do {
		set_current_state(TASK_INTERRUPTIBLE);

		while (tpd_halt) {
			tpd_flag = 0;
			msleep(20);
		}

		wait_event_interruptible(waiter, tpd_flag != 0);
		tpd_flag = 0;
		TPD_DEBUG_SET_TIME;
		set_current_state(TASK_RUNNING);

		i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);

#ifdef TPD_CONDITION_SWITCH
		/* Workaround for common mode noise */
		temp = upmu_is_chr_det();

		cfg = NULL;

		if (temp)	// charger is on
		{
			if (charger_plug == 0) {
				TPD_DEBUG
				    ("update configure for charger\n");
				charger_plug = 1;
				cfg = cfg_data_with_charger;
			}
		} else {
			if (charger_plug == 1) {
				TPD_DEBUG
				    ("update configure for no charger\n");
				charger_plug = 0;
				cfg = cfg_data;
			}
		}

		if (cfg) {
			TPD_DMESG
			    ("[mtk-tpd] charger change  rewrite config \n");
			i2c_write_bytes(i2c_client, TPD_CONFIG_REG_BASE,
					cfg, CONFIG_LEN);
			i2c_write_dummy(i2c_client,
					TPD_HANDSHAKING_END_REG);
			continue;
		}
#endif

		i2c_read_bytes(i2c_client, TPD_TOUCH_INFO_REG_BASE, buffer,
			       1);
		TPD_DEBUG("[mtk-tpd] STATUS : %x\n", buffer[0]);

#if (defined(TPD_HAVE_TOUCH_KEY) || defined(TPD_HAVE_BUTTON))
		i2c_read_bytes(i2c_client, TPD_KEY_INFO_REG_BASE, &key, 1);
		TPD_DEBUG("[mtk-tpd] STATUS : %x\n", key);

		key = key & 0x0f;

#ifdef TPD_HAVE_TOUCH_KEY

		if (last_key || key) {
			for (idx = 0; idx < TPD_TOUCH_KEY_NUM; idx++) {
				input_report_key(tpd->dev,
						 touch_key_array[idx],
						 !!(key & (0x01 << idx)));
			}
		}

		last_key = key;
#else
		key1 = (key & 0x01);
		key2 = (key & 0x02);
		key3 = (key & 0x04);
		key4 = (key & 0x08);

		if (key1 == 1) {
			tpd_down(key_1, 0, 0);
		} else if ((key1_old == 1) & (key1 == 0)) {
			tpd_up(key_1, 0);
		}

		if (key2 == 2) {
			tpd_down(key_2, 0, 0);
		} else if ((key2_old == 2) & (key2 == 0)) {
			tpd_up(key_2, 0);
		}

		if (key3 == 4) {
			tpd_down(key_3, 0, 0);
		} else if ((key3_old == 4) & (key3 == 0)) {
			tpd_up(key_3, 0);
		}
		if (key4 == 8) {
			tpd_down(key_4, 0, 0);
		} else if ((key4_old == 8) & (key4 == 0)) {
			tpd_up(key_4, 0);
		}

		key1_old = key1;
		key2_old = key2;
		key3_old = key3;
		key4_old = key4;

#endif
#endif
		finger_num = buffer[0] & 0x0f;
		buf_status = buffer[0] & 0xf0;

		if (tpd == NULL || tpd->dev == NULL) {
			i2c_write_dummy(i2c_client,
					TPD_HANDSHAKING_END_REG);
			TPD_DEBUG("[mtk-tpd] tpd=%x,  tpd->dev=%x\n", tpd,
				  tpd->dev);
			continue;
		}

		if (finger_num == 0x0f)	//reload cfg,MUST before judge "DATA NOT READY"
		{
			TPD_DMESG("[mtk-tpd] error!! send config again\n");
			i2c_write_bytes(i2c_client, TPD_CONFIG_REG_BASE,
					cfg_data, CONFIG_LEN);
			i2c_write_dummy(i2c_client,
					TPD_HANDSHAKING_END_REG);
			continue;
		}

		if ((buf_status & 0x30) != 0x20)	//data not ready
		{
			TPD_DEBUG("[mtk-tpd] STATUS : %x\n", buffer[0]);
			TPD_DMESG("[mtk-tpd] data not ready return \n");
			i2c_write_dummy(i2c_client,
					TPD_HANDSHAKING_END_REG);
			continue;
		}

		if (finger_num > 5)	//abnormal state
		{
			TPD_DMESG
			    ("[mtk-tpd] finger_num =%d abnormal state  !\n",
			     finger_num);
			TPD_DMESG("[mtk-tpd] STATUS  : %x\n", buffer[0]);
			i2c_write_dummy(i2c_client,
					TPD_HANDSHAKING_END_REG);
			continue;
		}
#ifdef TOUCH_PS
		if (PS_STATUS) {
			i2c_read_bytes(i2c_client, TPD_PROXIMITY_REG,
				       &ps_status, 1);
			printk("ps_status=0x%x\n", ps_status);
			printk("finger_num=%d\n", finger_num);
#if NOT_ONLY_LARGETOUCH
			if (((ps_status & 0x10) == 0x10)
			    || ((ps_status & 0x0f) == 0x0f)) {
				touch_set_ps(0);
			} else {
				touch_set_ps(1);
			}
#else
			//      if ((finger_num >= 2) || ((ps_status&0x0f) == 0x0f))
			if (((ps_status & 0x0f) == 0x0f)) {
				printk("set ps 0\n");
				touch_set_ps(0);
			} else {
				printk("set ps 1\n");
				touch_set_ps(1);
			}
#endif
		}
#endif

		if (finger_num) {
			i2c_read_bytes(i2c_client, TPD_POINT_INFO_REG_BASE,
				       buffer,
				       finger_num * TPD_POINT_INFO_LEN);
		} else {
			//In order to release INT
			i2c_read_bytes(i2c_client, TPD_POINT_INFO_REG_BASE,
				       buffer, 1);
		}

		cur_mask = 0;

		for (idx = 0; idx < finger_num; idx++) {
			u8 *ptr = &buffer[idx * TPD_POINT_INFO_LEN];
			id = ptr[0];

			if (id < TPD_MAX_POINTS + 1) {
				x = ptr[1] + (((int) ptr[2]) << 8);
				y = ptr[3] + (((int) ptr[4]) << 8);
				size = ptr[5] + (((int) ptr[6]) << 8);

				wrap_x = TPD_WARP_X(x);
				wrap_y = TPD_WARP_Y(y);

				if (gt868_div_2) {
					wrap_x = wrap_x / 2;
					wrap_y = wrap_y / 2;
				}
#if (defined(DCT_Z1))
				if (!cfg_sensor_id) {
					if (wrap_y >= 760 && wrap_y <= 805)
						wrap_y -= 15;
					else if (wrap_y >= 0
						 && wrap_y <= 155)
						wrap_y += 15;
				}
#endif
				tpd_down(wrap_x, wrap_y, size, id);

				cur_mask |= (1 << id);
				TPD_DEBUG("id: %3d x: %3d y: %3d \n", id,
					  x, y);
			} else {
				TPD_DEBUG("Invalid id %d\n", id);
			}
		}

		if ((!finger_num) && (!key) && (last_finger_num)) {
			tpd_up(0, 0, 0);
		}

		last_finger_num = finger_num;

		if (tpd != NULL && tpd->dev != NULL) {
			input_sync(tpd->dev);
		}

		i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);

	}
	while (!kthread_should_stop());

	return 0;
}

static int tpd_local_init(void)
{

	if (i2c_add_driver(&tpd_i2c_driver) != 0) {
		TPD_DMESG("unable to add i2c driver.\n");
		return -1;
	}

	if (tpd_load_status == 0)	// disable auto load touch driver for linux3.0 porting
	{
		TPD_DMESG("add error touch panel driver.\n");
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}
#ifdef TPD_HAVE_BUTTON
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);	// initialize tpd button data
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
	TPD_DO_WARP = 1;
	memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT * 4);
	memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT * 4);
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
	memcpy(tpd_calmat, tpd_def_calmat_local, 8 * 4);
	memcpy(tpd_def_calmat, tpd_def_calmat_local, 8 * 4);
#endif

	// set vendor string
	tpd->dev->id.vendor =
	    (tpd_info.vendor_id_2 << 8) | tpd_info.vendor_id_1;
	tpd->dev->id.product =
	    (tpd_info.product_id_2 << 8) | tpd_info.product_id_1;
	tpd->dev->id.version =
	    (tpd_info.version_2 << 8) | tpd_info.version_1;

	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);
	tpd_type_cap = 1;

	return 0;
}

/* Function to manage low power suspend */
//void tpd_suspend(struct i2c_client *client, pm_message_t message)
static void tpd_suspend(struct early_suspend *h)
{
	u8 mode = 0x01;

#ifdef TPD_ESD_PROTECT
	cancel_delayed_work_sync(&gtp_esd_check_work);
#endif

#ifdef TOUCH_PS
	if (!PS_STATUS) {
		i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);
		i2c_write_bytes(i2c_client, TPD_POWER_MODE_REG, &mode, 1);
		i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
		tpd_halt = 1;
		mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	}
#else
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);
	i2c_write_bytes(i2c_client, TPD_POWER_MODE_REG, &mode, 1);
	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
	tpd_halt = 1;
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#endif
}

/* Function to manage power-on resume */
static void tpd_resume(struct early_suspend *h)
{
#ifdef TPD_RESET_ISSUE_WORKAROUND
	struct tpd_info_t tpd_info;
	int err;
	char retry = 0;
	// use PMIC Power off leakage 0.3mA, sleep mode: 0.1mA
	// use raising edge of INT to wakeup
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
	msleep(1);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
	msleep(1);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
	msleep(1);
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
#endif

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

#ifdef TPD_RESET_ISSUE_WORKAROUND

	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_START_REG);

	memset(&tpd_info, 0, sizeof(struct tpd_info_t));

	while (retry++ < 5) {
		err =
		    i2c_read_bytes(i2c_client, TPD_VERSION_INFO_REG,
				   (u8 *) & tpd_info,
				   sizeof(struct tpd_info_t));

		if (err) {
			TPD_DMESG(TPD_DEVICE " wakeup sleep failed %d.\n",
				  err);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
			msleep(10);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
			msleep(50);
		} else {
			TPD_DMESG("wakeup sleep success.\n");
			TPD_DMESG("TPD info\n");
			TPD_DMESG("vendor %02X %02X\n",
				  tpd_info.vendor_id_1,
				  tpd_info.vendor_id_2);
			TPD_DMESG("product %02X %02X\n",
				  tpd_info.product_id_1,
				  tpd_info.product_id_2);
			TPD_DMESG("version %02X %02X\n",
				  tpd_info.version_1, tpd_info.version_2);
			break;
		}
	}

	i2c_write_dummy(i2c_client, TPD_HANDSHAKING_END_REG);
#endif
	tpd_halt = 0;

#ifdef TPD_ESD_PROTECT
	queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work,
			   TPD_ESD_CHECK_CIRCLE);
#endif

}

static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = "gt818",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif
#ifdef TOUCH_PS
	.tpd_ps = gt818_ps_enable,
#endif
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
	TPD_DMESG("MediaTek gt818 touch panel driver init\n");
	i2c_register_board_info(0, &i2c_tpd, 1);

	if (tpd_driver_add(&tpd_device_driver) < 0)
		TPD_DMESG("add generic driver failed\n");

	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
	TPD_DMESG("MediaTek gt818 touch panel driver exit\n");
	tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);
