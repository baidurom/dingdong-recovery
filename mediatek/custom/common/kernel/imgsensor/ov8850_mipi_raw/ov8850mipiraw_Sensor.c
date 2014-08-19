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
 *   Sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Image sensor driver function
 * 
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov8850mipiraw_Sensor.h"
#include "ov8850mipiraw_Camera_Sensor_para.h"
#include "ov8850mipiraw_CameraCustomized.h"

#define OV8850_DEBUG
#define OV8850_DRIVER_TRACE
#define LOG_TAG "[OV8850MIPIRaw]"
#ifdef OV8850_DEBUG
#define SENSORDB(fmt,arg...) printk(LOG_TAG "%s: " fmt "\n", __FUNCTION__ ,##arg)
#else
#define SENSORDB printk
#endif
//#define ACDK
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);



MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
static OV8850_sensor_struct OV8850_sensor =
{
  .eng =
  {
    .reg = OV8850_CAMERA_SENSOR_REG_DEFAULT_VALUE,
    .cct = OV8850_CAMERA_SENSOR_CCT_DEFAULT_VALUE,
  },
  .eng_info =
  {
    .SensorId = OV8850_SENSOR_ID,
    .SensorType = CMOS_SENSOR,
    .SensorOutputDataFormat = OV8850_COLOR_FORMAT,
  },
  .shutter = 0x20,  
  .gain = 0x20,
  .pv_pclk = OV8850_PREVIEW_CLK,
  .cap_pclk = OV8850_CAPTURE_CLK,
  .pclk = OV8850_PREVIEW_CLK,
  .frame_height = OV8850_PV_PERIOD_LINE_NUMS,
  .line_length = OV8850_PV_PERIOD_PIXEL_NUMS,
  .is_zsd = KAL_FALSE, //for zsd
  .dummy_pixels = 0,
  .dummy_lines = 0,  //for zsd
  .is_autofliker = KAL_FALSE,
};

static DEFINE_SPINLOCK(OV8850_drv_lock);

kal_uint16 OV8850_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,OV8850_sensor.write_id);
#ifdef OV8850_DRIVER_TRACE
	//SENSORDB("OV8850_read_cmos_sensor, addr:%x;get_byte:%x \n",addr,get_byte);
#endif
    return get_byte;
}


kal_uint16 OV8850_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    //kal_uint16 reg_tmp;

    char puSendCmd[3] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para & 0xFF)};

    iWriteRegI2C(puSendCmd , 3,OV8850_sensor.write_id);
    return ERROR_NONE;
}

/*****************************  OTP Feature  **********************************/
//#define OV8850_USE_OTP
void clear_otp_buffer()
{
	int i;
	// clear otp buffer
	for (i=0;i<16;i++) {
		OV8850_write_cmos_sensor(0x3d00 + i, 0x00);
	}
}


#if defined(OV8850_USE_OTP)

//For HW define
struct otp_struct {
	int product_year;
	int product_month;
	int product_day;
	int module_integrator_id; 
	int rg_ratio;
	int bg_ratio;
	int br_ratio;
	int VCM_start;
	int VCM_end;
	int lenc[62];
	int light_rg;
	int light_bg;

};

// R/G and B/G of typical camera module is defined here

int RG_Ratio_Typical = 0x230;
int BG_Ratio_Typical = 0x2B1;



//For HW
// index: index of otp group. (0, 1, 2)
// return: 	index 0, 1, 2, if empty, return 4;
int check_otp_wb()
{
	int flag;
	int index;
	int bank, address;

	for(index = 0;index<3;index++)
	{
		// select bank index
		bank = 0xc0 | (index*5+1);
		OV8850_write_cmos_sensor(0x3d84, bank);

		// read otp into buffer
		OV8850_write_cmos_sensor(0x3d81, 0x01);
		mdelay(5);
		// disable otp read
		//OV8850_write_cmos_sensor(0x3d81, 0x00);

		// read WB
		address = 0x3d08;
		int temp1 = OV8850_read_cmos_sensor(address);
		int temp2 =OV8850_read_cmos_sensor(address+1);
		flag = (OV8850_read_cmos_sensor(address) << 8)+ OV8850_read_cmos_sensor(address+1);
	
    	SENSORDB("check_otp_wb, temp1 = %x, temp2 = %x\n", temp1, temp2);
    	printk("check_otp_wb, temp1 = %x, temp2 = %x\n", temp1, temp2);

		OV8850_write_cmos_sensor(0x3d81, 0x00);

		clear_otp_buffer();

		if (flag==0) {
			return index-1;
		}
	
	}
	return 2;
}




// For HW
// index: index of otp group. (0, 1, 2)
// otp_ptr: pointer of otp_struct
// return: 	0, 
int read_otp_wb(int index, struct otp_struct * otp_ptr)
{
	int bank;
	int address;

	// select bank index
	bank = 0xc0 | (5*index+1);
	OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp into buffer
	OV8850_write_cmos_sensor(0x3d81, 0x01);

	address = 0x3d00;
	(*otp_ptr).product_year =  OV8850_read_cmos_sensor(address);
	(*otp_ptr).product_month = OV8850_read_cmos_sensor(address + 1);
	(*otp_ptr).product_day = OV8850_read_cmos_sensor(address + 2);
	(*otp_ptr).module_integrator_id = OV8850_read_cmos_sensor(address + 7);
	(*otp_ptr).rg_ratio = ((OV8850_read_cmos_sensor(address + 8))<<8)+(OV8850_read_cmos_sensor(address + 9));
	(*otp_ptr).bg_ratio = ((OV8850_read_cmos_sensor(address + 10))<<8)+(OV8850_read_cmos_sensor(address + 11));
	(*otp_ptr).br_ratio = ((OV8850_read_cmos_sensor(address + 12))<<8)+(OV8850_read_cmos_sensor(address + 13));
	(*otp_ptr).VCM_start = OV8850_read_cmos_sensor(address + 14);
	(*otp_ptr).VCM_end = OV8850_read_cmos_sensor(address + 15);
	
	// disable otp read
	OV8850_write_cmos_sensor(0x3d81, 0x00);

	clear_otp_buffer();

//no write light sourch
	(*otp_ptr).light_rg =0;
	(*otp_ptr).light_bg =0;

	//bank = 0xc0 | (5*index+5);
	//OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp into buffer
	//OV8850_write_cmos_sensor(0x3d81, 0x01);
	//address = 0x3d0e;
	//(*otp_ptr).light_rg =OV8850_read_cmos_sensor(address);
	//(*otp_ptr).light_bg =OV8850_read_cmos_sensor(address+1);

	// disable otp read
	//OV8850_write_cmos_sensor(0x3d81, 0x00);

	//clear_otp_buffer();
	return 0;	
}

// For HW
// index: index of otp group. (0, 1, 2)
// otp_ptr: pointer of otp_struct
// return: 	0, 
int read_otp_lenc(int index, struct otp_struct * otp_ptr)
{
	int bank, i;
	int address;

	// select bank:2,7,12,
	bank = 0xc0 + (index*5+2);
	OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp into buffer
	OV8850_write_cmos_sensor(0x3d81, 0x01);
	mdelay(10);

	address = 0x3d00;
	for(i=0;i<16;i++) {
		(* otp_ptr).lenc[i]=OV8850_read_cmos_sensor(address);
		address++;
	}
	
	// disable otp read
	OV8850_write_cmos_sensor(0x3d81, 0x00);

	clear_otp_buffer();

	// select 2nd bank
	bank++;
	OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp
	OV8850_write_cmos_sensor(0x3d81, 0x01);
	mdelay(10);

	address = 0x3d00;
	for(i=16;i<32;i++) {
		(* otp_ptr).lenc[i]=OV8850_read_cmos_sensor(address);
		address++;
	}

	// disable otp read
	OV8850_write_cmos_sensor(0x3d81, 0x00);

	clear_otp_buffer();

	// select 3rd bank
	bank++;
	OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp
	OV8850_write_cmos_sensor(0x3d81, 0x01);
	mdelay(10);

	address = 0x3d00;
	for(i=32;i<48;i++) {
		(* otp_ptr).lenc[i]=OV8850_read_cmos_sensor(address);
		address++;
	}

	// disable otp read
	OV8850_write_cmos_sensor(0x3d81, 0x00);

	clear_otp_buffer();

	// select 4th bank
	bank++;
	OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp
	OV8850_write_cmos_sensor(0x3d81, 0x01);
	mdelay(10);

	address = 0x3d00;
	for(i=48;i<62;i++) {
		(* otp_ptr).lenc[i]=OV8850_read_cmos_sensor(address);
		address++;
	}

	// disable otp read
	OV8850_write_cmos_sensor(0x3d81, 0x00);

	clear_otp_buffer();

	return 0;	
}


// R_gain, sensor red gain of AWB, 0x400 =1
// G_gain, sensor green gain of AWB, 0x400 =1
// B_gain, sensor blue gain of AWB, 0x400 =1
// return 0;
int update_awb_gain(int R_gain, int G_gain, int B_gain)
{
	if (R_gain>=0x400) {
		OV8850_write_cmos_sensor(0x3400, R_gain>>8);
		OV8850_write_cmos_sensor(0x3401, R_gain & 0x00ff);
	}

	if (G_gain>=0x400) {
		OV8850_write_cmos_sensor(0x3402, G_gain>>8);
		OV8850_write_cmos_sensor(0x3403, G_gain & 0x00ff);
	}

	if (B_gain>=0x400) {
		OV8850_write_cmos_sensor(0x3404, B_gain>>8);
		OV8850_write_cmos_sensor(0x3405, B_gain & 0x00ff);
	}

    SENSORDB("update_awb_gain, 0x3400 = %x\n", OV8850_read_cmos_sensor(0x3400));
    SENSORDB("update_awb_gain, 0x3401 = %x\n", OV8850_read_cmos_sensor(0x3401));
    SENSORDB("update_awb_gain, 0x3402 = %x\n", OV8850_read_cmos_sensor(0x3402));
    SENSORDB("update_awb_gain, 0x3403 = %x\n", OV8850_read_cmos_sensor(0x3403));
    SENSORDB("update_awb_gain, 0x3404 = %x\n", OV8850_read_cmos_sensor(0x3404));
    SENSORDB("update_awb_gain, 0x3405 = %x\n", OV8850_read_cmos_sensor(0x3405));
	
	return 0;
}

// call this function after OV8850 initialization
// otp_ptr: pointer of otp_struct
int update_lenc(struct otp_struct * otp_ptr)
{
	int i, temp;
	temp = 0x80|OV8850_read_cmos_sensor(0x5000);
	OV8850_write_cmos_sensor(0x5000, temp);

	for(i=0;i<62;i++) {
		OV8850_write_cmos_sensor(0x5800 + i, (*otp_ptr).lenc[i]);
	}

	for(i=0;i<62;i++){
		SENSORDB("update_lenc, 0x5800 + %d = %x\n", i,OV8850_read_cmos_sensor((0x5800)+i));
	}

	return 0;
}

// call this function after OV8850 initialization
// return value: 0 update success
//		1, no OTP
int update_otp_wb()
{
	struct otp_struct current_otp;
	int otp_index;
	int R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
	int rg,bg;
	printk("update_otp_wb\n");


	// R/G and B/G of current camera module is read out from sensor OTP
	// check first OTP with valid data
	
	otp_index = check_otp_wb();
	if(otp_index==-1)
	{	
		// no valid wb OTP data
		return 1;
	}	

	read_otp_wb(otp_index, &current_otp);



	if(current_otp.light_rg==0) {
		// no light source information in OTP, light factor = 1
		rg = current_otp.rg_ratio;
	}
	else {
		rg = current_otp.rg_ratio * (current_otp.light_rg +512) /1024;
	}
	
	if(current_otp.light_bg==0) {
		// not light source information in OTP, light factor = 1
		bg = current_otp.bg_ratio;
	}
	else {
		bg = current_otp.bg_ratio * (current_otp.light_bg +512) /1024;
	}

    SENSORDB("OV8850_Upate_Otp_WB, r/g:0x%x, b/g:0x%x\n", rg, bg);

	//calculate G gain
	//0x400 = 1x gain
	if(bg < BG_Ratio_Typical) {
		if (rg< RG_Ratio_Typical) {
			// current_otp.bg_ratio < BG_Ratio_typical &&  
			// current_otp.rg_ratio < RG_Ratio_typical
   			G_gain = 0x400;
			B_gain = 0x400 * BG_Ratio_Typical / bg;
    		R_gain = 0x400 * RG_Ratio_Typical / rg; 
		}
		else {
			// current_otp.bg_ratio < BG_Ratio_typical &&  
			// current_otp.rg_ratio >= RG_Ratio_typical
    		R_gain = 0x400;
   	 		G_gain = 0x400 * rg / RG_Ratio_Typical;
    		B_gain = G_gain * BG_Ratio_Typical /bg;
		}
	}
	else {
		if (rg < RG_Ratio_Typical) {
			// current_otp.bg_ratio >= BG_Ratio_typical &&  
			// current_otp.rg_ratio < RG_Ratio_typical
    		B_gain = 0x400;
    		G_gain = 0x400 * bg / BG_Ratio_Typical;
    		R_gain = G_gain * RG_Ratio_Typical / rg;
		}
		else {
			// current_otp.bg_ratio >= BG_Ratio_typical &&  
			// current_otp.rg_ratio >= RG_Ratio_typical
    		G_gain_B = 0x400 * bg / BG_Ratio_Typical;
   	 		G_gain_R = 0x400 * rg / RG_Ratio_Typical;

    		if(G_gain_B > G_gain_R ) {
        				B_gain = 0x400;
        				G_gain = G_gain_B;
 	     			R_gain = G_gain * RG_Ratio_Typical /rg;
  			}
    		else {
        			R_gain = 0x400;
       				G_gain = G_gain_R;
        			B_gain = G_gain * BG_Ratio_Typical / bg;
			}
    	}    
	}

	update_awb_gain(R_gain, G_gain, B_gain);

	return 0;

}


