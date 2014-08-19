#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
//#define TPD_POWER_SOURCE_CUSTOM MT65XX_POWER_LDO_VGP4 
#define TPD_POWER_SOURCE_1800 MT65XX_POWER_LDO_VGP5

#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

#define TPD_DELAY                (2*HZ/100)
//#define TPD_RES_X                480
//#define TPD_RES_Y                800
#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};

#define TPD_HAVE_CALIBRATION
#define TPD_HAVE_BUTTON
#define TPD_HAVE_TREMBLE_ELIMINATION

//#define TPD_NO_GPIO
#define TPD_RESET_PIN_ADDR   (PERICFG_BASE + 0xC000)

#define PRESSURE_FACTOR	10

#if (defined(WVGA))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,870,60,50},{180,870,60,50},{300,870,60,50},{420,870,60,50}}

#elif (defined(FWVGA))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,920,60,50},{180,920,60,50},{300,920,60,50},{420,920,60,50}}

#elif (defined(QHD))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{85,1030,60,50},{185,1030,60,50},{350,1030,60,50},{500,1030,60,50}}

#elif (defined(HD) || defined(HD720))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{90,1350,60,50},{270,1350,60,50},{430,1350,60,50},{630,1350,60,50}}

#elif (defined(FHD))

#define TPD_KEY_COUNT           3
#define TPD_KEYS                {KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM            {{200,2100,100,100},{500,2100,100,100},{800,2100,100,100}}

#elif (defined(HVGA))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{40,530,60,50},{120,530,60,50},{200,530,60,50},{280,530,60,50}}

#elif (defined(LQHD))

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{50,1030,60,50},{185,1030,60,50},{350,1030,60,50},{500,1030,60,50}}

#else

#define TPD_KEY_COUNT           4
#define TPD_KEYS                {KEY_HOMEPAGE, KEY_MENU, KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,920,60,50},{180,920,60,50},{300,920,60,50},{420,920,60,50}}

#endif
#endif /* TOUCHPANEL_H__ */
