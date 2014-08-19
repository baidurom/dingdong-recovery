#include "SpeechDriverLAD.h"
#include "SpeechMessengerCCCI.h"
#include "SpeechEnhancementController.h"
#include "SpeechVMRecorder.h"

#include "AudioUtility.h"

#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"

#define LOG_TAG "SpeechDriverLAD"

namespace android
{

/*==============================================================================
 *                     Const Value
 *============================================================================*/
static const int16_t kUnreasonableGainValue = 0x8000;

static const uint16_t kWaitModemAckMaxTimeMs = 200;

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechDriverLAD *SpeechDriverLAD::mLad1 = NULL;
SpeechDriverLAD *SpeechDriverLAD::mLad2 = NULL;

SpeechDriverLAD *SpeechDriverLAD::GetInstance(modem_index_t modem_index)
{
    SpeechDriverLAD *pLad = NULL;

    switch (modem_index) {
        case MODEM_1:
            if (mLad1 == NULL) {
                mLad1 = new SpeechDriverLAD(modem_index);
            }
            pLad = mLad1;
            break;
        case MODEM_2:
            if (mLad2 == NULL) {
                mLad2 = new SpeechDriverLAD(modem_index);
            }
            pLad = mLad2;
            break;
        default:
            ALOGE("%s: no such modem_index %d", __FUNCTION__, modem_index);
            break;
    }

    ASSERT(pLad != NULL);
    return pLad;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechDriverLAD::SpeechDriverLAD(modem_index_t modem_index)
{
    ALOGD("%s(), modem_index = %d", __FUNCTION__, modem_index);

    mModemIndex = modem_index;

    pCCCI = new SpeechMessengerCCCI(mModemIndex, this);
    status_t ret = pCCCI->Initial();

    if (ret == NO_ERROR) {
        RecoverModemSideStatusToInitState();
        //SetAllSpeechEnhancementInfoToModem(); // only for debug
    }

    // Record capability
    mRecordSampleRateType = RECORD_SAMPLE_RATE_08K;
    mRecordChannelType    = RECORD_CHANNEL_MONO;
}

SpeechDriverLAD::~SpeechDriverLAD()
{
    pCCCI->Deinitial();

    delete pCCCI;
    ALOGD("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     Speech Control
 *============================================================================*/
speech_mode_t SpeechDriverLAD::GetSpeechModeByOutputDevice(const audio_devices_t output_device)
{
    speech_mode_t speech_mode = SPEECH_MODE_NORMAL;
    if (output_device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT) {
        speech_mode = SPEECH_MODE_BT_CARKIT;
    }
    else if (output_device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO ||
             output_device == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET) {
        speech_mode = SPEECH_MODE_BT_EARPHONE;
    }
    else if (output_device == AUDIO_DEVICE_OUT_SPEAKER) {
        speech_mode = SPEECH_MODE_LOUD_SPEAKER;
    }
    else if (output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
             output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        speech_mode = SPEECH_MODE_EARPHONE;
    }
    else if (output_device == AUDIO_DEVICE_OUT_EARPIECE) {
        speech_mode = SPEECH_MODE_NORMAL;
    }

    return speech_mode;
}

status_t SpeechDriverLAD::SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device)
{
    speech_mode_t speech_mode = GetSpeechModeByOutputDevice(output_device);
    ALOGD("%s(), input_device = 0x%x, output_device = 0x%x, speech_mode = %d", __FUNCTION__, input_device, output_device, speech_mode);

    // AP side have to set speech mode before speech/record/loopback on,
    // hence we check whether modem side get all necessary speech enhancement parameters here
    // if not, re-send it !!
    if (pCCCI->CheckSpeechParamAckAllArrival() == false) {
        ALOGW("%s(), Do SetAllSpeechEnhancementInfoToModem() done. Start set speech_mode = %d", __FUNCTION__, speech_mode);
    }

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_SPH_MODE, speech_mode, 0));
}

status_t SpeechDriverLAD::SpeechOn()
{
    ALOGD("%s()", __FUNCTION__);

    CheckApSideModemStatusAllOffOrDie();
    ASSERT(pCCCI->CheckSpeechParamAckAllArrival() == true);
    SetApSideModemStatus(SPEECH_STATUS_MASK);

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ON, RAT_2G_MODE, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kWaitModemAckMaxTimeMs);
    }

    return retval;
}

