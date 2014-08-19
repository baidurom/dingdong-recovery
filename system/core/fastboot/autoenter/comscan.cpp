#include "windows.h"
#include <initguid.h>
#include <setupapi.h>
#include <objbase.h>
#include <vector>

#include "brom.h"
#include "status.h"
using namespace std;

//System lib
#pragma   comment(lib,   "Setupapi.lib")  //for   SetupDiGetClassDevs

//DEFINE_GUID is recognized by compiler, objbase.h/initguid.h must be included!!!

/*  
*  GUID of device interface class
*  Copy from usbiodef.h in WinDDK
*/
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

/*  
*  GUID of device setup class
*  Copy from devguid.h in WinDDK
*/
DEFINE_GUID( GUID_DEVCLASS_PORTS,  0x4d36e978L, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 );
//DEFINE_GUID( GUID_DEVCLASS_USB,  0x36fc9e60L, 0xc465, 0x11cf, 0x80, 0x56, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 );




static HANDLE BeginEnumerateDevice(const GUID * pGuid, bool bDeviceInterface, bool bPresentOnly)
{
	HDEVINFO DeviceInfoSet;
	DWORD dwFlags = 0;


	dwFlags = (bDeviceInterface) ? (dwFlags | DIGCF_DEVICEINTERFACE) : (dwFlags);
	dwFlags = (bPresentOnly) ? (dwFlags | DIGCF_PRESENT) : (dwFlags);

	DeviceInfoSet = SetupDiGetClassDevs(
		pGuid, 
		0, 0,  
		dwFlags);
	return DeviceInfoSet;
}


static void EndEnumerateDevice(HANDLE DeviceInfoSet)
{
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}

static void GetNumFromFriendly(char * pFriendly, unsigned int & iNum)
{
	char * pIndex = NULL;

	pIndex = strstr(pFriendly, "(COM");
	if(NULL != pIndex)
	{
		sscanf(pIndex, "(COM%u)", &iNum);
	}
	else
	{
		iNum = -1;
	}
}

static int EnumerateDevice(HANDLE hDevInfoSet, const GUID * pGuid, bool bDeviceInterface, int iIndex, COM_PROPERTY_S & stProp)
{
	SP_DEVINFO_DATA devInfoData = {0};

	//Enum device interfaces
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	if(!SetupDiEnumDeviceInfo(hDevInfoSet, iIndex, &devInfoData))
	{
		if (ERROR_NO_MORE_ITEMS != GetLastError())
		{
			printf("ERROR_NO_MORE_ITEMS != GetLastError()");
		}
		return -1;
	}

	//Get instance ID of device
	if(!SetupDiGetDeviceInstanceId(
		hDevInfoSet,
		&devInfoData,
		(PTSTR)stProp.m_rInstanceID,
		sizeof(stProp.m_rInstanceID) - 1,
		NULL))
	{

		sprintf(stProp.m_rInstanceID, "Unknown");
	}

	// Get friendly name of device
	if(!SetupDiGetDeviceRegistryProperty(
		hDevInfoSet,
		&devInfoData,
		SPDRP_FRIENDLYNAME,
		NULL,
		(PBYTE)stProp.m_rFriendly,
		sizeof(stProp.m_rFriendly) - 1,
		NULL))
	{       
		sprintf(stProp.m_rFriendly, "Unknown");
		stProp.m_uNumber = 0;
	}
	else
	{
		GetNumFromFriendly(stProp.m_rFriendly, stProp.m_uNumber);
	}



	//Get device interface information
	if(bDeviceInterface)
	{
		DWORD i = 0;
		SP_DEVICE_INTERFACE_DATA devInterfaceData;
		devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		SP_DEVICE_INTERFACE_DETAIL_DATA * pDetData = NULL;
		DWORD ReqLen = 0;

		while(1)
		{
			if(SetupDiEnumDeviceInterfaces(hDevInfoSet,  &devInfoData, pGuid, i++, &devInterfaceData))
			{
				// Get size of symbolic link name 		
				SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &devInterfaceData, NULL, 0, &ReqLen, NULL);
				char* pData = new char[ReqLen];
				pDetData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(pData);
				pDetData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

				// Get symbolic link name
				if(!SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &devInterfaceData, pDetData, ReqLen, NULL, &devInfoData)) 
				{                    
					sprintf(stProp.m_rSymbolic,"Unknown");
					delete [] pData;
				}
				else
				{
					strncpy(stProp.m_rSymbolic, pDetData->DevicePath, (strlen(pDetData->DevicePath)+1));

					delete [] pData;
					break;
				}
			}
			else
			{
				sprintf(stProp.m_rSymbolic,"Unknown");            

				break;
			}
		}
		pDetData = NULL;
	}
	else
	{
		sprintf(stProp.m_rSymbolic,"Unsupported");
	}

	return 0;
}

