#ifndef _BROM_H_
#define _BROM_H_

typedef enum 
{
    NORMAL_BOOT         = 0,    
    FASTBOOT       = 99,
    UNKNOWN_BOOT
    
} BOOT_MODE;

typedef struct 
{    
    //New parameters
    unsigned int m_uTimeout;
    unsigned int m_uRetryTime;
    unsigned int m_uInterval;
    unsigned int m_uBaudrate;
    int * m_pStopFlag;
    
    bool m_bIsUSBEnable;
    bool m_bIsSymbolicEnable;
    bool m_bIsCompositeDeviceEnable;
    BOOT_MODE m_euBootMode;
    unsigned int m_uPortNumber;
    char m_szPortSymbolic[256];
    
} BOOT_ARG_S;

typedef enum {
    WHITE_LIST = 0,
    BLACK_LIST,
} FILTER_TYPE_E;

typedef struct 
{
    unsigned int m_uCount;
    FILTER_TYPE_E m_eType;
    char ** m_ppFilterID;
} COM_FILTER_LIST_S;

typedef struct
{
    int m_iFilterIndex;
    unsigned int m_uNumber;
    char m_rFriendly[512];
    char m_rInstanceID[512];
    char m_rSymbolic[512];
    
} COM_PROPERTY_S;

typedef enum
{
	PreloaderUSB = 0,
	BootROMUSB
}BOOTTYPE;

int __stdcall Preloader_BootMode(BOOT_ARG_S * pArg);
#endif