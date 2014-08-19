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

#include "ncp6335.h"
#include <cust_pmic.h>

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define ncp6335_SLAVE_ADDR_WRITE   0x38
#define ncp6335_SLAVE_ADDR_Read    0x39

#ifdef NCP6335_I2C_CHANNEL_CHANGED_0
    #define ncp6335_BUSNUM 0
#else
    #ifdef NCP6335_I2C_CHANNEL_CHANGED_2
        #define ncp6335_BUSNUM 2
    #else
        #define ncp6335_BUSNUM 1
    #endif
#endif

static struct i2c_client *new_client = NULL;
static const struct i2c_device_id ncp6335_i2c_id[] = {{"ncp6335",0},{}};   
static int ncp6335_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_driver ncp6335_driver = {
    .driver = {
        .name    = "ncp6335",
    },
    .probe       = ncp6335_driver_probe,
    .id_table    = ncp6335_i2c_id,
};

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
kal_uint8 ncp6335_reg[ncp6335_REG_NUM] = {0};

static DEFINE_MUTEX(ncp6335_i2c_access);

int g_ncp6335_driver_ready=0;
int g_ncp6335_hw_exist=0;
/**********************************************************
  *
  *   [I2C Function For Read/Write ncp6335] 
  *
  *********************************************************/
int ncp6335_read_byte(kal_uint8 cmd, kal_uint8 *returnData)
{
    char     cmd_buf[1]={0x00};
    char     readData = 0;
    int      ret=0;

    mutex_lock(&ncp6335_i2c_access);
    
    //new_client->addr = ((new_client->addr) & I2C_MASK_FLAG) | I2C_WR_FLAG;    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG;

    cmd_buf[0] = cmd;
    ret = i2c_master_send(new_client, &cmd_buf[0], (1<<8 | 1));
    if (ret < 0) 
    {   
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_read_byte] ret=%d\n", ret);
        
        //new_client->addr = new_client->addr & I2C_MASK_FLAG;
        new_client->ext_flag=0;
        mutex_unlock(&ncp6335_i2c_access);
        return 0;
    }
    
    readData = cmd_buf[0];
    *returnData = readData;

    // new_client->addr = new_client->addr & I2C_MASK_FLAG;
    new_client->ext_flag=0;
    
    mutex_unlock(&ncp6335_i2c_access);    
    return 1;
}

int ncp6335_write_byte(kal_uint8 cmd, kal_uint8 writeData)
{
    char    write_data[2] = {0};
    int     ret=0;
    
    mutex_lock(&ncp6335_i2c_access);
    
    write_data[0] = cmd;
    write_data[1] = writeData;
    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_DIRECTION_FLAG;
    
    ret = i2c_master_send(new_client, write_data, 2);
    if (ret < 0) 
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_write_byte] ret=%d\n", ret);
        
        new_client->ext_flag=0;
        mutex_unlock(&ncp6335_i2c_access);
        return 0;
    }
    
    new_client->ext_flag=0;
    mutex_unlock(&ncp6335_i2c_access);
    return 1;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 ncp6335_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 ncp6335_reg = 0;
    int ret = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","--------------------------------------------------\n");

    ret = ncp6335_read_byte(RegNum, &ncp6335_reg);

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_read_interface] Reg[%x]=0x%x\n", RegNum, ncp6335_reg);
    
    ncp6335_reg &= (MASK << SHIFT);
    *val = (ncp6335_reg >> SHIFT);
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_read_interface] val=0x%x\n", *val);
    
    return ret;
}

kal_uint32 ncp6335_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 ncp6335_reg = 0;
    int ret = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","--------------------------------------------------\n");

    ret = ncp6335_read_byte(RegNum, &ncp6335_reg);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_config_interface] Reg[%x]=0x%x\n", RegNum, ncp6335_reg);
    
    ncp6335_reg &= ~(MASK << SHIFT);
    ncp6335_reg |= (val << SHIFT);

    ret = ncp6335_write_byte(RegNum, ncp6335_reg);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_config_interface] write Reg[%x]=0x%x\n", RegNum, ncp6335_reg);

    // Check
    //ncp6335_read_byte(RegNum, &ncp6335_reg);
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_config_interface] Check Reg[%x]=0x%x\n", RegNum, ncp6335_reg);

    return ret;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
void ncp6335_dump_register(void)
{
    int i=0;
    printk("[ncp6335] ");
    for (i=0;i<ncp6335_REG_NUM;i++)
    {
        ncp6335_read_byte(i, &ncp6335_reg[i]);
        printk("[0x%x]=0x%x ", i, ncp6335_reg[i]);        
    }
    printk("\n");
}

int get_ncp6335_i2c_ch_num(void)
{
    return ncp6335_BUSNUM;
}

extern void ext_buck_vproc_vsel(int val);
extern unsigned int g_vproc_vsel_gpio_number;

void ncp6335_hw_init(void)
{    
    kal_uint32 ret=0;
   
    ret = ncp6335_config_interface(0x10,0xD0,0xFF,0); // VSEL=high, 1.1V

    if(g_vproc_vsel_gpio_number!=0)
    {
        ext_buck_vproc_vsel(1); 
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[ncp6335_hw_init] ext_buck_vproc_vsel(1)\n");
    }
    
    ret = ncp6335_config_interface(0x11,0x90,0xFF,0); // VSEL=low, 0.7V
    ret = ncp6335_config_interface(0x14,0x01,0xFF,0);
    ret = ncp6335_config_interface(0x16,0xE3,0xFF,0);
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_hw_init] Done\n");       
}

