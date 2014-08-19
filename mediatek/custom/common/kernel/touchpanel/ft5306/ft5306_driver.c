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

 
#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "tpd_custom_ft5306.h"

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "cust_gpio_usage.h"
#include <asm/uaccess.h>

//#define VELOCITY_CUSTOM_FT5306
#define FT5306_APK_SUPPORT

#ifdef VELOCITY_CUSTOM_FT5306
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#endif
#include <linux/dma-mapping.h>

 
extern struct tpd_device *tpd;
 
struct i2c_client *i2c_client = NULL;
struct task_struct *thread = NULL;
 
static DECLARE_WAIT_QUEUE_HEAD(waiter);
 
 
static void tpd_eint_interrupt_handler(void);
 
#ifdef TOUCH_PS
static bool PS_STATUS = 0;
extern int touch_set_ps(bool val);
#endif
 
 extern void mt65xx_eint_unmask(unsigned int line);
 extern void mt65xx_eint_mask(unsigned int line);
 extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
 extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
 extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
									  kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
									  kal_bool auto_umask);

 
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
 

static int tpd_flag = 0;
static int ft_halt=0;
static int point_num = 0;
static int p_point_num = 0;


#define TPD_OK 0
//register define

#define DEVICE_MODE 0x00
#define GEST_ID 0x01
#define TD_STATUS 0x02

#define TOUCH1_XH 0x03
#define TOUCH1_XL 0x04
#define TOUCH1_YH 0x05
#define TOUCH1_YL 0x06

#define TOUCH2_XH 0x09
#define TOUCH2_XL 0x0A
#define TOUCH2_YH 0x0B
#define TOUCH2_YL 0x0C

#define TOUCH3_XH 0x0F
#define TOUCH3_XL 0x10
#define TOUCH3_YH 0x11
#define TOUCH3_YL 0x12

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

#ifdef VELOCITY_CUSTOM_FT5306
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

// for magnify velocity********************************************
#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

#define TPD_VELOCITY_CUSTOM_X 12
#define TPD_VELOCITY_CUSTOM_Y 16

static int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;
static int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;
static int tpd_misc_open(struct inode *inode, struct file *file)
{
/*
	file->private_data = adxl345_i2c_client;

	if(file->private_data == NULL)
	{
		printk("tpd: null pointer!!\n");
		return -EINVAL;
	}
	*/
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tpd_misc_release(struct inode *inode, struct file *file)
{
	//file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int adxl345_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	//struct i2c_client *client = (struct i2c_client*)file->private_data;
	//struct adxl345_i2c_data *obj = (struct adxl345_i2c_data*)i2c_get_clientdata(client);	
	//char strbuf[256];
	void __user *data;
	
	long err = 0;
	
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
		printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case TPD_GET_VELOCITY_CUSTOM_X:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

	   case TPD_GET_VELOCITY_CUSTOM_Y:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
			{
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
//	.owner = THIS_MODULE,
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

#ifdef FT5306_APK_SUPPORT
// jashe add for focaltech's app update fw s
#define FT5x0x_REG_FW_VER		0xA6

#define IC_FT5X06       0

#define DEVICE_IC_TYPE IC_FT5X06

#define FT_UPGRADE_AA   0xAA
#define FT_UPGRADE_55   0x55

/*upgrade config of FT5316*/
#define FT5316_UPGRADE_AA_DELAY 		50
#define FT5316_UPGRADE_55_DELAY 		30
#define FT5316_UPGRADE_ID_1			0x79
#define FT5316_UPGRADE_ID_2			0x07
#define FT5316_UPGRADE_READID_DELAY 	1
#define FT5316_UPGRADE_EARSE_DELAY	1500

/*upgrade config of FT5x06(x=2,3,4)*/
#define FT5X06_UPGRADE_AA_DELAY 		50
#define FT5X06_UPGRADE_55_DELAY 		30
#define FT5X06_UPGRADE_ID_1			0x79
#define FT5X06_UPGRADE_ID_2			0x03
#define FT5X06_UPGRADE_READID_DELAY 	1
#define FT5X06_UPGRADE_EARSE_DELAY	2000

/*upgrade config of FT6208*/
#define FT6208_UPGRADE_AA_DELAY 		60
#define FT6208_UPGRADE_55_DELAY 		10
#define FT6208_UPGRADE_ID_1			0x79
#define FT6208_UPGRADE_ID_2			0x05
#define FT6208_UPGRADE_READID_DELAY 	10
#define FT6208_UPGRADE_EARSE_DELAY	2000

/*upgrade config of FT6206*/
#define FT6206_UPGRADE_AA_DELAY 		100
#define FT6206_UPGRADE_55_DELAY 		10
#define FT6206_UPGRADE_ID_1			0x79
#define FT6206_UPGRADE_ID_2			0x08
#define FT6206_UPGRADE_READID_DELAY 	10
#define FT6206_UPGRADE_EARSE_DELAY	2000

#define FTS_PACKET_LENGTH        	128
#define FTS_UPGRADE_LOOP        	3
#define FTS_FACTORYMODE_VALUE		0x40
#define FTS_WORKMODE_VALUE		0x00
struct Upgrade_Info {
        u16 delay_aa;           /*delay of write FT_UPGRADE_AA */
        u16 delay_55;           /*delay of write FT_UPGRADE_55 */
        u8 upgrade_id_1;        /*upgrade id 1 */
        u8 upgrade_id_2;        /*upgrade id 2 */
        u16 delay_readid;       /*delay of read id */
        u16 delay_earse_flash; /*delay of earse flash*/
};

enum {
	FTS_FT5X06 = 0,
	FTS_FT5316,
	FTS_FT6206,
	FTS_FT6208,
};

u8 *I2CDMABuf_va = NULL;
volatile u32 I2CDMABuf_pa = NULL;

int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;
	int i = 0;

	client->addr = client->addr & I2C_MASK_FLAG;
	if(writelen < 8)
	{
		client->ext_flag = client->ext_flag & (~I2C_DMA_FLAG)& (~I2C_ENEXT_FLAG);

		//MSE_ERR("Sensor non-dma write timing is %x!\r\n", this_client->timing);
		return i2c_master_send(client, writebuf, writelen);
	}
	else
	{
		for(i = 0 ; i < writelen; i++)
		{
			I2CDMABuf_va[i] = writebuf[i];
		}
		client->ext_flag = client->ext_flag | I2C_DMA_FLAG | I2C_ENEXT_FLAG;

		if((ret=i2c_master_send(client, (unsigned char *)I2CDMABuf_pa, writelen))!=writelen)
			dev_err(&client->dev, "###%s i2c write len=%x,buffaddr=%x\n", __func__,ret,I2CDMABuf_pa);
		//MSE_ERR("Sensor dma timing is %x!\r\n", this_client->timing);
		return ret;
	}
}

int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,int writelen, char *readbuf, int readlen)
{
        int ret;
		int i;
	if(writelen!=0)
	{
		//DMA Write
		if(writelen < 8  )
		{
			client->ext_flag = client->ext_flag & (~I2C_DMA_FLAG)& (~I2C_ENEXT_FLAG);

			//MSE_ERR("Sensor non-dma write timing is %x!\r\n", this_client->timing);
			ret= i2c_master_send(client, writebuf, writelen);
		}
		else
		{
			for(i = 0 ; i < writelen; i++)
			{
				I2CDMABuf_va[i] = writebuf[i];
			}
			client->ext_flag = client->ext_flag | I2C_DMA_FLAG | I2C_ENEXT_FLAG;

			if((ret=i2c_master_send(client, (unsigned char *)I2CDMABuf_pa, writelen))!=writelen)
				dev_err(&client->dev, "###%s i2c write len=%x,buffaddr=%x\n", __func__,ret,I2CDMABuf_pa);
			//MSE_ERR("Sensor dma timing is %x!\r\n", this_client->timing);
			//return ret;
		}
	}
	//DMA Read
	if(readlen!=0)
	{
		if (readlen <8) {
			client->ext_flag = client->ext_flag & (~I2C_DMA_FLAG)& (~I2C_ENEXT_FLAG);
			ret = i2c_master_recv(client, (unsigned char *)readbuf, readlen);
		}
		else
		{
			client->ext_flag = client->ext_flag | I2C_DMA_FLAG | I2C_ENEXT_FLAG;

			ret = i2c_master_recv(client, (unsigned char *)I2CDMABuf_pa, readlen);

			for(i = 0; i < readlen; i++)
			{
				readbuf[i] = I2CDMABuf_va[i];
			}
		}
	}

	return ret;
}

int ft5x0x_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
        unsigned char buf[2] = {0};
        buf[0] = regaddr;
        buf[1] = regvalue;

        return ft5x0x_i2c_Write(client, buf, sizeof(buf));
}

