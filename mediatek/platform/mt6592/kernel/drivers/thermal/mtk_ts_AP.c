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
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <asm/uaccess.h>

#include <mach/system.h>
#include "mach/mtk_thermal_monitor.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"

static unsigned int interval = 0; /* seconds, 0 : no auto polling */
static int trip_temp[10] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};
static struct thermal_zone_device *thz_dev;
static int mtkts_AP_debug_log = 0;
static int kernelmode = 0;
static int g_THERMAL_TRIP[10] = {0,0,0,0,0,0,0,0,0,0};
static int num_trip=0;
static char g_bind0[20]={0};
static char g_bind1[20]={0};
static char g_bind2[20]={0};
static char g_bind3[20]={0};
static char g_bind4[20]={0};
static char g_bind5[20]={0};
static char g_bind6[20]={0};
static char g_bind7[20]={0};
static char g_bind8[20]={0};
static char g_bind9[20]={0};
extern int read_tAP_value(void);
//static int APtery2_write_flag=0;

#define mtkts_AP_TEMP_CRIT 60000 /* 60.000 degree Celsius */


// 1: turn on arbitration reasonable temo; 0: turn off
#define AUTO_ARBITRATION_REASONABLE_TEMP (0)


#if AUTO_ARBITRATION_REASONABLE_TEMP
#define XTAL_BTS_TEMP_DIFF 10000  //10 degree

extern int mtktscpu_get_Tj_temp(void);
extern int mtktsxtal_get_xtal_temp(void);
#endif


