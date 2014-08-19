#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>     
#include <platform/bq24158.h>
#include <printf.h>

int g_bq24158_log_en=0;

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define bq24158_SLAVE_ADDR_WRITE 0xD4
#define bq24158_SLAVE_ADDR_Read    0xD5

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
#define bq24158_REG_NUM 7  
kal_uint8 bq24158_reg[bq24158_REG_NUM] = {0};

/**********************************************************
  *
  *   [I2C Function For Read/Write bq24158] 
  *
  *********************************************************/
U32 bq24158_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = mt_i2c_write(I2C6, chip, cmdBuffer, cmdBufferLen,1);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = mt_i2c_read(I2C6, chip, dataBuffer, dataBufferLen,1);

    //printf("[bq24158_i2c_read] Done\n");

    return ret_code;
}

U32 bq24158_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        printf("[bq24158_i2c_write] exceed I2C FIFO length!! \n");
        return 0;
    }

    //write_data[0] = cmd;
    //write_data[1] = writeData;

    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmdBuffer[cmdIndex];
        cmdIndex++;
        i++;
    }

    while(dataIndex < dataBufferLen)
    {
        write_data[i] = dataBuffer[dataIndex];
        dataIndex++;
        i++;
    }

    /* dump write_data for check */
    for( i=0 ; i < transfer_len ; i++ )
    {
        //printf("[bq24158_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = mt_i2c_write(I2C6, chip, write_data, transfer_len,1);

    //printf("[bq24158_i2c_write] Done\n");

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 bq24158_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    printf("[bq24158_read_interface] Reg[%x]=0x%x\n", RegNum, data);
    
    data &= (MASK << SHIFT);
    *val = (data >> SHIFT);
    
    printf("[bq24158_read_interface] val=0x%x\n", *val);

    if(g_bq24158_log_en>1)        
        printf("%d\n", result_tmp);

    return 1;
}

kal_uint32 bq24158_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[bq24158_config_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = bq24158_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[bq24158_config_interface] write Reg[%x]=0x%x\n", RegNum, data);

    // Check
    result_tmp = bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[bq24158_config_interface] Check Reg[%x]=0x%x\n", RegNum, data);

    if(g_bq24158_log_en>1)        
        printf("%d\n", result_tmp);
    
    return 1;
}

/**********************************************************
  *
  *   [bq24158 Function] 
  *
  *********************************************************/
//CON0----------------------------------------------------

void bq24158_set_tmr_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_TMR_RST_MASK),
                                    (kal_uint8)(CON0_TMR_RST_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

kal_uint32 bq24158_get_otg_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_OTG_MASK),
                                    (kal_uint8)(CON0_OTG_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

void bq24158_set_en_stat(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_EN_STAT_MASK),
                                    (kal_uint8)(CON0_EN_STAT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
            printf("%d\n", ret);    
}

kal_uint32 bq24158_get_chip_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_STAT_MASK),
                                    (kal_uint8)(CON0_STAT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_boost_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_BOOST_MASK),
                                    (kal_uint8)(CON0_BOOST_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_fault_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_FAULT_MASK),
                                    (kal_uint8)(CON0_FAULT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

//CON1----------------------------------------------------

void bq24158_set_input_charging_current(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LIN_LIMIT_MASK),
                                    (kal_uint8)(CON1_LIN_LIMIT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);    
}

