/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2005
 *
 *  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
 *  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 *  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 *  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 *  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
 *  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
 *  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 *  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 *  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 *  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
 *  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
 *  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
 *  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
 *  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
 *  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   gc0308yuv_Sensor.c
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *   V1.2.3
 *
 * Author:
 * -------
 *   Leo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2012.02.29  kill bugs
 *   
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *=============================================================
 ******************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "gc0308yuv_Sensor.h"
#include "gc0308yuv_Camera_Sensor_para.h"
#include "gc0308yuv_CameraCustomized.h"

//#define GC0308YUV_DEBUG
#ifdef GC0308YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 GC0308_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 2, GC0308_WRITE_ID);

}
kal_uint16 GC0308_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte, 1, GC0308_READ_ID);
	
    return get_byte;
}


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   GC0308_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 GC0308_dummy_pixels = 0, GC0308_dummy_lines = 0;
kal_bool   GC0308_MODE_CAPTURE = KAL_FALSE;
kal_bool   GC0308_NIGHT_MODE = KAL_FALSE;

kal_uint32 GC0308_isp_master_clock;
static kal_uint32 GC0308_g_fPV_PCLK = 24;

kal_uint8 GC0308_sensor_write_I2C_address = GC0308_WRITE_ID;
kal_uint8 GC0308_sensor_read_I2C_address = GC0308_READ_ID;

UINT8 GC0308PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT GC0308SensorConfigData;

#define GC0308_SET_PAGE0 	GC0308_write_cmos_sensor(0xfe, 0x00)
#define GC0308_SET_PAGE1 	GC0308_write_cmos_sensor(0xfe, 0x01)



/*************************************************************************
 * FUNCTION
 *	GC0308_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of GC0308 to change exposure time.
 *
 * PARAMETERS
 *   iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0308_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_GC0308_Shutter */


/*************************************************************************
 * FUNCTION
 *	GC0308_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of GC0308 .
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0308_Read_Shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	temp_reg1 = GC0308_read_cmos_sensor(0x04);
	temp_reg2 = GC0308_read_cmos_sensor(0x03);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	return shutter;
} /* GC0308_read_shutter */


/*************************************************************************
 * FUNCTION
 *	GC0308_write_reg
 *
 * DESCRIPTION
 *	This function set the register of GC0308.
 *
 * PARAMETERS
 *	addr : the register index of GC0308
 *  para : setting parameter of the specified register of GC0308
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0308_write_reg(kal_uint32 addr, kal_uint32 para)
{
	GC0308_write_cmos_sensor(addr, para);
} /* GC0308_write_reg() */


/*************************************************************************
 * FUNCTION
 *	GC0308_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from GC0308.
 *
 * PARAMETERS
 *	addr : the register index of GC0308
 *
 * RETURNS
 *	the data that read from GC0308
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 GC0308_read_reg(kal_uint32 addr)
{
	return GC0308_read_cmos_sensor(addr);
} /* OV7670_read_reg() */


/*************************************************************************
* FUNCTION
*	GC0308_AWB_enable
*
* DESCRIPTION
*	This function enable or disable the awb (Auto White Balance).
*
* PARAMETERS
*	1. kal_bool : KAL_TRUE - enable awb, KAL_FALSE - disable awb.
*
* RETURNS
*	kal_bool : It means set awb right or not.
*
*************************************************************************/


static void GC0308_AWB_enable(kal_bool AWB_enable)
{	 
	kal_uint16 temp_AWB_reg = 0;

	temp_AWB_reg = GC0308_read_cmos_sensor(0x22);
	
	if (AWB_enable == KAL_TRUE)
	{
		GC0308_write_cmos_sensor(0x22, (temp_AWB_reg |0x02));
	}
	else
	{
		GC0308_write_cmos_sensor(0x22, (temp_AWB_reg & (~0x02)));
	}

}




static void  GC0308_set_AE_mode(kal_bool AE_enable)
{
       kal_uint8 temp_AE_reg = 0;
	   
	temp_AE_reg = GC0308_read_cmos_sensor(0xd2);
	
   if (AE_enable == KAL_TRUE)
 	{
  	
		 GC0308_write_cmos_sensor(0xd2, (temp_AE_reg | 0x80));          
 	}
  else
  	{
   		 GC0308_write_cmos_sensor(0xd2, (temp_AE_reg & (~0x80)));
  	}

}

/*************************************************************************
 * FUNCTION
 *	GC0308_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of GC0308 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from GC0308
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0308_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* GC0308_config_window */


/*************************************************************************
 * FUNCTION
 *	GC0308_SetGain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *   iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0308_SetGain(kal_uint16 iGain)
{
	return iGain;
}


/*************************************************************************
 * FUNCTION
 *	GC0308_NightMode
 *
 * DESCRIPTION
 *	This function night mode of GC0308.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0308NightMode(kal_bool bEnable)
{
	if (bEnable)
	{	

		if(GC0308_MPEG4_encode_mode == KAL_TRUE)
			GC0308_write_cmos_sensor(0xec, 0x00);
		else
			GC0308_write_cmos_sensor(0xec, 0x30);
		GC0308_NIGHT_MODE = KAL_TRUE;
	}
	else 
	{
		if(GC0308_MPEG4_encode_mode == KAL_TRUE)
			GC0308_write_cmos_sensor(0xec, 0x00);
		else
			GC0308_write_cmos_sensor(0xec, 0x20);
		GC0308_NIGHT_MODE = KAL_FALSE;
	}
} /* GC0308_NightMode */

