#ifndef _S5K3H2YXAF_H
#define _S5K3H2YXAF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define S5K3H2YXAF_MAGIC 'A'
//IOCTRL(inode * ,file * ,cmd ,arg )


//Structures
typedef struct {
//current position
unsigned long u4CurrentPosition;
//macro position
unsigned long u4MacroPosition;
//Infiniti position
unsigned long u4InfPosition;
//Motor Status
bool          bIsMotorMoving;
//Motor Open?
bool          bIsMotorOpen;
//Support SR?
bool          bIsSupportSR;
} stS5K3H2YXAF_MotorInfo;

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"             
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define S5K3H2YXAFIOC_G_MOTORINFO _IOR(S5K3H2YXAF_MAGIC,0,stS5K3H2YXAF_MotorInfo)

#define S5K3H2YXAFIOC_T_MOVETO _IOW(S5K3H2YXAF_MAGIC,1,unsigned long)

#define S5K3H2YXAFIOC_T_SETINFPOS _IOW(S5K3H2YXAF_MAGIC,2,unsigned long)

#define S5K3H2YXAFIOC_T_SETMACROPOS _IOW(S5K3H2YXAF_MAGIC,3,unsigned long)

#else
#endif
