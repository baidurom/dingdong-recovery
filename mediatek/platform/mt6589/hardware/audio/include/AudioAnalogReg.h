#ifndef ANDROID_AUDIO_ANALOGREG_H
#define ANDROID_AUDIO_ANALOGREG_H

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

// AudioAnalogReg only provide basic funciton to set register ,
// other function should be move to other module to control this class.
#include "AudioType.h"
#include "AudioDef.h"
#include "AudioUtility.h"
#include "AudioIoctl.h"


//---------------digital pmic  register define -------------------------------------------
#define AFE_PMICDIG_AUDIO_BASE        (0x4000)
#define AFE_UL_DL_CON0               (AFE_PMICDIG_AUDIO_BASE+0x0000)
#define AFE_DL_SRC2_CON0_H     (AFE_PMICDIG_AUDIO_BASE+0x0002)
#define AFE_DL_SRC2_CON0_L     (AFE_PMICDIG_AUDIO_BASE+0x0004)
#define AFE_DL_SRC2_CON1_H     (AFE_PMICDIG_AUDIO_BASE+0x0006)
#define AFE_DL_SRC2_CON1_L     (AFE_PMICDIG_AUDIO_BASE+0x0008)
#define AFE_DL_SDM_CON0           (AFE_PMICDIG_AUDIO_BASE+0x000A)
#define AFE_DL_SDM_CON1           (AFE_PMICDIG_AUDIO_BASE+0x000C)
#define AFE_UL_SRC_CON0_H       (AFE_PMICDIG_AUDIO_BASE+0x000E)
#define AFE_UL_SRC_CON0_L       (AFE_PMICDIG_AUDIO_BASE+0x0010)
#define AFE_UL_SRC_CON1_H      (AFE_PMICDIG_AUDIO_BASE+0x0012)
#define AFE_UL_SRC_CON1_L       (AFE_PMICDIG_AUDIO_BASE+0x0014)
#define AFE_PREDIS_CON0_H       (AFE_PMICDIG_AUDIO_BASE+0x0016)
#define AFE_PREDIS_CON0_L       (AFE_PMICDIG_AUDIO_BASE+0x0018)
#define AFE_PREDIS_CON1_H       (AFE_PMICDIG_AUDIO_BASE+0x001a)
#define AFE_PREDIS_CON1_L       (AFE_PMICDIG_AUDIO_BASE+0x001c)
#define ANA_AFE_I2S_CON1                  (AFE_PMICDIG_AUDIO_BASE+0x001e)
#define AFE_I2S_FIFO_UL_CFG0  (AFE_PMICDIG_AUDIO_BASE+0x0020)
#define AFE_I2S_FIFO_DL_CFG0  (AFE_PMICDIG_AUDIO_BASE+0x0022)
#define ANA_AFE_TOP_CON0                  (AFE_PMICDIG_AUDIO_BASE+0x0024)
#define ANA_AUDIO_TOP_CON0             (AFE_PMICDIG_AUDIO_BASE+0x0026)
#define AFE_UL_SRC_DEBUG       (AFE_PMICDIG_AUDIO_BASE+0x0028)
#define AFE_DL_SRC_DEBUG       (AFE_PMICDIG_AUDIO_BASE+0x002a)
#define AFE_UL_SRC_MON0        (AFE_PMICDIG_AUDIO_BASE+0x002c)
#define AFE_DL_SRC_MON0        (AFE_PMICDIG_AUDIO_BASE+0x002e)
#define AFE_DL_SDM_TEST0       (AFE_PMICDIG_AUDIO_BASE+0x0030)
#define AFE_MON_DEBUG0         (AFE_PMICDIG_AUDIO_BASE+0x0032)
#define AFUNC_AUD_CON0         (AFE_PMICDIG_AUDIO_BASE+0x0034)
#define AFUNC_AUD_CON1         (AFE_PMICDIG_AUDIO_BASE+0x0036)
#define AFUNC_AUD_CON2         (AFE_PMICDIG_AUDIO_BASE+0x0038)
#define AFUNC_AUD_CON3         (AFE_PMICDIG_AUDIO_BASE+0x003A)
#define AFUNC_AUD_CON4         (AFE_PMICDIG_AUDIO_BASE+0x003C)
#define AFUNC_AUD_MON0         (AFE_PMICDIG_AUDIO_BASE+0x003E)
#define AFUNC_AUD_MON1         (AFE_PMICDIG_AUDIO_BASE+0x0040)
#define AUDRC_TUNE_MON0        (AFE_PMICDIG_AUDIO_BASE+0x0042)
#define AFE_I2S_FIFO_MON0      (AFE_PMICDIG_AUDIO_BASE+0x0044)
#define AFE_DL_DC_COMP_CFG0    (AFE_PMICDIG_AUDIO_BASE+0x0046)
#define AFE_DL_DC_COMP_CFG1    (AFE_PMICDIG_AUDIO_BASE+0x0048)
#define AFE_DL_DC_COMP_CFG2    (AFE_PMICDIG_AUDIO_BASE+0x004a)
#define AFE_MBIST_CFG0                 (AFE_PMICDIG_AUDIO_BASE+0x004c)
#define AFE_MBIST_CFG1                 (AFE_PMICDIG_AUDIO_BASE+0x004e)
#define AFE_MBIST_CFG2                 (AFE_PMICDIG_AUDIO_BASE+0x0050)
#define AFE_I2S_FIFO_CFG0           (AFE_PMICDIG_AUDIO_BASE+0x0052)
//---------------digital pmic  register define end ---------------------------------------