int ft5x0x_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
        return ft5x0x_i2c_Read(client, &regaddr, 1, regvalue, 1);
}

int fts_ctpm_auto_clb(struct i2c_client *client)
{
        unsigned char uc_temp = 0x00;
        unsigned char i = 0;

        /*start auto CLB */
        msleep(200);

        ft5x0x_write_reg(client, 0, FTS_FACTORYMODE_VALUE);
        /*make sure already enter factory mode */
        msleep(100);
        /*write command to start calibration */
        ft5x0x_write_reg(client, 2, 0x4);
        msleep(300);
        for (i = 0; i < 100; i++) {
                ft5x0x_read_reg(client, 0, &uc_temp);
                /*return to normal mode, calibration finish */
                if (0x0 == ((uc_temp & 0x70) >> 4))
                        break;
        }

        //msleep(200);
        /*calibration OK */
        msleep(300);
        ft5x0x_write_reg(client, 0, FTS_FACTORYMODE_VALUE);     /*goto factory mode for store */
        msleep(100);    /*make sure already enter factory mode */
        ft5x0x_write_reg(client, 2, 0x5);       /*store CLB result */
        msleep(300);
        ft5x0x_write_reg(client, 0, FTS_WORKMODE_VALUE);        /*return to normal mode */
        msleep(300);

        /*store CLB result OK */
        return 0;
}

struct Upgrade_Info fts_updateinfo[] =
{
	{FT5X06_UPGRADE_AA_DELAY, FT5X06_UPGRADE_55_DELAY, FT5X06_UPGRADE_ID_1, FT5X06_UPGRADE_ID_2, FT5X06_UPGRADE_READID_DELAY, FT5X06_UPGRADE_EARSE_DELAY},
	{FT5316_UPGRADE_AA_DELAY, FT5316_UPGRADE_55_DELAY, FT5316_UPGRADE_ID_1, FT5316_UPGRADE_ID_2, FT5316_UPGRADE_READID_DELAY, FT5316_UPGRADE_EARSE_DELAY},
	{FT6206_UPGRADE_AA_DELAY, FT6206_UPGRADE_55_DELAY, FT6206_UPGRADE_ID_1, FT6206_UPGRADE_ID_2, FT6206_UPGRADE_READID_DELAY, FT6206_UPGRADE_EARSE_DELAY},
	{FT6208_UPGRADE_AA_DELAY, FT6208_UPGRADE_55_DELAY, FT6208_UPGRADE_ID_1, FT6208_UPGRADE_ID_2, FT6208_UPGRADE_READID_DELAY, FT6208_UPGRADE_EARSE_DELAY},
};