void ncp6335_hw_component_detect(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;
    ret=ncp6335_read_interface(0x03,&val,0xFF,0);
    
    // check PID
    if(val==0x10)
    {
        g_ncp6335_hw_exist=1;        
    }
    else
    {
        g_ncp6335_hw_exist=0;
    }
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_hw_component_detect] exist=%d, Reg[0x03]=0x%x\n",
        g_ncp6335_hw_exist, val);
}

int is_ncp6335_sw_ready(void)
{
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","g_ncp6335_driver_ready=%d\n", g_ncp6335_driver_ready);
    
    return g_ncp6335_driver_ready;
}

int is_ncp6335_exist(void)
{
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","g_ncp6335_hw_exist=%d\n", g_ncp6335_hw_exist);
    
    return g_ncp6335_hw_exist;
}

int ncp6335_vosel(unsigned long val)
{
    int ret=1;
    unsigned long reg_val=0;

    //reg_val = ( (val) - 60000 ) / 625;
    reg_val = ((((val*10)-600000)/625)+9)/10;

    if(reg_val > 127)
        reg_val = 127;
    
    reg_val = reg_val | 0x80;
    ret=ncp6335_write_byte(0x10, reg_val);

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_vosel] val=%d, reg_val=%d\n", val, reg_val);

    return ret;
}

static int ncp6335_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{             
    int err=0; 

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_driver_probe] \n");

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }    
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client = client;    
    
    //---------------------        
    ncp6335_hw_component_detect();        
    if(g_ncp6335_hw_exist==1)
    {
        ncp6335_hw_init();
        ncp6335_dump_register();
    }
    g_ncp6335_driver_ready=1;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_driver_probe] g_ncp6335_hw_exist=%d, g_ncp6335_driver_ready=%d\n", 
        g_ncp6335_hw_exist, g_ncp6335_driver_ready);

    if(g_ncp6335_hw_exist==0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_driver_probe] return err\n");
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
#ifdef NCP6335_AUTO_DETECT_DISABLE
    //
#else
kal_uint8 g_reg_value_ncp6335=0;
static ssize_t show_ncp6335_access(struct device *dev,struct device_attribute *attr, char *buf)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[show_ncp6335_access] 0x%x\n", g_reg_value_ncp6335);
    return sprintf(buf, "%u\n", g_reg_value_ncp6335);
}
static ssize_t store_ncp6335_access(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int ret=0;
    char *pvalue = NULL;
    unsigned int reg_value = 0;
    unsigned int reg_address = 0;
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_ncp6335_access] \n");
    
    if(buf != NULL && size != 0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_ncp6335_access] buf is %s and size is %d \n",buf,size);
        reg_address = simple_strtoul(buf,&pvalue,16);
        
        if(size > 3)
        {        
            reg_value = simple_strtoul((pvalue+1),NULL,16);        
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_ncp6335_access] write ncp6335 reg 0x%x with value 0x%x !\n",reg_address,reg_value);
            ret=ncp6335_config_interface(reg_address, reg_value, 0xFF, 0x0);
        }
        else
        {    
            ret=ncp6335_read_interface(reg_address, &g_reg_value_ncp6335, 0xFF, 0x0);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_ncp6335_access] read ncp6335 reg 0x%x with value 0x%x !\n",reg_address,g_reg_value_ncp6335);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[store_ncp6335_access] Please use \"cat ncp6335_access\" to get value\r\n");
        }        
    }    
    return size;
}
static DEVICE_ATTR(ncp6335_access, 0664, show_ncp6335_access, store_ncp6335_access); //664

static int ncp6335_user_space_probe(struct platform_device *dev)    
{    
    int ret_device_file = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","******** ncp6335_user_space_probe!! ********\n" );
    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_ncp6335_access);
    
    return 0;
}

struct platform_device ncp6335_user_space_device = {
    .name   = "ncp6335-user",
    .id     = -1,
};

static struct platform_driver ncp6335_user_space_driver = {
    .probe      = ncp6335_user_space_probe,
    .driver     = {
        .name = "ncp6335-user",
    },
};

static struct i2c_board_info __initdata i2c_ncp6335 = { I2C_BOARD_INFO("ncp6335", (ncp6335_SLAVE_ADDR_WRITE>>1))};
#endif

extern unsigned int g_vproc_vsel_gpio_number;

static int __init ncp6335_init(void)
{   
#ifdef NCP6335_AUTO_DETECT_DISABLE

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_init] NCP6335_AUTO_DETECT_DISABLE\n");    
    g_ncp6335_hw_exist=0;
    g_ncp6335_driver_ready=1;

#else

    int ret=0;

    if(g_vproc_vsel_gpio_number != 0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_init] init start.\n");
        
        i2c_register_board_info(ncp6335_BUSNUM, &i2c_ncp6335, 1);

        if(i2c_add_driver(&ncp6335_driver)!=0)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_init] failed to register ncp6335 i2c driver.\n");
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_init] Success to register ncp6335 i2c driver.\n");
        }

        // ncp6335 user space access interface
        ret = platform_device_register(&ncp6335_user_space_device);
        if (ret) {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","****[ncp6335_init] Unable to device register(%d)\n", ret);
            return ret;
        }    
        ret = platform_driver_register(&ncp6335_user_space_driver);
        if (ret) {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","****[ncp6335_init] Unable to register driver (%d)\n", ret);
            return ret;
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC","[ncp6335_init] DCT no define EXT BUCK\n");    
        g_ncp6335_hw_exist=0;
        g_ncp6335_driver_ready=1;
    }
    
#endif    
    
    return 0;        
}

static void __exit ncp6335_exit(void)
{
    i2c_del_driver(&ncp6335_driver);
}

module_init(ncp6335_init);
module_exit(ncp6335_exit);
   
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C ncp6335 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");
