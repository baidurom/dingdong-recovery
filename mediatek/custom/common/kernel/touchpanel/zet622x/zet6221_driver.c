/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "tpd.h"
#include <cust_eint.h>
#include <linux/jiffies.h>

#include <linux/input/mt.h>

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#include "cust_gpio_usage.h"
#include "tpd_custom_zet622x.h"


#define ZET6221_DEBUG		0

#if ZET6221_DEBUG
#define ZET(f) 					printk("[ZET6221]%s => %s, line: %d\n", __FILE__, __FUNCTION__, __LINE__)
#define ZET_DEBUG(fmt, arg...) 		printk("[ZET6221]" fmt "\n", ##arg)
#else
#define ZET(f) 
#define ZET_DEBUG(fmt, arg...)
#endif

extern struct tpd_device *tpd;

static int tpd_flag = 0;
static int tpd_halt=0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

///////////////////////////


#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#define TPD_KEY_COUNT sizeof(tpd_keys_local)/sizeof(tpd_keys_local[0])
/*
static int tpd_keys_local[] = { KEY_MENU, KEY_HOMEPAGE, KEY_BACK};

#define TPD_KEY_COUNT sizeof(tpd_keys_local)/sizeof(tpd_keys_local[0])

#define TPD_KEY_GAP 30
#define TPD_KEY_Y_CENTER (1000)
#define TPD_KEY_Y_START (TPD_KEY_Y_CENTER-TPD_KEY_GAP/2)

static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = 
{
	{205, 2020, 50, 50},
	{520, 2020, 50, 50},
	{865, 2020, 50, 50},	
};
*/
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

#define TPD_OK 0

#define TPD_POINT_INFO_LEN      4 //albert
#define TPD_MAX_POINTS          5
#define MAX_TRANSACTION_LENGTH 8
#define I2C_DEVICE_ADDRESS_LEN 2

#define TPD_WARP_Y(y) ( TPD_Y_RES - 1 - y )
#define TPD_WARP_X(x)
 
//#define MAX_I2C_TRANSFER_SIZE (MAX_TRANSACTION_LENGTH - I2C_DEVICE_ADDRESS_LEN)
#define MAX_I2C_TRANSFER_SIZE MAX_TRANSACTION_LENGTH
#define ZET6221_I2C_FREQ		400

//albert: slave address = 0x76 
static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"zet6221",0},{}};

#define ANDROID40 
#ifdef ANDROID40	// for android 4.0
static struct i2c_board_info __initdata zet6221_i2c_tpd={ I2C_BOARD_INFO("zet6221", (0xec>>1))};
#else
static unsigned short force[] = {0, 0xEC, I2C_CLIENT_END,I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
static struct i2c_client_address_data addr_data = { .forces = forces,};
#endif

static struct i2c_driver tpd_i2c_driver =
{                       
	.driver = {
		 .name = "zet6221",
#ifndef ANDROID40
		 .owner = THIS_MODULE,
#endif
	  },

    .probe = tpd_i2c_probe,                                   
    .remove = tpd_i2c_remove,                           
    .detect = tpd_i2c_detect,                           
    //.driver.name = "mtk-tpd", 
    .id_table = tpd_i2c_id,              
#ifndef ANDROID40
    .address_data = &addr_data,                        
#endif
}; 

//albert
//#define TPINFO
//#define CHARGER_MODE
//#define FW_UPGRADE
#define MT_TYPE_B
#define MAX_KEY_NUMBER      8
#define MAX_FINGER_NUMBER	16
#define DEBOUNCE_NUMBER	    1
static int ResolutionX;
static int ResolutionY;
static int bufLength=0;
static u16 FingerNum=5;
static u16 KeyNum=0;
static u8 inChargerMode=0;
static u8 xyExchange=0;
static int f_up_cnt=0;
u8 pc[8];
static u8 BHover=0;

static u8 zet6221_ts_get_xy_from_panel(struct i2c_client *client, u32 *x, u32 *y, u32 *z, u32 *pr, u32 *ky);

u8 intZetGetHoverStatus()
{
	return BHover;
}
EXPORT_SYMBOL_GPL(intZetGetHoverStatus);

#ifdef CHARGER_MODE

extern int enable_cmd;
extern int disable_cmd;

static u8 ChargeChange = 0;//discharge

void ts_write_charge_enable_cmd();
void ts_write_charge_disable_cmd();

struct zet6221_tsdrv {
	//struct i2c_client *i2c_ts;
	//struct work_struct work1;
	struct work_struct work2; //  write_cmd
	//struct workqueue_struct *ts_workqueue; // 
	struct workqueue_struct *ts_workqueue1; //write_cmd
	//struct input_dev *input;
	struct timer_list polling_timer;
};

static u16 polling_time = 100;

#endif

#ifdef FW_UPGRADE
//#include "zet622x_fw.h"
//#include "zet6221_fw_8_inches.h"

extern int __init zet622x_downloader( struct i2c_client *client );
#endif


/* proc file system */
static int i2c_write_bytes( struct i2c_client *client, u16 addr, u8 *txbuf, int len );
static int i2c_write_dummy( struct i2c_client *client, u16 addr );
static struct proc_dir_entry *gt818_config_proc = NULL;

/***********************************************************************
    [function]: 
		        callback: read data by i2c interface;
    [parameters]:
			    client[in]:  struct i2c_client �� represent an I2C slave device;
			    data [out]:  data buffer to read;
			    length[in]:  data length to read;
    [return]:
			    Returns negative errno, else the number of messages executed;
************************************************************************/
s32 zet6221_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = length;
	msg.buf = data;
	msg.timing = ZET6221_I2C_FREQ;
	return i2c_transfer(client->adapter,&msg, 1);
}

