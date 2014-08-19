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

/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "S5K3H2YXAF.h"
#include "../camera/kd_camera_hw.h"
//#include "kd_cust_lens.h"

//#include <mach/mt6573_pll.h>
//#include <mach/mt6573_gpt.h>
//#include <mach/mt6573_gpio.h>

#define LENS_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("S5K3H2YXAF", 0x20)};

#define S5K3H2YXAF_DRVNAME "S5K3H2YXAF"
#define S5K3H2YXAF_VCM_WRITE_ID           0x20

#define S5K3H2YXAF_DEBUG
#ifdef S5K3H2YXAF_DEBUG
#define S5K3H2YXAFDB printk
#else
#define S5K3H2YXAFDB(x,...)
#endif

static spinlock_t g_S5K3H2YXAF_SpinLock;
/* Kirby: remove old-style driver
static unsigned short g_pu2Normal_S5K3H2YXAF_i2c[] = {S5K3H2YXAF_VCM_WRITE_ID , I2C_CLIENT_END};
static unsigned short g_u2Ignore_S5K3H2YXAF = I2C_CLIENT_END;

static struct i2c_client_address_data g_stS5K3H2YXAF_Addr_data = {
    .normal_i2c = g_pu2Normal_S5K3H2YXAF_i2c,
    .probe = &g_u2Ignore_S5K3H2YXAF,
    .ignore = &g_u2Ignore_S5K3H2YXAF
};*/

static struct i2c_client * g_pstS5K3H2YXAF_I2Cclient = NULL;

static dev_t g_S5K3H2YXAF_devno;
static struct cdev * g_pS5K3H2YXAF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4S5K3H2YXAF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static long g_i4Position = 0;
static unsigned long g_u4S5K3H2YXAF_INF = 0;
static unsigned long g_u4S5K3H2YXAF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;
//static struct work_struct g_stWork;     // --- Work queue ---
//static XGPT_CONFIG	g_GPTconfig;		// --- Interrupt Config ---


extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);


static int s4S5K3H2YXAF_ReadReg(unsigned short * a_pu2Result)
{
#if 1//for AD5823
	int  i4RetValue1 = 0,i4RetValue2 = 0,i4RetValue3 = 0,i4RetValue4 = 0;
	char pBuff1[1]={0x04},pBuff2[1]={0x05},pBuff3[1],pBuff4[1];
	g_pstS5K3H2YXAF_I2Cclient->addr=0x0c;
	i4RetValue1 = i2c_master_send(g_pstS5K3H2YXAF_I2Cclient, pBuff1, 1);

	i4RetValue2 = i2c_master_recv(g_pstS5K3H2YXAF_I2Cclient, pBuff3 , 1);

	i4RetValue3 = i2c_master_send(g_pstS5K3H2YXAF_I2Cclient, pBuff2, 1);

	i4RetValue4 = i2c_master_recv(g_pstS5K3H2YXAF_I2Cclient, pBuff4 , 1);
	S5K3H2YXAFDB("[S5K3H2YXAF] I2C read i4RetValue1 = %d,i4RetValue2 = %d,i4RetValue3 = %d,i4RetValue4 = %d\n",i4RetValue1,i4RetValue2,i4RetValue3,i4RetValue4);

	if ((i4RetValue1 < 0)||(i4RetValue2 < 0)||(i4RetValue3 < 0)||(i4RetValue4 < 0)) 
	{
		S5K3H2YXAFDB("[S5K3H2YXAF] I2C read failed!! \n");
		return -1;
	}

	//*a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);
	*a_pu2Result = (((u16)pBuff3[0]) << 8)&0xFF00 + (pBuff4[0]);


	return 0;

#endif
#if 0//for AD5820
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstS5K3H2YXAF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0) 
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] I2C read failed!! \n");
        return -1;
    }

    *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

    return 0;
#endif
}

