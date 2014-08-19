#include "AudioMTKStreamInManager.h"
#include "AudioResourceManager.h"
#include "AudioResourceFactory.h"
#include "AudioAnalogType.h"
#include <utils/String16.h>
#include "AudioUtility.h"

#include "SpeechDriverFactory.h"

#ifdef MTK_AUDIO_HD_REC_SUPPORT
#include "AudioCustParam.h"
#endif

extern "C" {
#include "bli_exp.h"
}

#define LOG_TAG "AudioMTKStreamInManager"
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

#define RECORD_DROP_MS_MAX (200)
#define MTK_STREAMIN_VOLUEM_MAX (0x1000)
#define MTK_STREAMIN_VOLUME_VALID_BIT (12)
#define MAX_DUMP_NUM (6)
#define MAX_FILE_LENGTH (60000000)

namespace android
{

AudioMTKStreamInManager *AudioMTKStreamInManager::UniqueStreamInManagerInstance = NULL;
int AudioMTKStreamInManager::AudioMTkRecordThread::DumpFileNum =0;

#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
bool AudioMTKStreamInManager::checkFmMicConflict(AudioMTKStreamInClient *Client, bool status)
{
    bool FmMicConflict = false;
    uint32 sameGroupCount = 0;
    ALOGD("+%s(), Client->mAdcGroup = %d, mAudioInput.size() = %d\n", __FUNCTION__, Client->mAdcGroup, mAudioInput.size());

    if(AudioAnalogType::ADC_GROUP_LINE_IN != Client->mAdcGroup &&
        AudioAnalogType::ADC_GROUP_MIC_IN != Client->mAdcGroup) {
        return FmMicConflict;
    }

    for (int i = 0 ; i < mAudioInput.size() ; i ++) {
        AudioMTKStreamInClient *temp = mAudioInput.valueAt(i);
        if(temp->mAdcGroup == AudioAnalogType::ADC_GROUP_NONE)
            continue;
        if(Client->mAdcGroup != temp->mAdcGroup) {
            ALOGD("%s, find conflict about Mic & FM, device 0x%x - 0x%x", __FUNCTION__,
                    Client->mAttributeClient->mdevices, temp->mAttributeClient->mdevices);
            FmMicConflict = true;
        }
        else if(temp != Client){
            sameGroupCount++; // Other then current client, other clients with the same Group
        }
    }
    for (int i = 0 ; i < mAudioInput.size() ; i ++) {
        AudioMTKStreamInClient *temp = mAudioInput.valueAt(i);
        if(temp->mAdcGroup == AudioAnalogType::ADC_GROUP_NONE)
            continue;
        if(Client->mAdcGroup != temp->mAdcGroup) {
            if(temp->mAdcGroup == AudioAnalogType::ADC_GROUP_LINE_IN){
                if(status == false && sameGroupCount == 0){//When no other stream will conflict Line In
                    temp->mConflict = false;
                    ALOGD("%s, set FM stream as non-conflict device 0x%x ", __FUNCTION__,
                    temp->mAttributeClient->mdevices);
                }
                else if(status == true){
                    temp->mConflict = true;
                    ALOGD("%s, set FM stream as conflict device 0x%x ", __FUNCTION__,
                    temp->mAttributeClient->mdevices);
                }
            }
            else{
                Client->mConflict = status;
                break;
            }
        }
    }

    if(true == FmMicConflict && true == status) {
        ALOGD("%s, set drop count, device 0x%x", __FUNCTION__, Client->mAttributeClient->mdevices);
        Client->mLock.lock();
        Client->mDropCount = 3;
        Client->mLock.unlock();
    }
    return FmMicConflict;
}
#endif

AudioMTKStreamInManager *AudioMTKStreamInManager::getInstance()
{
    if (UniqueStreamInManagerInstance == NULL) {
        ALOGD("+AudioMTKStreamInManager");
        UniqueStreamInManagerInstance = new AudioMTKStreamInManager();
        ALOGD("-AudioMTKStreamInManager");
    }
    ALOGV("getInstance()");
    return UniqueStreamInManagerInstance;
}

void AudioMTKStreamInManager::freeInstance()
{
    return;
}

AudioMTKStreamInManager::AudioMTKStreamInManager()
{
    mClientNumber = 1  ;
    ALOGD("AudioMTKStreamInManager constructor");

    // allcoate buffer
    mAWBbuffer = new char[MemAWBBufferSize];
    mVULbuffer = new char[MemVULBufferSize];
    mDAIbuffer = new char[MemDAIBufferSize];
    mMODDAIbuffer = new char[MemDAIBufferSize];
    // get digital control
    mAudioDigital = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioAnalog =  AudioAnalogControlFactory::CreateAudioAnalogControl();
    mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();

    memset((void *)&mIncallRingBuffer, 0, sizeof(RingBuf));
    mIncallRingBuffer.pBufBase = new char[INCALL_RINGBUFFERE_SIZE];
    if (mIncallRingBuffer.pBufBase  == NULL) {
        ALOGW("mRingBuf.pBufBase allocate fail");
    }
    mIncallRingBuffer.bufLen = INCALL_RINGBUFFERE_SIZE;
    mIncallRingBuffer.pRead = mIncallRingBuffer.pBufBase ;
    mIncallRingBuffer.pWrite = mIncallRingBuffer.pBufBase ;
    mMicMute = false;
    mMuteTransition = false;

    PreLoadHDRecParams();
}

AudioMTKStreamInManager::~AudioMTKStreamInManager()
{
    ALOGD("AudioMTKStreamInManager destructor");
}

status_t  AudioMTKStreamInManager::initCheck()
{
    return NO_ERROR;
}

void AudioMTKStreamInManager::PreLoadHDRecParams(void)
{
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    ALOGD("PreLoadHDRecParams+++");
    //for NVRAM create file first to reserve the memory
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT DummyhdRecordSceneTable;
    AUDIO_HD_RECORD_PARAM_STRUCT DummyhdRecordParam;
    GetHdRecordSceneTableFromNV(&DummyhdRecordSceneTable);
    GetHdRecordParamFromNV(&DummyhdRecordParam);
    ALOGD("PreLoadHDRecParams---");
#endif
}

uint32_t AudioMTKStreamInManager::CopyBufferToClient(uint32 mMemDataType, void *buffer , uint32 copy_size)
{
    /*
    ALOGD("CopyBufferToClient mMemDataType = %d buffer = %p copy_size = %d mAudioInput.size() = %d buffer = 0x%x",
         mMemDataType, buffer, copy_size, mAudioInput.size(), *(unsigned int *)buffer);*/

    int ret = 0;
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, 200);
    if (ret) {
        ALOGW("EnableAudioLock AUDIO_STREAMINMANAGER_LOCK fail ret = %d", ret);
        return 0;
    }
    int FreeSpace = 0;
    for (int i = 0 ; i < mAudioInput.size() ; i ++) {
        AudioMTKStreamInClient *temp = mAudioInput.valueAt(i) ;
        if (temp->mMemDataType == mMemDataType && true == temp->mEnable) {
            temp->mLock.lock();
            FreeSpace =  RingBuf_getFreeSpace(&temp->mRingBuf);
            if (FreeSpace >= copy_size) {
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
                if(true == temp->mConflict || temp->mDropCount != 0) {
                    RingBuf_fillZero(&temp->mRingBuf, copy_size);
                    if(temp->mDropCount > 0) {
                        temp->mDropCount -= 1;
                        ALOGD("%s, drop count %d, device 0x%x", __FUNCTION__, temp->mDropCount, temp->mAttributeClient->mdevices);
                    }
                } else
#endif
                {
                    //ALOGD("1 RingBuf_copyToLinear FreeSpace = %d temp = %p copy_size = %d mRingBuf = %p", FreeSpace, temp, copy_size, &temp->mRingBuf);
                    RingBuf_copyFromLinear(&temp->mRingBuf, (char *)buffer, copy_size);
                }
            }
            else {
                // do not copy , let buffer keep going
            }
            temp->mWaitWorkCV.signal();
            temp->mLock.unlock();
        }
    }
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    return 0;
}