/***********************************************************************
    [function]: 
		        callback: write data by i2c interface;
    [parameters]:
			    client[in]:  struct i2c_client �� represent an I2C slave device;
			    data [out]:  data buffer to write;
			    length[in]:  data length to write;
    [return]:
			    Returns negative errno, else the number of messages executed;
************************************************************************/
s32 zet6221_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = length;
	msg.buf = data;
	msg.timing = ZET6221_I2C_FREQ;
	return i2c_transfer(client->adapter,&msg, 1);
}

/***********************************************************************
    [function]: 
		        callback: get dynamic report information;
    [parameters]:
    			client[in]:  struct i2c_client ??represent an I2C slave device;

    [return]:
			    1;
************************************************************************/
u8 zet6221_ts_get_report_mode_t(struct i2c_client *client)
{
	u8 ts_report_cmd[1] = {0xb2};
	u8 ts_reset_cmd[1] = {0xb0};
	u8 ts_in_data[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ret;
	int i;
	
	ret=zet6221_i2c_write_tsdata(client, ts_report_cmd, 1);

	if (ret > 0)
	{
			//udelay(10);
			msleep(10);
			printk ("=============== zet6221_ts_get_report_mode_t ===============\n");
			ret=zet6221_i2c_read_tsdata(client, ts_in_data, 17);
			//ret=i2c_read_bytes( client, 0x0, ts_in_data, 17);
			
			if(ret > 0)
			{
				
				for(i=0;i<8;i++)
				{
					pc[i]=ts_in_data[i] & 0xff;
				}

				xyExchange = (ts_in_data[16] & 0x8) >> 3;
				if(xyExchange == 1)
				{
					ResolutionY= ts_in_data[9] & 0xff;
					ResolutionY= (ResolutionY << 8)|(ts_in_data[8] & 0xff);
					ResolutionX= ts_in_data[11] & 0xff;
					ResolutionX= (ResolutionX << 8) | (ts_in_data[10] & 0xff);
				}
				else
				{
					ResolutionX = ts_in_data[9] & 0xff;
					ResolutionX = (ResolutionX << 8)|(ts_in_data[8] & 0xff);
					ResolutionY = ts_in_data[11] & 0xff;
					ResolutionY = (ResolutionY << 8) | (ts_in_data[10] & 0xff);
				}
					
				FingerNum = (ts_in_data[15] & 0x7f);
				KeyNum = (ts_in_data[15] & 0x80);
				inChargerMode = (ts_in_data[16] & 0x2) >> 1;

				if(KeyNum==0)
					bufLength  = 3+4*FingerNum;
				else
					bufLength  = 3+4*FingerNum+1;
				
			}else
			{
				printk ("=============== zet6221_ts_get_report_mode_t READ ERROR ===============\n");
				return ret;
			}
							
	}else
	{
		return ret;
	}
	return 1;
}

#ifdef CHARGER_MODE

void ts_write_charge_enable_cmd()
{
	printk("%s is running ==========",__FUNCTION__);
	u8 ts_write_charge_cmd[1] = {0xb5}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(i2c_client, ts_write_charge_cmd, 1);
}
EXPORT_SYMBOL_GPL(ts_write_charge_enable_cmd);

void ts_write_charge_disable_cmd()
{
	printk("%s is running ==========",__FUNCTION__);
	u8 ts_write_cmd[1] = {0xb6}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(i2c_client, ts_write_cmd, 1);
}
EXPORT_SYMBOL_GPL(ts_write_charge_disable_cmd);

/***********************************************************************
    [function]: 
		        callback: Timer Function if there is no interrupt fuction;
    [parameters]:
			    arg[in]:  arguments;
    [return]:
			    NULL;
************************************************************************/

static void polling_timer_func(unsigned long arg)
{

	struct zet6221_tsdrv *ts_drv = (struct zet6221_tsdrv *)arg;
	queue_work(ts_drv->ts_workqueue1, &ts_drv->work2);
	mod_timer(&ts_drv->polling_timer,jiffies + msecs_to_jiffies(polling_time));	
}

void write_cmd_work()
{
	if(enable_cmd != ChargeChange)
	{	
		if(enable_cmd == 1) {
			ts_write_charge_enable_cmd();
			
		}else if(enable_cmd == 0)
		{
			ts_write_charge_disable_cmd();
		}
		ChargeChange = enable_cmd;
	}

}

#endif

static int i2c_read_bytes( struct i2c_client *client, u16 addr, u8 *rxbuf, int len )
{
    u8 buffer[I2C_DEVICE_ADDRESS_LEN];
    u8 retry;
    u16 left = len;
    u16 offset = 0;

    struct i2c_msg msg[2] =
    {
        {
            .addr = client->addr,
            .flags = 0,
            .buf = buffer,
            .len = I2C_DEVICE_ADDRESS_LEN,
            .timing = ZET6221_I2C_FREQ
        },
        {
            .addr = client->addr,
            .flags = I2C_M_RD,
            .timing = ZET6221_I2C_FREQ
        },
    };

    if ( rxbuf == NULL )
        return -1;

    ZET_DEBUG("i2c_read_bytes to device 0x%02X address 0x%04X len %d\n", client->addr, addr, len );

    while ( left > 0 )
    {
        buffer[0] = ( ( addr+offset ) >> 8 ) & 0xFF;
        buffer[1] = ( addr+offset ) & 0xFF;

        msg[1].buf = &rxbuf[offset];

        if ( left > MAX_TRANSACTION_LENGTH )
        {
            msg[1].len = MAX_TRANSACTION_LENGTH;
            left -= MAX_TRANSACTION_LENGTH;
            offset += MAX_TRANSACTION_LENGTH;
        }
        else
        {
            msg[1].len = left;
            left = 0;
        }

        retry = 0;

        //while ( i2c_transfer( client->adapter, &msg[0], 2 ) != 2 )
        while ( i2c_transfer( client->adapter, &msg[1], 1 ) != 1 )
        {
            retry++;

            if ( retry == 20 )
            {
                ZET_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, len);
                return -1;
            }
        }
    }

    return 0;
}

