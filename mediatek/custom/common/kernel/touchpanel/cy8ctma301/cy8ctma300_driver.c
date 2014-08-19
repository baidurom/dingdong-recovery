#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>

#include "tpd_custom_cy8ctma300.h"
#include "tpd.h"
#include <cust_eint.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif

extern struct tpd_device *tpd;
extern int tpd_show_version;
extern int tpd_debuglog;
extern int tpd_register_flag;
extern int tpd_load_status;

static int tpd_flag = 0;
static int tpd_halt=0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"cy8ctma300",0},{}};
static struct i2c_board_info __initdata cy8ctma300_i2c_tpd={ I2C_BOARD_INFO("cy8ctma300", (0x55))};


//static const struct i2c_device_id tpd_i2c_id[] = {{"mtk-tpd",0},{}};
//static unsigned short force[] = {0, 0xce, I2C_CLIENT_END,I2C_CLIENT_END};
//static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces,};
struct i2c_driver tpd_i2c_driver = {                       
    .probe = tpd_i2c_probe,                                   
    .remove = tpd_i2c_remove,                           
 //   .detect = tpd_i2c_detect,
    .driver.name = "cy8ctma300",
    .id_table = tpd_i2c_id,                             
  //  .address_data = &addr_data,
}; 
/*
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-tpd");
    return 0;
}
*/
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {             
    int err = 0;
    char buffer[8];
    int status=0;
    i2c_client = client;    
    
    #ifdef TPD_NO_GPIO
    u16 temp;
    temp = *(volatile u16 *) TPD_RESET_PIN_ADDR;
    temp = temp | 0x40;
    *(volatile u16 *) TPD_RESET_PIN_ADDR = temp;
    #endif
    
    #ifndef TPD_NO_GPIO 
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(10);  
#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
    msleep(5);
#endif
    
 //   mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
 //   mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
 //   mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
 //   msleep(10);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(1);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
    #endif 

    msleep(10);
    buffer[0] =  0x00;
    buffer[1] =  0x03;
    i2c_master_send(i2c_client,buffer,2);
    msleep(50);
    status = i2c_master_recv(i2c_client, buffer, 8);
    
    if(status<0) {
        TPD_DMESG("[mtk-tpd], cy8ctma300 tpd_i2c_probe failed!!\n");
        return status;
    }
    TPD_DMESG("[mtk-tpd], cy8ctma300 tpd_i2c_probe success!!\n");		
    if(buffer[2] == 0x31)
    tpd_load_status = 1;
    buffer[0] =  0x00;
    buffer[1] =  0x05;
    i2c_master_send(i2c_client,buffer,2);
    //status = i2c_master_recv(i2c_client, buffer, 3);
    msleep(50);
    buffer[0] =  0x00;
    buffer[1] =  0x21;
    buffer[2] =  0x07;
    buffer[3] =  0x01;
    buffer[4] =  0x00;
    i2c_master_send(i2c_client,buffer,5);
    msleep(50);


    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread)) { 
        err = PTR_ERR(thread);
        TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", err);
    }
    
    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    
    return 0;
}

void tpd_eint_interrupt_handler(void) { 
	if(tpd_debuglog==1) TPD_DMESG("[mtk-tpd], %s\n", __FUNCTION__);
    TPD_DEBUG_PRINT_INT; tpd_flag=1; wake_up_interruptible(&waiter);
} 
static int tpd_i2c_remove(struct i2c_client *client) {return 0;}

void tpd_down(int raw_x, int raw_y, int x, int y, int p) {
	if(tpd && tpd->dev && tpd_register_flag==1) {
    input_report_abs(tpd->dev, ABS_PRESSURE, p/PRESSURE_FACTOR);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, p/PRESSURE_FACTOR);
    input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, p/PRESSURE_FACTOR);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_mt_sync(tpd->dev);
    TPD_DEBUG("D[%4d %4d %4d]\n", x, y, p);
    TPD_EM_PRINT(raw_x, raw_y, x, y, p, 1);
  }  
}

void tpd_up(int raw_x, int raw_y, int x, int y, int p) {
	if(tpd && tpd->dev && tpd_register_flag==1) {
    input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_mt_sync(tpd->dev);
    TPD_DEBUG("U[%4d %4d %4d]\n", x, y, 0);
    TPD_EM_PRINT(raw_x, raw_y, x, y, p, 0);
  }  
}

