/*
 * Copyright (C) 2011 MediaTek, Inc.
 *
 * Author: Holmes Chiou <holmes.chiou@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/sched_clock.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <board-custom.h>


#include "mach/mt_freqhopping.h"
#include "mach/mt_fhreg.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_gpio.h"
#include "mach/mt_gpufreq.h"
#include "mach/mt_cpufreq.h"
#include "mach/emi_bwl.h"
#include "mach/sync_write.h"
#include "mach/mt_sleep.h"

#include <mach/mt_freqhopping_drv.h>
#include <mach/mt_clkmgr.h>
//#define DUMMY_READ_ENGINE
#define APDMA_DUMMY_READ
//#define TEMP_SENSOR
#if defined(APDMA_DUMMY_READ)
#undef DUMMY_READ_ENGINE
unsigned int DMA_TIMES_RECORDER;
extern int DFS_APDMA_Enable(void);
extern int DFS_APDMA_Init(void);
extern int DFS_APDMA_early_init(void);
extern int DFS_APDMA_Init_2(void);
extern int DFS_APDMA_END(void);
#endif

#if defined(DUMMY_READ_ENGINE)
#include "mach/dfe_drv.h"
#endif


//#define FH_MSG printk //TODO
//#define FH_BUG_ON(x) printk("BUGON %s:%d %s:%d",__FUNCTION__,__LINE__,current->comm,current->pid)//TODO
//#define FH_BUG_ON(...) //TODO

#define MT_FH_CLK_GEN 		0

#define USER_DEFINE_SETTING_ID 	1

static DEFINE_SPINLOCK(freqhopping_lock);

extern int DFS_Detection(void);
//current DRAMC@mempll 
static unsigned int 	g_curr_dramc=266; //default @266MHz ==> LPDDR2/DDR3 data rate 1066

//mempll mode
static unsigned int 	g_pll_mode;

#define PERCENT_TO_DDSLMT(dDS, pERCENT_M10)  ((dDS * pERCENT_M10 >> 5) / 100)
#if MT_FH_CLK_GEN
static unsigned int 	g_curr_clkgen=MT658X_FH_PLL_TOTAL_NUM+1; //default clkgen ==> no clkgen output
#endif 

static unsigned char 	g_mempll_fh_table[8];

static unsigned int	g_initialize=0;

static unsigned int	g_clk_en=0;

#ifndef PER_PROJECT_FH_SETTING	

#if 0
#define LOW_DRAMC_DDS		0x000E55EC
#define LOW_DRAMC_INT		57 //200
#define LOW_DRAMC_FRACTION	5612 //200
#define LOW_DRAMC		200//233.5
#define LOW_DRAMC_FREQ		200000
#endif

// default pll freq. in MT6592
#define ARMPLL_DEF_FREQ          1040000
#define MAINPLL_DEF_FREQ         1092000
#define MEMPLL_DEF_FREQ          266000
#define MMPLL_DEF_FREQ           2002000
#define VENCPLL_DEF_FREQ         1183000
#define MSDCPLL_DEF_FREQ         1664000



//TODO: fill in the default freq & corresponding setting_id
static  fh_pll_t g_fh_pll[MT658X_FH_PLL_TOTAL_NUM] = { //keep track the status of each PLL 
	{FH_FH_DISABLE,		FH_PLL_ENABLE   , 0, ARMPLL_DEF_FREQ, 0},  //ARMPLL   default SSC disable
	{FH_FH_ENABLE_SSC,	FH_PLL_ENABLE   , 0, MAINPLL_DEF_FREQ, 0}, //MAINPLL  default SSC disable
	{FH_FH_ENABLE_SSC,	FH_PLL_ENABLE   , 0, MEMPLL_DEF_FREQ, 0},  //MEMPLL   default SSC enable
	{FH_FH_ENABLE_SSC,	FH_PLL_ENABLE   , 0, MSDCPLL_DEF_FREQ, 0},  //MSDCPLL  default SSC enable
	{FH_FH_DISABLE,		FH_PLL_ENABLE   , 0, MMPLL_DEF_FREQ, 0},  //MMPLL   default SSC enable
	{FH_FH_ENABLE_SSC,	FH_PLL_ENABLE   , 0, VENCPLL_DEF_FREQ, 0}    //VENCPLL  default SSC enable
};



//ARMPLL
#define ARMPLL_TARGETVCO_1			2002000
#define ARMPLL_TARGETVCO_2			1664000
#define ARMPLL_TARGETVCO_3			1495000
#define ARMPLL_TARGETVCO_4			1365000
#define ARMPLL_TARGETVCO_5			1248000
#define ARMPLL_TARGETVCO_6			1144000
#define ARMPLL_TARGETVCO_7			1690000
#define ARMPLL_TARGETVCO_8			1456000
#define ARMPLL_TARGETVCO_9			1716000
#define ARMPLL_TARGETVCO_10			1768000
#define ARMPLL_TARGETVCO_11			2054000
#define ARMPLL_TARGETVCO_12			2106000

#define ARMPLL_TARGETVCO_1_DDS		0x00134000
#define ARMPLL_TARGETVCO_2_DDS		0x00100000
#define ARMPLL_TARGETVCO_3_DDS		0x000E6000
#define ARMPLL_TARGETVCO_4_DDS		0x000D2000
#define ARMPLL_TARGETVCO_5_DDS		0x000C0000
#define ARMPLL_TARGETVCO_6_DDS		0x000B0000
#define ARMPLL_TARGETVCO_7_DDS		0x00104000
#define ARMPLL_TARGETVCO_8_DDS		0x000E0000
#define ARMPLL_TARGETVCO_9_DDS		0x00108000
#define ARMPLL_TARGETVCO_10_DDS		0x00110000
#define ARMPLL_TARGETVCO_11_DDS		0x0013C000
#define ARMPLL_TARGETVCO_12_DDS		0x00144000

static const struct freqhopping_ssc mt_ssc_armpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{0,0,0,0,0,0} //EOF
};

static const struct freqhopping_ssc mt_ssc_mainpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{MAINPLL_DEF_FREQ ,0 ,9 ,0, 8, 0xA8000},// 0~-8%
	{0,0,0,0,0,0} //EOF
};

#define MEMPLL333MHZ_DDS	0x0014363F	// the value in MT6592
#define MEMPLL266MHZ_DDS	0x001029D8	// the value in MT6592
static const struct freqhopping_ssc mt_ssc_mempll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{MEMPLL_DEF_FREQ ,2 ,7 ,0, 8, MEMPLL266MHZ_DDS},//266MHz , default 0~-8%
	{333000 ,2 ,7 ,0, 8, MEMPLL333MHZ_DDS},//333MHz , 
	{333266 ,2 ,7 ,8, 0, MEMPLL266MHZ_DDS},//DFS 333MHz -> 266MHz , DFS 8~0%
	{0,0,0,0,0,0} //EOF
};

static const struct freqhopping_ssc mt_ssc_msdcpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{MSDCPLL_DEF_FREQ, 0, 9, 0, 8, 0xF6275}, //0~-8%   range:0xF6275 ~ 0xE2795(0xF6275-(0x09d7 x 32))
	{0,0,0,0,0,0} //EOF

};
#define MMPLL_TARGETVCO_1			2782000
#define MMPLL_TARGETVCO_2			1796000
#define MMPLL_TARGETVCO_3			1014000
#define MMPLL_TARGETVCO_4			2405000
#define MMPLL_TARGETVCO_1_DDS		0x001AC000
#define MMPLL_TARGETVCO_2_DDS		0x00130000
#define MMPLL_TARGETVCO_3_DDS		0x0009C000
#define MMPLL_TARGETVCO_4_DDS		0x00172000

static const struct freqhopping_ssc mt_ssc_mmpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{MMPLL_DEF_FREQ ,0 ,9 ,0, 8, 0x134000}, //0~-8%   range:0x134000 ~ 0x11B5C0 (0x134000-(0x0c52 x 32))
	{0,0,0,0,0,0} //EOF
};


#define VENCPLL_TARGETVCO_1			1599000
#define VENCPLL_TARGETVCO_2			1183000
#define VENCPLL_TARGETVCO_1_DDS		0xF6000
#define VENCPLL_TARGETVCO_2_DDS		0xB6000
static const struct freqhopping_ssc mt_ssc_vencpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{VENCPLL_DEF_FREQ, 0, 9, 0, 4, 0xB6000}, //0~-4%  range:0xB6000 ~ 0xAEBA0 (0xB6000-(0x03a3 x 32))
	{0,0,0,0,0,0} //EOF
};

static struct freqhopping_ssc mt_ssc_fhpll_userdefined[MT_FHPLL_MAX]= {
	{0,1,1,2,2,0}, //ARMPLL
	{0,1,1,2,2,0}, //MAINPLL
	{0,1,1,2,2,0}, //MEMPLL
	{0,1,1,2,2,0}, //MSDCPLL
	{0,1,1,2,2,0}, //MMPLL
	{0,1,1,2,2,0}  //VENCPLL
};

#else //PER_PROJECT_FH_SETTING

PER_PROJECT_FH_SETTING

#endif	//PER_PROJECT_FH_SETTING

extern int get_ddr_type(void);
extern unsigned int mt_get_emi_freq(void);

#define PLL_STATUS_ENABLE 1
#define PLL_STATUS_DISABLE 0

/************************  Test function  ********************************/
/* The freq.meter is imlemented for measuring SSC and DFS function work or NOT */
/* Now this is only specific for MT6592, remember to change it in different platform */
/* Begin																  */
/*********************************************************************/
#if 0
static unsigned int mt_get_mempllclk_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_8, clk_misc_cfg_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0xFFFFFF00); // select divider

    clk_cfg_8 = DRV_Reg32(CLK_CFG_8);
    DRV_WriteReg32(CLK_CFG_8, (14 << 8)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x1); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x1)
    {
        //dbg_print("wait for frequency meter finish, CLK26CALI_0 = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //kal_sleep_task(100000);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;
	
    output = ((temp * 26000) / 1024); // Khz

    DRV_WriteReg32(CLK_CFG_8, clk_cfg_8);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    //dbg_print("CLK26CALI_1 = 0x%x, ddr frequency = %d Khz\n", temp, output);
	// output = (mem vco freq/4)
    return output;
}

static unsigned int mt_get_vencpllclk_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_9, clk_misc_cfg_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0x00FFFFFF); // select divider

    clk_cfg_9 = DRV_Reg32(CLK_CFG_9);
    DRV_WriteReg32(CLK_CFG_9, (5 << 16)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x10); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        //printk("wait for emi frequency meter finish, CLK26CALI = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //mdelay(10);
        //kal_sleep_task(100000);
    }

    temp = DRV_Reg32(CLK26CALI_2) & 0xFFFF;

    output = (temp * 26000) / 1024; // Khz

    DRV_WriteReg32(CLK_CFG_9, clk_cfg_9);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    return output;
}

static unsigned int mt_get_mmpllclk_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_9, clk_misc_cfg_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0x00FFFFFF); // select divider

    clk_cfg_9 = DRV_Reg32(CLK_CFG_9);
    DRV_WriteReg32(CLK_CFG_9, (8 << 16)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x10); // start fmeter

    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x10)
    {
        //kal_sleep_task(100000);
    }

    temp = DRV_Reg32(CLK26CALI_2) & 0xFFFF;

    output = (temp * 26000) / 1024; // Khz

    DRV_WriteReg32(CLK_CFG_9, clk_cfg_9);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    return output;
}

