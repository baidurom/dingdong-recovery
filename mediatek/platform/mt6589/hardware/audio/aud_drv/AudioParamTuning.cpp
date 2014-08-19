/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioParamTuning.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the method for  handling param tuning.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *******************************************************************************/

#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <cutils/xlog.h>

#include "AudioParamTuning.h"
#include "AudioCustParam.h"
//#include "AudioUtility.h"

#include "AudioVolumeFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"
#include "SpeechDriverInterface.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

//#define PLAYBUF_SIZE 6400l
#define PLAYBUF_SIZE 16384
#define A2M_SHARED_BUFFER_OFFSET  (1408)
#define WAV_HEADER_SIZE 44

// define in AudioMtkVolumeControler.cpp
#define  AUDIO_BUFFER_HW_GAIN_STEP (13)

#define LOG_TAG "AudioParamTuning"
namespace android {

static void *Play_PCM_With_SpeechEnhance_Routine(void *arg)
{
/*
    SXLOGD("Play_PCM_With_SpeechEnhance_Routine in +");
    AudioParamTuning *pAUDParamTuning = (AudioParamTuning*)arg;	
	
    if(pAUDParamTuning == NULL) {
        SXLOGE("Play_PCM_With_SpeechEnhance_Routine pAUDParamTuning = NULL arg = %x",arg);
        return 0;
    }

    uint32 PCM_BUF_SIZE = pAUDParamTuning->m_bWBMode?(2*PLAYBUF_SIZE):(PLAYBUF_SIZE);
    unsigned long sleepTime = (PLAYBUF_SIZE/320)*20*1000;
    // open AudioRecord
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"PlaybackWithSphEnRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    //Prepare file pointer
    FILE* pFd = pAUDParamTuning->m_pInputFile;                 //file for input

    // Prepare input & output memory
    RingBuf* playBuf = &pAUDParamTuning->m_sPlayBuf;
    playBuf->pBufBase = new char[PCM_BUF_SIZE];
    playBuf->pRead    = playBuf->pBufBase;
    playBuf->pWrite   = playBuf->pBufBase;
    playBuf->bufLen   = PCM_BUF_SIZE;
    memset((void*)playBuf->pBufBase , 0, PCM_BUF_SIZE);

    // ----start the loop --------
    pAUDParamTuning->m_bPPSThreadExit = false;
    int numOfBytesPlay =0;
    int playBufFreeCnt = 0;
    int totalBytes =0;
    int cntR = 0;
    char* tmp = new char[PCM_BUF_SIZE+1];
    fread(tmp, sizeof(char), WAV_HEADER_SIZE, pFd);
    memset(tmp, 0, PCM_BUF_SIZE+1);

    SXLOGD("pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond), buffer size=%d",PCM_BUF_SIZE);
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);

    while((!pAUDParamTuning->m_bPPSThreadExit) && pFd) {
        pthread_mutex_lock(&pAUDParamTuning->mPlayBufMutex);

        //handling playback buffer
        playBufFreeCnt = playBuf->bufLen - rb_getDataCount(playBuf) - 1;
        cntR = fread(tmp, sizeof(char), playBufFreeCnt, pFd);
        if (cntR>0&&playBuf->pBufBase!=NULL) {
            rb_copyFromLinear(playBuf, tmp, cntR);
            numOfBytesPlay += cntR;
        }
        SXLOGD(" Playback buffer, readReq:%d, read:%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlay);
        pthread_mutex_unlock(&pAUDParamTuning->mPlayBufMutex);

        if (cntR<playBufFreeCnt) {
            SXLOGD("File reach the end");
            usleep(sleepTime); ////wait to all data is played
            break;
        }

        usleep(sleepTime/2);
    }

    // free buffer
    if (tmp!=NULL) {
        delete[] tmp;
        tmp = NULL;
    }
	
    pthread_mutex_lock(&pAUDParamTuning->mPlayBufMutex);
    if (playBuf->pBufBase!=NULL) {
        delete[] playBuf->pBufBase;
        playBuf->pBufBase = NULL;
    }
    pthread_mutex_unlock(&pAUDParamTuning->mPlayBufMutex);

    if (!pAUDParamTuning->m_bPPSThreadExit) {
        pAUDParamTuning->m_bPPSThreadExit = true;
        pAUDParamTuning->enableModemPlaybackVIASPHPROC(false);
        AudioTasteTuningStruct sRecoveryParam;
        sRecoveryParam.cmd_type = (unsigned short)AUD_TASTE_STOP;
        sRecoveryParam.wb_mode  = pAUDParamTuning->m_bWBMode;
        pAUDParamTuning->updataOutputFIRCoffes(&sRecoveryParam);      
    }
	
    //exit thread
    SXLOGD( "playbackRoutine pthread_mutex_lock");
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);
    SXLOGD("pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond)");
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);
*/
    return 0;

}

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
static void *DMNR_Play_Rec_Routine(void *arg)
{
    SXLOGD("DMNR_Play_Rec_Routine in +");
    AudioParamTuning *pDMNRTuning = (AudioParamTuning*)arg;	
    if(pDMNRTuning == NULL) {
        SXLOGE("DMNR_Play_Rec_Routine pDMNRTuning = NULL arg = %x",arg);
        return 0;
    }

    uint32 PCM_BUF_SIZE = pDMNRTuning->m_bWBMode ? 640 : 320;
    unsigned long sleepTime = (PLAYBUF_SIZE/PCM_BUF_SIZE)*20*1000;
    
    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
	
    //Prepare file pointer
    FILE* pInFp = pDMNRTuning->m_pInputFile;      //file for input
    FILE* pOutFp = pDMNRTuning->m_pOutputFile;    //file for input

    // ----start the loop --------
    char* tmp = new char[PLAYBUF_SIZE];
    pDMNRTuning->m_bDMNRThreadExit = false;
    int cntR = 0;
    int cntW = 0;
    int numOfBytesPlay =0;
    int numOfBytesRec =0;
    
    int playBufFreeCnt = 0;
    int recBufDataCnt = 0;

    SXLOGD("pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    while((!pDMNRTuning->m_bDMNRThreadExit)&&pOutFp) {
        //handling playback buffer
        pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
        if (pInFp) {
            playBufFreeCnt = pDMNRTuning->mPlay2WayInstance->GetFreeBufferCount() - 1;
            cntR = fread(tmp, sizeof(char), playBufFreeCnt, pInFp);
            pDMNRTuning->mPlay2WayInstance->Write(tmp, cntR);
            numOfBytesPlay += cntR;
            SXLOGV(" Playback buffer, free:%d, read from :%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlay);
        }
        pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);
		
        // handling record buffer
        pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
        recBufDataCnt = pDMNRTuning->mRec2WayInstance->GetBufferDataCount();
        pDMNRTuning->mRec2WayInstance->Read(tmp, recBufDataCnt);
        cntW = fwrite((void*)tmp,sizeof(char),recBufDataCnt,pOutFp);
        numOfBytesRec += cntW;
        SXLOGV(" Record buffer, available:%d, write to file:%d, total rec:%d", recBufDataCnt, cntW, numOfBytesRec);
        pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);

        usleep(sleepTime/2);
    }