static int i2c_write_bytes( struct i2c_client *client, u16 addr, u8 *txbuf, int len )
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u16 left = len;
    u16 offset = 0;
    u8 retry = 0;

    struct i2c_msg msg = 
    {
        .addr = client->addr,
        .flags = 0,
        .buf = buffer
    };


    if ( txbuf == NULL )
        return -1;

    ZET_DEBUG("i2c_write_bytes to device %02X address %04X len %d\n", client->addr, addr, len );

    while ( left > 0 )
    {
        retry = 0;

        //buffer[0] = ( (addr+offset) >> 8 ) & 0xFF;
        //buffer[1] = ( addr+offset ) & 0xFF;

        if ( left > MAX_I2C_TRANSFER_SIZE )
        {
            //memcpy( &buffer[I2C_DEVICE_ADDRESS_LEN], &txbuf[offset], MAX_I2C_TRANSFER_SIZE );
            memcpy( &buffer, &txbuf[offset], MAX_I2C_TRANSFER_SIZE );
            msg.len = MAX_TRANSACTION_LENGTH;
            left -= MAX_I2C_TRANSFER_SIZE;
            offset += MAX_I2C_TRANSFER_SIZE;
        }
        else
        {
            //memcpy( &buffer[I2C_DEVICE_ADDRESS_LEN], &txbuf[offset], left );
            memcpy( &buffer, &txbuf[offset], left );
            //msg.len = left + I2C_DEVICE_ADDRESS_LEN;
            msg.len = left;
            left = 0;
        }

        ZET_DEBUG("byte left %d offset %d\n", left, offset );

        while ( i2c_transfer( client->adapter, &msg, 1 ) != 1 )
        {
            retry++;

            if ( retry == 20 )
            {
                ZET_DEBUG("I2C write 0x%X%X length=%d failed\n", buffer[0], buffer[1], len);
                return -1;
            }
            else
                 ZET_DEBUG("I2C write retry %d addr 0x%X%X\n", retry, buffer[0], buffer[1]);

        }
    }

    return 0;
}

