/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <cutils/memory.h>

//
/*
#include <gui/Surface.h>
#include <gui/ISurface.h>
#include <gui/SurfaceComposerClient.h>
*/
//
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
//
#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"
#if defined(HAVE_COMMAND_test_preview)
//
using namespace android;
//
#include <sys/prctl.h>
//
//
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCmd_test_params {
char const
gPromptText[] = {
    "\n"
    "\n test_preview <action> <optional arguments...>"
    "\n "
    "\n where <action> may be one of the following:"
    "\n  <-h>                       help"
    "\n  <getNumberOfCameras>       get the number of cameras."
    "\n  <getCameraInfo>            get the camera info."
    "\n  <connect>                  connect camera."
    "\n  <disconnect>               disconnect camera."
    "\n  <setParameters>            set camera parameters."
    "\n  <getParameters>            get camera parameters."
    "\n "
    "\n where <optional arguments...> may be a combination of the followings:"
    "\n  <-app-mode=Default>        app mode; 'Default' by default."
    "\n                             -> 'Default' 'MtkEng' 'MtkAtv' 'MtkS3d' 'MtkVt'"
    "\n  <-open-id=0>               open id; 0 by default (main camera)."
    "\n  <-cam-mode=1>              camera mode in KEY_CAMERA_MODE; 0 by default."
    "\n                             TODO: should modify this definition!!!"
    "\n                             '0' refers to CAMERA_MODE_NORMAL"
    "\n                             '1' refers to CAMERA_MODE_MTK_PRV"
    "\n                             '2' refers to CAMERA_MODE_MTK_VDO"
    "\n                             '3' refers to CAMERA_MODE_MTK_VT"
};
struct CmdImp : public CmdBase, public CameraListener
{
    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CameraListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Interface.
    virtual void                notify(int32_t msgType, int32_t ext1, int32_t ext2) {}
    virtual void                postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {}
    virtual void                postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                                struct Argument : LightRefBase<Argument>
                                {
                                    String8                     ms8AppMode;
                                    int32_t                     mi4CamMode;
                                    int32_t                     mOpenId;
                                };

protected:  ////                Implementation.
    virtual bool                onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);

    virtual bool                onGetNumberOfCameras();
    virtual bool                onGetCameraInfo(sp<Argument> pArgument);
    virtual bool                onConnectCamera(sp<Argument> pArgument);
    virtual bool                onDisconnectCamera();
    virtual bool                onSetParameters(sp<Camera> spCamera, sp<Argument> pArgument);
    virtual bool                onGetParameters(sp<Camera> spCamera);

protected:  ////                Implementation.

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Data Members (Camera)
    sp<Camera>                  mpCamera;

protected:  ////                Data Members (Parameters)
//    sp<Argument>                mpArgument;

};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_test_params, new CmdImp(HAVE_COMMAND_test_params));
};  // NSCmd_test_params
using namespace NSCmd_test_params;


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    sp<Argument> pArgument = new Argument;
    onParseArgumentCommand(rvCmd, pArgument);
    return  onParseActionCommand(rvCmd, pArgument);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Set default.
    pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
    pArgument->mi4CamMode = MtkCameraParameters::CAMERA_MODE_MTK_PRV;
    pArgument->mOpenId = 0;

    //  (2) Start to parse commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        String8 key, val;
        if  ( ! parseOneCmdArgument(s8Cmd, key, val) ) {
            continue;
        }
//        MY_LOGD("<key/val>=<%s/%s>", key.string(), val.string());
        //
        //
        if  ( key == "-app-mode" ) {
            if  ( val == "MtkEng" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ENG);
                continue;
            }
            if  ( val == "MtkAtv" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ATV);
                continue;
            }
            if  ( val == "MtkS3d" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_S3D);
                continue;
            }
            if  ( val == "MtkVt" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_VT);
                continue;
            }
#if 0
            if  ( val == "MtkPhoto" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO);
                continue;
            }
            if  ( val == "MtkVideo" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO);
                continue;
            }
            if  ( val == "MtkZsd" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ZSD);
                continue;
            }
