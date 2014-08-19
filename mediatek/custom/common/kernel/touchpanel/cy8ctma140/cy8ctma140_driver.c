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
#include "tpd_custom_cy8ctma140.h"

#include "tpd.h"
#include <cust_eint.h>

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "cust_gpio_usage.h"

#ifndef TPD_NO_GPIO
#include "cust_gpio_usage.h"
#endif

#ifdef TPD_HAVE_BUTTON
static int boot_mode;
#endif

extern struct tpd_device *tpd;

static struct i2c_client *i2c_client = NULL;
static struct task_struct *thread = NULL;

static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
static int boot_mode = 0;
#endif


static struct early_suspend early_suspend;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tpd_early_suspend(struct early_suspend *handler);
static void tpd_late_resume(struct early_suspend *handler);
#endif


extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
        kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
        kal_bool auto_umask);

#ifdef TOUCH_PS
static bool PS_STATUS = 0;
extern int touch_set_ps(bool val);
#endif

static void tpd_eint_interrupt_handler(void);
static int tpd_get_bl_info(int show);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
static int tpd_initialize(struct i2c_client * client);


static int tpd_flag = 0;

#define TPD_OK 					0
#define TPD_EREA_Y                             799
#define TPD_EREA_X                             479

#define TPD_DISTANCE_LIMIT                     100

#define TPD_REG_BASE 0x00
#define TPD_SOFT_RESET_MODE 0x01
#define TPD_OP_MODE 0x00
#define TPD_LOW_PWR_MODE 0x04
#define TPD_SYSINFO_MODE 0x10
#define GET_HSTMODE(reg)  ((reg & 0x70) >> 4)  // in op mode or not
#define GET_BOOTLOADERMODE(reg) ((reg & 0x10) >> 4)  // in bl mode

#define TP_POINTS_CNT	5

static u8 bl_cmd[] = {
    0x00, 0xFF, 0xA5,
    0x00, 0x01, 0x02,
    0x03, 0x04, 0x05,
    0x06, 0x07};
//exit bl mode
struct tpd_operation_data_t{
    U8 hst_mode;
    U8 tt_mode;
    U8 tt_stat;

    U8 x1_M,x1_L;
    U8 y1_M,y1_L;
    U8 z1;
    U8 touch12_id;

    U8 x2_M,x2_L;
    U8 y2_M,y2_L;
    U8 z2;
    U8 gest_cnt;
    U8 gest_id;
    //U8 gest_set;


    U8 x3_M,x3_L;
    U8 y3_M,y3_L;
    U8 z3;
    U8 touch34_id;

    U8 x4_M,x4_L;
    U8 y4_M,y4_L;
    U8 z4;

    U8 x5_M,x5_L;
    U8 y5_M,y5_L;
    U8 touch5_id;
};

struct tpd_bootloader_data_t{
    U8 bl_file;
    U8 bl_status;
    U8 bl_error;
    U8 blver_hi,blver_lo;
    U8 bld_blver_hi,bld_blver_lo;

    U8 ttspver_hi,ttspver_lo;
    U8 appid_hi,appid_lo;
    U8 appver_hi,appver_lo;

    U8 cid_0;
    U8 cid_1;
    U8 cid_2;

};

struct tpd_sysinfo_data_t{
    U8   hst_mode;
    U8  mfg_cmd;
    U8  mfg_stat;
    U8 cid[3];
    u8 tt_undef1;

    u8 uid[8];
    U8  bl_verh;
    U8  bl_verl;

    u8 tts_verh;
    u8 tts_verl;

    U8 app_idh;
    U8 app_idl;
    U8 app_verh;
    U8 app_verl;

    u8 tt_undef2[6];
    U8  act_intrvl;
    U8  tch_tmout;
    U8  lp_intrvl;

};

struct touch_info {
	int x[5];
	int y[5];
	int p[5];
	int id[5];
	int count;
};

struct id_info{
    int pid1;
    int pid2;
    int reportid1;
    int reportid2;
    int id1;
    int id2;

};
static struct tpd_operation_data_t g_operation_data;
static struct tpd_bootloader_data_t g_bootloader_data;
static struct tpd_sysinfo_data_t g_sysinfo_data;