/*************************************************************************
* FUNCTION
*	GC0308_Sensor_Init
*
* DESCRIPTION
*	This function apply all of the initial setting to sensor.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
*************************************************************************/
void GC0308_Sensor_Init(void)
{
	GC0308_write_cmos_sensor(0xfe , 0x80);   	
		
	GC0308_SET_PAGE0;       // set page0

		
	GC0308_write_cmos_sensor(0xd2 , 0x10);   // close AEC
	GC0308_write_cmos_sensor(0x22 , 0x55);   // close AWB

	GC0308_write_cmos_sensor(0x5a , 0x56); 
	GC0308_write_cmos_sensor(0x5b , 0x40);
	GC0308_write_cmos_sensor(0x5c , 0x4a);			

	GC0308_write_cmos_sensor(0x22 , 0x57);  // Open AWB
	GC0308_write_cmos_sensor(0x01  ,0x26);
	GC0308_write_cmos_sensor(0x02  ,0x98);
	GC0308_write_cmos_sensor(0x0f  ,0x03);

	GC0308_write_cmos_sensor(0x03 , 0x01);
	GC0308_write_cmos_sensor(0x04 , 0x90);
	GC0308_write_cmos_sensor(0xe2  ,0x00); 	//anti-flicker step [11:8]
	GC0308_write_cmos_sensor(0xe3  ,0x50);   //anti-flicker step [7:0]

	GC0308_write_cmos_sensor(0xe4  ,0x02);   //exp level 0  12.5fps
	GC0308_write_cmos_sensor(0xe5  ,0x80);
	GC0308_write_cmos_sensor(0xe6  ,0x03);   //exp level 1  10fps
	GC0308_write_cmos_sensor(0xe7  ,0x20);
	GC0308_write_cmos_sensor(0xe8  ,0x04);   //exp level 2  7.69fps
	GC0308_write_cmos_sensor(0xe9  ,0x10);
	GC0308_write_cmos_sensor(0xea  ,0x06);   //exp level 3  5.00fps
	GC0308_write_cmos_sensor(0xeb  ,0x40);
/*
	GC0308_write_cmos_sensor(0x01 , 0xfa); 
	GC0308_write_cmos_sensor(0x02 , 0x70); 
	GC0308_write_cmos_sensor(0x0f , 0x01); 

	GC0308_write_cmos_sensor(0x03 , 0x01); 
	GC0308_write_cmos_sensor(0x04 , 0x2c); 

	GC0308_write_cmos_sensor(0xe2 , 0x00); 	//anti-flicker step [11:8]
	GC0308_write_cmos_sensor(0xe3 , 0x64);   //anti-flicker step [7:0]
		
	GC0308_write_cmos_sensor(0xe4 , 0x02);   //exp level 0  16.67fps
	GC0308_write_cmos_sensor(0xe5 , 0x58); 
	GC0308_write_cmos_sensor(0xe6 , 0x03);   //exp level 1  12.5fps
	GC0308_write_cmos_sensor(0xe7 , 0x20); 
	GC0308_write_cmos_sensor(0xe8 , 0x04);   //exp level 2  8.33fps
	GC0308_write_cmos_sensor(0xe9 , 0xb0); 
	GC0308_write_cmos_sensor(0xea , 0x09);   //exp level 3  4.00fps
	GC0308_write_cmos_sensor(0xeb , 0xc4);
*/
	GC0308_write_cmos_sensor(0x05 , 0x00);
	GC0308_write_cmos_sensor(0x06 , 0x00);
	GC0308_write_cmos_sensor(0x07 , 0x00);
	GC0308_write_cmos_sensor(0x08 , 0x00);
	GC0308_write_cmos_sensor(0x09 , 0x01);
	GC0308_write_cmos_sensor(0x0a , 0xe8);
	GC0308_write_cmos_sensor(0x0b , 0x02);
	GC0308_write_cmos_sensor(0x0c , 0x88);
	GC0308_write_cmos_sensor(0x0d , 0x02);
	GC0308_write_cmos_sensor(0x0e , 0x02);
	GC0308_write_cmos_sensor(0x10 , 0x22);
	GC0308_write_cmos_sensor(0x11 , 0xfd);
	GC0308_write_cmos_sensor(0x12 , 0x2a);
	GC0308_write_cmos_sensor(0x13 , 0x00);
	
	if(0 == strncmp(VANZO_SUB_CAM_ROTATION, "180", 3))
		GC0308_write_cmos_sensor(0x14 , 0x13);
else
		GC0308_write_cmos_sensor(0x14 , 0x10);

	GC0308_write_cmos_sensor(0x15 , 0x0a);
	GC0308_write_cmos_sensor(0x16 , 0x05);
	GC0308_write_cmos_sensor(0x17 , 0x01);
	GC0308_write_cmos_sensor(0x18 , 0x44);
	GC0308_write_cmos_sensor(0x19 , 0x44);
	GC0308_write_cmos_sensor(0x1a , 0x1e);
	GC0308_write_cmos_sensor(0x1b , 0x00);
	GC0308_write_cmos_sensor(0x1c , 0xc1);
	GC0308_write_cmos_sensor(0x1d , 0x08);
	GC0308_write_cmos_sensor(0x1e , 0x60);
	GC0308_write_cmos_sensor(0x1f , 0x2a);//16

	
	GC0308_write_cmos_sensor(0x20 , 0xff);
	GC0308_write_cmos_sensor(0x21 , 0xf8);
	GC0308_write_cmos_sensor(0x22 , 0x57);
	GC0308_write_cmos_sensor(0x24 , 0xa0);
	GC0308_write_cmos_sensor(0x25 , 0x0f);
	                         
	//output sync_mode       
	GC0308_write_cmos_sensor(0x26 , 0x02);
	GC0308_write_cmos_sensor(0x2f , 0x01);
	GC0308_write_cmos_sensor(0x30 , 0xf7);
	GC0308_write_cmos_sensor(0x31 , 0x50);
	GC0308_write_cmos_sensor(0x32 , 0x00);
	GC0308_write_cmos_sensor(0x39 , 0x04);
	GC0308_write_cmos_sensor(0x3a , 0x18);
	GC0308_write_cmos_sensor(0x3b , 0x20);
	GC0308_write_cmos_sensor(0x3c , 0x00);
	GC0308_write_cmos_sensor(0x3d , 0x00);
	GC0308_write_cmos_sensor(0x3e , 0x00);
	GC0308_write_cmos_sensor(0x3f , 0x00);
	GC0308_write_cmos_sensor(0x50 , 0x16);
	GC0308_write_cmos_sensor(0x53 , 0x82);
	GC0308_write_cmos_sensor(0x54 , 0x80);
	GC0308_write_cmos_sensor(0x55 , 0x80);
	GC0308_write_cmos_sensor(0x56 , 0x82);
	GC0308_write_cmos_sensor(0x8b , 0x40);
	GC0308_write_cmos_sensor(0x8c , 0x40);
	GC0308_write_cmos_sensor(0x8d , 0x40);
	GC0308_write_cmos_sensor(0x8e , 0x2e);
	GC0308_write_cmos_sensor(0x8f , 0x2e);
	GC0308_write_cmos_sensor(0x90 , 0x2e);
	GC0308_write_cmos_sensor(0x91 , 0x3c);
	GC0308_write_cmos_sensor(0x92 , 0x50);
	GC0308_write_cmos_sensor(0x5d , 0x12);
	GC0308_write_cmos_sensor(0x5e , 0x1a);
	GC0308_write_cmos_sensor(0x5f , 0x24);
	GC0308_write_cmos_sensor(0x60 , 0x07);
	GC0308_write_cmos_sensor(0x61 , 0x15);
	GC0308_write_cmos_sensor(0x62 , 0x08);
	GC0308_write_cmos_sensor(0x64 , 0x03);
	GC0308_write_cmos_sensor(0x66 , 0xe8);
	GC0308_write_cmos_sensor(0x67 , 0x86);
	GC0308_write_cmos_sensor(0x68 , 0xa2);
	GC0308_write_cmos_sensor(0x69 , 0x18);
	GC0308_write_cmos_sensor(0x6a , 0x0f);
	GC0308_write_cmos_sensor(0x6b , 0x00);
	GC0308_write_cmos_sensor(0x6c , 0x5f);
	GC0308_write_cmos_sensor(0x6d , 0x8f);
	GC0308_write_cmos_sensor(0x6e , 0x55);
	GC0308_write_cmos_sensor(0x6f , 0x38);
	GC0308_write_cmos_sensor(0x70 , 0x15);
	GC0308_write_cmos_sensor(0x71 , 0x33);
	GC0308_write_cmos_sensor(0x72 , 0xdc);
	GC0308_write_cmos_sensor(0x73 , 0x80);
	GC0308_write_cmos_sensor(0x74 , 0x02);
	GC0308_write_cmos_sensor(0x75 , 0x3f);
	GC0308_write_cmos_sensor(0x76 , 0x02);
	GC0308_write_cmos_sensor(0x77 , 0x36);
	GC0308_write_cmos_sensor(0x78 , 0x88);
	GC0308_write_cmos_sensor(0x79 , 0x81);
	GC0308_write_cmos_sensor(0x7a , 0x81);
	GC0308_write_cmos_sensor(0x7b , 0x22);
	GC0308_write_cmos_sensor(0x7c , 0xff);
	GC0308_write_cmos_sensor(0x93 , 0x3c);
	GC0308_write_cmos_sensor(0x94 , 0x00);
	GC0308_write_cmos_sensor(0x95 , 0x07);
	GC0308_write_cmos_sensor(0x96 , 0xe8);
	GC0308_write_cmos_sensor(0x97 , 0x40);
	GC0308_write_cmos_sensor(0x98 , 0xf0);
	GC0308_write_cmos_sensor(0xb1 , 0x38);
	GC0308_write_cmos_sensor(0xb2 , 0x38);
	GC0308_write_cmos_sensor(0xbd , 0x38);
	GC0308_write_cmos_sensor(0xbe , 0x36);
	GC0308_write_cmos_sensor(0xd0 , 0xc9);
	GC0308_write_cmos_sensor(0xd1 , 0x10);
	//GC0308_write_cmos_sensor(0xd2 , 0x90);
	GC0308_write_cmos_sensor(0xd3 , 0x80);
	GC0308_write_cmos_sensor(0xd5 , 0xf2);
	GC0308_write_cmos_sensor(0xd6 , 0x16);
	GC0308_write_cmos_sensor(0xdb , 0x92);
	GC0308_write_cmos_sensor(0xdc , 0xa5);
	GC0308_write_cmos_sensor(0xdf , 0x23);
	GC0308_write_cmos_sensor(0xd9 , 0x00);
	GC0308_write_cmos_sensor(0xda , 0x00);
	GC0308_write_cmos_sensor(0xe0 , 0x09);
	GC0308_write_cmos_sensor(0xec , 0x20);
	GC0308_write_cmos_sensor(0xed , 0x04);
	GC0308_write_cmos_sensor(0xee , 0xa0);
	GC0308_write_cmos_sensor(0xef , 0x40);
	GC0308_write_cmos_sensor(0x80 , 0x03);
	GC0308_write_cmos_sensor(0x80 , 0x03);
	GC0308_write_cmos_sensor(0x9F , 0x10);
	GC0308_write_cmos_sensor(0xA0 , 0x20);
	GC0308_write_cmos_sensor(0xA1 , 0x38);
	GC0308_write_cmos_sensor(0xA2 , 0x4E);
	GC0308_write_cmos_sensor(0xA3 , 0x63);
	GC0308_write_cmos_sensor(0xA4 , 0x76);
	GC0308_write_cmos_sensor(0xA5 , 0x87);
	GC0308_write_cmos_sensor(0xA6 , 0xA2);
	GC0308_write_cmos_sensor(0xA7 , 0xB8);
	GC0308_write_cmos_sensor(0xA8 , 0xCA);
	GC0308_write_cmos_sensor(0xA9 , 0xD8);
	GC0308_write_cmos_sensor(0xAA , 0xE3);
	GC0308_write_cmos_sensor(0xAB , 0xEB);
	GC0308_write_cmos_sensor(0xAC , 0xF0);
	GC0308_write_cmos_sensor(0xAD , 0xF8);
	GC0308_write_cmos_sensor(0xAE , 0xFD);
	GC0308_write_cmos_sensor(0xAF , 0xFF);
	GC0308_write_cmos_sensor(0xc0 , 0x00);
	GC0308_write_cmos_sensor(0xc1 , 0x10);
	GC0308_write_cmos_sensor(0xc2 , 0x1C);
	GC0308_write_cmos_sensor(0xc3 , 0x30);
	GC0308_write_cmos_sensor(0xc4 , 0x43);
	GC0308_write_cmos_sensor(0xc5 , 0x54);
	GC0308_write_cmos_sensor(0xc6 , 0x65);
	GC0308_write_cmos_sensor(0xc7 , 0x75);
	GC0308_write_cmos_sensor(0xc8 , 0x93);
	GC0308_write_cmos_sensor(0xc9 , 0xB0);
	GC0308_write_cmos_sensor(0xca , 0xCB);
	GC0308_write_cmos_sensor(0xcb , 0xE6);
	GC0308_write_cmos_sensor(0xcc , 0xFF);
	GC0308_write_cmos_sensor(0xf0 , 0x02);
	GC0308_write_cmos_sensor(0xf1 , 0x01);
	GC0308_write_cmos_sensor(0xf2 , 0x01);
	GC0308_write_cmos_sensor(0xf3 , 0x30);
	GC0308_write_cmos_sensor(0xf9 , 0x9f);
	GC0308_write_cmos_sensor(0xfa , 0x78);

	//---------------------------------------------------------------
	GC0308_SET_PAGE1;

	GC0308_write_cmos_sensor(0x00 , 0xf5);
	GC0308_write_cmos_sensor(0x02 , 0x1a);
	GC0308_write_cmos_sensor(0x0a , 0xa0);
	GC0308_write_cmos_sensor(0x0b , 0x60);
	GC0308_write_cmos_sensor(0x0c , 0x08);
	GC0308_write_cmos_sensor(0x0e , 0x4c);
	GC0308_write_cmos_sensor(0x0f , 0x39);
	GC0308_write_cmos_sensor(0x11 , 0x3f);
	GC0308_write_cmos_sensor(0x12 , 0x72);
	GC0308_write_cmos_sensor(0x13 , 0x13);
	GC0308_write_cmos_sensor(0x14 , 0x40);
	GC0308_write_cmos_sensor(0x15 , 0x43);
	GC0308_write_cmos_sensor(0x16 , 0xc2);
	GC0308_write_cmos_sensor(0x17 , 0xa8);
	GC0308_write_cmos_sensor(0x18 , 0x18);
	GC0308_write_cmos_sensor(0x19 , 0x40);
	GC0308_write_cmos_sensor(0x1a , 0xc0);
	GC0308_write_cmos_sensor(0x1b , 0xf5);
	GC0308_write_cmos_sensor(0x70 , 0x40);
	GC0308_write_cmos_sensor(0x71 , 0x58);
	GC0308_write_cmos_sensor(0x72 , 0x30);
	GC0308_write_cmos_sensor(0x73 , 0x48);
	GC0308_write_cmos_sensor(0x74 , 0x20);
	GC0308_write_cmos_sensor(0x75 , 0x60);
	GC0308_write_cmos_sensor(0x77 , 0x20);
	GC0308_write_cmos_sensor(0x78 , 0x32);
	GC0308_write_cmos_sensor(0x30 , 0x03);
	GC0308_write_cmos_sensor(0x31 , 0x40);
	GC0308_write_cmos_sensor(0x32 , 0xe0);
	GC0308_write_cmos_sensor(0x33 , 0xe0);
	GC0308_write_cmos_sensor(0x34 , 0xe0);
	GC0308_write_cmos_sensor(0x35 , 0xb0);
	GC0308_write_cmos_sensor(0x36 , 0xc0);
	GC0308_write_cmos_sensor(0x37 , 0xc0);
	GC0308_write_cmos_sensor(0x38 , 0x04);
	GC0308_write_cmos_sensor(0x39 , 0x09);
	GC0308_write_cmos_sensor(0x3a , 0x12);
	GC0308_write_cmos_sensor(0x3b , 0x1C);
	GC0308_write_cmos_sensor(0x3c , 0x28);
	GC0308_write_cmos_sensor(0x3d , 0x31);
	GC0308_write_cmos_sensor(0x3e , 0x44);
	GC0308_write_cmos_sensor(0x3f , 0x57);
	GC0308_write_cmos_sensor(0x40 , 0x6C);
	GC0308_write_cmos_sensor(0x41 , 0x81);
	GC0308_write_cmos_sensor(0x42 , 0x94);
	GC0308_write_cmos_sensor(0x43 , 0xA7);
	GC0308_write_cmos_sensor(0x44 , 0xB8);
	GC0308_write_cmos_sensor(0x45 , 0xD6);
	GC0308_write_cmos_sensor(0x46 , 0xEE);
	GC0308_write_cmos_sensor(0x47 , 0x0d); 
	
	GC0308_SET_PAGE0;

    	GC0308_write_cmos_sensor(0xd2 , 0x90);  // Open AEC at last.  

}



