#include "AudioMTKStreamOut.h"
#include "AudioResourceFactory.h"
#include "AudioResourceManagerInterface.h"
#include "AudioIoctl.h"
#include "AudioDigitalType.h"
#include "SpeechDriverFactory.h"
#include "SpeechBGSPlayer.h"
#include "SpeechPhoneCallController.h"
#include "LoopbackManager.h"

#include "audio_custom_exp.h"
#include "AudioVUnlockDL.h"
#include "AudioIoctl.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM_FILE (5)
#define MAX_FILE_LENGTH (50000000)

#define LOG_TAG  "AudioMTKStreamOut"
#ifndef ANDROID_DEFAULT_CODE
    #include <cutils/xlog.h>
    #ifdef ALOGE
    #undef ALOGE
    #endif
    #ifdef ALOGW
    #undef ALOGW
    #endif ALOGI
    #undef ALOGI
    #ifdef ALOGD
    #undef ALOGD
    #endif
    #ifdef ALOGV
    #undef ALOGV
    #endif
    #define ALOGE XLOGE
    #define ALOGW XLOGW
    #define ALOGI XLOGI
    #define ALOGD XLOGD
    #define ALOGV XLOGV
#else
    #include <utils/Log.h>
#endif

//Configure ACF Work Mode
#if defined(ENABLE_AUDIO_COMPENSATION_FILTER) && defined(ENABLE_AUDIO_DRC_SPEAKER)
//5
#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC

#elif defined(ENABLE_AUDIO_COMPENSATION_FILTER)
// 4
#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_COMP

#elif defined(ENABLE_AUDIO_DRC_SPEAKER)
// 3
#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_LITE

#endif


namespace android
{

AudioMTKStreamOut::AudioMTKStreamOut()
{
    ALOGD("+AudioMTKStreamOut default constructor");
    mStreamOutCompFltEnable[AUDIO_COMP_FLT_AUDIO] = mStreamOutCompFltEnable[AUDIO_COMP_FLT_HEADPHONE] = mStreamOutCompFltEnable[AUDIO_COMP_FLT_AUDENH] = false;
    mpClsCompFltObj[AUDIO_COMP_FLT_AUDIO] = mpClsCompFltObj[AUDIO_COMP_FLT_HEADPHONE] = mpClsCompFltObj[AUDIO_COMP_FLT_AUDENH] = NULL;
    mdCompFltMode[AUDIO_COMP_FLT_AUDIO] = mdCompFltMode[AUDIO_COMP_FLT_HEADPHONE] = mdCompFltMode[AUDIO_COMP_FLT_AUDENH] = AUDIO_CMP_FLT_LOUDNESS_NONE;
    mStreamOutCompFltApplyStatus[AUDIO_COMP_FLT_AUDIO] = mStreamOutCompFltApplyStatus[AUDIO_COMP_FLT_HEADPHONE] = mStreamOutCompFltApplyStatus[AUDIO_COMP_FLT_AUDENH] = false;
    mPDacPCMDumpFile = NULL;
    mPFinalPCMDumpFile = NULL;
    mSteroToMono = false;
    mForceStandby = false;
    DumpFileNum =0;
    ALOGD("-AudioMTKStreamOut default constructor");
}

AudioMTKStreamOut::AudioMTKStreamOut(uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status)
{
    mStreamOutCompFltEnable[AUDIO_COMP_FLT_AUDIO] = mStreamOutCompFltEnable[AUDIO_COMP_FLT_HEADPHONE] = mStreamOutCompFltEnable[AUDIO_COMP_FLT_AUDENH] = false;
    mpClsCompFltObj[AUDIO_COMP_FLT_AUDIO] = mpClsCompFltObj[AUDIO_COMP_FLT_HEADPHONE] = mpClsCompFltObj[AUDIO_COMP_FLT_AUDENH] = NULL;
    mdCompFltMode[AUDIO_COMP_FLT_AUDIO] = mdCompFltMode[AUDIO_COMP_FLT_HEADPHONE] = mdCompFltMode[AUDIO_COMP_FLT_AUDENH] = AUDIO_CMP_FLT_LOUDNESS_NONE;
    mStreamOutCompFltApplyStatus[AUDIO_COMP_FLT_AUDIO] = mStreamOutCompFltApplyStatus[AUDIO_COMP_FLT_HEADPHONE] = mStreamOutCompFltApplyStatus[AUDIO_COMP_FLT_AUDENH] = false;
    mFd = 0;
    mPDacPCMDumpFile = NULL;
    mPFinalPCMDumpFile = NULL;
    DumpFileNum =0;
    // here open audio hardware for register setting
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd == 0) {
        ALOGE("AudioMTKStreamOut open mFd fail");
    }

    ALOGD("+AudioMTKStreamOut constructor devices = %x format = %x channels = %x sampleRate = %d",
          devices, *format, *channels, *sampleRate);
    mDL1Attribute = new AudioStreamAttribute();
    mDL1Out = new AudioDigtalI2S();
    m2ndI2SOut = new AudioDigtalI2S();
    mDaiBt = new AudioDigitalDAIBT();
    mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioAnalogControl = AudioAnalogControlFactory::CreateAudioAnalogControl();
    mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
    if (!mAudioResourceManager) {
        ALOGE("mAudioResourceManager get fail  = %p", mAudioResourceManager);
    }

    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    if(!mAudioSpeechEnhanceInfoInstance){
        ALOGE("mAudioSpeechEnhanceInfoInstance get fail");
    }

    mAudioSpeechEnhanceInfoInstance->SetStreamOutPointer(this);
    mEcho_reference = NULL;

    ALOGD("format = %d, channels = %d, rate = %d", *format, *channels, *sampleRate);

    if (*format == AUDIO_FORMAT_PCM_16_BIT)
        mDL1Attribute->mFormat = AUDIO_FORMAT_PCM_16_BIT;
    else if (*format == AUDIO_FORMAT_PCM_8_BIT)
        mDL1Attribute->mFormat = AUDIO_FORMAT_PCM_8_BIT;
    else {
        ALOGE("Format is not a valid number");
        mDL1Attribute->mFormat = AUDIO_FORMAT_PCM_16_BIT;
    }

    if (*channels == AUDIO_CHANNEL_OUT_MONO) {
        mDL1Attribute->mChannels = 1;
    }
    else if (*channels == AUDIO_CHANNEL_OUT_STEREO) {
        mDL1Attribute->mChannels = 2;
    }
    else {
        ALOGE("Channelsis not a valid number");
        mDL1Attribute->mChannels = 2;
    }
    mDL1Attribute->mSampleRate = *sampleRate;

    // save the original sample rate & channels before SRC
    mSourceSampleRate  = *sampleRate;
    mSourceChannels    = mDL1Attribute->mChannels;

