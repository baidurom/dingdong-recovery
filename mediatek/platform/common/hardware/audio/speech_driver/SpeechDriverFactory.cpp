#include "SpeechDriverFactory.h"

#include "SpeechType.h"

#include "SpeechDriverInterface.h"
#include "SpeechDriverLAD.h"

#define LOG_TAG "SpeechDriverFactory"

namespace android
{

SpeechDriverFactory *SpeechDriverFactory::mSpeechDriverFactory = NULL;
SpeechDriverFactory *SpeechDriverFactory::GetInstance()
{
    if (mSpeechDriverFactory == NULL) {
        mSpeechDriverFactory = new SpeechDriverFactory();
    }
    ASSERT(mSpeechDriverFactory != NULL);
    return mSpeechDriverFactory;
}

SpeechDriverFactory::SpeechDriverFactory()
{
    mSpeechDriver1 = NULL;
    mSpeechDriver2 = NULL;

#if defined(MTK_ENABLE_MD1)
    mActiveModemIndex = MODEM_1; // default use modem 1
#elif defined(MTK_ENABLE_MD2)
    mActiveModemIndex = MODEM_2; // if modem 1 not enabled, default use modem 2
#else
    ALOGE("Somebody forgot to define MTK_ENABLE_MD1/MTK_ENABLE_MD2 for this project");
    mActiveModemIndex = MODEM_1; // default use modem 1
#endif

    CreateSpeechDriverInstances();
}
status_t SpeechDriverFactory::CreateSpeechDriverInstances()
{
#ifdef MTK_ENABLE_MD1
    // for internal modem_1, always return LAD
    if (mSpeechDriver1 == NULL) {
        mSpeechDriver1 = SpeechDriverLAD::GetInstance(MODEM_1);
    }
#endif

#ifdef MTK_ENABLE_MD2
    // for modem_2, might use internal/external modem
    // TODO(harvey): check LAD+CCCI / LAD+EMCS / ATCmd / EVDO / ...
    if (mSpeechDriver2 == NULL) {
        mSpeechDriver2 = SpeechDriverLAD::GetInstance(MODEM_2);
    }
#endif

    return NO_ERROR;
}

status_t SpeechDriverFactory::DestroySpeechDriverInstances()
{
    if (mSpeechDriver1 != NULL) {
        delete mSpeechDriver1;
        mSpeechDriver1 = NULL;
    }

    if (mSpeechDriver2 == NULL) {
        delete mSpeechDriver2;
        mSpeechDriver2 = NULL;
    }

    return NO_ERROR;
}

SpeechDriverFactory::~SpeechDriverFactory()
{
    DestroySpeechDriverInstances();
}

SpeechDriverInterface *SpeechDriverFactory::GetSpeechDriver()
{
    SpeechDriverInterface *pSpeechDriver = NULL;

    switch (mActiveModemIndex) {
        case MODEM_1:
            pSpeechDriver = mSpeechDriver1;
            break;
        case MODEM_2:
            pSpeechDriver = mSpeechDriver2;
            break;
        default:
            ALOGE("%s: no such modem index %d", __FUNCTION__, mActiveModemIndex);
            break;
    }

    ASSERT(pSpeechDriver != NULL);
    return pSpeechDriver;
}

/**
 * NO GUARANTEE that the returned pointer is not NULL!!
 * Be careful to use this function!!
 */
SpeechDriverInterface *SpeechDriverFactory::GetSpeechDriverByIndex(const modem_index_t modem_index)
{
    SpeechDriverInterface *pSpeechDriver = NULL;

    switch (modem_index) {
        case MODEM_1:
            pSpeechDriver = mSpeechDriver1;
            break;
        case MODEM_2:
            pSpeechDriver = mSpeechDriver2;
            break;
        default:
            ALOGE("%s: no such modem index %d", __FUNCTION__, modem_index);
            break;
    }

    return pSpeechDriver;
}


modem_index_t SpeechDriverFactory::GetActiveModemIndex() const
{
    ALOGD("%s(), active modem index = %d", __FUNCTION__, mActiveModemIndex);
    return mActiveModemIndex;
}

status_t SpeechDriverFactory::SetActiveModemIndex(const modem_index_t modem_index)
{
    ALOGD("%s(), old modem index = %d, new modem index = %d", __FUNCTION__, mActiveModemIndex, modem_index);
    mActiveModemIndex = modem_index;
    return NO_ERROR;
}


status_t SpeechDriverFactory::SetActiveModemIndexByAudioMode(int mode)
{
    status_t return_status = NO_ERROR;

    switch (mode) {
        case AUDIO_MODE_IN_CALL:
            return_status = SetActiveModemIndex(MODEM_1);
            break;
        case AUDIO_MODE_IN_CALL_2:
            return_status = SetActiveModemIndex(MODEM_2);
            break;
        default:
            ALOGE("%s() mode(%d) is neither MODE_IN_CALL nor MODE_IN_CALL_2!!", __FUNCTION__, mode);
            return_status = INVALID_OPERATION;
            break;
    }
    return return_status;
}


} // end of namespace android

