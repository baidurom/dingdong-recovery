/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/******************************************************************************
 * pmic_wrapper.c - MT6589 Linux pmic_wrapper Driver
 *
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 *
 * DESCRIPTION:
 *     This file provid the other drivers PMIC relative functions
 *
 ******************************************************************************/



//#include <mach/mt_reg_base.h>
//#include <mach/mt_typedefs.h>

#include <asm/arch/mtk_timer.h> //build error
#include <asm/arch/mt65xx_typedefs.h>
#include "asm/arch/mt_pmic_wrap_uboot.h"

//#include "reg_pmic.h"
//#include "reg_pmic_wrap.h"
//#include "mt_pmic_wrap.h"

/******************************************************************************
 wrapper timeout
******************************************************************************/
#define PWRAP_TIMEOUT
//use the same API name with kernel driver
//however,the timeout API in uboot use tick instead of ns
#ifdef PWRAP_TIMEOUT
static U64 _pwrap_get_current_time(void)
{
  return gpt4_get_current_tick();
}
//_pwrap_timeout_tick,use the same API name with kernel driver
static bool _pwrap_timeout_ns (U64 start_time, U64 elapse_time)
{
  return gpt4_timeout_tick(start_time, elapse_time);
}
//_pwrap_time2tick_us
static U64 _pwrap_time2ns (U64 time_us)
{
  return gpt4_time2tick_us(time_us);
}

#else
static U64 _pwrap_get_current_time(void)
{
  return 0;
}
static bool _pwrap_timeout_ns (U64 start_time, U64 elapse_time)//,U64 timeout_ns)
{
  return FALSE;
}
static U64 _pwrap_time2ns (U64 time_us)
{
  return 0;
}

#endif

//#####################################################################
//define macro and inline function (for do while loop)
//#####################################################################
typedef U32 (*loop_condition_fp)(U32);//define a function pointer

static inline U32 wait_for_fsm_idle(U32 x)
{
  return (GET_WACS0_FSM( x ) != WACS_FSM_IDLE );
}
static inline U32 wait_for_fsm_vldclr(U32 x)
{
  return (GET_WACS0_FSM( x ) != WACS_FSM_WFVLDCLR);
}
static inline U32 wait_for_sync(U32 x)
{
  return (GET_SYNC_IDLE0(x) != WACS_SYNC_IDLE);
}
static inline U32 wait_for_idle_and_sync(U32 x)
{
  return ((GET_WACS0_FSM(x) != WACS_FSM_IDLE) || (GET_SYNC_IDLE0(x) != WACS_SYNC_IDLE)) ;
}
static inline U32 wait_for_wrap_idle(U32 x)
{
  return ((GET_WRAP_FSM(x) != 0x0) || (GET_WRAP_CH_DLE_RESTCNT(x) != 0x0));
}
static inline U32 wait_for_wrap_sta_idle(U32 x)
{
  return ( GET_WRAP_AG_DLE_RESTCNT( x ) != 0 ) ;
}
static inline U32 wait_for_man_idle_and_noreq(U32 x)
{
  return ( (GET_MAN_REQ(x) != MAN_FSM_NO_REQ ) || (GET_MAN_FSM(x) != MAN_FSM_IDLE) );
}
static inline U32 wait_for_man_vldclr(U32 x)
{
  return  (GET_MAN_FSM( x ) != MAN_FSM_WFVLDCLR) ;
}
static inline U32 wait_for_cipher_ready(U32 x)
{
  return (x!=1) ;
}
static inline U32 wait_for_stdupd_idle(U32 x)
{
  return ( GET_STAUPD_FSM(x) != 0x0) ;
}


static inline U32 wait_for_state_ready_init(loop_condition_fp fp,U32 timeout_us,U32 wacs_register,U32 *read_reg)
{

  U32 start_time_ns=0, timeout_ns=0;
  U32 reg_rdata=0x0;
  //U32 rdata=0;
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(timeout_us);
  do
  {
    reg_rdata = WRAP_RD32(wacs_register);

    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("timeout when waiting for idle\n");
      return E_PWR_TIMEOUT;
    }
  } while( fp(reg_rdata)); //IDLE State
  if(read_reg)
   *read_reg=reg_rdata;
  return 0;
}


