#ifndef ANDROID_AUDIO_LOOPBACK_CONTROLLER_H
#define ANDROID_AUDIO_LOOPBACK_CONTROLLER_H

#include "AudioType.h"

#include "AudioVolumeFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"
#include "AudioResourceManager.h"

namespace android
{

class AudioLoopbackController
{
    public:
        virtual ~AudioLoopbackController();

        static AudioLoopbackController *GetInstance();

        status_t OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device);
        status_t CloseAudioLoopbackControlFlow();

    protected:
        AudioLoopbackController();

        status_t SetDACI2sOutAttribute(int sample_rate);
        status_t SetADCI2sInAttribute(int sample_rate);
        status_t SetDAIBTAttribute(int sample_rate);

        float mMasterVolumeCopy;
        int   mMicAmpLchGainCopy;
        int   mMicAmpRchGainCopy;

        AudioMTKVolumeInterface *mAudioVolumeInstance;
        AudioAnalogControlInterface *mAudioAnalogInstance;
        AudioDigitalControlInterface *mAudioDigitalInstance;
        AudioResourceManagerInterface *mAudioResourceManager;

    private:
        static AudioLoopbackController *mAudioLoopbackController; // singleton
};

} // end namespace android

#endif // end of ANDROID_AUDIO_LOOPBACK_CONTROLLER_H