status_t SpeechDriverLAD::SpeechOff()
{
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(SPEECH_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    // set a unreasonable gain value s.t. the reasonable gain can be set to modem next time
    mDownlinkGain   = kUnreasonableGainValue;
    mUplinkGain     = kUnreasonableGainValue;
    mSideToneGain   = kUnreasonableGainValue;
    mUplinkMuteOn   = false;
    mDownlinkMuteOn = false;

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_OFF, 0, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kWaitModemAckMaxTimeMs);
    }

    return retval;
}

status_t SpeechDriverLAD::VideoTelephonyOn()
{
    ALOGD("%s()", __FUNCTION__);

    CheckApSideModemStatusAllOffOrDie();
    ASSERT(pCCCI->CheckSpeechParamAckAllArrival() == true);
    SetApSideModemStatus(VT_STATUS_MASK);

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ON, RAT_3G324M_MODE, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kWaitModemAckMaxTimeMs);
    }

    return retval;
}

status_t SpeechDriverLAD::VideoTelephonyOff()
{
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(VT_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    // set a unreasonable gain value s.t. the reasonable gain can be set to modem next time
    mDownlinkGain   = kUnreasonableGainValue;
    mUplinkGain     = kUnreasonableGainValue;
    mSideToneGain   = kUnreasonableGainValue;
    mUplinkMuteOn   = false;
    mDownlinkMuteOn = false;

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_OFF, 0, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kWaitModemAckMaxTimeMs);
    }

    return retval;
}

/*==============================================================================
 *                     Recording Control
 *============================================================================*/

status_t SpeechDriverLAD::RecordOn()
{
    ALOGD("%s(), sample_rate = %d, channel = %d", __FUNCTION__, mRecordSampleRateType, mRecordChannelType);

    ASSERT(pCCCI->CheckSpeechParamAckAllArrival() == true);
    SetApSideModemStatus(RECORD_STATUS_MASK);

    // Note: the record capability is fixed in constructor
    uint16_t param_16bit = (RECORD_FORMAT_PCM << 0) | (mRecordSampleRateType << 4) | (mRecordChannelType << 8);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RECORD_ON, param_16bit, 0));
}

status_t SpeechDriverLAD::RecordOff()
{
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(RECORD_STATUS_MASK);

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RECORD_OFF, 0, 0));
}

status_t SpeechDriverLAD::VoiceMemoRecordOn()
{
    ALOGD("%s()", __FUNCTION__);

    ASSERT(pCCCI->CheckSpeechParamAckAllArrival() == true);
    SetApSideModemStatus(RECORD_STATUS_MASK);

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RECORD_ON, RECORD_FORMAT_VM, 0));
}

status_t SpeechDriverLAD::VoiceMemoRecordOff()
{
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(RECORD_STATUS_MASK);

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RECORD_OFF, 0, 0));
}

uint16_t SpeechDriverLAD::GetRecordSampleRate() const
{
    // Note: the record capability is fixed in constructor

    uint16_t num_sample_rate;

    switch (mRecordSampleRateType) {
        case RECORD_SAMPLE_RATE_08K:
            num_sample_rate = 8000;
            break;
        case RECORD_SAMPLE_RATE_16K:
            num_sample_rate = 16000;
            break;
        case RECORD_SAMPLE_RATE_32K:
            num_sample_rate = 32000;
            break;
        case RECORD_SAMPLE_RATE_48K:
            num_sample_rate = 48000;
            break;
        default:
            num_sample_rate = 8000;
            break;
    }

    ALOGD("%s(), num_sample_rate = %u", __FUNCTION__, num_sample_rate);
    return num_sample_rate;
}

