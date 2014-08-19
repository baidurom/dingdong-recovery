#include "AudioType.h"
#include "AudioResourceManager.h"
#include "AudioVolumeFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"
#include "AudioMTKHardware.h"

#include "SpeechPhoneCallController.h"

#define LOG_TAG "AudioResourceManager"
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

#define LOCK_TRY_INTERVAL (30) // in ms

namespace android
{

AudioResourceManager *AudioResourceManager::UniqueAudioResourceInstance = 0;

AudioResourceManager *AudioResourceManager::getInstance()
{
    if (UniqueAudioResourceInstance == 0) {
        printf("+UniqueAudioResourceInstance \n");
        UniqueAudioResourceInstance = new AudioResourceManager();
        printf("-UniqueAudioResourceInstance \n");
    }
    return UniqueAudioResourceInstance;
}

AudioResourceManager::AudioResourceManager()
{
    printf("AudioResourceManager construct \n");

    // init clock counter to 0
    for (int i = 0; i < AudioResourceManagerInterface::CLOCK_TYPE_MAX ; i++) {
        ClockCounter[i] = 0;;
    }
    mFd = 0;
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd == 0) {
        ALOGE("AudioResourceManager open mFd fail");
    }
    EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

    // create volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();
    mAudioVolumeInstance->initCheck();
    // create digital control instnace
    mAudioDigitalInstance  = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioDigitalInstance->InitCheck();
    // create digital control instnace
    mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();
    mAudioAnalogInstance->InitCheck();

    mDlOutputDevice = 0;
    mUlInputDevice = (AUDIO_DEVICE_IN_BUILTIN_MIC/*&~AUDIO_DEVICE_BIT_IN*/);
    mAudioHardware = NULL;
    mAudioMode = AUDIO_MODE_NORMAL;

    mMicDefaultsetting  = USE_REFMIC_IN_LOUDSPK;
    mMicInverseSetting = false;
    SetMicInvserse(mMicInverseSetting);

    EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
}

status_t AudioResourceManager::setDlOutputDevice(uint32 new_device)
{
    ALOGD("%s(), new_device = 0x%x", __FUNCTION__, new_device);
    if (new_device == 0) return NO_ERROR;

    ASSERT(new_device & AUDIO_DEVICE_OUT_ALL);

    mDlOutputDevice = new_device;

    if (IsModeIncall() == false) { // no need to set callback when in phone call mode
        mAudioHardware->DoDeviceChangeCallback(mDlOutputDevice);
    }

    // need to set corresponding input device by output device when in phone call mode
    if (IsModeIncall() == true) {
        switch (mDlOutputDevice) {
            case AUDIO_DEVICE_OUT_WIRED_HEADSET: {
                setUlInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                break;
            }
            case AUDIO_DEVICE_OUT_EARPIECE:
            case AUDIO_DEVICE_OUT_SPEAKER:
            case AUDIO_DEVICE_OUT_WIRED_HEADPHONE: {
                setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
                break;
            }
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT: {
                setUlInputDevice(AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET);
                break;
            }
        }
    }

    return NO_ERROR;
}

status_t AudioResourceManager::setUlInputDevice(uint32 new_device)
{
    ALOGD("%s(), new_device = 0x%x", __FUNCTION__, new_device);
    if (new_device == 0) return NO_ERROR;

    ASSERT(new_device & AUDIO_DEVICE_IN_ALL);

    mUlInputDevice = new_device;

#if defined(MTK_DUAL_MIC_SUPPORT)
    // swap original ADC I2S L/R ch
    if (mUlInputDevice == AUDIO_DEVICE_IN_BACK_MIC)
    {
        SetMicInvserse(true);
    }
    else
    { // recover to default setting
        SetMicInvserse(false);
    }
#endif

    return NO_ERROR;
}

status_t AudioResourceManager::setUlInputSource(uint32 Source)
{
    ALOGD("setUlInputSource Source = 0x%x", Source);
    mUlInputSource = Source;
    return NO_ERROR;
}