int update_otp_lenc()
{
	struct otp_struct current_otp;
	int otp_index;

	// R/G and B/G of current camera module is read out from sensor OTP
	// check first OTP with valid data
	
	otp_index = check_otp_wb();
	if(otp_index==-1)
	{	
		// no valid wb OTP data
		return 1;
	}	
	read_otp_lenc(otp_index, &current_otp);

	update_lenc(&current_otp);
	return 0;
}


#endif


// Always Do check_dcblc
// return: 	0 ¨C use module DCBLC, 
//			1 ¨C use sensor DCBL 
//			2 ¨C use defualt DCBLC
int check_dcblc()
{
	int bank, dcblc;
	int address;
	int temp, flag;

	// select bank 31
	bank = 0xc0 | 31;
	OV8850_write_cmos_sensor(0x3d84, bank);

	// read otp into buffer
	OV8850_write_cmos_sensor(0x3d81, 0x01);
	mdelay(10);

	temp = OV8850_read_cmos_sensor(0x4000);
	address = 0x3d0b;
	dcblc = OV8850_read_cmos_sensor(address);

	if ((dcblc>=0x15) && (dcblc<=0x40)){
		// module DCBLC value is valid
		if((temp && 0x08)==0) {
			// DCBLC auto load
			flag = 0;
			clear_otp_buffer();
			return flag;
		}
	}

	address--;
	dcblc = OV8850_read_cmos_sensor(address);
	if ((dcblc>=0x10) && (dcblc<=0x40)){
		// sensor DCBLC value is valid
		temp = temp | 0x08;		// DCBLC manual load enable
		OV8850_write_cmos_sensor(0x4000, temp);
		OV8850_write_cmos_sensor(0x4006, dcblc);	// manual load sensor level DCBLC

		flag = 1;				// sensor level DCBLC is used
	}
	else{
		OV8850_write_cmos_sensor(0x4006, 0x20);
		flag = 2;				// default DCBLC is used
	}

    SENSORDB("check_dcblc, 0x4000 = %x\n", OV8850_read_cmos_sensor(0x4000));
    SENSORDB("check_dcblc, 0x4006 = %x\n", OV8850_read_cmos_sensor(0x4006));
	// disable otp read
	OV8850_write_cmos_sensor(0x3d81, 0x00);

	clear_otp_buffer();

	return flag;	
}

/*****************************  OTP Feature  End**********************************/

void OV8850_Write_Shutter(kal_uint16 ishutter)
{

    kal_uint16 extra_shutter = 0;
    kal_uint16 realtime_fp = 0;
    kal_uint16 frame_height = 0;
    kal_uint16 line_length = 0;

    unsigned long flags;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850_write_shutter:%x \n",ishutter);
#endif
   if (!ishutter) ishutter = 1; /* avoid 0 */

    if (OV8850_sensor.pv_mode){
        //line_length = OV8850_PV_PERIOD_PIXEL_NUMS;
        frame_height = OV8850_PV_PERIOD_LINE_NUMS + OV8850_sensor.dummy_lines;
    }
    else if (OV8850_sensor.video_mode) {
        //line_length = OV8850_VIDEO_PERIOD_PIXEL_NUMS;
		  frame_height = OV8850_VIDEO_PERIOD_LINE_NUMS + OV8850_sensor.dummy_lines;
    }
    else{
        //line_length = OV8850_FULL_PERIOD_PIXEL_NUMS;
        frame_height = OV8850_FULL_PERIOD_LINE_NUMS + OV8850_sensor.dummy_lines;
    }

    if(ishutter > (frame_height -4))
    {
		extra_shutter = ishutter - frame_height + 4;
        SENSORDB("[shutter > frame_height] frame_height:%x extra_shutter:%x \n",frame_height,extra_shutter);
    }
    else  
    {
        extra_shutter = 0;
    }
    frame_height += extra_shutter;
    OV8850_sensor.frame_height = frame_height;
    SENSORDB("OV8850_sensor.is_autofliker:%x, OV8850_sensor.frame_height: %x \n",OV8850_sensor.is_autofliker,OV8850_sensor.frame_height);

    if(OV8850_sensor.is_autofliker == KAL_TRUE)
    {
        realtime_fp = OV8850_sensor.pclk *10 / (OV8850_sensor.line_length * OV8850_sensor.frame_height);
        SENSORDB("[OV8850_Write_Shutter]pv_clk:%d\n",OV8850_sensor.pclk);
        SENSORDB("[OV8850_Write_Shutter]line_length:%d\n",OV8850_sensor.line_length);
        SENSORDB("[OV8850_Write_Shutter]frame_height:%d\n",OV8850_sensor.frame_height);
        SENSORDB("[OV8850_Write_Shutter]framerate(10base):%d\n",realtime_fp);

        if((realtime_fp >= 297)&&(realtime_fp <= 303))
        {
            realtime_fp = 296;
            spin_lock_irqsave(&OV8850_drv_lock,flags);
            OV8850_sensor.frame_height = OV8850_sensor.pclk *10 / (OV8850_sensor.line_length * realtime_fp);
            spin_unlock_irqrestore(&OV8850_drv_lock,flags);

            SENSORDB("[autofliker realtime_fp=30,extern heights slowdown to 29.6fps][height:%d]",OV8850_sensor.frame_height);
        }
      else if((realtime_fp >= 147)&&(realtime_fp <= 153))
        {
            realtime_fp = 146;
            spin_lock_irqsave(&OV8850_drv_lock,flags);
            OV8850_sensor.frame_height = OV8850_sensor.pclk *10 / (OV8850_sensor.line_length * realtime_fp);
            spin_unlock_irqrestore(&OV8850_drv_lock,flags);
            SENSORDB("[autofliker realtime_fp=15,extern heights slowdown to 14.6fps][height:%d]",OV8850_sensor.frame_height);
        }
    //OV8850_sensor.frame_height = OV8850_sensor.frame_height +(OV8850_sensor.frame_height>>7);

    }
    OV8850_write_cmos_sensor(0x380e, (OV8850_sensor.frame_height>>8)&0xFF);
    OV8850_write_cmos_sensor(0x380f, (OV8850_sensor.frame_height)&0xFF);

    OV8850_write_cmos_sensor(0x3500, (ishutter >> 12) & 0xF);
    OV8850_write_cmos_sensor(0x3501, (ishutter >> 4) & 0xFF);
    OV8850_write_cmos_sensor(0x3502, (ishutter << 4) & 0xFF);

}


/*************************************************************************
* FUNCTION
*   OV8850_Set_Dummy
*
* DESCRIPTION
*   This function set dummy pixel or dummy line of OV8850
*
* PARAMETERS
*   iPixels : dummy pixel
*   iLines :  dummy linel
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/

static void OV8850_Set_Dummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
    kal_uint16 line_length, frame_height;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850_Set_Dummy:iPixels:%x; iLines:%x \n",iPixels,iLines);
#endif
    OV8850_sensor.dummy_lines = iLines;
    OV8850_sensor.dummy_pixels = iPixels;

    switch (CurrentScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            line_length = OV8850_PV_PERIOD_PIXEL_NUMS + iPixels;
            frame_height = OV8850_PV_PERIOD_LINE_NUMS + iLines;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            line_length = OV8850_VIDEO_PERIOD_PIXEL_NUMS + iPixels;
            frame_height = OV8850_VIDEO_PERIOD_LINE_NUMS + iLines;
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
            line_length = OV8850_FULL_PERIOD_PIXEL_NUMS + iPixels;
            frame_height = OV8850_FULL_PERIOD_LINE_NUMS + iLines;
            break;
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            line_length = OV8850_FULL_PERIOD_PIXEL_NUMS + iPixels;
            frame_height = OV8850_FULL_PERIOD_LINE_NUMS + iLines;
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
            line_length = OV8850_PV_PERIOD_PIXEL_NUMS + iPixels;
            frame_height = OV8850_PV_PERIOD_LINE_NUMS + iLines;
            break;
    }

#ifdef OV8850_DRIVER_TRACE
    SENSORDB("line_length:%x; frame_height:%x \n",line_length,frame_height);
#endif

    if ((line_length >= 0x1FFF)||(frame_height >= 0xFFF))
    {
        #ifdef OV8850_DRIVER_TRACE
        SENSORDB("Warnning: line length or frame height is overflow!!!!!!!!  \n");
        #endif
        return ERROR_NONE;
    }
//	if((line_length == OV8850_sensor.line_length)&&(frame_height == OV8850_sensor.frame_height))
//		return ;
    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.line_length = line_length;
    OV8850_sensor.frame_height = frame_height;
    spin_unlock(&OV8850_drv_lock);

    SENSORDB("line_length:%x; frame_height:%x \n",line_length,frame_height);

    /*  Add dummy pixels: */
    /* 0x380c [0:4], 0x380d defines the PCLKs in one line of OV8850  */  
    /* Add dummy lines:*/
    /* 0x380e [0:1], 0x380f defines total lines in one frame of OV8850 */
    OV8850_write_cmos_sensor(0x380c, line_length >> 8);
    OV8850_write_cmos_sensor(0x380d, line_length & 0xFF);
    OV8850_write_cmos_sensor(0x380e, frame_height >> 8);
    OV8850_write_cmos_sensor(0x380f, frame_height & 0xFF);
    return ERROR_NONE;
}   /*  OV8850_Set_Dummy    */


/*************************************************************************
* FUNCTION
*	OV8850_SetShutter
*
* DESCRIPTION
*	This function set e-shutter of OV8850 to change exposure time.
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


void set_OV8850_shutter(kal_uint16 iShutter)
{

    unsigned long flags;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("set_OV8850_shutter:%x \n",iShutter);
#endif

    #if 0
    if((OV8850_sensor.pv_mode == KAL_FALSE)&&(OV8850_sensor.is_zsd == KAL_FALSE))
    {
        SENSORDB("[set_OV8850_shutter]now is in 1/4size cap mode\n");
        //return;
    }
    else if((OV8850_sensor.is_zsd == KAL_TRUE)&&(OV8850_sensor.is_zsd_cap == KAL_TRUE))
    {
        SENSORDB("[set_OV8850_shutter]now is in zsd cap mode\n");

        //SENSORDB("[set_OV8850_shutter]0x3500:%x\n",OV8850_read_cmos_sensor(0x3500));
        //SENSORDB("[set_OV8850_shutter]0x3500:%x\n",OV8850_read_cmos_sensor(0x3501));
        //SENSORDB("[set_OV8850_shutter]0x3500:%x\n",OV8850_read_cmos_sensor(0x3502));
        //return;
    }
    #endif
    #if 0
    if(OV8850_sensor.shutter == iShutter)
    {
        SENSORDB("[set_OV8850_shutter]shutter is the same with previous, skip\n");
        return;
    }
    #endif

    spin_lock_irqsave(&OV8850_drv_lock,flags);
    OV8850_sensor.shutter = iShutter;
    spin_unlock_irqrestore(&OV8850_drv_lock,flags);

    OV8850_Write_Shutter(iShutter);

}   /*  Set_OV8850_Shutter */

 kal_uint16 OV8850Gain2Reg(const kal_uint16 iGain)
{
    kal_uint16 iReg = 0x00;

    //iReg = ((iGain / BASEGAIN) << 4) + ((iGain % BASEGAIN) * 16 / BASEGAIN);
    iReg = iGain *16 / BASEGAIN;

    iReg = iReg & 0xFF;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850Gain2Reg:iGain:%x; iReg:%x \n",iGain,iReg);
#endif
    return iReg;
}


kal_uint16 OV8850_SetGain(kal_uint16 iGain)
{
   kal_uint16 iReg;
   unsigned long flags;
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850_SetGain:%x;\n",iGain);
#endif

    #if 0
    if(OV8850_sensor.gain == iGain)
    {
        SENSORDB("[OV8850_SetGain]:gain is the same with previous,skip\n");
        return ERROR_NONE;
    }
    #endif
   spin_lock_irqsave(&OV8850_drv_lock,flags);
   OV8850_sensor.gain = iGain;
   spin_unlock_irqrestore(&OV8850_drv_lock,flags);

  iReg = OV8850Gain2Reg(iGain);
   
    if (iReg < 0x10) //MINI gain is 0x10	 16 = 1x
    {
        iReg = 0x10;
    }

    else if(iReg > 0xFF) //max gain is 0xFF
    {
        iReg = 0xFF;
    }

    //OV8850_write_cmos_sensor(0x350a, (iReg>>8)&0xFF);
    OV8850_write_cmos_sensor(0x350b, iReg&0xFF);//only use 0x350b for gain control
    return ERROR_NONE;
}




/*************************************************************************
* FUNCTION
*	OV8850_SetGain
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

#if 0
void OV8850_set_isp_driving_current(kal_uint16 current)
{
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850_set_isp_driving_current:current:%x;\n",current);
#endif
  //iowrite32((0x2 << 12)|(0<<28)|(0x8880888), 0xF0001500);
}
#endif

/*************************************************************************
* FUNCTION
*	OV8850_NightMode
*
* DESCRIPTION
*	This function night mode of OV8850.
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
void OV8850_night_mode(kal_bool enable)
{
}   /*  OV8850_NightMode    */


