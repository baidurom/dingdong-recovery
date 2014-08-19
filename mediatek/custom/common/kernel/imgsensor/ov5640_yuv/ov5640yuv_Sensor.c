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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
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
 *   sensor.c
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   Jackie Su (MTK02380)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 03 15 2011 koli.lin
 * [ALPS00034474] [Need Patch] [Volunteer Patch]
 * Move sensor driver current setting to isp of middleware.
 *
 * 10 12 2010 koli.lin
 * [ALPS00127101] [Camera] AE will flash
 * [Camera]Create Vsync interrupt to handle the exposure time, sensor gain and raw gain control.
 *
 * 08 27 2010 ronnie.lai
 * [DUMA00032601] [Camera][ISP]
 * Check in AD5820 Constant AF function.
 *
 * 08 26 2010 ronnie.lai
 * [DUMA00032601] [Camera][ISP]
 * Add AD5820 Lens driver function.
 * must disable SWIC and bus log, otherwise the lens initial time take about 30 second.(without log about 3 sec)
 *
 * 08 19 2010 ronnie.lai
 * [DUMA00032601] [Camera][ISP]
 * Merge dual camera relative settings. Main OV5640, SUB O7675 ready.
 *
 * 08 18 2010 ronnie.lai
 * [DUMA00032601] [Camera][ISP]
 * Mmodify ISP setting and add OV5640 sensor driver.
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
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
#include <asm/io.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "ov5640yuv_Sensor.h"
#include "ov5640yuv_Camera_Sensor_para.h"
#include "ov5640yuv_CameraCustomized.h"

#include "kd_camera_feature.h"



/*
DBGPARAM dpCurSettings = {
    TEXT("Sensor"), {
        TEXT("Preview"),TEXT("Capture"),TEXT("Init"),TEXT("Error"),
        TEXT("Gain"),TEXT("Shutter"),TEXT("Undef"),TEXT("Undef"),
        TEXT("Undef"),TEXT("Undef"),TEXT("Undef"),TEXT("Undef"),
        TEXT("Undef"),TEXT("Undef"),TEXT("Undef"),TEXT("Undef")},
    0x00FF	// ZONE_INIT | ZONE_WARNING | ZONE_ERROR
};

*/
kal_bool  OV5640YUV_MPEG4_encode_mode = KAL_FALSE;
kal_uint16  OV5640YUV_sensor_gain_base=0x0;
/* MAX/MIN Explosure Lines Used By AE Algorithm */
kal_uint16 OV5640YUV_MAX_EXPOSURE_LINES = OV5640_PV_PERIOD_LINE_NUMS-4;
kal_uint8  OV5640YUV_MIN_EXPOSURE_LINES = 2;
kal_uint32 OV5640YUV_isp_master_clock;
kal_uint16 OV5640YUV_CURRENT_FRAME_LINES = OV5640_PV_PERIOD_LINE_NUMS;

static kal_uint16 OV5640YUV_dummy_pixels=0, OV5640YUV_dummy_lines=0;
kal_uint16 OV5640YUV_PV_dummy_pixels=0,OV5640YUV_PV_dummy_lines=0;

kal_uint8 OV5640YUV_sensor_write_I2C_address = OV5640_WRITE_ID;
kal_uint8 OV5640YUV_sensor_read_I2C_address = OV5640_READ_ID;

static kal_uint32 OV5640YUV_zoom_factor = 0; 

//add by lingnan for af status
static UINT8 STA_FOCUS = 0x8F; 
//static kal_uint32 MAC = 255;
//static kal_uint32 INF = 0;
static kal_uint32 AF_XS = 64;//version0.21, aug.2009
static kal_uint32 AF_YS = 48;//version0.21, aug.2009
//static kal_bool AF_INIT = FALSE;
static UINT8 ZONE[4] = {24, 18, 40, 30};////version0.21, aug.2009,center 4:3 window


#define LOG_TAG "[OV5640Yuv]"
#define SENSORDB(fmt, arg...) printk( LOG_TAG  fmt, ##arg)
#define RETAILMSG(x,...)
#define TEXT

kal_uint16 OV5640YUV_g_iDummyLines = 28; 


UINT8 OV5640YUVPixelClockDivider=0;
kal_uint32 OV5640YUV_sensor_pclk=52000000;;
kal_uint32 OV5640YUV_PV_pclk = 5525; 

kal_uint32 OV5640YUV_CAP_pclk = 6175;

kal_uint16 OV5640YUV_pv_exposure_lines=0x100,OV5640YUV_g_iBackupExtraExp = 0,OV5640YUV_extra_exposure_lines = 0;

kal_uint16 OV5640YUV_sensor_id=0;

MSDK_SENSOR_CONFIG_STRUCT OV5640YUVSensorConfigData;

kal_uint32 OV5640YUV_FAC_SENSOR_REG;
kal_uint16 OV5640YUV_sensor_flip_value;


/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT OV5640YUVSensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT OV5640YUVSensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/


////////////////////////////////////////////////////////////////
typedef enum
{
  OV5640_720P,       //1M 1280x960
  OV5640_5M,     //5M 2592x1944
} OV5640_RES_TYPE;
OV5640_RES_TYPE OV5640YUV_g_RES=OV5640_720P;

typedef enum
{
  OV5640_MODE_PREVIEW,  //1M  	1280x960
  OV5640_MODE_CAPTURE   //5M    2592x1944
} OV5640_MODE;
OV5640_MODE g_iOV5640YUV_Mode=OV5640_MODE_PREVIEW;


extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
extern int iBurstWriteReg(u8 *pData, u32 bytes, u16 i2cId); 
#define OV5640YUV_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, OV5640_WRITE_ID)
#define OV5640YUV_burst_write_cmos_sensor(pData, bytes)  iBurstWriteReg(pData, bytes, OV5640_WRITE_ID)

static UINT32 g_sensorAfStatus = 0;

#define PROFILE 1

#if PROFILE 
static struct timeval OV5640YUV_ktv1, OV5640YUV_ktv2; 
inline void OV5640YUV_imgSensorProfileStart(void)
{
    do_gettimeofday(&OV5640YUV_ktv1);    
}

inline void OV5640YUV_imgSensorProfileEnd(char *tag)
{
    unsigned long TimeIntervalUS;    
    do_gettimeofday(&OV5640YUV_ktv2);

    TimeIntervalUS = (OV5640YUV_ktv2.tv_sec - OV5640YUV_ktv1.tv_sec) * 1000000 + (OV5640YUV_ktv2.tv_usec - OV5640YUV_ktv1.tv_usec); 
    SENSORDB("[%s]Profile = %lu\n",tag, TimeIntervalUS);
}
#else 
inline static void OV5640YUV_imgSensorProfileStart() {}
inline static void OV5640YUV_imgSensorProfileEnd(char *tag) {}
#endif 


kal_uint16 OV5640YUV_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,OV5640_WRITE_ID);
    return get_byte;
}


#define Sleep(ms) mdelay(ms)

static atomic_t OV5640_SetShutter_Flag; 
static wait_queue_head_t OV5640_SetShutter_waitQueue;
void OV5640YUV_write_shutter(kal_uint16 shutter)
{
    kal_uint16 iExp = shutter;
    kal_uint16 OV5640_g_iExtra_ExpLines = 0 ;
//    kal_uint16 OV5640_g_bXGA_Mode = 0; 
    int timeOut = 0; 

    if (atomic_read(&OV5640_SetShutter_Flag) == 1) {
        timeOut = wait_event_interruptible_timeout(
            OV5640_SetShutter_waitQueue, atomic_read(&OV5640_SetShutter_Flag) == 0, 1 * HZ);        
        if (timeOut == 0) {
            SENSORDB("[OV5640YUV_SetGain] Set Gain Wait Queue time out \n"); 
            return; 
        }
    }    
    atomic_set(&OV5640_SetShutter_Flag, 1); 

    if (OV5640YUV_g_RES == OV5640_720P) {
        if (iExp <= OV5640_PV_EXPOSURE_LIMITATION) {
            OV5640_g_iExtra_ExpLines = 0;
        }else {
            OV5640_g_iExtra_ExpLines = iExp - OV5640_PV_EXPOSURE_LIMITATION ;
        }

    }else {
        if (iExp <= OV5640_FULL_EXPOSURE_LIMITATION) {
            OV5640_g_iExtra_ExpLines = 0;
        }else {
            OV5640_g_iExtra_ExpLines = iExp - OV5640_FULL_EXPOSURE_LIMITATION;
        }
    }


//    OV5640YUV_write_cmos_sensor(0x3212, 0x01); 
    if (OV5640YUV_MPEG4_encode_mode != TRUE) {
    OV5640YUV_write_cmos_sensor(0x350c, OV5640_g_iExtra_ExpLines >> 8);
    OV5640YUV_write_cmos_sensor(0x350d, OV5640_g_iExtra_ExpLines & 0x00FF);
    }
    
    OV5640YUV_write_cmos_sensor(0x3500, (iExp >> 12) & 0xFF);
    OV5640YUV_write_cmos_sensor(0x3501, (iExp >> 4 ) & 0xFF);
    OV5640YUV_write_cmos_sensor(0x3502, (iExp <<4 ) & 0xFF);
//    OV5640YUV_write_cmos_sensor(0x3212, 0x11); 
//    OV5640YUV_write_cmos_sensor(0x3212, 0xa1);     

    OV5640YUV_g_iBackupExtraExp = OV5640_g_iExtra_ExpLines;    
    atomic_set(&OV5640_SetShutter_Flag, 0);
    wake_up_interruptible(&OV5640_SetShutter_waitQueue);    
}   /* write_OV5640_shutter */

static kal_uint16 OV5640YUVReg2Gain(const kal_uint8 iReg)
{
	return iReg;
	/*
    kal_uint8 iI;
    kal_uint16 iGain = BASEGAIN;    // 1x-gain base

    // Range: 1x to 32x
    // Gain = (GAIN[7] + 1) * (GAIN[6] + 1) * (GAIN[5] + 1) * (GAIN[4] + 1) * (1 + GAIN[3:0] / 16)
    for (iI = 7; iI >= 4; iI--) {
        iGain *= (((iReg >> iI) & 0x01) + 1);
    }

    return iGain +  iGain * (iReg & 0x0F) / 16;
    */
}

static kal_uint8 OV5640YUVGain2Reg(const kal_uint16 iGain)
{
	return iGain;
/*
    kal_uint8 iReg = 0x00;

    if (iGain < 2 * BASEGAIN) {
        // Gain = 1 + GAIN[3:0](0x00) / 16
        //iReg = 16 * (iGain - BASEGAIN) / BASEGAIN;
        iReg = 16 * iGain / BASEGAIN - 16; 
    }else if (iGain < 4 * BASEGAIN) {
        // Gain = 2 * (1 + GAIN[3:0](0x00) / 16)
        iReg |= 0x10;
        //iReg |= 8 * (iGain - 2 * BASEGAIN) / BASEGAIN;
        iReg |= (8 *iGain / BASEGAIN - 16); 
    }else if (iGain < 8 * BASEGAIN) {
        // Gain = 4 * (1 + GAIN[3:0](0x00) / 16)
        iReg |= 0x30;
        //iReg |= 4 * (iGain - 4 * BASEGAIN) / BASEGAIN;
        iReg |= (4 * iGain / BASEGAIN - 16); 
    }else if (iGain < 16 * BASEGAIN) {
        // Gain = 8 * (1 + GAIN[3:0](0x00) / 16)
        iReg |= 0x70;
        //iReg |= 2 * (iGain - 8 * BASEGAIN) / BASEGAIN;
        iReg |= (2 * iGain / BASEGAIN - 16); 
    }else if (iGain < 32 * BASEGAIN) {
        // Gain = 16 * (1 + GAIN[3:0](0x00) / 16)
        iReg |= 0xF0;
        //iReg |= (iGain - 16 * BASEGAIN) / BASEGAIN;
        iReg |= (iGain / BASEGAIN - 16); 
    }else {
        ASSERT(0);
    }

    return iReg;
*/
}

