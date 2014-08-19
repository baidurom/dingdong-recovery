/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioParamTuning.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the method for  handling param tuning.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *******************************************************************************/

#ifndef _AUDIO_PARAM_TUNING_H_
#define _AUDIO_PARAM_TUNING_H_

#include <utils/threads.h>

#include "AudioMTKVolumeInterface.h"
#include "AudioResourceManager.h"
#include "AudioResourceFactory.h"
#include "SpeechDriverFactory.h"
#include "AudioDigitalControlInterface.h"
#include "AudioAnalogControlInterface.h"
#include "SpeechPcm2way.h"

#define MAX_VOICE_VOLUME VOICE_VOLUME_MAX
#define FILE_NAME_LEN_MAX 128
#define P2W_RECEIVER_OUT 0
#define P2W_HEADSET_OUT 1
#define P2W_NORMAL 0
#define P2W_RECONLY 1


typedef struct {
    unsigned short cmd_type;
    unsigned short slected_fir_index;
    unsigned short dlDigitalGain;
    unsigned short dlPGA;
    unsigned short phone_mode;
    unsigned short wb_mode;
    char input_file[FILE_NAME_LEN_MAX];
}AudioTasteTuningStruct;

typedef enum {
    AUD_TASTE_STOP = 0,
    AUD_TASTE_START,
    AUD_TASTE_DLDG_SETTING,
    AUD_TASTE_DLPGA_SETTING,
    AUD_TASTE_INDEX_SETTING,

    AUD_TASTE_CMD_NUM
}AUD_TASTE_CMD_TYPE;

typedef enum {
    PCM_FORMAT = 0,
    WAVE_FORMAT,

    UNSUPPORT_FORMAT
}FILE_FORMAT;

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
typedef enum {
    AUD_MIC_GAIN = 0,
    AUD_RECEIVER_GAIN,
    AUD_HS_GAIN,

    AUD_GAIN_TYPE_NUM
}DMNRGainType;

typedef enum {
	DUAL_MIC_REC_PLAY_STOP=	0,
	DUAL_MIC_REC,
	DUAL_MIC_REC_PLAY,
	DUAL_MIC_REC_PLAY_HS,
	   
	DMNR_TUNING_CMD_CNT
}DMNRTuningCmdType;
#endif

namespace android {
#define MODE_NUM NUM_OF_VOL_MODE

class AudioParamTuning
{
public:
    AudioParamTuning();
    ~AudioParamTuning();

    static AudioParamTuning *getInstance();
   
    //for taste tool
    bool isPlaying();
    status_t setMode(uint32 mode);
    uint32 getMode();
    status_t setPlaybackFileName(const char *fileName);
    status_t setDLPGA(uint32 gain);
    void updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam);
    status_t enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB=false);
    status_t putDataToModem();
    FILE_FORMAT playbackFileFormat();

    // for playback thread
    //rb m_sPlayBuf;

    // protect Play PCM With Speech Enhancement buffers
    pthread_mutex_t mPlayBufMutex;  
    pthread_cond_t mPPSExit_Cond;
    pthread_mutex_t mPPSMutex;
    pthread_mutex_t mP2WMutex;

    bool m_bPPSThreadExit;
    bool m_bWBMode;
    FILE *m_pInputFile;

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    // For DMNR Tuning	
    status_t setRecordFileName(const char *fileName);
    status_t setDMNRGain(unsigned short type, unsigned short value); //for DMNR
    status_t getDMNRGain(unsigned short type, unsigned short *value); //for DMNR
    status_t enableDMNRModem2Way(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode);
    status_t readDataFromModem(uint32 offset, uint32 len);

    // for DMNR playback+record thread
    //rb m_sRecBuf;

    // protect DMNR Playback+record buffers
    pthread_mutex_t mDMNRMutex;	
    pthread_mutex_t mRecBufMutex;  
    pthread_cond_t mDMNRExit_Cond;

    bool m_bDMNRThreadExit;
    FILE *m_pOutputFile;
    
    Play2Way *mPlay2WayInstance;
    Record2Way *mRec2WayInstance;
#endif
	
private:

    status_t setSphVolume(uint32 mode, uint32 gain);
    status_t OpenModemSpeechControlFlow(int mode);
    status_t openModemDualMicCtlFlow(bool bWB, bool bRecPly);
    status_t closeModemDualMicCtlFlow(bool bRecPly);
		
    // the uniqe
    static AudioParamTuning *UniqueTuningInstance;

    SpeechDriverFactory *mSpeechDriverFactory;
    AudioMTKVolumeInterface *mAudioVolumeInstance;
    AudioAnalogControlInterface *mAudioAnalogInstance;
    AudioDigitalControlInterface *mAudioDigitalInstance;
    AudioResourceManagerInterface *mAudioResourceManager;

    uint32 mSideTone;
    uint32 mOutputVolume[MODE_NUM];
    uint32 mMode;
    char m_strInputFileName[FILE_NAME_LEN_MAX];
    bool m_bPlaying;

    pthread_t mTasteThreadID;

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    bool m_bDMNRPlaying;
    char m_strOutFileName[FILE_NAME_LEN_MAX]; // for reord
    unsigned short mDualMicTool_micGain;
    unsigned short mDualMicTool_receiverGain;
    unsigned short mDualMicTool_headsetGain;
    
    pthread_t mDMNRThreadID;
#endif
};
}
#endif