int fts_ctpm_fw_upgrade(struct i2c_client *client, u8 *pbt_buf,u32 dw_lenth)
{
        u8 reg_val[2] = {0};
        u32 i = 0;
        u32 packet_number;
        u32 j;
        u32 temp;
        u32 lenght;
        u8 packet_buf[FTS_PACKET_LENGTH + 6];
        u8 auc_i2c_write_buf[10];
        u8 bt_ecc;
        int i_ret;
		u8 chip_id, chip_curr;
        struct Upgrade_Info upgradeinfo;

//		upgradeinfo.delay_55 = FT5X06_UPGRADE_55_DELAY;
//		upgradeinfo.delay_aa = FT5X06_UPGRADE_AA_DELAY;
//		upgradeinfo.upgrade_id_1 = FT5X06_UPGRADE_ID_1;
//		upgradeinfo.upgrade_id_2 = FT5X06_UPGRADE_ID_2;
//		upgradeinfo.delay_readid = FT5X06_UPGRADE_READID_DELAY;
//		upgradeinfo.delay_earse_flash = FT5X06_UPGRADE_EARSE_DELAY;
        //fts_get_upgrade_info(&upgradeinfo);

        /***************************************************************************
					         Update fw depends on chip start
		****************************************************************************/
		//Step 1:Read chip id
		ft5x0x_read_reg(client,0xA3,&chip_id);
		TPD_DEBUG("%s chip_id = %x\n", __func__, chip_id);

		//Step 2:Decide which chip
		switch(chip_id){
			case 0x55:
				chip_curr = FTS_FT5X06;
				break;
			case 0x0A:
				chip_curr = FTS_FT5316;
				break;
			case 0x06:
				chip_curr = FTS_FT6206;
				break;
			case 0x05:
				chip_curr = FTS_FT6208;
				break;
			default:
				chip_curr = FTS_FT5X06;
				break;
			}

		//Step 3: Set current upgradeinfo
		memcpy(&upgradeinfo, &fts_updateinfo[chip_curr], sizeof(struct Upgrade_Info));
        /***************************************************************************
					         Update fw depends on chip end
		****************************************************************************/

        for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
                /*********Step 1:Reset  CTPM *****/
                /*write 0xaa to register 0xfc */
                if ((chip_curr == FTS_FT6206)||(chip_curr == FTS_FT6208))
                        ft5x0x_write_reg(client, 0xbc, FT_UPGRADE_AA);
                else
                        ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_AA);
                msleep(upgradeinfo.delay_aa);

                /*write 0x55 to register 0xfc */
                if ((chip_curr == FTS_FT6206)||(chip_curr == FTS_FT6208))
                        ft5x0x_write_reg(client, 0xbc, FT_UPGRADE_55);
                else
                        ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_55);

                msleep(upgradeinfo.delay_55);
                /*********Step 2:Enter upgrade mode *****/

                auc_i2c_write_buf[0] = FT_UPGRADE_55;
                auc_i2c_write_buf[1] = FT_UPGRADE_AA;
                do {
                        i++;
                        i_ret = ft5x0x_i2c_Write(client, auc_i2c_write_buf, 2);
                        msleep(5);
                } while (i_ret <= 0 && i < 5);


                /*********Step 3:check READ-ID***********************/
                msleep(upgradeinfo.delay_readid);
                auc_i2c_write_buf[0] = 0x90;
                auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =
                        0x00;
                ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);


                if (reg_val[0] == upgradeinfo.upgrade_id_1
                        && reg_val[1] == upgradeinfo.upgrade_id_2) {
                        //dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
                                //reg_val[0], reg_val[1]);
                        //DBG("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
                        //        reg_val[0], reg_val[1]);
                        break;
                } else {
                        dev_err(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
                                reg_val[0], reg_val[1]);
                }
        }
        if (i >= FTS_UPGRADE_LOOP)
                return -EIO;
        auc_i2c_write_buf[0] = 0xcd;

        ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);


        /*Step 4:erase app and panel paramenter area*/
        //DBG("Step 4:erase app and panel paramenter area\n");
        auc_i2c_write_buf[0] = 0x61;
        ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1); /*erase app area */
        msleep(upgradeinfo.delay_earse_flash);
        /*erase panel parameter area */
        auc_i2c_write_buf[0] = 0x63;
        ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
        msleep(100);

        /*********Step 5:write firmware(FW) to ctpm flash*********/
        bt_ecc = 0;
        //DBG("Step 5:write firmware(FW) to ctpm flash\n");

        dw_lenth = dw_lenth - 8;
        packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
        packet_buf[0] = 0xbf;
        packet_buf[1] = 0x00;

        for (j = 0; j < packet_number; j++) {
                temp = j * FTS_PACKET_LENGTH;
                packet_buf[2] = (u8) (temp >> 8);
                packet_buf[3] = (u8) temp;
                lenght = FTS_PACKET_LENGTH;
                packet_buf[4] = (u8) (lenght >> 8);
                packet_buf[5] = (u8) lenght;

                for (i = 0; i < FTS_PACKET_LENGTH; i++) {
                        packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
                        bt_ecc ^= packet_buf[6 + i];
                }

                ft5x0x_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH + 6);
                msleep(FTS_PACKET_LENGTH / 6 + 1);
                //DBG("write bytes:0x%04x\n", (j+1) * FTS_PACKET_LENGTH);
                //delay_qt_ms(FTS_PACKET_LENGTH / 6 + 1);
        }

        if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
                temp = packet_number * FTS_PACKET_LENGTH;
                packet_buf[2] = (u8) (temp >> 8);
                packet_buf[3] = (u8) temp;
                temp = (dw_lenth) % FTS_PACKET_LENGTH;
                packet_buf[4] = (u8) (temp >> 8);
                packet_buf[5] = (u8) temp;

                for (i = 0; i < temp; i++) {
                        packet_buf[6 + i] = pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
                        bt_ecc ^= packet_buf[6 + i];
                }

                ft5x0x_i2c_Write(client, packet_buf, temp + 6);
                msleep(20);
        }

        /*send the last six byte */
        for (i = 0; i < 6; i++) {
                temp = 0x6ffa + i;
                packet_buf[2] = (u8) (temp >> 8);
                packet_buf[3] = (u8) temp;
                temp = 1;
                packet_buf[4] = (u8) (temp >> 8);
                packet_buf[5] = (u8) temp;
                packet_buf[6] = pbt_buf[dw_lenth + i];
                bt_ecc ^= packet_buf[6];
                ft5x0x_i2c_Write(client, packet_buf, 7);
                msleep(20);
        }


        /*********Step 6: read out checksum***********************/
        /*send the opration head */
        //DBG("Step 6: read out checksum\n");
        auc_i2c_write_buf[0] = 0xcc;
        ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
        if (reg_val[0] != bt_ecc) {
                dev_err(&client->dev, "[FTS]--ecc error! FW=%02x bt_ecc=%02x\n",
                                        reg_val[0],
                                        bt_ecc);
                return -EIO;
        }

        /*********Step 7: reset the new FW***********************/
        //DBG("Step 7: reset the new FW\n");
        auc_i2c_write_buf[0] = 0x07;
        ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
        msleep(300);    /*make sure CTP startup normally */

        return 0;
}