static int s4S5K3H2YXAF_WriteReg(u16 a_u2Data)
{
#if 1//for AD5823
int  i4RetValue1 = 0,i4RetValue2 = 0;
 S5K3H2YXAFDB("[S5K3H2YXAF] AFPosition= %d\n",a_u2Data);

 //char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+0xF)};
 char puSendCmd1[2] = {(char)(0x04) , (char)((a_u2Data & 0x0300)>> 8)};
 char puSendCmd2[2] = {(char)(0x05) , (char)(a_u2Data & 0x00FF)};
 	//mt_set_gpio_out(97,1);
	//i4RetValue = i2c_master_send(g_pstS5K3H2YXAF_I2Cclient, puSendCmd, 2);
	g_pstS5K3H2YXAF_I2Cclient->addr=0x0c;
	i4RetValue1 = i2c_master_send(g_pstS5K3H2YXAF_I2Cclient, puSendCmd1, 2);
	i4RetValue2 = i2c_master_send(g_pstS5K3H2YXAF_I2Cclient, puSendCmd2, 2);

	//mt_set_gpio_out(97,0);

	if ((i4RetValue1 < 0)||(i4RetValue2 < 0)) 
	{
		S5K3H2YXAFDB("[S5K3H2YXAF] I2C send failed!! \n");
		return -1;
	}

    return 0;

#endif
#if 0//for AD5820
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+0xF)};

	//mt_set_gpio_out(97,1);
    i4RetValue = i2c_master_send(g_pstS5K3H2YXAF_I2Cclient, puSendCmd, 2);
	//mt_set_gpio_out(97,0);
	
    if (i4RetValue < 0) 
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] I2C send failed!! \n");
        return -1;
    }

    return 0;
#endif
}

