#ifndef __UFOE_REG_H__
#define __UFOE_REG_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned ufoe_start                	: 1;
	unsigned ufoe_out_sel				: 1; 
    unsigned ufoe_bypass                : 1;
    unsigned rsv_3						: 5;
	unsigned ufoe_sw_reset				: 1;
	unsigned rsv_9						: 7;
	unsigned ufoe_dbg_sel				: 8;
	unsigned rsv_24						: 8;
} UFOE_START_REG, *PUFOE_START_REG;

typedef struct
{
    unsigned ufoe_fra_complete   	: 1;
	unsigned ufoe_fra_done		: 1; 
    unsigned ufoe_fra_underrun	: 1;
    unsigned rsv_3						: 29;
} UFOE_INTEN_REG, *PUFOE_INTEN_REG;

typedef struct
{
    unsigned ufoe_fra_complete   	: 1;
	unsigned ufoe_fra_done		: 1; 
    unsigned ufoe_fra_underrun	: 1;
    unsigned rsv_3						: 29;
} UFOE_INTSTA_REG, *PUFOE_INTSTA_REG;

typedef struct
{
    unsigned ufoe_crc_cen   	: 1;
	unsigned ufoe_crc_start		: 1; 
    unsigned ufoe_crc_clr		: 1;
    unsigned rsv_3				: 29;
} UFOE_CRC_REG, *PUFOE_CRC_REG;

typedef struct
{
    unsigned ufoe_crc_out_0   	: 16;
	unsigned ufoe_crc_rdy_0		: 1; 
    unsigned ufoe_engine_end	: 1;
    unsigned rsv_18				: 14;
} UFOE_R0_CRC_REG, *PUFOE_R0_CRC_REG;

typedef struct
{
	UINT32				UFOE_CFG_0B;//0x800
	UINT32				UFOE_CFG_1B;//0x804
	UINT32				rsv_808[446];//0x808~0xEFC
	UFOE_START_REG		UFOE_START;//0xF00
	UFOE_INTEN_REG		UFOE_INTEN;//0xF04
	UFOE_INTSTA_REG		UFOE_INTSTA;//0xF08
	UINT32				UFOE_DBUF;//0xF0C
	UINT32				rsv_F10;//0xF10
	UFOE_CRC_REG		UFOE_CRC;//0xF14
	UINT32				UFOE_SW_SCRATCH;//0xF18
	UINT32				rsv_F1C[3];
	UINT32				UFOE_CK_ON;//0xF28
	UINT32				rsv_F2C[9];
	UINT32				UFOE_FRA_WIDTH;//0xF50
	UINT32				UFOE_FRA_HEIGHT;//0xF54
	UINT32				rsv_F58[38];//0xF58~0xFEC
	UFOE_R0_CRC_REG		UFOE_R0_CRC;//0xFF0
} volatile UFOE_REGS, *PUFOE_REGS;

#ifndef BUILD_LK
STATIC_ASSERT(0x0700 == offsetof(UFOE_REGS, UFOE_START));
STATIC_ASSERT(0x0728 == offsetof(UFOE_REGS, UFOE_CK_ON));
STATIC_ASSERT(0x0750 == offsetof(UFOE_REGS, UFOE_FRA_WIDTH));
STATIC_ASSERT(0x07F0 == offsetof(UFOE_REGS, UFOE_R0_CRC));
#endif

#ifdef __cplusplus
}
#endif

#endif // __UFOE_REG_H__

