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

#include <utils/Log.h>
#include <camutils/CameraProfile.h>

#ifdef MTK_MMPROFILE_SUPPORT
#include "linux/mmprofile.h"

#define LOGD(fmt, arg...) ALOGD(fmt, ##arg)
#define LOGW(fmt, arg...) ALOGW(fmt, ##arg)

static CPT_Event_Info gCPTEventInfo[Event_Max_Num] =
				{
						{Event_Camera, Event_Camera, "Camera"}

    			    // Define the event info used in Cameraservice
    				,{Event_CameraService, Event_Camera, "CameraService"}
        	            ,{Event_CS_connect, Event_CameraService, "CS_connect"}
            	        ,{Event_CS_newMediaPlayer, Event_CS_connect, "newMediaPlayer"}
        	            ,{Event_CS_newCamHwIF, Event_CS_connect, "newCamHwIF"}
        	            ,{Event_CS_newClient, Event_CS_connect, "newClient"}
        	            ,{Event_CS_getParameters, Event_CameraService, "getParameters"}
            	        ,{Event_CS_setParameters, Event_CameraService, "setParameters"}
        	            ,{Event_CS_setPreviewDisplay, Event_CameraService, "setPreviewDisplay"}
                        ,{Event_CS_sendCommand, Event_CameraService, "sendCommand"}
        	            ,{Event_CS_startPreview, Event_CameraService, "startPreview"}
            	        ,{Event_CS_takePicture, Event_CameraService, "takePicture"}
        	            ,{Event_CS_stopPreview, Event_CameraService, "stopPreview"}
        	            ,{Event_CS_startRecording, Event_CameraService, "startRecording"}
                        ,{Event_CS_releaseRecordingFrame, Event_CameraService, "releaseRecordingFrame"}
                        ,{Event_CS_dataCallbackTimestamp, Event_CameraService, "dataCallbackTimestamp"}
        	            ,{Event_CS_stopRecording, Event_CameraService, "stopRecording"}
        	            ,{Event_CS_playSound, Event_CameraService, "playSound"}
            	        ,{Event_CS_disconnect, Event_CameraService, "disconnect"}
        	            ,{Event_CS_disconnectWindow, Event_CameraService, "disconnectWindow"}

                    // Define the event used in Hal
                    ,{Event_Hal, Event_Camera, "Hal"}

                        // Define the event used in Hal::Device
                        ,{Event_Hal_Device, Event_Hal, "Device"}

                            // Define the event used in Hal::Device::DefaultCamDevice
                            ,{Event_Hal_DefaultCamDevice, Event_Hal_Device, "DefaultCamDevice"}
            		            ,{Event_Hal_DefaultCamDevice_init, Event_Hal_DefaultCamDevice, "init"}
                                ,{Event_Hal_DefaultCamDevice_uninit, Event_Hal_DefaultCamDevice, "uninit"}

                        // Define the event used in Hal::Adapter
                        ,{Event_Hal_Adapter, Event_Hal, "Adapter"}

                            // Define the event used in Hal::Adapter::Scenario
                            ,{Event_Hal_Adapter_Scenario, Event_Hal_Adapter, "Scenario"}

                                // Define the event used in Hal::Adapter::Scenario::Shot
                                ,{Event_Hal_Adapter_Scenario_Shot, Event_Hal_Adapter_Scenario, "Shot"}

                                    // --Define the event used in continuous shot
                                    ,{Event_CShot, Event_Hal_Adapter_Scenario_Shot, "ContinuousShot"}
                                        ,{Event_CShot_capture, Event_CShot, "capture"}
                                        ,{Event_CShot_sendCmd, Event_CShot, "sendCommand"}
                                        ,{Event_CShot_cancel, Event_CShot, "cancelCapture"}
                                        ,{Event_CShot_handleNotifyCb, Event_CShot, "handleNotifyCb"}
                                        ,{Event_CShot_handlePVData, Event_CShot, "handlePostViewData"}
                                        ,{Event_CShot_handleJpegData, Event_CShot, "handleJpegData"}
                                     // -- Define the event used in normal shot
                                    ,{Event_Shot,  Event_Hal_Adapter_Scenario_Shot, "NormalShot"}
                                        ,{Event_Shot_capture, Event_Shot, "capture"}
                                        ,{Event_Shot_sendCmd, Event_Shot, "sendCommand"}
                                        ,{Event_Shot_cancel, Event_Shot, "cancelCapture"}
                                        ,{Event_Shot_handleNotifyCb, Event_Shot, "handleNotifyCb"}
                                        ,{Event_Shot_handlePVData, Event_Shot, "handlePostViewData"}
                                        ,{Event_Shot_handleJpegData, Event_Shot, "handleJpegData"}
                                    // --Define the event info used in FaceBeautyShot
        	                        ,{Event_FcaeBeautyShot, Event_Hal_Adapter_Scenario_Shot, "FaceBeautyShot"}
            	                        ,{Event_FBShot_createFullFrame, Event_FcaeBeautyShot, "createFullFrame"}
                                        ,{Event_FBShot_STEP1, Event_FcaeBeautyShot, "STEP1"}
                                        ,{Event_FBShot_STEP1Algo, Event_FcaeBeautyShot, "STEP1Algo"}
            		                    ,{Event_FBShot_STEP2, Event_FcaeBeautyShot, "STEP2"}
            		                    ,{Event_FBShot_STEP3, Event_FcaeBeautyShot, "STEP3"}
            		                    ,{Event_FBShot_STEP4, Event_FcaeBeautyShot, "STEP4"}
            		                    ,{Event_FBShot_STEP4Algo, Event_FcaeBeautyShot, "STEP4Algo"}
                                        ,{Event_FBShot_STEP5, Event_FcaeBeautyShot, "STEP5"}
                                        ,{Event_FBShot_STEP5Algo, Event_FcaeBeautyShot, "STEP5Algo"}
                                        ,{Event_FBShot_STEP6, Event_FcaeBeautyShot, "STEP6"}
                                        ,{Event_FBShot_createFBJpegImg, Event_FcaeBeautyShot, "createFBJpegImg"}
            		                ,{Event_FBShot_Utility, Event_FcaeBeautyShot, "Utility"}
            		                    ,{Event_FBShot_requestBufs, Event_FBShot_Utility, "requestBufs"}
            		                    ,{Event_FBShot_InitialAlgorithm, Event_FBShot_Utility, "InitialAlgorithm"}
                                        ,{Event_FBShot_JpegEncodeImg, Event_FBShot_Utility, "JpegEncodeImg"}
                                        ,{Event_FBShot_ResizeImg, Event_FBShot_Utility, "ResizeImg"}
                                     // -- Define the event used in zsd shot
                                    ,{Event_ZsdShot,  Event_Hal_Adapter_Scenario_Shot, "ZsdShot"}
                                        ,{Event_ZsdShot_capture, Event_ZsdShot, "capture"}
                                        ,{Event_ZsdShot_handleJpegData, Event_ZsdShot, "handleJpegData"}

                            // Define the event used in Hal::Adapter::Preview
                            ,{Event_Hal_Adapter_MtkPhotoPreview, Event_Hal_Adapter, "MtkPhotoPreview"}

                                ,{Event_Hal_Adapter_MtkPhotoPreview_start, Event_Hal_Adapter_MtkPhotoPreview, "start"}
                                ,{Event_Hal_Adapter_MtkPhotoPreview_start_init, Event_Hal_Adapter_MtkPhotoPreview, "start_init"}
                                ,{Event_Hal_Adapter_MtkPhotoPreview_start_stable, Event_Hal_Adapter_MtkPhotoPreview, "start_stable"}
                                ,{Event_Hal_Adapter_MtkPhotoPreview_proc, Event_Hal_Adapter_MtkPhotoPreview, "proc"}
                                ,{Event_Hal_Adapter_MtkPhotoPreview_precap, Event_Hal_Adapter_MtkPhotoPreview, "precap"}
                                ,{Event_Hal_Adapter_MtkPhotoPreview_stop, Event_Hal_Adapter_MtkPhotoPreview, "stop"}

                            // Define the event used in Hal::Adapter::Default
                            ,{Event_Hal_Adapter_MtkDefaultPreview, Event_Hal_Adapter, "MtkDefaultPreview"}

                                ,{Event_Hal_Adapter_MtkDefaultPreview_start, Event_Hal_Adapter_MtkDefaultPreview, "start"}
                                ,{Event_Hal_Adapter_MtkDefaultPreview_start_init, Event_Hal_Adapter_MtkDefaultPreview, "start_init"}
                                ,{Event_Hal_Adapter_MtkDefaultPreview_start_stable, Event_Hal_Adapter_MtkDefaultPreview, "start_stable"}
                                ,{Event_Hal_Adapter_MtkDefaultPreview_proc, Event_Hal_Adapter_MtkDefaultPreview, "proc"}
                                ,{Event_Hal_Adapter_MtkDefaultPreview_precap, Event_Hal_Adapter_MtkDefaultPreview, "precap"}
                                ,{Event_Hal_Adapter_MtkDefaultPreview_stop, Event_Hal_Adapter_MtkDefaultPreview, "stop"}
                                ,{Event_Hal_Adapter_MtkDefaultPreview_vss, Event_Hal_Adapter_MtkDefaultPreview, "vss"}

                            // Define the event used in Hal::Adapter::ZSD
                            ,{Event_Hal_Adapter_MtkZsdPreview, Event_Hal_Adapter, "MtkZsdPreview"}

                                ,{Event_Hal_Adapter_MtkZsdPreview_start, Event_Hal_Adapter_MtkZsdPreview, "start"}
                                ,{Event_Hal_Adapter_MtkZsdPreview_start_init, Event_Hal_Adapter_MtkZsdPreview, "start_init"}
                                ,{Event_Hal_Adapter_MtkZsdPreview_start_stable, Event_Hal_Adapter_MtkZsdPreview, "start_stable"}
                                ,{Event_Hal_Adapter_MtkZsdPreview_proc, Event_Hal_Adapter_MtkZsdPreview, "proc"}
                                ,{Event_Hal_Adapter_MtkZsdPreview_precap, Event_Hal_Adapter_MtkZsdPreview, "precap"}
                                ,{Event_Hal_Adapter_MtkZsdPreview_stop, Event_Hal_Adapter_MtkZsdPreview, "stop"}
                            ,{Event_Hal_Adapter_MtkZsdCapture, Event_Hal_Adapter, "MtkZsdCapture"}

                        // Define the event used in Hal::Client
                        ,{Event_Hal_Client, Event_Hal, "Client"}

                            // Define the event used in Hal::Client::CamClient
                            ,{Event_Hal_Client_CamClient, Event_Hal_Client, "CamClient"}

                                // Define the event used in Hal::Adapter::Scenario::Shot
                                ,{Event_Hal_Client_CamClient_FD, Event_Hal_Client_CamClient, "FD"}

                    // Define the event used in Core
                    ,{Event_Core, Event_Camera, "Core"}

                        // Define the event used in Core::CamShot
                        ,{Event_Core_CamShot, Event_Core, "CamShot"}

                            // --Define the event used in multi shot
                            ,{Event_MShot, Event_Core_CamShot, "MultiShot"}
                                ,{Event_MShot_init, Event_MShot, "init"}
                                ,{Event_MShot_uninit, Event_MShot, "uninit"}
                                ,{Event_MShot_start, Event_MShot, "start"}
                                ,{Event_MShot_onCreateImage, Event_MShot, "onCreateImage"}
                                ,{Event_MShot_stop, Event_MShot, "stop"}
                                ,{Event_MShot_onCreateYuvImage, Event_MShot, "onCreateYuvImage"}
                                ,{Event_MShot_onCreateThumbImage, Event_MShot, "onCreateThumbImage"}
                                ,{Event_MShot_onCreateJpegImage, Event_MShot, "onCreateJpegImage"}
                                ,{Event_MShot_createSensorRawImg, Event_MShot, "createSensorRawImg"}
                                ,{Event_MShot_createYuvRawImg, Event_MShot, "createYuvRawImg"}
                                ,{Event_MShot_createJpegImg, Event_MShot, "createJpegImg"}
                                ,{Event_MShot_createJpegImgSW, Event_MShot, "createJpegImgSW"}
                                ,{Event_MShot_convertImage, Event_MShot, "convertImage"}
                                ,{Event_MShot_YV12ToJpeg, Event_MShot, "YV12ToJpeg"}

                               // --Define the event used in single shot
                           ,{Event_SShot, Event_Core_CamShot, "SingleShot"}
                                ,{Event_SShot_init, Event_SShot, "init"}
                                ,{Event_SShot_uninit, Event_SShot, "uninit"}
                                ,{Event_SShot_startOneSensor, Event_SShot, "startOneSensor"}
                                ,{Event_SShot_startOneMem, Event_SShot, "startOneMem"}
                                ,{Event_SShot_createSensorRawImg, Event_SShot, "createSensorRawImg"}
                                ,{Event_SShot_createYuvRawImg, Event_SShot, "createYuvRawImg"}
                                ,{Event_SShot_createJpegImg, Event_SShot, "createJpegImg"}

                        // Define the event used in Core::drv
                        ,{Event_Core_Drv, Event_Core, "Drv"}

                            // --Define the event used in sensor driver
                            ,{Event_Sensor, Event_Core_Drv, "Sensor"}
                                ,{Event_Sensor_search, Event_Sensor, "searchSensor"}
                                ,{Event_Sensor_open, Event_Sensor, "open"}
                                ,{Event_Sensor_close, Event_Sensor, "close"}
                                ,{Event_Sensor_setScenario, Event_Sensor, "setScenario"}

                            // --Define the event used in tpipe driver
                            ,{Event_TpipeDrv, Event_Core_Drv, "TpipeDrv"}

                        // Define the event used in Core::FeatureIO
                        ,{Event_Core_FeatureIO, Event_Core, "FeatureIO"}

                            // --Define the event used in 3A pipe
                            ,{Event_Pipe_3A, Event_Core_FeatureIO, "Pipe3A"}
                                ,{Event_Pipe_3A_AE, Event_Pipe_3A, "AE"}
                                ,{Event_Pipe_3A_Single_AF, Event_Pipe_3A, "SingleAF"}
                                ,{Event_Pipe_3A_Continue_AF, Event_Pipe_3A, "ContinueAF"}
                                ,{Event_Pipe_3A_AWB, Event_Pipe_3A, "AWB"}
                                ,{Event_Pipe_3A_Strobe, Event_Pipe_3A, "Strobe"}
                                ,{Event_Pipe_3A_ISP, Event_Pipe_3A, "ISP"}
                                ,{Event_Pipe_3A_ISP_DRVMGR_INIT, Event_Pipe_3A, "ISPDrvMgrInit"}
                                ,{Event_Pipe_3A_ISP_TDRIMGR_INIT, Event_Pipe_3A, "ISPTdriMgrInit"}
                                ,{Event_Pipe_3A_ISP_LSCMGR_INIT, Event_Pipe_3A, "ISPLscMgrInit"}
                                ,{Event_Pipe_3A_ISP_VALIDATE_FRAMELESS, Event_Pipe_3A, "ISPValidateFrameless"}
                                ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME, Event_Pipe_3A, "ISPValidatePerframe"}
                                ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING, Event_Pipe_3A, "ISPValidatePerframeDynamicTuning"}
                                ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, Event_Pipe_3A, "ISPValidatePerframePrepare"}
                                ,{Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, Event_Pipe_3A, "ISPValidatePerframeApply"}

                            // --Define the event used in tdriMgr
                            ,{Event_TdriMgr, Event_Core_FeatureIO, "TdriMgr"}

				};

