/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "platform.h"
#include "mtk_key.h"
#include "mtk_pmic_6320.h"
#include <gpio.h>

#define GPIO_DIN_BASE	(GPIO_BASE + 0x0a00)
extern pmic_detect_powerkey(void);
void mtk_kpd_gpio_set(void);
extern U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 get_pmic6320_chip_version (void);

void mtk_kpd_gpios_get(unsigned int ROW_REG[], unsigned int COL_REG[])
{
	int i;
	for(i = 0; i< 8; i++)
	{
		ROW_REG[i] = 0;
		COL_REG[i] = 0;
	}
	#ifdef GPIO_KPD_KROW0_PIN
		ROW_REG[0] = GPIO_KPD_KROW0_PIN;
	#endif

	#ifdef GPIO_KPD_KROW1_PIN
		ROW_REG[1] = GPIO_KPD_KROW1_PIN;
	#endif

	#ifdef GPIO_KPD_KROW2_PIN
		ROW_REG[2] = GPIO_KPD_KROW2_PIN;
	#endif

	#ifdef GPIO_KPD_KROW3_PIN
		ROW_REG[3] = GPIO_KPD_KROW3_PIN;
	#endif

	#ifdef GPIO_KPD_KROW4_PIN
		ROW_REG[4] = GPIO_KPD_KROW4_PIN;
	#endif

	#ifdef GPIO_KPD_KROW5_PIN
		ROW_REG[5] = GPIO_KPD_KROW5_PIN;
	#endif

	#ifdef GPIO_KPD_KROW6_PIN
		ROW_REG[6] = GPIO_KPD_KROW6_PIN;
	#endif

	#ifdef GPIO_KPD_KROW7_PIN
		ROW_REG[7] = GPIO_KPD_KROW7_PIN;
	#endif


	#ifdef GPIO_KPD_KCOL0_PIN
		COL_REG[0] = GPIO_KPD_KCOL0_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL1_PIN
		COL_REG[1] = GPIO_KPD_KCOL1_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL2_PIN
		COL_REG[2] = GPIO_KPD_KCOL2_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL3_PIN
		COL_REG[3] = GPIO_KPD_KCOL3_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL4_PIN
		COL_REG[4] = GPIO_KPD_KCOL4_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL5_PIN
		COL_REG[5] = GPIO_KPD_KCOL5_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL6_PIN
		COL_REG[6] = GPIO_KPD_KCOL6_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL7_PIN
		COL_REG[7] = GPIO_KPD_KCOL7_PIN;
	#endif
}

void mtk_kpd_gpio_set(void)
{
	unsigned int ROW_REG[8];
	unsigned int COL_REG[8];
	int i;

	//print("Enter mtk_kpd_gpio_set! \n");
	mtk_kpd_gpios_get(ROW_REG, COL_REG);
	
	//print("kpd debug column : %d, %d, %d, %d, %d, %d, %d, %d\n",COL_REG[0],COL_REG[1],COL_REG[2],COL_REG[3],COL_REG[4],COL_REG[5],COL_REG[6],COL_REG[7]);
	//print("kpd debug row : %d, %d, %d, %d, %d, %d, %d, %d\n",ROW_REG[0],ROW_REG[1],ROW_REG[2],ROW_REG[3],ROW_REG[4],ROW_REG[5],ROW_REG[6],ROW_REG[7]);
	
	for(i = 0; i < 8; i++)
	{
		if (COL_REG[i] != 0)
		{
			/* KCOL: GPIO INPUT + PULL ENABLE + PULL UP */
			mt_set_gpio_mode(COL_REG[i], 1);
			mt_set_gpio_dir(COL_REG[i], 0);
			mt_set_gpio_pull_enable(COL_REG[i], 1);
			mt_set_gpio_pull_select(COL_REG[i], 1);
		}
		
		if(ROW_REG[i] != 0)
		{
			/* KROW: GPIO output + pull disable + pull down */
			mt_set_gpio_mode(ROW_REG[i], 1);
			mt_set_gpio_dir(ROW_REG[i], 1);
			mt_set_gpio_pull_enable(ROW_REG[i], 0);	
			mt_set_gpio_pull_select(ROW_REG[i], 0);
		}
	}
	mdelay(33);
}

void set_kpd_pmic_mode()
{
	unsigned int a,c;
	a = pwrap_read(0x0502,&c);
	if(a != 0)
 	print("kpd write fail, addr: 0x0502\n");
 	
	//print("kpd read addr: 0x0502: data:0x%x\n", c);
	c=c&0xFFFE;
	a = pwrap_write(0x0502,c);
	if(a != 0)
 	print("kpd write fail, addr: 0x0502\n");

	mtk_kpd_gpio_set();

	int rel = 0;
	rel = pmic_config_interface(WRP_CKPDN,0x0,PMIC_RG_WRP_KP_PDN_MASK, PMIC_RG_WRP_KP_PDN_SHIFT);
	if(rel !=  0){
		print("kpd set clock register fail!\n");
	}

	*(volatile u16 *)(KP_PMIC) = 0x1;
	print("kpd register for pmic set!\n");
	return;
}