    // free buffer
    delete[] tmp;
    tmp = NULL;

    //exit thread
    SXLOGD( "VmRecordRoutine pthread_mutex_lock");
    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);
    SXLOGD("pthread_cond_signal(&mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    return 0;
}
#endif

AudioParamTuning *AudioParamTuning::UniqueTuningInstance = 0;

AudioParamTuning *AudioParamTuning::getInstance()
{
    if (UniqueTuningInstance==0) {
        SXLOGD("create AudioParamTuning instance --");
        UniqueTuningInstance = new AudioParamTuning();
        SXLOGD("create AudioParamTuning instance ++");
    }

    return UniqueTuningInstance;
}

AudioParamTuning::AudioParamTuning() :
	mMode(0),
	mSideTone(0xFFFFFF40),
	m_bPlaying(false),
	m_bWBMode(false),
	m_bPPSThreadExit(false),
	m_pInputFile(NULL)
{
    SXLOGD("AudioParamTuning in +");
    // create volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();
    mAudioVolumeInstance->initCheck();

    // create digital control instnace
    mAudioDigitalInstance  = AudioDigitalControlFactory::CreateAudioDigitalControl();

    // create analog control instnace
    mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();

    // create audio resource manager instance
    mAudioResourceManager = AudioResourceManager::getInstance();

    // create speech driver instance
    mSpeechDriverFactory = SpeechDriverFactory::GetInstance();
	
    memset(mOutputVolume, 0, MODE_NUM*sizeof(uint32));
    memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX*sizeof(char));

    int ret = pthread_mutex_init(&mP2WMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize pthread mP2WMutex!");

    ret = pthread_mutex_init(&mPPSMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mPPSMutex!");
	
    ret = pthread_mutex_init(&mPlayBufMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mPlayBufMutex!");
	
    ret = pthread_cond_init(&mPPSExit_Cond, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mPPSExit_Cond!");

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    m_bDMNRPlaying = false;
    m_bDMNRThreadExit = false;
    m_pOutputFile = NULL;
    
    mPlay2WayInstance = 0;
    mRec2WayInstance = 0;
    
    memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX*sizeof(char));

    AUDIO_VER1_CUSTOM_VOLUME_STRUCT VolumeCustomParam;//volume custom data
    GetVolumeVer1ParamFromNV(&VolumeCustomParam);

    mDualMicTool_micGain = VolumeCustomParam.audiovolume_mic[VOLUME_NORMAL_MODE][3];
    if (mDualMicTool_micGain>UPLINK_GAIN_MAX)
        mDualMicTool_micGain = UPLINK_GAIN_MAX;

    mDualMicTool_receiverGain = VolumeCustomParam.audiovolume_sph[VOLUME_NORMAL_MODE][CUSTOM_VOLUME_STEP-1];
    if (mDualMicTool_receiverGain>MAX_VOICE_VOLUME)
        mDualMicTool_receiverGain = MAX_VOICE_VOLUME;
	
    mDualMicTool_headsetGain = VolumeCustomParam.audiovolume_sph[VOLUME_HEADSET_MODE][3];
    if (mDualMicTool_headsetGain>MAX_VOICE_VOLUME)
        mDualMicTool_headsetGain = MAX_VOICE_VOLUME;

    ret = pthread_mutex_init(&mDMNRMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mDMNRMutex!");
	
    ret = pthread_mutex_init(&mRecBufMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mRecBufMutex!");
	
    ret = pthread_cond_init(&mDMNRExit_Cond, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mDMNRExit_Cond!");
	
    SXLOGD("AudioYusuParamTuning: default mic gain:%d, receiver gain:%d, headset Gain:%d", mDualMicTool_micGain, mDualMicTool_receiverGain, mDualMicTool_headsetGain);
#endif

}
	
AudioParamTuning::~AudioParamTuning()
{
    SXLOGD("~AudioParamTuning in +");
}
	
//for taste tool
bool AudioParamTuning::isPlaying()
{
    SXLOGV("isPlaying - playing:%d", m_bPlaying);
    bool ret = false;
    pthread_mutex_lock(&mP2WMutex);
#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    SXLOGV("isPlaying - DMNR playing:%d", m_bDMNRPlaying);
    ret = (m_bPlaying | m_bDMNRPlaying)?true:false;
#else
    ret = m_bPlaying;
#endif
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

uint32 AudioParamTuning::getMode()
{
    SXLOGD("getMode - mode:%d", mMode);
    pthread_mutex_lock(&mP2WMutex);
    uint32 ret = mMode;
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::setMode(uint32 mode)
{
    SXLOGD("setMode - mode:%d", mode);
    pthread_mutex_lock(&mP2WMutex);
    mMode = mode;
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setPlaybackFileName(const char * fileName)
{
    SXLOGD("setPlaybackFileName in +");
    pthread_mutex_lock(&mP2WMutex);
    if (fileName!=NULL && strlen(fileName)<FILE_NAME_LEN_MAX-1) {
        SXLOGD("input file name:%s", fileName);
        memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX);
        strcpy(m_strInputFileName,fileName);
    }else {
        SXLOGE("input file name NULL or too long!");
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setDLPGA(uint32 gain)
{
    SXLOGD("setDLPGA in +");
    uint32 outputDev = 0;
	
    if (gain>MAX_VOICE_VOLUME) {
        SXLOGE("setDLPGA gain error  gain=%x",gain);
        return BAD_VALUE;
    }
	
    pthread_mutex_lock(&mP2WMutex);
    mOutputVolume[mMode] = gain;
    SXLOGD("setDLPGA mode=%d, gain=%d, lad volume=0x%x", mMode, gain, mOutputVolume[mMode]);

    if (m_bPlaying) {
        SXLOGD("setDLPGA lad_Volume=%x",mOutputVolume[mMode]);
        setSphVolume(mMode, mOutputVolume[mMode]);
    }

    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}
	 
void AudioParamTuning::updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam)
{
    SXLOGD("updataOutputFIRCoffes in +");
/*
    int ret = 0;
    unsigned short mode = pCustParam->phone_mode;
    unsigned short cmdType = pCustParam->cmd_type;

    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
	
    pthread_mutex_lock(&mP2WMutex);

    if (m_bPlaying&&mode==mMode){
        pSpeechDriver->PCM2WayOff(); // trun off PCM2Way
    //    mAudioResourceManager->StopInputDevice();
    //    mAudioResourceManager->TurnoffAudioDevice(mAudioResourceManager->getDlOutputDevice());
        usleep(10*1000);   //wait to make sure all message is processed
    }
	
    if (pCustParam->wb_mode) {
#if defined(MTK_WB_SPEECH_SUPPORT)
        AUDIO_CUSTOM_WB_PARAM_STRUCT sCustWbParam;
        GetCustWBParamFromNV(&sCustWbParam);
        if(cmdType&&sCustWbParam.speech_mode_wb_para[mode][7]!=pCustParam->dlDigitalGain) {
            SXLOGD("updataOutputFIRCoffes mode=%d, ori dlDG gain=%d, new dlDG gain=%d", mode, sCustWbParam.speech_mode_wb_para[mode][7],pCustParam->dlDigitalGain);
            sCustWbParam.speech_mode_wb_para[mode][7] = pCustParam->dlDigitalGain;
        }
        ret = pSpeechDriver->SetWBSpeechParameters((void *)&sCustWbParam);
#endif
    }else {
        AUDIO_CUSTOM_PARAM_STRUCT sCustParam;
        AUDIO_PARAM_MED_STRUCT sCustMedParam;
        unsigned short index = pCustParam->slected_fir_index;
        unsigned short dlGain = pCustParam->dlDigitalGain;
        GetCustParamFromNV(&sCustParam);
        GetMedParamFromNV(&sCustMedParam);

        if ((cmdType==(unsigned short)AUD_TASTE_START||cmdType==(unsigned short)AUD_TASTE_INDEX_SETTING)&&sCustMedParam.select_FIR_output_index[mode]!=index) {
            SXLOGD("updataOutputFIRCoffes mode=%d, old index=%d, new index=%d", mode, sCustMedParam.select_FIR_output_index[mode], index);
            //save  index to MED with different mode.
            sCustMedParam.select_FIR_output_index[mode]= index;

            SXLOGD("updataOutputFIRCoffes ori sph_out_fir[%d][0]=%d, ori sph_out_fir[%d][44]", mode,sCustParam.sph_out_fir[mode][0],mode,sCustParam.sph_out_fir[mode][44]);
            //copy med data into audio_custom param
            memcpy((void*)sCustParam.sph_out_fir[mode],(void*)sCustMedParam.speech_output_FIR_coeffs[mode][index],sizeof(sCustParam.sph_out_fir[index]));
            SXLOGD("updataOutputFIRCoffes new sph_out_fir[%d][0]=%d, new sph_out_fir[%d][44]", mode,sCustParam.sph_out_fir[mode][0],mode,sCustParam.sph_out_fir[mode][44]);
            SetCustParamToNV(&sCustParam);
            SetMedParamToNV(&sCustMedParam);
        }

        if ((cmdType==(unsigned short)AUD_TASTE_START||cmdType==(unsigned short)AUD_TASTE_DLDG_SETTING)&&sCustParam.speech_mode_para[mode][7]!=dlGain) {
            SXLOGD("updataOutputFIRCoffes mode=%d, old dlDGGain=%d, new dlDGGain=%d", mode, sCustParam.speech_mode_para[mode][7], dlGain);
            sCustParam.speech_mode_para[mode][7] = dlGain;
        }
        SXLOGD("updataOutputFIRCoffes  sph_out_fir[%d][0]=%d, sph_out_fir[%d][44]", mode,sCustParam.sph_out_fir[mode][0],mode,sCustParam.sph_out_fir[mode][44]);
        ret = pSpeechDriver->SetSpeechParameters((void *)&sCustParam);
    }

    if (m_bPlaying&&mode==mMode){
         //mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
		 //mAudioResourceManager->StartInputDevice();
		
    //    switch(mMode) {
    //      case SPH_MODE_NORMAL:
    //      {
    //          mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
    //          mAudioResourceManager->TurnonAudioDevice(AUDIO_DEVICE_OUT_EARPIECE);
              
    //          mAudioVolumeInstance->ApplySideTone(EarPiece_SideTone_Gain);                                                                                                         // in 0.5dB
    //          setSphVolume(mMode, mOutputVolume[mMode]);
    //          SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

    //          m_pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
    //          break;
    //      }
    //      case SPH_MODE_EARPHONE:
    //      {
    //          mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
    //            mAudioResourceManager->TurnonAudioDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);

    //          mAudioVolumeInstance->ApplySideTone(Headset_SideTone_Gain); 
    //          setSphVolume(mMode, mOutputVolume[mMode]);
    //          SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

    //          m_pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
    //          break;
    //      }
    //      case SPH_MODE_LOUDSPK:
    //          mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_SPEAKER);
		//    mAudioResourceManager->TurnonAudioDevice(AUDIO_DEVICE_OUT_SPEAKER);

    //          mAudioVolumeInstance->ApplySideTone(LoudSpk_SideTone_Gain); 
    //          setSphVolume(mMode, mOutputVolume[mMode]);
    //          SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

    //          m_pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
    //          break;
    //      default:
    //          break;
    //    }
        

        pSpeechDriver->SetSpeechMode(mode);
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
     //   SPH_ENH_INFO_T eSphEnhInfo;
     //   eSphEnhInfo.spe_usr_mask = 0xFFFF;
     //   eSphEnhInfo.spe_usr_subFunc_mask = 0xFFFFFFFF;
     //   pSpeechDriver->LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);
#endif		
        //m_pLad->LAD_PCM2WayOn(m_bWBMode); // start PCM 2 way
        pSpeechDriver->PCM2WayOn();
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
     //   pSpeechDriver->LAD_Set_Speech_Enhancement_Enable(true);
#endif
    }
    pthread_mutex_unlock(&mP2WMutex);
*/
}

status_t AudioParamTuning::enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB)
{
/*
    SXLOGD("enableModemPlaybackVIASPHPROC bEnable:%d, bWBMode:%d", bEnable, bWB);
    uint32 lad_Volume = 0;
    int degradegain = SIDETONE_MAX_GAIN_DEGRADE;
    int ret = 0;

    // 3 sec for creat thread timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;

    if (bEnable&&(isPlaying()==false)) {
        pthread_mutex_lock(&mP2WMutex);
        m_pInputFile = fopen(m_strInputFileName,"rb");
        if(m_pInputFile == NULL) {
            m_pInputFile = fopen("/mnt/sdcard2/test.wav","rb");
            if (m_pInputFile==NULL) {
            SXLOGD("open input file fail!!");
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
          }
        }
        m_bWBMode = bWB;
        // do basic setting to modem side
        mSideTone = -1 * degradegain<<1;
		
        m_pHW->SwitchAudioClock(true);  // Enable the audio power
        m_pLad->LAD_SetInputSource(LADIN_Microphone1);
        switch(mMode) {
          case SPH_MODE_NORMAL:
          {
              m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
              
              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
              break;
          }
          case SPH_MODE_EARPHONE:
          {
              m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);

              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
              break;
          }
          case SPH_MODE_LOUDSPK:
              m_pHW->ForceEnableSpeaker();
              m_pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);

              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
              break;
          default:
              break;
        }

        SXLOGD("open taste_threadloop thread~");
        pthread_mutex_lock(&mPPSMutex);
        ret = pthread_create(&mTasteThreadID, NULL,Play_PCM_With_SpeechEnhance_Routine,(void*)this);
        if(ret !=0)
        {
            SXLOGE("Play_PCM_With_SpeechEnhance_Routine thread pthread_create error!!");
            pthread_mutex_unlock(&mP2WMutex);
            return UNKNOWN_ERROR;
        }

        SXLOGD("+mPPSExit_Cond wait");
        ret = pthread_cond_timedwait(&mPPSExit_Cond, &mPPSMutex, &timeout);
        SXLOGD("-mPPSExit_Cond receive ret=%d",ret);
        pthread_mutex_unlock(&mPPSMutex);
        usleep(100*1000);

        // really enable the process
        if (mMode==SPH_MODE_LOUDSPK)
            m_pHW->ForceEnableSpeaker();
        m_bPlaying = true;
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
        SPH_ENH_INFO_T eSphEnhInfo;
        eSphEnhInfo.spe_usr_mask = 0xFFFF;
        eSphEnhInfo.spe_usr_subFunc_mask = 0xFFFFFFFF;
        m_pLad->LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);
#endif		
        m_pLad->LAD_PCM2WayOn(bWB); // start PCM 2 way
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
        m_pLad->LAD_Set_Speech_Enhancement_Enable(true);
#endif
        pthread_mutex_unlock(&mP2WMutex);
    }else if((!bEnable)&&m_bPlaying){
        pthread_mutex_lock(&mP2WMutex);
        pthread_mutex_lock(&mPPSMutex);
        if(!m_bPPSThreadExit) {
            m_bPPSThreadExit = true;
            SXLOGD("+mPPSExit_Cond wait");
            ret = pthread_cond_timedwait(&mPPSExit_Cond,&mPPSMutex, &timeout);
            SXLOGD("-mPPSExit_Cond receive ret=%d",ret);
        }
        pthread_mutex_unlock(&mPPSMutex);

        m_pLad->LAD_PCM2WayOff();
        usleep(200*1000);   //wait to make sure all message is processed
        if (mMode==SPH_MODE_LOUDSPK)
            m_pHW->ForceDisableSpeaker();

        m_bPlaying = false;
        m_pHW->SwitchAudioClock(false);  // Disable the audio power

        m_pHW->doOutputDeviceRouting();

        if (m_pInputFile) fclose(m_pInputFile); 
        m_pInputFile = NULL;
        
        pthread_mutex_unlock(&mP2WMutex);
    }else {
        SXLOGD("The Audio Taste Tool State is error, bEnable=%d, playing=%d", bEnable,m_bPlaying);
        return BAD_VALUE;
    }
*/
    return NO_ERROR;
}

status_t AudioParamTuning::putDataToModem()
{
    SXLOGV("putDataToModem in +" );
/*
    int OutputBufDataCount = 0;
    uint32 PCMdataToModemSize = (m_bWBMode==true)? PCM2WAY_BUF_SIZE*2 : PCM2WAY_BUF_SIZE;
    uint32 offset=0, len=0;
    int header_size = 0;
    int8* pA2M_BufPtr;
    int32 maxBufLen;

    pthread_mutex_lock(&mPlayBufMutex);

    // BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: 320+320 bytes)
    pA2M_BufPtr = (int8 *)m_pLad->pCCCI->GetA2MShareBufAddress() + A2M_SHARED_BUFFER_OFFSET;
    maxBufLen = m_pLad->pCCCI->GetA2MShareBufLen();

    // check if the dataReq size is larger than modem/AP share buffer size
    if ( PCMdataToModemSize > maxBufLen ){
       SXLOGD("AudTaste_PutDataToModem PCMdataToModemSize=%d",PCMdataToModemSize );
       PCMdataToModemSize = maxBufLen;
    }

    // check the output buffer data count
    OutputBufDataCount = rb_getDataCount(&m_sPlayBuf);
    SXLOGV("AudTaste_PutDataToModem OutputBufDataCount=%d",OutputBufDataCount );

    // if output buffer's data is not enough, fill it with zero to PCMdataToModemSize (ex: 320 bytes)
    if ((OutputBufDataCount<(int)PCMdataToModemSize)&&(m_sPlayBuf.pBufBase!=NULL)) {
       rb_writeDataValue(&m_sPlayBuf, 0, PCMdataToModemSize - OutputBufDataCount);
       SXLOGD("AudTaste_PutDataToModem underflow OutBufSize:%d", OutputBufDataCount);
    }

    header_size = WriteShareBufHeader(pA2M_BufPtr, (int16)LADBUFID_PCM_FillSpk, (int16)PCMdataToModemSize, (int16)A2M_BUF_HEADER);

    // header size (6 bytes)
    pA2M_BufPtr += header_size;
    if (m_sPlayBuf.pBufBase==NULL) {
        memset(pA2M_BufPtr, 0, PCMdataToModemSize);
    }else {
        rb_copyToLinear(pA2M_BufPtr, &m_sPlayBuf, PCMdataToModemSize);
    }
    offset = A2M_SHARED_BUFFER_OFFSET;
    len = PCMdataToModemSize + header_size;  // PCMdataToModemSize=320. header_size=LAD_SHARE_HEADER_LEN(6)
    m_pLad->LAD_PCM2WayDataNotify(offset, len);

    SXLOGV("OutputBuf B:0x%x, R:%d, W:%d, L:%d\n",m_sPlayBuf.pBufBase, m_sPlayBuf.pRead-m_sPlayBuf.pBufBase, m_sPlayBuf.pWrite-m_sPlayBuf.pBufBase, m_sPlayBuf.bufLen);
    SXLOGV("pA2M_BufPtr B:0x%x, Offset:%d, L:%d\n",pA2M_BufPtr, offset, len);

    pthread_mutex_unlock(&mPlayBufMutex);
*/
    return NO_ERROR;
}

FILE_FORMAT AudioParamTuning::playbackFileFormat()
{
    SXLOGD("playbackFileFormat - playback file name:%s", m_strInputFileName);
    FILE_FORMAT ret = UNSUPPORT_FORMAT;
    char *pFileSuffix = m_strInputFileName;

    strsep(&pFileSuffix, ".");
    if (pFileSuffix!=NULL) {
        if (strcmp(pFileSuffix,"pcm")==0 || strcmp(pFileSuffix,"PCM")==0) {
            SXLOGD("playbackFileFormat - playback file format is pcm");
            ret = PCM_FORMAT;
        }else if (strcmp(pFileSuffix,"wav")==0 || strcmp(pFileSuffix,"WAV")==0) {
            SXLOGD("playbackFileFormat - playback file format is wav");
            ret = WAVE_FORMAT;
        }else {
            SXLOGD("playbackFileFormat - playback file format is unsupport");
            ret = UNSUPPORT_FORMAT;
        }
    }
    
    return ret;
}

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
// For DMNR Tuning	
status_t AudioParamTuning::setRecordFileName(const char *fileName)
{
    SXLOGD("setRecordFileName in+");
    pthread_mutex_lock(&mP2WMutex);
    if (fileName!=NULL && strlen(fileName)<FILE_NAME_LEN_MAX-1) {
        SXLOGD("input file name:%s", fileName);
        memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX);
        strcpy(m_strOutFileName,fileName);
    }else {
        SXLOGE("input file name NULL or too long!");
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }

    pthread_mutex_unlock(&mP2WMutex);	
    return NO_ERROR;
}

status_t AudioParamTuning::setDMNRGain(unsigned short type, unsigned short value)
{
    SXLOGD("setDMNRGain: type=%d, gain=%d", type, value);
    status_t ret = NO_ERROR;

    if (value<0)
        return BAD_VALUE;

    pthread_mutex_lock(&mP2WMutex);
    switch(type) {
      case AUD_MIC_GAIN:
          mDualMicTool_micGain = (value > UPLINK_GAIN_MAX) ? UPLINK_GAIN_MAX : value;
          break;
      case AUD_RECEIVER_GAIN:
          mDualMicTool_receiverGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
          break;
      case AUD_HS_GAIN:
          mDualMicTool_headsetGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
          break;
      default:
          SXLOGW("setDMNRGain unknown type");
          ret = BAD_VALUE;
          break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::getDMNRGain(unsigned short type, unsigned short *value)
{
    SXLOGD("getDMNRGain: type=%d", type);
    status_t ret = NO_ERROR;

    pthread_mutex_lock(&mP2WMutex);
    switch(type) {
      case AUD_MIC_GAIN:
          *value = mDualMicTool_micGain;  
          break;
      case AUD_RECEIVER_GAIN:
          *value = mDualMicTool_receiverGain;
          break;
      case AUD_HS_GAIN:
          *value = mDualMicTool_headsetGain;
          break;
      default:
          SXLOGW("getDMNRGain unknown type");
          ret = BAD_VALUE;
          break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::enableDMNRModem2Way(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode)
{
    SXLOGD("enableDMNRModem2Way bEnable:%d, wb mode:%d, work mode:%d", bEnable, bWBMode, workMode);

    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;

    // get speech driver interface
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
    
    if (mRec2WayInstance==0)
        mRec2WayInstance = Record2Way::GetInstance();
        	
    if ((!workMode)&&(mPlay2WayInstance==0))
        mPlay2WayInstance = Play2Way::GetInstance();

    if(bEnable&&(isPlaying()==false)) {
        pthread_mutex_lock(&mP2WMutex);
        // open output file
        if (!workMode) {
            m_pInputFile = fopen(m_strInputFileName,"rb");
            SXLOGD("[Dual-Mic] open input file filename:%s", m_strInputFileName);
            if(m_pInputFile == NULL) {
                SXLOGW("[Dual-Mic] open input file fail!!");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
            FILE_FORMAT fileType = playbackFileFormat();
            char waveHeader[WAV_HEADER_SIZE];
            if (fileType==WAVE_FORMAT) {
                fread(waveHeader, sizeof(char), WAV_HEADER_SIZE, m_pInputFile);
            }else if(fileType==UNSUPPORT_FORMAT) {
                SXLOGW("[Dual-Mic] playback file format is not support");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }

        m_pOutputFile = fopen(m_strOutFileName,"wb");
        SXLOGD("[Dual-Mic] open output file filename:%s", m_strOutFileName);
        if(m_pOutputFile == NULL) {
            SXLOGW("[Dual-Mic] open output file fail!!");
            fclose(m_pInputFile);
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
        }

        // do basic setting to modem side
        m_bWBMode = bWBMode;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

        //set input devices
        mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);

        //set output device
        if ((!workMode)&&(outputDevice==P2W_RECEIVER_OUT)) {
            mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
        }else if(!workMode){
            mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
        }

        // connect modem and PCM interface
        openModemDualMicCtlFlow(bWBMode, (const bool)workMode);

        // start pcm2way
        mRec2WayInstance->Start();
        if (!workMode)
            mPlay2WayInstance->Start();
            
        // open buffer thread
        SXLOGD("open DMNR_Tuning_threadloop thread~");
        pthread_mutex_lock(&mDMNRMutex);
        ret = pthread_create(&mDMNRThreadID, NULL,DMNR_Play_Rec_Routine,(void*)this);
        if(ret !=0)
            SXLOGE("DMNR_threadloop pthread_create error!!");

        SXLOGD("+mDMNRExit_Cond wait");
        ret = pthread_cond_timedwait(&mDMNRExit_Cond, &mDMNRMutex, &timeout);
        SXLOGD("-mDMNRExit_Cond receive ret=%d",ret);
        pthread_mutex_unlock(&mDMNRMutex);

        m_bDMNRPlaying = true;
        usleep(10*1000);

        // really enable the process
        pSpeechDriver->DualMicPCM2WayOn(bWBMode, (const bool)workMode);
        
        // start input device
#if defined(MTK_DIGITAL_MIC_SUPPORT)
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#else
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_PREAMP_L);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_PREAMP_R);

#if defined(MTK_DUAL_MIC_SUPPORT)  // base on dual or single mic select mic input source.
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC1);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC3);
#else
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC1);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC1);
#endif
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#endif
        // set input device gain
        uint16_t swAGCGain = mAudioVolumeInstance->MappingToDigitalGain(mDualMicTool_micGain);
        uint16_t degradedBGain = mAudioVolumeInstance->MappingToPGAGain(mDualMicTool_micGain);
        SXLOGD("ApplyMicGain DegradedBGain:%d, swAGCGain:%d", degradedBGain, swAGCGain);

        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, degradedBGain);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, degradedBGain);
        mAudioVolumeInstance->ApplyMdUlGain(swAGCGain);
        
        //start output device and set output gain
        if ((!workMode)&&(outputDevice==P2W_RECEIVER_OUT)) {
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            setSphVolume(VOLUME_NORMAL_MODE, mDualMicTool_receiverGain);
            SXLOGD("Play+Rec set dual mic, receiver gain: %d",mDualMicTool_receiverGain);
        }else if(!workMode){
            if (mAudioAnalogInstance->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINR) ||
               mAudioAnalogInstance->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINL)) {
                SXLOGD("SetHeadsetPinmux as MUX_LINEIN_AUDIO_MONO ");
                mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::MUX_LINEIN_AUDIO_MONO);
                mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, AudioAnalogType::MUX_LINEIN_AUDIO_MONO);
            }
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            setSphVolume(VOLUME_HEADSET_MODE, mDualMicTool_headsetGain);
            SXLOGD("Play+Rec set dual mic, headset gain: %d",mDualMicTool_headsetGain);
        }
        
        pthread_mutex_unlock(&mP2WMutex);
    } else if (!bEnable&&m_bDMNRPlaying){ 
        pthread_mutex_lock(&mP2WMutex);
        //stop buffer thread
        SXLOGD("close DMNR_tuning_threadloop");
        pthread_mutex_lock(&mDMNRMutex);
        if(!m_bDMNRThreadExit) {
            m_bDMNRThreadExit = true;
            SXLOGD("+mDMNRExit_Cond wait");
            ret = pthread_cond_timedwait(&mDMNRExit_Cond,&mDMNRMutex, &timeout);
            SXLOGD("-mDMNRExit_Cond receive ret=%d",ret);
        }
        pthread_mutex_unlock(&mDMNRMutex);

        if (!workMode) {
            // Stop output device
            uint32_t output_device = mAudioResourceManager->getDlOutputDevice();
            if (output_device==AUDIO_DEVICE_OUT_EARPIECE) {
        	      mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            } else if(output_device==AUDIO_DEVICE_OUT_WIRED_HEADSET){
        	      mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        	  }
        }
        // stop input device
#if defined(MTK_DIGITAL_MIC_SUPPORT)
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#else
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#endif

        pSpeechDriver->DualMicPCM2WayOff();
        
        mRec2WayInstance->Stop();
        if (!workMode)
            mPlay2WayInstance->Stop();
        //wait to make sure all message is processed
        usleep(200*1000);

        m_bDMNRPlaying = false;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);

        closeModemDualMicCtlFlow((const bool)workMode);

        if (m_pInputFile) fclose(m_pInputFile);
        if (m_pOutputFile) fclose(m_pOutputFile);
        m_pInputFile = NULL;
        m_pOutputFile = NULL;
        pthread_mutex_unlock(&mP2WMutex);
    }else {
        SXLOGD("The DMNR Tuning State is error, bEnable=%d, playing=%d", bEnable,m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

status_t AudioParamTuning::readDataFromModem(uint32 offset, uint32 len)
{
    SXLOGV("readDataFromModem in +" );
    int recBufFreeCnt = 0;
/*
    int8* pM2A_BufPtr = (int8*)m_pLad->pCCCI->GetM2AShareBufAddress();
    uint32 m2aBufLen  = m_pLad->pCCCI->GetM2AShareBufLen();
    int8 *pBufRead = pM2A_BufPtr + offset + LAD_SHARE_HEADER_LEN;
    int pcmDataCnt = len - LAD_SHARE_HEADER_LEN;
    SXLOGD("readDataFromModem sharebuffer ptr=0x%x, bufferLen=%d, offset=%d, datacnt=%d", pM2A_BufPtr, m2aBufLen, offset, len);

	if ((offset+len)>m2aBufLen) {
        SXLOGW("readDataFromModem, no data... offset=%d, M2ABufLen=%d, data len=%d", offset, m2aBufLen, len);
        return BAD_VALUE;
    }

    pthread_mutex_lock(&mRecBufMutex);
    recBufFreeCnt = m_sRecBuf.bufLen - rb_getDataCount(&m_sRecBuf) - 1;
    SXLOGD("readDataFromModem recBufFreeCount=%d",recBufFreeCnt );

    // if rec buffer's free space is not enough
    if (recBufFreeCnt>=pcmDataCnt) {
        SXLOGD("read pcm from share buffer, free count=%d, data count=%d", recBufFreeCnt, pcmDataCnt);
        if (m_sRecBuf.pBufBase)
            rb_copyFromLinear(&m_sRecBuf, pBufRead, pcmDataCnt);
    }else {
        SXLOGW("record buffer overflow, do not copy, free count=%d, data count=%d", recBufFreeCnt, pcmDataCnt);
    }
    pthread_mutex_unlock(&mRecBufMutex);

    //send back
    m_pLad->LAD_PCM2WayDataSendBack();
*/
    return NO_ERROR;
}
#endif

status_t AudioParamTuning::setSphVolume(uint32 mode, uint32 gain)
{
    SXLOGV("setSphVolume in +" );
    int32 degradeDb = (DEVICE_VOLUME_STEP - gain) / VOICE_ONEDB_STEP;
    int voiceAnalogRange = DEVICE_MAX_VOLUME - DEVICE_MIN_VOLUME;
		
    switch(mode)
    {
      case VOLUME_NORMAL_MODE:
	      if (gain<=AUDIO_BUFFER_HW_GAIN_STEP) {
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, gain);
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, gain);
          }else {
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, voiceAnalogRange);
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, voiceAnalogRange);
              degradeDb -= voiceAnalogRange;
              mAudioVolumeInstance->ApplyMdDlGain(degradeDb);
          }
		  break;
      case VOLUME_HEADSET_MODE:
          if (gain<=AUDIO_BUFFER_HW_GAIN_STEP) {
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, gain);
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, gain);
          }else {
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, voiceAnalogRange);
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, voiceAnalogRange);
              degradeDb -= voiceAnalogRange;
              mAudioVolumeInstance->ApplyMdDlGain(degradeDb);
          }
          break;
      case VOLUME_SPEAKER_MODE:
          if (gain<=AUDIO_BUFFER_HW_GAIN_STEP) {
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, gain);
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, gain);
          }else {
              voiceAnalogRange = DEVICE_AMP_MAX_VOLUME - DEVICE_AMP_MIN_VOLUME;
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, voiceAnalogRange);
              mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, voiceAnalogRange);
              degradeDb -= voiceAnalogRange;
              mAudioVolumeInstance->ApplyMdDlGain(degradeDb);
          }
		  break;
      case VOLUME_HEADSET_SPEAKER_MODE:
          // nothing to do
          break;
      default:
          break;
    }

    return NO_ERROR;
}

