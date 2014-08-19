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
#define LOG_TAG "aaa_hal_yuv"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
//#include <aaa_state.h>   //by jmac
//#include <camera_custom_nvram.h>
//#include <awb_param.h>
//#include <awb_mgr.h>
//#include <af_param.h>
#include <mcu_drv.h>
#include <isp_reg.h>
//#include <af_mgr.h>
#include <flash_param.h>
//#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <IBaseCamExif.h>
#include <sensor_hal.h>
//#include <ae_param.h>
#include <CamDefs.h>
//#include <ae_mgr.h>
#include <kd_camera_feature.h>
#include "aaa_hal_yuv.h"
#include "kd_imgsensor_define.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <config/PriorityDefs.h>
#include <sys/prctl.h>
MINT32        g_bAFThreadLoop_yuv;
pthread_t     g_AFThread_yuv;
sem_t         g_semAFThreadEnd_yuv;    
IspDrv*       g_pIspDrv_yuv;

using namespace NS3A;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define ERROR_CHECK(API)\
   {\
   MRESULT err = API;\
   if (FAILED(err))\
   {\
       setErrorCode(err);\
       return MFALSE;\
   }}\

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv*
Hal3AYuv::
createInstance(MINT32 const i4SensorDevId)
{
    Hal3AYuv *pHal3AYuv  = Hal3AYuv::getInstance();

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            pHal3AYuv->init(ESensorDev_Main);
        break;
        case SENSOR_DEV_SUB:
            pHal3AYuv->init(ESensorDev_Sub);
        break;
        case SENSOR_DEV_MAIN_2:
            pHal3AYuv->init(ESensorDev_MainSecond);
        break;
        case SENSOR_DEV_MAIN_3D:
            pHal3AYuv->init(ESensorDev_Main3D);
        break;
        case SENSOR_DEV_ATV:
            pHal3AYuv->init(ESensorDev_Atv);
        break;
        default:
            MY_ERR("Unsupport sensor device: %d\n", i4SensorDevId);
            return MNULL;
        break;
    }

    return pHal3AYuv;
}