UINT32 GC0308GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int i;

    Sleep(20);

    do
    {
        	// check if sensor ID correct
        	for(i = 0; i < 3; i++)
		{
	            	sensor_id = GC0308_read_cmos_sensor(0x00);
	            	printk("GC0308 Sensor id = %x\n", sensor_id);
	            	if (sensor_id == GC0308_SENSOR_ID)
			{
	               	break;
	            	}
        	}
        	mdelay(50);
    }while(0);

    if(sensor_id != GC0308_SENSOR_ID)
    {
        SENSORDB("GC0308 Sensor id read failed, ID = %x\n", sensor_id);
		*sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    *sensorID = sensor_id;

    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
	
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	GC0308_GAMMA_Select
*
* DESCRIPTION
*	This function is served for FAE to select the appropriate GAMMA curve.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/

void GC0308GammaSelect(kal_uint32 GammaLvl)
{
	switch(GammaLvl)
	{
		case 1:                                             //smallest gamma curve
			GC0308_write_cmos_sensor( 0x9F, 0x0B ); 
			GC0308_write_cmos_sensor( 0xA0, 0x16 ); 
			GC0308_write_cmos_sensor( 0xA1, 0x29 ); 
			GC0308_write_cmos_sensor( 0xA2, 0x3C ); 
			GC0308_write_cmos_sensor( 0xA3, 0x4F ); 
			GC0308_write_cmos_sensor( 0xA4, 0x5F ); 
			GC0308_write_cmos_sensor( 0xA5, 0x6F ); 
			GC0308_write_cmos_sensor( 0xA6, 0x8A ); 
			GC0308_write_cmos_sensor( 0xA7, 0x9F ); 
			GC0308_write_cmos_sensor( 0xA8, 0xB4 ); 
			GC0308_write_cmos_sensor( 0xA9, 0xC6 ); 
			GC0308_write_cmos_sensor( 0xAA, 0xD3 ); 
			GC0308_write_cmos_sensor( 0xAB, 0xDD );  
			GC0308_write_cmos_sensor( 0xAC, 0xE5 );  
			GC0308_write_cmos_sensor( 0xAD, 0xF1 ); 
			GC0308_write_cmos_sensor( 0xAE, 0xFA ); 
			GC0308_write_cmos_sensor( 0xAF, 0xFF ); 	
			break;
		case 2:			
			GC0308_write_cmos_sensor( 0x9F, 0x0E ); 
			GC0308_write_cmos_sensor( 0xA0, 0x1C ); 
			GC0308_write_cmos_sensor( 0xA1, 0x34 ); 
			GC0308_write_cmos_sensor( 0xA2, 0x48 ); 
			GC0308_write_cmos_sensor( 0xA3, 0x5A ); 
			GC0308_write_cmos_sensor( 0xA4, 0x6B ); 
			GC0308_write_cmos_sensor( 0xA5, 0x7B ); 
			GC0308_write_cmos_sensor( 0xA6, 0x95 ); 
			GC0308_write_cmos_sensor( 0xA7, 0xAB ); 
			GC0308_write_cmos_sensor( 0xA8, 0xBF );
			GC0308_write_cmos_sensor( 0xA9, 0xCE ); 
			GC0308_write_cmos_sensor( 0xAA, 0xD9 ); 
			GC0308_write_cmos_sensor( 0xAB, 0xE4 );  
			GC0308_write_cmos_sensor( 0xAC, 0xEC ); 
			GC0308_write_cmos_sensor( 0xAD, 0xF7 ); 
			GC0308_write_cmos_sensor( 0xAE, 0xFD ); 
			GC0308_write_cmos_sensor( 0xAF, 0xFF ); 
		break;
		case 3:
			GC0308_write_cmos_sensor( 0x9F, 0x10 ); 
			GC0308_write_cmos_sensor( 0xA0, 0x20 ); 
			GC0308_write_cmos_sensor( 0xA1, 0x38 ); 
			GC0308_write_cmos_sensor( 0xA2, 0x4E ); 
			GC0308_write_cmos_sensor( 0xA3, 0x63 ); 
			GC0308_write_cmos_sensor( 0xA4, 0x76 ); 
			GC0308_write_cmos_sensor( 0xA5, 0x87 ); 
			GC0308_write_cmos_sensor( 0xA6, 0xA2 ); 
			GC0308_write_cmos_sensor( 0xA7, 0xB8 ); 
			GC0308_write_cmos_sensor( 0xA8, 0xCA ); 
			GC0308_write_cmos_sensor( 0xA9, 0xD8 ); 
			GC0308_write_cmos_sensor( 0xAA, 0xE3 ); 
			GC0308_write_cmos_sensor( 0xAB, 0xEB ); 
			GC0308_write_cmos_sensor( 0xAC, 0xF0 ); 
			GC0308_write_cmos_sensor( 0xAD, 0xF8 ); 
			GC0308_write_cmos_sensor( 0xAE, 0xFD ); 
			GC0308_write_cmos_sensor( 0xAF, 0xFF ); 

			break;
		case 4:
			GC0308_write_cmos_sensor( 0x9F, 0x14 ); 
			GC0308_write_cmos_sensor( 0xA0, 0x28 ); 
			GC0308_write_cmos_sensor( 0xA1, 0x44 ); 
			GC0308_write_cmos_sensor( 0xA2, 0x5D ); 
			GC0308_write_cmos_sensor( 0xA3, 0x72 ); 
			GC0308_write_cmos_sensor( 0xA4, 0x86 ); 
			GC0308_write_cmos_sensor( 0xA5, 0x95 ); 
			GC0308_write_cmos_sensor( 0xA6, 0xB1 ); 
			GC0308_write_cmos_sensor( 0xA7, 0xC6 ); 
			GC0308_write_cmos_sensor( 0xA8, 0xD5 ); 
			GC0308_write_cmos_sensor( 0xA9, 0xE1 ); 
			GC0308_write_cmos_sensor( 0xAA, 0xEA ); 
			GC0308_write_cmos_sensor( 0xAB, 0xF1 ); 
			GC0308_write_cmos_sensor( 0xAC, 0xF5 ); 
			GC0308_write_cmos_sensor( 0xAD, 0xFB ); 
			GC0308_write_cmos_sensor( 0xAE, 0xFE ); 
			GC0308_write_cmos_sensor( 0xAF, 0xFF );
		break;
		case 5:								// largest gamma curve
			GC0308_write_cmos_sensor( 0x9F, 0x15 ); 
			GC0308_write_cmos_sensor( 0xA0, 0x2A ); 
			GC0308_write_cmos_sensor( 0xA1, 0x4A ); 
			GC0308_write_cmos_sensor( 0xA2, 0x67 ); 
			GC0308_write_cmos_sensor( 0xA3, 0x79 ); 
			GC0308_write_cmos_sensor( 0xA4, 0x8C ); 
			GC0308_write_cmos_sensor( 0xA5, 0x9A ); 
			GC0308_write_cmos_sensor( 0xA6, 0xB3 ); 
			GC0308_write_cmos_sensor( 0xA7, 0xC5 ); 
			GC0308_write_cmos_sensor( 0xA8, 0xD5 ); 
			GC0308_write_cmos_sensor( 0xA9, 0xDF ); 
			GC0308_write_cmos_sensor( 0xAA, 0xE8 ); 
			GC0308_write_cmos_sensor( 0xAB, 0xEE ); 
			GC0308_write_cmos_sensor( 0xAC, 0xF3 ); 
			GC0308_write_cmos_sensor( 0xAD, 0xFA ); 
			GC0308_write_cmos_sensor( 0xAE, 0xFD ); 
			GC0308_write_cmos_sensor( 0xAF, 0xFF );
			break;
		default:
		break;
	}
}



/*************************************************************************
* FUNCTION
*	GC0308_Write_More_Registers
*
* DESCRIPTION
*	This function is served for FAE to modify the necessary Init Regs. Do not modify the regs
*     in init_GC0308() directly.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GC0308_Write_More_Registers(void)
{
	//  TODO: FAE Modify the Init Regs here!!! 

	//-----------Update the registers 2010/07/07-------------//

	//Registers of Page0
	GC0308_SET_PAGE0; 

	GC0308_write_cmos_sensor(0x10 , 0x26);                                 
	GC0308_write_cmos_sensor(0x11 , 0x0d);  	// fd,modified by mormo 2010/07/06                               
	GC0308_write_cmos_sensor(0x1a , 0x2a);  	// 1e,modified by mormo 2010/07/06                                  

	GC0308_write_cmos_sensor(0x1c , 0x49); 	// c1,modified by mormo 2010/07/06                                 
	GC0308_write_cmos_sensor(0x1d , 0x9a);	// 08,modified by mormo 2010/07/06                                 
	GC0308_write_cmos_sensor(0x1e , 0x61);	// 60,modified by mormo 2010/07/06                                 

	GC0308_write_cmos_sensor(0x3a , 0x20);

	GC0308_write_cmos_sensor(0x50 , 0x16);  	// 10,modified by mormo 2010/07/06                               
	GC0308_write_cmos_sensor(0x53 , 0x80);                                  
	GC0308_write_cmos_sensor(0x56 , 0x80);
	
	GC0308_write_cmos_sensor(0x8b , 0x20); 	//LSC                                 
	GC0308_write_cmos_sensor(0x8c , 0x20);                                  
	GC0308_write_cmos_sensor(0x8d , 0x20);                                  
	GC0308_write_cmos_sensor(0x8e , 0x14);                                  
	GC0308_write_cmos_sensor(0x8f , 0x10);                                  
	GC0308_write_cmos_sensor(0x90 , 0x14);                                  

	GC0308_write_cmos_sensor(0x94 , 0x00);                                  
	GC0308_write_cmos_sensor(0x95 , 0x07);                                  
	GC0308_write_cmos_sensor(0x96 , 0xe0);                                  

	GC0308_write_cmos_sensor(0xb1 , 0x38); // YCPT                                 
	GC0308_write_cmos_sensor(0xb2 , 0x38);                                  
	GC0308_write_cmos_sensor(0xb3 , 0x40);
	GC0308_write_cmos_sensor(0xb6 , 0xe0);

	GC0308_write_cmos_sensor(0xd0 , 0xcb); // AECT  c9,modifed by mormo 2010/07/06                                
	GC0308_write_cmos_sensor(0xd3 , 0x48); // 80,modified by mormor 2010/07/06                           

	GC0308_write_cmos_sensor(0xf2 , 0x02);                                  
	GC0308_write_cmos_sensor(0xf7 , 0x12);
	GC0308_write_cmos_sensor(0xf8 , 0x0a);

	//Registers of Page1
	GC0308_SET_PAGE1;

	GC0308_write_cmos_sensor(0x02 , 0x20);
	GC0308_write_cmos_sensor(0x04 , 0x10);
	GC0308_write_cmos_sensor(0x05 , 0x08);
	GC0308_write_cmos_sensor(0x06 , 0x20);
	GC0308_write_cmos_sensor(0x08 , 0x0a);

	GC0308_write_cmos_sensor(0x0e , 0x44);                                  
	GC0308_write_cmos_sensor(0x0f , 0x32);
	GC0308_write_cmos_sensor(0x10 , 0x41);                                  
	GC0308_write_cmos_sensor(0x11 , 0x37);                                  
	GC0308_write_cmos_sensor(0x12 , 0x22);                                  
	GC0308_write_cmos_sensor(0x13 , 0x19);                                  
	GC0308_write_cmos_sensor(0x14 , 0x40);
	GC0308_write_cmos_sensor(0x15 , 0x44);  
	
	GC0308_write_cmos_sensor(0x19 , 0x50);                                  
	GC0308_write_cmos_sensor(0x1a , 0xc0);
	
	GC0308_write_cmos_sensor(0x32 , 0x10); 
	
	GC0308_write_cmos_sensor(0x35 , 0x00);                                  
	GC0308_write_cmos_sensor(0x36 , 0x80);                                  
	GC0308_write_cmos_sensor(0x37 , 0x00); 
	//-----------Update the registers end---------//

    	GC0308_SET_PAGE0; 
    	/*Customer can adjust GAMMA, MIRROR & UPSIDEDOWN here!*/

    	GC0308GammaSelect(3);
}