status_t AudioParamTuning::OpenModemSpeechControlFlow(int mode)
{
/*
    if (mode!=AUDIO_MODE_IN_CALL &&
            mode!=AUDIO_MODE_IN_CALL_2) {
        SXLOGE("mode is not MODE_IN_CALL or MODE_IN_CALL_2 mode:%d", mode);
        return INVALID_OPERATION;
    }
	
    const int32_t output_device = mAudioResourceManager->getDlOutputDevice();
    const int32_t input_device = mAudioResourceManager->getUlInputDevice();

    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 16000; // TODO: MT6628 BT only use NB

    AudioDigitalType::InterConnectionOutput modem_pcm_tx_lch = (mode == AUDIO_MODE_IN_CALL) ? AudioDigitalType::O17 : AudioDigitalType::O07;
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_rch = (mode == AUDIO_MODE_IN_CALL) ? AudioDigitalType::O18 : AudioDigitalType::O08;
    AudioDigitalType::InterConnectionInput  modem_pcm_rx     = (mode == AUDIO_MODE_IN_CALL) ? AudioDigitalType::I14 : AudioDigitalType::I09;

    if (bt_device_on) { // DAIBT
#ifndef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I02, modem_pcm_tx_lch);      // DAIBT_IN       -> MODEM_PCM_TX_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx         , AudioDigitalType::O02); // MODEM_PCM_RX   -> DAIBT_OUT
#else
        AudioDigitalType::InterConnectionOutput hw_gain_2_in_lch  = AudioDigitalType::O15;
        AudioDigitalType::InterConnectionOutput hw_gain_2_in_rch  = AudioDigitalType::O16;
        AudioDigitalType::InterConnectionInput  hw_gain_2_out_lch = AudioDigitalType::I12;
        AudioDigitalType::InterConnectionInput  hw_gain_2_out_rch = AudioDigitalType::I13;
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I02, hw_gain_2_in_lch);      // DAIBT_IN       -> HW_GAIN2_IN_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, hw_gain_2_out_lch    , modem_pcm_tx_lch);      // HW_GAIN2_OUT_L -> MODEM_PCM_TX_L

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx         , hw_gain_2_in_rch);      // MODEM_PCM_RX   -> HW_GAIN2_IN_R
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, hw_gain_2_out_rch    , AudioDigitalType::O02); // HW_GAIN2_OUT_R -> DAIBT_OUT
#endif        

        AudioDigitalDAIBT daibt_attribute;

#ifdef I2S_DAIBT_MERGE_INTERFACE_SUPPORT
        daibt_attribute.mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_MGRIF;
#else
        daibt_attribute.mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_BT;
#endif
        daibt_attribute.mDAI_BT_MODE = (sample_rate == 8000) ? AudioDigitalDAIBT::Mode8K : AudioDigitalDAIBT::Mode16K;
        daibt_attribute.mDAI_DEL = AudioDigitalDAIBT::HighWord; // suggest always HighWord
        daibt_attribute.mBT_LEN  = 5;
        daibt_attribute.mDATA_RDY = true;
        daibt_attribute.mBT_SYNC = AudioDigitalDAIBT::Long_Sync;
        daibt_attribute.mBT_ON = true;
        daibt_attribute.mDAIBT_ON = false;
        mAudioDigitalInstance->SetDAIBBT(&daibt_attribute);
        mAudioDigitalInstance->SetDAIBTEnable(true);

#ifdef DAIBT_NO_INTERCONNECTION_TO_MODEM_PCM // DAIBT <-> HW_GAIN2 <-> MODEM_PCM
        mAudioDigitalInstance->SetHwDigitalGainMode(AudioDigitalType::HW_DIGITAL_GAIN2, (sample_rate == 8000) ? AudioMEMIFAttribute::AFE_8000HZ : AudioMEMIFAttribute::AFE_16000HZ, 0xC8);
        mAudioDigitalInstance->SetHwDigitalGain(0x80000, AudioDigitalType::HW_DIGITAL_GAIN2);
        mAudioDigitalInstance->SetHwDigitalGainEnable(AudioDigitalType::HW_DIGITAL_GAIN2, true);
#endif
    }
    else { // ADC/DAC I2S
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, modem_pcm_tx_lch);      // ADC_I2S_IN_L -> MODEM_PCM_TX_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I04, modem_pcm_tx_rch);      // ADC_I2S_IN_R -> MODEM_PCM_TX_R

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx         , AudioDigitalType::O03); // MODEM_PCM_RX -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx         , AudioDigitalType::O04); // MODEM_PCM_RX -> DAC_I2S_OUT_R

        AudioDigtalI2S adc_i2s_in_attribute;

        adc_i2s_in_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
        adc_i2s_in_attribute.mBuffer_Update_word = 8;
        adc_i2s_in_attribute.mFpga_bit_test = 0;
        adc_i2s_in_attribute.mFpga_bit = 0;
        adc_i2s_in_attribute.mloopback = 0;
        adc_i2s_in_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
        adc_i2s_in_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
        adc_i2s_in_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
        adc_i2s_in_attribute.mI2S_SAMPLERATE = sample_rate;
        adc_i2s_in_attribute.mI2S_EN = false;
        mAudioDigitalInstance->SetI2SAdcIn(&adc_i2s_in_attribute);

        AudioDigtalI2S dac_i2s_out_attribute;

        dac_i2s_out_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
        dac_i2s_out_attribute.mI2S_SAMPLERATE = sample_rate;
        dac_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
        dac_i2s_out_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
        dac_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
        mAudioDigitalInstance->SetI2SDacOut(&dac_i2s_out_attribute);

        mAudioDigitalInstance->SetI2SAdcEnable(true);
        mAudioDigitalInstance->SetI2SDacEnable(true);
    }

    // set MODEM_PCM
    AudioDigitalPCM modem_pcm_attribute;

    // modem 2 only
    if (mode == AUDIO_MODE_IN_CALL_2) {
        // TODO: only config internal modem here.. Add external modem setting by project config!!
        modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_INTERNAL_MODEM;
        modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::SALVE_MODE;
        modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASYNC_FIFO;

        modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
        modem_pcm_attribute.mExtendBckSyncLength  = 0;
    }

    // here modem_1 & modem_2 use the same config, but register field offset are not the same
    modem_pcm_attribute.mVbt16kModeSel      = AudioDigitalPCM::VBT_16K_MODE_ENABLE;

    modem_pcm_attribute.mSingelMicSel       = AudioDigitalPCM::DUAL_MIC_ON_TX;
    modem_pcm_attribute.mTxLchRepeatSel     = AudioDigitalPCM::TX_LCH_NO_REPEAT;

    modem_pcm_attribute.mPcmWordLength      = AudioDigitalPCM::PCM_16BIT;
    modem_pcm_attribute.mPcmModeWidebandSel = (sample_rate == 8000) ? AudioDigitalPCM::PCM_MODE_8K : AudioDigitalPCM::PCM_MODE_16K;
    modem_pcm_attribute.mPcmFormat          = AudioDigitalPCM::PCM_MODE_B;
    modem_pcm_attribute.mModemPcmOn         = false;

    mAudioDigitalInstance->SetModemPcmConfig(mode, &modem_pcm_attribute);
    mAudioDigitalInstance->SetModemPcmEnable(mode, true);

    // AFE_ON
    mAudioDigitalInstance->SetAfeEnable(true);


    // get speech driver instance
    const modem_index_t    modem_index   = mSpeechDriverFactory->GetActiveModemIndex();
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

#if 0 // TODO: Emulate it after
    // set device info
#if defined(MTK_DUAL_MIC_SUPPORT)
    pSpeechDriver->SetDualMicCapacityOn((bt_device_on == true) ? false : true);
#else
    pSpeechDriver->SetDualMicCapacityOn(false);
#endif
    pSpeechDriver->SetModemSideSamplingRate(sample_rate); // TODO: MT6628 BT only use NB
#endif

    // set speech mode
    speech_mode_t speech_mode = SPEECH_MODE_NORMAL;
    if (output_device& AUDIO_DEVICE_OUT_EARPIECE) {
        speech_mode = SPEECH_MODE_NORMAL;
    }else if (output_device&AUDIO_DEVICE_OUT_SPEAKER) {
        speech_mode = SPEECH_MODE_LOUD_SPEAKER;
    }else if (output_device&AUDIO_DEVICE_OUT_WIRED_HEADSET ||
             output_device&AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        speech_mode = SPEECH_MODE_EARPHONE;
    }
    
    pSpeechDriver->SetSpeechMode(speech_mode);
    pSpeechDriver->PCM2WayOn(); // trun on P2W for Video Telephony
    
    // start analog part at the end
    mAudioResourceManager->StartOutputDevice(); // TODO(Harvey): Wait speech on ack??
*/
    return NO_ERROR;
}

