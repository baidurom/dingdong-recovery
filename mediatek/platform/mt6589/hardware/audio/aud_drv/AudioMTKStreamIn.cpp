#include "AudioMTKStreamIn.h"
#include "AudioResourceManager.h"
#include "AudioMTKStreamInManager.h"
#include "AudioMTKStreamInManagerInterface.h"
#include <utils/Log.h>
#include <math.h>

#include "SpeechDriverFactory.h"
#include "SpeechVMRecorder.h"

#include "AudioCustParam.h"

#ifdef MTK_AUDIO_HD_REC_SUPPORT
#include "CFG_Audio_Default.h"
#endif
#include "AudioVUnlockDL.h"
#define MAX_FILE_LENGTH (60000000)
extern "C" {
#include "bli_exp.h"
}

#define LOG_TAG "AudioMTKStreamIn"
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

#define VOICE_RECOGNITION_RECORD_SAMPLE_RATE (16000)
#define HD_RECORD_SAMPLE_RATE (48000)
#define DAIBT_SAMPLE_RATE (8000)
#define MAX_DUMP_NUM (6)

#ifdef MTK_VOICEUNLOCK_DEBUG_ENABLE
static char const *const kAudioDeviceName = "/dev/eac";
#endif

#ifdef MTK_AUDIO_HD_REC_SUPPORT
static const unsigned long HDRecordEnhanceParasCommon[] =
{
    0,
    0,
    0,
    10752,
    32769,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const AUDIO_HD_RECORD_SCENE_TABLE_STRUCT DefaultRecordSceneTable[] = {Hd_Recrod_Scene_Table_default};
static const AUDIO_HD_RECORD_PARAM_STRUCT DefaultRecordParam[] = {Hd_Recrod_Par_default};
int AudioMTKStreamIn::DumpFileNum =0;
#endif

bool AudioMTKStreamIn::CheckFormat(int *pFormat)
{
    if (*pFormat != AUDIO_FORMAT_PCM_16_BIT)
    {
        return false;
    }
    return true;
}

bool AudioMTKStreamIn::CheckSampleRate(uint32_t device, uint32_t *pRate)
{
    if(mIsHDRecTunningEnable)   //tool tunning case
    {
        ALOGD("CheckSampleRate HDRecTunningEnable \n");
        if (*pRate == 48000)
        {
            mHDRecTunning16K = false;
            return true;
        }
        else if(*pRate == 16000)
        {
            mHDRecTunning16K = true;
            *pRate = 48000;
            return true;
        }

        return false;
    }
#ifdef FM_DIGITAL_IN_SUPPORT
    if (device == AUDIO_DEVICE_IN_AUX_DIGITAL) {
        if (*pRate != FM_I2S_IN_DEFAULT_SAMPLE_RATE)
            return false;
        else
            return true;
    }
    else
#endif
#ifdef MATV_AUDIO_SUPPORT
    ALOGD("%s(), MATV_AUDIO_SUPPORT, device= %d, *pRate=%d\n", __FUNCTION__, device, *pRate);
    if (device == AUDIO_DEVICE_IN_AUX_DIGITAL2) {
        if (*pRate != MATV_I2S_IN_DEFAULT_SAMPLE_RATE)
            return false;
        else
            return true;
    }
    else
#endif
    if (*pRate != mStream_Default_SampleRate)
    {
        return false;
    }
    return true;
}

bool AudioMTKStreamIn::CheckChannel(uint32_t device, uint32_t *pChannels)
{
    if (*pChannels !=  AUDIO_CHANNEL_IN_STEREO)
    {
        return false;
    }
    return true;
}

status_t AudioMTKStreamIn::dump(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

AudioMTKStreamIn::AudioMTKStreamIn()
{
    ALOGD("AudioMTKStreamIn contructor \n");
    memset((void *)&mAttribute, 0 , sizeof(AudioStreamAttribute));
    mStreamInManager = AudioMTKStreamInManager::getInstance();

    if (mStreamInManager == NULL)
    {
        ALOGW("AudioMTKStreamIn get mStreamInManager fail!! ");
    }
    mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    SetStreamInPreprocessStatus(true);
    mStarting = false;
    mLatency = 0;
    mSuspend = false;
    mPAdcPCMDumpFile = NULL;
    mPAdcPCMInDumpFile = NULL;
    mReadCount =0;
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    mpSPELayer = NULL;
    mHDRecordModeIndex = -1;
    mHDRecordSceneIndex = -1;
    mStereoMode = false;
#endif

#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
    mAPPS = NULL;
    mEcho_Reference = NULL;
#endif

#ifdef MTK_DUAL_MIC_SUPPORT
    mLRChannelSwitch = false;
    mSpecificMicChoose = 0;
#endif

    mBliHandler1 = NULL;
    mBliHandler1Buffer = NULL;
    mBliHandler2 = NULL;
    mBliHandler2Buffer = NULL;

    mBliSrc = NULL;
    mBliSrc = new BliSrc();

    mSwapBufferTwo = NULL;
    mSwapBufferTwo = new uint8_t[bufferSize()];
    if (mSwapBufferTwo == NULL)
    {
        ALOGE("mSwapBufferTwo for BliSRC allocate fail1!!! \n");
    }

    mIsHDRecTunningEnable = mAudioSpeechEnhanceInfoInstance->IsHDRecTunningEnable();
    memset(m_strTunningFileName, 0, 128);
    mAudioSpeechEnhanceInfoInstance->GetHDRecVMFileName(m_strTunningFileName);
    mHDRecTunning16K = false;
#ifdef MTK_VOICEUNLOCK_DEBUG_ENABLE
    mNoDataCount = 0;
    mFd =0;
#endif
}

uint32_t AudioMTKStreamIn::StreamInPreprocess(void *buffer , uint32_t bytes)
{
    uint32_t ProcessedBytes = bytes, ReadDataBytes = bytes;
    int TryCount = 20;
    char *pRead = (char *)buffer;
//    ALOGD("StreamInPreprocess buffer = %p bytes = %d, pRead=%p", buffer, bytes,pRead);
    // do streamin preprocess
    if (mEnablePreprocess)
    {
        do
        {
#ifdef MTK_AUDIO_HD_REC_SUPPORT
            ProcessedBytes = HDRecordPreprocess((void *)pRead, ReadDataBytes);
#endif

#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
            ProcessedBytes = NativeRecordPreprocess((void *)pRead, ProcessedBytes);
#endif
            pRead += ProcessedBytes;
            ReadDataBytes -= ProcessedBytes;
//            ALOGD("StreamInPreprocess pRead=%p,ReadDataBytes=%d,TryCount=%d,ProcessedBytes=%d",pRead,ReadDataBytes,TryCount,ProcessedBytes);
            ProcessedBytes = ReadDataBytes;

            if(ReadDataBytes>0) //it might happened when has SRC process
                Refilldata((char *)pRead,ReadDataBytes);
            TryCount--;
        }
        while(ReadDataBytes && TryCount);
    }
    return bytes;
}

ssize_t AudioMTKStreamIn::Refilldata(char *buffer, ssize_t bytes)
{
//    ALOGD("Refilldata buffer=%p,bytes=%d",buffer,bytes);
    int TryCount = 20;
    uint32 RingBufferSize = 0, ReadDataBytes = bytes;
    ASSERT(mStreamInClient != NULL);

    if(StreamIn_NeedToSRC() == true)
    {
		StreamInSRC_Process(buffer, bytes);
    }
    else
    {
        char *pWrite = (char *)buffer;
        do
        {
            mStreamInClient->mLock.lock();
            RingBufferSize = RingBuf_getDataCount(&mStreamInClient->mRingBuf);
            ALOGD("Refilldata RingBufferSize = %d TryCount = %d ReadDataBytes = %d mRingBuf = %p", RingBufferSize, TryCount, ReadDataBytes, &mStreamInClient->mRingBuf);
            if (RingBufferSize >= ReadDataBytes)   // ring buffer is enough, copy & exit
            {
                RingBuf_copyToLinear((char *)pWrite, &mStreamInClient->mRingBuf, ReadDataBytes);
                mStreamInClient->mLock.unlock();
                break;
            }
            else   // ring buffer is not enough, copy all data & wait
            {
                RingBuf_copyToLinear((char *)pWrite, &mStreamInClient->mRingBuf, RingBufferSize);
                ReadDataBytes -= RingBufferSize;
                pWrite += RingBufferSize;
            }
    //        ALOGD("+Refilldata wait for mStreamInClient->mLock mStreamInClient = %p", mStreamInClient);
            if (mStreamInClient->mWaitWorkCV.waitRelative(mStreamInClient->mLock, milliseconds(300)) != NO_ERROR)
            {
                ALOGW("waitRelative fail");
                if (mStreamInClient != NULL)   // busy
                {
                    mStreamInClient->mLock.unlock();
                    break;
                }
                else   // die
                {
                    return 0;
                }
            }
            //ALOGD("-read wait for mStreamInClient->mLock");
            mStreamInClient->mLock.unlock();
            TryCount--;
        }
        while (ReadDataBytes && TryCount);
    }

    if (mPAdcPCMInDumpFile) //fwrite((void *)buffer, sizeof(char), bytes, mPAdcPCMInDumpFile);
        AudioDumpPCMData((void *)buffer , bytes,mPAdcPCMInDumpFile);
    ALOGD("Refilldata ReadDataBytes=%d",ReadDataBytes);
    return bytes;
}

status_t AudioMTKStreamIn::SetStreamInPreprocessStatus(bool Enable)
{
    ALOGD("SetStreamInPreprocessStatus enable = %d", Enable);
    mEnablePreprocess = Enable;
    return NO_ERROR;
}

uint32 AudioMTKStreamIn::GetBufferSizeBydevice(uint32_t devices)
{
    switch (devices)
    {
    case AUDIO_DEVICE_IN_AMBIENT :
    case AUDIO_DEVICE_IN_BUILTIN_MIC :
    case AUDIO_DEVICE_IN_WIRED_HEADSET :
    case AUDIO_DEVICE_IN_BACK_MIC :
    case AUDIO_DEVICE_IN_COMMUNICATION :
    case AUDIO_DEVICE_IN_VOICE_CALL:
        return Default_Mic_Buffer;
        break;
    case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET :
        return Default_BT_Buffer;
        break;
    case AUDIO_DEVICE_IN_AUX_DIGITAL :
    case AUDIO_DEVICE_IN_AUX_DIGITAL2:
        return Default_Mic_Buffer;
        break;
    }
    return Default_Mic_Buffer;
}
void AudioMTKStreamIn::Set(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    ALOGD("AudioMTKStreamIn set devices = 0x%x format = 0x%x channele = 0x%x samplerate = %d, mIsHDRecTunningEnable=%x",
          devices, *format, *channels, *sampleRate, mIsHDRecTunningEnable);

    // check if can contruct successfully
    if (CheckFormat(format) && CheckSampleRate(devices, sampleRate) && CheckChannel(devices, channels))
    {
        // stream attribute
        mAttribute.mdevices = devices;
        mAttribute.mSampleRate = *sampleRate;
        mAttribute.mChannels = *channels;
        mAttribute.mAcoustic = acoustics;
        mAttribute.mFormat = *format;
        mAttribute.mInterruptSample = 0;
        mAttribute.mBufferSize = GetBufferSizeBydevice(devices);
        ALOGD("set mAttribute.mBufferSize  = %d",mAttribute.mBufferSize);
        mAttribute.mPredevices = devices;
#ifdef MTK_DIGITAL_MIC_SUPPORT        //for digital MIC sample rate constrain
        mAttribute.mIsDigitalMIC = true;
#else
        mAttribute.mIsDigitalMIC = false;
#endif
        *status = NO_ERROR;
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        mpSPELayer = new SPELayer();
        if (!mpSPELayer)
            ALOGE("new SPELayer() FAIL");
        else
        {
            LoadHDRecordParams();
            mpSPELayer->SetVMDumpEnable(mIsHDRecTunningEnable);
            mpSPELayer->SetVMDumpFileName(m_strTunningFileName);
        }
#endif
#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
        mAPPS = new AudioPreProcess();
        mEcho_Reference = NULL;
        if (!mAPPS)
            ALOGD("mAPPS() FAIL");
#endif

    }
    else
    {
        // modify default paramters and let Audiorecord open again for reampler.
#ifdef FM_DIGITAL_IN_SUPPORT
        // I2S FM use 44.1KHz
        if (devices == AUDIO_DEVICE_IN_AUX_DIGITAL) {
            *sampleRate = FM_I2S_IN_DEFAULT_SAMPLE_RATE;
        }
        else
#endif
        if (devices == AUDIO_DEVICE_IN_AUX_DIGITAL2) {
            *sampleRate = MATV_I2S_IN_DEFAULT_SAMPLE_RATE;
        }
        else
        {
            *sampleRate = mStream_Default_SampleRate;
        }
        *format  = mStream_Default_Format;

        *channels =  mStream_Default_Channels;
        *status = BAD_VALUE;
    }
}


AudioMTKStreamIn::~AudioMTKStreamIn()
{
    ALOGD("AudioMTKStreamIn destructor");

    standby();

#ifdef MTK_AUDIO_HD_REC_SUPPORT
    if (mpSPELayer != NULL)
    {
        delete mpSPELayer;
        mpSPELayer = NULL;
    }
#endif

#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
    if (mAPPS)
    {
        delete mAPPS;
        mAPPS = NULL;
        ALOGD("delete mAPPS() ");
        mEcho_Reference = NULL;
    }
#endif

    if (mBliSrc)
    {
        mBliSrc->close();
        delete mBliSrc;
        mBliSrc = NULL;
    }
    if (mSwapBufferTwo)
    {
        delete []mSwapBufferTwo;
        mSwapBufferTwo = NULL;
    }
}

uint32_t AudioMTKStreamIn::sampleRate() const
{
    return mAttribute.mSampleRate;
}

uint32_t AudioMTKStreamIn::bufferSize() const
{
    return mAttribute.mBufferSize;
}

uint32_t AudioMTKStreamIn::channels() const
{
    return mAttribute.mChannels;
}

int AudioMTKStreamIn::format() const
{
    return mAttribute.mFormat;
}

status_t AudioMTKStreamIn::setGain(float gain)
{
    return NO_ERROR;
}

bool AudioMTKStreamIn::SetIdentity(uint32_t id)
{
    mIdentity = id;
    return true;
}

uint32_t AudioMTKStreamIn::GetIdentity()
{
    return mIdentity;
}

status_t AudioMTKStreamIn::SetClientSourceandMemType(AudioMTKStreamInClient *mStreamInClient)
{
    ALOGD("+SetClientSourceandMemType mStreamInClient = %p", mStreamInClient);
    if (mStreamInClient == NULL)
    {
        ALOGW("SetClientSourceandMemType with null pointer");
        return INVALID_OPERATION;
    }

    switch (mStreamInClient->mAttributeClient->mSource)
    {
    case AUDIO_SOURCE_DEFAULT:
    case AUDIO_SOURCE_MIC:
    case AUDIO_SOURCE_VOICE_UPLINK :
    case AUDIO_SOURCE_VOICE_DOWNLINK :
    case AUDIO_SOURCE_VOICE_CALL :
    case AUDIO_SOURCE_CAMCORDER :
        mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
        break;
    case AUDIO_SOURCE_VOICE_RECOGNITION :
        mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
        break;
    case AUDIO_SOURCE_VOICE_COMMUNICATION :
        mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
        break;
    case AUDIO_SOURCE_VOICE_UNLOCK:
        mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
        break;
    case AUDIO_SOURCE_CUSTOMIZATION1:
    case AUDIO_SOURCE_CUSTOMIZATION2:
    case AUDIO_SOURCE_CUSTOMIZATION3:
        mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
        break;
#ifdef MATV_AUDIO_SUPPORT
    case AUDIO_SOURCE_MATV:
        if (mStreamInClient->mAttributeClient->mdevices == AUDIO_DEVICE_IN_AUX_DIGITAL2)
            mStreamInClient->mSourceType = AudioDigitalType::I2S_INOUT_2;
        else
            mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
    break;
#endif
    case AUDIO_SOURCE_FM:
#ifdef FM_ANALOG_IN_SUPPORT
        mStreamInClient->mSourceType = AudioDigitalType::I2S_IN_ADC;
#else
        mStreamInClient->mSourceType = AudioDigitalType::MRG_I2S_IN;
#endif
        break;
    }

    switch (mStreamInClient->mAttributeClient->mdevices)
    {
    case AUDIO_DEVICE_IN_AMBIENT :
    case AUDIO_DEVICE_IN_BUILTIN_MIC :
    case AUDIO_DEVICE_IN_WIRED_HEADSET :
    case AUDIO_DEVICE_IN_BACK_MIC :
    case AUDIO_DEVICE_IN_COMMUNICATION :
    case AUDIO_DEVICE_IN_VOICE_CALL:
        mStreamInClient->mMemDataType = AudioDigitalType::MEM_VUL;
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
        mStreamInClient->mAdcGroup = AudioAnalogType::ADC_GROUP_MIC_IN;
#endif
        break;
        // for BT scenario , it's a special case nned to modify source to DAI_BT
    case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET :
        mStreamInClient->mMemDataType = AudioDigitalType::MEM_DAI;
        mStreamInClient->mSourceType = AudioDigitalType::DAI_BT;
        break;
    case AUDIO_DEVICE_IN_AUX_DIGITAL :
    case AUDIO_DEVICE_IN_AUX_DIGITAL2:
        mStreamInClient->mMemDataType = AudioDigitalType::MEM_AWB;
        break;
    case AUDIO_DEVICE_IN_FM :
#ifdef MATV_AUDIO_SUPPORT
        if(mStreamInClient->mAttributeClient->mSource == AUDIO_SOURCE_MATV)//mATV usage
        {
            mStreamInClient->mMemDataType = AudioDigitalType::MEM_VUL;
        }
        else//FM usage
#endif
        {
#ifdef FM_ANALOG_IN_SUPPORT
            mStreamInClient->mMemDataType = AudioDigitalType::MEM_VUL;
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
            mStreamInClient->mAdcGroup = AudioAnalogType::ADC_GROUP_LINE_IN;
#endif
#else
            mStreamInClient->mMemDataType = AudioDigitalType::MEM_AWB;
#endif
        }
        break;
    default:
        ALOGW("no proper device match !!!");
        break;
    }
    ALOGD("-%s(), mStreamInClient = %p, mStreamInClient->mSourceType = %d, mStreamInClient->mMemDataType = %d\n", __FUNCTION__, mStreamInClient, mStreamInClient->mSourceType, mStreamInClient->mMemDataType);
    return NO_ERROR;
}

void AudioMTKStreamIn::StreamInSRC_Init(void)
{
	if(mStreamInClient->mMemDataType == AudioDigitalType::MEM_DAI) // cases to do UL SRC
	{
	    if (mBliSrc)
	    {
	        if (mBliSrc->initStatus() != OK)
	        {
	            // 6628 only support 8k BTSCO
	            const uint8_t num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
	            ALOGD("StreamInSRC_Init BLI_SRC, %d to mAttribute.mSampleRate=%d, num_channel=%d",DAIBT_SAMPLE_RATE, mAttribute.mSampleRate,num_channel);
	            mBliSrc->init(DAIBT_SAMPLE_RATE, 1, mAttribute.mSampleRate, num_channel);
	        }
	    }
	    else
	    {
	        ALOGW("StreamInSRC_Init() mBliSrc=NULL!!!");
	    }
	}
	else if(IsNeedDMICSRC())   // for digital MIC
	{
	    if (mBliSrc)
	    {
	        if (mBliSrc->initStatus() != OK)
	        {
	            // digital MIC only support 32k sample rate
	            const uint8_t num_channel = (mAttribute.mChannels == android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO) ? 2 : 1;
	            ALOGD("StreamInSRC_Init BLI_SRC, digital MIC 32k to mAttribute.mSampleRate=%d, num_channel=%d", mAttribute.mSampleRate,num_channel);
	            mBliSrc->init(32000, 2, mAttribute.mSampleRate, num_channel);
	        }
	    }
	    else
	    {
	        ALOGW("StreamInSRC_Init() DMIC mBliSrc=NULL!!!");
	    }
	}
}

uint32 AudioMTKStreamIn::GetSrcbufvalidSize(RingBuf *SrcInputBuf)
{

    //ALOGD("GetSrcbufvalidSize SrcInputBuf->pWrite=%x,SrcInputBuf->pRead=%x, SrcInputBuf->bufLen=%d ",SrcInputBuf->pWrite,SrcInputBuf->pRead,SrcInputBuf->bufLen);
    if(SrcInputBuf!= NULL)
    {
        if(SrcInputBuf->pWrite >=SrcInputBuf->pRead)
        {
            return SrcInputBuf->pWrite - SrcInputBuf->pRead;
        }
        else
        {
            return SrcInputBuf->pRead + SrcInputBuf->bufLen - SrcInputBuf->pWrite;
        }
    }
    ALOGW("SrcInputBuf == NULL");
    return 0;
}

uint32 AudioMTKStreamIn::GetSrcbufFreeSize(RingBuf *SrcInputBuf)
{
    if(SrcInputBuf != NULL)
    {
        return SrcInputBuf->bufLen - GetSrcbufvalidSize(SrcInputBuf);
    }
    ALOGW("SrcInputBuf == NULL");
    return 0;
}

// here copy SRCbuf to input buffer , and return how many buffer copied
uint32 AudioMTKStreamIn::CopySrcBuf(char *buffer,uint32 *bytes, RingBuf *SrcInputBuf, uint32 *length)
{
    uint32 consume =0;
    uint32 outputbyes = *bytes;
    //ALOGD("+CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    consume = mBliSrc->process((short*)SrcInputBuf->pRead,length,(short*)buffer,bytes);
    //ALOGD("-CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    SrcInputBuf->pRead += consume;
    if(SrcInputBuf->pRead >= (SrcInputBuf->pBufBase + SrcInputBuf->bufLen))
    {
        //ALOGD("SrcInputBuf->pRead = %d, SrcInputBuf->pBufBase=%d,SrcInputBuf->bufLen = %d",SrcInputBuf->pRead,SrcInputBuf->pBufBase, SrcInputBuf->bufLen);
        SrcInputBuf->pRead -= SrcInputBuf->bufLen;
    }
    *bytes = outputbyes -*bytes;
    return consume;
}

status_t AudioMTKStreamIn::RequesetRecordclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    return NO_ERROR;
}
status_t AudioMTKStreamIn::ReleaseRecordclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    return NO_ERROR;
}

