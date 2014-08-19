/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2001
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Error_code.h
 *
 * Project:
 * --------
 *   Device Test
 *
 * Description:
 * ------------
 *   Type definition.
 *
 * Author:
 * -------
 *   Shalyn Chua  (mtk00576)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *
 ****************************************************************************/


#ifndef _MTK_DVC_TEST_TYPEDEFS_H
#define _MTK_DVC_TEST_TYPEDEFS_H


/*==== CONSTANTS ==================================================*/

#define IMPORT  EXTERN
#ifndef __cplusplus
  #define EXTERN  extern
#else
  #define EXTERN  extern "C"
#endif
#define LOCAL     static
#define GLOBAL
#define EXPORT    GLOBAL


#define EQ        ==
#define NEQ       !=
#define AND       &&
#define OR        ||
#define XOR(A,B)  ((!(A) AND (B)) OR ((A) AND !(B)))

#ifndef FALSE
  #define FALSE   0
#endif

#ifndef TRUE
  #define TRUE    1
#endif

#ifndef NULL
  #define NULL    0
#endif

enum boolean { false, true};
enum {RX, TX, NONE};


#ifndef BOOL
typedef unsigned char  BOOL;
#endif

/*==== TYPES ======================================================*/


typedef volatile unsigned char  *P_kal_uint8;
typedef volatile unsigned short *P_kal_uint16;
typedef volatile unsigned int   *P_kal_uint32;


typedef long            LONG;
typedef unsigned char   UBYTE;
typedef short           SHORT;


typedef signed char     kal_int8;
typedef signed short    kal_int16;
typedef signed int      kal_int32;
typedef long long       kal_int64;
typedef unsigned char   kal_uint8;
typedef unsigned short  kal_uint16;
typedef unsigned int    kal_uint32;
typedef unsigned long long  kal_uint64;
typedef char            kal_char;


typedef unsigned int            *UINT32P;
typedef volatile unsigned short *UINT16P;
typedef volatile unsigned char  *UINT8P;
typedef unsigned char           *U8P;

typedef volatile unsigned char  *P_U8;
typedef volatile signed char    *P_S8;
typedef volatile unsigned short *P_U16;
typedef volatile signed short   *P_S16;
typedef volatile unsigned int   *P_U32;
typedef volatile signed int     *P_S32;
typedef unsigned long long      *P_U64;
typedef signed long long        *P_S64;


typedef unsigned char       U8;
typedef signed char         S8;
typedef unsigned short      U16;
typedef signed short        S16;
typedef unsigned int        U32;
typedef signed int          S32;
typedef unsigned long long  U64;
typedef signed long long    S64;
typedef unsigned char       bool;


typedef enum {
   KAL_FALSE = 0,
   KAL_TRUE  = 1,
} kal_bool;


typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef unsigned short  USHORT;
typedef signed char     INT8;
typedef signed short    INT16;
typedef signed int      INT32;
typedef signed int      DWORD;
typedef void            VOID;
typedef unsigned char   BYTE;
typedef float           FLOAT;

typedef char           *LPCSTR;
typedef short          *LPWSTR;


/*==== EXPORT =====================================================*/

#define MAXIMUM(A,B)       (((A)>(B))?(A):(B))
#define MINIMUM(A,B)       (((A)<(B))?(A):(B))

#define READ_REGISTER_UINT32(reg) \
    (*(volatile UINT32 * const)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
    (*(volatile UINT32 * const)(reg)) = (val)

#define READ_REGISTER_UINT16(reg) \
    (*(volatile UINT16 * const)(reg))

#define WRITE_REGISTER_UINT16(reg, val) \
    (*(volatile UINT16 * const)(reg)) = (val)

#define READ_REGISTER_UINT8(reg) \
    (*(volatile UINT8 * const)(reg))

#define WRITE_REGISTER_UINT8(reg, val) \
    (*(volatile UINT8 * const)(reg)) = (val)

#define INREG8(x)           READ_REGISTER_UINT8((UINT8*)(x))
#define OUTREG8(x, y)       WRITE_REGISTER_UINT8((UINT8*)(x), (UINT8)(y))
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))
#define MASKREG8(x, y, z)   OUTREG8(x, (INREG8(x)&~(y))|(z))

#define INREG16(x)          READ_REGISTER_UINT16((UINT16*)(x))
#define OUTREG16(x, y)      WRITE_REGISTER_UINT16((UINT16*)(x),(UINT16)(y))
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))
#define MASKREG16(x, y, z)  OUTREG16(x, (INREG16(x)&~(y))|(z))

#define INREG32(x)          READ_REGISTER_UINT32((UINT32*)(x))
#define OUTREG32(x, y)      WRITE_REGISTER_UINT32((UINT32*)(x), (UINT32)(y))
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))
#define MASKREG32(x, y, z)  OUTREG32(x, (INREG32(x)&~(y))|(z))


#define DRV_Reg8(addr)              INREG8(addr)
#define DRV_WriteReg8(addr, data)   OUTREG8(addr, data)
#define DRV_SetReg8(addr, data)     SETREG8(addr, data)
#define DRV_ClrReg8(addr, data)     CLRREG8(addr, data)

#define DRV_Reg16(addr)             INREG16(addr)
#define DRV_WriteReg16(addr, data)  OUTREG16(addr, data)
#define DRV_SetReg16(addr, data)    SETREG16(addr, data)
#define DRV_ClrReg16(addr, data)    CLRREG16(addr, data)

#define DRV_Reg32(addr)             INREG32(addr)
#define DRV_WriteReg32(addr, data)  OUTREG32(addr, data)
#define DRV_SetReg32(addr, data)    SETREG32(addr, data)
#define DRV_ClrReg32(addr, data)    CLRREG32(addr, data)

// !!! DEPRECATED, WILL BE REMOVED LATER !!!
#define DRV_Reg(addr)               DRV_Reg16(addr)
#define DRV_WriteReg(addr, data)    DRV_WriteReg16(addr, data)
#define DRV_SetReg(addr, data)      DRV_SetReg16(addr, data)
#define DRV_ClrReg(addr, data)      DRV_ClrReg16(addr, data)

#endif

