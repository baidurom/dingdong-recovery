#include "AudioMachineDevice.h"
#include "AudioAnalogType.h"
#include "AudioIoctl.h"
#include "audio_custom_exp.h"
#include "AudioUtility.h"
#include <utils/Log.h>

#define LOG_TAG "AudioMachineDevice"
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


#define PMIC_TRIM_ADDRESS1 (0x1c2)
#define PMIC_TRIM_ADDRESS2 (0x1c4)
#define PMIC_TRIM_REG1_DEFAULT (0x0220)
#define PMIC_TRIM_REG2_DEFAULT (0x0006)

#define PMIC_TRIM_SPK_ADDRESS (0x01CA)
#define PMIC_TRIM_SPKREG1_DEFAULT (0x0010)
#define PMIC_AUTO_TRIM_ADRESS (0x014E)

namespace android
{

status_t AudioMachineDevice::InitCheck()
{
    ALOGD("InitCheck");
    return NO_ERROR;
}

 int AudioMachineDevice::GetChipVersion()
{
    int ret = ::ioctl(mFd, GET_PMIC_VERSION, 0);
    //ALOGD("GetChipVersion ret = %d",ret);
    return ret;
}

AudioMachineDevice::AudioMachineDevice()
{
    mAudioAnalogReg = NULL;
    mAudioAnalogReg = AudioAnalogReg::getInstance();
    mFd =0;
    if (!mAudioAnalogReg)
    {
        ALOGW("AudioMachineDevice init mAudioAnalogReg fail =  %p", mAudioAnalogReg);
    }
    // init analog part.
    mFd = ::open(kAudioDeviceName, O_RDWR);
    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++) {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++) {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }
}

status_t AudioMachineDevice::StartHeadphoneTrimFunction()
{
    GetTrimOffset();
    if(GetChipVersion() == CHIP_VERSION_E1)
    {
        mAudioAnalogReg->SetAnalogReg(0x071A, 0x1000, 0x1000);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::StartSpkTrimFunction()
{
    #ifdef USING_EXTAMP_HP
    #else
    if(GetChipVersion() == CHIP_VERSION_E1){
        GetSPKTrimOffset();
    }
    else
    {
        AnalogOpen(AudioAnalogType::DEVICE_OUT_SPEAKERL);
        SPKAutoTrimOffset(); // use spk auto trim
        GetSPKAutoTrimOffset();
    }
    #endif
    return NO_ERROR;
}

status_t AudioMachineDevice::GetTrimOffset()
{
     uint32 reg1 = 0, reg2 =0;
     bool trim_enable =0;
     // get to check if trim happen
     reg1 = mAudioAnalogReg->GetAnalogReg (PMIC_TRIM_ADDRESS1);
     reg2 = mAudioAnalogReg->GetAnalogReg (PMIC_TRIM_ADDRESS2);
     ALOGD("reg1 = 0x%x reg2 = 0x%x",reg1,reg2);
     trim_enable = (reg1>>12)&1;
     if(trim_enable == 0)
     {
         reg1 = (PMIC_TRIM_REG1_DEFAULT&0xfff0);
         reg2 = (PMIC_TRIM_REG2_DEFAULT&0x0fff);
     }
     ALOGD("trim_enable = %d reg1 = 0x%x reg2 = 0x%x",trim_enable,reg1,reg2);
     mHPRtrim = (reg1>>8)&0x000f;
     mHPRfinetrim =((reg1 >> 15)&0x0001) | ((reg2&0x0001)<<1);
     mHPLtrim =  (reg1>>4) &0x000f;
     mHPLfinetrim =(reg1>>13)&0x0003;
     mIVHPLtrim = (reg2>>1)&0x000f;
     mIVHPLfinetrim  = (reg2>>9)&0x0003;
     ALOGD("GetTrimOffset mHPRtrim = 0x%x mHPRfinetrim = 0x%x mHPLtrim = 0x%x mHPLfinetrim = 0x%x mIVHPLtrim = 0x%x mIVHPLfinetrim = 0x%x",
         mHPRtrim,mHPRfinetrim,mHPLtrim,mHPLfinetrim,mIVHPLtrim,mIVHPLfinetrim);

     return NO_ERROR;
}

status_t AudioMachineDevice::GetSPKTrimOffset()
{
     uint32 reg1 = 0;
     bool trim_enable =0;
     // get to check if trim happen
     reg1 = mAudioAnalogReg->GetAnalogReg (PMIC_TRIM_SPK_ADDRESS);
     trim_enable =  (reg1>>13)&0x1;
     if(trim_enable ==0)
     {
         reg1 = PMIC_TRIM_SPKREG1_DEFAULT;
     }
     mSPKpolarity = (reg1>>12)&0x1;
     mISPKtrim = (reg1>>7)&0x1f;
     ALOGD("trim_enable = %d mSPKpolarity = %d mISPKtrim = 0x%x",
         trim_enable,mSPKpolarity,mISPKtrim);
     return NO_ERROR;
}

status_t AudioMachineDevice::SPKAutoTrimOffset()
{
    uint32_t WaitforReady =0;
    int retyrcount = 10;
   mAudioAnalogReg->SetAnalogReg(SPK_CON9,0x2018,0xffff);    // Choose new mode for trim (E2 Trim)
   mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x0008,0xffff);// Enable auto trim
   mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x3000,0xf000);// set gain
   mAudioAnalogReg->SetAnalogReg(SPK_CON9,0x0a00,0x0f00);// set gain
   mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x0001,0x0001);// Enable amplifier & auto trim
   do
   {
       WaitforReady = mAudioAnalogReg->GetAnalogReg (SPK_CON1);
       WaitforReady = (WaitforReady&0x8000)>>14;
       usleep(10*1000);
       ALOGD("SPKAutoTrimOffset sleep........");
   }
   while((!WaitforReady)&&retyrcount);
   ALOGD("SPKAutoTrimOffset done");
   mAudioAnalogReg->SetAnalogReg(SPK_CON9,0x0,0xffff);
   mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x0000,0x0001);
   return NO_ERROR;
}