static const struct i2c_device_id tpd_i2c_id[] = {{"cy8ctma140",0},{}};
static struct i2c_board_info __initdata cy8ctma140_i2c_tpd={ I2C_BOARD_INFO("cy8ctma140", (0x48 >> 1))};

//static const struct i2c_device_id tpd_id[] = {{TPD_DEVICE,0},{}};
//static unsigned short force[] = {0,0x48,I2C_CLIENT_END,I2C_CLIENT_END};
//static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces, };
#ifdef TOUCH_PS
static int ft_halt=0;
static void cy_ps_enable(bool val)
{
	int ret=0;
	int state=0;
	char buffer[2] = {0,0};
	buffer[0] = 0x1c;
	//ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1c, 1,&state);
	if(val)
	{
		if(ft_halt)
		{
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
			msleep(5);
#ifdef TPD_POWER_SOURCE_1800
			hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
			mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
			mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
			msleep(5);
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
			msleep(200);
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		}
		buffer[1] = 0x01;
	}
	else
	{
		buffer[1] = 0x00;
		touch_set_ps(1);
	}
	i2c_master_send(i2c_client,buffer,2);
	//ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1c, 1,&state);
	PS_STATUS = val;
}
#endif
static struct i2c_driver tpd_i2c_driver = {
	.driver =
	{
		.name = TPD_DEVICE,
		.owner = THIS_MODULE,
	},
	.probe = tpd_probe,
	.remove = __devexit_p(tpd_remove),
	.id_table = tpd_i2c_id,
	.detect = tpd_detect,
	//.address_data = &addr_data,
};

static void tpd_down(int x, int y, int i) {

#ifdef TPD_HAVE_BUTTON
    if (boot_mode != NORMAL_BOOT) {
	    if(y > 480) {
		    tpd_button(x, y, 1);
	    }
    }
#endif

    //input_report_abs(tpd->dev, ABS_PRESSURE,100);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev,ABS_MT_TRACKING_ID,i);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    TPD_DMESG("Down x:%4d, y:%4d, id:%4d \n ", x, y, i);
    input_mt_sync(tpd->dev);
    if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
    {
          tpd_button(x, y, 1);
    }
    TPD_DOWN_DEBUG_TRACK(x,y);
}

static void tpd_up(int x, int y,int i) {

    //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    //input_report_abs(tpd->dev,ABS_MT_TRACKING_ID,i);
    //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    //input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    //input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    TPD_DMESG("Up x:%4d, y:%4d, p:%4d \n", x, y, 0);
    input_mt_sync(tpd->dev);
    if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
    {
         tpd_button(x, y, 0);
    }
    TPD_UP_DEBUG_TRACK(x,y);
}


 static int tpd_touchinfo(struct touch_info *cinfo)
 {

	u32 retval;
	static u8 tt_mode;

	memset(cinfo, 0, sizeof(struct touch_info));

	retval = i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE, 8, (u8 *)&g_operation_data);
	retval += i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE + 8, 8, (((u8 *)(&g_operation_data)) + 8));
	retval += i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE + 16, 8, (((u8 *)(&g_operation_data)) + 16));
	retval += i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE + 24, 8, (((u8 *)(&g_operation_data)) + 24));
#ifdef TOUCH_PS
	if(PS_STATUS)
	{
		if(g_operation_data.y5_M == 0x01)
			touch_set_ps(0);
		else if(g_operation_data.y5_M == 0x00)
			touch_set_ps(1);
	}