Hal3AYuv*
Hal3AYuv::
getInstance()
{
    static Hal3AYuv singleton;
    return &singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AYuv::
destroyInstance()
{
    uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv::Hal3AYuv()
    : Hal3ABase()
    , m_Users(0)
    , m_Lock()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    , bAELockSupp(0)
    , bAWBLockSupp(0)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv::~Hal3AYuv()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AYuv::
init(MINT32 i4SensorDev)
{
    MY_LOG("[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;

   	Mutex::Autolock lock(m_Lock);
   
   	if (m_Users > 0){
      		MY_LOG("%d has created \n", m_Users);
      		android_atomic_inc(&m_Users);
      		return S_3A_OK;
   	}
    //SensorHal init
    if (!m_pSensorHal)   {
        m_pSensorHal = SensorHal::createInstance();
        MY_LOG("[m_pSensorHal]:0x%08x \n",m_pSensorHal);
        if (!m_pSensorHal) {
            MY_ERR("SensorHal::createInstance() fail \n");
            return ret;
        }
    }
    m_i4SensorDev = i4SensorDev;
    // init
    sendCommand(ECmd_Init, 0);
    //ERROR_CHECK(IspTuningMgr::getInstance().init(getSensorDev()))

    android_atomic_inc(&m_Users);

    return S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AYuv::
uninit()
{
    MRESULT ret = S_3A_OK;

    MY_LOG("[%s()] m_Users: %d \n", __FUNCTION__, m_Users);
    
    Mutex::Autolock lock(m_Lock);
    
    // If no more users, return directly and do nothing.
    if (m_Users <= 0){
    	   return S_3A_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    // There is no more User after decrease one User
    if (m_Users == 0) {
        //Reset Parameter
        Param_T npara;
        m_rParam = npara;
        
        sendCommand(ECmd_Uninit, 0);
        //ERROR_CHECK(IspTuningMgr::getInstance().uninit())
        //SensorHal uninit
        if (m_pSensorHal){
            m_pSensorHal->destroyInstance();
            m_pSensorHal = NULL;
        }
    }
    // There are still some users
    else{
    	   MY_LOG("Still %d users \n", m_Users);
    }
    
    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::sendCommand(ECmd_T const eCmd, MINT32 const i4Arg)
{
    if(eCmd != ECmd_Update)
	      MY_LOG("[%s()],%d\n", __FUNCTION__,eCmd);

    if (eCmd == ECmd_Init){        
        //EnableAFThread(1);
        return MTRUE;
    }
    else if  (eCmd == ECmd_CameraPreviewStart || eCmd == ECmd_RecordingStart){
         //Force reset Parameter
         Param_T old_para,rst_para;
         old_para = m_rParam;
         m_rParam = rst_para;
         setParams(old_para);

        return MTRUE;
    }
    else if  (eCmd == ECmd_Uninit){
        //EnableAFThread(0);
        return MTRUE;
    }
    else if (eCmd == ECmd_PrecaptureStart){
        //workaround by jmac
        m_bReadyToCapture = 1;
        return MTRUE;
    }
    else if (eCmd == ECmd_Update){
         return MTRUE;
    }
    else{
        MY_LOG("undefine \n");    
        return MTRUE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setParams(Param_T const &rNewParam)
{
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0; 

    MY_LOG("[%s()] \n", __FUNCTION__);

    if (m_rParam.u4EffectMode != rNewParam.u4EffectMode){
        MY_LOG("[FID_COLOR_EFFECT],(%d)->(%d) \n",m_rParam.u4EffectMode,rNewParam.u4EffectMode);
        yuvCmd = FID_COLOR_EFFECT;
        yuvParam = rNewParam.u4EffectMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex){
        MY_LOG("[FID_AE_EV],Idx:(%d)->(%d),Step:(%d)->(%d) \n",m_rParam.i4ExpIndex,rNewParam.i4ExpIndex,m_rParam.fExpCompStep, rNewParam.fExpCompStep);
        yuvCmd = FID_AE_EV;
        yuvParam = mapAEToEnum(rNewParam.i4ExpIndex,rNewParam.fExpCompStep);        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.u4SceneMode != rNewParam.u4SceneMode){
        MY_LOG("[FID_SCENE_MODE],(%d)->(%d) \n",m_rParam.u4SceneMode,rNewParam.u4SceneMode);
        yuvCmd = FID_SCENE_MODE;
        yuvParam = rNewParam.u4SceneMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.u4AwbMode != rNewParam.u4AwbMode){
        MY_LOG("[FID_AWB_MODE],(%d)->(%d) \n",m_rParam.u4AwbMode,rNewParam.u4AwbMode);
        yuvCmd = FID_AWB_MODE;
        yuvParam = rNewParam.u4AwbMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.u4AntiBandingMode != rNewParam.u4AntiBandingMode){
        MY_LOG("[FID_AE_FLICKER],(%d)->(%d) \n",m_rParam.u4AntiBandingMode,rNewParam.u4AntiBandingMode);
        yuvCmd = FID_AE_FLICKER;
        yuvParam = rNewParam.u4AntiBandingMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }    
    if (m_rParam.u4SaturationMode != rNewParam.u4SaturationMode){
        MY_LOG("[FID_ISP_SAT],(%d)->(%d) \n",m_rParam.u4SaturationMode,rNewParam.u4SaturationMode);
        yuvCmd = FID_ISP_SAT;
        yuvParam = rNewParam.u4SaturationMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.u4ContrastMode != rNewParam.u4ContrastMode){
        MY_LOG("[FID_ISP_CONTRAST],(%d)->(%d) \n",m_rParam.u4ContrastMode,rNewParam.u4ContrastMode);
        yuvCmd = FID_ISP_CONTRAST;
        yuvParam = rNewParam.u4ContrastMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.u4IsoSpeedMode != rNewParam.u4IsoSpeedMode){
        MY_LOG("[FID_AE_ISO],(%d)->(%d) \n",m_rParam.u4IsoSpeedMode,rNewParam.u4IsoSpeedMode);
        yuvCmd = FID_AE_ISO;
        yuvParam = mapISOToEnum(rNewParam.u4IsoSpeedMode);
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    if (m_rParam.bIsAELock != rNewParam.bIsAELock && bAELockSupp==1){
         MY_LOG("[FID_AE_LOCK],(%d)->(%d) \n",m_rParam.bIsAELock,rNewParam.bIsAELock);
         yuvCmd = FID_AE_SCENE_MODE;
         yuvParam=rNewParam.bIsAELock==1?AE_MODE_OFF:AE_MODE_AUTO;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);             
    }
    if (m_rParam.bIsAWBLock != rNewParam.bIsAWBLock && bAWBLockSupp==1){
         MY_LOG("[FID_AWB_LOCK],(%d)->(%d) \n",m_rParam.bIsAWBLock,rNewParam.bIsAWBLock);
         yuvCmd = FID_AWB_MODE;
         yuvParam=(rNewParam.bIsAWBLock==1)?AWB_MODE_OFF:AWB_MODE_AUTO;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);             
    }
    //for cam-mode
    if (m_rParam.u4CamMode != rNewParam.u4CamMode){
    	   if (rNewParam.u4CamMode == eAppMode_VideoMode||rNewParam.u4CamMode == eAppMode_VtMode){
             MY_LOG("[FID_CAM_MODE],(%d)->(%d),fps(%d) \n",m_rParam.u4CamMode, rNewParam.u4CamMode,rNewParam.i4MaxFps);
             yuvParam=(rNewParam .i4MaxFps<=20000)?15:30;
             m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_VIDEO_FRAME_RATE,(int)&yuvParam,0,0);             
    	   }
    }
    //for frame rate
    if (m_rParam.i4MaxFps!=rNewParam.i4MaxFps||m_rParam.i4MinFps!=rNewParam.i4MinFps){
        if(rNewParam.i4MinFps==rNewParam.i4MaxFps&&rNewParam.i4MaxFps>0){
             MY_LOG("[FID_FIX_FRAMERATE],Max(%d)->(%d) \n",m_rParam.i4MaxFps,rNewParam.i4MaxFps);
             yuvParam=(rNewParam.i4MaxFps<=20000)?15:30;
             m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_VIDEO_FRAME_RATE,(int)&yuvParam,0,0);             
        }
    }

    m_rParam = rNewParam;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::getSupportedParams(FeatureParam_T &rFeatureParam) 
{	
    MINT32 ae_lock=0,awb_lock=0;
    MINT32 max_focus=0,max_meter=0;

    MY_LOG("[%s()] \n", __FUNCTION__);

    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AE_AWB_LOCK,(int)&ae_lock,(int)&awb_lock,0);
    bAELockSupp = ae_lock==1?1:0;
    bAWBLockSupp = awb_lock==1?1:0;
    rFeatureParam.bExposureLockSupported = bAELockSupp;
    rFeatureParam.bAutoWhiteBalanceLockSupported = bAWBLockSupp;
    MY_LOG("AE_sup(%d),AWB_sub(%d) \n",bAELockSupp,bAWBLockSupp);

    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,(int)&max_focus,0,0);
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,(int)&max_meter,0,0);
    rFeatureParam.u4MaxMeterAreaNum = max_meter>=1?1:0;
    rFeatureParam.u4MaxFocusAreaNum = max_focus>=1?1:0;    
    MY_LOG("FOCUS_max(%d),METER_max(%d) \n",max_focus,max_meter);

    //rFeatureParam.i4MaxLensPos = AfMgr::getInstance().getMaxLensPos();
    //rFeatureParam.i4MinLensPos = AfMgr::getInstance().getMinLensPos();
    rFeatureParam.i4AFBestPos = 0;
    rFeatureParam.i8BSSVlu = 0;
    
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::autoFocus()
{
//    ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFStart)) //jmac
    m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::cancelAutoFocus()
{
//    ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFEnd))  // by jmac

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
//    ERROR_CHECK(AeMgr::getInstance().setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height))

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
	   MY_LOG("[%s()] \n", __FUNCTION__);
	   
    SENSOR_EXIF_INFO_STRUCT mSensorInfo;
    EXIF_INFO_T rEXIFInfo;
    memset(&rEXIFInfo, 0, sizeof(EXIF_INFO_T));
    memset(&mSensorInfo, 0, sizeof(SENSOR_EXIF_INFO_STRUCT));

    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_EXIF_INFO,(int)&mSensorInfo,0,0);             

    MY_LOG("FNumber=%d, AEISOSpeed=%d, AWBMode=%d, CapExposureTime=%d, FlashLightTimeus=%d, RealISOValue=%d\n", 
           mSensorInfo.FNumber, mSensorInfo.AEISOSpeed, mSensorInfo.AWBMode, 
           mSensorInfo.CapExposureTime, mSensorInfo.FlashLightTimeus, mSensorInfo.RealISOValue);

    rEXIFInfo.u4FNumber = mSensorInfo.FNumber>0 ? mSensorInfo.FNumber : 28;
    rEXIFInfo.u4FocalLength = 350;
    rEXIFInfo.u4SceneMode = m_rParam.u4SceneMode>0 ? m_rParam.u4SceneMode : 0;
    rEXIFInfo.u4AWBMode = mSensorInfo.AWBMode>0 ? mSensorInfo.AWBMode : 0;
    rEXIFInfo.u4CapExposureTime = mSensorInfo.CapExposureTime>0? mSensorInfo.CapExposureTime : 0;
    rEXIFInfo.u4FlashLightTimeus = mSensorInfo.FlashLightTimeus>0? mSensorInfo.FlashLightTimeus : 0;
    rEXIFInfo.u4AEISOSpeed =mapEnumToISO(mSensorInfo.AEISOSpeed);
    rEXIFInfo.u4AEISOSpeed = mapEnumToISO(mSensorInfo.AEISOSpeed);
    rEXIFInfo.i4AEExpBias = 0;
    	
    pIBaseCamExif->set3AEXIFInfo(&rEXIFInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setDebugInfo(IBaseCamExif *pIBaseCamExif) const
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::getDelayFrame(EQueryType_T const eQueryType) const
{
    MUINT32 ret = 0;
    SENSOR_DELAY_INFO_STRUCT pDelay;
	   
    MY_LOG("[%s()] \n", __FUNCTION__);

    memset(&pDelay,0x0,sizeof(SENSOR_DELAY_INFO_STRUCT));
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_DELAY_INFO,(int)&pDelay,0,0);
    MY_LOG("Init:%d,effect:%d,awb:%d \n",pDelay.InitDelay,pDelay.EffectDelay,pDelay.AwbDelay);

    switch (eQueryType)
    {
        case EQueryType_Init:
        {
            ret = (pDelay.InitDelay>0 && pDelay.InitDelay<5)?pDelay.InitDelay:3;
            return ret;
        }
        case EQueryType_Effect:
        {
             ret = (pDelay.EffectDelay>0 && pDelay.EffectDelay<5)?pDelay.EffectDelay:0;
             return ret;
        }
        case EQueryType_AWB:
        {
            ret = (pDelay.AwbDelay>0 && pDelay.AwbDelay<5)?pDelay.AwbDelay:0;
            return ret;
        }
        default:
            return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setIspProfile(EIspProfile_T const eIspProfile)
{
//    ERROR_CHECK(IspTuningMgr::getInstance().setIspProfile(eIspProfile))
 //   ERROR_CHECK(IspTuningMgr::getInstance().validate())

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT Hal3AYuv::EnableAFThread(MINT32 a_bEnable)
{
    MRESULT ret = S_3A_OK;
#if 0
    if (a_bEnable)  {

        if (g_bAFThreadLoop_yuv == 0)
        {  
            ret = AfMgr::getInstance().init();
            if (FAILED(ret)) {
                MY_ERR("AfMgr::getInstance().init() fail\n");
                return ret;
            }
            
            // create AF thread
            MY_LOG("[AFThread] Create");    
            g_bAFThreadLoop_yuv = 1;
            sem_init(&g_semAFThreadEnd_yuv, 0, 0);
            ::prctl(PR_SET_NAME,"AFthread", 0, 0, 0);
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_AF_THREAD};
            pthread_create(&g_AFThread_yuv, &attr, AFThreadFunc, NULL);
        }
    }
    else   {

        if (g_bAFThreadLoop_yuv == 1)
        {
            ret = AfMgr::getInstance().uninit();
            if (FAILED(ret)) {
                MY_ERR("AfMgr::getInstance().init() fail\n");
                return ret;
            }
        
            g_bAFThreadLoop_yuv = 0;
            ::sem_wait(&g_semAFThreadEnd_yuv);
            MY_LOG("[AFThread] Delete");
        }
    }
#endif
    return ret;
}

MVOID * Hal3AYuv::AFThreadFunc(void *arg)
{
#if 0
    MY_LOG("[AFThread] tid: %d \n", gettid());

    if (!g_pIspDrv_yuv) {
        MY_LOG("[AFThread] m_pIspDrv null\n");
        return NULL;
    }

    // wait AFO done
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_AF_DON_ST;
    WaitIrq.Timeout = 200; // 200 msec

    while (g_bAFThreadLoop_yuv) {
       
        if (g_pIspDrv_yuv->waitIrq(WaitIrq) >= 0) // success
        {  
//            StateMgr::getInstance().sendCmd(ECmd_AFUpdate);  // by jmac
        }
        else
        {
            MY_LOG("[AFThread] AF irq timeout\n");
        }
    }

    ::sem_post(&g_semAFThreadEnd_yuv);

    MY_LOG("[AFThread] End \n");

#endif
    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setCallbacks(I3ACallBack* cb)
{
	MY_LOG("[%s()][p]%d\n", __FUNCTION__, cb);

    m_pAFYuvCallBack=cb;


    return MTRUE;

   // return AfMgr::getInstance().setCallbacks(cb);
}

//******************************************************************************
// Map AE exposure to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapAEToEnum(MINT32 mval,MFLOAT mstep)
{
    MINT32 pEv,ret;  

    pEv = 100 * mval * mstep;
    
    if     (pEv <-250) { ret = AE_EV_COMP_n30;}  // EV compensate -3.0
    else if(pEv <-200) { ret = AE_EV_COMP_n25;}  // EV compensate -2.5
    else if(pEv <-170) { ret = AE_EV_COMP_n20;}  // EV compensate -2.0
    else if(pEv <-160) { ret = AE_EV_COMP_n17;}  // EV compensate -1.7
    else if(pEv <-140) { ret = AE_EV_COMP_n15;}  // EV compensate -1.5
    else if(pEv <-130) { ret = AE_EV_COMP_n13;}  // EV compensate -1.3    
    else if(pEv < -90) { ret = AE_EV_COMP_n10;}  // EV compensate -1.0
    else if(pEv < -60) { ret = AE_EV_COMP_n07;}  // EV compensate -0.7
    else if(pEv < -40) { ret = AE_EV_COMP_n05;}  // EV compensate -0.5
    else if(pEv < -10) { ret = AE_EV_COMP_n03;}  // EV compensate -0.3    
    else if(pEv ==  0) { ret = AE_EV_COMP_00; }  // EV compensate -2.5
    else if(pEv <  40) { ret = AE_EV_COMP_03; }  // EV compensate  0.3
    else if(pEv <  60) { ret = AE_EV_COMP_05; }  // EV compensate  0.5
    else if(pEv <  90) { ret = AE_EV_COMP_07; }  // EV compensate  0.7
    else if(pEv < 110) { ret = AE_EV_COMP_10; }  // EV compensate  1.0
    else if(pEv < 140) { ret = AE_EV_COMP_13; }  // EV compensate  1.3
    else if(pEv < 160) { ret = AE_EV_COMP_15; }  // EV compensate  1.5
    else if(pEv < 180) { ret = AE_EV_COMP_17; }  // EV compensate  1.7    
    else if(pEv < 210) { ret = AE_EV_COMP_20; }  // EV compensate  2.0
    else if(pEv < 260) { ret = AE_EV_COMP_25; }  // EV compensate  2.5
    else if(pEv < 310) { ret = AE_EV_COMP_30; }  // EV compensate  3.0
    else               { ret = AE_EV_COMP_00;}
    
    MY_LOG("[%s()]EV:(%d),Ret:(%d)\n", __FUNCTION__, pEv,ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapISOToEnum(MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret;  
    
    switch(u4NewAEISOSpeed){
        case 0:
            ret = AE_ISO_AUTO;
            break;
        case 100:
            ret = AE_ISO_100;
            break;
        case 200:
            ret = AE_ISO_200;
            break;
        case 400:
            ret = AE_ISO_400;
            break;
        case 800:
             ret = AE_ISO_800;
           break;
        case 1600:
            ret = AE_ISO_1600;
           break;
        default:
            MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);            
            ret = AE_ISO_AUTO;
            break;
    }
    MY_LOG("[%s()]ISOVal:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEISOSpeed, ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapEnumToISO(MUINT32 u4NewAEIsoEnum) const
{
    MINT32 ret;  
    
    switch(u4NewAEIsoEnum){
        case AE_ISO_AUTO:
            ret = 100;
            break;
        case AE_ISO_100:
            ret = 100;
            break;
        case AE_ISO_200:
            ret = 200;
            break;
        case AE_ISO_400:
            ret = 400;
            break;
        case AE_ISO_800:
             ret = 800;
           break;
        case AE_ISO_1600:
            ret = 1600;
           break;
        default:
            ret = 100;
            break;
    }
    MY_LOG("[%s()]ISOEnum:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEIsoEnum, ret);

    return ret;
}

