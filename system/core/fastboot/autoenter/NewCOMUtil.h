#ifndef _NEW_COMUTIL_H_
#define _NEW_COMUTIL_H_

#include "windows.h"

class NewCOMUtil
{
public:

    NewCOMUtil();
    ~NewCOMUtil();

    int SetCommTimeout(unsigned int rd_timeout_msec, unsigned int wr_timeout_msec);
    int GetCommTimeout(unsigned int &rd_timeout_msec, unsigned int &wr_timeout_msec);
    int ChangeCommState(unsigned int baudrate);
    int DumpCommState();
    int WriteComm(const void * pBuf,  const DWORD dwPredictLen, DWORD *pWriteLen);
    int ReadComm(void * pBuf, const DWORD dwPredictLen, DWORD *pReadLen);
    int FlushComm();
    int GetCommBaudrate(DWORD *pBaudrate);
    int SetCommBaudrate(const DWORD dwBaudrate);
    int PurgeComm(DWORD *p_ret);
    int SetupCommBuffer(const DWORD dwInput, const DWORD dwOutput);

    int DumpState();
    
    int Open(const char * pPortName);
    int Close();

public:
    HANDLE m_hCOM;
    
};

#endif