#endif


	 TPD_DEBUG("received raw data from touch panel as following:\n");

		 TPD_DEBUG("hst_mode = %02X, tt_mode = %02X, tt_stat = %02X\n", \
				 g_operation_data.hst_mode,\
				 g_operation_data.tt_mode,\
				 g_operation_data.tt_stat);

	cinfo->count = (g_operation_data.tt_stat & 0x0f) ; //point count

	TPD_DEBUG("cinfo->count =%d\n",cinfo->count);

	//TPD_DEBUG("Procss raw data...\n");

	cinfo->x[0] = (( g_operation_data.x1_M << 8) | ( g_operation_data.x1_L)); //point 1
	cinfo->y[0]  = (( g_operation_data.y1_M << 8) | ( g_operation_data.y1_L));
	cinfo->p[0] = g_operation_data.z1;

	//TPD_DEBUG("Before:	cinfo->x0 = %3d, cinfo->y0 = %3d, cinfo->p0 = %3d cinfo->id0 = %3d\n", cinfo->x[0] ,cinfo->y[0] ,cinfo->p[0], cinfo->id[0]);
	if(cinfo->x[0] < 1) cinfo->x[0] = 1;
    	if(cinfo->y[0] < 1) cinfo->y[0] = 1;
	cinfo->id[0] = ((g_operation_data.touch12_id & 0xf0) >>4);
	TPD_DEBUG("After:		cinfo->x0 = %3d, cinfo->y0 = %3d, cinfo->p0 = %3d cinfo->id[0] = %3d\n", cinfo->x[0] ,cinfo->y[0] ,cinfo->p[0], cinfo->id[0]);

	if(cinfo->count >1)
	{
		 cinfo->x[1] = (( g_operation_data.x2_M << 8) | ( g_operation_data.x2_L)); //point 2
		 cinfo->y[1] = (( g_operation_data.y2_M << 8) | ( g_operation_data.y2_L));
		 cinfo->p[1] = g_operation_data.z2;

		//TPD_DEBUG("before:	 cinfo->x2 = %3d, cinfo->y2 = %3d,  cinfo->p2 = %3d\n", cinfo->x2, cinfo->y2,  cinfo->p2);
		if(cinfo->x[1] < 1) cinfo->x[1] = 1;
		if(cinfo->y[1] < 1) cinfo->y[1] = 1;
		cinfo->id[1] = ((g_operation_data.touch12_id & 0x0f));
		TPD_DEBUG("After:	 cinfo->x[1] = %3d, cinfo->y[1] = %3d,  cinfo->p[1] = %3d, cinfo->id[1] = %3d\n", cinfo->x[1], cinfo->y[1], cinfo->p[1], cinfo->id[1]);

		if (cinfo->count > 2)
		{
			cinfo->x[2]= (( g_operation_data.x3_M << 8) | ( g_operation_data.x3_L)); //point 3
			cinfo->y[2] = (( g_operation_data.y3_M << 8) | ( g_operation_data.y3_L));
			cinfo->p[2] = g_operation_data.z3;
			cinfo->id[2] = ((g_operation_data.touch34_id & 0xf0) >> 4) ;

			//TPD_DEBUG("before:	 cinfo->x[2] = %3d, cinfo->y[2]  = %3d, cinfo->p[2]  = %3d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2]);
			if(cinfo->x[2] < 1) cinfo->x[2] = 1;
			if(cinfo->y[2]< 1) cinfo->y[2] = 1;
			TPD_DEBUG("After:	 cinfo->x[2]= %3d, cinfo->y[2] = %3d, cinfo->p[2]= %3d, cinfo->id[2] = %3d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2], cinfo->id[2]);

			if (cinfo->count > 3)
			{
				cinfo->x[3] = (( g_operation_data.x4_M << 8) | ( g_operation_data.x4_L)); //point 3
				cinfo->y[3] = (( g_operation_data.y4_M << 8) | ( g_operation_data.y4_L));
				cinfo->p[3] = g_operation_data.z4;
				cinfo->id[3] = ((g_operation_data.touch34_id & 0x0f)) ;

				//TPD_DEBUG("before:	 cinfo->x[3] = %3d, cinfo->y[3] = %3d, cinfo->p[3] = %3d, cinfo->id[3] = %3d\n", cinfo->x[3], cinfo->y[3], cinfo->p[3], cinfo->id[3]);
				if(cinfo->x[3] < 1) cinfo->x[3] = 1;
				if(cinfo->y[3] < 1) cinfo->y[3] = 1;
				TPD_DEBUG("After:	 cinfo->x[3] = %3d, cinfo->y[3] = %3d, cinfo->p[3]= %3d, cinfo->id[3] = %3d\n", cinfo->x[3], cinfo->y[3], cinfo->p[3], cinfo->id[3]);
			}
			if (cinfo->count > 4)
			{
				cinfo->x[4] = (( g_operation_data.x5_M << 8) | ( g_operation_data.x5_L)); //point 3
				cinfo->y[4] = (( g_operation_data.y5_M << 8) | ( g_operation_data.y5_L));
				cinfo->p[4] = 0;//g_operation_data.z4;
				cinfo->id[4] = (g_operation_data.touch5_id & 0xff) ;

				//TPD_DEBUG("before:	 cinfo->x[4] = %3d, cinfo->y[4] = %3d, cinfo->id[4] = %3d\n", cinfo->x[4], cinfo->y[4], cinfo->id[4]);
				TPD_DEBUG("before:	 x5_M = %3d, x5_L = %3d\n", g_operation_data.x5_M,  g_operation_data.x5_L);
				if(cinfo->x[4] < 1) cinfo->x[4] = 1;
				if(cinfo->y[4] < 1) cinfo->y[4] = 1;
				TPD_DEBUG("After:	 cinfo->x[4] = %3d, cinfo->y[4] = %3d,  cinfo->id[4] = %3d\n", cinfo->x[4], cinfo->y[4], cinfo->id[4]);
			}
		}

	}

	if (!cinfo->count) return true; // this is a touch-up event

	if (g_operation_data.tt_mode & 0x20) return false; // buffer is not ready for use

	// data toggle
	u8 data0,data1;

	data0 = i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE, 1, (u8*)&g_operation_data);
	TPD_DEBUG("before hst_mode = %02X \n", g_operation_data.hst_mode);

	if((g_operation_data.hst_mode & 0x80)==0)
				  g_operation_data.hst_mode = g_operation_data.hst_mode|0x80;
			 else
				 g_operation_data.hst_mode = g_operation_data.hst_mode & (~0x80);

			 TPD_DEBUG("after hst_mode = %02X \n", g_operation_data.hst_mode);
	data1 = i2c_smbus_write_i2c_block_data(i2c_client, TPD_REG_BASE, sizeof(g_operation_data.hst_mode), &g_operation_data.hst_mode);

	if (tt_mode == g_operation_data.tt_mode) return false; // sampling not completed
	 else tt_mode = g_operation_data.tt_mode;
	 return true;

 };


