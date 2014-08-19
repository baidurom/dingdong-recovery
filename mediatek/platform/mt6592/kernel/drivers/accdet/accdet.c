//#include "accdet.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/ctype.h>

#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/switch.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/input.h>
#include <linux/wakelock.h>
#include <linux/time.h>

#include <linux/string.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/irqs.h>
#include <mach/reg_accdet.h>
#include <accdet_custom.h>
#include <accdet_custom_def.h>

//#include <mach/mt_boot.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>




/*----------------------------------------------------------------------
static variable defination
----------------------------------------------------------------------*/

#define REGISTER_VALUE(x)   (x - 1)
#define MULTIKEY_ADC_CHANNEL	 (8)


static int button_press_debounce = 0x400;

static int debug_enable = 1;

struct headset_mode_settings *cust_headset_settings_hal = NULL;

#define ACCDET_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_WARNING format,##args);\
	}\
}while(0)


extern S32 pwrap_read( U32  adr, U32 *rdata );
extern S32 pwrap_write( U32  adr, U32  wdata );
extern struct headset_mode_settings* get_cust_headset_settings(void);
extern struct headset_key_custom* get_headset_key_custom_setting(void);
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
extern void accdet_eint_func(void);

#if 0
int get_long_press_time(void)
{
	return long_press_time_cust;
}
#endif

/*********************************************************************
static  APIs
*********************************************************************/

// pmic wrap read and write func
static U32 pmic_pwrap_read(U32 addr)
{

	U32 val =0;
	pwrap_read(addr, &val);
	//ACCDET_DEBUG("[Accdet]wrap write func addr=0x%x, val=0x%x\n", addr, val);
	return val;
	
}

static void pmic_pwrap_write(unsigned int addr, unsigned int wdata)

{
	//unsigned int val =0;
    pwrap_write(addr, wdata);
	//ACCDET_DEBUG("[Accdet]wrap write func addr=0x%x, wdate=0x%x\n", addr, wdata);
}

#ifdef ACCDET_EINT
static void accdet_eint_set_polarity_pre(void)
{

	mt_eint_set_polarity(CUST_EINT_ACCDET_NUM, 1);

}

static void accdet_eint_set_polarity_next(void)
{

	mt_eint_set_polarity(CUST_EINT_ACCDET_NUM, 0);

}
#endif
/*********************************************************************
HAL APIs
*********************************************************************/
void accdet_eint_unmask(void)
{
#ifdef ACCDET_EINT
	mt_eint_unmask(CUST_EINT_ACCDET_NUM);  
#endif
}

void accdet_eint_mask(void)
{
#ifdef ACCDET_EINT
	mt_eint_mask(CUST_EINT_ACCDET_NUM); 
#endif
}

void accdet_eint_set_hw_debounce(void)
{
#ifdef ACCDET_EINT
	mt_eint_set_hw_debounce(CUST_EINT_ACCDET_NUM, CUST_EINT_ACCDET_DEBOUNCE_CN);
#endif
}

void accdet_eint_set_hw_cust_debounce(void)
{
#ifdef ACCDET_EINT
	mt_eint_set_hw_debounce(CUST_EINT_ACCDET_NUM, ACCDET_SHORT_PLUGOUT_DEBOUNCE_CN);
#endif
}

void accdet_eint_set_sens(void)
{
#ifdef ACCDET_EINT
	//mt_eint_set_sens(CUST_EINT_ACCDET_NUM, CUST_EINT_ACCDET_SENSITIVE);
#endif
}

void accdet_eint_set(void)
{
#ifdef ACCDET_EINT
	mt_set_gpio_mode(GPIO_ACCDET_EINT_PIN, GPIO_ACCDET_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_ACCDET_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_ACCDET_EINT_PIN, GPIO_PULL_DISABLE); //To disable GPIO PULL.
    accdet_eint_set_sens();
	accdet_eint_set_hw_debounce();
#endif
	
}
void accdet_eint_registration(void)
{
#ifdef ACCDET_EINT
	mt_eint_registration(CUST_EINT_ACCDET_NUM, CUST_EINT_ACCDET_TYPE, accdet_eint_func, 0);
	ACCDET_DEBUG("[Accdet]accdet set EINT finished, accdet_eint_num=%d, accdet_eint_debounce_en=%d, accdet_eint_polarity=%d\n", CUST_EINT_ACCDET_NUM, CUST_EINT_ACCDET_DEBOUNCE_EN, CUST_EINT_ACCDET_TYPE);
#endif
}
void accdet_eint_polarity_reverse_plugout(void)
{	
#ifdef ACCDET_EINT
	if (CUST_EINT_ACCDET_TYPE == CUST_EINTF_TRIGGER_HIGH){
		//mt_set_gpio_pull_select(GPIO_ACCDET_EINT_PIN, GPIO_PULL_DOWN);
		accdet_eint_set_polarity_pre();
	}else{
		//mt_set_gpio_pull_select(GPIO_ACCDET_EINT_PIN, GPIO_PULL_UP);
		accdet_eint_set_polarity_next();
	}
	accdet_eint_set_hw_debounce();
#endif
}