bool AudioMTKStreamIn::IsNeedDMICSRC(void)
{
    if((mStreamInClient->mMemDataType == AudioDigitalType::MEM_VUL) && (mAttribute.mIsDigitalMIC==true) && (mStreamInClient->mSourceType == AudioDigitalType::I2S_IN_ADC))   // for digital MIC
    {
        return true;
    }
    return false;
}

bool AudioMTKStreamIn::StreamIn_NeedToSRC(void)
{
	if((mStreamInClient->mMemDataType == AudioDigitalType::MEM_DAI && mAttribute.mSampleRate!=DAIBT_SAMPLE_RATE) || IsNeedDMICSRC()){	// cases to Do UL SRC
		return true;
	}
	else{
		return false;
	}
}

void AudioMTKStreamIn::StreamInSRC_Process(void *buffer, uint32_t bytes)
{
    uint32_t CopyBufferwriteIdx =0;
    RingBuf *SrcInputBuf = &mStreamInClient->mRingBuf;
    uint32_t readbytes = bytes;

	if(mBliSrc->initStatus()== OK)
	{
		//ALOGD("read(), StreamInSRC_Process");
		do
		{
			//here need to do SRC and copy to buffer,check if there is any buf in src , exhaust it.
			if(GetSrcbufvalidSize(SrcInputBuf))
			{
				//ALOGD("read(), GetSrcbufvalidSize OK");
				uint32 bufLen =0;
				uint32 ConsumeReadbytes =0;
				if(SrcInputBuf->pWrite >= SrcInputBuf->pRead)
				{
					bufLen = GetSrcbufvalidSize(SrcInputBuf);
					ALOGV("SrcInputBuf->pWrite = %d SrcInputBuf->pRead = %d SrcInputBuf->bufLen = %d readbytes = %d CopyBufferwriteIdx = %d",
						  SrcInputBuf->pWrite,SrcInputBuf->pRead,SrcInputBuf->bufLen,readbytes,CopyBufferwriteIdx);
					ConsumeReadbytes = readbytes;
					CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,SrcInputBuf,&bufLen);
					CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
					if(readbytes == 0)
					{
						break;
					}
				}
				else
				{
					bufLen = SrcInputBuf->pBufBase + SrcInputBuf->bufLen - SrcInputBuf->pRead;
					ALOGV("SrcInputBuf->pWrite = %d SrcInputBuf->pRead = %d SrcInputBuf->bufLen = %d readbytes = %d CopyBufferwriteIdx = %d",
						  SrcInputBuf->pWrite,SrcInputBuf->pRead,SrcInputBuf->bufLen,readbytes,CopyBufferwriteIdx);
					ConsumeReadbytes = readbytes;
					CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,SrcInputBuf,&bufLen);
					CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
					if(readbytes == 0)
					{
						break;
					}
					else
					{
						bufLen = SrcInputBuf->pWrite - SrcInputBuf->pRead;
						ALOGV("SrcInputBuf->pWrite = %d SrcInputBuf->pRead = %d SrcInputBuf->bufLen = %d readbytes = %d CopyBufferwriteIdx = %d",
							  SrcInputBuf->pWrite,SrcInputBuf->pRead,SrcInputBuf->bufLen,readbytes,CopyBufferwriteIdx);
						ConsumeReadbytes = readbytes;
						CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,SrcInputBuf,&bufLen);
						CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
						if(readbytes == 0)
						{
							break;
						}
					}
				}
			}
			usleep(10*1000); //let MTKRecordThread to copy data
		}
		while(readbytes);
		//ALOGD("read(), StreamInSRC_Process, finish while loop~");
	}
}

