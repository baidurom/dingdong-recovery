#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include "mach/dma.h"
#include <mach/sync_write.h>
#include <mach/mt_irq.h>
#include "mach/mtk_thermal_monitor.h"
#include <mach/system.h>
//#include "mach/mtk_cpu_management.h"

#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"
//#include "mach/mt_gpufreq.h"
#include <mach/mt_clkmgr.h>
//#include <mach/mt_spm.h>

// 1: turn on RT kthread for thermal protection in this sw module; 0: turn off
#define MTK_TS_CPU_RT                     (1)

#if MTK_TS_CPU_RT
#include <linux/sched.h>
#include <linux/kthread.h>
#endif

// 1: turn on adaptive AP cooler; 0: turn off
#define CPT_ADAPTIVE_AP_COOLER          (1)

// 1: turn on supports to MET logging; 0: turn off
#define CONFIG_SUPPORT_MET_MTKTSCPU     (0)

// Thermal controller HW filtering function. Only 1, 2, 4, 8, 16 are valid values, they means one reading is a avg of X samples
#define THERMAL_CONTROLLER_HW_FILTER    (8) // 1, 2, 4, 8, 16

// 1: turn on thermal controller HW thermal protection; 0: turn off
#define THERMAL_CONTROLLER_HW_TP        (1)

// 1: turn on SW filtering in this sw module; 0: turn off
#define MTK_TS_CPU_SW_FILTER            (1)

//Only for debug ,SODI,SPM deep idle, normal,SPM suspend,resume status
#define MTK_TS_DEBUG_LOG                (0)

// 1: turn on fast polling in this sw module; 0: turn off
#define MTKTSCPU_FAST_POLLING           (1)

#if CPT_ADAPTIVE_AP_COOLER
#define MAX_CPT_ADAPTIVE_COOLERS        (3)
#endif

#define MIN(_a_, _b_) ((_a_) > (_b_) ? (_b_) : (_a_))
#define MAX(_a_, _b_) ((_a_) > (_b_) ? (_a_) : (_b_))

#if CPT_ADAPTIVE_AP_COOLER
static int g_curr_temp = 0;
static int g_prev_temp = 0;
#endif

#if MTKTSCPU_FAST_POLLING
// Combined fast_polling_trip_temp and fast_polling_factor, it means polling_delay will be 1/3 of original interval after mtktscpu reports > 85C w/o exit point
static int fast_polling_trip_temp = 85000;
static int fast_polling_factor = 3;
static int cur_fp_factor = 1;
static int next_fp_factor = 1;
#endif

static int g_Tj_temp = 0;

static unsigned int interval = 0; /* mseconds, 0 : no auto polling */
// trip_temp[0] must be initialized to the thermal HW protection point.
static int trip_temp[10] = {117000,110000,100000,90000,80000,70000,65000,60000,55000,50000};

static unsigned int *cl_dev_state;
static unsigned int cl_dev_sysrst_state=0;
#if CPT_ADAPTIVE_AP_COOLER
static unsigned int cl_dev_adp_cpu_state[MAX_CPT_ADAPTIVE_COOLERS] = {0};
static unsigned int cl_dev_adp_cpu_state_active = 0;
#endif
static struct thermal_zone_device *thz_dev;

static struct thermal_cooling_device **cl_dev = NULL;
static struct thermal_cooling_device *cl_dev_sysrst = NULL;
#if CPT_ADAPTIVE_AP_COOLER
static struct thermal_cooling_device *cl_dev_adp_cpu[MAX_CPT_ADAPTIVE_COOLERS] = {NULL};
#endif