static int touch_event_handler(void *unused)
{
	int i,j;
	struct touch_info cinfo;
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };

	sched_setscheduler(current, SCHED_RR, &param);
	do
	{
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(waiter,tpd_flag!=0);
		tpd_flag = 0;

		set_current_state(TASK_RUNNING);
		if (tpd_touchinfo(&cinfo))
		{
			if(cinfo.count >0)
			{
				switch(cinfo.count)
				{
					case 5:
					{
						tpd_down(cinfo.x[4], cinfo.y[4], cinfo.id[4]);
					}
					case 4:
					{
						tpd_down(cinfo.x[3], cinfo.y[3], cinfo.id[3]);
					}
					case 3:
					{
						tpd_down(cinfo.x[2], cinfo.y[2], cinfo.id[2]);
					}
					case 2:
					{
						tpd_down(cinfo.x[1], cinfo.y[1], cinfo.id[1]);
					}
					case 1:
					{
						tpd_down(cinfo.x[0], cinfo.y[0], cinfo.id[0]);
					}
					default:
						break;
				}
			}
			else if(cinfo.count == 0)
			{
				tpd_up(cinfo.x[0], cinfo.y[0], 0);
			}
			input_sync(tpd->dev);

		}
	}while(!kthread_should_stop());

	return 0;
}

static int tpd_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    strcpy(info->type, "mtk-tpd");
    return 0;
}


static void tpd_eint_interrupt_handler(void)
{
    //mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    TPD_DMESG("TPD interrupt has been triggered\n");
    tpd_flag = 1;
    wake_up_interruptible(&waiter);


}
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

    int retval = TPD_OK;
    i2c_client = client;

    char buffer[2];
    int status=0;

#ifdef TPD_NO_GPIO
    u16 temp;
    temp = *(volatile u16 *) TPD_RESET_PIN_ADDR;
    temp = temp | 0x40;
    *(volatile u16 *) TPD_RESET_PIN_ADDR = temp;