/* write camera_para to sensor register */
static void OV8850_camera_para_to_sensor(void)
{
    kal_uint32 i;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850_camera_para_to_sensor\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV8850_sensor.eng.reg[i].Addr; i++)
  {
    OV8850_write_cmos_sensor(OV8850_sensor.eng.reg[i].Addr, OV8850_sensor.eng.reg[i].Para);
  }
  for (i = OV8850_FACTORY_START_ADDR; 0xFFFFFFFF != OV8850_sensor.eng.reg[i].Addr; i++)
  {
    OV8850_write_cmos_sensor(OV8850_sensor.eng.reg[i].Addr, OV8850_sensor.eng.reg[i].Para);
  }
  OV8850_SetGain(OV8850_sensor.gain); /* update gain */
}

/* update camera_para from sensor register */
static void OV8850_sensor_to_camera_para(void)
{
  kal_uint32 i;
  kal_uint32 temp_data;
  
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850_sensor_to_camera_para\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV8850_sensor.eng.reg[i].Addr; i++)
  {
    temp_data = OV8850_read_cmos_sensor(OV8850_sensor.eng.reg[i].Addr);

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.eng.reg[i].Para = temp_data;
    spin_unlock(&OV8850_drv_lock);

    }
  for (i = OV8850_FACTORY_START_ADDR; 0xFFFFFFFF != OV8850_sensor.eng.reg[i].Addr; i++)
  {
    temp_data = OV8850_read_cmos_sensor(OV8850_sensor.eng.reg[i].Addr);

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.eng.reg[i].Para = temp_data;
    spin_unlock(&OV8850_drv_lock);
  }
}

/* ------------------------ Engineer mode ------------------------ */
inline static void OV8850_get_sensor_group_count(kal_int32 *sensor_count_ptr)
{
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850_get_sensor_group_count\n");
#endif
  *sensor_count_ptr = OV8850_GROUP_TOTAL_NUMS;
}

inline static void OV8850_get_sensor_group_info(MSDK_SENSOR_GROUP_INFO_STRUCT *para)
{
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850_get_sensor_group_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8850_PRE_GAIN:
    sprintf(para->GroupNamePtr, "CCT");
    para->ItemCount = 5;
    break;
  case OV8850_CMMCLK_CURRENT:
    sprintf(para->GroupNamePtr, "CMMCLK Current");
    para->ItemCount = 1;
    break;
  case OV8850_FRAME_RATE_LIMITATION:
    sprintf(para->GroupNamePtr, "Frame Rate Limitation");
    para->ItemCount = 2;
    break;
  case OV8850_REGISTER_EDITOR:
    sprintf(para->GroupNamePtr, "Register Editor");
    para->ItemCount = 2;
    break;
  default:
    ASSERT(0);
  }
}

inline static void OV8850_get_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{

  const static kal_char *cct_item_name[] = {"SENSOR_BASEGAIN", "Pregain-R", "Pregain-Gr", "Pregain-Gb", "Pregain-B"};
  const static kal_char *editer_item_name[] = {"REG addr", "REG value"};
  
#ifdef OV8850_DRIVER_TRACE
	 SENSORDB("OV8850_get_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8850_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV8850_SENSOR_BASEGAIN:
    case OV8850_PRE_GAIN_R_INDEX:
    case OV8850_PRE_GAIN_Gr_INDEX:
    case OV8850_PRE_GAIN_Gb_INDEX:
    case OV8850_PRE_GAIN_B_INDEX:
      break;
    default:
      ASSERT(0);
    }
    sprintf(para->ItemNamePtr, cct_item_name[para->ItemIdx - OV8850_SENSOR_BASEGAIN]);
    para->ItemValue = OV8850_sensor.eng.cct[para->ItemIdx].Para * 1000 / BASEGAIN;
    para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
    para->Min = OV8850_MIN_ANALOG_GAIN * 1000;
    para->Max = OV8850_MAX_ANALOG_GAIN * 1000;
    break;
  case OV8850_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Drv Cur[2,4,6,8]mA");
      switch (OV8850_sensor.eng.reg[OV8850_CMMCLK_CURRENT_INDEX].Para)
      {
      case ISP_DRIVING_2MA:
        para->ItemValue = 2;
        break;
      case ISP_DRIVING_4MA:
        para->ItemValue = 4;
        break;
      case ISP_DRIVING_6MA:
        para->ItemValue = 6;
        break;
      case ISP_DRIVING_8MA:
        para->ItemValue = 8;
        break;
      default:
        ASSERT(0);
      }
      para->IsTrueFalse = para->IsReadOnly = KAL_FALSE;
      para->IsNeedRestart = KAL_TRUE;
      para->Min = 2;
      para->Max = 8;
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8850_FRAME_RATE_LIMITATION:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Max Exposure Lines");
      para->ItemValue = 5998;
      break;
    case 1:
      sprintf(para->ItemNamePtr, "Min Frame Rate");
      para->ItemValue = 5;
      break;
    default:
      ASSERT(0);
    }
    para->IsTrueFalse = para->IsNeedRestart = KAL_FALSE;
    para->IsReadOnly = KAL_TRUE;
    para->Min = para->Max = 0;
    break;
  case OV8850_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
    case 0:
    case 1:
      sprintf(para->ItemNamePtr, editer_item_name[para->ItemIdx]);
      para->ItemValue = 0;
      para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
      para->Min = 0;
      para->Max = (para->ItemIdx == 0 ? 0xFFFF : 0xFF);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
}

inline static kal_bool OV8850_set_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{
  kal_uint16 temp_para;
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850_set_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8850_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV8850_SENSOR_BASEGAIN:
    case OV8850_PRE_GAIN_R_INDEX:
    case OV8850_PRE_GAIN_Gr_INDEX:
    case OV8850_PRE_GAIN_Gb_INDEX:
    case OV8850_PRE_GAIN_B_INDEX:
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.eng.cct[para->ItemIdx].Para = para->ItemValue * BASEGAIN / 1000;
        spin_unlock(&OV8850_drv_lock);

        OV8850_SetGain(OV8850_sensor.gain); /* update gain */
        break;
    default:
        ASSERT(0);
    }
    break;
  case OV8850_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      switch (para->ItemValue)
      {
      case 2:
        temp_para = ISP_DRIVING_2MA;
        break;
      case 3:
      case 4:
        temp_para = ISP_DRIVING_4MA;
        break;
      case 5:
      case 6:
        temp_para = ISP_DRIVING_6MA;
        break;
      default:
        temp_para = ISP_DRIVING_8MA;
        break;
      }
        //OV8850_set_isp_driving_current((kal_uint16)temp_para);
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.eng.reg[OV8850_CMMCLK_CURRENT_INDEX].Para = temp_para;
        spin_unlock(&OV8850_drv_lock);
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8850_FRAME_RATE_LIMITATION:
    ASSERT(0);
    break;
  case OV8850_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
      static kal_uint32 fac_sensor_reg;
    case 0:
      if (para->ItemValue < 0 || para->ItemValue > 0xFFFF) return KAL_FALSE;
      fac_sensor_reg = para->ItemValue;
      break;
    case 1:
      if (para->ItemValue < 0 || para->ItemValue > 0xFF) return KAL_FALSE;
      OV8850_write_cmos_sensor(fac_sensor_reg, para->ItemValue);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
  return KAL_TRUE;
}

static int Flag = 0;