/*************************************************************************
* FUNCTION
*    OV5640YUV_SetGain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    gain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
//! Due to the OV5640 set gain will happen race condition. 
//! It need to use a critical section to protect it. 
static atomic_t OV5640_SetGain_Flag; 
static wait_queue_head_t OV5640_SetGain_waitQueue;
void OV5640YUV_SetGain(UINT16 iGain)
{
    kal_uint8 iReg;
    int timeOut = 0; 

    //OV5640YUV_imgSensorProfileStart();
    //SENSORDB("[OV5640YUV_SetGain] E Gain = %d \n", iGain); 
    if (atomic_read(&OV5640_SetGain_Flag) == 1) {
        timeOut = wait_event_interruptible_timeout(
            OV5640_SetGain_waitQueue, atomic_read(&OV5640_SetGain_Flag) == 0, 1 * HZ);        
        if (timeOut == 0) {
            SENSORDB("[OV5640YUV_SetGain] Set Gain Wait Queue time out \n"); 
            return; 
        }
    }    
    atomic_set(&OV5640_SetGain_Flag, 1); 
    //iReg = OV5640YUVGain2Reg(iGain);
    //SENSORDB("transfer gain 0x%x(%d) to 0x%x(%d)\n",iGain,iGain,iReg,iReg);
    //! For OV5640 sensor, the set gain don't have double buffer,  
    //! it needs use group write to write sensor gain 
    OV5640YUV_write_cmos_sensor(0x3212, 0x00); 
    //OV5640YUV_write_cmos_sensor(0x350B, (kal_uint32)iReg);
    OV5640YUV_write_cmos_sensor(0x350B, (kal_uint32)iGain);
    OV5640YUV_write_cmos_sensor(0x3212, 0x10); 
    OV5640YUV_write_cmos_sensor(0x3212, 0xA0); 
    
    //OV5640YUV_imgSensorProfileEnd("OV5640YUV_SetGain"); 
    //SENSORDB("Gain = %x\n", iReg);

    atomic_set(&OV5640_SetGain_Flag, 0);
    wake_up_interruptible(&OV5640_SetGain_waitQueue);
    //SENSORDB("[OV5640YUV_SetGain] X Gain = %d \n", iGain); 
}   /*  OV5640YUV_SetGain  */


/*************************************************************************
* FUNCTION
*    read_OV5640YUV_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 read_OV5640YUV_gain(void)
{

    kal_uint8 temp_gain;
    kal_uint16 gain;
    //temp_gain = OV5640YUV_read_cmos_sensor(0x350B);
    //gain = OV5640YUVReg2Gain(temp_gain);
    gain = OV5640YUV_read_cmos_sensor(0x350B);
    SENSORDB("0x350b=0x%x, transfer to gain=0x%x,%d\n",temp_gain,gain,gain);
    return gain;
}  /* read_OV5640YUV_gain */

