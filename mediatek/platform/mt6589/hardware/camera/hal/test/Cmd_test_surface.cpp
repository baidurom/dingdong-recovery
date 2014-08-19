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
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
*/
//
#include <gui/Surface.h>
#include <gui/ISurface.h>
#include <gui/SurfaceComposerClient.h>
//
#include <camera/CameraParameters.h>
//
#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"
#if defined(HAVE_COMMAND_test_surface)
//
#include "res/YV12_640x480.h"
//
using namespace android;
//
//
//
/******************************************************************************
 *  Command
 *      test_surface <start> <stop> <report> <-h> <-loop-count=10> <-loop-interval=5> <-source=builtin> <-format=yv12> <-preview-size=640x480> <-display-rotation=90>
 *
 *      start:              start to execute this test.
 *      stop:               stop testing.
 *      report:             report a snapshot of the current status.
 *      -h:                 help
 *      -loop-count:        loop count; 10 by default.
 *      -loop-interval:     sleep interval (sec) per loop; 5 sec by default.
 *      -source:            the source to render the buffer.
 *                          "const" "builtin" "sdcard"
 *      -format:            source format: yv12, i420
 *      -preview-size:      preview size; 640x480 by default.
 *      -display-rotation:  display rotation; 90 by default.
 *
 ******************************************************************************/
namespace NSCmd_test_surface {
struct CmdImp : public CmdBase, public Thread
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
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                                struct Argument : LightRefBase<Argument>
                                {
                                    size_t                      mLoopCount;
                                    int32_t                     mLoopInterval;
                                    Size                        mPreviewSize;
                                    int32_t                     mHalPixelFormat;
                                    int32_t                     mHalTransformRotation;
                                    String8                     ms8RenderSource;
                                };

protected:  ////                Implementation.
    virtual bool                onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);

    virtual bool                onStart(sp<Argument> pArgument);
    virtual bool                onStop();
    virtual bool                onReset();

protected:  ////                Implementation.
    virtual void                renderBuffer_ByConst(int index, void* pBuf, int stride);
    virtual void                renderBuffer_ByBuiltinImage(int index, void* pBuf, int stride);
    virtual void                renderBuffer_BySdcard(int index, void* pBuf, int stride);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Surface)
    virtual bool                initSurface();
    virtual void                uninitSurface();

protected:  ////                Data Members (Surface)
    int32_t                     mi4SurfaceID;
    sp<SurfaceComposerClient>   mpSurfaceClient;
    sp<SurfaceControl>          mpSurfaceControl;
    sp<Surface>                 mpSurface;
    ANativeWindow*              mpWindow;

protected:  ////                Data Members (Parameters)
    sp<Argument>                mpArgument;
    int32_t volatile            mIsExecuting;
};
/******************************************************************************
 *
 ******************************************************************************/