static MMP_Event gMMPEvent[Event_Max_Num];
static bool gbInit = false;

void initCameraProfile()
{
	if(!gbInit)
	{
		gMMPEvent[Event_Camera] = MMProfileRegisterEvent(MMP_RootEvent, "Camera");

        if(0 == gMMPEvent[Event_Camera])
        {
            LOGW("register failed, maybe not enable MMProfile");
            return;
        }

		for(int i=1; i<Event_Max_Num; i++ )
		{
		    if(NULL == gCPTEventInfo[i].name)
                continue;

			gMMPEvent[gCPTEventInfo[i].event] = MMProfileRegisterEvent(gMMPEvent[gCPTEventInfo[i].parent], gCPTEventInfo[i].name);
			LOGD("Event: %s is registered as id %d", gCPTEventInfo[i].name, gMMPEvent[gCPTEventInfo[i].event]);
		}

		gbInit = true;
	}
	return;
}

bool CPTEnableEvent(CPT_Event event, int enable)
{
    MMProfileEnableEvent(gMMPEvent[event], enable);
    return true;
}

bool CPTLog(CPT_Event event, CPT_LogType type)
{
    if(!gbInit)
    {
        return true;
    }
    switch(type){
    	case 	CPTFlagStart:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagStart);
    		break;
    	case 	CPTFlagEnd:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagEnd);
    		break;
    	case 	CPTFlagPulse:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagPulse);
    		break;
        case CPTFlagSeparator:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagEventSeparator);
    		break;
    	default:
    		break;
    }

    return true;
}

