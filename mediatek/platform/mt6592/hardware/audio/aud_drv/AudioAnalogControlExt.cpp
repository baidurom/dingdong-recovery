#include "AudioAnalogControlExt.h"
#include "AudioAnalogControl.h"
#include "AudioIoctl.h"
#include "AudioType.h"
#include "audio_custom_exp.h"
#include <utils/Log.h>

#define LOG_TAG "AudioAnalogControlExt"

namespace android
{

AudioAnalogControlExt *AudioAnalogControlExt::UniqueAnalogExtInstance = 0;

AudioAnalogControlExt *AudioAnalogControlExt::getInstance()
{
    if (UniqueAnalogExtInstance == 0)
    {
        ALOGD("+AudioAnalogControlExt");
        UniqueAnalogExtInstance = new AudioAnalogControlExt();
        ALOGD("-AudioAnalogControlExt");
    }
    return UniqueAnalogExtInstance;
}

AudioAnalogControlExt::~AudioAnalogControlExt()
{

}

AudioAnalogControlExt::AudioAnalogControlExt()
{
    ALOGD("AudioAnalogControlExt contructor \n");
    mFd = 0;

    // init analog part.
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd <= 0)
    {
        ALOGW("AudioAnalogControlExt open kernel device fail");
    }
    ALOGD("mFd=0x%x \n", mFd);

    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++)
    {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++)
    {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }
}

status_t AudioAnalogControlExt::InitCheck()
{
    ALOGD("InitCheck \n");
    return NO_ERROR;
}

//analog gain setting
status_t AudioAnalogControlExt::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("AudioAnalogControlExt::SetAnalogGain VoleumType = %d volume = %d \n", VoleumType, volume);
#ifdef MTK_AUDIO_EXTCODEC_SUPPORT
    ALOGD("SetAnalogGain VOLUME_TYPE = %d volume = %d ", VoleumType, volume);
    switch (VoleumType)
    {
        case AudioAnalogType::VOLUME_HSOUTL:
        case AudioAnalogType::VOLUME_HSOUTR:
            break;
        case AudioAnalogType::VOLUME_HPOUTL:
        case AudioAnalogType::VOLUME_HPOUTR:
#ifdef HP_USING_EXTDAC
            SetHeadPhoneGain(VoleumType, volume);
#endif
            break;
        case AudioAnalogType::VOLUME_SPKL:
        case AudioAnalogType::VOLUME_SPKR:
            break;
        case AudioAnalogType::VOLUME_LINEINL:
        case AudioAnalogType::VOLUME_LINEINR:
            break;
        case AudioAnalogType::VOLUME_MICAMPL:
        case AudioAnalogType::VOLUME_MICAMPR:
            break;
        case AudioAnalogType::VOLUME_LEVELSHIFTL:
        case AudioAnalogType::VOLUME_LEVELSHIFTR:
            break;
        case AudioAnalogType::VOLUME_IV_BUFFER:
            break;
            // default no support device
        case AudioAnalogType::VOLUME_LINEOUTL:
        case AudioAnalogType::VOLUME_LINEOUTR:
        default:
            break;

    }
    return NO_ERROR;

#else
    return NO_ERROR;
#endif
}

status_t AudioAnalogControlExt::SetHeadPhoneGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{

    uint32_t index = 0;
    int32_t gain;
    //volume is degrade gain in DB, need to convert to register index
    ALOGD("SetHeadPhoneGain leftright %d, gain= -%d dB", volume_Type, volume);
    index = (volume << 1) & 0xff; //CS4398 volume step is 0.5dB, volume=3, -3dB==> set 6 to register

    if (volume_Type == AudioAnalogType::VOLUME_HPOUTL)
    {
        gain = index | 0x0000;
        ::ioctl(mFd, SET_EXTCODEC_GAIN, gain); // 0x0000BBAA,  BB(00: left, 01: right), AA(volume set to DAC register)
    }
    else if (volume_Type == AudioAnalogType::VOLUME_HPOUTR)
    {
        gain = index | 0x0100;
        ::ioctl(mFd, SET_EXTCODEC_GAIN, gain); // 0x0000BBAA,  BB(00: left, 01: right), AA(volume set to DAC register)
    }
    return NO_ERROR;
}
int AudioAnalogControlExt::GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType)
{
    ALOGD("GetAnalogGain VoleumType = %d", VoleumType);
    return 0;
}

status_t AudioAnalogControlExt::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("SetAnalogMute VoleumType = %d mute = %d \n", VoleumType, mute);
    ::ioctl(mFd, SET_EXTCODEC_MUTE, NULL);
    return NO_ERROR;
}