static inline U32 wait_for_state_ready(loop_condition_fp fp,U32 timeout_us,U32 wacs_register,U32 *read_reg)
{

  U32 start_time_ns=0, timeout_ns=0;
  U32 reg_rdata;
  //struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(timeout_us);
  do
  {
    reg_rdata = WRAP_RD32(wacs_register);

    if( GET_INIT_DONE0( reg_rdata ) != WACS_INIT_DONE)
    {
      PWRAPERR("initialization isn't finished \n");
      return E_PWR_NOT_INIT_DONE;
    }
    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("timeout when waiting for idle\n");
      return E_PWR_WAIT_IDLE_TIMEOUT;
    }
  } while( fp(reg_rdata)); //IDLE State
  if(read_reg)
   *read_reg=reg_rdata;
  return 0;
}
//#####################################################################
S32 pwrap_read( U32  adr, U32 *rdata )
{
	return pwrap_wacs2( 0, adr,0,rdata );
}

S32 pwrap_write( U32  adr, U32  wdata )
{
	return pwrap_wacs2( 1, adr,wdata,0 );
}
//--------------------------------------------------------
//    Function : pwrap_wacs2()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 pwrap_wacs2( U32  write, U32  adr, U32  wdata, U32 *rdata )
{
  U64 wrap_access_time=0x0;
  U32 res=0;
  U32 reg_rdata=0;
  U32 wacs_write=0;
  U32 wacs_adr=0;
  U32 wacs_cmd=0;
  U32 return_value=0;
  unsigned long flags=0;
  //struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
  //if (!pwrap_obj)
  //      PWRAPERR("NULL pointer\n");
  PWRAPFUC();
  //PWRAPLOG("wrapper access,write=%x,add=%x,wdata=%x,rdata=%x\n",write,adr,wdata,rdata);

  // Check argument validation
  if( (write & ~(0x1))    != 0)  return E_PWR_INVALID_RW;
  if( (adr   & ~(0xffff)) != 0)  return E_PWR_INVALID_ADDR;
  if( (wdata & ~(0xffff)) != 0)  return E_PWR_INVALID_WDAT;

  //spin_lock_irqsave(&pwrap_obj->spin_lock,flags);
  // Check IDLE & INIT_DONE in advance
  return_value=wait_for_state_ready(wait_for_fsm_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n",return_value);
    goto FAIL;
  }
  wacs_write  = write << 31;
  wacs_adr    = (adr >> 1) << 16;
  wacs_cmd= wacs_write | wacs_adr | wdata;

  WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);
  if( write == 0 )
  {
    if (NULL == rdata)
    {
      PWRAPERR("rdata is a NULL pointer\n");
      return_value= E_PWR_INVALID_ARG;
      goto FAIL;
      }
    return_value=wait_for_state_ready(wait_for_fsm_vldclr,TIMEOUT_READ,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
    if(return_value!=0)
      {
      PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n",return_value);
		return_value+=1;//E_PWR_NOT_INIT_DONE_READ or E_PWR_WAIT_IDLE_TIMEOUT_READ
        goto FAIL;
      }
    *rdata = GET_WACS0_RDATA( reg_rdata );
    WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
}
FAIL:
  //spin_unlock_irqrestore(&pwrap_obj->spin_lock,flags);
  wrap_access_time=_pwrap_get_current_time();
  //pwrap_trace(wrap_access_time,return_value,write, adr, wdata,rdata);
  return return_value;
}
//#############################################################################################
//#define PWRAP_EARLY_PORTING
#ifdef CFG_MT6589_FPGA
static S32 pwrap_read_nochk( U32  adr, U32 *rdata )
{
	return _pwrap_wacs2_nochk( 0, adr,  0, rdata );
}

