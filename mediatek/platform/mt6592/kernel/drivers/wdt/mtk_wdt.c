#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>

#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_wdt.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/aee.h>
#include <mach/sync_write.h>
#include <mach/ext_wd_drv.h>

#include <mach/wd_api.h>





//#undef  DRV_WriteReg32
//#define DRV_WriteReg32(a, v)			mt65xx_reg_sync_writel(v, a) //write32

extern void aee_wdt_printf(const char *fmt, ...);

/**---------------------------------------------------------------------
 * Sub feature switch region
 *----------------------------------------------------------------------
 */
#define NO_DEBUG 1

/*----------------------------------------------------------------------
 *   IRQ ID 
 *--------------------------------------------------------------------*/
#define AP_RGU_WDT_IRQ_ID    MT_WDT_IRQ_ID
//#define CONFIG_KICK_SPM_WDT
#define CONFIG_RGU_ECO

/* 
 * internal variables 
 */
//static char expect_close; // Not use
//static spinlock_t rgu_reg_operation_spinlock = SPIN_LOCK_UNLOCKED;
static DEFINE_SPINLOCK(rgu_reg_operation_spinlock);
#ifndef CONFIG_KICK_SPM_WDT
static unsigned int timeout;
#endif
static volatile BOOL  rgu_wdt_intr_has_trigger; // For test use
static int g_last_time_time_out_value = 0;
static int g_wdt_enable= 1;
#ifdef CONFIG_KICK_SPM_WDT
#include <mach/mt_spm.h>
static void spm_wdt_init(void);

#endif

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
/*
    this function set the timeout value.
    value: second
*/

static void mtk_wdt_set_time_out_value_init(unsigned int value)
{
	/*
	 * TimeOut = BitField 15:5
	 * Key	   = BitField  4:0 = 0x08
	 */	
	spin_lock(&rgu_reg_operation_spinlock);
	
    #ifdef CONFIG_KICK_SPM_WDT		
	spm_wdt_set_timeout(value);	
    #else	
		
	// 1 tick means 512 * T32K -> 1s = T32/512 tick = 64
	// --> value * (1<<6)
	timeout = (unsigned int)(value * ( 1 << 6) );
	timeout = timeout << 5; 
	DRV_WriteReg32(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY) );
    #endif
	spin_unlock(&rgu_reg_operation_spinlock);
}