static void GetPresentDeviceInfo(const GUID * pGuid, bool bDeviceInterface, vector<COM_PROPERTY_S> & vtCOM)
{
	HANDLE hDevInfoSet;
	COM_PROPERTY_S stProp;
	int iIndex = 0;


	hDevInfoSet = BeginEnumerateDevice(pGuid, bDeviceInterface, true);

	while(1)
	{
		memset(stProp.m_rFriendly, 0, 512);
		memset(stProp.m_rInstanceID, 0, 512);
		memset(stProp.m_rSymbolic, 0, 512);

		int iRet = EnumerateDevice(hDevInfoSet, pGuid, bDeviceInterface, iIndex++, stProp);
		if(0 != iRet)
		{
			break;
		}
		else
		{
			vtCOM.push_back(stProp);
		}
	}

	EndEnumerateDevice(hDevInfoSet);
}
static int DumpParameters(COM_FILTER_LIST_S * pCOMFilter, const GUID * pGuid, bool bDeviceInterface, int * pStopFlag, double dTimeout)
{
	//printf( "DumpParameters(): ===============\n");
	//printf( "DumpParameters(): COM filter number(%u)\n", pCOMFilter->m_uCount);
	//printf( "DumpParameters(): COM filter type(%s)\n", (pCOMFilter->m_eType) ? ("BLACK") : ("WHITE"));
	if((0 != pCOMFilter->m_uCount) && (NULL == pCOMFilter->m_ppFilterID))
	{
		printf(  "DumpParameters(): Filter list is null!\n");
		return -1;
	}

	for(unsigned int i = 0; i < pCOMFilter->m_uCount; i++)
	{
		if(NULL == pCOMFilter->m_ppFilterID[i])
		{
			printf( "DumpParameters(): Filter %u() is null!\n", i+1);
			return -1;
		}
		else
		{
			//printf( "DumpParameters(): Filter %u(%s)\n", i+1, pCOMFilter->m_ppFilterID[i]);
		}
	}


	char buf[512];
	char tmp[64];
	memset(buf, 0, 256);

	//GUID structure defined in Guiddef.h
	sprintf(buf, "0x%08lX-0x%04X-0x%04X", pGuid->Data1, pGuid->Data2, pGuid->Data3);
	for(int i = 0; i < 8; i++)
	{
		sprintf(tmp, "-0x%02X", pGuid->Data4[i]);
		strcat(buf, tmp);
	}

	//printf( "DumpParameters(): GUID(%s)\n", buf);
	//printf( "DumpParameters(): Interface(%d)\n", bDeviceInterface);
	//printf( "DumpParameters(): StopFlag(0x%p)\n", pStopFlag);
//	printf( "DumpParameters(): Timeout(%lf)\n", dTimeout);
	//printf( "DumpParameters(): ===============\n");
	return 0;
}