//---------------analog pmic  register define start --------------------------------------
//---------------digital pmic  register define -------------------------------------------
#define AFE_PMICANA_AUDIO_BASE        (0x0)

#define TOP_CKPDN                 (AFE_PMICANA_AUDIO_BASE + 0x102)
#define TOP_CKPDN_SET        (AFE_PMICANA_AUDIO_BASE + 0x104)
#define TOP_CKPDN_CLR        (AFE_PMICANA_AUDIO_BASE + 0x106)
#define TOP_CKPDN2               (AFE_PMICANA_AUDIO_BASE + 0x108)
#define TOP_CKPDN2_SET      (AFE_PMICANA_AUDIO_BASE + 0x10a)
#define TOP_CKPDN2_CLR      (AFE_PMICANA_AUDIO_BASE + 0x10c)
#define TOP_CKPDN2_CLR      (AFE_PMICANA_AUDIO_BASE + 0x10c)
#define TOP_CKCON1              (AFE_PMICANA_AUDIO_BASE + 0x128)

#define SPK_CON0                    (AFE_PMICANA_AUDIO_BASE+0x0600)
#define SPK_CON1                    (AFE_PMICANA_AUDIO_BASE+0x0602)
#define SPK_CON2                    (AFE_PMICANA_AUDIO_BASE+0x0604)
#define SPK_CON3                    (AFE_PMICANA_AUDIO_BASE+0x0606)
#define SPK_CON4                    (AFE_PMICANA_AUDIO_BASE+0x0608)
#define SPK_CON5                    (AFE_PMICANA_AUDIO_BASE+0x060A)
#define SPK_CON6                    (AFE_PMICANA_AUDIO_BASE+0x060C)
#define SPK_CON7                    (AFE_PMICANA_AUDIO_BASE+0x060E)
#define SPK_CON8                    (AFE_PMICANA_AUDIO_BASE+0x0610)
#define SPK_CON9                    (AFE_PMICANA_AUDIO_BASE+0x0612)
#define SPK_CON10                  (AFE_PMICANA_AUDIO_BASE+0x0614)
#define SPK_CON11                  (AFE_PMICANA_AUDIO_BASE+0x0616)