uint16_t SpeechDriverLAD::GetRecordChannelNumber() const
{
    // Note: the record capability is fixed in constructor

    uint16_t num_channel;

    switch (mRecordChannelType) {
        case RECORD_CHANNEL_MONO:
            num_channel = 1;
            break;
        case RECORD_CHANNEL_STEREO:
            num_channel = 2;
            break;
        default:
            num_channel = 1;
            break;
    }

    ALOGD("%s(), num_channel = %u", __FUNCTION__, num_channel);
    return num_channel;
}


/*==============================================================================
 *                     Background Sound
 *============================================================================*/

status_t SpeechDriverLAD::BGSoundOn()
{
    ALOGD("%s()", __FUNCTION__);
    SetApSideModemStatus(BGS_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_ON, 0, 0));
}

status_t SpeechDriverLAD::BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain)
{
    ALOGD("%s(), ul_gain = 0x%x, dl_gain = 0x%x", __FUNCTION__, ul_gain, dl_gain);
    uint16_t param_16bit = (ul_gain << 8) | dl_gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_CONFIG, param_16bit, 0));
}

status_t SpeechDriverLAD::BGSoundOff()
{
    ALOGD("%s()", __FUNCTION__);
    ResetApSideModemStatus(BGS_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_OFF, 0, 0));
}

/*==============================================================================
 *                     PCM 2 Way
 *============================================================================*/