//Sensor globle setting: 4 lane 504M bps/lane
static void OV8850_global_setting(void)
{
	kal_uint16 BLC_Reg = 0;
	SENSORDB("OV8850_Sensor_Init enter :\n ");	
	
	/**************************************************
	** Input Clock = 24MHz
	** 4-lane MIPI
	** Max data rate = 648Mbps/lane
	** DCBLC ON with auto load mode, LENC OFF, DPC ON, MWB ON
	** MIPI data rate = 648Mbps/lane
	** free running MIPI clock with short package
	***************************************************/
	/*Slave_ID = 0x6c(SID = H) or 0x20(SID = L)*/
    SENSORDB("OV8850_globle_setting  start \n");
    OV8850_write_cmos_sensor(0x0103, 0x01);  /*software reset*/
    mdelay(6);
    OV8850_write_cmos_sensor(0x0102, 0x01);
    OV8850_write_cmos_sensor(0x3002, 0x08);
    OV8850_write_cmos_sensor(0x3004, 0x00);
    OV8850_write_cmos_sensor(0x3005, 0x00);
    OV8850_write_cmos_sensor(0x3011, 0x41);//4
    OV8850_write_cmos_sensor(0x3012, 0x08);
    OV8850_write_cmos_sensor(0x3014, 0x4a);
    OV8850_write_cmos_sensor(0x3015, 0x0a);//4 
    OV8850_write_cmos_sensor(0x3021, 0x00);
    OV8850_write_cmos_sensor(0x3022, 0x02);
    OV8850_write_cmos_sensor(0x3081, 0x02);
    OV8850_write_cmos_sensor(0x3083, 0x01);
	
    //OV8850_write_cmos_sensor(0x3091, 0x11); 

    OV8850_write_cmos_sensor(0x3092, 0x00);
	OV8850_write_cmos_sensor(0x3093, 0x00);
    OV8850_write_cmos_sensor(0x309a, 0x00);
    OV8850_write_cmos_sensor(0x309b, 0x00);
    OV8850_write_cmos_sensor(0x309c, 0x00);
    //OV8850_write_cmos_sensor(0x30b3, 0x2b);//516Mbps/lane
	// 780Mbps/lane	   
    //OV8850_write_cmos_sensor(0x30b3, 0x3c);//516Mbps/lane
    OV8850_write_cmos_sensor(0x30b3, 0x64); //780M
    //OV8850_write_cmos_sensor(0x30b3, 0x32); //780M
    
    OV8850_write_cmos_sensor(0x30b4, 0x03);
    OV8850_write_cmos_sensor(0x30b5, 0x04);
    OV8850_write_cmos_sensor(0x30b6, 0x01);
    OV8850_write_cmos_sensor(0x3104, 0xa1);
    OV8850_write_cmos_sensor(0x3106, 0x01);

    OV8850_write_cmos_sensor(0x3503, 0x07);
    OV8850_write_cmos_sensor(0x350a, 0x00);
    OV8850_write_cmos_sensor(0x350b, 0x38);
    OV8850_write_cmos_sensor(0x3602, 0x70);
    OV8850_write_cmos_sensor(0x3620, 0x64);
    OV8850_write_cmos_sensor(0x3622, 0x0f);
    OV8850_write_cmos_sensor(0x3623, 0x68);

    OV8850_write_cmos_sensor(0x3625, 0x40);
    OV8850_write_cmos_sensor(0x3631, 0x83);
    OV8850_write_cmos_sensor(0x3633, 0x34);
    OV8850_write_cmos_sensor(0x3634, 0x03);
    OV8850_write_cmos_sensor(0x364c, 0x00);
    OV8850_write_cmos_sensor(0x364d, 0x00);
    OV8850_write_cmos_sensor(0x364e, 0x00);
    OV8850_write_cmos_sensor(0x364f, 0x00);
    OV8850_write_cmos_sensor(0x3660, 0x80);
    OV8850_write_cmos_sensor(0x3662, 0x10);
    OV8850_write_cmos_sensor(0x3665, 0x00);
    OV8850_write_cmos_sensor(0x3666, 0x00);
    OV8850_write_cmos_sensor(0x366f, 0x20);

    OV8850_write_cmos_sensor(0x3703, 0x2e);
    OV8850_write_cmos_sensor(0x3732, 0x05);
	OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373d, 0x22);
    OV8850_write_cmos_sensor(0x3754, 0xc0);
    OV8850_write_cmos_sensor(0x3756, 0x2a);
    OV8850_write_cmos_sensor(0x3759, 0x0f);
    OV8850_write_cmos_sensor(0x376b, 0x44);
    OV8850_write_cmos_sensor(0x3795, 0x00);
    OV8850_write_cmos_sensor(0x379c, 0x0c);
    OV8850_write_cmos_sensor(0x3810, 0x00);
    OV8850_write_cmos_sensor(0x3811, 0x04);
    OV8850_write_cmos_sensor(0x3812, 0x00);
    OV8850_write_cmos_sensor(0x3813, 0x04);
    OV8850_write_cmos_sensor(0x3820, 0x10);
    OV8850_write_cmos_sensor(0x3821, 0x0e);
	
    OV8850_write_cmos_sensor(0x3826, 0x00);
    //OV8850_write_cmos_sensor(0x3a04, 0x09);
    //OV8850_write_cmos_sensor(0x3a05, 0xa9);
    OV8850_write_cmos_sensor(0x4000, 0x10);//DCLBC auto load mode

    OV8850_write_cmos_sensor(0x4002, 0xc5);
    OV8850_write_cmos_sensor(0x4005, 0x18);
    OV8850_write_cmos_sensor(0x4006, 0x20);
	OV8850_write_cmos_sensor(0x4007, 0x90);
    OV8850_write_cmos_sensor(0x4008, 0x20);//DCBLC on
    OV8850_write_cmos_sensor(0x4009, 0x10);
    OV8850_write_cmos_sensor(0x404f, 0xA0);

    OV8850_write_cmos_sensor(0x4100, 0x1d);
    OV8850_write_cmos_sensor(0x4101, 0x23);
    OV8850_write_cmos_sensor(0x4102, 0x44);
    OV8850_write_cmos_sensor(0x4104, 0x5c);
    OV8850_write_cmos_sensor(0x4109, 0x03);

    OV8850_write_cmos_sensor(0x4300, 0xff);
    OV8850_write_cmos_sensor(0x4301, 0x00);
    OV8850_write_cmos_sensor(0x4315, 0x00);
    OV8850_write_cmos_sensor(0x4512, 0x01);
    OV8850_write_cmos_sensor(0x4800, 0x14);//short pacakge enable
    //OV8850_write_cmos_sensor(0x4837, 0x10);
	
    //OV8850_write_cmos_sensor(0x4837, 0x0A);
	OV8850_write_cmos_sensor(0x4837, 0x0c);

    OV8850_write_cmos_sensor(0x4a00, 0xaa);
    OV8850_write_cmos_sensor(0x4a03, 0x01);
    OV8850_write_cmos_sensor(0x4a05, 0x08);
    OV8850_write_cmos_sensor(0x4d00, 0x04);
    OV8850_write_cmos_sensor(0x4d01, 0x52);
    OV8850_write_cmos_sensor(0x4d02, 0xfe);
    OV8850_write_cmos_sensor(0x4d03, 0x05);
    OV8850_write_cmos_sensor(0x4d04, 0xff);
    OV8850_write_cmos_sensor(0x4d05, 0xff);
    OV8850_write_cmos_sensor(0x5000, 0x06);//LENC ON
    OV8850_write_cmos_sensor(0x5001, 0x01);//MWB ON
    OV8850_write_cmos_sensor(0x5002, 0x80);
    OV8850_write_cmos_sensor(0x5013, 0x00);  // Add according to reference
    OV8850_write_cmos_sensor(0x5041, 0x04);
    OV8850_write_cmos_sensor(0x5043, 0x48);
    OV8850_write_cmos_sensor(0x5e00, 0x00);
    OV8850_write_cmos_sensor(0x5e10, 0x1c);
    
	//Add 2M size preview for default
	mdelay(5);
	
    //OV8850_write_cmos_sensor(0x0100, 0x01);
	#if 1
    OV8850_write_cmos_sensor(0x0100, 0x00);

	//Mipi lane setting, PLL setting
	#ifdef OV8850_2_LANE
    OV8850_write_cmos_sensor(0x3011, 0x21);// 2 lane MIPI
    OV8850_write_cmos_sensor(0x3015, 0xca);// 2 lane enable
    
	OV8850_write_cmos_sensor(0x3090, 0x03);// PLL2 prediv 
    OV8850_write_cmos_sensor(0x3091, 0x22);// PLL2 multiplier
    OV8850_write_cmos_sensor(0x3092, 0x00);
	OV8850_write_cmos_sensor(0x3093, 0x02);
	#else //default use 4 lane
    OV8850_write_cmos_sensor(0x3011, 0x41);//2 
    OV8850_write_cmos_sensor(0x3015, 0x0a);// MIPI mode,  ca
    
	OV8850_write_cmos_sensor(0x3090, 0x02);//216MHz SCLK 
    OV8850_write_cmos_sensor(0x3091, 0x12);
	OV8850_write_cmos_sensor(0x3092, 0x00);
	OV8850_write_cmos_sensor(0x3093, 0x00);
	#endif
    
    
    OV8850_write_cmos_sensor(0x3094, 0x00);
    OV8850_write_cmos_sensor(0x3098, 0x03);//240MHz DAC
    OV8850_write_cmos_sensor(0x3099, 0x1e);//1c
    
    OV8850_write_cmos_sensor(0x30b3, 0x36);//648Mbps/lane, 3C:780M
    OV8850_write_cmos_sensor(0x30b4, 0x02);
    OV8850_write_cmos_sensor(0x30b5, 0x04);
    OV8850_write_cmos_sensor(0x30b6, 0x01);
	
    OV8850_write_cmos_sensor(0x3500, 0x00);
    OV8850_write_cmos_sensor(0x3501, 0x7c);
    OV8850_write_cmos_sensor(0x3502, 0x00);
	
    OV8850_write_cmos_sensor(0x3624, 0x00);
    OV8850_write_cmos_sensor(0x3680, 0xe0);
    OV8850_write_cmos_sensor(0x3702, 0xf3);//db
    OV8850_write_cmos_sensor(0x3704, 0x71);
    OV8850_write_cmos_sensor(0x3708, 0xe6);
    OV8850_write_cmos_sensor(0x3709, 0xc3);
    OV8850_write_cmos_sensor(0x371F, 0x0c);//0x18
    OV8850_write_cmos_sensor(0x3739, 0x30);
    //OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x20);//38
    OV8850_write_cmos_sensor(0x3781, 0x0c);
    OV8850_write_cmos_sensor(0x3786, 0x16);
	
    OV8850_write_cmos_sensor(0x3796, 0x64);//78
    OV8850_write_cmos_sensor(0x3800, 0x00);
    OV8850_write_cmos_sensor(0x3801, 0x00);
    OV8850_write_cmos_sensor(0x3802, 0x00);
    OV8850_write_cmos_sensor(0x3803, 0x00);
    OV8850_write_cmos_sensor(0x3804, 0x0c);
    OV8850_write_cmos_sensor(0x3805, 0xcf);
    OV8850_write_cmos_sensor(0x3806, 0x09);
    OV8850_write_cmos_sensor(0x3807, 0x9f);
    OV8850_write_cmos_sensor(0x3808, 0x06);
    OV8850_write_cmos_sensor(0x3809, 0x60);
    OV8850_write_cmos_sensor(0x380a, 0x04);
    OV8850_write_cmos_sensor(0x380b, 0xC8);

	//HTS,VTS setting
	#ifdef OV8850_2_LANE
    OV8850_write_cmos_sensor(0x380c, 0x0E);//for 30fps
    OV8850_write_cmos_sensor(0x380d, 0x18);
    OV8850_write_cmos_sensor(0x380e, 0x04);
    OV8850_write_cmos_sensor(0x380f, 0xE8);//fa
    #else
    OV8850_write_cmos_sensor(0x380c, 0x0E);//for 30fps
    OV8850_write_cmos_sensor(0x380d, 0x18);
    OV8850_write_cmos_sensor(0x380e, 0x07);
    OV8850_write_cmos_sensor(0x380f, 0xcc);//fa
	#endif
    OV8850_write_cmos_sensor(0x3814, 0x31);
    OV8850_write_cmos_sensor(0x3815, 0x31);
    OV8850_write_cmos_sensor(0x3820, 0x11);
    OV8850_write_cmos_sensor(0x3821, 0x0f);
	
    OV8850_write_cmos_sensor(0x3a04, 0x07);
    OV8850_write_cmos_sensor(0x3a05, 0xc8);
	
    OV8850_write_cmos_sensor(0x4001, 0x02);
    OV8850_write_cmos_sensor(0x4004, 0x04);
    //OV8850_write_cmos_sensor(0x4100, 0x04);
    //OV8850_write_cmos_sensor(0x4101, 0x04);
    //OV8850_write_cmos_sensor(0x4102, 0x04);
    //OV8850_write_cmos_sensor(0x4104, 0x04);
    //OV8850_write_cmos_sensor(0x4109, 0x04);

    OV8850_write_cmos_sensor(0x4005, 0x18);
    OV8850_write_cmos_sensor(0x404f, 0xa0);
	OV8850_write_cmos_sensor(0x5013, 0x04);
	OV8850_write_cmos_sensor(0x5045, 0x55);
	OV8850_write_cmos_sensor(0x5048, 0x0f);
    OV8850_write_cmos_sensor(0x0100, 0x01);
	
	//Flag = 0;
	#endif
	/*
	mdelay(5);
	//BLC OTP Check
	OV8850_write_cmos_sensor(0x3d84, 0xdf);
	OV8850_write_cmos_sensor(0x3d81, 0x01);
	mdelay(20);
	BLC_Reg= OV8850_read_cmos_sensor(0x3d0b);
	if(0 == BLC_Reg){
		OV8850_write_cmos_sensor(0x4006, BLC_Reg);
		mdelay(20);
		BLC_Reg= OV8850_read_cmos_sensor(0x3d0a);
		mdelay(20);
		OV8850_write_cmos_sensor(0x4006, BLC_Reg);
	}else{
		OV8850_write_cmos_sensor(0x4000, 0x10);
	}
	mdelay(5);
	*/
	mdelay(5);
	
#ifdef OV8850_USE_OTP
	update_otp_wb();
	update_otp_lenc();
#endif
	check_dcblc();

	
    SENSORDB("OV8850_Sensor_Init exit :\n ");
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static void OV8850_1632_1224_30fps_Mclk24M_setting(void)
{
/*
	//OV8850_1632*1224_setting
	//4lanes: 648Mbps/lane_30fps
    H Total: 0x0e18 = 3608
    V Total: 0x07cc = 1996
	//;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;	  
*/
	SENSORDB("OV8850_1632_1224_30fps_Mclk24M_setting start \n");
//if(Flag == 1)
	//Flag = 0;
//else
    OV8850_write_cmos_sensor(0x0100, 0x00);

	//Mipi lane setting, PLL setting
#ifdef OV8850_2_LANE
    OV8850_write_cmos_sensor(0x3011, 0x21);// 2 lane MIPI
    OV8850_write_cmos_sensor(0x3015, 0xca);// 2 lane enable
    
	OV8850_write_cmos_sensor(0x3090, 0x03);// PLL2 prediv 
    OV8850_write_cmos_sensor(0x3091, 0x22);// PLL2 multiplier
    OV8850_write_cmos_sensor(0x3092, 0x00);
	OV8850_write_cmos_sensor(0x3093, 0x02);
	
	SENSORDB("OV8850_1632_1224_30fps_Mclk24M_setting 2 lane start \n");
#else //default use 4 lane
    OV8850_write_cmos_sensor(0x3011, 0x41);//2 
    OV8850_write_cmos_sensor(0x3015, 0x0a);// MIPI mode,  ca
    
	OV8850_write_cmos_sensor(0x3090, 0x02);//216MHz SCLK 
    OV8850_write_cmos_sensor(0x3091, 0x12);
	OV8850_write_cmos_sensor(0x3092, 0x00);
	OV8850_write_cmos_sensor(0x3093, 0x00);
#endif
    
    
    OV8850_write_cmos_sensor(0x3094, 0x00);
    OV8850_write_cmos_sensor(0x3098, 0x03);//240MHz DAC
    OV8850_write_cmos_sensor(0x3099, 0x1e);//1c
    
    OV8850_write_cmos_sensor(0x30b3, 0x36);//648Mbps/lane, 3C:780M
    OV8850_write_cmos_sensor(0x30b4, 0x02);
    OV8850_write_cmos_sensor(0x30b5, 0x04);
    OV8850_write_cmos_sensor(0x30b6, 0x01);
	
    OV8850_write_cmos_sensor(0x3500, 0x00);
    OV8850_write_cmos_sensor(0x3501, 0x7c);
    OV8850_write_cmos_sensor(0x3502, 0x00);
	
    OV8850_write_cmos_sensor(0x3624, 0x00);
    OV8850_write_cmos_sensor(0x3680, 0xe0);
    OV8850_write_cmos_sensor(0x3702, 0xf3);//db
    OV8850_write_cmos_sensor(0x3704, 0x71);
    OV8850_write_cmos_sensor(0x3708, 0xe6);
    OV8850_write_cmos_sensor(0x3709, 0xc3);
    OV8850_write_cmos_sensor(0x371F, 0x0c);//0x18
    OV8850_write_cmos_sensor(0x3739, 0x30);
    //OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x20);//38
    OV8850_write_cmos_sensor(0x3781, 0x0c);
    OV8850_write_cmos_sensor(0x3786, 0x16);
	
    OV8850_write_cmos_sensor(0x3796, 0x64);//78
    OV8850_write_cmos_sensor(0x3800, 0x00);
    OV8850_write_cmos_sensor(0x3801, 0x00);
    OV8850_write_cmos_sensor(0x3802, 0x00);
    OV8850_write_cmos_sensor(0x3803, 0x00);
    OV8850_write_cmos_sensor(0x3804, 0x0c);
    OV8850_write_cmos_sensor(0x3805, 0xcf);
    OV8850_write_cmos_sensor(0x3806, 0x09);
    OV8850_write_cmos_sensor(0x3807, 0x9f);
    OV8850_write_cmos_sensor(0x3808, 0x06);
    OV8850_write_cmos_sensor(0x3809, 0x60);
    OV8850_write_cmos_sensor(0x380a, 0x04);
    OV8850_write_cmos_sensor(0x380b, 0xC8);

	//HTS,VTS setting
#ifdef OV8850_2_LANE
    OV8850_write_cmos_sensor(0x380c, 0x0E);//for 30fps
    OV8850_write_cmos_sensor(0x380d, 0x18);
    OV8850_write_cmos_sensor(0x380e, 0x04);
    OV8850_write_cmos_sensor(0x380f, 0xE8);//fa
#else
    OV8850_write_cmos_sensor(0x380c, 0x0E);//for 30fps
    OV8850_write_cmos_sensor(0x380d, 0x18);
    OV8850_write_cmos_sensor(0x380e, 0x07);
    OV8850_write_cmos_sensor(0x380f, 0xcc);//fa
#endif
    OV8850_write_cmos_sensor(0x3814, 0x31);
    OV8850_write_cmos_sensor(0x3815, 0x31);
    OV8850_write_cmos_sensor(0x3820, 0x11);
    OV8850_write_cmos_sensor(0x3821, 0x0f);
	
    OV8850_write_cmos_sensor(0x3a04, 0x07);
    OV8850_write_cmos_sensor(0x3a05, 0xc8);
	
    OV8850_write_cmos_sensor(0x4001, 0x02);
    OV8850_write_cmos_sensor(0x4004, 0x04);
    //OV8850_write_cmos_sensor(0x4100, 0x04);
    //OV8850_write_cmos_sensor(0x4101, 0x04);
    //OV8850_write_cmos_sensor(0x4102, 0x04);
    //OV8850_write_cmos_sensor(0x4104, 0x04);
    //OV8850_write_cmos_sensor(0x4109, 0x04);

    OV8850_write_cmos_sensor(0x4005, 0x18);
    OV8850_write_cmos_sensor(0x404f, 0xa0);
	OV8850_write_cmos_sensor(0x5013, 0x04);
	OV8850_write_cmos_sensor(0x5045, 0x55);
	OV8850_write_cmos_sensor(0x5048, 0x0f);
    OV8850_write_cmos_sensor(0x0100, 0x01);
	

	mdelay(30);

	SENSORDB("OV8850_1632_1224_30fps_Mclk24M_setting end \n");
}