status_t AudioMachineDevice::GetSPKAutoTrimOffset()
{
     uint32 reg1 = 0;
     ALOGD("GetSPKAutoTrimOffset ");
     mAudioAnalogReg->SetAnalogReg(0x013A,0x0802,0xffff); // totally hardcode....
     reg1 = mAudioAnalogReg->GetAnalogReg (PMIC_AUTO_TRIM_ADRESS);
     mSPKpolarity = (reg1>>9)&0x1;
     mISPKtrim = (reg1>>10)&0x1f;
     ALOGD("mSPKpolarity = %d mISPKtrim = 0x%x",mSPKpolarity,mISPKtrim);
     return NO_ERROR;
}

status_t AudioMachineDevice::SetSPKTrimOffset(void)
{
    uint32 AUDBUG_reg =0;
    AUDBUG_reg |= 1<<14;// enable trim function
    AUDBUG_reg |= mSPKpolarity <<13 ; // polarity
    AUDBUG_reg |= mISPKtrim <<8 ; // polarity
    ALOGD("SetSPKTrimOffset AUDBUG_reg = 0x%x",AUDBUG_reg);
    mAudioAnalogReg->SetAnalogReg (SPK_CON1, AUDBUG_reg, 0x7f00);
    return NO_ERROR;
}

status_t AudioMachineDevice::SetHPTrimOffset(void)
{
    uint32 AUDBUG_reg =0;
    ALOGD("SetHPTrimOffset");
    AUDBUG_reg |= 1<<8;// enable trim function
    AUDBUG_reg |= mHPRfinetrim <<11 ;
    AUDBUG_reg |= mHPLfinetrim<<9;
    AUDBUG_reg |= mHPRtrim<<4;
    AUDBUG_reg |= mHPLtrim;
    mAudioAnalogReg->SetAnalogReg (AUDBUF_CFG3, AUDBUG_reg, 0x1fff);
    return NO_ERROR;
}

status_t AudioMachineDevice::SetIVHPTrimOffset(void)
{
    uint32 AUDBUG_reg =0;
    ALOGD("SetIVHPTrimOffset");
    AUDBUG_reg |= 1<<8;// enable trim function
    AUDBUG_reg |= mHPRfinetrim <<11 ;
    if((mHPRfinetrim ==0) || (mHPLfinetrim == 0))
    {
        mIVHPLfinetrim =0;
    }
    else
    {
        mIVHPLfinetrim = 2;
    }
    AUDBUG_reg |= mIVHPLfinetrim<<9;
    AUDBUG_reg |= mHPRtrim<<4;
    AUDBUG_reg |= mIVHPLtrim;
    mAudioAnalogReg->SetAnalogReg (AUDBUF_CFG3, AUDBUG_reg, 0x1fff);
    return NO_ERROR;
}

bool AudioMachineDevice::GetDownLinkStatus(void)
{
    for(int i=0; i <= AudioAnalogType::DEVICE_2IN1_SPK; i++)
    {
        if(mBlockAttribute[i].mEnable == true)
            return true;
    }
    return false;
}

bool AudioMachineDevice::GetULinkStatus(void)
{
    for(int i= AudioAnalogType::DEVICE_IN_ADC1; i <= AudioAnalogType::DEVICE_IN_DIGITAL_MIC; i++)
    {
        if(mBlockAttribute[i].mEnable == true)
            return true;
    }
    return false;
}

status_t AudioMachineDevice::SetAmpGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec, use 15dB for
    uint32 index =  13;

    // condition for gainb not mute
    if(volume > 11){
        volume = 11;
    }
    //const int HWgain[] =  {-60,0,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    index -= volume;
    if (index < 1) {
        index= 1; // min to 0dB
    }
    if(volume_Type == AudioAnalogType::VOLUME_SPKL ||volume_Type == AudioAnalogType::VOLUME_SPKR)
    {
        mAudioAnalogReg->SetAnalogReg(SPK_CON9,index<<8,0x00000f00);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetHandSetGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec.
    uint32 index=  0;
    //const int HWgain[] =  {8,7,6,5,4,3,2,1,0,-1,-2,-3,-4};
    index += volume;
    if(index > 12){
        index= 12;
    }
    if(volume_Type == AudioAnalogType::VOLUME_HSOUTL ||volume_Type == AudioAnalogType::VOLUME_HSOUTR)
    {
        mAudioAnalogReg->SetAnalogReg(ZCD_CON3,index,0x0000000f);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetHeadPhoneGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec.
    uint32 index=  0;
    //const int HWgain[] = {8,7,6,5,4,3,2,1,0,-1,-2,-3,-4};
    index  += (volume);
    if(index > 12){
        index = 12;
    }
    if(volume_Type == AudioAnalogType::VOLUME_HPOUTL)
    {
        mAudioAnalogReg->SetAnalogReg(ZCD_CON2,index,0x0000000f);
    }
    else if(volume_Type == AudioAnalogType::VOLUME_HPOUTR)
    {
        mAudioAnalogReg->SetAnalogReg(ZCD_CON2,index<<8,0x00000f00);
    }
    return NO_ERROR;
}

 status_t AudioMachineDevice::SetLineinGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
     // this will base on hw spec.
     uint32 index=  0;
    //const int HWgain[] = {10, 8,6,4,2,0,-2,-4,-6,-8,-10,-12,-14,-16,-18,-20};
     index += (volume/2);
     if(index > 15){
         index= 15;
     }
     if(volume_Type == AudioAnalogType::VOLUME_LINEINL ||volume_Type == AudioAnalogType::VOLUME_LINEINR)
     {
         mAudioAnalogReg->SetAnalogReg(ZCD_CON1,index,0x0000000f);
     }
     return NO_ERROR;
}

