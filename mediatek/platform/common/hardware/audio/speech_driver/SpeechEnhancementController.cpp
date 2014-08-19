#include "SpeechEnhancementController.h"
#include "SpeechDriverFactory.h"
#include "SpeechDriverInterface.h"
#include "SpeechType.h"

#include <cutils/properties.h>

#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"

static const char PROPERTY_KEY_SPH_ENH_MASKS[PROPERTY_KEY_MAX] = "persist.af.modem.sph_enh_mask";

namespace android
{

SpeechEnhancementController *SpeechEnhancementController::mSpeechEnhancementController = NULL;
SpeechEnhancementController *SpeechEnhancementController::GetInstance()
{
    if (mSpeechEnhancementController == NULL) {
        mSpeechEnhancementController = new SpeechEnhancementController();
    }
    ASSERT(mSpeechEnhancementController != NULL);
    return mSpeechEnhancementController;
}


SpeechEnhancementController::SpeechEnhancementController()
{
    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sprintf(property_default_value, "0x%x 0x%x", SPH_ENH_MAIN_MASK_ALL, SPH_ENH_DYNAMIC_MASK_ALL);

    // get sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "0x%x 0x%x",
           &mSpeechEnhancementMask.main_func,
           &mSpeechEnhancementMask.dynamic_func);

    ALOGD("mSpeechEnhancementMask: main_func = 0x%x, sub_func = 0x%x",
          mSpeechEnhancementMask.main_func,
          mSpeechEnhancementMask.dynamic_func);
}

SpeechEnhancementController::~SpeechEnhancementController()
{

}

status_t SpeechEnhancementController::SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetNBSpeechParameters(pSphParamNB);
        }
    }

    return NO_ERROR;
}


#if defined(MTK_DUAL_MIC_SUPPORT)
status_t SpeechEnhancementController::SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetDualMicSpeechParameters(pSphParamDualMic);
        }
    }

    return NO_ERROR;
}
#endif


#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechEnhancementController::SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetWBSpeechParameters(pSphParamWB);
        }
    }

    return NO_ERROR;
}
#endif


status_t SpeechEnhancementController::SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask)
{
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "0x%x 0x%x", mask.main_func, mask.dynamic_func);
    property_set(PROPERTY_KEY_SPH_ENH_MASKS, property_value);

    mSpeechEnhancementMask = mask;

    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetSpeechEnhancementMask(mSpeechEnhancementMask);
        }
    }

    return NO_ERROR;
}

}
