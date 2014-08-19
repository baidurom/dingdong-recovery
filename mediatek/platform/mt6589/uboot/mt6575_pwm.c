/*******************************************************************************
* mt6575_pwm.c PWM Drvier                                                     
*                                                                                             
* Copyright (c) 2010, Media Teck.inc                                           
*                                                                             
* This program is free software; you can redistribute it and/or modify it     
* under the terms and conditions of the GNU General Public Licence,            
* version 2, as publish by the Free Software Foundation.                       
*                                                                              
* This program is distributed and in hope it will be useful, but WITHOUT       
* ANY WARRNTY; without even the implied warranty of MERCHANTABITLITY or        
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for     
* more details.                                                                
*                                                                              
*                                                                              
********************************************************************************
* Author : Changlei Gao (changlei.gao@mediatek.com)                              
********************************************************************************
*/
#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt6575_pwm.h>  
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/io.h>

#ifdef PWM_DEBUG
	#define PWMDBG(fmt, args ...) printf("pwm %5d: " fmt, __LINE__,##args)
#else
	#define PWMDBG(fmt, args ...)
#endif

#define PWMMSG(fmt, args ...)  printf(fmt, ##args)

#define APMCUSYS_PDN  			(PERI_CON_BASE+0x0010)


#define PWM_SET_BITS(BS,REG)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define PWM_CLR_BITS(BS,REG)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

static U32 pwm_status=0; 


enum {                                                                                     
	PWM_CON,                                                                              
	PWM_HDURATION,                                                                        
	PWM_LDURATION,                                                                        
	PWM_GDURATION,                                                                        
	PWM_BUF0_BASE_ADDR,                                                                   
	PWM_BUF0_SIZE,                                                                        
	PWM_BUF1_BASE_ADDR,                                                                   
	PWM_BUF1_SIZE,                                                                        
	PWM_SEND_DATA0,                                                                       
	PWM_SEND_DATA1,                                                                       
	PWM_WAVE_NUM,                                                                         
	PWM_DATA_WIDTH,      //only PWM1, PWM2, PWM3, PWM7 have such a register for old mode  
	PWM_THRESH,          //only PWM1, PWM2, PWM3, PWM7 have such a register for old mode  
	PWM_SEND_WAVENUM,                                                                     
	PWM_VALID                                                                             
}PWM_REG_OFF;

U32 PWM_register[PWM_NUM]={                                                                                                                                                     
	(PWM_BASE+0x0010),     //PWM1 REGISTER BASE,   15 registers  
	(PWM_BASE+0x0050),     //PWM2 register base    15 registers                
	(PWM_BASE+0x0090),     //PWM3 register base    15 registers                
	(PWM_BASE+0x00d0),     //PWM4 register base    13 registers                
	(PWM_BASE+0x0110),     //PWM5 register base    13 registers                
	(PWM_BASE+0x0150),     //PWM6 register base    13 registers                
	(PWM_BASE+0x0190)      //PWM7 register base    15 registers                
}; 

void mt_power_on(U32 pwm_no)
{
	if ( (pwm_status & 0x38) == 0) {
		PWM_CLR_BITS(APMCUSYS_PDN, (1<<0x5));
		PWMDBG("EnableClock PWM456\n");
	}
	if ( (pwm_status & 0x01) == 0) {
		PWM_CLR_BITS(APMCUSYS_PDN, (1<<0x2));
		PWMDBG("EnableClock PWM1\n");
	}
	if ( (pwm_status & 0x02) == 0) {
		PWM_CLR_BITS(APMCUSYS_PDN, (1<<0x3));
		PWMDBG("EnableClock PWM2\n");
	}
	if ( (pwm_status & 0x04) == 0) {
		PWM_CLR_BITS(APMCUSYS_PDN, (1<<0x4));
		PWMDBG("EnableClock PWM3\n");
	}
	if ( (pwm_status & 0x40) == 0) {
		PWM_CLR_BITS(APMCUSYS_PDN, (1<<0x6));
		PWMDBG("EnableClock PWM7\n");
	}
	
	pwm_status |= (1 << pwm_no);
}

void mt_power_off (U32 pwm_no)
{
	pwm_status &= ~(1 << pwm_no );
	
	if ( (pwm_status & 0x38) == 0) {
		PWM_SET_BITS(APMCUSYS_PDN, (1<<0x5));
		PWMDBG("DisableClock PWM456\n");
	}
	if ( (pwm_status & 0x01) == 0) {
		PWM_SET_BITS(APMCUSYS_PDN, (1<<0x2));
		PWMDBG("DisableClock PWM1\n");
	}
	if ( (pwm_status & 0x02) == 0) {
		PWM_SET_BITS(APMCUSYS_PDN, (1<<0x3));
		PWMDBG("DisableClock PWM2\n");
	}
	if ( (pwm_status & 0x04) == 0) {
		PWM_SET_BITS(APMCUSYS_PDN, (1<<0x4));
		PWMDBG("DisableClock PWM3\n");
	}
	if ( (pwm_status & 0x40) == 0) {
		PWM_SET_BITS(APMCUSYS_PDN, (1<<0x6));
		PWMDBG("DisableClock PWM7\n");
	}
}

