#include <linux/delay.h>
#include <disp_drv_log.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include "disp_drv_platform.h"

#include "debug.h"
#include "ufoe_reg.h"

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
UFOE_REGS regBackup;
static bool s_isUfoePowerOn = FALSE;
static PUFOE_REGS const UFOE_REG = (PUFOE_REGS)(DISP_UFOE_BASE + 0x800);

static void _ResetBackupedUFOERegisterValues(void)
{
    UFOE_REGS *regs = &regBackup;
    memset((void*)regs, 0, sizeof(UFOE_REGS));
}

void UFOE_DumpRegisters(void)
{
	unsigned int i = 0;
	for (i = 0; i < 32; i += 16)
	{
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "UFOE", "UFOE+%04x : 0x%08x   0x%08x  0x%08x  0x%08x\n", i, INREG32((DISP_UFOE_BASE + 0x800+i)), INREG32((DISP_UFOE_BASE + 0x800+i+0x4)), INREG32((DISP_UFOE_BASE + 0x800+i+0x8)), INREG32((DISP_UFOE_BASE + 0x800+i+0xc)));
	}
	for (i = 0x700; i < 0x700 + 96; i += 16)
	{
		DISP_LOG_PRINT(ANDROID_LOG_INFO, "UFOE", "UFOE+%04x : 0x%08x   0x%08x  0x%08x  0x%08x\n", i + 0x800, INREG32((DISP_UFOE_BASE + 0x800+i)), INREG32((DISP_UFOE_BASE + 0x800+i+0x4)), INREG32((DISP_UFOE_BASE + 0x800+i+0x8)), INREG32((DISP_UFOE_BASE + 0x800+i+0xc)));
	}
}

void UFOE_BackupRegisters(void)
{
    UFOE_REGS *regs = &regBackup;

    OUTREG32(&regs->UFOE_CFG_0B, AS_UINT32(&UFOE_REG->UFOE_CFG_0B));
    OUTREG32(&regs->UFOE_CFG_1B, AS_UINT32(&UFOE_REG->UFOE_CFG_1B));
    OUTREG32(&regs->UFOE_FRA_WIDTH, AS_UINT32(&UFOE_REG->UFOE_FRA_WIDTH));
    OUTREG32(&regs->UFOE_FRA_HEIGHT, AS_UINT32(&UFOE_REG->UFOE_FRA_HEIGHT));
	OUTREG32(&regs->UFOE_INTEN, AS_UINT32(&UFOE_REG->UFOE_INTEN));

	regs->UFOE_START.ufoe_bypass = UFOE_REG->UFOE_START.ufoe_bypass;

	OUTREG32(&regs->UFOE_DBUF, AS_UINT32(&UFOE_REG->UFOE_DBUF));
	OUTREG32(&regs->UFOE_CRC, AS_UINT32(&UFOE_REG->UFOE_CRC));
	OUTREG32(&regs->UFOE_SW_SCRATCH, AS_UINT32(&UFOE_REG->UFOE_SW_SCRATCH));
	OUTREG32(&regs->UFOE_CK_ON, AS_UINT32(&UFOE_REG->UFOE_CK_ON));
	OUTREG32(&regs->UFOE_R0_CRC, AS_UINT32(&UFOE_REG->UFOE_R0_CRC));
}


void UFOE_RestoreRegisters(void)
{
    UFOE_REGS *regs = &regBackup;

    OUTREG32(&UFOE_REG->UFOE_CFG_0B, AS_UINT32(&regs->UFOE_CFG_0B));
    OUTREG32(&UFOE_REG->UFOE_CFG_1B, AS_UINT32(&regs->UFOE_CFG_1B));
    OUTREG32(&UFOE_REG->UFOE_FRA_WIDTH, AS_UINT32(&regs->UFOE_FRA_WIDTH));
    OUTREG32(&UFOE_REG->UFOE_FRA_HEIGHT, AS_UINT32(&regs->UFOE_FRA_HEIGHT));

    OUTREG32(&UFOE_REG->UFOE_INTEN, AS_UINT32(&regs->UFOE_INTEN));

	UFOE_REG->UFOE_START.ufoe_bypass = regs->UFOE_START.ufoe_bypass;

	OUTREG32(&UFOE_REG->UFOE_DBUF, AS_UINT32(&regs->UFOE_DBUF));
	OUTREG32(&UFOE_REG->UFOE_CRC, AS_UINT32(&regs->UFOE_CRC));
	OUTREG32(&UFOE_REG->UFOE_SW_SCRATCH, AS_UINT32(&regs->UFOE_SW_SCRATCH));
	OUTREG32(&UFOE_REG->UFOE_CK_ON, AS_UINT32(&regs->UFOE_CK_ON));
	OUTREG32(&UFOE_REG->UFOE_R0_CRC, AS_UINT32(&regs->UFOE_R0_CRC));
}

void UFOE_PowerOn(void)
{
    int ret = 0;
	if (!s_isUfoePowerOn)
    {
        ret += enable_clock(MT_CG_DISP0_UFOE, "UFOE");

        if(ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "UFOE", "UFOE power manager API return FALSE\n");
        }
        s_isUfoePowerOn = TRUE;
//		UFOE_RestoreRegisters();
    }
}

void UFOE_PowerOff(void)
{
    int ret = 0;

	if (s_isUfoePowerOn)
    {
//    	UFOE_BackupRegisters();
        ret += disable_clock(MT_CG_DISP0_UFOE, "UFOE");

        if(ret > 0)
        {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "UFOE", "UFOE power manager API return FALSE\n");
        }

        s_isUfoePowerOn = FALSE;
    }
}

void UFOE_Init(BOOL isUFOEPoweredOn)
{
    if (isUFOEPoweredOn) {
        UFOE_BackupRegisters();
    } else {
        _ResetBackupedUFOERegisterValues();
    }
	UFOE_PowerOn();
    OUTREGBIT(UFOE_INTEN_REG, UFOE_REG->UFOE_INTEN, ufoe_fra_underrun, 1);
    OUTREGBIT(UFOE_INTEN_REG, UFOE_REG->UFOE_INTEN, ufoe_fra_done, 1);
    OUTREGBIT(UFOE_INTEN_REG, UFOE_REG->UFOE_INTEN, ufoe_fra_complete, 1);
//	UFOE_REG->UFOE_CK_ON = 0x1;
}

void UFOE_Config(UFOE_para ufoe_config)
{
    OUTREGBIT(UFOE_START_REG, UFOE_REG->UFOE_START, ufoe_bypass, ufoe_config.bypass);
    OUTREG32(&UFOE_REG->UFOE_CFG_0B, ufoe_config.ufoe_cfg_0);
    OUTREG32(&UFOE_REG->UFOE_CFG_1B, ufoe_config.ufoe_cfg_1);
	OUTREG32(&UFOE_REG->UFOE_FRA_WIDTH, ufoe_config.width);
    OUTREG32(&UFOE_REG->UFOE_FRA_HEIGHT, ufoe_config.height);
	UFOE_DumpRegisters();
}

void UFOE_Start(void)
{
    OUTREGBIT(UFOE_START_REG, UFOE_REG->UFOE_START, ufoe_start, 0);
    OUTREGBIT(UFOE_START_REG, UFOE_REG->UFOE_START, ufoe_start, 1);
}