    mStarting = false;
    mSuspend = 0;
    mSteroToMono = false;
    mDL1Attribute->mBufferSize = 4096;
    mHwBufferSize = mAudioDigitalControl->GetMemBufferSize(AudioDigitalType::MEM_DL1);
    mLatency = (mHwBufferSize * 1000) / (mDL1Attribute->mSampleRate * mDL1Attribute->mChannels *
                                        (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_8_BIT ? 1 : 2));
    AudioVUnlockDL* VUnlockhdl = AudioVUnlockDL::getInstance();
    if(VUnlockhdl !=NULL)
    {
        VUnlockhdl->GetStreamOutLatency(mLatency);
    }
    calInterrupttime();

    Set2ndI2SOutAttribute();
    SetI2SOutDACAttribute();
    SetDAIBTAttribute();

    mBliSrc = NULL;
    mBliSrc = new BliSrc();

    mSwapBufferTwo = NULL;
    mSwapBufferTwo = new uint8_t[bufferSize()];
    if (mSwapBufferTwo == NULL) {
        ALOGE("mSwapBufferTwo for BliSRC allocate fail1!!! \n");
    }

    mSwapBufferThree = NULL;
    mForceStandby = false;

#if defined(ENABLE_AUDIO_COMPENSATION_FILTER)||defined(ENABLE_AUDIO_DRC_SPEAKER)    //For reduce resource
    StreamOutCompFltCreate(AUDIO_COMP_FLT_AUDIO,AUDIO_COMPENSATION_FLT_MODE);
#endif

#if defined(ENABLE_HEADPHONE_COMPENSATION_FILTER)   //For reduce resource
    StreamOutCompFltCreate(AUDIO_COMP_FLT_HEADPHONE,AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE);
#endif

#if defined(MTK_AUDENH_SUPPORT) //For reduce resource
    mSwapBufferThree = new uint8_t[bufferSize()];
    if (mSwapBufferThree == NULL) {
        ALOGE("mSwapBufferThree for AudEnh allocate fail1!!! \n");
    }

    StreamOutCompFltCreate(AUDIO_COMP_FLT_AUDENH,AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH);

    #if 0
    char value[PROPERTY_VALUE_MAX];
    int result = 0 ;
    property_get("persist.af.audenh.ctrl", value, "1");
    result = atoi(value);
    #else

    unsigned int result = 0 ;
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    if(GetAudEnhControlOptionParamFromNV(&audioParam))
        result = audioParam.u32EnableFlg;
    #endif

    mUseAudCompFltHeadphoneWithFixedParameter = (result?true:false);
#else
    mUseAudCompFltHeadphoneWithFixedParameter = false;
#endif
    ALOGD("Init mUseAudCompFltHeadphoneWithFixedParameter [%d]\n",mUseAudCompFltHeadphoneWithFixedParameter);
    *status = NO_ERROR;
    ALOGD("-AudioMTKStreamOut constructor \n");
}

AudioMTKStreamOut::~AudioMTKStreamOut()
{
    ALOGD("AudioMTKStreamOut desstructor \n");

    StreamOutCompFltDestroy(AUDIO_COMP_FLT_AUDIO);
    StreamOutCompFltDestroy(AUDIO_COMP_FLT_HEADPHONE);
    StreamOutCompFltDestroy(AUDIO_COMP_FLT_AUDENH);

    if (mBliSrc) {
        mBliSrc->close();
        delete mBliSrc;
        mBliSrc = NULL;
    }
    if (mSwapBufferTwo) {
        delete []mSwapBufferTwo;
        mSwapBufferTwo = NULL;
    }

    if (mSwapBufferThree) {
        delete []mSwapBufferThree;
        mSwapBufferThree = NULL;
    }

    if(mEcho_reference)
        mEcho_reference = NULL;
}

uint32_t AudioMTKStreamOut::calInterrupttime()
{
    int SampleCount = mHwBufferSize / mDL1Attribute->mChannels;
    ALOGD("calInterrupttime bufferSize = %d mDL1Attribute->mChannels = %d", bufferSize(), mDL1Attribute->mChannels);

    if (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_16_BIT)
        SampleCount = SampleCount >> 1;

    mDL1Attribute->mInterruptSample =  SampleCount >> 1;
    ALOGD("calInterrupttime mInterruptCounter = %d", mDL1Attribute->mInterruptSample);
    return  mDL1Attribute->mInterruptSample ;
}