uint32_t AudioMTKStreamInManager::CopyBufferToClientIncall(RingBuf ul_ring_buf)
{
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, 1000);

    // get M2A share buffer record data
    const uint32_t kNumModemData = RingBuf_getDataCount(&ul_ring_buf);
    char *p_modem_data = new char[kNumModemData];
    RingBuf_copyToLinear(p_modem_data, &ul_ring_buf, kNumModemData);

    for (size_t i = 0; i < mAudioInput.size(); ++i) { // copy data to all clients
        AudioMTKStreamInClient *pClient = mAudioInput.valueAt(i) ;
        if (pClient->mEnable) {
            pClient->mLock.lock();

            uint32_t num_free_space = RingBuf_getFreeSpace(&pClient->mRingBuf);

            if (pClient->mBliHandlerBuffer == NULL) { // No need SRC
                //ASSERT(num_free_space >= kNumModemData);
                if (num_free_space < kNumModemData) {
                    ALOGW("%s(), num_free_space(%u) < kNumModemData(%u)", __FUNCTION__, num_free_space, kNumModemData);
                }
                else {
                    RingBuf_copyFromLinear(&pClient->mRingBuf, p_modem_data, kNumModemData);
                }
            }
            else { // Need SRC
                char *p_read = p_modem_data;
                uint32_t num_modem_left_data = kNumModemData;
                uint32_t num_converted_data = num_free_space; // max convert num_free_space

                p_read += BLI_Convert(pClient->mBliHandlerBuffer,
                                      (int16_t *)p_read, &num_modem_left_data,
                                      (int16_t *)pClient->mBliOutputLinearBuffer, &num_converted_data);
                SLOGV("%s(), num_modem_left_data = %u, num_converted_data = %u", __FUNCTION__, num_modem_left_data, num_converted_data);

                if (num_modem_left_data > 0) ALOGW("%s(), num_modem_left_data(%u) > 0", __FUNCTION__, num_modem_left_data);
                //ASSERT(num_modem_left_data == 0);

                RingBuf_copyFromLinear(&pClient->mRingBuf, pClient->mBliOutputLinearBuffer, num_converted_data);
                SLOGV("%s(), pRead:%u, pWrite:%u, dataCount:%u", __FUNCTION__,
                      pClient->mRingBuf.pRead - pClient->mRingBuf.pBufBase,
                      pClient->mRingBuf.pWrite - pClient->mRingBuf.pBufBase,
                      RingBuf_getDataCount(&pClient->mRingBuf));
            }

            pClient->mWaitWorkCV.signal();

            pClient->mLock.unlock();
        }
    }

    // free linear UL data buffer
    delete[] p_modem_data;

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    return 0;
}