void write_OV5640YUV_gain(kal_uint16 gain)
{
    OV5640YUV_SetGain(gain);
}
void OV5640YUV_camera_para_to_sensor(void)
{
    kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=OV5640YUVSensorReg[i].Addr; i++)
    {
        OV5640YUV_write_cmos_sensor(OV5640YUVSensorReg[i].Addr, OV5640YUVSensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=OV5640YUVSensorReg[i].Addr; i++)
    {
        OV5640YUV_write_cmos_sensor(OV5640YUVSensorReg[i].Addr, OV5640YUVSensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        OV5640YUV_write_cmos_sensor(OV5640YUVSensorCCT[i].Addr, OV5640YUVSensorCCT[i].Para);
    }
}


/*************************************************************************
* FUNCTION
*    OV5640YUV_sensor_to_camera_para
*
* DESCRIPTION
*    // update camera_para from sensor register
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV5640YUV_sensor_to_camera_para(void)
{
    kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=OV5640YUVSensorReg[i].Addr; i++)
    {
        OV5640YUVSensorReg[i].Para = OV5640YUV_read_cmos_sensor(OV5640YUVSensorReg[i].Addr);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=OV5640YUVSensorReg[i].Addr; i++)
    {
        OV5640YUVSensorReg[i].Para = OV5640YUV_read_cmos_sensor(OV5640YUVSensorReg[i].Addr);
    }
}


/*************************************************************************
* FUNCTION
*    OV5640YUV_get_sensor_group_count
*
* DESCRIPTION
*    //
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_int32  OV5640YUV_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void OV5640YUV_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
   switch (group_idx)
   {
        case PRE_GAIN:
            sprintf((char *)group_name_ptr, "CCT");
            *item_count_ptr = 2;
            break;
        case CMMCLK_CURRENT:
            sprintf((char *)group_name_ptr, "CMMCLK Current");
            *item_count_ptr = 1;
            break;
        case FRAME_RATE_LIMITATION:
            sprintf((char *)group_name_ptr, "Frame Rate Limitation");
            *item_count_ptr = 2;
            break;
        case REGISTER_EDITOR:
            sprintf((char *)group_name_ptr, "Register Editor");
            *item_count_ptr = 2;
            break;
        default:
            ASSERT(0);
}
}

void OV5640YUV_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{
    kal_int16 temp_reg=0;
    kal_uint16 temp_gain=0, temp_addr=0, temp_para=0;
    
    switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Global");
                    temp_addr = PRE_GAIN_INDEX;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"GLOBAL_GAIN");
                    temp_addr = GLOBAL_GAIN_INDEX;
                    break;
                default:
                    ASSERT(0);
            }
            temp_para=OV5640YUVSensorCCT[temp_addr].Para;
            temp_gain = OV5640YUVReg2Gain(temp_para);

            temp_gain=(temp_gain*1000)/BASEGAIN;

            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min=1000;
            info_ptr->Max=15875;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                
                    //temp_reg=OV5640YUVSensorReg[CMMCLK_CURRENT_INDEX].Para;
                    temp_reg = ISP_DRIVING_2MA;
                    if(temp_reg==ISP_DRIVING_2MA)
                    {
                        info_ptr->ItemValue=2;
                    }
                    else if(temp_reg==ISP_DRIVING_4MA)
                    {
                        info_ptr->ItemValue=4;
                    }
                    else if(temp_reg==ISP_DRIVING_6MA)
                    {
                        info_ptr->ItemValue=6;
                    }
                    else if(temp_reg==ISP_DRIVING_8MA)
                    {
                        info_ptr->ItemValue=8;
                    }
                
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_TRUE;
                    info_ptr->Min=2;
                    info_ptr->Max=8;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Max Exposure Lines");
                    info_ptr->ItemValue=OV5640YUV_MAX_EXPOSURE_LINES;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"Min Frame Rate");
                    info_ptr->ItemValue=12;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Addr.");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Value");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                default:
                ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
}

//void OV5640YUV_set_isp_driving_current(kal_uint8 current)
//{
//}

kal_bool OV5640YUV_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
//   kal_int16 temp_reg;
   kal_uint16 temp_addr=0, temp_para=0;

   switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
                case 0:
                    temp_addr = PRE_GAIN_INDEX;
                    break;
                case 1:
                    temp_addr = GLOBAL_GAIN_INDEX;
                    break;
                default:
                    ASSERT(0);
            }

            temp_para = OV5640YUVGain2Reg(ItemValue);


            OV5640YUVSensorCCT[temp_addr].Para = temp_para;
            OV5640YUV_write_cmos_sensor(OV5640YUVSensorCCT[temp_addr].Addr,temp_para);

            OV5640YUV_sensor_gain_base=read_OV5640YUV_gain();

            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    if(ItemValue==2)
                    {
                        OV5640YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_2MA;
                        //OV5640YUV_set_isp_driving_current(ISP_DRIVING_2MA);
                    }
                    else if(ItemValue==3 || ItemValue==4)
                    {
                        OV5640YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_4MA;
                        //OV5640YUV_set_isp_driving_current(ISP_DRIVING_4MA);
                    }
                    else if(ItemValue==5 || ItemValue==6)
                    {
                        OV5640YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_6MA;
                        //OV5640YUV_set_isp_driving_current(ISP_DRIVING_6MA);
                    }
                    else
                    {
                        OV5640YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_8MA;
                        //OV5640YUV_set_isp_driving_current(ISP_DRIVING_8MA);
                    }
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            ASSERT(0);
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    OV5640YUV_FAC_SENSOR_REG=ItemValue;
                    break;
                case 1:
                    OV5640YUV_write_cmos_sensor(OV5640YUV_FAC_SENSOR_REG,ItemValue);
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

static void OV5640YUV_SetDummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
    kal_uint16 LinesOneframe;
    kal_uint16 PixelsOneline = OV5640_FULL_PERIOD_PIXEL_NUMS;
    if(OV5640_720P == OV5640YUV_g_RES)
    {
        PixelsOneline = (OV5640_PV_PERIOD_PIXEL_NUMS_HTS + iPixels );
        LinesOneframe =iLines + OV5640_PV_PERIOD_LINE_NUMS_VTS;
        if(OV5640YUV_MPEG4_encode_mode == KAL_FALSE)//for Fix video framerate
            OV5640YUV_CURRENT_FRAME_LINES = iLines + OV5640_PV_PERIOD_LINE_NUMS_VTS;
    }
    else if (OV5640_5M == OV5640YUV_g_RES)
    {
        PixelsOneline = OV5640_FULL_PERIOD_PIXEL_NUMS_HTS + iPixels;
	 LinesOneframe =iLines + OV5640_FULL_PERIOD_LINE_NUMS_VTS;

        OV5640YUV_CURRENT_FRAME_LINES = iLines + OV5640_FULL_PERIOD_LINE_NUMS_VTS;
    }
    if(iPixels)
    {
    	OV5640YUV_write_cmos_sensor(0x380c, (PixelsOneline >> 8) & 0xFF);
    	OV5640YUV_write_cmos_sensor(0x380d, PixelsOneline & 0xFF);
    }
    if(iLines)
    {
    	OV5640YUV_write_cmos_sensor(0x380e, (LinesOneframe >> 8) & 0xFF);
    	OV5640YUV_write_cmos_sensor(0x380f, LinesOneframe & 0xFF);
    }
    

#if 0 //! Don't set shutter related register 
    if(OV5640YUV_MPEG4_encode_mode == KAL_FALSE)//for Fix video framerate,set the frame lines in night mode function
    {
        //Set dummy lines.
        //The maximum shutter value = Line_Without_Dummy + Dummy_lines
        OV5640YUV_write_cmos_sensor(0x350C, (OV5640YUV_CURRENT_FRAME_LINES >> 8) & 0xFF);
        OV5640YUV_write_cmos_sensor(0x350D, OV5640YUV_CURRENT_FRAME_LINES & 0xFF);
        OV5640YUV_MAX_EXPOSURE_LINES = OV5640YUV_CURRENT_FRAME_LINES - OV5640_SHUTTER_LINES_GAP;
    }
#endif     
}   /*  OV5640YUV_SetDummy */

static void OV5640YUV_set_AE_mode(kal_bool AE_enable)
{
    kal_uint8 temp_AE_reg = 0;

    if (AE_enable == KAL_TRUE)
    {
        // turn on AEC/AGC
        temp_AE_reg = OV5640YUV_read_cmos_sensor(0x3503);
        OV5640YUV_write_cmos_sensor(0x3503, temp_AE_reg&~0x07);
  
    }
    else
    {
        // turn off AEC/AGC
        temp_AE_reg = OV5640YUV_read_cmos_sensor(0x3503);
        OV5640YUV_write_cmos_sensor(0x3503, temp_AE_reg| 0x07);

    }
}


static void OV5640YUV_set_AWB_mode(kal_bool AWB_enable)
{
    kal_uint8 temp_AWB_reg = 0;

    //return ;

    if (AWB_enable == KAL_TRUE)
    {
        //enable Auto WB
        temp_AWB_reg = OV5640YUV_read_cmos_sensor(0x3406);
        OV5640YUV_write_cmos_sensor(0x3406, temp_AWB_reg & ~0x01);        
    }
    else
    {
        //turn off AWB
        temp_AWB_reg = OV5640YUV_read_cmos_sensor(0x3406);
        OV5640YUV_write_cmos_sensor(0x3406, temp_AWB_reg | 0x01);        
    }
}

static void OV5640_FOCUS_AD5820_Check_MCU()
{
    kal_uint8 check[13] = {0x00};
	//mcu on
    check[0] = OV5640YUV_read_cmos_sensor(0x3000);
    check[1] = OV5640YUV_read_cmos_sensor(0x3004);
	//soft reset of mcu
    check[2] = OV5640YUV_read_cmos_sensor(0x3f00);	
	//afc on
    check[3] = OV5640YUV_read_cmos_sensor(0x3001);
    check[4] = OV5640YUV_read_cmos_sensor(0x3005);
	//gpio1,gpio2
    check[5] = OV5640YUV_read_cmos_sensor(0x3018);
    check[6] = OV5640YUV_read_cmos_sensor(0x301e);
    check[7] = OV5640YUV_read_cmos_sensor(0x301b);
    check[8] = OV5640YUV_read_cmos_sensor(0x3042);
	//y0
    check[9] = OV5640YUV_read_cmos_sensor(0x3018);
    check[10] = OV5640YUV_read_cmos_sensor(0x301e);
    check[11] = OV5640YUV_read_cmos_sensor(0x301b);
    check[12] = OV5640YUV_read_cmos_sensor(0x3042);

    int i = 0;
    for(i = 0; i < 13; i++)
    SENSORDB("check[%d]=0x%x\n", i, check[i]);

	
}


void OV5640YUV_Sensor_Init_set_720P(void);
void OV5640YUV_set_5M_init(void);
void OV5640YUV_IQ(void);
static void OV5640_FOCUS_AD5820_Init(void);



/*******************************************************************************
*
********************************************************************************/
static void OV5640YUV_Sensor_Init(void)
{
    SENSORDB("lln:: OV5640YUV_Sensor_Init, use OV5640YUV_Sensor_Init_set_720P");
    OV5640YUV_Sensor_Init_set_720P();

//    OV5640_FOCUS_AD5820_Init();

    SENSORDB("Init Success \n");
}   /*  OV5640YUV_Sensor_Init  */


void OV5640YUV_Sensor_Init_set_720P(void)
{
	//24Mhz Mclk ,56MHz Pclk, 15fps
	OV5640YUV_write_cmos_sensor(0x3103, 0x11);
	OV5640YUV_write_cmos_sensor(0x3008, 0x82);
	OV5640YUV_write_cmos_sensor(0x3008, 0x42);
	OV5640YUV_write_cmos_sensor(0x3103, 0x03);
	OV5640YUV_write_cmos_sensor(0x3017, 0xff);
	OV5640YUV_write_cmos_sensor(0x3018, 0xff);
	OV5640YUV_write_cmos_sensor(0x3034, 0x1a);
	OV5640YUV_write_cmos_sensor(0x3035, 0x21);
	OV5640YUV_write_cmos_sensor(0x3036, 0x46);
	OV5640YUV_write_cmos_sensor(0x3037, 0x13);
	OV5640YUV_write_cmos_sensor(0x3108, 0x01);
	OV5640YUV_write_cmos_sensor(0x3630, 0x2e);
	OV5640YUV_write_cmos_sensor(0x3632, 0xe2);
	OV5640YUV_write_cmos_sensor(0x3633, 0x23);
	OV5640YUV_write_cmos_sensor(0x3621, 0xe0);
	OV5640YUV_write_cmos_sensor(0x3704, 0xa0);
	OV5640YUV_write_cmos_sensor(0x3703, 0x5a);
	OV5640YUV_write_cmos_sensor(0x3715, 0x78);
	OV5640YUV_write_cmos_sensor(0x3717, 0x01);
	OV5640YUV_write_cmos_sensor(0x370b, 0x60);
	OV5640YUV_write_cmos_sensor(0x3705, 0x1a);
	OV5640YUV_write_cmos_sensor(0x3905, 0x02);
	OV5640YUV_write_cmos_sensor(0x3906, 0x10);
	OV5640YUV_write_cmos_sensor(0x3901, 0x0a);
	OV5640YUV_write_cmos_sensor(0x3731, 0x12);
	OV5640YUV_write_cmos_sensor(0x3600, 0x08);
	OV5640YUV_write_cmos_sensor(0x3601, 0x33);
	OV5640YUV_write_cmos_sensor(0x302d, 0x60);
	OV5640YUV_write_cmos_sensor(0x3620, 0x52);
	OV5640YUV_write_cmos_sensor(0x371b, 0x20);
	OV5640YUV_write_cmos_sensor(0x471c, 0x50);
	OV5640YUV_write_cmos_sensor(0x3a18, 0x00);
	OV5640YUV_write_cmos_sensor(0x3a19, 0xf8);
	OV5640YUV_write_cmos_sensor(0x3635, 0x1c);
	OV5640YUV_write_cmos_sensor(0x3634, 0x40);
	OV5640YUV_write_cmos_sensor(0x3622, 0x01);
	OV5640YUV_write_cmos_sensor(0x3c00, 0x04);
	OV5640YUV_write_cmos_sensor(0x3c01, 0xb4);
	OV5640YUV_write_cmos_sensor(0x3c04, 0x28);
	OV5640YUV_write_cmos_sensor(0x3c05, 0x98);
	OV5640YUV_write_cmos_sensor(0x3c06, 0x00);
	OV5640YUV_write_cmos_sensor(0x3c07, 0x08);
	OV5640YUV_write_cmos_sensor(0x3c08, 0x00);
	OV5640YUV_write_cmos_sensor(0x3c09, 0x1c);
	OV5640YUV_write_cmos_sensor(0x3c0a, 0x9c);
	OV5640YUV_write_cmos_sensor(0x3c0b, 0x40);
	OV5640YUV_write_cmos_sensor(0x3820, 0x41);
	OV5640YUV_write_cmos_sensor(0x3821, 0x07);
	OV5640YUV_write_cmos_sensor(0x3814, 0x31);
	OV5640YUV_write_cmos_sensor(0x3815, 0x31);
	OV5640YUV_write_cmos_sensor(0x3800, 0x00);
	OV5640YUV_write_cmos_sensor(0x3801, 0x00);
	OV5640YUV_write_cmos_sensor(0x3802, 0x00);
	OV5640YUV_write_cmos_sensor(0x3803, 0x04);
	OV5640YUV_write_cmos_sensor(0x3804, 0x0a);
	OV5640YUV_write_cmos_sensor(0x3805, 0x3f);
	OV5640YUV_write_cmos_sensor(0x3806, 0x07);
	OV5640YUV_write_cmos_sensor(0x3807, 0x9b);
	OV5640YUV_write_cmos_sensor(0x3808, 0x05); 
	OV5640YUV_write_cmos_sensor(0x3809, 0x00);
	OV5640YUV_write_cmos_sensor(0x380a, 0x03);
	OV5640YUV_write_cmos_sensor(0x380b, 0xc0);
	OV5640YUV_write_cmos_sensor(0x380c, 0x07);
	OV5640YUV_write_cmos_sensor(0x380d, 0x68);
	OV5640YUV_write_cmos_sensor(0x380e, 0x03);
	OV5640YUV_write_cmos_sensor(0x380f, 0xd8);
	OV5640YUV_write_cmos_sensor(0x3810, 0x00);
	OV5640YUV_write_cmos_sensor(0x3811, 0x04);
	OV5640YUV_write_cmos_sensor(0x3812, 0x00);
	OV5640YUV_write_cmos_sensor(0x3813, 0x00);
	OV5640YUV_write_cmos_sensor(0x3618, 0x00);
	OV5640YUV_write_cmos_sensor(0x3612, 0x29);
	OV5640YUV_write_cmos_sensor(0x3708, 0x64);
	OV5640YUV_write_cmos_sensor(0x3709, 0x52);
	OV5640YUV_write_cmos_sensor(0x370c, 0x03);
	OV5640YUV_write_cmos_sensor(0x3a02, 0x03);
	OV5640YUV_write_cmos_sensor(0x3a03, 0xd8);
	OV5640YUV_write_cmos_sensor(0x3a08, 0x00);
	OV5640YUV_write_cmos_sensor(0x3a09, 0x94);
	OV5640YUV_write_cmos_sensor(0x3a0a, 0x00);
	OV5640YUV_write_cmos_sensor(0x3a0b, 0x7b);
	OV5640YUV_write_cmos_sensor(0x3a0e, 0x06);
	OV5640YUV_write_cmos_sensor(0x3a0d, 0x08);
	OV5640YUV_write_cmos_sensor(0x3a14, 0x03);
	OV5640YUV_write_cmos_sensor(0x3a15, 0xd8);
	OV5640YUV_write_cmos_sensor(0x4001, 0x02);
	OV5640YUV_write_cmos_sensor(0x4004, 0x02);
	OV5640YUV_write_cmos_sensor(0x3000, 0x00);
	OV5640YUV_write_cmos_sensor(0x3002, 0x1c);
	OV5640YUV_write_cmos_sensor(0x3004, 0xff);
	OV5640YUV_write_cmos_sensor(0x3006, 0xc3);
	OV5640YUV_write_cmos_sensor(0x300e, 0x58);
	OV5640YUV_write_cmos_sensor(0x302e, 0x00);
	OV5640YUV_write_cmos_sensor(0x4300, 0x30);
	OV5640YUV_write_cmos_sensor(0x501f, 0x00);
	OV5640YUV_write_cmos_sensor(0x4713, 0x03);
	OV5640YUV_write_cmos_sensor(0x4407, 0x04);
	OV5640YUV_write_cmos_sensor(0x460b, 0x35);
	OV5640YUV_write_cmos_sensor(0x460c, 0x20);
	OV5640YUV_write_cmos_sensor(0x3824, 0x02);
	OV5640YUV_write_cmos_sensor(0x5000, 0xa7);
	OV5640YUV_write_cmos_sensor(0x5001, 0xa3);
	OV5640YUV_write_cmos_sensor(0x5180, 0xff);
	OV5640YUV_write_cmos_sensor(0x5181, 0xf2);
	OV5640YUV_write_cmos_sensor(0x5182, 0x00);
	OV5640YUV_write_cmos_sensor(0x5183, 0x14);
	OV5640YUV_write_cmos_sensor(0x5184, 0x25);
	OV5640YUV_write_cmos_sensor(0x5185, 0x24);
	OV5640YUV_write_cmos_sensor(0x5186, 0x09);
	OV5640YUV_write_cmos_sensor(0x5187, 0x09);
	OV5640YUV_write_cmos_sensor(0x5188, 0x09);
	OV5640YUV_write_cmos_sensor(0x5189, 0x75);
	OV5640YUV_write_cmos_sensor(0x518a, 0x54);
	OV5640YUV_write_cmos_sensor(0x518b, 0xe0);
	OV5640YUV_write_cmos_sensor(0x518c, 0xb2);
	OV5640YUV_write_cmos_sensor(0x518d, 0x42);
	OV5640YUV_write_cmos_sensor(0x518e, 0x3d);
	OV5640YUV_write_cmos_sensor(0x518f, 0x56);
	OV5640YUV_write_cmos_sensor(0x5190, 0x46);
	OV5640YUV_write_cmos_sensor(0x5191, 0xf8);
	OV5640YUV_write_cmos_sensor(0x5192, 0x04);
	OV5640YUV_write_cmos_sensor(0x5193, 0x70);
	OV5640YUV_write_cmos_sensor(0x5194, 0xf0);
	OV5640YUV_write_cmos_sensor(0x5195, 0xf0);
	OV5640YUV_write_cmos_sensor(0x5196, 0x03);
	OV5640YUV_write_cmos_sensor(0x5197, 0x01);
	OV5640YUV_write_cmos_sensor(0x5198, 0x04);
	OV5640YUV_write_cmos_sensor(0x5199, 0x12);
	OV5640YUV_write_cmos_sensor(0x519a, 0x04);
	OV5640YUV_write_cmos_sensor(0x519b, 0x00);
	OV5640YUV_write_cmos_sensor(0x519c, 0x06);
	OV5640YUV_write_cmos_sensor(0x519d, 0x82);
	OV5640YUV_write_cmos_sensor(0x519e, 0x38);
	OV5640YUV_write_cmos_sensor(0x5381, 0x1c);
	OV5640YUV_write_cmos_sensor(0x5382, 0x5a);
	OV5640YUV_write_cmos_sensor(0x5383, 0x06);
	OV5640YUV_write_cmos_sensor(0x5384, 0x0a);
	OV5640YUV_write_cmos_sensor(0x5385, 0x7e);
	OV5640YUV_write_cmos_sensor(0x5386, 0x88);
	OV5640YUV_write_cmos_sensor(0x5387, 0x7c);
	OV5640YUV_write_cmos_sensor(0x5388, 0x6c);
	OV5640YUV_write_cmos_sensor(0x5389, 0x10);
	OV5640YUV_write_cmos_sensor(0x538a, 0x01);
	OV5640YUV_write_cmos_sensor(0x538b, 0x98);
	OV5640YUV_write_cmos_sensor(0x5300, 0x08);
	OV5640YUV_write_cmos_sensor(0x5301, 0x30);
	OV5640YUV_write_cmos_sensor(0x5302, 0x10);
	OV5640YUV_write_cmos_sensor(0x5303, 0x00);
	OV5640YUV_write_cmos_sensor(0x5304, 0x08);
	OV5640YUV_write_cmos_sensor(0x5305, 0x30);
	OV5640YUV_write_cmos_sensor(0x5306, 0x08);
	OV5640YUV_write_cmos_sensor(0x5307, 0x16);
	OV5640YUV_write_cmos_sensor(0x5309, 0x08);
	OV5640YUV_write_cmos_sensor(0x530a, 0x30);
	OV5640YUV_write_cmos_sensor(0x530b, 0x04);
	OV5640YUV_write_cmos_sensor(0x530c, 0x06);
	OV5640YUV_write_cmos_sensor(0x5480, 0x01);
	OV5640YUV_write_cmos_sensor(0x5481, 0x08);
	OV5640YUV_write_cmos_sensor(0x5482, 0x14);
	OV5640YUV_write_cmos_sensor(0x5483, 0x28);
	OV5640YUV_write_cmos_sensor(0x5484, 0x51);
	OV5640YUV_write_cmos_sensor(0x5485, 0x65);
	OV5640YUV_write_cmos_sensor(0x5486, 0x71);
	OV5640YUV_write_cmos_sensor(0x5487, 0x7d);
	OV5640YUV_write_cmos_sensor(0x5488, 0x87);
	OV5640YUV_write_cmos_sensor(0x5489, 0x91);
	OV5640YUV_write_cmos_sensor(0x548a, 0x9a);
	OV5640YUV_write_cmos_sensor(0x548b, 0xaa);
	OV5640YUV_write_cmos_sensor(0x548c, 0xb8);
	OV5640YUV_write_cmos_sensor(0x548d, 0xcd);
	OV5640YUV_write_cmos_sensor(0x548e, 0xdd);
	OV5640YUV_write_cmos_sensor(0x548f, 0xea);
	OV5640YUV_write_cmos_sensor(0x5490, 0x1d);
	OV5640YUV_write_cmos_sensor(0x5580, 0x02);
	OV5640YUV_write_cmos_sensor(0x5583, 0x40);
	OV5640YUV_write_cmos_sensor(0x5584, 0x10);
	OV5640YUV_write_cmos_sensor(0x5589, 0x10);
	OV5640YUV_write_cmos_sensor(0x558a, 0x00);
	OV5640YUV_write_cmos_sensor(0x558b, 0xf8);
	OV5640YUV_write_cmos_sensor(0x5800, 0x23);
	OV5640YUV_write_cmos_sensor(0x5801, 0x14);
	OV5640YUV_write_cmos_sensor(0x5802, 0x0f);
	OV5640YUV_write_cmos_sensor(0x5803, 0x0f);
	OV5640YUV_write_cmos_sensor(0x5804, 0x12);
	OV5640YUV_write_cmos_sensor(0x5805, 0x26);
	OV5640YUV_write_cmos_sensor(0x5806, 0x0c);
	OV5640YUV_write_cmos_sensor(0x5807, 0x08);
	OV5640YUV_write_cmos_sensor(0x5808, 0x05);
	OV5640YUV_write_cmos_sensor(0x5809, 0x05);
	OV5640YUV_write_cmos_sensor(0x580a, 0x08);
	OV5640YUV_write_cmos_sensor(0x580b, 0x0d);
	OV5640YUV_write_cmos_sensor(0x580c, 0x08);
	OV5640YUV_write_cmos_sensor(0x580d, 0x03);
	OV5640YUV_write_cmos_sensor(0x580e, 0x00);
	OV5640YUV_write_cmos_sensor(0x580f, 0x00);
	OV5640YUV_write_cmos_sensor(0x5810, 0x03);
	OV5640YUV_write_cmos_sensor(0x5811, 0x09);
	OV5640YUV_write_cmos_sensor(0x5812, 0x07);
	OV5640YUV_write_cmos_sensor(0x5813, 0x03);
	OV5640YUV_write_cmos_sensor(0x5814, 0x00);
	OV5640YUV_write_cmos_sensor(0x5815, 0x01);
	OV5640YUV_write_cmos_sensor(0x5816, 0x03);
	OV5640YUV_write_cmos_sensor(0x5817, 0x08);
	OV5640YUV_write_cmos_sensor(0x5818, 0x0d);
	OV5640YUV_write_cmos_sensor(0x5819, 0x08);
	OV5640YUV_write_cmos_sensor(0x581a, 0x05);
	OV5640YUV_write_cmos_sensor(0x581b, 0x06);
	OV5640YUV_write_cmos_sensor(0x581c, 0x08);
	OV5640YUV_write_cmos_sensor(0x581d, 0x0e);
	OV5640YUV_write_cmos_sensor(0x581e, 0x29);
	OV5640YUV_write_cmos_sensor(0x581f, 0x17);
	OV5640YUV_write_cmos_sensor(0x5820, 0x11);
	OV5640YUV_write_cmos_sensor(0x5821, 0x11);
	OV5640YUV_write_cmos_sensor(0x5822, 0x15);
	OV5640YUV_write_cmos_sensor(0x5823, 0x28);
	OV5640YUV_write_cmos_sensor(0x5824, 0x46);
	OV5640YUV_write_cmos_sensor(0x5825, 0x26);
	OV5640YUV_write_cmos_sensor(0x5826, 0x08);
	OV5640YUV_write_cmos_sensor(0x5827, 0x26);
	OV5640YUV_write_cmos_sensor(0x5828, 0x64);
	OV5640YUV_write_cmos_sensor(0x5829, 0x26);
	OV5640YUV_write_cmos_sensor(0x582a, 0x24);
	OV5640YUV_write_cmos_sensor(0x582b, 0x22);
	OV5640YUV_write_cmos_sensor(0x582c, 0x24);
	OV5640YUV_write_cmos_sensor(0x582d, 0x24);
	OV5640YUV_write_cmos_sensor(0x582e, 0x06);
	OV5640YUV_write_cmos_sensor(0x582f, 0x22);
	OV5640YUV_write_cmos_sensor(0x5830, 0x40);
	OV5640YUV_write_cmos_sensor(0x5831, 0x42);
	OV5640YUV_write_cmos_sensor(0x5832, 0x24);
	OV5640YUV_write_cmos_sensor(0x5833, 0x26);
	OV5640YUV_write_cmos_sensor(0x5834, 0x24);
	OV5640YUV_write_cmos_sensor(0x5835, 0x22);
	OV5640YUV_write_cmos_sensor(0x5836, 0x22);
	OV5640YUV_write_cmos_sensor(0x5837, 0x26);
	OV5640YUV_write_cmos_sensor(0x5838, 0x44);
	OV5640YUV_write_cmos_sensor(0x5839, 0x24);
	OV5640YUV_write_cmos_sensor(0x583a, 0x26);
	OV5640YUV_write_cmos_sensor(0x583b, 0x28);
	OV5640YUV_write_cmos_sensor(0x583c, 0x42);
	OV5640YUV_write_cmos_sensor(0x583d, 0xce);
	OV5640YUV_write_cmos_sensor(0x5025, 0x00);
/*	
	OV5640YUV_write_cmos_sensor(0x3a0f, 0x30);
	OV5640YUV_write_cmos_sensor(0x3a10, 0x28);
	OV5640YUV_write_cmos_sensor(0x3a1b, 0x30);
	OV5640YUV_write_cmos_sensor(0x3a1e, 0x26);
	OV5640YUV_write_cmos_sensor(0x3a11, 0x60);
	OV5640YUV_write_cmos_sensor(0x3a1f, 0x14);
*/
	OV5640YUV_write_cmos_sensor(0x3a0f, 0x38);
	OV5640YUV_write_cmos_sensor(0x3a10, 0x30);
	OV5640YUV_write_cmos_sensor(0x3a11, 0x61);
	OV5640YUV_write_cmos_sensor(0x3a1b, 0x38);
	OV5640YUV_write_cmos_sensor(0x3a1e, 0x30);
	OV5640YUV_write_cmos_sensor(0x3a1f, 0x10);
	

        OV5640YUV_IQ();

	OV5640YUV_write_cmos_sensor(0x3008, 0x02);

	
}
void OV5640YUV_set_720P(void)
{
    int dummy_pixels, dummy_lines;
    OV5640YUV_g_RES = OV5640_720P;
    SENSORDB("OV5640YUV_set_720P Start \n"); 
    
    OV5640YUV_Sensor_Init_set_720P();

    OV5640YUV_PV_pclk = 5600; 
    OV5640YUV_sensor_pclk=56000000;



//    SENSORDB("preview start may 1 change mcu\n");
//    OV5640_FOCUS_AD5820_Check_MCU();

    SENSORDB("Set 720P End\n"); 
}
void OV5640YUV_IQ(void)
{
	OV5640YUV_write_cmos_sensor(0x5180, 0xff);   
	OV5640YUV_write_cmos_sensor(0x5181, 0xf2);   
	OV5640YUV_write_cmos_sensor(0x5182, 0x0 );  
	OV5640YUV_write_cmos_sensor(0x5183, 0x14);   
	OV5640YUV_write_cmos_sensor(0x5184, 0x25);   
	OV5640YUV_write_cmos_sensor(0x5185, 0x24);   
	OV5640YUV_write_cmos_sensor(0x5186, 0x20);   
	OV5640YUV_write_cmos_sensor(0x5187, 0x16);   
	OV5640YUV_write_cmos_sensor(0x5188, 0x17);   
	OV5640YUV_write_cmos_sensor(0x5189, 0x81);   
	OV5640YUV_write_cmos_sensor(0x518a, 0x6b);   
	OV5640YUV_write_cmos_sensor(0x518b, 0xb3);   
	OV5640YUV_write_cmos_sensor(0x518c, 0x87);   
	OV5640YUV_write_cmos_sensor(0x518d, 0x3b);   
	OV5640YUV_write_cmos_sensor(0x518e, 0x35);   
	OV5640YUV_write_cmos_sensor(0x518f, 0x63);   
	OV5640YUV_write_cmos_sensor(0x5190, 0x4b);   
	OV5640YUV_write_cmos_sensor(0x5191, 0xf8);   
	OV5640YUV_write_cmos_sensor(0x5192, 0x4 );  
	OV5640YUV_write_cmos_sensor(0x5193, 0x70);   
	OV5640YUV_write_cmos_sensor(0x5194, 0xf0);   
	OV5640YUV_write_cmos_sensor(0x5195, 0xf0);   
	OV5640YUV_write_cmos_sensor(0x5196, 0x3 );  
	OV5640YUV_write_cmos_sensor(0x5197, 0x1 );  
	OV5640YUV_write_cmos_sensor(0x5198, 0x5 );  
	OV5640YUV_write_cmos_sensor(0x5199, 0xfd);   
	OV5640YUV_write_cmos_sensor(0x519a, 0x4 );  
	OV5640YUV_write_cmos_sensor(0x519b, 0x0 );  
	OV5640YUV_write_cmos_sensor(0x519c, 0x4 );  
	OV5640YUV_write_cmos_sensor(0x519d, 0xc6);   
	OV5640YUV_write_cmos_sensor(0x519e, 0x38);   
	                                        
	//CCM                                   
	OV5640YUV_write_cmos_sensor(0x5381, 0x1c);
	OV5640YUV_write_cmos_sensor(0x5382, 0x5a);
	OV5640YUV_write_cmos_sensor(0x5383, 0x06);
	OV5640YUV_write_cmos_sensor(0x5384, 0x0a);
	OV5640YUV_write_cmos_sensor(0x5385, 0x7e);
	OV5640YUV_write_cmos_sensor(0x5386, 0x88);
	OV5640YUV_write_cmos_sensor(0x5387, 0x7c);
	OV5640YUV_write_cmos_sensor(0x5388, 0x6c);
	OV5640YUV_write_cmos_sensor(0x5389, 0x10);
	OV5640YUV_write_cmos_sensor(0x538a, 0x01);
	OV5640YUV_write_cmos_sensor(0x538b, 0x98);
	                                        
	//sharpness&noise                       
	OV5640YUV_write_cmos_sensor(0x5308, 0x25);
	OV5640YUV_write_cmos_sensor(0x5300, 0x09);
	OV5640YUV_write_cmos_sensor(0x5301, 0x11);
	OV5640YUV_write_cmos_sensor(0x5302, 0x10);
	OV5640YUV_write_cmos_sensor(0x5303, 0x00);
	OV5640YUV_write_cmos_sensor(0x5304, 0x08);
	OV5640YUV_write_cmos_sensor(0x5305, 0x11);
	OV5640YUV_write_cmos_sensor(0x5306, 0x08);
	OV5640YUV_write_cmos_sensor(0x5307, 0x18);
	OV5640YUV_write_cmos_sensor(0x5309, 0x08);
	OV5640YUV_write_cmos_sensor(0x530a, 0x30);
	OV5640YUV_write_cmos_sensor(0x530b, 0x04);
	OV5640YUV_write_cmos_sensor(0x530c, 0x06);
	//GAMMA                            
	OV5640YUV_write_cmos_sensor(0x5480, 0x01); 
	OV5640YUV_write_cmos_sensor(0x5481, 0x8 );
	OV5640YUV_write_cmos_sensor(0x5482, 0x14); 
	OV5640YUV_write_cmos_sensor(0x5483, 0x28); 
	OV5640YUV_write_cmos_sensor(0x5484, 0x51);
	OV5640YUV_write_cmos_sensor(0x5485, 0x65); 
	OV5640YUV_write_cmos_sensor(0x5486, 0x71); 
	OV5640YUV_write_cmos_sensor(0x5487, 0x7e); 
	OV5640YUV_write_cmos_sensor(0x5488, 0x88); 
	OV5640YUV_write_cmos_sensor(0x5489, 0x92); 
	OV5640YUV_write_cmos_sensor(0x548a, 0x9b); 
	OV5640YUV_write_cmos_sensor(0x548b, 0xab); 
	OV5640YUV_write_cmos_sensor(0x548c, 0xba); 
	OV5640YUV_write_cmos_sensor(0x548d, 0xd0); 
	OV5640YUV_write_cmos_sensor(0x548e, 0xe1); 
	OV5640YUV_write_cmos_sensor(0x548f, 0xf3);
	OV5640YUV_write_cmos_sensor(0x5490, 0x11);
	                                
	//UV adjust                     
	OV5640YUV_write_cmos_sensor(0x5580, 0x02);
	OV5640YUV_write_cmos_sensor(0x5583, 0x40);
	OV5640YUV_write_cmos_sensor(0x5584, 0x18);
	OV5640YUV_write_cmos_sensor(0x5589, 0x12);
	OV5640YUV_write_cmos_sensor(0x558a, 0x00);
	OV5640YUV_write_cmos_sensor(0x558b, 0x58);
	//blc                                   
	OV5640YUV_write_cmos_sensor(0x4005, 0x1a);
	                                        
	//OV5640 LENC setting
	OV5640YUV_write_cmos_sensor(0x5800, 0x28);
	OV5640YUV_write_cmos_sensor(0x5801, 0x13);
	OV5640YUV_write_cmos_sensor(0x5802, 0x10);
	OV5640YUV_write_cmos_sensor(0x5803, 0xf );
	OV5640YUV_write_cmos_sensor(0x5804, 0x12);
	OV5640YUV_write_cmos_sensor(0x5805, 0x20);
	OV5640YUV_write_cmos_sensor(0x5806, 0xa );
	OV5640YUV_write_cmos_sensor(0x5807, 0x6 );
	OV5640YUV_write_cmos_sensor(0x5808, 0x6 );
	OV5640YUV_write_cmos_sensor(0x5809, 0x6 );
	OV5640YUV_write_cmos_sensor(0x580a, 0x5 );
	OV5640YUV_write_cmos_sensor(0x580b, 0xa );
	OV5640YUV_write_cmos_sensor(0x580c, 0x8 );
	OV5640YUV_write_cmos_sensor(0x580d, 0x4 );
	OV5640YUV_write_cmos_sensor(0x580e, 0x0 );
	OV5640YUV_write_cmos_sensor(0x580f, 0x0 );
	OV5640YUV_write_cmos_sensor(0x5810, 0x3 );
	OV5640YUV_write_cmos_sensor(0x5811, 0x8 );
	OV5640YUV_write_cmos_sensor(0x5812, 0x7 );
	OV5640YUV_write_cmos_sensor(0x5813, 0x3 );
	OV5640YUV_write_cmos_sensor(0x5814, 0x0 );
	OV5640YUV_write_cmos_sensor(0x5815, 0x0 );
	OV5640YUV_write_cmos_sensor(0x5816, 0x3 );
	OV5640YUV_write_cmos_sensor(0x5817, 0x8 );
	OV5640YUV_write_cmos_sensor(0x5818, 0xa );
	OV5640YUV_write_cmos_sensor(0x5819, 0x7 );
	OV5640YUV_write_cmos_sensor(0x581a, 0x5 );
	OV5640YUV_write_cmos_sensor(0x581b, 0x5 );
	OV5640YUV_write_cmos_sensor(0x581c, 0x5 );
	OV5640YUV_write_cmos_sensor(0x581d, 0xa );
	OV5640YUV_write_cmos_sensor(0x581e, 0x1d);
	OV5640YUV_write_cmos_sensor(0x581f, 0xd );
	OV5640YUV_write_cmos_sensor(0x5820, 0xc );
	OV5640YUV_write_cmos_sensor(0x5821, 0xc );
	OV5640YUV_write_cmos_sensor(0x5822, 0xe );
	OV5640YUV_write_cmos_sensor(0x5823, 0x1a);
	OV5640YUV_write_cmos_sensor(0x5824, 0x28);
	OV5640YUV_write_cmos_sensor(0x5825, 0x28);
	OV5640YUV_write_cmos_sensor(0x5826, 0x2a);
	OV5640YUV_write_cmos_sensor(0x5827, 0x28);
	OV5640YUV_write_cmos_sensor(0x5828, 0x28);
	OV5640YUV_write_cmos_sensor(0x5829, 0x2c);
	OV5640YUV_write_cmos_sensor(0x582a, 0x28);
	OV5640YUV_write_cmos_sensor(0x582b, 0x26);
	OV5640YUV_write_cmos_sensor(0x582c, 0x26);
	OV5640YUV_write_cmos_sensor(0x582d, 0x2a);
	OV5640YUV_write_cmos_sensor(0x582e, 0x2a);
	OV5640YUV_write_cmos_sensor(0x582f, 0x24);
	OV5640YUV_write_cmos_sensor(0x5830, 0x40);
	OV5640YUV_write_cmos_sensor(0x5831, 0x42);
	OV5640YUV_write_cmos_sensor(0x5832, 0xa );
	OV5640YUV_write_cmos_sensor(0x5833, 0x2c);
	OV5640YUV_write_cmos_sensor(0x5834, 0x28);
	OV5640YUV_write_cmos_sensor(0x5835, 0x26);
	OV5640YUV_write_cmos_sensor(0x5836, 0x26);
	OV5640YUV_write_cmos_sensor(0x5837, 0xe );
	OV5640YUV_write_cmos_sensor(0x5838, 0x2e);
	OV5640YUV_write_cmos_sensor(0x5839, 0x2c);
	OV5640YUV_write_cmos_sensor(0x583a, 0x2e);
	OV5640YUV_write_cmos_sensor(0x583b, 0x2c);
	OV5640YUV_write_cmos_sensor(0x583c, 0x2c);
	OV5640YUV_write_cmos_sensor(0x583d, 0xce);
}

void OV5640YUV_set_5M_init(void)
{
	OV5640YUV_write_cmos_sensor(0x3820, 0x40); 
	OV5640YUV_write_cmos_sensor(0x3821, 0x06); 
	OV5640YUV_write_cmos_sensor(0x3814, 0x11); 
	OV5640YUV_write_cmos_sensor(0x3815, 0x11); 
	OV5640YUV_write_cmos_sensor(0x3803, 0x00); 
	OV5640YUV_write_cmos_sensor(0x3807, 0x9f); 
	OV5640YUV_write_cmos_sensor(0x3808, 0x0a); 
	OV5640YUV_write_cmos_sensor(0x3809, 0x20); 
	OV5640YUV_write_cmos_sensor(0x380a, 0x07); 
	OV5640YUV_write_cmos_sensor(0x380b, 0x98); 
	OV5640YUV_write_cmos_sensor(0x380c, 0x0b); 
	OV5640YUV_write_cmos_sensor(0x380d, 0x1c); 
	OV5640YUV_write_cmos_sensor(0x380e, 0x07); 
	OV5640YUV_write_cmos_sensor(0x380f, 0xb0); 
	OV5640YUV_write_cmos_sensor(0x3811, 0x10); //
	OV5640YUV_write_cmos_sensor(0x3813, 0x04); 
	OV5640YUV_write_cmos_sensor(0x3618, 0x04); 
	OV5640YUV_write_cmos_sensor(0x3612, 0x2b); //4b 
	OV5640YUV_write_cmos_sensor(0x3708, 0x64);
	OV5640YUV_write_cmos_sensor(0x3709, 0x12); 
	OV5640YUV_write_cmos_sensor(0x370c, 0x00); 
	OV5640YUV_write_cmos_sensor(0x3a02, 0x07); 
	OV5640YUV_write_cmos_sensor(0x3a03, 0xb0); 
	OV5640YUV_write_cmos_sensor(0x3a0e, 0x06); 
	OV5640YUV_write_cmos_sensor(0x3a0d, 0x08); 
	OV5640YUV_write_cmos_sensor(0x3a14, 0x07); 
	OV5640YUV_write_cmos_sensor(0x3a15, 0xb0); 
	OV5640YUV_write_cmos_sensor(0x4004, 0x06);
	OV5640YUV_write_cmos_sensor(0x5000, 0xa7); 
	OV5640YUV_write_cmos_sensor(0x5001, 0x83);
	OV5640YUV_write_cmos_sensor(0x519e, 0x38);
	OV5640YUV_write_cmos_sensor(0x5381, 0x1e);
	OV5640YUV_write_cmos_sensor(0x5382, 0x5b);
	OV5640YUV_write_cmos_sensor(0x5383, 0x08);
	OV5640YUV_write_cmos_sensor(0x460b, 0x37); 
	OV5640YUV_write_cmos_sensor(0x460c, 0x20); 
	OV5640YUV_write_cmos_sensor(0x3824, 0x01); 
	OV5640YUV_write_cmos_sensor(0x4005, 0x1A); 

}
void OV5640YUV_set_5M(void)
{
    SENSORDB("Set 5M begin\n"); 
    OV5640YUV_g_RES = OV5640_5M;

    OV5640YUV_set_5M_init();

    //OV5640YUV_set_AE_mode(KAL_FALSE);
    //OV5640YUV_set_AWB_mode(KAL_FALSE);

    //OV5640_Sensor_Total_5M();	
    OV5640YUV_PV_pclk = 5600; 
    OV5640YUV_sensor_pclk=56000000;
    SENSORDB("Set 5M End\n");  
}

void OV5640YUV_dump_5M(void)
{
}

/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*   OV5640YUVOpen
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/

UINT32 OV5640YUVOpen(void)
{
    int  retry = 0; 

    OV5640YUV_sensor_id = ((OV5640YUV_read_cmos_sensor(0x300A) << 8) | OV5640YUV_read_cmos_sensor(0x300B));
    // check if sensor ID correct
    retry = 3; 
    do {
        OV5640YUV_sensor_id = ((OV5640YUV_read_cmos_sensor(0x300A) << 8) | OV5640YUV_read_cmos_sensor(0x300B));        
        if (OV5640YUV_sensor_id == OV5640_SENSOR_ID)
            break; 
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", OV5640YUV_sensor_id); 
        retry--; 
    } while (retry > 0);

//    if (OV5640YUV_sensor_id != OV5640_SENSOR_ID)
//        return ERROR_SENSOR_CONNECT_FAIL;

    OV5640YUV_Sensor_Init();


    OV5640YUV_sensor_gain_base = read_OV5640YUV_gain();
    
    OV5640YUV_g_iBackupExtraExp = 0;
    atomic_set(&OV5640_SetGain_Flag, 0); 
    atomic_set(&OV5640_SetShutter_Flag, 0); 
    init_waitqueue_head(&OV5640_SetGain_waitQueue);
    init_waitqueue_head(&OV5640_SetShutter_waitQueue); 
    return ERROR_NONE;
}



/*************************************************************************
* FUNCTION
*   OV5640YUV_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of OV5640 to change exposure time.
*
* PARAMETERS
*   shutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV5640YUV_SetShutter(kal_uint16 iShutter)
{
#if 0 
    if (iShutter < 4 )
        iShutter = 4;
#else 
    if (iShutter < 1)
        iShutter = 1; 
#endif     

    OV5640YUV_pv_exposure_lines = iShutter;

    //OV5640YUV_imgSensorProfileStart();
    OV5640YUV_write_shutter(iShutter);
    //OV5640YUV_imgSensorProfileEnd("OV5640YUV_SetShutter"); 
    //SENSORDB("iShutter = %d\n", iShutter);

}   /*  OV5640YUV_SetShutter   */



/*************************************************************************
* FUNCTION
*   OV5640YUV_read_shutter
*
* DESCRIPTION
*   This function to  Get exposure time.
*
* PARAMETERS
*   None
*
* RETURNS
*   shutter : exposured lines
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT16 OV5640YUV_read_shutter(void)
{
    kal_uint8 temp_reg1, temp_reg2, temp_reg3;
    kal_uint16 temp_reg;
    temp_reg1 = OV5640YUV_read_cmos_sensor(0x3500);
    temp_reg2 = OV5640YUV_read_cmos_sensor(0x3501);
    temp_reg3 = OV5640YUV_read_cmos_sensor(0x3502);

   // SENSORDB("ov5640read shutter 0x3500=0x%x,0x3501=0x%x,0x3502=0x%x\n",
	//	temp_reg1,temp_reg2,temp_reg3);
    temp_reg = ((temp_reg1<<12) & 0xF000) | ((temp_reg2<<4) & 0x0FF0) | ((temp_reg3>>4) & 0x0F);

    //SENSORDB("ov5640read shutter = 0x%x\n", temp_reg);
	
    return (UINT16)temp_reg;
}

/*************************************************************************
* FUNCTION
*   OV5640_night_mode
*
* DESCRIPTION
*   This function night mode of OV5640.
*
* PARAMETERS
*   none
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV5640YUV_NightMode(kal_bool bEnable)
{
    if(bEnable)
    {
        if(OV5640YUV_MPEG4_encode_mode==KAL_TRUE)
        {
		OV5640YUV_write_cmos_sensor(0x3a02 ,0x03);
		OV5640YUV_write_cmos_sensor(0x3a03 ,0xd8);
		OV5640YUV_write_cmos_sensor(0x3a14 ,0x03);
		OV5640YUV_write_cmos_sensor(0x3a15 ,0xd8);
		OV5640YUV_write_cmos_sensor(0x3a00, 0x38);
		OV5640YUV_write_cmos_sensor(0x3a19, 0xb0);
        }
        else
        {
		OV5640YUV_write_cmos_sensor(0x3a02 ,0x08);
		OV5640YUV_write_cmos_sensor(0x3a03 ,0xa6);
		OV5640YUV_write_cmos_sensor(0x3a14 ,0x0a);
		OV5640YUV_write_cmos_sensor(0x3a15 ,0x68);
		OV5640YUV_write_cmos_sensor(0x3a00, 0x3c);
		OV5640YUV_write_cmos_sensor(0x3a19, 0xb0);
        }
    }
    else
    {
        if(OV5640YUV_MPEG4_encode_mode==KAL_TRUE)
        {
		OV5640YUV_write_cmos_sensor(0x3a02 ,0x03);
		OV5640YUV_write_cmos_sensor(0x3a03 ,0xd8);
		OV5640YUV_write_cmos_sensor(0x3a14 ,0x03);
		OV5640YUV_write_cmos_sensor(0x3a15 ,0xd8);
		OV5640YUV_write_cmos_sensor(0x3a00, 0x38);
		OV5640YUV_write_cmos_sensor(0x3a19, 0xb0);
        }
        else
        {
		OV5640YUV_write_cmos_sensor(0x3a02 ,0x05);
		OV5640YUV_write_cmos_sensor(0x3a03 ,0xc4);
		OV5640YUV_write_cmos_sensor(0x3a14 ,0x06);
		OV5640YUV_write_cmos_sensor(0x3a15 ,0xF0);
		OV5640YUV_write_cmos_sensor(0x3a00, 0x3c);
		OV5640YUV_write_cmos_sensor(0x3a19, 0xb0);
        }

    }
    
}

/*************************************************************************
* FUNCTION
*   OV5640YUVClose
*
* DESCRIPTION
*   This function is to turn off sensor module power.
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV5640YUVClose(void)
{
    //  CISModulePowerOn(FALSE);

    //s_porting
    //  DRV_I2CClose(OV5640YUVhDrvI2C);
    //e_porting
    return ERROR_NONE;
}	/* OV5640YUVClose() */

/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_AD5820_Init
*
* DESCRIPTION
*   This function is to load micro code for AF function
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/



void OV5640YUV_Set_Mirror_Flip(kal_uint8 image_mirror)
{
	kal_uint16 iMirror, iFlip;
	iMirror = OV5640YUV_read_cmos_sensor(0x3820);
	iFlip =OV5640YUV_read_cmos_sensor(0x3821);
       switch (image_mirror)
	{
	    case IMAGE_NORMAL:
	      OV5640YUV_write_cmos_sensor(0x3820,iMirror&0xf9);	//Set normal
             OV5640YUV_write_cmos_sensor(0x3821,iFlip|0x06);

              //SET_FIRST_GRAB_COLOR(BAYER_Gr);
	      break;				
	    case IMAGE_HV_MIRROR:
		OV5640YUV_write_cmos_sensor(0x3820,iMirror|0x06);
		OV5640YUV_write_cmos_sensor(0x3821,iFlip&0xf9);

		    //SET_FIRST_GRAB_COLOR(BAYER_B);
	        break;
    }
}

/*************************************************************************
* FUNCTION
*   OV5640YUVPreview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV5640YUVPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;
    g_iOV5640YUV_Mode = OV5640_MODE_PREVIEW;

    //if(OV5640_720P == OV5640YUV_g_RES)
    {
        OV5640YUV_set_720P();
    }

    if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        OV5640YUV_MPEG4_encode_mode = KAL_TRUE;
    }
    else
    {
        OV5640YUV_MPEG4_encode_mode = KAL_FALSE;
    }

    iStartX += OV5640_IMAGE_SENSOR_PV_STARTX;
    iStartY += OV5640_IMAGE_SENSOR_PV_STARTY;
    sensor_config_data->SensorImageMirror = IMAGE_HV_MIRROR; 
    
    OV5640YUV_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

#if 0    
    iTemp = OV5640YUV_read_cmos_sensor(0x3818) & 0x9f;	//Clear the mirror and flip bits.
    switch (sensor_config_data->SensorImageMirror)
    {
        case IMAGE_NORMAL:
            OV5640YUV_write_cmos_sensor(0x3818, iTemp | 0xc0);	//Set normal
            OV5640YUV_write_cmos_sensor(0x3621, 0xc7);
            //SET_FIRST_GRAB_COLOR(BAYER_Gr);
            break;

        case IMAGE_V_MIRROR:
            OV5640YUV_write_cmos_sensor(0x3818, iTemp | 0xe0);	//Set flip
            OV5640YUV_write_cmos_sensor(0x3621, 0xc7);

            //SET_FIRST_GRAB_COLOR(BAYER_B);
            break;

        case IMAGE_H_MIRROR:
            OV5640YUV_write_cmos_sensor(0x3818, iTemp | 0x80);	//Set mirror
            OV5640YUV_write_cmos_sensor(0x3621, 0xe7);
            //SET_FIRST_GRAB_COLOR(BAYER_Gr);
        break;

        case IMAGE_HV_MIRROR:
            OV5640YUV_write_cmos_sensor(0x3818, iTemp | 0xa0);	//Set mirror and flip
            OV5640YUV_write_cmos_sensor(0x3621, 0xe7);
            //SET_FIRST_GRAB_COLOR(BAYER_B);
            break;
    }
#endif     

    OV5640YUV_dummy_pixels = 0;
    OV5640YUV_dummy_lines = 0;
    OV5640YUV_PV_dummy_pixels = OV5640YUV_dummy_pixels;
    OV5640YUV_PV_dummy_lines = OV5640YUV_dummy_lines;

    OV5640YUV_SetDummy(OV5640YUV_dummy_pixels, OV5640YUV_dummy_lines);
    //OV5640YUV_SetShutter(OV5640YUV_pv_exposure_lines);

    memcpy(&OV5640YUVSensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    image_window->GrabStartX= iStartX;
    image_window->GrabStartY= iStartY;
    image_window->ExposureWindowWidth= OV5640_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
    image_window->ExposureWindowHeight= OV5640_IMAGE_SENSOR_PV_HEIGHT - 2*iStartY;
    return ERROR_NONE;
}	/* OV5640YUVPreview() */


UINT32 OV5640YUVCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    //kal_uint32 shutter=OV5640YUV_pv_exposure_lines;
    kal_uint32 shutter = 0;
    kal_uint32 temp_shutter = 0;	
    kal_uint16 iStartX = 0, iStartY = 0;
    kal_uint32 pv_gain = 0;	
    kal_uint8 temp = 0;//for night mode	

   
    SENSORDB("Preview Shutter = %d, Gain = %d\n", shutter, read_OV5640YUV_gain());    

    g_iOV5640YUV_Mode = OV5640_MODE_CAPTURE;

    if(sensor_config_data->EnableShutterTansfer==KAL_TRUE)
        shutter=sensor_config_data->CaptureShutter;


//1. disable night mode
    temp= OV5640YUV_read_cmos_sensor(0x3a00);
    OV5640YUV_write_cmos_sensor(0x3a00,temp&0xfb);
//2. disable AE,AWB
    OV5640YUV_set_AE_mode(KAL_FALSE);
    OV5640YUV_set_AWB_mode(KAL_FALSE);

//3. read shutter, gain
    shutter = OV5640YUV_read_shutter();
    pv_gain = read_OV5640YUV_gain();
//4. set 5M mode

    if ((image_window->ImageTargetWidth<= OV5640_IMAGE_SENSOR_PV_WIDTH) &&
        (image_window->ImageTargetHeight<= OV5640_IMAGE_SENSOR_PV_HEIGHT)) {
        OV5640YUV_dummy_pixels= 0;
        OV5640YUV_dummy_lines = 0;

        //shutter = ((UINT32)(shutter*(OV5640_PV_PERIOD_PIXEL_NUMS_HTS  + OV5640YUV_PV_dummy_pixels)))/
        //                                                ((OV5640_FULL_PERIOD_PIXEL_NUMS_HTS + OV5640YUV_dummy_pixels)) ;
        //shutter = shutter * OV5640YUV_CAP_pclk / OV5640YUV_PV_pclk; 
        
        iStartX = OV5640_IMAGE_SENSOR_PV_STARTX;
        iStartY = OV5640_IMAGE_SENSOR_PV_STARTY;
        image_window->GrabStartX=iStartX;
        image_window->GrabStartY=iStartY;
        image_window->ExposureWindowWidth=OV5640_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
        image_window->ExposureWindowHeight=OV5640_IMAGE_SENSOR_PV_HEIGHT- 2*iStartY;
    }
    else { // 5M  Mode
        OV5640YUV_dummy_pixels= 0;
        OV5640YUV_dummy_lines = 0;        
        OV5640YUV_set_5M();
//        sensor_config_data->SensorImageMirror = IMAGE_HV_MIRROR;      
//        OV5640SetFlipMirror(sensor_config_data->SensorImageMirror); 
        sensor_config_data->SensorImageMirror = IMAGE_HV_MIRROR; 
        OV5640YUV_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

 	//OV5640YUV_CAP_pclk = 5600;
 	//OV5640YUV_PV_pclk = 5600;		
	//To avoid overflow...
        OV5640YUV_CAP_pclk = 560;
        OV5640YUV_PV_pclk = 560;		
        temp_shutter = (shutter*(OV5640_PV_PERIOD_PIXEL_NUMS_HTS+OV5640YUV_PV_dummy_pixels)*OV5640YUV_CAP_pclk)
        			/(OV5640_FULL_PERIOD_PIXEL_NUMS_HTS+OV5640YUV_dummy_pixels)/OV5640YUV_PV_pclk;	
        
        shutter = (kal_uint32)(temp_shutter);
        SENSORDB("cap shutter calutaed = %d, 0x%x\n", shutter,shutter);
		//shutter = shutter*2; 
        //SVGA Internal CLK = 1/4 UXGA Internal CLK
        //shutter = 4* shutter;
       // shutter = ((UINT32)(shutter*(OV5640_IMAGE_SENSOR_720P_PIXELS_LINE + OV5640_PV_PERIOD_EXTRA_PIXEL_NUMS + OV5640YUV_PV_dummy_pixels)))/
        //                                                ((OV5640_IMAGE_SENSOR_5M_PIXELS_LINE+ OV5640_FULL_PERIOD_EXTRA_PIXEL_NUMS + OV5640YUV_dummy_pixels)) ;
        //shutter = shutter * OV5640YUV_CAP_pclk / OV5640YUV_PV_pclk; 
        iStartX = 2* OV5640_IMAGE_SENSOR_PV_STARTX;
        iStartY = 2* OV5640_IMAGE_SENSOR_PV_STARTY;

        image_window->GrabStartX=iStartX;
        image_window->GrabStartY=iStartY;
        image_window->ExposureWindowWidth=OV5640_IMAGE_SENSOR_FULL_WIDTH -2*iStartX;
        image_window->ExposureWindowHeight=OV5640_IMAGE_SENSOR_FULL_HEIGHT-2*iStartY;
    }//5M Capture
    // config flashlight preview setting
    if(OV5640_5M == OV5640YUV_g_RES) //add start
    {
        sensor_config_data->DefaultPclk = 32500000;
        sensor_config_data->Pixels = OV5640_IMAGE_SENSOR_5M_PIXELS_LINE + OV5640YUV_PV_dummy_pixels;
        sensor_config_data->FrameLines =OV5640_PV_PERIOD_LINE_NUMS+OV5640YUV_PV_dummy_lines;
    }
    else
    {
        sensor_config_data->DefaultPclk = 32500000;
        sensor_config_data->Pixels = OV5640_IMAGE_SENSOR_5M_PIXELS_LINE+OV5640YUV_dummy_pixels;
        sensor_config_data->FrameLines =OV5640_FULL_PERIOD_LINE_NUMS+OV5640YUV_dummy_lines;
    }

    sensor_config_data->Lines = image_window->ExposureWindowHeight;
    sensor_config_data->Shutter =shutter;

    OV5640YUV_SetDummy(OV5640YUV_dummy_pixels, OV5640YUV_dummy_lines);

//6.set shutter
    OV5640YUV_SetShutter(shutter);    
//7.set gain
    write_OV5640YUV_gain(pv_gain);	
//aec, awb is close?
    //SENSORDB("aec reg0x3503=0x%x, awb reg0x3406=0x%x\n",
    //OV5640YUV_read_cmos_sensor(0x3503),
    //OV5640YUV_read_cmos_sensor(0x3406));
    //SENSORDB("Capture Shutter = %d, Gain = %d\n", shutter, read_OV5640YUV_gain());     
    memcpy(&OV5640YUVSensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}	/* OV5640YUVCapture() */

UINT32 OV5640YUVGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH - 4*OV5640_IMAGE_SENSOR_PV_STARTX;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT - 4*OV5640_IMAGE_SENSOR_PV_STARTY;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH - 2*OV5640_IMAGE_SENSOR_PV_STARTX;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT - 2*OV5640_IMAGE_SENSOR_PV_STARTY;
    pSensorResolution->SensorVideoWidth=IMAGE_SENSOR_PV_WIDTH - 2*OV5640_IMAGE_SENSOR_PV_STARTX;
    pSensorResolution->SensorVideoHeight=IMAGE_SENSOR_PV_HEIGHT - 2*OV5640_IMAGE_SENSOR_PV_STARTY;

    return ERROR_NONE;
}   /* OV5640YUVGetResolution() */
UINT32 OV5640YUVGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH - 2*OV5640_IMAGE_SENSOR_PV_STARTX;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT - 2*OV5640_IMAGE_SENSOR_PV_STARTY;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH - 4*OV5640_IMAGE_SENSOR_PV_STARTX;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_HEIGHT - 4*OV5640_IMAGE_SENSOR_PV_STARTY;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;//SENSOR_OUTPUT_FORMAT_UYVY;
	
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW; /*??? */
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;


    //pSensorInfo->CaptureDelayFrame = 1; 
    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 2; 
    pSensorInfo->VideoDelayFrame = 3; 
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;      

	   
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        default:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=3;
            pSensorInfo->SensorClockRisingCount=0;
            pSensorInfo->SensorClockFallingCount=2;
            pSensorInfo->SensorPixelClockCount=3;
            pSensorInfo->SensorDataLatchCount=2;
            pSensorInfo->SensorGrabStartX = 4;
            pSensorInfo->SensorGrabStartY = 3;
            break;
    }

    //OV5640YUVPixelClockDivider=pSensorInfo->SensorPixelClockCount;
    //memcpy(pSensorConfigData, &OV5640YUVSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* OV5640YUVGetInfo() */


