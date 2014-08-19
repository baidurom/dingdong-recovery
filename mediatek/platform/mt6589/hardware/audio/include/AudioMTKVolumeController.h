#ifndef ANDROID_AUDIO_MTK_VOLUME_CONTROLLER_H
#define ANDROID_AUDIO_MTK_VOLUME_CONTROLLER_H

#include "AudioMTKVolumeInterface.h"
#include "AudioDigitalControlFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioType.h"
//#include "AudioVolumeDefault.h"
#include "AudioCustParam.h"

#include <IATVCtrlService.h>
#include <binder/IServiceManager.h>
namespace android
{
#define NUM_AUDIO_ROUTE  (17)

/****************************************************
* Define Volume Range of  sound & Voice.
*****************************************************/
#define DEVICE_AUDIO_BUFFER_MAX_VOLUME           (8)
#define DEVICE_AUDIO_BUFFER_MIN_VOLUME           (-4)
#define DEVICE_AMP_MAX_VOLUME     (15)
#define DEVICE_AMP_MIN_VOLUME     (4)
#define DEVICE_IV_BUFFER_MAX_VOLUME     (5)
#define DEVICE_IV_BUFFER_MIN_VOLUME     (-2)
#define DEVICE_LINEIN_PLAY_MAX_VOLUME  (20)

#define AUDIO_VOLUME_MAX       (160)
#define AUDIO_ONEDB_STEP         (4)
#define VOICE_VOLUME_MAX       (160)
#define VOICE_ONEDB_STEP         (4)
#define AMP_VOLUME_MAX           (172)
#define AMP_ONEDB_STEP             (4)
#define UPLINK_GAIN_MAX            (252)
#define UPLINK_ONEDB_STEP         (4)
#define LEVEL_SHIFT_BUFFER_GAIN_MAX   (240)
#define LEVEL_SHIFT_THREEDB_STEP              (32)
#define LEVEL_SHIFT_BUFFER_MAXDB        (21)
#define LINEIN_GAIN_MAX                       (240)
#define LINEIN_GAIN_2DB_STEP                (16)
#define SIDETONE_GAIN_MAX            (240)
#define SIDETONE_ONEDB_STEP         (4)

#define HW_DIGITAL_GAIN_MAX (252)
#define HW_DIGITAL_GAIN_STEP (4)   // 2==> 0.5 dB ,4==>1dB

#define AUDIO_SYSTEM_UL_GAIN_MAX            (45)
#define MIN_PGA_GAIN                                         (2)
#define MAX_PGA_GAIN_RANGE                          (30)
#define AUDIO_UL_PGA_STEP                               (6)
#define SW_AGC_GAIN_MAX                                (17)
#define TRANSPARENT_AGC_GAIN_OFFSET       (33)                        //for digital microphone


typedef enum {
    Idle_Normal_Record = 0,
    Idle_Headset_Record,
    Voice_Rec_Mic_Handset,
    Voice_Rec_Mic_Headset,
    Idle_Video_Record_Handset,
    Idle_Video_Record_Headset,
    Normal_Mic,
    Headset_Mic,
    Handfree_Mic,
    VOIP_Normal_Mic,
    VOIP_Headset_Mic,
    VOIP_Handfree_Mic,
    TTY_CTM_Mic,
    Level_Shift_Buffer_Gain,
    Analog_PLay_Gain,
    Voice_UnLock_Mic_Handset,
    Voice_UnLock_Mic_Headset,
    Customization1_Mic_Handset,
    Customization1_Mic_Headset,
    Customization2_Mic_Handset,
    Customization2_Mic_Headset,
    Customization3_Mic_Handset,
    Customization3_Mic_Headset,
    Num_Mic_Gain
} MIC_GAIN_MODE;

typedef enum {
    EarPiece_SideTone_Gain = 0,
    Headset_SideTone_Gain ,
    LoudSpk_SideTone_Gain ,
    Num_Side_Tone_Gain
} SIDETOEN_GAIN_MODE;

typedef enum {
    Audio_Earpiece = 0,
    Audio_Headset,
    Audio_Headphone,
    Audio_Speaker,
    Audio_DualMode_Earpiece,
    Audio_DualMode_Headset,
    Audio_DualMode_Headphone,
    Audio_DualMode_speaker,
    Ringtone_Earpiece,
    Ringtone_Headset,
    Ringtone_Headphone,
    Ringtone_Speaker,
    Sipcall_Earpiece,
    Sipcall_Headset,
    Sipcall_Headphone,
    Sipcall_Speaker,
    Num_of_Audio_gain
} AUDIO_GAIN_MODE;

static const uint32_t GainMap[] = {
    0x80000, ////0 -0.000000 dB
    0x78d6f, ////1 -0.500000 dB
    0x72148, ////2 -1.000000 dB
    0x6bb2d, ////3 -1.500000 dB
    0x65ac8, ////4 -2.000000 dB
    0x5ffc8, ////5 -2.500000 dB
    0x5a9df, ////6 -3.000000 dB
    0x558c4, ////7 -3.500000 dB
    0x50c33, ////8 -4.000000 dB
    0x4c3ea, ////9 -4.500000 dB
    0x47fac, ////10 -5.000000 dB
    0x43f40, ////11 -5.500000 dB
    0x4026e, ////12 -6.000000 dB
    0x3c903, ////13 -6.500000 dB
    0x392ce, ////14 -7.000000 dB
    0x35fa2, ////15 -7.500000 dB
    0x32f52, ////16 -8.000000 dB
    0x301b7, ////17 -8.500000 dB
    0x2d6a8, ////18 -9.000000 dB
    0x2ae02, ////19 -9.500000 dB
    0x287a2, ////20 -10.000000 dB
    0x26368, ////21 -10.500000 dB
    0x24134, ////22 -11.000000 dB
    0x220ea, ////23 -11.500000 dB
    0x2026f, ////24 -12.000000 dB
    0x1e5a8, ////25 -12.500000 dB
    0x1ca7d, ////26 -13.000000 dB
    0x1b0d7, ////27 -13.500000 dB
    0x198a1, ////28 -14.000000 dB
    0x181c5, ////29 -14.500000 dB
    0x16c31, ////30 -15.000000 dB
    0x157d1, ////31 -15.500000 dB
    0x14496, ////32 -16.000000 dB
    0x1326d, ////33 -16.500000 dB
    0x12149, ////34 -17.000000 dB
    0x1111a, ////35 -17.500000 dB
    0x101d3, ////36 -18.000000 dB
    0xf367 ,////37 -18.500000 dB
    0xe5ca ,////38 -19.000000 dB
    0xd8ef ,////39 -19.500000 dB
    0xcccc ,////40 -20.000000 dB
    0xc157 ,////41 -20.500000 dB
    0xb687 ,////42 -21.000000 dB
    0xac51 ,////43 -21.500000 dB
    0xa2ad ,////44 -22.000000 dB
    0x9994 ,////45 -22.500000 dB
    0x90fc ,////46 -23.000000 dB
    0x88e0 ,////47 -23.500000 dB
    0x8138 ,////48 -24.000000 dB
    0x79fd ,////49 -24.500000 dB
    0x732a ,////50 -25.000000 dB
    0x6cb9 ,////51 -25.500000 dB
    0x66a4 ,////52 -26.000000 dB
    0x60e6 ,////53 -26.500000 dB
    0x5b7b ,////54 -27.000000 dB
    0x565d ,////55 -27.500000 dB
    0x5188 ,////56 -28.000000 dB
    0x4cf8 ,////57 -28.500000 dB
    0x48aa ,////58 -29.000000 dB
    0x4499 ,////59 -29.500000 dB
    0x40c3 ,////60 -30.000000 dB
    0x3d24 ,////61 -30.500000 dB
    0x39b8 ,////62 -31.000000 dB
    0x367d ,////63 -31.500000 dB
    0x3371 ,////64 -32.000000 dB
    0x3090 ,////65 -32.500000 dB
    0x2dd9 ,////66 -33.000000 dB
    0x2b48 ,////67 -33.500000 dB
    0x28dc ,////68 -34.000000 dB
    0x2693 ,////69 -34.500000 dB
    0x246b ,////70 -35.000000 dB
    0x2261 ,////71 -35.500000 dB
    0x2075 ,////72 -36.000000 dB
    0x1ea4 ,////73 -36.500000 dB
    0x1ced ,////74 -37.000000 dB
    0x1b4f ,////75 -37.500000 dB
    0x19c8 ,////76 -38.000000 dB
    0x1857 ,////77 -38.500000 dB
    0x16fa ,////78 -39.000000 dB
    0x15b1 ,////79 -39.500000 dB
    0x147a ,////80 -40.000000 dB
    0x1355 ,////81 -40.500000 dB
    0x1240 ,////82 -41.000000 dB
    0x113b ,////83 -41.500000 dB
    0x1044 ,////84 -42.000000 dB
    0xf5b ,////85 -42.500000 dB
    0xe7f ,////86 -43.000000 dB
    0xdb0 ,////87 -43.500000 dB
    0xcec ,////88 -44.000000 dB
    0xc32 ,////89 -44.500000 dB
    0xb84 ,////90 -45.000000 dB
    0xadf ,////91 -45.500000 dB
    0xa43 ,////92 -46.000000 dB
    0x9b0 ,////93 -46.500000 dB
    0x925 ,////94 -47.000000 dB
    0x8a2 ,////95 -47.500000 dB
    0x827 ,////96 -48.000000 dB
    0x7b2 ,////97 -48.500000 dB
    0x744 ,////98 -49.000000 dB
    0x6dc ,////99 -49.500000 dB
    0x679 ,////100 -50.000000 dB
    0x61d ,////101 -50.500000 dB
    0x5c5 ,////102 -51.000000 dB
    0x572 ,////103 -51.500000 dB
    0x524 ,////104 -52.000000 dB
    0x4db ,////105 -52.500000 dB
    0x495 ,////106 -53.000000 dB
    0x454 ,////107 -53.500000 dB
    0x416 ,////108 -54.000000 dB
    0x3db ,////109 -54.500000 dB
    0x3a4 ,////110 -55.000000 dB
    0x370 ,////111 -55.500000 dB
    0x33e ,////112 -56.000000 dB
    0x310 ,////113 -56.500000 dB
    0x2e4 ,////114 -57.000000 dB
    0x2bb ,////115 -57.500000 dB
    0x294 ,////116 -58.000000 dB
    0x26f ,////117 -58.500000 dB
    0x24c ,////118 -59.000000 dB
    0x22b ,////119 -59.500000 dB
    0x20c ,////120 -60.000000 dB
    0x1ee ,////121 -60.500000 dbg
    0x1d3 ,////122 -61.000000 dB
    0x1b9 ,////123 -61.500000 dB
    0x1a0 ,////124 -62.000000 dB
    0x189 ,////125 -62.500000 dB
    0x173 ,////126 -63.000000 dB
    0x15e ,////127 -63.500000 dB
    0x0 , //
};

class AudioMTKVolumeController : public AudioMTKVolumeInterface
{
    public:
        static float linearToLog(int volume);
        static int logToLinear(float volume);

