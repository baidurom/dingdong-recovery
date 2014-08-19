/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
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
 *   CFG_file_info_custom.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   Configuration File List for Customer
 *
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 ****************************************************************************/

#ifndef __CFG_FILE_INFO_CUSTOM_H__
#define __CFG_FILE_INFO_CUSTOM_H__

#include "CFG_file_public.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"
#include "../../../common/cgen/cfgfileinc/CFG_AUDIO_File.h"
#include "../../../common/cgen/cfgdefault/CFG_Audio_Default.h"
#include "../cfgfileinc/CFG_GPS_File.h"
#include "../cfgdefault/CFG_GPS_Default.h"
#include "../cfgfileinc/CFG_Wifi_File.h"
#include "../cfgdefault/CFG_WIFI_Default.h"
#include "../cfgfileinc/CFG_PRODUCT_INFO_File.h"
#include "../cfgdefault/CFG_PRODUCT_INFO_Default.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif

    const TCFG_FILE g_akCFG_File_Custom[]=
    {
        {
            "/data/nvram/APCFG/APRDEB/WIFI",	    	VER(AP_CFG_RDEB_FILE_WIFI_LID),		    CFG_FILE_WIFI_REC_SIZE,
            CFG_FILE_WIFI_REC_TOTAL,		    	SIGNLE_DEFUALT_REC,				    (char *)&stWifiCfgDefault
        },

        {
            "/data/nvram/APCFG/APRDEB/WIFI_CUSTOM",	VER(AP_CFG_RDEB_WIFI_CUSTOM_LID),	CFG_FILE_WIFI_CUSTOM_REC_SIZE,
            CFG_FILE_WIFI_CUSTOM_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stWifiCustomDefault
        },
        {
            "/data/nvram/APCFG/APRDCL/Audio_Sph_Med",       VER(AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID),         CFG_FILE_AUDIO_PARAM_MED_REC_SIZE,
            CFG_FILE_AUDIO_PARAM_MED_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_param_med_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Vol_custom",       VER(AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID),         CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&audio_volume_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Sph_Dual_Mic",       VER(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID),         CFG_FILE_SPEECH_DUAL_MIC_SIZE,
            CFG_FILE_SPEECH_DUAL_MIC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&dual_mic_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Wb_Sph",       VER(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID),         CFG_FILE_WB_SPEECH_REC_SIZE,
            CFG_FILE_WB_SPEECH_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&wb_speech_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/PRODUCT_INFO",       VER(AP_CFG_REEB_PRODUCT_INFO_LID),         CFG_FILE_PRODUCT_INFO_SIZE,
            CFG_FILE_PRODUCT_INFO_TOTAL,
            SIGNLE_DEFUALT_REC,                                   (char *)&stPRODUCT_INFOConfigDefault,DataReset, NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Headphone_CompFlt",       VER(AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID),         CFG_FILE_AUDIO_COMPFLT_REC_SIZE,
            CFG_FILE_HEADPHONE_COMPFLT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_hcf_custom_default, DataReset , NULL
        },

        { "/data/nvram/APCFG/APRDCL/Audio_gain_table",   VER(AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID), CFG_FILE_AUDIO_GAIN_TABLE_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_GAIN_TABLE_CUSTOM_REC_TOTAL, SIGNLE_DEFUALT_REC  ,	 (char *)&Gain_control_table_default, DataReset , NULL
        },

        { "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID), CFG_FILE_AUDIO_HD_REC_PAR_SIZE,
            CFG_FILE_AUDIO_HD_REC_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_Par_default, DataReset , NULL
        },
        { "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_Scene_Table",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID), CFG_FILE_AUDIO_HD_REC_SCENE_TABLE_SIZE,
            CFG_FILE_AUDIO_HD_REC_SCENE_TABLE_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_Scene_Table_default, DataReset , NULL
        },        
    };

    int iNvRamFileMaxLID=AP_CFG_CUSTOM_FILE_MAX_LID;
    extern int iNvRamFileMaxLID;
    const unsigned int g_i4CFG_File_Custom_Count = sizeof(g_akCFG_File_Custom)/sizeof(TCFG_FILE);

    extern const TCFG_FILE g_akCFG_File_Custom[];

    extern const unsigned int g_i4CFG_File_Custom_Count;

    int iFileWIFILID=AP_CFG_RDEB_FILE_WIFI_LID;
    extern int iFileWIFILID;
    int iFileCustomWIFILID=AP_CFG_RDEB_WIFI_CUSTOM_LID;
    extern int iFileCustomWIFILID;
    int iFilePRODUCT_INFOLID=AP_CFG_REEB_PRODUCT_INFO_LID;
    extern int iFilePRODUCT_INFOLID;

#ifdef __cplusplus
}
#endif

#endif
