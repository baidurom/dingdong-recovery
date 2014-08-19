
#include "preloadercmd.h"
#include "brom.h"
#include "status.h"
#include "stdio.h"

//Flow config
const unsigned int kDefTimeout = 20000;
const unsigned int kDefRetryCount = 1000;
const unsigned int kDefInterval = 10;

//COM port config
const unsigned int kInputBufferSize = 0x10000;
const unsigned int kOutputBufferSize = 0x2000;
const unsigned int kReadTimeout = 0;
const unsigned int kWriteTimeout = 1000;

//FBT mode
const char kStrReady[] = "READY";
const char kStrFBTReq[] = "FASTBOOT";
const char kStrFBTSuccess[] = "TOOBTSAF";



PreloaderCmd::PreloaderCmd()
{
	m_uTimeout = kDefTimeout;
	m_uRetryTime = kDefRetryCount;
	m_uInterval = kDefInterval;
	m_pStopFlag = NULL;
}


PreloaderCmd::~PreloaderCmd()
{
}


int PreloaderCmd::SelectBootMode(BOOT_ARG_S * pArg)
{
	switch(pArg->m_euBootMode)
	{
	case FASTBOOT:
		return CMD_BootAsFastboot(pArg);        

	default:
		return S_PL_MODE_UNSUPPORTED;
	}
}


void PreloaderCmd::SetParameters(BOOT_ARG_S * pArg)
{
	m_uTimeout = pArg->m_uTimeout;
	m_uRetryTime = pArg->m_uRetryTime;
	m_uInterval = pArg->m_uInterval;
	m_pStopFlag = pArg->m_pStopFlag;
	//printf( "Set parameters completely.\n");
}


int PreloaderCmd::SetPort(BOOT_ARG_S * pArg)
{
	char szCOM[32] = {0};

	//Judge if Symbollic name is enabled
	//printf( "Open COM port...\n");
	if(pArg->m_bIsSymbolicEnable)
	{
		//Open COM port by symbolic name
		if( m_cPort.Open(pArg->m_szPortSymbolic) ) 
		{

			printf( "Open COM port fail!, Str(%s), Err(%u).\n", pArg->m_szPortSymbolic, GetLastError());

			return S_COM_PORT_OPEN_FAIL;
		}
	}
	else
	{            
		//Open COM port by port number
		sprintf(szCOM,"\\\\.\\COM%d", pArg->m_uPortNumber);
		if( m_cPort.Open(szCOM) ) 
		{

			printf("Open COM port fail!, Str(%s), Err(%u).\n", szCOM, GetLastError());

			return S_COM_PORT_OPEN_FAIL;
		}
	}

	//Configure port
	int iRet = 0;
	//printf("Configure COM port...\n");
	iRet = m_cPort.SetupCommBuffer(kInputBufferSize, kOutputBufferSize);
	iRet = m_cPort.ChangeCommState(pArg->m_uBaudrate);
	iRet = m_cPort.SetCommTimeout(kReadTimeout, kWriteTimeout);

	return iRet;
}


unsigned int PreloaderCmd::ReadPattern(PATTERN_INFO_S * pPattern, const unsigned int kPatternNum)
{
	DWORD  dwReadLen;
	char szRevBuf[MAX_REVBUF_SIZE];
	char szFlowPool[MAX_PATTERN_LEN];
	unsigned int uPoolSize;

	//Check parameters
	if( (NULL == pPattern) ||(1 > kPatternNum))
	{
		printf("Incorrect parameters!(NULL pointer or wrong pattern number)\n");
		return S_INVALID_ARGUMENTS;
	}

	uPoolSize = MAX_PATTERN_LEN;

	memset(szRevBuf, 0, MAX_REVBUF_SIZE);
	memset(szFlowPool, 0, MAX_PATTERN_LEN);


	try
	{    
		for(unsigned int i = 0; i < m_uRetryTime; i++)
		{
			//if(BOOT_STOP == *m_pStopFlag)
			//{
			//    printf("PreloaderCmd::ReadPattern(): STOP!");
			//    return S_STOP;
			//}


			memset(szRevBuf, 0, MAX_REVBUF_SIZE);
			dwReadLen = 0;
			if(m_cPort.ReadComm(szRevBuf, MAX_REVBUF_SIZE, &dwReadLen)) 
			{
				printf("ReadComm Read fail!\n");
				return S_PL_READ_FAIL;
			}
			else if(0 != dwReadLen)
			{
				//printf("PreloaderCmd::ReadPattern(): Dump data: ===%s===\n", szRevBuf);
				//printf("PreloaderCmd::ReadPattern(): Dump data size: %u\n", dwReadLen);

				for(unsigned int k = 0; k < dwReadLen; k++)
				{				
					memcpy(szFlowPool, szFlowPool+1, uPoolSize-1);
					memcpy(szFlowPool+uPoolSize-1, szRevBuf+k, 1);

					for(unsigned int s = 0; s < kPatternNum; s++ )
					{
						if(0 == memcmp(szFlowPool + (uPoolSize - pPattern[s].m_uLen), pPattern[s].m_szPattern, pPattern[s].m_uLen))
						{
							//printf("Pattern matched: %s\n", pPattern[s].m_szPattern);
							return pPattern[s].m_uStatus;
						}
					}
				}
			}

			Sleep(m_uInterval);
		}

		printf("Read timeout! Retry time(%u)\n", m_uRetryTime);
		return S_PL_READ_TIMEOUT;
	}
	catch(...)
	{

		printf( "Unknown exception! Err(%u)\n", GetLastError());

		return S_UNDEFINED_ERROR;
	}
}