status_t AudioMachineDevice::SetPreampBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
   ALOGD("SetPreampBufferGain volume_Type = %d volume = %d",volume_Type,volume);
    // this will base on hw spec.
    uint32 index=  5;

    // condifiton for gain.
    volume = volume/6;
    if(volume > 5)
        volume = 5;

    //const int PreAmpGain[] = {2, 8, 14, 20, 26, 32};
    index -= volume;
    if(index < 0){
        index = 0;
    }
    if(volume_Type == AudioAnalogType::VOLUME_MICAMPR)
    {
        mAudioAnalogReg->SetAnalogReg(AUDPREAMPGAIN_CON0,index<<4,0x00000070);
    }
    else if(volume_Type == AudioAnalogType::VOLUME_MICAMPL)
    {
        mAudioAnalogReg->SetAnalogReg(AUDPREAMPGAIN_CON0,index<<0,0x00000007);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetLevelShiftBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec.
    uint32 index=  7;
    volume = volume/3;
    if(volume>7)
        volume =7;
    //const int LevelShiftGain[] = {-3, 0, 3, 6, 9, 12, 15, 18};
    index -= volume;
    if(index < 0){
        index = 0;
    }
    if(volume_Type == AudioAnalogType::VOLUME_LEVELSHIFTL)
    {
        mAudioAnalogReg->SetAnalogReg(AUDLSBUF_CON0,index<<2,0x0000001c);
    }
    else if(volume_Type == AudioAnalogType::VOLUME_LEVELSHIFTR)
    {
        mAudioAnalogReg->SetAnalogReg(AUDLSBUF_CON0,index<<9,0x00000e00);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetIVBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec.
    uint32 index=  0;
    //const int LevelShiftGain[] = {5,4,3,2,1,0,-1,-2};
    index += volume;
    if(index > 7){
        index = 7;
    }
    index |= (index << 8);
    if(volume_Type == AudioAnalogType::VOLUME_IV_BUFFER)
    {
        mAudioAnalogReg->SetAnalogReg(ZCD_CON4,index,0x00000e0e);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetZCDStatus(AudioAnalogType::AUDIOANALOGZCD_TYPE ZcdType,
                                        AudioMachineDevice::ZCD_GAIN_STEP Gainstep ,
                                        AudioMachineDevice::ZCD_GAIN_TIME GainTime ,
                                        bool Enable)
{
    return NO_ERROR;
}

/**
,* a basic function for SetAnalogGain for different Volume Type
* @param VoleumType value want to set to analog volume
* @param volume function of analog gain , value between 0 ~ 255
* @return status_t
*/
status_t AudioMachineDevice::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("SetAnalogGain VOLUME_TYPE = %d volume = %d ", VoleumType, volume);
    switch (VoleumType) {
        case AudioAnalogType::VOLUME_HSOUTL:
        case AudioAnalogType::VOLUME_HSOUTR:
            SetHandSetGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_HPOUTL:
        case AudioAnalogType::VOLUME_HPOUTR:
            SetHeadPhoneGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_SPKL:
        case AudioAnalogType::VOLUME_SPKR:
            SetAmpGain( VoleumType,  volume);
            break;
        case AudioAnalogType::VOLUME_LINEINL:
        case AudioAnalogType::VOLUME_LINEINR:
            SetLineinGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_MICAMPL:
        case AudioAnalogType::VOLUME_MICAMPR:
            SetPreampBufferGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_LEVELSHIFTL:
        case AudioAnalogType::VOLUME_LEVELSHIFTR:
            SetLevelShiftBufferGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_IV_BUFFER:
            SetIVBufferGain(VoleumType, volume);
            break;
            // defdault no support device
        case AudioAnalogType::VOLUME_LINEOUTL:
        case AudioAnalogType::VOLUME_LINEOUTR:
            break;

    }
    return NO_ERROR;
}


/**
* a basic function for GetAnalogGain for different Volume Type
* @param VoleumType value want to get analog volume
* @return int
*/
int AudioMachineDevice::GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType)
{
    int volume =0;
    uint32 regvalue =0;
    // here only implement DL gain .
    switch (VoleumType)
    {
        case AudioAnalogType::VOLUME_HSOUTL:
        case AudioAnalogType::VOLUME_HSOUTR:
            regvalue = mAudioAnalogReg->GetAnalogReg (ZCD_CON3);
            volume = regvalue&0xf;
            volume = 8 - volume ;
            break;
        case AudioAnalogType::VOLUME_HPOUTL:
        case AudioAnalogType::VOLUME_HPOUTR:
            // here only use L as volume
            regvalue = mAudioAnalogReg->GetAnalogReg (ZCD_CON2);
            volume = regvalue&0xf;
            volume =  8 - volume ;
            break;
        case AudioAnalogType::VOLUME_SPKL:
        case AudioAnalogType::VOLUME_SPKR:
            #ifdef USING_EXTAMP_HP
            regvalue = mAudioAnalogReg->GetAnalogReg (ZCD_CON2);
            #else
            regvalue = mAudioAnalogReg->GetAnalogReg (SPK_CON9);
            #endif
            volume = regvalue&0xf00;
            volume = volume >> 7;
            if(volume ==0)
               volume = -64;
            else
               volume = volume +1;
            break;
        default:
            break;
    }
    ALOGD("GetAnalogGain regvalue = 0x%x volume = %d VoleumType = %d",regvalue,volume,VoleumType);
    return volume;
}

/**
* a basic function fo SetAnalogMute, if provide mute function of hardware.
* @param VoleumType value want to set to analog volume
* @param mute of volume type
* @return status_t
*/
status_t AudioMachineDevice::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("SetAnalogMute VOLUME_TYPE = %d mute = %d ", VoleumType, mute);
    return NO_ERROR;
}

