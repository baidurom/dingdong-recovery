#ifndef __DFE_DRV_H__
#define __DFE_DRV_H__

#include "mt_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned int test_length;
	unsigned int rd_enable;
	unsigned int rd_addr;
	unsigned int wr_enable;
	unsigned int wr_addr;
} DFE_para;

// ---------------------------------------------------------------------------
void DFE_Config(DFE_para parameters);
void DFE_Enable(bool enable);

#ifdef __cplusplus
}
#endif

#endif // __DPI_DRV_H__