#define mtkts_AP_dprintk(fmt, args...)   \
do {                                    \
	if (mtkts_AP_debug_log) {                \
		xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", fmt, ##args); \
	}                                   \
} while(0)


#define INPUT_PARAM_FROM_USER_AP

/*
 * kernel fopen/fclose
 */
/*
static mm_segment_t oldfs;

static void my_close(int fd)
{
	set_fs(oldfs);
	sys_close(fd);
}

static int my_open(char *fname, int flag)
{
	oldfs = get_fs();
    set_fs(KERNEL_DS);
    return sys_open(fname, flag, 0);
}
*/
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
extern int IMM_IsAdcInitReady(void);
typedef struct{
    INT32 APteryTemp;
    INT32 TemperatureR;
}AP_TEMPERATURE;



#ifdef INPUT_PARAM_FROM_USER_AP
static int g_RAP_pull_up_R = 39000;
static int g_TAP_over_critical_low = 68237;
static int g_RAP_pull_up_voltage = 1800;
static int g_RAP_ntc_table = 4;  //default is AP_NTC_10

//static int AP_NTC_BL197=0;
//static int AP_NTC_TSM_1=0;
//static int AP_NTC_10_SEN_1=0;
//static int AP_NTC_10=0;
//static int AP_NTC_47=0;
#else
#define RAP_PULL_UP_R             121000 //121K,pull up resister
#define TAP_OVER_CRITICAL_LOW     68237  //base on 10K NTC temp default value
#define RAP_PULL_UP_VOLT          2800 //2.8V ,pull up voltage


#define AP_NTC_10 1
#define AP_NTC_47 0
#endif

static int g_AP_TemperatureR = 0;
//AP_TEMPERATURE AP_Temperature_Table[] = {0};

#ifdef INPUT_PARAM_FROM_USER_AP
static AP_TEMPERATURE AP_Temperature_Table[] = {
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0}
};


//AP_NTC_BL197
AP_TEMPERATURE AP_Temperature_Table1[] = {
	{-20,74354},
	{-15,57626},
	{-10,45068},
	{ -5,35548},
	{  0,28267},
	{  5,22650},
	{ 10,18280},
	{ 15,14855},
	{ 20,12151},
	{ 25,10000},
	{ 30,8279},
	{ 35,6892},
	{ 40,5768},
	{ 45,4852},
	{ 50,4101},
	{ 55,3483},
	{ 60,2970}
};

//AP_NTC_TSM_1
AP_TEMPERATURE AP_Temperature_Table2[] = {
	{-20,70603},
	{-15,55183},
	{-10,43499},
	{ -5,34569},
	{  0,27680},
	{  5,22316},
	{ 10,18104},
	{ 15,14773},
	{ 20,12122},
	{ 25,10000},
	{ 30,8294},
	{ 35,6915},
	{ 40,5795},
	{ 45,4882},
	{ 50,4133},
	{ 55,3516},
	{ 60,3004}
};

//AP_NTC_10_SEN_1
AP_TEMPERATURE AP_Temperature_Table3[] = {
	 {-20,74354},
	 {-15,57626},
	 {-10,45068},
	 { -5,35548},
	 {  0,28267},
	 {  5,22650},
	 { 10,18280},
	 { 15,14855},
	 { 20,12151},
	 { 25,10000},
	 { 30,8279},
	 { 35,6892},
	 { 40,5768},
	 { 45,4852},
	 { 50,4101},
	 { 55,3483},
	 { 60,2970}
};

//AP_NTC_10
AP_TEMPERATURE AP_Temperature_Table4[] = {
    {-20,68237},
    {-15,53650},
    {-10,42506},
    { -5,33892},
    {  0,27219},
    {  5,22021},
    { 10,17926},
    { 15,14674},
    { 20,12081},
    { 25,10000},
    { 30,8315},
    { 35,6948},
    { 40,5834},
    { 45,4917},
    { 50,4161},
    { 55,3535},
    { 60,3014}
};


//AP_NTC_47
AP_TEMPERATURE AP_Temperature_Table5[] = {
    {-20,483954},
    {-15,360850},
    {-10,271697},
    { -5,206463},
    {  0,158214},
    {  5,122259},
    { 10,95227},
    { 15,74730},
    { 20,59065},
    { 25,47000},
    { 30,37643},
    { 35,30334},
    { 40,24591},
    { 45,20048},
    { 50,16433},
    { 55,13539},
    { 60,11210}
};


//NTCG104EF104F(100K)
AP_TEMPERATURE AP_Temperature_Table6[] = {
    {-20,1135000},
    {-15,837800},
    {-10,624100},
    { -5,469100},
    {  0,355600},
    {  5,271800},
    { 10,209400},
    { 15,162500},
    { 20,127000},
    { 25,100000},
    { 30,79230},
    { 35,63180},
    { 40,50680},
    { 45,40900},
    { 50,33190},
    { 55,27090},
    { 60,22220}
};

#else
#if defined(AP_NTC_BL197)
	AP_TEMPERATURE AP_Temperature_Table[] = {
		{-20,74354},
		{-15,57626},
		{-10,45068},
		{ -5,35548},
		{  0,28267},
		{  5,22650},
		{ 10,18280},
		{ 15,14855},
		{ 20,12151},
		{ 25,10000},
		{ 30,8279},
		{ 35,6892},
		{ 40,5768},
		{ 45,4852},
		{ 50,4101},
		{ 55,3483},
		{ 60,2970}
	};
#endif

#if defined(AP_NTC_TSM_1)
AP_TEMPERATURE AP_Temperature_Table[] = {
		{-20,70603},
		{-15,55183},
		{-10,43499},
		{ -5,34569},
		{  0,27680},
		{  5,22316},
		{ 10,18104},
		{ 15,14773},
		{ 20,12122},
		{ 25,10000},
		{ 30,8294},
		{ 35,6915},
		{ 40,5795},
		{ 45,4882},
		{ 50,4133},
		{ 55,3516},
		{ 60,3004}
		};
#endif

#if defined(AP_NTC_10_SEN_1)
AP_TEMPERATURE AP_Temperature_Table[] = {
		 {-20,74354},
		 {-15,57626},
		 {-10,45068},
		 { -5,35548},
		 {  0,28267},
		 {  5,22650},
		 { 10,18280},
		 { 15,14855},
		 { 20,12151},
		 { 25,10000},
		 { 30,8279},
		 { 35,6892},
		 { 40,5768},
		 { 45,4852},
		 { 50,4101},
		 { 55,3483},
		 { 60,2970}
		};
#endif

#if (AP_NTC_10 == 1)
    AP_TEMPERATURE AP_Temperature_Table[] = {
        {-20,68237},
        {-15,53650},
        {-10,42506},
        { -5,33892},
        {  0,27219},
        {  5,22021},
        { 10,17926},
        { 15,14674},
        { 20,12081},
        { 25,10000},
        { 30,8315},
        { 35,6948},
        { 40,5834},
        { 45,4917},
        { 50,4161},
        { 55,3535},
        { 60,3014}
    };
#endif

#if (AP_NTC_47 == 1)
    AP_TEMPERATURE AP_Temperature_Table[] = {
        {-20,483954},
        {-15,360850},
        {-10,271697},
        { -5,206463},
        {  0,158214},
        {  5,122259},
        { 10,95227},
        { 15,74730},
        { 20,59065},
        { 25,47000},
        { 30,37643},
        { 35,30334},
        { 40,24591},
        { 45,20048},
        { 50,16433},
        { 55,13539},
        { 60,11210}
    };
#endif
#endif


/* convert register to temperature  */
static INT16 APtThermistorConverTemp(INT32 Res)
{
    int i=0;
    INT32 RES1=0,RES2=0;
    INT32 TAP_Value=-200,TMP1=0,TMP2=0;

    if(Res>=AP_Temperature_Table[0].TemperatureR)
    {
        TAP_Value = -20;
    }
    else if(Res<=AP_Temperature_Table[16].TemperatureR)
    {
        TAP_Value = 60;
    }
    else
    {
        RES1=AP_Temperature_Table[0].TemperatureR;
        TMP1=AP_Temperature_Table[0].APteryTemp;

        for(i=0;i<=16;i++)
        {
            if(Res>=AP_Temperature_Table[i].TemperatureR)
            {
                RES2=AP_Temperature_Table[i].TemperatureR;
                TMP2=AP_Temperature_Table[i].APteryTemp;
                break;
            }
            else
            {
                RES1=AP_Temperature_Table[i].TemperatureR;
                TMP1=AP_Temperature_Table[i].APteryTemp;
            }
        }

        TAP_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))/(RES1-RES2);
    }

    #if 0
    xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", "APtThermistorConverTemp() : TAP_Value = %d\n",TAP_Value);
    xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", "APtThermistorConverTemp() : Res = %d\n",Res);
    xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", "APtThermistorConverTemp() : RES1 = %d\n",RES1);
    xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", "APtThermistorConverTemp() : RES2 = %d\n",RES2);
    xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", "APtThermistorConverTemp() : TMP1 = %d\n",TMP1);
    xlog_printk(ANDROID_LOG_INFO, "Power/AP_Thermal", "APtThermistorConverTemp() : TMP2 = %d\n",TMP2);
    #endif

    return TAP_Value;
}

