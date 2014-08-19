#include "AudioLoopbackController.h"

#define LOG_TAG "AudioLoopbackController"
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

// for use max gain for audio loopback
#define AUDIO_LOOPBACK_USE_MAX_GAIN

namespace android
{

static const float kMaxMasterVolume = 1.0;

// too big gain might cause "Bee~" tone due to the output sound is collected by input
static const int kPreAmpGainMapValue[] = {30, 24, 18, 12, 6, 0}; // Map to AUDPREAMPGAIN: (000) 2dB, (001) 8dB, (010) 14dB, ..., (101) 32dB

enum preamp_gain_index_t {
    PREAMP_GAIN_2_DB  = 0,
    PREAMP_GAIN_8_DB  = 1,
    PREAMP_GAIN_14_DB = 2,
    PREAMP_GAIN_20_DB = 3,
    PREAMP_GAIN_26_DB = 4,
    PREAMP_GAIN_32_DB = 5,
};



AudioLoopbackController *AudioLoopbackController::mAudioLoopbackController = NULL;
AudioLoopbackController *AudioLoopbackController::GetInstance()
{
    if (mAudioLoopbackController == NULL) {
        mAudioLoopbackController = new AudioLoopbackController();
    }
    ASSERT(mAudioLoopbackController != NULL);
    return mAudioLoopbackController;
}

AudioLoopbackController::AudioLoopbackController()
{
    // create volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();

    // create digital control instnace
    mAudioDigitalInstance  = AudioDigitalControlFactory::CreateAudioDigitalControl();

    // create digital control instnace
    mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();

    mAudioResourceManager = AudioResourceManager::getInstance();

    mMasterVolumeCopy  = 1.0;
    mMicAmpLchGainCopy = 0;
    mMicAmpRchGainCopy = 0;
}

AudioLoopbackController::~AudioLoopbackController()
{

}

status_t AudioLoopbackController::OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device)
{
    ALOGD("+%s(), input_device = 0x%x, output_device = 0x%x", __FUNCTION__, input_device, output_device);

    // check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);

    // set sample rate
#if defined(MTK_DIGITAL_MIC_SUPPORT)
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 32000;
#else
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 48000;
#endif

    // enable clock
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

    // set sampling rate
    mAudioAnalogInstance->SetFrequency(AudioAnalogType::DEVICE_OUT_DAC, sample_rate);
    mAudioAnalogInstance->SetFrequency(AudioAnalogType::DEVICE_IN_ADC, sample_rate);

    // set device
    mAudioResourceManager->setDlOutputDevice(output_device);
    mAudioResourceManager->setUlInputDevice(input_device);

    // Open ADC/DAC I2S, or DAIBT
    if (bt_device_on == true) { // DAIBT
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I02, AudioDigitalType::O02); // DAIBT_IN -> DAIBT_OUT

        SetDAIBTAttribute(sample_rate);

        mAudioDigitalInstance->SetDAIBTEnable(true);
    }
    else { // ADC/DAC I2S
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, AudioDigitalType::O03); // ADC_I2S_IN_L -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, AudioDigitalType::O04); // ADC_I2S_IN_L -> DAC_I2S_OUT_R

        SetADCI2sInAttribute(sample_rate);
        SetDACI2sOutAttribute(sample_rate);

        mAudioDigitalInstance->SetI2SAdcEnable(true);
        mAudioDigitalInstance->SetI2SDacEnable(true);
    }

    // AFE_ON
    mAudioDigitalInstance->SetAfeEnable(true);

    // Set PMIC digital/analog part - uplink has pop, open first
    mAudioResourceManager->StartInputDevice();
    usleep(100 * 1000); // HW pulse

    // Set PMIC digital/analog part - DL need trim code. Otherwise, just increase the delay between UL & DL...
    usleep(200 * 1000); // HW pulse
    mAudioResourceManager->StartOutputDevice();

#ifdef AUDIO_LOOPBACK_USE_MAX_GAIN
    // adjust downlink volume for current mode and routes
    mMasterVolumeCopy = mAudioVolumeInstance->getMasterVolume();
    mAudioVolumeInstance->setMasterVolume(kMaxMasterVolume, AUDIO_MODE_NORMAL, output_device);

    // adjust uplink volume for current mode and routes
    mMicAmpLchGainCopy = mAudioAnalogInstance->GetAnalogGain(AudioAnalogType::VOLUME_MICAMPL);
    mMicAmpRchGainCopy = mAudioAnalogInstance->GetAnalogGain(AudioAnalogType::VOLUME_MICAMPR);
    if (output_device == AUDIO_DEVICE_OUT_SPEAKER) {
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, kPreAmpGainMapValue[PREAMP_GAIN_2_DB]);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, kPreAmpGainMapValue[PREAMP_GAIN_2_DB]);
    }
    else {
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, kPreAmpGainMapValue[PREAMP_GAIN_20_DB]);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, kPreAmpGainMapValue[PREAMP_GAIN_20_DB]);
    }