status_t SpeechDriverLAD::PCM2WayPlayOn()
{
    ALOGD("%s(), old mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);

    status_t retval;
    if (mPCM2WayState == 0) {
        // nothing is on, just turn it on
        SetApSideModemStatus(P2W_STATUS_MASK);
        mPCM2WayState |= SPC_PNW_MSG_BUFFER_SPK;
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_SPK) {
        // only play on, return
        retval = INVALID_OPERATION;
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_MIC) {
        // only rec is on, turn off, modify state and turn on again
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
        mPCM2WayState |= SPC_PNW_MSG_BUFFER_SPK;
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
    }
    else if (mPCM2WayState == (SPC_PNW_MSG_BUFFER_MIC | SPC_PNW_MSG_BUFFER_SPK)) {
        // both on, return
        retval = INVALID_OPERATION;
    }
    else {
        retval = INVALID_OPERATION;
    }

    return retval;
}


status_t SpeechDriverLAD::PCM2WayPlayOff()
{
    ALOGD("%s(), current mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);

    status_t retval;
    if (mPCM2WayState == 0) {
        // nothing is on, return
        retval = INVALID_OPERATION;
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_SPK)  {
        // only play on, just turn it off
        ResetApSideModemStatus(P2W_STATUS_MASK);
        mPCM2WayState &= (~SPC_PNW_MSG_BUFFER_SPK);
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_MIC)  {
        // only rec on, return
        retval = INVALID_OPERATION;
    }
    else if (mPCM2WayState == (SPC_PNW_MSG_BUFFER_MIC | SPC_PNW_MSG_BUFFER_SPK)) {
        // both rec and play on, turn off, modify state and turn on again
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
        mPCM2WayState &= (~SPC_PNW_MSG_BUFFER_SPK);
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
    }
    else {
        retval = INVALID_OPERATION;
    }

    return retval;
}


status_t SpeechDriverLAD::PCM2WayRecordOn()
{
    ALOGD("%s(), old mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);

    status_t retval;
    if (mPCM2WayState == 0) {
        //nothing is on, just turn it on
        SetApSideModemStatus(P2W_STATUS_MASK);
        mPCM2WayState |= SPC_PNW_MSG_BUFFER_MIC;
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_SPK)  {
        // only play is on, turn off, modify state and turn on again
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
        mPCM2WayState |= SPC_PNW_MSG_BUFFER_MIC;
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_MIC)  {
        // only rec on, return
        retval = INVALID_OPERATION;
    }
    else if (mPCM2WayState == (SPC_PNW_MSG_BUFFER_MIC | SPC_PNW_MSG_BUFFER_SPK)) {
        retval = INVALID_OPERATION;
    }
    else {
        retval = INVALID_OPERATION;
    }

    return retval;
}


status_t SpeechDriverLAD::PCM2WayRecordOff()
{
    ALOGD("%s(), current mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);

    status_t retval;
    if (mPCM2WayState == 0) {
        //nothing is on, return
        retval = INVALID_OPERATION;
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_SPK)  {
        // only play on, return
        retval = INVALID_OPERATION;
    }
    else if (mPCM2WayState == SPC_PNW_MSG_BUFFER_MIC)  {
        // only rec on, just turn it off
        ResetApSideModemStatus(P2W_STATUS_MASK);
        mPCM2WayState &= (~SPC_PNW_MSG_BUFFER_MIC);
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
    }
    else if (mPCM2WayState == (SPC_PNW_MSG_BUFFER_MIC | SPC_PNW_MSG_BUFFER_SPK)) {
        // both rec and play on, turn off, modify state and turn on again
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
        mPCM2WayState &= (~SPC_PNW_MSG_BUFFER_MIC);
        retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
    }
    else {
        retval = INVALID_OPERATION;
    }

    return retval;
}


status_t SpeechDriverLAD::PCM2WayOn(const bool wideband_on)
{
    mPCM2WayState = (SPC_PNW_MSG_BUFFER_SPK | SPC_PNW_MSG_BUFFER_MIC | (wideband_on << 4));
    ALOGD("%s(), mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);
    SetApSideModemStatus(P2W_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
}

status_t SpeechDriverLAD::PCM2WayOff()
{
    mPCM2WayState = 0;
    ALOGD("%s(), mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);
    ResetApSideModemStatus(P2W_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
}

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
status_t SpeechDriverLAD::DualMicPCM2WayOn(const bool wideband_on, const bool record_only)
{
    ALOGD("%s(), wideband_on = %d, record_only = %d", __FUNCTION__, wideband_on, record_only);

    if (mPCM2WayState) { // prevent 'on' for second time cause problem
        ALOGW("%s(), mPCM2WayState(%d) > 0, return.", __FUNCTION__, mPCM2WayState);
        return INVALID_OPERATION;
    }

    SetApSideModemStatus(P2W_STATUS_MASK);

    dualmic_pcm2way_format_t dualmic_calibration_format =
        (wideband_on == false) ? P2W_FORMAT_NB_CAL : P2W_FORMAT_WB_CAL;

    if (record_only == true) { // uplink only
        mPCM2WayState = SPC_PNW_MSG_BUFFER_MIC;
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_DMNR_REC_ONLY_ON, dualmic_calibration_format, 0));
    }
    else { // downlink + uplink
        mPCM2WayState = SPC_PNW_MSG_BUFFER_SPK | SPC_PNW_MSG_BUFFER_MIC;
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_DMNR_RECPLAY_ON, dualmic_calibration_format, 0));
    }
}

status_t SpeechDriverLAD::DualMicPCM2WayOff()
{
    ALOGD("%s(), mPCM2WayState = %d", __FUNCTION__, mPCM2WayState);

    if (mPCM2WayState == 0) { // already turn off
        ALOGW("%s(), mPCM2WayState(%d) == 0, return.", __FUNCTION__, mPCM2WayState);
        return INVALID_OPERATION;
    }

    ResetApSideModemStatus(P2W_STATUS_MASK);

    if (mPCM2WayState == SPC_PNW_MSG_BUFFER_MIC) {
        mPCM2WayState &= ~(SPC_PNW_MSG_BUFFER_MIC);
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_DMNR_REC_ONLY_OFF, 0, 0));
    }
    else {
        mPCM2WayState &= ~(SPC_PNW_MSG_BUFFER_SPK | SPC_PNW_MSG_BUFFER_MIC);
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_DMNR_RECPLAY_OFF, 0, 0));
    }
}
#endif