void bq24158_set_v_low(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LOW_V_MASK),
                                    (kal_uint8)(CON1_LOW_V_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_te(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_TE_MASK),
                                    (kal_uint8)(CON1_TE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_ce(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_CE_MASK),
                                    (kal_uint8)(CON1_CE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_hz_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_HZ_MODE_MASK),
                                    (kal_uint8)(CON1_HZ_MODE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_opa_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_OPA_MODE_MASK),
                                    (kal_uint8)(CON1_OPA_MODE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON2----------------------------------------------------

void bq24158_set_oreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OREG_MASK),
                                    (kal_uint8)(CON2_OREG_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_otg_pl(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_PL_MASK),
                                    (kal_uint8)(CON2_OTG_PL_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_otg_en(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_EN_MASK),
                                    (kal_uint8)(CON2_OTG_EN_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON3----------------------------------------------------

kal_uint32 bq24158_get_vender_code(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_VENDER_CODE_MASK),
                                    (kal_uint8)(CON3_VENDER_CODE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_pn(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_PIN_MASK),
                                    (kal_uint8)(CON3_PIN_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_revision(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_REVISION_MASK),
                                    (kal_uint8)(CON3_REVISION_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

//CON4----------------------------------------------------

void bq24158_set_reset(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_RESET_MASK),
                                    (kal_uint8)(CON4_RESET_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);    
}

void bq24158_set_iocharge(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_CHR_MASK),
                                    (kal_uint8)(CON4_I_CHR_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_iterm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_TERM_MASK),
                                    (kal_uint8)(CON4_I_TERM_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON5----------------------------------------------------

void bq24158_set_dis_vreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_DIS_VREG_MASK),
                                    (kal_uint8)(CON5_DIS_VREG_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_io_level(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_IO_LEVEL_MASK),
                                    (kal_uint8)(CON5_IO_LEVEL_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

kal_uint32 bq24158_get_sp_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_SP_STATUS_MASK),
                                    (kal_uint8)(CON5_SP_STATUS_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_en_level(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_EN_LEVEL_MASK),
                                    (kal_uint8)(CON5_EN_LEVEL_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

void bq24158_set_vsp(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_VSP_MASK),
                                    (kal_uint8)(CON5_VSP_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON6----------------------------------------------------

void bq24158_set_i_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_ISAFE_MASK),
                                    (kal_uint8)(CON6_ISAFE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_v_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_VSAFE_MASK),
                                    (kal_uint8)(CON6_VSAFE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//debug 
unsigned int bq24158_config_interface_reg (unsigned char RegNum, unsigned char val)
{
    kal_uint32 ret=0;
    kal_uint8 ret_val=0;

    printf("--------------------------------------------------\n");
    
    //bq24158_read_byte(RegNum, &bq24158_reg);
    //ret=bq24158_read_interface(RegNum,&val,0xFF,0x0);
    //printk("[bq24158_config_interface] Reg[%x]=0x%x\n", RegNum, val);
    
    //bq24158_write_byte(RegNum, val);
    ret=bq24158_config_interface(RegNum,val,0xFF,0x0);
    printf("[bq24158_config_interface] write Reg[%x]=0x%x\n", RegNum, val);

    // Check
    //bq24158_read_byte(RegNum, &bq24158_reg);
    ret=bq24158_read_interface(RegNum,&ret_val,0xFF,0x0);
    printf("[bq24158_config_interface] Check Reg[%x]=0x%x\n", RegNum, ret_val);

    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);

    return 1;
}

void bq24158_dump_register(void)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    int i=0;
    for (i=0;i<bq24158_REG_NUM;i++)
    {
        cmd = i;
        bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
        bq24158_reg[i] = data;
        printf("[bq24158_dump_register] Reg[0x%X]=0x%X\n", i, bq24158_reg[i]);
    }
}

void bq24158_read_register(int i)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

    cmd = i;
    bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    bq24158_reg[i] = data;
    printf("[bq24158_read_register] Reg[0x%X]=0x%X\n", i, bq24158_reg[i]);
}

extern int g_enable_high_vbat_spec;
extern int g_pmic_cid;

void bq24158_hw_init(void)
{    
    if(g_enable_high_vbat_spec == 1)
    {
        if(g_pmic_cid == 0x1020)
        {
            printf("[bq24158_hw_init] (0x06,0x70) because 0x1020\n");
            bq24158_config_interface_reg(0x06,0x70); // set ISAFE
        }
        else
        {
            printf("[bq24158_hw_init] (0x06,0x77)\n");
            bq24158_config_interface_reg(0x06,0x77); // set ISAFE and HW CV point (4.34)
        }
    }
    else
    {
        printf("[bq24158_hw_init] (0x06,0x70)\n");
        bq24158_config_interface_reg(0x06,0x70); // set ISAFE
    }
}

