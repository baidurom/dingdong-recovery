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
*  MERCHANTABI LITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
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
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   CMOS sensor header file
 *
 ****************************************************************************/
#ifndef _OV8850_SENSOR_H
#define _OV8850_SENSOR_H

#define OV8850_DEBUG
#define OV8850_DRIVER_TRACE
//#define OV8850_TEST_PATTEM
#ifdef OV8850_DEBUG
//#define SENSORDB printk
#else
//#define SENSORDB(x,...)
#endif

//#define OV8850_2_LANE  // if you use 2 lane setting on MT6589, please define it
#define OV8850_FACTORY_START_ADDR 0
#define OV8850_ENGINEER_START_ADDR 10

//#define MIPI_INTERFACE

 
typedef enum OV8850_group_enum
{
  OV8850_PRE_GAIN = 0,
  OV8850_CMMCLK_CURRENT,
  OV8850_FRAME_RATE_LIMITATION,
  OV8850_REGISTER_EDITOR,
  OV8850_GROUP_TOTAL_NUMS
} OV8850_FACTORY_GROUP_ENUM;

typedef enum OV8850_register_index
{
  OV8850_SENSOR_BASEGAIN = OV8850_FACTORY_START_ADDR,
  OV8850_PRE_GAIN_R_INDEX,
  OV8850_PRE_GAIN_Gr_INDEX,
  OV8850_PRE_GAIN_Gb_INDEX,
  OV8850_PRE_GAIN_B_INDEX,
  OV8850_FACTORY_END_ADDR
} OV8850_FACTORY_REGISTER_INDEX;

typedef enum OV8850_engineer_index
{
  OV8850_CMMCLK_CURRENT_INDEX = OV8850_ENGINEER_START_ADDR,
  OV8850_ENGINEER_END
} OV8850_FACTORY_ENGINEER_INDEX;

typedef struct _sensor_data_struct
{
  SENSOR_REG_STRUCT reg[OV8850_ENGINEER_END];
  SENSOR_REG_STRUCT cct[OV8850_FACTORY_END_ADDR];
} sensor_data_struct;


#define OV8850_COLOR_FORMAT                    SENSOR_OUTPUT_FORMAT_RAW_B

#define OV8850_MIN_ANALOG_GAIN  1   /* 1x */
#define OV8850_MAX_ANALOG_GAIN      32 /* 32x */


/* FRAME RATE UNIT */
#define OV8850_FPS(x)                          (10 * (x))

#ifdef OV8850_2_LANE

#define OV8850_PREVIEW_CLK   136000000
#define OV8850_CAPTURE_CLK   132000000
#define OV8850_VIDEO_CLK     136000000
#define OV8850_ZSD_PRE_CLK   132000000

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
#define OV8850_FULL_PERIOD_PIXEL_NUMS          3608  //  25fps
#define OV8850_FULL_PERIOD_LINE_NUMS           2512   //

#define OV8850_PV_PERIOD_PIXEL_NUMS            3608 //
#define OV8850_PV_PERIOD_LINE_NUMS             1256 //

#define OV8850_VIDEO_PERIOD_PIXEL_NUMS         3608  //
#define OV8850_VIDEO_PERIOD_LINE_NUMS          1256 //1920  //

#define OV8850_3D_FULL_PERIOD_PIXEL_NUMS       3608 /* 15 fps */
#define OV8850_3D_FULL_PERIOD_LINE_NUMS        2512
#define OV8850_3D_PV_PERIOD_PIXEL_NUMS         3608 /* 30 fps */
#define OV8850_3D_PV_PERIOD_LINE_NUMS          1256
#define OV8850_3D_VIDEO_PERIOD_PIXEL_NUMS      3608 /* 30 fps */
#define OV8850_3D_VIDEO_PERIOD_LINE_NUMS       1920
/* SENSOR START/END POSITION */
#define OV8850_FULL_X_START                    10
#define OV8850_FULL_Y_START                    10
#define OV8850_IMAGE_SENSOR_FULL_WIDTH         (3264 - 64) /* 2560 */
#define OV8850_IMAGE_SENSOR_FULL_HEIGHT        (2448 - 48) /* 1920 */

#define OV8850_PV_X_START                      2
#define OV8850_PV_Y_START                      2
#define OV8850_IMAGE_SENSOR_PV_WIDTH           (1600)
#define OV8850_IMAGE_SENSOR_PV_HEIGHT          (1200)

#define OV8850_VIDEO_X_START                   2 //9
#define OV8850_VIDEO_Y_START                   2 //11
#define OV8850_IMAGE_SENSOR_VIDEO_WIDTH        1600 //1920 //(3264 - 64) /* 1264 */
#define OV8850_IMAGE_SENSOR_VIDEO_HEIGHT       1200 //1080 //(1836 - 48) /* 948 */

#define OV8850_3D_FULL_X_START                 10   //(1+16+6)
#define OV8850_3D_FULL_Y_START                 10  //(1+12+4)
#define OV8850_IMAGE_SENSOR_3D_FULL_WIDTH      (3264 - 64) //(2592 - 16) /* 2560 */
#define OV8850_IMAGE_SENSOR_3D_FULL_HEIGHT     (2448 - 48) //(1944 - 12) /* 1920 */
#define OV8850_3D_PV_X_START                   2
#define OV8850_3D_PV_Y_START                   2
#define OV8850_IMAGE_SENSOR_3D_PV_WIDTH        (1600) /* 1600 */
#define OV8850_IMAGE_SENSOR_3D_PV_HEIGHT       (1200) /* 1200 */
#define OV8850_3D_VIDEO_X_START                2
#define OV8850_3D_VIDEO_Y_START                2
#define OV8850_IMAGE_SENSOR_3D_VIDEO_WIDTH     (1600) /* 1600 */
#define OV8850_IMAGE_SENSOR_3D_VIDEO_HEIGHT    (1200) /* 1200 */