static int ft5x0x_ReadFirmware(char *firmware_name,
                               unsigned char *firmware_buf)
{
        struct file *pfile = NULL;
        struct inode *inode;
        unsigned long magic;
        off_t fsize;
        char filepath[128];
        loff_t pos;
        mm_segment_t old_fs;

        memset(filepath, 0, sizeof(filepath));
        sprintf(filepath, "%s", firmware_name);
        if (NULL == pfile)
                pfile = filp_open(filepath, O_RDONLY, 0);
        if (IS_ERR(pfile)) {
                pr_err("error occured while opening file %s.\n", filepath);
                return -EIO;
        }

        inode = pfile->f_dentry->d_inode;
        magic = inode->i_sb->s_magic;
        fsize = inode->i_size;
        old_fs = get_fs();
        set_fs(KERNEL_DS);
        pos = 0;
        vfs_read(pfile, firmware_buf, fsize, &pos);
        filp_close(pfile, NULL);
        set_fs(old_fs);

        return 0;
}

static int ft5x0x_GetFirmwareSize(char *firmware_name)
{
        struct file *pfile = NULL;
        struct inode *inode;
        unsigned long magic;
        off_t fsize = 0;
        char filepath[128];
        memset(filepath, 0, sizeof(filepath));

        sprintf(filepath, "%s", firmware_name);

        if (NULL == pfile)
                pfile = filp_open(filepath, O_RDONLY, 0);

        if (IS_ERR(pfile)) {
                pr_err("error occured while opening file %s.\n", filepath);
                return -EIO;
        }

        inode = pfile->f_dentry->d_inode;
        magic = inode->i_sb->s_magic;
        fsize = inode->i_size;
        filp_close(pfile, NULL);
        return fsize;
}

int fts_ctpm_fw_upgrade_with_app_file(struct i2c_client *client,
                                       char *firmware_name)
{
        u8 *pbt_buf = NULL;
        int i_ret;
        int fwsize = ft5x0x_GetFirmwareSize(firmware_name);

        if (fwsize <= 0) {
                dev_err(&client->dev, "%s ERROR:Get firmware size failed\n",__func__);
                return -EIO;
        }

        if (fwsize < 8 || fwsize > 32 * 1024) {
                dev_dbg(&client->dev, "%s:FW length error\n", __func__);
                return -EIO;
        }

        /*=========FW upgrade========================*/
        pbt_buf = kmalloc(fwsize + 1, GFP_ATOMIC);

        if (ft5x0x_ReadFirmware(firmware_name, pbt_buf)) {
                dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n",__func__);
                kfree(pbt_buf);
                return -EIO;
        }

        if ((pbt_buf[fwsize - 8] ^ pbt_buf[fwsize - 6]) == 0xFF
                && (pbt_buf[fwsize - 7] ^ pbt_buf[fwsize - 5]) == 0xFF
                && (pbt_buf[fwsize - 3] ^ pbt_buf[fwsize - 4]) == 0xFF) {
                /*call the upgrade function */
                i_ret = fts_ctpm_fw_upgrade(client, pbt_buf, fwsize);
                if (i_ret != 0) {
                        dev_dbg(&client->dev, "%s() - ERROR:[FTS] upgrade failed..\n",__func__);
		}
                else {
			//#ifdef AUTO_CLB
                        fts_ctpm_auto_clb(client);      /*start auto CLB*/
			//#endif
                }
                kfree(pbt_buf);
        } else {
                dev_dbg(&client->dev, "%s:FW format error\n", __func__);
                kfree(pbt_buf);
                return -EIO;
        }

        return i_ret;
}

static ssize_t ft5x0x_tpfwver_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t num_read_chars = 0;
	u8 fwver = 0;

	//mutex_lock(&g_device_mutex);

	if (ft5x0x_read_reg(i2c_client, FT5x0x_REG_FW_VER, &fwver) < 0)
		num_read_chars = snprintf(buf, PAGE_SIZE,
					"get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02X\n", fwver);

	//mutex_unlock(&g_device_mutex);

	return num_read_chars;
}

static ssize_t ft5x0x_tpfwver_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	/*place holder for future use*/
	return -EPERM;
}

static DEVICE_ATTR(ftstpfwver, S_IRUGO | S_IWUSR, ft5x0x_tpfwver_show,
			ft5x0x_tpfwver_store);

static ssize_t ft5x0x_fwupgradeapp_show(struct device *dev,struct device_attribute *attr,char *buf)
{
        /*place holder for future use*/
        return -EPERM;
}
/*upgrade from app.bin*/
static ssize_t ft5x0x_fwupgradeapp_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
        char fwname[128];

        memset(fwname, 0, sizeof(fwname));
        sprintf(fwname, "%s", buf);
        fwname[count - 1] = '\0';

        //mutex_lock(&g_device_mutex);
        //disable_irq(client->irq);
        mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

        fts_ctpm_fw_upgrade_with_app_file(i2c_client, fwname);

        mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
        //enable_irq(client->irq);
        //mutex_unlock(&g_device_mutex);

        return count;
}