/* convert ADC_AP_temp_volt to register */
/*Volt to Temp formula same with 6589*/
static INT16 APtVoltToTemp(UINT32 dwVolt)
{
    INT32 TRes;
    INT32 dwVCriAP = 0;
    INT32 sAPTMP = -100;

    //SW workaround-----------------------------------------------------
    //dwVCriAP = (TAP_OVER_CRITICAL_LOW * 1800) / (TAP_OVER_CRITICAL_LOW + 39000);
    #ifdef INPUT_PARAM_FROM_USER_AP
    //dwVCriAP = (TAP_OVER_CRITICAL_LOW * RAP_PULL_UP_VOLT) / (TAP_OVER_CRITICAL_LOW + RAP_PULL_UP_R);
    dwVCriAP = (g_TAP_over_critical_low * g_RAP_pull_up_voltage) / (g_TAP_over_critical_low + g_RAP_pull_up_R);

    if(dwVolt > dwVCriAP)
    {
        TRes = g_TAP_over_critical_low;
    }
    else
    {
        //TRes = (39000*dwVolt) / (1800-dwVolt);
       // TRes = (RAP_PULL_UP_R*dwVolt) / (RAP_PULL_UP_VOLT-dwVolt);
        TRes = (g_RAP_pull_up_R*dwVolt) / (g_RAP_pull_up_voltage-dwVolt);
    }
    //------------------------------------------------------------------
	#else
    //SW workaround-----------------------------------------------------
    //dwVCriAP = (TAP_OVER_CRITICAL_LOW * 1800) / (TAP_OVER_CRITICAL_LOW + 39000);
    dwVCriAP = (TAP_OVER_CRITICAL_LOW * RAP_PULL_UP_VOLT) / (TAP_OVER_CRITICAL_LOW + RAP_PULL_UP_R);

    if(dwVolt > dwVCriAP)
    {
        TRes = TAP_OVER_CRITICAL_LOW;
    }
    else
    {
        //TRes = (39000*dwVolt) / (1800-dwVolt);
        TRes = (RAP_PULL_UP_R*dwVolt) / (RAP_PULL_UP_VOLT-dwVolt);
    }
    //------------------------------------------------------------------
    #endif
    g_AP_TemperatureR = TRes;

    /* convert register to temperature */
    sAPTMP = APtThermistorConverTemp(TRes);

    return sAPTMP;
}

