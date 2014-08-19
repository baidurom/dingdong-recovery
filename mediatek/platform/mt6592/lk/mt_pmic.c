#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/mt_gpt.h>
#include <platform/mt_pmic_wrap_init.h>
#include <printf.h>

//==============================================================================
// Global variable
//==============================================================================
int Enable_PMIC_LOG = 1;

CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

extern int g_R_BAT_SENSE;
extern int g_R_I_SENSE;
extern int g_R_CHARGER_1;
extern int g_R_CHARGER_2;

#ifdef MTK_MT6333_SUPPORT
extern void mt6333_init (void);
#endif

//==============================================================================
// PMIC access API
//==============================================================================
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;    

    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //dprintf(INFO, "[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= (MASK << SHIFT);
    *val = (pmic_reg >> SHIFT);    
    //dprintf(INFO, "[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;

    //1. mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //dprintf(INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= ~(MASK << SHIFT);
    pmic_reg |= (val << SHIFT);

    //2. mt_write_byte(RegNum, pmic_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic_reg, &rdata);
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //dprintf(INFO, "[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic_reg);    

#if 0
    //3. Double Check    
    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        dprintf(INFO, "[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    dprintf(INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
#endif    

    return return_value;
}

//==============================================================================
// PMIC APIs
//==============================================================================


//==============================================================================
// PMIC Usage APIs
//==============================================================================
kal_bool upmu_is_chr_det(void)
{
	U32 tmp32=0;
	tmp32=upmu_get_rgs_chrdet();
	if(tmp32 == 0)
	{		
		return KAL_FALSE;
	}
	else
	{	
		return KAL_TRUE;
	}
}

kal_bool pmic_chrdet_status(void)
{
    if( upmu_is_chr_det() == KAL_TRUE )    
    {
        #ifndef USER_BUILD
        dprintf(INFO, "[pmic_chrdet_status] Charger exist\r\n");
        #endif
        
        return KAL_TRUE;
    }
    else
    {
        #ifndef USER_BUILD
        dprintf(INFO, "[pmic_chrdet_status] No charger\r\n");
        #endif
        
        return KAL_FALSE;
    }
}

int pmic_detect_powerkey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(CHRSTATUS),
                             (&val),
                             (U32)(PMIC_PWRKEY_DEB_MASK),
                             (U32)(PMIC_PWRKEY_DEB_SHIFT)
                             );

    if(Enable_PMIC_LOG>1) 
      dprintf(INFO, "%d", ret);

    if (val==1){
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic powerkey Release\n");
        #endif
        
        return 0;
    }else{
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic powerkey Press\n");
        #endif
        
        return 1;
    }
}

int pmic_detect_homekey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(CHRSTATUS),
                             (&val),
                             (U32)(PMIC_FCHRKEY_DEB_MASK),
                             (U32)(PMIC_FCHRKEY_DEB_SHIFT)
                             );

    if(Enable_PMIC_LOG>1) 
      dprintf(INFO, "%d", ret);

    if (val==1){     
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic FCHRKEY Release\n");
        #endif
        
        return 0;
    }else{
        #ifndef USER_BUILD
        dprintf(INFO, "LK pmic FCHRKEY Press\n");
        #endif
        
        return 1;
    }
}

kal_uint32 upmu_get_reg_value(kal_uint32 reg)
{
	U32 ret=0;
	U32 temp_val=0;

	ret=pmic_read_interface(reg, &temp_val, 0xFFFF, 0x0);

    if(Enable_PMIC_LOG>1) 
      dprintf(INFO, "%d", ret);

	return temp_val;
}

void PMIC_DUMP_ALL_Register(void)
{
    U32 i=0;
    U32 ret=0;
    U32 reg_val=0;

    for (i=0;i<0x800;i++)
    {
        ret=pmic_read_interface(i,&reg_val,0xFFFF,0);
        dprintf(INFO, "Reg[0x%x]=0x%x, %d\n", i, reg_val, ret);
    }
}

//==============================================================================
// PMIC Init Code
//==============================================================================
void PMIC_INIT_SETTING_V1(void)
{
    U32 chip_version = 0;

    chip_version = upmu_get_cid();

    dprintf(CRITICAL, "[LK_PMIC_INIT_SETTING_V1] PMIC Chip = 0x%x\n",chip_version);
    //printf("[LK_PMIC_INIT_SETTING_V1] PMIC Chip = 0x%x\n",chip_version);
}

void PMIC_CUSTOM_SETTING_V1(void)
{
    //dprintf(INFO, "[PMIC_CUSTOM_SETTING_V1] \n");
}