/*******************************************************
*   Set PWM_ENABLE register bit to enable pwm1~pwm7
*
********************************************************/
S32 mt_set_pwm_enable(U32 pwm_no) 
{
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number is not between PWM1~PWM7(0~6)\n" );
		return -EEXCESSPWMNO;
	} 

	SETREG32(PWM_ENABLE, 1 << pwm_no);
	return RSUCCESS;
}


/*******************************************************/
S32 mt_set_pwm_disable ( U32 pwm_no )  
{
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number is not between PWM1~PWM7 (0~6)\n" );
		return -EEXCESSPWMNO;
	}

	CLRREG32 ( PWM_ENABLE, 1 << pwm_no );

	mdelay(1);

	return RSUCCESS;
}

void mt_set_pwm_enable_seqmode(void)
{	
	SETREG32 ( PWM_ENABLE, 1 << PWM_ENABLE_SEQ_OFFSET );
}

void mt_set_pwm_disable_seqmode(void)
{
	CLRREG32 ( PWM_ENABLE,1 << PWM_ENABLE_SEQ_OFFSET );
}

S32 mt_set_pwm_test_sel(U32 val)  //val as 0 or 1
{
	if (val == TEST_SEL_TRUE)
		SETREG32 ( PWM_ENABLE, 1 << PWM_ENABLE_TEST_SEL_OFFSET );
	else if ( val == TEST_SEL_FALSE )
		CLRREG32 ( PWM_ENABLE, 1 << PWM_ENABLE_TEST_SEL_OFFSET );
	else
		goto err;	
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

S32 mt_set_pwm_clk ( U32 pwm_no, U32 clksrc, U32 div )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	if ( div >= CLK_DIV_MAX ) {
		PWMDBG ("division excesses CLK_DIV_MAX\n");
		return -EPARMNOSUPPORT;
	}

	if (clksrc > CLK_BLOCK_BY_1625_OR_32K) {
		PWMDBG("clksrc excesses CLK_BLOCK_BY_1625_OR_32K\n");
		return -EPARMNOSUPPORT;
	}

	reg_con = PWM_register [pwm_no] + 4* PWM_CON;

	MASKREG32 ( reg_con, PWM_CON_CLKDIV_MASK, div );
	if (clksrc == CLK_BLOCK)
		CLRREG32 ( reg_con, 1 << PWM_CON_CLKSEL_OFFSET );
	else if (clksrc == CLK_BLOCK_BY_1625_OR_32K)
		SETREG32 ( reg_con, 1 << PWM_CON_CLKSEL_OFFSET );
	
	return RSUCCESS;
}

/******************************************
* Set PWM_CON register data source
* pwm_no: pwm1~pwm7(0~6)
*val: 0 is fifo mode
*       1 is memory mode
*******************************************/

S32 mt_set_pwm_con_datasrc ( U32 pwm_no, U32 val )
{
	U32 reg_con;
		
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == FIFO )
		CLRREG32 ( reg_con, 1 << PWM_CON_SRCSEL_OFFSET );
	else if ( val == MEMORY )
		SETREG32 ( reg_con, 1 << PWM_CON_SRCSEL_OFFSET );
	else 
		goto err;
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}