static int i2c_write_dummy( struct i2c_client *client, u16 addr )
{
    u8 buffer[MAX_TRANSACTION_LENGTH];

    struct i2c_msg msg =
    {
        .addr = client->addr,
        .flags = 0,
        .buf = buffer,
        .len = 2
    };

    ZET_DEBUG("i2c_write_dummy to device %02X address %04X\n", client->addr, addr );

    buffer[0] = (addr >> 8) & 0xFF;
    buffer[1] = (addr) & 0xFF;

    i2c_transfer( client->adapter, &msg, 1 ); 

    return 0;
}

static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
    strcpy(info->type, "zet6221");
    return 0;
}

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{             
	int err = 0;
	int retry = 0;
	char data[30] = {0};
	int reset_count = 0;
	
reset_proc:	
	i2c_client = client;

	// Power on
    
#ifdef TPD_POWER_SOURCE_CUSTOM
        hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
        hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
        mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif

    
#ifdef FW_UPGRADE
	if(zet622x_downloader(client)<=0)
	{
		printk("[zet6221] FW upgrade failed!\n");
	}else
	{
		printk("[zet6221] FW upgrade successed!\n");
	}
#endif    

	//albert: reset ic
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(1);  
	//mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	//mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

	
#if defined(TPINFO)

	msleep(3);  	
	
	if(zet6221_ts_get_report_mode_t(client)<=0)
	{
		//goto exit_check_functionality_failed;
	}
		
	//if(pc[3]!=0x8)  // not zeitec ic
	//	goto exit_check_functionality_failed;
	

	if(KeyNum>0)
	{
		input_set_abs_params(tpd->dev, ABS_MT_POSITION_X, 0, ResolutionX, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_POSITION_Y, 0, ResolutionY, 0, 0);
	}else
	{
#ifndef TPD_HAVE_BUTTON   
		input_set_abs_params(tpd->dev, ABS_MT_POSITION_X, 0, ResolutionX, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_POSITION_Y, 0, ResolutionY, 0, 0);	
#endif
	}

	
#else

	msleep(20);  	

	ResolutionX = X_MAX;
	ResolutionY = Y_MAX;
	FingerNum = TPD_MAX_POINTS;
	KeyNum = 1;   
	if(KeyNum==0)
		bufLength  = 3+TPD_POINT_INFO_LEN*TPD_MAX_POINTS;
	else
		bufLength  = 3+TPD_POINT_INFO_LEN*TPD_MAX_POINTS+1;
#endif
	printk( "[ZET6221] TP ResolutionX=%d ResolutionY=%d FingerNum=%d KeyNum=%d\n",ResolutionX,ResolutionY,FingerNum,KeyNum);

#ifdef MT_TYPE_B
	input_mt_init_slots(tpd->dev, FingerNum);	
#endif
	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);
	msleep(20); 
 
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(10);
	
	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

	msleep(100);
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
	{
		TPD_DMESG("I2C transfer error, line: %d\n", __LINE__);
#ifdef TPD_RESET_ISSUE_WORKAROUND
        if ( reset_count < TPD_MAX_RESET_COUNT )
        {
            reset_count++;
            goto reset_proc;
        }
#endif
		   return -1; 
	}

	tpd_load_status = 1;

	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);


	if (IS_ERR(thread))
	{ 
		err = PTR_ERR(thread);
		ZET_DEBUG(TPD_DEVICE " failed to create kernel thread: %d\n", err);
	}

	ZET_DEBUG("zet6221 tpd_i2c_probe\n");
	