static S32 pwrap_write_nochk( U32  adr, U32  wdata )
{
	return _pwrap_wacs2_nochk( 1, adr,wdata,0 );
}
//--------------------------------------------------------
//    Function : _pwrap_wacs2_nochk()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_wacs2_nochk( U32 write, U32 adr, U32 wdata, U32 *rdata )
{
  U32 reg_rdata=0x0;
  U32 wacs_write=0x0;
  U32 wacs_adr=0x0;
  U32 wacs_cmd=0x0;
  U32 return_value=0x0;
  //struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
  //PWRAPFUC();
  // Check argument validation
  if( (write & ~(0x1))    != 0)  return E_PWR_INVALID_RW;
  if( (adr   & ~(0xffff)) != 0)  return E_PWR_INVALID_ADDR;
  if( (wdata & ~(0xffff)) != 0)  return E_PWR_INVALID_WDAT;

  // Check IDLE
  return_value=wait_for_state_ready_init(wait_for_fsm_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    if (NULL == rdata)
      return E_PWR_INVALID_ARG;
    PWRAPERR("_pwrap_wacs2_nochk write command fail,return_value=%x\n", return_value);
    return return_value;
  }

  wacs_write  = write << 31;
  wacs_adr    = (adr >> 1) << 16;
  wacs_cmd = wacs_write | wacs_adr | wdata;
  WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);

  if( write == 0 )
  {

    // wait for read data ready
    return_value=wait_for_state_ready_init(wait_for_fsm_vldclr,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
    if(return_value!=0)
    {
      PWRAPERR("_pwrap_wacs2_nochk read fail,return_value=%x\n", return_value);
      return return_value;
    }
    *rdata = GET_WACS0_RDATA( reg_rdata );
    WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
  }
  return 0;
}
//--------------------------------------------------------
//    Function : _pwrap_init_dio()
// Description :call it in pwrap_init,mustn't check init done
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_dio( U32 dio_en )
{
  U32 arb_en_backup=0x0;
  U32 rdata=0x0;
  U32 return_value=0;
  //struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;

  //PWRAPFUC();
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS2
  pwrap_write_nochk(DEW_DIO_EN, dio_en);

  // Check IDLE & INIT_DONE in advance
  return_value=wait_for_state_ready_init(wait_for_idle_and_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("_pwrap_init_dio fail,return_value=%x\n", return_value);
    return return_value;
  }
  WRAP_WR32(PMIC_WRAP_DIO_EN , dio_en);
  // Read Test
  pwrap_read_nochk(DEW_READ_TEST,&rdata);
  if( rdata != DEFAULT_VALUE_READ_TEST )
  {
    PWRAPERR("[Dio_mode][Read Test] fail,dio_en = %x, READ_TEST rdata=%x, exp=0x5aa5\n", dio_en, rdata);
    return E_PWR_READ_TEST_FAIL;
  }
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_cipher()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_cipher( void )
{
  U32 arb_en_backup=0;
  U32 rdata=0;
  U32 cipher_rdy=0;
  U32 return_value=0;
  //struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
  U32 start_time_ns=0, timeout_ns=0;
  //PWRAPFUC();
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS0

  WRAP_WR32(PMIC_WRAP_CIPHER_SWRST , 1);
  WRAP_WR32(PMIC_WRAP_CIPHER_SWRST , 0);
  WRAP_WR32(PMIC_WRAP_CIPHER_KEY_SEL , 1);
  WRAP_WR32(PMIC_WRAP_CIPHER_IV_SEL  , 2);
  WRAP_WR32(PMIC_WRAP_CIPHER_LOAD    , 1);
  WRAP_WR32(PMIC_WRAP_CIPHER_START   , 1);

  //Config CIPHER @ PMIC
  pwrap_write_nochk(DEW_CIPHER_SWRST,   0x1);
  pwrap_write_nochk(DEW_CIPHER_SWRST,   0x0);
  pwrap_write_nochk(DEW_CIPHER_KEY_SEL, 0x1);
  pwrap_write_nochk(DEW_CIPHER_IV_SEL,  0x2);
  pwrap_write_nochk(DEW_CIPHER_LOAD,    0x1);
  pwrap_write_nochk(DEW_CIPHER_START,   0x1);

  //wait for cipher data ready@AP
  return_value=wait_for_state_ready_init(wait_for_cipher_ready,TIMEOUT_WAIT_IDLE,PMIC_WRAP_CIPHER_RDY,0);
  if(return_value!=0)
  {
    PWRAPERR("wait for cipher data ready@AP fail,return_value=%x\n", return_value);
    return return_value;
  }

  //wait for cipher data ready@PMIC
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(TIMEOUT_WAIT_IDLE);
  do
  {
    pwrap_read_nochk(DEW_CIPHER_RDY,&rdata);
    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("timeout when waiting for idle\n");
      return E_PWR_WAIT_IDLE_TIMEOUT;
    }
  } while( rdata != 0x1 ); //cipher_ready

  pwrap_write_nochk(DEW_CIPHER_MODE, 0x1);
  //wait for cipher mode idle
  return_value=wait_for_state_ready_init(wait_for_idle_and_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("wait for cipher mode idle fail,return_value=%x\n", return_value);
    return return_value;
  }
  WRAP_WR32(PMIC_WRAP_CIPHER_MODE , 1);

  // Read Test
  pwrap_read_nochk(DEW_READ_TEST,&rdata);
  if( rdata != DEFAULT_VALUE_READ_TEST )
  {
    PWRAPERR("_pwrap_init_cipher,read test error,error code=%x, rdata=%x\n", 1, rdata);
    return E_PWR_READ_TEST_FAIL;
  }

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_sidly()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_sidly( void )
{
  U32 arb_en_backup=0;
  U32 rdata=0;
  U32 ind=0;
  U32 result=0;
  U32 result_faulty=0;
  //PWRAPFUC();
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS2

  // Scan all SIDLY by Read Test
  result = 0;
  for( ind=0 ; ind<4 ; ind++)
  {
    WRAP_WR32(PMIC_WRAP_SIDLY , ind);
    pwrap_read_nochk(DEW_READ_TEST,&rdata);
    if( rdata == DEFAULT_VALUE_READ_TEST )
    {
      //PWRAPLOG("_pwrap_init_sidly [Read Test] pass,SIDLY=%x\n", ind);
      result |= (0x1 << ind);
    }
    else
      PWRAPLOG("_pwrap_init_sidly [Read Test] fail,SIDLY=%x\n", ind);
  }

  // Config SIDLY according to result
  switch( result )
  {
    // Only 1 pass, choose it
    case 0x1:
      WRAP_WR32(PMIC_WRAP_SIDLY , 0);
      break;
    case 0x2:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1);
      break;
    case 0x4:
      WRAP_WR32(PMIC_WRAP_SIDLY , 2);
      break;
    case 0x8:
      WRAP_WR32(PMIC_WRAP_SIDLY , 3);
      break;

    // two pass, choose the one on SIDLY boundary
    case 0x3:
      WRAP_WR32(PMIC_WRAP_SIDLY , 0);
      break;
    case 0x6:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1); //no boundary, choose smaller one
      break;
    case 0xc:
      WRAP_WR32(PMIC_WRAP_SIDLY , 3);
      break;

    // three pass, choose the middle one
    case 0x7:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1);
      break;
    case 0xe:
      WRAP_WR32(PMIC_WRAP_SIDLY , 2);
      break;
    // four pass, choose the smaller middle one
    case 0xf:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1);
      break;

    // pass range not continuous, should not happen
    default:
      WRAP_WR32(PMIC_WRAP_SIDLY , 0);
      result_faulty = 0x1;
      break;
  }

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  if( result_faulty == 0 )
    return 0;
  else
  {
    PWRAPERR("error,_pwrap_init_sidly fail,result=%x\n",result);
    return result_faulty;
  }
}