/*==============================================================================
 *                     TTY-CTM Control
 *============================================================================*/
status_t SpeechDriverLAD::TtyCtmOn(ctm_interface_t ctm_interface)
{
    ALOGD("%s(), ctm_interface = %d, force set to BAUDOT_MODE = %d", __FUNCTION__, ctm_interface, BAUDOT_MODE);
    status_t retval;
    const bool uplink_mute_on_copy = mUplinkMuteOn;
    SetUplinkMute(true);
    SetApSideModemStatus(TTY_STATUS_MASK);
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
    TtyCtmDebugOn(pSpeechVMRecorder->GetVMRecordCapabilityForCTM4Way());
    retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTM_ON, BAUDOT_MODE, 0));
    SetUplinkMute(uplink_mute_on_copy);
    return retval;
}

status_t SpeechDriverLAD::TtyCtmOff()
{
    ALOGD("%s()", __FUNCTION__);
    ResetApSideModemStatus(TTY_STATUS_MASK);
    TtyCtmDebugOn(false);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTM_OFF, 0, 0));
}

status_t SpeechDriverLAD::TtyCtmDebugOn(bool tty_debug_flag)
{
    ALOGD("%s(), tty_debug_flag = %d", __FUNCTION__, tty_debug_flag);
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
    if (tty_debug_flag) {
        pSpeechVMRecorder->StartCtmDebug();
    }
    else {
        pSpeechVMRecorder->StopCtmDebug();
    }
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTM_DUMP_DEBUG_FILE, tty_debug_flag, 0));
}

/*==============================================================================
 *                     Acoustic Loopback
 *============================================================================*/
status_t SpeechDriverLAD::SetAcousticLoopback(bool loopback_on)
{
    ALOGD("%s(), loopback_on = %d", __FUNCTION__, loopback_on);

    if (loopback_on == true) {
        CheckApSideModemStatusAllOffOrDie();
        ASSERT(pCCCI->CheckSpeechParamAckAllArrival() == true);
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
    }
    else {
        ResetApSideModemStatus(LOOPBACK_STATUS_MASK);
        CheckApSideModemStatusAllOffOrDie();

        // set a unreasonable gain value s.t. the reasonable gain can be set to modem next time
        mDownlinkGain   = kUnreasonableGainValue;
        mUplinkGain     = kUnreasonableGainValue;
        mSideToneGain   = kUnreasonableGainValue;
        mUplinkMuteOn   = false;
        mDownlinkMuteOn = false;
    }

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_ACOUSTIC_LOOPBACK, loopback_on, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kWaitModemAckMaxTimeMs);
    }

    return retval;
}

/*==============================================================================
 *                     Volume Control
 *============================================================================*/

status_t SpeechDriverLAD::SetDownlinkGain(int16_t gain)
{
    ALOGD("%s(), gain = 0x%x, old mDownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkGain);
    if (gain == mDownlinkGain) return NO_ERROR;

    mDownlinkGain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_DL_DIGIT_VOLUME, gain, 0));
}

status_t SpeechDriverLAD::SetUplinkGain(int16_t gain)
{
    ALOGD("%s(), gain = 0x%x, old mUplinkGain = 0x%x", __FUNCTION__, gain, mUplinkGain);
    if (gain == mUplinkGain) return NO_ERROR;

    mUplinkGain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_UL_DIGIT_VOLUME, gain, 0));
}

status_t SpeechDriverLAD::SetDownlinkMute(bool mute_on)
{
    ALOGD("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    if (mute_on == mDownlinkMuteOn) return NO_ERROR;

    mDownlinkMuteOn = mute_on;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_MUTE_SPH_DL, mute_on, 0));
}

