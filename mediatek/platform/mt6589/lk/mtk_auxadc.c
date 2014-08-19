/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *    mt6589_auxadc.c
 *
 * Project:
 * --------
 *   Android_Software
 *
 * Description:
 * ------------
 *   This Module defines functions of mt6573 AUXADC
 *
 * Author:
 * -------
 * James Lo
 *
 ****************************************************************************/
//#include <common.h>
//#include <asm/io.h>

#include <platform/mtk_auxadc_sw.h>
#include <platform/mtk_auxadc_hw.h>

///////////////////////////////////////////////////////////////////////////////////////////
//// Define
static int adc_auto_set =0;
//use efuse cali
static int g_adc_ge = 0;
static int g_adc_oe = 0;
static int g_o_vts = 0;
static int g_o_vbg = 0;
static int g_degc_cali = 0;
static int g_adc_cali_en = 0;
static int g_o_vts_abb = 0;
static int g_o_slope = 0;
static int g_o_slope_sign = 0;
static int g_id = 0;
static int g_y_vbg = 0;//defaul 1967 fi cali_en=0


typedef unsigned short  kal_uint16;

#define DRV_Reg(addr)               (*(volatile kal_uint16 *)(addr))
#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))

#define DRV_ClearBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp &=~(data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp |= (data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetData(addr, bitmask, value)     {\
   kal_uint16 temp;\
   temp = (~(bitmask)) & DRV_Reg(addr);\
   temp |= (value);\
   DRV_WriteReg(addr,temp);\
}

#define AUXADC_DRV_ClearBits16(addr, data)           DRV_ClearBits(addr,data)
#define AUXADC_DRV_SetBits16(addr, data)             DRV_SetBits(addr,data)
#define AUXADC_DRV_WriteReg16(addr, data)            DRV_WriteReg(addr, data)
#define AUXADC_DRV_ReadReg16(addr)                   DRV_Reg(addr)
#define AUXADC_DRV_SetData16(addr, bitmask, value)   DRV_SetData(addr, bitmask, value)

#define AUXADC_DVT_DELAYMACRO(u4Num)                                     \
{                                                                        \
    unsigned int u4Count = 0 ;                                                 \
    for (u4Count = 0; u4Count < u4Num; u4Count++ );                      \
}

#define AUXADC_SET_BITS(BS,REG)       ((*(volatile unsigned int*)(REG)) |= (unsigned int)(BS))
#define AUXADC_CLR_BITS(BS,REG)       ((*(volatile unsigned int*)(REG)) &= ~((unsigned int)(BS)))

#define VOLTAGE_FULL_RANGE 	2500 // VA voltage
#define ADC_PRECISE 		4096 // 12 bits

static void auxadc_cal_prepare(void);

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//// Internal API
inline static void mt65xx_ADC_2G_power_up(void)
{
    //2010/07/27: mt6573, the ADC 2G power on is controlled by APMCU_CG_CLR0
    //#define PDN_CLR0 (0xF7026308)  
    #define PDN_CLR0 (0x70026308)  
    unsigned int poweron = 1 << 4;
    AUXADC_SET_BITS(poweron, PDN_CLR0);
}

inline static void mt65xx_ADC_2G_power_down(void)
{
    //2010/07/27: mt6573, the ADC 2G power on is controlled by APMCU_CG_SET0
    //#define PDN_SET0 (0xF7026304)  
    #define PDN_SET0 (0x70026304)  
    unsigned int poweroff = 1 << 4;
    AUXADC_SET_BITS(poweroff, PDN_SET0);
}

inline static void mt65xx_ADC_3G_power_up(void)
{
    //2010/07/27: mt6573, the ADC 3G power on is controlled by APMCU_CG_CLR0
    //#define PDN_CLR0 (0xF7026308)  
    #define PDN_CLR0 (0x70026308)  
    unsigned int poweron = 1 << 13;
    AUXADC_SET_BITS(poweron, PDN_CLR0);
}

inline static void mt65xx_ADC_3G_power_down(void)
{
    //2010/07/27: mt6573, the ADC 3G power on is controlled by APMCU_CG_SET0
    //#define PDN_SET0 (0xF7026304)  
    #define PDN_SET0 (0x70026304)  
    unsigned int poweroff = 1 << 13;
    AUXADC_SET_BITS(poweroff, PDN_SET0);
}
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//// Common API