status_t AudioMTKStreamOut::SetMEMIFAttribute(AudioDigitalType::Digital_Block Mem_IF, AudioStreamAttribute *Attribute)
{
    ALOGD("SetMEMIFAttribute Mem_IF = %d sampleRate = %d mInterruptSample = %d", Mem_IF, Attribute->mSampleRate, Attribute->mInterruptSample);

    mAudioDigitalControl->SetMemIfSampleRate(Mem_IF, Attribute->mSampleRate);
    mAudioDigitalControl->SetMemIfChannelCount(Mem_IF, Attribute->mChannels);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetMEMIFEnable(AudioDigitalType::Digital_Block Mem_IF, bool bEnable)
{
    ALOGD("SetMEMIFEnable Mem_IF = %d bEnable = %d", Mem_IF, bEnable);
    mAudioDigitalControl->SetMemIfEnable(Mem_IF, bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetI2SOutDACAttribute()
{
    ALOGD("SetI2SOutDACAttribute");
    mDL1Out->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    mDL1Out->mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    mDL1Out->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    mDL1Out->mI2S_FMT = AudioDigtalI2S::I2S;
    mDL1Out->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    mDL1Out->mI2S_SAMPLERATE = sampleRate();
    mAudioDigitalControl->SetI2SDacOut(mDL1Out);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::Set2ndI2SOutAttribute()
{
    ALOGD("Set2ndI2SOutAttribute");
    m2ndI2SOut->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    m2ndI2SOut->mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    m2ndI2SOut->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    m2ndI2SOut->mI2S_FMT = AudioDigtalI2S::I2S;
    m2ndI2SOut->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    m2ndI2SOut->mI2S_SAMPLERATE = sampleRate();
    mAudioDigitalControl->Set2ndI2SOut(m2ndI2SOut);
    return NO_ERROR;
}


status_t AudioMTKStreamOut::Set2ndI2SOutAttribute(
                                               AudioDigtalI2S::LR_SWAP LRswap ,
                                               AudioDigtalI2S::I2S_SRC mode ,
                                               AudioDigtalI2S::INV_LRCK inverse,
                                               AudioDigtalI2S::I2S_FORMAT format,
                                               AudioDigtalI2S::I2S_WLEN Wlength,
                                               int samplerate )
{
    ALOGD("Set2ndI2SOutAttribute with dedicated define");
    m2ndI2SOut->mLR_SWAP = LRswap;
    m2ndI2SOut->mI2S_SLAVE = mode;
    m2ndI2SOut->mINV_LRCK = inverse;
    m2ndI2SOut->mI2S_FMT = format;
    m2ndI2SOut->mI2S_WLEN = Wlength;
    m2ndI2SOut->mI2S_SAMPLERATE = samplerate;
    mAudioDigitalControl->Set2ndI2SOut(m2ndI2SOut);
    return NO_ERROR;
}


status_t AudioMTKStreamOut::SetI2SDACOut(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAudioDigitalControl->SetI2SDacEnable(bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::Set2ndI2SOut(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAudioDigitalControl->Set2ndI2SEnable(bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetDAIBTAttribute()
{
    // fix me , ned to base on actual situation
#if defined(MTK_MERGE_INTERFACE_SUPPORT)
    mDaiBt->mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_MGRIF;
#else
    mDaiBt->mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_BT;
#endif
    mDaiBt->mDAI_BT_MODE = AudioDigitalDAIBT::Mode8K;
    mDaiBt->mDAI_DEL = AudioDigitalDAIBT::HighWord;
    mDaiBt->mBT_LEN  = 0;
    mDaiBt->mDATA_RDY = true;
    mDaiBt->mBT_SYNC = AudioDigitalDAIBT::Short_Sync;
    mDaiBt->mBT_ON = true;
    mDaiBt->mDAIBT_ON = false;
    mAudioDigitalControl->SetDAIBBT(mDaiBt);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetDAIBTOut(bool bEnable)
{
    mAudioDigitalControl->SetDAIBTEnable(bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetIMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode, AudioStreamAttribute *Attribute)
{
    ALOGD("SetIMcuIRQ1 IRQ_mode = %d sampleRate = %d mInterruptSample = %d", IRQ_mode, Attribute->mSampleRate, Attribute->mInterruptSample);
    mAudioDigitalControl->SetIrqMcuSampleRate(IRQ_mode, Attribute->mSampleRate);
    mAudioDigitalControl->SetIrqMcuCounter(IRQ_mode , Attribute->mInterruptSample);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::EnableIMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode , bool bEnable)
{
    ALOGD("EnableIMcuIRQ1 IRQ_mode = %d bEnable = %d", IRQ_mode, bEnable);
    mAudioDigitalControl->SetIrqMcuEnable(IRQ_mode, bEnable);
    return NO_ERROR;
}

uint32_t AudioMTKStreamOut::sampleRate() const
{
    return mDL1Attribute->mSampleRate;
}

size_t AudioMTKStreamOut::bufferSize() const
{
    return mDL1Attribute->mBufferSize;
}

uint32_t AudioMTKStreamOut::channels() const // TODO(Harvey): WHY? 1 & 2 ??? or 0x1 & 0x3 ???
{
    if (mDL1Attribute->mChannels == 1) {
        return AUDIO_CHANNEL_OUT_MONO;
    }
    else {
        return AUDIO_CHANNEL_OUT_STEREO;
    }
    return AUDIO_CHANNEL_OUT_STEREO;
}

int AudioMTKStreamOut::format() const
{
    return mDL1Attribute->mFormat;
}

uint32_t AudioMTKStreamOut::latency() const
{
    return mLatency;
}

status_t AudioMTKStreamOut::setVolume(float left, float right)
{
    return NO_ERROR;
}

status_t AudioMTKStreamOut::RequesetPlaybackclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    return NO_ERROR;
}
status_t AudioMTKStreamOut::ReleasePlaybackclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetPlayBackPinmux()
{
    ALOGD("SetPlayBackPinmux");
    if (mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINR) ||
        mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINL)) {
        if(!mAudioResourceManager->IsWiredHeadsetOn()){
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::MUX_AUDIO);
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL, AudioAnalogType::MUX_AUDIO);
            ALOGD("SetPlayBackPinmux spk pinmux as MUX_AUDIO");
        } else {
            ALOGD("SetPlayBackPinmux keep the same");
        }
        // keep the same pinmux
    }
    else {
        ALOGD("SetPlayBackPinmux set audio pinmux");
        uint32 DlDevice = mAudioResourceManager->getDlOutputDevice();
        if((DlDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (DlDevice & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::MUX_AUDIO);
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, AudioAnalogType::MUX_AUDIO);
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::MUX_AUDIO);
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL, AudioAnalogType::MUX_AUDIO);
        }
        if(DlDevice & AUDIO_DEVICE_OUT_EARPIECE) {
            mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_EARPIECEL, AudioAnalogType::MUX_VOICE);
        }
    }
    return NO_ERROR;
}

bool AudioMTKStreamOut::DoStereoMonoConvert(void *buffer, size_t byte)
{
    //ALOGD("DoStereoMonoConvert mSteroToMono = %d",mSteroToMono);
    if(mSteroToMono == true)
    {
        short *Sample = (short*)buffer;
        int FinalValue  =0;
        while(byte > 0)
        {
            FinalValue = ((*Sample) + (*(Sample+1)));
            *Sample++= FinalValue>>1;
            *Sample++ = FinalValue >>1;
            byte -= 4;
        }
    }
    return true;
}

status_t AudioMTKStreamOut::SetStereoToMonoFlag(int device)
{
    if((device&AUDIO_DEVICE_OUT_SPEAKER)&&(IsStereoSpeaker()== false))
    {
        mSteroToMono = true;
    }
    else
    {
        mSteroToMono = false;
    }
    return NO_ERROR;
}

void  AudioMTKStreamOut::OpenPcmDumpFile()
{
    char Buf[10];
    sprintf(Buf, "%d.pcm", DumpFileNum);
    if(mPDacPCMDumpFile == NULL){
        DumpFileName =String8(streamout);
        DumpFileName.append ((const char*)Buf);
        ALOGD("mPDacPCMDumpFile DumpFileName = %s",DumpFileName.string ());
        mPDacPCMDumpFile = AudioOpendumpPCMFile(DumpFileName.string (), streamout_propty);
    }
    if(mPFinalPCMDumpFile == NULL){
        DumpFileName =String8(streamoutfinal);
        DumpFileName.append ((const char*)Buf);
        ALOGD("mPFinalPCMDumpFile DumpFileName = %s",DumpFileName.string ());
        mPFinalPCMDumpFile = AudioOpendumpPCMFile(DumpFileName.string (), streamout_propty);
    }
    DumpFileNum++;
    DumpFileNum %=MAX_NUM_FILE;
}

void  AudioMTKStreamOut::ClosePcmDumpFile()
{
    AudioCloseDumpPCMFile(mPDacPCMDumpFile);
    AudioCloseDumpPCMFile(mPFinalPCMDumpFile);
    mPDacPCMDumpFile = NULL;
    mPFinalPCMDumpFile = NULL;
}

int AudioMTKStreamOut::WriteOriginPcmDump(void *buffer, size_t bytes)
{
    int written_data =0;
    if (mPDacPCMDumpFile)
    {
        long int position =0;
        position = ftell (mPDacPCMDumpFile);
        ALOGD("position = %d",position);
        if(position> MAX_FILE_LENGTH)
        {
            rewind(mPDacPCMDumpFile);
        }
        written_data = fwrite((void *)buffer, 1, bytes, mPDacPCMDumpFile);
    }
    return written_data;
}

bool AudioMTKStreamOut::IsStereoSpeaker()
{
    #ifdef ENABLE_STEREO_SPEAKER
    return true;
    #else
    return false;
    #endif
}

ssize_t AudioMTKStreamOut::write(const void *buffer, size_t bytes)
{
    int ret =0;
    ALOGD("%s(), buffer = %p bytes = %d mLatency = %d", __FUNCTION__, buffer, bytes, mLatency);
    // here need write data to hardware
    ssize_t WrittenBytes = 0;
    if (mSuspend || LoopbackManager::GetInstance()->GetLoopbackType() != NO_LOOPBACK) {
        usleep(mLatency * 1000);//slee for a while
        ALOGD("%s() suspend write", __FUNCTION__);
        return bytes;
    }
    AudioVUnlockDL* VUnlockhdl = AudioVUnlockDL::getInstance();
    // need lock first
    ret=  mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, 3000);
     if(ret)
     {
         ALOGW("write EnableAudioLock  AUDIO_HARDWARE_LOCK fail");
         usleep(50 * 1000);
         return bytes;
    }
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, 3000);
    if(ret)
    {
        ALOGW("write EnableAudioLock AUDIO_STREAMOUT_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }

    audio_mode_t Mode = mAudioResourceManager->GetAudioMode();
    //staring DL digital part.
    if (GetStreamRunning() == false) {
        RequesetPlaybackclock();
        OpenPcmDumpFile();
        SetStreamRunning(true);
        switch (Mode) {
            case AUDIO_MODE_NORMAL:
            case AUDIO_MODE_RINGTONE:
            case AUDIO_MODE_IN_COMMUNICATION:
            {
                ::ioctl(mFd, START_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // fp for write indentify
                uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mDL1Attribute->mdevices);
                SetAnalogFrequency(DigitalPart);
                SetPlayBackPinmux();

                writeDataToEchoReference(buffer, bytes);
                DoStereoMonoConvert((void*)buffer,bytes);

                if (DigitalPart == AudioDigitalType::DAI_BT) {
                    WrittenBytes = WriteDataToBTSCOHW(buffer, bufferSize());
                }
                else {
                    WrittenBytes =WriteDataToAudioHW(buffer, bufferSize());
                    if(VUnlockhdl != NULL)
                    {
                        VUnlockhdl->SetInputStandBy(false);
                        VUnlockhdl-> GetSRCInputParameter(mDL1Attribute->mSampleRate, mDL1Attribute->mChannels);
                        VUnlockhdl->GetStreamOutLatency(mLatency);
                    }
                    //WrittenBytes =::write(mFd, buffer, bufferSize());
                }

                TurnOnAfeDigital(DigitalPart);
                mAudioDigitalControl->SetAfeEnable(true);

                mAudioResourceManager->StartOutputDevice();  // open analog device

                // turn on irq 1 INTERRUPT_IRQ1_MCU
                SetIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, mDL1Attribute);
                EnableIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, true);

                SetMEMIFAttribute(AudioDigitalType::MEM_DL1, mDL1Attribute);
                SetMEMIFEnable(AudioDigitalType::MEM_DL1, true);
                if(VUnlockhdl != NULL)
                {
                   VUnlockhdl->GetFirstDLTime();
                }
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
                return WrittenBytes;
            }
            case AUDIO_MODE_IN_CALL:
            case AUDIO_MODE_IN_CALL_2: {
                SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
                BGSPlayer *pBGSPlayer = BGSPlayer::GetInstance();
                pBGSPlayer->mBGSMutex.lock();
                pBGSPlayer->CreateBGSPlayBuffer(mSourceSampleRate, mSourceChannels, format()); // TODO(Harvey): use channels() // Set target sample rate = 16000 Hz
                pBGSPlayer->Open(pSpeechDriver, 0x0, 0xFF);
                pBGSPlayer->mBGSMutex.unlock();
                break;
            }
            default: {
                break;
            }
        }
    }

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
    WriteOriginPcmDump((void*)buffer,bytes);

    switch (Mode) {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:
        {
            uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mDL1Attribute->mdevices);
            writeDataToEchoReference(buffer, bytes);
            DoStereoMonoConvert((void*)buffer,bytes);
            if (DigitalPart == AudioDigitalType::DAI_BT) {
                WrittenBytes = WriteDataToBTSCOHW(buffer, bufferSize());
            }
            else {
                //WrittenBytes =::write(mFd, buffer, bufferSize());
                VUnlockhdl->SetInputStandBy(false);
                WrittenBytes =WriteDataToAudioHW(buffer, bufferSize());
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2: {
            WrittenBytes = BGSPlayer::GetInstance()->Write(const_cast<void *>(buffer), bytes);
            break;
        }
        default: {
            break;
        }
    }

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
    return WrittenBytes;
}

ssize_t AudioMTKStreamOut::WriteDataToBTSCOHW(const void *buffer, size_t bytes)
{
    ssize_t WrittenBytes = 0;
    size_t outputSize = 0;
    uint8_t *outbuffer;

    outputSize = DoBTSCOSRC(buffer, bytes, (void **)&outbuffer);
    WrittenBytes =::write(mFd, outbuffer, outputSize);
    return WrittenBytes;
}

ssize_t AudioMTKStreamOut::DoBTSCOSRC(const void *buffer, size_t bytes, void **outbuffer)
{
    size_t outputSize = 0;

    if (mBliSrc) {
        if (mBliSrc->initStatus() != OK) {
            // 6628 only support 8k BTSCO
            ALOGD("DoBTSCOSRC Init BLI_SRC,mDL1Attribute->mSampleRate=%d, target=8000",mDL1Attribute->mSampleRate);
            mBliSrc->init(44100, mDL1Attribute->mChannels, 8000, mDL1Attribute->mChannels);
        }
        *outbuffer = mSwapBufferTwo;
        outputSize = mBliSrc->process(buffer, bytes, *outbuffer);
        if (outputSize <= 0) {
            outputSize = bytes;
            *outbuffer = (void *)buffer;
        }
        return outputSize;
    }
    else {
        ALOGW("DoBTSCOSRC() mBliSrc=NULL!!!");
        *outbuffer = (void *)buffer;
        return bytes;
    }
}

status_t AudioMTKStreamOut::setForceStandby(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mForceStandby = bEnable;
    return NO_ERROR;
}

status_t AudioMTKStreamOut::standby()
{
    ALOGD("+AudioMTKStreamOut standby");
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, 3000);
    ALOGD("+AudioMTKStreamOut standby() EnableAudioLock AUDIO_STREAMOUT_LOCK");
    AudioVUnlockDL* VUnlockhdl = AudioVUnlockDL::getInstance();
    //staring DL digital part.
    if (GetStreamRunning() == true) {
        SetStreamRunning(false);
        ClosePcmDumpFile();
        switch (mAudioResourceManager->GetAudioMode())
        {
            case AUDIO_MODE_NORMAL:
            case AUDIO_MODE_RINGTONE:
            case AUDIO_MODE_IN_COMMUNICATION: {
                if (mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINR) ||
                    mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINL) ||
                    mAudioDigitalControl->GetI2SConnectStatus() ) {
                    // when analog lin in enable ......
                    ALOGD("AudioMTKStreamOut standby() FM is on, LineInR(%d), LineInL(%d), I2S(%d), do nothing!!!!!!", mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINR), mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINL) , mAudioDigitalControl->GetI2SConnectStatus());
                }
                else {
                    mAudioResourceManager->StopOutputDevice();
                }
                uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mAudioResourceManager->getDlOutputDevice());
                if( mAudioDigitalControl->GetI2SConnectStatus() )
                    TurnOffAfeDigital(DigitalPart, true);
                else
                    TurnOffAfeDigital(DigitalPart, false);
                EnableIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, false);
                usleep(1*1000);
                SetMEMIFEnable(AudioDigitalType::MEM_DL1, false); // disable irq 1
                if( !mAudioDigitalControl->GetI2SConnectStatus() )
                    mAudioDigitalControl->SetAfeEnable(false);
                ::ioctl(mFd, STANDBY_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // disable mem interface mem1
                if(VUnlockhdl != NULL)
                {
                    VUnlockhdl->SetInputStandBy(true);
                }
                break;
            }
            case AUDIO_MODE_IN_CALL:
            case AUDIO_MODE_IN_CALL_2: {
                SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
                BGSPlayer *pBGSPlayer = BGSPlayer::GetInstance();
                pBGSPlayer->mBGSMutex.lock();
                pBGSPlayer->Close();
                pBGSPlayer->DestroyBGSPlayBuffer();
                pBGSPlayer->mBGSMutex.unlock();
                break;
            }
        }
        ReleasePlaybackclock();
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO,false);
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_HEADPHONE,false);
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDENH,false);
        if (mBliSrc) {
            mBliSrc->close();
        }
        StopWriteDataToEchoReference();
    }
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
    ALOGD("-AudioMTKStreamOut standby");
    return NO_ERROR;
}