void mtk_wdt_set_time_out_value(unsigned int value)
{
	printk("mtk_wdt_set_time_out_value do nothing in mt6592\n");
}
/*
    watchdog mode:
    debug_en:   debug module reset enable. 
    irq:        generate interrupt instead of reset
    ext_en:     output reset signal to outside
    ext_pol:    polarity of external reset signal
    wdt_en:     enable watch dog timer
*/
void mtk_wdt_mode_config(	BOOL dual_mode_en, 
					BOOL irq, 
					BOOL ext_en, 
					BOOL ext_pol, 
					BOOL wdt_en )
{
    #ifndef CONFIG_KICK_SPM_WDT	
	//unsigned int tmp; 
    #endif
	#ifdef CONFIG_RGU_ECO
	unsigned int tmp_eco; 
	#endif
	
	spin_lock(&rgu_reg_operation_spinlock);
	#ifdef CONFIG_KICK_SPM_WDT	
	if(wdt_en == TRUE){
		  printk("wdt enable spm timer.....\n");
		  spm_wdt_enable_timer();
	}
	else{
		  printk("wdt disable spm timer.....\n");
		  spm_wdt_disable_timer();
	}
	spin_unlock(&rgu_reg_operation_spinlock);
	return;
	#endif
	
	#ifdef CONFIG_RGU_ECO
	tmp_eco = DRV_Reg32(MTK_WDT_MODE_DUMMY);
	tmp_eco |= MTK_WDT_MODE_KEY;
	if(wdt_en == TRUE)
	{
		
		tmp_eco |= MTK_WDT_MODE_ENABLE;
		DRV_WriteReg32(MTK_WDT_MODE_DUMMY,tmp_eco);
		printk("wdt enable rgu eco(%x)\n",tmp_eco);
		
	}
	else
	{
		
		tmp_eco &= ~MTK_WDT_MODE_ENABLE;
		DRV_WriteReg32(MTK_WDT_MODE_DUMMY,tmp_eco);
		printk("wdt disable rgu eco.....\n");
		
	}
	spin_unlock(&rgu_reg_operation_spinlock);
	return;
    #else
	//printk(" mtk_wdt_mode_config  mode value=%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE),current->pid);
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	// Bit 0 : Whether enable watchdog or not
	if(wdt_en == TRUE)
	{
		tmp |= MTK_WDT_MODE_ENABLE;
	}
	else
	{
		tmp &= ~MTK_WDT_MODE_ENABLE;
	}

	// Bit 1 : Configure extern reset signal polarity.
	if(ext_pol == TRUE)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	// Bit 2 : Whether enable external reset signal
	if(ext_en == TRUE)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	// Bit 3 : Whether generating interrupt instead of reset signal
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;

	// Bit 6 : Whether enable debug module reset
	if(dual_mode_en == TRUE)
		tmp |= MTK_WDT_MODE_DUAL_MODE;
	else
		tmp &= ~MTK_WDT_MODE_DUAL_MODE;


	DRV_WriteReg32(MTK_WDT_MODE,tmp);

	printk(" mtk_wdt_mode_config  mode value=%x, tmp:%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE), tmp,current->pid);
	//set auto restart bit
	//MTK_WDT_NONRST_REG2 Bit 1: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
	//Because HW reboot always need reboot to kernel, we set it always.
	tmp = DRV_Reg32(MTK_WDT_NONRST_REG2);
	tmp |= MTK_WDT_AUTO_RESTART_FLAG;
	DRV_WriteReg32(MTK_WDT_NONRST_REG2,tmp);
	
	#endif
	spin_unlock(&rgu_reg_operation_spinlock);
}
//EXPORT_SYMBOL(mtk_wdt_mode_config);

static void mtk_wdt_mode_config_init_use(	BOOL dual_mode_en, 
					BOOL irq, 
					BOOL ext_en, 
					BOOL ext_pol, 
					BOOL wdt_en )
{
    #ifndef CONFIG_KICK_SPM_WDT	
	unsigned int tmp; 
    #endif

	#ifdef CONFIG_RGU_ECO
	unsigned int tmp_eco; 
	#endif
	
	spin_lock(&rgu_reg_operation_spinlock);
	#ifdef CONFIG_KICK_SPM_WDT	
	if(wdt_en == TRUE){
		  printk("wdt enable spm timer.....\n");
		  spm_wdt_enable_timer();
	}
	else{
		  printk("wdt disable spm timer.....\n");
		  spm_wdt_disable_timer();
	}
    #else	
	//printk(" mtk_wdt_mode_config  mode value=%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE),current->pid);
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	#ifdef CONFIG_RGU_ECO
	tmp_eco = DRV_Reg32(MTK_WDT_MODE_DUMMY);
	tmp_eco |= MTK_WDT_MODE_KEY;
	#endif

	// Bit 0 : Whether enable watchdog or not
	if(wdt_en == TRUE)
	{
		#ifdef CONFIG_RGU_ECO
		tmp_eco |= MTK_WDT_MODE_ENABLE;
		tmp |= MTK_WDT_MODE_DUMMY_RESTART;
		printk("wdt enable rgu eco(%x)\n",tmp_eco);
		#else
		tmp |= MTK_WDT_MODE_ENABLE;
		#endif
	}
	else
	{
		#ifdef CONFIG_RGU_ECO
		tmp_eco &= ~MTK_WDT_MODE_ENABLE;
		printk("wdt disable rgu eco.....\n");
		#else
		tmp &= ~MTK_WDT_MODE_ENABLE;
		#endif
	}

	// Bit 1 : Configure extern reset signal polarity.
	if(ext_pol == TRUE)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	// Bit 2 : Whether enable external reset signal
	if(ext_en == TRUE)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	// Bit 3 : Whether generating interrupt instead of reset signal
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;

	// Bit 6 : Whether enable debug module reset
	if(dual_mode_en == TRUE)
		tmp |= MTK_WDT_MODE_DUAL_MODE;
	else
		tmp &= ~MTK_WDT_MODE_DUAL_MODE;


	DRV_WriteReg32(MTK_WDT_MODE,tmp);

	#ifdef CONFIG_RGU_ECO
	DRV_WriteReg32(MTK_WDT_MODE_DUMMY,tmp_eco);
	#endif

	printk(" mtk_wdt_mode_config  mode value=%x, tmp:%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE), tmp,current->pid);
	//set auto restart bit
	//MTK_WDT_NONRST_REG2 Bit 1: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
	//Because HW reboot always need reboot to kernel, we set it always.
	tmp = DRV_Reg32(MTK_WDT_NONRST_REG2);
	tmp |= MTK_WDT_AUTO_RESTART_FLAG;
	DRV_WriteReg32(MTK_WDT_NONRST_REG2,tmp);
	
	#endif
	spin_unlock(&rgu_reg_operation_spinlock);
}



int mtk_wdt_enable(enum wk_wdt_en en)
{
	unsigned int tmp =0;
	#ifdef CONFIG_RGU_ECO
	unsigned int tmp_eco; 
	#endif
    
	spin_lock(&rgu_reg_operation_spinlock);
	
    #ifdef CONFIG_KICK_SPM_WDT
	if(WK_WDT_EN ==en)
	{
		spm_wdt_enable_timer();
		printk("wdt enable spm timer\n");
		
		tmp = DRV_Reg32(MTK_WDT_REQ_MODE);
	    tmp |=  MTK_WDT_REQ_MODE_KEY;
	    tmp |= (MTK_WDT_REQ_MODE_SPM_SCPSYS);
	    DRV_WriteReg32(MTK_WDT_REQ_MODE,tmp);
		g_wdt_enable = 1;
	}
	if(WK_WDT_DIS==en)
	{
	    spm_wdt_disable_timer();
		printk("wdt disable spm timer\n ");
		tmp = DRV_Reg32(MTK_WDT_REQ_MODE);
	    tmp |=  MTK_WDT_REQ_MODE_KEY;
	    tmp &= ~(MTK_WDT_REQ_MODE_SPM_SCPSYS);
	    DRV_WriteReg32(MTK_WDT_REQ_MODE,tmp);
		g_wdt_enable = 0;
	}
    #else

	#ifdef CONFIG_RGU_ECO
	tmp_eco = DRV_Reg32(MTK_WDT_MODE_DUMMY);
	tmp_eco |= MTK_WDT_MODE_KEY;
	#else
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;
	#endif
	if(WK_WDT_EN == en)
	{
		#ifdef CONFIG_RGU_ECO
		tmp_eco |= MTK_WDT_MODE_ENABLE;
		printk("wdt enable rgu eco(%x)\n",tmp_eco);
		#else
	  	tmp |= MTK_WDT_MODE_ENABLE;
		#endif
	  	g_wdt_enable = 1;
	 
	}
	if(WK_WDT_DIS == en)
	{
		#ifdef CONFIG_RGU_ECO
		tmp_eco &= ~MTK_WDT_MODE_ENABLE;
		printk("wdt disable rgu eco.....\n");
		#else
	  	tmp &= ~MTK_WDT_MODE_ENABLE;
		#endif
	  	g_wdt_enable = 0;
	}
    printk("mtk_wdt_enable value=%x,pid=%d\n",tmp,current->pid);
	#ifdef CONFIG_RGU_ECO
	DRV_WriteReg32(MTK_WDT_MODE_DUMMY,tmp_eco);
	#else
	DRV_WriteReg32(MTK_WDT_MODE,tmp);
	#endif
    #endif //end spm
	spin_unlock(&rgu_reg_operation_spinlock);

	return 0;
}
int  mtk_wdt_confirm_hwreboot(void)
{
    //aee need confirm wd can hw reboot
    //printk("mtk_wdt_probe : Initialize to dual mode \n");
	//mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
	return 0;
}

void mtk_wdt_restart(enum wd_restart_type type)
{

    //printk("WDT:[mtk_wdt_restart] type  =%d, pid=%d\n",type,current->pid);

	#ifdef CONFIG_RGU_ECO
	UINT32 tmp;
	if(type == WD_TYPE_NORMAL) 
	{
		spin_lock(&rgu_reg_operation_spinlock); 
    	tmp = DRV_Reg32(MTK_WDT_RESTART_DUMMY);
		tmp = ~tmp;
		tmp = tmp & MTK_WDT_RESTART_BIT;
		tmp |= MTK_WDT_RESTART_KEY;
		DRV_WriteReg32(MTK_WDT_RESTART_DUMMY, tmp);
		spin_unlock(&rgu_reg_operation_spinlock);
	}
	else if(type == WD_TYPE_NOLOCK) 
	{
		tmp = *(volatile u32 *)( MTK_WDT_RESTART_DUMMY);
		tmp = ~tmp;
		tmp = tmp & MTK_WDT_RESTART_BIT;
		tmp |= MTK_WDT_RESTART_KEY;
		*(volatile u32 *)( MTK_WDT_RESTART_DUMMY) = tmp;
	}
	else
	{
	}
	#else
	if(type == WD_TYPE_NORMAL) 
	{     
	    //printk("WDT:ext restart\n" );
	    spin_lock(&rgu_reg_operation_spinlock); 
        #ifdef CONFIG_KICK_SPM_WDT
		spm_wdt_restart_timer();
        #else		
	    DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
        #endif
	    spin_unlock(&rgu_reg_operation_spinlock);
	}
	else if(type == WD_TYPE_NOLOCK) 
	{
	    #ifdef CONFIG_KICK_SPM_WDT
		spm_wdt_restart_timer_nolock();
        #else
	    *(volatile u32 *)( MTK_WDT_RESTART) =MTK_WDT_RESTART_KEY ;
		#endif
	}
	else
	{
	    //printk("WDT:[mtk_wdt_restart] type error pid =%d\n",type,current->pid);
	}
	#endif
}

void wdt_dump_reg(void)
{
	printk("****************dump wdt reg start*************\n");
	printk("MTK_WDT_MODE:0x%x\n", DRV_Reg32(MTK_WDT_MODE));
	printk("MTK_WDT_LENGTH:0x%x\n", DRV_Reg32(MTK_WDT_LENGTH));
	printk("MTK_WDT_RESTART:0x%x\n", DRV_Reg32(MTK_WDT_RESTART));
	printk("MTK_WDT_STATUS:0x%x\n", DRV_Reg32(MTK_WDT_STATUS));
	printk("MTK_WDT_INTERVAL:0x%x\n", DRV_Reg32(MTK_WDT_INTERVAL));
	printk("MTK_WDT_SWRST:0x%x\n", DRV_Reg32(MTK_WDT_SWRST));
	printk("MTK_WDT_NONRST_REG:0x%x\n", DRV_Reg32(MTK_WDT_NONRST_REG));
	printk("MTK_WDT_NONRST_REG2:0x%x\n", DRV_Reg32(MTK_WDT_NONRST_REG2));
	printk("MTK_WDT_REQ_MODE:0x%x\n", DRV_Reg32(MTK_WDT_REQ_MODE));
	printk("MTK_WDT_REQ_IRQ_EN:0x%x\n", DRV_Reg32(MTK_WDT_REQ_IRQ_EN));
	printk("MTK_WDT_DRAMC_CTL:0x%x\n", DRV_Reg32(MTK_WDT_DRAMC_CTL));
	printk("****************dump wdt reg end*************\n");
	
}


void wdt_arch_reset(char mode)
{

    unsigned int wdt_mode_val;
	unsigned int wdt_non_reg2;
	printk("wdt_arch_reset called@Kernel mode =%c\n",mode);
	
	spin_lock(&rgu_reg_operation_spinlock);
	/* Watchdog Rest */
	DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
	wdt_mode_val = DRV_Reg32(MTK_WDT_MODE);
	printk("wdt_arch_reset called MTK_WDT_MODE =%x \n",wdt_mode_val);
	/* clear autorestart bit: autoretart: 1, bypass power key, 0: not bypass power key */
	wdt_non_reg2 = DRV_Reg32(MTK_WDT_NONRST_REG2);
	wdt_non_reg2 &=(~MTK_WDT_AUTO_RESTART_FLAG);
	
	
	/* make sure WDT mode is hw reboot mode, can not config isr mode  */
	wdt_mode_val &=(~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE | MTK_WDT_MODE_DUAL_MODE));
	if(mode)
	{
		/* mode != 0 means by pass power key reboot, We using auto_restart bit as by pass power key flag */
		wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
		wdt_non_reg2 = wdt_non_reg2 | MTK_WDT_AUTO_RESTART_FLAG;
	}else
	{
	       wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
		 
	}
	// disable ddr reserve mode;
	wdt_mode_val &= (~MTK_WDT_MODE_DDR_RESERVE);

	DRV_WriteReg32(MTK_WDT_MODE,wdt_mode_val);
	printk("wdt_arch_reset called end  MTK_WDT_MODE =%x \n",wdt_mode_val);
	
	DRV_WriteReg32(MTK_WDT_NONRST_REG2,wdt_non_reg2);
	
	udelay(100);
	DRV_WriteReg32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
    printk("wdt_arch_reset: SW_reset happen\n");
	spin_unlock(&rgu_reg_operation_spinlock);

	while (1)
	{
	    wdt_dump_reg();
		printk("wdt_arch_reset error\n");
	}
	
}

static int mtk_wdt_dram_reserved(int enable)
{
	volatile unsigned int tmp, ret = 0;
	if (1 == enable)
	{	/* enable ddr reserved mode */
		tmp = DRV_Reg32(MTK_WDT_MODE);
		tmp |= (MTK_WDT_MODE_DDR_RESERVE|MTK_WDT_MODE_KEY);
		DRV_WriteReg32(MTK_WDT_MODE, tmp);
	} 
	else if (0 == enable)
	{
		/* disable ddr reserved mode, set reset mode,
		disable watchdog output reset signal */
		tmp = DRV_Reg32(MTK_WDT_MODE);
		tmp &= (~MTK_WDT_MODE_DDR_RESERVE);
		tmp |= MTK_WDT_MODE_KEY;
		DRV_WriteReg32(tmp, MTK_WDT_MODE);
	} 
	else
	{
			printk("Wrong input %d, should be 1(enable) or 0(disable) in %s\n", enable, __func__);
			ret = -1;
	}
	printk("RGU %s:MTK_WDT_MODE(%x)\n", __func__,tmp);
	return ret;
}

int mtk_wdt_swsysret_config(int bit,int set_value)
{
    unsigned int wdt_sys_val;
	spin_lock(&rgu_reg_operation_spinlock);
	wdt_sys_val = DRV_Reg32(MTK_WDT_SWSYSRST);
	printk("fwq2 before set wdt_sys_val =%x\n",wdt_sys_val);
    wdt_sys_val |= MTK_WDT_SWSYS_RST_KEY;
	switch(bit)
	{
	  case MTK_WDT_SWSYS_RST_MD_RST:
	  	   if(1==set_value)
	  	   {
	  	     wdt_sys_val |= MTK_WDT_SWSYS_RST_MD_RST;
	  	   }
		   if(0==set_value)
		   {
		     wdt_sys_val &= ~MTK_WDT_SWSYS_RST_MD_RST;
		   }
	  	break;
	 case MTK_WDT_SWSYS_RST_MD_LITE_RST:
	  	   if(1==set_value)
	  	   {
	  	     wdt_sys_val |= MTK_WDT_SWSYS_RST_MD_LITE_RST;
	  	   }
		   if(0==set_value)
		   {
		     wdt_sys_val &= ~MTK_WDT_SWSYS_RST_MD_LITE_RST;
		   }
	  	break;
	case 0x10000000:
			// aee need config ddr reserve mode
			mtk_wdt_dram_reserved(set_value);
		break;
	  	   
	}
	DRV_WriteReg32(MTK_WDT_SWSYSRST,wdt_sys_val);
	spin_unlock(&rgu_reg_operation_spinlock);

	mdelay(10);
	printk("after set wdt_sys_val =%x,wdt_sys_val=%x\n",DRV_Reg32(MTK_WDT_SWSYSRST),wdt_sys_val);
    return 0;
}

int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en)
{
    int res=0;
    unsigned int tmp;
	spin_lock(&rgu_reg_operation_spinlock);
	tmp = DRV_Reg32(MTK_WDT_REQ_MODE);
	tmp |=  MTK_WDT_REQ_MODE_KEY;
	
    if(MTK_WDT_REQ_MODE_SPM_SCPSYS == mark_bit)
    {
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_SPM_SCPSYS);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_SPM_SCPSYS);
		}
    }
	else if(MTK_WDT_REQ_MODE_SPM_THERMAL == mark_bit)
	{
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_SPM_THERMAL);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_SPM_THERMAL);
		}
	}
	else
	{
		    res =-1;
	}
      
   DRV_WriteReg32(MTK_WDT_REQ_MODE,tmp);
   spin_unlock(&rgu_reg_operation_spinlock);
   return res;
   
}