//--------------------------------------------------------
//    Function : _pwrap_reset_spislv()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_reset_spislv( void )
{
  U32 rdata=0;
  U32 ret=0;
  U32 return_value=0;
  //struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
  //PWRAPFUC();
  // This driver does not using _pwrap_switch_mux
  // because the remaining requests are expected to fail anyway

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0);
  WRAP_WR32(PMIC_WRAP_WRAP_EN , 0);
  WRAP_WR32(PMIC_WRAP_MUX_SEL , 1);
  WRAP_WR32(PMIC_WRAP_MAN_EN ,1);
  WRAP_WR32(PMIC_WRAP_DIO_EN , 0);

  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_CSL  << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8)); //to reset counter
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_CSH  << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));

  return_value=wait_for_state_ready_init(wait_for_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("_pwrap_reset_spislv fail,return_value=%x\n", return_value);
    ret=E_PWR_TIMEOUT;
    goto timeout;
  }

  WRAP_WR32(PMIC_WRAP_MAN_EN , 0);
  WRAP_WR32(PMIC_WRAP_MUX_SEL , 0);

timeout:
  WRAP_WR32(PMIC_WRAP_MAN_EN , 0);
  WRAP_WR32(PMIC_WRAP_MUX_SEL , 0);
  return ret;
}

