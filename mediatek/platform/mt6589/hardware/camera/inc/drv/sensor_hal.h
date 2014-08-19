/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _SENSOR_HAL_H_
#define _SENSOR_HAL_H_
class IBaseCamExif;



typedef enum
{
	ACDK_SCENARIO_ID_CAMERA_PREVIEW=0,
	ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,
	ACDK_SCENARIO_ID_VIDEO_PREVIEW,
	ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO,
	ACDK_SCENARIO_ID_CAMERA_ZSD,
	ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW,
	ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE,
	ACDK_SCENARIO_ID_CAMERA_3D_VIDEO,
	ACDK_SCENARIO_ID_TV_OUT,
	ACDK_SCENARIO_ID_MAX,	 
}	ACDK_SCENARIO_ID_ENUM;

/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef unsigned short MUINT16;
/*******************************************************************************
*
********************************************************************************/


typedef struct halSensorIFParam_s {
    MUINT32 u4SrcW;          // For input sensor width 
    MUINT32 u4SrcH;          // For input sensor height 
    MUINT32 u4CropW;		//TG crop width
	MUINT32 u4CropH;	    //TG crop height
    MUINT32 u4IsContinous;
    MUINT32 u4IsBypassSensorScenario;
    MUINT32 u4IsBypassSensorDelay;
	ACDK_SCENARIO_ID_ENUM scenarioId;
} halSensorIFParam_t;
//
typedef struct halSensorRawImageInfo_s {
    MUINT32 u4Width; 
    MUINT32 u4Height;
    MUINT32 u4BitDepth; 
    MUINT32 u4IsPacked; 
    MUINT32 u4Size;
    MUINT32 u1Order;
} halSensorRawImageInfo_t; 
//

// vend_edwin.yang
typedef struct {
    MUINT32 u4GrabX;          // For input sensor width
    MUINT32 u4GrabY;          // For input sensor height
    MUINT32 u4SrcW;          // For input sensor width
    MUINT32 u4SrcH;          // For input sensor height
    MUINT32 u4CropW;        //TG crop width
    MUINT32 u4CropH;        //TG crop height
    MUINT32 DataFmt;
} SENSOR_CROP_INFO;


typedef struct
{
  MUINT16 u4SensorGrabStartX;
  MUINT16 u4SensorGrabStartY; 
} SENSOR_GRAB_INFO_STRUCT;



typedef enum halSensorCmd_s {
    SENSOR_CMD_SET_SENSOR_DEV          		= 0x1000,
    SENSOR_CMD_SET_SENSOR_EXP_TIME,
    SENSOR_CMD_SET_SENSOR_EXP_LINE,
	SENSOR_CMD_SET_SENSOR_GAIN,
    SENSOR_CMD_SET_FLICKER_FRAME_RATE,
    SENSOR_CMD_SET_VIDEO_FRAME_RATE,
    SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC,
    SENSOR_CMD_SET_CCT_FEATURE_CONTROL,
    SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA,
    SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO,
    SENSOR_CMD_GET_SENSOR_DEV				= 0x2000,
    SENSOR_CMD_GET_SENSOR_PRV_RANGE,
    SENSOR_CMD_GET_SENSOR_FULL_RANGE,
    SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
    SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE,
    SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE,
    SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE,
    SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE,
    SENSOR_CMD_GET_SENSOR_ID,
    SENSOR_CMD_GET_RAW_PADDING_RANGE,
    SENSOR_CMD_GET_SENSOR_NUM,
    SENSOR_CMD_GET_SENSOR_TYPE,
    SENSOR_CMD_GET_RAW_INFO,
    SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT,
    SENSOR_CMD_GET_INPUT_BIT_ORDER,
    SENSOR_CMD_GET_PAD_PCLK_INV,
    SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE,
    SENSOR_CMD_GET_SENSOR_FACING_DIRECTION,
    SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,
    SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,
    SENSOR_CMD_GET_SENSOR_FEATURE_INFO,
    SENSOR_CMD_GET_ATV_DISP_DELAY_FRAME,
    SENSOR_CMD_GET_SENSOR_SCENARIO,
    SENSOR_CMD_GET_SENSOR_CROPINFO,
    SENSOR_CMD_GET_SENSOR_GRAB_INFO,
    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
    SENSOR_CMD_GET_FAKE_ORIENTATION,
    SENSOR_CMD_SET_YUV_FEATURE_CMD			= 0x3000,
    SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,
    SENSOR_CMD_SET_YUV_CANCEL_AF,
    SENSOR_CMD_SET_YUV_CONSTANT_AF,
    SENSOR_CMD_SET_YUV_AF_WINDOW,    
    SENSOR_CMD_SET_YUV_AE_WINDOW,   
    SENSOR_CMD_GET_YUV_AF_STATUS			= 0x4000,
    SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN,    
    SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN,
    SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,
    SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,
    SENSOR_CMD_GET_YUV_EXIF_INFO,
    SENSOR_CMD_GET_YUV_DELAY_INFO,
    SENSOR_CMD_GET_YUV_AE_AWB_LOCK,
    SENSOR_CMD_MAX                 = 0xFFFF
} halSensorCmd_e;
//
typedef enum halSensorDev_s {
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_ATV  = 0x04,
    SENSOR_DEV_MAIN_2 = 0x08,
    SENSOR_DEV_MAIN_3D = 0x09,
} halSensorDev_e;

typedef enum halSensorType_s {
    SENSOR_TYPE_RAW = 0, 
    SENSOR_TYPE_YUV = 1, 
    SENSOR_TYPE_YCBCR = 2,
    SENSOR_TYPE_RGB565 = 3, 
    SENSOR_TYPE_RGB888 = 4,     
    SENSOR_TYPE_JPEG = 5,    
    SENSOR_TYPE_UNKNOWN = 0xFF,
} halSensorType_e; 


typedef enum halSensorDelayFrame_s {
	SENSOR_PREVIEW_DELAY = 0,
	SENSOR_VIDEO_DELAY,
	SENSOR_CAPTURE_DELAY,
	SENSOR_YUV_AWB_SETTING_DELAY,
	SENSOR_YUV_EFFECT_SETTING_DELAY,
	SENSOR_AE_SHUTTER_DELAY,
	SENSOR_AE_GAIN_DELAY,
	SENSOR_AE_ISP_DELAY,
}halSensorDelayFrame_e;
//


/*******************************************************************************
*
********************************************************************************/
class SensorHal {

public:
    //
    static SensorHal* createInstance();
    virtual void destroyInstance() = 0;

protected:
    virtual ~SensorHal() {};

public:
    virtual MINT32 searchSensor() = 0;
    //
    virtual MINT32 init() = 0;
    //
    virtual MINT32 uninit() = 0;
    //
    virtual MINT32 setATVStart() = 0;
    //
    virtual MINT32 setConf(halSensorIFParam_t halSensorIFParam[2]) = 0;
    //
    virtual MINT32 sendCommand(
	    halSensorDev_e sensorDevId,
        int cmd,
        int arg1 = 0,
        int arg2 = 0,
        int arg3 = 0) = 0;
    //
    virtual MINT32 dumpReg() = 0;
	//
	virtual MINT32 setDebugInfo(IBaseCamExif *pIBaseCamExif) = 0;
//
	virtual MINT32 reset() = 0;
//
};

/*******************************************************************************
*
********************************************************************************/


#endif // _ISP_DRV_H_