// analog open power , need to open by mux setting
status_t AudioAnalogControlExt::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AudioAnalogControlExt::AnalogOpen DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
#ifdef MTK_AUDIO_EXTCODEC_SUPPORT

    mLock.lock();
    if (mBlockAttribute[DeviceType].mEnable == true)
    {
        ALOGW("AnalogOpen bypass with DeviceType = %d", DeviceType);
        mLock.unlock();
        return NO_ERROR;;
    }
    mBlockAttribute[DeviceType].mEnable = true;
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:

            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            // tell kernel to open device
#ifdef HP_USING_EXTDAC
            ALOGD("AnalogOpen mFd=0x%x \n", mFd);
            ::ioctl(mFd, SET_EXTCODEC_ON, NULL); // GPIO turn ON CS4398, and call ExtCodec_PowerOn()
            ::ioctl(mFd, SET_EXTHEADPHONE_AMP_ON, NULL); // GPIO turn ON TPA6141 in kernel (similar to cust_matv_gpio_on()), TPA6141 gain is fixed at 0dB or +6dB?
#endif
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:

            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:

            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:

            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            break;
        case AudioAnalogType::DEVICE_OUT_LINEOUTR:
        case AudioAnalogType::DEVICE_OUT_LINEOUTL:
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            WARNING("AnalogOpen with not support device");
            break;
    }
    mLock.unlock();
    return NO_ERROR;

#else
    return NO_ERROR;
#endif
}

status_t AudioAnalogControlExt::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("AudioAnalogControlExt SetFrequency DeviceType = %dfrequency = %d", DeviceType, frequency);

    return NO_ERROR;
}

status_t AudioAnalogControlExt::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    ALOGD("AudioAnalogControlExt::AnalogClose DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);

#ifdef MTK_AUDIO_EXTCODEC_SUPPORT
    uint16_t i;
    mLock.lock();
    mBlockAttribute[DeviceType].mEnable = false;
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            // tell kernel to close device
#ifdef HP_USING_EXTDAC
            ::ioctl(mFd, SET_EXTHEADPHONE_AMP_OFF, NULL); // GPIO turn OFF TPA6141 in kernel (similar to cust_matv_gpio_off())
            ::ioctl(mFd, SET_EXTCODEC_OFF, NULL); // GPIO turn OFF CS4398, and call ExtCodec_PowerOff()
#endif
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            break;
        case AudioAnalogType::DEVICE_OUT_LINEOUTR:
        case AudioAnalogType::DEVICE_OUT_LINEOUTL:
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            WARNING("AnalogOpen with not support device");
            break;
    }
    mLock.unlock();
    return NO_ERROR;

#else
    return NO_ERROR;
#endif
}

status_t AudioAnalogControlExt::SetAnalogPinmuxInverse(bool bEnable)
{
    ALOGD("SetAnalogPinmuxInverse bEnable = %d", bEnable);
    return NO_ERROR;
}

bool AudioAnalogControlExt::GetAnalogPinmuxInverse(void)
{
    return false;
}

//some analog part may has mux for different output
AudioAnalogType::MUX_TYPE AudioAnalogControlExt::AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioAnalogControlExt AnalogGetMux");
    return AudioAnalogType::MUX_AUDIO;
}

//some analog part may has mux for different output
status_t AudioAnalogControlExt::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AudioAnalogControlExt AnalogSetMux");
    return NO_ERROR;
}

bool AudioAnalogControlExt::GetAnalogState(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    return false;
}

bool AudioAnalogControlExt::AnalogDLlinkEnable(void)
{
    return false;
}

bool AudioAnalogControlExt::AnalogUplinkEnable(void)
{
    return false;
}

// set parameters and get parameters
status_t AudioAnalogControlExt::setParameters(int command1 , int command2 , unsigned int data)
{
    return NO_ERROR;
}

status_t AudioAnalogControlExt::setParameters(int command1 , void *data)
{
    return NO_ERROR;
}

int AudioAnalogControlExt::getParameters(int command1 , int command2 , void *data)
{
    return 0;
}

status_t AudioAnalogControlExt::setmode(audio_mode_t mode)
{
    return NO_ERROR;
}

// Fade out / fade in
status_t AudioAnalogControlExt::FadeOutDownlink(uint16_t sample_rate)
{
    return NO_ERROR;
}

status_t AudioAnalogControlExt::FadeInDownlink(uint16_t sample_rate)
{
    return NO_ERROR;
}

status_t AudioAnalogControlExt::SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value)
{
    return NO_ERROR;
}

bool AudioAnalogControlExt::GetAnalogSpkOCState(void)
{
    return false;
}

status_t AudioAnalogControlExt::AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    return NO_ERROR;
}
status_t AudioAnalogControlExt::AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting)
{
    return NO_ERROR;
}



}