        static AudioMTKVolumeController *getInstance();
        ~AudioMTKVolumeController() {};
        /**
         * check to see if the audio hardware interface has been initialized.
         */
        status_t    initCheck();
        status_t    initVolumeController();

        status_t setMasterVolume(float v, audio_mode_t mode, uint32_t devices);
        float getMasterVolume();
        status_t setVoiceVolume(float v, audio_mode_t mode, uint32_t devices);
        float getVoiceVolume(void);
        bool ModeSetVoiceVolume(int mode);

        // here only valid stream can be set .
        status_t setStreamVolume(int stream, float v);
        status_t setStreamMute(int stream, bool mute);
        float getStreamVolume(int stream);

        // should depend on different usage , FM ,MATV and output device to setline in gain
        status_t SetLineInPlaybackGain(int type);
        status_t SetLineInRecordingGain(int type);
        bool SetFmVolume(int volume);
        virtual bool SetFmChipVolume(int volume);
        int  GetFmVolume(void);
        /**
          * volume controller setMatvVolume
          * set Matv Volume
          * @param volume
          * @return bool
          */
        virtual bool setMatvVolume(int volume);
        
        /**
          * volume controller SetMatvMute
          * Set Matv Mute
          * @param b_mute
          * @return bool
          */
        virtual bool SetMatvMute(bool b_mute);
        int GetMatvVolume(void);
        void GetMatvService(void);
        bool SetLevelShiftBufferGain(uint32 MicMode, uint32 Gain);
        bool ApplyLevelShiftBufferGain(uint32 MicMode);