status_t AudioParamTuning::openModemDualMicCtlFlow(bool bWB, bool bRecOnly)
{
    // get speech driver instance
    int sampleRate = 16000;
    const modem_index_t    modem_index   = mSpeechDriverFactory->GetActiveModemIndex();
    
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_lch = (modem_index==MODEM_1) ? AudioDigitalType::O17 : AudioDigitalType::O07;
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_rch = (modem_index==MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08;
    
    // connect UL path
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, modem_pcm_tx_lch); // ADC_I2S_IN_L   -> MODEM_PCM_TX_L
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I04, modem_pcm_tx_rch); // ADC_I2S_IN_R   -> MODEM_PCM_TX_R
    
    if (!bRecOnly) {
        // connect DL path
        AudioDigitalType::InterConnectionInput  modem_pcm_rx = (modem_index==MODEM_1) ? AudioDigitalType::I14 : AudioDigitalType::I09;
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx, AudioDigitalType::O03); // MODEM_PCM_RX   -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx, AudioDigitalType::O04); // MODEM_PCM_RX   -> DAC_I2S_OUT_R
    }
  
    // start ADC setting
    AudioDigtalI2S AdcI2SIn;
    AdcI2SIn.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    AdcI2SIn.mBuffer_Update_word = 8;
    AdcI2SIn.mFpga_bit_test = 0;
    AdcI2SIn.mFpga_bit = 0;
    AdcI2SIn.mloopback = 0;
    AdcI2SIn.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    AdcI2SIn.mI2S_FMT = AudioDigtalI2S::I2S;
    AdcI2SIn.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    AdcI2SIn.mI2S_SAMPLERATE = sampleRate;
    AdcI2SIn.mI2S_EN = false;
    mAudioDigitalInstance->SetI2SAdcIn(&AdcI2SIn);
    
    if (!bRecOnly) {
        AudioDigtalI2S dac_i2s_out_attribute;
        memset((void *)&dac_i2s_out_attribute, 0, sizeof(dac_i2s_out_attribute));

        dac_i2s_out_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
        dac_i2s_out_attribute.mI2S_SAMPLERATE = sampleRate;
        dac_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
        dac_i2s_out_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
        dac_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
        mAudioDigitalInstance->SetI2SDacOut(&dac_i2s_out_attribute);
    }
    
    mAudioDigitalInstance->SetI2SAdcEnable(true);
    mAudioAnalogInstance->SetFrequency(AudioAnalogType::DEVICE_IN_ADC, sampleRate);
    mAudioAnalogInstance->SetFrequency(AudioAnalogType::DEVICE_OUT_DAC, sampleRate);
    if (!bRecOnly) {
        mAudioDigitalInstance->SetI2SDacEnable(true);
    }
    
    // set MODEM_PCM
    AudioDigitalPCM modem_pcm_attribute;
    memset((void *)&modem_pcm_attribute, 0, sizeof(modem_pcm_attribute));

    // modem 2 only
    if (modem_index==MODEM_2) {
        // TODO: only config internal modem here.. Add external modem setting by project config!!
        modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_INTERNAL_MODEM;
        modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::SALVE_MODE;
        modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASRC;

        modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
        modem_pcm_attribute.mExtendBckSyncLength  = 0;
    }

    // here modem_1 & modem_2 use the same config, but register field offset are not the same
    modem_pcm_attribute.mVbt16kModeSel      = AudioDigitalPCM::VBT_16K_MODE_DISABLE;

    modem_pcm_attribute.mSingelMicSel       = AudioDigitalPCM::DUAL_MIC_ON_TX;
    modem_pcm_attribute.mTxLchRepeatSel     = AudioDigitalPCM::TX_LCH_NO_REPEAT;

    modem_pcm_attribute.mPcmWordLength      = AudioDigitalPCM::PCM_16BIT;
    modem_pcm_attribute.mPcmModeWidebandSel = (sampleRate == 8000) ? AudioDigitalPCM::PCM_MODE_8K : AudioDigitalPCM::PCM_MODE_16K;
    modem_pcm_attribute.mPcmFormat          = AudioDigitalPCM::PCM_MODE_B;
    modem_pcm_attribute.mModemPcmOn         = false;

    mAudioDigitalInstance->SetModemPcmConfig(modem_index, &modem_pcm_attribute);
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, true);
    
    mAudioDigitalInstance->SetAfeEnable(true);
    
    return NO_ERROR;
}

