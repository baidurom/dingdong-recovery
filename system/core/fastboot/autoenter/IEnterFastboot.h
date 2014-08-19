#ifndef _IENTERFASTBOOT_H_
#define _IENTERFASTBOOT_H_

/* Notify phone enter FASTBOOT mode. 
*  This function will scan and communicate with preloader's COM port that parameters indicated.
*  Any COM port in parameters was detected, this port will be the communication agent.
*  return: 0 is OK, -1 is error.
*  params: 
*    preloaderSinglePortFilter: preloader single COM port's PID&VID, string like "VID_0E8D&PID_2000"
*    preloaderCompositePortFilter: preloader composite COM port's PID&VID, string like "VID_1004&PID_6000"
*    if these two were NULL, then default PID&VID will be used: "VID_0E8D&PID_2000" and "VID_1004&PID_6000"
*/
int NotifyEnterFastbootMode(char* preloaderSinglePortFilter, char* preloaderCompositePortFilter);

#endif