#endif
            pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
            continue;
        }
        //
        if  ( key == "-cam-mode" ) {
            pArgument->mi4CamMode = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-open-id" ) {
            pArgument->mOpenId = ::atoi(val);
            continue;
        }
        //
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            MY_LOGD("%s", gPromptText);
            return  true;
        }
        //
        if  ( s8Cmd == "getNumberOfCameras" ) {
            return  onGetNumberOfCameras();
        }
        //
        if  ( s8Cmd == "getCameraInfo" ) {
            return  onGetCameraInfo(pArgument);
        }
        //
        if  ( s8Cmd == "connect" ) {
            return  onConnectCamera(pArgument);
        }
        //
        if  ( s8Cmd == "disconnect" ) {
            return  onDisconnectCamera();
        }
        //
        if  ( s8Cmd == "setParameters" ) {
            return  onSetParameters(mpCamera, pArgument);
        }
        //
        if  ( s8Cmd == "getParameters" ) {
            return  onGetParameters(mpCamera);
        }
        //
    }
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onGetNumberOfCameras()
{
    int32_t const i4CameraCount = Camera::getNumberOfCameras();
    MY_LOGD("Camera::getNumberOfCameras() = %d ", i4CameraCount);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onGetCameraInfo(sp<Argument> pArgument)
{
    int const iOpenId = pArgument->mOpenId;

    int32_t const i4CameraCount = Camera::getNumberOfCameras();
    if  ( iOpenId >= i4CameraCount && iOpenId != 0xFF )
    {
        MY_LOGE("bad open id=%d >= NumberOfCameras(%d)", iOpenId, i4CameraCount);
        return  false;
    }
    //
    CameraInfo cameraInfo = {0};
    status_t status = Camera::getCameraInfo(iOpenId, &cameraInfo);
    MY_LOGD(
        "CameraInfo:(facing, orientation)=(%d, %d), status(%d)", 
        cameraInfo.facing, cameraInfo.orientation, status
    );
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onConnectCamera(sp<Argument> pArgument)
{
    if  ( mpCamera != 0 ) {
        MY_LOGE("connect before !!!");
        return  false;
    }
    //
    status_t status = OK;
    status = Camera::setProperty(String8(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE), pArgument->ms8AppMode);
    MY_LOGD("status(%d), app-mode=%s", status, pArgument->ms8AppMode.string());

    int id = pArgument->mOpenId;

    mpCamera = Camera::connect(id);
    if  ( mpCamera == 0 )
    {
        MY_LOGE("Camera::connect, id(%d)", id);
        return  false;
    }
    //
    //
    mpCamera->setListener(this);
    //
    MY_LOGD("Camera::connect, id(%d), camera(%p)", id, mpCamera.get());
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onDisconnectCamera()
{
    if  ( mpCamera != 0 )
    {
        MY_LOGD("Camera::disconnect, camera(%p)", mpCamera.get());
        mpCamera->disconnect();
        mpCamera = NULL;
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onSetParameters(sp<Camera> spCamera, sp<Argument> pArgument)
{
    CameraParameters params(spCamera->getParameters());
    //
    params.set(MtkCameraParameters::KEY_CAMERA_MODE, pArgument->mi4CamMode);
    //
/*
    params.setPreviewSize(pArgument->mPreviewSize.width, pArgument->mPreviewSize.height);
    //
    params.set(CameraParameters::KEY_SCENE_MODE, pArgument->ms8SceneMode.string());
    //
    params.set(CameraParameters::KEY_ZOOM, pArgument->mZoomStep);
    //
    params.set(CameraParameters::KEY_WHITE_BALANCE, pArgument->ms8AWBmode.string());
    //
    params.set(CameraParameters::KEY_EFFECT, pArgument->ms8EFFECTmode.string());
    //
    params.set(CameraParameters::KEY_PREVIEW_FORMAT, pArgument->ms8PrvFmt.string());  
*/
    //
    if  (OK != spCamera->setParameters(params.flatten()))
    {
        CAM_LOGE("setParameters\n");
        return  false;
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onGetParameters(sp<Camera> spCamera)
{
    if  ( spCamera == 0 )
    {
        MY_LOGW("spCamera == 0");
        return  false;
    }
    //
    CameraParameters params(spCamera->getParameters());
    Vector<String16> dummy;
    params.dump(1, dummy);
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