status_t AudioMTKStreamInManager::StartModemRecord(AudioMTKStreamInClient *Client)
{
    // start modem record only for the first client
    if (mAudioInput.size() != 1) {
        ALOGW("%s(), mAudioInput.size() = %u != 1", __FUNCTION__, mAudioInput.size());
        return ALREADY_EXISTS;
    }
    else {
        return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->RecordOn();
    }
}

status_t AudioMTKStreamInManager::StopModemRecord()
{
    // stop modem record only for the last client
    if (mAudioInput.size() != 1) {
        ALOGW("%s(), mAudioInput.size() = %u != 1", __FUNCTION__, mAudioInput.size());
        return ALREADY_EXISTS;
    }
    else {
        return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->RecordOff();
    }
}

bool AudioMTKStreamInManager::checkMemInUse(AudioMTKStreamInClient *Client)
{
    ALOGD("checkMemInUse Client = %p", Client);
    for (int i = 0; i < mAudioInput.size(); i++) {
        AudioMTKStreamInClient *mTemp = mAudioInput.valueAt(i);
        // if mem is the same  and other client is enable , measn this mem is alreay on
        if (mTemp->mMemDataType == Client->mMemDataType && mTemp->mEnable) {
            ALOGD("vector has same memif in use Client->mem = ", Client->mMemDataType);
            return true;
        }
    }
    return false;
}

