/******************************************************************************
 * mt_gpio_base.c - MTKLinux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     This file provid the other drivers GPIO relative functions
 *
 ******************************************************************************/

#include <mach/sync_write.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <mach/mt_gpio_base.h>
#include <cust_gpio_usage.h>

/******************************************************************************/
/*-------for special kpad pupd-----------*/
struct kpad_pupd {
	unsigned char 	pin;
	unsigned char	reg;
	unsigned char	bit;
};
static struct kpad_pupd kpad_pupd_spec[] = {
	{GPIO74,	0,	2},
	{GPIO75,	1,	2},
	{GPIO92,	0,	6},
	{GPIO93,	0,	10},
	{GPIO167,	1,	6},
	{GPIO168,	1,	10}
};
/*-------for msdc pupd-----------*/
struct msdc_pupd {
	unsigned char 	pin;
	unsigned char	reg;
	unsigned char	bit;
};
static struct msdc_pupd msdc_pupd_spec[3][6] = {
	{
	{GPIO124,	1,	2},
	{GPIO125,	0,	2},
	{GPIO126,	3,	2},
	{GPIO127,	3,	6},
	{GPIO128,	3,	10},
	{GPIO129,	3,	14}},
	{
	{GPIO114,	1,	2},
	{GPIO115,	0,	2},
	{GPIO116,	3,	2},
	{GPIO117,	3,	6},
	{GPIO118,	3,	10},
	{GPIO119,	3,	14}},
	{
	{GPIO99,	1,	2},
	{GPIO100,	0,	2},
	{GPIO101,	3,	2},
	{GPIO102,	3,	6},
	{GPIO103,	3,	10},
	{GPIO104,	3,	14}}
};
/*---------------------------------------*/

struct mt_ies_smt_set{
	unsigned char	index_start;
	unsigned char	index_end;
	unsigned char	reg_index;
	unsigned char	bit;
};
static struct mt_ies_smt_set mt_ies_smt_map[] = {
	{GPIO0,		GPIO6,	0,	2},
	{GPIO7,		GPIO10,	0,	3},
	{GPIO11,	GPIO16,	0,	12},
	{GPIO17,	GPIO20,	0,	13},
	{GPIO21,	GPIO28,	0,	0},
	{GPIO29,	GPIO38,	0,	1},
	{GPIO39,	GPIO46,	0,	2},
	{GPIO47,	GPIO49,	0,	4},
	{GPIO50,	GPIO52,	0,	5},
	{GPIO53,	GPIO56,	0,	6},
	{GPIO57,	GPIO65,	0,	7},
	{GPIO66,	GPIO71,	0,	8},
	{GPIO72,	GPIO73,	0,	9},
	{GPIO74,	GPIO75,	0,	10},
	{GPIO76,	GPIO79,	0,	11},
	{GPIO80,	GPIO83,	0,	14},
	{GPIO84,	GPIO85,	0,	15},
	{GPIO86,	GPIO87,	1,	0},
	{GPIO88,	GPIO89,	1,	1},
	{GPIO90,	GPIO91,	1,	2},
	{GPIO92,	GPIO93,	0,	10},
	{GPIO94,	GPIO98,	1,	2},
	{GPIO99,	GPIO104,1,	3},
	{GPIO105,	GPIO107,1,	4},
	{GPIO108,	GPIO111,1,	5},
	{GPIO112,	GPIO113,1,	6},
	{GPIO120,	GPIO123,1,	7},
	{GPIO167,	GPIO168,0,	10}
};

