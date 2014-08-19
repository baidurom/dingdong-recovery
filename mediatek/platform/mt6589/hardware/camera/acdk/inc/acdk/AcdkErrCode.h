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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////


//! \file  AcdkErrCode.h

#ifndef _ACDKERRCODE_H_
#define _ACDKERRCODE_H_

#include "AcdkTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//! Helper macros to define error code
#define ERRCODE(modid, errid)  ((MINT32)((MUINT32)((modid & 0xff) << 20) | (MUINT32)(errid & 0xff)))

//! Helper macros to define ok code
#define OKCODE(modid, okid)    ((MINT32)((MUINT32)((modid & 0xff) << 20) | (MUINT32)(okid & 0xff)))

//! Helper macros to indicate succeed
#define SUCCEEDED(Status)   ((MINT32)(Status) >= 0)

//! Helper macros to indicate fail
#define FAILED(Status)      ((MINT32)(Status) < 0)


/*********************************************************************************
*
*********************************************************************************/

/**
*@brief Use enum to check duplicated error ID
*/
enum  
{
    MODULE_ACDK_CALIBRATION  = 0x91,    //! module calibration
    MOUDLE_ACDK_IMGTOOL      = 0x92,    //! module imagetool
    MODULE_ACDK_CCAP         = 0x93,    //! module ccap
    MODULE_ACDK_IF           = 0x94     //! module AcdkIF
};  

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define ImageTool ok code
#define ACDK_IMGTOOL_OKCODE(errid)          OKCODE(MOUDLE_ACDK_IMGTOOL, errid)

//! Helper macros to define ImageTool error code
#define ACDK_IMGTOOL_ERRCODE(errid)         ERRCODE(MOUDLE_ACDK_IMGTOOL, errid)

/**  
*@brief Return value of Image Tool
*/
enum 
{
    S_ACDK_IMGTOOL_OK             = ACDK_IMGTOOL_OKCODE(0),
    E_ACDK_IMGTOOL_BAD_ARG        = ACDK_IMGTOOL_ERRCODE(0x0001),   //! bad arguments
    E_ACDK_IMGTOOL_API_FAIL       = ACDK_IMGTOOL_ERRCODE(0x0002),   //! API Fail
    E_ACDK_IMGTOOL_NULL_OBJ       = ACDK_IMGTOOL_ERRCODE(0x0003),   //! Null Obj 
    E_ACDK_IMGTOOL_TIMEOUT        = ACDK_IMGTOOL_ERRCODE(0x0004),   //! Time out 
    E_ACDK_IMGTOOL_FILE_OPEN_FAIL = ACDK_IMGTOOL_ERRCODE(0x0005),   //! Open File Fail
    E_ACDK_IMGTOOL_MEMORY_MAX     = ACDK_IMGTOOL_ERRCODE(0x0080)    //! Max error code     
};

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define calibration ok code
#define ACDK_CALIBRATION_OKCODE(errid)          OKCODE(MODULE_ACDK_CALIBRATION, errid)

//! Helper macros to define calibration error code
#define ACDK_CALIBRATION_ERRCODE(errid)         ERRCODE(MODULE_ACDK_CALIBRATION, errid)

/**  
*@brief Return value of Calibration Tool
*/
enum 
{
    S_ACDK_CALIBRATION_OK               = ACDK_CALIBRATION_OKCODE(0),
    E_ACDK_CALIBRATION_BAD_ARG          = ACDK_CALIBRATION_ERRCODE(0x0001),    //! bad arguments
    E_ACDK_CALIBRATION_API_FAIL         = ACDK_CALIBRATION_ERRCODE(0x0002),    //! API Fail
    E_ACDK_CALIBRATION_NULL_OBJ         = ACDK_CALIBRATION_ERRCODE(0x0003),    //! Null Obj 
    E_ACDK_CALIBRATION_TIMEOUT          = ACDK_CALIBRATION_ERRCODE(0x0004),    //! Time out 
    E_ACDK_CALIBRATION_GET_FAIL         = ACDK_CALIBRATION_ERRCODE(0x0005),    //! Get calibration result fail
    E_ACDK_CALIBRATION_FILE_OPEN_FAIL   = ACDK_CALIBRATION_ERRCODE(0x0006),    //! Open File Fail 
    E_ACDK_CALIBRATION_DISABLE          = ACDK_CALIBRATION_ERRCODE(0x0007),    //! Disable
    E_ACDK_CALIBRATION_MEMORY_MAX       = ACDK_CALIBRATION_ERRCODE(0x0080)     //! Max error code     
};

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define AcdkIF ok code
#define ACDK_IF_OKCODE(errid)   OKCODE(MODULE_ACDK_IF, errid) 

//! Helper macros to define AcdkIF error code 
#define ACDK_IF_ERRCODE(errid)  ERRCODE(MODULE_ACDK_IF, errid) 

