
#include "brom.h"
#include "status.h"
#include "stdio.h"
#include "preloadercmd.h"

void DumpBootArg(BOOT_ARG_S * pArg)
{
    printf("DumpBootArg(): <<<<<<<<<<<<<<<<Boot arguments>>>>>>>>>>>>>>>>\n");
    printf("DumpBootArg(): Preloader timeout(%u)\n", pArg->m_uTimeout);
    printf("DumpBootArg(): Preloader retry time(%u)\n", pArg->m_uRetryTime);
    printf("DumpBootArg(): Preloader retry interval(%u)\n", pArg->m_uInterval);
    printf("DumpBootArg(): UART port baudrate(%u)\n", pArg->m_uBaudrate);
    printf("DumpBootArg(): Stop flag addr(0x%p)\n", pArg->m_pStopFlag);
    
    printf("DumpBootArg(): USB enable(%u)\n", pArg->m_bIsUSBEnable);
    printf("DumpBootArg(): Symbolic name enable(%u)\n", pArg->m_bIsSymbolicEnable);
    printf("DumpBootArg(): Composite device enable(%u)\n", pArg->m_bIsCompositeDeviceEnable);
    printf("DumpBootArg(): Boot mode type(%u)\n", pArg->m_euBootMode);
    printf("DumpBootArg(): Port number(COM%u)\n", pArg->m_uPortNumber);
    printf("DumpBootArg(): Symbolic name(%s)\n", pArg->m_szPortSymbolic);  
 
}

int __stdcall Preloader_BootMode(BOOT_ARG_S * pArg)
{
    //printf("\n================== DLL Calling ====================\n");
    
    int iRet = S_DONE;    
    
    //Check parameters
    if(NULL == pArg)
    {
			printf( "Preloader_BootMode(): invalid arguments!\n");
			return S_INVALID_ARGUMENTS;
    }
    //printf("Preloader_BootMode(): Boot mode entry procedure start...\n");
   // DumpBootArg(pArg);


    PreloaderCmd cCmd;
    return cCmd.SelectBootMode(pArg);   
}