int auxadc_test(void ) 
{
   int i = 0, data[4] = {0,0,0,0};
   int res =0;
   int rawdata=0;
   int Voltiage_cali =0;
   
      auxadc_cal_prepare();
      for (i = 0; i < 16; i++) 
      {
        //printk("[adc_driver]: i=%d\n",i);
       
		res = IMM_GetOneChannelValue(i,data,&rawdata);
		if(res < 0)
		{ 
			   printf("[adc_uboot]: get data error\n");
			   break;
			   
		}
		else
		{
		       printf("[adc_uboot]: channel[%d]raw =%d\n",i,rawdata);
		       printf("[adc_uboot]: channel[%d]=%d.%.02d \n",i,data[0],data[1]);
			  
		}
		res= IMM_GetOneChannelValue_Cali(i, &Voltiage_cali);
		if(res < 0)
		{ 
			   printf("[adc_driver]: get cali voltage error\n");
			   break;
			   
		}
		else
		{
		       printf("[adc_driver]: channel[%d] cali_voltage =%d\n",i,Voltiage_cali);
  
		}	
      } 


   return 0;
}


//step1 check con3 if auxadc is busy
//step2 clear bit
//step3  read channle and make sure old ready bit ==0
//step4 set bit  to trigger sample
//step5  read channle and make sure  ready bit ==1
//step6 read data

