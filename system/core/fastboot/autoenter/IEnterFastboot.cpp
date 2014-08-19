// auto.cpp : Defines the entry point for the console application.
//

#include "windows.h"

#include "brom.h"
#include "status.h"
#include "comscan.h"
#include "stdio.h"

/* Notify phone enter FASTBOOT mode. 
*  This function will scan and communicate with preloader COM port that parameters indicated.
*  Any COM port in parameters was detected, this port will be the communication agent.
*  return: 0 is OK, -1 is error.
*  params: 
*    preloaderSinglePortFilter: preloader single COM port's PID&VID, string like "VID_0E8D&PID_2000"
*    preloaderCompositePortFilter: preloader composite COM port's PID&VID, string like "VID_1004&PID_6000"
*    if these two were NULL, then default PID&VID will be used. "VID_0E8D&PID_2000" and "VID_1004&PID_6000"
*/
extern "C" int NotifyEnterFastbootMode(char* preloaderSinglePortFilter, char* preloaderCompositePortFilter)
{
	char* asBROMPortFilter = "VID_0E8D&PID_0003";
	char* asPreloaderSinglePortFilter = "VID_0E8D&PID_2000";
	char* asPreloaderCompositePortFilter = "VID_1004&PID_6000";

	if(preloaderSinglePortFilter != NULL)
	{
		asPreloaderSinglePortFilter = preloaderSinglePortFilter;
	}

	if(preloaderCompositePortFilter != NULL)
	{
		asPreloaderCompositePortFilter = preloaderCompositePortFilter;
	}

	BOOT_ARG_S stArg;
	int iRet;
	int BootStop = 0;  
	BOOTTYPE eType;    

	stArg.m_pStopFlag = &BootStop;    
	stArg.m_uBaudrate = 115200;
	stArg.m_uInterval = 10;  //2000 will failed.
	stArg.m_uRetryTime = 3;
	stArg.m_uTimeout = 3500;
	stArg.m_bIsUSBEnable = true;
	stArg.m_bIsSymbolicEnable = false;
	stArg.m_bIsCompositeDeviceEnable = false;   
	stArg.m_euBootMode = FASTBOOT;


	//Use USB port, necessary to scan
	COM_FILTER_LIST_S sCOMFilter;
	COM_PROPERTY_S sCOMProperty;

	int asDeviceType = 1;//Composite
	unsigned int uTimout = 35000; // ms
	char * ppFilter[3];

	ppFilter[0] = asPreloaderSinglePortFilter;
	ppFilter[1] = asPreloaderCompositePortFilter;
	ppFilter[2] = asBROMPortFilter;
	sCOMFilter.m_uCount = 2;      //do no use BRom.
	sCOMFilter.m_eType = WHITE_LIST;
	sCOMFilter.m_ppFilterID = ppFilter;

  printf("Wait %d seconds for connection..\n", uTimout/1000);
	if( asDeviceType == 1 )  //Composite
	{
		iRet = GetIncrementCOMPortWithFilter(&sCOMFilter, &sCOMProperty, NULL, false, &BootStop, uTimout);
	}
	else                       //Single
	{
		iRet = GetIncrementCOMPortWithFilter(&sCOMFilter, &sCOMProperty, NULL, true, &BootStop, uTimout);
	}
	if(0 == iRet)
	{
		switch(sCOMProperty.m_iFilterIndex)
		{
		case 0:
			eType = PreloaderUSB;
			break;
		case 1:
			eType = PreloaderUSB;
			break;
		case 2:
			eType = BootROMUSB;
			break;
		default:
			printf("Search filter index error!\n");
			return -1;
		}

		stArg.m_uPortNumber = sCOMProperty.m_uNumber;
		strncpy(stArg.m_szPortSymbolic, sCOMProperty.m_rSymbolic, 256);
	}
	else
	{
		switch(iRet)
		{		
		case S_TIMEOUT:
			printf("Search Preloader/BootROM USB COM port timout!\n");
			break;
		case S_INVALID_ARGUMENTS:
			printf("Invalid arguments for searching COM port!\n");
			break;
		default:
			printf("Search Preloader/BootROM USB COM port fail!\n");
		}		
		return -1;
	}

	if( asDeviceType == 1 )   //Composite
	{
		Sleep(500);
	}


	if(BootROMUSB == eType)
	{
		printf("Assert, BRom mode not open in code.\n");
		return -1;
	}
	else if(PreloaderUSB == eType)
	{
		iRet = Preloader_BootMode(&stArg);
		if(0 != iRet)
		{ 
			printf("Enter Fastboot Mode Fail. Err(%u).\n", iRet);
			return -1;
		}
	}
	else
	{
		printf("Error: PreloaderUSB != eType\n");
		return -1;
	}

	printf("Enter Fastboot Mode Success.\n");
	return 0;
}