static S32 _pwrap_init_reg_clock( U32 regck_sel )
{
  U32 wdata=0;
  U32 rdata=0;

  // Set reg clk freq
  pwrap_read_nochk(PMIC_TOP_CKCON2,&rdata);

  if( regck_sel == 1 )
    wdata = rdata | 0x8000;
  else
    wdata = rdata & ~(0x8000);

  pwrap_write_nochk(PMIC_TOP_CKCON2, wdata);
  pwrap_read_nochk(PMIC_TOP_CKCON2, &rdata);
  if( rdata != wdata ) {
    PWRAPERR("_pwrap_init_reg_clock,PMIC_TOP_CKCON2 Write [15]=1 Fail, rdata=%x\n",rdata);
    return E_PWR_WRITE_TEST_FAIL;
  }

  // Config SPI Waveform according to reg clk
  if( regck_sel == 1 ) { //18MHz
    WRAP_WR32(PMIC_WRAP_CSHEXT, 0xc);
    WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x4);
    WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0xc);
    WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0x0);
    WRAP_WR32(PMIC_WRAP_CSLEXT_END     , 0x0);
  } else if( regck_sel == 2 ){ //36MHz
    WRAP_WR32(PMIC_WRAP_CSHEXT         , 0x4);
    WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x0);
    WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0x4);
    WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0x0);
    WRAP_WR32(PMIC_WRAP_CSLEXT_END     , 0x0);
  } else { //Safe mode
    WRAP_WR32(PMIC_WRAP_CSHEXT         , 0xf);
    WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0xf);
    WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0xf);
    WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0xf);
    WRAP_WR32(PMIC_WRAP_CSLEXT_END     , 0xf);
  }

  return 0;
}