/**
* a basic function fo AnalogOpen, open analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioMachineDevice::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioMachineDevice AnalogOpen DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock ();
    if(mBlockAttribute[DeviceType].mEnable == true)
    {
        ALOGW("AnalogOpen with DeviceType = %d",DeviceType);
        mLock.unlock ();
        return NO_ERROR;;
    }
    mBlockAttribute[DeviceType].mEnable = true;
    switch (DeviceType) {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            mAudioAnalogReg->SetAnalogReg(0x0714, 0x0D92, 0xffff); // enable VA28 , VA 33 VBAT ref , set dc
            mAudioAnalogReg->SetAnalogReg(0x0718, 0x000C, 0xffff); // set ACC mode  enable NVREF
            mAudioAnalogReg->SetAnalogReg(0x071A, 0xE000, 0xE000); // enable LDO ; seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0744, 0x102B, 0xffff); // RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0746, 0x0000, 0xffff); // NCP on
            usleep(1*1000);

            mAudioAnalogReg->SetAnalogReg(0x0738, 0x0201, 0xffff); // ZCD setting gain step gain and enable
            mAudioAnalogReg->SetAnalogReg(0x070E, 0x0030, 0xffff); // select charge current l; fix me
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0008, 0xffff); // set voice playback with headset
            mAudioAnalogReg->SetAnalogReg(0x070C, 0x0552, 0xffff); // audio bias adjustment

            mAudioAnalogReg->SetAnalogReg(0x073E, 0x000F, 0xffff); // handset gain , minimun gain
            mAudioAnalogReg->SetAnalogReg(0x0706, 0x00A2, 0xffff); // short HS to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0001, 0xffff); // handset gain , minimun gain
            mAudioAnalogReg->SetAnalogReg(0x0706, 0x0022, 0xffff); // short HS to vcm and HS output stability enhance

            mAudioAnalogReg->SetAnalogReg(0x073E, 0x0002, 0xffff); // handset gain , normal gain
            mAudioAnalogReg->SetAnalogReg(0x0712, 0x0001, 0x0001); // reset decoder
            mAudioAnalogReg->SetAnalogReg(0x0700, 0x0009, 0xffff); // power on audio DAC right channels
            usleep(1000);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);

            AnalogSetMux(AudioAnalogType::DEVICE_OUT_EARPIECER, (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            AnalogSetMux(AudioAnalogType::DEVICE_OUT_EARPIECEL, (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0001, 0x0001); // mux selection
            usleep(1000);

            // tell kernel to open device
            ioctl(mFd,SET_EARPIECE_ON,NULL);
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            SetHPTrimOffset();
            if(mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINR].mEnable || mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINL].mEnable)
            {
                mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG3, 0x4000, 0x6000);  // set pull low enablle
            }
            mAudioAnalogReg->SetAnalogReg(0x0714, 0x0D92, 0xffff); // enable VA28 , VA 33 VBAT ref , set dc
            mAudioAnalogReg->SetAnalogReg(0x0718, 0x000C, 0xffff); // set ACC mode  enable NVREF
            mAudioAnalogReg->SetAnalogReg(0x071A, 0xE000, 0xE000); // enable LDO ; fix me , seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0744, 0x102b, 0xffff); //RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0746, 0x0000, 0xffff); // NCP on
            usleep(1*1000);

            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0008, 0xffff); // set voice playback with headset
            mAudioAnalogReg->SetAnalogReg(0x0738, 0x0101, 0xffff); // ZCD setting gain step gain and enable
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0008, 0xffff); // set voice playback with headset
            mAudioAnalogReg->SetAnalogReg(0x070C, 0x0552, 0xffff); // audio bias adjustment
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0900, 0xffff); // short HP to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0706, 0x0082, 0xffff); // short HS to vcm and HS output stability EnhanceParasNum

            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0009, 0xffff); // open and headset power on
            usleep(1000);
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0940, 0xffff); // not short HP to vcm
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x000F, 0xffff); // HP power on
            usleep(1000);
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0100, 0xffff); // HP enhance
            mAudioAnalogReg->SetAnalogReg(0x0706, 0x0082, 0xffff); // short HS to vcm and HS output stability EnhanceParasNum
            mAudioAnalogReg->SetAnalogReg(0x073C, 0x0c0c, 0xffff); // HP PA gain
            usleep(1*1000);

            mAudioAnalogReg->SetAnalogReg(0x0712, 0x0001, 0x0001); // reset docoder
            mAudioAnalogReg->SetAnalogReg(0x0700, 0x000F, 0xffff); // power on DAC
            usleep(1000);

            if(mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINR].mEnable || mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINL].mEnable)
            {
                usleep(30*1000);
            }
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0006, 0x0007);

            AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR, (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);


            if(mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINR].mEnable || mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINL].mEnable)
            {
                mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG3,0x0000,0x6000); // set pull low resistance
            }
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            ioctl(mFd,SET_HEADPHONE_ON,NULL); // tell kernel to open device
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
            #ifdef USING_EXTAMP_HP
            mLock.unlock ();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock ();
            #else
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            SetSPKTrimOffset();
            mAudioAnalogReg->SetAnalogReg(0x0714, 0x0D92, 0xffff); // enable VA28 , VA 33 VBAT ref , set dc
            mAudioAnalogReg->SetAnalogReg(0x0718, 0x000C, 0xffff); // set ACC mode  enable NVREF
            mAudioAnalogReg->SetAnalogReg(0x071A, 0xE000, 0xE000); // enable LDO ; fix me , seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0744, 0x102B, 0xffff); //RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0746, 0x0000, 0xffff); // NCP on
            usleep(1*1000);

            mAudioAnalogReg->SetAnalogReg(0x0738, 0x0301, 0xffff); // ZCD setting gain step gain and enable
            mAudioAnalogReg->SetAnalogReg(0x070E, 0x0030, 0xffff); // select charge current l; fix me
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0008, 0xffff); // set voice playback with headset
            mAudioAnalogReg->SetAnalogReg(0x070C, 0x0552, 0xffff); // audio bias adjustment
            mAudioAnalogReg->SetAnalogReg(0x073C, 0x0C0C, 0xffff); // headphone gain , minimun gain
            mAudioAnalogReg->SetAnalogReg(0x073E, 0x000F, 0xffff); // handset gain , normal gain
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0900, 0xffff); // short HP to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0706, 0x0082, 0xffff); // short HS to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0009, 0xffff); // open and headset power on ; fix me
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0940, 0xffff); // not short HP to vcm
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0007, 0xffff); // HP power on
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0000, 0xffff); // HP enhance
            mAudioAnalogReg->SetAnalogReg(0x0706,0x0022,0xffff);// HS enahnce
            mAudioAnalogReg->SetAnalogReg(0x073C,0x0505,0xffff);// HP PA gain
            mAudioAnalogReg->SetAnalogReg(0x0740,0x0505,0xffff); // set DUDIV gain ,iv buffer gain
            mAudioAnalogReg->SetAnalogReg(0x0710,0x0011,0xffff); // set IV buffer on
            mAudioAnalogReg->SetAnalogReg(0x0712,0x0001,0x0001); // reset docoder
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0009,0xffff); // power on DAC
            usleep(1000);
            AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR,  (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL,  (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0000,0x0007); // set Mux
            usleep(1000);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);

            #ifdef USING_CLASSD_AMP
            mAudioAnalogReg->SetAnalogReg(0x0600,0x3009,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0604,0x0014,0xffff);
            #else
            mAudioAnalogReg->SetAnalogReg(0x0600,0x3005,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0604,0x0014,0xffff);
            #endif

            mAudioAnalogReg->SetAnalogReg(0x0612,0x0800,0xffff); // SPK gain setting
            mAudioAnalogReg->SetAnalogReg(0x0616,0x0f00,0xffff); // spk output stage enabke and enable
            #endif
           // tell kernel to open device
            ioctl(mFd,SET_SPEAKER_ON,NULL);
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            #ifdef USING_EXTAMP_HP
            mLock.unlock ();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock ();
            #else
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            SetHPTrimOffset();
            SetIVHPTrimOffset();
            SetSPKTrimOffset();

            if(mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINR].mEnable || mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINL].mEnable)
            {
                mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG3, 0x4000, 0x6000);  // set pull low enablle
            }
            mAudioAnalogReg->SetAnalogReg(0x0714,0x0D92,0xffff);// enable VA28 , VA 33 VBAT ref , set dc
            mAudioAnalogReg->SetAnalogReg(0x0718,0x000C,0xffff);// set ACC mode  enable NVREF
            mAudioAnalogReg->SetAnalogReg(0x071A,0xE000,0xE000);// enable LDO ; fix me , seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0744,0x102B,0xffff); //RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0746,0x0000,0xffff);// NCP on
            usleep(1*1000);

            mAudioAnalogReg->SetAnalogReg(0x0738,0x0301,0xffff); // ZCD setting gain step gain and enable
            mAudioAnalogReg->SetAnalogReg(0x070E,0x0030,0xffff);// select charge current ; fix me
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0008,0xffff);// set voice playback with headset
            mAudioAnalogReg->SetAnalogReg(0x070C,0x0552,0xffff);// audio bias adjustment
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0900,0xffff);// HP enhance
            mAudioAnalogReg->SetAnalogReg(0x0706,0x0082,0xffff);// HS enahnce
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0009,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0940,0xffff); // HP vcm short
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0007,0xffff); //HP power on
            usleep(1000);
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0100,0xffff); // HP vcm not short
            mAudioAnalogReg->SetAnalogReg(0x0706,0x0022,0xffff); // HS VCM not short
            mAudioAnalogReg->SetAnalogReg(0x073C,0x0808,0xffff) ;// HP PGA gain
            mAudioAnalogReg->SetAnalogReg(0x0740,0x0505,0xffff); // set DUDIV gain ,iv buffer gain
            mAudioAnalogReg->SetAnalogReg(0x0710,0x0011,0xffff); // set IV buffer on
            usleep(1*1000);
            mAudioAnalogReg->SetAnalogReg(0x0712,0x0001,0x0001) ;// reset docoder
            mAudioAnalogReg->SetAnalogReg(0x0700,0x000F,0xffff); // power on DAC

            if(mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINR].mEnable || mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINL].mEnable)
            {
                usleep(50*1000);
            }

            AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR,  (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL,  (AudioAnalogType::MUX_TYPE )mBlockAttribute[DeviceType].mMuxSelect);
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0906,0x0906); // set headhpone mux
            usleep(1000);
            if(mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINR].mEnable || mBlockAttribute[AudioAnalogType::DEVICE_IN_LINEINL].mEnable)
            {
                mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG3,0x0000,0x6000); // set pull low resistance
            }

            #ifdef USING_CLASSD_AMP
            mAudioAnalogReg->SetAnalogReg(0x0600,0x3009,0xffff); // speaker gain setting , trim enable , spk enable , class AB or D
            mAudioAnalogReg->SetAnalogReg(0x0604,0x0014,0xffff); // speaker gain setting , trim enable , spk enable , class AB or D
            #else
            mAudioAnalogReg->SetAnalogReg(0x0600,0x3005,0xffff); // speaker gain setting , trim enable , spk enable , class AB or D
            mAudioAnalogReg->SetAnalogReg(0x0604,0x0014,0xffff); // speaker gain setting , trim enable , spk enable , class AB or D
            #endif

            mAudioAnalogReg->SetAnalogReg(0x0612,0x0400,0xffff); // SPK gain setting
            mAudioAnalogReg->SetAnalogReg(0x0616,0x0f00,0xffff); // spk output stage enabke and enableAudioClockPortDST
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            #endif

            // tell kernel to open device
            ioctl(mFd,SET_SPEAKER_ON,NULL);
            ioctl(mFd,SET_HEADPHONE_ON,NULL);
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            #ifdef USING_2IN1_SPEAKER
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            mAudioAnalogReg->SetAnalogReg(0x0714, 0x0D92, 0xffff); // enable VA28 , VA 33 VBAT ref , set dc
            mAudioAnalogReg->SetAnalogReg(0x0718, 0x000C, 0xffff); // set ACC mode  enable NVREF
            mAudioAnalogReg->SetAnalogReg(0x071A, 0xE000, 0xE000); // enable LDO ; fix me , seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0744, 0x102B, 0xffff); //RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0746, 0x0000, 0xffff); // NCP on
            usleep(1000);

            mAudioAnalogReg->SetAnalogReg(0x0738, 0x0301, 0xffff); // ZCD setting gain step gain and enable
            mAudioAnalogReg->SetAnalogReg(0x070E, 0x0030, 0xffff); // select charge current l; fix me
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0008, 0xffff); // set voice playback with headset
            mAudioAnalogReg->SetAnalogReg(0x070C, 0x0552, 0xffff); // audio bias adjustment
            mAudioAnalogReg->SetAnalogReg(0x073C, 0x0C0C, 0xffff); // headphone gain , minimun gain
            mAudioAnalogReg->SetAnalogReg(0x073E, 0x000F, 0xffff); // handset gain , normal gain
            mAudioAnalogReg->SetAnalogReg(0x0704, 0x0900, 0xffff); // short HP to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0706, 0x0082, 0xffff); // short HS to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0009, 0xffff); // open and headset power on ;
            mAudioAnalogReg->SetAnalogReg(0x0702, 0x0007, 0xffff); // HP power on
            usleep(1000);
            mAudioAnalogReg->SetAnalogReg(0x0706,0x0022,0xffff);// HS enahnce
            mAudioAnalogReg->SetAnalogReg(0x073C,0x0505,0xffff);// HP PA gain
            mAudioAnalogReg->SetAnalogReg(0x0740,0x0505,0xffff); // set DUDIV gain ,iv buffer gain
            usleep(1*1000);
            mAudioAnalogReg->SetAnalogReg(0x0710,0x0011,0xffff); // set IV buffer on
            mAudioAnalogReg->SetAnalogReg(0x0712,0x0001,0x0001); // reset docoder
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0009,0xffff); // power on DAC
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0000,0x0007); // set MuxOMX

            // class AB , -3 and 0db.
            mAudioAnalogReg->SetAnalogReg(0x0600,0x2005,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0604,0x0014,0xffff); // speaker gain setting , trim enable , spk enable , class AB or D

            mAudioAnalogReg->SetAnalogReg(0x0612,0x0100,0xffff); // SPK gain setting 0db
            mAudioAnalogReg->SetAnalogReg(0x0616,0x0f00,0xffff); // spk output stage enabke and enable
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            #endif
            break;
        case AudioAnalogType::DEVICE_IN_LINEINR:
        case AudioAnalogType::DEVICE_IN_LINEINL:
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            ALOGD("AudioAnalogType::DEVICE_IN_ADC2:");
            mAudioAnalogReg->SetAnalogReg(0x0718 , 0x000c, 0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0714 , 0x0D92, 0xffff);
            mAudioAnalogReg->SetAnalogReg(0x071A , 0x9000, 0x9000);
            if( mBlockAttribute[DeviceType].mMuxSelect == AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER ) {
                mAudioAnalogReg->SetAnalogReg(0x0730 , 0x0011, 0x003f); // Select LSB MUX as Line-In
                mAudioAnalogReg->SetAnalogReg(0x072E , 0x0003, 0x0003); // Power On LSB
            }
            else { // If not LSB, then must use preAmplifier. So set MUX of preamplifier
                AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L,  (AudioAnalogType::MUX_TYPE )mBlockAttribute[AudioAnalogType::DEVICE_IN_PREAMP_L].mMuxSelect);
                AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R,  (AudioAnalogType::MUX_TYPE )mBlockAttribute[AudioAnalogType::DEVICE_IN_PREAMP_R].mMuxSelect);
            }
            mAudioAnalogReg->SetAnalogReg(0x071c , 0x0003, 0x0003); // open power
            usleep(1*1000);
            if( mBlockAttribute[DeviceType].mMuxSelect == AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER ) {
                mAudioAnalogReg->SetAnalogReg(0x071e , 0x00B7, 0x00ff); // Set ADC input as LSB
            }
            else{
                mAudioAnalogReg->SetAnalogReg(0x071e , 0x0093, 0xffff);
            }
            mAudioAnalogReg->SetAnalogReg(0x0744 , 0x102B, 0x102B);
            mAudioAnalogReg->SetAnalogReg(0x0746 , 0x0000, 0xffff);
            mAudioAnalogReg->SetAnalogReg(0x072c , 0x0180, 0x0180);
            mAudioAnalogReg->SetAnalogReg(0x0736 , 0x0033, 0x0033);

            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(0x0718 , 0x000c, 0xffff);
            mAudioAnalogReg->SetAnalogReg(0x072C , 0x0181, 0xffff);
            break;
        case AudioAnalogType::DEVICE_OUT_LINEOUTR:
        case AudioAnalogType::DEVICE_OUT_LINEOUTL:
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            break;
    }
    mLock.unlock ();
    return NO_ERROR;
}


status_t AudioMachineDevice::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("SetFrequency");
    mBlockSampleRate[DeviceType] = frequency;
    return NO_ERROR;
}

/**
* a basic function fo AnalogClose, ckose analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioMachineDevice::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AnalogClose DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock ();
    mBlockAttribute[DeviceType].mEnable = false;
    switch (DeviceType) {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            // tell kernel to open device
            ioctl(mFd,SET_EARPIECE_OFF,NULL);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);

            mAudioAnalogReg->SetAnalogReg(0x0706,0x0022,0xffff); // short HS to vcm and HS output stability enhance
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0880,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x070c,0x1552,0xffff);  //RG DEV ck off
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0000,0xffff);  // NCP off
            mAudioAnalogReg->SetAnalogReg(0x0712,0x0000,0x0001);  // Audio headset power off
            mAudioAnalogReg->SetAnalogReg(0x0706,0x0022,0xffff); // short HS to vcm and HS output stability EnhanceParasNum
            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0004,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0006,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(0x0746,0x0001,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x071A,0x0000,0x6000);
            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0992,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0192,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            // tell kernel to open device
            ioctl(mFd,SET_HEADPHONE_OFF,NULL);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            mAudioAnalogReg->SetAnalogReg(0x073C,0x0c0c,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0880,0x1fe0);
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0000,0x0007);
            mAudioAnalogReg->SetAnalogReg(0x070c,0x1552,0xffff);  //RG DEV ck off;
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0000,0xffff);  // NCP off
            mAudioAnalogReg->SetAnalogReg(0x0712,0x0000,0x0001);
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0000,0x0100);

            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0004,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0006,0xffff);
                }
            }

            mAudioAnalogReg->SetAnalogReg(0x0746,0x0001,0xffff);   // fix me
            mAudioAnalogReg->SetAnalogReg(0x071A,0x0000,0x6000);
            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0992,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0192,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
            // tell kernel to open device
            ioctl(mFd,SET_SPEAKER_OFF,NULL);

            #ifdef USING_EXTAMP_HP
            mLock.unlock ();
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock ();
            #else
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            mAudioAnalogReg->SetAnalogReg(0x0600,0x0000,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0616,0x0000,0xffff);

            mAudioAnalogReg->SetAnalogReg(0x0712,0x0000,0x0001);  // enable LDO ; fix me , seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0000,0xffff);  //RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0710,0x0000,0xffff);  // NCP on
            mAudioAnalogReg->SetAnalogReg(0x070c,0x1552,0xffff);  // Audio headset power on
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0000,0x0100);

            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0004,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0006,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(0x0746,0x0001,0xffff);   // fix me
            mAudioAnalogReg->SetAnalogReg(0x071A,0x0000,0x6000);
            if(GetULinkStatus () == false)
            {
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0992,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0192,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            #endif
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            mAudioAnalogReg->SetAnalogReg(0x0600,0x0000,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0616,0x0000,0xffff);

            mAudioAnalogReg->SetAnalogReg(0x0712,0x0000,0x0001);  // enable LDO ; fix me , seperate for UL  DL LDO
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0000,0xffff);  //RG DEV ck on
            mAudioAnalogReg->SetAnalogReg(0x0710,0x0000,0xffff);  // NCP on
            mAudioAnalogReg->SetAnalogReg(0x070c,0x1552,0xffff);  // Audio headset power on
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0000,0x0100);

            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0004,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0006,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(0x0746,0x0001,0xffff);   // fix me
            mAudioAnalogReg->SetAnalogReg(0x071A,0x0000,0x6000);
            if(GetULinkStatus () == false)
            {
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0992,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0192,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            // tell kernel to open device
            ioctl(mFd,SET_HEADPHONE_OFF,NULL);
            ioctl(mFd,SET_SPEAKER_OFF,NULL);
            #ifdef USING_EXTAMP_HP
            mLock.unlock ();
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock ();
            #else
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0080, 0x0080);
            mAudioAnalogReg->SetAnalogReg(0x0600,0x0000,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0616,0x0000,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x073c,0x0C0C,0x0f0f);
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0000,0x0007);
            mAudioAnalogReg->SetAnalogReg(0x0702,0x0880,0x1fe0);
            mAudioAnalogReg->SetAnalogReg(0x070c,0x1552,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0700,0x0000,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0712,0x0000,0x0001);
            mAudioAnalogReg->SetAnalogReg(0x0710,0x0010,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0704,0x0000,0x0100);
            mAudioAnalogReg->SetAnalogReg(0x0706,0x0000,0x0080);

            if(GetULinkStatus () == false){
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0004,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0006,0xffff);
                }
            }

            mAudioAnalogReg->SetAnalogReg(0x0746,0x0001,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x071A,0x0000,0x6000);
            if(GetULinkStatus () == false)
            {
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0992,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0192,0xffff);
                }
            }
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2, 0x0000, 0x0080);
            #endif

            break;
        case AudioAnalogType::DEVICE_IN_LINEINR:
        case AudioAnalogType::DEVICE_IN_LINEINL:
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            mAudioAnalogReg->SetAnalogReg(0x071C ,0x0000,0x0003); // LDO off
            mAudioAnalogReg->SetAnalogReg(0x071E ,0x00B4,0xffff); // RD_CLK off
            mAudioAnalogReg->SetAnalogReg(0x072c ,0x0080,0xffff); // NCP off
            if(!GetChipVersion() == CHIP_VERSION_E1)
            {
                mAudioAnalogReg->SetAnalogReg(0x071A ,0x0000,0x1000); // turn iogg LDO
            }
            mAudioAnalogReg->SetAnalogReg(0x072E ,0x0000,0x0003); // Power Off LSB
            if(GetDownLinkStatus ()== false){
                if(!GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0004,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0718,0x0006,0xffff);
                }
            }
            if(GetDownLinkStatus () == false)
            {
                if(GetChipVersion() == CHIP_VERSION_E1)
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0992,0xffff);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(0x0714,0x0192,0xffff);
                }
            }
            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(0x072C ,0x0080,0xffff);
            break;
        case AudioAnalogType::DEVICE_OUT_LINEOUTR:
        case AudioAnalogType::DEVICE_OUT_LINEOUTL:
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            break;
    }
    mLock.unlock ();
    return NO_ERROR;
}

AudioAnalogType::MUX_TYPE AudioMachineDevice::AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    uint32 Reg_Value = 0;
    AudioAnalogType::MUX_TYPE MuxType = AudioAnalogType::MUX_AUDIO;
    switch(DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
        {
            Reg_Value = mAudioAnalogReg->GetAnalogReg(AUDBUF_CFG0);
            Reg_Value = (Reg_Value & 0x000001e0) >> 5;
            if(Reg_Value == 0)
                MuxType = AudioAnalogType::MUX_OPEN;
            else if(Reg_Value == 1)
                MuxType = AudioAnalogType::MUX_LINEIN_L;
            else if(Reg_Value == 2)
                MuxType = AudioAnalogType::MUX_LINEIN_R;
            else if(Reg_Value == 3)
                MuxType = AudioAnalogType::MUX_LINEIN_STEREO;
            else if(Reg_Value == 4)
                MuxType = AudioAnalogType::MUX_AUDIO;
            else if(Reg_Value == 5)
                MuxType = AudioAnalogType::MUX_LINEIN_AUDIO_MONO;
            else if(Reg_Value == 8)
                MuxType = AudioAnalogType::MUX_IV_BUFFER;
            else{
                MuxType = AudioAnalogType::MUX_AUDIO;
                ALOGW("AnalogGetMux warning");
            }
            break;
        }
        default:
            break;
    }
    return MuxType;
}

/**
* a basic function fo select mux of device type, not all device may have mux
* if select a device with no mux support , report error.
* @param DeviceType analog part
* @param MuxType analog mux selection
* @return status_t
*/
status_t AudioMachineDevice::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AnalogSetMux DeviceType = %s MuxType = %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
    mBlockAttribute[DeviceType].mMuxSelect = MuxType ;
    uint32 Reg_Value =0;
    switch(DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        {
            if(MuxType == AudioAnalogType::MUX_OPEN){
                Reg_Value = 0;
            }
            else if(MuxType == AudioAnalogType::MUX_MUTE){
                Reg_Value = 1<<3;
            }
            else if(MuxType == AudioAnalogType::MUX_VOICE){
                Reg_Value = 2<<3;
            }
            else{
                Reg_Value = 2<<3;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG0,Reg_Value,0x000000018); // bits 3,4
            break;
        }
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
        {
            if(MuxType == AudioAnalogType::MUX_OPEN)
                Reg_Value = 0;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_L)
                Reg_Value = 1<<5;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_R)
                Reg_Value = 2<<5;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_STEREO)
                Reg_Value = 3<<5;
            else if(MuxType == AudioAnalogType::MUX_AUDIO)
                Reg_Value = 4<<5;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_AUDIO_MONO)
                Reg_Value = 5<<5;
            else if(MuxType == AudioAnalogType::MUX_IV_BUFFER)
                Reg_Value = 8<<5;
            else{
                Reg_Value = 4<<5;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG0,Reg_Value,0x000001e0);
            break;
        }
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        {
            if(MuxType == AudioAnalogType::MUX_OPEN)
                Reg_Value = 0;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_L)
                Reg_Value = 1<<9;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_R)
                Reg_Value = 2<<9;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_STEREO)
                Reg_Value = 3<<9;
            else if(MuxType == AudioAnalogType::MUX_AUDIO)
                Reg_Value = 4<<9;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_AUDIO_MONO)
                Reg_Value = 5<<9;
            else if(MuxType == AudioAnalogType::MUX_IV_BUFFER)
                Reg_Value = 8<<9;
            else{
                Reg_Value = 4<<9;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG0,Reg_Value,0x00001e00);
            break;
        }
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
        {
            #ifdef USING_EXTAMP_HP
            ALOGD("USING_EXTAMP_HP");
            #else
            if(MuxType == AudioAnalogType::MUX_OPEN)
                Reg_Value = 0;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_L || MuxType == AudioAnalogType::MUX_LINEIN_R)
                Reg_Value = 1<<2;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_STEREO)
                Reg_Value = 2<<2;
            else if(MuxType == AudioAnalogType::MUX_OPEN)
                Reg_Value = 3<<2;
            else if(MuxType == AudioAnalogType::MUX_AUDIO)
                Reg_Value = 4<<2;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_AUDIO_MONO)
                Reg_Value = 5<<2;
            else if(MuxType == AudioAnalogType::MUX_LINEIN_AUDIO_STEREO)
                Reg_Value = 6<<2;
            else{
                Reg_Value = 4<<2;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUD_IV_CFG0,Reg_Value,0x0000001c);
            #endif
            break;
        }
        case AudioAnalogType::DEVICE_IN_PREAMP_L:
        {
            if(MuxType == AudioAnalogType::MUX_IN_MIC1)
                Reg_Value = 1<<2;
            else if(MuxType == AudioAnalogType::MUX_IN_MIC2)
                Reg_Value = 2<<2;
            else if(MuxType == AudioAnalogType::MUX_IN_MIC3)
                Reg_Value = 3<<2;
            else{
                Reg_Value = 1<<2;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDPREAMP_CON0,Reg_Value,0x0000001c);
            break;
        }
        case AudioAnalogType::DEVICE_IN_PREAMP_R:
        {
            if(MuxType == AudioAnalogType::MUX_IN_MIC1)
                Reg_Value = 1<<5;
            else if(MuxType == AudioAnalogType::MUX_IN_MIC2)
                Reg_Value = 2<<5;
            else if(MuxType == AudioAnalogType::MUX_IN_MIC3)
                Reg_Value = 3<<5;
            else{
                Reg_Value = 1<<5;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDPREAMP_CON0,Reg_Value,0x000000e0);
            break;
        }
        case AudioAnalogType::DEVICE_IN_ADC1:
        {
            if(MuxType == AudioAnalogType::MUX_IN_MIC1)
                Reg_Value = 1<<2;
            else if(MuxType == AudioAnalogType::MUX_IN_PREAMP_L)
                Reg_Value = 4<<2;
            else if(MuxType == AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER)
                Reg_Value = 5<<2;
            else{
                Reg_Value = 1<<2;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDADC_CON0,Reg_Value,0x0000001c);
            break;
        }
        case AudioAnalogType::DEVICE_IN_ADC2:
        {
            if(MuxType == AudioAnalogType::MUX_IN_MIC1)
                Reg_Value = 1<<5;
            else if(MuxType == AudioAnalogType::MUX_IN_PREAMP_R)
                Reg_Value = 4 << 5;
            else if (MuxType == AudioAnalogType::MUX_IN_LEVEL_SHIFT_BUFFER)
                Reg_Value = 5 << 5;
            else {
                Reg_Value = 1<<5;
                ALOGW("AnalogSetMux warning");
            }
            mAudioAnalogReg->SetAnalogReg(AUDADC_CON0,Reg_Value,0x000000e0);
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}

/**
* a  function for setParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return status_t
*/
status_t AudioMachineDevice::setParameters(int command1 , int command2 , unsigned int data)
{
    return NO_ERROR;
}

/**
* a function for setParameters , provide wide usage of analog control
* @param command1
* @param data
* @return status_t
*/
status_t AudioMachineDevice::setParameters(int command1 , void *data)
{
    return NO_ERROR;
}

/**
* a function fo getParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return copy_size
*/
int AudioMachineDevice::getParameters(int command1 , int command2 , void *data)
{
    return 0;
}


}