static GPIO_REGS *gpio_reg = (GPIO_REGS*)(GPIO_BASE);
/*---------------------------------------------------------------------------*/
int mt_set_gpio_dir_base(unsigned long pin, unsigned long dir)
{
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    if (dir == GPIO_DIR_IN)
        GPIO_SET_BITS((1L << bit), &reg->dir[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &reg->dir[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_dir_base(unsigned long pin)
{    
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    data = GPIO_RD32(&reg->dir[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_pull_enable_base(unsigned long pin, unsigned long enable)
{
    unsigned long pos;
    unsigned long bit;
	unsigned long i,j;
    GPIO_REGS *reg = gpio_reg;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			if (enable == GPIO_PULL_DISABLE){
				GPIO_SET_BITS((3L << (kpad_pupd_spec[i].bit-2)), &reg->kpad_ctrl[kpad_pupd_spec[i].reg].rst);
			} else {
				GPIO_SET_BITS((1L << (kpad_pupd_spec[i].bit-2)), &reg->kpad_ctrl[kpad_pupd_spec[i].reg].set);
			}
			return RSUCCESS;
		}
	}

	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO99)&&(pin <= GPIO104))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (enable == GPIO_PULL_DISABLE){
						if (i == 0) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 2)), &reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 1) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 2)), &reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 2) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 2)), &reg->msdc3_ctrl[msdc_pupd_spec[i][j].reg].rst);
						}
					}else{
						if (i == 0) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 2)), &reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].set);
						} else if (i == 1) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 2)), &reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].set);
						} else if (i == 2) {
							GPIO_SET_BITS((3L << (msdc_pupd_spec[i][j].bit - 2)), &reg->msdc3_ctrl[msdc_pupd_spec[i][j].reg].set);
						}
					}
					return RSUCCESS;
				}
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (enable == GPIO_PULL_DISABLE)
		GPIO_SET_BITS((1L << bit), &reg->pullen[pos].rst);
	else
		GPIO_SET_BITS((1L << bit), &reg->pullen[pos].set);
	}
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int mt_get_gpio_pull_enable_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data=0;
	unsigned long i,j;
    GPIO_REGS *reg = gpio_reg;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			data = GPIO_RD32(&reg->kpad_ctrl[kpad_pupd_spec[i].reg].val);
			return (((data & (3L << (kpad_pupd_spec[i].bit-2))) == 0)? 0: 1);
		}
	}

	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO99)&&(pin <= GPIO104))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (i == 0) {
						data = GPIO_RD32(&reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].val);
					} else if (i == 1) {
						data = GPIO_RD32(&reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].val);
					} else if (i == 2) {
						data = GPIO_RD32(&reg->msdc3_ctrl[msdc_pupd_spec[i][j].reg].val);
					}
				}
				return (((data & (3L << (msdc_pupd_spec[i][j].bit - 2))) == 0)? 0: 1);
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;
		data = GPIO_RD32(&reg->pullen[pos].val);
	}
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_ies_base(unsigned long pin, unsigned long enable)
{
	int i = 0;
	GPIO_REGS *reg = gpio_reg;
	
	for(; i < ARRAY_SIZE(mt_ies_smt_map); i++) {
		if(pin >= mt_ies_smt_map[i].index_start && pin <= mt_ies_smt_map[i].index_end) 
			break;
	}
	if (enable == GPIO_IES_DISABLE)
		GPIO_SET_BITS((1L << mt_ies_smt_map[i].bit), &reg->ies[mt_ies_smt_map[i].reg_index].rst);
	else
		GPIO_SET_BITS((1L << mt_ies_smt_map[i].bit), &reg->ies[mt_ies_smt_map[i].reg_index].set);

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_ies_base(unsigned long pin)
{
	int i = 0;
	unsigned long data;
	GPIO_REGS *reg = gpio_reg;
	
	for(; i < ARRAY_SIZE(mt_ies_smt_map); i++) {
		if(pin >= mt_ies_smt_map[i].index_start && pin <= mt_ies_smt_map[i].index_end) 
			break;
	}	
	data = GPIO_RD32(&reg->ies[mt_ies_smt_map[i].reg_index].val);
    return (((data & (1L << mt_ies_smt_map[i].bit)) != 0)? 1: 0);
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_pull_select_base(unsigned long pin, unsigned long select)
{
    unsigned long pos;
    unsigned long bit;
	unsigned long i,j;
    GPIO_REGS *reg = gpio_reg;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			if (select == GPIO_PULL_DOWN)
				GPIO_SET_BITS((1L << kpad_pupd_spec[i].bit), &reg->kpad_ctrl[kpad_pupd_spec[i].reg].set);
			else
				GPIO_SET_BITS((1L << kpad_pupd_spec[i].bit), &reg->kpad_ctrl[kpad_pupd_spec[i].reg].rst);
			return RSUCCESS;
		}
	}

	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO99)&&(pin <= GPIO104))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (select == GPIO_PULL_DOWN){
						if (i == 0) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].set);
						} else if (i == 1) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].set);
						} else if (i == 2) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &reg->msdc3_ctrl[msdc_pupd_spec[i][j].reg].set);
						}
					}else{
						if (i == 0) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 1) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].rst);
						} else if (i == 2) {
							GPIO_SET_BITS((1L << msdc_pupd_spec[i][j].bit), &reg->msdc3_ctrl[msdc_pupd_spec[i][j].reg].rst);
						}
					}
					return RSUCCESS;
				}
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;
	
	if (select == GPIO_PULL_DOWN)
		GPIO_SET_BITS((1L << bit), &reg->pullsel[pos].rst);
	else
		GPIO_SET_BITS((1L << bit), &reg->pullsel[pos].set);
	}
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_pull_select_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data=0;
	unsigned long i,j;
    GPIO_REGS *reg = gpio_reg;

	/*for special kpad pupd*/
	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin){
			data = GPIO_RD32(&reg->kpad_ctrl[kpad_pupd_spec[i].reg].val);
			return (((data & (1L << kpad_pupd_spec[i].bit)) != 0)? 0: 1);
		}
	}

	if (((pin >= GPIO114)&&(pin <= GPIO119))||((pin >= GPIO99)&&(pin <= GPIO104))||((pin >= GPIO124)&&(pin <= GPIO129))){
		/* msdc IO */
		for(i = 0; i < sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]); i++){
			for(j = 0; j < sizeof(msdc_pupd_spec[0])/sizeof(msdc_pupd_spec[0][0]); j++){
				if (pin == msdc_pupd_spec[i][j].pin){
					if (i == 0) {
						data = GPIO_RD32(&reg->msdc1_ctrl[msdc_pupd_spec[i][j].reg].val);
					} else if (i == 1) {
						data = GPIO_RD32(&reg->msdc2_ctrl[msdc_pupd_spec[i][j].reg].val);
					} else if (i == 2) {
						data = GPIO_RD32(&reg->msdc3_ctrl[msdc_pupd_spec[i][j].reg].val);
					}
				}
				return (((data & (1L << msdc_pupd_spec[i][j].bit)) != 0)? 0: 1);
			}
		}
	}else if((pin >= GPIO141)&&(pin <= GPIO166)){
		return GPIO_PULL_EN_UNSUPPORTED;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		data = GPIO_RD32(&reg->pullsel[pos].val);
	}
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_inversion_base(unsigned long pin, unsigned long enable)
{
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_inversion_base(unsigned long pin)
{
	return 0;//FIX-ME
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_out_base(unsigned long pin, unsigned long output)
{
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    if (output == GPIO_OUT_ZERO)
        GPIO_SET_BITS((1L << bit), &reg->dout[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &reg->dout[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_out_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    data = GPIO_RD32(&reg->dout[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_in_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    data = GPIO_RD32(&reg->din[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_mode_base(unsigned long pin, unsigned long mode)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    unsigned long mask = (1L << GPIO_MODE_BITS) - 1;    
    GPIO_REGS *reg = gpio_reg;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;
   
	data = GPIO_RD32(&reg->mode[pos].val);

	data &= ~(mask << (GPIO_MODE_BITS*bit));
	data |= (mode << (GPIO_MODE_BITS*bit));
	
	GPIO_WR32(&reg->mode[pos].val, data);

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_mode_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    unsigned long mask = (1L << GPIO_MODE_BITS) - 1;    
    GPIO_REGS *reg = gpio_reg;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	data = GPIO_RD32(&reg->mode[pos].val);
	
	return ((data >> (GPIO_MODE_BITS*bit)) & mask);
}
/*---------------------------------------------------------------------------*/