static unsigned int mt_get_armpllclk_freq(void)
{
    int output = 0;
    unsigned int temp, clk26cali_0, clk_cfg_8, clk_misc_cfg_1;

    clk26cali_0 = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0 | 0x80); // enable fmeter_en

    clk_misc_cfg_1 = DRV_Reg32(CLK_MISC_CFG_1);
    DRV_WriteReg32(CLK_MISC_CFG_1, 0xFFFF0300); // select divider

    clk_cfg_8 = DRV_Reg32(CLK_CFG_8);
    DRV_WriteReg32(CLK_CFG_8, (39 << 8)); // select abist_cksw

    temp = DRV_Reg32(CLK26CALI_0);
    DRV_WriteReg32(CLK26CALI_0, temp | 0x1); // start fmeter

    //dbg_print("ARMPLL_CON1 = 0x%x, TOP_CKMUXSEL = 0x%x, TOP_CKDIV1 = 0x%x\n", DRV_Reg32(ARMPLL_CON1), DRV_Reg32(TOP_CKMUXSEL), DRV_Reg32(TOP_CKDIV1));
	
    /* wait frequency meter finish */
    while (DRV_Reg32(CLK26CALI_0) & 0x1)
    {
        //dbg_print("wait for frequency meter finish, CLK26CALI_0 = 0x%x\n", DRV_Reg32(CLK26CALI_0));
        //kal_sleep_task(100000);
    }

    temp = DRV_Reg32(CLK26CALI_1) & 0xFFFF;
	
    output = ((temp * 26000) / 1024) * 4; // Khz

    DRV_WriteReg32(CLK_CFG_8, clk_cfg_8);
    DRV_WriteReg32(CLK_MISC_CFG_1, clk_misc_cfg_1);
    DRV_WriteReg32(CLK26CALI_0, clk26cali_0);

    //dbg_print("CLK26CALI_1 = 0x%x, cpu frequency = %d Khz\n", temp, output);

    return output;
}
#endif
/************************  Test function  ********************************/
/* The freq.meter is imlemented for measuring SSC and DFS function work or NOT */
/* Now this is only specific for MT6592, remember to change it in different platform */
/* End																  */
/*********************************************************************/

static void mt_fh_hal_default_conf(void)
{
	FH_MSG("EN: %s",__func__);

	// default enable pll SSC here. call from clk mgr
	freqhopping_config(MT658X_FH_MEM_PLL, MEMPLL_DEF_FREQ, true);
	freqhopping_config(MT658X_FH_MM_PLL, MMPLL_DEF_FREQ, true);
	freqhopping_config(MT658X_FH_VENC_PLL, VENCPLL_DEF_FREQ, true);
	freqhopping_config(MT658X_FH_MAIN_PLL, MAINPLL_DEF_FREQ, true);
	freqhopping_config(MT658X_FH_MSDC_PLL, MSDCPLL_DEF_FREQ, true);
}


static void mt_fh_hal_switch_register_to_FHCTL_control(enum FH_PLL_ID pll_id, int i_control)
{
	if (pll_id==MT658X_FH_ARM_PLL)  //FHCTL0
		{
			fh_set_field(PLL_HP_CON0, (0x1U<<0), i_control);
		}
	else if (pll_id==MT658X_FH_MAIN_PLL) //FHCTL1
		{
			fh_set_field(PLL_HP_CON0, (0x1U<<1), i_control);
		}
	else if (pll_id==MT658X_FH_MEM_PLL) //FHCTL2
		{
			fh_set_field(PLL_HP_CON0, (0x1U<<3), i_control);
		}
	else if (pll_id==MT658X_FH_MSDC_PLL) //FHCTL3
		{
			fh_set_field(PLL_HP_CON0, (0x1U<<4), i_control);
		}
	else if (pll_id==MT658X_FH_MM_PLL) //FHCTL4
		{
			fh_set_field(PLL_HP_CON0, (0x1U<<2), i_control);
		}
	else if (pll_id==MT658X_FH_VENC_PLL) //FHCTL5
		{
			fh_set_field(PLL_HP_CON0, (0x1U<<5), i_control);
		}
}
static int mt_fh_hal_sync_ncpo_to_FHCTL_DDS(enum FH_PLL_ID pll_id)
{
	if (pll_id==MT658X_FH_ARM_PLL)  //FHCTL0
		{
			fh_write32(REG_FHCTL0_DDS, (fh_read32(ARMPLL_CON1)&0x1FFFFF)|(1U<<31));
		}
	else if (pll_id==MT658X_FH_MAIN_PLL) //FHCTL1
		{
			fh_write32(REG_FHCTL1_DDS, (fh_read32(MAINPLL_CON1)&0x1FFFFF)|(1U<<31));
		}
	else if (pll_id==MT658X_FH_MEM_PLL) //FHCTL2
		{
			fh_write32(REG_FHCTL2_DDS, ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )&0x1FFFFF)|(1U<<31));
		}
	else if (pll_id==MT658X_FH_MSDC_PLL) //FHCTL3
		{
			fh_write32(REG_FHCTL3_DDS, (fh_read32(MSDCPLL_CON1)&0x1FFFFF)|(1U<<31));
		}
	else if (pll_id==MT658X_FH_MM_PLL) //FHCTL4
		{
			fh_write32(REG_FHCTL4_DDS, (fh_read32(MMPLL_CON1)&0x1FFFFF)|(1U<<31));
		}
	else if (pll_id==MT658X_FH_VENC_PLL) //FHCTL5
		{
			fh_write32(REG_FHCTL5_DDS, (fh_read32(VENCPLL_CON1)&0x1FFFFF)|(1U<<31));
		}
	else
		{
			FH_MSG("Incorrect PLL");
			return 0;
		}
	return 1;
}
static void update_fhctl_status(const int pll_id, const int enable)
{
        int i = 0 ;
        int enabled_num = 0 ;
        static unsigned int pll_status[] = {
                PLL_STATUS_DISABLE, //ARMPLL
                PLL_STATUS_DISABLE, //MAINPLL
                PLL_STATUS_DISABLE, //MEMPLL
                PLL_STATUS_DISABLE, //MSDCPLL
                PLL_STATUS_DISABLE, //TVDPLL
                PLL_STATUS_DISABLE  //LVDSPLL
        } ;

        //FH_MSG("PLL#%d ori status is %d, you hope to change to %d\n", pll_id, pll_status[pll_id], enable) ;
        //FH_MSG("PL%d:%d->%d", pll_id, pll_status[pll_id], enable) ;
        if(pll_status[pll_id] == enable) {
                //FH_MSG("no ch") ;//no change
                return ;
        }

        pll_status[pll_id] = enable ;

        for(i = MT658X_FH_MINIMUMM_PLL ; i <= MT658X_FH_MAXIMUMM_PLL ; i++) {

                if(pll_status[i] == PLL_STATUS_ENABLE) {
                        //FH_MSG("PLL#%d is enabled", i) ;
                        enabled_num++ ;
                }
                /*else {
                        FH_MSG("PLL#%d is disabled", i) ;
                }*/
        }

        //FH_MSG("PLen#=%d",enabled_num) ;

        if((g_clk_en == 0)&&(enabled_num >= 1)) {
                //wait_for_sophie enable_clock_ext_locked(MT_CG_PERI1_FHCTL, "FREQHOP") ;
	//register change.                 enable_clock(MT_CG_PERI1_FHCTL, "FREQHOP") ;
                g_clk_en = 1;                
        }
        else if((g_clk_en == 1)&&(enabled_num == 0)) {
                //wait_for_sophie disable_clock_ext_locked(MT_CG_PERI1_FHCTL, "FREQHOP") ;
	//register change.                 disable_clock(MT_CG_PERI1_FHCTL, "FREQHOP") ;
                g_clk_en = 0;
        }
}



static int __mt_enable_freqhopping(unsigned int pll_id,const struct freqhopping_ssc* ssc_setting)
{
	//unsigned int 	pll_hp=0;
	unsigned long 	flags=0;

	//FH_MSG("EN: en_fh");
	FH_MSG("EN: %s:: %x u: %x df: %d dt: %d dds:%x",__func__ ,ssc_setting->lowbnd
			   			      ,ssc_setting->upbnd
			   			      ,ssc_setting->df
			   			      ,ssc_setting->dt
			   			      ,ssc_setting->dds);
	
	update_fhctl_status(pll_id, PLL_STATUS_ENABLE) ;
        mb() ;


	//lock @ __freqhopping_ctrl_lock()
	//spin_lock_irqsave(&freqhopping_lock, flags);

	g_fh_pll[pll_id].fh_status = FH_FH_ENABLE_SSC;		


	//TODO: should we check the following here ??
	//if(unlikely(FH_PLL_STATUS_ENABLE == g_fh_pll[pll_id].fh_status)){
	//Do nothing due to this not allowable flow
	//We shall DISABLE and then re-ENABLE for the new setting or another round	
	//FH_MSG("ENABLE the same FH",pll_id);
	//WARN_ON(1);
	//spin_unlock_irqrestore(&freqhopping_lock, flags);
	//return 1;
	//}else {
	
	local_irq_save(flags);	

	//Set the relative parameter registers (dt/df/upbnd/downbnd)
	//Enable the fh bit
	fh_set_field(REG_FHCTL0_CFG+(0x14*pll_id),FH_SFSTRX_DYS,ssc_setting->df);
	fh_set_field(REG_FHCTL0_CFG+(0x14*pll_id),FH_SFSTRX_DTS,ssc_setting->dt);

	//fh_write32(REG_FHCTL0_UPDNLMT+(0x14*pll_id), (((ssc_setting->lowbnd) << 16) | (ssc_setting->upbnd)));
	//FH_MSG("REG_FHCTL%d_UPDNLMT: 0x%08x", pll_id, fh_read32(REG_FHCTL2_UPDNLMT));	

		
	//fh_write32(REG_FHCTL0_DDS+(0x14*pll_id), (ssc_setting->dds)|(1U<<31));
	//FH_MSG("FHCTL%d_DDS: 0x%08x", pll_id, (fh_read32(REG_FHCTL0_DDS+(pll_id*0x14))&0x1FFFFF));
	
	mt_fh_hal_sync_ncpo_to_FHCTL_DDS(pll_id);
	//FH_MSG("FHCTL%d_DDS: 0x%08x", pll_id, (fh_read32(REG_FHCTL0_DDS+(pll_id*0x14))&0x1FFFFF));
	
	fh_write32(REG_FHCTL0_UPDNLMT+(pll_id*0x14), (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL0_DDS+(pll_id*0x14))&0x1FFFFF),ssc_setting->lowbnd) << 16));
	//FH_MSG("REG_FHCTL%d_UPDNLMT: 0x%08x", pll_id, fh_read32(REG_FHCTL2_UPDNLMT));		
	//FH_MSG("ssc_setting->lowbnd: 0x%x    ssc_setting->upbnd: 0x%x", ssc_setting->lowbnd, ssc_setting->upbnd);

	mt_fh_hal_switch_register_to_FHCTL_control(pll_id, 1);
	//pll_hp = fh_read32(PLL_HP_CON0) | (1 << pll_id);
	//fh_write32( PLL_HP_CON0,  pll_hp );

	mb();
	
	fh_set_field(REG_FHCTL0_CFG+(0x14*pll_id),FH_FRDDSX_EN,1);
	fh_set_field(REG_FHCTL0_CFG+(0x14*pll_id),FH_FHCTLX_EN,1);
	//lock @ __freqhopping_ctrl_lock()
	//spin_unlock_irqrestore(&freqhopping_lock, flags);

	local_irq_restore(flags);

	//FH_MSG("Exit");
	return 0;
}

static int __mt_disable_freqhopping(unsigned int pll_id,const struct freqhopping_ssc* ssc_setting)
{
	unsigned long 	flags=0;
//	unsigned int 	pll_hp=0;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;


	//FH_MSG("EN: _dis_fh");
	
	//lock @ __freqhopping_ctrl_lock()
	//spin_lock_irqsave(&freqhopping_lock, flags);

	local_irq_save(flags);
	//Set the relative registers	
	fh_set_field(REG_FHCTL0_CFG+(0x14*pll_id),FH_FRDDSX_EN,0);
	fh_set_field(REG_FHCTL0_CFG+(0x14*pll_id),FH_FHCTLX_EN,0);
	mb();
	local_irq_restore(flags);


	if(pll_id == MT658X_FH_MEM_PLL){ //for mempll
		unsigned int i=0;
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){

			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}
	else{
		//TODO:  sync&wait DDS back to PLLCON1@other PLLs
		//FH_MSG("n-mempll");
	}
	
		
	local_irq_save(flags);
	mt_fh_hal_switch_register_to_FHCTL_control(pll_id, 0);
	//pll_hp = fh_read32(PLL_HP_CON0) & ~(1 << pll_id);
	//fh_write32( (PLL_HP_CON0),  pll_hp );
	
	g_fh_pll[pll_id].fh_status = FH_FH_DISABLE;
	local_irq_restore(flags);
	
	//lock @ __freqhopping_ctrl_lock()
	//spin_unlock_irqrestore(&freqhopping_lock, flags);

	mb() ;
        update_fhctl_status(pll_id, PLL_STATUS_DISABLE) ;
	

	//FH_MSG("Exit");

	return 0;
}