int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata)
{
   unsigned int channel[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int idle_count =0;
   int data_ready_count=0;
   
  // mutex_lock(&mutex_get_cali_value);
   /* in uboot no pms api
    if(enable_clock(MT65XX_PDN_PERI_AUXADC,"AUXADC"))
   {
	    //printk("hwEnableClock AUXADC !!!.");
	    if(enable_clock(MT65XX_PDN_PERI_AUXADC,"AUXADC"))
	    {printk("hwEnableClock AUXADC failed.");}
        
   }
	*/
   //step1 check con3 if auxadc is busy
   while ((*(volatile unsigned short *)AUXADC_CON2) & 0x01) 
   {
       printf("[adc_api]: wait for module idle\n");
       udelay(100000);
	   idle_count++;
	   if(idle_count>30)
	   {
	      //wait for idle time out
	      printf("[adc_api]: wait for aux/adc idle time out\n");
	      return -1;
	   }
   } 
   // step2 clear bit
   if(0 == adc_auto_set)
   {
	   //clear bit
	   *(volatile unsigned short *)AUXADC_CON1 = *(volatile unsigned short *)AUXADC_CON1 & (~(1 << dwChannel));
   }
   

   //step3  read channle and make sure old ready bit ==0
   while ((*(volatile unsigned short *)(AUXADC_DAT0 + dwChannel * 0x04)) & (1<<12)) 
   {
       printf("[adc_api]: wait for channel[%d] ready bit clear\n",dwChannel);
       udelay(10000);
	   data_ready_count++;
	   if(data_ready_count>30)
	   {
	      //wait for idle time out
	      printf("[adc_api]: wait for channel[%d] ready bit clear time out\n",dwChannel);
	      return -2;
	   }
   }
  
   //step4 set bit  to trigger sample
   if(0==adc_auto_set)
   {  
	  *(volatile unsigned short *)AUXADC_CON1 = *(volatile unsigned short *)AUXADC_CON1 | (1 << dwChannel);
   }
   //step5  read channle and make sure  ready bit ==1
   udelay(1000);//we must dealay here for hw sample cahnnel data
   while (0==((*(volatile unsigned short *)(AUXADC_DAT0 + dwChannel * 0x04)) & (1<<12))) 
   {
       printf("[adc_api]: wait for channel[%d] ready bit ==1\n",dwChannel);
       udelay(10000);
	 data_ready_count++;
	 if(data_ready_count>30)
	 {
	      //wait for idle time out
	      printf("[adc_api]: wait for channel[%d] data ready time out\n",dwChannel);
	      return -3;
	 }
   }
   ////step6 read data
   
   channel[dwChannel] = (*(volatile unsigned short *)(AUXADC_DAT0 + dwChannel * 0x04)) & 0x0FFF;
   if(rawdata)
   {
      *rawdata = channel[dwChannel];
   }
   //printk("[adc_api: imm mode raw data => channel[%d] = %d\n",dwChannel, channel[dwChannel]);
   //printk("[adc_api]: imm mode => channel[%d] = %d.%d\n", dwChannel, (channel[dwChannel] * 250 / 4096 / 100), ((channel[dwChannel] * 250 / 4096) % 100));
   data[0] = (channel[dwChannel] * 150 / 4096 / 100);
   data[1] = ((channel[dwChannel] * 150 / 4096) % 100);

   /*

   if(disable_clock(MT65XX_PDN_PERI_AUXADC,"AUXADC"))
   {
        printk("hwEnableClock AUXADC failed.");
   }
    mutex_unlock(&mutex_get_cali_value);
   */
   return 0;
   
}

// 1v == 1000000 uv
// this function voltage Unit is uv
int IMM_GetOneChannelValue_Cali(int Channel, int*voltage)
{
     int ret = 0, data[4], rawvalue;
     long a =0;
     // long b =0;
     long slop = 0;
     long offset =0;
     
     ret = IMM_GetOneChannelValue( Channel,  data, &rawvalue);
     if(ret)
     {
         ret = IMM_GetOneChannelValue( Channel,  data, &rawvalue);
	   if(ret)
	   {
	        printf("[adc_api]:IMM_GetOneChannelValue_Cali  get raw value error %d \n",ret);
		  return -1;
	   }
     }

     a = (1000000+g_adc_ge)*(g_y_vbg-g_adc_oe)/((g_o_vbg+1800)-g_adc_oe);
     // b = g_adc_oe;
     slop = ((1500000/4096)*1000000)/a;
     offset = 1500000*g_adc_oe/a; 

     *voltage = rawvalue*slop + offset;
	     
      //printk("[adc_api]:IMM_GetOneChannelValue_Cali  voltage= %d uv \n",*voltage);

      return 0;
     
}


static int IMM_auxadc_get_evrage_data(int times, int Channel)
{
	int ret = 0, data[4], i, ret_value = 0, ret_temp = 0;

	i = times;
	while (i--)
	{
		ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
		ret += ret_temp;
		printf("[auxadc_get_data(channel%d)]: ret_temp=%d\n",Channel,ret_temp);        
//		msleep(10);
	}

	ret = ret / times;
	return ret;
}


static void auxadc_cal_prepare(void)
{
	int temp = 0;
	
	temp = __raw_readl(0xC1019048);
	g_adc_ge = (temp & 0x000000FF);
	printf("[auxadc]lk temp = 0x%x, g_adc_ge = 0x%x\n", temp, g_adc_ge);
	
	temp = __raw_readl(0xC1019044);
	g_adc_oe = (temp & 0x000000FF);
	printf("[auxadc]lk temp = 0x%x, g_adc_oe = 0x%x\n", temp, g_adc_oe);
	
	temp = __raw_readl(0xC1019040);
	//g_o_vts_abb   = ((temp & 0xFC000000) >> 26);
	g_o_vts       = ((temp & 0x03FE0000) >> 17);
	g_o_vbg       = ((temp & 0x0001FF00) >> 8);
	//g_degc_cali   = ((temp & 0x000000FE) >> 1);
	g_degc_cali   = ((temp & 0x0000007E) >> 1);
	g_adc_cali_en = ((temp & 0x00000001) >> 0);

	g_o_slope     = ((temp & 0xFC000000) >> 26);
	g_o_slope_sign= ((temp & 0x00000080) >> 7);    

      //get y_vbg
      *(volatile unsigned int *)(0xc000780c) = (*(volatile unsigned short *)(0xc000780c) | 0x0100); 
	printf("fwq \n ");
      *(volatile unsigned int *)(0xc0007804) = 0x0002;
      //DRV_WriteReg32(0xf0007804, 0x0002);//TS_CON1
      *(volatile unsigned int *)(0xc0007808) = 0x0200;
	//DRV_WriteReg32(0xf0007808, 0x0200);//TS_CON2
	udelay(10000);
	g_y_vbg = IMM_auxadc_get_evrage_data(20,5);

	//temp = DRV_Reg32(0xF1019100);
	temp = *(volatile unsigned int *)(0xC1019100);
	g_id = ((temp & 0x80000000) >> 31);

	if(g_id==0)
	{
		g_o_slope = 0;
	}
	
	if(g_adc_cali_en == 1)
	{
		//get y_vbg      
	}
	else
	{
		g_adc_ge = 128;
		g_adc_oe = 128;
		g_o_vts = 292;
		g_o_vbg = 167;
		g_degc_cali = 40;
		g_o_slope = 0;
		g_o_slope_sign = 0;
		g_y_vbg = 1967;
	}
	
	printf("[auxadc]temp = 0x%x, g_y_vbg=%d, g_o_vts = 0x%x, g_o_vbg = 0x%x, g_degc_cali = 0x%x, g_adc_cali_en = 0x%x, g_o_vts_abb = 0x%x, g_o_slope = 0x%x, g_o_slope_sign = 0x%x, g_id = 0x%x\n", 
		temp, g_y_vbg, g_o_vts, g_o_vbg, g_degc_cali, g_adc_cali_en, g_o_vts_abb, g_o_slope, g_o_slope_sign, g_id);

}


///////////////////////////////////////////////////////////////////////////////////////////