        status_t SetMicGain(uint32_t Mode, uint32_t devices);
        status_t SetSideTone(uint32_t Mode, uint32_t devices);

        /**
        * volume controller GetSideToneGain
        * base on output device get sidetone gain
        * @param device
        * @return gain
        */
        virtual uint32_t GetSideToneGain(uint32_t device);

        status_t ApplySideTone(uint32_t Mode);
        uint32_t GetSideToneGainType(uint32 devices);

        /**
        * volume controller ApplyMicGain
        * base on mode gain and route to decide sidetone gain
        * @param Mode
        * @param Gain
        * @return status_t
        */
        virtual status_t ApplyMicGain(uint32_t MicType, int mode);

        bool CheckMicUsageWithMode(uint32_t MicType, int mode);

        /**
        * volume controller SetDigitalHwGain
        * base on mode gain and route to set digital HW gain
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t SetDigitalHwGain(uint32_t Mode, uint32_t Gain , uint32_t routes);

        /**
        * volume controller Set modem DL gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdDlGain(int  Gain);

        /**
        * volume controller Set modem Ul gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdUlGain(int  Gain);
        
        /**
        * volume controller map volume to digital gain
        * @param Gain
        * @return digital gain
        */
        virtual uint16_t MappingToDigitalGain(unsigned char Gain);
        