status_t AudioResourceManager::SetAudioMode(audio_mode_t Mode)
{
    ALOGD("SetAudioMode Mode = 0x%d", Mode);
    mAudioMode = Mode;
    mAudioAnalogInstance->setmode(Mode);
    return NO_ERROR;
}

audio_mode_t AudioResourceManager::GetAudioMode()
{
    //ALOGD("GetAudioMode");
    return mAudioMode;
}

uint32 AudioResourceManager::getDlOutputDevice()
{
    return mDlOutputDevice;
}

uint32 AudioResourceManager::getUlInputDevice()
{
    return mUlInputDevice;
}

/**
 * a function for tell AudioResourceManager to aquire lock
 */
status_t AudioResourceManager::EnableAudioLock(int AudioLockType, int mTimeout)
{
    //ALOGD("EnableAudioLock AudioLockType = %d, mTimeout = %d", AudioLockType, mTimeout);

    int ret = 0 ;

    if (mTimeout == 0) {
        ret = mAudioLock[AudioLockType].lock();
    }
    else {
        ret = mAudioLock[AudioLockType].lock_timeout(mTimeout);
    }

    if (ret != 0) {
        ALOGE("EnableAudioLock fail, AudioLockType = %d, mTimeout = %d, ret = %d", AudioLockType, mTimeout, ret);
    }

    return ret;
}

/**
 * a function for tell AudioResourceManager to release lock
 */
status_t AudioResourceManager::DisableAudioLock(int AudioLockType)
{
    //ALOGD("DisableAudioLock, AudioLockType = %d", AudioLockType);
    mAudioLock[AudioLockType].unlock();
    return NO_ERROR;
}


status_t AudioResourceManager::EnableAudioClock(int AudioLockType , bool bEnable)
{
    ALOGD("EnableAudioClock AudioLockType = %d bEnable = %d", AudioLockType, bEnable);
    switch (AudioLockType) {
        case AudioResourceManagerInterface::CLOCK_AUD_CORE:
            ::ioctl(mFd, AUD_SET_26MCLOCK, bEnable);
            break;
        case AudioResourceManagerInterface::CLOCK_AUD_AFE:
            ::ioctl(mFd, AUD_SET_CLOCK, bEnable);
            break;
        case AudioResourceManagerInterface::CLOCK_AUD_ADC:
            ::ioctl(mFd, AUD_SET_ADC_CLOCK, bEnable);
            break;
        case AudioResourceManagerInterface::CLOCK_AUD_I2S:
            ::ioctl(mFd, AUD_SET_I2S_CLOCK, bEnable);
            break;
        case AudioResourceManagerInterface::CLOCK_AUD_ANA:
            ::ioctl(mFd, AUD_SET_ANA_CLOCK, bEnable);
            break;
        case AudioResourceManagerInterface::CLOCK_AUD_LINEIN:
            ::ioctl(mFd, AUD_SET_LINE_IN_CLOCK, bEnable);
            break;
        case AudioResourceManagerInterface::CLOCK_AUD_HDMI:
            ::ioctl(mFd, AUD_SET_HDMI_CLOCK, bEnable);
            break;
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioResourceManager::TurnonAudioDevice(unsigned int mDlOutputDevice)
{
    uint32 DeviceCount = PopCount(mDlOutputDevice);
    ALOGD("TurnonAudioDevice = 0x%x DeviceCount = %d", mDlOutputDevice, DeviceCount);
    if (DeviceCount == 1) {
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_EARPIECE) {
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if ((mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            if (mAudioAnalogInstance->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINR) ||
               mAudioAnalogInstance->GetAnalogState(AudioAnalogType::DEVICE_IN_LINEINL)) {
                ALOGD("SetHeadsetPinmux as MUX_LINEIN_AUDIO_MONO ");
                mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::MUX_LINEIN_AUDIO_MONO);
                mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, AudioAnalogType::MUX_LINEIN_AUDIO_MONO);
            }
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_FM_TX) {
            // do nothing
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET) {
            // fix me , may not use headset
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET) {
            // do nothing
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
            EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_HDMI,true);
            //todo :: if htere is analog block need to turn on
        }
    }
    else { // open for dual mode
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_SPEAKER)
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }

    // do set analog gain control
    if (IsModeIncall() == false) {
        mAudioVolumeInstance->setMasterVolume(mAudioVolumeInstance->getMasterVolume(), mAudioMode, mDlOutputDevice);
    }
    else {
        mAudioVolumeInstance->ApplyMicGain(GetIncallMicDevice(mDlOutputDevice), mAudioMode); // set incall mic gain
        mAudioVolumeInstance->setVoiceVolume(mAudioVolumeInstance->getVoiceVolume(), mAudioMode, mDlOutputDevice);// set DL volume
    }
    return NO_ERROR;
}