static DEVICE_ATTR(ftsfwupgradeapp, S_IRUGO | S_IWUSR, ft5x0x_fwupgradeapp_show,ft5x0x_fwupgradeapp_store);

static struct attribute *ft5x0x_attributes[] = {
    &dev_attr_ftstpfwver.attr,
    //&dev_attr_ftsfwupdate.attr,
    //&dev_attr_ftstprwreg.attr,
    &dev_attr_ftsfwupgradeapp.attr,
    NULL
};

static struct attribute_group ft5x0x_attribute_group = {
    .attrs = ft5x0x_attributes
};

#define FTS_DMA_BUF_SIZE 				1024

int ft5x0x_create_sysfs(struct i2c_client *client)
{
    int err;

	I2CDMABuf_va = (u8 *)dma_alloc_coherent(NULL, FTS_DMA_BUF_SIZE, &I2CDMABuf_pa, GFP_KERNEL);

    if(!I2CDMABuf_va)
	{
		dev_dbg(&client->dev,"%s Allocate DMA I2C Buffer failed!\n",__func__);
		return -EIO;
	}
	printk("FTP: I2CDMABuf_pa=%x,val=%x val2=%x\n",&I2CDMABuf_pa,I2CDMABuf_pa,(unsigned char *)I2CDMABuf_pa);

    err = sysfs_create_group(&client->dev.kobj, &ft5x0x_attribute_group);
    if (0 != err) {
        dev_err(&client->dev,"%s() - ERROR: sysfs_create_group() failed.\n",__func__);
        sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
        return -EIO;
    } else {
        //mutex_init(&g_device_mutex);
        pr_info("ft5x0x:%s() - sysfs_create_group() succeeded.\n",__func__);
    }
    return err;
}

void ft5x0x_release_sysfs(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
	//mutex_destroy(&g_device_mutex);
	if(I2CDMABuf_va)
	{
		dma_free_coherent(NULL, FTS_DMA_BUF_SIZE, I2CDMABuf_va, I2CDMABuf_pa);
		I2CDMABuf_va = NULL;
		I2CDMABuf_pa = 0;
	}
}

// jashe add for focaltech's app update fw e 


// jashe add for ft5306's apk update fw 2 s
#define FTS_PACKET_LENGTH        128

#define PROC_UPGRADE                    0
#define PROC_READ_REGISTER              1
#define PROC_WRITE_REGISTER     2
#define PROC_RAWDATA                    3
#define PROC_AUTOCLB                    4

#define PROC_NAME       "ft5x0x-debug"
static unsigned char proc_operate_mode = PROC_RAWDATA;
static struct proc_dir_entry *ft5x0x_proc_entry;

static int ft5x0x_debug_write(struct file *filp,
        const char __user *buff, unsigned long len, void *data)
{
        struct i2c_client *client = (struct i2c_client *)ft5x0x_proc_entry->data;
        unsigned char writebuf[FTS_PACKET_LENGTH];
        int buflen = len;
        int writelen = 0;
        int ret = 0;

	printk("[CCI TP DEBUG] %s enter\n",__func__);

        if (copy_from_user(&writebuf, buff, buflen)) {
                dev_err(&client->dev, "%s:copy from user error\n", __func__);
                return -EFAULT;
        }
        proc_operate_mode = writebuf[0];

        switch (proc_operate_mode) {
        case PROC_UPGRADE:
                {
			printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE 1\n",__func__);
                        char upgrade_file_path[64];
                        memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
                        sprintf(upgrade_file_path, "%s", writebuf + 1);
                        upgrade_file_path[buflen-1] = '\0';
                        //DBG("%s\n", upgrade_file_path);
			printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE 2\n",__func__);
                        disable_irq(client->irq);
			printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE 3\n",__func__);
                        ret = fts_ctpm_fw_upgrade_with_app_file(client, upgrade_file_path);
			printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE 4\n",__func__);
                        enable_irq(client->irq);
			printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE 5\n",__func__);
                        if (ret < 0) {
				printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE fail\n",__func__);
                                dev_err(&client->dev, "%s:upgrade failed.\n", __func__);
                                return ret;
                        }
			printk("[CCI TP DEBUG] %s CASE:PROC_UPGRADE 6\n",__func__);
			
                }
                break;
        case PROC_READ_REGISTER:
                writelen = 1;
		printk("[CCI TP DEBUG] %s CASE:PROC_READ_REGISTER 1\n",__func__);
                //DBG("%s:register addr=0x%02x\n", __func__, writebuf[1]);
                ret = ft5x0x_i2c_Write(client, writebuf + 1, writelen);
		printk("[CCI TP DEBUG] %s CASE:PROC_READ_REGISTER 2\n",__func__);
                if (ret < 0) {
			printk("[CCI TP DEBUG] %s CASE:PROC_READ_REGISTER fail\n",__func__);
                        dev_err(&client->dev, "%s:write iic error\n", __func__);
                        return ret;
                }
		printk("[CCI TP DEBUG] %s CASE:PROC_READ_REGISTER 3\n",__func__);
                break;
        case PROC_WRITE_REGISTER:
                writelen = 2;
		printk("[CCI TP DEBUG] %s CASE:PROC_WRITE_REGISTER 1\n",__func__);
                ret = ft5x0x_i2c_Write(client, writebuf + 1, writelen);
		printk("[CCI TP DEBUG] %s CASE:PROC_WRITE_REGISTER 2\n",__func__);
                if (ret < 0) {
			printk("[CCI TP DEBUG] %s CASE:PROC_WRITE_REGISTER fail\n",__func__);
                        dev_err(&client->dev, "%s:write iic error\n", __func__);
                        return ret;
                }
		printk("[CCI TP DEBUG] %s CASE:PROC_WRITE_REGISTER 3\n",__func__);
                break;
        case PROC_RAWDATA:
                break;
        case PROC_AUTOCLB:
		printk("[CCI TP DEBUG] %s CASE:PROC_AUTOCLB 1\n",__func__);
                fts_ctpm_auto_clb(client);
		printk("[CCI TP DEBUG] %s CASE:PROC_AUTOCLB 2\n",__func__);
                break;
        default:
                break;
        }

	return buflen;
        //return len;
}