int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode)
{
    int res=0;
    unsigned int tmp;
	spin_lock(&rgu_reg_operation_spinlock);
	tmp = DRV_Reg32(MTK_WDT_REQ_IRQ_EN);
	tmp |=  MTK_WDT_REQ_IRQ_KEY;
    if(MTK_WDT_REQ_MODE_SPM_SCPSYS == mark_bit)
    {
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
		}
    }
	else if ( MTK_WDT_REQ_MODE_SPM_THERMAL == mark_bit)
	{
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_SPM_THERMAL_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_SPM_THERMAL_EN);
		}
	}
	else
	{
		res =-1;
    }
  
   DRV_WriteReg32(MTK_WDT_REQ_IRQ_EN,tmp);
   spin_unlock(&rgu_reg_operation_spinlock);
   return res;
}

// diglitch patch 

static int mtk_wdt_deglitch_en(void)
{
    unsigned int tmp=0;
	tmp =  MTK_WDT_DEGLITCH_KEY;
	DRV_WriteReg32(MTK_WDT_DEGLITCH_EN,tmp);
	return 0;
}	
#else 
//-------------------------------------------------------------------------------------------------
//      Dummy functions
//-------------------------------------------------------------------------------------------------
void mtk_wdt_set_time_out_value(unsigned int value){}
static void mtk_wdt_set_reset_length(unsigned int value){}
void mtk_wdt_mode_config(BOOL dual_mode_en,BOOL irq,	BOOL ext_en, BOOL ext_pol, BOOL wdt_en){}
int mtk_wdt_enable(enum wk_wdt_en en){ return 0;}
void mtk_wdt_restart(enum wd_restart_type type){}
static void mtk_wdt_sw_trigger(void){}
static unsigned char mtk_wdt_check_status(void){ return 0;}
void wdt_arch_reset(char mode){}
int  mtk_wdt_confirm_hwreboot(void){return 0;}
void mtk_wd_suspend(void){}
void mtk_wd_resume(void){}
void wdt_dump_reg(void){}
int mtk_wdt_swsysret_config(int bit,int set_value){ return 0;}
int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode){return 0;}
int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en){return 0;}








