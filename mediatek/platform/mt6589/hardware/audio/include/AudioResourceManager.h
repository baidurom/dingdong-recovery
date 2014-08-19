#ifndef _AUDIO_RESOURCE_MANAGER_H
#define _AUDIO_RESOURCE_MANAGER_H

#include "AudioAnalogType.h"
#include "AudioUtility.h"
#include "AudioType.h"
#include "AudioResourceManagerInterface.h"
#include "AudioMTKVolumeInterface.h"
#include "AudioDigitalControlInterface.h"
#include "AudioAnalogControlInterface.h"
//#include "SpeechControlInterface.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <hardware_legacy/AudioSystemLegacy.h>

namespace android
{

class AudioMTKHardware;

class AudioResourceManager : public AudioResourceManagerInterface
{
    public:
        static AudioResourceManager *getInstance();

        /**
         * a function for ~AudioResourceManagerInterface destructor
         */
        virtual ~AudioResourceManager() {};

        /**
        * a function for tell AudioResourceManager output device, usually use for output routing
        * outdevice change may also effetc input device , take care of this.
        * @param new_device
        */
        virtual status_t setDlOutputDevice(uint32 new_device);

        /**
        * a function for tell AudioResourceManager input device, usually use for output routing
        * @param new_device
        */
        virtual status_t setUlInputDevice(uint32 new_device);

        /**
        * a function for tell AudioResourceManager ionput source, usually use for input routing
        * @param mDevice
        */
        virtual status_t setUlInputSource(uint32 Source);

        /**
         * a function for tell AudioResourceManager mMode
         * @param mMode
         */
        virtual status_t SetAudioMode(audio_mode_t NewMode);

        /**
         * a function for tell AudioResourceManager mMode
         * @param mMode
         */

        virtual audio_mode_t GetAudioMode();

        /**
         * a function for tell AudioResourceManager select StartOutputDevice
         */
        virtual status_t StartOutputDevice();

        /**
         * a function for tell AudioResourceManager select StopOutputDevice
         */
        virtual status_t StopOutputDevice();

        /**
        * a function for tell AudioResourceManager select outputdevice
        */
        virtual status_t SelectOutputDevice(uint32_t new_device);

        /**
         * a function for tell AudioResourceManager select StartInputDevice
         */
        virtual status_t StartInputDevice();

        /**
         * a function for tell AudioResourceManager select StopInputDevice
         */
        virtual status_t StopInputDevice();

        /**
         * a function for tell AudioResourceManager select inputdevice
         */
        virtual status_t SelectInputDevice(uint32_t device) ;

        /**
         * a function for tell AudioResourceManager get current output device
         */
        virtual uint32 getDlOutputDevice(void);

        /**
         * a function for tell AudioResourceManager get current input device
         */
        virtual uint32 getUlInputDevice(void);

        /**
         * a function for tell AudioResourceManager to aquire lock
         */
        virtual status_t EnableAudioLock(int AudioLockType, int mTimeout);

        /**
         * a function for tell AudioResourceManager to release lock
         */
        virtual status_t DisableAudioLock(int AudioLockType);

        /**
        * a function for tell AudioResourceManager to request  or release clock
        */
        virtual status_t EnableAudioClock(int AudioLockType , bool bEnable);

        /**
        * a  function fo setParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , int command2 , unsigned int data);

        /**
        * a function fo setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , void *data);

        /**
        * a function fo getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy size of data
        */
        virtual int getParameters(int command1 , int command2 , void *data);

        /**
        * a function fo dump AudioResourceManager , can base on command1 to dump different information
        * @param command1
        * @param command2
        */
        virtual int dump(int command1);

        /**
        * a function to turn on audiodevice
        * @param mDlOutputDevice
        */
        status_t  TurnonAudioDevice(unsigned int mDlOutputDevice);

        /**
        * a function to turn on audiodevice when incall mode
        * @param mDlOutputDevice
        */
        status_t TurnonAudioDeviceIncall(unsigned int mDlOutputDevice);

        /**
        * a function to turn off audiodevice
        * @param mDlOutputDevice
        */
        status_t  TurnoffAudioDevice(unsigned int mDlOutputDevice);

        /**

        */
        virtual bool IsWiredHeadsetOn(void);

        /**
        * a function to set audiohardawre pointer , audioresource manager will save this hardware pointer.
        * @param paudioHardware
        */
        virtual void SetHardwarePointer(void *paudioHardware);

        /**
        * a function to return Mode Incall
        */
        virtual bool IsModeIncall(void) ;

        /**
         * a function for to set input device gain , and will base on different audio mode normal , incall
         * ex: mic
         */
        virtual status_t SetInputDeviceGain();


        status_t SelectInPutMicEnable(bool bEnable);

        /**
        * a function for to get active device
        */
        uint32_t PopCount(uint32_t u);

        /**
        * a function for to incall mode device base on output device
        */
        uint32_t GetIncallMicDevice(uint32 device);

		/**
        * a function for to get MIC digital gain for HD Record
        */
        virtual long GetSwMICDigitalGain();

       /**
        * a function for doing setMode()
        */
        virtual status_t doSetMode();

       /**
        * a function for set mic inverse
        */
        virtual status_t SetMicInvserse(bool bEnable);

    private:
        AudioResourceManager();
        // use to open deivce file descriptor
        static AudioResourceManager *UniqueAudioResourceInstance;
        AudioResourceManager(const AudioResourceManager &);             // intentionally undefined
        AudioResourceManager &operator=(const AudioResourceManager &);  // intentionally undefined

        // lock for hardware , mopde, streamout , streamin .
        // lock require sequence , always hardware ==> mode ==> streamout ==> streamin
        AudioLock mAudioLock[AudioResourceManagerInterface::NUM_OF_AUDIO_LOCK];

        // user for clock control to kernel space
        int ClockCounter[AudioResourceManagerInterface::CLOCK_TYPE_MAX];
        int mFd;

        // when change device and output , need to change in Audio resource manager
        unsigned int mDlOutputDevice;
        unsigned int mUlInputDevice;
        unsigned int mUlInputSource;
        unsigned int mUlI2SInputDevice;
        unsigned int mUlI2SInputSource;
        audio_mode_t mAudioMode;
        int mBgsStatus;
        int mPlaybackstatus;
        int mRecordingstatus;
        int m2waystatus;
        int m4waystatus;
        bool mMicDefaultsetting;
        bool mMicInverseSetting;

        AudioMTKVolumeInterface *mAudioVolumeInstance;
        AudioAnalogControlInterface *mAudioAnalogInstance;
        AudioDigitalControlInterface *mAudioDigitalInstance;
        //SpeechControlInterface *mSpeechControlInstance1;
        //SpeechControlInterface *mSpeechControlInstance2;
        AudioMTKHardware *mAudioHardware;
};

}

#endif
