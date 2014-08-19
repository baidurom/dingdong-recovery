#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mtk_pmic_6320.h>
#include <asm/arch/upmu_common.h>
#include <asm/arch/upmu_hw.h>
#include "asm/arch/mt_pmic_wrap_uboot.h"

//==============================================================================
// PMIC lock/unlock APIs
//==============================================================================
void pmic_lock(void)
{
    //TODO
}

void pmic_unlock(void)
{
    //TODO
}

//==============================================================================
// PMIC6320 Access Interface
//==============================================================================
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6320_reg = 0;
    U32 rdata;    

    //mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;
    if(return_value!=0)
    {   
        printf("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    printf("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
	
    pmic6320_reg &= (MASK << SHIFT);
    *val = (pmic6320_reg >> SHIFT);	
    printf("[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6320_reg = 0;
    U32 rdata;

    //1. mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;    
    if(return_value!=0)
    {   
        printf("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    printf("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
	
    pmic6320_reg &= ~(MASK << SHIFT);
    pmic6320_reg |= (val << SHIFT);

    //2. mt6320_write_byte(RegNum, pmic6320_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6320_reg, &rdata);
    if(return_value!=0)
    {   
        printf("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    printf("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic6320_reg);    

#if 0
    //3. Double Check    
    //mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;    
    if(return_value!=0)
    {   
        printf("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    printf("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
#endif	

    return return_value;
}

//==============================================================================
// PMIC6320 API 
//==============================================================================

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC-Charger Type Detection
//////////////////////////////////////////////////////////////////////////////////////////
CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

CHARGER_TYPE hw_charger_type_detection(void)
{
#if 0
	CHARGER_TYPE ret 				= CHARGER_UNKNOWN;
	unsigned int USB_U2PHYACR3_2 	= 0xC126081C;
	unsigned int USBPHYRegs			= 0xC1260800; //U2B20_Base+0x800
	unsigned short wChargerAvail	= 0;
	unsigned short bLineState_B		= 0;
	unsigned short bLineState_C 	= 0;
	int ret_val						= 0;
	unsigned short testReadValue	= 0;

	//msleep(400);
	//printf("mt_charger_type_detection : start!\r\n");

/********* Step 0.0 : enable USB memory and clock *********/
	//hwEnableClock(MT65XX_PDN_PERI_USB1, "pmu");
	//hwEnableClock(MT65XX_PDN_PERI_USB2, "pmu");
	//mdelay(1);

/********* Step 1.0 : PMU_BC11_Detect_Init ***************/		
	SETREG8(USB_U2PHYACR3_2,0x80); //USB_U2PHYACR3[7]=1		
	
	//BC11_RST=1
	ret_val=pmic_config_interface(0x33,0x1,BANK_0_BC11_RST_MASK,BANK_0_BC11_RST_SHIFT); 
	//BC11_RST=0	
	//ret_val=pmic_config_interface(0x33,0x0,BANK_0_BC11_RST_MASK,BANK_0_BC11_RST_SHIFT); 
	//BC11_BB_CTRL=1
	ret_val=pmic_config_interface(0x33,0x1,BANK_0_BC11_BB_CTRL_MASK,BANK_0_BC11_BB_CTRL_SHIFT);
	
	//RG_BC11_BIAS_EN=1	
	ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_BIAS_EN_MASK,BANK_0_BC11_BIAS_EN_SHIFT); 
	//RG_BC11_VSRC_EN[1:0]=00
	ret_val=pmic_config_interface(0x33,0x0,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_VREF_VTH = 0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_VREF_VTH_MASK,BANK_0_BC11_VREF_VTH_SHIFT); 
	//RG_BC11_CMP_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	//RG_BC11_IPU_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
	//RG_BC11_IPD_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);

	//ret_val=pmic_read_interface(0x33,&testReadValue,0xFF,0);		
	//printf("Reg[0x33]=%x, ", testReadValue);
	//ret_val=pmic_read_interface(0x34,&testReadValue,0xFF,0);		
	//printf("Reg[0x34]=%x \r\n", testReadValue);

/********* Step A *************************************/
	//printf("mt_charger_type_detection : step A\r\n");
	//RG_BC11_IPU_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
	
	SETREG8(USBPHYRegs+0x1C,0x1000);//RG_PUPD_BIST_EN = 1	
	CLRREG8(USBPHYRegs+0x1C,0x0400);//RG_EN_PD_DM=0
	
	//RG_BC11_VSRC_EN[1.0] = 10 
	ret_val=pmic_config_interface(0x33,0x2,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_IPD_EN[1:0] = 01
	ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);
	//RG_BC11_VREF_VTH = 0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_VREF_VTH_MASK,BANK_0_BC11_VREF_VTH_SHIFT);
  	//RG_BC11_CMP_EN[1.0] = 01
  	ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	//mdelay(400);
	mdelay(100);
		
	ret_val=pmic_read_interface(0x33,&wChargerAvail,BANK_0_BC11_CMP_OUT_MASK,BANK_0_BC11_CMP_OUT_SHIFT); 
	//printf("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
	
	//RG_BC11_VSRC_EN[1:0]=00
	ret_val=pmic_config_interface(0x33,0x0,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_IPD_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);
	//RG_BC11_CMP_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	mdelay(50);
	
	if(wChargerAvail==1)
	{
/********* Step B *************************************/
		//printf("mt_charger_type_detection : step B\r\n");

		//RG_BC11_IPU_EN[1:0]=10
		ret_val=pmic_config_interface(0x34,0x2,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);		
		mdelay(80);
		
		bLineState_B = INREG8(USBPHYRegs+0x76);
		//printf("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
		if(bLineState_B & 0x80)
		{
			ret = STANDARD_CHARGER;
			printf("mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
		}
		else
		{
			ret = CHARGING_HOST;
			printf("mt_charger_type_detection : step B : Charging Host!\r\n");
		}
	}
	else
	{
/********* Step C *************************************/
		//printf("mt_charger_type_detection : step C\r\n");

		//RG_BC11_IPU_EN[1:0]=01
		ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
		//RG_BC11_CMP_EN[1.0] = 01
		ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
		//ret_val=pmic_read_interface(0x34,&testReadValue,0xFF,0);			
		//printf("mt_charger_type_detection : step C : Reg[0x34]=%x\r\n", testReadValue);		
		mdelay(80);
		
		ret_val=pmic_read_interface(0x33,&bLineState_C,0xFF,0);
		//printf("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
		if(bLineState_C & 0x80)
		{
			ret = NONSTANDARD_CHARGER;
			printf("mt_charger_type_detection : step C : UNSTANDARD CHARGER!!\r\n");
			//RG_BC11_IPU_EN[1:0]=10
			ret_val=pmic_config_interface(0x34,0x2,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);			
			mdelay(80);
		}
		else
		{
			ret = STANDARD_HOST;
			printf("mt_charger_type_detection : step C : Standard USB Host!!\r\n");
		}
	}
/********* Finally setting *******************************/
	//RG_BC11_VSRC_EN[1:0]=00
	ret_val=pmic_config_interface(0x33,0x0,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_VREF_VTH = 0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_VREF_VTH_MASK,BANK_0_BC11_VREF_VTH_SHIFT);
	//RG_BC11_CMP_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	//RG_BC11_IPU_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
	//RG_BC11_IPD_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);
	//RG_BC11_BIAS_EN=0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_BIAS_EN_MASK,BANK_0_BC11_BIAS_EN_SHIFT); 
	
	CLRREG8(USB_U2PHYACR3_2,0x80); //USB_U2PHYACR3[7]=0

	//hwDisableClock(MT65XX_PDN_PERI_USB1, "pmu");
	//hwDisableClock(MT65XX_PDN_PERI_USB2, "pmu");

	//step4:done, ret the type	
	return ret;
#endif	
}

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
		g_first_check = 1;
		g_ret = hw_charger_type_detection();
    }
    else
    {
		printf("[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

//==============================================================================
// PMIC6320 API for read value
//==============================================================================
kal_uint32 upmu_get_reg_value(kal_uint32 reg)
{
    return 0;
}

//==============================================================================
// PMIC6320 API for UBOOT : AUXADC
//==============================================================================
int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount)
{	
    return 0;	
}

//==============================================================================
// PMIC6320 API for UBOOT : Get CHRDET status
//==============================================================================
U32 pmic_IsUsbCableIn (void) 
{
#if 0
    U32 ret_code = I2C_OK;
    U32 delay_time_us = 50000;
    U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    //U8 cmd = PMIC_STATUS_REG;
    U8 cmd = BANK0_CHRSTATUS;
    int cmd_len = 1;
    U8 data = 0x00;
    int data_len = 1;	
    
    //gpt4_busy_wait_us(delay_time_us);

    ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //if (data & (CHRDET_MASK << CHRDET_SHIFT)){
    if (data & (BANK_0_PCHR_CHRDET_MASK << BANK_0_PCHR_CHRDET_SHIFT)){
        //printf("[PMIC_IsUsbCableIn] Get register %x : data = %x\n", cmd, data);
        printf("[UBOOT_PMIC_IsUsbCableIn] PMIC_CHRDET_EXIST\n");
        ret_code = PMIC_CHRDET_EXIST;
    }else{
        //printf("[PMIC_IsUsbCableIn] Get register %x : data = %x\n", cmd, data);
        printf("[UBOOT_PMIC_IsUsbCableIn] PMIC_CHRDET_NOT_EXIST\n");
        //ret_code = PMIC_CHRDET_NOT_EXIST;
    }

    return ret_code;
#endif
    return 0;
}

kal_uint32 upmu_get_PCHR_CHRDET(void)
{
    return 0;
}

kal_bool upmu_is_chr_det(void)
{
	kal_uint32 tmp32;
	tmp32=upmu_get_PCHR_CHRDET();
	if(tmp32 == 0)
	{
		//printf("[upmu_is_chr_det] No charger\n");
		return KAL_FALSE;
	}
	else
	{
		//printf("[upmu_is_chr_det] Charger exist\n");
		return KAL_TRUE;
	}
}

//==============================================================================
// PMIC6320 API for UBOOT : Get key status
//==============================================================================
int pmic_detect_powerkey(void)
{
#if 0
	U32 ret_code = I2C_OK;
	U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    U8 cmd = 0x09;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

	ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

	if (data & (BANK_0_PWRKEY_DEB_MASK << BANK_0_PWRKEY_DEB_SHIFT)){     
        printf("[mt6575_detect_powerkey] Release\n");
        //return 1;
        return 0;
    }else{
        printf("[mt6575_detect_powerkey] Press\n");
        //return 0;
        return 1;
    }
#endif
    return 0;
}

int pmic_detect_homekey(void)
{
#if 0
	U32 ret_code = I2C_OK;
	U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0C;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

	ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

	if (data & (BANK_0_HOMEKEY_DEB_MASK << BANK_0_HOMEKEY_DEB_SHIFT)){     
        printf("[mt6575_detect_homekey] Release\n");
        return 0;
    }else{
        printf("[mt6575_detect_homekey] Press\n");
        return 1;
    }
#endif
    return 0;
}

//==============================================================================
// PMIC6320 API for UBOOT : Initial setting
//==============================================================================
U32 get_pmic6320_chip_version (void)
{    
    U32 chip_version = 0;

    chip_version = upmu_get_cid();

    return chip_version;
}

void PMIC_DUMP_ALL_Register(void)
{
#if 0
	kal_uint32 i=0;
	kal_uint32 ret=0;
	kal_uint8 reg_val=0;
	
	for (i=0;i<0xFF;i++)
	{
		ret=pmic_read_interface(i,&reg_val,0xFF,0);
        printf("[Bank0] Reg[0x%x]=0x%x\n", i, reg_val);        
    }
	
    for (i=0;i<0xFF;i++)
	{
		ret=pmic_bank1_read_interface(i,&reg_val,0xFF,0);
        printf("[Bank1] Reg[0x%x]=0x%x\n", i, reg_val);        
    }
#endif
}

void PMIC_INIT_SETTING_V1(void)
{
    U32 chip_version = 0;

	chip_version = upmu_get_cid();

	if(chip_version == PMIC6320_E1_CID_CODE)
	{
	    printf("[UBOOT_PMIC_INIT_SETTING_V1] PMIC Chip = %d\n", chip_version);

		//put init setting from DE/SA
		//...
	}
	else
	{
	    printf("[UBOOT_PMIC_INIT_SETTING_V1] Unknown PMIC Chip (%d)\n", chip_version);
	}
}

void PMIC_CUSTOM_SETTING_V1(void)
{	
}

U32 pmic6320_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;

	printf("\n[UBOOT_pmic6320_init] Start...................\n");

	//Put init setting here
#if 0	
	upmu_strup_usbdl_en(0);
	upmu_chr_chrind_on(0);
	printf("[UBOOT_PMIC_INIT] Turn Off chrind\n");
#endif	

	PMIC_INIT_SETTING_V1();
	printf("[UBOOT_PMIC_INIT_SETTING_V1] Done\n");
	PMIC_CUSTOM_SETTING_V1();
	printf("[UBOOT_PMIC_CUSTOM_SETTING_V1] Done\n");	

    //PMIC_DUMP_ALL_Register();

	return ret_code;
}