/**  
*@brief Return value of AcdkIF
*/
enum 
{
    S_ACDK_IF_OK                             = ACDK_IF_OKCODE(0), 
    E_ACDK_IF_API_FAIL                       = ACDK_IF_ERRCODE(0x0001), 
    E_ACDK_IF_NO_SUPPORT_FOMAT               = ACDK_IF_ERRCODE(0x0002),
    E_ACDK_IF_IS_ACTIVED                     = ACDK_IF_ERRCODE(0x0003), 
    E_ACDK_IF_INVALID_DRIVER_MOD_ID          = ACDK_IF_ERRCODE(0x0004),
    E_ACDK_IF_INVALID_FEATURE_ID             = ACDK_IF_ERRCODE(0x0005), 
    E_ACDK_IF_INVALID_SCENARIO_ID            = ACDK_IF_ERRCODE(0x0006), 
    E_ACDK_IF_INVALID_CTRL_CODE              = ACDK_IF_ERRCODE(0x0007), 
    E_ACDK_IF_VIDEO_ENCODER_BUSY             = ACDK_IF_ERRCODE(0x0008),
    E_ACDK_IF_INVALID_PARA                   = ACDK_IF_ERRCODE(0x0009),
    E_ACDK_IF_OUT_OF_BUFFER_NUMBER           = ACDK_IF_ERRCODE(0x000A),
    E_ACDK_IF_INVALID_ISP_STATE              = ACDK_IF_ERRCODE(0x000B),
    E_ACDK_IF_INVALID_ACDK_STATE             = ACDK_IF_ERRCODE(0x000C),
    E_ACDK_IF_PHY_VIR_MEM_MAP_FAIL           = ACDK_IF_ERRCODE(0x000D),
    E_ACDK_IF_ENQUEUE_BUFFER_NOT_FOUND       = ACDK_IF_ERRCODE(0x000E),
    E_ACDK_IF_BUFFER_ALREADY_INIT            = ACDK_IF_ERRCODE(0x000F),
    E_ACDK_IF_BUFFER_OUT_OF_MEMORY           = ACDK_IF_ERRCODE(0x0010),
    E_ACDK_IF_SENSOR_POWER_ON_FAIL           = ACDK_IF_ERRCODE(0x0011),
    E_ACDK_IF_SENSOR_CONNECT_FAIL            = ACDK_IF_ERRCODE(0x0012),
    E_ACDK_IF_IO_CONTROL_CODE                = ACDK_IF_ERRCODE(0x0013),
    E_ACDK_IF_IO_CONTROL_MSG_QUEUE_OPEN_FAIL = ACDK_IF_ERRCODE(0x0014),
    E_ACDK_IF_DRIVER_INIT_FAIL               = ACDK_IF_ERRCODE(0x0015),
    E_ACDK_IF_WRONG_NVRAM_CAMERA_VERSION     = ACDK_IF_ERRCODE(0x0016),
    E_ACDK_IF_NVRAM_CAMERA_FILE_FAIL         = ACDK_IF_ERRCODE(0x0017),
    E_ACDK_IF_IMAGE_DECODE_FAIL              = ACDK_IF_ERRCODE(0x0018),
    E_ACDK_IF_IMAGE_ENCODE_FAIL              = ACDK_IF_ERRCODE(0x0019),
    E_ACDK_IF_LED_FLASH_POWER_ON_FAIL        = ACDK_IF_ERRCODE(0x001A)
};

/*********************************************************************************
*
*********************************************************************************/

//! Helper macros to define CCAP ok code
#define ACDK_CCAP_OKCODE(errid)   OKCODE(MODULE_ACDK_CCAP, errid)

//! Helper macros to define CCAP error code 
#define ACDK_CCAP_ERRCODE(errid)  ERRCODE(MODULE_ACDK_CCAP, errid)

/**  
*@brief Return value of CCAP
*/
enum 
{
    S_ACDK_CCAP_OK       = ACDK_CCAP_OKCODE(0), 
    E_ACDK_CCAP_API_FAIL = ACDK_CCAP_ERRCODE(0x0001), 
};

/*********************************************************************************
*
*********************************************************************************/

/**  
*@enum ACDK_ERROR_ENUM_S
*@brief Return value of ACDK
*/
typedef enum ACDK_ERROR_ENUM_S
{
    ACDK_RETURN_NO_ERROR       = 0, //! no error
    ACDK_RETURN_INVALID_DRIVER,     //! invalid driver object
    ACDK_RETURN_API_FAIL,           //! api fail
    ACDK_RETURN_INVALID_PARA,       //! invalid parameter
    ACDK_RETURN_NULL_OBJ,           //! null object
    ACDK_RETURN_INVALID_SENSOR,     //! invalid sensor object
    ACDK_RETURN_MEMORY_ERROR,       //! memory error
    ACDK_RETURN_ERROR_STATE,        //! error state
    ACDK_RETURN_UNKNOWN_ERROR       //! unknown error
} ACDK_ERROR_ENUM_T;

#ifdef __cplusplus
}
#endif

#endif //end _ACDKERRCODE_H_