#if CPT_ADAPTIVE_AP_COOLER
static int TARGET_TJS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int PACKAGE_THETA_JA_RISES[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int PACKAGE_THETA_JA_FALLS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int MINIMUM_CPU_POWERS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int MAXIMUM_CPU_POWERS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int MINIMUM_GPU_POWERS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int MAXIMUM_GPU_POWERS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int FIRST_STEP_TOTAL_POWER_BUDGETS[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
static int MINIMUM_BUDGET_CHANGES[MAX_CPT_ADAPTIVE_COOLERS] = { 0 };
#endif

#if MTK_TS_DEBUG_LOG
static int mtkts_suspend_counter = 0;
static int mtkts_resume_counter = 0;
extern unsigned long dpidle_cnt[8];//deep idle counter
extern unsigned long mcidle_cnt[8];//sodi counter
#endif



static int mtktscpu_debug_log = 0;
static int kernelmode = 0;
static int g_THERMAL_TRIP[10] = {0,0,0,0,0,0,0,0,0,0};

static int Temp_TS2=0, Temp_TS3=0, Temp_TS4=0;
static int RAW_TS2=0, RAW_TS3=0, RAW_TS4=0;

static int num_trip=10;

int MA_len_temp=0;
static int proc_write_flag=0;
static char *cooler_name;
#define CPU_COOLER_NUM 34

static DEFINE_MUTEX(TS_lock);


#if CPT_ADAPTIVE_AP_COOLER
static char adaptive_cooler_name[] = "cpu_adaptive_";
#endif

static char g_bind0[20]="mtktscpu-sysrst";
static char g_bind1[20]="2300";
static char g_bind2[20]="2100";
static char g_bind3[20]="1900";
static char g_bind4[20]="1700";
static char g_bind5[20]="1500";
static char g_bind6[20]="1300";
static char g_bind7[20]="1100";
static char g_bind8[20]="900";
static char g_bind9[20]="700";

static int read_curr_temp;

#define MTKTSCPU_TEMP_CRIT 120000 /* 120.000 degree Celsius */

#if MTK_TS_CPU_RT
static struct task_struct *ktp_thread_handle = NULL;
#endif

static int tc_mid_trip = -275000;

#define mtktscpu_dprintk(fmt, args...)   \
do {                                    \
	if (mtktscpu_debug_log) {                \
		xlog_printk(ANDROID_LOG_INFO, "Power/CPU_Thermal", fmt, ##args); \
	}                                   \
} while(0)

#define mtktscpu_printk(fmt, args...)   \
do {                                    \
	xlog_printk(ANDROID_LOG_INFO, "Power/CPU_Thermal", fmt, ##args); \
} while(0)


extern void mt_cpufreq_thermal_protect(unsigned int limited_power);


extern u32 get_devinfo_with_index(u32 index);


extern void mt_gpufreq_thermal_protect(unsigned int limited_power);


extern unsigned int mt_socfreq_get_cur_gpufreq(void);





#if THERMAL_CONTROLLER_HW_TP
static kal_int32 temperature_to_raw_abb(kal_uint32 ret);
#endif
//static int last_cpu_t=0;
int last_abb_t=0;
int last_CPU1_t=0;
int last_CPU2_t=0;


//static kal_int32 g_adc_ge = 0;
//static kal_int32 g_adc_oe = 0;
static kal_int32 g_adc_ge_t = 0;
static kal_int32 g_adc_oe_t = 0;
//static kal_int32 g_corner_TS = 0;
//static kal_int32 g_o_vtsmcu1 = 0;
static kal_int32 g_o_vtsmcu2 = 0;
static kal_int32 g_o_vtsmcu3 = 0;
static kal_int32 g_o_vtsmcu4 = 0;
//static kal_int32 g_o_vtsabb = 0;
static kal_int32 g_degc_cali = 0;
//static kal_int32 g_adc_cali_en = 0;
static kal_int32 g_adc_cali_en_t = 0;
static kal_int32 g_o_slope = 0;
static kal_int32 g_o_slope_sign = 0;
static kal_int32 g_id = 0;

static kal_int32 g_ge = 0;
static kal_int32 g_oe = 0;
static kal_int32 g_gain = 0;

static kal_int32 g_x_roomt2 = 0;
static kal_int32 g_x_roomt3 = 0;
static kal_int32 g_x_roomt4 = 0;
//static kal_int32 g_x_roomtabb = 0;
static int Num_of_OPP=0;

#if 0
static int Num_of_GPU_OPP=1; //Set this value =1 for non-DVS GPU
#else //DVFS GPU
static int Num_of_GPU_OPP=0;
#endif


#define y_curr_repeat_times 1
#define THERMAL_NAME    "mtk-thermal"
//#define GPU_Default_POWER	456

struct mtk_cpu_power_info
{
	unsigned int cpufreq_khz;
	unsigned int cpufreq_ncpu;
	unsigned int cpufreq_power;
};
struct mtk_gpu_power_info
{
	unsigned int gpufreq_khz;
	unsigned int gpufreq_power;
};
static struct mtk_cpu_power_info *mtk_cpu_power = NULL;
static int tscpu_num_opp = 0;
static struct mtk_gpu_power_info *mtk_gpu_power = NULL;

static int tscpu_cpu_dmips[CPU_COOLER_NUM] = {0};
int mtktscpu_limited_dmips = 0;

static bool talking_flag=false;
static int temperature_switch=0;
void set_taklking_flag(bool flag)
{
	talking_flag = flag;
	mtktscpu_printk("talking_flag=%d\n", talking_flag);
	return;
}

static unsigned int adaptive_cpu_power_limit = 0x7FFFFFFF, static_cpu_power_limit = 0x7FFFFFFF;

static void set_adaptive_cpu_power_limit(unsigned int limit)
{
    unsigned int final_limit;
    adaptive_cpu_power_limit = (limit != 0) ? limit : 0x7FFFFFFF;
    final_limit = MIN(adaptive_cpu_power_limit, static_cpu_power_limit);
    mtktscpu_printk("set_adaptive_cpu_power_limit %d, T=%d,%d,%d\n", (final_limit != 0x7FFFFFFF) ? final_limit : 0, Temp_TS2,Temp_TS3,Temp_TS4);

    mt_cpufreq_thermal_protect((final_limit != 0x7FFFFFFF) ? final_limit : 0);
}

static void set_static_cpu_power_limit(unsigned int limit)
{
    unsigned int final_limit;
    static_cpu_power_limit = (limit != 0) ? limit : 0x7FFFFFFF;
    final_limit = MIN(adaptive_cpu_power_limit, static_cpu_power_limit);
    mtktscpu_printk("set_static_cpu_power_limit %d, T=%d,%d,%d\n", (final_limit != 0x7FFFFFFF) ? final_limit : 0, Temp_TS2,Temp_TS3,Temp_TS4);

    mt_cpufreq_thermal_protect((final_limit != 0x7FFFFFFF) ? final_limit : 0);
}


static unsigned int adaptive_gpu_power_limit = 0x7FFFFFFF, static_gpu_power_limit = 0x7FFFFFFF;


static void set_adaptive_gpu_power_limit(unsigned int limit)
{
    unsigned int final_limit;

    adaptive_gpu_power_limit = (limit != 0) ? limit : 0x7FFFFFFF;
    final_limit = MIN(adaptive_gpu_power_limit, static_gpu_power_limit);
    mtktscpu_printk("set_adaptive_gpu_power_limit %d\n", (final_limit != 0x7FFFFFFF) ? final_limit : 0);
    mt_gpufreq_thermal_protect((final_limit != 0x7FFFFFFF) ? final_limit : 0);
}

static void set_static_gpu_power_limit(unsigned int limit)
{
    unsigned int final_limit;

    static_gpu_power_limit = (limit != 0) ? limit : 0x7FFFFFFF;
    final_limit = MIN(adaptive_gpu_power_limit, static_gpu_power_limit);
    mtktscpu_printk("set_static_gpu_power_limit %d\n", (final_limit != 0x7FFFFFFF) ? final_limit : 0);
    mt_gpufreq_thermal_protect((final_limit != 0x7FFFFFFF) ? final_limit : 0);
}


// TODO: We also need a pair of setting functions for GPU power limit, which is not supported on MT6582.

int mtktscpu_thermal_clock_on(void)
{
	mtktscpu_dprintk("mtktscpu_thermal_clock_on\n");
    enable_clock(MT_CG_PERI_THERM, "THERMAL");
    return 0;
}

int mtktscpu_thermal_clock_off(void)
{
	mtktscpu_dprintk("mtktscpu_thermal_clock_off\n");
    disable_clock(MT_CG_PERI_THERM, "THERMAL");
    return 0;
}

void get_thermal_all_register(void)
{
	mtktscpu_dprintk("get_thermal_all_register\n");

	mtktscpu_dprintk("TEMPMSR1    		  = 0x%8x\n", DRV_Reg32(TEMPMSR1));
	mtktscpu_dprintk("TEMPMSR2            = 0x%8x\n", DRV_Reg32(TEMPMSR2));

	mtktscpu_dprintk("TEMPMONCTL0   	  = 0x%8x\n", DRV_Reg32(TEMPMONCTL0));
	mtktscpu_dprintk("TEMPMONCTL1   	  = 0x%8x\n", DRV_Reg32(TEMPMONCTL1));
	mtktscpu_dprintk("TEMPMONCTL2   	  = 0x%8x\n", DRV_Reg32(TEMPMONCTL2));
	mtktscpu_dprintk("TEMPMONINT    	  = 0x%8x\n", DRV_Reg32(TEMPMONINT));
	mtktscpu_dprintk("TEMPMONINTSTS 	  = 0x%8x\n", DRV_Reg32(TEMPMONINTSTS));
	mtktscpu_dprintk("TEMPMONIDET0  	  = 0x%8x\n", DRV_Reg32(TEMPMONIDET0));

	mtktscpu_dprintk("TEMPMONIDET1  	  = 0x%8x\n", DRV_Reg32(TEMPMONIDET1));
	mtktscpu_dprintk("TEMPMONIDET2  	  = 0x%8x\n", DRV_Reg32(TEMPMONIDET2));
	mtktscpu_dprintk("TEMPH2NTHRE   	  = 0x%8x\n", DRV_Reg32(TEMPH2NTHRE));
	mtktscpu_dprintk("TEMPHTHRE     	  = 0x%8x\n", DRV_Reg32(TEMPHTHRE));
	mtktscpu_dprintk("TEMPCTHRE     	  = 0x%8x\n", DRV_Reg32(TEMPCTHRE));
	mtktscpu_dprintk("TEMPOFFSETH   	  = 0x%8x\n", DRV_Reg32(TEMPOFFSETH));

	mtktscpu_dprintk("TEMPOFFSETL   	  = 0x%8x\n", DRV_Reg32(TEMPOFFSETL));
	mtktscpu_dprintk("TEMPMSRCTL0   	  = 0x%8x\n", DRV_Reg32(TEMPMSRCTL0));
	mtktscpu_dprintk("TEMPMSRCTL1   	  = 0x%8x\n", DRV_Reg32(TEMPMSRCTL1));
	mtktscpu_dprintk("TEMPAHBPOLL   	  = 0x%8x\n", DRV_Reg32(TEMPAHBPOLL));
	mtktscpu_dprintk("TEMPAHBTO     	  = 0x%8x\n", DRV_Reg32(TEMPAHBTO));
	mtktscpu_dprintk("TEMPADCPNP0   	  = 0x%8x\n", DRV_Reg32(TEMPADCPNP0));

	mtktscpu_dprintk("TEMPADCPNP1   	  = 0x%8x\n", DRV_Reg32(TEMPADCPNP1));
	mtktscpu_dprintk("TEMPADCPNP2   	  = 0x%8x\n", DRV_Reg32(TEMPADCPNP2));
	mtktscpu_dprintk("TEMPADCMUX    	  = 0x%8x\n", DRV_Reg32(TEMPADCMUX));
	mtktscpu_dprintk("TEMPADCEXT    	  = 0x%8x\n", DRV_Reg32(TEMPADCEXT));
	mtktscpu_dprintk("TEMPADCEXT1   	  = 0x%8x\n", DRV_Reg32(TEMPADCEXT1));
	mtktscpu_dprintk("TEMPADCEN     	  = 0x%8x\n", DRV_Reg32(TEMPADCEN));


	mtktscpu_dprintk("TEMPPNPMUXADDR      = 0x%8x\n", DRV_Reg32(TEMPPNPMUXADDR));
	mtktscpu_dprintk("TEMPADCMUXADDR      = 0x%8x\n", DRV_Reg32(TEMPADCMUXADDR));
	mtktscpu_dprintk("TEMPADCEXTADDR      = 0x%8x\n", DRV_Reg32(TEMPADCEXTADDR));
	mtktscpu_dprintk("TEMPADCEXT1ADDR     = 0x%8x\n", DRV_Reg32(TEMPADCEXT1ADDR));
	mtktscpu_dprintk("TEMPADCENADDR       = 0x%8x\n", DRV_Reg32(TEMPADCENADDR));
	mtktscpu_dprintk("TEMPADCVALIDADDR    = 0x%8x\n", DRV_Reg32(TEMPADCVALIDADDR));

	mtktscpu_dprintk("TEMPADCVOLTADDR     = 0x%8x\n", DRV_Reg32(TEMPADCVOLTADDR));
	mtktscpu_dprintk("TEMPRDCTRL          = 0x%8x\n", DRV_Reg32(TEMPRDCTRL));
	mtktscpu_dprintk("TEMPADCVALIDMASK    = 0x%8x\n", DRV_Reg32(TEMPADCVALIDMASK));
	mtktscpu_dprintk("TEMPADCVOLTAGESHIFT = 0x%8x\n", DRV_Reg32(TEMPADCVOLTAGESHIFT));
	mtktscpu_dprintk("TEMPADCWRITECTRL    = 0x%8x\n", DRV_Reg32(TEMPADCWRITECTRL));
	mtktscpu_dprintk("TEMPMSR0            = 0x%8x\n", DRV_Reg32(TEMPMSR0));


	mtktscpu_dprintk("TEMPIMMD0           = 0x%8x\n", DRV_Reg32(TEMPIMMD0));
	mtktscpu_dprintk("TEMPIMMD1           = 0x%8x\n", DRV_Reg32(TEMPIMMD1));
	mtktscpu_dprintk("TEMPIMMD2           = 0x%8x\n", DRV_Reg32(TEMPIMMD2));
	mtktscpu_dprintk("TEMPPROTCTL         = 0x%8x\n", DRV_Reg32(TEMPPROTCTL));

	mtktscpu_dprintk("TEMPPROTTA          = 0x%8x\n", DRV_Reg32(TEMPPROTTA));
	mtktscpu_dprintk("TEMPPROTTB 		  = 0x%8x\n", DRV_Reg32(TEMPPROTTB));
	mtktscpu_dprintk("TEMPPROTTC 		  = 0x%8x\n", DRV_Reg32(TEMPPROTTC));
	mtktscpu_dprintk("TEMPSPARE0 		  = 0x%8x\n", DRV_Reg32(TEMPSPARE0));
	mtktscpu_dprintk("TEMPSPARE1 		  = 0x%8x\n", DRV_Reg32(TEMPSPARE1));
	mtktscpu_dprintk("TEMPSPARE2 		  = 0x%8x\n", DRV_Reg32(TEMPSPARE2));
	mtktscpu_dprintk("TEMPSPARE3 		  = 0x%8x\n", DRV_Reg32(TEMPSPARE3));
	mtktscpu_dprintk("0x11001040 		  = 0x%8x\n", DRV_Reg32(0xF1001040));

}

void get_thermal_slope_intercept(struct TS_PTPOD *ts_info)
{
	unsigned int temp0, temp1, temp2;
	struct TS_PTPOD ts_ptpod;

	//mtktscpu_dprintk("get_thermal_slope_intercept\n");

	//temp0 = (10000*100000/4096/g_gain)*15/18;
	temp0 = (10000*100000/g_gain)*15/18;
//	mtktscpu_printk("temp0=%d\n", temp0);
	if(g_o_slope_sign==0)
	{
		temp1 = temp0/(165+g_o_slope);
	}
	else
	{
		temp1 = temp0/(165-g_o_slope);
	}
//	mtktscpu_printk("temp1=%d\n", temp1);
	//ts_ptpod.ts_MTS = temp1 - (2*temp1) + 2048;
	ts_ptpod.ts_MTS = temp1;

	temp0 = (g_degc_cali *10 / 2);
	temp1 = ((10000*100000/4096/g_gain)*g_oe + g_x_roomt3*10)*15/18;
//	mtktscpu_printk("temp1=%d\n", temp1);
	if(g_o_slope_sign==0)
	{
		temp2 = temp1*10/(165+g_o_slope);
	}
	else
	{
		temp2 = temp1*10/(165-g_o_slope);
	}
//	mtktscpu_printk("temp2=%d\n", temp2);
	ts_ptpod.ts_BTS = (temp0+temp2-250)*4/10;

	//ts_info = &ts_ptpod;
	ts_info->ts_MTS = ts_ptpod.ts_MTS;
	ts_info->ts_BTS = ts_ptpod.ts_BTS;
	mtktscpu_printk("ts_MTS=%d, ts_BTS=%d\n",ts_ptpod.ts_MTS, ts_ptpod.ts_BTS);

	return;
}
EXPORT_SYMBOL(get_thermal_slope_intercept);



static irqreturn_t thermal_interrupt_handler(int irq, void *dev_id)
{
	kal_uint32 ret = 0;
//	int i=0;

	//for SPM reset debug
	/*
	mtktscpu_printk("SPM_SLEEP_ISR_RAW_STA      =0x%08x\n",spm_read(SPM_SLEEP_ISR_RAW_STA));
	mtktscpu_printk("SPM_PCM_REG13_DATA         =0x%08x\n",spm_read(SPM_PCM_REG13_DATA));
	mtktscpu_printk("SPM_SLEEP_WAKEUP_EVENT_MASK=0x%08x\n",spm_read(SPM_SLEEP_WAKEUP_EVENT_MASK));
	mtktscpu_printk("SPM_POWERON_CONFIG_SET     =0x%08x\n",spm_read(SPM_POWERON_CONFIG_SET));
	mtktscpu_printk("SPM_POWER_ON_VAL1		     =0x%08x\n",spm_read(SPM_POWER_ON_VAL1));
	mtktscpu_printk("SPM_PCM_IM_LEN		     =0x%08x\n",spm_read(SPM_PCM_IM_LEN));
   	mtktscpu_printk("SPM_PCM_PWR_IO_EN	   	     =0x%08x\n",spm_read(SPM_PCM_PWR_IO_EN));

   	mtktscpu_printk("SPM_PCM_CON0			     =0x%08x\n",spm_read(SPM_PCM_CON0));
   	mtktscpu_printk("SPM_PCM_CON1			     =0x%08x\n",spm_read(SPM_PCM_CON1));
   	mtktscpu_printk("SPM_PCM_IM_PTR		     =0x%08x\n",spm_read(SPM_PCM_IM_PTR));
   	mtktscpu_printk("SPM_PCM_REG1_DATA			 =0x%08x\n",spm_read(SPM_PCM_REG1_DATA));
   	mtktscpu_printk("SPM_PCM_REG2_DATA			 =0x%08x\n",spm_read(SPM_PCM_REG2_DATA));
   	mtktscpu_printk("SPM_PCM_REG3_DATA			 =0x%08x\n",spm_read(SPM_PCM_REG3_DATA));
   	mtktscpu_printk("SPM_PCM_REG7_DATA			 =0x%08x\n",spm_read(SPM_PCM_REG7_DATA));
   	mtktscpu_printk("SPM_PCM_REG9_DATA			 =0x%08x\n",spm_read(SPM_PCM_REG9_DATA));
   	mtktscpu_printk("SPM_PCM_REG12_DATA		 =0x%08x\n",spm_read(SPM_PCM_REG12_DATA));
   	mtktscpu_printk("SPM_PCM_REG14_DATA		 =0x%08x\n",spm_read(SPM_PCM_REG14_DATA));
   	mtktscpu_printk("SPM_PCM_REG15_DATA		 =0x%08x\n",spm_read(SPM_PCM_REG15_DATA));
   	mtktscpu_printk("SPM_PCM_FSM_STA			 =0x%08x\n",spm_read(SPM_PCM_FSM_STA));
	*/

	ret = DRV_Reg32(TEMPMONINTSTS);
	mtktscpu_dprintk("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	mtktscpu_printk("thermal_interrupt_handler,ret=0x%08x\n",ret);
	mtktscpu_dprintk("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");




	mtktscpu_dprintk("thermal_isr: [Interrupt trigger]: status = 0x%x\n", ret);
	if (ret & THERMAL_MON_CINTSTS0)
	{
		mtktscpu_printk("thermal_isr: thermal sensor point 0 - cold interrupt trigger\n");
	}
	if (ret & THERMAL_MON_HINTSTS0)
	{
		mtktscpu_printk("thermal_isr: thermal sensor point 0 - hot interrupt trigger\n");
	}

	if(ret & THERMAL_tri_SPM_State0)
		mtktscpu_printk("thermal_isr: Thermal state0 to trigger SPM state0\n");
	if(ret & THERMAL_tri_SPM_State1){
        mtktscpu_printk("thermal_isr: Thermal state1 to trigger SPM state1\n");
#if MTK_TS_CPU_RT
        Temp_TS2 = MAX(tc_mid_trip, Temp_TS2);
        Temp_TS3 = MAX(tc_mid_trip, Temp_TS3);
        Temp_TS4 = MAX(tc_mid_trip, Temp_TS4);

        wake_up_process(ktp_thread_handle);
#endif
    }
	if(ret & THERMAL_tri_SPM_State2)
		mtktscpu_printk("thermal_isr: Thermal state2 to trigger SPM state2\n");

	return IRQ_HANDLED;
}

static void thermal_reset_and_initial(void)
{
	UINT32 temp = 0;

	mtktscpu_dprintk("thermal_reset_and_initial\n");

	/*
		fix ALPS00848017
		can't turn off thermal, this will cause PTPOD  issue abnormal interrupt
		and let system crash.(because PTPOD can't get thermal's temperature)
	*/
	//mtktscpu_thermal_clock_on();

	//reset thremal ctrl
	temp = DRV_Reg32(PERI_GLOBALCON_RST0); //MT6592_PERICFG.xml // TODO: check this line
	temp |= 0x00010000;//1: Reset THERM
	THERMAL_WRAP_WR32(temp, PERI_GLOBALCON_RST0);

	temp = DRV_Reg32(PERI_GLOBALCON_RST0);
	temp &= 0xFFFEFFFF;//0: Not reset THERM
	THERMAL_WRAP_WR32(temp, PERI_GLOBALCON_RST0);

	// AuxADC Initialization,ref MT6592_AUXADC.doc // TODO: check this line
	temp = DRV_Reg32(AUXADC_CON0_V);//Auto set enable for CH11
	temp &= 0xFFFFF7FF;//0: Not AUTOSET mode
	THERMAL_WRAP_WR32(temp, AUXADC_CON0_V);        // disable auxadc channel 11 synchronous mode

	THERMAL_WRAP_WR32(0x800, AUXADC_CON1_CLR_V);    // disable auxadc channel 11 immediate mode


    THERMAL_WRAP_WR32(0x000003FF, TEMPMONCTL1);    // counting unit is 1023 * 15.15ns ~ 15.5us

#if THERMAL_CONTROLLER_HW_FILTER == 2
    THERMAL_WRAP_WR32(0x07E007E0, TEMPMONCTL2);     // both filt and sen interval is 31.25ms
    THERMAL_WRAP_WR32(0x001F7972, TEMPAHBPOLL);     // poll is set to 31.25ms
    THERMAL_WRAP_WR32(0x00000049, TEMPMSRCTL0);     // temperature sampling control, 2 out of 4 samples
#elif THERMAL_CONTROLLER_HW_FILTER == 4
    THERMAL_WRAP_WR32(0x050A050A, TEMPMONCTL2);     // both filt and sen interval is 20ms
    THERMAL_WRAP_WR32(0x001424C4, TEMPAHBPOLL);     // poll is set to 20ms
    THERMAL_WRAP_WR32(0x000000DB, TEMPMSRCTL0);     // temperature sampling control, 4 out of 6 samples
#elif THERMAL_CONTROLLER_HW_FILTER == 8
    THERMAL_WRAP_WR32(0x03390339, TEMPMONCTL2);     // both filt and sen interval is 12.5ms
    THERMAL_WRAP_WR32(0x000C96FA, TEMPAHBPOLL);     // poll is set to 12.5ms
    THERMAL_WRAP_WR32(0x00000124, TEMPMSRCTL0);     // temperature sampling control, 8 out of 10 samples
#elif THERMAL_CONTROLLER_HW_FILTER == 16
    THERMAL_WRAP_WR32(0x01C001C0, TEMPMONCTL2);     // both filt and sen interval is 6.94ms
	THERMAL_WRAP_WR32(0x0006FE8B, TEMPAHBPOLL);     // poll is set to 6.94ms
    THERMAL_WRAP_WR32(0x0000016D, TEMPMSRCTL0);     // temperature sampling control, 16 out of 18 samples
#else // default 1
    THERMAL_WRAP_WR32(0x03FF0000, TEMPMONCTL2);	    // filter interval is 1023 * 15.5us ~ 15.86ms
    THERMAL_WRAP_WR32(0x00FFFFFF, TEMPAHBPOLL);		// poll is set to 254.17ms
    THERMAL_WRAP_WR32(0x00000000, TEMPMSRCTL0);      // temperature sampling control, 1 sample
#endif

	THERMAL_WRAP_WR32(0xFFFFFFFF, TEMPAHBTO);      // exceed this polling time, IRQ would be inserted

	THERMAL_WRAP_WR32(0x00000000, TEMPMONIDET0);   // times for interrupt occurrance
	THERMAL_WRAP_WR32(0x00000000, TEMPMONIDET1);   // times for interrupt occurrance

//	THERMAL_WRAP_WR32(0x000008FC, TEMPHTHRE);     // set hot threshold
//	THERMAL_WRAP_WR32(0x00000960, TEMPOFFSETH);    // set high offset threshold
//	THERMAL_WRAP_WR32(0x00000A8C, TEMPH2NTHRE);    // set hot to normal threshold
//	THERMAL_WRAP_WR32(0x00000C80, TEMPOFFSETL);    // set low offset threshold
//	THERMAL_WRAP_WR32(0x00000CE4, TEMPCTHRE);      // set cold threshold



	THERMAL_WRAP_WR32(0x800, AUXADC_CON1_SET_V);    // enable auxadc channel 11 immediate mode

	//THERMAL_WRAP_WR32(0x0, TEMPADCPNP0);                        // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
	//THERMAL_WRAP_WR32((UINT32) TEMPSPARE0, TEMPPNPMUXADDR);     // AHB address for pnp sensor mux selection


	THERMAL_WRAP_WR32(0x800, TEMPADCMUX);                         // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
	THERMAL_WRAP_WR32((UINT32) AUXADC_CON1_CLR_P, TEMPADCMUXADDR);// AHB address for auxadc mux selection

	THERMAL_WRAP_WR32(0x800, TEMPADCEN);                          // AHB value for auxadc enable
	THERMAL_WRAP_WR32((UINT32) AUXADC_CON1_SET_P, TEMPADCENADDR); // AHB address for auxadc enable (channel 0 immediate mode selected)
																  // this value will be stored to TEMPADCENADDR automatically by hw

	THERMAL_WRAP_WR32((UINT32) AUXADC_DAT11_P, TEMPADCVALIDADDR); // AHB address for auxadc valid bit
	THERMAL_WRAP_WR32((UINT32) AUXADC_DAT11_P, TEMPADCVOLTADDR);  // AHB address for auxadc voltage output
	THERMAL_WRAP_WR32(0x0, TEMPRDCTRL);               			  // read valid & voltage are at the same register
	THERMAL_WRAP_WR32(0x0000002C, TEMPADCVALIDMASK);              // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	THERMAL_WRAP_WR32(0x0, TEMPADCVOLTAGESHIFT);                  // do not need to shift
	THERMAL_WRAP_WR32(0x2, TEMPADCWRITECTRL);                     // enable auxadc mux write transaction

	//THERMAL_WRAP_WR32(0x0000FFFF, TEMPMONINT);                  // enable all interrupt

	temp = DRV_Reg32(TS_CON0);
    temp &=~(0x000000C0);										  //TSCON0[7:6]=2'b00,   00: Buffer on, TSMCU to AUXADC
	THERMAL_WRAP_WR32(temp, TS_CON0);	                          //read abb need

	udelay(150);//RG_TS2AUXADC < set from 2'b11 to 2'b00 when resume.wait 100uS than turn on thermal controller.

	//TSCON1[2:0]=3'b001
    THERMAL_WRAP_WR32(0x1,TEMPADCPNP0);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
	//TSCON1[2:0]=3'b010
    THERMAL_WRAP_WR32(0x2,TEMPADCPNP1);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
	//TSCON1[2:0]=3'b011
    THERMAL_WRAP_WR32(0x3,TEMPADCPNP2);

    THERMAL_WRAP_WR32((UINT32) TS_CON1_P,TEMPPNPMUXADDR);  // AHB address for pnp sensor mux selection
   	THERMAL_WRAP_WR32(0x3, TEMPADCWRITECTRL);

    THERMAL_WRAP_WR32(0x00000007, TEMPMONCTL0);   // enable periodoc temperature sensing point 0, point 1 and point 2


}

#if 0 //for Move thermal "HW protection ahead"
static void set_thermal_ctrl_trigger_SPM2(int temperature)
{
#if THERMAL_CONTROLLER_HW_TP
	int temp = 0;
	int raw_high;
	mtktscpu_printk("set_thermal_ctrl_trigger_SPM2 temperature=%d\n", temperature);

	//temperature to trigger SPM state2
	raw_high   = temperature_to_raw_abb(temperature);

//	mtktscpu_printk("1 SPM_PCM_REG12_DATA  =0x%08x\n",spm_read(SPM_PCM_REG12_DATA));
//  mtktscpu_printk("1 SPM_PCM_REG13_DATA  =0x%08x\n",spm_read(SPM_PCM_REG13_DATA));

	THERMAL_WRAP_WR32(temp | 0x80000000, TEMPMONINT);	// enable trigger HOT SPM interrupt

	temp = DRV_Reg32(TEMPMONINT);
	//THERMAL_WRAP_WR32(temp & 0x1FFFFFFF, TEMPMONINT);	// disable trigger SPM interrupt

	THERMAL_WRAP_WR32(0x20000, TEMPPROTCTL);// set hot to wakeup event control
	THERMAL_WRAP_WR32(raw_high, TEMPPROTTC);// set hot to HOT wakeup event

//	mtktscpu_printk("2 SPM_PCM_REG12_DATA  =0x%08x\n",spm_read(SPM_PCM_REG12_DATA));
//  mtktscpu_printk("2 SPM_PCM_REG13_DATA  =0x%08x\n",spm_read(SPM_PCM_REG13_DATA));


//	THERMAL_WRAP_WR32(temp | 0x80000000, TEMPMONINT);	// enable trigger SPM interrupt
#endif
}
#endif

/**
 *  temperature2 to set the middle threshold for interrupting CPU. -275000 to disable it.
 */
static void set_thermal_ctrl_trigger_SPM(int temperature, int temperature2)
{
#if THERMAL_CONTROLLER_HW_TP
	int temp = 0;
	int raw_high, raw_middle, raw_low;
	mtktscpu_printk("set_thermal_ctrl_trigger_SPM t1=%d t2=%d\n", temperature, temperature2);

	//temperature to trigger SPM state2
	raw_high   = temperature_to_raw_abb(temperature);
    if (temperature2 > -275000)
		raw_middle = temperature_to_raw_abb(temperature2);
	raw_low    = temperature_to_raw_abb(5000);

	temp = DRV_Reg32(TEMPMONINT);
	THERMAL_WRAP_WR32(temp & 0x1FFFFFFF, TEMPMONINT);	// disable trigger SPM interrupt

	#if MTK_TS_DEBUG_LOG
	mtktscpu_printk("3 SPM_PCM_REG12_DATA  =0x%08x\n",spm_read(SPM_PCM_REG12_DATA));
   	mtktscpu_printk("3 SPM_PCM_REG13_DATA  =0x%08x\n",spm_read(SPM_PCM_REG13_DATA));
	#endif

	THERMAL_WRAP_WR32(0x20000, TEMPPROTCTL);// set hot to wakeup event control
	THERMAL_WRAP_WR32(raw_low, TEMPPROTTA);
    if (temperature2 > -275000)
		THERMAL_WRAP_WR32(raw_middle, TEMPPROTTB); // register will remain unchanged if -275000...
	THERMAL_WRAP_WR32(raw_high, TEMPPROTTC);// set hot to HOT wakeup event

	#if MTK_TS_DEBUG_LOG
	mtktscpu_printk("4 SPM_PCM_REG12_DATA  =0x%08x\n",spm_read(SPM_PCM_REG12_DATA));
   	mtktscpu_printk("4 SPM_PCM_REG13_DATA  =0x%08x\n",spm_read(SPM_PCM_REG13_DATA));
	#endif

	/*trigger cold ,normal and hot interrupt*/
	//remove for temp	THERMAL_WRAP_WR32(temp | 0xE0000000, TEMPMONINT);	// enable trigger SPM interrupt
	/*Only trigger hot interrupt*/
	if (temperature2 > -275000)
		THERMAL_WRAP_WR32(temp | 0xC0000000, TEMPMONINT);	// enable trigger middle & Hot SPM interrupt
	else
		THERMAL_WRAP_WR32(temp | 0x80000000, TEMPMONINT);	// enable trigger Hot SPM interrupt
#endif
}

int mtk_cpufreq_register(struct mtk_cpu_power_info *freqs, int num)
{
	int i = 0;
    int gpu_power = 0;
	mtktscpu_dprintk("mtk_cpufreq_register\n");

	tscpu_num_opp = num;

	mtk_cpu_power = kzalloc((num) * sizeof(struct mtk_cpu_power_info), GFP_KERNEL);
	if (mtk_cpu_power==NULL)
		return -ENOMEM;

	// WARNING: here assumes mtk_gpufreq_register is always called first. If not, here the tscpu_cpu_dmips is wrong...
	if (0 != Num_of_GPU_OPP && NULL != mtk_gpu_power)
        gpu_power = mtk_gpu_power[Num_of_GPU_OPP-1].gpufreq_power;
    else
        mtktscpu_printk("Num_of_GPU_OPP is 0!\n");

#if 0
	if(Num_of_GPU_OPP==3)
        gpu_power = mtk_gpu_power[2].gpufreq_power;
    else if(Num_of_GPU_OPP==2)
        gpu_power = mtk_gpu_power[1].gpufreq_power;
	else //Num_of_GPU_OPP=1
        gpu_power = mtk_gpu_power[0].gpufreq_power;
#endif

	for (i=0; i<num; i++)
	{
	    int dmips = freqs[i].cpufreq_khz * freqs[i].cpufreq_ncpu / 1000;

	    int cl_id = (((freqs[i].cpufreq_power + gpu_power) + 99) / 100) - 7; // TODO: this line must be modified every time cooler mapping table changes

		mtk_cpu_power[i].cpufreq_khz = freqs[i].cpufreq_khz;
		mtk_cpu_power[i].cpufreq_ncpu = freqs[i].cpufreq_ncpu;
		mtk_cpu_power[i].cpufreq_power = freqs[i].cpufreq_power;

        if (cl_id < CPU_COOLER_NUM)
        {
            if (tscpu_cpu_dmips[cl_id] < dmips)
                tscpu_cpu_dmips[cl_id] = dmips;
        }

		mtktscpu_printk("[%d].cpufreq_khz=%u, .cpufreq_ncpu=%u, .cpufreq_power=%u\n",
                        i, freqs[i].cpufreq_khz, freqs[i].cpufreq_ncpu, freqs[i].cpufreq_power);
	}

    {
        // TODO: this assumes the last OPP consumes least power...need to check this every time OPP table changes
        int base = (mtk_cpu_power[num-1].cpufreq_khz * mtk_cpu_power[num-1].cpufreq_ncpu)/1000;
    	for (i=0; i<CPU_COOLER_NUM; i++)
    	{
    	    if (0 == tscpu_cpu_dmips[i] || tscpu_cpu_dmips[i] < base)
    	        tscpu_cpu_dmips[i] = base;
    	    else
    	        base = tscpu_cpu_dmips[i];
    	}
    	mtktscpu_limited_dmips = base;
	}

	return 0;
}
EXPORT_SYMBOL(mtk_cpufreq_register);

// Init local structure for AP coolers
static int init_cooler(void)
{
	int i;
	int num = CPU_COOLER_NUM; //700~4000, 92

	cl_dev_state = kzalloc((num) * sizeof(unsigned int), GFP_KERNEL);
	if(cl_dev_state==NULL)
		return -ENOMEM;

	cl_dev = (struct thermal_cooling_device **)kzalloc((num) * sizeof(struct thermal_cooling_device *), GFP_KERNEL);
	if(cl_dev==NULL)
		return -ENOMEM;

	cooler_name = kzalloc((num) * sizeof(char) * 20, GFP_KERNEL);
	if(cooler_name==NULL)
		return -ENOMEM;

	for(i=0; i<num; i++)
		sprintf(cooler_name+(i*20), "cpu%02d", i); //using index=>0=700,1=800 ~ 33=4000

	Num_of_OPP = num; // CPU COOLER COUNT, not CPU OPP count
	return 0;
}

int mtk_gpufreq_register(struct mtk_gpu_power_info *freqs, int num)
{
	int i = 0;
	mtktscpu_dprintk("mtk_gpufreq_register\n");
	mtk_gpu_power = kzalloc((num) * sizeof(struct mtk_gpu_power_info), GFP_KERNEL);
	if (mtk_gpu_power == NULL)
		return -ENOMEM;

	for (i=0; i<num; i++)
	{
		mtk_gpu_power[i].gpufreq_khz = freqs[i].gpufreq_khz;
		mtk_gpu_power[i].gpufreq_power = freqs[i].gpufreq_power;

		mtktscpu_printk("[%d].gpufreq_khz=%u, .gpufreq_power=%u\n",
                        i, freqs[i].gpufreq_khz, freqs[i].gpufreq_power);
	}

	Num_of_GPU_OPP = num; // GPU OPP count
	return 0;
}
EXPORT_SYMBOL(mtk_gpufreq_register);

void mtkts_dump_cali_info(void)
{
	mtktscpu_printk("[calibration] g_adc_ge_t      = 0x%x\n",g_adc_ge_t);
	mtktscpu_printk("[calibration] g_adc_oe_t      = 0x%x\n",g_adc_oe_t);
	mtktscpu_printk("[calibration] g_degc_cali     = 0x%x\n",g_degc_cali);
	mtktscpu_printk("[calibration] g_adc_cali_en_t = 0x%x\n",g_adc_cali_en_t);
	mtktscpu_printk("[calibration] g_o_slope       = 0x%x\n",g_o_slope);
	mtktscpu_printk("[calibration] g_o_slope_sign  = 0x%x\n",g_o_slope_sign);
	mtktscpu_printk("[calibration] g_id            = 0x%x\n",g_id);

	mtktscpu_printk("[calibration] g_o_vtsmcu2     = 0x%x\n",g_o_vtsmcu2);
	mtktscpu_printk("[calibration] g_o_vtsmcu3     = 0x%x\n",g_o_vtsmcu3);
	mtktscpu_printk("[calibration] g_o_vtsmcu4     = 0x%x\n",g_o_vtsmcu4);
}

kal_uint32 temp0_0xF020617C=0;
kal_uint32 temp1_0xF0206178=0;
static void thermal_cal_prepare(void)
{
	kal_uint32 temp0 = 0, temp1 = 0;
#if 1
	//Thermal and ptopd's efuse was exchanged
	/*
	 PTPOD                           THERMAL
	0x10206100  (7)      <->     0x10206178  (16)
	0x10206104  (8)      <->     0x1020617C  (17)
	0x10206108  (9)      <->     0x10206180  (18)
	0x10206170  (14)     <->     0x10206184  (19)
	*/
	temp0 = get_devinfo_with_index(17);
	temp1 = get_devinfo_with_index(16);
    temp0_0xF020617C = temp0;
    temp1_0xF0206178 = temp1;
	//temp2 = get_devinfo_with_index(18);
	//temp2 = get_devinfo_with_index(19);
#else
	temp0 = DRV_Reg32(0xF020617C);//92
    temp1 = DRV_Reg32(0xF0206178);//92
#endif


	mtktscpu_printk("[calibration] temp0=0x%x, temp1=0x%x\n", temp0, temp1);
	//mtktscpu_dprintk("thermal_cal_prepare\n");

    g_adc_ge_t     = ((temp0 & 0xFFC00000)>>22);//ADC_GE_T [9:0] *(0xF020617C)[31:22]
	g_adc_oe_t     = ((temp0 & 0x003FF000)>>12);//ADC_OE_T [9:0] *(0xF020617C)[21:12]


	g_o_vtsmcu2    = (temp1 & 0x03FE0000)>>17;  //O_VTSMCU2    (9b) *(0xF0206178)[25:17]
	g_o_vtsmcu3    = (temp1 & 0x0001FF00)>>8;   //O_VTSMCU3    (9b) *(0xF0206178)[16:8]
	g_o_vtsmcu4    = (temp0 & 0x000001FF);      //O_VTSMCU4    (9b) *(0xF020617C)[8:0]

 	//g_o_vtsabb     = (temp1 & 0x000001FF);		//O_VTSABB     (9b) *(0x10206104)[8:0]

	g_degc_cali    = (temp1 & 0x0000007E)>>1;   //DEGC_cali    (6b) *(0xF0206178)[6:1]
	g_adc_cali_en_t= (temp1 & 0x00000001);		//ADC_CALI_EN_T(1b) *(0xF0206178)[0]
	g_o_slope      = (temp1 & 0xFC000000)>>26;  //O_SLOPE      (6b) *(0xF0206178)[31:26]
	g_o_slope_sign = (temp1 & 0x00000080)>>7;   //O_SLOPE_SIGN (1b) *(0xF0206178)[7]

	g_id           = (temp0 & 0x00000200)>>9;   //ID           (1b) *(0xF020617C)[9]

	/*
	Check ID bit
	If ID=0 (TSMC sample)    , ignore O_SLOPE EFuse value and set O_SLOPE=0.
	If ID=1 (non-TSMC sample), read O_SLOPE EFuse value for following calculation.
    */
	if(g_id==0)
	{
		g_o_slope=0;
	}


	//g_adc_cali_en_t=0;//test only
	if(g_adc_cali_en_t == 1)
	{
		//thermal_enable = true;
	}
	else
	{
		mtktscpu_printk("This sample is not Thermal calibrated\n");
        #if 1 //default
		g_adc_ge_t = 512;
		g_adc_oe_t = 512;
		g_degc_cali = 40;
		g_o_slope = 0;
		g_o_slope_sign = 0;
		g_o_vtsmcu2 = 260;
		g_o_vtsmcu3 = 260;
		g_o_vtsmcu4 = 260;
		//g_o_vtsabb = 260;
		#endif

	}

	mtkts_dump_cali_info();
}

static void thermal_cal_prepare_2(kal_uint32 ret)
{
	kal_int32 format_2= 0, format_3= 0, format_4= 0;

	mtktscpu_dprintk("thermal_cal_prepare_2\n");

	g_ge = ((g_adc_ge_t - 512) * 10000 ) / 4096; // ge * 10000
	g_oe =  (g_adc_oe_t - 512);

	g_gain = (10000 + g_ge);

	format_2   = (g_o_vtsmcu2 + 3350 - g_oe);
	format_3   = (g_o_vtsmcu3 + 3350 - g_oe);
	format_4   = (g_o_vtsmcu4 + 3350 - g_oe);

	g_x_roomt2   = (((format_2   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt3   = (((format_3   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt4   = (((format_4   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000


    mtktscpu_dprintk("[calibration] g_ge       = 0x%x\n",g_ge);
    mtktscpu_dprintk("[calibration] g_gain     = 0x%x\n",g_gain);
    mtktscpu_dprintk("[calibration] g_x_roomt2 = 0x%x\n",g_x_roomt2);
    mtktscpu_dprintk("[calibration] g_x_roomt3 = 0x%x\n",g_x_roomt3);
    mtktscpu_dprintk("[calibration] g_x_roomt4 = 0x%x\n",g_x_roomt4);
}

#if THERMAL_CONTROLLER_HW_TP
static kal_int32 temperature_to_raw_abb(kal_uint32 ret)
{
	// Ycurr = [(Tcurr - DEGC_cali/2)*(165+O_slope)*(18/15)*(1/10000)+X_roomtabb]*Gain*4096 + OE

	kal_int32 t_curr = ret;
//	kal_int32 y_curr = 0;
	kal_int32 format_1 = 0;
	kal_int32 format_2 = 0;
	kal_int32 format_3 = 0;
	kal_int32 format_4 = 0;

	mtktscpu_dprintk("temperature_to_raw_abb\n");

	if(g_o_slope_sign==0)//O_SLOPE is Positive.
	{
		//format_1 = t_curr-(g_degc_cali/2)*1000;
		format_1 = t_curr-(g_degc_cali*1000/2);
		format_2 = format_1 * (165 + g_o_slope) * 18/15;
		format_2 = format_2 - 2*format_2;
		format_3 = format_2/1000 + g_x_roomt2*10;
		format_4 = (format_3*4096/10000*g_gain)/100000 + g_oe;
		//mtktscpu_dprintk("temperature_to_raw_abb format_1=%d, format_2=%d, format_3=%d, format_4=%d\n", format_1, format_2, format_3, format_4);
	}
	else //O_SLOPE is Negative.
	{
		//format_1 = t_curr-(g_degc_cali/2)*1000;
		format_1 = t_curr-(g_degc_cali*1000/2);
		format_2 = format_1 * (165 - g_o_slope) * 18/15;
		format_2 = format_2 - 2*format_2;
		format_3 = format_2/1000 + g_x_roomt2*10;
		format_4 = (format_3*4096/10000*g_gain)/100000 + g_oe;
		//mtktscpu_dprintk("temperature_to_raw_abb format_1=%d, format_2=%d, format_3=%d, format_4=%d\n", format_1, format_2, format_3, format_4);
	}

	mtktscpu_dprintk("temperature_to_raw_abb temperature=%d, raw=%d\n", ret, format_4);
	return format_4;
}
#endif


//TS2
static kal_int32 raw_to_temperature_MCU2(kal_uint32 ret)
{
	kal_int32 t_current = 0;
	kal_int32 y_curr = ret;
	kal_int32 format_1 = 0;
	kal_int32 format_2 = 0;
	kal_int32 format_3 = 0;
	kal_int32 format_4 = 0;


	//mtktscpu_printk("raw_to_temperature_MCU2\n");
	if(ret==0)
	{
		return 0;
	}

	//format_1 = (g_degc_cali / 2);
	format_1 = (g_degc_cali*10 / 2);
	format_2 = (y_curr - g_oe);
	format_3 = (((((format_2) * 10000) / 4096) * 10000) / g_gain) - g_x_roomt2;
	format_3 = format_3 * 15/18;

	//format_4 = ((format_3 * 100) / 139); // uint = 0.1 deg
	if(g_o_slope_sign==0)
	{
		//format_4 = ((format_3 * 100) / (139+g_o_slope)); // uint = 0.1 deg
		format_4 = ((format_3 * 100) / (165+g_o_slope)); // uint = 0.1 deg
	}
	else
	{
		//format_4 = ((format_3 * 100) / (139-g_o_slope)); // uint = 0.1 deg
		format_4 = ((format_3 * 100) / (165-g_o_slope)); // uint = 0.1 deg
	}
	format_4 = format_4 - (2 * format_4);

	//t_current = (format_1 * 10) + format_4; // uint = 0.1 deg
	t_current = format_1 + format_4; // uint = 0.1 deg

	return t_current;
}

//TS3
static kal_int32 raw_to_temperature_MCU3(kal_uint32 ret)
{
	kal_int32 t_current = 0;
	kal_int32 y_curr = ret;
	kal_int32 format_1 = 0;
	kal_int32 format_2 = 0;
	kal_int32 format_3 = 0;
	kal_int32 format_4 = 0;


	//mtktscpu_printk("raw_to_temperature_MCU3\n");
	if(ret==0)
	{
		return 0;
	}

	//format_1 = (g_degc_cali / 2);
	format_1 = (g_degc_cali*10 / 2);
	format_2 = (y_curr - g_oe);
	format_3 = (((((format_2) * 10000) / 4096) * 10000) / g_gain) - g_x_roomt3;
	format_3 = format_3 * 15/18;

	//format_4 = ((format_3 * 100) / 139); // uint = 0.1 deg
	if(g_o_slope_sign==0)
	{
		//format_4 = ((format_3 * 100) / (139+g_o_slope)); // uint = 0.1 deg
		format_4 = ((format_3 * 100) / (165+g_o_slope)); // uint = 0.1 deg
	}
	else
	{
		//format_4 = ((format_3 * 100) / (139-g_o_slope)); // uint = 0.1 deg
		format_4 = ((format_3 * 100) / (165-g_o_slope)); // uint = 0.1 deg
	}
	format_4 = format_4 - (2 * format_4);

	//t_current = (format_1 * 10) + format_4; // uint = 0.1 deg
	t_current = format_1 + format_4; // uint = 0.1 deg

	return t_current;
}


//TS4
#if 1
static kal_int32 raw_to_temperature_MCU4(kal_uint32 ret)
{
	kal_int32 t_current = 0;
	kal_int32 y_curr = ret;
	kal_int32 format_1 = 0;
	kal_int32 format_2 = 0;
	kal_int32 format_3 = 0;
	kal_int32 format_4 = 0;


	//mtktscpu_printk("raw_to_temperature_MCU4\n");
	if(ret==0)
	{
		return 0;
	}

	//format_1 = (g_degc_cali / 2);
	format_1 = (g_degc_cali*10 / 2);
	format_2 = (y_curr - g_oe);
	format_3 = (((((format_2) * 10000) / 4096) * 10000) / g_gain) - g_x_roomt4;
	format_3 = format_3 * 15/18;

	//format_4 = ((format_3 * 100) / 139); // uint = 0.1 deg
	if(g_o_slope_sign==0)
	{
		//format_4 = ((format_3 * 100) / (139+g_o_slope)); // uint = 0.1 deg
		format_4 = ((format_3 * 100) / (165+g_o_slope)); // uint = 0.1 deg
	}
	else
	{
		//format_4 = ((format_3 * 100) / (139-g_o_slope)); // uint = 0.1 deg
		format_4 = ((format_3 * 100) / (165-g_o_slope)); // uint = 0.1 deg
	}
	format_4 = format_4 - (2 * format_4);

	//t_current = (format_1 * 10) + format_4; // uint = 0.1 deg
	t_current = format_1 + format_4; // uint = 0.1 deg

	return t_current;
}
#endif


static void thermal_calibration(void)
{
	if (g_adc_cali_en_t == 0)
		mtktscpu_printk("Not Calibration\n");

	//mtktscpu_dprintk("thermal_calibration\n");
	thermal_cal_prepare_2(0);
}




/*
TS2 between CAM/VENC/VDEC
TS3 between CPU/WIFI
TS4 between CPU/GPU
TS2 for ABB, TS3 for CPU
*/
static int get_immediate_temp1(void)
{
    int curr_raw1, curr_temp1;

	curr_raw1 = DRV_Reg32(TEMPMSR0);
	curr_raw1 = curr_raw1 & 0x0fff;//bit0~bit11
	curr_temp1 = raw_to_temperature_MCU3(curr_raw1);//TS3 for CPU
	curr_temp1 = curr_temp1*100;
	Temp_TS3 = curr_temp1;
    RAW_TS3 = curr_raw1;

	mtktscpu_dprintk("get_immediate_temp1 temp1=%d, raw1=%d\n", curr_temp1, curr_raw1);

#if MTK_TS_DEBUG_LOG
	mtktscpu_printk("1 dpidle_cnt=%d, mcidle_cnt=%d\n", dpidle_cnt[0], mcidle_cnt[0]);
	mtktscpu_printk("1 mtkts_suspend_counter=%d, mtkts_resume_counter=%d\n", mtkts_suspend_counter, mtkts_resume_counter);
#endif

	return curr_temp1;
}

static int get_immediate_temp2(void)
{
    int curr_raw2, curr_temp2;

	curr_raw2 = DRV_Reg32(TEMPMSR1);
	curr_raw2 = curr_raw2 & 0x0fff;//bit0~bit11
	curr_temp2 = raw_to_temperature_MCU2(curr_raw2);//TS2 for ABB
	curr_temp2 = curr_temp2*100;
	Temp_TS2 = curr_temp2;
    RAW_TS2 = curr_raw2;

    mtktscpu_dprintk("get_immediate_temp2 temp2=%d, raw2=%d\n", curr_temp2, curr_raw2);
	return curr_temp2;
}

int get_immediate_temp2_wrap(void)
{
	int curr_raw;

	curr_raw = get_immediate_temp2();

    //mtktscpu_dprintk("get_immediate_temp2_wrap curr_raw=%d\n", curr_raw);
    return curr_raw;
}


static int get_immediate_temp3(void)
{
    int curr_raw3, curr_temp3;

	curr_raw3 = DRV_Reg32(TEMPMSR2);
	curr_raw3 = curr_raw3 & 0x0fff;//bit0~bit11
	curr_temp3 = raw_to_temperature_MCU4(curr_raw3);
	curr_temp3 = curr_temp3*100;
	Temp_TS4 = curr_temp3;
	RAW_TS4 = curr_raw3;

    mtktscpu_dprintk("get_immediate_temp3 temp3=%d, raw3=%d\n", curr_temp3, curr_raw3);
	return curr_temp3;
}


static int mtktscpu_get_temp(struct thermal_zone_device *thermal,
							unsigned long *t)
{
#if MTK_TS_CPU_SW_FILTER == 1
    int ret = 0;
	int curr_temp;
    int curr_temp2;
    int temp_temp;
    static int last_cpu_real_temp = 0;

	curr_temp = get_immediate_temp1();
	curr_temp2 =get_immediate_temp3();
	mtktscpu_dprintk("mtktscpu_get_temp CPU T1=%d\n",curr_temp);

	if((curr_temp > (trip_temp[0] - 15000)) || (curr_temp < -30000) || (curr_temp > 85000) || (curr_temp2 > 85000))
	{
		mtktscpu_printk("CPU T=%d,TS2 = %d,TS3 = %d,TS4 = %d\n",curr_temp,Temp_TS2,Temp_TS3,Temp_TS4);
        mtktscpu_printk("RAW_TS2 = %d,RAW_TS3 = %d,RAW_TS4 = %d\n",RAW_TS2,RAW_TS3,RAW_TS4);
    }

    temp_temp = curr_temp;
    if (curr_temp != 0) // not resumed from suspensio...
    {
        if ((curr_temp > 150000) || (curr_temp < -20000)) // invalid range
        {
            mtktscpu_printk("CPU temp invalid=%d\n", curr_temp);
            temp_temp = 50000;
            ret = -1;
        }
        else if (last_cpu_real_temp != 0)
        {
            if ((curr_temp - last_cpu_real_temp > 20000) || (last_cpu_real_temp - curr_temp > 20000)) //delta 20C, invalid change
            {
                mtktscpu_printk("CPU temp float hugely temp=%d, lasttemp=%d\n", curr_temp, last_cpu_real_temp);
                mtktscpu_printk("RAW_TS2 = %d,RAW_TS3 = %d,RAW_TS4 = %d\n",RAW_TS2,RAW_TS3,RAW_TS4);
                temp_temp = 50000;
                ret = -1;
            }
        }
    }

    last_cpu_real_temp = curr_temp;
    curr_temp = temp_temp;
	mtktscpu_dprintk("TS2 = %d,TS3 = %d,TS4 = %d\n", Temp_TS2,Temp_TS3,Temp_TS4);
#else
    int ret = 0;
	int curr_temp;

	curr_temp = get_immediate_temp1();

	mtktscpu_dprintk("mtktscpu_get_temp CPU T1=%d\n",curr_temp);

	if((curr_temp > (trip_temp[0] - 15000)) || (curr_temp<-30000))
		printk("[Power/CPU_Thermal] CPU T=%d\n",curr_temp);
#endif

	read_curr_temp = curr_temp;
	*t = (unsigned long) curr_temp;

#if MTKTSCPU_FAST_POLLING
    cur_fp_factor = next_fp_factor;

    if (curr_temp >= fast_polling_trip_temp)
    {
        next_fp_factor = fast_polling_factor;
        // it means next timeout will be in interval/fast_polling_factor
        thermal->polling_delay = interval/fast_polling_factor;
    }
    else
    {
        next_fp_factor = 1;
        thermal->polling_delay = interval;
    }
#endif

#if CPT_ADAPTIVE_AP_COOLER
	g_prev_temp = g_curr_temp;
	g_curr_temp = curr_temp;
#endif


	g_Tj_temp = curr_temp;

	return ret;
}


int mtktscpu_get_Tj_temp(void)
{
	mtktscpu_dprintk(" mtktscpu_get_Tj_temp,g_Tj_temp=%d\n",g_Tj_temp);
	return g_Tj_temp;
}
EXPORT_SYMBOL(mtktscpu_get_Tj_temp);

int mtktscpu_get_cpu_temp(void)
{
	int curr_temp;

	curr_temp = get_immediate_temp1();

//	mtktscpu_dprintk(" mtktscpu_get_cpu_temp CPU T1=%d\n",curr_temp);

    if((curr_temp > (trip_temp[0] - 15000)) || (curr_temp<-30000))
		mtktscpu_printk("mtktscpu_get_cpu_temp T=%d\n", curr_temp);


	return ((unsigned long) curr_temp);
}

static int mtktscpu_bind(struct thermal_zone_device *thermal,
						struct thermal_cooling_device *cdev)
{
	int table_val = 0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
		set_thermal_ctrl_trigger_SPM(trip_temp[0], tc_mid_trip);
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
		tc_mid_trip = trip_temp[1]; // only when a valid cooler is tried to bind here, we set tc_mid_trip to trip_temp[1];
        set_thermal_ctrl_trigger_SPM(trip_temp[0], tc_mid_trip);
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
		mtktscpu_dprintk("mtktscpu_bind %s\n", cdev->type);
	}
	else
	{
		return 0;
	}

	if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
		mtktscpu_printk("mtktscpu_bind error binding cooling dev\n");
		return -EINVAL;
	} else {
		mtktscpu_printk("mtktscpu_bind binding OK, %d\n", table_val);
	}

	return 0;
}

static int mtktscpu_unbind(struct thermal_zone_device *thermal,
                           struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
		tc_mid_trip = -275000; // only when a valid cooler is tried to bind here, we set tc_mid_trip to trip_temp[1];
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
		mtktscpu_dprintk("mtktscpu_unbind %s\n", cdev->type);
	}
	else
		return 0;


	if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
		mtktscpu_printk("mtktscpu_unbind error unbinding cooling dev\n");
	return -EINVAL;
	} else {
		mtktscpu_printk("mtktscpu_unbind unbinding OK\n");
	}

	return 0;
}

static int mtktscpu_get_mode(struct thermal_zone_device *thermal,
							enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
		: THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktscpu_set_mode(struct thermal_zone_device *thermal,
							enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktscpu_get_trip_type(struct thermal_zone_device *thermal, int trip,
								enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktscpu_get_trip_temp(struct thermal_zone_device *thermal, int trip,
								unsigned long *temp)
{
	*temp = trip_temp[trip];
	return 0;
}

static int mtktscpu_get_crit_temp(struct thermal_zone_device *thermal,
								unsigned long *temperature)
{
	*temperature = MTKTSCPU_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktscpu_dev_ops = {
	.bind = mtktscpu_bind,
	.unbind = mtktscpu_unbind,
	.get_temp = mtktscpu_get_temp,
	.get_mode = mtktscpu_get_mode,
	.set_mode = mtktscpu_set_mode,
	.get_trip_type = mtktscpu_get_trip_type,
	.get_trip_temp = mtktscpu_get_trip_temp,
	.get_crit_temp = mtktscpu_get_crit_temp,
};

static int previous_step=-1;

/*
static int step0_mask[11] = {1,1,1,1,1,1,1,1,1,1,1};
static int step1_mask[11] = {0,1,1,1,1,1,1,1,1,1,1};
static int step2_mask[11] = {0,0,1,1,1,1,1,1,1,1,1};
static int step3_mask[11] = {0,0,0,1,1,1,1,1,1,1,1};
static int step4_mask[11] = {0,0,0,0,1,1,1,1,1,1,1};
static int step5_mask[11] = {0,0,0,0,0,1,1,1,1,1,1};
static int step6_mask[11] = {0,0,0,0,0,0,1,1,1,1,1};
static int step7_mask[11] = {0,0,0,0,0,0,0,1,1,1,1};
static int step8_mask[11] = {0,0,0,0,0,0,0,0,1,1,1};
static int step9_mask[11] = {0,0,0,0,0,0,0,0,0,1,1};
static int step10_mask[11]= {0,0,0,0,0,0,0,0,0,0,1};
*/
static int _mtktscpu_set_power_consumption_state(void)
{
	int i=0;
	int power=0;

	mtktscpu_dprintk("_mtktscpu_set_power_consumption_state Num_of_OPP=%d\n",Num_of_OPP);

	//in 92, Num_of_OPP=34
	for(i=0; i<Num_of_OPP; i++)
	{
		if(1==cl_dev_state[i])
		{
			if(i!=previous_step)
			{
				mtktscpu_printk("previous_opp=%d, now_opp=%d\n", previous_step, i);
				previous_step=i;
				mtktscpu_limited_dmips = tscpu_cpu_dmips[previous_step];
				if(Num_of_GPU_OPP==3)
				{
					power = (i*100+700) - mtk_gpu_power[Num_of_GPU_OPP-1].gpufreq_power;
					set_static_cpu_power_limit(power);
					set_static_gpu_power_limit(mtk_gpu_power[Num_of_GPU_OPP-1].gpufreq_power);
                    mtktscpu_dprintk("Num_of_GPU_OPP=%d, gpufreq_power=%d, power=%d\n", Num_of_GPU_OPP, mtk_gpu_power[Num_of_GPU_OPP-1].gpufreq_power, power);
				}
				else if(Num_of_GPU_OPP==2)
				{
					power = (i*100+700) - mtk_gpu_power[1].gpufreq_power;
					set_static_cpu_power_limit(power);
					set_static_gpu_power_limit(mtk_gpu_power[1].gpufreq_power);
                    mtktscpu_dprintk("Num_of_GPU_OPP=%d, gpufreq_power=%d, power=%d\n", Num_of_GPU_OPP, mtk_gpu_power[1].gpufreq_power, power);
				}
				else if(Num_of_GPU_OPP==1)
				{
					#if 0
			        // 653mW,GPU 500Mhz,1V(preloader default)
			        //1016mW,GPU 700Mhz,1.1V
					power = (i*100+700) - 653;
                    #else
                    power = (i*100+700) - mtk_gpu_power[0].gpufreq_power;
                    #endif
					set_static_cpu_power_limit(power);
                    mtktscpu_dprintk("Num_of_GPU_OPP=%d, gpufreq_power=%d, power=%d\n", Num_of_GPU_OPP, mtk_gpu_power[0].gpufreq_power,power);
				}
			}
			break;
		}
	}

	//If temp drop to our expect value, we need to restore initial cpu freq setting
	if (i == Num_of_OPP)
	{
		if (previous_step != -1)
		{
			previous_step = -1;
			mtktscpu_limited_dmips = tscpu_cpu_dmips[CPU_COOLER_NUM-1]; // highest dmips
			mtktscpu_printk("Free all static thermal limit, previous_opp=%d\n", previous_step);
			set_static_cpu_power_limit(0);
			set_static_gpu_power_limit(0);
		}
	}
	return 0;
}

#if CPT_ADAPTIVE_AP_COOLER

extern unsigned long (*mtk_thermal_get_gpu_loading_fp)(void);

static int GPU_L_H_TRIP = 80, GPU_L_L_TRIP = 40;

static int TARGET_TJ = 65000;
static int TARGET_TJ_HIGH = 66000;
static int TARGET_TJ_LOW = 64000;
static int PACKAGE_THETA_JA_RISE = 10;
static int PACKAGE_THETA_JA_FALL = 10;
static int MINIMUM_CPU_POWER = 500;
static int MAXIMUM_CPU_POWER = 1240;
static int MINIMUM_GPU_POWER = 676;
static int MAXIMUM_GPU_POWER = 676;
static int MINIMUM_TOTAL_POWER = 500+676;
static int MAXIMUM_TOTAL_POWER = 1240+676;
static int FIRST_STEP_TOTAL_POWER_BUDGET = 1750;

// 1. MINIMUM_BUDGET_CHANGE = 0 ==> thermal equilibrium maybe at higher than TARGET_TJ_HIGH
// 2. Set MINIMUM_BUDGET_CHANGE > 0 if to keep Tj at TARGET_TJ
static int MINIMUM_BUDGET_CHANGE = 50;

static int P_adaptive(int total_power, unsigned int gpu_loading)
{
    // Here the gpu_power is the gpu power limit for the next interval, not exactly what gpu power selected by GPU DVFS
    // But the ground rule is real gpu power should always under gpu_power for the same time interval
    static int cpu_power = 0, gpu_power = 0;
    static int last_cpu_power = 0, last_gpu_power = 0;

    last_cpu_power = cpu_power;
    last_gpu_power = gpu_power;

    if (total_power == 0)
    {
        cpu_power = gpu_power = 0;
        set_adaptive_cpu_power_limit(0);
        set_adaptive_gpu_power_limit(0);
        return 0;
    }

    if (total_power <= MINIMUM_TOTAL_POWER)
    {
        cpu_power = MINIMUM_CPU_POWER;
        gpu_power = MINIMUM_GPU_POWER;
    }
    else if (total_power >= MAXIMUM_TOTAL_POWER)
    {
        cpu_power = MAXIMUM_CPU_POWER;
        gpu_power = MAXIMUM_GPU_POWER;
    }
    else
    {
        int max_allowed_gpu_power = MIN((total_power - MINIMUM_CPU_POWER), MAXIMUM_GPU_POWER);
        int highest_possible_gpu_power = -1;
        //int highest_possible_gpu_power_idx = 0;
        int i = 0;
        unsigned int cur_gpu_freq = mt_socfreq_get_cur_gpufreq();
        //int cur_idx = 0;
        unsigned int cur_gpu_power = 0;
        unsigned int next_lower_gpu_power = 0;

        // get GPU highest possible power and index and current power and index and next lower power
        for (; i < Num_of_GPU_OPP; i++)
        {
            if ((mtk_gpu_power[i].gpufreq_power <= max_allowed_gpu_power) &&
                (-1 == highest_possible_gpu_power))
            {
                highest_possible_gpu_power = mtk_gpu_power[i].gpufreq_power;
                //highest_possible_gpu_power_idx = i;
            }

            if (mtk_gpu_power[i].gpufreq_khz == cur_gpu_freq)
            {
                next_lower_gpu_power = cur_gpu_power = mtk_gpu_power[i].gpufreq_power;
                //cur_idx = i;

                if ((i != Num_of_GPU_OPP - 1) && (mtk_gpu_power[i+1].gpufreq_power >= MINIMUM_GPU_POWER))
                {
                    next_lower_gpu_power = mtk_gpu_power[i+1].gpufreq_power;
                }
            }
        }

        // decide GPU power limit by loading
        if (gpu_loading > GPU_L_H_TRIP)
        {
            gpu_power = highest_possible_gpu_power;
        }
        else if (gpu_loading <= GPU_L_L_TRIP)
        {
            gpu_power = MIN(next_lower_gpu_power, highest_possible_gpu_power);
            gpu_power = MAX(gpu_power, MINIMUM_GPU_POWER);
        }
        else
        {
            gpu_power = MIN(highest_possible_gpu_power, cur_gpu_power);
        }

        cpu_power = MIN((total_power - gpu_power), MAXIMUM_CPU_POWER);
    }

    if (cpu_power != last_cpu_power)
    {
        set_adaptive_cpu_power_limit(cpu_power);
	}
    if (gpu_power != last_gpu_power)
    {
        set_adaptive_gpu_power_limit(gpu_power);
    }

    mtktscpu_dprintk("P_adaptive cpu %d, gpu %d\n", cpu_power, gpu_power);

    return 0;
}

static int _adaptive_power(long prev_temp, long curr_temp, unsigned int gpu_loading)
{
	static int triggered = 0, total_power = 0;
	int delta_power = 0;

	if (cl_dev_adp_cpu_state_active == 1)
	{
        mtktscpu_dprintk("_adaptive_power %d %d %d %d %d %d %d\n", FIRST_STEP_TOTAL_POWER_BUDGET, PACKAGE_THETA_JA_RISE, PACKAGE_THETA_JA_FALL, MINIMUM_BUDGET_CHANGE, MINIMUM_CPU_POWER, MAXIMUM_CPU_POWER, MINIMUM_GPU_POWER, MAXIMUM_GPU_POWER);

		/* Check if it is triggered */
		if (!triggered)
		{
			if (curr_temp < TARGET_TJ)
				return 0;
			else
			{
				triggered = 1;
				total_power = FIRST_STEP_TOTAL_POWER_BUDGET;
				mtktscpu_dprintk("_adaptive_power Tp %d, Tc %d, Pt %d\n", prev_temp, curr_temp, total_power);
				return P_adaptive(total_power, gpu_loading);
			}
		}

		/* Adjust total power budget if necessary */
		if ((curr_temp > TARGET_TJ_HIGH) && (curr_temp >= prev_temp))
		{
#if MTKTSCPU_FAST_POLLING
            delta_power = ((curr_temp - prev_temp)*cur_fp_factor) / PACKAGE_THETA_JA_RISE;
#else
            delta_power = (curr_temp - prev_temp) / PACKAGE_THETA_JA_RISE;
#endif
			if (prev_temp > TARGET_TJ_HIGH)
			{
				delta_power = (delta_power > MINIMUM_BUDGET_CHANGE) ? delta_power : MINIMUM_BUDGET_CHANGE;
			}
			total_power -= delta_power;
			total_power = (total_power > MINIMUM_TOTAL_POWER) ? total_power : MINIMUM_TOTAL_POWER;
		}

		if ((curr_temp < TARGET_TJ_LOW) && (curr_temp <= prev_temp))
		{
#if MTKTSCPU_FAST_POLLING
            delta_power = ((prev_temp - curr_temp)*cur_fp_factor) / PACKAGE_THETA_JA_FALL;
#else
            delta_power = (prev_temp - curr_temp) / PACKAGE_THETA_JA_FALL;
#endif
			if (prev_temp < TARGET_TJ_LOW)
			{
				delta_power = (delta_power > MINIMUM_BUDGET_CHANGE) ? delta_power : MINIMUM_BUDGET_CHANGE;
			}
			total_power += delta_power;
			total_power = (total_power < MAXIMUM_TOTAL_POWER) ? total_power : MAXIMUM_TOTAL_POWER;
		}

        mtktscpu_dprintk("_adaptive_power Tp %d, Tc %d, Pt %d\n", prev_temp, curr_temp, total_power);
		return P_adaptive(total_power, gpu_loading);
	}
	else
	{
		if (triggered)
		{
			triggered = 0;
			mtktscpu_dprintk("_adaptive_power Tp %d, Tc %d, Pt %d\n", prev_temp, curr_temp, total_power);
			return P_adaptive(0, 0);
		}
	}

	return 0;
}

static int decide_ttj(void)
{
    int i = 0;
    int active_cooler_id = -1;
    int ret = 117000; // highest allowable TJ
    int temp_cl_dev_adp_cpu_state_active = 0;
    for (; i < MAX_CPT_ADAPTIVE_COOLERS; i++)
    {
        if (cl_dev_adp_cpu_state[i])
        {
            ret = MIN(ret, TARGET_TJS[i]);
            temp_cl_dev_adp_cpu_state_active = 1;

            if (ret == TARGET_TJS[i])
                active_cooler_id = i;
        }
    }
    cl_dev_adp_cpu_state_active = temp_cl_dev_adp_cpu_state_active;
    TARGET_TJ = ret;
    TARGET_TJ_HIGH = TARGET_TJ + 1000;
    TARGET_TJ_LOW = TARGET_TJ - 1000;

    if (0 <= active_cooler_id && MAX_CPT_ADAPTIVE_COOLERS > active_cooler_id)
    {
        PACKAGE_THETA_JA_RISE = PACKAGE_THETA_JA_RISES[active_cooler_id];
        PACKAGE_THETA_JA_FALL = PACKAGE_THETA_JA_FALLS[active_cooler_id];
        MINIMUM_CPU_POWER = MINIMUM_CPU_POWERS[active_cooler_id];
        MAXIMUM_CPU_POWER = MAXIMUM_CPU_POWERS[active_cooler_id];
        MINIMUM_GPU_POWER = MINIMUM_GPU_POWERS[active_cooler_id];
        MAXIMUM_GPU_POWER = MAXIMUM_GPU_POWERS[active_cooler_id];
        MINIMUM_TOTAL_POWER = MINIMUM_CPU_POWER+MINIMUM_GPU_POWER;
        MAXIMUM_TOTAL_POWER = MAXIMUM_CPU_POWER+MAXIMUM_GPU_POWER;
        FIRST_STEP_TOTAL_POWER_BUDGET = FIRST_STEP_TOTAL_POWER_BUDGETS[active_cooler_id];
        MINIMUM_BUDGET_CHANGE = MINIMUM_BUDGET_CHANGES[active_cooler_id];
    }

    return ret;
}
#endif

static int cpufreq_F0x2_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	mtktscpu_dprintk("cpufreq_F0x2_get_max_state\n");
	*state = 1;
	return 0;
}

static int cpufreq_F0x2_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	int i=0;
	mtktscpu_dprintk("cpufreq_F0x2_get_cur_state %s\n", cdev->type);

	for(i=0; i<Num_of_OPP; i++)
	{
		if(!strcmp(cdev->type, &cooler_name[i*20]))
		{
			*state = cl_dev_state[i];
		}
	}
	return 0;
}

static int cpufreq_F0x2_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	int i=0;
	mtktscpu_dprintk("cpufreq_F0x2_set_cur_state %s\n", cdev->type);

	for(i=0; i<Num_of_OPP; i++)
	{
		if(!strcmp(cdev->type, &cooler_name[i*20]))
		{
			cl_dev_state[i]=state;
			_mtktscpu_set_power_consumption_state();
			break;
		}
	}
	return 0;
}

/*
 * cooling device callback functions (mtktscpu_cooling_sysrst_ops)
 * 1 : ON and 0 : OFF
 */
static int tscpu_sysrst_get_max_state(struct thermal_cooling_device *cdev,
				unsigned long *state)
{
	mtktscpu_dprintk("tscpu_sysrst_get_max_state\n");
	*state = 1;
	return 0;
}

static int tscpu_sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				unsigned long *state)
{
	mtktscpu_dprintk("tscpu_sysrst_get_cur_state\n");
	*state = cl_dev_sysrst_state;
	return 0;
}

static int tscpu_sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				unsigned long state)
{
	cl_dev_sysrst_state = state;

	if(cl_dev_sysrst_state == 1)
	{
	    mtkts_dump_cali_info();
		mtktscpu_printk("tscpu_sysrst_set_cur_state = 1\n");
		mtktscpu_dprintk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		mtktscpu_dprintk("*****************************************\n");
		mtktscpu_dprintk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		BUG();
	}
	return 0;
}

#if CPT_ADAPTIVE_AP_COOLER
static int adp_cpu_get_max_state(struct thermal_cooling_device *cdev,
				unsigned long *state)
{
	mtktscpu_dprintk("adp_cpu_get_max_state\n");
	*state = 1;
	return 0;
}

static int adp_cpu_get_cur_state(struct thermal_cooling_device *cdev,
				unsigned long *state)
{
	mtktscpu_dprintk("adp_cpu_get_cur_state\n");
    *state = cl_dev_adp_cpu_state[(cdev->type[13] - '0')];
	//*state = cl_dev_adp_cpu_state;
	return 0;
}

static int adp_cpu_set_cur_state(struct thermal_cooling_device *cdev,
				unsigned long state)
{
    int ttj = 117000;

	cl_dev_adp_cpu_state[(cdev->type[13] - '0')] = state;

    ttj = decide_ttj();

	mtktscpu_dprintk("adp_cpu_set_cur_state[%d] =%d, ttj=%d\n", (cdev->type[13] - '0'), state, ttj);

	{
	    unsigned int gpu_loading = (NULL == mtk_thermal_get_gpu_loading_fp) ? 0 : mtk_thermal_get_gpu_loading_fp();
		_adaptive_power(g_prev_temp, g_curr_temp, (unsigned int) gpu_loading);
		//_adaptive_power(g_prev_temp, g_curr_temp, (unsigned int) 0);
	}
	return 0;
}
#endif

/* bind fan callbacks to fan device */

static struct thermal_cooling_device_ops mtktscpu_cooling_F0x2_ops = {
	.get_max_state = cpufreq_F0x2_get_max_state,
	.get_cur_state = cpufreq_F0x2_get_cur_state,
	.set_cur_state = cpufreq_F0x2_set_cur_state,
};

#if CPT_ADAPTIVE_AP_COOLER
static struct thermal_cooling_device_ops mtktscpu_cooler_adp_cpu_ops = {
	.get_max_state = adp_cpu_get_max_state,
	.get_cur_state = adp_cpu_get_cur_state,
	.set_cur_state = adp_cpu_set_cur_state,
};
#endif

static struct thermal_cooling_device_ops mtktscpu_cooling_sysrst_ops = {
	.get_max_state = tscpu_sysrst_get_max_state,
	.get_cur_state = tscpu_sysrst_get_cur_state,
	.set_cur_state = tscpu_sysrst_set_cur_state,
};

static int mtktscpu_read_opp(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

    unsigned int cpu_power, gpu_power;
    cpu_power = MIN(adaptive_cpu_power_limit, static_cpu_power_limit);

    gpu_power = MIN(adaptive_gpu_power_limit, static_gpu_power_limit);

#if CPT_ADAPTIVE_AP_COOLER
    p += sprintf(p, "%d,%d,%d,%d\n",
                    ((cpu_power != 0x7FFFFFFF)?cpu_power:0),
                    ((gpu_power != 0x7FFFFFFF)?gpu_power:0),
                    ((NULL == mtk_thermal_get_gpu_loading_fp) ? 0 : mtk_thermal_get_gpu_loading_fp()),
                    mt_socfreq_get_cur_gpufreq());

#else
    p += sprintf(p, "%d,%d,0,%d\n",
                    ((cpu_power != 0x7FFFFFFF)?cpu_power:0),
                    ((gpu_power != 0x7FFFFFFF)?gpu_power:0),
                    mt_socfreq_get_cur_gpufreq());
#endif

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static int mtktscpu_read_temperature_info(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

    p += sprintf(p, "current temp:%d\n", read_curr_temp);
  	p += sprintf(p, "Index17:0x%x\n", temp0_0xF020617C);
  	p += sprintf(p, "Index16:0x%x\n", temp1_0xF0206178);
  	p += sprintf(p, "g_adc_ge_t:%d\n", g_adc_ge_t);
    p += sprintf(p, "g_adc_oe_t:%d\n", g_adc_oe_t);
    p += sprintf(p, "g_degc_cali:%d\n", g_degc_cali);
    p += sprintf(p, "g_adc_cali_en_t:%d\n", g_adc_cali_en_t);
    p += sprintf(p, "g_o_slope:%d\n", g_o_slope);
    p += sprintf(p, "g_o_slope_sign:%d\n", g_o_slope_sign);
    p += sprintf(p, "g_id:%d\n", g_id);
    p += sprintf(p, "g_o_vtsmcu2:%d\n", g_o_vtsmcu2);
    p += sprintf(p, "g_o_vtsmcu3:%d\n", g_o_vtsmcu3);
    p += sprintf(p, "g_o_vtsmcu4:%d\n", g_o_vtsmcu4);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static int mtktscpu_talking_flag_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

  	p += sprintf(p, "%d\n", talking_flag);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktscpu_talking_flag_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int lv_talking_flag;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d", &lv_talking_flag) == 1)
	{
		talking_flag = lv_talking_flag;
		mtktscpu_dprintk("mtktscpu_talking_flag_write talking_flag=%d\n", talking_flag);
		return count;
	}
	else
	{
		mtktscpu_dprintk("mtktscpu_talking_flag_write bad argument\n");
	}
	return -EINVAL;
}

static int mtktscpu_set_temperature_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

  	p += sprintf(p, "%d\n", temperature_switch);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktscpu_set_temperature_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int lv_tempe_switch;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	mtktscpu_dprintk("mtktscpu_set_temperature_write\n");

	if (sscanf(desc, "%d", &lv_tempe_switch) == 1)
	{
		temperature_switch = lv_tempe_switch;
        set_thermal_ctrl_trigger_SPM(temperature_switch, tc_mid_trip);
		mtktscpu_dprintk("mtktscpu_set_temperature_write temperature_switch=%d\n", temperature_switch);
		return count;
	}
	else
	{
		mtktscpu_printk("mtktscpu_set_temperature_write bad argument\n");
	}
	return -EINVAL;
}

static int mtktscpu_read_log(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[ mtktscpu_read_log] log = %d\n", mtktscpu_debug_log);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static int mtktscpu_read_cal(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "mtktscpu cal:\n devinfo index(16)=0x%x, devinfo index(17)=0x%x, devinfo index(18)=0x%x\n",
	                get_devinfo_with_index(16), get_devinfo_with_index(17), get_devinfo_with_index(18));

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static int mtktscpu_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;
	int i;

	p += sprintf(p,
	            "[mtktscpu_read]%d\ntrip_0=%d %d %s\ntrip_1=%d %d %s\ntrip_2=%d %d %s\ntrip_3=%d %d %s\ntrip_4=%d %d %s\ntrip_5=%d %d %s\ntrip_6=%d %d %s\ntrip_7=%d %d %s\ntrip_8=%d %d %s\ntrip_9=%d %d %s\ninterval=%d\n",
                num_trip,
				trip_temp[0],g_THERMAL_TRIP[0],g_bind0,
				trip_temp[1],g_THERMAL_TRIP[1],g_bind1,
				trip_temp[2],g_THERMAL_TRIP[2],g_bind2,
				trip_temp[3],g_THERMAL_TRIP[3],g_bind3,
				trip_temp[4],g_THERMAL_TRIP[4],g_bind4,
				trip_temp[5],g_THERMAL_TRIP[5],g_bind5,
				trip_temp[6],g_THERMAL_TRIP[6],g_bind6,
				trip_temp[7],g_THERMAL_TRIP[7],g_bind7,
				trip_temp[8],g_THERMAL_TRIP[8],g_bind8,
				trip_temp[9],g_THERMAL_TRIP[9],g_bind9,
				interval);

    for (i=0; i < Num_of_GPU_OPP; i++)
        p += sprintf(p, "g %d %d %d\n", i, mtk_gpu_power[i].gpufreq_khz, mtk_gpu_power[i].gpufreq_power);

    for (i=0; i < tscpu_num_opp; i++)
        p += sprintf(p, "c %d %d %d %d\n", i, mtk_cpu_power[i].cpufreq_khz, mtk_cpu_power[i].cpufreq_ncpu, mtk_cpu_power[i].cpufreq_power);

    for (i=0; i < CPU_COOLER_NUM; i++)
        p += sprintf(p, "d %d %d\n", i, tscpu_cpu_dmips[i]);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktscpu_write_log(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int log_switch;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d", &log_switch) == 1)
	{
		mtktscpu_debug_log = log_switch;
		mtktscpu_dprintk("mtktscpu_write_log mtktscpu_debug_log=%d\n", mtktscpu_debug_log);
		return count;
	}
	else
	{
		mtktscpu_dprintk("mtktscpu_write_log bad argument\n");
	}
	return -EINVAL;
}

#if CPT_ADAPTIVE_AP_COOLER
static int mtktscpu_read_dtm_setting(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i;
	char *p = buf;

    for (i = 0; i < MAX_CPT_ADAPTIVE_COOLERS; i++)
    {
        p += sprintf(p, "%s%02d\n", adaptive_cooler_name, i);
    	p += sprintf(p, " first_step = %d\n", FIRST_STEP_TOTAL_POWER_BUDGETS[i]);
    	p += sprintf(p, " theta rise = %d\n", PACKAGE_THETA_JA_RISES[i]);
    	p += sprintf(p, " theta fall = %d\n", PACKAGE_THETA_JA_FALLS[i]);
    	p += sprintf(p, " min_budget_change = %d\n", MINIMUM_BUDGET_CHANGES[i]);
    	p += sprintf(p, " m cpu = %d\n", MINIMUM_CPU_POWERS[i]);
    	p += sprintf(p, " M cpu = %d\n", MAXIMUM_CPU_POWERS[i]);
    	p += sprintf(p, " m gpu = %d\n", MINIMUM_GPU_POWERS[i]);
    	p += sprintf(p, " M gpu = %d\n", MAXIMUM_GPU_POWERS[i]);
    }

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktscpu_write_dtm_setting(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[128];
	//char arg_name[32] = {0};
	//int arg_val = 0;
	int len = 0;

	int i_id = -1, i_first_step = -1, i_theta_r = -1, i_theta_f = -1, i_budget_change = -1, i_min_cpu_pwr = -1, i_max_cpu_pwr = -1, i_min_gpu_pwr = -1, i_max_gpu_pwr = -1;


	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (9 <= sscanf(desc, "%d %d %d %d %d %d %d %d %d", &i_id, &i_first_step, &i_theta_r, &i_theta_f, &i_budget_change, &i_min_cpu_pwr, &i_max_cpu_pwr, &i_min_gpu_pwr, &i_max_gpu_pwr))
	{
        mtktscpu_printk("mtktscpu_write_dtm_setting input %d %d %d %d %d %d %d %d %d\n", i_id, i_first_step, i_theta_r, i_theta_f, i_budget_change, i_min_cpu_pwr, i_max_cpu_pwr, i_min_gpu_pwr, i_max_gpu_pwr);

        if (i_id >= 0 && i_id < MAX_CPT_ADAPTIVE_COOLERS)
        {
            if (i_first_step>0) FIRST_STEP_TOTAL_POWER_BUDGETS[i_id] = i_first_step;
            if (i_theta_r>0) PACKAGE_THETA_JA_RISES[i_id] = i_theta_r;
            if (i_theta_f>0) PACKAGE_THETA_JA_FALLS[i_id] = i_theta_f;
            if (i_budget_change>0) MINIMUM_BUDGET_CHANGES[i_id] = i_budget_change;
            if (i_min_cpu_pwr>0) MINIMUM_CPU_POWERS[i_id] = i_min_cpu_pwr;
            if (i_max_cpu_pwr>0) MAXIMUM_CPU_POWERS[i_id] = i_max_cpu_pwr;
            if (i_min_gpu_pwr>0) MINIMUM_GPU_POWERS[i_id] = i_min_gpu_pwr;
            if (i_max_gpu_pwr>0) MAXIMUM_GPU_POWERS[i_id] = i_max_gpu_pwr;

            mtktscpu_printk("mtktscpu_write_dtm_setting applied %d %d %d %d %d %d %d %d %d\n", i_id, FIRST_STEP_TOTAL_POWER_BUDGETS[i_id], PACKAGE_THETA_JA_RISES[i_id], PACKAGE_THETA_JA_FALLS[i_id], MINIMUM_BUDGET_CHANGES[i_id], MINIMUM_CPU_POWERS[i_id], MAXIMUM_CPU_POWERS[i_id], MINIMUM_GPU_POWERS[i_id], MAXIMUM_GPU_POWERS[i_id]);
        }
        else
        {
            mtktscpu_dprintk("mtktscpu_write_dtm_setting out of range\n");
        }

        //MINIMUM_TOTAL_POWER = MINIMUM_CPU_POWER + MINIMUM_GPU_POWER;
        //MAXIMUM_TOTAL_POWER = MAXIMUM_CPU_POWER + MAXIMUM_GPU_POWER;

		return count;
	}
	else
	{
		mtktscpu_dprintk("mtktscpu_write_dtm_setting bad argument\n");
	}
	return -EINVAL;
}

static int mtktscpu_read_gpu_threshold(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i;
	char *p = buf;

    {
        p += sprintf(p, "H %d L %d\n", GPU_L_H_TRIP, GPU_L_L_TRIP);
    }

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktscpu_write_gpu_threshold(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[128];
	int len = 0;

	int gpu_h = -1, gpu_l = -1;


	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (2 <= sscanf(desc, "%d %d", &gpu_h, &gpu_l))
	{
        mtktscpu_printk("mtktscpu_write_gpu_threshold input %d %d\n", gpu_h, gpu_l);

        if ((gpu_h > 0) && (gpu_l > 0) && (gpu_h > gpu_l))
        {
            GPU_L_H_TRIP = gpu_h;
            GPU_L_L_TRIP = gpu_l;

            mtktscpu_printk("mtktscpu_write_gpu_threshold applied %d %d\n", GPU_L_H_TRIP, GPU_L_L_TRIP);
        }
        else
        {
            mtktscpu_dprintk("mtktscpu_write_gpu_threshold out of range\n");
        }

		return count;
	}
	else
	{
		mtktscpu_dprintk("mtktscpu_write_gpu_threshold bad argument\n");
	}
	return -EINVAL;
}
#endif

#if MTKTSCPU_FAST_POLLING
static int mtktscpu_read_fastpoll(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i;
	char *p = buf;

    {
        p += sprintf(p, "trip %d factor %d\n", fast_polling_trip_temp, fast_polling_factor);
    }

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktscpu_write_fastpoll(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[128];
	int len = 0;

	int trip = -1, factor = -1;


	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (2 <= sscanf(desc, "%d %d", &trip, &factor))
	{
        mtktscpu_printk("mtktscpu_write_fastpoll input %d %d\n", trip, factor);

        if ((trip >= 85000) && (factor > 0))
        {
            fast_polling_trip_temp = trip;
            fast_polling_factor = factor;

            mtktscpu_printk("mtktscpu_write_fastpoll applied %d %d\n", fast_polling_trip_temp, fast_polling_factor);
        }
        else
        {
            mtktscpu_dprintk("mtktscpu_write_fastpoll out of range\n");
        }

		return count;
	}
	else
	{
		mtktscpu_dprintk("mtktscpu_write_fastpoll bad argument\n");
	}
	return -EINVAL;
}
#endif

int mtktscpu_register_thermal(void);
void mtktscpu_unregister_thermal(void);

static ssize_t mtktscpu_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len=0,time_msec=0;
	int trip[10]={0};
	int t_type[10]={0};
	int i;
	char bind0[20],bind1[20],bind2[20],bind3[20],bind4[20];
	char bind5[20],bind6[20],bind7[20],bind8[20],bind9[20];
	char desc[512];

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d",
				&num_trip, &trip[0],&t_type[0],bind0, &trip[1],&t_type[1],bind1,
				&trip[2],&t_type[2],bind2, &trip[3],&t_type[3],bind3,
				&trip[4],&t_type[4],bind4, &trip[5],&t_type[5],bind5,
				&trip[6],&t_type[6],bind6, &trip[7],&t_type[7],bind7,
				&trip[8],&t_type[8],bind8, &trip[9],&t_type[9],bind9,
				&time_msec, &MA_len_temp) == 33)
	{

		mtktscpu_dprintk("mtktscpu_write mtktscpu_unregister_thermal MA_len_temp=%d\n",MA_len_temp);

		/*	modify for PTPOD, if disable Thermal,
			PTPOD still need to use this function for getting temperature
		*/
		#if defined(CONFIG_THERMAL)
		mtktscpu_unregister_thermal();
		#endif

		for(i=0; i<num_trip; i++)
			g_THERMAL_TRIP[i] = t_type[i];

		g_bind0[0]=g_bind1[0]=g_bind2[0]=g_bind3[0]=g_bind4[0]=g_bind5[0]=g_bind6[0]=g_bind7[0]=g_bind8[0]=g_bind9[0]='\0';

		for(i=0; i<20; i++)
		{
			g_bind0[i]=bind0[i];
			g_bind1[i]=bind1[i];
			g_bind2[i]=bind2[i];
			g_bind3[i]=bind3[i];
			g_bind4[i]=bind4[i];
			g_bind5[i]=bind5[i];
			g_bind6[i]=bind6[i];
			g_bind7[i]=bind7[i];
			g_bind8[i]=bind8[i];
			g_bind9[i]=bind9[i];
		}

#if CPT_ADAPTIVE_AP_COOLER
        // initialize...
        for (i = 0; i < MAX_CPT_ADAPTIVE_COOLERS; i++)
        {
            TARGET_TJS[i] = 117000;
        }

		if(!strncmp(bind0, adaptive_cooler_name, 13))
		{
		    TARGET_TJS[(bind0[13] - '0')] = trip[0];
		}

		if(!strncmp(bind1, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind1[13] - '0')] = trip[1];
		}

		if(!strncmp(bind2, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind2[13] - '0')] = trip[2];
		}

		if(!strncmp(bind3, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind3[13] - '0')] = trip[3];
		}

		if(!strncmp(bind4, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind4[13] - '0')] = trip[4];
		}

		if(!strncmp(bind5, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind5[13] - '0')] = trip[5];
		}

		if(!strncmp(bind6, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind6[13] - '0')] = trip[6];
		}

		if(!strncmp(bind7, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind7[13] - '0')] = trip[7];
		}

		if(!strncmp(bind8, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind8[13] - '0')] = trip[8];
		}

		if(!strncmp(bind9, adaptive_cooler_name, 13))
		{
			TARGET_TJS[(bind9[13] - '0')] = trip[9];
		}

		mtktscpu_dprintk("mtktscpu_write TTJ0=%d, TTJ1=%d, TTJ2=%d\n", TARGET_TJS[0], TARGET_TJS[1], TARGET_TJS[2]);
#endif

		mtktscpu_dprintk("mtktscpu_write g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n",
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
		mtktscpu_dprintk("mtktscpu_write cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s\n",
				g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9);

		for(i=0; i<num_trip; i++)
		{
			trip_temp[i]=trip[i];
		}

		interval=time_msec;

		mtktscpu_dprintk("mtktscpu_write trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,time_ms=%d, num_trip=%d\n",
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval, num_trip);


		//get temp, set high low threshold
/*
        curr_temp = get_immediate_temp();
		for(i=0; i<num_trip; i++)
		{
			if(curr_temp>trip_temp[i])
				break;
		}
		if(i==0)
		{
			mtktscpu_printk("mtktscpu_write setting error");
		}
		else if(i==num_trip)
			set_high_low_threshold(trip_temp[i-1], 10000);
		else
			set_high_low_threshold(trip_temp[i-1], trip_temp[i]);
*/
        #if defined(CONFIG_THERMAL)
		mtktscpu_dprintk("mtktscpu_write mtktscpu_register_thermal\n");
		mtktscpu_register_thermal();
        #endif
		proc_write_flag=1;

		return count;
	}
	else
	{
		mtktscpu_dprintk("mtktscpu_write bad argument\n");
	}

	return -EINVAL;
}

int mtktscpu_register_DVFS_hotplug_cooler(void)
{
#if defined(CONFIG_THERMAL)
	int i;

	mtktscpu_dprintk("mtktscpu_register_DVFS_hotplug_cooler\n");
	for(i=0; i<Num_of_OPP; i++)
	{
		cl_dev[i] = mtk_thermal_cooling_device_register(&cooler_name[i*20], NULL,
					 &mtktscpu_cooling_F0x2_ops);
	}
	cl_dev_sysrst = mtk_thermal_cooling_device_register("mtktscpu-sysrst", NULL,
					&mtktscpu_cooling_sysrst_ops);
#if CPT_ADAPTIVE_AP_COOLER
    cl_dev_adp_cpu[0] = mtk_thermal_cooling_device_register("cpu_adaptive_0", NULL,
					&mtktscpu_cooler_adp_cpu_ops);

    cl_dev_adp_cpu[1] = mtk_thermal_cooling_device_register("cpu_adaptive_1", NULL,
					&mtktscpu_cooler_adp_cpu_ops);

    cl_dev_adp_cpu[2] = mtk_thermal_cooling_device_register("cpu_adaptive_2", NULL,
					&mtktscpu_cooler_adp_cpu_ops);
#endif

#endif
	return 0;
}

int mtktscpu_register_thermal(void)
{
#if defined(CONFIG_THERMAL)
	mtktscpu_dprintk("mtktscpu_register_thermal\n");

	/* trips : trip 0~3 */
	thz_dev = mtk_thermal_zone_device_register("mtktscpu", num_trip, NULL,
				&mtktscpu_dev_ops, 0, 0, 0, interval);
#endif
	return 0;
}

void mtktscpu_unregister_DVFS_hotplug_cooler(void)
{
#if defined(CONFIG_THERMAL)
	int i;
	for(i=0; i<Num_of_OPP; i++)
	{
		if(cl_dev[i])
		{
			mtk_thermal_cooling_device_unregister(cl_dev[i]);
			cl_dev[i] = NULL;
		}
	}
	if(cl_dev_sysrst) {
		mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
		cl_dev_sysrst = NULL;
	}

#if CPT_ADAPTIVE_AP_COOLER
    if(cl_dev_adp_cpu[0]) {
		mtk_thermal_cooling_device_unregister(cl_dev_adp_cpu[0]);
		cl_dev_adp_cpu[0] = NULL;
	}

	if(cl_dev_adp_cpu[1]) {
		mtk_thermal_cooling_device_unregister(cl_dev_adp_cpu[1]);
		cl_dev_adp_cpu[1] = NULL;
	}

	if(cl_dev_adp_cpu[2]) {
		mtk_thermal_cooling_device_unregister(cl_dev_adp_cpu[2]);
		cl_dev_adp_cpu[2] = NULL;
	}
#endif

#endif
}

void mtktscpu_unregister_thermal(void)
{
#if defined(CONFIG_THERMAL)
	mtktscpu_dprintk("mtktscpu_unregister_thermal\n");
	if(thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
#endif
}

static int mtk_thermal_suspend(struct platform_device *dev, pm_message_t state)
{
	//int temp=0;
    //int cnt=0;

	mtktscpu_dprintk("mtk_thermal_suspend\n");
#if MTK_TS_DEBUG_LOG
    mtktscpu_printk("mtk_thermal_suspend");
    mtkts_suspend_counter++;
#endif
    if(talking_flag==false)
    {
    	mtktscpu_dprintk("mtk_thermal_suspend no talking\n");


		// disable ALL periodoc temperature sensing point
        THERMAL_WRAP_WR32(0x00000000, TEMPMONCTL0);

		/*
		fix ALPS00848017
		can't turn off thermal, this will cause PTPOD  issue abnormal interrupt
		and let system crash.(because PTPOD can't get thermal's temperature)
		*/
		//mtktscpu_thermal_clock_off();

        THERMAL_WRAP_WR32(DRV_Reg32(TS_CON0) | 0x000000C0, TS_CON0); // turn off the sensor buffer to save power
		//THERMAL_WRAP_WR32(DRV_Reg32(TS_CON1) | 0x000000C0, TS_CON1); // turn off the sensor buffer to save power
    }
	return 0;
}

static int mtk_thermal_resume(struct platform_device *dev)
{
	mtktscpu_dprintk("mtk_thermal_resume\n");
#if MTK_TS_DEBUG_LOG
    mtktscpu_printk("[mtk_thermal_resume] ");
    mtkts_resume_counter++;
#endif
	if(talking_flag==false)
	{
		thermal_reset_and_initial();
		set_thermal_ctrl_trigger_SPM(trip_temp[0], tc_mid_trip);
	}

	return 0;
}

void thermal_SW_protect(void)
{
	int temp1,temp2,temp3;

	temp1 = get_immediate_temp1();
    temp2 = get_immediate_temp2();
	temp3 = get_immediate_temp3();
    mtktscpu_printk("thermal_SW_protect:TS3=%d,TS2=%d,TS4=%d\n",temp1,temp2,temp3);
    if((temp1 > trip_temp[0])||(temp2 > trip_temp[0]) ||(temp3 > trip_temp[0]))
    {
        //mtktscpu_printk("temp1=%d, temp2=%d, temp3=%d\n",temp1,temp2,temp3);
        mtktscpu_printk("reset, reset, reset, reset, reset!!!\n");
        BUG();
     }
}


/*must wait until AUXADC initial ready*/
static int thermal_prob(struct platform_device *dev)
{


    mtktscpu_printk("[thermal_prob] ");



#if 1 //SW reset
	thermal_SW_protect();
#else //HW reset
	int temp1,temp2;


	/*read temp first than set SPM reset*/
    temp1 = get_immediate_temp1();
    temp2 = get_immediate_temp2();
    if((temp1 > 0) ||(temp2 > 0)){
	    mtktscpu_printk("temp1=%d\n", get_immediate_temp1());
	    mtktscpu_printk("temp2=%d\n", get_immediate_temp2());
		//set_thermal_ctrl_trigger_SPM2(25000); // Move thermal HW protection ahead...
        set_thermal_ctrl_trigger_SPM2(trip_temp[0], tc_mid_trip); // Move thermal HW protection ahead...
    }
#endif
    //mtktscpu_printk("thermal_prob done\n");

    return 0;
}


static struct platform_driver mtk_thermal_driver = {
	.remove     = NULL,
	.shutdown   = NULL,
	.probe      = thermal_prob,
	.suspend	= mtk_thermal_suspend,
	.resume		= mtk_thermal_resume,
	.driver     = {
		.name = THERMAL_NAME,
    },
};

#if MTK_TS_CPU_RT
static int ktp_limited = -275000;

static int ktp_thread(void *arg)
{
    int max_temp = 0;
    //int threshold = 0;
    struct sched_param param = { .sched_priority = 98 }; //

    sched_setscheduler(current, SCHED_FIFO, &param);
    set_current_state(TASK_INTERRUPTIBLE);

    mtktscpu_printk("ktp_thread 1st run\n");

    schedule();

    for(;;)
	{
        int temp_tc_mid_trip = tc_mid_trip;
        int temp_ktp_limited = ktp_limited;

	    mtktscpu_printk("ktp_thread awake\n");
	    if (kthread_should_stop()) break;

	    max_temp = MAX(Temp_TS2, Temp_TS3);
	    max_temp = MAX(max_temp, Temp_TS4);
	    mtktscpu_printk("ktp_thread temp=%d\n", max_temp);

	    if ((temp_tc_mid_trip > -275000) && (max_temp >= temp_tc_mid_trip)) // trip_temp[1] should be shutdown point...
        {
            // Do what ever we want
            mtktscpu_printk("ktp_thread overheat %d\n", max_temp);

            // freq/volt down or cpu down or backlight down or charging down...
            mt_cpufreq_thermal_protect(489);
            mt_gpufreq_thermal_protect(411);

            ktp_limited = temp_tc_mid_trip;

            msleep(20 * 1000);
        }
        else if ((temp_ktp_limited > -275000) && (max_temp < temp_ktp_limited))
        {
            unsigned int final_limit;
            final_limit = MIN(static_cpu_power_limit, adaptive_cpu_power_limit);
            mtktscpu_printk("ktp_thread unlimit cpu=%d\n", final_limit);
            mt_cpufreq_thermal_protect((final_limit != 0x7FFFFFFF) ? final_limit : 0);


            final_limit = MIN(static_gpu_power_limit, adaptive_gpu_power_limit);
            mtktscpu_printk("ktp_thread unlimit gpu=%d\n", final_limit);
            mt_gpufreq_thermal_protect((final_limit != 0x7FFFFFFF) ? final_limit : 0);

            ktp_limited = -275000;

            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
        }
        else
        {
            mtktscpu_printk("ktp_thread else temp=%d, trip=%d, ltd=%d\n", max_temp, temp_tc_mid_trip, temp_ktp_limited);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
        }
	}

	mtktscpu_printk("ktp_thread stopped\n");

    return 0;
}
#endif




static int __init mtktscpu_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktscpu_dir = NULL;

	mtktscpu_printk("[mtktscpu_init] ");


	thermal_cal_prepare();
	thermal_calibration();

	THERMAL_WRAP_WR32(DRV_Reg32(TS_CON0) | 0x000000C0, TS_CON0); // turn off the sensor buffer to save power

	thermal_reset_and_initial();

#if MTK_TS_CPU_RT
    {
    	mtktscpu_dprintk("mtktscpu_register_thermal creates kthermp\n");
        ktp_thread_handle = kthread_create(ktp_thread, (void *) NULL, "kthermp");
        if (IS_ERR(ktp_thread_handle)) {
			ktp_thread_handle = NULL;
			mtktscpu_printk("mtktscpu_register_thermal kthermp creation fails\n");
		    goto err_unreg;
		}
		wake_up_process(ktp_thread_handle);
    }
#endif

#if defined(CONFIG_THERMAL)
    err= request_irq(THERM_CTRL_IRQ_ID, thermal_interrupt_handler, IRQF_TRIGGER_LOW, THERMAL_NAME, NULL);
	if(err)
		mtktscpu_printk("mtktscpu_init IRQ register fail\n");
#endif

	set_thermal_ctrl_trigger_SPM(trip_temp[0], tc_mid_trip); // Move thermal HW protection ahead...



#if defined(CONFIG_THERMAL)
    err = platform_driver_register(&mtk_thermal_driver);
    if (err)
    {
        mtktscpu_printk("thermal driver callback register failed..\n");
        return err;
    }

	err = init_cooler();
	if(err)
		return err;

	err = mtktscpu_register_DVFS_hotplug_cooler();
	if(err){
        mtktscpu_printk("mtktscpu_register_DVFS_hotplug_cooler fail\n");
		return err;
	}
	err = mtktscpu_register_thermal();
	if(err){
        mtktscpu_printk("mtktscpu_register_thermal fail\n");
		goto err_unreg;
	}


	mtktscpu_dir = proc_mkdir("mtktscpu", NULL);
	if (!mtktscpu_dir)
	{
		mtktscpu_printk("mtktscpu_init mkdir /proc/mtktscpu failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktscpu", S_IRUGO | S_IWUSR, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read;
			entry->write_proc = mtktscpu_write;
		}

		entry = create_proc_entry("mtktscpu_log", S_IRUGO | S_IWUSR, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read_log;
			entry->write_proc = mtktscpu_write_log;
		}

		entry = create_proc_entry("mtktscpu_opp", S_IRUGO, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read_opp;
			entry->write_proc = NULL;
		}

		entry = create_proc_entry("mtktscpu_cal", S_IRUGO, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read_cal;
			entry->write_proc = NULL;
		}
		entry = create_proc_entry("mtktscpu_read_temperature", S_IRUGO, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc =mtktscpu_read_temperature_info;
			entry->write_proc = NULL;
		}
		entry = create_proc_entry("mtktscpu_set_temperature", S_IRUGO | S_IWUSR, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc =mtktscpu_set_temperature_read;
			entry->write_proc = mtktscpu_set_temperature_write;
		}
		entry = create_proc_entry("mtktscpu_talking_flag", S_IRUGO | S_IWUSR, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_talking_flag_read;
			entry->write_proc = mtktscpu_talking_flag_write;
		}
#if CPT_ADAPTIVE_AP_COOLER
		entry = create_proc_entry("mtktscpu_dtm_setting", S_IRUGO | S_IWUSR | S_IWGRP, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read_dtm_setting;
			entry->write_proc = mtktscpu_write_dtm_setting;
			entry->gid = 1000;
		}

		entry = create_proc_entry("mtktscpu_gpu_threshold", S_IRUGO | S_IWUSR | S_IWGRP, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read_gpu_threshold;
			entry->write_proc = mtktscpu_write_gpu_threshold;
			entry->gid = 1000;
		}
#endif


#if MTKTSCPU_FAST_POLLING
        entry = create_proc_entry("mtktscpu_fastpoll", S_IRUGO | S_IWUSR | S_IWGRP, mtktscpu_dir);
		if (entry)
		{
			entry->read_proc = mtktscpu_read_fastpoll;
			entry->write_proc = mtktscpu_write_fastpoll;
			entry->gid = 1000;
		}
#endif
	}
#endif
	return 0;

	/*	modify for PTPOD, if disable Thermal,
		PTPOD still need to use this function for getting temperature
	*/
#if defined(CONFIG_THERMAL)
	err_unreg:
	mtktscpu_unregister_DVFS_hotplug_cooler();
	return err;
#endif

}


static void __exit mtktscpu_exit(void)
{
#if defined(CONFIG_THERMAL)
	mtktscpu_dprintk("mtktscpu_exit\n");

#if MTK_TS_CPU_RT
	if (ktp_thread_handle)
    	kthread_stop(ktp_thread_handle);
#endif

	mtktscpu_unregister_thermal();
	mtktscpu_unregister_DVFS_hotplug_cooler();

#endif
}

module_init(mtktscpu_init);
module_exit(mtktscpu_exit);


//late_initcall(thermal_late_init);
//device_initcall(thermal_late_init);