UINT32 OV5640YUVControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            OV5640YUVPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
            OV5640YUVCapture(pImageWindow, pSensorConfigData);
            break;
        default:
            break;
    }
    return TRUE;
} /* OV5640YUVControl() */


UINT32 OV5640YUVSetVideoMode(UINT16 u2FrameRate)
{
    return TRUE;
}
kal_uint32 OV5640_set_param_wb(kal_uint32 para)
{

    switch (para)
    {
        case AWB_MODE_AUTO:
		OV5640YUV_write_cmos_sensor(0x3212, 0x03);
		OV5640YUV_write_cmos_sensor(0x3406, 0x00);
		OV5640YUV_write_cmos_sensor(0x3400, 0x04);
		OV5640YUV_write_cmos_sensor(0x3401, 0x00);
		OV5640YUV_write_cmos_sensor(0x3402, 0x04);
		OV5640YUV_write_cmos_sensor(0x3403, 0x00);
		OV5640YUV_write_cmos_sensor(0x3404, 0x04);
		OV5640YUV_write_cmos_sensor(0x3405, 0x00);
		OV5640YUV_write_cmos_sensor(0x3212, 0x13);
		OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
          break;

        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy           
              OV5640YUV_write_cmos_sensor(0x3212, 0x03);
		OV5640YUV_write_cmos_sensor(0x3406, 0x01);
		OV5640YUV_write_cmos_sensor(0x3400, 0x06);
		OV5640YUV_write_cmos_sensor(0x3401, 0x48);
		OV5640YUV_write_cmos_sensor(0x3402, 0x04);
		OV5640YUV_write_cmos_sensor(0x3403, 0x00);
		OV5640YUV_write_cmos_sensor(0x3404, 0x04);
		OV5640YUV_write_cmos_sensor(0x3405, 0xd3);
		OV5640YUV_write_cmos_sensor(0x3212, 0x13);
		OV5640YUV_write_cmos_sensor(0x3212, 0xa3);            
          break;

        case AWB_MODE_DAYLIGHT: //sunny            
              OV5640YUV_write_cmos_sensor(0x3212, 0x03);
		OV5640YUV_write_cmos_sensor(0x3406, 0x01);
		OV5640YUV_write_cmos_sensor(0x3400, 0x06);
		OV5640YUV_write_cmos_sensor(0x3401, 0x1c);
		OV5640YUV_write_cmos_sensor(0x3402, 0x04);
		OV5640YUV_write_cmos_sensor(0x3403, 0x00);
		OV5640YUV_write_cmos_sensor(0x3404, 0x04);
		OV5640YUV_write_cmos_sensor(0x3405, 0xf3);
		OV5640YUV_write_cmos_sensor(0x3212, 0x13);
		OV5640YUV_write_cmos_sensor(0x3212, 0xa3);            
          break;

        case AWB_MODE_INCANDESCENT: //office
		OV5640YUV_write_cmos_sensor(0x3212, 0x03);
		OV5640YUV_write_cmos_sensor(0x3406, 0x01);
		OV5640YUV_write_cmos_sensor(0x3400, 0x05);
		OV5640YUV_write_cmos_sensor(0x3401, 0x48);
		OV5640YUV_write_cmos_sensor(0x3402, 0x04);
		OV5640YUV_write_cmos_sensor(0x3403, 0x00);
		OV5640YUV_write_cmos_sensor(0x3404, 0x07);
		OV5640YUV_write_cmos_sensor(0x3405, 0xcf);
		OV5640YUV_write_cmos_sensor(0x3212, 0x13);
		OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
          break;

        case AWB_MODE_TUNGSTEN: //home
		OV5640YUV_write_cmos_sensor(0x3212, 0x03);
		OV5640YUV_write_cmos_sensor(0x3406, 0x01);
		OV5640YUV_write_cmos_sensor(0x3400, 0x04);
		OV5640YUV_write_cmos_sensor(0x3401, 0x10);
		OV5640YUV_write_cmos_sensor(0x3402, 0x04);
		OV5640YUV_write_cmos_sensor(0x3403, 0x00);
		OV5640YUV_write_cmos_sensor(0x3404, 0x08);
		OV5640YUV_write_cmos_sensor(0x3405, 0x40);
		OV5640YUV_write_cmos_sensor(0x3212, 0x13);
		OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
          break;

        case AWB_MODE_FLUORESCENT:            
              OV5640YUV_write_cmos_sensor(0x3212, 0x03);
		OV5640YUV_write_cmos_sensor(0x3406, 0x01);
		OV5640YUV_write_cmos_sensor(0x3400, 0x05);
		OV5640YUV_write_cmos_sensor(0x3401, 0x8c);
		OV5640YUV_write_cmos_sensor(0x3402, 0x04);
		OV5640YUV_write_cmos_sensor(0x3403, 0x00);
		OV5640YUV_write_cmos_sensor(0x3404, 0x06);
		OV5640YUV_write_cmos_sensor(0x3405, 0xe8);
		OV5640YUV_write_cmos_sensor(0x3212, 0x13);
		OV5640YUV_write_cmos_sensor(0x3212, 0xa3);          
          break;
        default:
            return KAL_FALSE;
    }

    return KAL_TRUE;
} 

