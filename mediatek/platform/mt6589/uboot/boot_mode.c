/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   boot_mode.c
 *
 * Project:
 * --------
 *   YuSu 
 *
 * Description:
 * ------------
 *   This file implements MT6575 boot mode.
 *
 * Author:
 * -------
 *   CC Hwang (mtk00702)
 *   Hong-Rong Hsu (mtk02678)
 *
 ****************************************************************************/
#include <common.h>
#include <command.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/boot_mode.h>
#include <asm/mach-types.h>
#include <asm/errno.h>
#include <asm/byteorder.h>



// global variable for specifying boot mode (default = NORMAL)
BOOTMODE g_boot_mode = NORMAL_BOOT;

BOOL meta_mode_check(void)
{	
	//printf(" > check meta mode to bypass power key\n");
	if(g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT)
	{	
	  return TRUE;
	}
	else
	{	return FALSE;
	}
}


// check the boot mode : (1) meta mode or (2) recovery mode ...
void boot_mode_select(void)
{	
    #ifdef CFG_META_MODE
    if (meta_detection())
    {	
      return;
    }
    #endif   
    #ifdef CFG_FACTORY_MODE
    if (factory_detection())
    {	
      return;
    }
    #endif    
    #ifdef CFG_RECOVERY_MODE
    if(recovery_detection())
    {	
      //**************************************
  		//* CHECK IMAGE
  		//**************************************
  		if(DRV_Reg32(0x40002300)==0xE92D4800)
  		{	
  		  printf(" > do recovery_check\n");
  			//jump(0x40002300); 
  		}
  		else
  		{	
  		  printf(" > bypass recovery_check\n");
  		}
    	return;
    }
    #endif   
}

unsigned int get_chip_sw_ver_code(void)
{  
    return DRV_Reg32(APSW_VER);
}

CHIP_SW_VER mt_get_chip_sw_ver(void)
{
    return (CHIP_SW_VER)get_chip_sw_ver_code();
}