bool CmdImp::isInstantiate = CmdMap::inst().addCommand(HAVE_COMMAND_test_surface, new CmdImp(HAVE_COMMAND_test_surface));
};  // NSCmd_test_surface
using namespace NSCmd_test_surface;


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
    pArgument->mPreviewSize = Size(640, 480);
    pArgument->mHalPixelFormat = HAL_PIXEL_FORMAT_YV12;
    pArgument->mHalTransformRotation = HAL_TRANSFORM_ROT_90;
    pArgument->mLoopInterval = 5;
    pArgument->mLoopCount = 10;
    pArgument->ms8RenderSource = "builtin";

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
        if  ( key == "-loop-interval" ) {
            pArgument->mLoopInterval = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-loop-count" ) {
            pArgument->mLoopCount = ::atoi(val);
            continue;
        }
        //
        if  ( key == "-source" ) {
            pArgument->ms8RenderSource = val;
            continue;
        }
        //
        if  ( key == "-format" ) {
            if  ( val == "i420" ) {
                pArgument->mHalPixelFormat = HAL_PIXEL_FORMAT_I420;
            }
            else {
                pArgument->mHalPixelFormat = HAL_PIXEL_FORMAT_YV12;
            }
            continue;
        }
        //
        if  ( key == "-preview-size" ) {
            ::sscanf(val.string(), "%dx%d", &pArgument->mPreviewSize.width, &pArgument->mPreviewSize.height);
            MY_LOGD("preview-size : %d %d", pArgument->mPreviewSize.width, pArgument->mPreviewSize.height);
            continue;
        }
        //
        if  ( key == "-display-rotation" ) {
            switch  (::atoi(val))
            {
            case   0: pArgument->mHalTransformRotation = 0;                    break;
            case  90: pArgument->mHalTransformRotation = HAL_TRANSFORM_ROT_90; break;
            case 180: pArgument->mHalTransformRotation = HAL_TRANSFORM_ROT_180;break;
            case 270: pArgument->mHalTransformRotation = HAL_TRANSFORM_ROT_270;break;
            default:  break;
            }
            continue;
        }
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
            String8 text;
            text += "\n";
            text += "\n   test_surface <start> <stop> <report> <-h> <-loop-count=10> <-loop-interval=5> <-source=builtin> <-format=yv12> <-preview-size=640x480> <-display-rotation=90>";
            text += "\n   -h:                 help";
            text += "\n   start:              start to execute this test.";
            text += "\n   stop:               stop testing.";
            text += "\n   report:             report a snapshot of the current status.";
            text += "\n   -loop-count:        loop count; 10 by default.";
            text += "\n   -loop-interval:     sleep interval (sec) per loop; 5 sec by default.";
            text += "\n   -source:            the source to render the buffer.";
            text += "\n                       'const' 'builtin' 'sdcard'";
            text += "\n   -format:            source format: yv12, i420";
            text += "\n   -preview-size:      preview size; 640x480 by default.";
            text += "\n   -display-rotation:  display rotation; 90 by default.";
            MY_LOGD("%s", text.string());
            return  true;
        }
        //
        if  ( s8Cmd == "start" ) {
            return  onStart(pArgument);
        }
        //
        if  ( s8Cmd == "stop" ) {
            return  onStop();
        }
        //
        if  ( s8Cmd == "report" ) {
            return  printf(" <Status> %s \n", mIsExecuting ? "executing test" : "idle");;
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
onStart(sp<Argument> pArgument)
{
    if  ( 1 == ::android_atomic_release_load(&mIsExecuting) ) {
        MY_LOGW("the test has been starting before...skip this command.");
        return  false;
    }
    //
    onReset();
    ::android_atomic_release_store(1, &mIsExecuting);
    mpArgument = pArgument;
    return  (OK == run());
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onStop()
{
    ::android_atomic_release_store(0, &mIsExecuting);
    requestExit();
    return  (OK == join());
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
onReset()
{
    ::android_atomic_release_store(0, &mIsExecuting);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
// Derived class must implement threadLoop(). The thread starts its life
// here. There are two ways of using the Thread object:
// 1) loop: if threadLoop() returns true, it will be called again if
//          requestExit() wasn't called.
// 2) once: if threadLoop() returns false, the thread will exit upon return.
bool
CmdImp::
threadLoop()
{
    status_t status;
    bool ret = false;
    //
    ret = initSurface();
    if  ( ! ret )
    {
        goto lbExit;
    }
    //
    //
    MY_LOGD("window=%p", mpWindow);
    native_window_api_connect(mpWindow, NATIVE_WINDOW_API_CAMERA);
    native_window_set_buffers_geometry(mpWindow, mpArgument->mPreviewSize.width, mpArgument->mPreviewSize.height, HAL_PIXEL_FORMAT_YV12);
    native_window_set_usage(mpWindow, GraphicBuffer::USAGE_SW_WRITE_OFTEN|GraphicBuffer::USAGE_HW_TEXTURE);
    native_window_set_scaling_mode(mpWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);    
    native_window_set_buffers_transform(mpWindow, mpArgument->mHalTransformRotation);
    native_window_set_buffer_count(mpWindow, 3);
    //
    for (size_t i = 0; i < mpArgument->mLoopCount && 1 == ::android_atomic_release_load(&mIsExecuting); i++)
    {
        void *buffer;
        ANativeWindowBuffer* pANWBuffer = NULL;
        sp<GraphicBuffer> pGraphicBuffer = 0;
        //
        status = mpWindow->dequeueBuffer(mpWindow, &pANWBuffer);
        if  ( OK != status )
        {
            CAM_LOGE("mpWindow->dequeueBuffer - status[%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
        pGraphicBuffer = new GraphicBuffer(pANWBuffer, false);
        status = pGraphicBuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&buffer));
        if  ( OK != status )
        {
            CAM_LOGE("GraphicBuffer->lock - status[%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if  ( mpArgument->ms8RenderSource == "const" ) {
            renderBuffer_ByConst(i, buffer, pANWBuffer->stride);
        }
        else if ( mpArgument->ms8RenderSource == "builtin" ) {
            renderBuffer_ByBuiltinImage(i, buffer, pANWBuffer->stride);
        }
        else if ( mpArgument->ms8RenderSource == "sdcard" ) {
            renderBuffer_BySdcard(i, buffer, pANWBuffer->stride);
        }
        //----------------------------------------------------------------------
        status = pGraphicBuffer->unlock();
        if  ( OK != status )
        {
            CAM_LOGE("GraphicBuffer->unlock - status[%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
        status = mpWindow->queueBuffer(mpWindow, pANWBuffer);
        if  ( OK != status )
        {
            CAM_LOGE("mpWindow->queueBuffer - status[%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
        //
        ::sleep(mpArgument->mLoopInterval);
    }
    //
    //
lbExit:
    uninitSurface();
    onReset();
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CmdImp::
initSurface()
{
    mi4SurfaceID = 0;

    // create a client to surfaceflinger
    mpSurfaceClient = new SurfaceComposerClient();

    mpSurfaceControl = mpSurfaceClient->createSurface(
        String8("surface"), mi4SurfaceID, 480, 800, PIXEL_FORMAT_RGBA_8888
    );
    SurfaceComposerClient::openGlobalTransaction();
    mpSurfaceControl->setLayer(100000);
    SurfaceComposerClient::closeGlobalTransaction();
    // pretend it went cross-process
    Parcel parcel;
    SurfaceControl::writeSurfaceToParcel(mpSurfaceControl, &parcel);
    parcel.setDataPosition(0);
    mpSurface = Surface::readFromParcel(parcel);
    mpWindow = mpSurface.get();
    //
    CAM_LOGD("setupSurface: %p", mpSurface.get());
    return  (mpSurface != 0);
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
uninitSurface()
{
    mpWindow = NULL;
    mpSurface = 0;
    mpSurfaceControl = 0;
    if  ( mpSurfaceClient != 0 )
    {
        mpSurfaceClient->destroySurface(mi4SurfaceID);
        mpSurfaceClient = 0;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
renderBuffer_ByConst(int index, void* pBuf, int stride)
{
    //  Assume YUV420.
    size_t bufsize = (stride * mpArgument->mPreviewSize.height * 3/2);
    //
    char color[] = {0, 128, 255};
    ::memset(pBuf, color[index%3], bufsize);
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
renderBuffer_ByBuiltinImage(int index, void* pBuf, int stride)
{
    struct BuiltinImage
    {
        void const* base;
        size_t      size;
    } builtinImage[] = {
        {TestImage_YV12_640x480_1, sizeof(TestImage_YV12_640x480_1)}, 
        {TestImage_YV12_640x480_2, sizeof(TestImage_YV12_640x480_2)}, 
        {TestImage_YV12_640x480_1, sizeof(TestImage_YV12_640x480_1)}, 
    };

    //  Assume YUV420.
    size_t bufsize = (stride * mpArgument->mPreviewSize.height * 3/2);

    BuiltinImage* pBuiltinImage = &builtinImage[index%3];

    size_t copysize = (bufsize <= pBuiltinImage->size) ? bufsize : pBuiltinImage->size;

    ::memcpy(pBuf, pBuiltinImage->base, copysize);
}


/******************************************************************************
 *
 ******************************************************************************/
void
CmdImp::
renderBuffer_BySdcard(int index, void* pBuf, int stride)
{
    //  Assume YUV420.
    size_t bufsize = (stride * mpArgument->mPreviewSize.height * 3/2);

    String8 s8Filename = String8::format("/sdcard/yv12_640x480_%02d.yuv", (index%3));
    //
    FILE* fp = ::fopen(s8Filename.string(), "rb");
    if  (fp) {
        ::fread(pBuf, bufsize, 1, fp);
        ::fclose(fp);
    }
}



/******************************************************************************
*
*******************************************************************************/
#endif  //  HAVE_COMMAND_xxx