status_t AudioResourceManager::TurnonAudioDeviceIncall(unsigned int mDlOutputDevice)
{
    ALOGD("TurnonAudioDeviceIncall mDlOutputDevice = 0x%x", mDlOutputDevice);

    SetMicInvserse(mMicInverseSetting);

    if (mDlOutputDevice & AUDIO_DEVICE_OUT_EARPIECE) {
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_EARPIECER , AudioAnalogType::MUX_VOICE);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_EARPIECER , AudioAnalogType::MUX_VOICE);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    if (mDlOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR , AudioAnalogType::MUX_AUDIO);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL , AudioAnalogType::MUX_AUDIO);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    if ((mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR , AudioAnalogType::MUX_AUDIO);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL , AudioAnalogType::MUX_AUDIO);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    if (mDlOutputDevice & AUDIO_DEVICE_OUT_FM_TX) {
        // do nothing
    }
    if (mDlOutputDevice & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET) {
        // fix me , may not use HeadSetDetect
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR , AudioAnalogType::MUX_AUDIO);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL , AudioAnalogType::MUX_AUDIO);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    if (mDlOutputDevice & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)   {
        // do nothing
    }
    if (mDlOutputDevice & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
        //todo :: if htere is analog block need to turn onAsBinde
    }
    // do set analog gain control
    if (IsModeIncall() == false) {
        mAudioVolumeInstance->setMasterVolume(mAudioVolumeInstance->getMasterVolume(), mAudioMode, mDlOutputDevice);
    }
    else {
        // check TTY status to avoid override mic gain
        if (SpeechPhoneCallController::GetInstance()->CheckTtyNeedOn() == false){
            mAudioVolumeInstance->ApplyMicGain(GetIncallMicDevice(mDlOutputDevice), mAudioMode); // set incall mic gain
        }

        mAudioVolumeInstance->setVoiceVolume(mAudioVolumeInstance->getVoiceVolume(), mAudioMode, mDlOutputDevice);// set DL volume
    }
    return NO_ERROR;
}

status_t AudioResourceManager::TurnoffAudioDevice(unsigned int mDlOutputDevice)
{
    uint32 DeviceCount = PopCount(mDlOutputDevice);
    ALOGD("TurnoffAudioDevice = 0x%x mDlOutputDevice = %d", mDlOutputDevice, mDlOutputDevice);
    if (DeviceCount == 1)
    {
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_EARPIECE) {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_FM_TX) {
            // do nothing
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET) {
            // fix me , may not use headset
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)
        {
            // do nothing
        }
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_AUX_DIGITAL)
        {
            EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_HDMI,false);
            //todo :: if htere is analog block need to turn on
        }
    }
    else
    { // open for dual mode
        if (mDlOutputDevice & AUDIO_DEVICE_OUT_SPEAKER)
        {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
    }
    return NO_ERROR;
}