static void OV8850_3264_2448_4Lane_25fps_Mclk24M_setting(void)
{
/*
    Raw 10bit 3264*2448 25fps 4lane 648M bps/lane,SCLK=228M
    H Total: 0x0e18 = 3608
    V Total: 0x09da = 2522
*/
    OV8850_write_cmos_sensor(0x0100, 0x00); // software standby
    OV8850_write_cmos_sensor(0x3011, 0x41); // 4 Lane, MIPI enable
    OV8850_write_cmos_sensor(0x3015, 0x0a); // MIPI mode,
    
    OV8850_write_cmos_sensor(0x3090, 0x02); //228MHz SCLK 
    //OV8850_write_cmos_sensor(0x3091, 0x15); 
	OV8850_write_cmos_sensor(0x3091, 0x13);
    OV8850_write_cmos_sensor(0x3092, 0x00); 
    OV8850_write_cmos_sensor(0x3093, 0x00); 
    OV8850_write_cmos_sensor(0x3094, 0x00); 
    OV8850_write_cmos_sensor(0x3098, 0x02); //264MHz DAC 
    OV8850_write_cmos_sensor(0x3099, 0x16);
	
    OV8850_write_cmos_sensor(0x30b3, 0x36);//648Mbps/lane //3c
    OV8850_write_cmos_sensor(0x30b4, 0x02); 
    OV8850_write_cmos_sensor(0x30b5, 0x04); 
    OV8850_write_cmos_sensor(0x30b6, 0x01);

    //OV8850_write_cmos_sensor(0x4837, 0x0c);//780Mbps/lane 3c  //0a


    OV8850_write_cmos_sensor(0x3500, 0x00); 
    OV8850_write_cmos_sensor(0x3501, 0x9c); 
    OV8850_write_cmos_sensor(0x3502, 0x20); 
    OV8850_write_cmos_sensor(0x3624, 0x04);
    OV8850_write_cmos_sensor(0x3680, 0xB0);
    OV8850_write_cmos_sensor(0x3702, 0x6E);
    OV8850_write_cmos_sensor(0x3704, 0x55);
    OV8850_write_cmos_sensor(0x3708, 0xe4); 
    OV8850_write_cmos_sensor(0x3709, 0xc3); 
    OV8850_write_cmos_sensor(0x371F, 0x0D);
    OV8850_write_cmos_sensor(0x3739, 0x80);
    //OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x24);
    OV8850_write_cmos_sensor(0x3781, 0xc8);
    OV8850_write_cmos_sensor(0x3786, 0x08);
    OV8850_write_cmos_sensor(0x3796, 0x43);
    OV8850_write_cmos_sensor(0x3800, 0x00); 
    OV8850_write_cmos_sensor(0x3801, 0x04); 
    OV8850_write_cmos_sensor(0x3802, 0x00); 
    OV8850_write_cmos_sensor(0x3803, 0x0c); 
    OV8850_write_cmos_sensor(0x3804, 0x0c); 
    OV8850_write_cmos_sensor(0x3805, 0xcb); 
    OV8850_write_cmos_sensor(0x3806, 0x09); 
    OV8850_write_cmos_sensor(0x3807, 0xa3); 
    OV8850_write_cmos_sensor(0x3808, 0x0c); 
    OV8850_write_cmos_sensor(0x3809, 0xc0); 
    OV8850_write_cmos_sensor(0x380a, 0x09); 
    OV8850_write_cmos_sensor(0x380b, 0x90); 
    OV8850_write_cmos_sensor(0x380c, 0x0e); 
    OV8850_write_cmos_sensor(0x380d, 0x18); 
    OV8850_write_cmos_sensor(0x380e, 0x09); //09 
    OV8850_write_cmos_sensor(0x380f, 0xda); //da
	
    OV8850_write_cmos_sensor(0x3814, 0x11); 
    OV8850_write_cmos_sensor(0x3815, 0x11); 
    OV8850_write_cmos_sensor(0x3820, 0x10); 
    OV8850_write_cmos_sensor(0x3821, 0x0e);
	
    OV8850_write_cmos_sensor(0x3a04, 0x09);
    OV8850_write_cmos_sensor(0x3a05, 0xcc);

    OV8850_write_cmos_sensor(0x4001, 0x06);
    OV8850_write_cmos_sensor(0x4004, 0x04); 
    //OV8850_write_cmos_sensor(0x4100, 0x22); 
    //OV8850_write_cmos_sensor(0x4101, 0x23); 
    //OV8850_write_cmos_sensor(0x4102, 0x44); 
    //OV8850_write_cmos_sensor(0x4104, 0x5c); 
    //OV8850_write_cmos_sensor(0x4109, 0x03);

    OV8850_write_cmos_sensor(0x4005, 0x1A);
	OV8850_write_cmos_sensor(0x5013, 0x00);
	OV8850_write_cmos_sensor(0x5045, 0x55);
	OV8850_write_cmos_sensor(0x5048, 0x10);

    OV8850_write_cmos_sensor(0x0100, 0x01); //wake up from software standby
	mdelay(30);

}


static void OV8850_3264_1836_4Lane_30fps_Mclk24M_setting(void)
{
/*
    SCLK=216M, 4lane 648M bps/lane,
    H: 0x0e18  3608
    V:0x07fa  2042
*/
    OV8850_write_cmos_sensor(0x0100, 0x00);// software standby
    OV8850_write_cmos_sensor(0x3011, 0x41);// 4 Lane, MIPI enable
    OV8850_write_cmos_sensor(0x3015, 0x0a);// MIPI mode,
    
    OV8850_write_cmos_sensor(0x3090, 0x02);// 216MHz SCLK 
    OV8850_write_cmos_sensor(0x3091, 0x12); 
    OV8850_write_cmos_sensor(0x3092, 0x00); 
    OV8850_write_cmos_sensor(0x3093, 0x00); 
    OV8850_write_cmos_sensor(0x3094, 0x00); 
    OV8850_write_cmos_sensor(0x3098, 0x03);//240MHz DAC 
    OV8850_write_cmos_sensor(0x3099, 0x1e); 
	
    OV8850_write_cmos_sensor(0x30b3, 0x36);//648Mbps/lane, 3C:780M
    OV8850_write_cmos_sensor(0x30b4, 0x02); 
    OV8850_write_cmos_sensor(0x30b5, 0x04); 
    OV8850_write_cmos_sensor(0x30b6, 0x01); 
	
    OV8850_write_cmos_sensor(0x3500, 0x00); 
    OV8850_write_cmos_sensor(0x3501, 0x7c);      
    OV8850_write_cmos_sensor(0x3502, 0x00); 
	
    OV8850_write_cmos_sensor(0x3624, 0x00);
    OV8850_write_cmos_sensor(0x3680, 0xe0);
    OV8850_write_cmos_sensor(0x3702, 0xF3); 
    OV8850_write_cmos_sensor(0x3704, 0x71);
    OV8850_write_cmos_sensor(0x3708, 0xe3);
    OV8850_write_cmos_sensor(0x3709, 0xc3); 
    OV8850_write_cmos_sensor(0x371F, 0x0C); 
    OV8850_write_cmos_sensor(0x3739, 0x30);
    //OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x20); 
    OV8850_write_cmos_sensor(0x3781, 0x0c);
    OV8850_write_cmos_sensor(0x3786, 0x16);
    OV8850_write_cmos_sensor(0x3796, 0x64);
    OV8850_write_cmos_sensor(0x3800, 0x00); 
    OV8850_write_cmos_sensor(0x3801, 0x04);
    OV8850_write_cmos_sensor(0x3802, 0x01); 
    OV8850_write_cmos_sensor(0x3803, 0x38); 
    OV8850_write_cmos_sensor(0x3804, 0x0c); 
    OV8850_write_cmos_sensor(0x3805, 0xcb); 
    OV8850_write_cmos_sensor(0x3806, 0x08); 
    OV8850_write_cmos_sensor(0x3807, 0x6b); 
    OV8850_write_cmos_sensor(0x3808, 0x0c); 
    OV8850_write_cmos_sensor(0x3809, 0xc0); 
    OV8850_write_cmos_sensor(0x380a, 0x07); 
    OV8850_write_cmos_sensor(0x380b, 0x2c); 
    OV8850_write_cmos_sensor(0x380c, 0x0e);//for 30fps  
    OV8850_write_cmos_sensor(0x380d, 0x18); 
    OV8850_write_cmos_sensor(0x380e, 0x07); 
    OV8850_write_cmos_sensor(0x380f, 0xcc);
	
    OV8850_write_cmos_sensor(0x3814, 0x11); 
    OV8850_write_cmos_sensor(0x3815, 0x11); 
    OV8850_write_cmos_sensor(0x3820, 0x10); 
    OV8850_write_cmos_sensor(0x3821, 0x0e); 
	
    OV8850_write_cmos_sensor(0x3a04, 0x07);
    OV8850_write_cmos_sensor(0x3a05, 0xc8);

    OV8850_write_cmos_sensor(0x4001, 0x02);
    OV8850_write_cmos_sensor(0x4004, 0x08);
    //OV8850_write_cmos_sensor(0x4100, 0x04); 
    //OV8850_write_cmos_sensor(0x4101, 0x04); 
    //OV8850_write_cmos_sensor(0x4102, 0x04); 
    //OV8850_write_cmos_sensor(0x4104, 0x04);
    //OV8850_write_cmos_sensor(0x4109, 0x04);

    OV8850_write_cmos_sensor(0x4005, 0x18);
    OV8850_write_cmos_sensor(0x404f, 0xa0);
	OV8850_write_cmos_sensor(0x5013, 0x04);
	OV8850_write_cmos_sensor(0x5045, 0x55);
	OV8850_write_cmos_sensor(0x5048, 0x10);

    OV8850_write_cmos_sensor(0x0100, 0x01);//wake up from software standby
	mdelay(30);

}

