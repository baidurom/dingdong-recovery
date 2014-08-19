#ifndef _PRELOADER_CMD_H_
#define _PRELOADER_CMD_H_

#include "NewCOMUtil.h"
#include "status.h"
#include "brom.h"


#define MAX_MSG_SIZE  256
#define MAX_REVBUF_SIZE  256
#define MAX_PATTERN_LEN 16


/* 
  * The structure definition of mode parameters
  */
//*********************************************************
typedef struct
{
    unsigned char m_cKernelUSBType;    //0(Single interface device), 1(Composite device)
    unsigned char m_cKernelUSBNum;    //The COM port number of Kernel USB
    unsigned char dummy1;
    unsigned char dummy2;
    
} PARA_VALUE_S_V1;

typedef union
{
    PARA_VALUE_S_V1 m_tParaV1;
    
} PARA_VALUE_U;

typedef struct
{
    unsigned char m_aMode[8];    //Mode magic, such as "METAMETA"
    unsigned int m_uParaLen;    //The length of mode parameters
    unsigned int m_uParaVer;    //The version of mode parameters
    PARA_VALUE_U m_nParaVal;
    
} PL_MODE_PARA_S;

//*********************************************************


typedef struct
{
    unsigned int m_uStatus;
    unsigned int m_uLen;
    char m_szPattern[MAX_PATTERN_LEN];
    
} PATTERN_INFO_S;

class PreloaderCmd: public NewCOMUtil
{
public:
    PreloaderCmd();
    ~PreloaderCmd();

    int SelectBootMode(BOOT_ARG_S * pArg);
    void SetParameters(BOOT_ARG_S * pArg);
    int SetPort(BOOT_ARG_S * pArg);

    unsigned int ReadPattern(PATTERN_INFO_S * pPattern, const unsigned int kPatternNum);
    unsigned int WritePattern(const char * pPattern, const int kPatternLen);
    unsigned int CMD_BootAsFastboot(BOOT_ARG_S * pArg);

    //inline functions
    inline void SetTimeout(unsigned int uTimeout) { m_uTimeout = uTimeout; }
    inline void SetRetryCount(unsigned int uRetryTime) { m_uRetryTime = uRetryTime; }
    inline void SetInterval(unsigned int uInterval) { m_uInterval = uInterval; }
    inline void SetStopFlagAddr(int * pStopFlag) { m_pStopFlag= pStopFlag; }


private:
    unsigned int m_uTimeout;
    unsigned int m_uRetryTime;
    unsigned int m_uInterval;
    int * m_pStopFlag;

    NewCOMUtil m_cPort;
};



#endif //_PRELOADER_CMD_H_