static int ft5x0x_debug_read( char *page, char **start,
        off_t off, int count, int *eof, void *data )
{
        struct i2c_client *client = (struct i2c_client *)ft5x0x_proc_entry->data;
        int ret = 0, err = 0;
        u8 tx = 0, rx = 0;
        int i, j;
        unsigned char buf[PAGE_SIZE];
        int num_read_chars = 0;
        int readlen = 0;
        u8 regvalue = 0x00, regaddr = 0x00;
        switch (proc_operate_mode) {
        case PROC_UPGRADE:
                /*after calling ft5x0x_debug_write to upgrade*/
                regaddr = 0xA6;
                ret = ft5x0x_read_reg(client, regaddr, &regvalue);
                if (ret < 0)
                        num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
                else
                        num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
                break;
        case PROC_READ_REGISTER:
                readlen = 1;
                ret = ft5x0x_i2c_Read(client, NULL, 0, buf, readlen);
                if (ret < 0) {
                        dev_err(&client->dev, "%s:read iic error\n", __func__);
                        return ret;
                } else
                        //DBG("%s:value=0x%02x\n", __func__, buf[0]);
                num_read_chars = 1;
                break;
        case PROC_RAWDATA:
                break;
        default:
                break;
        }

        memcpy(page, buf, num_read_chars);

        return num_read_chars;
}
int ft5x0x_create_apk_debug_channel(struct i2c_client * client)
{
        ft5x0x_proc_entry = create_proc_entry(PROC_NAME, 0666, NULL);
        if (NULL == ft5x0x_proc_entry) {
                dev_err(&client->dev, "Couldn't create proc entry!\n");
                return -ENOMEM;
        } else {
                dev_info(&client->dev, "Create proc entry success!\n");
                ft5x0x_proc_entry->data = client;
                ft5x0x_proc_entry->write_proc = ft5x0x_debug_write;
                ft5x0x_proc_entry->read_proc = ft5x0x_debug_read;
        }
        return 0;
}

void ft5x0x_release_apk_debug_channel(void)
{
        if (ft5x0x_proc_entry)
                remove_proc_entry(PROC_NAME, NULL);
}

// jashe add for ft5306's apk update fw 2 e

#endif

//register define

struct touch_info {
    int y[5];
    int x[5];
    int p[5];
    int count;
};
 
 static const struct i2c_device_id ft5306_tpd_id[] = {{"ft5306",0},{}};
 //unsigned short force[] = {0,0x70,I2C_CLIENT_END,I2C_CLIENT_END}; 
 //static const unsigned short * const forces[] = { force, NULL };
 //static struct i2c_client_address_data addr_data = { .forces = forces, };
 static struct i2c_board_info __initdata ft5306_i2c_tpd={ I2C_BOARD_INFO("ft5306", (0x70>>1))};
 
 
 static struct i2c_driver tpd_i2c_driver = {
  .driver = {
	 .name = "ft5306",//.name = TPD_DEVICE,
//	 .owner = THIS_MODULE,
  },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
  .id_table = ft5306_tpd_id,
  .detect = tpd_detect,
//  .address_data = &addr_data,
 };
 

static  void tpd_down(int x, int y, int p) {
	// input_report_abs(tpd->dev, ABS_PRESSURE, p);
	 input_report_key(tpd->dev, BTN_TOUCH, 1);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	 //printk("D[%4d %4d %4d] ", x, y, p);
	 input_mt_sync(tpd->dev);
   if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
   {   
       tpd_button(x, y, 1);  
   }	 
       //tpd_button(x, y, 1);  
	 //TPD_DOWN_DEBUG_TRACK(x,y);
 }
 