static void OV8850_3264_2448_2Lane_15fps_Mclk24M_setting(void)
{
/*
    Raw 10bit 3264*2448 25fps 2lane 648M bps/lane,SCLK=132M
    H Total: 0x0e18 = 3608
    V Total: 0x09da = 2522

    high framerate need enlarge mipi clk & system clk.
    It should notify OVT friends
*/


    OV8850_write_cmos_sensor(0x0100, 0x00); // software standby
    
    OV8850_write_cmos_sensor(0x3500, 0x00); 
    OV8850_write_cmos_sensor(0x3501, 0x9a); 
    OV8850_write_cmos_sensor(0x3502, 0x60);
	
    OV8850_write_cmos_sensor(0x3011, 0x21); // 2 Lane, MIPI enable
    OV8850_write_cmos_sensor(0x3015, 0xca); // MIPI mode,
    
    OV8850_write_cmos_sensor(0x3090, 0x03); //228MHz SCLK 
    //OV8850_write_cmos_sensor(0x3091, 0x15); 
	OV8850_write_cmos_sensor(0x3091, 0x21);
    OV8850_write_cmos_sensor(0x3092, 0x00); 
    OV8850_write_cmos_sensor(0x3093, 0x02); 
    OV8850_write_cmos_sensor(0x3094, 0x00); 
    OV8850_write_cmos_sensor(0x3098, 0x03); //264MHz DAC 
    OV8850_write_cmos_sensor(0x3099, 0x1e);
	
    OV8850_write_cmos_sensor(0x30b3, 0x36);//648Mbps/lane //3c
    OV8850_write_cmos_sensor(0x30b4, 0x02); 
    OV8850_write_cmos_sensor(0x30b5, 0x04); 
    OV8850_write_cmos_sensor(0x30b6, 0x01);

    //OV8850_write_cmos_sensor(0x4837, 0x0c);//780Mbps/lane 3c  //0a
	
    OV8850_write_cmos_sensor(0x3624, 0x00);
    OV8850_write_cmos_sensor(0x3680, 0xe0);
    OV8850_write_cmos_sensor(0x3702, 0xf3);
    OV8850_write_cmos_sensor(0x3704, 0x71);
    OV8850_write_cmos_sensor(0x3708, 0xe3); 
    OV8850_write_cmos_sensor(0x3709, 0xc3); 
    OV8850_write_cmos_sensor(0x371F, 0x0c);
    OV8850_write_cmos_sensor(0x3739, 0x30);
    //OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x20);
    OV8850_write_cmos_sensor(0x3781, 0x0c);
    OV8850_write_cmos_sensor(0x3786, 0x16);
    OV8850_write_cmos_sensor(0x3796, 0x64);
    OV8850_write_cmos_sensor(0x3800, 0x00); 
    OV8850_write_cmos_sensor(0x3801, 0x04); 
    OV8850_write_cmos_sensor(0x3802, 0x00); 
    OV8850_write_cmos_sensor(0x3803, 0x0c); 
    OV8850_write_cmos_sensor(0x3804, 0x0c); 
    OV8850_write_cmos_sensor(0x3805, 0xcb); 
    OV8850_write_cmos_sensor(0x3806, 0x09); 
    OV8850_write_cmos_sensor(0x3807, 0xa3); 
    OV8850_write_cmos_sensor(0x3808, 0x0c); 
    OV8850_write_cmos_sensor(0x3809, 0xc0); 
    OV8850_write_cmos_sensor(0x380a, 0x09); 
    OV8850_write_cmos_sensor(0x380b, 0x90); 
    OV8850_write_cmos_sensor(0x380c, 0x0e); 
    OV8850_write_cmos_sensor(0x380d, 0x18); 
    OV8850_write_cmos_sensor(0x380e, 0x09); //09 
    OV8850_write_cmos_sensor(0x380f, 0xd0); //da
	
    OV8850_write_cmos_sensor(0x3814, 0x11); 
    OV8850_write_cmos_sensor(0x3815, 0x11); 
    OV8850_write_cmos_sensor(0x3820, 0x10); 
    OV8850_write_cmos_sensor(0x3821, 0x0e);
	
    OV8850_write_cmos_sensor(0x3a04, 0x09);
    OV8850_write_cmos_sensor(0x3a05, 0xb0);

    OV8850_write_cmos_sensor(0x4001, 0x02);
    OV8850_write_cmos_sensor(0x4004, 0x08); 
    //OV8850_write_cmos_sensor(0x4100, 0x22); 
    //OV8850_write_cmos_sensor(0x4101, 0x23); 
    //OV8850_write_cmos_sensor(0x4102, 0x44); 
    //OV8850_write_cmos_sensor(0x4104, 0x5c); 
    //OV8850_write_cmos_sensor(0x4109, 0x03);

    OV8850_write_cmos_sensor(0x4005, 0x1a);
	OV8850_write_cmos_sensor(0x404f, 0xa0);
	OV8850_write_cmos_sensor(0x4837, 0x0c); 
;
	OV8850_write_cmos_sensor(0x5013, 0x00);
	OV8850_write_cmos_sensor(0x5045, 0x55);
	OV8850_write_cmos_sensor(0x5048, 0x10);

    OV8850_write_cmos_sensor(0x0100, 0x01); //wake up from software standby
	mdelay(30);

}

#if 0  // Do not use this 1080P setting. we don't verify it.
static void OV8850_1920_1080_2Lane_31fps_Mclk24M_setting(void)
{
/*
	
	//;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;	  
*/
	SENSORDB("OV8850_1920_1080_2lane_31fps_Mclk24M_setting start \n");

    OV8850_write_cmos_sensor(0x0100, 0x00);

	//Mipi lane setting, PLL setting
    OV8850_write_cmos_sensor(0x3011, 0x21);// 2 lane MIPI
    OV8850_write_cmos_sensor(0x3015, 0xca);// 2 lane enable
    
	OV8850_write_cmos_sensor(0x3090, 0x03);// PLL2 prediv 
    OV8850_write_cmos_sensor(0x3091, 0x22);// PLL2 multiplier
    
    OV8850_write_cmos_sensor(0x3092, 0x00);
    OV8850_write_cmos_sensor(0x3093, 0x02);
    OV8850_write_cmos_sensor(0x3094, 0x00);
    OV8850_write_cmos_sensor(0x3098, 0x03);//240MHz DAC
    OV8850_write_cmos_sensor(0x3099, 0x1e);//1c
    
    OV8850_write_cmos_sensor(0x30b3, 0x36);//648Mbps/lane, 3C:780M
    OV8850_write_cmos_sensor(0x30b4, 0x02);
    OV8850_write_cmos_sensor(0x30b5, 0x04);
    OV8850_write_cmos_sensor(0x30b6, 0x01);
	
    OV8850_write_cmos_sensor(0x3500, 0x00);
    OV8850_write_cmos_sensor(0x3501, 0x7c);
    OV8850_write_cmos_sensor(0x3502, 0x00);
	
    OV8850_write_cmos_sensor(0x3624, 0x00);
    OV8850_write_cmos_sensor(0x3680, 0xe0);
    OV8850_write_cmos_sensor(0x3702, 0xf3);//db
    OV8850_write_cmos_sensor(0x3704, 0x71);
    OV8850_write_cmos_sensor(0x3708, 0xe3);
    OV8850_write_cmos_sensor(0x3709, 0xc3);
    OV8850_write_cmos_sensor(0x371F, 0x0c);//0x18
    OV8850_write_cmos_sensor(0x3739, 0x30);
    //OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x20);//38
    OV8850_write_cmos_sensor(0x3781, 0x0c);
    OV8850_write_cmos_sensor(0x3786, 0x16);
	OV8850_write_cmos_sensor(0x3796, 0x64);//78
    OV8850_write_cmos_sensor(0x3800, 0x00);
    OV8850_write_cmos_sensor(0x3801, 0x0c);
    OV8850_write_cmos_sensor(0x3802, 0x01);
    //OV8850_write_cmos_sensor(0x3803, 0x3E);
	OV8850_write_cmos_sensor(0x3803, 0x40);
    OV8850_write_cmos_sensor(0x3804, 0x0c);
    OV8850_write_cmos_sensor(0x3805, 0xd3);
    OV8850_write_cmos_sensor(0x3806, 0x08);
    OV8850_write_cmos_sensor(0x3807, 0x73);
	//OV8850_write_cmos_sensor(0x3807, 0x71);
    OV8850_write_cmos_sensor(0x3808, 0x07);
    OV8850_write_cmos_sensor(0x3809, 0x80);
    OV8850_write_cmos_sensor(0x380a, 0x04);
    OV8850_write_cmos_sensor(0x380b, 0x38);

	//HTS,VTS setting
    OV8850_write_cmos_sensor(0x380c, 0x0E);//for 30fps
    OV8850_write_cmos_sensor(0x380d, 0x18);
    OV8850_write_cmos_sensor(0x380e, 0x07);
    OV8850_write_cmos_sensor(0x380f, 0x80);//fa

    OV8850_write_cmos_sensor(0x3814, 0x11);
    OV8850_write_cmos_sensor(0x3815, 0x11);
    OV8850_write_cmos_sensor(0x3820, 0x10);
    OV8850_write_cmos_sensor(0x3821, 0x0e);
	
    OV8850_write_cmos_sensor(0x3a04, 0x07);
    OV8850_write_cmos_sensor(0x3a05, 0xc8);
	
    OV8850_write_cmos_sensor(0x4001, 0x02);
    OV8850_write_cmos_sensor(0x4004, 0x08);
    //OV8850_write_cmos_sensor(0x4100, 0x04);
    //OV8850_write_cmos_sensor(0x4101, 0x04);
    //OV8850_write_cmos_sensor(0x4102, 0x04);
    //OV8850_write_cmos_sensor(0x4104, 0x04);
    //OV8850_write_cmos_sensor(0x4109, 0x04);

    OV8850_write_cmos_sensor(0x4005, 0x18); //BLC trigger when gain changed
    OV8850_write_cmos_sensor(0x404f, 0xa0);
	OV8850_write_cmos_sensor(0x4837, 0x0c);
	OV8850_write_cmos_sensor(0x5013, 0x04);
	OV8850_write_cmos_sensor(0x5045, 0x55);
	OV8850_write_cmos_sensor(0x5048, 0x10);
    OV8850_write_cmos_sensor(0x0100, 0x01);
	
	mdelay(30);

	SENSORDB("OV8850_1920_1080_2lane_31fps_Mclk24M_setting end \n");
}
static void OV8850_1920_1080_4Lane_30fps_Mclk26M_setting(void)
{
/*
    Raw 10bit 1920*1080 30fps 4lane 516M bps/lane,SCLK=204M
*/
    OV8850_write_cmos_sensor(0x0100, 0x00);// software standby
    OV8850_write_cmos_sensor(0x3011, 0x41);// 4 Lane, MIPI enable
    OV8850_write_cmos_sensor(0x3015, 0x0a);// MIPI mode,
    OV8850_write_cmos_sensor(0x3090, 0x02);// 204MHz SCLK 
    OV8850_write_cmos_sensor(0x3091, 0x11); 
    OV8850_write_cmos_sensor(0x3092, 0x00); 
    OV8850_write_cmos_sensor(0x3093, 0x00); 
    OV8850_write_cmos_sensor(0x3094, 0x00); 
    OV8850_write_cmos_sensor(0x3098, 0x03);//240MHz DAC 
    OV8850_write_cmos_sensor(0x3099, 0x1e); 
    OV8850_write_cmos_sensor(0x30b3, 0x2b);//516Mbps/lane
    OV8850_write_cmos_sensor(0x30b4, 0x02); 
    OV8850_write_cmos_sensor(0x30b5, 0x04); 
    OV8850_write_cmos_sensor(0x30b6, 0x01); 
    OV8850_write_cmos_sensor(0x3500, 0x00); 
    OV8850_write_cmos_sensor(0x3501, 0x75);      
    OV8850_write_cmos_sensor(0x3502, 0x80); 
    OV8850_write_cmos_sensor(0x3624, 0x04);
    OV8850_write_cmos_sensor(0x3680, 0xe0);
    OV8850_write_cmos_sensor(0x3702, 0xF3); 
    OV8850_write_cmos_sensor(0x3704, 0x71);
    OV8850_write_cmos_sensor(0x3708, 0xe3);
    OV8850_write_cmos_sensor(0x3709, 0xc3); 
    OV8850_write_cmos_sensor(0x371F, 0x0C); 
    OV8850_write_cmos_sensor(0x3739, 0x30);
    OV8850_write_cmos_sensor(0x373a, 0x51);
    OV8850_write_cmos_sensor(0x373C, 0x20); 
    OV8850_write_cmos_sensor(0x3781, 0x0c);
    OV8850_write_cmos_sensor(0x3786, 0x16);
    OV8850_write_cmos_sensor(0x3796, 0x64);
    OV8850_write_cmos_sensor(0x3800, 0x00); 
    OV8850_write_cmos_sensor(0x3801, 0x0c);
    OV8850_write_cmos_sensor(0x3802, 0x01); 
    OV8850_write_cmos_sensor(0x3803, 0x40); 
    OV8850_write_cmos_sensor(0x3804, 0x0c); 
    OV8850_write_cmos_sensor(0x3805, 0xd3); 
    OV8850_write_cmos_sensor(0x3806, 0x08); 
    OV8850_write_cmos_sensor(0x3807, 0x73); 
    OV8850_write_cmos_sensor(0x3808, 0x07); 
    OV8850_write_cmos_sensor(0x3809, 0x80); 
    OV8850_write_cmos_sensor(0x380a, 0x04); 
    OV8850_write_cmos_sensor(0x380b, 0x38); 
    OV8850_write_cmos_sensor(0x380c, 0x0e);//for 30fps  
    OV8850_write_cmos_sensor(0x380d, 0x18); 
    OV8850_write_cmos_sensor(0x380e, 0x07); 
    OV8850_write_cmos_sensor(0x380f, 0x5c); 
    OV8850_write_cmos_sensor(0x3814, 0x11); 
    OV8850_write_cmos_sensor(0x3815, 0x11); 
    OV8850_write_cmos_sensor(0x3820, 0x10); 
    OV8850_write_cmos_sensor(0x3821, 0x0e); 
    OV8850_write_cmos_sensor(0x4001, 0x02);
    OV8850_write_cmos_sensor(0x4004, 0x04);
    OV8850_write_cmos_sensor(0x4100, 0x04); 
    OV8850_write_cmos_sensor(0x4101, 0x04); 
    OV8850_write_cmos_sensor(0x4102, 0x04); 
    OV8850_write_cmos_sensor(0x4104, 0x04);
    OV8850_write_cmos_sensor(0x4109, 0x04);

    OV8850_write_cmos_sensor(0x4005, 0x1a);
    OV8850_write_cmos_sensor(0x404f, 0xa0);

    OV8850_write_cmos_sensor(0x0100, 0x01);//wake up from software standby

}
#endif