#endif //#ifndef __USING_DUMMY_WDT_DRV__

#ifndef CONFIG_FIQ_GLUE
static void wdt_report_info (void)
{
    //extern struct task_struct *wk_tsk;
    struct task_struct *task ;
    task = &init_task ;
    
    printk ("Qwdt: -- watchdog time out\n") ;
    for_each_process (task)
    {
        if (task->state == 0)
        {
            printk ("PID: %d, name: %s\n backtrace:\n", task->pid, task->comm) ;
            show_stack (task, NULL) ;
            printk ("\n") ;
        }
    }
    
    
    printk ("backtrace of current task:\n") ;
    show_stack (NULL, NULL) ;
    
       
    printk ("Qwdt: -- watchdog time out\n") ;    
}
#endif


#ifdef CONFIG_FIQ_GLUE
static void wdt_fiq(void *arg, void *regs, void *svc_sp)
{
	unsigned int wdt_mode_val;
	struct wd_api*wd_api = NULL;
    get_wd_api(&wd_api);
	wdt_mode_val = DRV_Reg32(MTK_WDT_STATUS);
	DRV_WriteReg32(MTK_WDT_NONRST_REG, wdt_mode_val);
    #ifdef	CONFIG_MTK_WD_KICKER
	aee_wdt_printf("kick=0x%08x,check=0x%08x",wd_api->wd_get_kick_bit(),wd_api->wd_get_check_bit());
    #endif 

     aee_wdt_fiq_info(arg, regs, svc_sp);
#if 0
     asm volatile("mov %0, %1\n\t"
                  "mov fp, %2\n\t"
                 : "=r" (sp)
                 : "r" (svc_sp), "r" (preg[11])
                 );

     *((volatile unsigned int*)(0x00000000)); /* trigger exception */
#endif
}
#else //CONFIG_FIQ_GLUE
static irqreturn_t mtk_wdt_isr(int irq, void *dev_id)

