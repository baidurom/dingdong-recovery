/******************************************************************************
 * mt65xx_leds.c
 * 
 * Copyright 2010 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *
 ******************************************************************************/

//#include <common.h>
//#include <platform/mt.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_typedefs.h>


// FIXME: should include power related APIs

#include <platform/mt_pwm.h>
#include <platform/mt_gpio.h>
#include <platform/mt_leds.h>
//#include <asm/io.h>

#include <platform/mt_pmic.h> 


//extern void mt_power_off (U32 pwm_no);
//extern S32 mt_set_pwm_disable ( U32 pwm_no );
extern void mt_pwm_disable(U32 pwm_no, BOOL pmic_pad);

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
int debug_enable = 0;
#define LEDS_DEBUG(format, args...) do{ \
		if(debug_enable) \
		{\
	//		printf(format,##args);\
		}\
	}while(0)
#define LEDS_INFO LEDS_DEBUG 	
/****************************************************************************
 * structures
 ***************************************************************************/
static int g_lastlevel[MT65XX_LED_TYPE_TOTAL] = {-1, -1, -1, -1, -1, -1, -1};
int backlight_PWM_div = CLK_DIV1;
/****************************************************************************
 * function prototypes
 ***************************************************************************/

/* import functions */
// FIXME: should extern from pmu driver
//void pmic_backlight_on(void) {}
//void pmic_backlight_off(void) {}
//void pmic_config_interface(kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT) {}

/* internal functions */
static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data);
static int led_set_pwm(int pwm_num, enum led_brightness level);
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level);
//static int brightness_set_gpio(int gpio_num, enum led_brightness level);
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level);

/****************************************************************************
 * global variables
 ***************************************************************************/

/****************************************************************************
 * internal functions
 ***************************************************************************/

static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data)
{
struct pwm_spec_config pwm_setting;
	
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.mode = PWM_MODE_FIFO; //new mode fifo and periodical mode
//	pwm_setting.clk_div = CLK_DIV1;
//	pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
	pwm_setting.pmic_pad = config_data->pmic_pad;
	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
   else
     pwm_setting.clk_div = CLK_DIV1;
	if(config_data->clock_source)
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;

	if(config_data->High_duration && config_data->low_duration)
		{
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
			//pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = config_data->low_duration;
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION;
		}
	else
		{
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION = 4;
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = 4;
		}
	
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
//	pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
//	pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION+1)*32 - 1;
	//pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	
	printf("[LEDS]LK: backlight_set_pwm:duty is %d\n", level);
  
	if(level > 0)
	{
        pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA0 = ((1 << 30) - 1);
	    //pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0 ;
		pwm_set_spec_config(&pwm_setting);
        }
        else
 	{
 	    //mt_set_pwm_disable(pwm_setting.pwm_no);
	    //mt_power_off (pwm_setting.pwm_no);
	    mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
    }
	
	
	return 0;
	
}

static int led_set_pwm(int pwm_num, enum led_brightness level)
{
	struct pwm_spec_config pwm_setting;
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.clk_div = CLK_DIV1; 		
	//pwm_setting.duration = 10;
	pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH = 10;
    
    if(pwm_num != PWM3 && pwm_num != PWM4 && pwm_num != PWM5)
		pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
    
	if(level)
	{
		//pwm_setting.duty = 30;
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = 30;
	}else
	{
		//pwm_setting.duty = 0;
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = 0;
	}
	printf("[LEDS]LK: brightness_set_pwm: level=%d, clk=%d \n\r", level, pwm_setting.clk_src);
	//pwm_set_easy_config(&pwm_setting);
	pwm_set_spec_config(&pwm_setting);
	return 0;
	
}