volatile unsigned long g_temptimerdiff;
int x_min=0, y_min=0, x_max=0, y_max=0, counter_pointer=0;
static int touch_event_handler(void *unused) {
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD }; 
    int i = 0,x1=0, y1=0, finger_num = 0;
    char buffer[32];//[16];
	u32 temp;
    
    sched_setscheduler(current, SCHED_RR, &param); 
    g_temptimerdiff=get_jiffies_64();//jiffies;
    do {
		if(tpd_debuglog==1) {
			TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__); 
		}	    	
        set_current_state(TASK_INTERRUPTIBLE);
	if(tpd_debuglog==1)
		TPD_DMESG("[mtk-tpd], %s, tpd_halt=%d\n", __FUNCTION__, tpd_halt);
        while (tpd_halt) {tpd_flag = 0; msleep(20);}
        wait_event_interruptible(waiter, tpd_flag != 0);
        //tpd_flag = 0;
        TPD_DEBUG_SET_TIME;
        set_current_state(TASK_RUNNING); 
        i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buffer[0]));
        i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(buffer[8]));
        i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(buffer[16]));
        i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(buffer[24]));		
        if((buffer[1]&0x01) != (buffer[30]&0x01))
        {        
            i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buffer[0]));
            i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(buffer[8]));
            i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(buffer[16]));
            i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(buffer[24]));		
        }
        if((buffer[1]&0xf0) != 0x10)
        {
            buffer[0] =  0x00;
            buffer[1] =  0x03;
            i2c_master_send(i2c_client,buffer,2);
            msleep(50);
            buffer[0] =  0x00;
            buffer[1] =  0x05;
            i2c_master_send(i2c_client,buffer,2);
            //status = i2c_master_recv(i2c_client, buffer, 3);
            msleep(50);
            buffer[0] =  0x00;
            buffer[1] =  0x21;
            buffer[2] =  0x07;
            buffer[3] =  0x01;
            buffer[4] =  0x00;
            i2c_master_send(i2c_client,buffer,5);
            msleep(50);
            continue; 
        }
	if(tpd_debuglog==1)
	{
        TPD_DMESG("[mtk-tpd]HST_MODE  : %x\n", buffer[0]); 
        TPD_DMESG("[mtk-tpd]TT_MODE   : %x\n", buffer[1]); 
        TPD_DMESG("[mtk-tpd]TT_STAT   : %x\n", buffer[2]);
       // TPD_DEBUG("[mtk-tpd]TOUCH_ID  : %x\n", buffer[8]);
		TPD_DMESG("[mtk-tpd]TOUCH_12ID  : %x\n", buffer[8]);
		TPD_DMESG("[mtk-tpd]TOUCH_34ID  : %x\n", buffer[21]);
	}	
                finger_num = 0;
        for(i = 0; i < 7;i++)
        { 
            x1 = buffer[4*i + 2] | ((buffer[4*i+4]&0x0f) << 8) ;
            y1 = buffer[4*i + 3] | ((buffer[4*i+4]&0xf0) << 4) ;
            if(x1 != 0xfff){
                tpd_down(x1, y1, x1, y1, 100);
                finger_num++;
                //printk("-----x:%d,y:%d-------down\n",x1,y1);            
            }
        }
           //printk("-----finger_num = %d-------\n",finger_num);            
        if(finger_num > 0)
            input_sync(tpd->dev);
        else
        {
           //printk("-----x:%d,y:%d-------up\n",x1,y1);            
            tpd_up(x1, y1, x1, y1, 0);
            //input_mt_sync(tpd->dev);
            input_sync(tpd->dev);
        }
       tpd_flag = 0;
   
    } while (!kthread_should_stop()); 
    return 0;
}

int tpd_local_init(void) 
{
	if(tpd_debuglog==1) {
		TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__); 
	}
     if(i2c_add_driver(&tpd_i2c_driver)!=0) {
      TPD_DMESG("unable to add i2c driver.\n");
      return -1;
    }
    if(tpd_load_status == 0)
    {
    	TPD_DMESG("add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }
#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		tpd_type_cap = 1;
    return 0;
}

/* Function to manage low power suspend */
void tpd_suspend(struct early_suspend *h)
{
	if(tpd_debuglog==1) {
		TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__); 
	}
  	tpd_halt = 1;	
	while(1){
		if(tpd_flag == 1) msleep(1000);
		else break;	
	}
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
    mdelay(1);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);    
}

/* Function to manage power-on resume */
void tpd_resume(struct early_suspend *h) 
{
    char buffer[8];
	if(tpd_debuglog==1) {
		TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__); 
	}
   // msleep(100);
#ifdef TPD_POWER_SOURCE_CUSTOM
   hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
   hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
    msleep(1);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(1);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);    
    msleep(100);
    buffer[0] =  0x00;
    buffer[1] =  0x03;
    i2c_master_send(i2c_client,buffer,2);
    msleep(50);
    buffer[0] =  0x00;
    buffer[1] =  0x05;
    i2c_master_send(i2c_client,buffer,2);
    //status = i2c_master_recv(i2c_client, buffer, 3);
    msleep(50);
    buffer[0] =  0x00;
    buffer[1] =  0x21;
    buffer[2] =  0x07;
    buffer[3] =  0x01;
    buffer[4] =  0x00;
    i2c_master_send(i2c_client,buffer,5);
    msleep(50);

    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
    tpd_halt = 0;
}

static struct tpd_driver_t tpd_device_driver = {
		.tpd_device_name = "cy8ctma300",
		.tpd_local_init = tpd_local_init,
		.suspend = tpd_suspend,
		.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		.tpd_have_button = 1,
#else
		.tpd_have_button = 0,
#endif		
};
/* called when loaded into kernel */
static int __init tpd_driver_init(void) {
    printk("MediaTek cy8ctma300 touch panel driver init\n");
    i2c_register_board_info(0, &cy8ctma300_i2c_tpd, 1);
		if(tpd_driver_add(&tpd_device_driver) < 0)
			TPD_DMESG("add generic driver failed\n");
    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) {
    TPD_DMESG("MediaTek cy8ctma300 touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