kal_uint32 OV5640_set_param_exposure(kal_uint32 para)
{
    switch (para)
    {
        case AE_EV_COMP_n10:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x20);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x18);
		OV5640YUV_write_cmos_sensor(0x3a11, 0x40);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x20);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x18);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x10);
        break;

        case AE_EV_COMP_n07:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x28);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x20);
		OV5640YUV_write_cmos_sensor(0x3a11, 0x40);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x28);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x20);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x10);
        break;

        case AE_EV_COMP_n03:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x30);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x28);
		OV5640YUV_write_cmos_sensor(0x3a11, 0x61);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x30);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x28);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x10);
        break;

        case AE_EV_COMP_00:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x38);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x30);
		OV5640YUV_write_cmos_sensor(0x3a11, 0x61);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x38);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x30);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x10);
        break;

        case AE_EV_COMP_03:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x40);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x38);
		OV5640YUV_write_cmos_sensor(0x3a11, 0x71);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x40);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x38);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x10);
        break;

        case AE_EV_COMP_07:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x50);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x48);
		OV5640YUV_write_cmos_sensor(0x3a11, 0x90);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x50);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x48);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x20);
        break;

        case AE_EV_COMP_10:
		OV5640YUV_write_cmos_sensor(0x3a0f, 0x60);
		OV5640YUV_write_cmos_sensor(0x3a10, 0x58);
		OV5640YUV_write_cmos_sensor(0x3a11, 0xa0);
		OV5640YUV_write_cmos_sensor(0x3a1b, 0x60);
		OV5640YUV_write_cmos_sensor(0x3a1e, 0x58);
		OV5640YUV_write_cmos_sensor(0x3a1f, 0x20);
        break;

        default:
            return KAL_FALSE;
    }
    return KAL_TRUE;
} 