status_t AudioResourceManager::StartOutputDevice()
{
    ALOGD("%s(), mDlOutputDevice = 0x%x\n", __FUNCTION__, mDlOutputDevice);
    switch (mAudioMode) {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE: {
            TurnonAudioDevice(mDlOutputDevice);
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2: {
            TurnonAudioDeviceIncall(mDlOutputDevice);
            break;
        }

        case AUDIO_MODE_IN_COMMUNICATION: {
            TurnonAudioDevice(mDlOutputDevice);
            break;
        }
    }
    return NO_ERROR;
}

status_t AudioResourceManager::StopOutputDevice()
{
    ALOGD("%s(), mDlOutputDevice = 0x%x\n", __FUNCTION__, mDlOutputDevice);
    switch (mAudioMode) {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE: {
            TurnoffAudioDevice(mDlOutputDevice);
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2: {
            TurnoffAudioDevice(mDlOutputDevice);
            break;
        }
        case AUDIO_MODE_IN_COMMUNICATION: {
            TurnoffAudioDevice(mDlOutputDevice);
            break;
        }
    }
    return NO_ERROR;
}

status_t AudioResourceManager::SelectOutputDevice(uint32_t new_device)
{
    // TODO: print device name but not just a enum to increase log readibiltiy
    uint32_t pre_device ;
    ALOGD("%s(), pre_device(mDlOutputDevice) = 0x%x, new_device = 0x%x\n", __FUNCTION__, mDlOutputDevice, new_device);

    if (new_device == mDlOutputDevice) return NO_ERROR;
    pre_device = mDlOutputDevice;

    // close
    StopOutputDevice();
    // open
    mDlOutputDevice = new_device;
    if(mAudioHardware != NULL){
#ifdef FM_DIGITAL_IN_SUPPORT
        mAudioHardware->SetFmDigitalFmHw(pre_device, new_device);
#endif
        mAudioHardware->DoDeviceChangeCallback(new_device);
    }
    StartOutputDevice();

    return NO_ERROR;
}

uint32_t AudioResourceManager::PopCount(uint32_t u)
{
    u = ((u & 0x55555555) + ((u >> 1) & 0x55555555));
    u = ((u & 0x33333333) + ((u >> 2) & 0x33333333));
    u = ((u & 0x0f0f0f0f) + ((u >> 4) & 0x0f0f0f0f));
    u = ((u & 0x00ff00ff) + ((u >> 8) & 0x00ff00ff));
    u = (u & 0x0000ffff) + (u >> 16);
    return u;
}

uint32_t AudioResourceManager::GetIncallMicDevice(uint32 device)
{
    ALOGD("GetIncallMicDevice device = 0x%x",device);
    uint32_t mictype = Num_Mic_Gain;
    if (device & AUDIO_DEVICE_OUT_SPEAKER) {
        mictype = Handfree_Mic;
    }
    else if ((device & (AUDIO_DEVICE_IN_WIRED_HEADSET)) || (device & AUDIO_DEVICE_OUT_WIRED_HEADSET)) {
        mictype = Headset_Mic;
    }
    else if (device & AUDIO_DEVICE_OUT_EARPIECE ) {
        mictype = Normal_Mic;
    }
    else
    {
        mictype = Handfree_Mic;
        ALOGD("GetIncallMicDevice with device = 0x%x", device);
    }
    return mictype;
}

status_t AudioResourceManager::SelectInPutMicEnable(bool bEnable)
{
    ALOGD("SelectInPutMicEnable bEnable  = %d", bEnable);
    if (IsWiredHeadsetOn() && (mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET)) {
        if (bEnable) {
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_PREAMP_L);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_PREAMP_R);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC2);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC2);
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        else {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
    }
    else {
        if (bEnable) {
#if defined(MTK_DIGITAL_MIC_SUPPORT)
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#else
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_MIC2);


#if defined(MTK_DUAL_MIC_SUPPORT)  // base on dual or single mic select mic input source.
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC3);
#else
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC1);
#endif
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);


#endif
        }
        else {
#if defined(MTK_DIGITAL_MIC_SUPPORT)
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#else
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#endif
        }
    }
    return NO_ERROR;
}