/*************************************************************************
 * FUNCTION
 *	GC0308Open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0308Open(void)
{
    kal_uint16 sensor_id=0;
    int i;

    Sleep(20);

    do
    {
        	// check if sensor ID correct
        	for(i = 0; i < 3; i++)
		{
	            	sensor_id = GC0308_read_cmos_sensor(0x00);
	            	printk("GC0308 Sensor id = %x\n", sensor_id);
	            	if (sensor_id == GC0308_SENSOR_ID)
			{
	               	break;
	            	}
        	}
        	mdelay(50);
    }while(0);

    if(sensor_id != GC0308_SENSOR_ID)
    {
        SENSORDB("GC0308 Sensor id read failed, ID = %x\n", sensor_id);
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
    // initail sequence write in
    GC0308_Sensor_Init();
    GC0308_Write_More_Registers();
	
    return ERROR_NONE;
} /* GC0308Open */


/*************************************************************************
 * FUNCTION
 *	GC0308Close
 *
 * DESCRIPTION
 *	This function is to turn off sensor module power.
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0308Close(void)
{
    return ERROR_NONE;
} /* GC0308Close */


/*************************************************************************
 * FUNCTION
 * GC0308Preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *	*image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0308Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        RETAILMSG(1, (TEXT("Camera Video preview\r\n")));
        GC0308_MPEG4_encode_mode = KAL_TRUE;
       
    }
    else
    {
        RETAILMSG(1, (TEXT("Camera preview\r\n")));
        GC0308_MPEG4_encode_mode = KAL_FALSE;
    }

    image_window->GrabStartX= IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    // copy sensor_config_data
    memcpy(&GC0308SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0308Preview */


