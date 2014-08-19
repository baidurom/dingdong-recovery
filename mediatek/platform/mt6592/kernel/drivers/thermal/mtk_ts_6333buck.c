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

#include <mach/system.h>
#include "mach/mtk_thermal_monitor.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"


#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt6333.h>
static unsigned int interval = 0; /* seconds, 0 : no auto polling */
static int trip_temp[10] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};

#if 0
static unsigned int cl_dev_sysrst_state = 0;
#endif
static struct thermal_zone_device *thz_dev;

#if 0
static struct thermal_cooling_device *cl_dev_sysrst;
#endif
static int mtktsbuck_debug_log = 0;
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

#define mtktsbuck_TEMP_CRIT 150000 /* 150.000 degree Celsius */

#define PMIC6333_INT_TEMP_CUNT 0xF
static kal_uint32 tempsetting_count=0;
typedef struct{
    INT32 regsetting;
    INT32 Temperature;
}pmic6333_TEMPERATURE;


//from 6333 spec
static pmic6333_TEMPERATURE pmic6333_temp_map[] = {
	{0,140},
	{1,135},
	{2,130},/*(default)*/
	{3,125},
	{4,120},
	{5,115},
	{6,110},
	{7,105},
	{8,100},
	{9,95},
	{10,90},
	{11,85},
	{12,80},
	{13,75},
	{14,70},
	{15,65}
};



#define mtktsbuck_dprintk(fmt, args...)   \
do {									\
	if (mtktsbuck_debug_log) {				\
		xlog_printk(ANDROID_LOG_INFO, "Power/buck_Thermal", fmt, ##args); \
	}								   \
} while(0)

extern kal_uint32 get_thermal_mt6333_buck_int_status(void);
extern void set_thermal_mt6333_buck_int_status(kal_uint32 val);

static kal_uint32 mt6333_buck_int=0;

static int mtktsbuck6333_thermal_zone_handler(void);





static int mtktsbuck_get_temp(struct thermal_zone_device *thermal,
				   unsigned long *t)
{
	mtktsbuck_dprintk("[mtktsbuck_get_temp]\n");


	*t =  mtktsbuck6333_thermal_zone_handler();

	return 0;
}

static int mtktsbuck_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
	}
	else
	{
		return 0;
	}

	if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
		mtktsbuck_dprintk("[mtktsbuck_bind] error binding cooling dev\n");
		return -EINVAL;
	} else {
		mtktsbuck_dprintk("[mtktsbuck_bind] binding OK, %d\n", table_val);
	}

	return 0;
}

static int mtktsbuck_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
	}
	else
		return 0;

	if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
		mtktsbuck_dprintk("[mtktsbuck_unbind] error unbinding cooling dev\n");
		return -EINVAL;
	} else {
		mtktsbuck_dprintk("[mtktsbuck_unbind] unbinding OK\n");
	}

	return 0;
}

static int mtktsbuck_get_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
				 : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktsbuck_set_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktsbuck_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktsbuck_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip];
	return 0;
}

static int mtktsbuck_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = mtktsbuck_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktsbuck_dev_ops = {
	.bind = mtktsbuck_bind,
	.unbind = mtktsbuck_unbind,
	.get_temp = mtktsbuck_get_temp,
	.get_mode = mtktsbuck_get_mode,
	.set_mode = mtktsbuck_set_mode,
	.get_trip_type = mtktsbuck_get_trip_type,
	.get_trip_temp = mtktsbuck_get_trip_temp,
	.get_crit_temp = mtktsbuck_get_crit_temp,
};

#if 0
static int tsbuck_sysrst_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	*state = 1;
	return 0;
}
static int tsbuck_sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	*state = cl_dev_sysrst_state;
	return 0;
}
static int tsbuck_sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	cl_dev_sysrst_state = state;
	if(cl_dev_sysrst_state == 1)
	{
		printk("Power/buck_Thermal: reset, reset, reset!!!");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		printk("*****************************************");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

		BUG();
		//arch_reset(0,NULL);
	}
	return 0;
}

static struct thermal_cooling_device_ops mtktsbuck_cooling_sysrst_ops = {
	.get_max_state = tsbuck_sysrst_get_max_state,
	.get_cur_state = tsbuck_sysrst_get_cur_state,
	.set_cur_state = tsbuck_sysrst_set_cur_state,
};
#endif