UINT32 OV8850Open(void)
{
    kal_uint16 sensor_id=0; 
    int i;
    const kal_uint16 sccb_writeid[] = {OV8850_SLAVE_WRITE_ID_1,OV8850_SLAVE_WRITE_ID_2};
	SENSORDB("OV8850Open\n");

   spin_lock(&OV8850_drv_lock);
   OV8850_sensor.is_zsd = KAL_FALSE;  //for zsd full size preview
   OV8850_sensor.is_zsd_cap = KAL_FALSE;
   OV8850_sensor.is_autofliker = KAL_FALSE; //for autofliker.
   OV8850_sensor.pv_mode = KAL_TRUE;
   OV8850_sensor.pclk = OV8850_PREVIEW_CLK;
   spin_unlock(&OV8850_drv_lock);
   
  for(i = 0; i <(sizeof(sccb_writeid)/sizeof(sccb_writeid[0])); i++)
    {
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.write_id = sccb_writeid[i];
        OV8850_sensor.read_id = (sccb_writeid[i]|0x01);
        spin_unlock(&OV8850_drv_lock);

        sensor_id=((OV8850_read_cmos_sensor(0x300A) << 8) | OV8850_read_cmos_sensor(0x300B));

#ifdef OV8850_DRIVER_TRACE
        SENSORDB("OV8850Open, sensor_id:%x \n",sensor_id);
#endif
        if(OV8850_SENSOR_ID == sensor_id)
        {
            SENSORDB("OV8850 slave write id:%x \n",OV8850_sensor.write_id);
            break;
        }
    }
  
    // check if sensor ID correct
    if (sensor_id != OV8850_SENSOR_ID) 
    {
        SENSORDB("OV8850 Check ID fails! \n");
		
		SENSORDB("[Warning]OV8850GetSensorID, sensor_id:%x \n",sensor_id);
		//sensor_id = OV8850_SENSOR_ID;
        sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
    }

	OV8850_global_setting();

//#if defined(OV8850_USE_OTP)
//    OV8850_Update_Otp();
//#endif

    SENSORDB("test for bootimage \n");

   return ERROR_NONE;
}   /* OV8850Open  */

/*************************************************************************
* FUNCTION
*   OV5642GetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8850GetSensorID(UINT32 *sensorID) 
{
  //added by mandrave
   int i;
   const kal_uint16 sccb_writeid[] = {OV8850_SLAVE_WRITE_ID_1, OV8850_SLAVE_WRITE_ID_2};
 
 mt_set_gpio_mode(GPIO_CAMERA_CMPDN_PIN, GPIO_CAMERA_CMPDN_PIN_M_GPIO);
 mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN, GPIO_OUT_ONE);
 mDELAY(10);

 SENSORDB("OV8850GetSensorID enter,\n");
    for(i = 0; i <(sizeof(sccb_writeid)/sizeof(sccb_writeid[0])); i++)
    {
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.write_id = sccb_writeid[i];
        OV8850_sensor.read_id = (sccb_writeid[i]|0x01);
        spin_unlock(&OV8850_drv_lock);

        *sensorID=((OV8850_read_cmos_sensor(0x300A) << 8) | OV8850_read_cmos_sensor(0x300B));	

#ifdef OV8850_DRIVER_TRACE
        SENSORDB("OV8850GetSensorID, sensor_id:%x \n",*sensorID);
#endif
        if(OV8850_SENSOR_ID == *sensorID)
        {
            SENSORDB("OV8850 slave write id:%x \n",OV8850_sensor.write_id);
            break;
        }
    }

    // check if sensor ID correct		
    if (*sensorID != OV8850_SENSOR_ID) 
    {
        	SENSORDB("[Warning]OV8850GetSensorID, sensor_id:%x \n",*sensorID);
			*sensorID = 0xFFFFFFFF;
			return ERROR_SENSOR_CONNECT_FAIL;
    }
	SENSORDB("OV8850GetSensorID exit,\n");
   return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	OV8850Close
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
UINT32 OV8850Close(void)
{
#ifdef OV8850_DRIVER_TRACE
   SENSORDB("OV8850Close\n");
#endif

    return ERROR_NONE;
}   /* OV8850Close */

/*************************************************************************
* FUNCTION
* OV8850Preview
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
UINT32 OV8850Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint16 dummy_line;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850Preview \n");
#endif
	OV8850_1632_1224_30fps_Mclk24M_setting();


    //msleep(10);
    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.pv_mode = KAL_TRUE;
    spin_unlock(&OV8850_drv_lock);

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.video_mode = KAL_FALSE;
    spin_unlock(&OV8850_drv_lock);
    dummy_line = 0;

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.dummy_pixels = 0;
    OV8850_sensor.dummy_lines = 0;
    OV8850_sensor.line_length = OV8850_PV_PERIOD_PIXEL_NUMS;
    OV8850_sensor.frame_height = OV8850_PV_PERIOD_LINE_NUMS + dummy_line;
    OV8850_sensor.pclk = OV8850_PREVIEW_CLK;
    spin_unlock(&OV8850_drv_lock);

    OV8850_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
    //OV8850_Write_Shutter(OV8850_sensor.shutter);

    mdelay(10);

    return ERROR_NONE;

}   /*  OV8850Preview   */


/*************************************************************************
* FUNCTION
* OV8850VIDEO
*
* DESCRIPTION
*	This function start the sensor Video preview.
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

UINT32 OV8850VIDEO(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 dummy_line;
    kal_uint16 ret;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850VIDEO \n");
#endif
	#ifdef OV8850_2_LANE
	OV8850_1632_1224_30fps_Mclk24M_setting();
	#else
	OV8850_3264_1836_4Lane_30fps_Mclk24M_setting();
	#endif
    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.pv_mode = KAL_FALSE;
    spin_unlock(&OV8850_drv_lock);

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.video_mode = KAL_TRUE;
    spin_unlock(&OV8850_drv_lock);
    dummy_line = 0;

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.dummy_pixels = 0;
    OV8850_sensor.dummy_lines = 0;
    OV8850_sensor.line_length = OV8850_VIDEO_PERIOD_PIXEL_NUMS;
    OV8850_sensor.frame_height = OV8850_VIDEO_PERIOD_LINE_NUMS+ dummy_line;
    OV8850_sensor.pclk = OV8850_VIDEO_CLK;
    spin_unlock(&OV8850_drv_lock);

    OV8850_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
    mdelay(10);

    return ERROR_NONE;
	
}   /*  OV8850VIDEO   */


/*************************************************************************
* FUNCTION
*    OV8850ZsdPreview
*
* DESCRIPTION
*    This function setup the CMOS sensor in Full Size output  mode
*
* PARAMETERS
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8850ZsdPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
    MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint16 dummy_pixel = 0;
    kal_uint16 dummy_line = 0;
    kal_uint16 ret;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850ZsdPreview \n");
#endif

	#ifdef OV8850_2_LANE
    OV8850_3264_2448_2Lane_15fps_Mclk24M_setting();
	#else
    OV8850_3264_2448_4Lane_25fps_Mclk24M_setting();
	#endif
    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.pv_mode = KAL_FALSE;
    spin_unlock(&OV8850_drv_lock);


    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.video_mode = KAL_FALSE;
    spin_unlock(&OV8850_drv_lock);
    dummy_line = 0;

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.dummy_pixels = 0;
    OV8850_sensor.dummy_lines = 0;
    OV8850_sensor.line_length = OV8850_FULL_PERIOD_PIXEL_NUMS;
    OV8850_sensor.frame_height = OV8850_FULL_PERIOD_LINE_NUMS+ dummy_line;
    OV8850_sensor.pclk = OV8850_ZSD_PRE_CLK;
    spin_unlock(&OV8850_drv_lock);

    OV8850_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
    mdelay(10);

    return ERROR_NONE;
}



/*************************************************************************
* FUNCTION
*OV8850Capture
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
UINT32 OV8850Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 dummy_pixel = 0;
    kal_uint16 dummy_line = 0;
    kal_uint16 ret;
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850Capture start \n");
#endif


    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.video_mode = KAL_FALSE;
    OV8850_sensor.is_autofliker = KAL_FALSE;
    OV8850_sensor.pv_mode = KAL_FALSE;
    spin_unlock(&OV8850_drv_lock);

	#ifdef OV8850_2_LANE
    OV8850_3264_2448_2Lane_15fps_Mclk24M_setting();
	#else
    OV8850_3264_2448_4Lane_25fps_Mclk24M_setting();
	#endif

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.dummy_pixels = 0;
    OV8850_sensor.dummy_lines = 0;
    spin_unlock(&OV8850_drv_lock);

    dummy_pixel = 0;
    dummy_line = 0;

    spin_lock(&OV8850_drv_lock);
    OV8850_sensor.pclk = OV8850_CAPTURE_CLK;
    OV8850_sensor.line_length = OV8850_FULL_PERIOD_PIXEL_NUMS + dummy_pixel;
    OV8850_sensor.frame_height = OV8850_FULL_PERIOD_LINE_NUMS + dummy_line;
    spin_unlock(&OV8850_drv_lock);

    OV8850_Set_Dummy(dummy_pixel, dummy_line);


#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850Capture end\n");
#endif
    mdelay(10);

    return ERROR_NONE;
}   /* OV8850_Capture() */


UINT32 OV88503DPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint16 dummy_line;
	kal_uint16 ret;
#ifdef OV8850_DRIVER_TRACE
	SENSORDB("OV88503DPreview \n");
#endif
	//OV8850_Sensor_1M();
	OV8850_1632_1224_30fps_Mclk24M_setting();
    //msleep(30);
    spin_lock(&OV8850_drv_lock);
	OV8850_sensor.pv_mode = KAL_TRUE;
	spin_unlock(&OV8850_drv_lock);
	
	//OV8850_set_mirror(sensor_config_data->SensorImageMirror);
	switch (sensor_config_data->SensorOperationMode)
	{
	  case MSDK_SENSOR_OPERATION_MODE_VIDEO: 
	  	spin_lock(&OV8850_drv_lock);
		OV8850_sensor.video_mode = KAL_TRUE;		
		spin_unlock(&OV8850_drv_lock);
		dummy_line = 0;
#ifdef OV8850_DRIVER_TRACE
		SENSORDB("Video mode \n");
#endif
	   break;
	  default: /* ISP_PREVIEW_MODE */
	  	spin_lock(&OV8850_drv_lock);
		OV8850_sensor.video_mode = KAL_FALSE;
		spin_unlock(&OV8850_drv_lock);
		dummy_line = 0;
#ifdef OV8850_DRIVER_TRACE
		SENSORDB("Camera preview mode \n");
#endif
	  break;
	}

	spin_lock(&OV8850_drv_lock);
	OV8850_sensor.dummy_pixels = 0;
	OV8850_sensor.dummy_lines = 0;
	OV8850_sensor.line_length = OV8850_PV_PERIOD_PIXEL_NUMS;
	OV8850_sensor.frame_height = OV8850_PV_PERIOD_LINE_NUMS + dummy_line;
	OV8850_sensor.pclk = OV8850_PREVIEW_CLK;
	spin_unlock(&OV8850_drv_lock);
	
	OV8850_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
	//OV8850_Write_Shutter(OV8850_sensor.shutter);
		
	return ERROR_NONE;
	
}


UINT32 OV8850GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850GetResolution \n");
#endif
    pSensorResolution->SensorFullWidth=OV8850_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=OV8850_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=OV8850_IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorVideoWidth=OV8850_IMAGE_SENSOR_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight=OV8850_IMAGE_SENSOR_VIDEO_HEIGHT;
	pSensorResolution->Sensor3DFullWidth=OV8850_IMAGE_SENSOR_3D_FULL_WIDTH;
	pSensorResolution->Sensor3DFullHeight=OV8850_IMAGE_SENSOR_3D_FULL_HEIGHT;
	pSensorResolution->Sensor3DPreviewWidth=OV8850_IMAGE_SENSOR_3D_PV_WIDTH;
	pSensorResolution->Sensor3DPreviewHeight=OV8850_IMAGE_SENSOR_3D_PV_HEIGHT;	
	pSensorResolution->Sensor3DVideoWidth=OV8850_IMAGE_SENSOR_3D_VIDEO_WIDTH;
	pSensorResolution->Sensor3DVideoHeight=OV8850_IMAGE_SENSOR_3D_VIDEO_HEIGHT;
    return ERROR_NONE;
}/* OV8850GetResolution() */

