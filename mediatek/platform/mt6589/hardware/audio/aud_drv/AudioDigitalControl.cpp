#include "AudioDigitalControl.h"
#include "AudioDigitalType.h"
#include "AudioInterConnection.h"
#include "AudioAfeReg.h"
#include "audio_custom_exp.h"
#include <linux/fm.h>

#define LOG_TAG "AudioDigitalControl"
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

namespace android
{

AudioDigitalControl *AudioDigitalControl::UniqueDigitalInstance = 0;

AudioDigitalControl *AudioDigitalControl::getInstance()
{
    if (UniqueDigitalInstance == 0) {
        ALOGD("+UniqueDigitalInstance\n");
        UniqueDigitalInstance = new AudioDigitalControl();
        ALOGD("-UniqueDigitalInstance\n");
    }
    return UniqueDigitalInstance;
}

AudioDigitalControl::AudioDigitalControl()
{
    ALOGD("+%s(), contructor\n", __FUNCTION__);
    mAfeReg = NULL;
    mFd = 0;
    mAfeReg = AudioAfeReg::getInstance();
    if (!mAfeReg) {
        ALOGW("mAfeReg init fail ! \n");
    }
    mFd = mAfeReg->GetAfeFd();
    if (mFd == 0) {
        ALOGW("mFd  AudioDigitalControl = %d ", mFd);
    }

    for (int i = 0; i < AudioDigitalType::NUM_OF_IRQ_MODE ; i++) {
        memset((void *)&mAudioMcuMode[i], 0, sizeof(mAudioMcuMode));
    }
    memset((void *)&mMrgIf, 0, sizeof(AudioMrgIf));
    mMrgIf.Mrg_I2S_SampleRate = AudioMrgIf::MRFIF_I2S_32K;

    // init default for stream attribute
    for (int i = 0; i < AudioDigitalType::NUM_OF_MEM_INTERFACE ; i++) {
        mAudioMEMIF[i].mState = AudioMEMIFAttribute::STATE_FREE;
        mAudioMEMIF[i].mMemoryInterFaceType = i;
        mAudioMEMIF[i].mChannels = 0;
        mAudioMEMIF[i].mInterruptSample = 0;
        mAudioMEMIF[i].mBufferSize = 0;
        mAudioMEMIF[i].mMonoSel = -1 ;
        mAudioMEMIF[i].mdupwrite = -1 ;
    }
    mAudioMEMIF[AudioDigitalType::MEM_DL1].mDirection = AudioDigitalType::DIRECTION_OUTPUT;
    mAudioMEMIF[AudioDigitalType::MEM_DL2].mDirection = AudioDigitalType::DIRECTION_OUTPUT;
    mAudioMEMIF[AudioDigitalType::MEM_DAI].mDirection = AudioDigitalType::DIRECTION_INPUT;
    mAudioMEMIF[AudioDigitalType::MEM_AWB].mDirection = AudioDigitalType::DIRECTION_INPUT;
    mAudioMEMIF[AudioDigitalType::MEM_MOD_DAI].mDirection = AudioDigitalType::DIRECTION_INPUT;
    mAudioMEMIF[AudioDigitalType::MEM_VUL].mDirection = AudioDigitalType::DIRECTION_INPUT;
    mI2SConnectStatus = false;
    mFmDigitalStatus = false;

    mMicInverse = false;

    // init digital control
    mInterConnectionInstance = new AudioInterConnection();

    mAfeReg->SetAfeReg(FPGA_CFG0, 0x00001007, 0xffffffff);   // hopping 32m, MCLK : 3.072M
    mAfeReg->SetAfeReg(FPGA_CFG1, 0x00000000, 0xffffffff);   // hopping 32m, MCLK : 3.072M

    // test loop to test if there is someting wrong
    /*
    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::Connection,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }
    mInterConnectionInstance->dump();

    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::DisConnect,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }

    mInterConnectionInstance->dump();
    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::ConnectionShift,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }
    mInterConnectionInstance->dump();
    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::DisConnect,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }
    mInterConnectionInstance->dump();
    */

    // side tone filter
    mSideToneFilterOn = false;

}

status_t AudioDigitalControl::InitCheck()
{
    ALOGD("+%s()\n", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemBufferSize(uint32 InterfaceType, uint32 BufferSize)
{
    ALOGV("+%s(), InterfaceType = %d, BufferSize = %d\n", __FUNCTION__, InterfaceType, BufferSize);
    // only State Free can
    if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_FREE) {
        mAudioMEMIF[InterfaceType].mBufferSize = BufferSize;
    }
    else if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_FREE) {
        ALOGD("MemType = %d mState = %d\n", InterfaceType, mAudioMEMIF[InterfaceType].mState);
        mAudioMEMIF[InterfaceType].mBufferSize = BufferSize;
    }
    else {
        // State is executing , cannot set
        ALOGD("MemType = %d mState = %d\n", InterfaceType, mAudioMEMIF[InterfaceType].mState);
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemBufferSize(uint32 InterfaceType)
{
    ALOGV("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    return mAudioMEMIF[InterfaceType].mBufferSize;
}

status_t AudioDigitalControl::AllocateMemBufferSize(uint32 InterfaceType)
{
    // here to calocate buffer with audio hardware base on mem if
    ALOGD("+%s(), allocate buffer InterfaceType = %d, mBufferSize = %d \n", __FUNCTION__, InterfaceType, mAudioMEMIF[InterfaceType].mBufferSize);
    int ret = 0;
    if (mAudioMEMIF[InterfaceType].mBufferSize && mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_FREE) {
        // todo ::here to tell kernel drvier to do allocate memory
        switch (InterfaceType) {
            case AudioDigitalType::MEM_DL1: {
                ret = ::ioctl(mFd, ALLOCATE_MEMIF_DL1, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_DL2: {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_DL2, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_AWB: {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_AWB, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_VUL: {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_ADC, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_DAI: {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_DAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_MOD_DAI: {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_MODDAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            default:
                ALOGW("no such MEM interface");
                break;
        }
        mAudioMEMIF[InterfaceType].mState = AudioMEMIFAttribute::STATE_STANDBY;
    }
    ALOGD("allocate buffer done");
    return ret;
}

status_t AudioDigitalControl::FreeMemBufferSize(uint32 InterfaceType)
{
    // here to calocate buffer with audio hardware base on mem if
    ALOGD("+%s(), buffer InterfaceType = %d \n", __FUNCTION__, InterfaceType);
    int ret = 0;
    if (mAudioMEMIF[InterfaceType].mBufferSize && mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_STANDBY) {
        switch (InterfaceType) {
            case AudioDigitalType::MEM_DL1: {
                ret =::ioctl(mFd, FREE_MEMIF_DL1, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_DL2: {
                ret =::ioctl(mFd, FREE_MEMIF_DL2, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_AWB: {
                ret =::ioctl(mFd, FREE_MEMIF_AWB, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_VUL: {
                ret =::ioctl(mFd, FREE_MEMIF_ADC, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_DAI: {
                ret =::ioctl(mFd, FREE_MEMIF_DAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_MOD_DAI: {
                ret = ::ioctl(mFd, FREE_MEMIF_MODDAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            default:
                ALOGW("no such MEM interface");
                break;
        }
        mAudioMEMIF[InterfaceType].mState = AudioMEMIFAttribute::STATE_FREE;
        mAudioMEMIF[InterfaceType].mBufferSize = 0;
    }
    ALOGD("-%s()\n", __FUNCTION__);
    return ret;
}

AudioMEMIFAttribute::SAMPLINGRATE AudioDigitalControl::SampleRateTransform(uint32 SampleRate)
{
    ALOGD("+%s(), SampleRate = %d\n", __FUNCTION__, SampleRate);
    switch (SampleRate) {
        case 8000:
            return AudioMEMIFAttribute::AFE_8000HZ;
        case 11025:
            return AudioMEMIFAttribute::AFE_11025HZ;
        case 12000:
            return AudioMEMIFAttribute::AFE_12000HZ;
        case 16000:
            return AudioMEMIFAttribute::AFE_16000HZ;
        case 22050:
            return AudioMEMIFAttribute::AFE_22050HZ;
        case 24000:
            return AudioMEMIFAttribute::AFE_24000HZ;
        case 32000:
            return AudioMEMIFAttribute::AFE_32000HZ;
        case 44100:
            return AudioMEMIFAttribute::AFE_44100HZ;
        case 48000:
            return AudioMEMIFAttribute::AFE_48000HZ;
        default:
            ALOGE("SampleRateTransform no such samplerate matching SampleRate = %d", SampleRate);
            break;
    }
    return AudioMEMIFAttribute::AFE_44100HZ;
}

status_t AudioDigitalControl::SetMemIfSampleRate(uint32 InterfaceType, uint32 SampleRate)
{
    mAudioMEMIF[InterfaceType].mSampleRate = SampleRateTransform(SampleRate);
    ALOGD("+%s(), InterfaceType = %d, SampleRate = %d, mAudioMEMIF[InterfaceType].mSampleRate = %d\n"
          , __FUNCTION__, InterfaceType, SampleRate, mAudioMEMIF[InterfaceType].mSampleRate);
    switch (InterfaceType) {
        case AudioDigitalType::MEM_DL1: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate , 0x0000000f);
            break;
        }
        case AudioDigitalType::MEM_DL2: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 4 , 0x000000f0);
            break;
        }
        case AudioDigitalType::MEM_I2S: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 8 , 0x00000f00);
            break;
        }
        case AudioDigitalType::MEM_AWB: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 12, 0x0000f000);
            break;
        }
        case AudioDigitalType::MEM_VUL: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 16, 0x000f0000);
            break;
        }
        case AudioDigitalType::MEM_DAI: {
            if (SampleRate == AudioMEMIFAttribute::AFE_8000HZ) {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 0 << 20 , 1 << 20);
            }
            else if (SampleRate == AudioMEMIFAttribute::AFE_16000HZ) {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 1 << 20 , 1 << 20);
            }
            else {
                return INVALID_OPERATION;
            }
            break;
        }
        case AudioDigitalType::MEM_MOD_DAI: {
            if (SampleRate == AudioMEMIFAttribute::AFE_8000HZ) {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 0 << 30, 1 << 30);
            }
            else if (SampleRate == AudioMEMIFAttribute::AFE_16000HZ) {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 1 << 30, 1 << 30);
            }
            else {
                return INVALID_OPERATION;
            }
            break;
        }
        default:
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemIfSampleRate(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    mAudioMEMIF[InterfaceType].mSampleRate;
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemIfChannelCount(uint32 InterfaceType, uint32 Channels)
{
    ALOGD("+%s(), InterfaceType = %d, Channels = %d\n", __FUNCTION__, InterfaceType, Channels);
    mAudioMEMIF[InterfaceType].mChannels = Channels;
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemIfChannelCount(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    int MemType = (int)InterfaceType;
    return mAudioMEMIF[MemType].mChannels;
}

status_t AudioDigitalControl::SetMemIfEnable(uint32 InterfaceType, uint32 State)
{
    ALOGD("+%s(), InterfaceType %d, State %d\n", __FUNCTION__, InterfaceType, State);
    ALOGD("state from %d ==> %d", mAudioMEMIF[InterfaceType].mState, State);
    if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_EXECUTING && State == AudioMEMIFAttribute::STATE_STANDBY) {
        //todo disable momory interface
        SetMemoryPathEnable(InterfaceType , 0);

    }
    else if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_STANDBY && State == AudioMEMIFAttribute::STATE_EXECUTING) {
        //todo Enable momory interface.
        SetMemoryPathEnable(InterfaceType , 1);
    }
    mAudioMEMIF[InterfaceType].mState = State;
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemoryPathEnable(uint32 InterfaceType, bool bEnable)
{
    ALOGD("+%s(), InterfaceType = %d, bEnable = %d\n", __FUNCTION__, InterfaceType, bEnable);
    if (bEnable && InterfaceType < AudioDigitalType::NUM_OF_MEM_INTERFACE) {
        mAfeReg->SetAfeReg(AFE_DAC_CON0, bEnable << (InterfaceType + 1) , 1 << (InterfaceType + 1));
    }
    else {
        mAfeReg->SetAfeReg(AFE_DAC_CON0, bEnable << (InterfaceType + 1), 1 << (InterfaceType + 1));
#ifdef SIDEGEN_ENABLE
        mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x0, 0xffffffff);
#endif
    }
    //mark interface ewnable or disable
    switch (InterfaceType) {
        case (AudioDigitalType::MEM_DL1):
            mAudioDigitalBlock[AudioDigitalType::MEM_DL1] = bEnable;
            break;
        case (AudioDigitalType::MEM_DL2):
            mAudioDigitalBlock[AudioDigitalType::MEM_DL2] = bEnable;
            break;
        case (AudioDigitalType::MEM_VUL):
            mAudioDigitalBlock[AudioDigitalType::MEM_VUL] = bEnable;
#ifdef SIDEGEN_ENABLE
            mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xc4662662, 0xffffffff);
#endif
            break;
        case (AudioDigitalType::MEM_DAI):
            mAudioDigitalBlock[AudioDigitalType::MEM_DAI] = bEnable;
            break;
        case (AudioDigitalType::MEM_AWB):
            mAudioDigitalBlock[AudioDigitalType::MEM_AWB] = bEnable;
            break;
        case (AudioDigitalType::MEM_MOD_DAI):
            mAudioDigitalBlock[AudioDigitalType::MEM_MOD_DAI] = bEnable;
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemDuplicateWrite(uint32 InterfaceType, int dupwrite)
{
    ALOGD("+%s(), InterfaceType = %d, dupwrite = %d\n", __FUNCTION__, InterfaceType, dupwrite);
    mAudioMEMIF[InterfaceType].mdupwrite = dupwrite;
    switch (InterfaceType) {
        case AudioDigitalType::MEM_DAI: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, dupwrite << 29, 1 << 29);
            break;
        }
        case AudioDigitalType::MEM_MOD_DAI: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, dupwrite << 31, 1 << 31);
            break;
        }
        default:
            ALOGW("%s(), InterfaceType = %d, dupwrite = %d\n", __FUNCTION__, InterfaceType, dupwrite);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemMonoChannel(uint32 Memory_Interface, bool channel)
{
    ALOGD("+%s(), Memory_Interface = %d, channel = %d\n", __FUNCTION__, Memory_Interface, channel);
    mAudioMEMIF[Memory_Interface].mMonoSel = channel;
    switch (Memory_Interface) {
        case AudioDigitalType::MEM_AWB: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, channel << 24, 1 << 24);
            break;
        }
        case AudioDigitalType::MEM_VUL: {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, channel << 27, 1 << 27);
            break;
        }
        default:
            ALOGW("%s(), Memory_Interface = %d, channel = %d\n", __FUNCTION__, Memory_Interface, channel);
            return INVALID_OPERATION;
    }
    return NO_ERROR;

}

status_t AudioDigitalControl::SetMemIfInterruptSample(uint32 InterfaceType, uint32 SampleCount)
{
    ALOGD("+%s(), InterfaceType = %d, SampleCount = %d\n", __FUNCTION__, InterfaceType, SampleCount);
    mAudioMEMIF[InterfaceType].mInterruptSample = SampleCount;
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemIfInterruptSample(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    return mAudioMEMIF[InterfaceType].mInterruptSample;
}

status_t AudioDigitalControl::SetinputConnection(uint32 ConnectionState, uint32 Input , uint32 Output)
{
    ALOGD("+%s(), ConnectionState = %d, Input = %d, Output = %d\n", __FUNCTION__, ConnectionState, Input, Output);
    if (mInterConnectionInstance) {
        return mInterConnectionInstance->SetinputConnection(ConnectionState, Input, Output);
    }
    else {
        return INVALID_OPERATION;
    }
}

status_t AudioDigitalControl::SetIrqMcuEnable(AudioDigitalType::IRQ_MCU_MODE Irqmode, bool bEnable)
{
    ALOGD("+%s(), Irqmode = %d, bEnable = %d\n", __FUNCTION__, Irqmode, bEnable);
    switch (Irqmode) {
        case AudioDigitalType::IRQ1_MCU_MODE:
        case AudioDigitalType::IRQ2_MCU_MODE:
        case AudioDigitalType::IRQ3_MCU_MODE: {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CON, (bEnable << Irqmode), (1 << Irqmode));
            mAudioMcuMode[Irqmode].mStatus = bEnable;
            break;
        }
        default:
            ALOGW("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetIrqMcuSampleRate(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 SampleRate)
{
    ALOGD("+%s(), Irqmode = %d, SampleRate = %d\n", __FUNCTION__, Irqmode, SampleRate);
    switch (Irqmode) {
        case AudioDigitalType::IRQ1_MCU_MODE: {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CON, (SampleRateTransform(SampleRate) << 4), 0x000000f0);
            mAudioMcuMode[Irqmode].mSampleRate = SampleRate;
            break;
        }
        case AudioDigitalType::IRQ2_MCU_MODE: {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CON, (SampleRateTransform(SampleRate) << 8), 0x00000f00);
            mAudioMcuMode[Irqmode].mSampleRate = SampleRate;
            break;
        }
        case AudioDigitalType::IRQ3_MCU_MODE:
        default:
            ALOGW("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetIrqMcuCounter(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 Counter)
{
    ALOGD("+%s(), Irqmode = %d, Counter = %d\n", __FUNCTION__, Irqmode, Counter);
    switch (Irqmode) {
        case AudioDigitalType::IRQ1_MCU_MODE: {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CNT1, Counter, 0xffffffff);
            mAudioMcuMode[Irqmode].mIrqMcuCounter = Counter;
            break;
        }
        case AudioDigitalType::IRQ2_MCU_MODE: {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CNT2, Counter, 0xffffffff);
            mAudioMcuMode[Irqmode].mIrqMcuCounter = Counter;
            break;
        }
        case AudioDigitalType::IRQ3_MCU_MODE:
        default:
            ALOGW("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMicinputInverse(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mMicInverse = bEnable;
    return NO_ERROR;
}


status_t AudioDigitalControl::GetIrqStatus(AudioDigitalType::IRQ_MCU_MODE Irqmode, AudioIrqMcuMode *Mcumode)
{
    switch (Irqmode) {
        case AudioDigitalType::IRQ1_MCU_MODE:
        case AudioDigitalType::IRQ2_MCU_MODE:
        case AudioDigitalType::IRQ3_MCU_MODE:
            memcpy((void *)Mcumode, (const void *)&mAudioMcuMode[Irqmode], sizeof(AudioIrqMcuMode));
            break;
        default:
            ALOGE("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetAfeEnable(bool  bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    bool AfeRunning = GetAfeDigitalStatus();
    ALOGD("SetAfeEnable bEnable = %d", bEnable);
    if (AfeRunning && (!bEnable)) {
        ALOGW("SetAfeEnable disable but digital still running");
        return INVALID_OPERATION;
    }
    mAfeReg->SetAfeReg(AFE_DAC_CON0, bEnable, 0x1); // AFE_DAC_CON0[0]: AFE_ON
    return NO_ERROR;
}



bool AudioDigitalControl::GetAfeEnable(bool  bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    uint32 AFE_DAC_CON0_reg  = mAfeReg->GetAfeReg(AFE_DAC_CON0);
    return (AFE_DAC_CON0_reg & 0x1);
}

int AudioDigitalControl::GetDigitalBlockState(int block)
{
    ALOGD("+%s(), block = %d\n", __FUNCTION__, block);
    if (block = AudioDigitalType::NUM_OF_DIGITAL_BLOCK) {
        ALOGW("%s(), block = %d\n", __FUNCTION__, block);
        return false;
    }
    else {
        return mAudioDigitalBlock[block];
    }
}

bool AudioDigitalControl::GetAfeDigitalStatus()
{
    ALOGD("+%s()\n", __FUNCTION__);
    for (int i = 0 ; i < AudioDigitalType::NUM_OF_DIGITAL_BLOCK ; i++) {
        if (mAudioMEMIF[i].mState == AudioMEMIFAttribute::STATE_EXECUTING) {
            ALOGW("GetAfeDigitalStatus mAudioMEMIF[%d] state = %d", i, mAudioMEMIF[i].mState);
            return true;
        }
    }
    return false;
}

status_t AudioDigitalControl::SetI2SDacOutAttribute(uint32_t sampleRate)
{
    ALOGD("+%s(), sampleRate = %d\n", __FUNCTION__, sampleRate);
    mDacI2SOut.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    mDacI2SOut.mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    mDacI2SOut.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    mDacI2SOut.mI2S_FMT = AudioDigtalI2S::I2S;
    mDacI2SOut.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    mDacI2SOut.mI2S_SAMPLERATE = sampleRate;
    SetI2SDacOut(&mDacI2SOut);
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SDacOut(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)&mDacI2SOut, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));
    uint32 Audio_I2S_Dac = 0;
    Audio_I2S_Dac |= (mDacI2SOut.mLR_SWAP << 31);
    Audio_I2S_Dac |= (SampleRateTransform(mDacI2SOut.mI2S_SAMPLERATE) << 8);
    Audio_I2S_Dac |= (mDacI2SOut.mINV_LRCK << 5);
    Audio_I2S_Dac |= (mDacI2SOut.mI2S_FMT << 3);
    Audio_I2S_Dac |= (mDacI2SOut.mI2S_WLEN << 1);
    ALOGD("%s(), Audio_I2S_Dac = 0x%x\n", __FUNCTION__, Audio_I2S_Dac);
    mAfeReg->SetAfeReg(AFE_I2S_CON1, Audio_I2S_Dac, AFE_MASK_ALL);
    return NO_ERROR;
}

status_t AudioDigitalControl::GetI2SDacOut(AudioDigtalI2S *mDigitalI2S)
{
    memcpy((void *)mDigitalI2S, (void *)&mDacI2SOut, sizeof(AudioDigtalI2S));
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SDacEnable(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mDacI2SOut.mI2S_EN = bEnable;
    mAfeReg->SetAfeReg(AFE_I2S_CON1, bEnable, 0x1); // TODO: AFE_I2S_CON1 defined both in "AudioAfeReg.h" & AudioAnalogReg.h
    mAudioDigitalBlock[AudioDigitalType::I2S_OUT_DAC] = bEnable;
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SOut(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)&m2ndI2S, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));
    uint32 u32AudioI2S = 0;
    // set 2nd samplerate to AFE_ADC_CON1
    SetMemIfSampleRate(AudioDigitalType::MEM_I2S, m2ndI2S.mI2S_SAMPLERATE);
    u32AudioI2S |= (m2ndI2S.mINV_LRCK << 5);
    u32AudioI2S |= (AudioDigtalI2S::I2S_INPUT << 4); // default set to input mode, which turn on both input & output direction
    u32AudioI2S |= (m2ndI2S.mI2S_FMT << 3);
    u32AudioI2S |= (AudioDigtalI2S::MASTER_MODE << 2); // default set to master mode
    u32AudioI2S |= (m2ndI2S.mI2S_WLEN << 1);
    ALOGD("Set2ndI2SOut u32AudioI2S= 0x%x", u32AudioI2S);
    mAfeReg->SetAfeReg(AFE_I2S_CON, u32AudioI2S, AFE_MASK_ALL);
    return NO_ERROR;
}

status_t AudioDigitalControl::Get2ndI2SOut(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)mDigitalI2S, (void *)&m2ndI2S, sizeof(AudioDigtalI2S));
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SEnable(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    m2ndI2S.mI2S_EN = bEnable;
    mAfeReg->SetAfeReg(AFE_I2S_CON, bEnable, 0x1);
    mAudioDigitalBlock[AudioDigitalType::I2S_INOUT_2] = bEnable;
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SAdcIn(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)&mAdcI2SIn, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));
    uint32 Audio_I2S_Adc = 0;

    // ADC swap or not only controlled by mMicInverse
    mAdcI2SIn.mLR_SWAP = mMicInverse;

    Audio_I2S_Adc |= (mAdcI2SIn.mLR_SWAP << 31);
    Audio_I2S_Adc |= (mAdcI2SIn.mBuffer_Update_word << 24);
    Audio_I2S_Adc |= (mAdcI2SIn.mINV_LRCK << 23);
    Audio_I2S_Adc |= (mAdcI2SIn.mFpga_bit_test << 22);
    Audio_I2S_Adc |= (mAdcI2SIn.mFpga_bit << 21);
    Audio_I2S_Adc |= (mAdcI2SIn.mloopback << 20);
    Audio_I2S_Adc |= (SampleRateTransform(mAdcI2SIn.mI2S_SAMPLERATE) << 8);
    Audio_I2S_Adc |= (mAdcI2SIn.mI2S_FMT << 3);
    Audio_I2S_Adc |= (mAdcI2SIn.mI2S_WLEN << 1);
    ALOGD("%s Audio_I2S_Adc = 0x%x\n", __FUNCTION__, Audio_I2S_Adc);
    mAfeReg->SetAfeReg(AFE_I2S_CON2, Audio_I2S_Adc, AFE_MASK_ALL);
    return NO_ERROR;
}

status_t AudioDigitalControl::GetI2SAdcIn(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)mDigitalI2S, (void *)&mAdcI2SIn, sizeof(AudioDigtalI2S));
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SAdcEnable(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAdcI2SIn.mI2S_EN = bEnable;
    mAfeReg->SetAfeReg(AFE_I2S_CON2, bEnable, 0x1);
    mAudioDigitalBlock[AudioDigitalType::I2S_IN_ADC] = bEnable;
    return NO_ERROR;
}

AudioDigtalI2S::I2S_SAMPLERATE AudioDigitalControl::I2SSampleRateTransform(unsigned int SampleRate)
{
    ALOGD("+%s(), SampleRate = %d\n", __FUNCTION__, SampleRate);
    switch (SampleRate) {
        case 8000:
            return AudioDigtalI2S::I2S_8K;
        case 11025:
            return AudioDigtalI2S::I2S_11K;
        case 12000:
            return AudioDigtalI2S::I2S_12K;
        case 16000:
            return AudioDigtalI2S::I2S_16K;
        case 22050:
            return AudioDigtalI2S::I2S_22K;
        case 24000:
            return AudioDigtalI2S::I2S_24K;
        case 32000:
            return AudioDigtalI2S::I2S_32K;
        case 44100:
            return AudioDigtalI2S::I2S_44K;
        case 48000:
            return AudioDigtalI2S::I2S_48K;
        default:
            ALOGE("I2SSampleRateTransform no such samplerate matching SampleRate = %d", SampleRate);
            break;
    }
    return AudioDigtalI2S::I2S_44K;
}

status_t AudioDigitalControl::SetMrgI2SEnable(bool bEnable, unsigned int sampleRate)
{
#if defined(MTK_MERGE_INTERFACE_SUPPORT)
    ALOGD("+%s(), bEnable = %d, sampleRate = %d. Current setting enable %d, SR_idx %d,I2S %d, DAI %d",
          __FUNCTION__, bEnable, sampleRate, mMrgIf.MrgIf_En, mMrgIf.Mrg_I2S_SampleRate, mMrgIf.Mergeif_I2S_Enable, mDaiBt.mDAIBT_ON);
    if (bEnable == true) {
        // To enable MrgI2S
        if (mMrgIf.MrgIf_En == true) {
            // Merge Interface already turn on.
            //if sample Rate change, then it need to restart with new setting; else do nothing.
            if (mMrgIf.Mrg_I2S_SampleRate != I2SSampleRateTransform(sampleRate)) {
                //Turn off Merge Interface first to switch I2S sampling rate
                mAfeReg->SetAfeReg(AFE_MRGIF_CON, 0, 1 << 16); // Turn off I2S
                if (mDaiBt.mDAIBT_ON == true)
                    mAfeReg->SetAfeReg(AFE_DAIBT_CON0, 0, 0x1); //Turn off DAIBT first

                mAfeReg->SetAfeReg(AFE_MRGIF_CON, 0, 0x1);    // Turn off Merge Interface
                // need delay? then turn on merge interface, I2S, PCM
                mAfeReg->SetAfeReg(AFE_MRGIF_CON, 1, 0x1);    // Turn on Merge Interface
                if (mDaiBt.mDAIBT_ON == true)
                    mAfeReg->SetAfeReg(AFE_DAIBT_CON0, 1, 0x1); //Turn on DAIBT
                mMrgIf.Mrg_I2S_SampleRate = I2SSampleRateTransform(sampleRate);
                mAfeReg->SetAfeReg(AFE_MRGIF_CON, mMrgIf.Mrg_I2S_SampleRate << 20, 0xF00000); // set Mrg_I2S Samping Rate
                mAfeReg->SetAfeReg(AFE_MRGIF_CON, 1 << 16, 1 << 16); // set Mrg_I2S enable
            }
        }
        else {
            // turn on merge Interface from off state
            mMrgIf.Mrg_I2S_SampleRate = I2SSampleRateTransform(sampleRate);
            mAfeReg->SetAfeReg(AFE_MRGIF_CON, mMrgIf.Mrg_I2S_SampleRate << 20, 0xF00000); // set Mrg_I2S Samping Rate
            mAfeReg->SetAfeReg(AFE_MRGIF_CON, 1 << 16, 1 << 16);                        // set Mrg_I2S enable
            mAfeReg->SetAfeReg(AFE_MRGIF_CON, 1, 0x1);                              // Turn on Merge Interface
        }
        mMrgIf.MrgIf_En = true;
        mMrgIf.Mergeif_I2S_Enable = true;
    }
    else {
        // To disable MrgI2S
        if (mMrgIf.MrgIf_En == true) {
            mAfeReg->SetAfeReg(AFE_MRGIF_CON, 0, 1 << 16); // Turn off I2S
            if (mDaiBt.mDAIBT_ON == false) {
                // DAIBT also not using, then it's OK to disable Merge Interface
                usleep(30);//delay 30us
                mAfeReg->SetAfeReg(AFE_MRGIF_CON, 0, 0x1);    // Turn off Merge Interface
                mMrgIf.MrgIf_En = false;
            }
        }
        mMrgIf.Mergeif_I2S_Enable = false;
    }
#endif
    return NO_ERROR;
}

status_t AudioDigitalControl::ResetFmChipMrgIf(void)
{
    ALOGD("+%s(), Reset MergeIf HW of FM Chip\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    ::ioctl(mFd, AUDDRV_RESET_FMCHIP_MERGEIF);
    ALOGD("-%s()\n", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioDigitalControl::SetDAIBBT(AudioDigitalDAIBT *DAIBT)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)&mDaiBt, (void *)DAIBT, sizeof(AudioDigitalDAIBT));
    uint32 Audio_BT_DAI = 0;
    Audio_BT_DAI |= (mDaiBt.mUSE_MRGIF_INPUT << 12);
    Audio_BT_DAI |= (mDaiBt.mDAI_BT_MODE << 9);
    Audio_BT_DAI |= (mDaiBt.mDAI_DEL << 8);
    Audio_BT_DAI |= (mDaiBt.mBT_LEN << 4);
    Audio_BT_DAI |= (mDaiBt.mDATA_RDY << 3);
    Audio_BT_DAI |= (mDaiBt.mBT_SYNC << 2);
    Audio_BT_DAI |= (mDaiBt.mBT_ON << 1);
    mAfeReg->SetAfeReg(AFE_DAIBT_CON0, Audio_BT_DAI, AFE_MASK_ALL);
    return NO_ERROR;
}

status_t AudioDigitalControl::GetDAIBTOut(AudioDigitalDAIBT *DAIBT)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)DAIBT, (void *)&mDaiBt, sizeof(AudioDigitalDAIBT));
    return NO_ERROR;
}

status_t AudioDigitalControl::SetDAIBTEnable(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);

    if (bEnable == true) {
		#if defined(MTK_MERGE_INTERFACE_SUPPORT)
        if (mMrgIf.MrgIf_En == false) {
            //::ioctl(mFd, AUDDRV_SET_MRGIF_GPIO, 0);   // set merge intf GPIO221~GPIO224 driving

            ResetFmChipMrgIf(); // send reset sequence to 6628 mergeinterface by GPIO
			usleep(100); //delay between GPIO reset and merge interface enable

			#if defined(FM_ANALOG_INPUT)
			ALOGD("MrgIf_En=0, SetDAIBTEnable AUDDRV_SET_BT_PCM_GPIO");
			::ioctl(mFd, AUDDRV_SET_BT_PCM_GPIO);				//PCM_ON,  Analog FM
			#else
			ALOGD("MrgIf_En=0, SetDAIBTEnable AUDDRV_SET_BT_FM_GPIO");
			::ioctl(mFd, AUDDRV_SET_BT_FM_GPIO);				//PCM_ON, Digital FM
			#endif

            mAfeReg->SetAfeReg(AFE_MRGIF_CON, 0x900000, 0xf00000); // set merge intf I2S mode to 44.1k(programming guide)
            //Should turn on merge interface first then turn on DAI
            mAfeReg->SetAfeReg(AFE_MRGIF_CON, 1, 0x1);

            mMrgIf.MrgIf_En = true;
        }
        else {
            // if merge interface already turn on, take care of it if DAIBT sampleRate is different(8K->16K or 16K->8K)
			#if defined(FM_ANALOG_INPUT)
				ALOGD("MrgIf_En=1, SetDAIBTEnable AUDDRV_SET_BT_PCM_GPIO");
				::ioctl(mFd, AUDDRV_SET_BT_PCM_GPIO);				//PCM_ON,  Analog FM
			#else
				ALOGD("MrgIf_En=1, SetDAIBTEnable AUDDRV_SET_BT_FM_GPIO");
				::ioctl(mFd, AUDDRV_SET_BT_FM_GPIO);				//PCM_ON, Digital FM
			#endif
        }
		#else
			#if defined(FM_ANALOG_INPUT)
			ALOGD("Non-MrgIf, SetDAIBTEnable AUDDRV_SET_BT_PCM_GPIO");
			::ioctl(mFd, AUDDRV_SET_BT_PCM_GPIO);				//PCM_ON,  Analog FM
			#else
			ALOGD("Non-MrgIf, SetDAIBTEnable AUDDRV_SET_BT_FM_GPIO");
			::ioctl(mFd, AUDDRV_SET_BT_FM_GPIO);				//PCM_ON, Digital FM
			#endif
		#endif
    }

    mDaiBt.mDAIBT_ON = bEnable;
    mAfeReg->SetAfeReg(AFE_DAIBT_CON0, bEnable, 0x1);
    mAudioDigitalBlock[AudioDigitalType::DAI_BT] = bEnable;

    if (bEnable == false) {
		#if defined(MTK_MERGE_INTERFACE_SUPPORT)
        // When try to disable DAIBT, should turn off merge interface after turn off DAI
        if (mMrgIf.MrgIf_En == true) {
            if (mMrgIf.Mergeif_I2S_Enable == false) {
                // Turn off merge interface if I2S is not used
                usleep(50); //must delay at least 1/32000 = 31.25us to avoid mrgif clk is shutdown before the sync sequence(sent after AFE_DAIBT_CON0 bit0 is OFF) is complete
                ALOGD("SetDAIBTEnable disable mrgif clk after sync sequence done");
                mAfeReg->SetAfeReg(AFE_MRGIF_CON, 0, 0x1);
                mMrgIf.MrgIf_En = false;
            }
        }
		#endif

		#if defined(FM_ANALOG_INPUT)
			ALOGD("SetDAIBTEnable AUDDRV_RESET_BT_FM_GPIO");
			::ioctl(mFd, AUDDRV_RESET_BT_FM_GPIO);				//PCM_OFF,  Analog FM
		#else
			ALOGD("SetDAIBTEnable AUDDRV_SET_FM_I2S_GPIO");
			::ioctl(mFd, AUDDRV_SET_FM_I2S_GPIO);				//PCM_OFF, Digital FM
		#endif
    }

    return NO_ERROR;
}

status_t AudioDigitalControl::SetHwDigitalGainMode(AudioDigitalType::Hw_Digital_Gain GainType, AudioMEMIFAttribute::SAMPLINGRATE SampleRate,  uint32 SamplePerStep)
{
    ALOGD("+%s(), GainType = %d, SampleRate = %d, SamplePerStep= %d\n", __FUNCTION__, GainType, SampleRate, SamplePerStep);
    uint32 value = 0;
    value = SamplePerStep << 8 | SampleRate << 4;
    switch (GainType) {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CON0, value, 0xfff0);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CON0, value, 0xfff0);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetHwDigitalGainEnable(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable)
{
    ALOGD("+%s(), GainType = %d, Enable = %d\n", __FUNCTION__, GainType, Enable);
    switch (GainType) {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CON0, Enable, 0x1);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CON0, Enable, 0x1);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

uint32 AudioDigitalControl::DlPolicyByDevice(uint32_t Device)
{
    ALOGV("+%s(), Device = %d\n", __FUNCTION__, Device);
    if (Device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO ||
        Device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
        Device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT) {
        return AudioDigitalType::DAI_BT;
    }
    else if (Device & AUDIO_DEVICE_OUT_AUX_DIGITAL ||
             Device & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET ||
             Device & AUDIO_DEVICE_OUT_FM_TX) {
        return AudioDigitalType::I2S_INOUT_2;
    }
    else {
        return AudioDigitalType::I2S_OUT_DAC;
    }
}


bool AudioDigitalControl::CheckDlDigitalChange(uint32_t PreDevice, uint32_t NewDevice)
{
    uint32 OuputPreDevice = 0 , OutPutNewDevice = 0;
    ALOGD("+%s(), PreDevice = 0x%x, NewDevice = %d\n", __FUNCTION__, PreDevice, NewDevice);
    OuputPreDevice = DlPolicyByDevice(PreDevice);
    OutPutNewDevice = DlPolicyByDevice(NewDevice);
    if (OuputPreDevice != OutPutNewDevice) {
        return true;
    }
    else {
        return false;
    }
}

status_t AudioDigitalControl::SetHwDigitalGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType)
{
    ALOGD("+%s(), Gain = 0x%x, gain type = %d\n", __FUNCTION__, Gain, GainType);
    switch (GainType) {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CON1, Gain, 0xffffffff);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CON1, Gain, 0xffffffff);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetModemPcmConfig(modem_index_t modem_index, AudioDigitalPCM *p_modem_pcm_attribute)
{
    ALOGD("+%s(), modem_index = %d\n", __FUNCTION__, modem_index);
    if (modem_index == MODEM_1) { // MODEM_1 use PCM2_INTF_CON (0x53C) !!!
        memcpy((void *)&mModemPcm1, (void *)p_modem_pcm_attribute, sizeof(AudioDigitalPCM));

        uint32 reg_pcm2_intf_con = 0;
        reg_pcm2_intf_con |= (mModemPcm1.mTxLchRepeatSel     & 0x1) << 13;
        reg_pcm2_intf_con |= (mModemPcm1.mVbt16kModeSel      & 0x1) << 12;
        reg_pcm2_intf_con |= (mModemPcm1.mSingelMicSel       & 0x1) << 7;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmWordLength      & 0x1) << 4;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmModeWidebandSel & 0x1) << 3;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmFormat          & 0x3) << 1;

        ALOGD("%s(), PCM2_INTF_CON(0x%x) = 0x%x\n", __FUNCTION__, PCM2_INTF_CON, reg_pcm2_intf_con);
        mAfeReg->SetAfeReg(PCM2_INTF_CON, reg_pcm2_intf_con, AFE_MASK_ALL);
    }
    else if (modem_index == MODEM_2) { // MODEM_2 use PCM_INTF_CON1 (0x530) !!!
        memcpy((void *)&mModemPcm2, (void *)p_modem_pcm_attribute, sizeof(AudioDigitalPCM));

        // config ASRC for modem 2
        if (mModemPcm2.mPcmModeWidebandSel == AudioDigitalPCM::PCM_MODE_8K) {
            mAfeReg->SetAfeReg(AFE_ASRC_CON1, 0x00001964, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON2, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON3, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON4, 0x00001964, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON7, 0x00000CB2, AFE_MASK_ALL);
        }
        else if (mModemPcm2.mPcmModeWidebandSel == AudioDigitalPCM::PCM_MODE_16K) {
            mAfeReg->SetAfeReg(AFE_ASRC_CON1, 0x00000cb2, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON2, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON3, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON4, 0x00000cb2, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON7, 0x00000659, AFE_MASK_ALL);
        }

        // config modem 2
        uint32 reg_pcm_intf_con1 = 0;
        reg_pcm_intf_con1 |= (mModemPcm2.mTxLchRepeatSel       & 0x01) << 19;
        reg_pcm_intf_con1 |= (mModemPcm2.mVbt16kModeSel        & 0x01) << 18;
        reg_pcm_intf_con1 |= (mModemPcm2.mExtModemSel          & 0x01) << 17;
        reg_pcm_intf_con1 |= (mModemPcm2.mExtendBckSyncLength  & 0x1F) << 9;
        reg_pcm_intf_con1 |= (mModemPcm2.mExtendBckSyncTypeSel & 0x01) << 8;
        reg_pcm_intf_con1 |= (mModemPcm2.mSingelMicSel         & 0x01) << 7;
        reg_pcm_intf_con1 |= (mModemPcm2.mAsyncFifoSel         & 0x01) << 6;
        reg_pcm_intf_con1 |= (mModemPcm2.mSlaveModeSel         & 0x01) << 5;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmWordLength        & 0x01) << 4;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmModeWidebandSel   & 0x01) << 3;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmFormat            & 0x03) << 1;

        ALOGD("%s(), PCM_INTF_CON1(0x%x) = 0x%x", __FUNCTION__, PCM_INTF_CON1, reg_pcm_intf_con1);
        mAfeReg->SetAfeReg(PCM_INTF_CON1, reg_pcm_intf_con1, AFE_MASK_ALL);
    }
    else {
        ALOGE("%s(), no such modem_index: %d!!", __FUNCTION__, modem_index);
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetModemPcmEnable(modem_index_t modem_index, bool modem_pcm_on)
{
    ALOGD("+%s(), modem_index = %d, modem_pcm_on = %d\n", __FUNCTION__, modem_index, modem_pcm_on);

    if (modem_index == MODEM_1) { // MODEM_1 use PCM2_INTF_CON (0x53C) !!!
        mModemPcm1.mModemPcmOn = modem_pcm_on;
        mAfeReg->SetAfeReg(PCM2_INTF_CON, modem_pcm_on, 0x1);
        mAudioDigitalBlock[AudioDigitalType::MODEM_PCM_1_O] = modem_pcm_on;
    }
    else if (modem_index == MODEM_2) { // MODEM_2 use PCM_INTF_CON1 (0x530) !!!
        mModemPcm2.mModemPcmOn = modem_pcm_on;
        if (modem_pcm_on == true) { // turn on ASRC before Modem PCM on
            mAfeReg->SetAfeReg(AFE_ASRC_CON6, 0x0001183F, AFE_MASK_ALL); // pre ver. 0x0001188F
            mAfeReg->SetAfeReg(AFE_ASRC_CON0, 0x06003031, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(PCM_INTF_CON1, 0x1, 0x1);
        }
        else if (modem_pcm_on == false) { // turn off ASRC after Modem PCM off
            mAfeReg->SetAfeReg(PCM_INTF_CON1, 0x0, 0x1);
            mAfeReg->SetAfeReg(AFE_ASRC_CON6, 0x00000000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON0, 0x0, 0x1);
        }
        mAudioDigitalBlock[AudioDigitalType::MODEM_PCM_2_O] = modem_pcm_on;
    }
    else {
        ALOGW("%s(), no such modem_index: %d!!", __FUNCTION__, modem_index);
        return INVALID_OPERATION;
    }

    return NO_ERROR;
}

static const uint16_t kSideToneCoefficientTable16k[] = {
    0x049C, 0x09E8, 0x09E0, 0x089C,
    0xFF54, 0xF488, 0xEAFC, 0xEBAC,
    0xfA40, 0x17AC, 0x3D1C, 0x6028,
    0x7538
};

static const uint16_t kSideToneCoefficientTable32k[] = {
    0xff58, 0x0063, 0x0086, 0x00bf,
    0x0100, 0x013d, 0x0169, 0x0178,
    0x0160, 0x011c, 0x00aa, 0x0011,
    0xff5d, 0xfea1, 0xfdf6, 0xfd75,
    0xfd39, 0xfd5a, 0xfde8, 0xfeea,
    0x005f, 0x0237, 0x0458, 0x069f,
    0x08e2, 0x0af7, 0x0cb2, 0x0df0,
    0x0e96
};

status_t AudioDigitalControl::EnableSideToneFilter(bool stf_on)
{
    ALOGD("+%s(), stf_on = %d", __FUNCTION__, stf_on);

    if (stf_on == mSideToneFilterOn) {
        ALOGD("-%s(), stf_on(%d) == mSideToneFilterOn(%d), return", __FUNCTION__, stf_on, mSideToneFilterOn);
        return NO_ERROR;
    }

    mSideToneFilterOn = stf_on;


    // MD max support 16K sampling rate
    const uint8_t kSideToneHalfTapNum = sizeof(kSideToneCoefficientTable16k) / sizeof(uint16_t);

    if (stf_on == false) {
        // bypass STF result & disable
        const bool bypass_stf_on = true;
        uint32_t reg_value = (bypass_stf_on << 31) | (stf_on << 8);
        mAfeReg->SetAfeReg(AFE_SIDETONE_CON1, reg_value, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_CON1[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_CON1, reg_value);

        // set side tone gain = 0
        mAfeReg->SetAfeReg(AFE_SIDETONE_GAIN, 0, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_GAIN[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_GAIN, 0);
    }
    else {
        // set side tone gain
        mAfeReg->SetAfeReg(AFE_SIDETONE_GAIN, 0x7FFF, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_GAIN[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_GAIN, 0x7FFF);

        // using STF result & enable & set half tap num
        const bool bypass_stf_on = false;
        uint32_t write_reg_value = (bypass_stf_on << 31) | (stf_on << 8) | kSideToneHalfTapNum;
        mAfeReg->SetAfeReg(AFE_SIDETONE_CON1, write_reg_value, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_CON1[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_CON1, write_reg_value);

        // set side tone coefficient
        const bool enable_read_write = true; // enable read/write side tone coefficient
        const bool read_write_sel = true;    // for write case
        const bool sel_ch2 = false;          // using uplink ch1 as STF input

        uint32_t   read_reg_value = mAfeReg->GetAfeReg(AFE_SIDETONE_CON0);
        for (size_t coef_addr = 0; coef_addr < kSideToneHalfTapNum; coef_addr++) {
            bool old_write_ready = (read_reg_value >> 29) & 0x1;
            write_reg_value = enable_read_write << 25 |
                              read_write_sel    << 24 |
                              sel_ch2           << 23 |
                              coef_addr         << 16 |
                              kSideToneCoefficientTable16k[coef_addr];
            mAfeReg->SetAfeReg(AFE_SIDETONE_CON0, write_reg_value, 0x39FFFFF);
            ALOGD("%s(), AFE_SIDETONE_CON0[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_CON0, write_reg_value);

            // wait until flag write_ready changed (means write done)
            for(int try_cnt = 0; try_cnt < 10; try_cnt++) { // max try 10 times
                usleep(3);
                read_reg_value = mAfeReg->GetAfeReg(AFE_SIDETONE_CON0);
                bool new_write_ready = (read_reg_value >> 29) & 0x1;
                if (new_write_ready != old_write_ready) { // flip => ok
                    break;
                }
                else {
                    ::ioctl(mFd, AUDDRV_LOG_PRINT, 0);
                    ALOGW("%s(), AFE_SIDETONE_CON0[0x%x] = 0x%x, old_write_ready = %d, new_write_ready = %d",
                          __FUNCTION__, AFE_SIDETONE_CON0, read_reg_value, old_write_ready, new_write_ready);
#if 1
                    ASSERT(new_write_ready != old_write_ready);
                    return UNKNOWN_ERROR;
#endif
                }
            }
        }
    }

    ALOGD("-%s(), stf_on = %d", __FUNCTION__, stf_on);
    return NO_ERROR;
}

status_t AudioDigitalControl::EnableSideToneHw(uint32 connection , bool direction  , bool  Enable)
{
    ALOGD("+%s(), connection = %d, direction = %d, Enable= %d\n", __FUNCTION__, connection, direction, Enable);
    if (Enable && direction) {
        switch (connection) {
            case AudioDigitalType::I00:
            case AudioDigitalType::I01:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x04662662, 0xffffffff);
                break;
            case AudioDigitalType::I02:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x14662662, 0xffffffff);
                break;
            case AudioDigitalType::I03:
            case AudioDigitalType::I04:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x24662662, 0xffffffff);
                break;
            case AudioDigitalType::I05:
            case AudioDigitalType::I06:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x34662662, 0xffffffff);
                break;
            case AudioDigitalType::I07:
            case AudioDigitalType::I08:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x44662662, 0xffffffff);
                break;
            case AudioDigitalType::I09:
            case AudioDigitalType::I10:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x54662662, 0xffffffff);
                break;
            case AudioDigitalType::I11:
            case AudioDigitalType::I12:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x64662662, 0xffffffff);
                break;
            default:
                ALOGW("EnableSideToneHw fail with conenction connection");
                break;
        }
    }
    else if (Enable) {
        switch (connection) {
            case AudioDigitalType::O00:
            case AudioDigitalType::O01:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x746c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O02:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x846c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O03:
            case AudioDigitalType::O04:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x946c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O05:
            case AudioDigitalType::O06:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xa46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O07:
            case AudioDigitalType::O08:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xb46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O09:
            case AudioDigitalType::O10:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xc46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O11:
            case AudioDigitalType::O12:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xd46c26c2, 0xffffffff);
                break;
            default:
                ALOGW("EnableSideToneHw fail with conenction connection");
        }
    }
    else {
        mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x0 , 0xffffffff);
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetFmChip(bool enable)
{
    int fd_FM = -1, ret_FM = 0;
    struct fm_i2s_setting fmSetting;        //defined in source\kernel\include\linux\fm.h

    ALOGD("+%s(), enable = %d\n", __FUNCTION__, enable);
    fmSetting.onoff  = FM_I2S_ON;            //intial setting, actually it's fake argument.
    fmSetting.mode   = FM_I2S_SLAVE;
    fmSetting.sample = FM_I2S_44K;
    fd_FM = open(FM_DEVICE_NAME, O_RDWR);

    if (fd_FM < 0) {
        ALOGD("Open 'dev/fm' Fail !fd_FM(%d)", fd_FM);
    }
    else {
        ret_FM = ioctl(fd_FM, FM_IOCTL_I2S_SETTING, &fmSetting);
        if (ret_FM) {
            ALOGD("set ioctl FM_IOCTL_I2S_SETTING Fail !ret_FM(%d)", ret_FM);
        }
        close(fd_FM);
    }

    if (enable == true) {
        ResetFmChipMrgIf();
    }
    if (mDaiBt.mDAIBT_ON ==false){
        ALOGD("%s(), AUDDRV_SET_FM_I2S_GPIO\n", __FUNCTION__);
        ::ioctl(mFd, AUDDRV_SET_FM_I2S_GPIO);          //DAIBT OFF, Digital FM
    }

    mI2SConnectStatus = enable;
    ALOGD("-%s()\n", __FUNCTION__);
    return NO_ERROR;
}

bool AudioDigitalControl::GetI2SConnectStatus(void)
{
    ALOGD("+%s(), mI2SConnectStatus = %d\n", __FUNCTION__, mI2SConnectStatus);
    return mI2SConnectStatus;
}

status_t AudioDigitalControl::SetFmDigitalStatus(bool bEnable)
{
    mFmDigitalStatus = bEnable;
    ALOGD("%s(), mFmDigitalStatus = %d\n", __FUNCTION__, mFmDigitalStatus);
    return NO_ERROR;
}

bool AudioDigitalControl::GetFmDigitalStatus(void)
{
    ALOGD("%s(), mFmDigitalStatus = %d\n", __FUNCTION__, mFmDigitalStatus);
    return mFmDigitalStatus;
}

}//namespace android
