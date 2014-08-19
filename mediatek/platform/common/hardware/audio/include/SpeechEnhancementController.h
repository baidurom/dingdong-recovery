#ifndef ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H
#define ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H

#include "AudioType.h"
#include "SpeechType.h"

#include "CFG_AUDIO_File.h"

namespace android
{
class SpeechEnhancementController
{
    public:
        static SpeechEnhancementController *GetInstance();

        virtual ~SpeechEnhancementController();

        /**
         * speech enhancement parameters setting
         */
        status_t SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
#if defined(MTK_DUAL_MIC_SUPPORT)
        status_t SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
        status_t SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
#endif

        /**
         * speech enhancement functions on/off
         */
        sph_enh_mask_struct_t GetSpeechEnhancementMask() const { return mSpeechEnhancementMask; }
        status_t SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask);


    private:
        SpeechEnhancementController();
        static SpeechEnhancementController *mSpeechEnhancementController; // singleton

        sph_enh_mask_struct_t mSpeechEnhancementMask;
};


} // end namespace android

#endif // end of ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H
