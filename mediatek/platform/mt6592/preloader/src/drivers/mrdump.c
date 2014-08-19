#include <typedefs.h>
#include <platform.h>

#define MRDUMP_SIG1_ADDR 0x10202060
#define MRDUMP_SIG2_ADDR 0x10202064
#define MRDUMP_SIG1 0x5544524d
#define MRDUMP_SIG2 0x3030504d

extern u32 g_ddr_reserve_enable;
extern  u32 g_ddr_reserve_success;

void mrdump_check_ok(void)
{
    if ((DRV_Reg32(MRDUMP_SIG1_ADDR) == 0x5544524d) && 
	(DRV_Reg32(MRDUMP_SIG2_ADDR) == 0x3030504d)) 
      {
	  DRV_WriteReg32(MRDUMP_SIG1_ADDR, 0);
	  DRV_WriteReg32(MRDUMP_SIG2_ADDR, 0);
	  g_ddr_reserve_enable = 1;
	  g_ddr_reserve_success = 1;
	  print("[MRDUMP] DDR reserve mode success.\n");
      }
}