#define AUDDAC_CON0           (AFE_PMICANA_AUDIO_BASE + 0x700)
#define AUDBUF_CFG0            (AFE_PMICANA_AUDIO_BASE + 0x702)
#define AUDBUF_CFG1            (AFE_PMICANA_AUDIO_BASE + 0x704)
#define AUDBUF_CFG2            (AFE_PMICANA_AUDIO_BASE + 0x706)
#define AUDBUF_CFG3            (AFE_PMICANA_AUDIO_BASE + 0x708)
#define AUDBUF_CFG4            (AFE_PMICANA_AUDIO_BASE + 0x70a)
#define IBIASDIST_CFG0        (AFE_PMICANA_AUDIO_BASE + 0x70c)
#define AUDACCDEPOP_CFG0        (AFE_PMICANA_AUDIO_BASE + 0x70e)
#define AUD_IV_CFG0             (AFE_PMICANA_AUDIO_BASE + 0x710)
#define AUDCLKGEN_CFG0        (AFE_PMICANA_AUDIO_BASE + 0x712)
#define AUDLDO_CFG0            (AFE_PMICANA_AUDIO_BASE + 0x714)
#define AUDLDO_CFG1            (AFE_PMICANA_AUDIO_BASE + 0x716)
#define AUDNVREGGLB_CFG0        (AFE_PMICANA_AUDIO_BASE + 0x718)
#define AUD_NCP0                        (AFE_PMICANA_AUDIO_BASE + 0x71a)
#define AUDPREAMP_CON0        (AFE_PMICANA_AUDIO_BASE + 0x71c)
#define AUDADC_CON0           (AFE_PMICANA_AUDIO_BASE + 0x71e)
#define AUDADC_CON1           (AFE_PMICANA_AUDIO_BASE + 0x720)
#define AUDADC_CON2           (AFE_PMICANA_AUDIO_BASE + 0x722)
#define AUDADC_CON3           (AFE_PMICANA_AUDIO_BASE + 0x724)
#define AUDADC_CON4           (AFE_PMICANA_AUDIO_BASE + 0x726)
#define AUDADC_CON5           (AFE_PMICANA_AUDIO_BASE + 0x728)
#define AUDADC_CON6           (AFE_PMICANA_AUDIO_BASE + 0x72a)
#define AUDDIGMI_CON0        (AFE_PMICANA_AUDIO_BASE + 0x72c)
#define AUDLSBUF_CON0        (AFE_PMICANA_AUDIO_BASE + 0x72e)
#define AUDLSBUF_CON1        (AFE_PMICANA_AUDIO_BASE + 0x730)
#define AUDENCSPARE_CON0        (AFE_PMICANA_AUDIO_BASE + 0x732)
#define AUDENCCLKSQ_CON0        (AFE_PMICANA_AUDIO_BASE + 0x734)
#define AUDPREAMPGAIN_CON0        (AFE_PMICANA_AUDIO_BASE + 0x736)
#define ZCD_CON0        (AFE_PMICANA_AUDIO_BASE + 0x738)
#define ZCD_CON1        (AFE_PMICANA_AUDIO_BASE + 0x73a)
#define ZCD_CON2        (AFE_PMICANA_AUDIO_BASE + 0x73c)
#define ZCD_CON3        (AFE_PMICANA_AUDIO_BASE + 0x73e)
#define ZCD_CON4        (AFE_PMICANA_AUDIO_BASE + 0x740)
#define ZCD_CON5        (AFE_PMICANA_AUDIO_BASE + 0x742)
#define NCP_CLKDIV_CON0        (AFE_PMICANA_AUDIO_BASE + 0x744)
#define NCP_CLKDIV_CON1        (AFE_PMICANA_AUDIO_BASE + 0x746)

//---------------analog pmic  register define end ---------------------------------------

namespace android
{

class AudioAnalogReg
{
    public:
        static AudioAnalogReg *getInstance();
        status_t SetAnalogReg(uint32 offset, uint32 value, uint32 mask);
        uint32 GetAnalogReg(uint32 offset);

        /**
        * a basic function to check regiseter range
        * @param offset
        * @return bool
        */
        bool CheckAnaRegRange(uint32 offset);

    private:
        /**
        * AudioAnalogReg contructor .
        * use private constructor to achieve single instance
        */
        AudioAnalogReg();
        ~AudioAnalogReg();

        /**
        * a private variable.
        * single instance to thois class
        */
        static AudioAnalogReg *UniqueAnalogRegInstance;

        AudioAnalogReg(const AudioAnalogReg &);             // intentionally undefined
        AudioAnalogReg &operator=(const AudioAnalogReg &);  // intentionally undefined

        /**
        * a loca varible for operation regiseter settingsAllowed
        */
        Register_Control mReg_Control;

        /**
        * a private variable.
        * file descriptor to open audio driver
        */
        int mFd;
};

}

#endif