status_t AudioMTKStreamOut::SetAnalogFrequency(uint32 AfeDigital)
{
    ALOGD("SetAnalogFrequency AfeDigital = %d", AfeDigital);
    switch (AfeDigital) {
        case (AudioDigitalType::DAI_BT): {
            break;
        }
        case (AudioDigitalType::I2S_OUT_DAC): {
            mAudioAnalogControl->SetFrequency(AudioAnalogType::DEVICE_OUT_DAC, mDL1Out->mI2S_SAMPLERATE);
            break;
        }
        case (AudioDigitalType::I2S_INOUT_2): {
            mAudioAnalogControl->SetFrequency(AudioAnalogType::DEVICE_OUT_DAC, mDL1Out->mI2S_SAMPLERATE);
            break;
        }
        default: {
            ALOGD("Turn on default I2S out DAC part");
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamOut::TurnOnAfeDigital(uint32 AfeDigital)
{
    ALOGD("TurnOnAfeDigital AfeDigital = %d", AfeDigital);
    switch (AfeDigital) {
        case (AudioDigitalType::DAI_BT): {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O02);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O02);
			mAudioDigitalControl->SetinputConnection(AudioDigitalType::ConnectionShift, AudioDigitalType::I05, AudioDigitalType::O02);
			mAudioDigitalControl->SetinputConnection(AudioDigitalType::ConnectionShift, AudioDigitalType::I06, AudioDigitalType::O02);
            SetDAIBTAttribute();
            SetDAIBTOut(true);
            // turn on dai_out
            break;
        }
        case (AudioDigitalType::I2S_OUT_DAC): {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O03);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O04);
            SetMEMIFEnable(AudioDigitalType::I2S_OUT_DAC, true);
            // turn on DAC_I2S out
            SetI2SOutDACAttribute();
            SetI2SDACOut(true);
            break;
        }
        case (AudioDigitalType::I2S_INOUT_2): {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O00);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O01);
            SetMEMIFEnable(AudioDigitalType::I2S_INOUT_2, true);
            // turn on 2nd_I2S out