kal_uint32 OV5640_set_param_effect(kal_uint32 para)
{
    kal_uint32 ret = KAL_TRUE;

    switch (para)
    {
        case MEFFECT_OFF:            
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x06);
			OV5640YUV_write_cmos_sensor(0x5583, 0x40);
			OV5640YUV_write_cmos_sensor(0x5584, 0x10);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
            break;

	 case MEFFECT_MONO:
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x1e);
			OV5640YUV_write_cmos_sensor(0x5583, 0x80);
			OV5640YUV_write_cmos_sensor(0x5584, 0x80);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
            break;
			
        case MEFFECT_SEPIA:         
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x1e);
			OV5640YUV_write_cmos_sensor(0x5583, 0x40);
			OV5640YUV_write_cmos_sensor(0x5584, 0xa0);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);           
            break;

        case MEFFECT_NEGATIVE:
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x40);
			OV5640YUV_write_cmos_sensor(0x5583, 0x40);
			OV5640YUV_write_cmos_sensor(0x5584, 0x10);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
            break;

        case MEFFECT_SEPIAGREEN:           
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x1e);
			OV5640YUV_write_cmos_sensor(0x5583, 0x60);
			OV5640YUV_write_cmos_sensor(0x5584, 0x60);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);           
            break;
 /*           
         case CAM_EFFECT_ENC_REDDISH:           
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x1e);
			OV5640YUV_write_cmos_sensor(0x5583, 0x80);
			OV5640YUV_write_cmos_sensor(0x5584, 0xc0);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);           
            break;
*/
        case MEFFECT_SEPIABLUE:           
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x1e);
			OV5640YUV_write_cmos_sensor(0x5583, 0xa0);
			OV5640YUV_write_cmos_sensor(0x5584, 0x40);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);                      
            break;
       /*     
        case CAM_EFFECT_ENC_GRAYSCALE:     
			OV5640YUV_write_cmos_sensor(0x3212, 0x03);
			OV5640YUV_write_cmos_sensor(0x5580, 0x1e);
			OV5640YUV_write_cmos_sensor(0x5583, 0x80);
			OV5640YUV_write_cmos_sensor(0x5584, 0x80);
			OV5640YUV_write_cmos_sensor(0x5003, 0x08);
			OV5640YUV_write_cmos_sensor(0x3212, 0x13);
			OV5640YUV_write_cmos_sensor(0x3212, 0xa3);
        	break;
        */