// this function should start a thread to read kernel sapce buffer. , base on memory type
status_t AudioMTKStreamInManager::StartStreamInThread(uint32 mMemDataType)
{

    ALOGD("StartStreamInThread mMemDataType = %d", mMemDataType);
    switch (mMemDataType) {
        case AudioDigitalType::MEM_VUL:
            mVULThread = new AudioMTkRecordThread(this, mMemDataType , mVULbuffer, MemVULBufferSize);
            if (mVULThread.get()) {
                mVULThread->run();
            }
            break;
        case AudioDigitalType::MEM_DAI:
            mDAIThread = new AudioMTkRecordThread(this, mMemDataType , mDAIbuffer, MemDAIBufferSize);
            if (mDAIThread.get()) {
                mDAIThread->run();
            }
            break;
        case AudioDigitalType::MEM_AWB:
            mAWBThread = new AudioMTkRecordThread(this, mMemDataType , mAWBbuffer, MemAWBBufferSize);

            if (mAWBThread.get()) {
                mAWBThread->run();
            }
            break;
        case AudioDigitalType::MEM_MOD_DAI:
            mMODDAIThread = new AudioMTkRecordThread(this, mMemDataType , mMODDAIbuffer, MemDAIBufferSize);
            if (mMODDAIThread.get()) {
                mMODDAIThread->run();
            }
            break;
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioMTKStreamInManager::AcquireMemBufferLock(AudioDigitalType::Digital_Block MemBlock, bool bEnable)
{
    switch (MemBlock) {
        case AudioDigitalType::MEM_VUL: {
            if (bEnable) {
                mAULBufferLock.lock();
            }
            else {

                mAULBufferLock.unlock();
            }
            break;
        }
        case AudioDigitalType::MEM_DAI: {
            if (bEnable) {
                mDAIBufferLock.lock();
            }
            else {
                mDAIBufferLock.unlock();
            }
            break;
        }
        case AudioDigitalType::MEM_AWB: {
            if (bEnable) {
                mAWBBufferLock.lock();
            }
            else {
                mAWBBufferLock.unlock();
            }
            break;
        }
        case AudioDigitalType::MEM_MOD_DAI: {
            if (bEnable) {
                mMODDAIBufferLock.lock();
            }
            else {
                mMODDAIBufferLock.unlock();
            }
            break;
        }
        default:
            ALOGE("AcquireMemBufferLock with wrong bufer lock");
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t  AudioMTKStreamInManager::Do_input_standby(AudioMTKStreamInClient *Client)
{
    ALOGD("+Do_input_standby Client = %p", Client);
    uint32 AudioIn1 = 0, AudioIn2 = 0 , AudioOut1 = 0, AudioOut2 = 0;
    int ret = 0;
    Client->mEnable = false;

#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
    bool FmMicConflict = false;
    FmMicConflict = checkFmMicConflict(Client, false);
#endif

    switch (mAudioResourceManager->GetAudioMode()) {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION: { // fix me, is that mode in communication needs to be care more??
            switch (Client->mMemDataType) {
                case AudioDigitalType::MEM_VUL:
                    AudioOut1 = AudioDigitalType::O09;
                    AudioOut2 = AudioDigitalType::O10;
                    if (mVULThread.get() && !checkMemInUse(Client)) {
                        ret = mVULThread->requestExitAndWait();
                        if (ret == WOULD_BLOCK) {
                            mVULThread->requestExit();
                        }
                        mVULThread.clear();
                        mAudioResourceManager->StopInputDevice();
                    }
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
                    if(FmMicConflict && AudioAnalogType::ADC_GROUP_MIC_IN == Client->mAdcGroup) {
                        mAudioResourceManager->SelectInputDevice(AUDIO_DEVICE_IN_FM);
                    }
#endif
                    break;
                case AudioDigitalType::MEM_DAI:
                    AudioOut1 = AudioDigitalType::O11;
                    AudioOut2 = AudioDigitalType::O11;
                    AudioIn1 = AudioDigitalType::I02;
                    AudioIn2 = AudioDigitalType::I02;
                    if (mDAIThread.get() && !checkMemInUse(Client)) {
                        ret = mDAIThread->requestExitAndWait();
                        if (ret == WOULD_BLOCK) {
                            mDAIThread->requestExit();
                        }
                        mDAIThread.clear();
                    }
                    break;
                case AudioDigitalType::MEM_AWB:
                    AudioOut1 = AudioDigitalType::O05;
                    AudioOut2 = AudioDigitalType::O06;
                    ALOGD("+Do_input_standby mAWBThread->requestExitAndWait()");
                    if (mAWBThread.get() && !checkMemInUse(Client)) {
                        ret = mAWBThread->requestExitAndWait();
                        if (ret == WOULD_BLOCK) {
                            mAWBThread->requestExit();
                        }
                        mAWBThread.clear();
                    }
                    break;
                default:
                    ALOGD("NO support for memory interface");
                    break;
            }
            // disable memif
            if (!checkMemInUse(Client)) {
                mAudioDigital->SetMemIfEnable(Client->mMemDataType, false);
            }
            // ih no user is used , disable irq2
            if (mAudioInput.size() == 1) {
                mAudioDigital->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, false);
            }

            // AFE_ON = false
            mAudioDigital->SetAfeEnable(false);
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2: {
            StopModemRecord();
            break;
        }
        default:
            break;
    }
    ALOGD("-Do_input_standby Client = %p", Client);
    return NO_ERROR;
}

status_t AudioMTKStreamInManager::I2SAdcInSet(AudioDigtalI2S *AdcI2SIn, AudioStreamAttribute *AttributeClient)
{
    AdcI2SIn->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    AdcI2SIn->mBuffer_Update_word = 8;
    AdcI2SIn->mFpga_bit_test = 0;
    AdcI2SIn->mFpga_bit = 0;
    AdcI2SIn->mloopback = 0;
    AdcI2SIn->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    AdcI2SIn->mI2S_FMT = AudioDigtalI2S::I2S;
    AdcI2SIn->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;

    if(AttributeClient->mIsDigitalMIC)  //for digital MIC 32K constrain
        AdcI2SIn->mI2S_SAMPLERATE = MemVULSamplerate;
    else
        AdcI2SIn->mI2S_SAMPLERATE = (AttributeClient->mSampleRate);

    AdcI2SIn->mI2S_EN = false;
    return NO_ERROR;
}

status_t AudioMTKStreamInManager::Set2ndI2SIn(AudioDigtalI2S *m2ndI2SIn, unsigned int mSampleRate)
{
    ALOGD("+%s()\n", __FUNCTION__);
    m2ndI2SIn->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    m2ndI2SIn->mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    m2ndI2SIn->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    m2ndI2SIn->mI2S_FMT = AudioDigtalI2S::I2S;
    m2ndI2SIn->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    m2ndI2SIn->mI2S_SAMPLERATE = mSampleRate;
    mAudioDigital->Set2ndI2SOut(m2ndI2SIn);
    return NO_ERROR;
}

status_t AudioMTKStreamInManager::Enable2ndI2SIn(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAudioDigital->Set2ndI2SEnable(bEnable);
    return NO_ERROR;
}

status_t  AudioMTKStreamInManager:: Do_input_start(AudioMTKStreamInClient *Client)
{
    // savfe interconnection
    ALOGD("+%s(), client = %p\n", __FUNCTION__, Client);
    uint32 AudioIn1 = 0, AudioIn2 = 0 , AudioOut1 = 0, AudioOut2 = 0;
    uint32 MemIfSamplerate = 0, MemIfChannel = 0;
    int ret = 0;


#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
    bool FmMicConflict = false;
    FmMicConflict = checkFmMicConflict(Client, true);
    ALOGD("%s(), FmMicConflict = %d\n", __FUNCTION__, FmMicConflict);
#endif

    switch (mAudioResourceManager->GetAudioMode()) {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:
        { // fix me, is that mode in communication needs to be care more??
            ALOGD("%s(), Client->mSourceType = %d, Client->mMemDataType = %d\n", __FUNCTION__, Client->mSourceType, Client->mMemDataType);
            switch (Client->mSourceType)
            {
                    // fix me:: pcm recording curretn get data from modem.
                case AudioDigitalType::MODEM_PCM_1_O:
                case AudioDigitalType::MODEM_PCM_2_O:
                    break;
                case AudioDigitalType::I2S_IN_ADC:
                    I2SAdcInSet(&mAdcI2SIn, Client->mAttributeClient);
                    mAudioDigital->SetI2SAdcIn(&mAdcI2SIn);
                    if(Client->mAttributeClient->mIsDigitalMIC)  //for digital MIC 32K constrain
                        mAudioAnalog->SetFrequency(AudioAnalogType::DEVICE_IN_ADC, MemVULSamplerate);
                    else
                        mAudioAnalog->SetFrequency(AudioAnalogType::DEVICE_IN_ADC, Client->mAttributeClient->mSampleRate);
                    mAudioDigital->SetI2SAdcEnable(true);
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
                    if(FmMicConflict && AUDIO_DEVICE_IN_FM == Client->mAttributeClient->mdevices) {
                    } else
#endif
                    {
                        // here open analog control
                        mAudioResourceManager->StartInputDevice();
                    }
                    AudioIn1 = AudioDigitalType::I03;
                    AudioIn2 = AudioDigitalType::I04;
                    break;
                case AudioDigitalType::I2S_INOUT_2:
                    AudioIn1 = AudioDigitalType::I00;
                    AudioIn2 = AudioDigitalType::I01;
                    if(Client->mAttributeClient->mSource == AUDIO_SOURCE_MATV)
                    {
                        Set2ndI2SIn(&m2ndI2S, Client->mAttributeClient->mSampleRate);
                        Enable2ndI2SIn(true);
                    }
                    break;
                case AudioDigitalType::MRG_I2S_IN:
#if defined(MTK_MERGE_INTERFACE_SUPPORT)
                    mAudioDigital->SetMrgI2SEnable(true, Client->mAttributeClient->mSampleRate);
#endif
                    AudioIn1 = AudioDigitalType::I15;
                    AudioIn2 = AudioDigitalType::I16;
                    break;
                case AudioDigitalType::DAI_BT:
                    AudioIn1 = AudioDigitalType::I02;
                    AudioIn2 = AudioDigitalType::I02;
                    // here for SW MIC gain calculate
                    mAudioResourceManager->StartInputDevice();
                    break;
                default:
                    break;
            }

            switch (Client->mMemDataType)
            {
                case AudioDigitalType::MEM_VUL:
                    AudioOut1 = AudioDigitalType::O09;
                    AudioOut2 = AudioDigitalType::O10;
                    MemIfSamplerate = MemVULSamplerate;
                    MemIfChannel = 2;
                    break;
                case AudioDigitalType::MEM_DAI:
                    AudioOut1 = AudioDigitalType::O11;
                    AudioOut2 = AudioDigitalType::O11;
                    AudioIn1 = AudioDigitalType::I02;
                    AudioIn2 = AudioDigitalType::I02;
                    MemIfSamplerate = MemDAISamplerate;
                    MemIfChannel = 1;
                    ALOGD("!!!Do_input_start MEM_DAI MemIfChannel=1");
                    break;
                case AudioDigitalType::MEM_AWB:
                    AudioOut1 = AudioDigitalType::O05;
                    AudioOut2 = AudioDigitalType::O06;
#ifdef FM_DIGITAL_IN_SUPPORT
                    if(Client->mSourceType == AudioDigitalType::MRG_I2S_IN)
                    {
                        MemIfSamplerate = Client->mAttributeClient->mSampleRate; // Use MRGIF interface to record
                    }
                    else
#endif
                    if(Client->mAttributeClient->mSource == AUDIO_SOURCE_MATV)
                        MemIfSamplerate = Client->mAttributeClient->mSampleRate; // mATV default sampling rate
                    else
                        MemIfSamplerate = MemAWBSamplerate;
                    MemIfChannel = 2;
                    break;
                default:
                    ALOGD("NO support for memory interface");
                    break;
            }

            // set digital memif attribute
            if (!checkMemInUse(Client)) {
                ALOGD("checkMemInUse Start memtype = %d", Client->mMemDataType);
                mAudioDigital->SetMemIfSampleRate(Client->mMemDataType, MemIfSamplerate);
                mAudioDigital->SetMemIfChannelCount(Client->mMemDataType, MemIfChannel);
                mAudioDigital->SetMemIfEnable(Client->mMemDataType, true);
                StartStreamInThread(Client->mMemDataType);
            }

            // set irq enable , need handle with irq2 mcu mode.
            AudioIrqMcuMode IrqStatus;
            mAudioDigital->GetIrqStatus(AudioDigitalType::IRQ2_MCU_MODE, &IrqStatus);
            if (IrqStatus.mStatus == false)
            {
                ALOGD("SetIrqMcuSampleRate mSampleRate = %d", Client->mAttributeClient->mSampleRate);
                if (Client->mMemDataType == AudioDigitalType::MEM_DAI)
                {
                    ALOGD("Do_input_start SetIrqMcuSampleRate = 8000, SetIrqMcuCounter=256 ");
                    mAudioDigital->SetIrqMcuSampleRate(AudioDigitalType::IRQ2_MCU_MODE, 8000);
                    mAudioDigital->SetIrqMcuCounter(AudioDigitalType::IRQ2_MCU_MODE, 256); //ccc 1107
                }
                else
                {
                    mAudioDigital->SetIrqMcuSampleRate(AudioDigitalType::IRQ2_MCU_MODE, 16000);
                    mAudioDigital->SetIrqMcuCounter(AudioDigitalType::IRQ2_MCU_MODE, 800); // 50ms
                }
                mAudioDigital->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, true);
            }
            else
            {
                ALOGD("IRQ2_MCU_MODE is enabled , use original irq2 interrupt mode");
            }

            // set interconnection
            if (!checkMemInUse(Client)) {
                mAudioDigital->SetinputConnection(AudioDigitalType::Connection, AudioIn1, AudioOut1);
                mAudioDigital->SetinputConnection(AudioDigitalType::Connection, AudioIn2, AudioOut2);
                mAudioDigital->SetAfeEnable(true);
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2: {
            StartModemRecord(Client);
            break;
        }
        default:
            break;
    }

    Client->mEnable = true;

    ALOGD("-Do_input_start client = %p", Client);
    return NO_ERROR;
}

AudioMTKStreamInClient *AudioMTKStreamInManager::RequestClient(uint32_t Buflen)
{
    AudioMTKStreamInClient *Client = NULL;
    Client = new AudioMTKStreamInClient(Buflen, mClientNumber);
    ALOGD("RequestClient Buflen = %d Client = %p AudioInput.size  = %d ", Buflen, Client, mAudioInput.size());
    if (Client == NULL) {
        ALOGW("RequestClient but return NULL");
        return NULL;
    }
    //until start should add output
    mClientNumber++;
    mAudioInput.add(mClientNumber, Client);
    Client->mClientId = mClientNumber;
    Client->mEnable = false;
#ifdef ENABLE_SUPPORT_FM_MIC_CONFLICT
    Client->mConflict = false;
    Client->mDropCount = 0;
    Client->mAdcGroup = AudioAnalogType::ADC_GROUP_NONE;
#endif
    ALOGD("%s(), mAudioInput.size() = %d, mClientNumber = %d", __FUNCTION__, mAudioInput.size(), mClientNumber);
    return Client;
}

status_t AudioMTKStreamInManager::ReleaseClient(AudioMTKStreamInClient *Client)
{
    // remove from vector
    uint32_t clientid = Client->mClientId;
    ssize_t index = mAudioInput.indexOfKey(clientid);
    ALOGD("ReleaseClient Client = %p clientid = %d mAudioInput.size  = %d", Client, clientid, mAudioInput.size());
    if (Client != NULL) {
        ALOGD("remove  mAudioInputcloient index = %d", index);
        delete mAudioInput.valueAt(index);
        mAudioInput.removeItem(clientid);
        //Client is deleted;
        Client = NULL;
    }
    ALOGD("ReleaseClient mAudioInput.size = %d", mAudioInput.size());
    return NO_ERROR;
}

void AudioMTKStreamInManager::SetInputMute(bool bEnable)
{
    if(mMicMute != bEnable)
    {
        mMicMute =  bEnable;
        mMuteTransition = false;
    }
}

static short clamp16(int sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

status_t AudioMTKStreamInManager::ApplyVolume(void* Buffer , uint32 BufferSize)
{
    // cehck if need apply mute
    if(mMicMute == true)
    {
        // do ramp down
        if(mMuteTransition == false)
        {
            uint32 count = BufferSize >> 1;
            float Volume_inverse =(float) (MTK_STREAMIN_VOLUEM_MAX/count)*-1;
            short *pPcm = (short*)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while(count)
            {
                value = *pPcm * (MTK_STREAMIN_VOLUEM_MAX+(Volume_inverse*ConsumeSample));
                *pPcm = clamp16(value>>MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
        else
        {
            memset(Buffer,0,BufferSize);
        }
    }
    else if(mMicMute== false)
    {
        // do ramp up
        if(mMuteTransition == false)
        {
            uint32 count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX/count);
            short *pPcm = (short*)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while(count)
            {
                value = *pPcm * (Volume_inverse*ConsumeSample);
                *pPcm = clamp16(value>>MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
    }
    return NO_ERROR;
}


AudioMTKStreamInManager::AudioMTkRecordThread::AudioMTkRecordThread(AudioMTKStreamInManager *AudioManager, uint32 Mem_type, char *RingBuffer, uint32 BufferSize)
{
    ALOGD("AudioMTkRecordThread constructor Mem_type = %d", Mem_type);
    mFd = 0;
    mMemType = Mem_type;
    mManager = AudioManager;
    char Buf[10];
    sprintf(Buf, "%d.pcm", DumpFileNum);
    switch (mMemType) {
        case AudioDigitalType::MEM_VUL:
            mName = String8("AudioMTkRecordThreadVUL");
            DumpFileName =String8(streaminmanager);
            DumpFileName.append ((const char*)Buf);
            ALOGD("AudioMTkRecordThread DumpFileName = %s",DumpFileName.string ());
            mPAdcPCMDumpFile = NULL;
            mPAdcPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
            break;
        case AudioDigitalType::MEM_DAI:
            mName = String8("AudioMTkRecordThreadDAI");
            DumpFileName =String8(streaminmanager);
            DumpFileName.append ((const char*)Buf);
            mPDAIInPCMDumpFile = NULL;
            mPDAIInPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
            break;
        case AudioDigitalType::MEM_AWB:
            mName = String8("AudioMTkRecordThreadAWB");
            DumpFileName =String8(streaminmanager);
            DumpFileName.append ((const char*)Buf);
            mPI2SPCMDumpFile = NULL;
            mPI2SPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty); //ccc
            break;
        default:
            ALOGD("NO support for memory interface");
            break;
    }
    // ring buffer to copy data into this ring buffer
    DumpFileNum++;
    DumpFileNum %= MAX_DUMP_NUM;
    mRingBuffer = RingBuffer;
    mBufferSize = BufferSize;
}

AudioMTKStreamInManager::AudioMTkRecordThread::~AudioMTkRecordThread()
{
    ALOGD("+AudioMTkRecordThread()");
    ClosePcmDumpFile();

    // do thread exit routine
    if (mFd) {
        ALOGD("threadLoop exit STANDBY_MEMIF_TYPE mMemType = %d", mMemType);
        ::ioctl(mFd, STANDBY_MEMIF_TYPE, mMemType);
    }
    if (mFd) {
        ::close(mFd);
        mFd = 0;
    }
    ALOGD("-AudioMTkRecordThread()");
}

void AudioMTKStreamInManager::AudioMTkRecordThread::onFirstRef()
{
    ALOGD("AudioMTkRecordThread onFirstRef");
    tempdata = 0;
    if(mMemType == AudioDigitalType::MEM_VUL)
    {
        mRecordDropms = AUDIO_RECORD_DROP_MS;
    }
    else
    {
        mRecordDropms =0;
    }
    run(mName, ANDROID_PRIORITY_URGENT_AUDIO);
}

// Good place to do one-time initializations
status_t  AudioMTKStreamInManager::AudioMTkRecordThread::readyToRun()
{
    ALOGD("AudioMTkRecordThread::readyToRun()");
    if (mFd == 0) {
        mFd =  ::open(kAudioDeviceName, O_RDWR);
        if (mFd <= 0)
            ALOGW("open device fail");
    }
    ::ioctl(mFd, START_MEMIF_TYPE, mMemType);
    if(mMemType == AudioDigitalType::MEM_VUL)
    {
        DropRecordData();
    }
    return NO_ERROR;
}

void AudioMTKStreamInManager::AudioMTkRecordThread::WritePcmDumpData()
{
    int written_data = 0;
    switch (mMemType) {
        case AudioDigitalType::MEM_VUL:
            if (mPAdcPCMDumpFile)
            {
                long int position =0;
                position = ftell (mPAdcPCMDumpFile);
                if(position> MAX_FILE_LENGTH)
                {
                    rewind(mPAdcPCMDumpFile);
                }
                written_data = fwrite((void *)mRingBuffer, 1, mBufferSize, mPAdcPCMDumpFile);
            }
            break;
        case AudioDigitalType::MEM_DAI:
            if (mPDAIInPCMDumpFile)
            {
                long int position =0;
                position = ftell (mPDAIInPCMDumpFile);
                if(position> MAX_FILE_LENGTH)
                {
                    rewind(mPDAIInPCMDumpFile);
                }
                written_data = fwrite((void *)mRingBuffer, 1, mBufferSize, mPDAIInPCMDumpFile);
            }
            break;
        case AudioDigitalType::MEM_AWB:
            if (mPI2SPCMDumpFile)
            {
                long int position =0;
                position = ftell (mPI2SPCMDumpFile);
                if(position> MAX_FILE_LENGTH)
                {
                    rewind(mPI2SPCMDumpFile);
                }
                written_data = fwrite((void *)mRingBuffer, 1, mBufferSize, mPI2SPCMDumpFile); //ccc
            }
            break;
    }
}

void AudioMTKStreamInManager::AudioMTkRecordThread::ClosePcmDumpFile()
{
    ALOGD("ClosePcmDumpFile");
    switch (mMemType) {
        case AudioDigitalType::MEM_VUL:
            if (mPAdcPCMDumpFile) {
                AudioCloseDumpPCMFile(mPAdcPCMDumpFile);
                ALOGD("ClosePcmDumpFile mPAdcPCMDumpFile");
            }
            break;
        case AudioDigitalType::MEM_DAI:
            if (mPDAIInPCMDumpFile) {
                AudioCloseDumpPCMFile(mPDAIInPCMDumpFile);
                ALOGD("ClosePcmDumpFile mPDAIInPCMDumpFile");
            }
            break;
        case AudioDigitalType::MEM_AWB:
            if (mPI2SPCMDumpFile) {
                AudioCloseDumpPCMFile(mPI2SPCMDumpFile);
                ALOGD("ClosePcmDumpFile mPI2SPCMDumpFile");
            }
            break;
    }
}

void AudioMTKStreamInManager::AudioMTkRecordThread::DropRecordData()
{
    int Read_Size = 0;
    int Read_Buffer_Size = 0;
    // drop data for pop
    if(mRecordDropms != 0)
    {
          if(mRecordDropms > RECORD_DROP_MS_MAX){
              mRecordDropms = RECORD_DROP_MS_MAX;
          }
          Read_Buffer_Size = ((AudioMTKStreamInManager::MemVULSamplerate * mRecordDropms << 2) / 1000);
          ALOGD("1. DropRecordData Read_Buffer_Size = %d Read_Size = %d",Read_Buffer_Size,Read_Size);
    }
    while(Read_Buffer_Size > 0)
    {
        if(Read_Buffer_Size > mBufferSize){
            Read_Size = ::read(mFd, mRingBuffer, mBufferSize);
        }
        else
        {
            Read_Size = ::read(mFd, mRingBuffer, Read_Buffer_Size);
        }
        Read_Buffer_Size -= mBufferSize;
        ALOGD("DropRecordData Read_Buffer_Size = %d Read_Size = %d",Read_Buffer_Size,Read_Size);
    }
}

bool AudioMTKStreamInManager::AudioMTkRecordThread::threadLoop()
{
    uint32 Read_Size = 0;
    while (!(exitPending() == true)) {
        //ALOGD("AudioMTkRecordThread threadLoop() read mBufferSize = %d mRingBuffer = %p ", mBufferSize, mRingBuffer);
        Read_Size = ::read(mFd, mRingBuffer, mBufferSize);
        WritePcmDumpData();
        mManager->ApplyVolume(mRingBuffer,mBufferSize);
        mManager->CopyBufferToClient(mMemType, (void *)mRingBuffer, mBufferSize);
        return true;
    }
    ALOGD("threadLoop exit");
    return false;
}

}