//freq is in KHz, return at which number of entry in mt_ssc_xxx_setting[]
static noinline int __freq_to_index(enum FH_PLL_ID pll_id,int freq) 
{
	unsigned int retVal = 0;
	unsigned int i=2; //0 is disable, 1 is user defines, so start from 2
	
	//FH_MSG("EN: %s , pll_id: %d, freq: %d",__func__,pll_id,freq);
	//FH_MSG("EN: , id: %d, f: %d",pll_id,freq);
	
	switch(pll_id) {

		//TODO: use Marco or something to make the code less redudant
		case MT658X_FH_ARM_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_armpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		
		case MT658X_FH_MAIN_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_mainpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;
		
		case MT658X_FH_MEM_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_mempll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		case MT658X_FH_MSDC_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_msdcpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		case MT658X_FH_MM_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_mmpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		case MT658X_FH_VENC_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_vencpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;
		
		case MT658X_FH_PLL_TOTAL_NUM:
		FH_MSG("Error MT658X_FH_PLL_TOTAL_NUM!");			
		break;
		

	};

	return retVal;	
}

static int __freqhopping_ctrl(struct freqhopping_ioctl* fh_ctl,bool enable)
{
	const struct freqhopping_ssc* 	pSSC_setting=NULL;
	unsigned int  			ssc_setting_id=0;
	int				retVal=1;
	
	//FH_MSG("EN: _fh_ctrl %d:%d",fh_ctl->pll_id,enable);
	//FH_MSG("%s fhpll_id: %d, enable: %d",(enable)?"enable":"disable",fh_ctl->pll_id,enable);

	//Check the out of range of frequency hopping PLL ID
	FH_BUG_ON(fh_ctl->pll_id>MT658X_FH_MAXIMUMM_PLL);
	FH_BUG_ON(fh_ctl->pll_id<MT658X_FH_MINIMUMM_PLL);	
	
	if (fh_ctl->pll_id==MT658X_FH_MAIN_PLL)
	{
		g_fh_pll[fh_ctl->pll_id].curr_freq = MAINPLL_DEF_FREQ;
	}
	else if (fh_ctl->pll_id==MT658X_FH_MEM_PLL)
	{
		g_fh_pll[fh_ctl->pll_id].curr_freq = MEMPLL_DEF_FREQ;
	}
	else if (fh_ctl->pll_id==MT658X_FH_MSDC_PLL)
	{
		g_fh_pll[fh_ctl->pll_id].curr_freq = MSDCPLL_DEF_FREQ;
	}
	else if (fh_ctl->pll_id==MT658X_FH_MM_PLL)
	{
		g_fh_pll[fh_ctl->pll_id].curr_freq = MMPLL_DEF_FREQ;
	}
	else if (fh_ctl->pll_id==MT658X_FH_VENC_PLL)
	{
		g_fh_pll[fh_ctl->pll_id].curr_freq = VENCPLL_DEF_FREQ;
	}
	else if (fh_ctl->pll_id==MT658X_FH_ARM_PLL)
	{
		g_fh_pll[fh_ctl->pll_id].curr_freq = 0;
	}
	else
	{
		FH_BUG_ON("Incorrect pll_id.");
	}

	if((enable == true) && (g_fh_pll[fh_ctl->pll_id].fh_status == FH_FH_ENABLE_SSC)  ){ 
		
		//The FH already enabled @ this PLL
	
		//FH_MSG("re-en FH");
		
		//disable FH first, will be enable later
		__mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting);
	}
	else if((enable == false) && (g_fh_pll[fh_ctl->pll_id].fh_status == FH_FH_DISABLE)){
		
		//The FH already been disabled @ this PLL, do nothing & return
		
		//FH_MSG("re-dis FH");	
		retVal = 0;
		goto Exit;		
	}
		
	
	//ccyeh fh_status set @ __mt_enable_freqhopping() __mt_disable_freqhopping()
	//g_fh_pll[fh_ctl->pll_id].fh_status = enable?FH_FH_ENABLE_SSC:FH_FH_DISABLE;
	
	if( enable == true) { //enable freq. hopping @ fh_ctl->pll_id

		if(g_fh_pll[fh_ctl->pll_id].pll_status == FH_PLL_DISABLE) {
			//FH_MSG("pll is dis");
			
			//update the fh_status & don't really enable the SSC
			g_fh_pll[fh_ctl->pll_id].fh_status = FH_FH_ENABLE_SSC;
			retVal = 0;
			goto Exit;
		} 
		else {
			//FH_MSG("pll is en");
			if(g_fh_pll[fh_ctl->pll_id].user_defined == true){
				FH_MSG("use u-def");

				pSSC_setting = &mt_ssc_fhpll_userdefined[fh_ctl->pll_id];
				g_fh_pll[fh_ctl->pll_id].setting_id = USER_DEFINE_SETTING_ID; 
			} 
			else {
				//FH_MSG("n-user def");
				
				if( g_fh_pll[fh_ctl->pll_id].curr_freq != 0 ){
					ssc_setting_id = g_fh_pll[fh_ctl->pll_id].setting_id = __freq_to_index(fh_ctl->pll_id, g_fh_pll[fh_ctl->pll_id].curr_freq);
				}
				else{
					ssc_setting_id = 0;
				}
					
				
				//FH_MSG("sid %d",ssc_setting_id);
				if(ssc_setting_id == 0){
					FH_MSG("!!! No corresponding setting found !!!");
					
					//just disable FH & exit
					__mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting);
					goto Exit;
				}
				
				switch(fh_ctl->pll_id) {
					case MT658X_FH_MAIN_PLL:
						FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_mainpll_setting)/sizeof(struct freqhopping_ssc)) );
						pSSC_setting = &mt_ssc_mainpll_setting[ssc_setting_id];
						break;
					case MT658X_FH_ARM_PLL:
						FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_armpll_setting)/sizeof(struct freqhopping_ssc)));
						pSSC_setting = &mt_ssc_armpll_setting[ssc_setting_id];
						break;
					case MT658X_FH_MSDC_PLL:
						FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_msdcpll_setting)/sizeof(struct freqhopping_ssc)));
						pSSC_setting = &mt_ssc_msdcpll_setting[ssc_setting_id];
						break;
					case MT658X_FH_MM_PLL:
						FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_mmpll_setting)/sizeof(struct freqhopping_ssc)));
						pSSC_setting = &mt_ssc_mmpll_setting[ssc_setting_id];
						break;
					case MT658X_FH_VENC_PLL:
						FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_vencpll_setting)/sizeof(struct freqhopping_ssc)));
						pSSC_setting = &mt_ssc_vencpll_setting[ssc_setting_id];
						break;
					case MT658X_FH_MEM_PLL:
						FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_mempll_setting)/sizeof(struct freqhopping_ssc)));
						pSSC_setting = &mt_ssc_mempll_setting[ssc_setting_id];
						break;
				}
			}//user defined

			if(pSSC_setting == NULL){
				FH_MSG("!!! pSSC_setting is NULL !!!");
				//just disable FH & exit
				__mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting);
				goto Exit;
			}
			
			if( 0 == __mt_enable_freqhopping(fh_ctl->pll_id,pSSC_setting)) {
				retVal = 0;
				FH_MSG("en ok");
			}
			else{
				FH_MSG("__mt_enable_freqhopping fail.");
			}
		}
		
	}
	else{ //disable req. hopping @ fh_ctl->pll_id
		if( 0 == __mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting)) {
			retVal = 0;
			FH_MSG("dis ok");
		}
		else{
			FH_MSG("__mt_disable_freqhopping fail.");
		}
	}

Exit:
			
	//FH_MSG("Exit");
	return retVal;
}

#if 0	// NOT used in mt6592
//mempll 266->293MHz using FHCTL
static int mt_h2oc_mempll(void)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;

	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);

	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */
#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
#endif
	if(g_curr_dramc != 266){
		FH_MSG("g_curr_dramc != 266)");
		return -1;	
	}

	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2d){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2d);
		FH_MSG("DDS != 0x131a2d");
		return -1;
	}
	
	//TODO: provelock issue spin_lock(&freqhopping_lock); 
	spin_lock_irqsave(&freqhopping_lock, flags);

	//disable SSC when OC
	__mt_disable_freqhopping(2, NULL);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}

	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//Target Freq. : NCPO INT : 84.165 (26MHz*84.165 = 2188.29 MHz, DRAMC = 293MHz)
	//INT : 84 => 0x54 << 14
	//FRACTION : 0.165 => 0x0A8F
	fh_write32(REG_FHSRAM_WR, ((0x54 << 14) | 0x0A8F) );
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

	mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 1);
        //fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
       
       	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = 293;

	//TODO: provelock issue local_irq_restore(flags);

	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((0x54 << 14) | 0x0A8F ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);

	//TODO: provelock issue local_irq_restore(flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
			mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 0);
        	//fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
        }

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= 293000;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);

	freqhopping_config(MT658X_FH_MEM_PLL, 293000, true); //update freq.

	return 0;
}

//mempll 293->266MHz using FHCTL
static int mt_oc2h_mempll(void)  
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
	
	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);
	
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */
#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
#endif	
	if(g_curr_dramc != 293){
		FH_MSG("g_curr_dramc != 293)");
		return -1;	
	}
	
	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));
	
	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x150A8F){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x150A8F);
		FH_MSG("DDS != 0x150A8F");
		return 0;			
	}

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}


	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	
	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//Target Freq. : NCPO INT : 76.409 (26MHz*76.409 = 1986.63MHz, DRAMC = 266MHz)
	//INT : 76 => 0x4C << 14
	//FRACTION : 0.409 => 0x1A2D
	fh_write32(REG_FHSRAM_WR, ((0x4C << 14) | 0x1A2D) );
	
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 1);
        //fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
	
	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = 266;

	//TODO: provelock issue local_irq_restore(flags);
	
	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((0x4C << 14) | 0x1A2D ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);
	//TODO: provelock issue local_irq_restore(flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
			mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 0);
        	//fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
	}

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));
	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= 266000;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);

	freqhopping_config(MT658X_FH_MEM_PLL, 266000, true); //update freq.
	
	return 0;
}

//mempll 200->266MHz using FHCTL
static int mt_fh_hal_l2h_mempll(void)  //mempll low to high (200->266MHz)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
	
	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);
	
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */
#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
#endif	
	if(g_curr_dramc == 266){
		return -1;	
	}
	
	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));
	
	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != LOW_DRAMC_DDS){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != LOW_DRAMC_DDS);
		FH_MSG("DDS != 0x%X",LOW_DRAMC_DDS);
		return 0;			
	}

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}


	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	
	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//Target Freq. : NCPO INT : 76.409 (26MHz*76.409 = 1986.63MHz, DRAMC = 266MHz)
	//INT : 76 => 0x4C << 14
	//FRACTION : 0.409 => 0x1A2D
	fh_write32(REG_FHSRAM_WR, ((0x4C << 14) | 0x1A2D) );
	
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 1);
        //fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
	
	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = 266;

	//TODO: provelock issue local_irq_restore(flags);
	
	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((0x4C << 14) | 0x1A2D ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);
	//TODO: provelock issue local_irq_restore(flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
			mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 0);
        	//fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
	}

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));
	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= 266000;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);

	freqhopping_config(MT658X_FH_MEM_PLL, 266000, true); //update freq.
	
	return 0;
}

//mempll 266->200MHz using FHCTL
static int mt_fh_hal_h2l_mempll(void)  //mempll low to high (200->266MHz)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
	
	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);
	
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */
#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
#endif
	if(g_curr_dramc == LOW_DRAMC){
		return -1;	
	}

	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2d){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2d);
		FH_MSG("DDS != 0x131a2d");
		return 0;			
	}
	
	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}

	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//>>>Example<<<
	//Target Freq. : NCPO INT : 59.89 (26MHz*59.89 = 1557.14 MHz, DRAMC = 208.5MHz)
	//INT : 59 => 0x3B << 14
	//FRACTION : 0.89 => 0x38F5
	fh_write32(REG_FHSRAM_WR, LOW_DRAMC_DDS);
	
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 1);
        //fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
        
       	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = LOW_DRAMC;

	//TODO: provelock issue local_irq_restore(flags);

	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((LOW_DRAMC_INT << 14) | LOW_DRAMC_FRACTION ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);

	//TODO: provelock issue local_irq_restore(flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
			mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL, 0);
        	//fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
        }

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= LOW_DRAMC_FREQ;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);

	freqhopping_config(MT658X_FH_MEM_PLL, LOW_DRAMC_FREQ, true); //update freq.

	return 0;
}
#endif