status_t SpeechDriverLAD::SetUplinkMute(bool mute_on)
{
    ALOGD("%s(), mute_on = %d, old mUplinkMuteOn = %d", __FUNCTION__, mute_on, mUplinkMuteOn);
    if (mute_on == mUplinkMuteOn) return NO_ERROR;

    mUplinkMuteOn = mute_on;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_MUTE_SPH_UL, mute_on, 0));
}

status_t SpeechDriverLAD::SetSidetoneGain(int16_t gain)
{
    ALOGD("%s(), gain = 0x%x, old mSideToneGain = 0x%x", __FUNCTION__, gain, mSideToneGain);
    if (gain == mSideToneGain) return NO_ERROR;

    mSideToneGain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SIDETONE_VOLUME, gain, 0));
}


/*==============================================================================
 *                     Device related Config
 *============================================================================*/

status_t SpeechDriverLAD::SetModemSideSamplingRate(uint16_t sample_rate)
{
    ALOGD("%s(), sample_rate = %d", __FUNCTION__, sample_rate);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_SAMPLE_RATE, sample_rate, 0));
}

/*==============================================================================
 *                     Speech Enhancement Control
 *============================================================================*/
status_t SpeechDriverLAD::SetSpeechEnhancement(bool enhance_on)
{
    ALOGD("%s(), enhance_on = %d", __FUNCTION__, enhance_on);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTRL_SPH_ENH, enhance_on, 0));
}

status_t SpeechDriverLAD::SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask)
{
    ALOGD("%s(), main_func = 0x%x, dynamic_func = 0x%x", __FUNCTION__, mask.main_func, mask.dynamic_func);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CONFIG_SPH_ENH, mask.main_func, mask.dynamic_func));
}

/*==============================================================================
 *                     Speech Enhancement Parameters
 *============================================================================*/
status_t SpeechDriverLAD::SetVariousKindsOfSpeechParameters(const void *param, const uint16_t data_length, const uint16_t ccci_message_id)
{
    if (pCCCI->A2MBufLock() == false) { // get buffer lock to prevent overwrite other's data
        ALOGE("%s() fail due to unalbe get A2MBufLock, ccci_message_id = 0x%x", __FUNCTION__, ccci_message_id);
        return TIMED_OUT;
    }
    else {
        // get share buffer address
        uint16_t offset = A2M_SHARED_BUFFER_SPH_PARAM_BASE;
        char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
        char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;

        // fill header info
        pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                 CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                 SHARE_BUFF_DATA_TYPE_CCCI_EM_PARAM,
                                 data_length);

        // fill speech enhancement parameter
        memcpy((void *)p_data_address, (void *)param, data_length);

        // send data notify to modem side
        const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(ccci_message_id, message_length, offset));
    }
}


status_t SpeechDriverLAD::SetNBSpeechParameters(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    ALOGD("%s()", __FUNCTION__);

    // send parameters to modem
    status_t retval = SetVariousKindsOfSpeechParameters(pSphParamNB, sizeof(AUDIO_CUSTOM_PARAM_STRUCT), MSG_A2M_EM_NB);

    // update VM/EPL/TTY record capability & enable if needed
    SpeechVMRecorder::GetInstance()->SetVMRecordCapability(pSphParamNB);

    if (GetApSideModemStatus(TTY_STATUS_MASK) == true) {
        TtyCtmDebugOn(SpeechVMRecorder::GetInstance()->GetVMRecordCapabilityForCTM4Way());
    }

    return retval;
}

