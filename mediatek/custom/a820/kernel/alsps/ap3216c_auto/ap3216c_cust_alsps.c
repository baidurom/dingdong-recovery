#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
  .i2c_num    = 3,
  .polling_mode_ps =0,
  .polling_mode_als =1,
  .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
  .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
  .i2c_addr   = {0x3C, 0x38, 0x3A, 0x00}, //
  .als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
  .als_value  = {800, 900,1200,  1500, 1800, 2500,  3000,  4000, 6000,  7500,  8500,  9500,  10000, 10000,  10240, 10240},
  .ps_threshold_high = 400,
  .ps_threshold_low = 200,
};
struct alsps_hw *AP3216C_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
