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
 *   mt6575.c
 *
 * Project:
 * --------
 *   MT6575
 *
 * Description:
 * ------------
 *   This file implements the MT6575 U-Boot board.
 *
 * Author:
 * -------
 *   Hong-Rong Hsu (mtk02678)
 *
 ****************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/mt65xx.h>
#include <asm/mach-types.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/boot_mode.h>
#include <config.h>

DECLARE_GLOBAL_DATA_PTR;
extern int Uboot_power_saving(void);
extern void cpu_clean_env (void);

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*
 * system_init: do the early system initialization.
 */
void system_init(void)
{
/* CC: remove temporarily for MT6573 porting */
#if 0
    mt6573_timer_init();
#endif

    /* enabling audio depop takes time,
     * so init it here rather than linux kernel
     */

    delay(2000);
/* CC: remove temporarily for MT6573 porting */
#if 0
    mt6573_aud_init();

 //   led_init();
#endif
#ifdef CFG_MT6589_FPGA
    mtk_timer_init();
#endif
    return;
}

#if 0
/*
 * cleanup_before_linx: Prepares the processor for linux
 */
int cleanup_before_linux (void)
{
	printf("system cleanup before entering linux ...\n");

	// (1) make sure the cache is off (turn off I/D-cache)
	// (2) make sure the cache is clean (flush I/D-cache)
	printf(" > clean cpu env\n");
	cpu_clean_env();

/* CC: remove temporarily for MT6573 porting */
#if 1
	// (3) deinit leds
	printf(" > deinit leds..\n");	
	leds_deinit();
#endif
	
	// (4) power off some unused LDO
	printf(" > perform power saving\n");		
	Uboot_power_saving();

	return (0);
}
#endif

/**********************************************************
 * Routine: mt6573_sw_env
 * Description: this function is called to init mt6516 sw enviroment
 * Notice: DO NOT this function or system might crash
 **********************************************************/
int mt65xx_sw_env (void)
{  
#ifdef CFG_RECOVERY_MODE
  if(g_boot_mode != META_BOOT && g_boot_mode != FACTORY_BOOT && recovery_detection() == TRUE)
  {    
      
  }
#endif   

    //**************************************
    //* CHECK BOOT MODE
    //**************************************
    #ifndef USER_BUILD
    switch(g_boot_mode)
    {        
         case META_BOOT :
	         video_printf(" => META MODE\n");
	         break;
	     case FACTORY_BOOT :
	         video_printf(" => FACTORY MODE\n");
	         break;
	     case RECOVERY_BOOT :
	         video_printf(" => RECOVERY MODE\n");
	         break;
	     case SW_REBOOT :
	         //video_printf(" => SW RESET\n");
	         break;
	     case NORMAL_BOOT :
	         video_printf(" => NORMAL BOOT\n");
	         break;
         case ADVMETA_BOOT:
            video_printf(" => ADVANCED META MODE\n");
            break;
		   case ATE_FACTORY_BOOT:
            video_printf(" => ATE FACTORY MODE\n");
		        break;     
	     default :
                 video_printf(" => UNKNOWN BOOT\n");
    }
    return 0;
#endif    
}


/* Transparent to DRAM customize */
static int g_nr_bank;
BOOT_ARGUMENT *g_boot_arg;
#include <memory_info.h> // for dram customize
int dram_init(void)
{  
  int i, index, num_record;
  BOOT_ARGUMENT *boot_arg;
  unsigned int dram_rank_num;
  unsigned int dram_rank_size;
  
  g_boot_arg = (volatile BOOT_ARGUMENT *)(BOOT_ARGUMENT_LOCATION);
  boot_arg = (volatile BOOT_ARGUMENT *)(BOOT_ARGUMENT_LOCATION);

#ifdef CFG_MT6589_FPGA
  dram_rank_num = 1;
  g_nr_bank = dram_rank_num;
  dram_rank_size = 0xCB00000;
  gd->bd->bi_dram[0].start = 0x81600000;
  gd->bd->bi_dram[0].size =  dram_rank_size;
#else
  dram_rank_num = boot_arg->dram_rank_num;
  dram_rank_size = boot_arg->dram_rank_size;
  g_nr_bank = dram_rank_num;


  if (g_nr_bank == 1){
    gd->bd->bi_dram[0].start = RIL_SIZE;
    gd->bd->bi_dram[0].size =  dram_rank_size - RIL_SIZE;
  }
  else if (g_nr_bank == 2){
    gd->bd->bi_dram[0].start = RIL_SIZE;
    gd->bd->bi_dram[0].size =  dram_rank_size - RIL_SIZE;
    gd->bd->bi_dram[1].start = SZ_256M;      
    gd->bd->bi_dram[1].size =  dram_rank_size;
  }
  else if (g_nr_bank == 3){
    gd->bd->bi_dram[0].start = RIL_SIZE;
    gd->bd->bi_dram[0].size =  dram_rank_size - RIL_SIZE;
    gd->bd->bi_dram[1].start = SZ_256M;
    gd->bd->bi_dram[1].size =  dram_rank_size;
    gd->bd->bi_dram[2].start = SZ_512M;      
    gd->bd->bi_dram[2].size =  dram_rank_size;
  }
  else if (g_nr_bank == 4){
    gd->bd->bi_dram[0].start = RIL_SIZE;
    gd->bd->bi_dram[0].size =  dram_rank_size - RIL_SIZE;
    gd->bd->bi_dram[1].start = SZ_256M;      
    gd->bd->bi_dram[1].size =  dram_rank_size;
    gd->bd->bi_dram[2].start = SZ_512M;      
    gd->bd->bi_dram[2].size =  dram_rank_size;
    gd->bd->bi_dram[3].start = SZ_512M + SZ_256M;      
    gd->bd->bi_dram[3].size =  dram_rank_size;
  }
  else{
    //ERROR! DRAM bank number is not correct
  }
#endif

  return 0;
}

int get_bank_nr(void)
{
  return g_nr_bank;
}

/*******************************************************
 * Routine: memory_size
 * Description: return DRAM size to LCM driver
 ******************************************************/
u32 memory_size(void)
{
	// each bank mapping to 256 mb physical address
	int nr_bank = g_nr_bank;
    int size;
    if(nr_bank == 1)
    {
	  size = 256 * 1024 * 1024 * (nr_bank - 1) + gd->bd->bi_dram[nr_bank-1].size + RIL_SIZE;
    }
    else
    {
      size = 256 * 1024 * 1024 * (nr_bank - 1) + gd->bd->bi_dram[nr_bank-1].size;
    }
	return size;
}