static int mt_fh_hal_dvfs(enum FH_PLL_ID pll_id, unsigned int dds_value)
{
	unsigned int	fh_dds=0;
	unsigned int	i=0;
	unsigned long 	flags=0;
	unsigned int    ilog =0;
#if defined(APDMA_DUMMY_READ)
    int apdma_result=0, hoppingWaitMax = 100;
#endif	
	
	FH_MSG("EN: %s:",__func__);

	local_irq_save(flags);
	
	//1. sync ncpo to DDS of FHCTL
	if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(pll_id)))
		return 0;
	if(ilog)
	{
		FH_MSG("1. sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL%d_DDS: 0x%08x", pll_id, (fh_read32(REG_FHCTL0_DDS+(pll_id*0x14))&0x1FFFFF));
	}
	
	//2. enable DVFS and Hopping control
	fh_set_field((REG_FHCTL0_CFG+(pll_id*0x14)), FH_SFSTRX_EN, 1);  //enable dvfs mode
    fh_set_field((REG_FHCTL0_CFG+(pll_id*0x14)), FH_FHCTLX_EN, 1);  //enable hopping control
    
#if 1    	//for slope setting.
    if(pll_id == MT658X_FH_MEM_PLL)
	fh_write32(REG_FHCTL_CON,0x6000263);
    else
	fh_write32(REG_FHCTL_CON,0x6003c97);
#endif    

	if(ilog)
	{
	    FH_MSG("2. enable DVFS and Hopping control");
	}
	//3. switch to hopping control
	mt_fh_hal_switch_register_to_FHCTL_control(pll_id,1);
	mb();
	
	if(ilog)
	{
		FH_MSG("3. switch to hopping control");
		FH_MSG("PLL_HP_CON0: 0x%08x",(fh_read32(PLL_HP_CON0)&0x3F));
	}	

#if defined(APDMA_DUMMY_READ)
	// enable DMA before setting target DDS
	if(pll_id == MT658X_FH_MEM_PLL)
	{		
                                hoppingWaitMax = 1000; 
                                //initial
				apdma_result = DFS_APDMA_Enable();
				if (apdma_result == 0)
				{
					FH_BUG_ON("dummy read engine failure");
                                        ASSERT(0);
				}	
	}
#endif

	//4. set DFS DDS
	fh_write32((REG_FHCTL0_DVFS+(pll_id*0x14)), (dds_value)|(1U<<31));  //set dds
	
	if(ilog)
	{
		FH_MSG("4. set DFS DDS");
		FH_MSG("FHCTL%d_DDS: 0x%08x", pll_id, (fh_read32(REG_FHCTL0_DDS+(pll_id*0x14))&0x1FFFFF));
		FH_MSG("FHCTL%d_DVFS: 0x%08x", pll_id, (fh_read32(REG_FHCTL0_DVFS+(pll_id*0x14))&0x1FFFFF));
	}
	//4.1 ensure jump to target DDS
	fh_dds	=  (DRV_Reg32(REG_FHCTL0_MON+(pll_id*0x14))) & 0x1FFFFF;
	while((dds_value != fh_dds) && ( i < hoppingWaitMax)){
		
#if defined(APDMA_DUMMY_READ)
	if(pll_id == MT658X_FH_MEM_PLL)
	{		
				apdma_result = DFS_APDMA_Enable();
				if (apdma_result == 0)
				{
					FH_BUG_ON("dummy read engine failure");
				}	
	}else {
				udelay(10);
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
	}
        }
#else
	                        udelay(10);	
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
#endif				
					
				fh_dds	=  (DRV_Reg32(REG_FHCTL0_MON+(pll_id*0x14))) & 0x1FFFFF;
				i++;
			}
	
        //Disable APDMA Clock for saving power
        //disable_clock(MT_CG_PERI_AP_DMA,"APDMA_MODULE");

#if defined(APDMA_DUMMY_READ)
        if(pll_id == MT658X_FH_MEM_PLL)
        {
             
                DFS_APDMA_END();
        }
        DMA_TIMES_RECORDER=i;
#endif
	
	if(ilog)
	{
		FH_MSG("4.1 ensure jump to target DDS");
		FH_MSG("i: %d",i);
	}			

	//5. write back to ncpo
	
	if(ilog)
	{
		FH_MSG("5. write back to ncpo");
	}
	if (pll_id==MT658X_FH_ARM_PLL)  //FHCTL0
		{
			fh_write32(ARMPLL_CON1, (fh_read32(REG_FHCTL0_DVFS)&0x1FFFFF)|(fh_read32(ARMPLL_CON1)&0xFFE00000)|(1U<<31));
			
			if(ilog)
			{
				FH_MSG("ARMPLL_CON1: 0x%08x",(fh_read32(ARMPLL_CON1)&0x1FFFFF));
			}
		}
	else if (pll_id==MT658X_FH_MAIN_PLL) //FHCTL1
		{
			fh_write32(MAINPLL_CON1, (fh_read32(REG_FHCTL1_DVFS)&0x1FFFFF)|(fh_read32(MAINPLL_CON1)&0xFFE00000)|(1U<<31));
			
			if(ilog)
			{
				FH_MSG("MAINPLL_CON1: 0x%08x",(fh_read32(MAINPLL_CON1)&0x1FFFFF));
			}
		}
	else if (pll_id==MT658X_FH_MEM_PLL) //FHCTL2
		{
			fh_write32((DDRPHY_BASE+0x624), ((fh_read32(REG_FHCTL2_DVFS)&0x1FFFFF)<<11)|((DRV_Reg32(DDRPHY_BASE+0x624))&0x7FF));
			
			if(ilog)
			{
				FH_MSG("(DDRPHY_BASE+0x624): 0x%08x   >>11: 0x%08x",(fh_read32(DDRPHY_BASE+0x624)),(fh_read32(DDRPHY_BASE+0x624)>>11));
			}
			if ((fh_read32(DDRPHY_BASE+0x624))&0x1)
				fh_write32((DDRPHY_BASE+0x624),((fh_read32(DDRPHY_BASE+0x624))&0xFFFFFFFE));
			else
				fh_write32((DDRPHY_BASE+0x624),((fh_read32(DDRPHY_BASE+0x624))|0x1));
			
			if(ilog)
			{
				FH_MSG("(DDRPHY_BASE+0x624): 0x%08x   >>11: 0x%08x",(fh_read32(DDRPHY_BASE+0x624)),(fh_read32(DDRPHY_BASE+0x624)>>11));	
			}
		}
	else if (pll_id==MT658X_FH_MSDC_PLL) //FHCTL3
		{
			fh_write32(MSDCPLL_CON1, (fh_read32(REG_FHCTL3_DVFS)&0x1FFFFF)|(fh_read32(MSDCPLL_CON1)&0xFFE00000)|(1U<<31));
			
			if(ilog)
			{
				FH_MSG("MSDCPLL_CON1: 0x%08x",(fh_read32(MSDCPLL_CON1)&0x1FFFFF));
			}
		}
	else if (pll_id==MT658X_FH_MM_PLL) //FHCTL4
		{
			fh_write32(MMPLL_CON1, (fh_read32(REG_FHCTL4_DVFS)&0x1FFFFF)|(fh_read32(MMPLL_CON1)&0xFFE00000)|(1U<<31));
			
			if(ilog)
			{
				FH_MSG("MMPLL_CON1: 0x%08x",(fh_read32(MMPLL_CON1)&0x1FFFFF));
			}
		}
	else if (pll_id==MT658X_FH_VENC_PLL) //FHCTL5
		{
			fh_write32(VENCPLL_CON1, (fh_read32(REG_FHCTL5_DVFS)&0x1FFFFF)|(fh_read32(VENCPLL_CON1)&0xFFE00000)|(1U<<31));
			
			if(ilog)
			{
				FH_MSG("VENCPLL_CON1: 0x%08x",(fh_read32(VENCPLL_CON1)&0x1FFFFF));
			}
		}
	

	//6. switch to register control
	mt_fh_hal_switch_register_to_FHCTL_control(pll_id,0);
	mb();
	
	if(ilog)
	{
		FH_MSG("6. switch to register control");
		FH_MSG("PLL_HP_CON0: 0x%08x",(fh_read32(PLL_HP_CON0)&0x3F));
	}
	
	local_irq_restore(flags);
	return 0;
}
static int mt_fh_hal_dfs_armpll(unsigned int current_freq, unsigned int target_freq)  //armpll dfs mdoe
{
	unsigned long 	flags;
	unsigned int	target_dds=0;
	
	FH_MSG("EN: %s:",__func__);
	FH_MSG("current freq:%d target freq:%d", current_freq, target_freq);
	FH_MSG("current dds(ARMPLL_CON1): 0x%x",(fh_read32(ARMPLL_CON1)&0x1FFFFF));

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	fh_set_field(REG_FHCTL0_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
	fh_set_field(REG_FHCTL0_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
       fh_set_field(REG_FHCTL0_CFG, FH_FHCTLX_EN, 0);  //disable hopping control 		

	//target_dds = (((target_freq/100) * (((fh_read32(ARMPLL_CON1)&0x1FFFFF)*1000)/(current_freq/100)))/1000);
	switch (target_freq)
	{
		case ARMPLL_TARGETVCO_1:
			target_dds = ARMPLL_TARGETVCO_1_DDS;
			break;
		case ARMPLL_TARGETVCO_2:
			target_dds = ARMPLL_TARGETVCO_2_DDS;
			break;
		case ARMPLL_TARGETVCO_3:
			target_dds = ARMPLL_TARGETVCO_3_DDS;
			break;
		case ARMPLL_TARGETVCO_4:
			target_dds = ARMPLL_TARGETVCO_4_DDS;
			break;
		case ARMPLL_TARGETVCO_5:
			target_dds = ARMPLL_TARGETVCO_5_DDS;
			break;
		case ARMPLL_TARGETVCO_6:
			target_dds = ARMPLL_TARGETVCO_6_DDS;
			break;
		case ARMPLL_TARGETVCO_7:
			target_dds = ARMPLL_TARGETVCO_7_DDS;
			break;
		case ARMPLL_TARGETVCO_8:
			target_dds = ARMPLL_TARGETVCO_8_DDS;
			break;
		case ARMPLL_TARGETVCO_9:
			target_dds = ARMPLL_TARGETVCO_9_DDS;
			break;
		case ARMPLL_TARGETVCO_10:
			target_dds = ARMPLL_TARGETVCO_10_DDS;
			break;
		case ARMPLL_TARGETVCO_11:
			target_dds = ARMPLL_TARGETVCO_11_DDS;
			break;
		case ARMPLL_TARGETVCO_12:
			target_dds = ARMPLL_TARGETVCO_12_DDS;
			break;				
		default:
			FH_BUG_ON("Incorrect ARMPLL_TARGETVCO.");
			return -1;
			break;			
	}
	FH_MSG("target dds: 0x%x",target_dds);
	
	mt_fh_hal_dvfs(MT658X_FH_ARM_PLL, target_dds); 

	fh_set_field(REG_FHCTL0_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
	fh_set_field(REG_FHCTL0_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
       fh_set_field(REG_FHCTL0_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	return 0;
}

static int mt_fh_hal_dfs_mmpll(unsigned int target_freq)  //mmpll dfs mode
{
	unsigned long 	flags;
	unsigned int	target_dds=0;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;

	FH_MSG("EN: %s:",__func__);
	//FH_MSG("current freq:%d target freq:%d", current_freq, target_freq);
	FH_MSG("current dds(MMPLL_CON1): 0x%x",(fh_read32(MMPLL_CON1)&0x1FFFFF));

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	if(g_fh_pll[MT658X_FH_MM_PLL].fh_status == FH_FH_ENABLE_SSC)
	{//only when SSC is enable	
			//Turn off MEMPLL hopping
			fh_set_field(REG_FHCTL4_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL4_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    			fh_set_field(REG_FHCTL4_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
		
			//udelay(800);
			
			pll_dds =  (DRV_Reg32(REG_FHCTL4_DDS)) & 0x1FFFFF;
			fh_dds	=  (DRV_Reg32(REG_FHCTL4_MON)) & 0x1FFFFF;
			
			FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
			
			while((pll_dds != fh_dds) && ( i < 100)){
		
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
					
				udelay(10);
				fh_dds	=  (DRV_Reg32(REG_FHCTL4_MON)) & 0x1FFFFF;
				i++;
			}	
						
			FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);

	}
	if(target_freq == MMPLL_TARGETVCO_1)
		target_dds = MMPLL_TARGETVCO_1_DDS;
	else if (target_freq == MMPLL_TARGETVCO_2)
		target_dds = MMPLL_TARGETVCO_2_DDS;
	else if (target_freq == MMPLL_TARGETVCO_3)
		target_dds = MMPLL_TARGETVCO_3_DDS;
	else if (target_freq == MMPLL_TARGETVCO_4)
		target_dds = MMPLL_TARGETVCO_4_DDS;
	else
        {
		FH_BUG_ON("Incorrect MMPLL_TARGETVCO.");
                return -1;
        } 
	
	FH_MSG("target dds: 0x%x",target_dds);
	
	mt_fh_hal_dvfs(MT658X_FH_MM_PLL, target_dds); 

	if(g_fh_pll[MT658X_FH_MM_PLL].fh_status == FH_FH_ENABLE_SSC)
	{
		fh_set_field(REG_FHCTL4_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
		fh_set_field(REG_FHCTL4_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL4_CFG, FH_FHCTLX_EN, 0);  //disable hopping control

		if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(MT658X_FH_MM_PLL)))
			return 0;
		FH_MSG("Enable mmpll SSC mode");
		FH_MSG("1. sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL4_DDS: 0x%08x", (fh_read32(REG_FHCTL4_DDS)&0x1FFFFF));

		fh_set_field(REG_FHCTL4_CFG,FH_SFSTRX_DYS,mt_ssc_mmpll_setting[2].df);
		fh_set_field(REG_FHCTL4_CFG,FH_SFSTRX_DTS,mt_ssc_mmpll_setting[3].dt);

		fh_write32(REG_FHCTL4_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL4_DDS)&0x1FFFFF),mt_ssc_mmpll_setting[2].lowbnd) << 16));
		FH_MSG("REG_FHCTL4_UPDNLMT: 0x%08x", fh_read32(REG_FHCTL4_UPDNLMT));
		
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MM_PLL,1);

		fh_set_field(REG_FHCTL4_CFG, FH_FRDDSX_EN, 1);  //enable SSC mode
    		fh_set_field(REG_FHCTL4_CFG, FH_FHCTLX_EN, 1);  //enable hopping control

		FH_MSG("REG_FHCTL4_CFG: 0x%08x", fh_read32(REG_FHCTL4_CFG));
		
	}
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	return 0;
}

