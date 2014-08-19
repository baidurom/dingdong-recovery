#include "LoopbackManager.h"

#include <media/AudioSystem.h>
#include <cutils/properties.h>
#include <hardware_legacy/power.h>

#include "SpeechLoopbackController.h"
#include "AudioLoopbackController.h"

#include "SpeechEnhancementController.h"
#include "SpeechDriverFactory.h"

#include "AudioResourceFactory.h"
#include "AudioDigitalControlFactory.h"
#include "AudioMTKStreamManager.h"


#define LOG_TAG "LoopbackManager"
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
static const char LOOPBACK_WAKELOCK_NAME[] = "LOOPBACK_WAKELOCK_NAME";

LoopbackManager *LoopbackManager::mLoopbackManager = NULL;
LoopbackManager *LoopbackManager::GetInstance()
{
    if (mLoopbackManager == NULL) {
        mLoopbackManager = new LoopbackManager();
    }
    ASSERT(mLoopbackManager != NULL);
    return mLoopbackManager;
}

LoopbackManager::LoopbackManager()
{
    mLoopbackType = NO_LOOPBACK;

    // default use modex 1 or 2 for speech loopback
#if defined(MTK_ENABLE_MD1)
    mWorkingModemIndex = MODEM_1;
#elif defined(MTK_ENABLE_MD2)
    mWorkingModemIndex = MODEM_2;
#else
    mWorkingModemIndex = MODEM_1;
#endif
}

LoopbackManager::~LoopbackManager()
{

}

loopback_t LoopbackManager::GetLoopbackType()
{
    Mutex::Autolock _l(mLock);
    ALOGV("%s(), mLoopbackType = %d", __FUNCTION__, mLoopbackType);
    return mLoopbackType;
}

status_t LoopbackManager::SetLoopbackOn(loopback_t loopback_type, loopback_output_device_t loopback_output_device)
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), loopback_type = %d, loopback_output_device = %d", __FUNCTION__,  loopback_type, loopback_output_device);

    if (mLoopbackType != NO_LOOPBACK) { // check no loobpack function on
        ALOGD("-%s() : Please Turn off Loopback Type %d First!!", __FUNCTION__, mLoopbackType);
        return ALREADY_EXISTS;
    }
    else if (CheckLoopbackTypeIsValid(loopback_type) != NO_ERROR) { // to avoid using undefined loopback type & ref/dual mic in single mic project
        ALOGW("-%s(): No such Loopback type %d", __FUNCTION__, loopback_type);
        return BAD_TYPE;
    }

    // lock
    AudioResourceManagerInterface *pAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
    pAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, 3000);
    pAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK, 3000);
    pAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_VOLUME_LOCK, 3000);

    // force all input/output streams standby
    AudioMTKStreamManager::getInstance()->ForceAllStandby();

    // copy current device
    mInputDeviceCopy  = (audio_devices_t)pAudioResourceManager->getUlInputDevice();
    mOutputDeviceCopy = (audio_devices_t)pAudioResourceManager->getDlOutputDevice();

    // get loopback device
    audio_devices_t input_device  = GetInputDeviceByLoopbackType(loopback_type);
    audio_devices_t output_device = GetOutputDeviceByLoopbackType(loopback_type, loopback_output_device);

    // check modem status
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex);
    if (pSpeechDriver->CheckModemIsReady() == false) { // modem is sleep...
        for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) { // get working modem index
            pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex((modem_index_t)modem_index);
            if (pSpeechDriver != NULL && pSpeechDriver->CheckModemIsReady() == true) {
                mWorkingModemIndex = (modem_index_t)modem_index;
                break;
            }
        }
    }

    // to avoid BT test being interferenced by modem side speech enhancement
    if (loopback_type == MD_BT_LOOPBACK) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex)->SetSpeechEnhancement(false);
    }

    // to turn on/off DMNR
    if (loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR ||
        loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR) {
        mMaskCopy = SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask(); // copy DMNR mask
        sph_enh_mask_struct_t mask = mMaskCopy;
        if (loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR) {
            mask.dynamic_func &= (~SPH_ENH_DYNAMIC_MASK_DMNR);
        }
        else if (loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR) {
            mask.dynamic_func |= SPH_ENH_DYNAMIC_MASK_DMNR;
        }
        SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex)->SetSpeechEnhancementMask(mask);
    }

    // Enable loopback function
    switch (loopback_type) {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case AP_REF_MIC_AFE_LOOPBACK:
        case AP_BT_LOOPBACK: {
            AudioLoopbackController::GetInstance()->OpenAudioLoopbackControlFlow(input_device, output_device);
            break;
        }
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
        case MD_REF_MIC_ACOUSTIC_LOOPBACK:
        case MD_BT_LOOPBACK: {
            SpeechLoopbackController::GetInstance()->OpenModemLoopbackControlFlow(mWorkingModemIndex, input_device, output_device);
            break;
        }
        default: {
            ALOGW("%s(): Loopback type %d not implemented!!", __FUNCTION__, loopback_type);
            ASSERT(0);
        }
    }

    // only use L ch data, so mute R ch. (Disconnect ADC_I2S_IN_R -> MODEM_PCM_TX_R)
    if (loopback_type == MD_MAIN_MIC_ACOUSTIC_LOOPBACK ||
        loopback_type == MD_REF_MIC_ACOUSTIC_LOOPBACK) {
        AudioDigitalControlFactory::CreateAudioDigitalControl()->SetinputConnection(
            AudioDigitalType::DisConnect,
            AudioDigitalType::I04,
            (mWorkingModemIndex == MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08);
    }

    // save opened loobpack type
    mLoopbackType = loopback_type;

    // acquire wake lock
    int ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, LOOPBACK_WAKELOCK_NAME);
    ALOGD("%s(), acquire_wake_lock:%s, return %d.", __FUNCTION__, LOOPBACK_WAKELOCK_NAME, ret);

    // unlock
    pAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_VOLUME_LOCK);
    pAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
    pAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

    ALOGD("-%s(), loopback_type = %d, loopback_output_device = %d", __FUNCTION__,  loopback_type, loopback_output_device);
    return NO_ERROR;
}

