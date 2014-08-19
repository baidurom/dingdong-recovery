
#include "NewCOMUtil.h"
#include "stdio.h"

NewCOMUtil::NewCOMUtil()
{
    m_hCOM = INVALID_HANDLE_VALUE;
}

NewCOMUtil::~NewCOMUtil()
{
}

int NewCOMUtil::SetCommTimeout(unsigned int rd_timeout_msec, unsigned int wr_timeout_msec)
{
    COMMTIMEOUTS  timeouts={0};
    
    timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = rd_timeout_msec;
    timeouts.WriteTotalTimeoutConstant = wr_timeout_msec;
    
    if(!::SetCommTimeouts(m_hCOM, &timeouts)) 
    {
        printf("NewCOMUtil::SetCommTimeout(): Set timeout fail! Handle(%p), COMMTIMEOUTS={ %u, %u, %u, %u, %u }, Err(%u).", m_hCOM,
        						timeouts.ReadIntervalTimeout,
        						timeouts.ReadTotalTimeoutMultiplier,
        						timeouts.ReadTotalTimeoutConstant,
        						timeouts.WriteTotalTimeoutMultiplier,
        						timeouts.WriteTotalTimeoutConstant,
        						GetLastError());
        return 1;
    }

    return 0;
}

int NewCOMUtil::GetCommTimeout(unsigned int &rd_timeout_msec, unsigned int &wr_timeout_msec) 
{
    COMMTIMEOUTS  timeouts={0};
    
    if(!::GetCommTimeouts(m_hCOM, &timeouts)) 
    {
        printf( "NewCOMUtil::GetCommTimeout(): Get timeout fail! Handle(%p), COMMTIMEOUTS={ %u, %u, %u, %u, %u }, Err(%u).", m_hCOM,
        				timeouts.ReadIntervalTimeout,
        				timeouts.ReadTotalTimeoutMultiplier,
        				timeouts.ReadTotalTimeoutConstant,
        				timeouts.WriteTotalTimeoutMultiplier,
        				timeouts.WriteTotalTimeoutConstant,
        				GetLastError());
        return 1;
    }
    
    rd_timeout_msec = timeouts.ReadTotalTimeoutConstant;
    wr_timeout_msec = timeouts.WriteTotalTimeoutConstant;
    
    return 0;
}