static int get_hw_AP_temp(void)
{

	int ret = 0, data[4], i, ret_value = 0, ret_temp = 0, output;
	int times=1, Channel=1;//6592=1(AUX_IN1_NTC)

	if( IMM_IsAdcInitReady() == 0 )
	{
        printk("[thermal_auxadc_get_data]: AUXADC is not ready\n");
		return 0;
	}

	i = times;
	while (i--)
	{
		ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
		ret += ret_temp;
		mtkts_AP_dprintk("[thermal_auxadc_get_data(AUX_IN0_NTC)]: ret_temp=%d\n",ret_temp);
	}

#if 0
	Channel = 0;
	ret = 0 ;
	ret_temp = 0;
	i = times;
	while (i--)
	{
		ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
		ret += ret_temp;
		printk("[thermal_auxadc_get_data(ADCIN %d)]: ret_temp=%d\n",Channel,ret_temp);
	}

	Channel = 2;
	ret = 0 ;
	ret_temp = 0;
	i = times;
	while (i--)
	{
		ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
		ret += ret_temp;
		printk("[thermal_auxadc_get_data(ADCIN %d)]: ret_temp=%d\n",Channel,ret_temp);
	}
#endif

	//ret = ret*1500/4096	;
	ret = ret*1800/4096;//82's ADC power
	mtkts_AP_dprintk("APtery output mV = %d\n",ret);
	output = APtVoltToTemp(ret);
	mtkts_AP_dprintk("APtery output temperature = %d\n",output);

	return output;
}

static DEFINE_MUTEX(AP_lock);
int ts_AP_at_boot_time=0;
static int mtkts_AP_get_hw_temp(void)
{
	int t_ret=0;
//	static int AP[60]={0};
//	int i=0;

	mutex_lock(&AP_lock);

    //get HW AP temp (TSAP)
    //cat /sys/class/power_supply/AP/AP_temp
	t_ret = get_hw_AP_temp();
	t_ret = t_ret * 1000;

	mutex_unlock(&AP_lock);

    if (t_ret > 60000) // abnormal high temp
        printk("[Power/AP_Thermal] T_AP=%d\n", t_ret);

	mtkts_AP_dprintk("[mtkts_AP_get_hw_temp] T_AP, %d\n", t_ret);
	return t_ret;
}

static int mtkts_AP_get_temp(struct thermal_zone_device *thermal,
			       unsigned long *t)
{
	*t = mtkts_AP_get_hw_temp();
	return 0;
}

static int mtkts_AP_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
		mtkts_AP_dprintk("[mtkts_AP_bind] %s\n", cdev->type);
	}
	else
	{
		return 0;
	}

	if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
		mtkts_AP_dprintk("[mtkts_AP_bind] error binding cooling dev\n");
		return -EINVAL;
	} else {
		mtkts_AP_dprintk("[mtkts_AP_bind] binding OK, %d\n", table_val);
	}

	return 0;
}

static int mtkts_AP_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
    int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
		mtkts_AP_dprintk("[mtkts_AP_unbind] %s\n", cdev->type);
	}
	else
		return 0;

	if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
		mtkts_AP_dprintk("[mtkts_AP_unbind] error unbinding cooling dev\n");
		return -EINVAL;
	} else {
		mtkts_AP_dprintk("[mtkts_AP_unbind] unbinding OK\n");
	}

	return 0;
}