#ifdef CHARGER_MODE

	struct zet6221_tsdrv *zet6221_ts;
	zet6221_ts = kzalloc(sizeof(struct zet6221_tsdrv), GFP_KERNEL);
	
	/*   charger detect : write_cmd */
	INIT_WORK(&zet6221_ts->work2, write_cmd_work);
	zet6221_ts->ts_workqueue1 = create_singlethread_workqueue(dev_name(&client->dev)); //  workqueue
	if (!zet6221_ts->ts_workqueue1) {
	//	err = -ESRCH;
		printk("ts_workqueue1 ts_probe error ==========\n");
		return;
	}
	/*   charger detect : write_cmd */

	setup_timer(&zet6221_ts->polling_timer, polling_timer_func, (unsigned long)zet6221_ts);
	mod_timer(&zet6221_ts->polling_timer,jiffies + msecs_to_jiffies(800));

#endif
	
	return 0;
}

static void tpd_eint_interrupt_handler(void)
{  		
    TPD_DEBUG_PRINT_INT;
    tpd_flag=1;
    wake_up_interruptible(&waiter);
} 
static int tpd_i2c_remove(struct i2c_client *client)
{
    return 0;
}

static  void tpd_down(int x, int y, int p) {
	// input_report_abs(tpd->dev, ABS_PRESSURE, p);
	 input_report_key(tpd->dev, BTN_TOUCH, 1);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	 //printk("D[%4d %4d %4d] ", x, y, p);
	 input_mt_sync(tpd->dev);
   if (1)//(FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
   {   
       tpd_button(x, y, 1);  
   }	 
	 TPD_DOWN_DEBUG_TRACK(x,y);
 }

static void tpd_up(int x, int y, int id)
{
    input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    /* track id Start 0 */
    input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id-1);
    input_mt_sync(tpd->dev);
    TPD_EM_PRINT(x, y, x, y, id, 0);
    if (1)//(FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
    {   
        tpd_button(x, y, 0); 
    }       
    //TPD_DEBUG_PRINT_POINT( x, y, 0 );
}