{
    printk("fwq mtk_wdt_isr\n" );
#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
	//mt65xx_irq_mask(AP_RGU_WDT_IRQ_ID);
	rgu_wdt_intr_has_trigger = 1;
	wdt_report_info () ;
	BUG();

#endif	
	return IRQ_HANDLED;
}
#endif //CONFIG_FIQ_GLUE

/* 
 * Device interface 
 */
static int mtk_wdt_probe(struct platform_device *dev)
{
	int ret=0;
	unsigned int interval_val;
	
	printk("******** MTK WDT driver probe!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */

#ifndef CONFIG_FIQ_GLUE	
    printk("******** MTK WDT register irq ********\n" );
    #ifdef CONFIG_KICK_SPM_WDT
	ret = spm_wdt_register_irq((irq_handler_t)mtk_wdt_isr);
    #else    
	ret = request_irq(AP_RGU_WDT_IRQ_ID, (irq_handler_t)mtk_wdt_isr, IRQF_TRIGGER_FALLING, "mtk_watchdog", NULL);
    #endif		//CONFIG_KICK_SPM_WDT		
#else
    printk("******** MTK WDT register fiq ********\n" );
    #ifdef CONFIG_KICK_SPM_WDT
	ret = spm_wdt_register_fiq(wdt_fiq);
    #else    
	ret = request_fiq(AP_RGU_WDT_IRQ_ID, wdt_fiq, IRQF_TRIGGER_FALLING, NULL);
    #endif		//CONFIG_KICK_SPM_WDT
#endif	

    if(ret != 0)
	{
		printk( "mtk_wdt_probe : failed to request irq (%d)\n", ret);
		return ret;
	}
	printk("mtk_wdt_probe : Success to request irq\n");
	
    #ifdef CONFIG_KICK_SPM_WDT
	spm_wdt_init();
    #endif		

	/* Set timeout vale and restart counter */
	g_last_time_time_out_value=30;
	mtk_wdt_set_time_out_value_init(g_last_time_time_out_value);
		
	mtk_wdt_restart(WD_TYPE_NORMAL);

	/**
	 * Set the reset lenght: we will set a special magic key.
	 * For Power off and power on reset, the INTERVAL default value is 0x7FF.
	 * We set Interval[1:0] to different value to distinguish different stage.
	 * Enter pre-loader, we will set it to 0x0
	 * Enter u-boot, we will set it to 0x1
	 * Enter kernel, we will set it to 0x2
	 * And the default value is 0x3 which means reset from a power off and power on reset
	 */
	#define POWER_OFF_ON_MAGIC	(0x3)
	#define PRE_LOADER_MAGIC	(0x0)
    #define U_BOOT_MAGIC		(0x1)
	#define KERNEL_MAGIC		(0x2)
	#define MAGIC_NUM_MASK		(0x3)


#ifdef  CONFIG_MTK_WD_KICKER	// Initialize to dual mode
	printk("mtk_wdt_probe : Initialize to dual mode \n");
	#ifdef CONFIG_RGU_ECO
	mtk_wdt_mode_config_init_use(TRUE, TRUE, TRUE, FALSE, TRUE);
	#else
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
	#endif
#else				// Initialize to disable wdt
	printk("mtk_wdt_probe : Initialize to disable wdt \n");
	#ifdef CONFIG_RGU_ECO
	mtk_wdt_mode_config_init_use(FALSE, FALSE, TRUE, FALSE, FALSE);
	#else
	mtk_wdt_mode_config(FALSE, FALSE, TRUE, FALSE, FALSE);
	#endif
	g_wdt_enable =0;
#endif
	mtk_wdt_restart(WD_TYPE_NORMAL);

	/* Update interval register value and check reboot flag */
	interval_val = DRV_Reg32(MTK_WDT_INTERVAL);
	interval_val &= ~(MAGIC_NUM_MASK);
	interval_val |= (KERNEL_MAGIC);
	/* Write back INTERVAL REG */
	DRV_WriteReg32(MTK_WDT_INTERVAL, interval_val);
#endif

	mtk_wdt_deglitch_en();
	udelay(100);
  	printk("mtk_wdt_probe : done WDT_MODE(%x) WDT_MODE_DUMMY(%x) MTK_WDT_DEGLITCH_EN(%x)\n",DRV_Reg32(MTK_WDT_MODE),DRV_Reg32(MTK_WDT_MODE_DUMMY),DRV_Reg32(MTK_WDT_DEGLITCH_EN));
	printk("mtk_wdt_probe : done MTK_WDT_REQ_MODE(%x)\n",DRV_Reg32(MTK_WDT_REQ_MODE));
	printk("mtk_wdt_probe : done MTK_WDT_REQ_IRQ_EN(%x)\n",DRV_Reg32(MTK_WDT_REQ_IRQ_EN));

	return ret;
}

static int mtk_wdt_remove(struct platform_device *dev)
{
	printk("******** MTK wdt driver remove!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
	free_irq(AP_RGU_WDT_IRQ_ID, NULL);
#endif
	return 0;
}

static void mtk_wdt_shutdown(struct platform_device *dev)
{	
	printk("******** MTK WDT driver shutdown!! ********\n" );

	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	//kick external wdt
	//mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, FALSE);

	mtk_wdt_restart(WD_TYPE_NORMAL);

   printk("******** MTK WDT driver shutdown done ********\n" );
}

void mtk_wd_suspend(void)
{
	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, dis wdt
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, FALSE);

	mtk_wdt_restart(WD_TYPE_NORMAL);

	aee_sram_printk("[WDT] suspend\n");
	printk("[WDT] suspend\n");
}

void mtk_wd_resume(void)
{
	
	if ( g_wdt_enable == 1 ) 
	{
		//mtk_wdt_set_time_out_value(g_last_time_time_out_value);
		mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
		mtk_wdt_restart(WD_TYPE_NORMAL);
		
	}

	aee_sram_printk("[WDT] resume(%d)\n", g_wdt_enable);
	printk("[WDT] resume(%d)\n", g_wdt_enable);
}



static struct platform_driver mtk_wdt_driver =
{
	.driver     = {
		.name	= "mtk-wdt",
	},
	.probe	= mtk_wdt_probe,
	.remove	= mtk_wdt_remove,
	.shutdown	= mtk_wdt_shutdown,
//	.suspend	= mtk_wdt_suspend,
//	.resume	= mtk_wdt_resume,
};

struct platform_device mtk_device_wdt = {
		.name		= "mtk-wdt",
		.id		= 0,
		.dev		= {
		}
};

#ifdef CONFIG_KICK_SPM_WDT
static void spm_wdt_init(void)
{

   unsigned int tmp;
   // set scpsys reset mode , not trigger irq
   tmp = DRV_Reg32(MTK_WDT_REQ_MODE);
   tmp |=  MTK_WDT_REQ_MODE_KEY;
   tmp |= (MTK_WDT_REQ_MODE_SPM_SCPSYS);
   DRV_WriteReg32(MTK_WDT_REQ_MODE,tmp);

   tmp = DRV_Reg32(MTK_WDT_REQ_IRQ_EN);
   tmp |= MTK_WDT_REQ_IRQ_KEY;
   tmp &= ~(MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
   DRV_WriteReg32(MTK_WDT_REQ_IRQ_EN,tmp);
   
	   
   printk( "mtk_wdt_init [MTK_WDT] not use RGU WDT use_SPM_WDT!! ********\n" );
	   
   tmp = DRV_Reg32(MTK_WDT_MODE);
   tmp |= MTK_WDT_MODE_KEY;
   //disable wdt
   tmp &= (~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE|MTK_WDT_MODE_DUAL_MODE));
	   
   // Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
   //	   Because HW reboot always need reboot to kernel, we set it always.
   tmp |= MTK_WDT_MODE_AUTO_RESTART;
   //BIt2  ext singal
   tmp |= MTK_WDT_MODE_EXTEN;
   DRV_WriteReg32(MTK_WDT_MODE,tmp);
	   
}
#endif


/*
 * init and exit function
 */
static int __init mtk_wdt_init(void)
{

	int ret;

	ret = platform_device_register(&mtk_device_wdt);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to device register(%d)\n", ret);
		return ret;
	}
	ret = platform_driver_register(&mtk_wdt_driver);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}
    printk("mtk_wdt_init ok\n");
	return 0;	
}

static void __exit mtk_wdt_exit (void)
{
}

module_init(mtk_wdt_init);
module_exit(mtk_wdt_exit);

MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("MTK Watchdog Device Driver");
MODULE_LICENSE("GPL");

