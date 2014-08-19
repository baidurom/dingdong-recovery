#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8850raw.h"
#include "camera_info_ov8850raw.h"
#include "camera_custom_AEPlinetable.h"
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
        80650,    // i4R_AVG
        16601,    // i4R_STD
        88575,    // i4B_AVG
        18234,    // i4B_STD
        {  // i4P00[9]
            5070000, -2380000, -127500, -837500, 4082500, -687500, -67500, -2477500, 5105000
        },
        {  // i4P10[9]
            2839740, -2934101, 84573, -431329, -469658, 910297, 106513, 1535044, -1673300
        },
        {  // i4P01[9]
            2522353, -2479441, -52433, -600990, -417886, 1024162, -40114, 684322, -679998
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
        /*
        {
            1152,    // u4MinGain, 1024 base = 1x
            8000,    // u4MaxGain, 16x
            75,    // u4MiniISOGain, ISOxx  
            64,    // u4GainStepUnit, 1x/8 
            17,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            17,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            16,    // u4CapExpUnit 
            25,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            22,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        */
        {
            1152,    // u4MinGain, 1024 base = 1x
            8000,    // u4MaxGain, 16x
            75,    // u4MiniISOGain, ISOxx  
            64,    // u4GainStepUnit, 1x/8 
            27,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            27,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            27,    // u4CapExpUnit 
            14,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            22,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            3, //4,    // u4HistHighThres
            40,    // u4HistLowThres
            1,  //2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {75, 108, 138, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
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
            20, //50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -8, //-10,    // i4BVOffset delta BV = value/10 
            70, //64,    // u4PreviewFlareOffset
            70,//64,    // u4CaptureFlareOffset
            3,    // u4CaptureFlareThres
            70, //64,    // u4VideoFlareOffset
            3,    // u4VideoFlareThres
            70,//64,    // u4StrobeFlareOffset
            3,    // u4StrobeFlareThres
            160,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            160,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            64 //75    // u4FlatnessStrength
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
                1016,    // i4R   //1016---906
                512,    // i4G
                655    // i4B   //655--->685
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                145,    // i4X
                -298    // i4Y
            },
            // Horizon
            {
                -338,    // i4X
                -327    // i4Y
            },
            // A
            {
                -217,    // i4X
                -339    // i4Y
            },
            // TL84
            {
                -59,    // i4X
                -379    // i4Y
            },
            // CWF
            {
                -26,    // i4X
                -411    // i4Y
            },
            // DNP
            {
                34,    // i4X
                -371    // i4Y
            },
            // D65
            {
                162,    // i4X
                -344    // i4Y
            },
            // DF
            {
                123,    // i4X
                -399    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                145,    // i4X
                -298    // i4Y
            },
            // Horizon
            {
                -338,    // i4X
                -327    // i4Y
            },
            // A
            {
                -217,    // i4X
                -339    // i4Y
            },
            // TL84
            {
                -59,    // i4X
                -379    // i4Y
            },
            // CWF
            {
                -26,    // i4X
                -411    // i4Y
            },
            // DNP
            {
                34,    // i4X
                -371    // i4Y
            },
            // D65
            {
                162,    // i4X
                -344    // i4Y
            },
            // DF
            {
                123,    // i4X
                -399    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                932,    // i4R
                512,    // i4G
                630    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                520,    // i4G
                1280    // i4B
            },
            // A 
            {
                603,    // i4R
                512,    // i4G
                1087    // i4B
            },
            // TL84 
            {
                790,    // i4R
                512,    // i4G
                927    // i4B
            },
            // CWF 
            {
                862,    // i4R
                512,    // i4G
                926    // i4B
            },
            // DNP 
            {
                886,    // i4R
                512,    // i4G
                807    // i4B
            },
            // D65 
            {
                1016,    // i4R
                512,    // i4G
                655    // i4B
            },
            // DF 
            {
                1038,    // i4R
                512,    // i4G
                744    // i4B
            }
        },
        // Rotation matrix parameter
        {
            0,    // i4RotationAngle
            256,    // i4Cos
            0    // i4Sin
        },
        // Daylight locus parameter
        {
            -126,    // i4SlopeNumerator
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
            -109,    // i4RightBound
            -759,    // i4LeftBound
            -253,    // i4UpperBound
            -393    // i4LowerBound
            },
            // Warm fluorescent
            {
            -109,    // i4RightBound
            -759,    // i4LeftBound
            -393,    // i4UpperBound
            -503    // i4LowerBound
            },
            // Fluorescent
            {
            -16,    // i4RightBound
            -109,    // i4LeftBound
            -273,    // i4UpperBound
            -395    // i4LowerBound
            },
            // CWF
            {
            -16,    // i4RightBound
            -109,    // i4LeftBound
            -395,    // i4UpperBound
            -461    // i4LowerBound
            },
            // Daylight
            {
            187,    // i4RightBound
            -16,    // i4LeftBound
            -254,    // i4UpperBound
            -434    // i4LowerBound
            },
            // Shade
            {
            547,    // i4RightBound
            187,    // i4LeftBound
            -254,    // i4UpperBound
            -434    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            130,    // i4RightBound
            -16,    // i4LeftBound
            -434,    // i4UpperBound
            -504    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            547,    // i4RightBound
            -759,    // i4LeftBound
            0,    // i4UpperBound
            -504    // i4LowerBound
            },
            // Daylight
            {
            212,    // i4RightBound
            -16,    // i4LeftBound
            -254,    // i4UpperBound
            -434    // i4LowerBound
            },
            // Cloudy daylight
            {
            312,    // i4RightBound
            137,    // i4LeftBound
            -254,    // i4UpperBound
            -434    // i4LowerBound
            },
            // Shade
            {
            412,    // i4RightBound
            137,    // i4LeftBound
            -254,    // i4UpperBound
            -434    // i4LowerBound
            },
            // Twilight
            {
            -16,    // i4RightBound
            -176,    // i4LeftBound
            -254,    // i4UpperBound
            -434    // i4LowerBound
            },
            // Fluorescent
            {
            212,    // i4RightBound
            -159,    // i4LeftBound
            -294,    // i4UpperBound
            -461    // i4LowerBound
            },
            // Warm fluorescent
            {
            -107,    // i4RightBound
            -317,    // i4LeftBound
            -244,    // i4UpperBound
            -411    // i4LowerBound
            },
            // Incandescent
            {
            -107,    // i4RightBound
            -317,    // i4LeftBound
            -224,    // i4UpperBound
            -404    // i4LowerBound
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
            931,    // i4R
            512,    // i4G
            714    // i4B
            },
            // Cloudy daylight
            {
            1105,    // i4R
            512,    // i4G
            602    // i4B
            },
            // Shade
            {
            1183,    // i4R
            512,    // i4G
            563    // i4B
            },
            // Twilight
            {
            716,    // i4R
            512,    // i4G
            929    // i4B
            },
            // Fluorescent
            {
            885,    // i4R
            512,    // i4G
            823    // i4B
            },
            // Warm fluorescent
            {
            599,    // i4R
            512,    // i4G
            1063    // i4B
            },
            // Incandescent
            {
            588,    // i4R
            512,    // i4G
            1044    // i4B
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
            6144    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            4692    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1341    // i4OffsetThr
            },
            // Daylight WB gain
            {
            854,    // i4R
            536,    // i4G
            512    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            520    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            516    // i4B
            },
            // Preference gain: fluorescent
            {
            498,    // i4R
            512,    // i4G
            525    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            525    // i4B
            },
            // Preference gain: daylight
            {
            502, //506 ,  // i4R
            512,    // i4G
            526  //518    // i4B  
            },
            // Preference gain: shade
            {
            504, //506,    // i4R
            512,    // i4G
            524  //518    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            514    // i4B
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
                -500,    // i4RotatedXCoordinate[0]
                -379,    // i4RotatedXCoordinate[1]
                -221,    // i4RotatedXCoordinate[2]
                -128,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace


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
                                             sizeof(AE_PLINETABLE_T)};

    if (CameraDataType > CAMERA_DATA_AE_PLINETABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
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
        default:
            break;
    }
    return 0;
}}; // NSFeature


