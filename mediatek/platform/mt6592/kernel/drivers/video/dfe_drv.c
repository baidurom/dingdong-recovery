#include <linux/delay.h>
#include <disp_drv_log.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include "disp_drv_platform.h"

#include "debug.h"

#include <mach/sync_write.h>
#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif

//////////////////////////////////////////////////////
typedef struct
{
    unsigned DFE_TEST_LEN                	: 20;
	unsigned rsv_20					: 2; 
    unsigned DFE_LOOP_MODE                	: 1;
    unsigned DFE_DRE_EN                	: 1;
    unsigned rsv_24					: 8;   	    	
} DFE_CON0_REG, *PDFE_CON0_REG;

typedef struct
{
    unsigned DFE_SLOW_DOWN                	: 10;
	unsigned DFE_RD_DIS					: 1; 
    unsigned DFE_WR_DIS                	: 1;
    unsigned rsv_12					: 20;   	    	
} DFE_CON1_REG, *PDFE_CON1_REG;

typedef struct
{
    unsigned DFE_BUSY                	: 1;
    unsigned rsv_1					: 7;  
	unsigned DFE_WR_ST					: 4; 
    unsigned DFE_RD_ST                	: 3;
    unsigned rsv_15					: 17;   	    	
} DFE_STATE_REG, *PDFE_STATE_REG;

typedef struct
{
    unsigned DFE_RESET                	: 1;
    unsigned rsv_1					: 31;   	    	
} DFE_RST_REG, *PDFE_RST_REG;

typedef struct
{
    unsigned DFE_MUTEX_SEL                	: 1;
	unsigned DFE_START                	: 1;
    unsigned rsv_2					: 30;   	    	
} DFE_EN_REG, *PDFE_EN_REG;

typedef struct
{
    DFE_EN_REG				  DFE_EN;				// 0000
    DFE_RST_REG	  			DFE_RST;					// 0004
    DFE_CON0_REG				DFE_CON0;				// 0008
    DFE_CON1_REG				DFE_CON1;				// 000C
		UINT32              DFE_RD_ADDR;		//10
		UINT32              DFE_WR_ADDR;		//14
		DFE_STATE_REG				DFE_STATE;
} volatile DFE_REGS, *PDFE_REGS;
//////////////////////////////////////////////////////
static PDFE_REGS const DFE_REG = (PDFE_REGS)(DISPSYS_BASE + 0x200);

void DFE_Config(DFE_para dfe_para)
{
	if(dfe_para.test_length != 0)
		DFE_REG->DFE_CON0.DFE_TEST_LEN = dfe_para.test_length;
	else
		DFE_REG->DFE_CON0.DFE_TEST_LEN = 0x1;
	
	DFE_REG->DFE_CON0.DFE_LOOP_MODE = 1;
	DFE_REG->DFE_CON1.DFE_RD_DIS = (1 - dfe_para.rd_enable);
	DFE_REG->DFE_CON1.DFE_WR_DIS = (1 - dfe_para.wr_enable);
	DFE_REG->DFE_RD_ADDR = dfe_para.rd_addr;
	DFE_REG->DFE_WR_ADDR = dfe_para.wr_addr;
	///other config need confirm with DE
	DFE_REG->DFE_EN.DFE_MUTEX_SEL = 1;
	DFE_REG->DFE_CON1.DFE_SLOW_DOWN = 10;
}

void DFE_Enable(bool enable)
{
	if(enable){
		DFE_REG->DFE_EN.DFE_START = 1;
	}
	else{
		//wait for DFE idle
		DFE_REG->DFE_EN.DFE_START = 0;
		while(DFE_REG->DFE_STATE.DFE_BUSY == 1);
//		DFE_REG->DFE_EN.DFE_START = 0;
	}
}