static S32 pwrap_init ( void )
{
  S32 sub_return=0;
  S32 sub_return1=0;
  S32 ret=0;
  U32 rdata=0x0;
  //U32 timeout=0;
  PWRAPFUC();
  //###############################
  //TBD: toggle PMIC_WRAP reset
  //###############################
   WRAP_SET_BIT(0x80,INFRA_GLOBALCON_RST0);
   WRAP_CLR_BIT(0x80,INFRA_GLOBALCON_RST0);

  //###############################
  //toggle PERI_PWRAP_BRIDGE reset
  //###############################
  WRAP_SET_BIT(0x04,PERI_GLOBALCON_RST1);
  WRAP_CLR_BIT(0x04,PERI_GLOBALCON_RST1);

  //###############################
  //Enable DCM
  //###############################
   WRAP_WR32(PMIC_WRAP_DCM_EN , 1);
   WRAP_WR32(PMIC_WRAP_DCM_DBC_PRD ,0);

  //###############################
  //Reset SPISLV
  //###############################
  sub_return=_pwrap_reset_spislv();
  if( sub_return != 0 )
  {
    PWRAPERR("error,_pwrap_reset_spislv fail,sub_return=%x\n",sub_return);
    return E_PWR_INIT_RESET_SPI;
  }
  //###############################
  // Enable WACS2
  //###############################
  WRAP_WR32(PMIC_WRAP_WRAP_EN,1);//enable wrap
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,8); //Only WACS2
  WRAP_WR32(PMIC_WRAP_WACS2_EN,1);

  //###############################
  //TBD: Set SPI_CK freq = 69MHz
  //###############################
  WRAP_WR32(CLK_CFG_8, 1);

  //###############################
  // SIDLY setting
  //###############################
  sub_return = _pwrap_init_sidly();
  //sub_return = 0; //TBD
  if( sub_return != 0 )
  {
    PWRAPERR("error,_pwrap_init_sidly fail,sub_return=%x\n",sub_return);
    return E_PWR_INIT_SIDLY;
  }
  //###############################
  // SPI Waveform Configuration
  //###############################
  sub_return = _pwrap_init_reg_clock(2); //0:safe mode, 1:18MHz, 2:36MHz //2
  if( sub_return != 0)  {
    PWRAPERR("error,_pwrap_init_reg_clock fail,sub_return=%x\n",sub_return);
    return E_PWR_INIT_REG_CLOCK;
  }

  //###############################
  // Enable PMIC
  // (May not be necessary, depending on S/W partition)
  //###############################
  sub_return= pwrap_write_nochk(PMIC_WRP_CKPDN,   0);//set dewrap clock bit
  sub_return1=pwrap_write_nochk(PMIC_WRP_RST_CON, 0);//clear dewrap reset bit
  if(( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPERR("Enable PMIC fail, sub_return=%x sub_return1=%x\n", sub_return,sub_return1);
    return E_PWR_INIT_ENABLE_PMIC;
  }
  //###############################
  // Enable DIO mode
  //###############################
  sub_return = _pwrap_init_dio(1);
  if( sub_return != 0 )
  {
    PWRAPERR("_pwrap_init_dio test error,error code=%x, sub_return=%x\n", 0x11, sub_return);
    return E_PWR_INIT_DIO;
  }

  //###############################
  // Enable Encryption
  //###############################
  sub_return = _pwrap_init_cipher();
  if( sub_return != 0 )
  {
    PWRAPERR("Enable Encryption fail, return=%x\n", 0x21, sub_return);
    return E_PWR_INIT_CIPHER;
  }

  //###############################
  // Write test using WACS2
  //###############################
  sub_return = pwrap_write_nochk(DEW_WRITE_TEST, WRITE_TEST_VALUE);
  sub_return1 = pwrap_read_nochk(DEW_WRITE_TEST,&rdata);
  if(( rdata != WRITE_TEST_VALUE )||( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPERR("read test error,rdata=%x,exp=0xa55a,sub_return=%,sub_return1=%\n", rdata,sub_return,sub_return1);
    return E_PWR_INIT_WRITE_TEST;
  }

  //###############################
  // Signature Checking - Using Write Test Register
  // should be the last to modify WRITE_TEST
  //###############################
  #if 0 //old mode
  _pwrap_wacs2_nochk(1, DEW_WRITE_TEST, 0x5678, &rdata);
  WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
  WRAP_WR32(PMIC_WRAP_SIG_VALUE,0x5678);
  WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x1);
  #endif

  //###############################
  // Signature Checking - Using CRC
  // should be the last to modify WRITE_TEST
  //###############################
  sub_return=pwrap_write_nochk(DEW_CRC_EN, 0x1);
  if( sub_return != 0 )
  {
    PWRAPERR("enable CRC fail,sub_return=%x\n", sub_return);
    return E_PWR_INIT_ENABLE_CRC;
  }
  WRAP_WR32(PMIC_WRAP_CRC_EN ,0x1);
  WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x0);
  WRAP_WR32(PMIC_WRAP_SIG_ADR , DEW_CRC_VAL);


  //###############################
  // PMIC_WRAP enables
  //###############################
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0x1ff);
  WRAP_WR32(PMIC_WRAP_RRARB_EN ,0x7);
  WRAP_WR32(PMIC_WRAP_WACS0_EN,0x1);
  WRAP_WR32(PMIC_WRAP_WACS1_EN,0x1);
  WRAP_WR32(PMIC_WRAP_WACS2_EN,0x1);//already enabled
  WRAP_WR32(PMIC_WRAP_EVENT_IN_EN,0x1);
  WRAP_WR32(PMIC_WRAP_EVENT_DST_EN,0xffff);
  WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x5);  //0x1:20us,for concurrence test,MP:0x5;  //100us
  WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN,0xff);
  WRAP_WR32(PMIC_WRAP_WDT_UNIT,0xf);
  WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,0xffffffff);
  WRAP_WR32(PMIC_WRAP_TIMER_EN,0x1);
  WRAP_WR32(PMIC_WRAP_INT_EN,0x7fffffff); //except for [31] debug_int

  //###############################
  // PERI_PWRAP_BRIDGE enables
  //###############################
  WRAP_WR32(PERI_PWRAP_BRIDGE_IORD_ARB_EN, 0x7f);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WACS3_EN , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WACS4_EN ,0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_UNIT , 0xf);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_SRC_EN , 0xffff);
  WRAP_WR32(PERI_PWRAP_BRIDGE_TIMER_EN  , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_INT_EN    , 0xfff);

  //###############################
  // PMIC_DEWRAP enables
  //###############################
  sub_return  = pwrap_write_nochk(DEW_EVENT_OUT_EN, 0x1);
  sub_return1 = pwrap_write_nochk(DEW_EVENT_SRC_EN, 0xffff);
  if(( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPERR("enable dewrap fail,sub_return=%,sub_return1=%\n", sub_return,sub_return1);
    return E_PWR_INIT_ENABLE_DEWRAP;
  }
  //###############################
  // Initialization Done
  //###############################
  WRAP_WR32(PMIC_WRAP_INIT_DONE2 , 0x1);

  //###############################
  //TBD: Should be configured by MD MCU
  //###############################
  #ifdef CONFIG_MTK_LDVT_PMIC_WRAP
    WRAP_WR32(PMIC_WRAP_INIT_DONE0 ,1);
    WRAP_WR32(PMIC_WRAP_INIT_DONE1 , 1);
    WRAP_WR32(PERI_PWRAP_BRIDGE_INIT_DONE3 , 1);
    WRAP_WR32(PERI_PWRAP_BRIDGE_INIT_DONE4 , 1);
  #endif
  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_event_Test()
// Description :only for early porting
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_event_test_porting( void )
{
  U32 i, j;
  U32 rdata;
  U32 sub_return;
  U32 ret=0;
  PWRAPFUC();
  WRAP_WR32(PMIC_WRAP_EVENT_STACLR , 0xffff);

  sub_return=pwrap_wacs2(1, DEW_EVENT_TEST, 0x1, &rdata);
  if( sub_return != 0 )
  {  /* TERR="Error: [DrvPWRAP_EventTest][DrvPWRAP_WACS2] fail, return=%x, exp=0x0", sub_return*/
     PWRAPERR("event test error,return=%x, exp=0x0", sub_return);
     return 0x02;
  }
  //pwrap_delay_us(10);////delay at least 2us
  for(i=0;i++;i<100);
  rdata = WRAP_RD32(PMIC_WRAP_EVENT_STA);
  if( (rdata & 0x8000) != 0x8000 )
  {
    /* TERR="Error: [_pwrap_init_cipher][Read Test] fail, READ_TEST rdata=%x, exp=0x5aa5", rdata */
    PWRAPERR("event test error,ret=%x, rdata=%x", ret, rdata);
    return 1;//DEBUG_CTP
   }
  WRAP_WR32(PMIC_WRAP_EVENT_STACLR , 0xffff);
  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_status_update_test()
// Description :only for early porting
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_status_update_test_porting( void )
{
  U32 i, j;
  U32 rdata;
  volatile U32 delay=1000*1000*1;
  PWRAPFUC();
  //disable signature interrupt
  WRAP_WR32(PMIC_WRAP_INT_EN,0x0);
  pwrap_wacs2(1, DEW_WRITE_TEST, 0x55AA, &rdata);
  WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
  WRAP_WR32(PMIC_WRAP_SIG_VALUE,0xAA55);
  WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x1);

  //pwrap_delay_us(5000);//delay 5 seconds

  while(delay--);

  rdata=WRAP_RD32(PMIC_WRAP_SIG_ERRVAL);
  if( rdata != 0x55AA )
  {
    PWRAPERR("_pwrap_status_update_test error,error code=%x, rdata=%x\n", 1, rdata);
    //return 1;
  }
  WRAP_WR32(PMIC_WRAP_SIG_VALUE,0x55AA);//tha same as write test
  //clear sig_error interrupt flag bit
  WRAP_WR32(PMIC_WRAP_INT_CLR,1<<1);

  //enable signature interrupt
  WRAP_WR32(PMIC_WRAP_INT_EN,0x7fffffff);
#if 0
  WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x0);
  WRAP_WR32(PMIC_WRAP_SIG_ADR , DEW_CRC_VAL);
#endif
  return 0;
}

int  pwrap_init_for_early_porting(void)
{
    int ret = 0;
    U32 res=0;
  PWRAPFUC();
  ret=pwrap_init();
    if(ret==0)
    {
      PWRAPLOG("wrap_init test pass.\n");
    }
    else
    {
      PWRAPLOG("error:wrap_init test fail.\n");
    res+=1;
    }
  ret=_pwrap_event_test_porting();
    if(ret==0)
    {
      PWRAPLOG("wrapper EventTest pass.\n");
    }
    else
    {
      PWRAPLOG("error:wrapper_EventTest fail.\n");
    res+=1;
    }
  ret=_pwrap_status_update_test_porting();
    if(ret==0)
    {
      PWRAPLOG("wrapper_StatusUpdateTest pass.\n");
    }
    else
    {
      PWRAPLOG("error:wrapper_StatusUpdateTest fail.\n");
    res+=1;
    }

}
#endif //PWRAP_EARLY_PORTING
