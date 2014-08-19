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
#include <mach/mt_pm_ldo.h>

#include "fan5402.h"
#include "cust_charging.h"
#include <mach/charging.h>


/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define fan5402_SLAVE_ADDR_WRITE   0xD6
#define fan5402_SLAVE_ADDR_Read    0xD7

static struct i2c_client *new_client = NULL;
static const struct i2c_device_id fan5402_i2c_id[] = {{"fan5402",0},{}};   
kal_bool chargin_hw_init_done = KAL_FALSE; 
static int fan5402_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_driver fan5402_driver = {
    .driver = {
        .name    = "fan5402",
    },
    .probe       = fan5402_driver_probe,
    .id_table    = fan5402_i2c_id,
};

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
kal_uint8 fan5402_reg[fan5402_REG_NUM] = {0};

static DEFINE_MUTEX(fan5402_i2c_access);
/**********************************************************
  *
  *   [I2C Function For Read/Write fan5402] 
  *
  *********************************************************/
int fan5402_read_byte(kal_uint8 cmd, kal_uint8 *returnData)
{
    char     cmd_buf[1]={0x00};
    char     readData = 0;
    int      ret=0;

    mutex_lock(&fan5402_i2c_access);
    
    //new_client->addr = ((new_client->addr) & I2C_MASK_FLAG) | I2C_WR_FLAG;    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG;

    cmd_buf[0] = cmd;
    ret = i2c_master_send(new_client, &cmd_buf[0], (1<<8 | 1));
    if (ret < 0) 
    {    
        //new_client->addr = new_client->addr & I2C_MASK_FLAG;
        new_client->ext_flag=0;

        mutex_unlock(&fan5402_i2c_access);
        return 0;
    }
    
    readData = cmd_buf[0];
    *returnData = readData;

    // new_client->addr = new_client->addr & I2C_MASK_FLAG;
    new_client->ext_flag=0;
    
    mutex_unlock(&fan5402_i2c_access);    
    return 1;
}

int fan5402_write_byte(kal_uint8 cmd, kal_uint8 writeData)
{
    char    write_data[2] = {0};
    int     ret=0;
    
    mutex_lock(&fan5402_i2c_access);
    
    write_data[0] = cmd;
    write_data[1] = writeData;
    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_DIRECTION_FLAG;
    
    ret = i2c_master_send(new_client, write_data, 2);
    if (ret < 0) 
    {
       
        new_client->ext_flag=0;
        mutex_unlock(&fan5402_i2c_access);
        return 0;
    }
    
    new_client->ext_flag=0;
    mutex_unlock(&fan5402_i2c_access);
    return 1;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 fan5402_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 fan5402_reg = 0;
    int ret = 0;

   battery_xlog_printk(BAT_LOG_FULL,"--------------------------------------------------\n");

    ret = fan5402_read_byte(RegNum, &fan5402_reg);

	battery_xlog_printk(BAT_LOG_FULL,"[fan5402_read_interface] Reg[%x]=0x%x\n", RegNum, fan5402_reg);
	
    fan5402_reg &= (MASK << SHIFT);
    *val = (fan5402_reg >> SHIFT);
	
	battery_xlog_printk(BAT_LOG_FULL,"[fan5402_read_interface] val=0x%x\n", *val);
	
    return ret;
}

kal_uint32 fan5402_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 fan5402_reg = 0;
    int ret = 0;

    battery_xlog_printk(BAT_LOG_FULL,"--------------------------------------------------\n");

    ret = fan5402_read_byte(RegNum, &fan5402_reg);
    battery_xlog_printk(BAT_LOG_FULL,"[fan5402_config_interface] Reg[%x]=0x%x\n", RegNum, fan5402_reg);
    
    fan5402_reg &= ~(MASK << SHIFT);
    fan5402_reg |= (val << SHIFT);

	if(RegNum == fan5402_CON4 && val == 1 && MASK ==CON4_RESET_MASK && SHIFT == CON4_RESET_SHIFT)
	{
		// RESET bit
	}
	else if(RegNum == fan5402_CON4)
	{
		fan5402_reg &= ~0x80;	//RESET bit read returs 1, so clear it
	}
	 

    ret = fan5402_write_byte(RegNum, fan5402_reg);
    battery_xlog_printk(BAT_LOG_FULL,"[fan5402_config_interface] write Reg[%x]=0x%x\n", RegNum, fan5402_reg);

    // Check
    //fan5402_read_byte(RegNum, &fan5402_reg);
    //printk("[fan5402_config_interface] Check Reg[%x]=0x%x\n", RegNum, fan5402_reg);

    return ret;
}