void accdet_eint_polarity_reverse_plugin(void)
{
#ifdef ACCDET_EINT
	if (CUST_EINT_ACCDET_TYPE == CUST_EINTF_TRIGGER_HIGH){
		//mt_set_gpio_pull_select(GPIO_ACCDET_EINT_PIN, GPIO_PULL_UP);
		accdet_eint_set_polarity_next();
	}else{
		//mt_set_gpio_pull_select(GPIO_ACCDET_EINT_PIN, GPIO_PULL_DOWN);
		accdet_eint_set_polarity_pre();
	}
	//ACCDET_DEBUG("[Accdet]EINT func :plug-in---------------------\n");
	/* update the eint status */
    accdet_eint_set_hw_cust_debounce();
#endif
}

int accdet_PMIC_IMM_GetOneChannelValue(void)
{
	return PMIC_IMM_GetOneChannelValue(MULTIKEY_ADC_CHANNEL,1,1);
}

void set_cust_headset_settings_hal(void)
{
	cust_headset_settings_hal = get_cust_headset_settings();
}

void accdet_set_19_mode(void)
{
 	pmic_pwrap_write(ACCDET_RSV, ACCDET_1V9_MODE_OFF);
	//ACCDET_DEBUG("ACCDET use in 1.9V mode!! \n");
}

void accdet_set_28_mode(void)
{
	pmic_pwrap_write(ACCDET_RSV, ACCDET_2V8_MODE_OFF);
	//ACCDET_DEBUG("ACCDET use in 2.8V mode!! \n");
}

void accdet_auxadc_19_switch(int enbale)
{
   if(enbale) {
   		pmic_pwrap_write(ACCDET_RSV, ACCDET_1V9_MODE_ON);
   }else {
   		pmic_pwrap_write(ACCDET_RSV, ACCDET_1V9_MODE_OFF);
   }
}

void accdet_auxadc_28_switch(int enbale)
{
   	if(enbale) {
		pmic_pwrap_write(ACCDET_RSV, ACCDET_2V8_MODE_ON);
   	}else {
   		pmic_pwrap_write(ACCDET_RSV, ACCDET_2V8_MODE_OFF);
   	}
}

void accdet_auxadc_switch(int enable)
{
   if (enable) {
	#ifndef ACCDET_28V_MODE
	 accdet_auxadc_19_switch(1);
     ACCDET_DEBUG("ACCDET enable switch in 1.9v mode \n");
	#else
	 accdet_auxadc_28_switch(1);
	 ACCDET_DEBUG("ACCDET enable switch in 2.8v mode \n");
	#endif
   }else {
   	#ifndef ACCDET_28V_MODE
	 accdet_auxadc_19_switch(0);
     ACCDET_DEBUG("ACCDET diable switch in 1.9v mode \n");
	#else
	 accdet_auxadc_28_switch(0);
	 ACCDET_DEBUG("ACCDET diable switch in 2.8v mode \n");
	#endif
   }
   	
}

void accdet_disable_hal(void)
{
   pmic_pwrap_write(ACCDET_CTRL, ACCDET_DISABLE);
   pmic_pwrap_write(ACCDET_STATE_SWCTRL, 0);
}

void accdet_enable_RG(void)
{
   pmic_pwrap_write(ACCDET_CTRL, ACCDET_ENABLE);
}

void accdet_enable_hal(u32 state_swctrl)
{
	// enable ACCDET unit
   ACCDET_DEBUG("accdet: enable_accdet\n");
   //enable clock
   pmic_pwrap_write(TOP_CKPDN_CLR, RG_ACCDET_CLK_CLR); 
   
   pmic_pwrap_write(ACCDET_STATE_SWCTRL, pmic_pwrap_read(ACCDET_STATE_SWCTRL)|state_swctrl);
   pmic_pwrap_write(ACCDET_CTRL, ACCDET_ENABLE);
}