static void FindVectorDiff(vector<COM_PROPERTY_S> & vtFirst, vector<COM_PROPERTY_S> & vtSecond, vector<COM_PROPERTY_S> & vtDiff)
{
	vector<COM_PROPERTY_S>::iterator iterFirst;
	vector<COM_PROPERTY_S>::iterator iterSecond;

	for(iterSecond = vtSecond.begin(); iterSecond != vtSecond.end(); iterSecond++)
	{
		for(iterFirst = vtFirst.begin(); iterFirst != vtFirst.end(); iterFirst++)
		{
			if(0 == strcmp((*iterFirst).m_rInstanceID, (*iterSecond).m_rInstanceID))
			{
				break;
			}
		}

		if(iterFirst == vtFirst.end())
		{
			vtDiff.push_back(*iterSecond);
		}
	}

	//Update second to first
	vtFirst.clear();
	for(iterSecond = vtSecond.begin(); iterSecond != vtSecond.end(); iterSecond++)
	{
		vtFirst.push_back(*iterSecond);
	}
}
static bool IsValidCOMPort(COM_FILTER_LIST_S * pCOMFilter, char * rInstanceID, int & iIndex)
{
	iIndex = -1;
	//printf("IsValidCOMPort(): Instance ID(%s)\n", rInstanceID);

	if(0 == pCOMFilter->m_uCount)
	{
		printf( "IsValidCOMPort(): COM port filter is empty, index(-1)\n");
		return true;
	}
	else
	{
		//printf( "IsValidCOMPort(): COM port filter is not empty.\n");
		strupr(rInstanceID);

		for(unsigned int i = 0; i < pCOMFilter->m_uCount; i++)
		{
			strupr(pCOMFilter->m_ppFilterID[i]);
			if(NULL != strstr(rInstanceID, pCOMFilter->m_ppFilterID[i]))
			{
				//printf("IsValidCOMPort(): Match filter(%s), index(%d)\n", pCOMFilter->m_ppFilterID[i], i);
				iIndex = i;
				return ((pCOMFilter->m_eType) ? (false) : (true));
			}
		}

		return ((pCOMFilter->m_eType) ? (true) : (false));    //IF all filters dismatch, return opposite result
	}
}

int __stdcall GetIncrementCOMPortWithFilter(COM_FILTER_LIST_S * pCOMFilter, COM_PROPERTY_S * pCOMPorperty, const void * pGuid, bool bDeviceInterface, int * pStopFlag, unsigned int dTimeout)
{
	vector<COM_PROPERTY_S> vtFirst;
	vector<COM_PROPERTY_S> vtSecond;
	vector<COM_PROPERTY_S> vtDiff;
	const GUID * pGuidTmp = NULL;

	if(NULL != pGuid)
	{
		pGuidTmp = (const GUID *)pGuid;
	}
	else
	{
		pGuidTmp = (bDeviceInterface) ? (&GUID_DEVINTERFACE_USB_DEVICE) : (&GUID_DEVCLASS_PORTS);
	}

	if((NULL == pCOMFilter) ||(NULL == pCOMPorperty) ||DumpParameters(pCOMFilter, pGuidTmp, bDeviceInterface, pStopFlag, dTimeout))
	{        
		return S_INVALID_ARGUMENTS;
	}


	vtDiff.clear();
	vtFirst.clear();
	GetPresentDeviceInfo(pGuidTmp, bDeviceInterface, vtFirst);

	//timer cTotal;
	unsigned int clock = GetTickCount();
	unsigned int dSpan = 0;
	while(true)
	{            
		vtDiff.clear();
		vtSecond.clear();
		//timer cOnce;
		GetPresentDeviceInfo(pGuidTmp, bDeviceInterface, vtSecond);            

		FindVectorDiff(vtFirst, vtSecond, vtDiff);
		if(0 != vtDiff.size())
		{
			for(unsigned int i = 0; i < vtDiff.size(); i++)
			{
				if(IsValidCOMPort(pCOMFilter, vtDiff[i].m_rInstanceID, vtDiff[i].m_iFilterIndex))
				{


					pCOMPorperty->m_iFilterIndex = vtDiff[i].m_iFilterIndex;
					pCOMPorperty->m_uNumber = vtDiff[i].m_uNumber;
					memcpy(pCOMPorperty->m_rInstanceID, vtDiff[i].m_rInstanceID, 512);
					memcpy(pCOMPorperty->m_rFriendly, vtDiff[i].m_rFriendly, 512);
					memcpy(pCOMPorperty->m_rSymbolic, vtDiff[i].m_rSymbolic, 512);


					return S_DONE;
				}
			}
		}

		Sleep(50);
		dSpan = GetTickCount() - clock;
		if((0 != dTimeout) && (dSpan > dTimeout))
		{

			return S_TIMEOUT;
		}

	}


	return S_UNDEFINED_ERROR;
}