static int mtkts_AP_get_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
			     : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtkts_AP_set_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtkts_AP_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtkts_AP_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip];
	return 0;
}

static int mtkts_AP_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = mtkts_AP_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtkts_AP_dev_ops = {
	.bind = mtkts_AP_bind,
	.unbind = mtkts_AP_unbind,
	.get_temp = mtkts_AP_get_temp,
	.get_mode = mtkts_AP_get_mode,
	.set_mode = mtkts_AP_set_mode,
	.get_trip_type = mtkts_AP_get_trip_type,
	.get_trip_temp = mtkts_AP_get_trip_temp,
	.get_crit_temp = mtkts_AP_get_crit_temp,
};

/*
static int dis_charge_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
		*state = 1;
		return 0;
}
static int dis_charge_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
		*state = cl_dev_dis_charge_state;
		return 0;
}
static int dis_charge_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
    cl_dev_dis_charge_state = state;
    if(cl_dev_dis_charge_state == 1) {
        mtkts_AP_dprintk("[dis_charge_set_cur_state] disable charging\n");
    }
    return 0;
}
*/
/*
static int sysrst_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	*state = 1;
	return 0;
}
static int sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	*state = cl_dev_sysrst_state;
	return 0;
}
static int sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	cl_dev_sysrst_state = state;
	if(cl_dev_sysrst_state == 1)
	{
		printk("Power/AP_Thermal: reset, reset, reset!!!");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		printk("*****************************************");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

		BUG();
		//arch_reset(0,NULL);
	}
	return 0;
}
*/
/*
static struct thermal_cooling_device_ops mtkts_AP_cooling_dis_charge_ops = {
	.get_max_state = dis_charge_get_max_state,
	.get_cur_state = dis_charge_get_cur_state,
	.set_cur_state = dis_charge_set_cur_state,
};*/
/*static struct thermal_cooling_device_ops mtkts_AP_cooling_sysrst_ops = {
	.get_max_state = sysrst_get_max_state,
	.get_cur_state = sysrst_get_cur_state,
	.set_cur_state = sysrst_set_cur_state,
};*/


static int mtkts_AP_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[mtkts_AP_read] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\n\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,\n\
g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\n\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n\
cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\n\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s,time_ms=%d\n",
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9],
				g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9,
				interval*1000);


	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

int mtkts_AP_register_thermal(void);
void mtkts_AP_unregister_thermal(void);

static ssize_t mtkts_AP_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
#if AUTO_ARBITRATION_REASONABLE_TEMP
	int Ap_temp=0,XTAL_temp=0,CPU_Tj=0;
    int AP_XTAL_diff=0;
#endif
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

	if (sscanf(desc, "%d %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d",
				&num_trip, &trip[0],&t_type[0],bind0, &trip[1],&t_type[1],bind1,
				&trip[2],&t_type[2],bind2, &trip[3],&t_type[3],bind3,
				&trip[4],&t_type[4],bind4, &trip[5],&t_type[5],bind5,
				&trip[6],&t_type[6],bind6, &trip[7],&t_type[7],bind7,
				&trip[8],&t_type[8],bind8, &trip[9],&t_type[9],bind9,
				&time_msec) == 32)
	{
		mtkts_AP_dprintk("[mtkts_AP_write] mtkts_AP_unregister_thermal\n");
		mtkts_AP_unregister_thermal();

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

		mtkts_AP_dprintk("[mtkts_AP_write] g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\
					g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n",
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
		mtkts_AP_dprintk("[mtkts_AP_write] cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\
					cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s\n",
				g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9);

		for(i=0; i<num_trip; i++)
		{
			trip_temp[i]=trip[i];
		}

		interval=time_msec / 1000;

		mtkts_AP_dprintk("[mtkts_AP_write] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,time_ms=%d\n",
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);

		mtkts_AP_dprintk("[mtkts_AP_write] mtkts_AP_register_thermal\n");

#if AUTO_ARBITRATION_REASONABLE_TEMP
		/*Thermal will issue "set parameter policy" than issue "register policy"*/
		Ap_temp = mtkts_AP_get_hw_temp();
		XTAL_temp = mtktsxtal_get_xtal_temp();
        CPU_Tj = mtktscpu_get_Tj_temp();
        printk("[ts_AP]Ap_temp=%d,XTAL_temp=%d,CPU_Tj=%d\n",Ap_temp,XTAL_temp,CPU_Tj);


        if(XTAL_temp > Ap_temp)
			AP_XTAL_diff = XTAL_temp - Ap_temp;
        else
            AP_XTAL_diff = Ap_temp - XTAL_temp;

        //check temp from Tj and Txal
        if(( Ap_temp < CPU_Tj) && (AP_XTAL_diff <= XTAL_BTS_TEMP_DIFF)){
            //printk("AP_XTAL_diff <= 10 degree\n");
			mtkts_AP_register_thermal();
		}
#else
		mtkts_AP_register_thermal();
#endif
		//AP_write_flag=1;
		return count;
	}
	else
	{
		mtkts_AP_dprintk("[mtkts_AP_write] bad argument\n");
	}

	return -EINVAL;
}

