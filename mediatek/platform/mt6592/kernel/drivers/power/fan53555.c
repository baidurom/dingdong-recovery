#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <linux/hwmsen_helper.h>
#include <linux/xlog.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>

#include "fan53555.h"
#include <cust_pmic.h>

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define fan53555_SLAVE_ADDR_WRITE   0xC0
#define fan53555_SLAVE_ADDR_Read    0xC1

#ifdef fan53555_I2C_CHANNEL_CHANGED_0
    #define fan53555_BUSNUM 0
#else
    #ifdef fan53555_I2C_CHANNEL_CHANGED_2
        #define fan53555_BUSNUM 2
    #else
        #define fan53555_BUSNUM 1
    #endif
#endif

static struct i2c_client *new_client = NULL;
static const struct i2c_device_id fan53555_i2c_id[] = {{"fan53555",0},{}};   
static int fan53555_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_driver fan53555_driver = {
    .driver = {
        .name    = "fan53555",
    },
    .probe       = fan53555_driver_probe,
    .id_table    = fan53555_i2c_id,
};

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
kal_uint8 fan53555_reg[fan53555_REG_NUM] = {0};

static DEFINE_MUTEX(fan53555_i2c_access);

int g_fan53555_driver_ready=0;
int g_fan53555_hw_exist=0;
/**********************************************************
  *
  *   [I2C Function For Read/Write fan53555] 
  *
  *********************************************************/
int fan53555_read_byte(kal_uint8 cmd, kal_uint8 *returnData)
{
    char     cmd_buf[1]={0x00};
    char     readData = 0;
    int      ret=0;

    mutex_lock(&fan53555_i2c_access);
    
    //new_client->addr = ((new_client->addr) & I2C_MASK_FLAG) | I2C_WR_FLAG;    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG;

    cmd_buf[0] = cmd;
    ret = i2c_master_send(new_client, &cmd_buf[0], (1<<8 | 1));
    if (ret < 0) 
    {   
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_read_byte] ret=%d\n", ret);
        
        //new_client->addr = new_client->addr & I2C_MASK_FLAG;
        new_client->ext_flag=0;
        mutex_unlock(&fan53555_i2c_access);
        return 0;
    }
    
    readData = cmd_buf[0];
    *returnData = readData;

    // new_client->addr = new_client->addr & I2C_MASK_FLAG;
    new_client->ext_flag=0;
    
    mutex_unlock(&fan53555_i2c_access);    
    return 1;
}

int fan53555_write_byte(kal_uint8 cmd, kal_uint8 writeData)
{
    char    write_data[2] = {0};
    int     ret=0;
    
    mutex_lock(&fan53555_i2c_access);
    
    write_data[0] = cmd;
    write_data[1] = writeData;
    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_DIRECTION_FLAG;
    
    ret = i2c_master_send(new_client, write_data, 2);
    if (ret < 0) 
    {   
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_write_byte] ret=%d\n", ret);
        
        new_client->ext_flag=0;
        mutex_unlock(&fan53555_i2c_access);        
        return 0;
    }
    
    new_client->ext_flag=0;
    mutex_unlock(&fan53555_i2c_access);
    return 1;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 fan53555_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 fan53555_reg = 0;
    int ret = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","--------------------------------------------------\n");

    ret = fan53555_read_byte(RegNum, &fan53555_reg);

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_read_interface] Reg[%x]=0x%x\n", RegNum, fan53555_reg);
    
    fan53555_reg &= (MASK << SHIFT);
    *val = (fan53555_reg >> SHIFT);
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_read_interface] val=0x%x\n", *val);
    
    return ret;
}

kal_uint32 fan53555_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 fan53555_reg = 0;
    int ret = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","--------------------------------------------------\n");

    ret = fan53555_read_byte(RegNum, &fan53555_reg);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_config_interface] Reg[%x]=0x%x\n", RegNum, fan53555_reg);
    
    fan53555_reg &= ~(MASK << SHIFT);
    fan53555_reg |= (val << SHIFT);

    ret = fan53555_write_byte(RegNum, fan53555_reg);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_config_interface] write Reg[%x]=0x%x\n", RegNum, fan53555_reg);

    // Check
    //fan53555_read_byte(RegNum, &fan53555_reg);
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_config_interface] Check Reg[%x]=0x%x\n", RegNum, fan53555_reg);

    return ret;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
void fan53555_dump_register(void)
{
    int i=0;
    printk("[fan53555] ");
    for (i=0;i<fan53555_REG_NUM;i++)
    {
        fan53555_read_byte(i, &fan53555_reg[i]);
        printk("[0x%x]=0x%x ", i, fan53555_reg[i]);        
    }
    printk("\n");
}

int get_fan53555_i2c_ch_num(void)
{
    return fan53555_BUSNUM;
}

extern void ext_buck_vproc_vsel(int val);
extern unsigned int g_vproc_vsel_gpio_number;

void fan53555_hw_init(void)
{    
    kal_uint32 ret=0;
   
    ret = fan53555_config_interface(0x01,0xB2,0xFF,0); // VSEL=high, 1.1V
    
    if(g_vproc_vsel_gpio_number!=0)
    {
        ext_buck_vproc_vsel(1); 
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[fan53555_hw_init] ext_buck_vproc_vsel(1)\n");
    }
    
    ret = fan53555_config_interface(0x00,0x8A,0xFF,0); // VSEL=low, 0.7V
    ret = fan53555_config_interface(0x02,0xA0,0xFF,0);
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_hw_init] Done\n");    
}