/*************************************************************************
 * FUNCTION
 *	GC0308Capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0308Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    GC0308_MODE_CAPTURE=KAL_TRUE;

    image_window->GrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&GC0308SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0308_Capture() */



UINT32 GC0308GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;
	pSensorResolution->SensorVideoWidth= IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorVideoHeight= IMAGE_SENSOR_PV_HEIGHT;

    return ERROR_NONE;
} /* GC0308GetResolution() */


UINT32 GC0308GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_WIDTH;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_UYVY;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;
/*
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=FALSE;
	*/
    pSensorInfo->CaptureDelayFrame = 1;
    pSensorInfo->PreviewDelayFrame = 0;
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_2MA;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;

        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
        break;
    default:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
        break;
    }
    GC0308PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &GC0308SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0308GetInfo() */



UINT32 GC0308Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        GC0308Preview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        GC0308Capture(pImageWindow, pSensorConfigData);
        break;
    }


    return TRUE;
}	/* GC0308Control() */



BOOL GC0308_set_param_wb(UINT16 para)
{
	switch (para)
	{
		case AWB_MODE_OFF:

		break;
		
		case AWB_MODE_AUTO:
			GC0308_write_cmos_sensor(0x5a,0x56); //for AWB can adjust back
			GC0308_write_cmos_sensor(0x5b,0x40);
			GC0308_write_cmos_sensor(0x5c,0x4a);	
			GC0308_AWB_enable(KAL_TRUE);
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			GC0308_AWB_enable(KAL_FALSE);
			GC0308_write_cmos_sensor(0x5a,0x8c); //WB_manual_gain 
			GC0308_write_cmos_sensor(0x5b,0x50);
			GC0308_write_cmos_sensor(0x5c,0x40);
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny
			GC0308_AWB_enable(KAL_FALSE);
			GC0308_write_cmos_sensor(0x5a,0x74); 
			GC0308_write_cmos_sensor(0x5b,0x52);
			GC0308_write_cmos_sensor(0x5c,0x40);			
		break;
		
		case AWB_MODE_INCANDESCENT: //office
			GC0308_AWB_enable(KAL_FALSE);
			GC0308_write_cmos_sensor(0x5a,0x48);
			GC0308_write_cmos_sensor(0x5b,0x40);
			GC0308_write_cmos_sensor(0x5c,0x5c);
		break;
		
		case AWB_MODE_TUNGSTEN: //home
			GC0308_AWB_enable(KAL_FALSE);
			GC0308_write_cmos_sensor(0x5a,0x40);
			GC0308_write_cmos_sensor(0x5b,0x54);
			GC0308_write_cmos_sensor(0x5c,0x70);
		break;
		
		case AWB_MODE_FLUORESCENT:
			GC0308_AWB_enable(KAL_FALSE);
			GC0308_write_cmos_sensor(0x5a,0x40);
			GC0308_write_cmos_sensor(0x5b,0x42);
			GC0308_write_cmos_sensor(0x5c,0x50);
		break;
		
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0308_set_param_wb */


BOOL GC0308_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
			GC0308_write_cmos_sensor(0x23,0x00);
			GC0308_write_cmos_sensor(0x2d,0x0a); // 0x08
			GC0308_write_cmos_sensor(0x20,0xff);
			GC0308_write_cmos_sensor(0xd2,0x90);
			GC0308_write_cmos_sensor(0x73,0x00);
			GC0308_write_cmos_sensor(0x77,0x54);
			
			GC0308_write_cmos_sensor(0xb3,0x40);
			GC0308_write_cmos_sensor(0xb4,0x80);
			GC0308_write_cmos_sensor(0xba,0x00);
			GC0308_write_cmos_sensor(0xbb,0x00);
		break;
		
		case MEFFECT_SEPIA:
			GC0308_write_cmos_sensor(0x23,0x02);		
			GC0308_write_cmos_sensor(0x2d,0x0a);
			GC0308_write_cmos_sensor(0x20,0xff);
			GC0308_write_cmos_sensor(0xd2,0x90);
			GC0308_write_cmos_sensor(0x73,0x00);

			GC0308_write_cmos_sensor(0xb3,0x40);
			GC0308_write_cmos_sensor(0xb4,0x80);
			GC0308_write_cmos_sensor(0xba,0xd0);
			GC0308_write_cmos_sensor(0xbb,0x28);	
		break;
		
		case MEFFECT_NEGATIVE:
			GC0308_write_cmos_sensor(0x23,0x01);		
			GC0308_write_cmos_sensor(0x2d,0x0a);
			GC0308_write_cmos_sensor(0x20,0xff);
			GC0308_write_cmos_sensor(0xd2,0x90);
			GC0308_write_cmos_sensor(0x73,0x00);

			GC0308_write_cmos_sensor(0xb3,0x40);
			GC0308_write_cmos_sensor(0xb4,0x80);
			GC0308_write_cmos_sensor(0xba,0x00);
			GC0308_write_cmos_sensor(0xbb,0x00);	
		break;
		
		case MEFFECT_SEPIAGREEN:
			GC0308_write_cmos_sensor(0x23,0x02);	
			GC0308_write_cmos_sensor(0x2d,0x0a);
			GC0308_write_cmos_sensor(0x20,0xff);
			GC0308_write_cmos_sensor(0xd2,0x90);
			GC0308_write_cmos_sensor(0x77,0x88);

			GC0308_write_cmos_sensor(0xb3,0x40);
			GC0308_write_cmos_sensor(0xb4,0x80);
			GC0308_write_cmos_sensor(0xba,0xc0);
			GC0308_write_cmos_sensor(0xbb,0xc0);	
		break;
		
		case MEFFECT_SEPIABLUE:
			GC0308_write_cmos_sensor(0x23,0x02);	
			GC0308_write_cmos_sensor(0x2d,0x0a);
			GC0308_write_cmos_sensor(0x20,0xff);
			GC0308_write_cmos_sensor(0xd2,0x90);
			GC0308_write_cmos_sensor(0x73,0x00);

			GC0308_write_cmos_sensor(0xb3,0x40);
			GC0308_write_cmos_sensor(0xb4,0x80);
			GC0308_write_cmos_sensor(0xba,0x50);
			GC0308_write_cmos_sensor(0xbb,0xe0);
		break;

		case MEFFECT_MONO:
			GC0308_write_cmos_sensor(0x23,0x02);	
			GC0308_write_cmos_sensor(0x2d,0x0a);
			GC0308_write_cmos_sensor(0x20,0xff);
			GC0308_write_cmos_sensor(0xd2,0x90);
			GC0308_write_cmos_sensor(0x73,0x00);

			GC0308_write_cmos_sensor(0xb3,0x40);
			GC0308_write_cmos_sensor(0xb4,0x80);
			GC0308_write_cmos_sensor(0xba,0x00);
			GC0308_write_cmos_sensor(0xbb,0x00);	
		break;
		default:
			ret = FALSE;
	}

	return ret;

} /* GC0308_set_param_effect */