static int mtktsbuck_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

		p += sprintf(p, "[ mtktsbuck_read] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\n\
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

int mtktsbuck_register_thermal(void);
void mtktsbuck_unregister_thermal(void);

static ssize_t mtktsbuck_write(struct file *file, const char *buffer, unsigned long count, void *data)
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

	if (sscanf(desc, "%d %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d",
							&num_trip, &trip[0],&t_type[0],bind0, &trip[1],&t_type[1],bind1,
												 &trip[2],&t_type[2],bind2, &trip[3],&t_type[3],bind3,
												 &trip[4],&t_type[4],bind4, &trip[5],&t_type[5],bind5,
											   &trip[6],&t_type[6],bind6, &trip[7],&t_type[7],bind7,
												 &trip[8],&t_type[8],bind8, &trip[9],&t_type[9],bind9,
												 &time_msec) == 32)
	{
		mtktsbuck_dprintk("[mtktsbuck_write] mtktsbuck_unregister_thermal\n");
		mtktsbuck_unregister_thermal();

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

		mtktsbuck_dprintk("[mtktsbuck_write] g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n",
													g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
													g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
		mtktsbuck_dprintk("[mtktsbuck_write] cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s\n",
													g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9);

		for(i=0; i<num_trip; i++)
		{
			trip_temp[i]=trip[i];
		}

		interval=time_msec / 1000;

		mtktsbuck_dprintk("[mtktsbuck_write] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,time_ms=%d\n",
						trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
						trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);

		mtktsbuck_dprintk("[mtktsbuck_write] mtktsbuck_register_thermal\n");
		mtktsbuck_register_thermal();

		return count;
	}
	else
	{
		mtktsbuck_dprintk("[mtktsbuck_write] bad argument\n");
	}

	return -EINVAL;
}

#if 0
int mtktsbuck_register_cooler(void)
{
	cl_dev_sysrst = mtk_thermal_cooling_device_register("mtktsbuck-sysrst", NULL,
					   &mtktsbuck_cooling_sysrst_ops);
   	return 0;
}
#endif

int mtktsbuck_register_thermal(void)
{
	mtktsbuck_dprintk("[mtktsbuck_register_thermal] \n");

	/* trips : trip 0~2 */
	thz_dev = mtk_thermal_zone_device_register("mtktsbuck", num_trip, NULL,
					  &mtktsbuck_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}

#if 0
void mtktsbuck_unregister_cooler(void)
{
	if (cl_dev_sysrst) {
		mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
		cl_dev_sysrst = NULL;
	}
}
#endif

void mtktsbuck_unregister_thermal(void)
{
	mtktsbuck_dprintk("[mtktsbuck_unregister_thermal] \n");

	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}


static int mtktsbuck6333_thermal_zone_handler(void )
{
	int temp=0;

    mtktsbuck_dprintk( "[mtktsbuck6333_thermal_zone_handler] ,tempsetting_count=0x%x\n",tempsetting_count);

	mt6333_buck_int = get_thermal_mt6333_buck_int_status();

	if(mt6333_buck_int==1){ //receive thermal buck INT
		tempsetting_count--;
        if(tempsetting_count <=0 )//140 degree
		{
			tempsetting_count = 0;
            printk("6333 temp is over 140 degree\n");
		}

		set_thermal_mt6333_buck_int_status(0);
		//increase temperature
        mt6333_set_rg_strup_ther_rg_th((pmic6333_temp_map[tempsetting_count].regsetting));

        mtktsbuck_dprintk("increase change INT threshold to tempsetting_count=%d\n",tempsetting_count);
	}
    else{
        tempsetting_count++;
        if(tempsetting_count >= PMIC6333_INT_TEMP_CUNT)//65 degree
		{
			tempsetting_count = PMIC6333_INT_TEMP_CUNT;//65 degree
            mtktsbuck_dprintk("6333 temp is below 65 degree\n");
		}
        //decrease temperature
        mt6333_set_rg_strup_ther_rg_th((pmic6333_temp_map[tempsetting_count].regsetting));
        mtktsbuck_dprintk("decrease change INT threshold to tempsetting_count=%d\n",tempsetting_count);
    }

    mtktsbuck_dprintk("decrease pmic6333_temp_map[%d].regsetting=0x%x\n",tempsetting_count,pmic6333_temp_map[tempsetting_count].regsetting);
    mtktsbuck_dprintk("decrease pmic6333_temp_map[%d].Temperature=%d\n",tempsetting_count,pmic6333_temp_map[tempsetting_count].Temperature);

    temp = pmic6333_temp_map[tempsetting_count].Temperature;

    if(temp >= 70) //printing high temperature
        printk("[Power/buck_Thermal] Buck 6333 T=%d\n",temp);


    mtktsbuck_dprintk("mt6333_buck_int=%d\n",mt6333_buck_int);

    return (temp*1000);

}

static void mtktspmic6333_thermal_zone_init(void)
{

	mtktsbuck_dprintk("[mtktspmic6333_thermal_zone_init] \n");

	mt6333_set_rg_strup_ther_rg_th(PMIC6333_INT_TEMP_CUNT);//65 degree

	tempsetting_count = PMIC6333_INT_TEMP_CUNT;
}



static int __init mtktsbuck_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktsbuck_dir = NULL;

	mtktsbuck_dprintk("[mtktsbuck_init] \n");


	mtktspmic6333_thermal_zone_init();

#if 0
	err = mtktsbuck_register_cooler();
	if(err)
		return err;
#endif
	err = mtktsbuck_register_thermal();
	if (err)
		goto err_unreg;

	mtktsbuck_dir = proc_mkdir("mtktsbuck", NULL);
	if (!mtktsbuck_dir)
	{
		mtktsbuck_dprintk("[mtktsbuck_init]: mkdir /proc/mtktsbuck failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktsbuck", S_IRUGO | S_IWUSR | S_IWGRP, mtktsbuck_dir);
		if (entry)
		{
			entry->read_proc = mtktsbuck_read;
			entry->write_proc = mtktsbuck_write;
			entry->gid = 1000;
		}
	}

	return 0;

err_unreg:
#if 0
		mtktsbuck_unregister_cooler();
#endif
		return err;
}

static void __exit mtktsbuck_exit(void)
{
	mtktsbuck_dprintk("[mtktsbuck_exit] \n");
	mtktsbuck_unregister_thermal();
#if 0
	mtktsbuck_unregister_cooler();
#endif
}

late_initcall(mtktsbuck_init);
module_exit(mtktsbuck_exit);