unsigned int PreloaderCmd::WritePattern(const char * pPattern, const int kPatternLen)
{
	DWORD dwWritelen = 0;

	// Check parameters
	if( (NULL== pPattern) ||(0 >= kPatternLen ) )
	{
		printf( "Incorrect parameters!(NULL pointer or wrong pattern length)\n");
		return S_INVALID_ARGUMENTS;
	}


	if(m_cPort.WriteComm(pPattern, kPatternLen, &dwWritelen)) 
	{
		printf("WriteComm Write fail!\n");
		return S_PL_WRITE_FAIL;
	}

	if(dwWritelen != kPatternLen)
	{
		printf("Write incomplete!May be timeout...\n");
		return S_PL_WRITE_TIMEOUT;
	}

	return S_DONE;
}


unsigned int PreloaderCmd::CMD_BootAsFastboot(BOOT_ARG_S * pArg)
{
	unsigned int iRet = S_DONE;

	//Check parameters
	if(NULL == pArg)
	{
		printf( "Incorrect parameters!(NULL pointer)\n");
		return S_INVALID_ARGUMENTS;
	}

	//Set parameters and open/configure port
	SetParameters(pArg);
	if(0 != (iRet = SetPort(pArg)))
	{
		printf("Set port fail!\n");
		return iRet;
	}


	//printf("Start handshake with Preloader...\n");

	//Read pattern "READY"
	PATTERN_INFO_S stPattern;
	stPattern.m_uStatus = S_DONE;
	stPattern.m_uLen = strlen(kStrReady);
	strcpy(stPattern.m_szPattern, kStrReady);
	iRet = ReadPattern(&stPattern, 1);
	if(S_DONE == iRet)
	{
		//printf("PreloaderCmd::CMD_BootAsFastboot(): Receive READY succeed!\n");
	}
	else
	{
		printf( "Cannot receive pattern wanted!Ret(%u).\n", iRet);
		m_cPort.Close();
		return iRet;
	}


	//Write pattern "FASTBOOT" and its parameters
	PL_MODE_PARA_S tModePara;
	PARA_VALUE_S_V1 tValue;

	tValue.m_cKernelUSBType = (pArg->m_bIsCompositeDeviceEnable) ? (0) : (1);
	tValue.m_cKernelUSBNum= 0;
	tValue.dummy1= 0;
	tValue.dummy2= 0;

	memcpy(tModePara.m_aMode, kStrFBTReq, strlen(kStrFBTReq));    
	tModePara.m_uParaLen = sizeof(PARA_VALUE_S_V1);
	tModePara.m_uParaVer = 1;
	tModePara.m_nParaVal.m_tParaV1 = tValue;


	//iRet = WritePattern(kStrFBTReq, strlen(kStrFBTReq));
	iRet = WritePattern((char *)&tModePara, sizeof(PL_MODE_PARA_S));
	if(S_DONE == iRet)
	{
		//printf( "Send Request.\n");
	}
	else
	{
		printf("Send request fail. Ret(%u).\n", iRet);
		m_cPort.Close();
		return iRet;
	}

	//Read pattern "TOOBTSAF"
	PATTERN_INFO_S stFBTPattern[1];
	stFBTPattern[0].m_uStatus = S_DONE;
	stFBTPattern[0].m_uLen = strlen(kStrFBTSuccess);
	strcpy(stFBTPattern[0].m_szPattern, kStrFBTSuccess);

	//stFBTPattern[1].m_uStatus = S_PL_MODE_FORBIDDEN;
	//stFBTPattern[1].m_uLen = strlen(kStrFBTInvalid);
	//strcpy(stFBTPattern[1].m_szPattern, kStrFBTInvalid);
	iRet = ReadPattern(stFBTPattern, 1);
#if 0
	stPattern.m_uStatus = S_DONE;
	stPattern.m_uLen = strlen(kStrFBTSuccess);
	strcpy(stPattern.m_szPattern, kStrFBTSuccess);
	iRet = ReadPattern(&stPattern, 1);
#endif

	if(S_DONE == iRet)
	{
		//printf("Receive confirm succeed.\n");
	}   
	else if(S_PL_MODE_INVALID_ARGUMETS == iRet)
	{
		printf("FASTBOOT mode arguments is invalid!\n");
	}
	else
	{
		printf("Cannot receive pattern wanted!Err(%u).\n", iRet);
	}

	m_cPort.Close();
	return iRet;    
}