BOOL GC0308_set_param_banding(UINT16 para)
{
	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			GC0308_write_cmos_sensor(0x01  ,0x26); 	
			GC0308_write_cmos_sensor(0x02  ,0x98); 
			GC0308_write_cmos_sensor(0x0f  ,0x03);

			GC0308_write_cmos_sensor(0xe2  ,0x00); 	//anti-flicker step [11:8]
			GC0308_write_cmos_sensor(0xe3  ,0x50);   //anti-flicker step [7:0]

			GC0308_write_cmos_sensor(0xe4  ,0x02);   //exp level 0  12.5fps
			GC0308_write_cmos_sensor(0xe5  ,0x80); 
			GC0308_write_cmos_sensor(0xe6  ,0x03);   //exp level 1  10fps
			GC0308_write_cmos_sensor(0xe7  ,0x20); 
			GC0308_write_cmos_sensor(0xe8  ,0x04);   //exp level 2  7.69fps
			GC0308_write_cmos_sensor(0xe9  ,0x10); 
			GC0308_write_cmos_sensor(0xea  ,0x06);   //exp level 3  5.00fps
			GC0308_write_cmos_sensor(0xeb  ,0x40); 
			break;

		case AE_FLICKER_MODE_60HZ:
			GC0308_write_cmos_sensor(0x01  ,0x97); 	
			GC0308_write_cmos_sensor(0x02  ,0x84); 
			GC0308_write_cmos_sensor(0x0f  ,0x03);

			GC0308_write_cmos_sensor(0xe2  ,0x00); 	//anti-flicker step [11:8]
			GC0308_write_cmos_sensor(0xe3  ,0x3e);   //anti-flicker step [7:0]
				
			GC0308_write_cmos_sensor(0xe4  ,0x02);   //exp level 0  12.00fps
			GC0308_write_cmos_sensor(0xe5  ,0x6c); 
			GC0308_write_cmos_sensor(0xe6  ,0x02);   //exp level 1  10.00fps
			GC0308_write_cmos_sensor(0xe7  ,0xe8); 
			GC0308_write_cmos_sensor(0xe8  ,0x03);   //exp level 2  7.50fps
			GC0308_write_cmos_sensor(0xe9  ,0xe0); 
			GC0308_write_cmos_sensor(0xea  ,0x05);   //exp level 3  5.00fps
			GC0308_write_cmos_sensor(0xeb  ,0xd0); 
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0308_set_param_banding */


BOOL GC0308_set_param_exposure(UINT16 para)
{

	switch (para)
	{
		case AE_EV_COMP_n13:
			GC0308_write_cmos_sensor(0xb5, 0xc0);
			GC0308_write_cmos_sensor(0xd3, 0x30);
		break;
		
		case AE_EV_COMP_n10:
			GC0308_write_cmos_sensor(0xb5, 0xd0);
			GC0308_write_cmos_sensor(0xd3, 0x38);
		break;
		
		case AE_EV_COMP_n07:
			GC0308_write_cmos_sensor(0xb5, 0xe0);
			GC0308_write_cmos_sensor(0xd3, 0x40);
		break;
		
		case AE_EV_COMP_n03:
			GC0308_write_cmos_sensor(0xb5, 0xf0);
			GC0308_write_cmos_sensor(0xd3, 0x48);
		break;				
		
		case AE_EV_COMP_00:
			GC0308_write_cmos_sensor(0xb5, 0x00);
			GC0308_write_cmos_sensor(0xd3, 0x50);
		break;

		case AE_EV_COMP_03:
			GC0308_write_cmos_sensor(0xb5, 0x10);
			GC0308_write_cmos_sensor(0xd3, 0x60);
		break;
		
		case AE_EV_COMP_07:
			GC0308_write_cmos_sensor(0xb5, 0x20);
			GC0308_write_cmos_sensor(0xd3, 0x70);
		break;
		
		case AE_EV_COMP_10:
			GC0308_write_cmos_sensor(0xb5, 0x30);
			GC0308_write_cmos_sensor(0xd3, 0x80);
		break;
		
		case AE_EV_COMP_13:
			GC0308_write_cmos_sensor(0xb5, 0x40);
			GC0308_write_cmos_sensor(0xd3, 0x90);
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0308_set_param_exposure */


UINT32 GC0308YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {
    case FID_AWB_MODE:
        GC0308_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        GC0308_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        GC0308_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        GC0308_set_param_banding(iPara);
		break;
    case FID_SCENE_MODE:
		GC0308NightMode(iPara);
        break;
    case FID_AE_SCENE_MODE: 
	  if (iPara == AE_MODE_OFF)
	  	{
			GC0308_set_AE_mode(KAL_FALSE);
		}
	else {
			GC0308_set_AE_mode(KAL_TRUE);
		}
	break;  
    default:
        break;
    }
    return TRUE;
} /* GC0308YUVSensorSetting */


UINT32 GC0308FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 GC0308SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    RETAILMSG(1, (_T("gaiyang GC0308FeatureControl FeatureId=%d\r\n"), FeatureId));

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(VGA_PERIOD_PIXEL_NUMS)+GC0308_dummy_pixels;
        *pFeatureReturnPara16=(VGA_PERIOD_LINE_NUMS)+GC0308_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = GC0308_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        //GC0308NightMode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        GC0308_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        GC0308_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = GC0308_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &GC0308SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
        *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
        break;
    case SENSOR_FEATURE_SET_CCT_REGISTER:
    case SENSOR_FEATURE_GET_CCT_REGISTER:
    case SENSOR_FEATURE_SET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
    case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
    case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
    case SENSOR_FEATURE_GET_GROUP_COUNT:
    case SENSOR_FEATURE_GET_GROUP_INFO:
    case SENSOR_FEATURE_GET_ITEM_INFO:
    case SENSOR_FEATURE_SET_ITEM_INFO:
    case SENSOR_FEATURE_GET_ENG_INFO:
        break;
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
        // if EEPROM does not exist in camera module.
        *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_YUV_CMD:
        GC0308YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
	GC0308GetSensorID(pFeatureData32);
	break;
    default:
        break;
	}
return ERROR_NONE;
}	/* GC0308FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncGC0308YUV=
{
	GC0308Open,
	GC0308GetInfo,
	GC0308GetResolution,
	GC0308FeatureControl,
	GC0308Control,
	GC0308Close
};


UINT32 GC0308_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncGC0308YUV;
	return ERROR_NONE;
} /* SensorInit() */