int NewCOMUtil::ChangeCommState(unsigned int baudrate) 
{
    DWORD err;
    DCB  dcb;
    COMSTAT  comstat;

    ::ClearCommError(m_hCOM, &err, &comstat);    
    if( !::GetCommState(m_hCOM, &dcb) ) 
    {
       printf("NewCOMUtil::ChangeCommState(): Get state fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
       return 1;
    }
    
    // WARNING!!! abort on error flag must be disabled, 
    // otherwise all the com port apis will fail after using HyperTerminal 
    dcb.fAbortOnError = FALSE;
    dcb.BaudRate = baudrate;
    
    dcb.DCBlength = sizeof(DCB);
    dcb.Parity = NOPARITY;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;    
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.ErrorChar = 0;
    dcb.EofChar = 0;
    dcb.EvtChar = 0;
    dcb.fTXContinueOnXoff = FALSE;	
    dcb.fRtsControl = RTS_CONTROL_ENABLE;  
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.XonChar = 0;
    dcb.XoffChar = 0;
    
    if( !::SetCommState(m_hCOM, &dcb) ) 
    {
        printf("NewCOMUtil::ChangeCommState(): Set state fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 2;
    }
    
    if( !::PurgeComm(m_hCOM, PURGE_TXABORT|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_RXCLEAR) ) 
    {
        printf("NewCOMUtil::ChangeCommState(): Purge fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 3;
    }
    
    return 0;
}

int NewCOMUtil::DumpState()
{
    COMMPROP prop;
    DCB dcb;

    if (!::GetCommProperties(m_hCOM, &prop)) 
    {    
        printf("NewCOMUtil::%s: Get properties fail! Handle(%p), Err(%u) !", __FUNCTION__, m_hCOM, GetLastError());
        return 1;
    } 
    else if (!::GetCommState(m_hCOM, &dcb)) 
    {
        printf("NewCOMUtil::%s: Get state fail! Handle(%p), Err(%u) !", __FUNCTION__, m_hCOM, GetLastError());
        return 2;
    }

   printf("NewCOMUtil::%s(%p): dwMaxTxQueue(%d), dwMaxRxQueue(%d), dwMaxBaud(0x%x), dwProvSubType(0x%x), dwProvCapabilities(0x%x) ", 
           __FUNCTION__, m_hCOM, prop.dwMaxTxQueue, prop.dwMaxRxQueue, prop.dwMaxBaud, prop.dwProvSubType, prop.dwProvCapabilities);
    printf( "NewCOMUtil::%s(%p): dwSettableParams(0x%x), dwSettableBaud(0x%x), wSettableData(0x%x), wSettableStopParity(0x%x), dwCurrentTxQueue(%d), dwCurrentRxQueue(%d)", 
           __FUNCTION__, m_hCOM, prop.dwSettableParams, prop.dwSettableBaud, prop.wSettableData, prop.wSettableStopParity, prop.dwCurrentTxQueue, prop.dwCurrentRxQueue);
    printf("NewCOMUtil::%s(%p): baud(%d), XonLim(%d), XoffLim(%d), ByteSize(%d), Parity(%d), StopBits(%d), XonChar(%d), XoffChar(%d), ErrorChar(%d), EofChar(%d), EvtChar(%d)",
           __FUNCTION__, m_hCOM, dcb.BaudRate, dcb.XonLim, dcb.XoffLim, dcb.ByteSize, dcb.Parity, dcb.StopBits, dcb.XonChar, dcb.XoffChar, dcb.ErrorChar, dcb.EofChar, dcb.EvtChar);    
    printf("NewCOMUtil::%s(%p): fBinary(%d), fParity(%d), fOutxCtsFlow(%d), fOutxDsrFlow(%d), fDtrControl(%d), fDsrSensitivity(%d)",
           __FUNCTION__, m_hCOM, dcb.fBinary, dcb.fParity, dcb.fOutxCtsFlow, dcb.fOutxDsrFlow, dcb.fDtrControl, dcb.fDsrSensitivity);
    printf("NewCOMUtil::%s(%p): fTXContinueOnXoff(%d), fOutX(%d), fInX(%d), fErrorChar(%d), fNull(%d), fRtsControl(%d), fAbortOnError(%d)",
           __FUNCTION__, m_hCOM, dcb.fTXContinueOnXoff, dcb.fOutX, dcb.fInX, dcb.fErrorChar, dcb.fNull, dcb.fRtsControl, dcb.fAbortOnError);    

    return 0;        
}

int NewCOMUtil::WriteComm(const void * pBuf,  const DWORD dwPredictLen, DWORD *pWriteLen)
{
    if(!WriteFile(m_hCOM, pBuf, dwPredictLen, pWriteLen, NULL)) 
    {
        printf("NewCOMUtil::WriteComm(): Write fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 1;
    }

    return 0;
}

int NewCOMUtil::ReadComm(void * pBuf, const DWORD dwPredictLen, DWORD *pReadLen)
{
    if(!ReadFile(m_hCOM, pBuf, dwPredictLen, pReadLen, NULL)) 
    {
        printf( "NewCOMUtil::ReadComm(): Read fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 1;
    }
    
    return 0;
}

int NewCOMUtil::FlushComm()
{
    if(!FlushFileBuffers(m_hCOM)) 
    {
        printf( "NewCOMUtil::FlushComm(): Flush fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 1;
    }

    return 0;
}

int NewCOMUtil::GetCommBaudrate(DWORD *pBaudrate)
{
    DCB  dcb;
    if(!::GetCommState(m_hCOM, &dcb)) 
    {
    	printf( "NewCOMUtil::GetCommBaudrate(): Get baudrate fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
    	return 1;
    }
	
    if (NULL != pBaudrate)
        *pBaudrate = dcb.BaudRate;
        
    return 0;
}

int NewCOMUtil::SetCommBaudrate(const DWORD dwBaudrate)
{
    DCB  dcb;
    
    if( !::GetCommState(m_hCOM, &dcb) ) 
    {
        printf("NewCOMUtil::SetCommBaudrate(): Get baudrate fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 1;
    }
    
    dcb.BaudRate = dwBaudrate;
    if( !::SetCommState(m_hCOM, &dcb) ) 
    {
        printf("NewCOMUtil::SetCommBaudrate(): Set baudrate fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 2;
    }
    
   // printf( "NewCOMUtil::SetCommBaudrate(): Set baudrate to %u", dwBaudrate);
    return 0;
}

int NewCOMUtil::PurgeComm(DWORD *p_ret) 
{
    if(!::PurgeComm(m_hCOM, PURGE_TXABORT|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_RXCLEAR)) 
    {
        printf("NewCOMUtil::PurgeComm(): Purge fail!, Handle(%p), Err(%u).",  m_hCOM, GetLastError());
        return 1;
    }
    
    return 0;
}

int NewCOMUtil::SetupCommBuffer(const DWORD dwInput, const DWORD dwOutput) 
{
    DWORD err;
    //DCB  dcb;
    COMSTAT  comstat;

    ::ClearCommError(m_hCOM, &err, &comstat);    
    if( !::SetupComm(m_hCOM, dwInput, dwOutput) ) 
    {
        printf("NewCOMUtil::SetupCommBuffer(): Setup buffer fail! Handle(%p), Err(%u).", m_hCOM, GetLastError());
        return 1;
    }

    return 0;
}

int NewCOMUtil::Open(const char * pPortName) 
{
    m_hCOM = ::CreateFile(pPortName, 
                                              GENERIC_READ | GENERIC_WRITE, 
                                              0, 0, OPEN_EXISTING, 
                                              FILE_ATTRIBUTE_NORMAL, 0);
    if( INVALID_HANDLE_VALUE == m_hCOM ) 
    {
        printf("NewCOMUtil::Open(): Open COM port  fail! Err(%u), Port name(%s).", GetLastError(), pPortName);
        return 1;    
    }
    
    return 0;
}

int NewCOMUtil::Close()
{
    if(!::CloseHandle(m_hCOM))
    {
        printf( "NewCOMUtil::Close(): Close COM port  fail! Err(%u).", GetLastError());
        return 1;        
    }

    return 0;
}