U32 accdet_get_enable_RG(void)
{
   return pmic_pwrap_read(ACCDET_CTRL);
}

void accdet_enable_clk(void)
{
   pmic_pwrap_write(TOP_CKPDN_CLR, RG_ACCDET_CLK_CLR); 
}

void accdet_disable_clk(void)
{
   pmic_pwrap_write(TOP_CKPDN_SET, RG_ACCDET_CLK_SET); 
}
void accdet_get_clk_log(void)
{
   ACCDET_DEBUG("[Accdet]accdet TOP_CKPDN=0x%x!\n", pmic_pwrap_read(TOP_CKPDN)); 
}
void accdet_RST_set(void)
{
   pmic_pwrap_write(TOP_RST_ACCDET_SET, ACCDET_RESET_SET); 
}
void accdet_RST_clr(void)
{
   pmic_pwrap_write(TOP_RST_ACCDET_CLR, ACCDET_RESET_CLR); 
}

void accdet_set_pwm_idle_on(void)
{
	pmic_pwrap_write(ACCDET_STATE_SWCTRL, (pmic_pwrap_read(ACCDET_STATE_SWCTRL)|ACCDET_SWCTRL_IDLE_EN));
	
}

void accdet_set_pwm_idle_off(void)
{
	pmic_pwrap_write(ACCDET_STATE_SWCTRL, pmic_pwrap_read(ACCDET_STATE_SWCTRL)&~ACCDET_SWCTRL_IDLE_EN);;
	
}

U32 accdet_get_swctrl(void)
{
	return pmic_pwrap_read(ACCDET_STATE_SWCTRL);	
}

U32 accdet_get_ctrl(void)
{
	return pmic_pwrap_read(ACCDET_CTRL);	
}

void accdet_set_pwm_enable(void)
{
	 pwrap_write(ACCDET_STATE_SWCTRL, 0x07);
}

void accdet_set_pwm_width(void)
{
	 //struct headset_mode_settings *cust_headset_settings_hal = get_cust_headset_settings();
	 pmic_pwrap_write(ACCDET_PWM_WIDTH, REGISTER_VALUE(cust_headset_settings_hal->pwm_width));
}

U32 accdet_get_pwm_width(void)
{
	 return pmic_pwrap_read(ACCDET_PWM_WIDTH);
}

void accdet_set_pwm_thresh(void)
{	 
	 //struct headset_mode_settings *cust_headset_settings_hal = get_cust_headset_settings();
	 pmic_pwrap_write(ACCDET_PWM_THRESH, REGISTER_VALUE(cust_headset_settings_hal->pwm_thresh));	 
}

U32 accdet_get_pwm_thresh(void)
{
	 return pmic_pwrap_read(ACCDET_PWM_THRESH);
}
void accdet_set_pwm_always_on(void)
{	 
	 pmic_pwrap_write(ACCDET_PWM_WIDTH, REGISTER_VALUE(cust_headset_settings_hal->pwm_width));
	 pmic_pwrap_write(ACCDET_PWM_THRESH, REGISTER_VALUE(cust_headset_settings_hal->pwm_width));	 
}

void accdet_set_pwm_delay(void)
{	 
	 pmic_pwrap_write(ACCDET_EN_DELAY_NUM,
		(cust_headset_settings_hal->fall_delay << 15 | cust_headset_settings_hal->rise_delay));	 
}

U32 accdet_get_pwm_delay(void)
{
	 return pmic_pwrap_read(ACCDET_EN_DELAY_NUM);
}

void accdet_set_debounce0(void)
{	 
	pmic_pwrap_write(ACCDET_DEBOUNCE0, cust_headset_settings_hal->debounce0);	 
}

void accdet_set_debounce0_reduce(void)
{	 
	pmic_pwrap_write(ACCDET_DEBOUNCE0, button_press_debounce);	 
}

U32 accdet_get_debounce0(void)
{
	 return pmic_pwrap_read(ACCDET_DEBOUNCE0);
}

void accdet_set_debounce1(void)
{	 
	pmic_pwrap_write(ACCDET_DEBOUNCE1, cust_headset_settings_hal->debounce1);	 
}

void accdet_set_debounce1_pin_recognition(void)
{	 
	pmic_pwrap_write(ACCDET_DEBOUNCE1, 0xFFFF);	 
}

U32 accdet_get_debounce1(void)
{
	 return pmic_pwrap_read(ACCDET_DEBOUNCE1);
}