status_t AudioResourceManager::SetInputDeviceGain()
{
    ALOGD("SetInputDeviceGain mUlInputDevice =  0x%x mUlInputSource = 0x%x", mUlInputDevice, mUlInputSource);
    switch (mAudioMode) {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE: {
            if (mUlInputSource == AUDIO_SOURCE_VOICE_RECOGNITION) {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Voice_Rec_Mic_Headset , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Voice_Rec_Mic_Handset, mAudioMode);
            }
            else if (mUlInputSource == AUDIO_SOURCE_CAMCORDER) {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Idle_Video_Record_Headset , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Idle_Video_Record_Handset, mAudioMode);
            }
            else if (mUlInputSource == AUDIO_SOURCE_VOICE_UNLOCK) {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Voice_UnLock_Mic_Headset , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Voice_UnLock_Mic_Handset, mAudioMode);
            }
            else if (mUlInputSource == AUDIO_SOURCE_CUSTOMIZATION1) {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Customization1_Mic_Headset , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Customization1_Mic_Handset, mAudioMode);
            }
            else if (mUlInputSource == AUDIO_SOURCE_CUSTOMIZATION2) {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Customization2_Mic_Headset , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Customization2_Mic_Handset, mAudioMode);
            }
            else if (mUlInputSource == AUDIO_SOURCE_CUSTOMIZATION3) {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Customization3_Mic_Headset , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Customization3_Mic_Handset, mAudioMode);
            }
            else {
                if (mUlInputDevice == (AUDIO_DEVICE_IN_WIRED_HEADSET/*&~AUDIO_DEVICE_BIT_IN*/))
                    mAudioVolumeInstance->ApplyMicGain(Idle_Headset_Record , mAudioMode);
                else
                    mAudioVolumeInstance->ApplyMicGain(Idle_Normal_Record , mAudioMode);
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2: {
            if (mDlOutputDevice == AUDIO_DEVICE_OUT_EARPIECE) {
                mAudioVolumeInstance->ApplyMicGain(Normal_Mic , mAudioMode);
            }
            else if (mDlOutputDevice == AUDIO_DEVICE_OUT_SPEAKER) {
                mAudioVolumeInstance->ApplyMicGain(Headset_Mic , mAudioMode);
            }
            else {
                mAudioVolumeInstance->ApplyMicGain(Handfree_Mic , mAudioMode);
            }
            break;
        }
        case AUDIO_MODE_IN_COMMUNICATION: {
            if (mDlOutputDevice == AUDIO_DEVICE_OUT_EARPIECE) {
                mAudioVolumeInstance->ApplyMicGain(VOIP_Normal_Mic , mAudioMode);
            }
            else if (mDlOutputDevice == AUDIO_DEVICE_OUT_SPEAKER) {
                mAudioVolumeInstance->ApplyMicGain(VOIP_Handfree_Mic , mAudioMode);
            }
            else {
                mAudioVolumeInstance->ApplyMicGain(VOIP_Headset_Mic , mAudioMode);
            }
            break;
        }
    }
    return NO_ERROR;
}