static int mt_fh_hal_dfs_vencpll(unsigned int target_freq)  //vencpll dfs mode
{
	unsigned long 	flags;
	unsigned int	target_dds=0;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;

	FH_MSG("EN: %s:",__func__);
	//FH_MSG("current freq:%d target freq:%d", current_freq, target_freq);
	FH_MSG("current dds(VENCPLL_CON1): 0x%x",(fh_read32(VENCPLL_CON1)&0x1FFFFF));

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	if(g_fh_pll[MT658X_FH_VENC_PLL].fh_status == FH_FH_ENABLE_SSC)
	{//only when SSC is enable	
			//Turn off MEMPLL hopping
			fh_set_field(REG_FHCTL5_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL5_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    			fh_set_field(REG_FHCTL5_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
		
			//udelay(800);
			
			pll_dds =  (DRV_Reg32(REG_FHCTL5_DDS)) & 0x1FFFFF;
			fh_dds	=  (DRV_Reg32(REG_FHCTL5_MON)) & 0x1FFFFF;
			
			FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
			
			while((pll_dds != fh_dds) && ( i < 100)){
		
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
					
				udelay(10);
				fh_dds	=  (DRV_Reg32(REG_FHCTL5_MON)) & 0x1FFFFF;
				i++;
			}	
						
			FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);

	}
	
	if(target_freq == VENCPLL_TARGETVCO_1)
		target_dds = VENCPLL_TARGETVCO_1_DDS;
	else if (target_freq == VENCPLL_TARGETVCO_2)
		target_dds = VENCPLL_TARGETVCO_2_DDS;
	else
        {
		FH_BUG_ON("Incorrect VENCPLL_TARGETVCO.");
                return -1;
        } 
	
	FH_MSG("target dds: 0x%x",target_dds);
	
	mt_fh_hal_dvfs(MT658X_FH_VENC_PLL, target_dds); 

	if(g_fh_pll[MT658X_FH_VENC_PLL].fh_status == FH_FH_ENABLE_SSC)
	{
		fh_set_field(REG_FHCTL5_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
		fh_set_field(REG_FHCTL5_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL5_CFG, FH_FHCTLX_EN, 0);  //disable hopping control

		if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(MT658X_FH_VENC_PLL)))
			return 0;
		FH_MSG("Enable vencpll SSC mode");
		FH_MSG("1. sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL5_DDS: 0x%08x", (fh_read32(REG_FHCTL5_DDS)&0x1FFFFF));

		fh_set_field(REG_FHCTL5_CFG,FH_SFSTRX_DYS,mt_ssc_vencpll_setting[2].df);
		fh_set_field(REG_FHCTL5_CFG,FH_SFSTRX_DTS,mt_ssc_vencpll_setting[2].dt);

		fh_write32(REG_FHCTL5_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL5_DDS)&0x1FFFFF),mt_ssc_vencpll_setting[2].lowbnd) << 16));
		FH_MSG("REG_FHCTL5_UPDNLMT: 0x%08x", fh_read32(REG_FHCTL5_UPDNLMT));
		
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_VENC_PLL,1);

		fh_set_field(REG_FHCTL5_CFG, FH_FRDDSX_EN, 1);  //enable SSC mode
    		fh_set_field(REG_FHCTL5_CFG, FH_FHCTLX_EN, 1);  //enable hopping control

		FH_MSG("REG_FHCTL5_CFG: 0x%08x", fh_read32(REG_FHCTL5_CFG));
		
	}
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	return 0;
}

static int mt_fh_hal_l2h_dvfs_mempll(void)  //mempll low to high (266->333MHz)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
#if defined(DUMMY_READ_ENGINE)	// B, enable dummy read engine here
		DFE_para fh_para;
#endif
	FH_MSG("EN: %s:",__func__);
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */

#if 1	
	// api change in mt6592, DFS_Detection() instead of get_ddr_type()
	// mem dfs only enable in LPDDR3 in mt6592
	if( DFS_Detection() == -1) {
		FH_MSG("DFS_Detection false*");
		return -1;
	}

	#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
	#endif	
#endif

	FH_MSG("dds: 0x%x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	g_pll_mode = (DRV_Reg32(DDRPHY_BASE+0x60C) & 0x200);
	FH_MSG("g_pll_mode(0:1pll, 1:3pll): %x",g_pll_mode);

	if (g_pll_mode==0) // 1pll mode
		{
		if ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )==MEMPLL333MHZ_DDS)	//dfs 333MHz
			{
				FH_MSG("DDR Already @1333MHz");
				return 0;
			}
		else
			FH_MSG("Jump to 1333MHz");		
		}
	else if (g_pll_mode==0x200) // 3pll mode
		{
		if ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )==MEMPLL333MHZ_DDS)	//dfs 333MHz
			{
				FH_MSG("DDR Already @1333MHz");
				return 0;
			}
		else
			FH_MSG("Jump to 1333MHz");
		}	

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC)
		{//only when SSC is enable	
			//Turn off MEMPLL hopping
			fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL2_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
		
			//udelay(800);
			
			pll_dds =  (DRV_Reg32(REG_FHCTL2_DDS)) & 0x1FFFFF;
			fh_dds	=  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			
			FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
			
			while((pll_dds != fh_dds) && ( i < 100)){
		
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
					
				udelay(10);
				fh_dds	=  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
				i++;
			}	
						
			FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);

		}

#if defined(DUMMY_READ_ENGINE)	// B, enable dummy read engine here
		fh_para.rd_enable =1;
		fh_para.test_length = 1;
		fh_para.rd_addr = 0xBFFFFF80;
		DFE_Config(fh_para);
		DFE_Enable(true);
		FH_MSG("dummy read engine enabled");
#endif	// E, enable dummy read engine here


	if (g_pll_mode==0) // 1pll mode
		mt_fh_hal_dvfs(MT658X_FH_MEM_PLL, MEMPLL333MHZ_DDS); //dfs 333MHz
	else if (g_pll_mode==0x200) // 3pll mode
		mt_fh_hal_dvfs(MT658X_FH_MEM_PLL, MEMPLL333MHZ_DDS); //dfs 333MHz

	g_curr_dramc = 333;

#if defined(DUMMY_READ_ENGINE)	// B, enable dummy read engine here
		DFE_Enable(false);
		FH_MSG("dummy read engine disabled");
#endif	// E, enable dummy read engine here
		
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC)
	{
		fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
		fh_set_field(REG_FHCTL2_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    	fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 0);  //disable hopping control

		if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(MT658X_FH_MEM_PLL)))
			return 0;
		FH_MSG("1. sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL2_DDS: 0x%08x", (fh_read32(REG_FHCTL2_DDS)&0x1FFFFF));

		fh_set_field(REG_FHCTL2_CFG,FH_SFSTRX_DYS,mt_ssc_mempll_setting[3].df);
		fh_set_field(REG_FHCTL2_CFG,FH_SFSTRX_DTS,mt_ssc_mempll_setting[3].dt);
		//low to high, apply down hopping, config low bound.
		fh_write32(REG_FHCTL2_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL2_DDS)&0x1FFFFF),mt_ssc_mempll_setting[3].lowbnd) << 16));
		FH_MSG("REG_FHCTL2_UPDNLMT: 0x%08x", fh_read32(REG_FHCTL2_UPDNLMT));
		
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL,1);

		fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 1);  //enable SSC mode
    	fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 1);  //enable hopping control
    	FH_MSG("REG_FHCTL2_CFG: 0x%08x", fh_read32(REG_FHCTL2_CFG));
		
	}
	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	return 0;
}

static int mt_fh_hal_h2l_dvfs_mempll(void)  //mempll high to low(333->266MHz)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
#if defined(DUMMY_READ_ENGINE)	// B, enable dummy read engine here
		DFE_para fh_para;
#endif
	FH_MSG("EN: %s:",__func__);
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */

#if 1	
	// api change in mt6592, DFS_Detection() instead of get_ddr_type()
	// mem dfs only enable in LPDDR3 in mt6592
	if( DFS_Detection() == -1) {
		FH_MSG("DFS_Detection false*");
		return -1;
	}

	#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
	#endif	
#endif

	FH_MSG("dds: 0x%x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	g_pll_mode = (DRV_Reg32(DDRPHY_BASE+0x60C) & 0x200);
	FH_MSG("g_pll_mode(0:1pll, 1:3pll): %x",g_pll_mode);

	if (g_pll_mode==0) // 1pll mode
		{
		if ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )==MEMPLL266MHZ_DDS)	// 266 Mhz
			{
				FH_MSG("DDR Already @1066MHz");
				return 0;
			}
		else
			FH_MSG("Jump to 1066MHz");
		}
	else if (g_pll_mode==0x200) // 3pll mode
		{
		if ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )==MEMPLL266MHZ_DDS)	// 266 Mhz
			{
				FH_MSG("DDR Already @1066MHz");
				return 0;
			}
		else
			FH_MSG("Jump to 1066MHz");
		}
	
	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC)
		{//only when SSC is enable	
			//Turn off MEMPLL hopping
			fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL2_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
		
			//udelay(800);
			
			pll_dds =  (DRV_Reg32(REG_FHCTL2_DDS)) & 0x1FFFFF;
			fh_dds	=  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			
			FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
			
			while((pll_dds != fh_dds) && ( i < 100)){
		
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
					
				udelay(10);
				fh_dds	=  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
				i++;
			}	
						
			FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
		}

