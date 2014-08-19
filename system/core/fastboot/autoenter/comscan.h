#ifndef _COMSCAN_H_
#define _COMSCAN_H_

int __stdcall GetIncrementCOMPortWithFilter(COM_FILTER_LIST_S * pCOMFilter, COM_PROPERTY_S * pCOMPorperty, const void * pGuid, bool bDeviceInterface, int * pStopFlag, unsigned int dTimeout);
#endif