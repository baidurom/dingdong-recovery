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

#ifndef _AAA_HAL_BASE_H_
#define _AAA_HAL_BASE_H_

class IBaseCamExif;

namespace NS3A
{

enum ESensorType_T {
    ESensorType_RAW = 0,
    ESensorType_YUV = 1
};

enum ECmd_T {
     ECmd_CameraPreviewStart,
     ECmd_CameraPreviewEnd,
     ECmd_CamcorderPreviewStart,
     ECmd_CamcorderPreviewEnd,
     ECmd_PrecaptureStart,
     ECmd_PrecaptureEnd,
     ECmd_CaptureStart,
     ECmd_CaptureEnd,
     ECmd_RecordingStart,
     ECmd_RecordingEnd,
     ECmd_Update,
     // internal command
     ECmd_Init,
     ECmd_Uninit,
     ECmd_AFUpdate, // sync with AF done
     ECmd_AFStart,
     ECmd_AFEnd

};

enum EQueryType_T{
    EQueryType_Init,
    EQueryType_Effect,
    EQueryType_AWB
};

enum EIspProfile_T
{
    //  NORMAL
    EIspProfile_NormalPreview              = 0,
    EIspProfile_ZsdPreview_CC,
    EIspProfile_ZsdPreview_NCC,
    EIspProfile_NormalCapture,
    EIspProfile_VideoPreview,
    EIspProfile_VideoCapture,
    //  MF
    EIspProfile_MFCapPass1,
    EIspProfile_MFCapPass2,
    EIspProfile_NUM
};

struct FrameOutputParam_T
{
    MUINT32 u4FRameRate_x10;     // 10 base frame rate
    MINT32 i4BrightValue_x10;       // 10 base brightness value
    MINT32 i4LightValue_x10;       // 10 base lumince value
};


struct Hal3A_HDROutputParam_T
{
    MUINT32 u4OutputFrameNum;     // Output frame number (2 or 3)
    MUINT32 u4FinalGainDiff[2];   // 1x=1024; [0]: Between short exposure and 0EV; [1]: Between 0EV and long exposure
    MUINT32 u4TargetTone; //Decide the curve to decide target tone
};

struct CaptureParam_T
{
	MUINT32 u4ExposureMode;  //0: exp. time, 1: exp. line
    MUINT32 u4Eposuretime;   //!<: Exposure time in us
    MUINT32 u4AfeGain;           //!<: sensor gain
    MUINT32 u4IspGain;           //!<: raw gain
    MUINT32 u4RealISO;           //!<: Real ISO speed
    MUINT32  u4FlareOffset;
    MUINT32  u4FlareGain;   // 512 is 1x
};

struct FeatureParam_T {
     MBOOL   bExposureLockSupported;
     MBOOL   bAutoWhiteBalanceLockSupported;
     MUINT32 u4MaxFocusAreaNum;
     MINT32  i4MaxLensPos;
     MINT32  i4MinLensPos;
     MINT32  i4AFBestPos;
     MINT64  i8BSSVlu;
     MUINT32 u4MaxMeterAreaNum;
};

struct CameraArea_T {
    MINT32 i4Left;
    MINT32 i4Top;
    MINT32 i4Right;
    MINT32 i4Bottom;
    MINT32 i4Weight;
};

#define MAX_FOCUS_AREAS  9

struct CameraFocusArea_T {
	CameraArea_T rAreas[MAX_FOCUS_AREAS];
	MUINT32 u4Count;
};

#define MAX_METERING_AREAS 9

struct CameraMeteringArea_T {
	CameraArea_T rAreas[MAX_METERING_AREAS];
	MUINT32 u4Count;
};

// 3A ASD info
struct ASDInfo_T {
    MINT32 i4AELv_x10;          // AE Lv
    MBOOL  bAEBacklit;          // AE backlit condition
    MBOOL  bAEStable;           // AE stable
    MINT16 i2AEFaceDiffIndex;   // Face AE difference index with central weighting
    MINT32 i4AWBRgain_X128;     // AWB Rgain
    MINT32 i4AWBBgain_X128;     // AWB Bgain
    MINT32 i4AWBRgain_D65_X128; // AWB Rgain (D65; golden sample)
    MINT32 i4AWBBgain_D65_X128; // AWB Bgain (D65; golden sample)
    MINT32 i4AWBRgain_CWF_X128; // AWB Rgain (CWF; golden sample)
    MINT32 i4AWBBgain_CWF_X128; // AWB Bgain (CWF; golden sample)
    MBOOL  bAWBStable;          // AWB stable
    MINT32 i4AFPos;             // AF position
    MVOID  *pAFTable;           // Pointer to AF table
    MINT32 i4AFTableOffset;     // AF table offset
    MINT32 i4AFTableMacroIdx;   // AF table macro index
    MINT32 i4AFTableIdxNum;     // AF table total index number
    MBOOL  bAFStable;           // AF stable
};

// LCE Info
struct LCEInfo_T {
    MINT32 i4NormalAEidx;    // gain >= 4x AE Pline table index at 30fps
    MINT32 i4LowlightAEidx;  // gain max AE Pline table index at 30fps
    MINT32 i4AEidxCur;          // AE current frame Pline table index
    MINT32 i4AEidxNext;        // AE next frame Pline table index
};

// 3A parameters
struct Param_T {