#if defined(MTK_DUAL_MIC_SUPPORT)
status_t SpeechDriverLAD::SetDualMicSpeechParameters(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic)
{
    ALOGD("%s()", __FUNCTION__);

#if defined(MTK_WB_SPEECH_SUPPORT)
    // NVRAM always contain(44+76), for WB we send full (44+76)
    const uint16_t data_length = sizeof(unsigned short) * (NUM_ABF_PARAM + NUM_ABFWB_PARAM); // NB + WB
#else
    // for NB we send (44) only
    const uint16_t data_length = sizeof(unsigned short) * (NUM_ABF_PARAM); // NB Only
#endif

    return SetVariousKindsOfSpeechParameters(pSphParamDualMic, data_length, MSG_A2M_EM_DMNR);
}
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechDriverLAD::SetWBSpeechParameters(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB)
{
    ALOGD("%s()", __FUNCTION__);
    return SetVariousKindsOfSpeechParameters(pSphParamWB, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT), MSG_A2M_EM_WB);
}
#endif

status_t SpeechDriverLAD::SetAllSpeechEnhancementInfoToModem()
{
    // Wait until modem ready
    pCCCI->WaitUntilModemReady();

    // Lock
    static AudioLock _mutex;
    _mutex.lock_timeout(3000); // wait 3 sec


    // NB Speech Enhancement Parameters
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    GetNBSpeechParamFromNVRam(&eSphParamNB);
    SetNBSpeechParameters(&eSphParamNB);

#if defined(MTK_DUAL_MIC_SUPPORT)
    // Dual Mic Speech Enhancement Parameters
    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT eSphParamDualMic;
    GetDualMicSpeechParamFromNVRam(&eSphParamDualMic);
    SetDualMicSpeechParameters(&eSphParamDualMic);
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
    // WB Speech Enhancement Parameters
    AUDIO_CUSTOM_WB_PARAM_STRUCT eSphParamWB;
    GetWBSpeechParamFromNVRam(&eSphParamWB);
    SetWBSpeechParameters(&eSphParamWB);
#endif


    // Set speech enhancement parameters' mask to modem side
    SetSpeechEnhancementMask(SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask());


    // Use lock to ensure the previous command with share buffer control is completed
    if (pCCCI->A2MBufLock() == true) {
        pCCCI->A2MBufUnLock();
    }
    else {
        ALOGE("%s() fail to get A2M Buffer Lock!!", __FUNCTION__);
    }

    // Unock
    _mutex.unlock();
    return NO_ERROR;
}


/*==============================================================================
 *                     Recover State
 *============================================================================*/

void SpeechDriverLAD::RecoverModemSideStatusToInitState()
{
    // Record
    if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, record_on = true",  __FUNCTION__, mModemIndex);
        SetApSideModemStatus(RECORD_STATUS_MASK);
        RecordOff();
    }

    // BGS
    if (pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, bgs_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(BGS_STATUS_MASK);
        BGSoundOff();
    }

    // TTY
    if (pCCCI->GetModemSideModemStatus(TTY_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, tty_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(TTY_STATUS_MASK);
        TtyCtmOff();
    }

    // P2W
    if (pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, p2w_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(P2W_STATUS_MASK);
        PCM2WayOff();
    }

    // Phone Call / Loopback
    if (pCCCI->GetModemSideModemStatus(VT_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, vt_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(VT_STATUS_MASK);
        VideoTelephonyOff();
    }
    else if (pCCCI->GetModemSideModemStatus(SPEECH_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, speech_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(SPEECH_STATUS_MASK);
        SpeechOff();
    }
    else if (pCCCI->GetModemSideModemStatus(LOOPBACK_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, loopback_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
        SetAcousticLoopback(false);
    }
}

/*==============================================================================
 *                     Check Modem Status
 *============================================================================*/
bool SpeechDriverLAD::CheckModemIsReady()
{
    return pCCCI->CheckModemIsReady();
};

/*==============================================================================
 *                     Debug Info
 *============================================================================*/
status_t SpeechDriverLAD::ModemDumpSpeechParam()
{
    ALOGD("%s()", __FUNCTION__);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PRINT_SPH_PARAM, 0, 0));
}

} // end of namespace android