#if defined(HDMI_2NDI2S_32BIT)
            // MT8193 need 64*fs , wordlength =32 bits.
            Set2ndI2SOutAttribute(AudioDigtalI2S::NO_SWAP,AudioDigtalI2S::MASTER_MODE,AudioDigtalI2S::NO_INVERSE,AudioDigtalI2S::I2S,AudioDigtalI2S::WLEN_32BITS,sampleRate());
#else
            Set2ndI2SOutAttribute();
#endif
            Set2ndI2SOut(true);
            break;
        }
        default: {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O03);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O04);
            SetI2SOutDACAttribute();
            SetI2SDACOut(true);
            ALOGD("Turn on default I2S out DAC part");
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamOut::TurnOffAfeDigital(uint32 AfeDigital, bool keepDacOpen)
{
    ALOGD("TurnOffAfeDigital AfeDigital = %d", AfeDigital);
    switch (AfeDigital) {
        case (AudioDigitalType::DAI_BT): {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O02);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O02);
            SetDAIBTOut(false);
            // turn off dai_out
            break;
        }
        case (AudioDigitalType::I2S_INOUT_2): {
            SetMEMIFEnable(AudioDigitalType::I2S_INOUT_2, false);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O00);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O01);
            Set2ndI2SOut(false);
            break;
        }
        case (AudioDigitalType::I2S_OUT_DAC): {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O03);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O04);
            SetMEMIFEnable(AudioDigitalType::I2S_OUT_DAC, false);
            if(!keepDacOpen)
                SetI2SDACOut(false);
            break;
        }
        default: {
            ALOGD("TurnOffAfeDigital no setting is available");
        }
    }
    return NO_ERROR;
}

