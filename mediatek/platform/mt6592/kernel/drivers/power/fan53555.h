/*****************************************************************************
*
* Filename:
* ---------
*   fan53555.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   fan53555 header file
*
* Author:
* -------
*
****************************************************************************/

#ifndef _fan53555_SW_H_
#define _fan53555_SW_H_

#define fan53555_REG_NUM 6 

extern void fan53555_dump_register(void);
extern kal_uint32 fan53555_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT);
extern kal_uint32 fan53555_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT);

#endif // _fan53555_SW_H_

