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
#include <accdet_custom_def.h>
#include <accdet_custom.h>
#include <mach/reg_accdet.h>


/*******************************************************************************
costom API
*******************************************************************************/
extern struct headset_key_custom* get_headset_key_custom_setting(void);


/*******************************************************************************
HAL API
*******************************************************************************/
void accdet_eint_unmask(void);
void accdet_eint_mask(void);
void accdet_eint_set_hw_debounce(void);
void accdet_eint_set_hw_cust_debounce(void);
void accdet_eint_set_sens(void);
void accdet_eint_set(void);
void accdet_eint_registration(void);
void accdet_eint_polarity_reverse_plugout(void);
void accdet_eint_polarity_reverse_plugin(void);
int accdet_PMIC_IMM_GetOneChannelValue(void);
void set_cust_headset_settings_hal(void);
void accdet_set_19_mode(void);
void accdet_set_28_mode(void);
void accdet_auxadc_19_switch(int enbale);
void accdet_auxadc_28_switch(int enbale);
void accdet_auxadc_switch(int enable);
void accdet_disable_hal(void);
void accdet_enable_RG(void);
void accdet_enable_hal(u32 state_swctrl);
U32 accdet_get_enable_RG(void);
void accdet_enable_clk(void);
void accdet_disable_clk(void);
void accdet_get_clk_log(void);
void accdet_RST_set(void);
void accdet_RST_clr(void);
void accdet_set_pwm_idle_on(void);
void accdet_set_pwm_idle_off(void);
U32 accdet_get_swctrl(void);
U32 accdet_get_ctrl(void);
void accdet_set_pwm_enable(void);
void accdet_set_pwm_width(void);
U32 accdet_get_pwm_width(void);
void accdet_set_pwm_thresh(void);
U32 accdet_get_pwm_thresh(void);
void accdet_set_pwm_always_on(void);
void accdet_set_pwm_delay(void);
U32 accdet_get_pwm_delay(void);
void accdet_set_debounce0(void);
void accdet_set_debounce0_reduce(void);
U32 accdet_get_debounce0(void);
void accdet_set_debounce1(void);
void accdet_set_debounce1_pin_recognition(void);
U32 accdet_get_debounce1(void);
void accdet_set_debounce2(void);
U32 accdet_get_debounce2(void);
void accdet_set_debounce3(void);
U32 accdet_get_debounce3(void);
void accdet_set_irq(void);
void accdet_disable_int(void);
void accdet_enable_int(void);
void accdet_int_log(void);
U32 accdet_get_irq(void);
U32 accdet_get_cmp_vth_mbias_clk(void);
U32 accdet_get_AB(void);
U32 accdet_get_state(void);
U32 accdet_get_irq_state(void);
void accdet_clear_irq_setbit(void);
int dump_register(void);

void accdet_FSA8049_enable(void);
void accdet_FSA8049_disable(void);
void accdet_enable_vrf28_power_on(void);
void accdet_enable_vrf28_power_off(void);

void accdet_enable_power(void);
void accdet_irq_register(void);
void accdet_irq_free(void);
struct headset_key_custom*  accdet_get_headset_key_custom_setting(void);


