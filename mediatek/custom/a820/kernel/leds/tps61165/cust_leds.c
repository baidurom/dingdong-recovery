/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <cust_leds.h>
#include <mach/mt_pwm.h>
#include <mach/mt_gpio.h>

#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>


static unsigned int back_level = 255;
unsigned int disp_set_backlight(int level)
{
    int now_level,addr = 0x72;
    int i;
#if (defined(V6_X2))
	if(level > 239) {
		level = 239;
	}
#else
	;
#endif
    now_level = (level >> 3);
    if(level == 0) {
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
    } else {
            if(back_level == 0) {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                mdelay(3);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(150);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(450);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(580);
            }
        mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
        udelay(4);
        for(i=0 ;i < 8;i++)
        {
            if(addr&0x80)
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(3);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(7);
            }
            else
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(7);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(3);
            }
            addr <<= 1;
        }
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
        udelay(4);
        mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
        udelay(4);
        for(i=0 ;i < 8;i++)
        {
            if(now_level&0x80)
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(3);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(7);
            }
            else
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(7);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(3);
            }
            now_level <<= 1;
        }
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
        udelay(4);
        mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
        udelay(4);
    }
        back_level = level ;
    return 0;
}
unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    mapped_level = level;

        return mapped_level;
}

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
#ifdef VANZO_LEDS_SUPPORT
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK2,{0}},
#else
	{"red",               MT65XX_LED_MODE_PMIC, -1,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, -1,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, -1,{0}},
#endif
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
#ifdef VANZO_KP_LEDS_SUPPORT
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
#else
	{"button-backlight",  MT65XX_LED_MODE_PMIC, -1,{0}},
#endif
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (int)disp_set_backlight,{0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