static int touch_event_handler(void *unused)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD }; 

	//albert
	//int pr = 0;
	u32 x[MAX_FINGER_NUMBER], y[MAX_FINGER_NUMBER], z[MAX_FINGER_NUMBER], pr, ky, points;
	u32 px,py,pz;
	u8 ret;
	u8 pressure;
	u8 i;
	
	//bufLength = 3+TPD_POINT_INFO_LEN*TPD_MAX_POINTS;
	//FingerNum = TPD_MAX_POINTS;
	//KeyNum = 0;

	sched_setscheduler(current, SCHED_RR, &param); 

	do
	{
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
	    set_current_state(TASK_INTERRUPTIBLE);
/*
	    while ( tpd_halt )
	    {
	        tpd_flag = 0;
	        msleep(3);
	    }
*/
	    wait_event_interruptible(waiter, tpd_flag != 0);
	    tpd_flag = 0;
	    TPD_DEBUG_SET_TIME;
	    set_current_state(TASK_RUNNING); 
	            
	    ///if ( tpd == NULL || tpd->dev == NULL )
	        ///continue;
	    
	    pr=0;
	    ky=0;
	    ret = zet6221_ts_get_xy_from_panel(i2c_client, x, y, z, &pr, &ky);
	    
	    if(ret == 0x3C)
		{

			points = pr;
			BHover = (z[0]>>7)&0x1;
			
			if(points == 0)
			{

#ifdef MT_TYPE_B
				for(i=0;i<FingerNum;i++){
					input_mt_slot(tpd->dev, i);
					input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER,false);
					input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
				}
				input_mt_report_pointer_emulation(tpd->dev, true);
#else
				input_report_key(tpd->dev, BTN_TOUCH, 0);
				input_mt_sync(tpd->dev);
#endif

				input_sync(tpd->dev);

#ifdef TPD_HAVE_BUTTON     			
				px = 0;
				py = 0;
				pz = 0;
				if(KeyNum > 0)
				{
					for(i=0;i<MAX_KEY_NUMBER;i++)
					{			
						pressure = ky & ( 0x01 << i );
						if(pressure && i < TPD_KEY_COUNT)
						{
							px = tpd_keys_dim_local[i][0];
							py = tpd_keys_dim_local[i][1];
							pz = 1;
							break;
						}
					}
					tpd_button(px, py, pz); 
				}else
				{
#ifndef MT6577
					tpd_button(0, 0, 0); 
#endif				
				}
				
#endif
				continue;
			}
						
			{

				for(i=0;i<FingerNum;i++)
				{
					pressure = (points >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
					
					ZET_DEBUG("valid=%d pressure[%d]= %d x= %d y= %d\n",points , i, pressure,x[i],y[i]);

					if(pressure)
					{
					
#if defined(TPINFO)
						px = x[i];
						py = y[i];
						pz = z[i];
#else
						px = x[i]*LCM_X_RES/X_MAX;
						py = y[i]*LCM_Y_RES/Y_MAX;			
						pz = z[i];
#endif
						ZET_DEBUG("%d after translate x= %d y= %d\n", i, px,py);
						
#ifdef TPD_HAVE_BUTTON 
#ifndef MT6577
						if(points == 0x8000 && py > LCM_Y_RES && KeyNum == 0)
						{
							tpd_button(px, py, 1);  
							break;
						}
#endif
#endif
						
#ifdef MT_TYPE_B
						input_mt_slot(tpd->dev, i);
						input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER,true);
#endif						
						// input_report_abs(tpd->dev, ABS_PRESSURE, p);
						input_report_key(tpd->dev, BTN_TOUCH, 1);
						input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 8);
						input_report_abs(tpd->dev, ABS_MT_POSITION_X, px);
						input_report_abs(tpd->dev, ABS_MT_POSITION_Y, py);
						//input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, i);
#ifndef MT_TYPE_B
						input_mt_sync(tpd->dev);	
#endif
							 
					}else
					{
#ifdef MT_TYPE_B
						input_mt_slot(tpd->dev, i);
						input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER,false);
						input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
#endif	
					}
					
				}

#ifdef MT_TYPE_B
				input_mt_report_pointer_emulation(tpd->dev, true);
#endif				
				
				input_sync(tpd->dev);
			}	


		}
		//msleep(1);

	} while ( !kthread_should_stop() ); 

    return 0;
}

static u8 zet6221_ts_get_xy_from_panel(struct i2c_client *client, u32 *x, u32 *y, u32 *z, u32 *pr, u32 *ky)
{
	u8  ts_data[70];
	int ret;
	int i;
	u32 num = 0;
	
	memset(ts_data,0,70);

	i2c_read_bytes( i2c_client, 0x0, ts_data, bufLength);

	num = ts_data[1];
	num = (num << 8) | ts_data[2];
	*pr = num;

	ZET_DEBUG( "num = %d --- *pr= %d y= %d\n", num, *pr);
		
	for(i=0;i<FingerNum;i++)
	{
		x[i]=(u8)((ts_data[3+4*i])>>4)*256 + (u8)ts_data[(3+4*i)+1];
		y[i]=(u8)((ts_data[3+4*i]) & 0x0f)*256 + (u8)ts_data[(3+4*i)+2];
		z[i]=(u8)((ts_data[(3+4*i)+3]) & 0xff);
	}
	for(i=0;i<bufLength+1;i++)
	ZET_DEBUG("zet6221_ts_get_xy_from_panel ts_data[%d] = %x \n", i, ts_data[i]);
		
	//if key enable
	if(KeyNum > 0)
		*ky = ts_data[3+4*FingerNum];

	return ts_data[0];
}

static int tpd_local_init(void) 
{
	ZET();
	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		ZET_DEBUG("unable to add i2c driver.\n");
		return -1;
	}
	
	if(tpd_load_status == 0)
	{
		ZET_DEBUG("add error touch panel driver.\n");
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}

	#ifdef TPD_HAVE_BUTTON     
		tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
	#endif   

	#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
		TPD_DO_WARP = 1;
		memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
		memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
	#endif 

	#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
		memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
		memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
	#endif  

	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
	tpd_type_cap = 1;

	return 0;
}