#endif


    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    mt_set_gpio_pull_enable(GPIO_CTP_RST_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_RST_PIN, GPIO_PULL_UP);

    //msleep(20);
#ifdef TPD_POWER_SOURCE_1800
	hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif

    msleep(100);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(50);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(50);
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
    msleep(50);

    status = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &(buffer[0]));
    if(status<0)
    {
        TPD_DMESG("[mtk-tpd], cy8ctma140 tpd_i2c_probe failed!!\n");
        status = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &(buffer[0]));
        if(status<0) {
            TPD_DMESG("[mtk-tpd], cy8ctma140 tpd_i2c_probe retry failed!!\n");
            //i2c_del_driver(&tpd_i2c_driver);
            return status;
        }
    }

    TPD_DMESG("[mtk-tpd], cy8ctma140 tpd_i2c_probe success!!, buffer[0] = 0x%x\n", buffer[0]);
    tpd_load_status = 1;
    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread)) {
        retval = PTR_ERR(thread);
		return retval;
        TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
    }

    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM,CUST_EINT_EDGE_SENSITIVE);//CUST_EINT_LEVEL_SENSITIVE
    mt65xx_eint_set_polarity(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_POLARITY);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 0);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

    //msleep(100);

    TPD_DMESG("Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");

    return retval;

}


static int __devexit tpd_remove(struct i2c_client *client)

{
    int error;

#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

    TPD_DMESG("TPD removed\n");

    return 0;
}


static int tpd_local_init(void)
{

	TPD_DMESG("Cypress CY8CTMA140 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

	if(i2c_add_driver(&tpd_i2c_driver)!=0) {
		TPD_DMESG("unable to add i2c driver.\n");
		return -1;
	}

	if(tpd_load_status == 0){
		TPD_DMESG("add error touch panel driver.\n");
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}

#ifdef TPD_HAVE_BUTTON
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
	boot_mode = get_boot_mode();
#endif
}

static int tpd_resume(struct i2c_client *client)
{
	int retval = TPD_OK;
#ifdef TOUCH_PS
	if(!PS_STATUS)
	{
		// msleep(100);
#ifdef TPD_POWER_SOURCE_1800
		hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
		mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(100);

		TPD_DEBUG("TPD wake up\n");
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		ft_halt = 0;
	}
#else
#ifdef TPD_POWER_SOURCE_1800
	hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
	msleep(10);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(10);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(100);

	TPD_DEBUG("TPD wake up\n");
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
#endif
    return retval;
}

static int tpd_suspend(struct i2c_client *client, pm_message_t message)
{
    int retval = TPD_OK;

    TPD_DEBUG("TPD enter sleep\n");

#ifdef TOUCH_PS
	if(!PS_STATUS)
	{
	ft_halt = 1;
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

#ifdef TPD_POWER_SOURCE_1800
	hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
    //msleep(1);
    mdelay(1);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	}
#else
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

#ifdef TPD_POWER_SOURCE_1800
	hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
    //msleep(1);
    mdelay(1);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
#endif
    return retval;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tpd_early_suspend(struct early_suspend *handler)
{
    tpd_suspend(i2c_client, PMSG_SUSPEND);
}

static void tpd_late_resume(struct early_suspend *handler)
{
    tpd_resume(i2c_client);
}
#endif

static struct tpd_driver_t tpd_device_driver = {
    .tpd_device_name = "cy8ctma140",
    .tpd_local_init = tpd_local_init,
    .suspend = tpd_suspend,
    .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
    .tpd_have_button = 1,
#else
    .tpd_have_button = 0,
#endif
#ifdef TOUCH_PS
	     .tpd_ps = cy_ps_enable,
#endif

};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
	TPD_DMESG("MediaTek cy8ctma140 touch panel driver init\n");
	i2c_register_board_info(0, &cy8ctma140_i2c_tpd, 1);

	if(tpd_driver_add(&tpd_device_driver) < 0)
	TPD_DMESG("add generic driver failed\n");
	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) {
    TPD_DMESG("MediaTek cy8ctma140 touch panel driver exit\n");
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