inline static int getS5K3H2YXAFInfo(__user stS5K3H2YXAF_MotorInfo * pstMotorInfo)
{
    stS5K3H2YXAF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4S5K3H2YXAF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4S5K3H2YXAF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = TRUE;}
	else						{stMotorInfo.bIsMotorMoving = FALSE;}

	if (g_s4S5K3H2YXAF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = TRUE;}
	else						{stMotorInfo.bIsMotorOpen = FALSE;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stS5K3H2YXAF_MotorInfo)))
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveS5K3H2YXAF(unsigned long a_u4Position)
{
	S5K3H2YXAFDB("[S5K3H2YXAF] a_u4Position = %d \n", a_u4Position);
    if((a_u4Position > g_u4S5K3H2YXAF_MACRO) || (a_u4Position < g_u4S5K3H2YXAF_INF))
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] out of range \n");
        return -EINVAL;
    }

	if (g_s4S5K3H2YXAF_Opened == 1)
	{
		unsigned short InitPos;
	
		if(s4S5K3H2YXAF_ReadReg(&InitPos) == 0)
		{
			S5K3H2YXAFDB("[S5K3H2YXAF] Init Pos %6d \n", InitPos);
		
			g_u4CurrPosition = (unsigned long)InitPos;
		}
		else
		{
			g_u4CurrPosition = 0;
		}
		
		g_s4S5K3H2YXAF_Opened = 2;
	}
	S5K3H2YXAFDB("[S5K3H2YXAF] g_u4CurrPosition = %d \n", g_u4CurrPosition);

	if      (g_u4CurrPosition < a_u4Position)	{g_i4Dir = 1;}
	else if (g_u4CurrPosition > a_u4Position)	{g_i4Dir = -1;}
	else										{return 0;}
	S5K3H2YXAFDB("[S5K3H2YXAF] g_i4Dir = %d \n", g_i4Dir);

	if (1)
	{
		g_i4Position = (long)g_u4CurrPosition;
		g_u4TargetPosition = a_u4Position;

		if (g_i4Dir == 1)
		{
			//if ((g_u4TargetPosition - g_u4CurrPosition)<60)
			{		
				g_i4MotorStatus = 0;
				if(s4S5K3H2YXAF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
				{
					g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				}
				else
				{
					S5K3H2YXAFDB("[S5K3H2YXAF] set I2C failed when moving the motor \n");
					g_i4MotorStatus = -1;
				}
			}
			//else
			//{
			//	g_i4MotorStatus = 1;
			//}
		}
		else if (g_i4Dir == -1)
		{
			//if ((g_u4CurrPosition - g_u4TargetPosition)<60)
			{
				g_i4MotorStatus = 0;		
				if(s4S5K3H2YXAF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
				{
					g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				}
				else
				{
					S5K3H2YXAFDB("[S5K3H2YXAF] set I2C failed when moving the motor \n");
					g_i4MotorStatus = -1;
				}
			}
			//else
			//{
			//	g_i4MotorStatus = 1;		
			//}
		}
	}
	else
	{
	g_i4Position = (long)g_u4CurrPosition;
	g_u4TargetPosition = a_u4Position;
	g_i4MotorStatus = 1;
	}

    return 0;
}

inline static int setS5K3H2YXAFInf(unsigned long a_u4Position)
{
	g_u4S5K3H2YXAF_INF = a_u4Position;
	return 0;
}

inline static int setS5K3H2YXAFMacro(unsigned long a_u4Position)
{
	g_u4S5K3H2YXAF_MACRO = a_u4Position;
	return 0;	
}

////////////////////////////////////////////////////////////////
static long S5K3H2YXAF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case S5K3H2YXAFIOC_G_MOTORINFO :
            i4RetValue = getS5K3H2YXAFInfo((__user stS5K3H2YXAF_MotorInfo *)(a_u4Param));
        break;

        case S5K3H2YXAFIOC_T_MOVETO :
            i4RetValue = moveS5K3H2YXAF(a_u4Param);
        break;
 
 		case S5K3H2YXAFIOC_T_SETINFPOS :
			 i4RetValue = setS5K3H2YXAFInf(a_u4Param);
		break;

 		case S5K3H2YXAFIOC_T_SETMACROPOS :
			 i4RetValue = setS5K3H2YXAFMacro(a_u4Param);
		break;
		
        default :
      	     S5K3H2YXAFDB("[S5K3H2YXAF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}
/*
static void S5K3H2YXAF_WORK(struct work_struct *work)
{
    g_i4Position += (25 * g_i4Dir);

    if ((g_i4Dir == 1) && (g_i4Position >= (long)g_u4TargetPosition))
	{
        g_i4Position = (long)g_u4TargetPosition;
        g_i4MotorStatus = 0;
    }

    if ((g_i4Dir == -1) && (g_i4Position <= (long)g_u4TargetPosition))
    {
        g_i4Position = (long)g_u4TargetPosition;
        g_i4MotorStatus = 0; 		
    }
	
    if(s4S5K3H2YXAF_WriteReg((unsigned short)g_i4Position) == 0)
    {
        g_u4CurrPosition = (unsigned long)g_i4Position;
    }
    else
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] set I2C failed when moving the motor \n");
        g_i4MotorStatus = -1;
    }
}

static void S5K3H2YXAF_ISR(UINT16 a_input)
{
	if (g_i4MotorStatus == 1)
	{	
		schedule_work(&g_stWork);		
	}
}
*/
//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int S5K3H2YXAF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    spin_lock(&g_S5K3H2YXAF_SpinLock);
	S5K3H2YXAFDB("[S5K3H2YXAF]opened \n");

    if(g_s4S5K3H2YXAF_Opened)
    {
        spin_unlock(&g_S5K3H2YXAF_SpinLock);
        S5K3H2YXAFDB("[S5K3H2YXAF] the device is opened \n");
        return -EBUSY;
    }

    g_s4S5K3H2YXAF_Opened = 1;
		
    spin_unlock(&g_S5K3H2YXAF_SpinLock);

	// --- Config Interrupt ---
	//g_GPTconfig.num = XGPT7;
	//g_GPTconfig.mode = XGPT_REPEAT;
	//g_GPTconfig.clkDiv = XGPT_CLK_DIV_1;//32K
	//g_GPTconfig.u4Compare = 32*2; // 2ms
	//g_GPTconfig.bIrqEnable = TRUE;
	
	//XGPT_Reset(g_GPTconfig.num);	
	//XGPT_Init(g_GPTconfig.num, S5K3H2YXAF_ISR);

	//if (XGPT_Config(g_GPTconfig) == FALSE)
	//{
        //S5K3H2YXAFDB("[S5K3H2YXAF] ISR Config Fail\n");	
	//	return -EPERM;
	//}

	//XGPT_Start(g_GPTconfig.num);		

	// --- WorkQueue ---	
	//INIT_WORK(&g_stWork,S5K3H2YXAF_WORK);

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int S5K3H2YXAF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
	unsigned int cnt = 0;
	S5K3H2YXAFDB("[S5K3H2YXAF]release \n");

	if (g_s4S5K3H2YXAF_Opened)
	{
	#if 0
		moveS5K3H2YXAF(g_u4S5K3H2YXAF_INF);
	#else
		moveS5K3H2YXAF(200);
		msleep(20);
		moveS5K3H2YXAF(100);
		msleep(20);
		moveS5K3H2YXAF(50);
		msleep(10);
		
	#endif

		while(g_i4MotorStatus)
		{
			msleep(1);
			cnt++;
			S5K3H2YXAFDB("cnt0 = %d\n",cnt);
			if (cnt>200)	{break;}
		}
		S5K3H2YXAFDB("cnt1 = %d\n",cnt);
    	spin_lock(&g_S5K3H2YXAF_SpinLock);

	    g_s4S5K3H2YXAF_Opened = 0;

    	spin_unlock(&g_S5K3H2YXAF_SpinLock);

    	//hwPowerDown(CAMERA_POWER_VCAM_A,"kd_camera_hw");

		//XGPT_Stop(g_GPTconfig.num);
	}

    return 0;
}

static const struct file_operations g_stS5K3H2YXAF_fops = 
{
    .owner = THIS_MODULE,
    .open = S5K3H2YXAF_Open,
    .release = S5K3H2YXAF_Release,
    .unlocked_ioctl = S5K3H2YXAF_Ioctl
};

inline static int Register_S5K3H2YXAF_CharDrv(void)
{
    struct device* vcm_device = NULL;
    S5K3H2YXAFDB("[S5K3H2YXAF] Register_S5K3H2YXAF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_S5K3H2YXAF_devno, 0, 1,S5K3H2YXAF_DRVNAME) )
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pS5K3H2YXAF_CharDrv = cdev_alloc();

    if(NULL == g_pS5K3H2YXAF_CharDrv)
    {
        unregister_chrdev_region(g_S5K3H2YXAF_devno, 1);

        S5K3H2YXAFDB("[S5K3H2YXAF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pS5K3H2YXAF_CharDrv, &g_stS5K3H2YXAF_fops);

    g_pS5K3H2YXAF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pS5K3H2YXAF_CharDrv, g_S5K3H2YXAF_devno, 1))
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] Attatch file operation failed\n");

        unregister_chrdev_region(g_S5K3H2YXAF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv0");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        S5K3H2YXAFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_S5K3H2YXAF_devno, NULL, S5K3H2YXAF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    S5K3H2YXAFDB("[S5K3H2YXAF] Register_S5K3H2YXAF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_S5K3H2YXAF_CharDrv(void)
{
    S5K3H2YXAFDB("[S5K3H2YXAF] Unregister_S5K3H2YXAF_CharDrv - Start\n");
    //Release char driver
    cdev_del(g_pS5K3H2YXAF_CharDrv);

    unregister_chrdev_region(g_S5K3H2YXAF_devno, 1);
    
    device_destroy(actuator_class, g_S5K3H2YXAF_devno);

    class_destroy(actuator_class);
    S5K3H2YXAFDB("[S5K3H2YXAF] Unregister_S5K3H2YXAF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////
/* Kirby: remove old-style driver
static int S5K3H2YXAF_i2c_attach(struct i2c_adapter * a_pstAdapter);
static int S5K3H2YXAF_i2c_detach_client(struct i2c_client * a_pstClient);
static struct i2c_driver S5K3H2YXAF_i2c_driver = {
    .driver = {
    .name = S5K3H2YXAF_DRVNAME,
    },
    //.attach_adapter = S5K3H2YXAF_i2c_attach,
    //.detach_client = S5K3H2YXAF_i2c_detach_client
};*/

/* Kirby: add new-style driver { */
//static int S5K3H2YXAF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int S5K3H2YXAF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int S5K3H2YXAF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id S5K3H2YXAF_i2c_id[] = {{S5K3H2YXAF_DRVNAME,0},{}};   
//static unsigned short force[] = {IMG_SENSOR_I2C_GROUP_ID, S5K3H2YXAF_VCM_WRITE_ID, I2C_CLIENT_END, I2C_CLIENT_END};   
//static const unsigned short * const forces[] = { force, NULL };              
//static struct i2c_client_address_data addr_data = { .forces = forces,}; 
struct i2c_driver S5K3H2YXAF_i2c_driver = {                       
    .probe = S5K3H2YXAF_i2c_probe,                                   
    .remove = S5K3H2YXAF_i2c_remove,                           
    .driver.name = S5K3H2YXAF_DRVNAME,                 
    .id_table = S5K3H2YXAF_i2c_id,                             
};  

#if 0 
static int S5K3H2YXAF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, S5K3H2YXAF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int S5K3H2YXAF_i2c_remove(struct i2c_client *client) {
    return 0;
}
/* Kirby: } */


/* Kirby: remove old-style driver
int S5K3H2YXAF_i2c_foundproc(struct i2c_adapter * a_pstAdapter, int a_i4Address, int a_i4Kind)
*/
/* Kirby: add new-style driver {*/
static int S5K3H2YXAF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
/* Kirby: } */
{
    int i4RetValue = 0;

    S5K3H2YXAFDB("[S5K3H2YXAF] Attach I2C \n");

    /* Kirby: remove old-style driver
    //Check I2C driver capability
    if (!i2c_check_functionality(a_pstAdapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] I2C port cannot support the format \n");
        return -EPERM;
    }

    if (!(g_pstS5K3H2YXAF_I2Cclient = kzalloc(sizeof(struct i2c_client), GFP_KERNEL)))
    {
        return -ENOMEM;
    }

    g_pstS5K3H2YXAF_I2Cclient->addr = a_i4Address;
    g_pstS5K3H2YXAF_I2Cclient->adapter = a_pstAdapter;
    g_pstS5K3H2YXAF_I2Cclient->driver = &S5K3H2YXAF_i2c_driver;
    g_pstS5K3H2YXAF_I2Cclient->flags = 0;

    strncpy(g_pstS5K3H2YXAF_I2Cclient->name, S5K3H2YXAF_DRVNAME, I2C_NAME_SIZE);

    if(i2c_attach_client(g_pstS5K3H2YXAF_I2Cclient))
    {
        kfree(g_pstS5K3H2YXAF_I2Cclient);
    }
    */
    /* Kirby: add new-style driver { */
    g_pstS5K3H2YXAF_I2Cclient = client;
    
    g_pstS5K3H2YXAF_I2Cclient->addr = g_pstS5K3H2YXAF_I2Cclient->addr >> 1;
    
    /* Kirby: } */

    //Register char driver
    i4RetValue = Register_S5K3H2YXAF_CharDrv();

    if(i4RetValue){

        S5K3H2YXAFDB("[S5K3H2YXAF] register char device failed!\n");

        /* Kirby: remove old-style driver
        kfree(g_pstS5K3H2YXAF_I2Cclient); */

        return i4RetValue;
    }

    spin_lock_init(&g_S5K3H2YXAF_SpinLock);

    S5K3H2YXAFDB("[S5K3H2YXAF] Attached!! \n");

    return 0;
}

/* Kirby: remove old-style driver
static int S5K3H2YXAF_i2c_attach(struct i2c_adapter * a_pstAdapter)
{

    if(a_pstAdapter->id == 0)
    {
    	 return i2c_probe(a_pstAdapter, &g_stS5K3H2YXAF_Addr_data ,  S5K3H2YXAF_i2c_foundproc);
    }

    return -1;

}

static int S5K3H2YXAF_i2c_detach_client(struct i2c_client * a_pstClient)
{
    int i4RetValue = 0;

    Unregister_S5K3H2YXAF_CharDrv();

    //detach client
    i4RetValue = i2c_detach_client(a_pstClient);
    if(i4RetValue)
    {
        dev_err(&a_pstClient->dev, "Client deregistration failed, client not detached.\n");
        return i4RetValue;
    }

    kfree(i2c_get_clientdata(a_pstClient));

    return 0;
}*/

static int S5K3H2YXAF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&S5K3H2YXAF_i2c_driver);
}

static int S5K3H2YXAF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&S5K3H2YXAF_i2c_driver);
    return 0;
}

static int S5K3H2YXAF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
//    int retVal = 0;
//    retVal = hwPowerDown(MT6516_POWER_VCAM_A,S5K3H2YXAF_DRVNAME);

    return 0;
}