static  void tpd_up(int x, int y,int *count) {
	 //if(*count>0) {
		 //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
		 input_report_key(tpd->dev, BTN_TOUCH, 0);
		 //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
		 //printk("U[%4d %4d %4d] ", x, y, 0);
		 input_mt_sync(tpd->dev);
		 TPD_EM_PRINT(x, y, x, y, 0, 0);
	//	 (*count)--;
     if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
     {
        tpd_button(x, y, 0); 
     }   		 
        //tpd_button(x, y, 0); 

 }

 static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
 {

	int i = 0;

	char data[33] = {0};

    u16 high_byte,low_byte;

	p_point_num = point_num;

	i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(data[0]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(data[8]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(data[16]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(data[24]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &(data[32]));
	TPD_DEBUG("FW version=%x]\n",data[32]);

	//TPD_DEBUG("received raw data from touch panel as following:\n");
	//TPD_DEBUG("[data[0]=%x,data[1]= %x ,data[2]=%x ,data[3]=%x ,data[4]=%x ,data[5]=%x]\n",data[0],data[1],data[2],data[3],data[4],data[5]);
	//TPD_DEBUG("[data[9]=%x,data[10]= %x ,data[11]=%x ,data[12]=%x]\n",data[9],data[10],data[11],data[12]);
	//TPD_DEBUG("[data[15]=%x,data[16]= %x ,data[17]=%x ,data[18]=%x]\n",data[15],data[16],data[17],data[18]);

	
	/* Device Mode[2:0] == 0 :Normal operating Mode*/
	if(data[0] & 0x70 != 0) return false; 

	/*get the number of the touch points*/
	point_num= data[2] & 0x0f;
	
	//TPD_DEBUG("point_num =%d\n",point_num);
#ifdef TOUCH_PS
        if(PS_STATUS)
        {
            if(data[1] == 0xc0)
                touch_set_ps(0);
            else if(data[1] == 0xe0)
                touch_set_ps(1);
        }
#endif
	
//	if(point_num == 0) return false;

	   //TPD_DEBUG("Procss raw data...\n");

		
		for(i = 0; i < point_num; i++)
		{
			cinfo->p[i] = data[3+6*i] >> 6; //event flag 

	       /*get the X coordinate, 2 bytes*/
			high_byte = data[3+6*i];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[3+6*i + 1];
			cinfo->x[i] = high_byte |low_byte;

				//cinfo->x[i] =  cinfo->x[i] * 480 >> 11; //calibra
		
			/*get the Y coordinate, 2 bytes*/
			
			high_byte = data[3+6*i+2];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[3+6*i+3];
			cinfo->y[i] = high_byte |low_byte;

			  //cinfo->y[i]=  cinfo->y[i] * 800 >> 11;
		
			cinfo->count++;
			
		}
		//TPD_DEBUG(" cinfo->x[0] = %d, cinfo->y[0] = %d, cinfo->p[0] = %d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);	
		//TPD_DEBUG(" cinfo->x[1] = %d, cinfo->y[1] = %d, cinfo->p[1] = %d\n", cinfo->x[1], cinfo->y[1], cinfo->p[1]);		
		//TPD_DEBUG(" cinfo->x[2]= %d, cinfo->y[2]= %d, cinfo->p[2] = %d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2]);	
		  
	 return true;

 };

 static int touch_event_handler(void *unused)
 {

     struct touch_info cinfo, pinfo;
     memset(&cinfo, 0 ,sizeof(struct touch_info));
     memset(&pinfo, 0 ,sizeof(struct touch_info));

	 struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	 sched_setscheduler(current, SCHED_RR, &param);
 
	 do
	 {
	  mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
		 set_current_state(TASK_INTERRUPTIBLE); 
		  wait_event_interruptible(waiter,tpd_flag!=0);
						 
			 tpd_flag = 0;
			 
		 set_current_state(TASK_RUNNING);
		 

		  if (tpd_touchinfo(&cinfo, &pinfo)) {
		  //TPD_DEBUG("point_num = %d\n",point_num);
		  
                  if(point_num >0) {
                      tpd_down(cinfo.x[0], cinfo.y[0], 1);
                      if(point_num>1)
                      {
                          tpd_down(cinfo.x[1], cinfo.y[1], 1);
                          if(point_num >2) {
                              tpd_down(cinfo.x[2], cinfo.y[2], 1);
                              if(point_num > 3){
                                  tpd_down(cinfo.x[3], cinfo.y[3], 1);
                                  if(point_num > 4) tpd_down(cinfo.x[4], cinfo.y[4], 1);
                              }
                          }
                      }
                      input_sync(tpd->dev);
                      TPD_DEBUG("press --->\n");

                  } else  {
                      tpd_up(cinfo.x[0], cinfo.y[0], 0);
                      //TPD_DEBUG("release --->\n"); 
                      //input_mt_sync(tpd->dev);
                      input_sync(tpd->dev);
                  }
                  }

 }while(!kthread_should_stop());
 
	 return 0;
 }
 
 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
 {
	 strcpy(info->type, TPD_DEVICE);	
	  return 0;
 }
 
 static void tpd_eint_interrupt_handler(void)
 {
	 //TPD_DEBUG("TPD interrupt has been triggered\n");
	 tpd_flag = 1;
	 wake_up_interruptible(&waiter);
	 
 }
#ifdef TOUCH_PS
static void ft5306_ps_enable(bool val)
 {
     char buffer[2] = {0,0};
     buffer[0] = 0xb0;
     if(val)
     {
         if(ft_halt)
         {
             mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
             mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
             mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
             msleep(5);
#ifdef GPIO_CTP_EN_PIN
             mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
             mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
             mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
             msleep(5);
#endif
#ifdef TPD_POWER_SOURCE_CUSTOM
        hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
        hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
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
     PS_STATUS = val;
 }
#endif

 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {	 
	int retval = TPD_OK;
	char data;
	i2c_client = client;
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

        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
        msleep(5);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(200);


        mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
        mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
        mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
        mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);

        mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
        mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
        mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 0);
        mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
 
	msleep(100);
 
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
	   {
		   TPD_DMESG("I2C transfer error, line: %d\n", __LINE__);
		   return -1; 
	   }

	tpd_load_status = 1;

#ifdef VELOCITY_CUSTOM_FT5306
	int err;
	if((err = misc_register(&tpd_misc_device)))
	{
		printk("FT5306_tpd: tpd_misc_device register failed\n");
		
	}
#endif

	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	 if (IS_ERR(thread))
		 { 
		  retval = PTR_ERR(thread);
		  TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
		}
	 
#ifdef FT5306_APK_SUPPORT
			 ft5x0x_create_apk_debug_channel(client);
		 
			 ft5x0x_create_sysfs(client);
#endif

	TPD_DMESG("Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
   return 0;
   
 }

 static int __devexit tpd_remove(struct i2c_client *client)
 
 {
   
	 TPD_DEBUG("TPD removed\n");
#ifdef FT5306_APK_SUPPORT
	 ft5x0x_release_sysfs(client);
#endif
         //mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
         //mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
         //mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
   return 0;
 }
 
 
 static int tpd_local_init(void)
 {

 
  TPD_DMESG("Focaltech FT5206 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
 
 
   if(i2c_add_driver(&tpd_i2c_driver)!=0)
   	{
  		//TPD_DMESG("unable to add i2c driver.\n");
      	return -1;
    }
   if(tpd_load_status == 0)
   {
       //TPD_DMESG("add error touch panel driver.\n");
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

 static int tpd_resume(struct i2c_client *client)
 {
  int retval = TPD_OK;
 
   TPD_DEBUG("TPD wake up\n");
#ifdef TOUCH_PS
if(!PS_STATUS)
{
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
        msleep(5);
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
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        msleep(10);
   mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
    ft_halt = 0;
}
#else
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
        msleep(5);
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
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        msleep(10);
   mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
    ft_halt = 0;
#endif	
	 return retval;
 }
 
 static int tpd_suspend(struct i2c_client *client, pm_message_t message)
 {
	 int retval = TPD_OK;
	 static char data[2] = {0xa5,0x3};
 
	 TPD_DEBUG("TPD enter sleep\n");
#ifdef TOUCH_PS
         if(!PS_STATUS)
         {
             ft_halt = 1;
             mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
             //i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
             i2c_master_send(i2c_client,data,2);
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

         }
#else
         ft_halt = 1;
         mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
         //i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
         i2c_master_send(i2c_client,data,2);
#ifdef TPD_POWER_SOURCE_CUSTOM
             hwPowerDown(TPD_POWER_SOURCE_CUSTOM,"TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
             hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
        mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
#endif
	 return retval;
 } 


 static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "FT5306",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
#ifdef TOUCH_PS
	 .tpd_ps = ft5306_ps_enable,
#endif
 };
 
#ifdef VELOCITY_CUSTOM_FT5306
  static  int			  focaltech_release(struct inode *, struct file *);
  static  int			  focaltech_open(struct inode *, struct file *);
  static  ssize_t		  focaltech_write(struct file *file, const char *buf, size_t count, loff_t *ppos);
  static  ssize_t		  focaltech_read(struct file *file, char *buf, size_t count, loff_t *ppos);
  static  struct		  cdev focaltech_cdev;
  static  struct		  class *focaltech_class;
  static  int			  focaltech_major = 0;
  
  static int  focaltech_open(struct inode *inode, struct file *filp)
  {
		  printk("--------------------open-------------------\n");
		  return 0;
  }
  EXPORT_SYMBOL(focaltech_open);
  
  static int  focaltech_release(struct inode *inode, struct file *filp)
  {
		  return 0;
  }
  EXPORT_SYMBOL(focaltech_release);
  
  static ssize_t  focaltech_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
  {
		  int ret;
		  char *tmp;
  
		  printk("-------------------write---------------------%s\n",buf);
  
		  if (count > 8192)
				  count = 8192;
  
		  tmp = (char *)kmalloc(count,GFP_KERNEL);
		  if (tmp==NULL)
				  return -ENOMEM;
		  if (copy_from_user(tmp,buf,count)) {
				  kfree(tmp);
				  return -EFAULT;
		  }
  
		  ret = i2c_master_send(i2c_client, tmp, count);
		  kfree(tmp);
		  return ret;
  }
  EXPORT_SYMBOL(focaltech_write);
  
  static ssize_t  focaltech_read(struct file *file, char *buf, size_t count, loff_t *ppos)
  {
		  u8 fw_version =0;
		  int ret;
  
		  printk("------------------------read--------------------------\n");
  
		  ret = i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &fw_version);
  
		  printk("----CCI----%d \n",fw_version);
  
  //		ret = i2c_master_recv(i2c_client, tmp, count);
		  if (ret >= 0)
				  ret = copy_to_user(buf,&fw_version,1)?-EFAULT:ret;
		  return ret;
  }
  EXPORT_SYMBOL(focaltech_read);
  
  static struct file_operations nc_fops = {
		  .owner =		  THIS_MODULE,
		  .write		  = focaltech_write,
		  .read 		  = focaltech_read,
		  .open 		  = focaltech_open,
		 // .unlocked_ioctl = sitronix_ioctl,
		  .release		  = focaltech_release,
  };
#endif
 /* called when loaded into kernel */
 static int __init tpd_driver_init(void) {
	 printk("MediaTek FT5206 touch panel driver init\n");
	   i2c_register_board_info(0, &ft5306_i2c_tpd, 1);
		 if(tpd_driver_add(&tpd_device_driver) < 0)
			 TPD_DMESG("add FT5206 driver failed\n");
#ifdef VELOCITY_CUSTOM_FT5306
				  int result;
					  int err = 0;
			  
					  dev_t devno = MKDEV(focaltech_major, 0);
					  result  = alloc_chrdev_region(&devno, 0, 1, "focaltechDev");
					  if(result < 0){
						  printk("fail to allocate chrdev (%d) \n", result);
						  return 0;
					  }
					  focaltech_major = MAJOR(devno);
					  cdev_init(&focaltech_cdev, &nc_fops);
					  focaltech_cdev.owner = THIS_MODULE;
					  focaltech_cdev.ops = &nc_fops;
					  err =  cdev_add(&focaltech_cdev, devno, 1);
					  if(err){
						  printk("fail to add cdev (%d) \n", err);
						  return 0;
					  }
			  
					  focaltech_class = class_create(THIS_MODULE, "focaltechDev");
					  if (IS_ERR(focaltech_class)) {
						  result = PTR_ERR(focaltech_class);
						  unregister_chrdev(focaltech_major, "focaltechDev");
						  printk("fail to create class (%d) \n", result);
						  return result;
					  }
					  device_create(focaltech_class, NULL, MKDEV(focaltech_major, 0), NULL, "focaltechDev");
#endif
	 return 0;
 }
 
 /* should never be called */
 static void __exit tpd_driver_exit(void) {
	 TPD_DMESG("MediaTek FT5206 touch panel driver exit\n");
	 //input_unregister_device(tpd->dev);
	 tpd_driver_remove(&tpd_device_driver);
 }
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);