status_t AudioResourceManager::StartInputDevice()
{
    ALOGD("+%s(), mUlInputDevice = 0x%x", __FUNCTION__, mUlInputDevice);
    if (mUlInputDevice  == AUDIO_DEVICE_IN_COMMUNICATION) {
        ALOGV("%s(), 00 mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_COMMUNICATION = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_COMMUNICATION);
        SelectInPutMicEnable(true);
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_AMBIENT) {
        ALOGV("%s(), 01 mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_AMBIENT = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_AMBIENT);
        SelectInPutMicEnable(true);
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_BUILTIN_MIC) {
        ALOGV("%s(), 02 mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_BUILTIN_MIC = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_BUILTIN_MIC);
        // here use built-in mic
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
    }
    else if (mUlInputDevice  == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        ALOGV("%s(), 03 mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET);
        // fix me::do nothing with analog
    }
    else if (mUlInputDevice  ==  AUDIO_DEVICE_IN_WIRED_HEADSET) {
        ALOGV("%s(), 04 mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_WIRED_HEADSET = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_WIRED_HEADSET);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_PREAMP_L);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_PREAMP_R);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC2);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC2);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    else if (mUlInputDevice ==(AUDIO_DEVICE_IN_AUX_DIGITAL)) {
        ALOGV("%s(), 05mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_AUX_DIGITAL = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_AUX_DIGITAL);
        // fix me::do nothing with analog
    }
    else if (mUlInputDevice == (AUDIO_DEVICE_IN_VOICE_CALL)) {
        ALOGV("%s(), 06mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_VOICE_CALL = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_VOICE_CALL);
        SelectInPutMicEnable(true);
    }
    else if (mUlInputDevice == (AUDIO_DEVICE_IN_BACK_MIC)) {
        ALOGV("%s(), 07mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_BACK_MIC = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_BACK_MIC);
        SelectInPutMicEnable(true);
    }
    else if (mUlInputDevice == (AUDIO_DEVICE_IN_FM)) {
        ALOGV("%s(), 08mUlInputDevice = 0x%x, AUDIO_DEVICE_IN_FM = 0x%x", __FUNCTION__, mUlInputDevice, AUDIO_DEVICE_IN_FM);
        // todo: make decision base on define
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER);
        mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    SetInputDeviceGain();
    ALOGD("-StartInputDevice mUlInputDevice = 0x%x", mUlInputDevice);

    return NO_ERROR;
}

status_t AudioResourceManager::StopInputDevice()
{
    ALOGD("%s(), mUlInputDevice = 0x%x", __FUNCTION__, mUlInputDevice);
    if (mUlInputDevice  == AUDIO_DEVICE_IN_COMMUNICATION) {
        SelectInPutMicEnable(false);
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_AMBIENT) {
        SelectInPutMicEnable(false);
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_BUILTIN_MIC) {
#if defined(MTK_DIGITAL_MIC_SUPPORT)
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#else
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
#endif
    }
    else if (mUlInputDevice  == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        // do nothing
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_WIRED_HEADSET)
    {
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_AUX_DIGITAL)
    {
        // do nothing
    }
    else if (mUlInputDevice == AUDIO_DEVICE_IN_VOICE_CALL)
    {
        SelectInPutMicEnable(false);
    }
    else if (mUlInputDevice  == AUDIO_DEVICE_IN_BACK_MIC)
    {
        SelectInPutMicEnable(false);
    }
    else if (mUlInputDevice  == AUDIO_DEVICE_IN_FM)
    {
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
    }
    return NO_ERROR;
}

status_t AudioResourceManager::SelectInputDevice(uint32_t device)
{
    ALOGD("%s(), mUlInputDevice = 0x%x, new device = 0x%x", __FUNCTION__, mUlInputDevice, device);

    StopInputDevice();
    mUlInputDevice = device;
    StartInputDevice();
    return NO_ERROR;
}

void AudioResourceManager::SetHardwarePointer(void *pAudioHardware)
{
    if (pAudioHardware == NULL) {
        ALOGW(" SetHardwarePointer paudioHardware ==NULL");
    }
    else {
        mAudioHardware = (AudioMTKHardware *)pAudioHardware;
    }
}


bool AudioResourceManager::IsWiredHeadsetOn()
{
    if ((mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (mDlOutputDevice & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
        return true;
    else
        return false;
}

bool AudioResourceManager::IsModeIncall(void)
{
    return (mAudioMode == AUDIO_MODE_IN_CALL ||
            mAudioMode == AUDIO_MODE_IN_CALL_2);
}

status_t AudioResourceManager::setParameters(int command1 , int command2 , unsigned int data)
{
    ALOGD("command1 = %d command2 = %d", command1, command2);
    return NO_ERROR;
}

status_t AudioResourceManager::setParameters(int command1 , void *data)
{
    ALOGD("command1 = %d ", command1);
    return NO_ERROR;
}

int AudioResourceManager::getParameters(int command1 , int command2 , void *data)
{
    return NO_ERROR;
}

int AudioResourceManager::dump(int command1)
{
    return NO_ERROR;
}


long AudioResourceManager::GetSwMICDigitalGain()
{
    long gain = 0;
    gain = mAudioVolumeInstance->GetSWMICGain();
    ALOGD("GetSwMICDigitalGain = %d ", gain);
    return gain;
}

status_t AudioResourceManager::doSetMode()
{
    return mAudioHardware->doSetMode();
}

status_t AudioResourceManager::SetMicInvserse(bool bEnable)
{
    mMicInverseSetting = bEnable;
    #ifdef MTK_DIGITAL_MIC_SUPPORT
    mAudioDigitalInstance->SetMicinputInverse (mMicInverseSetting^mMicDefaultsetting);
    #else
    mAudioAnalogInstance->SetAnalogPinmuxInverse(mMicInverseSetting^mMicDefaultsetting);
    #endif
    return NO_ERROR;
}

}
