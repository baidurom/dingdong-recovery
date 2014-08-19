/*****************************************************************************
*
* Filename:
* ---------
*   ncp6335.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   ncp6335 header file
*
* Author:
* -------
*
****************************************************************************/

#ifndef _ncp6335_SW_H_
#define _ncp6335_SW_H_

#define ncp6335_REG_NUM 23

extern void ncp6335_dump_register(void);
extern kal_uint32 ncp6335_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT);
extern kal_uint32 ncp6335_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT);

#endif // _ncp6335_SW_H_

