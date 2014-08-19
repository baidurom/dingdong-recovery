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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include <utils/Log.h>
#include <utils/String8.h>

#include "audio_custom_exp.h"
#include <assert.h>
#include <cutils/properties.h>
#include "AudioSpeechEnhLayer.h"

//#ifdef MTK_AP_SPEECH_ENHANCEMENT  
#include "AudioCustParam.h"
//#endif

#include "audio_hd_record_48k_custom.h"

#define LOG_TAG "AudioSPELayer"

namespace android {

Word16 DefaultRecDMNR_cal_data[DMNRCalDataNum]=
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
Word16 DefaultRecCompen_filter[CompenFilterNum]=
	{32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uWord32 DefaultRecEnhancePara[EnhanceParasNum]=	
	{96,253,16388,32796,49415,0,400,0,272,4325,99,0,0,0,0,8192,0,0,0,10752,32769,0,0,0,0,0,0,0};

Word16 mLoadRecCompenfilterPara[SPC_MAX_NUM_48K_RECORD_INPUT_FIR][WB_FIR_NUM] = {DEFAULT_HD_RECORD_48K_COMPEN_FIR_Coeff};


Word16 DefaultVoIPDMNR_cal_data[DMNRCalDataNum]=
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
Word16 DefaultVoIPCompen_filter[CompenFilterNum]=
	{32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
uWord32 DefaultVoIPEnhancePara[EnhanceParasNum]=
	{96,253,16388,31,57351,31,400,0,80,4325,99,0,20488,0,23,8192,0,0,0,10752,32769,0,0,0,0,0,0,0};

SPELayer::SPELayer()
{
    Mutex::Autolock lock(mLock);
    ALOGD("SPELayer::SPELayer");     
    
    mSphCtrlBuffer = NULL;
    mpSPEBufferUL1 = NULL;
    mpSPEBufferUL2 = NULL;
    mpSPEBufferDL = NULL;
    mpSPEBufferDLDelay= NULL;
    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mSPEProcessBufSize = 0;
#ifdef MTK_AP_SPEECH_ENHANCEMENT_VOIP
    memset(&mAudioParamVoIP, 0, sizeof(AUDIO_SPH_ENH_Param_STRUCT));
#endif    
    mLoadCustParams = false;
    
    //Record settings
	mRecordSampleRate = 48000;	// sample rate=48k HSR record   if sample rate=48k normal record
	mRecordFrameRate = 20;	// frame rate=20ms
	mRecordMICDigitalGain = 16;	//MIC_DG for AGC
	mRecordApp_table = 8;	//mode = "8" stereo record, "4" mono record
	mRecordFea_Cfg_table = 511;	//without turing off anything

	//VoIP settings
	mLatencyTime = 40;		//Latency time = 40ms
	mVoIPSampleRate = 16000;	//sample rate=16k
	mLatencySampleCount = mLatencyTime*mVoIPSampleRate/1000;    //Latency sample count
	mVoIPFrameRate = 20;	//frame rate=20ms
	mVoIPMICDigitalGain = 16;	//MIC_DG  for AGC
	mVoIPApp_table = 2;	//mode=WB_VOIP
	mVoIPFea_Cfg_table = 511;	//without turning off anything	
	mNeedDelayLatency = false;

	//default record and VoIP parameters
	for(int i=0; i<EnhanceParasNum; i++)
	{
		mRecordEnhanceParas[i] = DefaultRecEnhancePara[i];
		mVoIPEnhanceParas[i] = DefaultVoIPEnhancePara[i];
	}

	for(int i=0; i<DMNRCalDataNum; i++)
	{
		mRecordDMNRCalData[i] = DefaultRecDMNR_cal_data[i];
		mVoIPDMNRCalData[i] = DefaultVoIPDMNR_cal_data[i];
	}
	
	for(int i=0; i<CompenFilterNum; i++)
	{
		mRecordCompenFilter[i] = DefaultRecCompen_filter[i];
		mVoIPCompenFilter[i] = DefaultVoIPCompen_filter[i];
	}
			
	 
	mMode = SPE_MODE_NONE;
	mRoute = ROUTE_NORMAL;
	mState = SPE_STATE_IDLE;
	mError = false;

	//for debug purpose
	mfpInDL = NULL;
	mfpInUL = NULL;
	mfpOutDL = NULL;
	mfpOutUL = NULL;
	mfpProcessedDL = NULL;
	mfpProcessedUL = NULL;
	mfpEPL = NULL;
    mfpVM = NULL;
    mVMDumpEnable = false;
    
	hDumpThread = NULL;
	
	memset(&mSph_Enh_ctrl, 0, sizeof(SPH_ENH_ctrl_struct));

    //load HD record 48k FIR filter params
    //LoadRecCustFIRParam();
}


SPELayer::~SPELayer()
{
    Mutex::Autolock lock(mLock);
    
    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NORMAL;
    mState = SPE_STATE_IDLE;
#ifdef MTK_AP_SPEECH_ENHANCEMENT_VOIP    
    memset(&mAudioParamVoIP,0, sizeof(AUDIO_SPH_ENH_Param_STRUCT));
#endif    
    mLoadCustParams = false;
    mError = false;

    Clear();
    
    ALOGD("~SPELayer");
}


bool	SPELayer::LoadSPEParameter()
{
    Mutex::Autolock lock(mLock);
    ALOGD("SPELayer::LoadSPEParameter+++");
    bool ret = true;
    //only VoIP parameters, Record use the HD record parameters
#ifdef MTK_AP_SPEECH_ENHANCEMENT_VOIP
    GetSphEnhParamFromNV(&mAudioParamVoIP);

    for(int j=0; j<=ROUTE_BT; j++)
    {
        ALOGD("PGA_gain[%d]=%d", j,mAudioParamVoIP.PGA_gain_param[j]);
        
        for(int i=0;i<EnhanceParasNum;i++)
            ALOGD("Enhance_param[%d][%d]=%d", j,i,mAudioParamVoIP.Enhance_param[j][i]);
		
		for(int i=0; i<90; i++)
		    ALOGD("Compen_filter_param[%d][%d]=%d", j,i,mAudioParamVoIP.Compen_filter_param[j].Compen_filter_param_1[i]);
		for(int i=0; i<90; i++)
		    ALOGD("Compen_filter_param[%d][%d]=%d", j,i,mAudioParamVoIP.Compen_filter_param[j].Compen_filter_param_2[i]);
	}
      

    for(int i=0;i<DMNRCalDataNum;i++)
        ALOGD("DMNR_cal_data_param[0][%d]=%d", i,mAudioParamVoIP.DMNR_cal_data_param[0][i]);

	for(int i=0;i<DMNRCalDataNum;i++)
   		ALOGD("DMNR_cal_data_param[1][%d]=%d", i,mAudioParamVoIP.DMNR_cal_data_param[1][i]);


	ALOGD("LatencyTime=%d", mAudioParamVoIP.LatencyTime);
    mLoadCustParams = true;
#endif            
    
    ALOGD("SPELayer::LoadSPEParameter---");
    return ret;
}

#if 0
void SPELayer::LoadRecCustFIRParam()
{
    FILE *fp=fopen("/sdcard/48kFIR.txt","r");
    if(fp==NULL)
    { 
        ALOGD("file open error, use default param");
        return;
    }

    ALOGD("SPELayer::LoadRecCustFIRParam+++");        
    char buf[185];
    char str[6];
    int j =0;
    int k=0;
    int l=0;
    
    for(int m=0; m<17; m++)
    {
        j=0;
        memset(str,0,sizeof(str));
        memset(buf,0,sizeof(buf));
        while(!feof(fp))
        {            
            ALOGD(" fgets m=%d ",m);
            fgets(buf,sizeof(buf),fp);
            if((m==0)||(m%2==1))
            {
                ALOGD("m=%d break",m);
                break;
            }
            l=0;
            for(int i=0;i<185;i++)
            {
                if(buf[i]!=',')
                {
                    str[j]=buf[i];
                    j++;
                }
                else if(buf[i]==',')
                {
                    mLoadRecCompenfilterPara[k][l]=atoi(str);
                    ALOGD("mLoadRecCompenfilterPara[%d][%d]=%d",k,l,mLoadRecCompenfilterPara[k][l]);
                    memset(str,0,sizeof(str));      
                    j=0;
                    l++;
                }
            }
            k++;
            ALOGD(" finish, break %d",m);
            fgets(buf,sizeof(buf),fp);
            break;
        }
    }
    fclose(fp);
    
    ALOGD("SPELayer::LoadRecCustFIRParam---");
}
#endif

bool	SPELayer::SetSPHCtrlStruct(SPE_MODE mode)
{
	ALOGD("SPELayer::SetSPHCtrlStruct, SPE_MODE=%d",mode);

	if(!mLoadCustParams)
	{
	    ALOGD("no cust parameters, use default one");
	    return false;
	}
	if((mode!=SPE_MODE_REC) && (mode!=SPE_MODE_VOIP))
	{
		ALOGD("SPELayer::SetSPHCtrlStruct, SPE_MODE not correct");
		return false;
	}
	
//	Mutex::Autolock lock(mLock);
	ALOGD("SPELayer::SetSPHCtrlStruct+++");
	if(mode == SPE_MODE_VOIP)
	{
#ifdef MTK_AP_SPEECH_ENHANCEMENT_VOIP
        //mVoIPMICDigitalGain = mAudioParamVoIP.PGA_gain_param[mRoute];
        
   		for(int i=0;i<EnhanceParasNum;i++)
			mVoIPEnhanceParas[i] = mAudioParamVoIP.Enhance_param[mRoute][i];  // read enhance pars data

        for(int i=0;i<CompenFilterNum;i++)
        {
            if(i<90)    //UL1
      			mVoIPCompenFilter[i] = mAudioParamVoIP.Compen_filter_param[mRoute].Compen_filter_param_1[i]; // read compensation filter
      	    else if(i>=180) //DL
      	        mVoIPCompenFilter[i] = mAudioParamVoIP.Compen_filter_param[mRoute].Compen_filter_param_2[i-180]; // read compensation filter
      	    else    //UL2
      	        mVoIPCompenFilter[i] = 0;
  		}

  	    if(mRoute == ROUTE_SPEAKER)
  	    {
    		for(int i=0;i<DMNRCalDataNum;i++)
	    		mVoIPDMNRCalData[i] = mAudioParamVoIP.DMNR_cal_data_param[1][i];  // read DMNR calibration data	
	    }
	    else
	    {	        
    		for(int i=0;i<DMNRCalDataNum;i++)
	    		mVoIPDMNRCalData[i] = mAudioParamVoIP.DMNR_cal_data_param[0][i];  // read DMNR calibration data	
	    }	    
#endif		
	}
	else
	{
        ALOGD("SPELayer::SetSPHCtrlStruct, not support Record mode now");
/*
        
        for(int i=0;i<EnhanceParasNum;i++)
			mRecordEnhanceParas[i] = mAudioParam.Enhance_param[mRoute][i];  // read enhance pars data

        for(int i=0;i<CompenFilterNum;i++)
        {
            if(i<90)    //UL1
      			mRecordCompenFilter[i] = mAudioParam.Compen_filter_param[mRoute][i]; // read compensation filter
      	    else if(i>=180) //DL
      	        mRecordCompenFilter[i] = mAudioParam.Compen_filter_param[mRoute][i-90]; // read compensation filter
      	    else    //UL2
      	        mRecordCompenFilter[i] = 0;
  		}

*/  	    
		return false;
	}
	ALOGD("SPELayer::SetSPHCtrlStruct---");
	return true;
}

bool	SPELayer::MutexLock(void)
{
    mLock.lock ();
    return true;
}
bool	SPELayer::MutexUnlock(void)
{
    mLock.unlock ();
    return true;
}

bool SPELayer::DumpMutexLock(void)
{
    mDumpLock.lock ();
    return true;
}
bool SPELayer::DumpMutexUnlock(void)
{
    mDumpLock.unlock ();
    return true;
}

//parameters setting
bool	SPELayer::SetEnhPara(SPE_MODE mode, unsigned long *pEnhance_pars)
{
	switch(mode)
	{
		case SPE_MODE_REC:				
			memcpy(&mRecordEnhanceParas,pEnhance_pars,EnhanceParasNum*sizeof(unsigned long));
			break;
		case SPE_MODE_VOIP:
			memcpy(&mVoIPEnhanceParas,pEnhance_pars,EnhanceParasNum*sizeof(unsigned long));
			break;
		default:
			ALOGD("SPELayer::SetEnhPara, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetEnhPara, SPE_MODE=%d",mode);
/*	for(int i=0; i<EnhanceParasNum; i++)
	{
            ALOGD("mRecordEnhanceParas[%d] %d",i,mRecordEnhanceParas[i]);
    		ALOGD("mSph_Enh_ctrl.enhance_pars[%d] %d",i,mSph_Enh_ctrl.enhance_pars[i]);
    }	*/
	return true;
}

bool	SPELayer::SetDMNRPara(SPE_MODE mode, short *pDMNR_cal_data)
{
	switch(mode)
	{
        //DMNR only supprt VoIP mode
		case SPE_MODE_VOIP:
			memcpy(&mVoIPDMNRCalData,pDMNR_cal_data,DMNRCalDataNum*sizeof(short));
			break;
		default:
			ALOGD("SPELayer::SetDMNRPara, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetDMNRPara, SPE_MODE=%d",mode);
		
	return true;
}

bool	SPELayer::SetCompFilter(SPE_MODE mode, short *pCompen_filter)
{
	switch(mode)
	{
		case SPE_MODE_REC:				
			memcpy(&mRecordCompenFilter,pCompen_filter,CompenFilterNum*sizeof(short));
			break;
		case SPE_MODE_VOIP:
			memcpy(&mVoIPCompenFilter,pCompen_filter,CompenFilterNum*sizeof(short));
			break;
		default:
			ALOGD("SPELayer::SetDMNRPara, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetCompFilter, SPE_MODE=%d",mode);
		
	return true;
}

bool	SPELayer::SetRecCompFilter(int leftnum, int rightnum, bool stereo)
{    
    ALOGD("SetRecCompFilter, leftnum=%d,rightnum=%d,%x",leftnum,rightnum,stereo);    
    memcpy(&mRecordCompenFilter,mLoadRecCompenfilterPara[leftnum],WB_FIR_NUM*sizeof(short));
    if(stereo)
        memcpy(&mRecordCompenFilter[WB_FIR_NUM],mLoadRecCompenfilterPara[rightnum],WB_FIR_NUM*sizeof(short));
    else
        memcpy(&mRecordCompenFilter[WB_FIR_NUM],mLoadRecCompenfilterPara[leftnum],WB_FIR_NUM*sizeof(short));

    return true;
}

bool	SPELayer::SetMICDigitalGain(SPE_MODE mode, long gain)
{
	switch(mode)
	{
		case SPE_MODE_REC:				
			mRecordMICDigitalGain = gain;
			break;
		case SPE_MODE_VOIP:
			mVoIPMICDigitalGain = gain;
			break;
		default:
			ALOGD("SPELayer::SetMICDigitalGain, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetMICDigitalGain MIC_DG, SPE_MODE=%d, gain=%d",mode,gain);
		
	return true;
}

bool	SPELayer::SetSampleRate(SPE_MODE mode, long sample_rate)
{

	switch(mode)
	{
		case SPE_MODE_REC:		
		    if(sample_rate!= 16000 && sample_rate!= 48000)
	        {
        		ALOGD("SPELayer::SetSampleRate, Record only support 16k or 48k samplerate");
        		mRecordSampleRate = 48000;
		        return false;
        	}
			mRecordSampleRate = sample_rate;
			break;
		case SPE_MODE_VOIP:	
			if(sample_rate != 16000)
				ALOGD("SPELayer::SetSampleRate, VOIP only support 16k samplerate");
				
			mVoIPSampleRate = 16000;
			break;
		default:
			ALOGD("SPELayer::SetSampleRate, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetSampleRate, SPE_MODE=%d",mode);
		
	return true;
}

bool	SPELayer::SetFrameRate(SPE_MODE mode, long frame_rate)
{
	
	//fixme: now only support 20ms frame rate
	frame_rate = 20;
	
	if(frame_rate!=10 && frame_rate!=20)
	{
		ALOGD("SPELayer::SetFrameRate, only support 10ms and 20ms framerate");
		return false;
	}

	switch(mode)
	{
		case SPE_MODE_REC:
			mRecordFrameRate = frame_rate;
			break;
		case SPE_MODE_VOIP:
			mVoIPFrameRate = frame_rate;
			break;
		default:
			ALOGD("SPELayer::SetFrameRate, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetFrameRate, SPE_MODE=%d, frame_rate=%d",mode,frame_rate);
		
	return true;
}

bool	SPELayer::SetAPPTable(SPE_MODE mode, SPE_APP_TABLE App_table)
{
	switch(mode)
	{
		case SPE_MODE_REC:
			mRecordApp_table = App_table;
			break;
		case SPE_MODE_VOIP:
			mVoIPApp_table = App_table;
			break;
		default:
			ALOGD("SPELayer::SetAPPTable, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetAPPTable, SPE_MODE=%d, App_table=%x",mode,App_table);
	
	return true;
}

bool	SPELayer::SetFeaCfgTable(SPE_MODE mode, long Fea_Cfg_table)
{
	
	switch(mode)
	{
		case SPE_MODE_REC:
			mRecordFea_Cfg_table = Fea_Cfg_table;
			break;
		case SPE_MODE_VOIP:
			mVoIPFea_Cfg_table = Fea_Cfg_table;
			break;
		default:
			ALOGD("SPELayer::SetFeaCfgTable, not support mode");
			return false;
			break;
	}
	
	ALOGD("SPELayer::SetFeaCfgTable, SPE_MODE=%d,Fea_Cfg_table=%x",mode,Fea_Cfg_table);
	
	return true;
}

bool	SPELayer::SetLatencyTime(int ms)
{
	ALOGD("SPELayer::SetLatencyTime, old=%d, new=%d",mLatencyTime,ms);
	mLatencyTime = ms;
	return true;
}

bool	SPELayer::SetChannel(int channel)
{
    if(channel==1)  //mono
        mRecordApp_table = 4;	//mode = "8" stereo record, "4" mono record
    else    //stereo
        mRecordApp_table = 8;
	
    ALOGD("SPELayer::SetChannel only for recording, mRecordApp_table=%x",mRecordApp_table);		
	return true;
}

int	SPELayer::GetChannel()
{
    if(mRecordApp_table==4)
        return 1;
    else
        return 2;
}
/*
bool	SPELayer::SetMode(SPE_MODE mode)
{
	ALOGD("SPELayer::SetMode %d",mode);

	if((mode!=SPE_MODE_REC) && (mode!=SPE_MODE_VOIP))
	{
		ALOGD("SPELayer::SetMode, SPE_MODE not correct");
		return false;
	}
	
	if (mMode == mode)
		return true;	
		
	mMode = mode;
	return true;
}
*/

bool	SPELayer::SetRoute(SPE_ROUTE route)
{
	ALOGD("SPELayer::SetRoute %d",route);

	if((route < ROUTE_NORMAL)&&(route > ROUTE_BT))
	{
		ALOGD("SPELayer::SetRoute, route not correct");
		return false;
	}
	
	if (mRoute == route)
		return true;	
		
	mRoute = route;
	return true;
}


//Get parameters setting
int	SPELayer::GetLatencyTime()
{
	//only for VoIP
	return mLatencyTime;
}

SPE_MODE	SPELayer::GetMode()
{
	return mMode;
}

SPE_ROUTE   SPELayer::GetRoute()
{		
	return mRoute;
}
bool    SPELayer::IsSPERunning()
{
    if(mState == SPE_STATE_RUNNING)
        return true;
    else
        return false;
}


long	SPELayer::GetSampleRate(SPE_MODE mode)
{
	long retSampleRate = 0;
	switch(mode)
	{
		case SPE_MODE_REC:
			retSampleRate = mRecordSampleRate;
			break;
		case SPE_MODE_VOIP:
			retSampleRate = mVoIPSampleRate;
			break;
		default:
			retSampleRate = mSph_Enh_ctrl.sample_rate;
			break;
	}
	
	ALOGD("SPELayer::GetSampleRate, SPE_MODE=%d, retSampleRate=%d",mode,retSampleRate);
	return retSampleRate;
}

long	SPELayer::GetFrameRate(SPE_MODE mode)
{
	long retFrameRate = 0;
	switch(mode)
	{
		case SPE_MODE_REC:
			retFrameRate = mRecordFrameRate;
			break;
		case SPE_MODE_VOIP:
			retFrameRate = mVoIPFrameRate;
			break;
		default:
			retFrameRate = mSph_Enh_ctrl.frame_rate;
			break;
	}
	
	ALOGD("SPELayer::GetFrameRate, SPE_MODE=%d, retFrameRate=%d",mode,retFrameRate);
	return retFrameRate;
}

long	SPELayer::GetMICDigitalGain(SPE_MODE mode)
{
	long retPGAGain = 0;
	switch(mode)
	{
		case SPE_MODE_REC:
			retPGAGain = mRecordMICDigitalGain;
			break;
		case SPE_MODE_VOIP:
			retPGAGain = mVoIPMICDigitalGain;
			break;
		default:
			retPGAGain = mSph_Enh_ctrl.MIC_DG;
			break;
	}
	
	ALOGD("SPELayer::GetMICDigitalGain, SPE_MODE=%d, retPGAGain=%d",mode,retPGAGain);
	return retPGAGain;
}

long	SPELayer::GetAPPTable(SPE_MODE mode)
{
	long retAPPTable = 0;
	switch(mode)
	{
		case SPE_MODE_REC:
			retAPPTable = mRecordApp_table;
			break;
		case SPE_MODE_VOIP:
			retAPPTable = mVoIPApp_table;
			break;
		default:
			retAPPTable = mSph_Enh_ctrl.App_table;
			break;
	}
	
	ALOGD("SPELayer::GetAPPTable, SPE_MODE=%d, retAPPTable=%x",mode,retAPPTable);
	return retAPPTable;
}

long	SPELayer::GetFeaCfgTable(SPE_MODE mode)
{
	long retFeaCfgTable = 0;
	switch(mode)
	{
		case SPE_MODE_REC:
			retFeaCfgTable = mRecordFea_Cfg_table;
			break;
		case SPE_MODE_VOIP:
			retFeaCfgTable = mVoIPFea_Cfg_table;
			break;
		default:
			retFeaCfgTable = mSph_Enh_ctrl.Fea_Cfg_table;
			break;
	}
	
	ALOGD("SPELayer::GetFeaCfgTable, SPE_MODE=%d, retFeaCfgTable=%x",mode,retFeaCfgTable);
	return retFeaCfgTable;
}

bool	SPELayer::GetEnhPara(SPE_MODE mode, unsigned long *pEnhance_pars)
{
	ALOGD("SPELayer::GetEnhPara, SPE_MODE=%d",mode);
	switch(mode)
	{
		case SPE_MODE_REC:
			memcpy(pEnhance_pars,&mRecordEnhanceParas,EnhanceParasNum*sizeof(unsigned long));
			break;
		case SPE_MODE_VOIP:
			memcpy(pEnhance_pars,&mVoIPEnhanceParas,EnhanceParasNum*sizeof(unsigned long));
			break;
		default:
			memcpy(pEnhance_pars,&mSph_Enh_ctrl.enhance_pars,EnhanceParasNum*sizeof(unsigned long));
			break;
	}
	
	return true;
}

bool	SPELayer::GetDMNRPara(SPE_MODE mode, short *pDMNR_cal_data)
{
	ALOGD("SPELayer::GetDMNRPara, SPE_MODE=%d",mode);
	switch(mode)
	{
	//DMNR only support VoIP mode
		case SPE_MODE_VOIP:
			memcpy(pDMNR_cal_data,&mVoIPDMNRCalData,DMNRCalDataNum*sizeof(short));
			break;
		default:
			memcpy(pDMNR_cal_data,&mSph_Enh_ctrl.DMNR_cal_data,DMNRCalDataNum*sizeof(short));
			break;
	}
	
	return true;
}

bool	SPELayer::GetCompFilter(SPE_MODE mode, short *pCompen_filter)
{
	ALOGD("SPELayer::GetCompFilter, SPE_MODE=%d",mode);
	switch(mode)
	{
		case SPE_MODE_REC:
			memcpy(pCompen_filter,&mRecordCompenFilter,CompenFilterNum*sizeof(short));
			break;
		case SPE_MODE_VOIP:
			memcpy(pCompen_filter,&mVoIPCompenFilter,CompenFilterNum*sizeof(short));
			break;
		default:
			memcpy(pCompen_filter,&mSph_Enh_ctrl.Compen_filter,CompenFilterNum*sizeof(short));
			break;
	}
	
	return true;
}

//speech enhancement setting and process
bool    SPELayer::Start(SPE_MODE mode)  //for VOIP, both uplink/downlink
{

    ALOGD("SPELayer::Start mode=%d",mode);
    Mutex::Autolock lock(mLock);
	if(mState == SPE_STATE_RUNNING)
	{
		ALOGD("SPELayer::Start already start!");
		return false;
	}
	
#ifdef MTK_AP_SPEECH_ENHANCEMENT_VOIP
    if(!SetSPHCtrlStruct(mode))
        ALOGD("Get parameter fail, use default one");
#endif

    // set mSph_Enh_ctrl parameters
	if(mode == SPE_MODE_REC)
	{
	    mSph_Enh_ctrl.sample_rate = mRecordSampleRate;
	    mSph_Enh_ctrl.frame_rate = mRecordFrameRate;
	    mSph_Enh_ctrl.MIC_DG = mRecordMICDigitalGain;
	    mSph_Enh_ctrl.Fea_Cfg_table = mRecordFea_Cfg_table;
	    mSph_Enh_ctrl.App_table = mRecordApp_table;	    
	    memcpy(&mSph_Enh_ctrl.enhance_pars, &mRecordEnhanceParas, EnhanceParasNum*sizeof(uWord32));
	    memcpy(&mSph_Enh_ctrl.DMNR_cal_data, &mRecordDMNRCalData, DMNRCalDataNum*sizeof(Word16));
	    memcpy(&mSph_Enh_ctrl.Compen_filter, &mRecordCompenFilter, CompenFilterNum*sizeof(Word16));
	    ALOGD("mRecordSampleRate=%d, mRecordFrameRate=%d, mRecordApp_table=%x",mRecordSampleRate,mRecordFrameRate,mRecordApp_table);	    
	}
	else if(mode == SPE_MODE_VOIP)
	{
	    mSph_Enh_ctrl.sample_rate = mVoIPSampleRate;
	    mSph_Enh_ctrl.frame_rate = mVoIPFrameRate;
	    mSph_Enh_ctrl.MIC_DG = mVoIPMICDigitalGain;
	    mSph_Enh_ctrl.Fea_Cfg_table = mVoIPFea_Cfg_table;
	    mSph_Enh_ctrl.App_table = mVoIPApp_table;
	    memcpy(&mSph_Enh_ctrl.enhance_pars, &mVoIPEnhanceParas, EnhanceParasNum*sizeof(uWord32));
	    memcpy(&mSph_Enh_ctrl.DMNR_cal_data, &mVoIPDMNRCalData, DMNRCalDataNum*sizeof(Word16));
	    memcpy(&mSph_Enh_ctrl.Compen_filter, &mVoIPCompenFilter, CompenFilterNum*sizeof(Word16));

	    mLatencySampleCount = mLatencyTime*mVoIPSampleRate/1000;    //Latency sample count
	    mNeedDelayLatency = true;
	}
	else
	{
		ALOGD("SPELayer::Start wrong mode");
		return false;
	}
    
	
	if(mSphCtrlBuffer)
	{
        ALOGD("SPELayer::Start mSphCtrlBuffer already exist, delete and recreate");
        ENH_API_Free(&mSph_Enh_ctrl);
	    free(mSphCtrlBuffer);
	    mSphCtrlBuffer = NULL;
	}
    
    uint32 mem_size;
    mem_size = ENH_API_Get_Memory(&mSph_Enh_ctrl);    
    mSphCtrlBuffer = (int*) malloc(mem_size);
    if(mSphCtrlBuffer==NULL)
    {
        ALOGD("SPELayer::Start create fail");
        return false;
    }
    ALOGD("SPELayer::going to configure,mSphCtrlBuffer=%p,mem_size=%d",mSphCtrlBuffer,mem_size);
    memset( mSphCtrlBuffer, 0, mem_size );    
    ENH_API_Alloc( &mSph_Enh_ctrl, (Word32 *)mSphCtrlBuffer );

    ENH_API_Rst( &mSph_Enh_ctrl);

    
	mMode = mode;
	mState = SPE_STATE_START;
	mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;

    //set the address
	if(mMode == SPE_MODE_REC)
	{
        if(mSph_Enh_ctrl.frame_rate == 20)  //frame rate = 20ms, buffer size 320*2, input/output use the same address
        {            
			mpSPEBufferUL1= &mSph_Enh_ctrl.PCM_buffer[0];
			mpSPEBufferUL2= &mSph_Enh_ctrl.PCM_buffer[RecBufSize20ms];

			if(mSph_Enh_ctrl.sample_rate == 16000)
			{
			    mSPEProcessBufSize = 320*2*sizeof(short);   //for 16k samplerate with  20ms frame rate (stereo)
			}
			else
			{
			    mSPEProcessBufSize = RecBufSize20ms*2*sizeof(short);    //for 48k samplerate with  20ms frame rate (stereo)
			}
 
        }
        else    //frame rate = 10ms, buffer size 480*2
        {

			mpSPEBufferUL1= &mSph_Enh_ctrl.PCM_buffer[0];
			mpSPEBufferUL2= &mSph_Enh_ctrl.PCM_buffer[RecBufSize10ms];

            if(mSph_Enh_ctrl.sample_rate == 16000)
			{
			    mSPEProcessBufSize = 160*2*sizeof(short);   //for 16k samplerate with  10ms frame rate (stereo)
			}
			else    //48K samplerate
			{
			    mSPEProcessBufSize = RecBufSize10ms*2*sizeof(short);    //for 48k samplerate with  10ms frame rate (stereo)
			}

        }
	}
	else    //VoIP mode
	{   //only support 16K samplerate
	    if(mSph_Enh_ctrl.frame_rate == 20)  //frame rate = 20ms, buffer size 320*4
        {
			mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];
			mpSPEBufferUL2= &mSph_Enh_ctrl.PCM_buffer[320];
			mpSPEBufferDL= &mSph_Enh_ctrl.PCM_buffer[640];
			mpSPEBufferDLDelay= &mSph_Enh_ctrl.PCM_buffer[960];

            mSPEProcessBufSize = 320*2*sizeof(short);   //for 16k samplerate with  20ms frame rate (stereo)
	        
        }
        else    //frame rate = 10ms, buffer size 160*4
        {
			mpSPEBufferUL1= &mSph_Enh_ctrl.PCM_buffer[0];
			mpSPEBufferUL2= &mSph_Enh_ctrl.PCM_buffer[160];
			mpSPEBufferDL= &mSph_Enh_ctrl.PCM_buffer[320];
			mpSPEBufferDLDelay= &mSph_Enh_ctrl.PCM_buffer[480];

			mSPEProcessBufSize = 160*2*sizeof(short);   //for 16k samplerate with  20ms frame rate (stereo)

        }
        mpSPEBufferNE = mpSPEBufferUL1;   //samtest
        mpSPEBufferFE = mpSPEBufferDL; //samtest
	}

    dump();
    mfpInDL = NULL;
	mfpInUL = NULL;
	mfpOutDL = NULL;
	mfpOutUL = NULL;
	mfpProcessedDL = NULL;
	mfpProcessedUL = NULL;
	mfpEPL = NULL;
	mfpVM = NULL;
	ALOGD("mSPEProcessBufSize=%d",mSPEProcessBufSize);
	
	return true;
}


//normal record + VoIP
bool	SPELayer::Process(SPE_DATA_DIRECTION dir, short *inBuf, int  inBufLength, short *outBuf, int outBufLength)
{
    if(mError == true)
    {
        ReStart();
        mError = false;
    }
    Mutex::Autolock lock(mLock);
    if((mState == SPE_STATE_IDLE)||(dir==DOWNLINK && mMode!=SPE_MODE_VOIP))
    {
        ALOGD("SPELayer::Process wrong state,%d, mState=%d,mMode=%d",dir,mState,mMode);
        return false;
    }
    if(mMode == SPE_MODE_REC)
    {
        if((mULInBufferQ.size()>20)||(mULOutBufferQ.size()>20))
        {
            ALOGD("no service? mULInBufferQ.size=%d, mULOutBufferQ.size=%d",mULInBufferQ.size(),mULOutBufferQ.size());
        }
    }
    Dump_PCM_In(dir,inBuf,inBufLength);

    int copysize = inBufLength>>2;
    int printindex=0;
//    ALOGD("SPELayer::Process, dir=%x, inBuf=%p,inBufLength=%d,mMode=%x,copysize=%d",dir,inBuf,inBufLength,mMode,copysize);
        
	mState = SPE_STATE_RUNNING;
                

    BufferInfo* newInBuffer=new BufferInfo;
    newInBuffer->pBufBase = (short*) malloc(inBufLength);    

    memcpy(newInBuffer->pBufBase, inBuf, inBufLength);

    newInBuffer->BufLen= inBufLength;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite= newInBuffer->pBufBase;
//    ALOGD("inBufLength=%d,mULInBufQLenTotal=%d, Qsize=%d",newInBuffer->BufLen,mULInBufQLenTotal,mULInBufferQ.size());
    if(dir == UPLINK)
    {
   	    mULInBufferQ.add(newInBuffer);
   	    mULInBufQLenTotal += inBufLength;
//   	    ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, size=%d",mULInBufQLenTotal,mULInBufferQ.size());
    }
   	else
   	{
        //queue to the downlink input buffer queue
   	    mDLInBufferQ.add(newInBuffer);
   	    mDLInBufQLenTotal += inBufLength;
   	    ALOGD("SPELayer::Process, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());
   	    //also add to delay buffer queue
   	    if(mNeedDelayLatency) //the first DL buffer, add the delay time buffer as first delay buffer queue
   	    {
   	        mNeedDelayLatency = false;
   	        BufferInfo* newDelayBuffer = new BufferInfo;
   	        newDelayBuffer->pBufBase = (short*) malloc(mLatencySampleCount*2*sizeof(short));    //one channel, 16bits
   	        newDelayBuffer->BufLen= mLatencySampleCount*2*sizeof(short);    
            newDelayBuffer->pRead = newDelayBuffer->pBufBase;
            newDelayBuffer->pWrite= newDelayBuffer->pBufBase;
            newDelayBuffer->BufLen4Delay= mLatencySampleCount*2*sizeof(short); 
            newDelayBuffer->pRead4Delay= newDelayBuffer->pBufBase;
            newDelayBuffer->pWrite4Delay= newDelayBuffer->pBufBase;
   	        memset(newDelayBuffer->pBufBase,0,newDelayBuffer->BufLen);
   	        mDLDelayBufferQ.add(newDelayBuffer);
       	    mDLDelayBufQLenTotal += newDelayBuffer->BufLen;
//       	    ALOGD("newDelayBuffer->BufLen=%d, size=%d",newDelayBuffer->BufLen,mDLDelayBufferQ.size());
   	    }

        newInBuffer->BufLen4Delay = inBufLength;
        newInBuffer->pRead4Delay= newInBuffer->pBufBase;
        newInBuffer->pWrite4Delay= newInBuffer->pBufBase;
        mDLDelayBufferQ.add(newInBuffer);
   	    mDLDelayBufQLenTotal += inBufLength;
//   	    ALOGD("SPELayer::Process, mDLDelayBufQLenTotal=%d, size=%d",mDLDelayBufQLenTotal, mDLDelayBufferQ.size());
    }

    //process the input buffer queue
    if(mMode == SPE_MODE_REC)   //record
    {        

//       ALOGD("SPELayer::Process, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());
        if((mULInBufQLenTotal< mSPEProcessBufSize)&& (mULOutBufferQ.size()==0))  //not enough UL buffer for process
        {
            ALOGD("SPELayer::Process,going memset 0 inBuf=%p,inBufLength=%d",inBuf,inBufLength);
            memset(inBuf,0,inBufLength);  //return in same input buffer address
            //memset(outBuf,0,inBufLength);   //return in output buffer address
            return true;
        }

//        ALOGD("SPELayer::Process, enough mULInBufQLenTotal buffer,size=%d",mULInBufferQ.size());
        while(mULInBufQLenTotal>= mSPEProcessBufSize)
        {
            int tmpSPEProcessBufSize = mSPEProcessBufSize;
            int indexIn=0;
            int tempULIncopysize = mULInBufferQ[0]->BufLen>>2;

//            ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, SPERecBufSize=%d,tempULIncopysize=%d",mULInBufQLenTotal,mSPEProcessBufSize,tempULIncopysize);
            while(tmpSPEProcessBufSize)
            {
                if(mULInBufferQ.isEmpty())
                {
                    ALOGD("uplink input buffer queue is empty, something wrong!!");
                    mError = true;
                    break;
                }
            
//            ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,tempULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,tempULIncopysize);
                if(tempULIncopysize>0)   //get the buffer data from the first uplink input buffer queue
                {
                    *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);    //left channel
                    *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead +1); //right channel
                    //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));
                    mULInBufferQ[0]->pRead+=2;
                    tempULIncopysize--;
                    indexIn++;
                    tmpSPEProcessBufSize-=4;
                    mULInBufQLenTotal-=4;   //int and short transform
                    mULInBufferQ[0]->BufLen-=4; //record the buffer you consumed
                }
                else    //consume all the data in first queue buffer
                {
                    free(mULInBufferQ[0]->pBufBase);
                    delete mULInBufferQ[0];
                    mULInBufferQ.removeAt(0);
                    tempULIncopysize = mULInBufferQ[0]->BufLen>>2;
//                    ALOGD("UL in buffer consume finish, next BufferBase=%p",mULInBufferQ[0]->pBufBase);
                }
            }         

            if(mError)
            {
                ALOGD("error!!");
                break;
            }
                
            //process the fill in buffer
            ENH_API_Process(&mSph_Enh_ctrl);

            Dump_EPL(&mSph_Enh_ctrl.EPL_buffer,EPLBufSize*sizeof(short));
            EPLTransVMDump();
            
            BufferInfo *newOutBuffer = new BufferInfo;

            newOutBuffer->pBufBase = (short*) malloc(mSPEProcessBufSize);
            newOutBuffer->BufLen= mSPEProcessBufSize;
                                      
            newOutBuffer->pRead = newOutBuffer->pBufBase;
            newOutBuffer->pWrite= newOutBuffer->pBufBase;
 //           ALOGD("newOutBuffer->pBufBase=%p,newOutBuffer->pRead=%p,newOutBuffer->pWrite=%p,newOutBuffer->BufLen=%d",newOutBuffer->pBufBase,newOutBuffer->pRead,newOutBuffer->pWrite,newOutBuffer->BufLen);

            
            int indexOut=0;
   
            int copysizetest = newOutBuffer->BufLen>>2;
            while(copysizetest)
            {
//                ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
                *(newOutBuffer->pWrite) = *(mpSPEBufferUL1 + indexOut);
                *(newOutBuffer->pWrite+1) = *(mpSPEBufferUL2 + indexOut);
                newOutBuffer->pWrite+=2;
                indexOut++;
                copysizetest--;
            }

            Dump_PCM_Process(UPLINK,newOutBuffer->pBufBase,newOutBuffer->BufLen);
            
            mULOutBufferQ.add(newOutBuffer);
   	        mULOutBufQLenTotal += newOutBuffer->BufLen;

//            ALOGD("mULOutBufQLenTotal=%d, indexOut=%d,newOutBuffer->pWrite=%p, mULOutBufferQsize=%d",mULOutBufQLenTotal,indexOut,newOutBuffer->pWrite,mULOutBufferQ.size());
        }
                       
    }
    else    //VoIP
    {
//        ALOGD("SPELayer::process VoIP");
        //       ALOGD("SPELayer::Process, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());
        if(dir == UPLINK)
        {
            if(mULInBufQLenTotal< mSPEProcessBufSize)  //not enough UL buffer for process
            {
                if(mULOutBufQLenTotal<inBufLength)  //not enough UL output buffer, return 0 data
                {                    
                    ALOGD("SPELayer::Process,going memset 0 inBuf=%p,inBufLength=%d",inBuf,inBufLength);
                    memset(inBuf,0,inBufLength);  //return in same input buffer address
                    //memset(outBuf,0,inBufLength);   //return in output buffer address
                    return true;
                }
                else    //have enough UL output buffer data
                {
//                    ALOGD("has UL Output buffer mULOutBufferQ.size=%d,mULOutBufQLenTotal=%d",mULOutBufferQ.size(),mULOutBufQLenTotal);
                    int tmpInBufLength = inBufLength;
                    int count=0;
                    int tempULCopy= mULOutBufferQ[0]->BufLen>>2;
                    while(tmpInBufLength)
                    {
                        if(mULOutBufferQ.isEmpty())
                        {
                            ALOGD("SPELayer::uplink Output buffer queue is empty, something wrong!!");
                            mError = true;
                            break;
                        }
            

    //                              ALOGD("mDLOutBufferQ.size = %d,tempDLCopy=%d",mDLOutBufferQ.size(),tempDLCopy);

                        if(tempULCopy>0)   //get the buffer data from the first downlink input buffer queue
                        {
//                                         ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
//                                          ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                            *(inBuf+count) = *(mULOutBufferQ[0]->pRead);
                            *(inBuf+count+1) = *(mULOutBufferQ[0]->pRead +1);
                            mULOutBufferQ[0]->pRead+=2;
                            tmpInBufLength-=4;  //int and short transform
                            tempULCopy--;
                            count+=2;
                            mULOutBufQLenTotal-=4;   //int and short transform
                            mULOutBufferQ[0]->BufLen-=4;
                        }
                        else    //consume all the data in first queue buffer
                        {
                            free(mULOutBufferQ[0]->pBufBase);
                            delete mULOutBufferQ[0];
                            mULOutBufferQ.removeAt(0);
                            tempULCopy= mULOutBufferQ[0]->BufLen>>2;
//                            ALOGD("SPELayer::uplink Output buffer consumed a");
                        }
                    }
//                    ALOGD("SPELayer::uplink has UL Output buffer but not enough UL Input buffer");
                    Dump_PCM_Out(dir,inBuf,inBufLength);
                    return true;
                }
            }
        }
        else    //downlink data
        {
//            ALOGD("VoIP mDLInBufQLenTotal=%d, mDLDelayBufQLenTotal=%d",mDLInBufQLenTotal,mDLDelayBufQLenTotal);
            for(int i=0; i<mDLInBufferQ.size();i++)
        	{
	            ALOGD("mDLInBufQLenTotal Q[%d] BufLen=%d",i,mDLInBufferQ[i]->BufLen);
        	}
        	for(int i=0; i<mDLDelayBufferQ.size();i++)
        	{
	            ALOGD("mDLDelayBufferQ Q[%d] BufLen=%d",i,mDLDelayBufferQ[i]->BufLen4Delay);
        	}
        	
            if(mDLInBufQLenTotal< mSPEProcessBufSize)  //not enough DL buffer for process
            {
                if(mDLOutBufQLenTotal>=inBufLength) //downlink have processed enough data
                {
                    int tmpInBufLength = inBufLength;
                    int count=0;
                    int tempDLCopy= mDLOutBufferQ[0]->BufLen>>2;
                    while(tmpInBufLength)
                    {
                        if(mDLOutBufferQ.isEmpty())
                        {
                            ALOGD("SPELayer::downlink Output buffer queue is empty, something wrong!!");
                            mError = true;
                            break;
                        }
            
//                        ALOGD("mDLOutBufferQ.size = %d,tempDLCopy=%d",mDLOutBufferQ.size(),tempDLCopy);

                        if(tempDLCopy>0)   //get the buffer data from the first downlink input buffer queue
                        {
//                                        ALOGD("mDLOutBufferQ[0]->pRead = %p,mDLOutBufferQ[0]->pBufBase=%p,mDLOutBufferQ[0]->BufLen=%d",mDLOutBufferQ[0]->pRead,mDLOutBufferQ[0]->pBufBase,mDLOutBufferQ[0]->BufLen);
//                                          ALOGD("tmpInBufLength=%d,count=%d,mDLOutBufQLenTotal=%d,tempDLCopy=%d",tmpInBufLength,count,mDLOutBufQLenTotal,tempDLCopy);
                            *(inBuf+count) = *(mDLOutBufferQ[0]->pRead);
                            *(inBuf+count+1) = *(mDLOutBufferQ[0]->pRead +1);
                            mDLOutBufferQ[0]->pRead+=2;
                            tmpInBufLength-=4;  //int and short transform
                            tempDLCopy--;
                            count+=2;
                            mDLOutBufQLenTotal-=4;   //int and short transform
                            mDLOutBufferQ[0]->BufLen-=4;
                        }
                        else    //consume all the data in first queue buffer
                        {
                            free(mDLOutBufferQ[0]->pBufBase);
                            delete mDLOutBufferQ[0];
                            mDLOutBufferQ.removeAt(0);
                            tempDLCopy= mDLOutBufferQ[0]->BufLen>>2;
//                            ALOGD("SPELayer::downlink Output buffer consumed");
                        }
                    }
//                    ALOGD("SPELayer::downlink has DL Output buffer but not enough DL Input buffer");
                    Dump_PCM_Out(dir,inBuf,inBufLength);
                    return true;
                }
                else    //both uplink/downlink do not have enough data
                {
                    ALOGD("SPELayer::Process,going memset 0 inBuf=%p,inBufLength=%d",inBuf,inBufLength);
                    memset(inBuf,0,inBufLength);  //return in same input buffer address
                    //memset(outBuf,0,inBufLength);   //return in output buffer address
                    return true;
                }
            }
        }
        
        
        //processing?
        if((mULInBufQLenTotal>= mSPEProcessBufSize)&&(mDLInBufQLenTotal>= mSPEProcessBufSize))
        {
            while((mULInBufQLenTotal>= mSPEProcessBufSize)&&(mDLInBufQLenTotal>= mSPEProcessBufSize))
            {
                //fill in the data to process buffer
                int tmpSPEProcessBufSize = mSPEProcessBufSize;
                int indexIn=0;
                int ULIncopysize = mULInBufferQ[0]->BufLen>>2;

//                ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, SPERecBufSize=%d,ULIncopysize=%d",mULInBufQLenTotal,mSPEProcessBufSize,ULIncopysize);
//                ALOGD("SPELayer::Process, mDLDelayBufferQ size=%d,mDLDelayBufQLenTotal=%d, SPERecBufSize=%d",mDLDelayBufferQ.size(),mDLDelayBufQLenTotal,mSPEProcessBufSize);
//                ALOGD("SPELayer::Process, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d,DLIncopysizetest=%d",mDLInBufferQ.size(),mDLInBufQLenTotal,DLIncopysizetest);
                while(tmpSPEProcessBufSize)
                {
                    if(mULInBufferQ.isEmpty()||mDLInBufferQ.isEmpty()||mDLDelayBufferQ.isEmpty())
                    {
                        ALOGD("SPELayer::input buffer queue is empty, something wrong!!");
                        mError = true;
                        break;
                    }
            
//                              ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,ULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,ULIncopysize);
                    if(ULIncopysize>0)   //get the buffer data from the first uplink input buffer queue
                    {
                        //fill in uplink data
                        *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);
                        *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead +1);
                        mULInBufferQ[0]->pRead+=2;
                        mULInBufQLenTotal-=4;   //int and short transform
                        mULInBufferQ[0]->BufLen-=4; //record the buffer you consumed
                        
                        //fill in downlink data
                        if(mDLInBufferQ[0]->BufLen<=0)  //run out of DL queue0 buffer
                        {
                            //not to free the buffer here due to the data still queue in the DLDelay buffer
                            //free(mDLInBufferQ[0]->pBufBase);  //just remove the queue but not delete buffer since it also queue in the delay queue
                            //delete mDLInBufferQ[0];
                            mDLInBufferQ.removeAt(0);
//                            ALOGD("get next DLInBufferQ, size=%d, mDLInBufQLenTotal=%d",mDLInBufferQ.size(),mDLInBufQLenTotal);
                            if(mDLInBufferQ.isEmpty())
                            {
                                ALOGD("no DL buffer, something wrong");
                                mError = true;
                                break;
                            }
//                            ALOGD("DL in buffer consume finish, next BufferBase=%p",mDLInBufferQ[0]->pBufBase);
                        }
                        *(mpSPEBufferDL + indexIn) = (*(mDLInBufferQ[0]->pRead)>>1) + (*(mDLInBufferQ[0]->pRead+1)>>1); //only mono data
                        mDLInBufferQ[0]->pRead+=2;
                        mDLInBufQLenTotal-=4;   //int and short transform
                        mDLInBufferQ[0]->BufLen-=4; //record the buffer you consumed

                        
                        //fill in delay latency data
                        if(mDLDelayBufferQ[0]->BufLen4Delay<=0)  //run out of DL  delay queue0 buffer
                        {
                            ALOGD("DL delay ran out");
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                            if(mDLDelayBufferQ.isEmpty())
                            {
                                ALOGD("no DL delay buffer, something wrong");
                                mError = true;
                                break;
                            }
//                            ALOGD("DL delay in buffer consume finish, next BufferBase=%p, size=%d",mDLDelayBufferQ[0]->pBufBase,mDLDelayBufferQ.size());
                        }

                        *(mpSPEBufferDLDelay + indexIn) = (*(mDLDelayBufferQ[0]->pRead4Delay)>>1) + (*(mDLDelayBufferQ[0]->pRead4Delay+1)>>1); //only mono data
                        mDLDelayBufferQ[0]->pRead4Delay+=2;
                        mDLDelayBufQLenTotal-=4;   //int and short transform
                        mDLDelayBufferQ[0]->BufLen4Delay-=4; //record the buffer you consumed
                        //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));                    
                    
                        ULIncopysize--;
                        indexIn++;
                        tmpSPEProcessBufSize-=4;
                        
                        
                    }
                    else    //consume all the data in first queue buffer
                    {
                        free(mULInBufferQ[0]->pBufBase);
                        delete mULInBufferQ[0];
                        mULInBufferQ.removeAt(0);
                        ULIncopysize = mULInBufferQ[0]->BufLen>>2;
//                                    ALOGD("UL in buffer consume finish, next BufferBase=%p",mULInBufferQ[0]->pBufBase);
                    }
                }

                if(mError)
                {
                    ALOGD("error!!");
                    break;
                }
             
                //after fill buffer, process
                ENH_API_Process(&mSph_Enh_ctrl);

                Dump_EPL(&mSph_Enh_ctrl.EPL_buffer,EPLBufSize*sizeof(short));
                EPLTransVMDump();

                //record to the outputbuffer queue

                BufferInfo *newDLOutBuffer = new BufferInfo;
                BufferInfo *newULOutBuffer = new BufferInfo;

                newDLOutBuffer->pBufBase = (short*) malloc(mSPEProcessBufSize);
                newDLOutBuffer->BufLen= mSPEProcessBufSize;
                                      
                newDLOutBuffer->pRead = newDLOutBuffer->pBufBase;
                newDLOutBuffer->pWrite= newDLOutBuffer->pBufBase;

                newULOutBuffer->pBufBase = (short*) malloc(mSPEProcessBufSize);
                newULOutBuffer->BufLen= mSPEProcessBufSize;
                                      
                newULOutBuffer->pRead = newULOutBuffer->pBufBase;
                newULOutBuffer->pWrite= newULOutBuffer->pBufBase;
 //                     ALOGD("newDLOutBuffer->pBufBase=%p,newDLOutBuffer->pRead=%p,newDLOutBuffer->pWrite=%p,newDLOutBuffer->BufLen=%d",newDLOutBuffer->pBufBase,newDLOutBuffer->pRead,newDLOutBuffer->pWrite,newDLOutBuffer->BufLen);
                int indexOut=0;
   
                int copysizetest = newDLOutBuffer->BufLen>>2;
                while(copysizetest)
                { 
                    //ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
                    *(newDLOutBuffer->pWrite) = *(mpSPEBufferFE + indexOut);
                    *(newDLOutBuffer->pWrite+1) = *(mpSPEBufferFE + indexOut);
//                    *(newDLOutBuffer->pWrite) = *(mpSPEBufferDL + indexOut);
//                    *(newDLOutBuffer->pWrite+1) = *(mpSPEBufferDL + indexOut);

                    *(newULOutBuffer->pWrite) = *(mpSPEBufferNE + indexOut);
                    *(newULOutBuffer->pWrite+1) = *(mpSPEBufferNE + indexOut);
//                    *(newULOutBuffer->pWrite) = *(mpSPEBufferUL1 + indexOut);
//                    *(newULOutBuffer->pWrite+1) = *(mpSPEBufferUL2 + indexOut);

//                    ALOGD("indexOut=%d,mpSPEBufferFE =%d, mpSPEBufferNE=%d",indexOut,*(mpSPEBufferFE + indexOut),*(mpSPEBufferNE + indexOut));
                    newDLOutBuffer->pWrite+=2;
                    newULOutBuffer->pWrite+=2;
                    indexOut++;
                    copysizetest--;
                }
                     
                mDLOutBufferQ.add(newDLOutBuffer);
   	            mDLOutBufQLenTotal += newDLOutBuffer->BufLen;
//   	            ALOGD("queue to DLOut mDLOutBufQLenTotal=%d, size=%d",mDLOutBufQLenTotal,mDLOutBufferQ.size());
   	            Dump_PCM_Process(DOWNLINK,newDLOutBuffer->pBufBase,newDLOutBuffer->BufLen);
                mULOutBufferQ.add(newULOutBuffer);
   	            mULOutBufQLenTotal += newULOutBuffer->BufLen;
   	            Dump_PCM_Process(UPLINK,newULOutBuffer->pBufBase,newULOutBuffer->BufLen);
            }
//            ALOGD("return SPELayer::Process, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d,mDLInBufferQ[0]->BufLen=%d",mDLInBufferQ.size(),mDLInBufQLenTotal,mDLInBufferQ[0]->BufLen);

        }
        else    //not enough DL or UP link data, not process
        {
            ALOGD("not enough DL/UPlink data, not process");
        }
    }


    //process the output buffer queue
    if(mMode == SPE_MODE_REC)
    {
//        ALOGD("mULOutBufferQ=%d, mULOutBufQLenTotal=%d",mULOutBufferQ.size(),mULOutBufQLenTotal);
    
        if(mULOutBufferQ.isEmpty() || mULOutBufQLenTotal<inBufLength)
        {
            ALOGD("SPELayer not enought UL output buffer");
            memset(inBuf,0,inBufLength);  //return in same input buffer address
            //memset(outBuf,0,inBufLength);   //return in output buffer address
            return true;
        }
        int tmpInBufLength = inBufLength;
        int count=0;
        int tempULCopy= mULOutBufferQ[0]->BufLen>>2;
        while(tmpInBufLength)
        {
            if(mULOutBufferQ.isEmpty())
            {
                ALOGD("SPELayer::uplink Output buffer queue is empty, something wrong!!");
                mError = true;
                break;
            }
            

//            ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

            if(tempULCopy>0)   //get the buffer data from the first uplink input buffer queue
            {
//                ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
//                ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                *(inBuf+count) = *(mULOutBufferQ[0]->pRead);
                *(inBuf+count+1) = *(mULOutBufferQ[0]->pRead +1);
                mULOutBufferQ[0]->pRead+=2;
                tmpInBufLength-=4;  //int and short transform
                tempULCopy--;
                count+=2;
                mULOutBufQLenTotal-=4;   //int and short transform
                mULOutBufferQ[0]->BufLen-=4;
            }
            else    //consume all the data in first queue buffer
            {
                free(mULOutBufferQ[0]->pBufBase);
                delete mULOutBufferQ[0];
                mULOutBufferQ.removeAt(0);
                tempULCopy= mULOutBufferQ[0]->BufLen>>2;
//                ALOGD("SPELayer::uplink Output buffer consumed");
            }
         
        }
        
    }
    else    //VoIP mode
    {
//        ALOGD("mULOutBufferQ=%d, mULOutBufQLenTotal=%d, mDLOutBufferQ=%d,mDLOutBufQLenTotal=%d",mULOutBufferQ.size(),mULOutBufQLenTotal,mDLOutBufferQ.size(),mDLOutBufQLenTotal);

        if(dir == UPLINK)
        {

            if(mULOutBufferQ.isEmpty() || mULOutBufQLenTotal<inBufLength)
            {
                ALOGD("SPELayer not enought UL output buffer");
                memset(inBuf,0,inBufLength);  //return in same input buffer address
                //memset(outBuf,0,inBufLength);   //return in output buffer address
                return true;
            }
            int tmpInBufLength = inBufLength;
            int count=0;
            int tempULCopy= mULOutBufferQ[0]->BufLen>>2;
            while(tmpInBufLength)
            {
                if(mULOutBufferQ.isEmpty())
                {
                    ALOGD("SPELayer::uplink Output buffer queue is empty, something wrong!!");
                    break;
                }
            

        //            ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

                if(tempULCopy>0)   //get the buffer data from the first uplink input buffer queue
                {
    //                ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
    //                ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                    *(inBuf+count) = *(mULOutBufferQ[0]->pRead);
                    *(inBuf+count+1) = *(mULOutBufferQ[0]->pRead +1);
                    mULOutBufferQ[0]->pRead+=2;
                    tmpInBufLength-=4;  //int and short transform
                    tempULCopy--;
                    count+=2;
                    mULOutBufQLenTotal-=4;   //int and short transform
                    mULOutBufferQ[0]->BufLen-=4;
                }
                else    //consume all the data in first queue buffer
                {
                    free(mULOutBufferQ[0]->pBufBase);
                    delete mULOutBufferQ[0];
                    mULOutBufferQ.removeAt(0);
                    tempULCopy= mULOutBufferQ[0]->BufLen>>2;
//                    ALOGD("SPELayer::uplink Output buffer consumed");
                }
            
            }
        }
        else    //downlink output
        {    
            if(mDLOutBufferQ.isEmpty() || mDLOutBufQLenTotal<inBufLength)
            {
                ALOGD("SPELayer not enought DL output buffer");
                memset(inBuf,0,inBufLength);  //return in same input buffer address
                //memset(outBuf,0,inBufLength);   //return in output buffer address
                return true;
            }
            int tmpInBufLength = inBufLength;
            int count=0;
            int tempDLCopy= mDLOutBufferQ[0]->BufLen>>2;
            while(tmpInBufLength)
            {
                if(mDLOutBufferQ.isEmpty())
                {
                    ALOGD("SPELayer::downlink Output buffer queue is empty, something wrong!!");
                    mError = true;
                    break;
                }            

                if(tempDLCopy>0)   //get the buffer data from the first uplink input buffer queue
                {
    //                ALOGD("mDLOutBufferQ[0]->pRead = %p,mDLOutBufferQ[0]->pBufBase=%p,mDLOutBufferQ[0]->BufLen=%d",mDLOutBufferQ[0]->pRead,mDLOutBufferQ[0]->pBufBase,mDLOutBufferQ[0]->BufLen);
    //                ALOGD("tmpInBufLength=%d,count=%d,mDLOutBufQLenTotal=%d,tempDLCopy=%d",tmpInBufLength,count,mDLOutBufQLenTotal,tempDLCopy);
                    *(inBuf+count) = *(mDLOutBufferQ[0]->pRead);
                    *(inBuf+count+1) = *(mDLOutBufferQ[0]->pRead +1);
                    mDLOutBufferQ[0]->pRead+=2;
                    tmpInBufLength-=4;  //int and short transform
                    tempDLCopy--;
                    count+=2;
                    mDLOutBufQLenTotal-=4;   //int and short transform
                    mDLOutBufferQ[0]->BufLen-=4;
                }
                else    //consume all the data in first queue buffer
                {
                    free(mDLOutBufferQ[0]->pBufBase);
                    delete mDLOutBufferQ[0];
                    mDLOutBufferQ.removeAt(0);
                    tempDLCopy= mDLOutBufferQ[0]->BufLen>>2;
                    ALOGD("SPELayer::downlink Output buffer consumed");
                }
            
            }
        }
    }
    
	Dump_PCM_Out(dir,inBuf,inBufLength);
	return true;
}


bool	SPELayer::Stop()
{
    ALOGD("SPELayer::Stop");
    Mutex::Autolock lock(mLock);

    if(mState == SPE_STATE_IDLE)
    {
        ALOGD("not start before");
        return false;
    }

	Clear();
	return true;
}


void SPELayer::ReStart()
{
    ALOGD("SPELayer::ReStart, State=%d, mode=%d",mState,mMode);
    Stop();
    Start(mMode);
}

void SPELayer::Clear()
{
    ALOGD("SPELayer::Clear");            		    
    
	if(mSphCtrlBuffer)
	{
        ALOGD("free mSphCtrlBuffer %p",mSphCtrlBuffer);
        ENH_API_Free(&mSph_Enh_ctrl);
	    free(mSphCtrlBuffer);
	    mSphCtrlBuffer = NULL;
	    ALOGD("~free mSphCtrlBuffer");
	}

    mpSPEBufferUL1 = NULL;
    mpSPEBufferUL2 = NULL;
    mpSPEBufferDL = NULL;
    mpSPEBufferDLDelay= NULL;

    mNeedDelayLatency = false;    

    mState = SPE_STATE_IDLE;

    //clear the buffer queue

    ALOGD("SPELayer::mULOutBufferQ size=%d,mULInBufferQ.size=%d,mDLOutBufferQ.size()=%d,mDLInBufferQ.size()=%d,mDLDelayBufferQ.size()=%d",mULOutBufferQ.size(),mULInBufferQ.size(),mDLOutBufferQ.size(),mDLInBufferQ.size(),mDLDelayBufferQ.size());
    if(mULOutBufferQ.size()!=0)
    {
        while(mULOutBufferQ.size())
        {
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
        }
        mULOutBufferQ.clear();
    }
    if(mULInBufferQ.size()!=0)
    {
        while(mULInBufferQ.size())
        {
            free(mULInBufferQ[0]->pBufBase);
            delete mULInBufferQ[0];
            mULInBufferQ.removeAt(0);
        }
        mULInBufferQ.clear();
    }

    if(mDLOutBufferQ.size()!=0)
    {
        while(mDLOutBufferQ.size())
        {
            free(mDLOutBufferQ[0]->pBufBase);
            delete mDLOutBufferQ[0];
            mDLOutBufferQ.removeAt(0);
        }
        mDLOutBufferQ.clear();
    }
    if(mDLInBufferQ.size()!=0)
    {
        while(mDLInBufferQ.size())
        {            
            if(mDLInBufferQ[0]->pBufBase)
            {
                ALOGD("mDLInBufferQ::pBufBase=%d",mDLInBufferQ[0]->pBufBase);
//                free(mDLInBufferQ[0]->pBufBase);
                ALOGD("mDLInBufferQ::free");
//                delete mDLInBufferQ[0];
                ALOGD("mDLInBufferQ::delete");
                mDLInBufferQ.removeAt(0);
                ALOGD("mDLInBufferQ::done, free at DLDelay buffer");
            }            
        }
        mDLInBufferQ.clear();
    }

    if(mDLDelayBufferQ.size()!=0)
    {
        while(mDLDelayBufferQ.size())
        {
            if(mDLDelayBufferQ[0]->pBufBase)
            {
                ALOGD("mDLDelayBufferQ::pBufBase=%d",mDLDelayBufferQ[0]->pBufBase);
                free(mDLDelayBufferQ[0]->pBufBase);
                ALOGD("mDLDelayBufferQ::free");
                delete mDLDelayBufferQ[0];
                ALOGD("mDLDelayBufferQ::delete");
                mDLDelayBufferQ.removeAt(0);
                ALOGD("mDLDelayBufferQ::done");
            }
            
        }
        mDLDelayBufferQ.clear();
    }

    if(mfpInDL)
    {
        fclose(mfpInDL);
        mfpInDL = NULL;
    }
    if(mfpInUL)
    {
        fclose(mfpInUL);
        mfpInUL = NULL;
    }
    if(mfpOutDL)
    {
        fclose(mfpOutDL);
        mfpOutDL = NULL;
    }
    if(mfpOutUL)
    {
        fclose(mfpOutUL);
        mfpOutUL = NULL;
    }
    if(mfpProcessedDL)
    {
        fclose(mfpProcessedDL);
        mfpProcessedDL = NULL;
    }
    if(mfpProcessedUL)
    {
        fclose(mfpProcessedUL);
        mfpProcessedUL = NULL;
    }
    if(mfpEPL)
    {
        fclose(mfpEPL);
        mfpEPL = NULL;
    }    
    if(mfpVM)
    {
        fclose(mfpVM);
        mfpVM = NULL;
    }
    
    if(hDumpThread != NULL)
    {
        hDumpThread = NULL;
    }
    ALOGD("~Clear");
}

void SPELayer::dump()
{
	ALOGD("SPELayer::dump, State=%d, mode=%d",mState,mMode);	
	//dump normal record parameters
/*	ALOGD("Record:Samplerate = %d, FrameRate=%d,PGAGain=%d, App_table=%x, Fea_Cfg_table=%x",mRecordSampleRate,mRecordFrameRate,mRecordPGAGain,mRecordApp_table,mRecordFea_Cfg_table);
	ALOGD("Record:EnhanceParas");
	for(int i=0; i<EnhanceParasNum; i++)
		ALOGD("%d",mRecordEnhanceParas[i]);
	ALOGD("Record:DMNRCalData");
	for(int i=0; i<DMNRCalDataNum; i++)
		ALOGD("%d",mRecordDMNRCalData[i]);
	ALOGD("Record:CompenFilter");
	for(int i=0; i<CompenFilterNum; i++)
		ALOGD("%d",mRecordCompenFilter[i]);
		
		
	//dump VoIP parameters
	ALOGD("VoIP:Samplerate = %d, FrameRate=%d,PGAGain=%d, App_table=%x, Fea_Cfg_table=%x",mVoIPSampleRate,mVoIPFrameRate,mVoIPPGAGain,mVoIPApp_table,mVoIPFea_Cfg_table);
	ALOGD("VoIP:EnhanceParas");
	for(int i=0; i<EnhanceParasNum; i++)
		ALOGD("%d",mVoIPEnhanceParas[i]);
	ALOGD("VoIP:DMNRCalData");
	for(int i=0; i<DMNRCalDataNum; i++)
		ALOGD("%d",mVoIPDMNRCalData[i]);
	ALOGD("VoIP:CompenFilter");
	for(int i=0; i<CompenFilterNum; i++)
		ALOGD("%d",mVoIPCompenFilter[i]);
*/		
	//dump using parameters
	ALOGD("Using:Samplerate = %d, FrameRate=%d,MIC_DG=%d, App_table=%x, Fea_Cfg_table=%x",mSph_Enh_ctrl.sample_rate,mSph_Enh_ctrl.frame_rate,mSph_Enh_ctrl.MIC_DG, mSph_Enh_ctrl.App_table,mSph_Enh_ctrl.Fea_Cfg_table);
	ALOGD("Using:EnhanceParas");
	for(int i=0; i<EnhanceParasNum; i++)
		ALOGD("%d",mSph_Enh_ctrl.enhance_pars[i]);
	ALOGD("Using:DMNRCalData");
	for(int i=0; i<DMNRCalDataNum; i++)
		ALOGD("%d",mSph_Enh_ctrl.DMNR_cal_data[i]);
	ALOGD("Using:CompenFilter");
	for(int i=0; i<CompenFilterNum; i++)
		ALOGD("%d",mSph_Enh_ctrl.Compen_filter[i]);
	
}

static int checkAndCreateDirectory(const char * pC)
{
    char tmp[PATH_MAX];
    int i = 0;

    while(*pC)
    {
        tmp[i] = *pC;

        if(*pC == '/' && i)
        {
            tmp[i] = '\0';
            if(access(tmp, F_OK) != 0)
            {
                if(mkdir(tmp, 0770) == -1)
                {
                	ALOGE("AudioDumpPCM: mkdir error! %s\n",(char*)strerror(errno));
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }
	return 0;

}

bool SPELayer::HasBufferDump()
{
    Mutex::Autolock lock(mDumpLock);
    if(mDumpDLInBufferQ.size()==0 && mDumpDLOutBufferQ.size()==0 && mDumpULInBufferQ.size()==0 && mDumpULOutBufferQ.size()==0 
        && mDumpEPLBufferQ.size()==0)
        return false;

    return true;
}

void *DumpThread(void *arg)
{
    SPELayer *pSPEL = (SPELayer*)arg;
    ALOGD( "DumpThread");

    while(1)
    {
        if(pSPEL->hDumpThread==NULL)
        {
            ALOGD( "DumpThread hDumpThread null");
            if(!pSPEL->HasBufferDump())
            {
                pSPEL->DumpMutexLock();
                pSPEL->mDumpDLInBufferQ.clear();
                pSPEL->mDumpDLOutBufferQ.clear();
                pSPEL->mDumpULInBufferQ.clear();
                pSPEL->mDumpULOutBufferQ.clear();
                pSPEL->mDumpEPLBufferQ.clear();
                pSPEL->DumpMutexUnlock();
                ALOGD( "DumpThread exit");
                break;
            }
        }
        if(!pSPEL->HasBufferDump())
        {
            usleep(10*1000);
            continue;
        }
        //ALOGD( "DumpThread,mDumpULInBufferQ=%d, mDumpULOutBufferQ=%d, mDumpEPLBufferQ=%d",pSPEL->mDumpULInBufferQ.size(),pSPEL->mDumpULOutBufferQ.size(),pSPEL->mDumpEPLBufferQ.size());
        if(pSPEL->mDumpDLInBufferQ.size()>0)
        {
            fwrite(pSPEL->mDumpDLInBufferQ[0]->pBufBase,pSPEL->mDumpDLInBufferQ[0]->BufLen,1,pSPEL->mfpInDL);
            pSPEL->DumpMutexLock();
            free(pSPEL->mDumpDLInBufferQ[0]->pBufBase);
            delete pSPEL->mDumpDLInBufferQ[0];
            pSPEL->mDumpDLInBufferQ.removeAt(0);
            pSPEL->DumpMutexUnlock();
        }

        if(pSPEL->mDumpDLOutBufferQ.size()>0)
        {
            fwrite(pSPEL->mDumpDLOutBufferQ[0]->pBufBase,pSPEL->mDumpDLOutBufferQ[0]->BufLen,1,pSPEL->mfpOutDL);
            pSPEL->DumpMutexLock();
            free(pSPEL->mDumpDLOutBufferQ[0]->pBufBase);
            delete pSPEL->mDumpDLOutBufferQ[0];
            pSPEL->mDumpDLOutBufferQ.removeAt(0);
            pSPEL->DumpMutexUnlock();
        }
        
        if(pSPEL->mDumpULInBufferQ.size()>0)
        {
            fwrite(pSPEL->mDumpULInBufferQ[0]->pBufBase,pSPEL->mDumpULInBufferQ[0]->BufLen,1,pSPEL->mfpInUL);
            pSPEL->DumpMutexLock();
            free(pSPEL->mDumpULInBufferQ[0]->pBufBase);
            delete pSPEL->mDumpULInBufferQ[0];
            pSPEL->mDumpULInBufferQ.removeAt(0);
            pSPEL->DumpMutexUnlock();
        }
        
        if(pSPEL->mDumpULOutBufferQ.size()>0)
        {
            fwrite(pSPEL->mDumpULOutBufferQ[0]->pBufBase,pSPEL->mDumpULOutBufferQ[0]->BufLen,1,pSPEL->mfpOutUL);
            pSPEL->DumpMutexLock();
            free(pSPEL->mDumpULOutBufferQ[0]->pBufBase);
            delete pSPEL->mDumpULOutBufferQ[0];
            pSPEL->mDumpULOutBufferQ.removeAt(0);
            pSPEL->DumpMutexUnlock();
        }
        
        if(pSPEL->mDumpEPLBufferQ.size()>0)
        {
            fwrite(pSPEL->mDumpEPLBufferQ[0]->pBufBase,pSPEL->mDumpEPLBufferQ[0]->BufLen,1,pSPEL->mfpEPL);
            pSPEL->DumpMutexLock();
            free(pSPEL->mDumpEPLBufferQ[0]->pBufBase);
            delete pSPEL->mDumpEPLBufferQ[0];
            pSPEL->mDumpEPLBufferQ.removeAt(0);
            pSPEL->DumpMutexUnlock();
        }
    }

    ALOGD( "DumpThread exit!!");
    pthread_exit(NULL);
    return 0;
}

bool SPELayer::CreateDumpThread()
{
#if defined(PC_EMULATION)
    hDumpThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DumpThread,this,0,0);
    if(hDumpThread == 0) return false;
       return true;
#else
    //create PCM data dump thread here
    int ret;
    ret = pthread_create( &hDumpThread, NULL, DumpThread, (void*)this);
    if ( ret != 0 ) return false;
    
    ALOGD("-CreateDumpThread \n");
    return true;
#endif

}

void SPELayer::Dump_PCM_In(SPE_DATA_DIRECTION dir, void* buffer, int bytes)
{
    //set this property to dump data
    char value[PROPERTY_VALUE_MAX];
    property_get("SPEIn.pcm.dump", value, "0");
    int bflag=atoi(value);
    if(bflag)
    {
    	int ret;
    	if(hDumpThread == NULL)
    	    CreateDumpThread();
    	    
		if(dir == UPLINK)
        {
           char filename[]="/sdcard/mtklog/audio_dump/SPEIn_Uplink.pcm";

		   ret = checkAndCreateDirectory(filename);
		   if(ret<0)
		   {
			   ALOGE("SPELayer::Dump_PCM_In UPLINK checkAndCreateDirectory() fail!!!");
		   }
		   else
		   {
				if(mfpInUL==NULL)
				{
				  mfpInUL= fopen(filename, "ab+");
				}
				if(mfpInUL!=NULL)
				{
                    BufferInfo* newInBuffer=new BufferInfo;
                    newInBuffer->pBufBase = (short*) malloc(bytes);    
                    memcpy(newInBuffer->pBufBase, buffer, bytes);
                    
                    newInBuffer->BufLen= bytes;
                    newInBuffer->pRead = newInBuffer->pBufBase;
                    newInBuffer->pWrite= newInBuffer->pBufBase;
                    DumpMutexLock();
   	                mDumpULInBufferQ.add(newInBuffer);
   	                DumpMutexUnlock();
                    
				}
				else
				{
				  ALOGD("open  SPEIn_Uplink.pcm fail");
				}
			}
        }
        else
        {
           char filename[]="/sdcard/mtklog/audio_dump/SPEIn_Downlink.pcm";

		   ret = checkAndCreateDirectory(filename);
		   if(ret<0)
		   {
			   ALOGE("SPELayer::Dump_PCM_In DOWNLINK checkAndCreateDirectory() fail!!!");
		   }
		   else
		   {			
	            if(mfpInDL==NULL)
	            {
	                mfpInDL= fopen(filename, "ab+");
	            }
	            
	            if(mfpInDL!=NULL)
	            {
	                BufferInfo* newInBuffer=new BufferInfo;
                    newInBuffer->pBufBase = (short*) malloc(bytes);    
                    memcpy(newInBuffer->pBufBase, buffer, bytes);
                    
                    newInBuffer->BufLen= bytes;
                    newInBuffer->pRead = newInBuffer->pBufBase;
                    newInBuffer->pWrite= newInBuffer->pBufBase;
                    DumpMutexLock();
   	                mDumpDLInBufferQ.add(newInBuffer);
   	                DumpMutexUnlock();
	            }
	            else
	            {
	                ALOGD("open  SPEIn_Downlink.pcm fail");
	            }
		   }
        }
    }
}

void SPELayer::Dump_PCM_Process(SPE_DATA_DIRECTION dir, void* buffer, int bytes)
{
    //set this property to dump data
    char value[PROPERTY_VALUE_MAX];
    property_get("SPE.pcm.dump", value, "0");
    int bflag=atoi(value);
    if(bflag)
    {
    	int ret;
        if(dir == UPLINK)
        {
            char filename[]="/sdcard/mtklog/audio_dump/SPE_Uplink.pcm";
			ret = checkAndCreateDirectory(filename);
		   	if(ret<0)
		   	{
			   ALOGE("SPELayer::Dump_PCM_Process UPLINK checkAndCreateDirectory() fail!!!");
		   	}
		   	else
		   	{
	            if(mfpProcessedUL==NULL)
	            {
	                mfpProcessedUL= fopen(filename, "ab+");
	            }
	            if(mfpProcessedUL!=NULL)
	            {
	                fwrite(buffer,bytes,1,mfpProcessedUL);
	            }
	            else
	            {
	                ALOGD("open SPE_Uplink.pcm fail");
	            }    
		   	}
        }
        else
        {
            char filename[]="/sdcard/mtklog/audio_dump/SPE_Downlink.pcm";
			ret = checkAndCreateDirectory(filename);
		   	if(ret<0)
		   	{
			   ALOGE("SPELayer::Dump_PCM_Process DOWNLINK checkAndCreateDirectory() fail!!!");
		   	}
		   	else
		   	{
	            if(mfpProcessedDL==NULL)
	            {
	                mfpProcessedDL= fopen(filename, "ab+");
	            }
	            
	            if(mfpProcessedDL!=NULL)
	            {
	                fwrite(buffer,bytes,1,mfpProcessedDL);
	            }
	            else
	            {
	                ALOGD("open SPE_Downlink.pcm fail");
	            }
		   	}
        }
    }
}

void SPELayer::Dump_PCM_Out(SPE_DATA_DIRECTION dir, void* buffer, int bytes)
{
    //set this property to dump data
    char value[PROPERTY_VALUE_MAX];
    property_get("SPEOut.pcm.dump", value, "0");
    int bflag=atoi(value);
    if(bflag)
    {
    	int ret;
    	if(hDumpThread == NULL)
    	    CreateDumpThread();
        if(dir == UPLINK)
        {
            char filename[]="/sdcard/mtklog/audio_dump/SPEOut_Uplink.pcm";
			ret = checkAndCreateDirectory(filename);
		   	if(ret<0)
		   	{
			   ALOGE("SPELayer::Dump_PCM_Out UPLINK checkAndCreateDirectory() fail!!!");
		   	}
		   	else
		   	{
	            if(mfpOutUL==NULL)
	            {
	                mfpOutUL= fopen(filename, "ab+");
	            }
	            
	            if(mfpOutUL!=NULL)
	            {
                    BufferInfo* newInBuffer=new BufferInfo;
                    newInBuffer->pBufBase = (short*) malloc(bytes);    
                    memcpy(newInBuffer->pBufBase, buffer, bytes);
                    
                    newInBuffer->BufLen= bytes;
                    newInBuffer->pRead = newInBuffer->pBufBase;
                    newInBuffer->pWrite= newInBuffer->pBufBase;
                    DumpMutexLock();
   	                mDumpULOutBufferQ.add(newInBuffer);
   	                DumpMutexUnlock();
	            }
	            else
	            {
	                ALOGD("open SPEOut_Uplink.pcm fail");
	            }
		   	}
        }
        else
        {
            char filename[]="/sdcard/mtklog/audio_dump/SPEOut_Downlink.pcm";
			ret = checkAndCreateDirectory(filename);
		   	if(ret<0)
		   	{
			   ALOGE("SPELayer::Dump_PCM_Out DOWNLINK checkAndCreateDirectory() fail!!!");
		   	}
		   	else
		   	{
	            if(mfpOutDL==NULL)
	            {
	                mfpOutDL= fopen(filename, "ab+");
	            }
	            if(mfpOutDL!=NULL)
	            {
	                BufferInfo* newInBuffer=new BufferInfo;
                    newInBuffer->pBufBase = (short*) malloc(bytes);    
                    memcpy(newInBuffer->pBufBase, buffer, bytes);
                    
                    newInBuffer->BufLen= bytes;
                    newInBuffer->pRead = newInBuffer->pBufBase;
                    newInBuffer->pWrite= newInBuffer->pBufBase;
                    DumpMutexLock();
   	                mDumpDLOutBufferQ.add(newInBuffer);
   	                DumpMutexUnlock();
	            }
	            else
	            {
	                ALOGD("open SPEOut_Downlink.pcm fail");
	            }
		   	}
        }
    }
}

void SPELayer::Dump_EPL(void* buffer, int bytes)
{
    //set this property to dump data
   char value[PROPERTY_VALUE_MAX];
   property_get("SPE_EPL", value, "0");
   int bflag=atoi(value);
   if(bflag)
   {
  	  int ret;
      char filename[]="/sdcard/mtklog/audio_dump/SPE_EPL.EPL";

	  if(hDumpThread == NULL)
    	    CreateDumpThread();
    	    
	  ret = checkAndCreateDirectory(filename);
	  if(ret<0)
	  {
	  	ALOGE("SPELayer::Dump_EPL checkAndCreateDirectory() fail!!!");
	  }
	  else
	  {
            if(mfpEPL==NULL)
            {
                mfpEPL= fopen(filename, "ab+");
            }
	            
            if(mfpEPL!=NULL)
            {
                BufferInfo* newInBuffer=new BufferInfo;
                newInBuffer->pBufBase = (short*) malloc(bytes);    
                memcpy(newInBuffer->pBufBase, buffer, bytes);
                    
                newInBuffer->BufLen= bytes;
                newInBuffer->pRead = newInBuffer->pBufBase;
                newInBuffer->pWrite= newInBuffer->pBufBase;
                DumpMutexLock();
   	            mDumpEPLBufferQ.add(newInBuffer);
   	            DumpMutexUnlock();
	      }
	      else
	      {
	         ALOGD("open  SPE_EPL.EPL fail");
	      }
	  }
   }   
}

void SPELayer::EPLTransVMDump()
{
    
    char value[PROPERTY_VALUE_MAX];
    property_get("APVM.dump", value, "0");
    int bflag=atoi(value);
    if(bflag || mVMDumpEnable)
    {
        int ret;
        char filename[]="/sdcard/mtklog/audio_dump/SPE.VM";
        if(bflag)
        {
            memset(mVMDumpFileName, 0, 128);
            strcpy(mVMDumpFileName,filename);
        }
    	if(mVMDumpFileName==NULL)
    	    ALOGE("no mVMDumpFileName name?");
    	    
        ret = checkAndCreateDirectory(mVMDumpFileName);
        if(ret<0)
        {
            ALOGE("EPLTransVMDump checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if(mfpVM==NULL && mVMDumpFileName!=NULL)
            {
                mfpVM= fopen(mVMDumpFileName, "ab+");
            }
        }
        
        if(mfpVM!=NULL)
        {
            if(mSph_Enh_ctrl.sample_rate==48000)
            {
/*        memcpy(mVM, mSph_Enh_ctrl.EPL_buffer, RecBufSize20ms*2*sizeof(short));
        mVM[MaxVMSize-2] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
        mVM[MaxVMSize-1] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
*/        
                ALOGD("EPLTransVMDump 48k write to sdcard");
                for(int i =0; i<MaxVMSize; i++)
                {
                    if(i==(MaxVMSize-2))
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                    else if(i==(MaxVMSize-1))
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                    else
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[i];
                }
//                ALOGE("EPLTransVMDump write to sdcard");
                fwrite(mVM,MaxVMSize*sizeof(short),1,mfpVM);
            }
            else    //suppose only 16K
            {
/*        memcpy(mVM, &mSph_Enh_ctrl.EPL_buffer[160*4], 320*2*sizeof(short));
        mVM[640] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
        mVM[641] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
*/        
                ALOGD("EPLTransVMDump 16k write to sdcard");
                for(int i =0; i<642; i++)   //320*2+2
                {
                    if(i==640)
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                    else if(i==641)
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                    else
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[640+i];
                }
                fwrite(mVM,642*sizeof(short),1,mfpVM);
            }
        }
        else
        {
            ALOGD("open APVM.dump fail");
        }
    }
}

void SPELayer::SetVMDumpEnable(bool bEnable)
{
    ALOGD("SetVMDumpEnable bEnable=%x",bEnable);
    mVMDumpEnable = bEnable;
}

void SPELayer::SetVMDumpFileName(const char * VMFileName)
{
    ALOGD("+++SetVMDumpFileName VMFileName=%s",VMFileName);
    memset(mVMDumpFileName, 0, 128);
    strcpy(mVMDumpFileName,VMFileName);
    ALOGD("---SetVMDumpFileName VMFileName=%s, mVMDumpFileName=%s",VMFileName,mVMDumpFileName);
}

// ----------------------------------------------------------------------------
}