/*
        case CAM_EFFECT_ENC_GRAYINV:
        case CAM_EFFECT_ENC_COPPERCARVING:
        case CAM_EFFECT_ENC_BLUECARVING:
        case CAM_EFFECT_ENC_CONTRAST:
        case CAM_EFFECT_ENC_EMBOSSMENT:
        case CAM_EFFECT_ENC_SKETCH:
        case CAM_EFFECT_ENC_BLACKBOARD:
        case CAM_EFFECT_ENC_WHITEBOARD:
        case CAM_EFFECT_ENC_JEAN:
        case CAM_EFFECT_ENC_OIL:
*/
        default:
            ret = KAL_FALSE;
	return ret;
    }

    return KAL_TRUE;
} 

kal_uint32 OV5640_set_param_banding(kal_uint32 para)
{
    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
			OV5640YUV_write_cmos_sensor(0x3a09,0x94); 
			OV5640YUV_write_cmos_sensor(0x3a0e,0x06);
			OV5640YUV_write_cmos_sensor(0x3c00,0x04);
			OV5640YUV_write_cmos_sensor(0x3c01,0x80);
		break;

        case AE_FLICKER_MODE_60HZ:
			OV5640YUV_write_cmos_sensor(0x3a0b, 0x7b); 
			OV5640YUV_write_cmos_sensor(0x3a0d, 0x08);
			OV5640YUV_write_cmos_sensor(0x3c00,0x00);
			OV5640YUV_write_cmos_sensor(0x3c01,0x80);
		break;

	default:
	  return KAL_FALSE;
    }
    return KAL_TRUE;
} 