    // DEFAULT DEFINITION CATEGORY ( ordered by SDK )
    MINT32 i4MinFps;
    MINT32 i4MaxFps;
    MUINT32 u4AfMode;
    MUINT32 u4AwbMode;
    MUINT32 u4EffectMode;
    MUINT32 u4AntiBandingMode;
    MUINT32 u4SceneMode;
    MUINT32 u4StrobeMode;
    MINT32  i4ExpIndex;
    MFLOAT  fExpCompStep;
    MBOOL  bIsAELock;
    MBOOL  bIsAWBLock;

    MINT32  i4FullScanStep;
    MINT32  i4MFPos;
    CameraFocusArea_T       rFocusAreas;
    CameraMeteringArea_T    rMeteringAreas;

    // MTK DEFINITION CATEGORY
    MUINT32 u4AeMode;
    MUINT32 u4IsoSpeedMode;
    MUINT32 u4AfLampMode;
    //
    MUINT32 u4BrightnessMode;
    MUINT32 u4HueMode;
    MUINT32 u4SaturationMode;
    MUINT32 u4EdgeMode;
    MUINT32 u4ContrastMode;
    MINT32  i4RotateDegree;

    // NEWLY-ADDED CATEGORY
    MUINT32      u4CamMode;   //Photo, Video, ZSD, ENG mode
    MUINT32      u4ShotMode;

    //flash for engineer mode
    MINT32  i4PreFlashDuty;
    MINT32  i4PreFlashStep;
    MINT32  i4MainFlashDuty;
    MINT32  i4MainFlashStep;


    Param_T()
        : i4MinFps(0)
        , i4MaxFps(0)
        , u4AfMode(0)
        , u4AwbMode(1)
        , u4EffectMode(0)
        , u4AntiBandingMode(0)
        , u4SceneMode(0)
        , u4StrobeMode(0)
        , i4ExpIndex(0)
        , fExpCompStep(0)
        , bIsAELock(MFALSE)
        , bIsAWBLock(MFALSE)
        , i4FullScanStep(1)
        , i4MFPos(0)
        , rFocusAreas()
        , rMeteringAreas()
        , u4AeMode(0)
        , u4IsoSpeedMode(0)
        , u4AfLampMode(0)
        , u4BrightnessMode(0)
        , u4HueMode(0)
        , u4SaturationMode(0)
        , u4EdgeMode(0)
        , u4ContrastMode(0)
        , u4CamMode(0)
        , u4ShotMode(0)
        , i4PreFlashDuty(-1)
        , i4PreFlashStep(-1)
        , i4MainFlashDuty(-1)
        , i4MainFlashStep(-1)
    {}
};



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class I3ACallBack {

public:

    virtual             ~I3ACallBack() {}

public:

    virtual void        doNotifyCb (
                           int32_t _msgType,
                           int32_t _ext1,
                           int32_t _ext2,
                           int32_t _ext3
                        ) = 0;

    virtual void        doDataCb (
                           int32_t _msgType,
                           void*   _data,
                           uint32_t _size
                        ) = 0;
public:

enum ECallBack_T
{
    eID_NOTIFY_AF_FOCUSED,
    eID_NOTIFY_AF_MOVING,
    eID_DATA_AF_FOCUSED,
};

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Hal3ABase {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3ABase() {}
    virtual ~Hal3ABase() {}

private: // disable copy constructor and copy assignment operator
    Hal3ABase(const Hal3ABase&);
    Hal3ABase& operator=(const Hal3ABase&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3ABase* createInstance(MINT32 const i4SensorDevId);
    virtual MVOID destroyInstance() {}
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINT32 const i4Arg = 0) {return MTRUE;}
    virtual MINT32 getErrorCode() const {return 0;}
    virtual MBOOL getParams(Param_T &rParam) const {return MTRUE;}
    virtual MBOOL setParams(Param_T const &rNewParam) {return MTRUE;}
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam) {return MTRUE;}
    virtual MBOOL isReadyToCapture() const {return MTRUE;}
    virtual MBOOL autoFocus() {return MTRUE;}
    virtual MBOOL cancelAutoFocus() {return MTRUE;}
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height) {return MTRUE;}
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const {return MTRUE;}
    virtual MBOOL setDebugInfo(IBaseCamExif *pIBaseCamExif) const {return MTRUE;}
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const {return 0;}
    virtual MBOOL setCallbacks(I3ACallBack* cb) {return MTRUE;}
    virtual MBOOL setIspProfile(EIspProfile_T IspProfile) {return MTRUE;}
    virtual MINT32 getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo) {return 0;}
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}
    virtual MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo) {return 0;}
    virtual MBOOL setFDInfo(MVOID* a_sFaces) {return MTRUE;}
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo) {return 0;}
    virtual MINT32 isNeedFiringFlash() {return 0;}
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo) {return MTRUE;}
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo) {return MTRUE;}
    virtual MVOID endContinuousShotJobs() {}
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter) {return 0;}


#if 0
    virtual MBOOL getFocusData (vector<T.B.D> &rData) const = 0;
    // FD
    // EXIF (T.B.D)

#endif
};

}; // namespace NS3A

#endif

