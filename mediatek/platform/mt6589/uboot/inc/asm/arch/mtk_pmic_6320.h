#ifndef _MT65xx_PMIC_PL_SW_H_
#define _MT65xx_PMIC_PL_SW_H_

#include <asm/arch/mt65xx_typedefs.h>

//==============================================================================
// PMIC6320 define
//==============================================================================
#define PMIC6320_E1_CID_CODE    0x1020


typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    // AC : 450mA~1A 
    STANDARD_CHARGER,       // AC : ~1A
} CHARGER_TYPE;

//==============================================================================
// PMIC6320 Status Code
//==============================================================================
#define PMIC_TEST_PASS               0x0000
#define PMIC_TEST_FAIL               0xB001
#define PMIC_EXCEED_I2C_FIFO_LENGTH  0xB002
#define PMIC_CHRDET_EXIST            0xB003
#define PMIC_CHRDET_NOT_EXIST        0xB004

//==============================================================================
// PMIC6320 Exported Function
//==============================================================================
extern CHARGER_TYPE mt_charger_type_detection(void);
extern U32 pmic6320_IsUsbCableIn (void);
extern int pmic_detect_powerkey(void);
extern void hw_set_cc(int cc_val);
extern int hw_check_battery(void);
extern void pl_charging(int en_chr);
extern void pl_kick_chr_wdt(void);
extern void pl_close_pre_chr_led(void);
extern void pl_hw_ulc_det(void);
extern U32 pmic6320_init (void);

extern U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT);
extern void pmic_lock(void);
extern void pmic_unlock(void);

#endif // _MT65xx_PMIC_PL_SW_H_