void accdet_set_debounce2(void)
{	 
	//pmic_pwrap_write(ACCDET_DEBOUNCE2, cust_headset_settings_hal->debounce2);	 
}

U32 accdet_get_debounce2(void)
{
	 return pmic_pwrap_read(ACCDET_DEBOUNCE2);
}
void accdet_set_debounce3(void)
{	 
	pmic_pwrap_write(ACCDET_DEBOUNCE3, cust_headset_settings_hal->debounce3);	 
}

U32 accdet_get_debounce3(void)
{
	 return pmic_pwrap_read(ACCDET_DEBOUNCE3);
}

void accdet_set_irq(void)
{	 
	pmic_pwrap_write(ACCDET_IRQ_STS, (IRQ_CLR_BIT));	 
}
void accdet_disable_int(void)
{	 
	pmic_pwrap_write(INT_CON_ACCDET_CLR, RG_ACCDET_IRQ_CLR);	 
}
void accdet_enable_int(void)
{	 
	pmic_pwrap_write(INT_CON_ACCDET_SET, RG_ACCDET_IRQ_SET);	 
}
void accdet_int_log(void)
{	 
    ACCDET_DEBUG("[Accdet]accdet IRQ enable INT_CON_ACCDET=0x%x!\n", pmic_pwrap_read(INT_CON_ACCDET));	 
}

U32 accdet_get_irq(void)
{
	 return pmic_pwrap_read(ACCDET_IRQ_STS);
}

U32 accdet_get_cmp_vth_mbias_clk(void)
{	 
	return ((pmic_pwrap_read(ACCDET_STATE_RG) & 0x7000)>>12);	 
}

U32 accdet_get_AB(void)
{	 
	return ((pmic_pwrap_read(ACCDET_STATE_RG) & 0xc0)>>6);	 
}

U32 accdet_get_state(void)
{	 
	return ((pmic_pwrap_read(ACCDET_STATE_RG) & 0x0F00)>>8);	 
}

U32 accdet_get_irq_state(void)
{	 
	return (pmic_pwrap_read(ACCDET_IRQ_STS) & IRQ_STATUS_BIT);	 
}

void accdet_clear_irq_setbit(void)
{	 
	pmic_pwrap_write(ACCDET_IRQ_STS, (pmic_pwrap_read(ACCDET_IRQ_STS) & (~IRQ_CLR_BIT)));	 
}

struct headset_key_custom*  accdet_get_headset_key_custom_setting(void)
{	 
	return get_headset_key_custom_setting();	 
}

int dump_register(void)
{

   int i=0;
   for (i=0x077A; i<= 0x079A; i+=2)
   {
     ACCDET_DEBUG(" ACCDET_BASE + %x=%x\n",i,pmic_pwrap_read(ACCDET_BASE + i));
   }

   ACCDET_DEBUG(" TOP_RST_ACCDET =%x\n",pmic_pwrap_read(TOP_RST_ACCDET));// reset register in 6320
   ACCDET_DEBUG(" INT_CON_ACCDET =%x\n",pmic_pwrap_read(INT_CON_ACCDET));//INT register in 6320
   ACCDET_DEBUG(" TOP_CKPDN =%x\n",pmic_pwrap_read(TOP_CKPDN));// clock register in 6320
  #ifdef ACCDET_PIN_SWAP
   //ACCDET_DEBUG(" 0x00004000 =%x\n",pmic_pwrap_read(0x00004000));//VRF28 power for PIN swap feature
  #endif
   return 0;
}

#ifdef ACCDET_PIN_SWAP
void accdet_FSA8049_enable(void)
{
	mt_set_gpio_mode(GPIO_FSA8049_PIN, GPIO_FSA8049_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_FSA8049_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_FSA8049_PIN, GPIO_OUT_ONE);
}

void accdet_FSA8049_disable(void)
{
	mt_set_gpio_mode(GPIO_FSA8049_PIN, GPIO_FSA8049_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_FSA8049_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_FSA8049_PIN, GPIO_OUT_ZERO);
}

void accdet_enable_vrf28_power_on(void)
{
	pmic_pwrap_write(0x0400, pmic_pwrap_read(0x0400)|(1<<14)); 
}

void accdet_enable_vrf28_power_off(void)
{
	pmic_pwrap_write(0x0400, pmic_pwrap_read(0x0400)&~(1<<14));  
}
#endif

#ifdef FSA8049_V_POWER
void accdet_enable_power(void)
{
    hwPowerOn(FSA8049_V_POWER, VOL_2800, "ACCDET");
}
#endif