//write one register directly
kal_uint32 fan5402_reg_config_interface (kal_uint8 RegNum, kal_uint8 val)
{   
    int ret = 0;
    
    ret = fan5402_write_byte(RegNum, val);

    return ret;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//CON0----------------------------------------------------

void fan5402_set_tmr_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_TMR_RST_MASK),
                                    (kal_uint8)(CON0_TMR_RST_SHIFT)
                                    );
}

kal_uint32 fan5402_get_otg_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_OTG_MASK),
                                    (kal_uint8)(CON0_OTG_SHIFT)
                                    );
    return val;
}

void fan5402_set_en_stat(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_EN_STAT_MASK),
                                    (kal_uint8)(CON0_EN_STAT_SHIFT)
                                    );
}

kal_uint32 fan5402_get_chip_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_STAT_MASK),
                                    (kal_uint8)(CON0_STAT_SHIFT)
                                    );
    return val;
}

kal_uint32 fan5402_get_boost_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_BOOST_MASK),
                                    (kal_uint8)(CON0_BOOST_SHIFT)
                                    );
    return val;
}

kal_uint32 fan5402_get_fault_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_FAULT_MASK),
                                    (kal_uint8)(CON0_FAULT_SHIFT)
                                    );
    return val;
}

//CON1----------------------------------------------------

void fan5402_set_input_charging_current(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LIN_LIMIT_MASK),
                                    (kal_uint8)(CON1_LIN_LIMIT_SHIFT)
                                    );
}

void fan5402_set_v_low(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LOW_V_MASK),
                                    (kal_uint8)(CON1_LOW_V_SHIFT)
                                    );
}

void fan5402_set_te(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_TE_MASK),
                                    (kal_uint8)(CON1_TE_SHIFT)
                                    );
}

void fan5402_set_ce(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_CE_MASK),
                                    (kal_uint8)(CON1_CE_SHIFT)
                                    );
}

void fan5402_set_hz_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_HZ_MODE_MASK),
                                    (kal_uint8)(CON1_HZ_MODE_SHIFT)
                                    );
}

void fan5402_set_opa_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_OPA_MODE_MASK),
                                    (kal_uint8)(CON1_OPA_MODE_SHIFT)
                                    );
}

//CON2----------------------------------------------------

void fan5402_set_oreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OREG_MASK),
                                    (kal_uint8)(CON2_OREG_SHIFT)
                                    );
}

void fan5402_set_otg_pl(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_PL_MASK),
                                    (kal_uint8)(CON2_OTG_PL_SHIFT)
                                    );
}

void fan5402_set_otg_en(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_EN_MASK),
                                    (kal_uint8)(CON2_OTG_EN_SHIFT)
                                    );
}

//CON3----------------------------------------------------

kal_uint32 fan5402_get_vender_code(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_VENDER_CODE_MASK),
                                    (kal_uint8)(CON3_VENDER_CODE_SHIFT)
                                    );
    return val;
}

kal_uint32 fan5402_get_pn(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_PIN_MASK),
                                    (kal_uint8)(CON3_PIN_SHIFT)
                                    );
    return val;
}

kal_uint32 fan5402_get_revision(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5402_read_interface(     (kal_uint8)(fan5402_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_REVISION_MASK),
                                    (kal_uint8)(CON3_REVISION_SHIFT)
                                    );
    return val;
}

//CON4----------------------------------------------------

void fan5402_set_reset(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_RESET_MASK),
                                    (kal_uint8)(CON4_RESET_SHIFT)
                                    );
}

void fan5402_set_iocharge(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_CHR_MASK),
                                    (kal_uint8)(CON4_I_CHR_SHIFT)
                                    );
}

void fan5402_set_iterm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5402_config_interface(   (kal_uint8)(fan5402_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_TERM_MASK),
                                    (kal_uint8)(CON4_I_TERM_SHIFT)
                                    );
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
void fan5402_dump_register(void)
{
    int i=0;
    printk("[fan5402] ");
    for (i=0;i<fan5402_REG_NUM;i++)
    {
        fan5402_read_byte(i, &fan5402_reg[i]);
        printk("[0x%x]=0x%x ", i, fan5402_reg[i]);        
    }
    printk("\n");
}

#if 0
extern int g_enable_high_vbat_spec;
extern int g_pmic_cid;

void fan5402_hw_init(void)
{    
}
#endif

