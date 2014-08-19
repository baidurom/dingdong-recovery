#include "AudioSpeechEnhanceInfo.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include "AudioUtility.h"


#define LOG_TAG "AudioSpeechEnhanceInfo"

namespace android
{

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::UniqueAudioSpeechEnhanceInfoInstance = NULL;

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::getInstance()
{
    if (UniqueAudioSpeechEnhanceInfoInstance == NULL) {
        ALOGD("+AudioSpeechEnhanceInfo");
        UniqueAudioSpeechEnhanceInfoInstance = new AudioSpeechEnhanceInfo();
        ALOGD("-AudioSpeechEnhanceInfo");
    }
    ALOGD("getInstance()");
    return UniqueAudioSpeechEnhanceInfoInstance;
}

void AudioSpeechEnhanceInfo::freeInstance()
{
    return;
}

AudioSpeechEnhanceInfo::AudioSpeechEnhanceInfo()
{
    ALOGD("AudioSpeechEnhanceInfo constructor");    
    mHdRecScene = -1;
    mIsLRSwitch = false;
    mUseSpecificMic = 0;
    mHDRecTunningEnable = false;
}

AudioSpeechEnhanceInfo::~AudioSpeechEnhanceInfo()
{
    ALOGD("AudioSpeechEnhanceInfo destructor");
    mHdRecScene = -1;
}


void AudioSpeechEnhanceInfo::SetRecordLRChannelSwitch(bool bIsLRSwitch)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetRecordLRChannelSwitch=%x",bIsLRSwitch);
    mIsLRSwitch = bIsLRSwitch;
}

bool AudioSpeechEnhanceInfo::GetRecordLRChannelSwitch(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetRecordLRChannelSwitch=%x",mIsLRSwitch);
    return mIsLRSwitch;
}

void AudioSpeechEnhanceInfo::SetUseSpecificMIC(int32 UseSpecificMic)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetUseSpecificMIC=%x",UseSpecificMic);
    mUseSpecificMic = UseSpecificMic;
}

int AudioSpeechEnhanceInfo::GetUseSpecificMIC(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetUseSpecificMIC=%x",mUseSpecificMic);
    return mUseSpecificMic;
}

//----------------for HD Record Preprocess-----------------------------
void AudioSpeechEnhanceInfo::SetHDRecScene(int32 HDRecScene)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo SetHDRecScene=%d",HDRecScene);
    mHdRecScene = HDRecScene;
}

int32 AudioSpeechEnhanceInfo::GetHDRecScene()
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo GetHDRecScene=%d",mHdRecScene);
    return mHdRecScene;
}

void AudioSpeechEnhanceInfo::ResetHDRecScene()
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo ResetHDRecScene");
    mHdRecScene = -1;
}

//----------------for HDRec tunning --------------------------------
void AudioSpeechEnhanceInfo::SetHDRecTunningEnable(bool bEnable)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetHDRecTunningEnable=%d",bEnable);
    mHDRecTunningEnable = bEnable;
}

bool AudioSpeechEnhanceInfo::IsHDRecTunningEnable(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("IsHDRecTunningEnable=%d",mHDRecTunningEnable);
    return mHDRecTunningEnable;
}

status_t AudioSpeechEnhanceInfo::SetHDRecVMFileName(const char *fileName)
{
    Mutex::Autolock lock(mHDRInfoLock);
    if (fileName!=NULL && strlen(fileName)<128-1) {
        ALOGD("SetHDRecVMFileName file name:%s", fileName);
        memset(mVMFileName, 0, 128);
        strcpy(mVMFileName,fileName);
    }else {
        ALOGD("input file name NULL or too long!");
        return BAD_VALUE;
    }
    return NO_ERROR;
}
void AudioSpeechEnhanceInfo::GetHDRecVMFileName(char * VMFileName)
{
    Mutex::Autolock lock(mHDRInfoLock);
    memset(VMFileName, 0, 128);
    strcpy(VMFileName,mVMFileName);
    ALOGD("GetHDRecVMFileName mVMFileName=%s, VMFileName=%s",mVMFileName,VMFileName);
}

//----------------for Android Native Preprocess-----------------------------
void AudioSpeechEnhanceInfo::SetStreamOutPointer(void *pStreamOut)
{
    if (pStreamOut == NULL) {
        ALOGW(" SetStreamOutPointer pStreamOut = NULL");
    }
    else {
        mStreamOut = (AudioMTKStreamOut *)pStreamOut;
        ALOGW("SetStreamOutPointer mStreamOut=%p",mStreamOut);
    }
}

int AudioSpeechEnhanceInfo::GetOutputSampleRateInfo(void)
{
    int samplerate = 16000;
    samplerate = mStreamOut->GetSampleRate();
    ALOGD("AudioSpeechEnhanceInfo GetOutputSampleRateInfo=%d",samplerate);
    return samplerate;
}

int AudioSpeechEnhanceInfo::GetOutputChannelInfo(void)
{
    int chn = 1;
    chn = mStreamOut->GetChannel();
    ALOGD("AudioSpeechEnhanceInfo GetOutputChannelInfo=%d",chn);
    return chn;
}

bool AudioSpeechEnhanceInfo::IsOutputRunning(void)
{
    return mStreamOut->GetStreamRunning();
}

void AudioSpeechEnhanceInfo::add_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("AudioSpeechEnhanceInfo add_echo_reference=%p",reference);
    mStreamOut->add_echo_reference(reference);
}
void AudioSpeechEnhanceInfo::remove_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("AudioSpeechEnhanceInfo remove_echo_reference=%p",reference);
    mStreamOut->remove_echo_reference(reference);
}

}




