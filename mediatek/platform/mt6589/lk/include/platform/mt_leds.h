#ifndef _MT65XX_LEDS_H
#define _MT65XX_LEDS_H

#define ISINK_CHOP_CLK
//#include <platform/cust_leds.h>
#include <cust_leds.h>

enum led_color {
	LED_RED,
	LED_GREEN,
	LED_BLUE,
};

enum led_brightness {
	LED_OFF		= 0,
	LED_HALF	= 127,
	LED_FULL	= 255,
};

int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level);
void leds_battery_full_charging(void);
void leds_battery_low_charging(void);
void leds_battery_medium_charging(void);
void mt65xx_backlight_on(void);
void mt65xx_backlight_off(void);
void leds_init(void);
void leds_deinit(void);

#endif // !_MT65XX_LEDS_H