static int fan5402_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{             
    int err=0; 

    battery_xlog_printk(BAT_LOG_CRTI,"[fan5402_driver_probe] \n");

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }    
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client = client;    

    //---------------------
  //  fan5402_hw_init();
    fan5402_dump_register();
    chargin_hw_init_done = KAL_TRUE;
	
    return 0;                                                                                       

exit:
    return err;

}

/**********************************************************
  *
  *   [platform_driver API] 
  *
  *********************************************************/
kal_uint8 g_reg_value_fan5402=0;
static ssize_t show_fan5402_access(struct device *dev,struct device_attribute *attr, char *buf)
{
    battery_xlog_printk(BAT_LOG_FULL,"[show_fan5402_access] 0x%x\n", g_reg_value_fan5402);
    return sprintf(buf, "%u\n", g_reg_value_fan5402);
}
static ssize_t store_fan5402_access(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int ret=0;
    char *pvalue = NULL;
    unsigned int reg_value = 0;
    unsigned int reg_address = 0;
    
    battery_xlog_printk(BAT_LOG_FULL,"[store_fan5402_access] \n");
    
    if(buf != NULL && size != 0)
    {
        battery_xlog_printk(BAT_LOG_FULL,"[store_fan5402_access] buf is %s and size is %d \n",buf,size);
        reg_address = simple_strtoul(buf,&pvalue,16);
        
        if(size > 3)
        {        
            reg_value = simple_strtoul((pvalue+1),NULL,16);        
            battery_xlog_printk(BAT_LOG_FULL,"[store_fan5402_access] write fan5402 reg 0x%x with value 0x%x !\n",reg_address,reg_value);
            ret=fan5402_config_interface(reg_address, reg_value, 0xFF, 0x0);
        }
        else
        {    
            ret=fan5402_read_interface(reg_address, &g_reg_value_fan5402, 0xFF, 0x0);
            battery_xlog_printk(BAT_LOG_FULL,"[store_fan5402_access] read fan5402 reg 0x%x with value 0x%x !\n",reg_address,g_reg_value_fan5402);
            battery_xlog_printk(BAT_LOG_FULL,"[store_fan5402_access] Please use \"cat fan5402_access\" to get value\r\n");
        }        
    }    
    return size;
}
static DEVICE_ATTR(fan5402_access, 0664, show_fan5402_access, store_fan5402_access); //664

static int fan5402_user_space_probe(struct platform_device *dev)    
{    
    int ret_device_file = 0;

    battery_xlog_printk(BAT_LOG_CRTI,"******** fan5402_user_space_probe!! ********\n" );
    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_fan5402_access);
    
    return 0;
}

struct platform_device fan5402_user_space_device = {
    .name   = "fan5402-user",
    .id     = -1,
};

static struct platform_driver fan5402_user_space_driver = {
    .probe      = fan5402_user_space_probe,
    .driver     = {
        .name = "fan5402-user",
    },
};


static struct i2c_board_info __initdata i2c_fan5402 = { I2C_BOARD_INFO("fan5402", (fan5402_SLAVE_ADDR_WRITE>>1))};

static int __init fan5402_init(void)
{    
    int ret=0;
    
    battery_xlog_printk(BAT_LOG_CRTI,"[fan5402_init] init start\n");
    
    i2c_register_board_info(FAN5402_BUSNUM, &i2c_fan5402, 1);

    if(i2c_add_driver(&fan5402_driver)!=0)
    {
        battery_xlog_printk(BAT_LOG_CRTI,"[fan5402_init] failed to register fan5402 i2c driver.\n");
    }
    else
    {
        battery_xlog_printk(BAT_LOG_CRTI,"[fan5402_init] Success to register fan5402 i2c driver.\n");
    }

    // fan5402 user space access interface
    ret = platform_device_register(&fan5402_user_space_device);
    if (ret) {
        battery_xlog_printk(BAT_LOG_CRTI,"****[fan5402_init] Unable to device register(%d)\n", ret);
        return ret;
    }    
    ret = platform_driver_register(&fan5402_user_space_driver);
    if (ret) {
        battery_xlog_printk(BAT_LOG_CRTI,"****[fan5402_init] Unable to register driver (%d)\n", ret);
        return ret;
    }
    
    return 0;        
}

static void __exit fan5402_exit(void)
{
    i2c_del_driver(&fan5402_driver);
}

module_init(fan5402_init);
module_exit(fan5402_exit);
   
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C fan5402 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");