        /**
        * volume controller map volume to PGA gain
        * @param Gain
        * @return PGA gain
        */
        virtual uint16_t MappingToPGAGain(unsigned char Gain);

        void GetDefaultVolumeParameters(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param);

        // cal and set and set analog gainQuant
        void ApplyAudioGain(int Gain, uint32 mode, uint32 device);
        void ApplyAmpGain(int Gain, uint32 mode, uint32 device);
        void ApplyExtAmpHeadPhoneGain(int Gain, uint32 mode, uint32 device);
        void ApplyDualmodeGain(int Gain, uint32 mode, uint32 device);
        bool SetVolumeRange(uint32 mode, int32 MaxVolume, int32 MinVolume, int32 VolumeRange);

        bool IsHeadsetMicInput(uint32 device);
        uint16_t UpdateSidetone(int DL_PGA_Gain, int  Sidetone_Volume , uint8_t SW_AGC_Ul_Gain );
        uint8_t GetSWMICGain(){return mSwAgcGain;}
        bool Get_FMPower_info(void);

    private:
        static AudioMTKVolumeController *UniqueVolumeInstance;
        AudioMTKVolumeController();
        AudioMTKVolumeController(const AudioMTKVolumeController &);             // intentionally undefined
        AudioMTKVolumeController &operator=(const AudioMTKVolumeController &);  // intentionally undefined

        /**
        * volume controller mapping gain into DB
        * base on Gain and return Degrade in DB
        * @param Gain
        * @return uint32_t
        */
        uint32_t MapDigitalHwGain(uint32_t Gain);

        AudioAnalogControlInterface *mAudioAnalogControl;
        AudioDigitalControlInterface *mAudioDigitalControl;

		// Keeping pointer to ATVCtrlService
        sp<IATVCtrlService> spATVCtrlService;

        bool mMatvMute;
        float mVoiceVolume;
        uint8_t mSwAgcGain;
        float mMasterVolume;
        float mStreamVolume[android_audio_legacy::AudioSystem::NUM_STREAM_TYPES];
        int mFmVolume;
        int mFmChipVolume;
        int mMatvVolume;
        bool mInitDone;

        // data strcutre for volume
        AUDIO_VER1_CUSTOM_VOLUME_STRUCT mVolumeParam;
        int32 mVolumeMax[Num_of_Audio_gain];
        int32 mVolumeMin[Num_of_Audio_gain];
        int32 mVolumeRange[Num_of_Audio_gain];
        int mMicGain[Num_Mic_Gain];
        int mSideTone[Num_Side_Tone_Gain];
};

}

#endif

