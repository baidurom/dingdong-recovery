#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mtk_pmic_6320.h>
#include <asm/arch/upmu_common.h>
#include <asm/arch/upmu_hw.h>

U32 upmu_get_cid(void)
{
  U32 ret=0;
  U32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (U32)(CID),
                           (&val),
                           (U32)(PMIC_CID_MASK),
                           (U32)(PMIC_CID_SHIFT)
	                       );
  pmic_unlock();

  return val;
}