#if defined(DUMMY_READ_ENGINE)	// B, enable dummy read engine here
		fh_para.rd_enable =1;
		fh_para.test_length = 1;
		fh_para.rd_addr = 0xBFFFFF80;
		DFE_Config(fh_para);
		DFE_Enable(true);
		FH_MSG("dummy read engine enabled");
#endif	// E, enable dummy read engine here

	if (g_pll_mode==0) // 1pll mode
		mt_fh_hal_dvfs(MT658X_FH_MEM_PLL, MEMPLL266MHZ_DDS); // 266 Mhz
	else if (g_pll_mode==0x200) // 3pll mode
		mt_fh_hal_dvfs(MT658X_FH_MEM_PLL, MEMPLL266MHZ_DDS); // 266 Mhz

	g_curr_dramc = 266;


#if defined(DUMMY_READ_ENGINE)	// B, disable dummy read engine here
		DFE_Enable(false);
		FH_MSG("dummy read engine disabled");
#endif	// E, disable dummy read engine here

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC)
	{
		fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
		fh_set_field(REG_FHCTL2_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 0);  //disable hopping control

		if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(MT658X_FH_MEM_PLL)))
			return 0;
		FH_MSG("Enable mempll SSC mode");
		FH_MSG("1. sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL2_DDS: 0x%08x", (fh_read32(REG_FHCTL2_DDS)&0x1FFFFF));

		fh_set_field(REG_FHCTL2_CFG,FH_SFSTRX_DYS,mt_ssc_mempll_setting[4].df);
		fh_set_field(REG_FHCTL2_CFG,FH_SFSTRX_DTS,mt_ssc_mempll_setting[4].dt);
		//high to low, apply up hopping, config upper bound.
		fh_write32(REG_FHCTL2_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL2_DDS)&0x1FFFFF),mt_ssc_mempll_setting[4].upbnd)));		
		//fh_write32(REG_FHCTL2_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL2_DDS)&0x1FFFFF),mt_ssc_mempll_setting[3].lowbnd) << 16));		
		FH_MSG("REG_FHCTL2_UPDNLMT: 0x%08x", fh_read32(REG_FHCTL2_UPDNLMT));
		
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL,1);

		fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 1);  //enable SSC mode
    		fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 1);  //enable hopping control

		FH_MSG("REG_FHCTL2_CFG: 0x%08x", fh_read32(REG_FHCTL2_CFG));
		
	}
	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	return 0;
}
#if 0
static int mt_h2oc_dfs_mempll(void)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;

	FH_MSG("EN: %s:",__func__);
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */
#if 0
	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
#endif
	FH_MSG("dds: 0x%x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	g_pll_mode = (DRV_Reg32(DDRPHY_BASE+0x60C) & 0x200);
	FH_MSG("g_pll_mode(0:1pll, 1:3pll): %x",g_pll_mode);

	if (g_pll_mode==0) // 1pll mode
		{
		if ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )==0x160077)
			{
				FH_MSG("Already @1172MHz");
				return 0;
			}
		else
			FH_MSG("Jump to 1172MHz");		
		}
	else if (g_pll_mode==0x200) // 3pll mode
		{
		if ((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )==0x150071)
			{
				FH_MSG("Already @1172MHz");
				return 0;
			}
		else
			FH_MSG("Jump to 1172MHz");
		}	

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC)
		{//only when SSC is enable	
			//Turn off MEMPLL hopping
			fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL2_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
		
			//udelay(800);
			
			pll_dds =  (DRV_Reg32(REG_FHCTL2_DDS)) & 0x1FFFFF;
			fh_dds	=  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			
			FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
			
			while((pll_dds != fh_dds) && ( i < 100)){
		
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
					
				udelay(10);
				fh_dds	=  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
				i++;
			}	
						
			FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);

		}

	if (g_pll_mode==0) // 1pll mode
		mt_fh_hal_dvfs(MT658X_FH_MEM_PLL, 0x160077); //dfs293MHz
	else if (g_pll_mode==0x200) // 3pll mode
		mt_fh_hal_dvfs(MT658X_FH_MEM_PLL, 0x150071); //dfs 293MHz

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC)
	{
		fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
		fh_set_field(REG_FHCTL2_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    	fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 0);  //disable hopping control

		if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(MT658X_FH_MEM_PLL)))
			return 0;
		FH_MSG("1. sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL2_DDS: 0x%08x", (fh_read32(REG_FHCTL2_DDS)&0x1FFFFF));

		fh_set_field(REG_FHCTL2_CFG,FH_SFSTRX_DYS,mt_ssc_mempll_setting[2].df);
		fh_set_field(REG_FHCTL2_CFG,FH_SFSTRX_DTS,mt_ssc_mempll_setting[2].dt);

		fh_write32(REG_FHCTL2_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL2_DDS)&0x1FFFFF),mt_ssc_mempll_setting[2].lowbnd) << 16));
		FH_MSG("REG_FHCTL2_UPDNLMT: 0x%08x", fh_read32(REG_FHCTL2_UPDNLMT));
		
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MEM_PLL,1);

		fh_set_field(REG_FHCTL2_CFG, FH_FRDDSX_EN, 1);  //enable SSC mode
    	fh_set_field(REG_FHCTL2_CFG, FH_FHCTLX_EN, 1);  //enable hopping control
    	FH_MSG("REG_FHCTL2_CFG: 0x%08x", fh_read32(REG_FHCTL2_CFG));
		
	}
	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	return 0;
}
#endif
static int mt_fh_hal_dram_overclock(int clk)
{
        int ret=0;
	FH_MSG("EN: %s clk:%d",__func__,clk);
if (1)//DFS mode
{
        enable_clock(MT_CG_PERI_AP_DMA,"DFS_GDMA_MODULE");  //BIKE
	if( clk == 266)
	{
		g_curr_dramc = 266;
                ret=mt_fh_hal_h2l_dvfs_mempll(); 
                disable_clock(MT_CG_PERI_AP_DMA,"DFS_GDMA_MODULE");  //BIKE
		return ret;
	}

	if( clk == 333)
	{
		g_curr_dramc = 333;
                ret=mt_fh_hal_l2h_dvfs_mempll();
                disable_clock(MT_CG_PERI_AP_DMA,"DFS_GDMA_MODULE");  //BIKE
		return ret;	
	}
	
	#if 1	// not used in MT6592
	if( clk == 293)
	{
		g_curr_dramc = 293;
                ret=mt_fh_hal_l2h_dvfs_mempll();
                disable_clock(MT_CG_PERI_AP_DMA,"DFS_GDMA_MODULE");  //BIKE
                return ret;

	}
	#endif
       
        #if 1   // not used in MT6592
        if( clk == 367)
        {
                g_curr_dramc = 367;
                ret=mt_fh_hal_l2h_dvfs_mempll();
                disable_clock(MT_CG_PERI_AP_DMA,"DFS_GDMA_MODULE");  //BIKE
                return ret;
        }
        #endif

}
#if 0
else	
{
	if( clk == LOW_DRAMC){ //target freq: 208.5MHz
		if( g_curr_dramc != 266 ){ //266 -> 208.5 only
			FH_BUG_ON(1);
			return -1;
		}
		else{ //let's move from 266 to 208.5
			return(mt_fh_hal_h2l_mempll());			
		}
	}
	
	if( clk == 293){ //target freq: 293MHz
		if( g_curr_dramc != 266 ){ //266 -> 293 only
			FH_BUG_ON(1);
			return -1;
		}
		else{ //let's move from 266 to 293
			return(mt_h2oc_mempll());			
		}
	}
	
	if( clk == 266){ ////target freq: 293MHz
		if( g_curr_dramc == 266 ){ //cannot be 266 -> 266
			FH_BUG_ON(1);
			return -1;
		}
		else if( g_curr_dramc == LOW_DRAMC ){ //208 -> 266
			return(mt_fh_hal_l2h_mempll());			
		}
		else if( g_curr_dramc == 293 ){ //293 -> 266
			return(mt_oc2h_mempll());			
		}
	}
}	
#endif
	FH_BUG_ON(1);
	return(-1);
}

static int mt_fh_hal_get_dramc(void)
{
	return(g_curr_dramc);
}

static void mt_fh_hal_popod_save(void)
{
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
	
	FH_MSG("EN: %s",__func__);
	
	//disable maipll SSC mode
	if(g_fh_pll[MT658X_FH_MAIN_PLL].fh_status == FH_FH_ENABLE_SSC)//only when SSC is enable
		{	
			//Turn off MAINPLL hopping
			fh_set_field(REG_FHCTL1_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL1_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    		fh_set_field(REG_FHCTL1_CFG, FH_FHCTLX_EN, 0);  //disable hopping control
		
			//udelay(800);
			
			pll_dds =  (DRV_Reg32(REG_FHCTL1_DDS)) & 0x1FFFFF;
			fh_dds	=  (DRV_Reg32(REG_FHCTL1_MON)) & 0x1FFFFF;
			
			FH_MSG("Org pll_dds:%x fh_dds:%x",pll_dds,fh_dds);
			
			while((pll_dds != fh_dds) && ( i < 100))
			{
		
				if(unlikely(i > 100)){
					FH_BUG_ON(i > 100);
					break;
				}
					
				udelay(10);
				fh_dds	=  (DRV_Reg32(REG_FHCTL1_MON)) & 0x1FFFFF;
				i++;
			}	
						
			FH_MSG("pll_dds:%x  fh_dds:%x   i:%d",pll_dds,fh_dds,i);

			//write back to ncpo	
			fh_write32(MAINPLL_CON1, (fh_read32(REG_FHCTL1_DDS)&0x1FFFFF)|(fh_read32(MAINPLL_CON1)&0xFFE00000)|(1U<<31));
			FH_MSG("MAINPLL_CON1: 0x%08x",(fh_read32(MAINPLL_CON1)&0x1FFFFF));

			// switch to register control
			mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MAIN_PLL,0);
			mb();
			FH_MSG("switch to register control PLL_HP_CON0: 0x%08x",(fh_read32(PLL_HP_CON0)&0x3F));
			
		}
	
}

static void mt_fh_hal_popod_restore(void)
{
	
	FH_MSG("EN: %s",__func__);

	//enable maipll SSC mode
	if(g_fh_pll[MT658X_FH_MAIN_PLL].fh_status == FH_FH_ENABLE_SSC)
	{
		fh_set_field(REG_FHCTL1_CFG, FH_FRDDSX_EN, 0);  //disable SSC mode
		fh_set_field(REG_FHCTL1_CFG, FH_SFSTRX_EN, 0);  //disable dvfs mode
    	fh_set_field(REG_FHCTL1_CFG, FH_FHCTLX_EN, 0);  //disable hopping control

		if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(MT658X_FH_MAIN_PLL)))
			return;
		FH_MSG("Enable mainpll SSC mode");
		FH_MSG("sync ncpo to DDS of FHCTL");
		FH_MSG("FHCTL1_DDS: 0x%08x", (fh_read32(REG_FHCTL1_DDS)&0x1FFFFF));

		fh_set_field(REG_FHCTL1_CFG,FH_SFSTRX_DYS,mt_ssc_mainpll_setting[2].df);
		fh_set_field(REG_FHCTL1_CFG,FH_SFSTRX_DTS,mt_ssc_mainpll_setting[2].dt);

		fh_write32(REG_FHCTL1_UPDNLMT, (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL1_DDS)&0x1FFFFF),mt_ssc_mainpll_setting[2].lowbnd) << 16));
		FH_MSG("REG_FHCTL1_UPDNLMT: 0x%08x", fh_read32(REG_FHCTL1_UPDNLMT));
		
		mt_fh_hal_switch_register_to_FHCTL_control(MT658X_FH_MAIN_PLL,1);

		fh_set_field(REG_FHCTL1_CFG, FH_FRDDSX_EN, 1);  //enable SSC mode
    	fh_set_field(REG_FHCTL1_CFG, FH_FHCTLX_EN, 1);  //enable hopping control

		FH_MSG("REG_FHCTL1_CFG: 0x%08x", fh_read32(REG_FHCTL1_CFG));
		
	}

}
#define DRAMC_READ_REG(offset)         ( (readl(DRAMC0_BASE + (offset))) | (readl(DDRPHY_BASE + (offset))) | (readl(DRAMC_NAO_BASE + (offset))) )