UINT32 OV8850GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850GetInfo£¬FeatureId:%d\n",ScenarioId);
#endif

    switch(ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorPreviewResolutionX=OV8850_IMAGE_SENSOR_FULL_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
            pSensorInfo->SensorFullResolutionX=OV8850_IMAGE_SENSOR_FULL_WIDTH;
            pSensorInfo->SensorFullResolutionY=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate = 15;
            break;
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            pSensorInfo->SensorPreviewResolutionX=OV8850_IMAGE_SENSOR_PV_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=OV8850_IMAGE_SENSOR_PV_HEIGHT;
            pSensorInfo->SensorFullResolutionX=OV8850_IMAGE_SENSOR_FULL_WIDTH;
            pSensorInfo->SensorFullResolutionY=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate=30;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pSensorInfo->SensorPreviewResolutionX=OV8850_IMAGE_SENSOR_VIDEO_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=OV8850_IMAGE_SENSOR_VIDEO_HEIGHT;
            pSensorInfo->SensorFullResolutionX=OV8850_IMAGE_SENSOR_FULL_WIDTH;
            pSensorInfo->SensorFullResolutionY=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate=30;
            break;
	  case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
	  case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
	  case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added	 
		   pSensorInfo->SensorPreviewResolutionX=OV8850_IMAGE_SENSOR_3D_VIDEO_WIDTH;
		   pSensorInfo->SensorPreviewResolutionY=OV8850_IMAGE_SENSOR_3D_VIDEO_HEIGHT;
		   pSensorInfo->SensorFullResolutionX=OV8850_IMAGE_SENSOR_3D_FULL_WIDTH;
		   pSensorInfo->SensorFullResolutionY=OV8850_IMAGE_SENSOR_3D_FULL_HEIGHT;			   
		   pSensorInfo->SensorCameraPreviewFrameRate=30;		  
		  break;
        default:
            pSensorInfo->SensorPreviewResolutionX=OV8850_IMAGE_SENSOR_PV_WIDTH;
            pSensorInfo->SensorPreviewResolutionY=OV8850_IMAGE_SENSOR_PV_HEIGHT;
            pSensorInfo->SensorFullResolutionX=OV8850_IMAGE_SENSOR_FULL_WIDTH;
            pSensorInfo->SensorFullResolutionY=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
            pSensorInfo->SensorCameraPreviewFrameRate = 30;
            break;
    }

    //pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE; //low active
    pSensorInfo->SensorResetDelayCount=5; 

    pSensorInfo->SensorOutputDataFormat=OV8850_COLOR_FORMAT;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 4;
    pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_MIPI;
    pSensorInfo->CaptureDelayFrame = 1; 
    pSensorInfo->PreviewDelayFrame = 3; 
    pSensorInfo->VideoDelayFrame = 1;

    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_4MA;
    pSensorInfo->AEShutDelayFrame = 0;   /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0;/* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;    
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=3;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = OV8850_PV_X_START; 
            pSensorInfo->SensorGrabStartY = OV8850_PV_Y_START; 

			#ifdef OV8850_2_LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
			#else
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	3;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = OV8850_PV_X_START; 
            pSensorInfo->SensorGrabStartY = OV8850_PV_Y_START; 

			#ifdef OV8850_2_LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
			#else
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;

            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 3;
            pSensorInfo->SensorClockRisingCount=0;
            pSensorInfo->SensorClockFallingCount=2;
            pSensorInfo->SensorPixelClockCount=3;
            pSensorInfo->SensorDataLatchCount=2;
            pSensorInfo->SensorGrabStartX = OV8850_FULL_X_START; 
            pSensorInfo->SensorGrabStartY = OV8850_FULL_Y_START; 
			#ifdef OV8850_2_LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
			#else
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;	//Hesong Modify 10/25  SENSOR_MIPI_2_LANE	
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
		break;

        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX=  OV8850_3D_PV_X_START;
			pSensorInfo->SensorGrabStartY = OV8850_3D_PV_Y_START; 
			#ifdef OV8850_2_LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
			#else
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;	//Hesong Modify 10/25  SENSOR_MIPI_2_LANE	
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;

            break;
        default:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=3;
            pSensorInfo->SensorClockRisingCount=0;
            pSensorInfo->SensorClockFallingCount=2;
            pSensorInfo->SensorPixelClockCount=3;
            pSensorInfo->SensorDataLatchCount=2;
            pSensorInfo->SensorGrabStartX = OV8850_PV_X_START; 
            pSensorInfo->SensorGrabStartY = OV8850_PV_Y_START; 

			#ifdef OV8850_2_LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
			#else
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
    }
  return ERROR_NONE;
}	/* OV8850GetInfo() */


UINT32 OV8850Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850Control£¬ScenarioId:%d\n",ScenarioId);
#endif	

    spin_lock(&OV8850_drv_lock);
    CurrentScenarioId = ScenarioId;
    spin_unlock(&OV8850_drv_lock);

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            OV8850Preview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            OV8850VIDEO(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            OV8850Capture(pImageWindow, pSensorConfigData);
        break;
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            OV8850ZsdPreview(pImageWindow, pSensorConfigData);
		break;
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
        	OV88503DPreview(pImageWindow, pSensorConfigData);
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
}	/* OV8850Control() */

UINT32 OV8850SetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{

    //kal_uint32 pv_max_frame_rate_lines = OV8850_sensor.dummy_lines;

	SENSORDB("[OV8850SetAutoFlickerMode] bEnable = d%, frame rate(10base) = %d\n", bEnable, u2FrameRate);

    if(bEnable)
    {
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.is_autofliker = KAL_TRUE;
        spin_unlock(&OV8850_drv_lock);
    }
    else
    {
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.is_autofliker = KAL_FALSE;
        spin_unlock(&OV8850_drv_lock);
    }
    SENSORDB("[OV8850SetAutoFlickerMode]bEnable:%x \n",bEnable);
	return ERROR_NONE;
}


UINT32 OV8850SetCalData(PSET_SENSOR_CALIBRATION_DATA_STRUCT pSetSensorCalData)
{
    UINT32 i;
    SENSORDB("OV8850 Sensor write calibration data num = %d \r\n", pSetSensorCalData->DataSize);
    SENSORDB("OV8850 Sensor write calibration data format = %x \r\n", pSetSensorCalData->DataFormat);
    if(pSetSensorCalData->DataSize <= MAX_SHADING_DATA_TBL){
        for (i = 0; i < pSetSensorCalData->DataSize; i++){
            if (((pSetSensorCalData->DataFormat & 0xFFFF) == 1) && ((pSetSensorCalData->DataFormat >> 16) == 1)){
                SENSORDB("OV8850 Sensor write calibration data: address = %x, value = %x \r\n",(pSetSensorCalData->ShadingData[i])>>16,(pSetSensorCalData->ShadingData[i])&0xFFFF);
                OV8850_write_cmos_sensor((pSetSensorCalData->ShadingData[i])>>16, (pSetSensorCalData->ShadingData[i])&0xFFFF);
            }
        }
    }
    return ERROR_NONE;
}

UINT32 OV8850SetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) 
{
    kal_uint32 pclk;
    kal_int16 dummyLine;
    kal_uint16 lineLength,frameHeight;

    SENSORDB("OV8850SetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
    switch (scenarioId) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pclk = OV8850_PREVIEW_CLK;
            lineLength = OV8850_PV_PERIOD_PIXEL_NUMS;
            frameHeight = (10 * pclk)/frameRate/lineLength;
            dummyLine = frameHeight - OV8850_PV_PERIOD_LINE_NUMS;
            if (dummyLine < 0){
            dummyLine = 0;
            }
            OV8850_Set_Dummy(0, dummyLine);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pclk = OV8850_VIDEO_CLK;
            lineLength = OV8850_VIDEO_PERIOD_PIXEL_NUMS;
            frameHeight = (10 * pclk)/frameRate/lineLength;
            dummyLine = frameHeight - OV8850_VIDEO_PERIOD_LINE_NUMS;
            if (dummyLine < 0){
            dummyLine = 0;
            }
            OV8850_Set_Dummy(0, dummyLine);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pclk = OV8850_CAPTURE_CLK;
            lineLength = OV8850_FULL_PERIOD_PIXEL_NUMS;
            frameHeight = (10 * pclk)/frameRate/lineLength;
            if(frameHeight < OV8850_FULL_PERIOD_LINE_NUMS)
            frameHeight = OV8850_FULL_PERIOD_LINE_NUMS;
            dummyLine = frameHeight - OV8850_FULL_PERIOD_LINE_NUMS;
            SENSORDB("OV8850SetMaxFramerateByScenario: scenarioId = %d, frame rate calculate = %d\n",((10 * pclk)/frameHeight/lineLength));
            OV8850_Set_Dummy(0, dummyLine);
            break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
			break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			break;		
        default:
            break;
    }	
        return ERROR_NONE;
}


UINT32 OV8850GetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{
    switch (scenarioId) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        *pframeRate = 300;
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
		#ifdef OV8850_2_LANE
        *pframeRate = 145;
		#else
		*pframeRate = 250;
		#endif
        break;
    case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
    case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
        *pframeRate = 300;
        break;
    default:
        break;
    }

    return ERROR_NONE;
}

UINT32 OV8850SetVideoMode(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
    /* to fix VSYNC, to fix frame rate */
#ifdef OV8850_DRIVER_TRACE
    SENSORDB("OV8850SetVideoMode£¬u2FrameRate:%d\n",u2FrameRate);
#endif	

    if((30 == u2FrameRate)||(15 == u2FrameRate)||(24 == u2FrameRate))
    {
        dummy_line = OV8850_sensor.pclk / u2FrameRate / OV8850_sensor.line_length - OV8850_sensor.frame_height;
        if (dummy_line < 0) 
            dummy_line = 0;
#ifdef OV8850_DRIVER_TRACE
        SENSORDB("dummy_line %d\n", dummy_line);
#endif
        OV8850_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.video_mode = KAL_TRUE;
        spin_unlock(&OV8850_drv_lock);
    }
    else if(0 == u2FrameRate)
    {
        spin_lock(&OV8850_drv_lock);
        OV8850_sensor.video_mode = KAL_FALSE;
        spin_unlock(&OV8850_drv_lock);

        SENSORDB("disable video mode\n");
    }
    else{
        SENSORDB("[OV8850SetVideoMode],Error Framerate, u2FrameRate=%d",u2FrameRate);
    }
    return ERROR_NONE;
}


UINT32 OV8850FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 OV8850SensorRegNumber;
    UINT32 i;
    //PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    //MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    //MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    //MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    //MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;
    PSET_SENSOR_CALIBRATION_DATA_STRUCT pSetSensorCalData=(PSET_SENSOR_CALIBRATION_DATA_STRUCT)pFeaturePara;

#ifdef OV8850_DRIVER_TRACE
    //SENSORDB("OV8850FeatureControl£¬FeatureId:%d\n",FeatureId); 
#endif		
    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=OV8850_IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16=OV8850_IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:/* 3 */
            *pFeatureReturnPara16++= OV8850_sensor.line_length;
            *pFeatureReturnPara16= OV8850_sensor.frame_height;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:  /* 3 */
            *pFeatureReturnPara32 = OV8850_sensor.pclk;
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:	/* 4 */
            set_OV8850_shutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            //OV8850_night_mode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:	/* 6 */
            OV8850_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            OV8850_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = OV8850_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            //memcpy(&OV8850_sensor.eng.cct, pFeaturePara, sizeof(OV8850_sensor.eng.cct));
            OV8850SensorRegNumber = OV8850_FACTORY_END_ADDR;
            for (i=0;i<OV8850SensorRegNumber;i++)
            {
                spin_lock(&OV8850_drv_lock);
                OV8850_sensor.eng.cct[i].Addr=*pFeatureData32++;
                OV8850_sensor.eng.cct[i].Para=*pFeatureData32++;
                spin_unlock(&OV8850_drv_lock);
            }

            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:/* 12 */
            if (*pFeatureParaLen >= sizeof(OV8850_sensor.eng.cct) + sizeof(kal_uint32))
            {
                *((kal_uint32 *)pFeaturePara++) = sizeof(OV8850_sensor.eng.cct);
                memcpy(pFeaturePara, &OV8850_sensor.eng.cct, sizeof(OV8850_sensor.eng.cct));
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            //memcpy(&OV8850_sensor.eng.reg, pFeaturePara, sizeof(OV8850_sensor.eng.reg));
            OV8850SensorRegNumber = OV8850_ENGINEER_END;
            for (i=0;i<OV8850SensorRegNumber;i++)
            {
                spin_lock(&OV8850_drv_lock);
                OV8850_sensor.eng.reg[i].Addr=*pFeatureData32++;
                OV8850_sensor.eng.reg[i].Para=*pFeatureData32++;
                spin_unlock(&OV8850_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:	/* 14 */
            if (*pFeatureParaLen >= sizeof(OV8850_sensor.eng.reg) + sizeof(kal_uint32))
            {
                *((kal_uint32 *)pFeaturePara++) = sizeof(OV8850_sensor.eng.reg);
                memcpy(pFeaturePara, &OV8850_sensor.eng.reg, sizeof(OV8850_sensor.eng.reg));
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            ((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->Version = NVRAM_CAMERA_SENSOR_FILE_VERSION;
            ((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorId = OV8850_SENSOR_ID;
            memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorEngReg, &OV8850_sensor.eng.reg, sizeof(OV8850_sensor.eng.reg));
            memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorCCTReg, &OV8850_sensor.eng.cct, sizeof(OV8850_sensor.eng.cct));
            *pFeatureParaLen = sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pFeaturePara, &OV8850_sensor.cfg_data, sizeof(OV8850_sensor.cfg_data));
            *pFeatureParaLen = sizeof(OV8850_sensor.cfg_data);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            OV8850_camera_para_to_sensor();
            break;
        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            OV8850_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            OV8850_get_sensor_group_count((kal_uint32 *)pFeaturePara);
            *pFeatureParaLen = 4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            OV8850_get_sensor_group_info((MSDK_SENSOR_GROUP_INFO_STRUCT *)pFeaturePara);
            *pFeatureParaLen = sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            OV8850_get_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
            *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_SET_ITEM_INFO:
            OV8850_set_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
            *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ENG_INFO:
            memcpy(pFeaturePara, &OV8850_sensor.eng_info, sizeof(OV8850_sensor.eng_info));
            *pFeatureParaLen = sizeof(OV8850_sensor.eng_info);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            OV8850SetVideoMode(*pFeatureData16);
            break; 
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV8850GetSensorID(pFeatureReturnPara32); 
            break; 
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            OV8850SetAutoFlickerMode((BOOL)*pFeatureData16,*(pFeatureData16+1));
            break;
        case SENSOR_FEATURE_SET_CALIBRATION_DATA:
            OV8850SetCalData(pSetSensorCalData);
            break;
        case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
            OV8850SetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
            break;
        case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            OV8850GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
            break;
        default:
            break;
    }
    return ERROR_NONE;
}/* OV8850FeatureControl() */
SENSOR_FUNCTION_STRUCT SensorFuncOV8850=
{
    OV8850Open,
    OV8850GetInfo,
    OV8850GetResolution,
    OV8850FeatureControl,
    OV8850Control,
    OV8850Close
};

UINT32 OV8850_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
//UINT32 OV8850SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
/* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncOV8850;

    return ERROR_NONE;
}/* SensorInit() */