static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level)
{
	int temp_level;
	
	printf("[LEDS]LED PMIC#%d:%d\n", pmic_type, level);

		if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
	{
			if(level)
			{
				//hwBacklightISINKTuning(1, PMIC_PWM_0, 0x3, 0);
				//upmu_boost_isink_hw_sel(0x0);
				//upmu_isinks_ch1_mode(0);
				//upmu_isinks_ch1_step(3);
				//upmu_isinks_ch1_cabc_en(0);	

				//hwBacklightISINKTuning(2, PMIC_PWM_0, 0x3, 0);
				//upmu_isinks_ch2_mode(0);
				//upmu_isinks_ch2_step(3);
				//upmu_isinks_ch2_cabc_en(0);	

				//hwBacklightISINKTuning(3, PMIC_PWM_0, 0x3, 0);
				//upmu_isinks_ch3_mode(0);
				//upmu_isinks_ch3_step(3);
				//upmu_isinks_ch3_cabc_en(0);	

				//hwPWMsetting(PMIC_PWM_0, 6, 0);
				//upmu_isinks_dim0_duty(6);
				//upmu_isinks_dim0_fsel(0);

				//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK5);
				//upmu_top2_bst_drv_ck_pdn(0x0);
				//upmu_isinks_ch1_en(0x1);
				//upmu_isinks_ch2_en(0x1);
				//upmu_isinks_ch3_en(0x1);
			}
			else
			{
				//upmu_isinks_ch1_en(0x0);
				//upmu_isinks_ch2_en(0x0);
				//upmu_isinks_ch3_en(0x0);
			}
			
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_LCD_BOOST)
		{
			if (level) 
			{
				temp_level = level/17;
				printf("[LEDS]MT65XX_LED_PMIC_LCD_BOOST:level=%d  temp_level=%d\n", level, temp_level);
				//hwBacklightBoostTuning(PMIC_PWM_0, 0xA, 0);
				//upmu_boost_isink_hw_sel(0x1);
				//upmu_boost_mode(0);
				//upmu_boost_vrsel(0xA);
				//upmu_boost_cabc_en(0);
	
				//hwPWMsetting(PMIC_PWM_0, level, div);
				//upmu_isinks_dim0_duty(temp_level);
				//upmu_isinks_dim0_fsel(0);
				
				//upmu_top2_bst_drv_ck_pdn(0x0);
				//upmu_boost_en(0x1);
			}
			else 
			{	
				printf("[LEDS]MT65XX_LED_PMIC_LCD_BOOST:level=%d\n", level);
				//pmic_bank1_config_interface(0x3F, 0x00, 0xFF, 0x0);
				//upmu_boost_en(0x0);
			}
		return 0;
	}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK4)
	{
                        //upmu_isinks_ch0_mode(3);   //register mode
                        //upmu_isinks_ch0_step(0);  //ch0_step 4ma
                        //upmu_isinks_ch0_cabc_en(0);         

                        //upmu_top2_bst_drv_ck_pdn(0x0);
                        //upmu_isinks_ch0_en(0x1);  //enable the ISINK0

			if (level) 
			{
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK4, PMIC_PWM_1, 0x3, 0);
				//upmu_isinks_ch4_mode(1);
				//upmu_isinks_ch4_step(0);   //change to 4ma to cut the power consumption
				//upmu_isinks_ch4_cabc_en(0);	
				
				//hwPWMsetting(PMIC_PWM_1, 31, 8);
				//upmu_isinks_dim1_duty(15);  //change duty to 50%, can customize
				//upmu_isinks_dim1_fsel(8);
		
				//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK4);
				//upmu_top2_bst_drv_ck_pdn(0x0);
				//upmu_isinks_ch4_en(0x1);
			}
			else 
			{
				//hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK4);
				//upmu_isinks_ch4_en(0x0);
			}
		return 0;
	}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK5)
		{
                        //upmu_isinks_ch0_mode(3);   //register mode
                        //upmu_isinks_ch0_step(0);  //ch0_step 4ma
                        //upmu_isinks_ch0_cabc_en(0);         

                        //upmu_top2_bst_drv_ck_pdn(0x0);
                        //upmu_isinks_ch0_en(0x1);  //enable the ISINK0
				
			if (level) 
			{
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK5, PMIC_PWM_2, 0Xa, 0);
				//upmu_isinks_ch5_mode(2);
				//upmu_isinks_ch5_step(0);   //change to 4ma to cut the power consumption
				//upmu_isinks_ch5_cabc_en(0);	
				
				//hwPWMsetting(PMIC_PWM_2, 31, 8);
				//upmu_isinks_dim2_duty(15);  //change duty to 50%, can customize
				//upmu_isinks_dim2_fsel(8);
				
				//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK5);
				//upmu_top2_bst_drv_ck_pdn(0x0);
				//upmu_isinks_ch5_en(0x1);
			}
			else 
			{
				//hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK5);
				//upmu_isinks_ch5_en(0x0);
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0)
		{
                        
			if (level) 
			{
				upmu_set_isinks_ch0_mode(0x0);
				upmu_set_isinks_ch0_step(0);
				
				upmu_set_isink_dim0_duty(15);
				upmu_set_isink_dim0_fsel(11);

				upmu_set_rg_bst_drv_1m_ck_pdn(0);
				#ifdef ISINK_CHOP_CLK
				upmu_set_isinks0_chop_en(0x1);
				#endif
				upmu_set_isinks_ch0_en(0x1);
			}
			else 
			{
				//hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK5);
				upmu_set_isinks_ch0_en(0x0);
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK1)
		{
                        
			if (level) 
			{
				upmu_set_isinks_ch1_mode(0x0);
				upmu_set_isinks_ch1_step(0);
				
				upmu_set_isink_dim1_duty(15);
				upmu_set_isink_dim1_fsel(11);

				upmu_set_rg_bst_drv_1m_ck_pdn(0);
				#ifdef ISINK_CHOP_CLK
				upmu_set_isinks1_chop_en(0x1);
				#endif
				upmu_set_isinks_ch1_en(0x1);
			}
			else 
			{
				//hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK5);
				upmu_set_isinks_ch1_en(0x0);
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK2)
		{
                        
			if (level) 
			{
				upmu_set_isinks_ch2_mode(0x0);
				upmu_set_isinks_ch2_step(0);
				
				upmu_set_isink_dim2_duty(15);
				upmu_set_isink_dim2_fsel(11);

				upmu_set_rg_bst_drv_1m_ck_pdn(0);
				#ifdef ISINK_CHOP_CLK
				upmu_set_isinks2_chop_en(0x1);
				#endif
				upmu_set_isinks_ch2_en(0x1);
			}
			else 
			{
				//hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK5);
				upmu_set_isinks_ch2_en(0x0);
			}
			return 0;
		}
	return -1;
}

#if 0
static int brightness_set_gpio(int gpio_num, enum led_brightness level)
{
//	LEDS_INFO("LED GPIO#%d:%d\n", gpio_num, level);
	mt_set_gpio_mode(gpio_num, GPIO_MODE_00);// GPIO MODE
	mt_set_gpio_dir(gpio_num, GPIO_DIR_OUT);

	if (level)
		mt_set_gpio_out(gpio_num, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(gpio_num, GPIO_OUT_ZERO);

	return 0;
}
#endif

static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
			return brightness_set_pwm(cust->data, level,&cust->config_data);
			}
			else
			{
				return led_set_pwm(cust->data, level);
			}
		
		case MT65XX_LED_MODE_GPIO:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_PMIC:
			return brightness_set_pmic(cust->data, level);
		case MT65XX_LED_MODE_CUST_LCM:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_CUST_BLS_PWM:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

/****************************************************************************
 * external functions
 ***************************************************************************/
int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();

	if (type >= MT65XX_LED_TYPE_TOTAL)
		return -1;

	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

	if (g_lastlevel[type] != level) {
		g_lastlevel[type] = level;
		printf("[LEDS]LK: %s level is %d \n\r", cust_led_list[type].name, level);
		return mt65xx_led_set_cust(&cust_led_list[type], level);
	}
	else {
		return -1;
	}

}

void leds_battery_full_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_low_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_medium_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_init(void)
{
	printf("[LEDS]LK: leds_init: mt65xx_backlight_off \n\r");
	mt65xx_backlight_off();
}

void isink0_init(void)
{
	printf("[LEDS]LK: isink_init: turn on PMIC6320 isink \n\r");
	//upmu_isinks_ch0_mode(3);   //register mode
	//upmu_isinks_ch0_step(0);  //step 4ma,can customize
	//upmu_isinks_ch0_cabc_en(0);	

	//upmu_top2_bst_drv_ck_pdn(0x0);
	//upmu_isinks_ch0_en(0x1);  //enable the ISINK0	
}

void leds_deinit(void)
{
    printf("[LEDS]LK: leds_deinit: LEDS off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void mt65xx_backlight_on(void)
{
	printf("[LEDS]LK: mt65xx_backlight_on \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_FULL);
}

void mt65xx_backlight_off(void)
{
	printf("[LEDS]LK: mt65xx_backlight_off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_OFF);
}