void AudioMTKStreamOut::dokeyRouting(uint32_t new_device)
{
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, 3000);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK, 3000);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, 3000); // handle for output new_device change , need lock streamout first

    const uint32_t current_device = mAudioResourceManager->getDlOutputDevice();
    ALOGD("%s(), current_device = %d, new_device = %d", __FUNCTION__, current_device, new_device);

    SpeechPhoneCallController::GetInstance()->SetRoutingForTty((audio_devices_t)new_device);

    if ((new_device == 0 || new_device == current_device) ||
        (LoopbackManager::GetInstance()->GetLoopbackType() != NO_LOOPBACK)) {
#ifdef MTK_FM_SUPPORT_WFD_OUTPUT
        if(new_device == AUDIO_DEVICE_NONE && mAudioDigitalControl->GetFmDigitalStatus())
        {
            ALOGD("Replace NonDevice with Speaker for AP-path routing");
            new_device = AUDIO_DEVICE_OUT_SPEAKER;
        }
        else
#endif
            goto EXIT_SETPARAMETERS;
    }

    SetStereoToMonoFlag(new_device);
    // for DAC path
    //   Turn Off : 6320 off (analog off -> digital off) -> 6589 off
    //   Turn On  : 6589 on -> 6320 on  (digital on -> analog on)
    if (mAudioResourceManager->IsModeIncall() == true)
    {
        SpeechPhoneCallController::GetInstance()->ChangeDeviceForModemSpeechControlFlow(mAudioResourceManager->GetAudioMode(), (audio_devices_t)new_device);

        // TODO(Harvey): Reduce copy & paste code here...
        if (android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)new_device) == true) { // BT
            mDL1Attribute->mSampleRate = 8000;
            mDL1Attribute->mInterruptSample = 256;
        }
        else {
            mDL1Attribute->mSampleRate = mSourceSampleRate;
            calInterrupttime();
        }

        mDL1Attribute->mdevices = new_device;
    }
    else if (mStarting == true)
    {
        // only do with outputdevicechanged
        bool outputdevicechange = mAudioDigitalControl->CheckDlDigitalChange(current_device, new_device);
        if (true == outputdevicechange)
        {
            if(!(current_device&AUDIO_DEVICE_OUT_ALL_SCO)&&(new_device&AUDIO_DEVICE_OUT_ALL_SCO)) // change to BTSCO device
            {
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK); // disable AUDIO_STREAMOUT_LOCK since standby() will use itemAt
                standby();
                mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, 3000);
                mDL1Attribute->mSampleRate = 8000;
                mDL1Attribute->mInterruptSample = 256;
                ALOGD("setParameters mStarting=true change to BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ",mDL1Attribute->mSampleRate,mDL1Attribute->mInterruptSample);
            }
            else if((current_device&AUDIO_DEVICE_OUT_ALL_SCO)&&!(new_device&AUDIO_DEVICE_OUT_ALL_SCO)) // change from BTSCO device
            {
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK); // disable AUDIO_STREAMOUT_LOCK since standby() will use it
                standby();
                mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, 3000);
                mDL1Attribute->mSampleRate = mSourceSampleRate; //mSourceSampleRate is from AudioMTKStreamOut constructor
                calInterrupttime();
                SetI2SOutDACAttribute();
                ALOGD("setParameters mStarting=true change from BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ",mDL1Attribute->mSampleRate,mDL1Attribute->mInterruptSample);
            }
            else
            {
                int OuputPreDevice = 0 , OutPutNewDevice = 0;
                OuputPreDevice = mAudioDigitalControl->DlPolicyByDevice(current_device);
                OutPutNewDevice = mAudioDigitalControl->DlPolicyByDevice(new_device);
                TurnOffAfeDigital(OuputPreDevice, false);
                TurnOnAfeDigital(OutPutNewDevice);
            }
        }
        //select output new_device
        mAudioResourceManager->SelectOutputDevice(new_device);
        mDL1Attribute->mdevices = new_device;
    }
    else if( mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINR) ||
             mAudioAnalogControl->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINL) ||
             mAudioDigitalControl->GetFmDigitalStatus())
    {
            mAudioResourceManager->SelectOutputDevice(new_device);
            mDL1Attribute->mdevices = new_device;
    }
    else
    {
        if(!(current_device&AUDIO_DEVICE_OUT_ALL_SCO)&&(new_device&AUDIO_DEVICE_OUT_ALL_SCO)) // change to BTSCO device
        {
            mDL1Attribute->mSampleRate = 8000;
            mDL1Attribute->mInterruptSample = 256;
            ALOGD("setParameters mStarting=false change to BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ",mDL1Attribute->mSampleRate,mDL1Attribute->mInterruptSample);
        }
        else if((current_device&AUDIO_DEVICE_OUT_ALL_SCO)&&!(new_device&AUDIO_DEVICE_OUT_ALL_SCO)) // change from BTSCO device
        {
            mDL1Attribute->mSampleRate = mSourceSampleRate; //mSourceSampleRate is from AudioMTKStreamOut constructor
            SetI2SOutDACAttribute();
            calInterrupttime();
            ALOGD("setParameters mStarting=false change from BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ",mDL1Attribute->mSampleRate,mDL1Attribute->mInterruptSample);
        }
        mAudioResourceManager->setDlOutputDevice(new_device);
        mDL1Attribute->mdevices = new_device;  //mDL1Attribute->mdevices to be used in AudioMTKStreamOut::write()
    }
EXIT_SETPARAMETERS:
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
    return ;
}

status_t AudioMTKStreamOut::setParameters(const String8 &keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 keyRouting = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int devices = 0;
    ALOGD("setParameters() %s", keyValuePairs.string());
    if (param.getInt(keyRouting, devices) == NO_ERROR) {
        param.remove(keyRouting);
        dokeyRouting(devices);
        mAudioResourceManager->doSetMode();
    }
    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}


String8 AudioMTKStreamOut::getParameters(const String8 &keys)
{
    ALOGD("AudioMTKHardware getParameters\n");
    AudioParameter param = AudioParameter(keys);
    return param.toString();
}

status_t AudioMTKStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetStreamRunning(bool bEnable)
{
    ALOGD("+AudioMTKStreamOut SetStreamRunning bEnable = %d",bEnable);
    mStarting = bEnable;
    return NO_ERROR;
}

bool  AudioMTKStreamOut::GetStreamRunning()
{
    return mStarting;
}

uint32_t AudioMTKStreamOut::StreamOutCompFltProcess(AudioCompFltType_t eCompFltType, void *Buffer, uint32 bytes, void *pOutputBuf)
{
    // if return 0, means CompFilter can't do anything. Caller should use input buffer to write to Hw.
    // do post process
    if (mStreamOutCompFltEnable[eCompFltType]) {
        if (mpClsCompFltObj[eCompFltType]) {
            int dBytePerSample = ((format() == AUDIO_FORMAT_PCM_16_BIT) ? sizeof(int16_t) : sizeof(int8_t));
            int inputSampleCount = bytes / dBytePerSample;
            if (inputSampleCount >= 1024) {
                int consumedSampleCount  = inputSampleCount;
                int outputSampleCount = 0;
                mpClsCompFltObj[eCompFltType]->Process((const short *)Buffer, &consumedSampleCount, (short *)pOutputBuf, &outputSampleCount);
                size_t outputbytes = outputSampleCount * dBytePerSample;
                return outputbytes;
            }
        }
    }

    return 0;
}

status_t AudioMTKStreamOut::SetStreamOutCompFltStatus(AudioCompFltType_t eCompFltType, bool bEnable)
{

    if (NULL == mpClsCompFltObj[eCompFltType])
        return INVALID_OPERATION;

    if (false == mStreamOutCompFltEnable[eCompFltType]) {
        if (true == bEnable) {
            mpClsCompFltObj[eCompFltType]->SetWorkMode(channels()==AUDIO_CHANNEL_OUT_MONO?1:2, sampleRate(), mdCompFltMode[eCompFltType]);
            mpClsCompFltObj[eCompFltType]->Start();
            ALOGD("SetStreamOutCompFltStatus eCompFltType = %d  bEnable = %d", eCompFltType, bEnable);
            ALOGD("SetStreamOutCompFltStatus Start CompFilter");
            if(AUDIO_COMP_FLT_AUDENH==eCompFltType)
                ALOGD("AudEnh: Start\n");
        }
    }
    else {
        if (false == bEnable) {
            mpClsCompFltObj[eCompFltType]->Stop();
            mpClsCompFltObj[eCompFltType]->ResetBuffer();
                ALOGD("SetStreamOutCompFltStatus eCompFltType = %d  bEnable = %d", eCompFltType, bEnable);
                ALOGD("SetStreamOutCompFltStatus Stop CompFilter");
            if(AUDIO_COMP_FLT_AUDENH==eCompFltType)
                ALOGD("AudEnh: Stop\n");
        }
    }

    mStreamOutCompFltEnable[eCompFltType] = mStreamOutCompFltApplyStatus[eCompFltType] = bEnable;
    return NO_ERROR;
}