bool CPTLogEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2)
{
    if(!gbInit)
    {
        return true;
    }
    switch(type){
        case    CPTFlagStart:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagStart, data1, data2);
            break;
        case    CPTFlagEnd:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagEnd, data1, data2);
            break;
        case    CPTFlagPulse:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagPulse, data1, data2);
            break;
        case    CPTFlagSeparator:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagEventSeparator, data1, data2);
            break;
        default:
            break;
    }

    return true;

}

bool CPTLogStr(CPT_Event event, CPT_LogType type, const char* str)
{
    if(!gbInit)
    {
        return true;
    }
    switch(type){
    	case 	CPTFlagStart:
    		MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagStart, str);
    		break;
    	case 	CPTFlagEnd:
    		MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagEnd, str);
    		break;
    	case 	CPTFlagPulse:
    		MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagPulse, str);
    		break;
        case    CPTFlagSeparator:
            MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagEventSeparator, str);
            break;
    	default:
    		break;
    }
    return true;
}

bool CPTLogStrEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
{
	if(!gbInit)
	{
        return true;
	}
	switch(type){
		case 	CPTFlagStart:
			MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagStart, data1, data2, str);
			break;
		case 	CPTFlagEnd:
			MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagEnd, data1, data2, str);
			break;
		case 	CPTFlagPulse:
			MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagPulse, data1, data2, str);
			break;
        case    CPTFlagSeparator:
            MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagEventSeparator, data1, data2, str);
            break;
		default:
			break;
	}
    return true;
}

AutoCPTLog::AutoCPTLog(CPT_Event event, unsigned int data1, unsigned int data2)
{
    mEvent = event;
    mData1 = data1;
	mData2 = data2;

	if(!gbInit)
	{
		return;
	}

    MMProfileLogEx(gMMPEvent[mEvent], MMProfileFlagStart, mData1, mData2);
}
AutoCPTLog:: ~AutoCPTLog()
{

    MMProfileLogEx(gMMPEvent[mEvent], MMProfileFlagEnd, mData1, mData2);
}


#else

void initCameraProfile()
{
	return;
}

bool CPTEnableEvent(CPT_Event event, int enable)
{
	return true;
}

bool CPTLog(CPT_Event event, CPT_LogType type)
{
	return true;
}
bool CPTLogEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2)
{
	return true;
}
bool CPTLogStr(CPT_Event event, CPT_LogType type, const char* str)
{
	return true;
}

bool CPTLogStrEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
{
	return true;
}

AutoCPTLog::AutoCPTLog(CPT_Event event, unsigned int data1, unsigned int data2)
{
	return;
}
AutoCPTLog:: ~AutoCPTLog()
{

    return;
}

#endif


////////////////////////////////////////////////////////////////////////////////


