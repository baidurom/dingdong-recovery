#ifndef __UFOE_DRV_H__
#define __UFOE_DRV_H__

#ifdef BUILD_UBOOT
#include <asm/arch/mt65xx_typedefs.h>
#else
#include <mach/mt_typedefs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned int ufoe_cfg_0;
	unsigned int ufoe_cfg_1;
	unsigned int bypass;
	unsigned int width;
	unsigned int height;
} UFOE_para;

// ---------------------------------------------------------------------------
void UFOE_Config(UFOE_para ufoe_config);
void UFOE_Start(void);
void UFOE_BackupRegisters(void);
void UFOE_RestoreRegisters(void);
void UFOE_PowerOn(void);
void UFOE_PowerOff(void);
void UFOE_Init(BOOL isUFOEPoweredOn);
void UFOE_DumpRegisters(void);

#ifdef __cplusplus
}
#endif

#endif // __UFOE_DRV_H__