#ifdef INPUT_PARAM_FROM_USER_AP
void mtkts_AP_copy_table(AP_TEMPERATURE *des,AP_TEMPERATURE *src)
{
	int i=0;
    int j=0;

    j = (sizeof(AP_Temperature_Table)/sizeof(AP_TEMPERATURE));

    for(i=0;i<j;i++)
	{
		des[i] = src[i];
	}
}

void mtkts_AP_prepare_table(int table_num)
{
//	int i=0;
	switch(table_num)
    {
		case 1://AP_NTC_BL197
				mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table1);
			break;
		case 2://AP_NTC_TSM_1
                mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table2);
			break;
		case 3://AP_NTC_10_SEN_1
                mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table3);
			break;
		case 4://AP_NTC_10
                mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table4);
			break;
		case 5://AP_NTC_47
                mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table5);
			break;
		case 6://NTCG104EF104F
                mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table6);
			break;
        default://AP_NTC_10
	            mtkts_AP_copy_table(AP_Temperature_Table,AP_Temperature_Table4);
            break;
    }

#if 0
	for(i=0;i<(sizeof(AP_Temperature_Table)/sizeof(AP_TEMPERATURE));i++)
	{
		mtkts_AP_dprintk("AP_Temperature_Table[%d].APteryTemp =%d\n",i, AP_Temperature_Table[i].APteryTemp);
		mtkts_AP_dprintk("AP_Temperature_Table[%d].TemperatureR=%d\n",i, AP_Temperature_Table[i].TemperatureR);
	}
#endif
}

static int mtkts_AP_param_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "********** AP param**********\n");
    p += sprintf(p, "RAP_PULL_UP_R         = %d\n",g_RAP_pull_up_R);
    p += sprintf(p, "RAP_PULL_UP_VOLT      = %d\n",g_RAP_pull_up_voltage);
    p += sprintf(p, "TAP_OVER_CRITICAL_LOW = %d\n",g_TAP_over_critical_low);
    p += sprintf(p, "NTC_TABLE              = %d\n",g_RAP_ntc_table);
	p += sprintf(p, "********** AP param**********\n");

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}