U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;

	  //dprintf(INFO, "\n[LK_pmic_init] LK Start...................\n");

	  upmu_set_rg_chrind_on(0);    
	  //dprintf(INFO, "[LK_PMIC_INIT] Turn Off chrind\n");

	  PMIC_INIT_SETTING_V1();
	  //dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Done\n");	
    
	  PMIC_CUSTOM_SETTING_V1();
	  //dprintf(INFO, "[LK_PMIC_CUSTOM_SETTING_V1] Done\n");

	  pmic_detect_powerkey();
	  dprintf(INFO, "chr detection : %d \n",upmu_is_chr_det());

	  //PMIC_DUMP_ALL_Register();

      #ifdef MTK_MT6333_SUPPORT
      mt6333_init();
      #endif

	  return ret_code;
}

//==============================================================================
// PMIC API for LK : AUXADC
//==============================================================================
int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd)
{
	kal_int32 ret_data;	
	kal_int32 u4Sample_times = 0;
	kal_int32 u4channel=0;	
	kal_int32 adc_result_temp=0;
       kal_int32 r_val_temp=0;   
	kal_int32 adc_result=0;   
    kal_int32 ret=0;
    kal_uint32 adc_reg_val=0;
	
    /*
        0 : BATON2
        1 : CH6
        2 : THR SENSE2
        3 : THR SENSE1
        4 : VCDT
        5 : BATON1
        6 : ISENSE
        7 : BATSNS
        8 : ACCDET    
    */
    upmu_set_rg_vbuf_en(1);

    //set 0
    ret=pmic_read_interface(AUXADC_CON22,&adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
    adc_reg_val = adc_reg_val & (~(1<<dwChannel));
    ret=pmic_config_interface(AUXADC_CON22,adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);

    //set 1
    ret=pmic_read_interface(AUXADC_CON22,&adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
    adc_reg_val = adc_reg_val | (1<<dwChannel);
    ret=pmic_config_interface(AUXADC_CON22,adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);

    if(Enable_PMIC_LOG>1) 
      dprintf(INFO, "%d", ret);

	do
	{
	    ret_data=0;

	    switch(dwChannel){         
	        case 0:    
	            while( upmu_get_rg_adc_rdy_baton2() != 1 );
	            ret_data = upmu_get_rg_adc_out_baton2();				
	            break;
	        case 1:    
	            while( upmu_get_rg_adc_rdy_ch6() != 1 );
	            ret_data = upmu_get_rg_adc_out_ch6();				
	            break;
	        case 2:    
	            while( upmu_get_rg_adc_rdy_thr_sense2() != 1 );
	            ret_data = upmu_get_rg_adc_out_thr_sense2();				
	            break;				
	        case 3:    
	            while( upmu_get_rg_adc_rdy_thr_sense1() != 1 );
	            ret_data = upmu_get_rg_adc_out_thr_sense1();				
	            break;
	        case 4:    
	            while( upmu_get_rg_adc_rdy_vcdt() != 1 );
	            ret_data = upmu_get_rg_adc_out_vcdt();				
	            break;
	        case 5:    
	            while( upmu_get_rg_adc_rdy_baton1() != 1 );
	            ret_data = upmu_get_rg_adc_out_baton1();				
	            break;
	        case 6:    
	            while( upmu_get_rg_adc_rdy_isense() != 1 );
	            ret_data = upmu_get_rg_adc_out_isense();				
	            break;
	        case 7:    
	            while( upmu_get_rg_adc_rdy_batsns() != 1 );
	            ret_data = upmu_get_rg_adc_out_batsns();				
	            break; 
	        case 8:    
	            while( upmu_get_rg_adc_rdy_ch5() != 1 );
	            ret_data = upmu_get_rg_adc_out_ch5();				
	            break; 				
                
	        default:
	            dprintf(INFO, "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
	           
	            return -1;
	            break;
	    }

	    u4channel += ret_data;

	    u4Sample_times++;

	   // if (Enable_BATDRV_LOG == 1)
	    {
	        //debug
	        dprintf(INFO, "[AUXADC] u4channel[%d]=%d.\n", 
	            dwChannel, ret_data);
	    }
	    
	}while (u4Sample_times < deCount);

    /* Value averaging  */ 
    adc_result_temp = u4channel / deCount;

    switch(dwChannel){         
        case 0:                
            r_val_temp = 1;           
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 1:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 2:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 3:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 4:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 5:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 6:    
            r_val_temp = 4;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 7:    
            r_val_temp = 4;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;    
        case 8:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;    			
        default:
            dprintf(INFO, "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);

            return -1;
            break;
    }

        dprintf(INFO, "[AUXADC] adc_result_temp=%d, adc_result=%d, r_val_temp=%d.\n", 
                adc_result_temp, adc_result, r_val_temp);

	
    return adc_result;
}

int get_bat_sense_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(VBAT_CHANNEL_NUMBER,times,1);
}

int get_i_sense_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(ISENSE_CHANNEL_NUMBER,times,1);
}

int get_charger_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(VCHARGER_CHANNEL_NUMBER,times,1);
}

int get_tbat_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(VBATTEMP_CHANNEL_NUMBER,times,1);
}