#else

#define OV8850_PREVIEW_CLK   216000000
#define OV8850_CAPTURE_CLK   228000000
#define OV8850_VIDEO_CLK     216000000
#define OV8850_ZSD_PRE_CLK   228000000

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
#define OV8850_FULL_PERIOD_PIXEL_NUMS          3608  //  25fps
#define OV8850_FULL_PERIOD_LINE_NUMS           2522   //

#define OV8850_PV_PERIOD_PIXEL_NUMS            3608 //
#define OV8850_PV_PERIOD_LINE_NUMS             1996 //

#define OV8850_VIDEO_PERIOD_PIXEL_NUMS         3608  //
#define OV8850_VIDEO_PERIOD_LINE_NUMS          1996  //

#define OV8850_3D_FULL_PERIOD_PIXEL_NUMS       3608 /* 15 fps */
#define OV8850_3D_FULL_PERIOD_LINE_NUMS        2522
#define OV8850_3D_PV_PERIOD_PIXEL_NUMS         3608 /* 30 fps */
#define OV8850_3D_PV_PERIOD_LINE_NUMS          1996
#define OV8850_3D_VIDEO_PERIOD_PIXEL_NUMS      3608 /* 30 fps */
#define OV8850_3D_VIDEO_PERIOD_LINE_NUMS       2042
/* SENSOR START/END POSITION */
#define OV8850_FULL_X_START                    10
#define OV8850_FULL_Y_START                    10
#define OV8850_IMAGE_SENSOR_FULL_WIDTH         (3264 - 64) /* 2560 */
#define OV8850_IMAGE_SENSOR_FULL_HEIGHT        (2448 - 48) /* 1920 */

#define OV8850_PV_X_START                      2
#define OV8850_PV_Y_START                      2
#define OV8850_IMAGE_SENSOR_PV_WIDTH           (1600)
#define OV8850_IMAGE_SENSOR_PV_HEIGHT          (1200)

#define OV8850_VIDEO_X_START                   9
#define OV8850_VIDEO_Y_START                   11
#define OV8850_IMAGE_SENSOR_VIDEO_WIDTH        (3264 - 64) /* 1264 */
#define OV8850_IMAGE_SENSOR_VIDEO_HEIGHT       (1836 - 48) /* 948 */

#define OV8850_3D_FULL_X_START                 10   //(1+16+6)
#define OV8850_3D_FULL_Y_START                 10  //(1+12+4)
#define OV8850_IMAGE_SENSOR_3D_FULL_WIDTH      (3264 - 64) //(2592 - 16) /* 2560 */
#define OV8850_IMAGE_SENSOR_3D_FULL_HEIGHT     (2448 - 48) //(1944 - 12) /* 1920 */
#define OV8850_3D_PV_X_START                   2
#define OV8850_3D_PV_Y_START                   2
#define OV8850_IMAGE_SENSOR_3D_PV_WIDTH        (1600) /* 1600 */
#define OV8850_IMAGE_SENSOR_3D_PV_HEIGHT       (1200) /* 1200 */
#define OV8850_3D_VIDEO_X_START                2
#define OV8850_3D_VIDEO_Y_START                2
#define OV8850_IMAGE_SENSOR_3D_VIDEO_WIDTH     (1600) /* 1600 */
#define OV8850_IMAGE_SENSOR_3D_VIDEO_HEIGHT    (1200) /* 1200 */

#endif

/* SENSOR READ/WRITE ID */

#define OV8850_SLAVE_WRITE_ID_1   (0x6c)
#define OV8850_SLAVE_WRITE_ID_2   (0x20)
/************OTP Feature*********************/
//#define OV8850_USE_OTP

#if defined(OV8850_USE_OTP)

#endif
/************OTP Feature*********************/

/* SENSOR PRIVATE STRUCT */
typedef struct OV8850_sensor_STRUCT
{
  MSDK_SENSOR_CONFIG_STRUCT cfg_data;
  sensor_data_struct eng; /* engineer mode */
  MSDK_SENSOR_ENG_INFO_STRUCT eng_info;
  kal_uint8 mirror;
  kal_bool pv_mode;
  kal_bool video_mode;  
  //kal_bool NightMode;
  kal_bool is_zsd;
  kal_bool is_zsd_cap;
  kal_bool is_autofliker;
  //kal_uint16 normal_fps; /* video normal mode max fps */
  //kal_uint16 night_fps; /* video night mode max fps */  
  kal_uint16 FixedFps;
  kal_uint16 shutter;
  kal_uint16 gain;
  kal_uint32 pv_pclk;
  kal_uint32 cap_pclk;
  kal_uint32 pclk;
  kal_uint16 frame_height;
  kal_uint16 line_length;  
  kal_uint16 write_id;
  kal_uint16 read_id;
  kal_uint16 dummy_pixels;
  kal_uint16 dummy_lines;
} OV8850_sensor_struct;

//export functions
UINT32 OV8850Open(void);
UINT32 OV8850Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV8850FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV8850GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV8850GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV8850Close(void);

#define Sleep(ms) mdelay(ms)

#endif 