status_t LoopbackManager::SetLoopbackOff()
{
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), mLoopbackType = %d", __FUNCTION__, mLoopbackType);
    if (mLoopbackType == NO_LOOPBACK) { // check loobpack do exist to be turned off
        ALOGD("-%s() : No looback to be closed", __FUNCTION__);
        return INVALID_OPERATION;
    }

    // lock
    AudioResourceManagerInterface *pAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
    pAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, 3000);
    pAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK, 3000);
    pAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_VOLUME_LOCK, 3000);

    // force all input/output streams standby
    AudioMTKStreamManager::getInstance()->ForceAllStandby();

    // Disable Loopback function
    switch (mLoopbackType) {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case AP_REF_MIC_AFE_LOOPBACK:
        case AP_BT_LOOPBACK: {
            AudioLoopbackController::GetInstance()->CloseAudioLoopbackControlFlow();
            break;
        }
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
        case MD_REF_MIC_ACOUSTIC_LOOPBACK:
        case MD_BT_LOOPBACK: {
            SpeechLoopbackController::GetInstance()->CloseModemLoopbackControlFlow(mWorkingModemIndex);
            break;
        }
        default: {
            ALOGW("%s(): Loopback type %d not implemented!!", __FUNCTION__, mLoopbackType);
            ASSERT(0);
        }
    }

    // recover DMNR
    if (mLoopbackType == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR ||
        mLoopbackType == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex)->SetSpeechEnhancementMask(mMaskCopy);
    }

    // recover modem side speech enhancement
    if (mLoopbackType == MD_BT_LOOPBACK) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex)->SetSpeechEnhancement(true);
    }

    // recover device
    pAudioResourceManager->setDlOutputDevice(mOutputDeviceCopy);
    pAudioResourceManager->setUlInputDevice(mInputDeviceCopy);

    // clean
    mLoopbackType = NO_LOOPBACK;

    // release wake lock
    int ret = release_wake_lock(LOOPBACK_WAKELOCK_NAME);
    ALOGD("%s(), release_wake_lock:%s return %d.", __FUNCTION__, LOOPBACK_WAKELOCK_NAME, ret);

    // unlock
    pAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_VOLUME_LOCK);
    pAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
    pAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t LoopbackManager::CheckLoopbackTypeIsValid(loopback_t loopback_type)
{
    status_t retval;

    switch (loopback_type) {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case AP_REF_MIC_AFE_LOOPBACK:
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
        case MD_REF_MIC_ACOUSTIC_LOOPBACK:
        case AP_BT_LOOPBACK:
        case MD_BT_LOOPBACK:
            retval = NO_ERROR;
            break;
        default:
            retval = BAD_TYPE;
            break;
    }

    return retval;
}


audio_devices_t LoopbackManager::GetInputDeviceByLoopbackType(loopback_t loopback_type)
{
    audio_devices_t input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;

    switch (loopback_type) {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR: {
            input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
            break;
        }
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK: {
            input_device = AUDIO_DEVICE_IN_WIRED_HEADSET;
            break;
        }
        case AP_REF_MIC_AFE_LOOPBACK:
        case MD_REF_MIC_ACOUSTIC_LOOPBACK: {
            input_device = AUDIO_DEVICE_IN_BACK_MIC;
            break;
        }
        case AP_BT_LOOPBACK:
        case MD_BT_LOOPBACK: {
            input_device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
            break;
        }
        default: {
            ALOGW("%s(): Loopback type %d not implemented!!", __FUNCTION__, loopback_type);
            ASSERT(0);
        }
    }

    return input_device;
}

audio_devices_t LoopbackManager::GetOutputDeviceByLoopbackType(loopback_t loopback_type, loopback_output_device_t loopback_output_device)
{
    // BT Loopback only use BT headset
    if (loopback_type == AP_BT_LOOPBACK || loopback_type == MD_BT_LOOPBACK) { // BT
        return AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET;
    }

    // Get Output Devices By LoopbackType
    audio_devices_t output_device;

    switch (loopback_output_device) {
        case LOOPBACK_OUTPUT_RECEIVER: {
            output_device = AUDIO_DEVICE_OUT_EARPIECE;
            break;
        }
        case LOOPBACK_OUTPUT_EARPHONE: {
            if (loopback_type == AP_HEADSET_MIC_AFE_LOOPBACK ||
                loopback_type == MD_HEADSET_MIC_ACOUSTIC_LOOPBACK) {
                output_device = AUDIO_DEVICE_OUT_WIRED_HEADSET;
            }
            else {
                output_device = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
            }
            break;
        }
        case LOOPBACK_OUTPUT_SPEAKER: {
            output_device = AUDIO_DEVICE_OUT_SPEAKER;
            break;
        }
        default: {
            output_device = AUDIO_DEVICE_OUT_EARPIECE;
            break;
        }
    }

    return output_device;
}


} // end of namespace android
