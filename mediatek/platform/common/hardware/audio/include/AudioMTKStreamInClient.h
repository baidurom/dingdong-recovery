#ifndef AUDIO_MTK_STREAM_IN_CLIENT_H
#define AUDIO_MTK_STREAM_IN_CLIENT_H

#include "AudioType.h"
#include "AudioUtility.h"
#include "AudioStreamAttribute.h"
#include <utils/threads.h>

extern "C" {
#include "bli_exp.h"
}

namespace android
{

class AudioMTKStreamInClient
{
    public:
        AudioMTKStreamInClient(uint32 BuffeSize, uint32 ClientId);
        ~AudioMTKStreamInClient();
        RingBuf mRingBuf;
        uint32 mClientId;
        uint32 mMemDataType;    // which type of data need to be record
        uint32 mSourceType;
        AudioStreamAttribute *mAttributeClient;
        bool mEnable;
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
        bool mConflict;
        uint32 mAdcGroup;
        int mDropCount;
#endif
        Mutex mLock;
        Condition  mWaitWorkCV;

        // BLI_SRC
        BLI_HANDLE *mBliHandlerBuffer;
        char       *mBliOutputLinearBuffer;
};

}

#endif

