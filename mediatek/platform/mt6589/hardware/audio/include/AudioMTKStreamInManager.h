#ifndef AUDIO_MTK_STREAM_IN_MANAGER_H
#define AUDIO_MTK_STREAM_IN_MANAGER_H

#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioStreamAttribute.h"
#include "AudioDigitalControlFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioMTKStreamInManagerInterface.h"
#include "AudioResourceManagerInterface.h"
#include "AudioResourceFactory.h"
#include "AudioIoctl.h"
#include "AudioUtility.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String16.h>

#define INCALL_RINGBUFFERE_SIZE (0x20000)  // 128k BufferSizw
#define AUDIO_RECORD_DROP_MS (120) // at max 200

namespace android
{

class AudioMTkRecordThread;

class AudioMTKStreamInManager
{
    public:

        static AudioMTKStreamInManager *getInstance();
        static void freeInstance();

        /**
        * check init done.
        * @return status_t*/
        status_t  initCheck();

        /**
        * do input standby , all read will be blocked.
        * @return status_t*/
        status_t  Do_input_standby(AudioMTKStreamInClient *Client);

        /**
        * do input standby
        * @return status_t*/
        status_t  Do_input_start(AudioMTKStreamInClient *Client);

        /**
        * do request StreaminClient
        * @return status_t*/
        AudioMTKStreamInClient *RequestClient(uint32_t Buflen = 0);

        /**
        * do release StreaminClient
        * @return status_t*/
        status_t  ReleaseClient(AudioMTKStreamInClient *Client);

        /**
        * this function is set mic mute
        * @return status_t*/
        virtual void SetInputMute(bool bEnable);

        status_t I2SAdcInSet(AudioDigtalI2S *AdcI2SIn, AudioStreamAttribute *AttributeClient);
        status_t Set2ndI2SIn(AudioDigtalI2S *m2ndI2SIn, unsigned int mSampleRate);
        status_t Enable2ndI2SIn(bool bEnable);

        // check if same mem interface has been used
        bool checkMemInUse(AudioMTKStreamInClient *Client);

        status_t AcquireMemBufferLock(AudioDigitalType::Digital_Block MemBlock, bool bEnable);

        status_t StartStreamInThread(uint32 mMemDataType);

        uint32_t CopyBufferToClient(uint32 mMemDataType, void *buffer , uint32 copy_size);
        uint32_t CopyBufferToClientIncall(RingBuf ul_ring_buf);

        status_t StartModemRecord(AudioMTKStreamInClient *Client);
        status_t StopModemRecord();

        status_t ApplyVolume(void* Buffer , uint32 BufferSize);

#ifdef MTK_DIGITAL_MIC_SUPPORT
		static const uint32 MemVULSamplerate  = 32000;
#else
        static const uint32 MemVULSamplerate  = 48000;
#endif
        static const uint32 MemDAISamplerate   = 8000 ;
        static const uint32 MemAWBSamplerate = 48000;

        static const uint32 MemVULBufferSize  = 0x2000;
        static const uint32 MemDAIBufferSize   = 0x1000 ;
        static const uint32 MemAWBBufferSize = 0x2000;

        class AudioMTkRecordThread : public Thread
        {
            public:
                AudioMTkRecordThread(AudioMTKStreamInManager *AudioManager, uint32 Mem_type, char *mRingBuffer, uint32 mBufferSize);
                virtual ~AudioMTkRecordThread();

                // Good place to do one-time initializations
                virtual status_t    readyToRun();
                virtual void        onFirstRef();

                void WritePcmDumpData();
                void ClosePcmDumpFile();
                void DropRecordData();

                static int DumpFileNum;
            private:
                int mFd;
                int mMemType;
                String8 mName;
                virtual bool threadLoop();
                char *mRingBuffer;
                uint32 mBufferSize;
                AudioMTKStreamInManager *mManager;
                unsigned char tempdata;
                uint32 mRecordDropms;
                String8 DumpFileName;

                FILE *mPAdcPCMDumpFile;
                FILE *mPI2SPCMDumpFile;
                FILE *mPDAIInPCMDumpFile; //ccc add
        };

    private:
        AudioMTKStreamInManager();
        ~AudioMTKStreamInManager();

        static AudioMTKStreamInManager *UniqueStreamInManagerInstance;
        AudioMTKStreamInManager(const AudioMTKStreamInManager &);             // intentionally undefined
        AudioMTKStreamInManager &operator=(const AudioMTKStreamInManager &);  // intentionally undefined
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
        bool checkFmMicConflict(AudioMTKStreamInClient *Client, bool status);
#endif

		void PreLoadHDRecParams(void);
        AudioDigitalControlInterface *mAudioDigital;
        AudioAnalogControlInterface *mAudioAnalog;
        AudioResourceManagerInterface *mAudioResourceManager;
        uint32_t mMode;
        uint32_t mClientNumber ;

        KeyedVector<uint32_t, AudioMTKStreamInClient *> mAudioInput; // vector to save current recording client

        AudioDigtalI2S mAdcI2SIn;
        AudioDigtalI2S m2ndI2S;
        AudioDigitalPCM mModPcm_1;  // slave only ocm
        AudioDigitalPCM mModPcm_2;  // slave,master  pcm
        AudioDigitalDAIBT mDaiBt;
        AudioMrgIf mMrgIf;
        RingBuf mIncallRingBuffer;
        bool mMicMute;
        bool mMuteTransition;

        sp<AudioMTkRecordThread>  mAWBThread;
        char *mAWBbuffer;
        Mutex mAWBBufferLock;

        sp<AudioMTkRecordThread>  mVULThread;
        char *mVULbuffer;
        Mutex mAULBufferLock;

        sp<AudioMTkRecordThread>  mDAIThread;
        char *mDAIbuffer;
        Mutex mDAIBufferLock;

        sp<AudioMTkRecordThread>  mMODDAIThread;
        char *mMODDAIbuffer;
        Mutex mMODDAIBufferLock;

};

}

#endif