/************************************************
*  set the PWM_CON register
* pwm_no : pwm1~pwm7 (0~6)
* val: 0 is period mode
*        1 is random mode
*
***************************************************/
S32 mt_set_pwm_con_mode( U32 pwm_no, U32 val )
{
	U32 reg_con;
	
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == PERIOD )
		CLRREG32 ( reg_con, 1 << PWM_CON_MODE_OFFSET );
	else if (val == RAND)
		SETREG32 ( reg_con, 1 << PWM_CON_MODE_OFFSET );
	else
		goto err;
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/***********************************************
*Set PWM_CON register, idle value bit 
* val: 0 means that  idle state is not put out.
*       1 means that idle state is put out
*
*      IDLE_FALSE: 0
*      IDLE_TRUE: 1
***********************************************/
S32 mt_set_pwm_con_idleval(U32 pwm_no, U16 val)
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == IDLE_TRUE )
		SETREG32 ( reg_con,1 << PWM_CON_IDLE_VALUE_OFFSET );
	else if ( val == IDLE_FALSE )
		CLRREG32 ( reg_con, 1 << PWM_CON_IDLE_VALUE_OFFSET );
	else 
		goto err;
	
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/*********************************************
* Set PWM_CON register guardvalue bit
*  val: 0 means guard state is not put out.
*        1 mens guard state is put out.
*
*    GUARD_FALSE: 0
*    GUARD_TRUE: 1
**********************************************/
S32 mt_set_pwm_con_guardval(U32 pwm_no, U16 val)
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == GUARD_TRUE )
		SETREG32 ( reg_con, 1 << PWM_CON_GUARD_VALUE_OFFSET );
	else if ( val == GUARD_FALSE )
		CLRREG32 ( reg_con, 1 << PWM_CON_GUARD_VALUE_OFFSET );
	else 
		goto err;
	
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/*************************************************
* Set PWM_CON register stopbits
*stop bits should be less then 0x3f
*
**************************************************/
S32 mt_set_pwm_con_stpbit(U32 pwm_no, U32 stpbit, U32 srcsel )
{
	U32 reg_con;
	
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if (srcsel == FIFO) {
		if ( stpbit > 0x3f ) {
			PWMDBG ( "stpbit execesses the most of 0x3f in fifo mode\n" );
			return -EPARMNOSUPPORT;
		}
	}else if (srcsel == MEMORY){
		if ( stpbit > 0x1f) {
			PWMDBG ("stpbit excesses the most of 0x1f in memory mode\n");
			return -EPARMNOSUPPORT;
		}
	}

	if ( srcsel == FIFO )
		MASKREG32 ( reg_con, PWM_CON_STOP_BITS_MASK, stpbit << PWM_CON_STOP_BITS_OFFSET);
	if ( srcsel == MEMORY )
		MASKREG32 ( reg_con, PWM_CON_STOP_BITS_MASK & (0x1f << PWM_CON_STOP_BITS_OFFSET), stpbit << PWM_CON_STOP_BITS_OFFSET);

	return RSUCCESS;
}

/*****************************************************
*Set PWM_CON register oldmode bit
* val: 0 means disable oldmode
*        1 means enable oldmode
*
*      OLDMODE_DISABLE: 0
*      OLDMODE_ENABLE: 1
******************************************************/

S32 mt_set_pwm_con_oldmode ( U32 pwm_no, U32 val )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == OLDMODE_DISABLE )
		CLRREG32 ( reg_con, 1 << PWM_CON_OLD_MODE_OFFSET );
	else if ( val == OLDMODE_ENABLE )
		SETREG32 ( reg_con, 1 << PWM_CON_OLD_MODE_OFFSET );
	else 
		goto err;
	
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/***********************************************************
* Set PWM_HIDURATION register
*
*************************************************************/

S32 mt_set_pwm_HiDur(U32 pwm_no, U16 DurVal)  //only low 16 bits are valid
{
	U32 reg_HiDur;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}
	
	reg_HiDur = PWM_register[pwm_no]+4*PWM_HDURATION;
	OUTREG32 ( reg_HiDur, DurVal);	
	return RSUCCESS;
}

/************************************************
* Set PWM Low Duration register
*************************************************/
S32 mt_set_pwm_LowDur (U32 pwm_no, U16 DurVal)
{
	U32 reg_LowDur;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	reg_LowDur = PWM_register[pwm_no] + 4*PWM_LDURATION;
	OUTREG32 ( reg_LowDur, DurVal );
	return RSUCCESS;
}