static ssize_t mtkts_AP_param_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len=0;
	char desc[512];

    char pull_R[10],pull_V[10];
    char overcrilow[16];
    char NTC_TABLE[10];
    unsigned int valR,valV,over_cri_low,ntc_table;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';


	mtkts_AP_dprintk("[mtkts_AP_write]\n");


	if (sscanf(desc, "%s %d %s %d %s %d %s %d",pull_R, &valR, pull_V, &valV, overcrilow, &over_cri_low,NTC_TABLE,&ntc_table ) == 8)
	{

        if (!strcmp(pull_R, "PUP_R")) {
            g_RAP_pull_up_R = valR;
            mtkts_AP_dprintk("g_RAP_pull_up_R=%d\n",g_RAP_pull_up_R);
        }else{
			printk("[mtkts_AP_write] bad PUP_R argument\n");
            return -EINVAL;
        }

        if (!strcmp(pull_V, "PUP_VOLT")) {
            g_RAP_pull_up_voltage = valV;
            mtkts_AP_dprintk("g_Rat_pull_up_voltage=%d\n",g_RAP_pull_up_voltage);
        }else{
			printk("[mtkts_AP_write] bad PUP_VOLT argument\n");
            return -EINVAL;
        }

        if (!strcmp(overcrilow, "OVER_CRITICAL_L")) {
            g_TAP_over_critical_low = over_cri_low;
            mtkts_AP_dprintk("g_TAP_over_critical_low=%d\n",g_TAP_over_critical_low);
        }else{
			printk("[mtkts_AP_write] bad OVERCRIT_L argument\n");
            return -EINVAL;
        }

        if (!strcmp(NTC_TABLE, "NTC_TABLE")) {
            g_RAP_ntc_table = ntc_table;
            mtkts_AP_dprintk("g_RAP_ntc_table=%d\n",g_RAP_ntc_table);
        }else{
			printk("[mtkts_AP_write] bad NTC_TABLE argument\n");
            return -EINVAL;
        }

		mtkts_AP_prepare_table(g_RAP_ntc_table);

		return count;
	}
	else
	{
		printk("[mtkts_AP_write] bad argument\n");
	}


	return -EINVAL;
}
#endif
//int  mtkts_AP_register_cooler(void)
//{
	/* cooling devices */
	//cl_dev_sysrst = mtk_thermal_cooling_device_register("mtktsAPtery-sysrst", NULL,
	//	&mtkts_AP_cooling_sysrst_ops);
	//return 0;
//}

int mtkts_AP_register_thermal(void)
{
	mtkts_AP_dprintk("[mtkts_AP_register_thermal] \n");

	/* trips : trip 0~1 */
	thz_dev = mtk_thermal_zone_device_register("mtktsAP", num_trip, NULL,
		&mtkts_AP_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}

//void mtkts_AP_unregister_cooler(void)
//{
	//if (cl_dev_sysrst) {
	//	mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
	//	cl_dev_sysrst = NULL;
	//}
//}
void mtkts_AP_unregister_thermal(void)
{
	mtkts_AP_dprintk("[mtkts_AP_unregister_thermal] \n");

	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

static int __init mtkts_AP_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtkts_AP_dir = NULL;

	mtkts_AP_dprintk("[mtkts_AP_init] \n");

	//err = mtkts_AP_register_cooler();
	//if(err)
	//	return err;
#ifndef INPUT_PARAM_FROM_USER_AP
	err = mtkts_AP_register_thermal();
	if (err)
		goto err_unreg;
#endif
    // setup default table
    mtkts_AP_prepare_table(g_RAP_ntc_table);

	mtkts_AP_dir = proc_mkdir("mtktsAP", NULL);
	if (!mtkts_AP_dir)
	{
		mtkts_AP_dprintk("[mtkts_AP_init]: mkdir /proc/mtktsAP failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktsAP", S_IRUGO | S_IWUSR | S_IWGRP, mtkts_AP_dir);
		if (entry)
		{
			entry->read_proc = mtkts_AP_read;
			entry->write_proc = mtkts_AP_write;
			entry->gid = 1000;
		}
#ifdef INPUT_PARAM_FROM_USER_AP
		entry = create_proc_entry("mtkts_AP_param", S_IRUGO | S_IWUSR | S_IWGRP, mtkts_AP_dir);
		if (entry)
		{
			entry->read_proc = mtkts_AP_param_read;
			entry->write_proc = mtkts_AP_param_write;
			entry->gid = 1000;
		}
#endif
	}

	return 0;
#ifndef INPUT_PARAM_FROM_USER_AP
err_unreg:
#endif
	//mtkts_AP_unregister_cooler();
	return err;
}

static void __exit mtkts_AP_exit(void)
{
	mtkts_AP_dprintk("[mtkts_AP_exit] \n");
	mtkts_AP_unregister_thermal();
	//mtkts_AP_unregister_cooler();
}

#if AUTO_ARBITRATION_REASONABLE_TEMP
late_initcall(mtkts_AP_init);
module_exit(mtkts_AP_exit);
#else
module_init(mtkts_AP_init);
module_exit(mtkts_AP_exit);
#endif