/* Function to manage low power suspend */
//void tpd_suspend(struct i2c_client *client, pm_message_t message)
static int tpd_suspend( struct early_suspend *h )
{
	u8 data = 0xb1;
	
	ZET();
	
	tpd_halt = 1;
	//albert
	//zet6221_i2c_write_tsdata(i2c_client, &data, 1);
//	i2c_write_bytes( i2c_client, 0x0, &data, 1 );

	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	//mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);

#ifdef TPD_POWER_SOURCE_CUSTOM
        hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
        hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
        mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
	return 0;
}

/* Function to manage power-on resume */
//void tpd_resume(struct i2c_client *client)
static int tpd_resume( struct early_suspend *h )
{
	u8 data = 0xb4;
	
	ZET();

#ifdef TPD_POWER_SOURCE_CUSTOM
        hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
        hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
        mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif

#if 1
	//albert: reset ic
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	
	msleep(1);  
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(20);  

#else
	zet6221_i2c_write_tsdata(i2c_client, &data, 1);
	msleep(50);
#endif

	tpd_halt = 0;
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
	
#ifdef CHARGER_MODE
	ChargeChange = 0;
#endif
	
	return 0;
}

/**Called from android**/
#include <linux/fs.h>
static int Major;             /* Major number assigned to our device driver */
#define DEVICE_NAME "zet622x" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80            /* Max length of the message from the device */
static int Device_Open = 0;   /* Is device open?  Used to prevent multiple  */
static char msg[BUF_LEN];     /* The msg the device will give when asked    */
static char *msg_Ptr;

static int zet_device_open(struct inode *inode, struct file *file)
{
   static int counter = 0;
   if (Device_Open) return -EBUSY;
   Device_Open++;
   //MOD_INC_USE_COUNT;
	
  return 0;
}
	
static int zet_device_release(struct inode *inode, struct file *file)
{
	Device_Open --;     /* We're now ready for our next caller */	
	//MOD_DEC_USE_COUNT;
	
	return 0;
}
	
static ssize_t zet_device_read(struct file *filp,
	   char *buffer,    /* The buffer to fill with data */
	   size_t length,   /* The length of the buffer     */
	   loff_t *offset)  /* Our offset in the file       */
{
	   /* Number of bytes actually written to the buffer */
	   int bytes_read = 0;
	
	   sprintf(msg,"%d", intZetGetHoverStatus());	
	
	   /* If we're at the end of the message, return 0 signifying end of file */
	   if (*msg_Ptr == 0) return 0;
	
	   /* Actually put the data into the buffer */
	   while (length && *msg_Ptr)  {

	      put_user(*(msg_Ptr++), buffer++);
	
	      length--;
	      bytes_read++;
	   }
	
	   /* Most read functions return the number of bytes put into the buffer */
	   return bytes_read;
}
static struct file_operations zet_fops = {
	.read = zet_device_read, 
	.open = zet_device_open,
	.release = zet_device_release
};

/****/

static struct tpd_driver_t tpd_device_driver =
{
    .tpd_device_name = "zet6221",
    .tpd_local_init = tpd_local_init,
    .suspend = tpd_suspend,
    .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
    .tpd_have_button = 1,
#else
    .tpd_have_button = 0,
#endif		
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
    ZET();

#ifdef ANDROID40	// for android 4.0
	i2c_register_board_info(0, &zet6221_i2c_tpd, 1);
#endif

/**Called from android**/
	Major = register_chrdev(0, DEVICE_NAME, &zet_fops);
	
	if (Major < 0) {
	    printk("Registering the character device failed with %d\n", Major);
	}
/****/

    if ( tpd_driver_add(&tpd_device_driver) < 0)
        ZET_DEBUG("add generic driver failed\n");

    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
    ZET();
    //input_unregister_device(tpd->dev);
    
    int ret;
    /**Called from android**/
    //unregister_chrdev(Major, DEVICE_NAME);
    //if (ret < 0) printk("Error in unregister_chrdev: %d\n", ret);
    /****/    
    
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);