status_t AudioParamTuning::closeModemDualMicCtlFlow(bool bRecOnly)
{
    // get speech driver instance
    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();
    
    // set MODEM_PCM
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);
    
    if (!bRecOnly) {
        mAudioDigitalInstance->SetI2SDacEnable(false);
    }
    
    // disable ADC
    mAudioDigitalInstance->SetI2SAdcEnable(false);
    
    if (!bRecOnly) {
        // disconnect DL path
        AudioDigitalType::InterConnectionInput  modem_pcm_rx = (modem_index==MODEM_1) ? AudioDigitalType::I14 : AudioDigitalType::I09;
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, modem_pcm_rx, AudioDigitalType::O03); // MODEM_PCM_RX -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, modem_pcm_rx, AudioDigitalType::O04); // MODEM_PCM_RX -> DAC_I2S_OUT_R
    }
    
    // disconnect UL path
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_lch = (modem_index==MODEM_1) ? AudioDigitalType::O17 : AudioDigitalType::O07;
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_rch = (modem_index==MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08;
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I03, modem_pcm_tx_lch); // ADC_I2S_IN_L -> MODEM_PCM_TX_L
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I04, modem_pcm_tx_rch); // ADC_I2S_IN_R -> MODEM_PCM_TX_R
    
    // AFE_ON = false
    mAudioDigitalInstance->SetAfeEnable(false);
    return NO_ERROR;
}
};