#endif

    ALOGD("-%s(), input_device = 0x%x, output_device = 0x%x", __FUNCTION__, input_device, output_device);
    return NO_ERROR;
}

status_t AudioLoopbackController::CloseAudioLoopbackControlFlow()
{
    ALOGD("+%s()", __FUNCTION__);

    // Stop PMIC digital/analog part
    mAudioResourceManager->StopOutputDevice();
    mAudioResourceManager->StopInputDevice();

    // Stop AP side digital part
    const audio_devices_t output_device = (audio_devices_t)mAudioResourceManager->getDlOutputDevice();
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_device);
    if (bt_device_on) {
        mAudioDigitalInstance->SetDAIBTEnable(false);

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I02, AudioDigitalType::O02); // DAIBT_IN -> DAIBT_OUT
    }
    else {
        mAudioDigitalInstance->SetI2SDacEnable(false);
        mAudioDigitalInstance->SetI2SAdcEnable(false);

        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I03, AudioDigitalType::O03); // ADC_I2S_IN_L -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I03, AudioDigitalType::O04); // ADC_I2S_IN_R -> DAC_I2S_OUT_R
    }

    // AFE_ON = false
    mAudioDigitalInstance->SetAfeEnable(false);

    // recover sampling rate
    mAudioAnalogInstance->SetFrequency(AudioAnalogType::DEVICE_OUT_DAC, 44100);
    mAudioAnalogInstance->SetFrequency(AudioAnalogType::DEVICE_IN_ADC, 44100);

#ifdef AUDIO_LOOPBACK_USE_MAX_GAIN
    // recover volume for current mode and routes
    mAudioVolumeInstance->setMasterVolume(mMasterVolumeCopy, AUDIO_MODE_NORMAL, output_device);
    mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, mMicAmpLchGainCopy);
    mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, mMicAmpRchGainCopy);
#endif

    // disable clock
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);

    ALOGD("-%s()", __FUNCTION__);

    return NO_ERROR;
}


status_t AudioLoopbackController::SetDACI2sOutAttribute(int sample_rate)
{
    AudioDigtalI2S dac_i2s_out_attribute;
    memset((void *)&dac_i2s_out_attribute, 0, sizeof(dac_i2s_out_attribute));

    dac_i2s_out_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    dac_i2s_out_attribute.mI2S_SAMPLERATE = sample_rate;
    dac_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    dac_i2s_out_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
    dac_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    mAudioDigitalInstance->SetI2SDacOut(&dac_i2s_out_attribute);
    return NO_ERROR;
}


status_t AudioLoopbackController::SetADCI2sInAttribute(int sample_rate)
{
    AudioDigtalI2S adc_i2s_in_attribute;
    memset((void *)&adc_i2s_in_attribute, 0, sizeof(adc_i2s_in_attribute));

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
    return NO_ERROR;
}


status_t AudioLoopbackController::SetDAIBTAttribute(int sample_rate)
{
    AudioDigitalDAIBT daibt_attribute;
    memset((void *)&daibt_attribute, 0, sizeof(daibt_attribute));

#if defined(MTK_MERGE_INTERFACE_SUPPORT)
    daibt_attribute.mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_MGRIF;
#else
    daibt_attribute.mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_BT;
#endif
    daibt_attribute.mDAI_BT_MODE = (sample_rate == 8000) ? AudioDigitalDAIBT::Mode8K : AudioDigitalDAIBT::Mode16K;
    daibt_attribute.mDAI_DEL = AudioDigitalDAIBT::HighWord; // suggest always HighWord
    daibt_attribute.mBT_LEN  = 0;
    daibt_attribute.mDATA_RDY = true;
    daibt_attribute.mBT_SYNC = AudioDigitalDAIBT::Short_Sync;
    daibt_attribute.mBT_ON = true;
    daibt_attribute.mDAIBT_ON = false;
    mAudioDigitalInstance->SetDAIBBT(&daibt_attribute);
    return NO_ERROR;
}

} // end of namespace android