/***************************************************
* Set PWM_GUARDDURATION register
* pwm_no: PWM0~PWM6(0~6)
* DurVal:   the value of guard duration
****************************************************/
S32 mt_set_pwm_GuardDur ( U32 pwm_no, U16 DurVal )
{
	U32 reg_GuardDur;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	reg_GuardDur = PWM_register[pwm_no] + 4*PWM_GDURATION;
	OUTREG32 ( reg_GuardDur, DurVal );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_buf0_addr register
* pwm_no: pwm1~pwm7 (0~6)
* addr: data address
******************************************************/
S32 mt_set_pwm_buf0_addr (U32 pwm_no, U32 addr )
{
	U32 reg_buff0_addr;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff0_addr = PWM_register[pwm_no] + 4 * PWM_BUF0_BASE_ADDR;
	OUTREG32 ( reg_buff0_addr, addr );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_buf0_size register
* pwm_no: pwm1~pwm7 (0~6)
* size: size of data
******************************************************/
S32 mt_set_pwm_buf0_size ( U32 pwm_no, U16 size)
{
	U32 reg_buff0_size;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff0_size = PWM_register[pwm_no] + 4* PWM_BUF0_SIZE;
	OUTREG32 ( reg_buff0_size, size );
	return RSUCCESS;

}

/*****************************************************
* Set pwm_buf1_addr register
* pwm_no: pwm1~pwm7 (0~6)
* addr: data address
******************************************************/
S32 mt_set_pwm_buf1_addr (U32 pwm_no, U32 addr )
{
	U32 reg_buff1_addr;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff1_addr = PWM_register[pwm_no] + 4 * PWM_BUF1_BASE_ADDR;
	OUTREG32 ( reg_buff1_addr, addr );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_buf1_size register
* pwm_no: pwm1~pwm7 (0~6)
* size: size of data
******************************************************/
S32 mt_set_pwm_buf1_size ( U32 pwm_no, U16 size)
{
	U32 reg_buff1_size;
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff1_size = PWM_register[pwm_no] + 4* PWM_BUF1_SIZE;
	OUTREG32 ( reg_buff1_size, size );
	return RSUCCESS;

}

/*****************************************************
* Set pwm_send_data0 register
* pwm_no: pwm1~pwm7 (0~6)
* data: the data in the register
******************************************************/
S32 mt_set_pwm_send_data0 ( U32 pwm_no, U32 data )
{
	U32 reg_data0;
	
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_data0 = PWM_register[pwm_no] + 4 * PWM_SEND_DATA0;
	OUTREG32 ( reg_data0, data );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_send_data1 register
* pwm_no: pwm1~pwm7 (0~6)
* data: the data in the register
******************************************************/
S32 mt_set_pwm_send_data1 ( U32 pwm_no, U32 data )
{
	U32 reg_data1;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_data1 = PWM_register[pwm_no] + 4 * PWM_SEND_DATA1;
	OUTREG32 ( reg_data1, data );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_wave_num register
* pwm_no: pwm1~pwm7 (0~6)
* num:the wave number
******************************************************/
S32 mt_set_pwm_wave_num ( U32 pwm_no, U16 num )
{
	U32 reg_wave_num;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_wave_num = PWM_register[pwm_no] + 4 * PWM_WAVE_NUM;
	OUTREG32 ( reg_wave_num, num );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_data_width register. 
* This is only for old mode
* pwm_no: pwm1~pwm7 (0~6)
* width: set the guard value in the old mode
******************************************************/
S32 mt_set_pwm_data_width ( U32 pwm_no, U16 width )
{
	U32 reg_data_width;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_data_width = PWM_register[pwm_no] + 4 * PWM_DATA_WIDTH;
	OUTREG32 ( reg_data_width, width );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_thresh register
* pwm_no: pwm1~pwm7 (0~6)
* thresh:  the thresh of the wave
******************************************************/
S32 mt_set_pwm_thresh ( U32 pwm_no, U16 thresh )
{
	U32 reg_thresh;
	
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( " pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}

	reg_thresh = PWM_register[pwm_no] + 4 * PWM_THRESH;
	OUTREG32 ( reg_thresh, thresh );
	return RSUCCESS;
}

/*****************************************************
* Set pwm_send_wavenum register
* pwm_no: pwm1~pwm7 (0~6)
*
******************************************************/
S32 mt_get_pwm_send_wavenum ( U32 pwm_no )
{
	U32 reg_send_wavenum;
	S32 wave_num;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( pwm_no <= PWM3||pwm_no == PWM7)
		reg_send_wavenum = PWM_register[pwm_no] + 4 * PWM_SEND_WAVENUM;
	else
		reg_send_wavenum = PWM_register[pwm_no] + 4 * (PWM_SEND_WAVENUM - 2);  //pwm4,pwm5,pwm6 has no data width and thresh register
	wave_num = INREG32 ( reg_send_wavenum );

	return wave_num;
}

/*****************************************************
* Set pwm_send_data1 register
* pwm_no: pwm1~pwm7 (0~6)
* buf_valid_bit: 
* for buf0: bit0 and bit1 should be set 1. 
* for buf1: bit2 and bit3 should be set 1.
******************************************************/
S32 mt_set_pwm_valid ( U32 pwm_no, U32 buf_valid_bit )   //set 0  for BUF0 bit or set 1 for BUF1 bit
{
	U32 reg_valid;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( !buf_valid_bit>= BUF_EN_MAX) {
		PWMDBG ( "inavlid bit \n" );
		return -EPARMNOSUPPORT;
	}

	if (pwm_no <= PWM3 ||pwm_no == PWM7)
		reg_valid = PWM_register[pwm_no] + 4 * PWM_VALID;
	else
		reg_valid = PWM_register[pwm_no] + 4* (PWM_VALID -2);

	SETREG32 ( reg_valid, 1 << buf_valid_bit );

	return RSUCCESS;
}

/*************************************************
*  set PWM4_delay when using SEQ mode
*
**************************************************/
S32 mt_set_pwm_delay_duration(U32 pwm_delay_reg, U16 val)
{
	MASKREG32 ( pwm_delay_reg, PWM_DELAY_DURATION_MASK, val );
	return RSUCCESS;
}

/*******************************************************
* Set pwm delay clock
* 
*
********************************************************/
S32 mt_set_pwm_delay_clock (U32 pwm_delay_reg, U32 clksrc)
{
	MASKREG32 (pwm_delay_reg, PWM_DELAY_CLK_MASK, clksrc );
	return RSUCCESS;
}

/*******************************************
* Set intr enable register
* pwm_intr_enable_bit: the intr bit, 
*
*********************************************/
S32 mt_set_intr_enable(U32 pwm_intr_enable_bit)
{
	if (pwm_intr_enable_bit >= PWM_INT_ENABLE_BITS_MAX) {
		PWMDBG (" pwm inter enable bit is not right.\n"); 
		return -EEXCESSBITS; 
	}
	
	SETREG32 ( PWM_INT_ENABLE, 1 << pwm_intr_enable_bit );
	return RSUCCESS;
}

/*****************************************************
* Set intr status register
* pwm_no: pwm1~pwm7 (0~6)
* pwm_intr_status_bit
******************************************************/
S32 mt_get_intr_status(U32 pwm_intr_status_bit)
{
	int ret;

	if ( pwm_intr_status_bit >= PWM_INT_STATUS_BITS_MAX ) {
		PWMDBG ( "status bit excesses PWM_INT_STATUS_BITS_MAX\n" );
		return -EEXCESSBITS;
	}
	
	ret = INREG32 ( PWM_INT_STATUS );
	ret = ( ret >> pwm_intr_status_bit ) & 0x01;
	return ret;
}

/*****************************************************
* Set intr ack register
* pwm_no: pwm1~pwm7 (0~6)
* pwm_intr_ack_bit
******************************************************/
S32 mt_set_intr_ack ( U32 pwm_intr_ack_bit )
{
	if ( pwm_intr_ack_bit >= PWM_INT_ACK_BITS_MAX ) {
		PWMDBG ( "ack bit excesses PWM_INT_ACK_BITS_MAX\n" ); 
		return -EEXCESSBITS;
	}
	SETREG32 ( PWM_INT_ACK, 1 << pwm_intr_ack_bit );
	return RSUCCESS;
}

S32 pwm_set_easy_config ( struct pwm_easy_config *conf)
{

	U32 duty = 0;
	U16 duration = 0;
	U32 data_AllH=0xffffffff;
	U32 data0 = 0;
	U32 data1 = 0;
	
	if ( conf->pwm_no >= PWM_MAX || conf->pwm_no < PWM_MIN ) {
		PWMDBG("pwm number excess PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	if ((conf->clk_div >= CLK_DIV_MAX) || (conf->clk_div < CLK_DIV_MIN )) {
		PWMDBG ( "PWM clock division invalid\n" );
		return -EINVALID;
	}
	
	if ( ( conf ->clk_src >= PWM_CLK_SRC_INVALID) || (conf->clk_src < PWM_CLK_SRC_MIN) ) {
		PWMDBG ("PWM clock source invalid\n");
		return -EINVALID;
	}

	if  ( conf->duty < 0 ) {
		PWMDBG("duty parameter is invalid\n");
		return -EINVALID;
	}

	PWMDBG("pwm_set_easy_config\n");

	if ( conf->duty == 0 ) {
		mt_set_pwm_disable (conf->pwm_no);
		mt_power_off(conf->pwm_no);
		return RSUCCESS;
	}
	
	duty = conf->duty;
	duration = conf->duration;
	
	switch ( conf->clk_src ) {
		case PWM_CLK_OLD_MODE_BLOCK:
		case PWM_CLK_OLD_MODE_32K:
			if ( duration > 8191 || duration < 0 ) {
				PWMDBG ( "duration invalid parameter\n" );
				return -EPARMNOSUPPORT;
			}
			if ( conf->pwm_no == PWM4 || conf->pwm_no == PWM5 || conf->pwm_no == PWM6 ){
				PWMDBG ( "invalid parameters\n" );
				return -EPARMNOSUPPORT;
			}
			if ( duration < 10 ) 
				duration = 10;
			break;
			
		case PWM_CLK_NEW_MODE_BLOCK:
		case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
			if ( duration > 65535 || duration < 0 ){
				PWMDBG ("invalid paramters\n");
				return -EPARMNOSUPPORT;
			}
			break;
		default:
			PWMDBG("invalid clock source\n");
			return -EPARMNOSUPPORT;
	}
	
	if ( duty > 100 ) 
		duty = 100;

	if ( duty > 50 ){
		data0 = data_AllH;
		data1 = data_AllH >> ((PWM_NEW_MODE_DUTY_TOTAL_BITS * (100 - duty ))/100 );
	}else {
		data0 = data_AllH >> ((PWM_NEW_MODE_DUTY_TOTAL_BITS * (50 - duty))/100);
		PWMDBG("DATA0 :0x%x\n",data0);
		data1 = 0;
	}

	mt_power_on(conf->pwm_no);
	mt_set_pwm_con_guardval(conf->pwm_no, GUARD_TRUE);

	switch ( conf->clk_src ) {
		case PWM_CLK_OLD_MODE_32K:
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
			break;

		case PWM_CLK_OLD_MODE_BLOCK:
			mt_set_pwm_con_oldmode (conf->pwm_no, OLDMODE_ENABLE );
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK, conf->clk_div );
			break;

		case PWM_CLK_NEW_MODE_BLOCK:
			mt_set_pwm_con_oldmode (conf->pwm_no, OLDMODE_DISABLE );
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK , conf->clk_div );
			mt_set_pwm_con_datasrc( conf->pwm_no, FIFO);
			mt_set_pwm_con_stpbit ( conf->pwm_no, 0x3f, FIFO );
			break;

		case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
			mt_set_pwm_con_oldmode (conf->pwm_no,  OLDMODE_DISABLE );
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div );
			mt_set_pwm_con_datasrc( conf->pwm_no, FIFO);
			mt_set_pwm_con_stpbit ( conf->pwm_no, 0x3f, FIFO );
			break;

		default:
			break;
		}
	mt_set_pwm_HiDur ( conf->pwm_no, duration );
	mt_set_pwm_LowDur (conf->pwm_no, duration );
	mt_set_pwm_GuardDur (conf->pwm_no, 0 );
	mt_set_pwm_buf0_addr (conf->pwm_no, 0 );
	mt_set_pwm_buf0_size( conf->pwm_no, 0 );
	mt_set_pwm_buf1_addr (conf->pwm_no, 0 );
	mt_set_pwm_buf1_size (conf->pwm_no, 0 );
	mt_set_pwm_send_data0 (conf->pwm_no, data0 );
	mt_set_pwm_send_data1 (conf->pwm_no, data1 );
	mt_set_pwm_wave_num (conf->pwm_no, 0 );

	if ( conf->pwm_no <= PWM3||conf->pwm_no == PWM7)
	{
		mt_set_pwm_data_width (conf->pwm_no, duration );
		mt_set_pwm_thresh ( conf->pwm_no, (( duration * conf->duty)/100));
		mt_set_pwm_valid (conf->pwm_no, BUF0_EN_VALID );
		mt_set_pwm_valid ( conf->pwm_no, BUF1_EN_VALID );
		
	}

	mt_set_pwm_enable ( conf->pwm_no );
	PWMDBG("mt_set_pwm_enable\n");

	return RSUCCESS;
	
}

	
S32 pwm_set_spec_config(struct pwm_spec_config *conf)
{

	if ( conf->pwm_no >= PWM_MAX ) {
		PWMDBG("pwm number excess PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

       if ( ( conf->mode >= PWM_MODE_INVALID )||(conf->mode < PWM_MODE_MIN )) {
	   	PWMDBG ( "PWM mode invalid \n" );
		return -EINVALID;
       }

	if ( ( conf ->clk_src >= PWM_CLK_SRC_INVALID) || (conf->clk_src < PWM_CLK_SRC_MIN) ) {
		PWMDBG ("PWM clock source invalid\n");
		return -EINVALID;
	}

	if ((conf->clk_div >= CLK_DIV_MAX) || (conf->clk_div < CLK_DIV_MIN )) {
		PWMDBG ( "PWM clock division invalid\n" );
		return -EINVALID;
	}

	if ( (( conf->pwm_no == PWM4 || conf->pwm_no == PWM5||conf->pwm_no == PWM6 ) 
		&& conf->mode == PWM_MODE_OLD)  
		|| (conf->mode == PWM_MODE_OLD &&
			(conf->clk_src == PWM_CLK_NEW_MODE_BLOCK|| conf->clk_src == PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625)) 
		||(conf->mode != PWM_MODE_OLD &&
			(conf->clk_src == PWM_CLK_OLD_MODE_32K || conf->clk_src == PWM_CLK_OLD_MODE_BLOCK)) ) {

		PWMDBG ( "parameters match error\n" );
		return -ERROR;
	}

	mt_power_on(conf->pwm_no);

	switch (conf->mode ) {
		case PWM_MODE_OLD:
			PWMDBG("PWM_MODE_OLD\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_OLD_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->PWM_MODE_OLD_REGS.GUARD_VALUE);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->PWM_MODE_OLD_REGS.GDURATION);
			mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_OLD_REGS.WAVE_NUM);
			mt_set_pwm_data_width(conf->pwm_no, conf->PWM_MODE_OLD_REGS.DATA_WIDTH);
			mt_set_pwm_thresh(conf->pwm_no, conf->PWM_MODE_OLD_REGS.THRESH);
			PWMDBG ("PWM set old mode finish\n");
			break;
		case PWM_MODE_FIFO:
			PWMDBG("PWM_MODE_FIFO\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_con_datasrc(conf->pwm_no, FIFO);
			mt_set_pwm_con_mode (conf->pwm_no, PERIOD);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->PWM_MODE_FIFO_REGS.GUARD_VALUE);
			mt_set_pwm_HiDur (conf->pwm_no, conf->PWM_MODE_FIFO_REGS.HDURATION);
			mt_set_pwm_LowDur (conf->pwm_no, conf->PWM_MODE_FIFO_REGS.LDURATION);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->PWM_MODE_FIFO_REGS.GDURATION);
			mt_set_pwm_send_data0 (conf->pwm_no, conf->PWM_MODE_FIFO_REGS.SEND_DATA0);
			mt_set_pwm_send_data1 (conf->pwm_no, conf->PWM_MODE_FIFO_REGS.SEND_DATA1);
			mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.WAVE_NUM);
			mt_set_pwm_con_stpbit(conf->pwm_no, conf->PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE,FIFO);
			break;
		case PWM_MODE_MEMORY:
			PWMDBG("PWM_MODE_MEMORY\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_con_datasrc(conf->pwm_no, MEMORY);
			mt_set_pwm_con_mode (conf->pwm_no, PERIOD);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.GUARD_VALUE);
			mt_set_pwm_HiDur (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.HDURATION);
			mt_set_pwm_LowDur (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.LDURATION);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.GDURATION);
			mt_set_pwm_buf0_addr(conf->pwm_no, (U32)conf->PWM_MODE_MEMORY_REGS.BUF0_BASE_ADDR);
			mt_set_pwm_buf0_size (conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.BUF0_SIZE);
			mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.WAVE_NUM);
			mt_set_pwm_con_stpbit(conf->pwm_no, conf->PWM_MODE_MEMORY_REGS.STOP_BITPOS_VALUE,MEMORY);
			
			break;
		case PWM_MODE_RANDOM:
			PWMDBG("PWM_MODE_RANDOM\n");
			mt_set_pwm_disable(conf->pwm_no);
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_con_datasrc(conf->pwm_no, MEMORY);
			mt_set_pwm_con_mode (conf->pwm_no, RAND);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.GUARD_VALUE);
			mt_set_pwm_HiDur (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.HDURATION);
			mt_set_pwm_LowDur (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.LDURATION);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.GDURATION);
			mt_set_pwm_buf0_addr(conf->pwm_no, (U32 )conf->PWM_MODE_RANDOM_REGS.BUF0_BASE_ADDR);
			mt_set_pwm_buf0_size (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.BUF0_SIZE);
			mt_set_pwm_buf1_addr(conf->pwm_no, (U32 )conf->PWM_MODE_RANDOM_REGS.BUF1_BASE_ADDR);
			mt_set_pwm_buf1_size (conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.BUF1_SIZE);
			mt_set_pwm_wave_num(conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.WAVE_NUM);
			mt_set_pwm_con_stpbit(conf->pwm_no, conf->PWM_MODE_RANDOM_REGS.STOP_BITPOS_VALUE, MEMORY);
			mt_set_pwm_valid(conf->pwm_no, BUF0_EN_VALID);
			mt_set_pwm_valid(conf->pwm_no, BUF1_EN_VALID);
			break;

		case PWM_MODE_DELAY:
			PWMDBG("PWM_MODE_DELAY\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_enable_seqmode();
			mt_set_pwm_disable(PWM3);
			mt_set_pwm_disable(PWM4);
			mt_set_pwm_disable(PWM5);
			mt_set_pwm_disable(PWM6);
			if ( conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR <0 ||conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR >= (1<<17) ||
				conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR < 0|| conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR >= (1<<17) ||
				conf->PWM_MODE_DELAY_REGS.PWM6_DELAY_DUR <0 || conf->PWM_MODE_DELAY_REGS.PWM6_DELAY_DUR >=(1<<17) ) {
				PWMDBG("Delay value invalid\n");
				return -EINVALID;
			}
			mt_set_pwm_delay_duration(PWM4_DELAY, conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR );
			mt_set_pwm_delay_clock(PWM4_DELAY, conf->PWM_MODE_DELAY_REGS.PWM4_DELAY_CLK);
			mt_set_pwm_delay_duration(PWM5_DELAY, conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR);
			mt_set_pwm_delay_clock(PWM5_DELAY, conf->PWM_MODE_DELAY_REGS.PWM5_DELAY_CLK);
			mt_set_pwm_delay_duration(PWM6_DELAY, conf->PWM_MODE_DELAY_REGS.PWM6_DELAY_DUR);
			mt_set_pwm_delay_clock(PWM6_DELAY, conf->PWM_MODE_DELAY_REGS.PWM6_DELAY_CLK);
			
			mt_set_pwm_enable(PWM3);
			mt_set_pwm_enable(PWM4);
			mt_set_pwm_enable(PWM5);
			mt_set_pwm_enable(PWM6);
			break;
			
		default:
			break;
		}

	switch (conf->clk_src) {
		case PWM_CLK_OLD_MODE_BLOCK:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK, conf->clk_div);
			PWMDBG("Enable oldmode and set clock block\n");
			break;
		case PWM_CLK_OLD_MODE_32K:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
			PWMDBG("Enable oldmode and set clock 32K\n");
			break;
		case PWM_CLK_NEW_MODE_BLOCK:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK, conf->clk_div);
			PWMDBG("Enable newmode and set clock block\n");
			break;
		case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
			PWMDBG("Enable newmode and set clock 32K\n");
			break;
		default:
			break;
		} 

	mt_set_pwm_enable(conf->pwm_no); 
	PWMDBG("mt_set_pwm_enable\n");

	return RSUCCESS;
	
}	


void mt_pwm_dump_regs(void)
{
	int i;
	U32 reg_val;
	reg_val = INREG32(PWM_ENABLE);
	PWMMSG("\r\n[PWM_ENABLE is:%x]\n\r ", reg_val );
	for ( i = PWM1;  i <= PWM7; i ++ ) 
	{
		reg_val = INREG32(PWM_register[i] + 4* PWM_CON);
		PWMMSG("\r\n[PWM%d_CON is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_HDURATION);
		PWMMSG("\r\n[PWM%d_HDURATION is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_LDURATION);
		PWMMSG("\r\n[PWM%d_LDURATION is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_GDURATION);
		PWMMSG("\r\n[PWM%d_HDURATION is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_BUF0_BASE_ADDR);
		PWMMSG("\r\n[PWM%d_BUF0_BASE_ADDR is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_BUF0_SIZE);
		PWMMSG("\r\n[PWM%d_BUF0_SIZE is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_BUF1_BASE_ADDR);
		PWMMSG("\r\n[PWM%d_BUF1_BASE_ADDR is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_BUF1_SIZE);
		PWMMSG("\r\n[PWM%d_BUF1_SIZE is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_SEND_DATA0);
		PWMMSG("\r\n[PWM%d_SEND_DATA0 is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_SEND_DATA1);
		PWMMSG("\r\n[PWM%d_PWM_SEND_DATA1 is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_WAVE_NUM);
		PWMMSG("\r\n[PWM%d_WAVE_NUM is:%x]\r\n", i+1, reg_val);
		if ( i <= PWM3||i==PWM7) {
			reg_val=INREG32(PWM_register[i]+4* PWM_DATA_WIDTH);
			PWMMSG("\r\n[PWM%d_WIDTH is:%x]\r\n",i+1,reg_val);
			reg_val=INREG32(PWM_register[i]+4* PWM_THRESH);
			PWMMSG("\r\n[PWM%d_THRESH is:%x]\r\n",i+1,reg_val);
			reg_val=INREG32(PWM_register[i]+4* PWM_SEND_WAVENUM);
			PWMMSG("\r\n[PWM%d_SEND_WAVENUM is:%x]\r\n",i+1,reg_val);
						reg_val=INREG32(PWM_register[i]+4* PWM_VALID);
			PWMMSG("\r\n[PWM%d_SEND_VALID is:%x]\r\n",i+1,reg_val);
		}else {
			reg_val=INREG32(PWM_register[i]+4* (PWM_SEND_WAVENUM-2));
			PWMMSG("\r\n[PWM%d_SEND_WAVENUM is:%x]\r\n",i+1,reg_val);
			reg_val=INREG32(PWM_register[i]+4* (PWM_VALID-2));
			PWMMSG("\r\n[PWM%d_SEND_VALID is:%x]\r\n",i+1,reg_val);
		}
	}
}