UINT32 OV5640YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
	printk("\n OV5640YUVSensorSetting() is called ; \n");
	SENSORDB("cmd=%d, para = 0x%x\n", iCmd, iPara);

	switch (iCmd) {
	case FID_SCENE_MODE:	    
	    if (iPara == SCENE_MODE_OFF)
	    {
	        OV5640YUV_NightMode(FALSE); 
	    }
	    else if (iPara == SCENE_MODE_NIGHTSCENE)
	    {
			OV5640YUV_NightMode(TRUE); 
	    }	    
	break; 	    
	case FID_AWB_MODE:
		OV5640_set_param_wb(iPara);
	break;
	case FID_COLOR_EFFECT:
		OV5640_set_param_effect(iPara);
	break;
	case FID_AE_EV:
		OV5640_set_param_exposure(iPara);
	break;
	case FID_AE_FLICKER:
		OV5640_set_param_banding(iPara);
	break;
	case FID_ZOOM_FACTOR:
        OV5640YUV_zoom_factor = iPara; 		
	break;
		default:
	break;
    }

    return TRUE;

}

/*************************************************************************
* FUNCTION
*   OV5640YUVGetSensorID
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
UINT32 OV5640YUVGetSensorID(UINT32 *sensorID) 
{
    int  retry = 3; 
    
    // check if sensor ID correct
    do {
        *sensorID = ((OV5640YUV_read_cmos_sensor(0x300A) << 8) | OV5640YUV_read_cmos_sensor(0x300B));        
        if (*sensorID == OV5640_SENSOR_ID)
            break; 
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
    } while (retry > 0);

    if (*sensorID != OV5640_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}
UINT32 OV5640YUVFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{    
    UINT8   *pFeatureData8 =pFeaturePara;
    
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {


        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
            *pFeatureReturnPara16++=OV5640_PV_PERIOD_EXTRA_PIXEL_NUMS + OV5640_PV_PERIOD_PIXEL_NUMS + OV5640YUV_dummy_pixels;//OV5640_PV_PERIOD_PIXEL_NUMS+OV5640YUV_dummy_pixels;
            *pFeatureReturnPara16=OV5640_PV_PERIOD_LINE_NUMS+OV5640YUV_dummy_lines;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *pFeatureReturnPara32 = 55250000; //19500000;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            OV5640YUV_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            OV5640YUV_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            OV5640YUV_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            OV5640YUV_isp_master_clock=*pFeatureData32;
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            OV5640YUV_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = OV5640YUV_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
                OV5640YUVSensorCCT[i].Addr=*pFeatureData32++;
                OV5640YUVSensorCCT[i].Para=*pFeatureData32++;
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=OV5640YUVSensorCCT[i].Addr;
                *pFeatureData32++=OV5640YUVSensorCCT[i].Para;
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
                OV5640YUVSensorReg[i].Addr=*pFeatureData32++;
                OV5640YUVSensorReg[i].Para=*pFeatureData32++;
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=OV5640YUVSensorReg[i].Addr;
                *pFeatureData32++=OV5640YUVSensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=OV5640_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, OV5640YUVSensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, OV5640YUVSensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &OV5640YUVSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            OV5640YUV_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            OV5640YUV_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=OV5640YUV_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            OV5640YUV_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            OV5640YUV_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            OV5640YUV_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
			//test by lingnan
            //pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            //SENSORDB("OV5640_FOCUS_AD5820_Move_to %d\n", *pFeatureData16);
            break;
        case SENSOR_FEATURE_GET_AF_STATUS:
            //for yuv use:
            //SENSORDB("SENSOR_FEATURE_GET_AF_STATUS pFeatureReturnPara32=0x%x\n",pFeatureReturnPara32);
            break;
        case SENSOR_FEATURE_GET_AF_INF:
            break;
        case SENSOR_FEATURE_GET_AF_MACRO:
            break;                
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            OV5640YUVSetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_YUV_CMD:
            OV5640YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));	
            //OV5640YUVSensorSetting((FEATURE_ID)*pFeatureData8,(UINT32)(*(pFeatureData8+1)));	
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV5640YUVGetSensorID(pFeatureReturnPara32); 
            break; 				            
        case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
            //SENSORDB("SENSOR_FEATURE_SINGLE_FOCUS_MODE\n");
            break;		
        case SENSOR_FEATURE_CANCEL_AF:
            //SENSORDB("SENSOR_FEATURE_CANCEL_AF\n");
            break;			
        case SENSOR_FEATURE_SET_AF_WINDOW:
            //SENSORDB("SENSOR_FEATURE_SET_AF_WINDOW\n");
            //SENSORDB("get zone addr = 0x%x\n",*pFeatureData32);			
            break;	
		
        default:
            break;
    }
    return ERROR_NONE;
}	/* OV5640YUVFeatureControl() */

UINT32 OV5640_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
static SENSOR_FUNCTION_STRUCT	SensorFuncOV5640YUV=
{
    OV5640YUVOpen,
    OV5640YUVGetInfo,
    OV5640YUVGetResolution,
    OV5640YUVFeatureControl,
    OV5640YUVControl,
    OV5640YUVClose
};
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncOV5640YUV;

    return ERROR_NONE;
}   /* OV5640_YUV_SensorInit() */
