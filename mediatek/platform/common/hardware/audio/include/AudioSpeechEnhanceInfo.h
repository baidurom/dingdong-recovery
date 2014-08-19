#ifndef AUDIO_SPEECH_ENHANCE_INFO_H
#define AUDIO_SPEECH_ENHANCE_INFO_H

#include "AudioIoctl.h"
#include "AudioUtility.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String16.h>

#include <audio_utils/echo_reference.h>
#include "AudioMTKStreamOut.h"

namespace android
{
class AudioMTKStreamOut;
enum ENUM_SPH_FIR_COEFF
{
    SPH_FIR_COEFF_NORMAL            = 0,
    SPH_FIR_COEFF_HEADSET           = 1,
    //SPH_FIR_COEFF_HANDFREE        = 2,
    SPH_FIR_COEFF_BT                = 3,
    //SPH_FIR_COEFF_VOIP_NORMAL     = 4,
    //SPH_FIR_COEFF_VOIP_HANDFREE   = 5,
    SPH_FIR_COEFF_HANDSET_MIC2      = 6,
    SPH_FIR_COEFF_VOICE_REC      = 7 ,
    SPH_FIR_COEFF_NONE              = 0xFF
};

class AudioSpeechEnhanceInfo
{
    public:

        static AudioSpeechEnhanceInfo *getInstance();
		static void freeInstance();
		
		void SetHDRecScene(int32 HDRecScene);
		int32 GetHDRecScene(void);
		void ResetHDRecScene(void);
		
		void SetStreamOutPointer(void *pStreamOut);

		int GetOutputSampleRateInfo(void);
		int GetOutputChannelInfo(void);
		bool IsOutputRunning(void);

		void add_echo_reference(struct echo_reference_itfe *reference);
		void remove_echo_reference(struct echo_reference_itfe *reference);

		void SetRecordLRChannelSwitch(bool bIsLRSwitch);
		bool GetRecordLRChannelSwitch(void);

		void SetUseSpecificMIC(int32 UseSpecificMic);
		int GetUseSpecificMIC(void);
		
//for HDRec tunning
		void SetHDRecTunningEnable(bool bEnable);
		bool IsHDRecTunningEnable(void);

		status_t SetHDRecVMFileName(const char *fileName);
		void GetHDRecVMFileName(char * VMFileName);
		
    private:
        AudioSpeechEnhanceInfo();
        ~AudioSpeechEnhanceInfo();

		static AudioSpeechEnhanceInfo *UniqueAudioSpeechEnhanceInfoInstance;

    	int32 mHdRecScene; // for HD Record
    	bool mIsLRSwitch;
		int32 mUseSpecificMic;
        Mutex mHDRInfoLock;
		AudioMTKStreamOut *mStreamOut;

		//for HDRec tunning
		bool mHDRecTunningEnable;
		char mVMFileName[128];
};

}

#endif