void fan53555_hw_component_detect(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;
    ret=fan53555_read_interface(0x03,&val,0x7,5);
    
    // check vender ID
    if(val==0x4)
    {
        g_fan53555_hw_exist=1;
    }
    else
    {
        g_fan53555_hw_exist=0;
    }
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_hw_component_detect] exist=%d, Reg[0x03][7:5]=0x%x\n",
        g_fan53555_hw_exist, val);    	
}

int is_fan53555_sw_ready(void)
{
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","g_fan53555_driver_ready=%d\n", g_fan53555_driver_ready);
    
    return g_fan53555_driver_ready;
}

int is_fan53555_exist(void)
{
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","g_fan53555_hw_exist=%d\n", g_fan53555_hw_exist);
    
    return g_fan53555_hw_exist;
}

int fan53555_vosel(unsigned long val)
{
    int ret=1;
    unsigned long reg_val=0;

    //reg_val = ( (val) - 60000 ) / 1000; //600mV~1230mV, step=10mV
    reg_val = ((((val*10)-600000)/1000)+9)/10;

    if(reg_val > 63)
        reg_val = 63;

    reg_val = reg_val | 0x80;
    ret=fan53555_write_byte(0x01, reg_val);

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_vosel] val=%d, reg_val=%d\n", val, reg_val);

    return ret;
}

static int fan53555_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{             
    int err=0; 

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_driver_probe] \n");

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }    
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client = client;    

    //---------------------
    fan53555_hw_component_detect();
    if(g_fan53555_hw_exist==1)
    {
        fan53555_hw_init();
        fan53555_dump_register();
    }
    g_fan53555_driver_ready=1;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_driver_probe] g_fan53555_hw_exist=%d, g_fan53555_driver_ready=%d\n", 
        g_fan53555_hw_exist, g_fan53555_driver_ready);

    if(g_fan53555_hw_exist==0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_driver_probe] return err\n");
        return err;
    }
    
    return 0;                                                                                       

exit:
    return err;

}

/**********************************************************
  *
  *   [platform_driver API] 
  *
  *********************************************************/
#ifdef FAN53555_AUTO_DETECT_DISABLE
//
#else
kal_uint8 g_reg_value_fan53555=0;
static ssize_t show_fan53555_access(struct device *dev,struct device_attribute *attr, char *buf)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[show_fan53555_access] 0x%x\n", g_reg_value_fan53555);
    return sprintf(buf, "%u\n", g_reg_value_fan53555);
}
static ssize_t store_fan53555_access(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int ret=0;
    char *pvalue = NULL;
    unsigned int reg_value = 0;
    unsigned int reg_address = 0;
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_fan53555_access] \n");
    
    if(buf != NULL && size != 0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_fan53555_access] buf is %s and size is %d \n",buf,size);
        reg_address = simple_strtoul(buf,&pvalue,16);
        
        if(size > 3)
        {        
            reg_value = simple_strtoul((pvalue+1),NULL,16);        
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_fan53555_access] write fan53555 reg 0x%x with value 0x%x !\n",reg_address,reg_value);
            ret=fan53555_config_interface(reg_address, reg_value, 0xFF, 0x0);
        }
        else
        {    
            ret=fan53555_read_interface(reg_address, &g_reg_value_fan53555, 0xFF, 0x0);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_fan53555_access] read fan53555 reg 0x%x with value 0x%x !\n",reg_address,g_reg_value_fan53555);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_fan53555_access] Please use \"cat fan53555_access\" to get value\r\n");
        }        
    }    
    return size;
}
static DEVICE_ATTR(fan53555_access, 0664, show_fan53555_access, store_fan53555_access); //664

static int fan53555_user_space_probe(struct platform_device *dev)    
{    
    int ret_device_file = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","******** fan53555_user_space_probe!! ********\n" );
    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_fan53555_access);
    
    return 0;
}

struct platform_device fan53555_user_space_device = {
    .name   = "fan53555-user",
    .id     = -1,
};

static struct platform_driver fan53555_user_space_driver = {
    .probe      = fan53555_user_space_probe,
    .driver     = {
        .name = "fan53555-user",
    },
};

static struct i2c_board_info __initdata i2c_fan53555 = { I2C_BOARD_INFO("fan53555", (fan53555_SLAVE_ADDR_WRITE>>1))};
#endif

extern unsigned int g_vproc_vsel_gpio_number;;

static int __init fan53555_init(void)
{    
#ifdef FAN53555_AUTO_DETECT_DISABLE

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_init] FAN53555_AUTO_DETECT_DISABLE\n");
    g_fan53555_hw_exist=0;
    g_fan53555_driver_ready=1;    

#else

    int ret=0;

    if(g_vproc_vsel_gpio_number != 0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_init] init start.\n");
        
        i2c_register_board_info(fan53555_BUSNUM, &i2c_fan53555, 1);

        if(i2c_add_driver(&fan53555_driver)!=0)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_init] failed to register fan53555 i2c driver.\n");
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_init] Success to register fan53555 i2c driver.\n");
        }

        // fan53555 user space access interface
        ret = platform_device_register(&fan53555_user_space_device);
        if (ret) {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","****[fan53555_init] Unable to device register(%d)\n", ret);
            return ret;
        }    
        ret = platform_driver_register(&fan53555_user_space_driver);
        if (ret) {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","****[fan53555_init] Unable to register driver (%d)\n", ret);
            return ret;
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[fan53555_init] DCT no define EXT BUCK\n");
        g_fan53555_hw_exist=0;
        g_fan53555_driver_ready=1;
    }

#endif    
    
    return 0;        
}

static void __exit fan53555_exit(void)
{
    i2c_del_driver(&fan53555_driver);
}

module_init(fan53555_init);
module_exit(fan53555_exit);
   
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C fan53555 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");
