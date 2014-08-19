#ifndef ANDROID_AUDIO_MACHINE_DEVICE_H
#define ANDROID_AUDIO_MACHINE_DEVICE_H

#include "AudioAnalogType.h"
#include "AudioAnalogControlInterface.h"
#include "AudioAnalogReg.h"
#include "audio_custom_exp.h"

namespace android
{
//!  A AudioMachineDevice
/*!
  this class is use for control  amp , external audio sub-system or exxternal DAC
*/

enum
{
    CHIP_VERSION_E1 =0,
    CHIP_VERSION_E2 =1
};



class AudioMachineDevice
{
    public:

        enum ZCD_GAIN_STEP
        {
            GAIN_STEP_1DB =0,
            GAIN_STEP_2DB,
            GAIN_STEP_4DB,
            GAIN_STEP_8DB,
        };

        enum ZCD_GAIN_TIME
        {
            GAIN_TIME_0US=0,
            GAIN_TIME_250US,
            GAIN_TIME_500US ,
            GAIN_TIME_1MS ,
            GAIN_TIME_2MS ,
            GAIN_TIME_4MS ,
            GAIN_TIME_8MS ,
            GAIN_TIME_16MS ,
        };

        AudioMachineDevice();

        /**
        * AudioMachineDevice InitCheck
        * @return status_t
        */
        status_t InitCheck();

        /**
        * a basic function for SetAnalogGain for different Volume Type
        * @param VoleumType value want to set to analog volume
        * @param volume function of analog gain , value between 0 ~ 255
        * @return status_t
        */
        status_t SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume);

        /**
        * a basic function for GetAnalogGain for different Volume Type
        * @param VoleumType value want to get analog volume
        * @return int
        */
        int GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType);

        /**
        * a basic function for SetFrequency for different Device Type , like input and output
        * @param DeviceType
        * @param frequency
        * @return status_t
        */
        status_t SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency);

        /**
        * a basic function fo SetAnalogMute, if provide mute function of hardware.
        * @param VoleumType value want to set to analog volume
        * @param mute of volume type
        * @return status_t
        */
        status_t SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute);

        /**
        * a basic function fo AnalogOpen, open analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        status_t AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType);

        /**
        * a basic function fo AnalogClose, ckose analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        status_t AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType);

        /**
        * a basic function fo getting selection of mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @return AudioAnalogType::MUX_TYPE
        */
        AudioAnalogType::MUX_TYPE AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType);
    
        /**
        * a basic function fo select mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @param MuxType analog mux selection
        * @return status_t
        */
        status_t AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType);

        /**
        * a  function for setParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return status_t
        */
        status_t setParameters(int command1 , int command2 , unsigned int data);

        /**
        * a function for setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        status_t setParameters(int command1 , void *data);

        /**
        * a function fo getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy_size
        */
        int getParameters(int command1 , int command2 , void *data);

        status_t SetLevelShiftBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);
        status_t SetPreampBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);
        status_t SetLineinGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);
        status_t SetHeadPhoneGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);
        status_t SetHandSetGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);
        status_t SetIVBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);
        status_t SetAmpGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume);

        status_t StartTrimFunction(void);
        status_t StartSpkTrimFunction(void);
        status_t StartHeadphoneTrimFunction();
        status_t GetTrimOffset(void);
        status_t SetHPTrimOffset(void);
        status_t SetIVHPTrimOffset(void);
        status_t GetSPKTrimOffset(void);
        status_t SetSPKTrimOffset(void);
        status_t SPKAutoTrimOffset(void);
        status_t GetSPKAutoTrimOffset(void);

        status_t SetZCDStatus(AudioAnalogType::AUDIOANALOGZCD_TYPE ZcdType,
                                                AudioMachineDevice::ZCD_GAIN_STEP Gainstep ,
                                                AudioMachineDevice::ZCD_GAIN_TIME GainTime ,
                                                bool Enable);

        bool GetULinkStatus(void);
        bool GetDownLinkStatus(void);
        int GetChipVersion();

    private:

        AudioLock mLock;
        /**
        * AnalogBlockAttribute to telling Device Type has mux and now mux selection
        * @see AudioAnalogType::DEVICE_MAX
        */
        AnalogBlockAttribute mBlockAttribute[AudioAnalogType::DEVICE_MAX];
        uint32 mBlockSampleRate[AudioAnalogType::DEVICE_INOUT_MAX];

        /**
        * AnalogBlockAttribute to telling Device Type has mux and now mux selection
        * @see AudioAnalogType::VOLUME_TYPE_MAX
        */
        AnalogVolumeAttribute mVolumeAttribute[AudioAnalogType::VOLUME_TYPE_MAX];

        AudioAnalogReg  *mAudioAnalogReg;

         /**
         * file descriptor to open speaker , headset or earpiece
         */
         int mFd;

         /**
         * offset trim value
         */
         uint8_t mHPRtrim;
         uint8_t mHPRfinetrim;
         uint8_t mHPLtrim;
         uint8_t mHPLfinetrim;
         uint8_t mIVHPLtrim;
         uint8_t mIVHPLfinetrim;
         uint8_t mSPKpolarity;
         uint8_t mISPKtrim;

};

}

#endif