bool AudioMTKStreamOut::GetStreamOutCompFltStatus(AudioCompFltType_t eCompFltType)
{
    if (NULL == mpClsCompFltObj[eCompFltType])
        return false;

    return mStreamOutCompFltEnable[eCompFltType];
}

status_t AudioMTKStreamOut::SetStreamOutCompFltApplyStauts(AudioCompFltType_t eCompFltType, bool bEnable)
{

    if (NULL == mpClsCompFltObj[eCompFltType])
        return INVALID_OPERATION;

    if (false == mStreamOutCompFltEnable[eCompFltType]) {
        if (true == bEnable) {
                SetStreamOutCompFltStatus(eCompFltType,bEnable);
        }
    }
    else {

        if (true == mStreamOutCompFltApplyStatus[eCompFltType])
        {
            if (false == bEnable) {
                ALOGD("AudEnh: Pause\n");
                mpClsCompFltObj[eCompFltType]->Pause();
            }
        }
        else
        {
            if (true == bEnable) {
                ALOGD("AudEnh: Resume\n");
                mpClsCompFltObj[eCompFltType]->Resume();
            }
        }

        mStreamOutCompFltApplyStatus[eCompFltType] = bEnable;
    }

    return NO_ERROR;
}


status_t AudioMTKStreamOut::StreamOutCompFltCreate(AudioCompFltType_t eCompFltType, AudioComFltMode_t eCompFltMode)
{
    ALOGD("StreamOutFLTCreate eCompFltType = %d eCompFltMode = %d", eCompFltType, eCompFltMode);

    if (NULL != mpClsCompFltObj[eCompFltType])
        return ALREADY_EXISTS;

    if (eCompFltType >= AUDIO_COMP_FLT_NUM)
        return BAD_TYPE;

    mpClsCompFltObj[eCompFltType] = new AudioCompensationFilter(eCompFltType,bufferSize());

    if (NULL == mpClsCompFltObj[eCompFltType])
        return NO_INIT;

    mpClsCompFltObj[eCompFltType]->Init();
    mpClsCompFltObj[eCompFltType]->LoadACFParameter();
    mdCompFltMode[eCompFltType] = eCompFltMode;

    return NO_ERROR;

}

status_t AudioMTKStreamOut::StreamOutCompFltDestroy(AudioCompFltType_t eCompFltType)
{
    ALOGD("StreamOutACFDestroy eCompFltType = %d", eCompFltType);

    if (NULL == mpClsCompFltObj[eCompFltType])
        return DEAD_OBJECT;

    mpClsCompFltObj[eCompFltType]->Stop();//For Memory Leak if StreamOut doesn't stop Flt in this timing
    mpClsCompFltObj[eCompFltType]->Deinit();
    delete mpClsCompFltObj[eCompFltType];
    mpClsCompFltObj[eCompFltType] = NULL;
    mStreamOutCompFltEnable[eCompFltType] = false;
    mdCompFltMode[eCompFltType] = AUDIO_CMP_FLT_LOUDNESS_NONE;
    return NO_ERROR;
}

