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
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx111mipiraw.h"
#include "camera_info_imx111mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"  //liukun@wind-mobi.com 20131223
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        78550,    // i4R_AVG
        17897,    // i4R_STD
        103250,    // i4B_AVG
        26059,    // i4B_STD
        {  // i4P00[9]
            4995000, -2692500, 257500, -680000, 3417500, -180000, 70000, -1902500, 4395000
        },
        {  // i4P10[9]
            603026, -940985, 337844, 263644, -371750, 103249, 126061, -327200, 189079
        },
        {  // i4P01[9]
            142045, -434637, 295494, 62253, -378490, 316068, 45132, -857640, 801176
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1268,    // u4MinGain, 1024 base = 1x
            16384,    // u4MaxGain, 16x
            27,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            27,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            30,    // u4VideoExpUnit  
            18,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            27,    // u4CapExpUnit 
            15,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            2,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {86, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            47,    // u4AETarget
            47,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -10,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            8,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            8,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                915,    // i4R
                512,    // i4G
                679    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                110,    // i4X
                -319    // i4Y
            },
            // Horizon
            {
                -401,    // i4X
                -423    // i4Y
            },
            // A
            {
                -278,    // i4X
                -420    // i4Y
            },
            // TL84
            {
                -165,    // i4X
                -362    // i4Y
            },
            // CWF
            {
                -123,    // i4X
                -417    // i4Y
            },
            // DNP
            {
                -35,    // i4X
                -374    // i4Y
            },
            // D65
            {
                110,    // i4X
                -319    // i4Y
            },
            // DF
            {
                50,    // i4X
                -377    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                41,    // i4X
                -334    // i4Y
            },
            // Horizon
            {
                -479,    // i4X
                -330    // i4Y
            },
            // A
            {
                -358,    // i4X
                -353    // i4Y
            },
            // TL84
            {
                -236,    // i4X
                -319    // i4Y
            },
            // CWF
            {
                -206,    // i4X
                -382    // i4Y
            },
            // DNP
            {
                -112,    // i4X
                -358    // i4Y
            },
            // D65
            {
                41,    // i4X
                -334    // i4Y
            },
            // DF
            {
                -29,    // i4X
                -379    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                915,    // i4R
                512,    // i4G
                679    // i4B
            },
            // Horizon 
            {
                527,    // i4R
                512,    // i4G
                1561    // i4B
            },
            // A 
            {
                621,    // i4R
                512,    // i4G
                1318    // i4B
            },
            // TL84 
            {
                668,    // i4R
                512,    // i4G
                1046    // i4B
            },
            // CWF 
            {
                763,    // i4R
                512,    // i4G
                1064    // i4B
            },
            // DNP 
            {
                809,    // i4R
                512,    // i4G
                891    // i4B
            },
            // D65 
            {
                915,    // i4R
                512,    // i4G
                679    // i4B
            },
            // DF 
            {
                913,    // i4R
                512,    // i4G
                798    // i4B
            }
        },
        // Rotation matrix parameter
        {
            12,    // i4RotationAngle
            250,    // i4Cos
            53    // i4Sin
        },
        // Daylight locus parameter
        {
            -197,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -206,    // i4RightBound
            -936,    // i4LeftBound
            -291,    // i4UpperBound
            -391    // i4LowerBound
            },
            // Warm fluorescent
            {
            -206,    // i4RightBound
            -936,    // i4LeftBound
            -391,    // i4UpperBound
            -511    // i4LowerBound
            },
            // Fluorescent
            {
            -82,    // i4RightBound
            -206,    // i4LeftBound
            -290,    // i4UpperBound
            -370    // i4LowerBound
            },
            // CWF
            {
            -82,    // i4RightBound
            -206,    // i4LeftBound
            -370,    // i4UpperBound
            -432    // i4LowerBound
            },
            // Daylight
            {
            106,    // i4RightBound
            -82,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Shade
            {
            426,    // i4RightBound
            106,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            106,    // i4RightBound
            -82,    // i4LeftBound
            -414,    // i4UpperBound
            -490    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            426,    // i4RightBound
            -936,    // i4LeftBound
            0,    // i4UpperBound
            -511    // i4LowerBound
            },
            // Daylight
            {
            131,    // i4RightBound
            -82,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Cloudy daylight
            {
            231,    // i4RightBound
            56,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Shade
            {
            331,    // i4RightBound
            56,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Twilight
            {
            -82,    // i4RightBound
            -242,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Fluorescent
            {
            91,    // i4RightBound
            -336,    // i4LeftBound
            -269,    // i4UpperBound
            -432    // i4LowerBound
            },
            // Warm fluorescent
            {
            -258,    // i4RightBound
            -458,    // i4LeftBound
            -269,    // i4UpperBound
            -432    // i4LowerBound
            },
            // Incandescent
            {
            -258,    // i4RightBound
            -458,    // i4LeftBound
            -290,    // i4UpperBound
            -414    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            925,    // i4R
            512,    // i4G
            711    // i4B
            },
            // Cloudy daylight
            {
            1048,    // i4R
            512,    // i4G
            587    // i4B
            },
            // Shade
            {
            1104,    // i4R
            512,    // i4G
            542    // i4B
            },
            // Twilight
            {
            761,    // i4R
            512,    // i4G
            960    // i4B
            },
            // Fluorescent
            {
            791,    // i4R
            512,    // i4G
            899    // i4B
            },
            // Warm fluorescent
            {
            619,    // i4R
            512,    // i4G
            1313    // i4B
            },
            // Incandescent
            {
            620,    // i4R
            512,    // i4G
            1315    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            6970    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            4441    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1906    // i4OffsetThr
            },
            // Daylight WB gain
            {
            829,    // i4R
            512,    // i4G
            769    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            500,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            500    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -520,    // i4RotatedXCoordinate[0]
                -399,    // i4RotatedXCoordinate[1]
                -277,    // i4RotatedXCoordinate[2]
                -153,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace
/* liukun@wind-mobi.com 20131223 begin */
const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE = 
{ 
#include INCLUDE_FILENAME_TSF_PARA 
#include INCLUDE_FILENAME_TSF_DATA 
};
/* liukun@wind-mobi.com 20131223 end */

typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T),  //liukun@wind-mobi.com 20131223
                                             0,  //liukun@wind-mobi.com 20131223
                                             sizeof(CAMERA_TSF_TBL_STRUCT)}; //liukun@wind-mobi.com 20131223

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType])) //liukun@wind-mobi.com 20131223
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
/* liukun@wind-mobi.com 20131223 begin */
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
/* liukun@wind-mobi.com 20131223 end */
        default:
            break;
    }
    return 0;
}};  //  NSFeature