ssize_t AudioMTKStreamIn::read(void *buffer, ssize_t bytes)
{
    ssize_t ReadSize = 0;
    int ret =0;
    uint32 RingBufferSize = 0, ReadDataBytes = bytes;
    int TryCount = 20;
    if((mReadCount % 10) ==0)
    {
        ALOGD("AudioMTKStreamIn::read buffer = %p bytes = %d  this = %p", buffer, bytes, this);
    }
    mReadCount++;

    if (mSuspend || SpeechVMRecorder::GetInstance()->GetVMRecordStatus() == true)
    {
        // here to sleep a buffer size latency and return.
        ALOGD("read suspend = %d",mSuspend);
        memset(buffer, 0, bytes);
        usleep(30 * 1000);
        return bytes;
    }
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, 3000);
    if(ret)
    {
        ALOGW("read EnableAudioLock AUDIO_HARDWARE_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, 3000);
    if(ret)
    {
        ALOGW("read EnableAudioLock AUDIO_STREAMINMANAGER_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, 3000);
    if(ret)
    {
        ALOGW("read EnableAudioLock AUDIO_STREAMINMANAGERCLIENT_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }

    if (SpeechVMRecorder::GetInstance()->GetVMRecordStatus() == true)
    {
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

        memset(buffer, 0, bytes);
        usleep(30 * 1000);
        return bytes;
    }

    Mutex::Autolock _l(mLock);

    if (mStarting == false)
    {
        mStarting = true;
        RequesetRecordclock();
        ALOGD("read mStarting == false , first start");

        char Buf[10];
        sprintf(Buf, "%d.pcm", DumpFileNum);

        DumpFileName =String8(streamin);
        DumpFileName.append ((const char*)Buf);
        mPAdcPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);

        DumpFileName =String8(streaminOri);
        DumpFileName.append ((const char*)Buf);
        mPAdcPCMInDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);

        DumpFileNum++;
        DumpFileNum %= MAX_DUMP_NUM;

        // start to open record
        switch (mAudioResourceManager->GetAudioMode())
        {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:
        {
            mStreamInClient = mStreamInManager->RequestClient();
            if (mStreamInClient == NULL)
            {
                ALOGE("read mStarting = false cannot get mStreamInClient");
            }
            mStreamInClient->mAttributeClient = &mAttribute; // set attribute with request client
            SetClientSourceandMemType(mStreamInClient); // make sure mStreamInClient mem, set to digital CONTROL_IFACE_PATH
            ALOGD("mStreamInManager Do_input_start");
            mStreamInManager->Do_input_start(mStreamInClient);
            AudioVUnlockDL* VUnlockhdl = AudioVUnlockDL::getInstance();
            if(VUnlockhdl !=NULL) {
                VUnlockhdl->GetUplinkSystemTime();
            }
            StreamInSRC_Init();

#ifdef MTK_DUAL_MIC_SUPPORT
            mLRChannelSwitch = mAudioSpeechEnhanceInfoInstance->GetRecordLRChannelSwitch();
            mSpecificMicChoose = mAudioSpeechEnhanceInfoInstance->GetUseSpecificMIC();
#endif

#ifdef MTK_AUDIO_HD_REC_SUPPORT
            int modeIndex;
            bool forceupdate = ((mAttribute.mSource == AUDIO_SOURCE_VOICE_COMMUNICATION)||
                                (mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)||
                                (mAttribute.mPredevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET));
            modeIndex = CheckHDRecordMode();
            if ((modeIndex != mHDRecordModeIndex) && (modeIndex >= 0) ||forceupdate)
            {
                ALOGD("modeIndex=%d, forceupdate=%d",modeIndex,forceupdate);
                if(modeIndex >= 0)
                    mHDRecordModeIndex = modeIndex;
                ConfigHDRecordParams(SPE_MODE_REC);
            }

            if((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) ||mHDRecTunning16K)
            {
                // set blisrc 48k->16k and 16k->48k
                const uint16_t num_sample_rate = mAttribute.mSampleRate;
                const uint8_t num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
                uint16_t process_sample_rate = HD_RECORD_SAMPLE_RATE;
                if((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION)||mHDRecTunning16K)
                    process_sample_rate = VOICE_RECOGNITION_RECORD_SAMPLE_RATE;

                 ALOGD("need src? ori_sample_rate=%d, process_sample_rate=%d",num_sample_rate,process_sample_rate);
                if (process_sample_rate != num_sample_rate)
                {
                    uint32_t srcBufLen = 0;
                    // Need SRC 48k->16k
                    BLI_GetMemSize(num_sample_rate, num_channel,
                                   process_sample_rate, num_channel,
                                   &srcBufLen);
                    mBliHandler1 =
                        BLI_Open(num_sample_rate, num_channel,
                                 process_sample_rate, num_channel,
                                 new char[srcBufLen], NULL);
                    mBliHandler1Buffer = new char[mAttribute.mBufferSize]; // tmp buffer for blisrc out
                    ASSERT(mBliHandler1Buffer != NULL);

                    // Need SRC 16k->48k
                    srcBufLen = 0;
                    BLI_GetMemSize(process_sample_rate, num_channel,
                                   num_sample_rate, num_channel,
                                   &srcBufLen);
                    mBliHandler2 =
                        BLI_Open(process_sample_rate, num_channel,
                                 num_sample_rate, num_channel,
                                 new char[srcBufLen], NULL);
                    mBliHandler2Buffer = new char[mAttribute.mBufferSize]; // tmp buffer for blisrc out
                    ASSERT(mBliHandler2Buffer != NULL);
                }
            }

            StartHDRecord(SPE_MODE_REC);
#endif
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2:
        {
            // request client for ring buffer
            mStreamInClient = mStreamInManager->RequestClient();
            mStreamInClient->mAttributeClient = &mAttribute; // set attribute with request client

            // set blisrc
            SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
            const uint16_t modem_num_sample_rate = pSpeechDriver->GetRecordSampleRate();
            const uint16_t modem_num_channel     = pSpeechDriver->GetRecordChannelNumber();

            const uint16_t target_num_sample_rate = mAttribute.mSampleRate;
            const uint16_t target_num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;

            if (modem_num_sample_rate != target_num_sample_rate ||
                modem_num_channel     != target_num_channel) // Need SRC
            {
                uint32_t srcBufLen = 0;
                BLI_GetMemSize(modem_num_sample_rate, modem_num_channel,
                               target_num_sample_rate, target_num_channel,
                               &srcBufLen);
                mStreamInClient->mBliHandlerBuffer =
                    BLI_Open(modem_num_sample_rate, modem_num_channel,
                             target_num_sample_rate, target_num_channel,
                             new char[srcBufLen], NULL);
            }

            // open modem record path
            mStreamInManager->Do_input_start(mStreamInClient);

            // No need extra AP side speech enhancement
            SetStreamInPreprocessStatus(false);
            break;
        }
        default:
            ALOGW("no mode is select!!");
            break;
        }

        ALOGD("read mStarting == false , first done");
    }

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);

    ASSERT(mStreamInClient != NULL);

    //ALOGD("mStreamInClient->mMemDataType=%d,mAttribute.mSampleRate=%d,mBliSrc->initStatus()=%d",mStreamInClient->mMemDataType,mAttribute.mSampleRate,mBliSrc->initStatus());
    if(StreamIn_NeedToSRC() == true)
    {
		StreamInSRC_Process(buffer, bytes);
    }
    else
    {
        //ALOGD("read(), No SRC path");
        char *pWrite = (char *)buffer;

        do
        {
            mStreamInClient->mLock.lock();
            RingBufferSize = RingBuf_getDataCount(&mStreamInClient->mRingBuf);
            //ALOGD("read RingBufferSize = %d TryCount = %d ReadDataBytes = %d mRingBuf = %p", RingBufferSize, TryCount, ReadDataBytes, &mStreamInClient->mRingBuf);
            if (RingBufferSize >= ReadDataBytes)   // ring buffer is enough, copy & exit
            {
                RingBuf_copyToLinear((char *)pWrite, &mStreamInClient->mRingBuf, ReadDataBytes);
                mStreamInClient->mLock.unlock();
                break;
            }
            else   // ring buffer is not enough, copy all data & wait
            {
                RingBuf_copyToLinear((char *)pWrite, &mStreamInClient->mRingBuf, RingBufferSize);
                ReadDataBytes -= RingBufferSize;
                pWrite += RingBufferSize;
            }
            //ALOGD("+read wait for mStreamInClient->mLock mStreamInClient = %p", mStreamInClient);
            if (mStreamInClient->mWaitWorkCV.waitRelative(mStreamInClient->mLock, milliseconds(300)) != NO_ERROR)
            {
                ALOGW("waitRelative fail");
                if (mStreamInClient != NULL)   // busy
                {
                    mStreamInClient->mLock.unlock();
                    break;
                }
                else   // die
                {
                    return bytes;
                }
            }
            //ALOGD("-read wait for mStreamInClient->mLock");
            mStreamInClient->mLock.unlock();
            TryCount--;
        }
        while (ReadDataBytes && TryCount);
    }

    if (mPAdcPCMInDumpFile)
    {
        long int position =0;
        position = ftell (mPAdcPCMInDumpFile);
        if(position> MAX_FILE_LENGTH)
        {
            rewind(mPAdcPCMInDumpFile);
        }
        AudioDumpPCMData((void *)buffer , bytes,mPAdcPCMInDumpFile);
    }

    if (mAudioResourceManager->IsModeIncall() == false) {
        StreamInPreprocess(buffer, bytes);
        CheckNeedDataConvert((short *)buffer,bytes);
    }

    if (mPAdcPCMDumpFile)
    {
        long int position =0;
        position = ftell (mPAdcPCMDumpFile);
        if(position> MAX_FILE_LENGTH)
        {
            rewind(mPAdcPCMDumpFile);
        }
        AudioDumpPCMData((void *)buffer , bytes,mPAdcPCMDumpFile);
#ifdef MTK_VOICEUNLOCK_DEBUG_ENABLE
        if(CheckRecordNoData((short *)buffer, bytes))
        {
            if(mNoDataCount >= 200)
            {
                ALOGW("No input data around 4sec, something wrong");
                if(mFd== 0)
                    mFd = ::open(kAudioDeviceName, O_RDWR);
                ::ioctl(mFd,AUDDRV_LOG_PRINT,0);
                ASSERT(0);
                mNoDataCount = 0;
            }
        }
#endif
    }

    return bytes;
}

#ifdef MTK_VOICEUNLOCK_DEBUG_ENABLE
bool AudioMTKStreamIn::CheckRecordNoData(short *buffer , uint32_t bytes)
{
    if(mAttribute.mSource == AUDIO_SOURCE_VOICE_UNLOCK)
    {
        int copysize = bytes>>2;    //stereo, 16bits
        short left = 0;
        short right = 0;

        while(copysize)
        {
            left = *(buffer);
            right = *(buffer+1);
            if((left !=0) ||(right!=0))
            {
                mNoDataCount == 0;
                return false;
            }
            buffer+=2;
            copysize--;
        }
        ALOGW("CheckRecordNoData no record data!!!");
        mNoDataCount++;
        return true;
    }
    return false;
}
#endif

status_t AudioMTKStreamIn::standby()
{
    //here to stanby input stream
    ALOGD("audioMTKStreamIn::standby()");
    SetSuspend (true);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK, 3000);
    mLock.lock ();
    standbyWithMode();
    AudioCloseDumpPCMFile(mPAdcPCMDumpFile);
    AudioCloseDumpPCMFile(mPAdcPCMInDumpFile);
    mLock.unlock ();
    SetSuspend (false);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGERCLIENT_LOCK);
    return NO_ERROR;
}

status_t AudioMTKStreamIn::standbyWithMode()
{
    if (mStarting == true)
    {
        mStarting = false;
        switch (mAudioResourceManager->GetAudioMode())
        {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:
        {
            mStreamInManager->Do_input_standby(mStreamInClient);
            mStreamInManager->ReleaseClient(mStreamInClient);
#ifdef MTK_AUDIO_HD_REC_SUPPORT
            StopHDRecord();
#endif
            if(mBliHandler1)
            {
                BLI_Close(mBliHandler1, NULL);
                delete[] mBliHandler1;
                mBliHandler1=NULL;
            }
            if(mBliHandler1Buffer)
            {
                delete[] mBliHandler1Buffer;
                mBliHandler1Buffer = NULL;
            }
            if(mBliHandler2)
            {
                BLI_Close(mBliHandler2, NULL);
                delete[] mBliHandler2;
                mBliHandler2=NULL;
            }
            if(mBliHandler2Buffer)
            {
                delete[] mBliHandler2Buffer;
                mBliHandler2Buffer = NULL;
            }
            if (mBliSrc)
            {
                mBliSrc->close();
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2:
        {
            mStreamInManager->Do_input_standby(mStreamInClient);
            if (mStreamInClient->mBliHandlerBuffer != NULL) {
                BLI_Close(mStreamInClient->mBliHandlerBuffer, NULL);
                delete[] mStreamInClient->mBliHandlerBuffer;
            }
            mStreamInManager->ReleaseClient(mStreamInClient);
            break;
        }
        default:
        {
            break;
        }
        }
        ReleaseRecordclock();
    }
    return NO_ERROR;
}

status_t  AudioMTKStreamIn::setParameters(const String8 &keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key_routing = String8(AudioParameter::keyRouting);
    String8 key_input_source = String8(AudioParameter::keyInputSource);

    status_t status = NO_ERROR;
    int device;
    int input_source;
    bool InputSourceChange = false;
    ALOGD("+setParameters() %s", keyValuePairs.string());
    mLock.lock ();

    if (param.getInt(key_input_source, input_source) == NO_ERROR)
    {
        ALOGD("setParameters, input_source(%d)", input_source);
        if (mAttribute.mSource != input_source)
        {
            mAttribute.mSource = input_source;
            InputSourceChange = true;
        }
        switch (input_source)
        {
        case AUDIO_SOURCE_DEFAULT:
            input_source = AUDIO_SOURCE_MIC; // set default as mic source.
        case AUDIO_SOURCE_MIC:
        case AUDIO_SOURCE_CAMCORDER:
        case AUDIO_SOURCE_VOICE_UPLINK:
        case AUDIO_SOURCE_VOICE_DOWNLINK:
        case AUDIO_SOURCE_VOICE_CALL:
        case AUDIO_SOURCE_VOICE_COMMUNICATION:
            SetStreamInPreprocessStatus(true);
            break;
        case AUDIO_SOURCE_VOICE_RECOGNITION:
            // when recording use voice recognition , not to do pre processing
            //SetStreamInPreprocessStatus(false);
            SetStreamInPreprocessStatus(true);
            break;
        case AUDIO_SOURCE_VOICE_UNLOCK:
            // when voice unlock , not to do pre processing
            SetStreamInPreprocessStatus(false);
            break;
        case AUDIO_SOURCE_CUSTOMIZATION1:
        case AUDIO_SOURCE_CUSTOMIZATION2:
        case AUDIO_SOURCE_CUSTOMIZATION3:
            SetStreamInPreprocessStatus(true);
            break;
        case AUDIO_SOURCE_MATV:
        case AUDIO_SOURCE_FM:
            SetStreamInPreprocessStatus(false);
            #ifdef MTK_DIGITAL_MIC_SUPPORT        //for digital MIC sample rate constrain
            mAttribute.mIsDigitalMIC = false;
            #endif
            break;
        default:
            SetStreamInPreprocessStatus(false);
            break;
        }
        mAudioResourceManager->setUlInputSource(input_source);
        param.remove(key_input_source);
    }

    if (param.getInt(key_routing, device) == NO_ERROR)
    {
        if ((device !=  mAttribute.mdevices) && device)
        {
            if (mStarting == true)
            {
                if (CheckMemTypeChange(mAttribute.mdevices, device))
                {
                    standbyWithMode(); // standby this stream ,wait for read to reopen hardware again
                }

                mAudioResourceManager->SelectInputDevice(device);
            }
            else
            {
                mAudioResourceManager->setUlInputDevice(device);
            }

            // update device for streamclient
            mAttribute.mPredevices = mAttribute.mdevices;
            mAttribute.mdevices = device;
            //SetClientSourceandMemType(mStreamInClient);
#ifdef MTK_AUDIO_HD_REC_SUPPORT
            //restart the preprocess to apply new MIC gain if device change
            if (IsHDRecordRunning())
            {
                int modeIndex;
                bool forceupdate = ((mAttribute.mSource == AUDIO_SOURCE_VOICE_COMMUNICATION)||
                                    (mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)||
                                    (mAttribute.mPredevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET));
                modeIndex = CheckHDRecordMode();
                if (((modeIndex != mHDRecordModeIndex) && (modeIndex >= 0)) || forceupdate)
                {
                    ALOGD("modeIndex=%d, forceupdate=%d",modeIndex,forceupdate);
                    if(modeIndex >= 0)
                        mHDRecordModeIndex = modeIndex;
                    ConfigHDRecordParams(SPE_MODE_REC);
                }
                    StopHDRecord();
                    StartHDRecord(SPE_MODE_REC);
            }
#endif
        }
        else
        {
            mAudioResourceManager->setUlInputDevice(device); // Set Input device in AudioResource Manager
            ALOGD("set parameters with same input device", device);
        }
        ALOGD("setParameters, device(0x%x)", device);
        param.remove(key_routing);
    }

    if (param.size())
    {
        status = BAD_VALUE;
    }
    mLock.unlock ();
    ALOGD("-setParameters()");
    return status;
}

bool  AudioMTKStreamIn::CheckMemTypeChange(uint32_t oldDevice, uint32_t newDevice)
{
    uint32_t OldMemType = 0, NewMemType = 0;
    OldMemType = GetMemTypeByDevice(oldDevice);
    NewMemType = GetMemTypeByDevice(newDevice);
    ALOGD("oldDevice = 0x%x newDevice = 0x%x ", oldDevice, newDevice);
    return (OldMemType != NewMemType);
}

uint32_t  AudioMTKStreamIn::GetMemTypeByDevice(uint32_t Device)
{
    switch (Device)
    {
    case AUDIO_DEVICE_IN_AMBIENT :
    case AUDIO_DEVICE_IN_BUILTIN_MIC :
    case AUDIO_DEVICE_IN_WIRED_HEADSET :
    case AUDIO_DEVICE_IN_BACK_MIC :
    case AUDIO_DEVICE_IN_COMMUNICATION :
    case AUDIO_DEVICE_IN_VOICE_CALL:
        return AudioDigitalType::MEM_VUL;
        // for BT scenario , it's a special case need to modify source to DAI_BT
    case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET :
        return AudioDigitalType::MEM_DAI;
    case AUDIO_DEVICE_IN_AUX_DIGITAL :
    case AUDIO_DEVICE_IN_AUX_DIGITAL2:
        return AudioDigitalType::MEM_AWB;
    default:
        ALOGW("no proper device match !!!");
        return AudioDigitalType::MEM_VUL;
    }
}

String8 AudioMTKStreamIn::getParameters(const String8 &keys)
{
    ALOGD("AudioMTKHardware getParameters\n");
    AudioParameter param = AudioParameter(keys);
    return param.toString();
}

unsigned int AudioMTKStreamIn::getInputFramesLost() const
{
    //here need to implement frame lost , now not support
    return 0;
}

size_t  AudioMTKStreamIn::GetfixBufferSize(uint32_t sampleRate, int format, int channelCount)
{
    //for 8192 bytes ==> 48K , stereo , channel;
    size_t FixBufferSize =0;
    FixBufferSize = Default_Mic_Buffer ;
    ALOGD("GetfixBufferSize FixBufferSize = %d",FixBufferSize);
    return FixBufferSize;
}

bool AudioMTKStreamIn::MutexLock(void)
{
    mLock.lock();
    return true;
}
bool AudioMTKStreamIn::MutexUnlock(void)
{
    mLock.unlock();
    return true;
}

status_t AudioMTKStreamIn::addAudioEffect(effect_handle_t effect)
{
#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
    int status;
    effect_descriptor_t desc;
    const uint8_t num_channel = (channels() == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
    MutexLock();
    if (mAPPS->num_preprocessors >= MAX_PREPROCESSORS)
    {
        status = -ENOSYS;
        goto exit;
    }

    status = (*effect)->get_descriptor(effect, &desc);
    if (status != 0)
        goto exit;

    mAPPS->preprocessors[mAPPS->num_preprocessors].effect_itfe = effect;
    /* add the supported channel of the effect in the channel_configs */
    //in_read_audio_effect_channel_configs(&preprocessors[num_preprocessors]);

    mAPPS->num_preprocessors++;

    /* check compatibility between main channel supported and possible auxiliary channels */
    //in_update_aux_channels(effect);

    ALOGD("addAudioEffect, effect type: %08x", desc.type.timeLow);
    //echo reference
    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0)
    {
        mAPPS->need_echo_reference = true;
        ALOGD("find AEC: %x",mStarting);
        if(mStarting)
        {
            if (mEcho_Reference != NULL)
            {
                MutexUnlock();
                standby();
                MutexLock();
                /* stop reading from echo reference */
                mAPPS->stop_echo_reference(mEcho_Reference);
                mEcho_Reference = NULL;
            }
            else
            {
                ALOGD("open AEC even record is already start");
                mEcho_Reference = mAPPS->start_echo_reference(
                                      AUDIO_FORMAT_PCM_16_BIT,
                                      num_channel,
                                      sampleRate());
            }
        }
        mAPPS->in_configure_reverse(num_channel,sampleRate());
    }

exit:
    MutexUnlock();
    ALOGD_IF(status != 0, "addAudioEffect error %d", status);
    return status;
#else
    return -ENOSYS;
#endif

}

status_t AudioMTKStreamIn::removeAudioEffect(effect_handle_t effect)
{
#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
    int i;
    int status = -EINVAL;
    effect_descriptor_t desc;
    MutexLock();
    if (mAPPS->num_preprocessors <= 0)
    {
        status = -ENOSYS;
        goto exit;
    }

    for (i = 0; i < mAPPS->num_preprocessors; i++)
    {
        if (status == 0)   /* status == 0 means an effect was removed from a previous slot */
        {
            mAPPS->preprocessors[i - 1].effect_itfe = mAPPS->preprocessors[i].effect_itfe;
            mAPPS->preprocessors[i - 1].channel_configs = mAPPS->preprocessors[i].channel_configs;
            mAPPS->preprocessors[i - 1].num_channel_configs = mAPPS->preprocessors[i].num_channel_configs;
            ALOGD("in_remove_audio_effect moving fx from %d to %d", i, i - 1);
            continue;
        }
        if (mAPPS->preprocessors[i].effect_itfe == effect)
        {
            ALOGD("in_remove_audio_effect found fx at index %d", i);
            //            free(preprocessors[i].channel_configs);
            status = 0;
        }
    }

    if (status != 0)
        goto exit;

    mAPPS->num_preprocessors--;
    /* if we remove one effect, at least the last preproc should be reset */
    mAPPS->preprocessors[mAPPS->num_preprocessors].num_channel_configs = 0;
    mAPPS->preprocessors[mAPPS->num_preprocessors].effect_itfe = NULL;
    mAPPS->preprocessors[mAPPS->num_preprocessors].channel_configs = NULL;


    /* check compatibility between main channel supported and possible auxiliary channels */
    //in_update_aux_channels(in, NULL);

    status = (*effect)->get_descriptor(effect, &desc);
    if (status != 0)
        goto exit;

    ALOGD("in_remove_audio_effect(), effect type: %08x", desc.type.timeLow);

    //echo reference
    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0)
    {
        mAPPS->need_echo_reference = false;
        if(mStarting)
        {
            if (mEcho_Reference != NULL)
            {
                MutexUnlock();
                standby();
                MutexLock ();
                /* stop reading from echo reference */
                mAPPS->stop_echo_reference(mEcho_Reference);
                mEcho_Reference = NULL;
            }
        }
    }

exit:

    MutexUnlock();
    ALOGD_IF(status != 0, "remove_audio_effect() error %d", status);
    return status;
#else
    return -ENOSYS;
#endif


}

status_t AudioMTKStreamIn::SetSuspend(bool suspend)
{
    if (suspend)
    {
        mSuspend++;
    }
    else
    {
        mSuspend--;
        if(mSuspend <0)
        {
            ALOGW("mSuspend = %d",mSuspend);
            mSuspend =0;
        }
    }
    return NO_ERROR;
}

AudioMTKStreamIn::BliSrc::BliSrc()
    : mHandle(NULL), mBuffer(NULL), mInitCheck(NO_INIT)
{
}

AudioMTKStreamIn::BliSrc::~BliSrc()
{
    close();
}

status_t AudioMTKStreamIn::BliSrc::initStatus()
{
    return mInitCheck;
}

status_t  AudioMTKStreamIn::BliSrc::init(uint32 inSamplerate, uint32 inChannel, uint32 OutSamplerate, uint32 OutChannel)
{
    if (mHandle == NULL)
    {
        uint32_t workBufSize;
        BLI_GetMemSize(inSamplerate, inChannel, OutSamplerate, OutChannel, &workBufSize);
        ALOGD("BliSrc::init InputSampleRate=%u, inChannel=%u, OutputSampleRate=%u, OutChannel=%u, mWorkBufSize = %u",
              inSamplerate, inChannel, OutSamplerate, OutChannel, workBufSize);
        mBuffer = new uint8_t[workBufSize];
        if (!mBuffer)
        {
            ALOGE("BliSrc::init Fail to create work buffer");
            return NO_MEMORY;
        }
        memset((void *)mBuffer, 0, workBufSize);
        mHandle = BLI_Open(inSamplerate, inChannel, OutSamplerate, OutChannel, (char *)mBuffer, NULL);
        if (!mHandle)
        {
            ALOGE("BliSrc::init Fail to get blisrc handle");
            if (mBuffer)
            {
                delete []mBuffer;
                mBuffer = NULL;
            }
            return NO_INIT;
        }
        mInitCheck = OK;
    }
    return NO_ERROR;

}

size_t AudioMTKStreamIn::BliSrc::process(const void *inbuffer, size_t *inBytes, void *outbuffer, size_t *outBytes)
{
    if (mHandle)
    {
        size_t consume = BLI_Convert(mHandle, (short *)inbuffer, inBytes, (short *)outbuffer, outBytes);
        ALOGD_IF(consume != *inBytes, "inputLength=%d,consume=%d,outputLength=%d", *inBytes, consume, *outBytes);
        return consume;
    }
    ALOGW("BliSrc::process src not initialized");
    return 0;
}

status_t  AudioMTKStreamIn::BliSrc::close(void)
{
    if (mHandle)
    {
        BLI_Close(mHandle, NULL);
        mHandle = NULL;
    }
    if (mBuffer)
    {
        delete []mBuffer;
        mBuffer = NULL;
    }
    mInitCheck = NO_INIT;
    return NO_ERROR;
}



void AudioMTKStreamIn::CheckNeedDataConvert(short * buffer, ssize_t bytes)
{
#ifdef MTK_DUAL_MIC_SUPPORT
//specific MIC choose or LR channel Switch
    short left , right;
    int copysize = bytes>>2;    //stereo, 16bits

    char value[PROPERTY_VALUE_MAX];
    property_get("streamin.micchoose", value, "0");
    int bflag = atoi(value);

    char value1[PROPERTY_VALUE_MAX];
    property_get("streamin.LRSwitch", value1, "0");
    int bflag1 = atoi(value1);

//    ALOGD("mbLRChannelSwitch=%d, miSpecificMicChoose=%d,bflag %d, bflag1 %d",mLRChannelSwitch,mSpecificMicChoose,bflag,bflag1);

    if(mSpecificMicChoose==1 || bflag==1)    //use main MIC
    {
        while(copysize)
        {
            left = *(buffer);
            *(buffer) = left;
            *(buffer+1) = left;
            buffer+=2;
            copysize--;
        }
    }
    else if(mSpecificMicChoose==2 || bflag==2)   //use ref MIC
    {
        while(copysize)
        {
            right = *(buffer+1);
            *(buffer) = right;
            *(buffer+1) = right;
            buffer+=2;
            copysize--;
        }
    }
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    if((bflag1||mLRChannelSwitch) && (mSpecificMicChoose==0 && bflag!=1&& bflag!=2) && mStereoMode)   //channel switch
#else
    if((bflag1||mLRChannelSwitch) && (mSpecificMicChoose==0 && bflag!=1&& bflag!=2))   //channel switch
#endif
    {
        while(copysize)
        {
            left = *(buffer);
            right = *(buffer+1);
            *(buffer) = right;
            *(buffer+1) = left;
            buffer+=2;
            copysize--;
        }
    }
#endif

//Stereo/mono convert
    const uint8_t num_channel = (mAttribute.mChannels == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
//    ALOGD("num_channel=%d, mStereoMode=%d",num_channel,mStereoMode);
#ifndef AUDIO_HQA_SUPPORT
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    //need return stereo data
    if(num_channel==2)
    {
        if(!mStereoMode)    //speech enhancement output data is mono, need to convert to stereo
        {
            short left;
            int copysize = bytes>>2;

            while(copysize)     //only left channel data is processed
            {
                left = *(buffer);
                *(buffer) = left;
                *(buffer+1) = left;
                buffer+=2;
                copysize--;
            }
        }
    }
#endif
#endif

}

void AudioMTKStreamIn::StereoToMono(short *buffer , int length)
{
    short left , right;
    int sum;
    int copysize = length>>2;

    while(copysize)
    {
        left = *(buffer);
        right = *(buffer+1);
        sum = (left+right) >> 1;
        *(buffer) = (short)sum;
        *(buffer+1) = (short)sum;
        buffer+=2;
        copysize--;
    }
}

#ifdef NATIVE_AUDIO_PREPROCESS_ENABLE
ssize_t	AudioMTKStreamIn::NativeRecordPreprocess(void *buffer, ssize_t bytes)
{

    if(mAPPS==NULL)
    {
        return bytes;
    }

    if(mAPPS->num_preprocessors==0)
    {
        return bytes;
    }
    else
    {
        ssize_t frames_wr = 0;
        audio_buffer_t in_buf;
        audio_buffer_t out_buf;
        const uint8_t num_channel = (channels() == AUDIO_CHANNEL_IN_STEREO) ? 2 : 1;
        ssize_t frames = bytes/sizeof(int16_t)/num_channel;
        ssize_t needframes = frames + mAPPS->proc_buf_frames;
        int i;

        //ALOGD("process_frames(): %d bytes, %d frames, proc_buf_frames=%d, mAPPS->num_preprocessors=%d,num_channel=%d", bytes,frames,mAPPS->proc_buf_frames,mAPPS->num_preprocessors,num_channel);
        mAPPS->proc_buf_out = (int16_t *)buffer;

        if ((mAPPS->proc_buf_size < (size_t)needframes) || (mAPPS->proc_buf_in==NULL))
        {
            mAPPS->proc_buf_size = (size_t)needframes;
            mAPPS->proc_buf_in = (int16_t *)realloc(mAPPS->proc_buf_in, mAPPS->proc_buf_size*num_channel*sizeof(int16_t));
            //mpPreProcessIn->proc_buf_out = (int16_t *)realloc(mpPreProcessIn->proc_buf_out, mpPreProcessIn->proc_buf_size*mChNum*sizeof(int16_t));

            if(mAPPS->proc_buf_in == NULL)
            {
                ALOGW("proc_buf_in realloc fail");
                return bytes;
            }
            ALOGD("process_frames(): proc_buf_in %p extended to %d bytes",mAPPS->proc_buf_in, mAPPS->proc_buf_size*sizeof(int16_t)*num_channel);
        }

        memcpy(mAPPS->proc_buf_in + mAPPS->proc_buf_frames * num_channel, buffer, bytes);

        mAPPS->proc_buf_frames += frames;

        while (frames_wr < frames)
        {
            //AEC
            if (mEcho_Reference != NULL)
                mAPPS->push_echo_reference(mAPPS->proc_buf_frames);
            else
            {
                //prevent start_echo_reference fail previously due to output not enable
                if(mAPPS->need_echo_reference)
                {
                    ALOGD("try start_echo_reference");
                    mEcho_Reference = mAPPS->start_echo_reference(
                                          AUDIO_FORMAT_PCM_16_BIT,
                                          num_channel,
                                          sampleRate());
                }
            }

            /* in_buf.frameCount and out_buf.frameCount indicate respectively
                  * the maximum number of frames to be consumed and produced by process() */
            in_buf.frameCount = mAPPS->proc_buf_frames;
            in_buf.s16 = mAPPS->proc_buf_in;
            out_buf.frameCount = frames - frames_wr;
            out_buf.s16 = (int16_t *)mAPPS->proc_buf_out + frames_wr * num_channel;

            /* FIXME: this works because of current pre processing library implementation that
                 * does the actual process only when the last enabled effect process is called.
                 * The generic solution is to have an output buffer for each effect and pass it as
                 * input to the next.
                 */

            for (i = 0; i < mAPPS->num_preprocessors; i++)
            {
                (*mAPPS->preprocessors[i].effect_itfe)->process(mAPPS->preprocessors[i].effect_itfe,
                        &in_buf,
                        &out_buf);
            }

            /* process() has updated the number of frames consumed and produced in
                  * in_buf.frameCount and out_buf.frameCount respectively
                 * move remaining frames to the beginning of in->proc_buf_in */
            mAPPS->proc_buf_frames -= in_buf.frameCount;

            if (mAPPS->proc_buf_frames)
            {
                memcpy(mAPPS->proc_buf_in,
                       mAPPS->proc_buf_in + in_buf.frameCount * num_channel,
                       mAPPS->proc_buf_frames * num_channel * sizeof(int16_t));
            }

            /* if not enough frames were passed to process(), read more and retry. */
            if (out_buf.frameCount == 0)
            {
                ALOGV("No frames produced by preproc");
                break;
            }

            if ((frames_wr + (ssize_t)out_buf.frameCount) <= frames)
            {
                frames_wr += out_buf.frameCount;
                ALOGV("out_buf.frameCount=%d,frames_wr=%d",out_buf.frameCount,frames_wr);
            }
            else
            {
                /* The effect does not comply to the API. In theory, we should never end up here! */
                ALOGE("preprocessing produced too many frames: %d + %d  > %d !",
                      (unsigned int)frames_wr, out_buf.frameCount, (unsigned int)frames);
                frames_wr = frames;
            }
        }
//        ALOGD("frames_wr=%d, bytes=%d",frames_wr,frames_wr*num_channel*sizeof(int16_t));
        return frames_wr*num_channel*sizeof(int16_t);
    }

}
#endif


#ifdef MTK_AUDIO_HD_REC_SUPPORT
#define NORMAL_RECORDING_DEFAULT_MODE    (1)
#define VOICE_REC_RECORDING_DEFAULT_MODE (0)
#define VOICE_UnLock_RECORDING_DEFAULT_MODE (6)


void AudioMTKStreamIn::LoadHDRecordParams(void)
{
    uint8_t modeIndex;
    uint8_t total_num_scenes = MAX_HD_REC_SCENES;
    ALOGD("LoadHdRecordParams");
    // get scene table
    if(GetHdRecordSceneTableFromNV(&mhdRecordSceneTable)==0)
    {
        ALOGD("GetHdRecordSceneTableFromNV fail, use default value");
        memcpy(&mhdRecordSceneTable,&DefaultRecordSceneTable,sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    }

    // get hd rec param
    if(GetHdRecordParamFromNV(&mhdRecordParam)==0)
    {
        ALOGD("GetHdRecordParamFromNV fail, use default value");
        memcpy(&mhdRecordParam,&DefaultRecordParam,sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
    }

    //get hd rec 48k FIR param
//    GetHdRecord48kParamFromNV(&mhdRecord48kParam);

    for (int i = 0; i < total_num_scenes; i++)
    {
        // Handset
        modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HANDSET];
        if (modeIndex != 0xFF)
        {
            mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_NORMAL;
#if defined(MTK_DUAL_MIC_SUPPORT)
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HANDSET_MIC2;
#else
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_NORMAL;
#endif
            mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPEECH_MODE_NORMAL;
        }

        // Headset
        modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HEADSET];
        if (modeIndex != 0xFF)
        {
            mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_HEADSET;
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HEADSET;
            mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPEECH_MODE_EARPHONE;
        }

    }
    // specific for voice recognitaion

    // fix me , need hardware core for voice rec??
    int i = VOICE_REC_RECORDING_DEFAULT_MODE;
    // Handset
    modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HANDSET];
    ALOGD("modeIndex = %d", modeIndex);
    if (modeIndex != 0xFF)
    {
        mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
        mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_VOICE_REC;

        mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPEECH_MODE_NORMAL;
    }

    // Headset
    modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HEADSET];
    if (modeIndex != 0xFF)
    {
        mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
        mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
        mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPEECH_MODE_EARPHONE;
    }

#if 1 // Debug print
    for (int i = 0; i < total_num_scenes; i++)
        for (int j = 0; j < NUM_HD_REC_DEVICE_SOURCE; j++)
            ALOGD("vGetHdRecordModeInfo, scene_table[%d][%d] = %d", i, j, mhdRecordSceneTable.scene_table[i][j]);
#endif
}

int AudioMTKStreamIn::CheckHDRecordMode(void)
{
    if (mAudioResourceManager->IsModeIncall() == true)
    {
        ALOGD("CheckHDRecordMode, IsModeIncall = true, return ");
        return -1;
    }

    // HD Record
    uint8_t modeIndex = 0;
    int32 u4SceneIdx = 0;

    if (!GetHdRecordModeInfo(&modeIndex))   //no scene/mode get
    {
        ALOGD("CheckHDRecordMode mAttribute.mdevices=%x, mAttribute.mPredevices=%x, mHDRecordSceneIndex = %d ", mAttribute.mdevices,mAttribute.mPredevices,mHDRecordSceneIndex);
        //can not get match HD record mode, use the default one
        // check if 3rd party camcorder
        if (mAttribute.mSource != AUDIO_SOURCE_CAMCORDER)   //not camcorder
        {
            if(mAttribute.mdevices != mAttribute.mPredevices)   //device changed, use previous scene (since scene not changed)
            {
                if(mHDRecordSceneIndex==-1)
                    mHDRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
            else
            {
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else    //default use internal one
                {
                    modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
        }
        else  //camcoder
        {
            u4SceneIdx = mhdRecordSceneTable.num_voice_rec_scenes + NORMAL_RECORDING_DEFAULT_MODE;//1:cts verifier offset
            if(mAttribute.mdevices != mAttribute.mPredevices)   //device changed, use previous scene
            {
                if(mHDRecordSceneIndex==-1)
                    mHDRecordSceneIndex = u4SceneIdx;
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else
                {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
            else
            {
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)  //headset
                {
                    modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
                }
                else    //default use internal one
                {
                    modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            }
        }

#if defined(MTK_DUAL_MIC_SUPPORT)
        //also need to configure the channel when use default mode
        /* only stereo flag is true, the stereo record is enabled */
        if (mAttribute.mdevices ==  AUDIO_DEVICE_IN_BUILTIN_MIC)   //handset
        {
            if (mhdRecordParam.hd_rec_map_to_stereo_flag[modeIndex] != 0)
            {
                mStereoMode = true;
            }
        }
#endif
    }
    ALOGD("SetHdrecordingMode,modeIndex=%d,mdevices=%d", modeIndex, mAttribute.mdevices);
    return modeIndex;
}

bool AudioMTKStreamIn::GetHdRecordModeInfo(uint8_t *modeIndex)
{
    bool  ret = false;
    int32 u4SceneIdx = mAudioSpeechEnhanceInfoInstance->GetHDRecScene();
    mAudioSpeechEnhanceInfoInstance->ResetHDRecScene();
    mStereoMode = false;

    ALOGD("+GetHdRecordModeInfo: u4SceneIdx = %d", u4SceneIdx);
    //special input source case
    if((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) ||mHDRecTunning16K)
    {
        ALOGD("voice recognition case");
        u4SceneIdx = VOICE_REC_RECORDING_DEFAULT_MODE;
    }
    else if(mAttribute.mSource == AUDIO_SOURCE_VOICE_UNLOCK)
    {
        ALOGD("voice unlock case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE;
    }
    else if(mAttribute.mSource == AUDIO_SOURCE_CUSTOMIZATION1)
    {
        ALOGD("CUSTOMIZATION1 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 1;
    }
    else if(mAttribute.mSource == AUDIO_SOURCE_CUSTOMIZATION2)
    {
        ALOGD("CUSTOMIZATION2 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 2;
    }
    else if(mAttribute.mSource == AUDIO_SOURCE_CUSTOMIZATION3)
    {
        ALOGD("CUSTOMIZATION3 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 3;
    }

    //propitary API case
    if ((u4SceneIdx >= 0) && (u4SceneIdx<MAX_HD_REC_SCENES))
    {
        // get mode index
        if (mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF
                && mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: HEADSET,  modeIndex = %d", *modeIndex);
        }
        // Handset Mic
        else if (mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: HANDSET,  modeIndex = %d", *modeIndex);
#if defined(MTK_DUAL_MIC_SUPPORT)
            /* only stereo flag is true, the stereo record is enabled */
            if (mhdRecordParam.hd_rec_map_to_stereo_flag[*modeIndex] != 0)
            {
                mStereoMode = true;
            }
#endif
        }
        else
        {
            ALOGD("(RecOpen)vGetHdRecordModeInfo: Handset mode index shoule not be -1");
        }

#if 1 // Debug print
        ALOGD("(RecOpen)vGetHdRecordModeInfo: map_fir_ch1=%d, map_fir_ch2=%d, device_mode=%d",
              mhdRecordParam.hd_rec_map_to_fir_for_ch1[*modeIndex],
              mhdRecordParam.hd_rec_map_to_fir_for_ch2[*modeIndex],
              mhdRecordParam.hd_rec_map_to_dev_mode[*modeIndex]);
#endif
        mHDRecordSceneIndex = u4SceneIdx;
        ret = true;
    }
    else
    {
        *modeIndex = 0;
        ret = false;
    }

    ALOGD("(RecOpen)-vGetHdRecordModeInfo: ENUM_HD_Record_Mode = %d, mStereoMode = %d", *modeIndex, mStereoMode);
    return ret;
}

void AudioMTKStreamIn::ConfigHDRecordParams(SPE_MODE mode)
{
    bool ret = false;
    unsigned long HDRecordEnhanceParas[EnhanceParasNum] = {0};
    short HDRecordCompenFilter[CompenFilterNum] = {0};

    ALOGD("ConfigHDRecordParams,mStereoMode=%d, input source= %d, mdevices=%x", mStereoMode,mAttribute.mSource,mAttribute.mdevices);
    for (int i = 0; i < EnhanceParasNum; i++)   //EnhanceParasNum = 16+12(common parameters)
    {
        if (i < SPEECH_PARA_NUM)
        {
            //specific parameters
            if(mAttribute.mSource == AUDIO_SOURCE_VOICE_COMMUNICATION)
            {
                if(mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
                    HDRecordEnhanceParas[i] = mhdRecordParam.hd_rec_speech_mode_para[HD_REC_MODE_INDEX_NUM-1][i];
                else if(mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET)
                    HDRecordEnhanceParas[i] = mhdRecordParam.hd_rec_speech_mode_para[HD_REC_MODE_INDEX_NUM-2][i];
                else
                    HDRecordEnhanceParas[i] = mhdRecordParam.hd_rec_speech_mode_para[HD_REC_MODE_INDEX_NUM-3][i];
            }
            else
            {
                if(mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET){
                    HDRecordEnhanceParas[i] = mhdRecordParam.hd_rec_speech_mode_para[HD_REC_MODE_INDEX_NUM-1][i];
                }
                else
                    HDRecordEnhanceParas[i] = mhdRecordParam.hd_rec_speech_mode_para[mHDRecordModeIndex][i];
            }
        }
        else
        {
            //common parameters
            HDRecordEnhanceParas[i] = HDRecordEnhanceParasCommon[i - SPEECH_PARA_NUM];
        }
        ALOGD("HDRecordEnhanceParas[%d]=%d", i, HDRecordEnhanceParas[i]);
    }


    for (int i = 0; i < WB_FIR_NUM; i++)
    {
        HDRecordCompenFilter[i] = mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex]][i];
        ALOGD("HDRecordCompenFilter[%d]=%d", i, HDRecordCompenFilter[i]);
        if (mStereoMode) //stereo, DL2 use different FIR filter
            HDRecordCompenFilter[i + WB_FIR_NUM] = mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch2[mHDRecordModeIndex]][i];
        else    //mono, DL2 use the same FIR filter
            HDRecordCompenFilter[i + WB_FIR_NUM] = mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex]][i];
    }

    mpSPELayer->SetCompFilter(mode, HDRecordCompenFilter);

    mpSPELayer->SetEnhPara(mode, HDRecordEnhanceParas);
     //need to config as 16k sample rate for voice recognition
    if((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) ||mHDRecTunning16K)
    {
        mpSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
        mpSPELayer->SetAPPTable(mode, SPEECH_RECOGNITION);
    }
    else    //normal record  use 48k
    {
        mpSPELayer->SetSampleRate(mode, mAttribute.mSampleRate);
        if (mStereoMode)
            mpSPELayer->SetAPPTable(mode,STEREO_RECORD); //set library do stereo process
    	else
            mpSPELayer->SetAPPTable(mode,MONO_RECORD); //set library do mono process
    }

    //set MIC digital gain to library
    long gain = mAudioResourceManager->GetSwMICDigitalGain();
    if(mAttribute.mdevices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        gain = 0;
        ALOGD("BT path set Digital MIC gain = 0");
    }
    mpSPELayer->SetMICDigitalGain(mode, gain);

}

void AudioMTKStreamIn::StartHDRecord(SPE_MODE mode)
{
    if ((mAttribute.mdevices != AUDIO_DEVICE_IN_FM) && (mAttribute.mdevices !=AUDIO_DEVICE_IN_AUX_DIGITAL))
        mpSPELayer->Start(mode);
}

void AudioMTKStreamIn::StopHDRecord(void)
{
    if ((mAttribute.mdevices != AUDIO_DEVICE_IN_FM) && (mAttribute.mdevices !=AUDIO_DEVICE_IN_AUX_DIGITAL))
        mpSPELayer->Stop();
}

uint32_t AudioMTKStreamIn::HDRecordPreprocess(void *buffer , uint32_t bytes)
{
    if ((mAttribute.mdevices != AUDIO_DEVICE_IN_FM) && (mAttribute.mdevices !=AUDIO_DEVICE_IN_AUX_DIGITAL))
    {
        if((mAttribute.mSource == AUDIO_SOURCE_VOICE_RECOGNITION) ||mHDRecTunning16K)
        {
//            ALOGD("HDRecordPreprocess buffer=%p, bytes=%d", buffer, bytes);
            uint32 consume =0;
            size_t inputLength = bytes;
            size_t outputLength = bytes;
            //convert to 16k
            consume = BLI_Convert(mBliHandler1, (int16_t *)buffer, &inputLength, (int16_t *)mBliHandler1Buffer, &outputLength);
//            ALOGD("HDRecordPreprocess outputLength=%d,consume=%d", outputLength,consume);
            mpSPELayer->Process(UPLINK, (short *)mBliHandler1Buffer, outputLength);
            //convert back to 48k
            consume = BLI_Convert(mBliHandler2, (int16_t *)mBliHandler1Buffer, &outputLength, (int16_t *)buffer, &bytes);
//            ALOGD("HDRecordPreprocess bytes=%d,consume=%d", bytes,consume);
            return bytes;
        }
        else
        {
            mpSPELayer->Process(UPLINK, (short *)buffer, bytes);
            return bytes;
        }
    }
    return bytes;
}

bool AudioMTKStreamIn::IsHDRecordRunning(void)
{
    return mpSPELayer->IsSPERunning();
}

#endif

}