void disable_PMIC_kpd_clock()
{
	int rel = 0;
	//print("kpd disable_PMIC_kpd_clock register!\n");
	rel = pmic_config_interface(WRP_CKPDN,0x1, PMIC_RG_WRP_32K_PDN_MASK, PMIC_RG_WRP_32K_PDN_SHIFT);
	if(rel !=  0){
		print("kpd disable_PMIC_kpd_clock register fail!\n");
	}
}
void enable_PMIC_kpd_clock()
{
	int rel = 0;
	//print("kpd enable_PMIC_kpd_clock register!\n");
	rel = pmic_config_interface(WRP_CKPDN,0x0, PMIC_RG_WRP_32K_PDN_MASK, PMIC_RG_WRP_32K_PDN_SHIFT);
	if(rel !=  0){
		print("kpd enable_PMIC_kpd_clock register fail!\n");
	}
}

bool mtk_detect_key (unsigned short key)  /* key: HW keycode */
{
    unsigned short idx, bit, din;
    U32 just_rst;

    if (key >= KPD_NUM_KEYS)
        return false;

    if (key % 9 == 8)
        key = 8;
	
#if 0 //KPD_PWRKEY_USE_EINT 
    if (key == 8)
    {                         /* Power key */
        idx = KPD_PWRKEY_EINT_GPIO / 16;
        bit = KPD_PWRKEY_EINT_GPIO % 16;

        din = DRV_Reg16 (GPIO_DIN_BASE + (idx << 4)) & (1U << bit);
        din >>= bit;
        if (din == KPD_PWRKEY_GPIO_DIN)
        {
            print ("power key is pressed\n");
            return true;
        }
        return false;
    }
#else // check by PMIC

	if (key == 8)
		{	/* Power key */	
			#if 0	//for long press reboot not boot up from a reset
			pmic_read_interface(0x050C, &just_rst, 0x01, 14);
			if(just_rst)
			{
				pmic_config_interface(0x050C, 0x01, 0x01, 4);
				print("Just recover from a reset\n"); 
				return false;
			}
			#endif
			if (1 == pmic_detect_powerkey())
			{
				print ("power key is pressed\n");
				return true;
			}
			return false;
		}

#endif

    idx = key / 16;
    bit = key % 16;

#if 0 //debug code for using do not remove them
unsigned int a, b, c, j;
unsigned int addr1[]= {0x8404, 0x8408, 0x840C, 0x8410, 0x8414};
unsigned int addr2[]= {KP_MEM1, KP_MEM2, KP_MEM3, KP_MEM4, KP_MEM5};

for(j = 0; j < 5; j++) {
 a = pwrap_read(addr1[j],&c);
 if(a != 0)
 	print("kpd read fail, addr: 0x%x\n", addr1[j]);
 print("kpd read addr: 0x%x: data:0x%x\n",addr1[j], c);
}

for(j = 0; j < 5; j++) {
 a = *(volatile u16 *)(addr2[j]);
 print("kpd read addr: 0x%x: data:0x%x\n",addr2[j], a);
}
#endif	


    din = DRV_Reg16 (KP_MEM1 + (idx << 2)) & (1U << bit);
    if (!din)
    {
	    #if 0 //debug code for using do not remove them
		unsigned int a, b, c, j;
		unsigned int addr1[]= {0x8404, 0x8408, 0x840C, 0x8410, 0x8414};
		unsigned int addr2[]= {KP_MEM1, KP_MEM2, KP_MEM3, KP_MEM4, KP_MEM5};

		for(j = 0; j < 5; j++) {
		 a = pwrap_read(addr1[j],&c);
			 if(a != 0)
	 	print("kpd read fail, addr: 0x%x\n", addr1[j]);
	 	print("kpd read addr: 0x%x: data:0x%x\n",addr1[j], c);
		}

		for(j = 0; j < 5; j++) {
	 	a = *(volatile u16 *)(addr2[j]);
	 	print("kpd read addr: 0x%x: data:0x%x\n",addr2[j], a);
		}
		#endif
        print("key %d is pressed\n", key);
        return true;
    }
    return false;
}

bool mtk_detect_dl_keys (void)
{
#if 1
    if (mtk_detect_key (KPD_DL_KEY1) &&
        mtk_detect_key (KPD_DL_KEY2) && mtk_detect_key (KPD_DL_KEY3))
    {
        print("download keys are pressed\n");
        return true;
    }
#endif
    return false;
}