static int freqhopping_dramc_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)

{
	char *p = page;
	int len = 0;

	FH_MSG("EN: %s",__func__);
#if 1	
	p += sprintf(p, "DRAMC: %dMHz\r\n",g_curr_dramc);
	p += sprintf(p, "mt_get_emi_freq(): %dHz\r\n",mt_get_emi_freq());	
	p += sprintf(p, "get_ddr_type(): %d\r\n",get_ddr_type());	
	p += sprintf(p, "rank: 0x%x\r\n",(DRV_Reg32(EMI_CONA) & 0x20000));	
	p += sprintf(p, "R0 HW=%xh, R1 HW=%xh\r\n", DRAMC_READ_REG(0x374), DRAMC_READ_REG(0x378));
	*start = page + off;

	len = p - page;

	if (len > off)
		len -= off;
	else
		len = 0;
#endif
	return len < count ? len : count;
}


static int freqhopping_dramc_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len = 0, freq = 0;
	char dramc[32];

	FH_MSG("EN: proc");

	len = (count < (sizeof(dramc) - 1)) ? count : (sizeof(dramc) - 1);

	if (copy_from_user(dramc, buffer, len))
	{
		FH_MSG("copy_from_user fail!");
		return 1;
	}
	
	dramc[len] = '\0';
   
	if (sscanf(dramc, "%d", &freq) == 1)
	{
		if( (freq == 333) || (freq == 266)){
			FH_MSG("dramc:%d ", freq);
			(freq==333) ? mt_fh_hal_l2h_dvfs_mempll() : mt_fh_hal_h2l_dvfs_mempll();
		}
#if 0
		else if(freq == 293){
			mt_fh_hal_dram_overclock(293);
		}
#endif
		else{
			FH_MSG("must be 266/333!");
		}

#if 0 
		if(freq == 266){
			FH_MSG("==> %d",mt_fh_hal_dram_overclock(266));
		}
		else if(freq == 293){
			FH_MSG("==> %d",mt_fh_hal_dram_overclock(293));
		}
		else if(freq == LOW_DRAMC){
			FH_MSG("==> %d",mt_fh_hal_dram_overclock(208));
		}
#endif

		return count;
	}
	else
	{
		FH_MSG("  bad argument!!");
	}

	return -EINVAL;
}

static int freqhopping_dvfs_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len = 0;
	int 	i=0;

	FH_MSG("EN: %s",__func__);

	p += sprintf(p, "DVFS:\r\n");
	p += sprintf(p, "CFG: 0x3 is SSC mode;  0x5 is DVFS mode \r\n");
	for(i=0;i<MT_FHPLL_MAX;i++) {		
		p += sprintf(p, "FHCTL%d:   CFG:0x%08x    DVFS:0x%08x\r\n",i, DRV_Reg32(REG_FHCTL0_CFG+(i*0x14)), DRV_Reg32(REG_FHCTL0_DVFS+(i*0x14)));
	}

	*start = page + off;

	len = p - page;

	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len : count;
}

static int freqhopping_dvfs_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int 		ret;
	char 		kbuf[256];
	unsigned long 	len = 0;
	unsigned int	p1,p2,p3,p4,p5;
//	struct 		freqhopping_ioctl fh_ctl;
	
	p1=0;
	p2=0;
	p3=0;
	p4=0;
	p5=0;

	FH_MSG("EN: %s",__func__);
	
	len = min(count, (unsigned long)(sizeof(kbuf)-1));

	if (count == 0)return -1;
	if(count > 255)count = 255;

	ret = copy_from_user(kbuf, buffer, count);
	if (ret < 0)return -1;
	
	kbuf[count] = '\0';

	sscanf(kbuf, "%d %d %d %d %d", &p1, &p2, &p3, &p4, &p5);

	FH_MSG("EN: p1=%d p2=%d p3=%d", p1, p2, p3);

	if (p1==MT658X_FH_MEM_PLL)
		{
			if (p2==333)
				mt_fh_hal_l2h_dvfs_mempll();
			else if (p2==266)
				mt_fh_hal_h2l_dvfs_mempll();
			#if 0
			else if (p2==586)
				mt_h2oc_dfs_mempll();
			#endif
			else
				FH_MSG("not define %d freq @mempll",p2);
			//FH_MSG("DDR Freq. = %d", mt_get_mempllclk_freq());
			FH_MSG("MEMPLL DVFS completed\n");		
		}
	else if (p1==MT658X_FH_ARM_PLL)
		{
			mt_fh_hal_dfs_armpll(p2, p3);
			//FH_MSG("ARMPLL Freq. = %d", mt_get_armpllclk_freq());
			FH_MSG("ARMPLL DVFS completed\n");					
		}
	else if (p1==MT658X_FH_MM_PLL)
		{
			mt_fh_hal_dfs_mmpll(p3);
			//FH_MSG("MMPLLFreq. = %d", mt_get_mmpllclk_freq());
			FH_MSG("MMPLL DVFS completed\n");
		}
	else if (p1==MT658X_FH_VENC_PLL)
		{
			mt_fh_hal_dfs_vencpll(p3);
			//FH_MSG("VENCPLL Freq. = %d", mt_get_vencpllclk_freq());
			FH_MSG("VENCPLL DVFS completed\n");			
		}
	
	else if (p1==4370)
		{
			FH_MSG("EN: pllid=%d dt=%d df=%d lowbnd=%d", p2, p3, p4, p5);
			fh_set_field(REG_FHCTL0_CFG+(p2*0x14), FH_FRDDSX_EN, 0);  //disable SSC mode
			fh_set_field(REG_FHCTL0_CFG+(p2*0x14), FH_SFSTRX_EN, 0);  //disable dvfs mode
	    		fh_set_field(REG_FHCTL0_CFG+(p2*0x14), FH_FHCTLX_EN, 0);  //disable hopping control

			if (!(mt_fh_hal_sync_ncpo_to_FHCTL_DDS(p2)))
				return 0;
			FH_MSG("Enable FHCTL%d SSC mode",p2);
			FH_MSG("1. sync ncpo to DDS of FHCTL");
			FH_MSG("FHCTL%d_DDS: 0x%08x",p2 ,(fh_read32(REG_FHCTL0_DDS+(p2*0x14))&0x1FFFFF));

			fh_set_field(REG_FHCTL0_CFG+(p2*0x14),FH_SFSTRX_DYS,p4);
			fh_set_field(REG_FHCTL0_CFG+(p2*0x14),FH_SFSTRX_DTS,p3);

			fh_write32(REG_FHCTL0_UPDNLMT+(p2*0x14), (PERCENT_TO_DDSLMT((fh_read32(REG_FHCTL0_DDS+(p2*0x14))&0x1FFFFF),p5) << 16));
			FH_MSG("REG_FHCTL%d_UPDNLMT: 0x%08x",p2 , fh_read32(REG_FHCTL0_UPDNLMT+(p2*0x14)));
			
			mt_fh_hal_switch_register_to_FHCTL_control(p2,1);

			fh_set_field(REG_FHCTL0_CFG+(p2*0x14), FH_FRDDSX_EN, 1);  //enable SSC mode
	    		fh_set_field(REG_FHCTL0_CFG+(p2*0x14), FH_FHCTLX_EN, 1);  //enable hopping control

			FH_MSG("REG_FHCTL%d_CFG: 0x%08x",p2 , fh_read32(REG_FHCTL0_CFG+(p2*0x14)));
		}
	else if (p1==2222)
		{
			if (p2==0) //disable
				mt_fh_hal_popod_save();
			else if (p2==1) //enable
				mt_fh_hal_popod_restore();
		}
	else
		mt_fh_hal_dvfs(p1, p2); 

	return count;
}



static int freqhopping_dumpregs_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)

{
	char 	*p = page;
	int 	len = 0;
	int 	i=0;

	FH_MSG("EN: %s",__func__);

	p += sprintf(p, "FHDMA_CFG:\r\n");

	p += sprintf(p, "REG_FHDMA_CFG: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHDMA_CFG),
		DRV_Reg32(REG_FHDMA_2G1BASE),
		DRV_Reg32(REG_FHDMA_2G2BASE),
		DRV_Reg32(REG_FHDMA_INTMDBASE));

	p += sprintf(p, "REG_FHDMA_EXTMDBASE: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHDMA_EXTMDBASE),
		DRV_Reg32(REG_FHDMA_BTBASE),
		DRV_Reg32(REG_FHDMA_WFBASE),
		DRV_Reg32(REG_FHDMA_FMBASE));

	p += sprintf(p, "REG_FHSRAM_CON: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHSRAM_CON),
		DRV_Reg32(REG_FHSRAM_WR),
		DRV_Reg32(REG_FHSRAM_RD),
		DRV_Reg32(REG_FHCTL_CFG),
		DRV_Reg32(REG_FHCTL_CON));

	p += sprintf(p, "REG_FHCTL_2G1_CH: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHCTL_2G1_CH),
		DRV_Reg32(REG_FHCTL_2G2_CH),
		DRV_Reg32(REG_FHCTL_INTMD_CH),
		DRV_Reg32(REG_FHCTL_EXTMD_CH));

	p += sprintf(p, "REG_FHCTL_BT_CH: 0x%08x 0x%08x 0x%08x \r\n\r\n",
		DRV_Reg32(REG_FHCTL_BT_CH),
		DRV_Reg32(REG_FHCTL_WF_CH),
		DRV_Reg32(REG_FHCTL_FM_CH));

        p += sprintf(p, "REG_FHCTL_CON: 0x%08x \r\n\r\n", DRV_Reg32(REG_FHCTL_CON));

	for(i=0;i<MT_FHPLL_MAX;i++) {
		
		p += sprintf(p, "FHCTL%d_CFG:\r\n",i);
		p += sprintf(p, "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
			DRV_Reg32(REG_FHCTL0_CFG+(i*0x14)),
			DRV_Reg32(REG_FHCTL0_UPDNLMT+(i*0x14)),
			DRV_Reg32(REG_FHCTL0_DDS+(i*0x14)),
			DRV_Reg32(REG_FHCTL0_MON+(i*0x14)));
	}

	
	p += sprintf(p, "\r\nPLL_HP_CON0:\r\n0x%08x\r\n",
		DRV_Reg32(PLL_HP_CON0));
		
	p += sprintf(p, "\r\nPLL_CON0 :\r\nARM:0x%08x MAIN:0x%08x MSDC:0x%08x MM:0x%08x VENC:0x%08x\r\n",
			DRV_Reg32(ARMPLL_CON0),	
			DRV_Reg32(MAINPLL_CON0),	
			DRV_Reg32(MSDCPLL_CON0),	
			DRV_Reg32(MMPLL_CON0),
			DRV_Reg32(VENCPLL_CON0));

	p += sprintf(p, "\r\nPLL_CON1 :\r\nARM:0x%08x MAIN:0x%08x MSDC:0x%08x MM:0x%08x VENC:0x%08x\r\n",
			DRV_Reg32(ARMPLL_CON1),	
			DRV_Reg32(MAINPLL_CON1),	
			DRV_Reg32(MSDCPLL_CON1),	
			DRV_Reg32(MMPLL_CON1),
			DRV_Reg32(VENCPLL_CON1));
		

	p += sprintf(p, "\r\nMEMPLL :\r\nMEMPLL9: 0x%08x MEMPLL10: 0x%08x MEMPLL11: 0x%08x MEMPLL12: 0x%08x\r\n",
			DRV_Reg32(DDRPHY_BASE+0x624),
			DRV_Reg32(DDRPHY_BASE+0x628),
			DRV_Reg32(DDRPHY_BASE+0x62C),
			DRV_Reg32(DDRPHY_BASE+0x630)); //TODO: Hard code for now...
	p += sprintf(p, "\r\nMEMPLL :\r\nMEMPLL13: 0x%08x MEMPLL14: 0x%08x MEMPLL15: 0x%08x MEMPLL16: 0x%08x\r\n",
			DRV_Reg32(DDRPHY_BASE+0x634),
			DRV_Reg32(DDRPHY_BASE+0x638),
			DRV_Reg32(DDRPHY_BASE+0x63C),
			DRV_Reg32(DDRPHY_BASE+0x640)); //TODO: Hard code for now...
		

	*start = page + off;

	len = p - page;

	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len : count;
}