static int S5K3H2YXAF_resume(struct platform_device *pdev)
{
/*
    if(TRUE != hwPowerOn(MT6516_POWER_VCAM_A, VOL_2800,S5K3H2YXAF_DRVNAME))
    {
        S5K3H2YXAFDB("[S5K3H2YXAF] failed to resume S5K3H2YXAF\n");
        return -EIO;
    }
*/
    return 0;
}

// platform structure
static struct platform_driver g_stS5K3H2YXAF_Driver = {
    .probe		= S5K3H2YXAF_probe,
    .remove	= S5K3H2YXAF_remove,
    .suspend	= S5K3H2YXAF_suspend,
    .resume	= S5K3H2YXAF_resume,
    .driver		= {
        .name	= "lens_actuator0",
        .owner	= THIS_MODULE,
    }
};

static int __init S5K3H2YXAF_i2C_init(void)
{
	 i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	 S5K3H2YXAFDB("register S5K3H2YXAF driver\n");
    if(platform_driver_register(&g_stS5K3H2YXAF_Driver)){
        S5K3H2YXAFDB("failed to register S5K3H2YXAF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit S5K3H2YXAF_i2C_exit(void)
{
	platform_driver_unregister(&g_stS5K3H2YXAF_Driver);
}

module_init(S5K3H2YXAF_i2C_init);
module_exit(S5K3H2YXAF_i2C_exit);

MODULE_DESCRIPTION("S5K3H2YXAF lens module driver");
MODULE_AUTHOR("Gipi Lin <Gipi.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");


