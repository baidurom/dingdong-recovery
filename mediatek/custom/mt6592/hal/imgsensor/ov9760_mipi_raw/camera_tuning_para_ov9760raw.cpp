#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov9760raw.h"
#include "camera_info_ov9760raw.h"
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
        68325,    // i4R_AVG
        15383,    // i4R_STD
        90925,    // i4B_AVG
        20708,    // i4B_STD
        {  // i4P00[9]
            4230000, -1687500, 15000, -282500, 2987500, -140000, 230000, -2107500, 4440000
        },
        {  // i4P10[9]
            1075313, -1165531, 89848, 14483, -91999, 89732, 48124, 344515, -392270
        },
        {  // i4P01[9]
            714490, -797947, 87495, -164654, -177553, 349884, -32550, -264249, 292761
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
            1136,    // u4MinGain, 1024 base = 1x
            16384,    // u4MaxGain, 16x
            140,    // u4MiniISOGain, ISOxx  
            32,    // u4GainStepUnit, 1x/8 
            28934,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            28934,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            28934,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
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
            {75, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
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
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            0,    // u4HistStretchStrengthIndex
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
                880,    // i4R
                512,    // i4G
                666    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                103,    // i4X
                -297    // i4Y
            },
            // Horizon
            {
                -376,    // i4X
                -267    // i4Y
            },
            // A
            {
                -316,    // i4X
                -326    // i4Y
            },
            // TL84
            {
                -177,    // i4X
                -316    // i4Y
            },
            // CWF
            {
                -133,    // i4X
                -436    // i4Y
            },
            // DNP
            {
                -40,    // i4X
                -339    // i4Y
            },
            // D65
            {
                103,    // i4X
                -297    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                103,    // i4X
                -297    // i4Y
            },
            // Horizon
            {
                -376,    // i4X
                -267    // i4Y
            },
            // A
            {
                -316,    // i4X
                -326    // i4Y
            },
            // TL84
            {
                -177,    // i4X
                -316    // i4Y
            },
            // CWF
            {
                -133,    // i4X
                -436    // i4Y
            },
            // DNP
            {
                -40,    // i4X
                -339    // i4Y
            },
            // D65
            {
                103,    // i4X
                -297    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                880,    // i4R
                512,    // i4G
                666    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                593,    // i4G
                1417    // i4B
            },
            // A 
            {
                519,    // i4R
                512,    // i4G
                1221    // i4B
            },
            // TL84 
            {
                618,    // i4R
                512,    // i4G
                998    // i4B
            },
            // CWF 
            {
                772,    // i4R
                512,    // i4G
                1106    // i4B
            },
            // DNP 
            {
                767,    // i4R
                512,    // i4G
                856    // i4B
            },
            // D65 
            {
                880,    // i4R
                512,    // i4G
                666    // i4B
            },
            // DF 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
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
            -129,    // i4SlopeNumerator
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
            -227,    // i4RightBound
            -877,    // i4LeftBound
            -242,    // i4UpperBound
            -351    // i4LowerBound
            },
            // Warm fluorescent
            {
            -227,    // i4RightBound
            -877,    // i4LeftBound
            -351,    // i4UpperBound
            -471    // i4LowerBound
            },
            // Fluorescent
            {
            -90,    // i4RightBound
            -227,    // i4LeftBound
            -229,    // i4UpperBound
            -376    // i4LowerBound
            },
            // CWF
            {
            -90,    // i4RightBound
            -227,    // i4LeftBound
            -376,    // i4UpperBound
            -486    // i4LowerBound
            },
            // Daylight
            {
            128,    // i4RightBound
            -90,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
            },
            // Shade
            {
            488,    // i4RightBound
            128,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            488,    // i4RightBound
            -877,    // i4LeftBound
            -192,    // i4UpperBound
            -486    // i4LowerBound
            },
            // Daylight
            {
            153,    // i4RightBound
            -90,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
            },
            // Cloudy daylight
            {
            253,    // i4RightBound
            78,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
            },
            // Shade
            {
            353,    // i4RightBound
            78,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
            },
            // Twilight
            {
            -90,    // i4RightBound
            -250,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
            },
            // Fluorescent
            {
            153,    // i4RightBound
            -277,    // i4LeftBound
            -247,    // i4UpperBound
            -486    // i4LowerBound
            },
            // Warm fluorescent
            {
            -216,    // i4RightBound
            -416,    // i4LeftBound
            -247,    // i4UpperBound
            -486    // i4LowerBound
            },
            // Incandescent
            {
            -216,    // i4RightBound
            -416,    // i4LeftBound
            -217,    // i4UpperBound
            -377    // i4LowerBound
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
            799,    // i4R
            512,    // i4G
            733    // i4B
            },
            // Cloudy daylight
            {
            958,    // i4R
            512,    // i4G
            612    // i4B
            },
            // Shade
            {
            1025,    // i4R
            512,    // i4G
            572    // i4B
            },
            // Twilight
            {
            608,    // i4R
            512,    // i4G
            963    // i4B
            },
            // Fluorescent
            {
            773,    // i4R
            512,    // i4G
            915    // i4B
            },
            // Warm fluorescent
            {
            548,    // i4R
            512,    // i4G
            1290    // i4B
            },
            // Incandescent
            {
            499,    // i4R
            512,    // i4G
            1174    // i4B
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
            50,    // i4SliderValue
            4496    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            50,    // i4SliderValue
            4496    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            841    // i4OffsetThr
            },
            // Daylight WB gain
            {
            725,    // i4R
            512,    // i4G
            808    // i4B
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
            512    // i4B
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
                -479,    // i4RotatedXCoordinate[0]
                -419,    // i4RotatedXCoordinate[1]
                -280,    // i4RotatedXCoordinate[2]
                -143,    // i4RotatedXCoordinate[3]
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