#if MT_FH_CLK_GEN

static int freqhopping_clkgen_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)

{
	char *p = page;
	int len = 0;

	FH_MSG("EN: %s",__func__);
	
	if(g_curr_clkgen > MT658X_FH_PLL_TOTAL_NUM ){
		p += sprintf(p, "no clkgen output.\r\n");
	}
	else{
		p += sprintf(p, "clkgen:%d\r\n",g_curr_clkgen);
	}

	p += sprintf(p, "\r\nMBIST :\r\nMBIST_CFG_2: 0x%08x MBIST_CFG_6: 0x%08x MBIST_CFG_7: 0x%08x\r\n",
			DRV_Reg32(MBIST_CFG_2),
			DRV_Reg32(MBIST_CFG_6),
			DRV_Reg32(MBIST_CFG_7));
			
	p += sprintf(p, "\r\nCLK_CFG_3: 0x%08x\r\n",
			DRV_Reg32(CLK_CFG_3));
			
	p += sprintf(p, "\r\nTOP_CKMUXSEL: 0x%08x\r\n",
			DRV_Reg32(TOP_CKMUXSEL));

	p += sprintf(p, "\r\nGPIO: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
			DRV_Reg32(GPIO_BASE+0xC60),
			DRV_Reg32(GPIO_BASE+0xC70),
			DRV_Reg32(GPIO_BASE+0xCD0),
			DRV_Reg32(GPIO_BASE+0xD90));
	
	
	p += sprintf(p, "\r\nDDRPHY_BASE :\r\n0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
			DRV_Reg32(DDRPHY_BASE+0x600),
			DRV_Reg32(DDRPHY_BASE+0x604),
			DRV_Reg32(DDRPHY_BASE+0x608),
			DRV_Reg32(DDRPHY_BASE+0x60C),
			DRV_Reg32(DDRPHY_BASE+0x614),
			DRV_Reg32(DDRPHY_BASE+0x61C));

	*start = page + off;

	len = p - page;

	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len : count;
}


static int freqhopping_clkgen_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len = 0, pll_id = 0;
	char clkgen[32];

	FH_MSG("EN: %s",__func__);

	len = (count < (sizeof(clkgen) - 1)) ? count : (sizeof(clkgen) - 1);

	//if you want to measue the clk by evb, you should turn on GPIO34.
	//mt_set_gpio_mode(GPIO34, GPIO_MODE_03);

	if (copy_from_user(clkgen, buffer, len))
	{
		FH_MSG("copy_from_user fail!");
		return 1;
	}
	
	clkgen[len] = '\0';
   
	if (sscanf(clkgen, "%d", &pll_id) == 1)
	{
		if(pll_id == MT658X_FH_ARM_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_pre_clk [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_clk_sel [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000401); //abist_clk_sel [0100: armpll_occ_mon]
			udelay(1000);

		}
		else if(pll_id == MT658X_FH_MAIN_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000F0001); //pll_pre_clk [1111: AD_MAIN_H230P3M]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000F0001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000F0F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);

		}
		else if(pll_id == MT658X_FH_MEM_PLL){
						
			fh_write32(DDRPHY_BASE+0x600, ( (DRV_Reg32(DDRPHY_BASE+0x600)) | 0x1<<5));
			
			
			fh_write32(DDRPHY_BASE+0x60C, ( (DRV_Reg32(DDRPHY_BASE+0x60C)) | 0x1<<21));
			fh_write32(DDRPHY_BASE+0x614, ( (DRV_Reg32(DDRPHY_BASE+0x614)) | 0x1<<21));
			fh_write32(DDRPHY_BASE+0x61C, ( (DRV_Reg32(DDRPHY_BASE+0x61C)) | 0x1<<21));

			fh_write32(DDRPHY_BASE+0x60C, ( (DRV_Reg32(DDRPHY_BASE+0x60C)) & ~0x7));
			fh_write32(DDRPHY_BASE+0x60C, ( (DRV_Reg32(DDRPHY_BASE+0x60C)) | 0x2 ));
			fh_write32(DDRPHY_BASE+0x614, ( (DRV_Reg32(DDRPHY_BASE+0x614)) & ~0x7));
			fh_write32(DDRPHY_BASE+0x614, ( (DRV_Reg32(DDRPHY_BASE+0x614)) | 0x2 ));
			fh_write32(DDRPHY_BASE+0x61C, ( (DRV_Reg32(DDRPHY_BASE+0x61C)) & ~0x7));
			fh_write32(DDRPHY_BASE+0x61C, ( (DRV_Reg32(DDRPHY_BASE+0x61C)) | 0x2));

			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<3));
			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<7));
			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<4));
			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<9));

#if 0			
			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) & ~0x000E0000 ));
#endif 
			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) | 0x00040000 ));

			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) & ~0xF0000000 ));
			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) | 0x80000000 ));

			//fh_write32(MBIST_CFG_2, 0x00000001); //divide by 1+1
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_pre_clk [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_clk_sel [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000501); //abist_clk_sel [0101: AD_MEMPLL_MONCLK]
			udelay(1000);

		}
		else if(pll_id == MT658X_FH_MSDC_PLL){

			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00080001); //pll_pre_clk [1000: AD_MSDCPLL_H208M]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00080001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00080F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);
						
		}
		else if(pll_id == MT658X_FH_MM_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00090001); //pll_pre_clk [1001: AD_TVHDMI_H_CK]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00090001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00090F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);

		}
		else if(pll_id == MT658X_FH_VENC_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000A0001); //pll_pre_clk [1010: AD_LVDS_H180M_CK]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000A0001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000A0F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);
			
		}
	}
	else
	{
		FH_MSG("  bad argument!!");
	}
	
	g_curr_clkgen = pll_id;

	return count;

	//return -EINVAL;
}

#endif //MT_FH_CLK_GEN
//TODO: __init void mt_freqhopping_init(void)
static void mt_fh_hal_init(void)
{
	int 		i;
//	int 		ret = 0;
	unsigned long 	flags;
	
	FH_MSG("EN: %s",__func__);
	
	if(g_initialize == 1){
		FH_MSG("already init!");
		return;
	}
		
	//init hopping table for mempll 200<->266
	memset(g_mempll_fh_table, 0, sizeof(g_mempll_fh_table));

	
	for(i=0;i<MT_FHPLL_MAX;i++) {
		
		//TODO: use the g_fh_pll[] to init the FHCTL
		spin_lock_irqsave(&freqhopping_lock, flags);
				
		g_fh_pll[i].setting_id = 0;

		fh_write32(REG_FHCTL0_CFG+(i*0x14), 0x00000000); //No SSC and FH enabled
		fh_write32(REG_FHCTL0_UPDNLMT+(i*0x14), 0x00000000); //clear all the settings
		fh_write32(REG_FHCTL0_DDS+(i*0x14), 0x00000000); //clear all the settings
		
		//TODO: do we need this
		//fh_write32(REG_FHCTL0_MON+(i*0x10), 0x00000000); //clear all the settings
		
		spin_unlock_irqrestore(&freqhopping_lock, flags);
	}
	
	//TODO: update each PLL status (go through g_fh_pll)
	//TODO: wait for sophie's table & query the EMI clock
	//TODO: ask sophie to call this init function during her init call (mt_clkmgr_init() ??)
	//TODO: call __freqhopping_ctrl() to init each pll

	

	g_initialize = 1;
	//register change. 		enable_clock(MT_CG_PERI1_FHCTL, "FREQHOP") ;

#if defined(APDMA_DUMMY_READ)
        if(!DFS_APDMA_early_init())
        {
          FH_MSG("DFS APDMA allocate problem occur\n");
          ASSERT(0);
        }
        else
        {
          FH_MSG("DFS APDMA allocate correctly\n");
        }
        #ifdef TEMP_SENSOR
         DFS_APDMA_Init_2();
        #endif
#endif

}

static void mt_fh_hal_lock(unsigned long *flags)
{
	spin_lock_irqsave(&freqhopping_lock, *flags);	
}

static void mt_fh_hal_unlock(unsigned long *flags)
{
	spin_unlock_irqrestore(&freqhopping_lock, *flags);
}

static int mt_fh_hal_get_init(void)
{
	return(g_initialize);
}

static int mt_fh_hal_is_support_DFS_mode(void)
{
	return TRUE;
}

//TODO: module_init(mt_freqhopping_init);
//TODO: module_init(mt_freqhopping_init);
//TODO: module_exit(cpufreq_exit);

static struct mt_fh_hal_driver g_fh_hal_drv;

struct mt_fh_hal_driver *mt_get_fh_hal_drv(void)
{
	memset(&g_fh_hal_drv, 0, sizeof(g_fh_hal_drv));
	
	g_fh_hal_drv.fh_pll			= g_fh_pll;
	g_fh_hal_drv.fh_usrdef			= mt_ssc_fhpll_userdefined;
	g_fh_hal_drv.pll_cnt 			= MT658X_FH_PLL_TOTAL_NUM;
	g_fh_hal_drv.mempll 			= MT658X_FH_MEM_PLL;
	g_fh_hal_drv.mainpll 			= MT658X_FH_MAIN_PLL;
	g_fh_hal_drv.msdcpll 			= MT658X_FH_MSDC_PLL;
	g_fh_hal_drv.mmpll 			= MT658X_FH_MM_PLL;
	g_fh_hal_drv.vencpll 			= MT658X_FH_VENC_PLL;
	g_fh_hal_drv.lvdspll 			= MT658X_FH_VENC_PLL;
	
	g_fh_hal_drv.mt_fh_hal_init 		=  mt_fh_hal_init;

#if MT_FH_CLK_GEN	
	g_fh_hal_drv.proc.clk_gen_read 		=  freqhopping_clkgen_proc_read;
	g_fh_hal_drv.proc.clk_gen_write 	=  freqhopping_clkgen_proc_write;
#endif 
	
	g_fh_hal_drv.proc.dramc_read 		=  freqhopping_dramc_proc_read;
	g_fh_hal_drv.proc.dramc_write 		=  freqhopping_dramc_proc_write;	
	g_fh_hal_drv.proc.dumpregs_read 	=  freqhopping_dumpregs_proc_read;

	g_fh_hal_drv.proc.dvfs_read 		=  freqhopping_dvfs_proc_read;
	g_fh_hal_drv.proc.dvfs_write 		=  freqhopping_dvfs_proc_write;	

	g_fh_hal_drv.mt_fh_hal_ctrl		= __freqhopping_ctrl;
	g_fh_hal_drv.mt_fh_lock			= mt_fh_hal_lock;
	g_fh_hal_drv.mt_fh_unlock		= mt_fh_hal_unlock;
	g_fh_hal_drv.mt_fh_get_init		= mt_fh_hal_get_init;

	g_fh_hal_drv.mt_fh_popod_restore 	= mt_fh_hal_popod_restore;
	g_fh_hal_drv.mt_fh_popod_save		= mt_fh_hal_popod_save;

	//g_fh_hal_drv.mt_l2h_mempll		= mt_fh_hal_l2h_mempll;
	//g_fh_hal_drv.mt_h2l_mempll		= mt_fh_hal_h2l_mempll;
	g_fh_hal_drv.mt_l2h_dvfs_mempll		= mt_fh_hal_l2h_dvfs_mempll;
	g_fh_hal_drv.mt_h2l_dvfs_mempll		= mt_fh_hal_h2l_dvfs_mempll;
	g_fh_hal_drv.mt_dfs_armpll		= mt_fh_hal_dfs_armpll;
	g_fh_hal_drv.mt_dfs_mmpll		= mt_fh_hal_dfs_mmpll;
	g_fh_hal_drv.mt_dfs_vencpll		= mt_fh_hal_dfs_vencpll;
	g_fh_hal_drv.mt_is_support_DFS_mode		= mt_fh_hal_is_support_DFS_mode;
	g_fh_hal_drv.mt_dram_overclock		= mt_fh_hal_dram_overclock;
	g_fh_hal_drv.mt_get_dramc		= mt_fh_hal_get_dramc;
	g_fh_hal_drv.mt_fh_default_conf	= mt_fh_hal_default_conf;

	return (&g_fh_hal_drv);
}
//TODO: module_exit(cpufreq_exit);