status_t AudioMTKStreamOut::StreamOutCompFltPreviewParameter(AudioCompFltType_t eCompFltType,void *ptr , int len)
{
    ALOGD("StreamOutACFDestroy eCompFltType = %d", eCompFltType);

    if (NULL == mpClsCompFltObj[eCompFltType])
        return DEAD_OBJECT;

    //Stop ?
    mpClsCompFltObj[eCompFltType]->SetACFPreviewParameter((AUDIO_ACF_CUSTOM_PARAM_STRUCT *)ptr);
    //Start ?

    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetSuspend(bool suspend)
{
    if (suspend) {
        mSuspend++;
    }
    else {
        mSuspend--;
        if(mSuspend <0)
        {
            ALOGW("mSuspend = %d",mSuspend);
            mSuspend =0;
        }
    }
    ALOGD("SetSuspend mSuspend = %d suspend = %d", mSuspend, suspend);
    return NO_ERROR;
}

bool AudioMTKStreamOut::GetSuspend()
{
    return mSuspend;
}

status_t AudioMTKStreamOut::dump(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

AudioMTKStreamOut::BliSrc::BliSrc()
    : mHandle(NULL), mBuffer(NULL), mInitCheck(NO_INIT)
{
}

AudioMTKStreamOut::BliSrc::~BliSrc()
{
    close();
}

status_t AudioMTKStreamOut::BliSrc::initStatus()
{
    return mInitCheck;
}

status_t  AudioMTKStreamOut::BliSrc::init(uint32 inSamplerate, uint32 inChannel, uint32 OutSamplerate, uint32 OutChannel)
{
    if (mHandle == NULL) {
        uint32_t workBufSize;
        BLI_GetMemSize(inSamplerate, inChannel, OutSamplerate, OutChannel, &workBufSize);
        ALOGD("BliSrc::init InputSampleRate=%u, inChannel=%u, OutputSampleRate=%u, OutChannel=%u, mWorkBufSize = %u",
              inSamplerate, inChannel, OutSamplerate, OutChannel, workBufSize);
        mBuffer = new uint8_t[workBufSize];
        if (!mBuffer) {
            ALOGE("BliSrc::init Fail to create work buffer");
            return NO_MEMORY;
        }
        memset((void *)mBuffer, 0, workBufSize);
        mHandle = BLI_Open(inSamplerate, 2, OutSamplerate, 2, (char *)mBuffer, NULL);
        if (!mHandle) {
            ALOGE("BliSrc::init Fail to get blisrc handle");
            if (mBuffer) {
                delete []mBuffer;
                mBuffer = NULL;
            }
            return NO_INIT;
        }
        mInitCheck = OK;
    }
    return NO_ERROR;

}

size_t  AudioMTKStreamOut::BliSrc::process(const void *inbuffer, size_t inBytes, void *outbuffer)
{
    if (mHandle) {
        size_t inputLength = inBytes;
        size_t outputLength = inBytes;
        size_t consume = BLI_Convert(mHandle, (short *)inbuffer, &inputLength, (short *)outbuffer, &outputLength);
        ALOGD_IF(consume != inBytes, "inputLength=%d,consume=%d,outputLength=%d", inputLength, consume, outputLength);
        return outputLength;
    }
    ALOGW("BliSrc::process src not initialized");
    return 0;
}

status_t  AudioMTKStreamOut::BliSrc::close(void)
{
    if (mHandle) {
        BLI_Close(mHandle, NULL);
        mHandle = NULL;
    }
    if (mBuffer) {
        delete []mBuffer;
        mBuffer = NULL;
    }
    mInitCheck = NO_INIT;
    return NO_ERROR;
}

int AudioMTKStreamOut::GetSampleRate(void)
{
    return mDL1Attribute->mSampleRate;
}
int AudioMTKStreamOut::GetChannel(void)
{
    return mDL1Attribute->mChannels;
}

bool AudioMTKStreamOut::EffectMutexLock(void)
{
    return mEffectLock.lock ();
}
bool AudioMTKStreamOut::EffectMutexUnlock(void)
{
    mEffectLock.unlock ();
    return true;
}

void AudioMTKStreamOut::add_echo_reference(struct echo_reference_itfe *reference)
{
    Mutex::Autolock _l(mEffectLock);
    ALOGD("add_echo_reference %p",reference);
    mEcho_reference = reference;

}
void AudioMTKStreamOut::remove_echo_reference(struct echo_reference_itfe *reference)
{
    Mutex::Autolock _l(mEffectLock);
    ALOGD("remove_echo_reference %p",reference);
    if (mEcho_reference == reference) {
        /* stop writing to echo reference */
        reference->write(reference, NULL);
        mEcho_reference = NULL;
    }
    else
        ALOGW("remove wrong echo reference %p",reference);
}

int AudioMTKStreamOut::get_playback_delay(size_t frames, struct echo_reference_buffer *buffer)
{
    struct timespec tstamp;
    size_t kernel_frames;

    //FIXME:: calculate for more precise time delay

    int rc = clock_gettime(CLOCK_MONOTONIC, &tstamp);
    if (rc != 0) {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGW("get_playback_delay(): pcm_get_htimestamp error,"
                "setting playbackTimestamp to 0");
        return 0;
    }

    /* adjust render time stamp with delay added by current driver buffer.
     * Add the duration of current frame as we want the render time of the last
     * sample being written. */
    buffer->delay_ns = (long)(((int64_t)(frames)* 1000000000)/
                            mDL1Attribute->mSampleRate);

    buffer->time_stamp = tstamp;

//    ALOGD("get_playback_delay time_stamp = [%ld].[%ld], delay_ns: [%d]",buffer->time_stamp.tv_sec , buffer->time_stamp.tv_nsec, buffer->delay_ns);
    return 0;
}

size_t AudioMTKStreamOut::writeDataToEchoReference(const void* buffer, size_t bytes)
{
#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
    //push the output data to echo reference
    Mutex::Autolock _l(mEffectLock);
    if (mEcho_reference != NULL) {
        ALOGV("writeDataToEchoReference echo_reference %p",mEcho_reference);
        struct echo_reference_buffer b;
        b.raw = (void *)buffer;
        b.frame_count = bytes/sizeof(int16_t)/mDL1Attribute->mChannels;

       get_playback_delay(b.frame_count, &b);
       mEcho_reference->write(mEcho_reference, &b);
    }
#endif
    return bytes;
}

void AudioMTKStreamOut::StopWriteDataToEchoReference()
{
    /* stop writing to echo reference */
#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
    Mutex::Autolock _l(mEffectLock);
    ALOGD("StopWriteDataToEchoReference %p",mEcho_reference);
    if (mEcho_reference != NULL) {
        mEcho_reference->write(mEcho_reference, NULL);
        mEcho_reference = NULL;
    }
#endif
}

void AudioMTKStreamOut::SetMusicPlusStatus(bool bEnable)
{
    #if defined(MTK_AUDENH_SUPPORT)

    mUseAudCompFltHeadphoneWithFixedParameter = bEnable;
    #if 0
    if(bEnable)
        property_set("persist.af.audenh.ctrl", "1");
    else
        property_set("persist.af.audenh.ctrl", "0");
    #else
    AUDIO_AUDENH_CONTROL_OPTION_STRUCT audioParam;
    audioParam.u32EnableFlg = bEnable?1:0;
    SetAudEnhControlOptionParamToNV(&audioParam);
    #endif

    ALOGD("AudEnh: Set Control [%d]\n",bEnable);
    #else
    ALOGW("System Unsupport AudEnh Feature\n");
    #endif

    return;
}

bool AudioMTKStreamOut::GetMusicPlusStatus(void)
{
    ALOGD("AudEnh: Get Control [%d]\n",mUseAudCompFltHeadphoneWithFixedParameter);
    return mUseAudCompFltHeadphoneWithFixedParameter;
}

size_t AudioMTKStreamOut::WriteDataToAudioHW(const void *buffer, size_t bytes)
{
    const uint32_t current_device = mAudioResourceManager->getDlOutputDevice();
    size_t outputSize = 0;
    void * inbuffer = (void *)buffer; // mSwapBufferOne  or mixerbufer of audioflinger
    void * outbuffer =mSwapBufferTwo;
    void * outbuffer2 =mSwapBufferThree;
    AudioVUnlockDL* VUnlockhdl = AudioVUnlockDL::getInstance();
    //Remove All Filter Definition for Clear trace. However it won't affect Performance if the define is disabled (Return from process function,right now)
    if (current_device & AUDIO_DEVICE_OUT_SPEAKER)
    {
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO,true);
        outputSize = StreamOutCompFltProcess(AUDIO_COMP_FLT_AUDIO,inbuffer,bytes,outbuffer);
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_HEADPHONE,false);
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDENH,false);
    }
    else if ((current_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (current_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
    {
        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_HEADPHONE,true);

        if (true == mUseAudCompFltHeadphoneWithFixedParameter)
            SetStreamOutCompFltApplyStauts(AUDIO_COMP_FLT_AUDENH,true);
        else
            SetStreamOutCompFltApplyStauts(AUDIO_COMP_FLT_AUDENH,false);

        outputSize = StreamOutCompFltProcess(AUDIO_COMP_FLT_AUDENH,inbuffer,bytes,outbuffer2);

        if(outputSize==0)
            outputSize = bytes;
        else
            inbuffer = outbuffer2;

        outputSize = StreamOutCompFltProcess(AUDIO_COMP_FLT_HEADPHONE,inbuffer,outputSize,outbuffer);

        SetStreamOutCompFltStatus(AUDIO_COMP_FLT_AUDIO,false);

    }
    else//Other ?
    {
        outbuffer = (void *)buffer;
    }

    if(outputSize==0)
    {
        outbuffer=inbuffer; //acf not handle inbuffer;
        outputSize = bytes;
    }
    if (mPFinalPCMDumpFile)
    {
         long int position =0;
         position = ftell (mPFinalPCMDumpFile);
         if(position> MAX_FILE_LENGTH)
         {
             rewind(mPFinalPCMDumpFile);
         }
         int written_data = fwrite((void *)outbuffer, 1, outputSize, mPFinalPCMDumpFile);
         ALOGV("position = %d written_data = %d",position,written_data);
    }
    outputSize =::write(mFd, outbuffer, outputSize);
    if(VUnlockhdl != NULL)
    {
        //VUnlockhdl->SetInputStandBy(false);
         if ((current_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (current_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
        {
            memset(outbuffer, 0,outputSize);
        }
        VUnlockhdl->WriteStreamOutToRing(outbuffer, outputSize);
    }

    return  outputSize;
}

}
