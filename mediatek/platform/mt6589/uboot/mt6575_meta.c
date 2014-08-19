/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <video.h>
#include <asm/io.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_meta.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mtk_uart.h>
#include <asm/arch/mtk_serial.h>
#include <asm/arch/boot_mode.h>

#ifdef CFG_META_MODE

/**************************************************************************
 *  CONSTANT DEFINITION
 **************************************************************************/
#ifndef CONFIG_CFB_CONSOLE
	#define video_printf 	
#endif

#define META_STR_READY         "READY"         /* Ready Signal          */
#define	META_STR_REQ           "METAMETA"      /* PC META Request      */
#define META_STR_ACK  	       "ATEMATEM"      /* TARGET META Ack Response */
#define META_STR_MOD_PREF      "[META]"        /* Log prefix of meta module */
#define META_LOCK              "LOCK"          /* Meta lock */

#define META_ADV_REQ       "ADVMETA"
#define META_ADV_ACK       "ATEMVDA"

#define META_SZ_MAX_PBUF       20			   /* max protocol buffer size */

#define ATE_STR_REQ           "FACTORYM"      /* PC ATE Request      */
#define ATE_STR_ACK  	        "MYROTCAF"      /* TARGET ATE Ack Response */
#define ATE_STR_MOD_PREF      "[ATE]"        /* Log prefix of ate module */

#define ATE_SZ_MAX_PBUF       20			   /* max protocol buffer size */


/**************************************************************************
 *  DEBUG FLAG
 **************************************************************************/
 //#define META_DEBUG

/**************************************************************************
 *  LOCAL VARIABLE DECLARATION
 **************************************************************************/

/**************************************************************************
 *  FUNCTION IMPLEMENTATION
 **************************************************************************/


/******************************************************************************
 * meta_show_logo
 * 
 * DESCRIPTION:
 *   Display META logo on the screen after entering META mode
 *
******************************************************************************/
//Hong-Rong: Current not used.
#if 0
void meta_show_logo()
{
	int len = mboot_common_load_logo(memory_size() - mt6516_disp_get_vram_size(),
                              CFG_META_LOGO_NAME);

#ifdef META_DEBUG
	printf("\n%s 'Meta Logo' Length = %d\n", META_STR_MOD_PREF,len);
#endif
    /* display logo */
    if (len > 0) {
        mt6516_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
        mt6516_backlight_on();
    }
    else
    {
#ifdef META_DEBUG
		printf("%s Load 'Meta Logo' fail\n", META_STR_MOD_PREF);
#endif		
    }
}
#endif

/******************************************************************************
 * listen_tool
 * 
 * DESCRIPTION:
 *
******************************************************************************/
static bool meta_listen_tool(int listen_byte_cnt, int retry_cnt, int wait_time, char* buf)
{
    unsigned int  start_time = get_timer(0);  /* calculate wait time */
    char		  c = 0;
    int 		  i = 0;    
    int 		  rx_cnt = 0;
	ulong         begin_time = 0;		


 	
    for(i=0;i<retry_cnt;i++)
    {	
    	begin_time = get_timer(0);

    	while(get_timer(begin_time) < wait_time)
    	{
	        /* detect data from tool */
    	    c = serial_nonblock_getc();
    	    
	        if(c!=NULL)
    	    {
    	    	buf[rx_cnt]=c;
#ifdef META_DEBUG    	    
				mtk_serial_set_current_uart(UART4);
				printf("%s rx_cnt : %d\n",META_STR_MOD_PREF,rx_cnt);
    	    	printf("%s Receieve data from uart: %c\n",META_STR_MOD_PREF,buf[rx_cnt]);
    	    	mtk_serial_set_current_uart(UART1);
#endif    	    	
        		rx_cnt++;
	        }
        
    	    if (rx_cnt == listen_byte_cnt)
	            return TRUE;    
	    }
	}

	return FALSE;
}



/******************************************************************************
 * meta_check_pc_trigger
******************************************************************************/
BOOL meta_check_pc_trigger(void)
{
  ulong	  begin = 0;
  int     i = 0, j =0;
  char    buf[META_SZ_MAX_PBUF] = "";
  int     meta_lock = 0;
    
	printf("\n%s Check pc meta boot\n",META_STR_MOD_PREF);
	
	// delay for a while (it could be removed)
 	for(i=0;i<200;i++);
   	
  /* send "READY" to notify tool side */   	
	//printf("%s Send 'READY' to notify tool\n",META_STR_MOD_PREF);

	// set uart port to UART1
	//mt6575_serial_set_current_uart(UART1);
	//puts(META_STR_READY);	

  mtk_serial_set_current_uart(UART1);//added by Hong-Rong
	
  /* detect tool existence */  		
  meta_listen_tool(META_SZ_MAX_PBUF-1,1,10,buf);

  if(buf==NULL)
	{
		mtk_serial_set_current_uart(UART4);
		printf("%s Response timeout\n",META_STR_MOD_PREF);
		return;
	}

	mtk_serial_set_current_uart(UART4);
	printf("%s Receieve data from uart1: %s\n",META_STR_MOD_PREF,buf);
        
  /* check "METAMETA", judge whether META mode indicated */
  for(i=0;i<META_SZ_MAX_PBUF-sizeof(META_STR_REQ);i++)
  {
    if (buf[i]!=NULL && !strcmp(buf+i, META_STR_REQ) )
    {
      //Call API to check if META locked */
      meta_lock = UBoot_MetaLock_Check();
      if (meta_lock)
      {
        /* return "LOCK" to ack tool */
        mtk_serial_set_current_uart(UART1);
        puts(META_LOCK);
        
        for(i=0;i<200;i++);
        
        /* Reset the Target */
        printf("META LOCK! Rebooting...");
        mtk_wdt_reset();
      } 
	    /* return "ATEMATEM" to ack tool */
      mtk_serial_set_current_uart(UART1);
      puts(META_STR_ACK);		
	        
      for(i=0;i<200;i++);        

	   mtk_serial_set_current_uart(UART4);
      printf("\n");
      printf("%s Enable meta mode\n",META_STR_MOD_PREF);
      g_boot_mode = META_BOOT;
	
      //video_printf("%s : detect meta mode !\n",META_STR_MOD_PREF);
		return TRUE;
    }
	}
	
	 /* check "ADVMETA", judge whether META mode indicated */
  for(i=0;i<META_SZ_MAX_PBUF-sizeof(META_ADV_REQ);i++)
  {
    if (buf[i]!=NULL && !strcmp(buf+i, META_ADV_REQ) )
    {
      /* Call API to check if META locked */
      meta_lock = UBoot_MetaLock_Check();
      if (meta_lock)
      {
        /* return "LOCK" to ack tool */
        mtk_serial_set_current_uart(UART1);
        puts(META_LOCK);
        
        for(i=0;i<200;i++);
        
        /* Reset the Target */
        printf("META LOCK! Rebooting...");
        WDT_HW_Reset();
      }
	  /* return "ATEMVDA" to ack tool */
      mtk_serial_set_current_uart(UART1);
      puts(META_ADV_ACK);
	        
      for(i=0;i<200;i++);        

	  mtk_serial_set_current_uart(UART4);
      printf("\n");
	  printf("%s Enable meta mode\n",META_STR_MOD_PREF);
	  g_boot_mode = ADVMETA_BOOT;
      //video_printf("%s : detect meta mode !\n",META_STR_MOD_PREF);
	  return TRUE;
    }
  }
	
  /* check "FACTORYM", judge whether ATE mode indicated */
  for(i=0;i<ATE_SZ_MAX_PBUF-sizeof(ATE_STR_REQ);i++)
  {
    if (buf[i]!=NULL && !strcmp(buf+i, ATE_STR_REQ) )
    {      
	    /* return "MYROTCAF" to ack tool */
      mtk_serial_set_current_uart(UART1);
      puts(ATE_STR_ACK);		
	        
      for(i=0;i<200;i++);        

	    mtk_serial_set_current_uart(UART4);
      printf("\n");
	    printf("%s Enable ate_factory mode\n",ATE_STR_MOD_PREF);
	    g_boot_mode = ATE_FACTORY_BOOT;
	    return TRUE;
   }
	}
  
  return FALSE;	    
}

/******************************************************************************
 * meta_detection
 * 
 * DESCRIPTION:
 *   Detect META mode is on or off
 *
******************************************************************************/
BOOL meta_detection(void)
{
  BOOT_ARGUMENT *boot_arg;
  int mode = 0;
#ifdef CONFIG_CMDLINE_TAG    
  boot_arg = (volatile BOOT_ARGUMENT *)(BOOT_ARGUMENT_LOCATION);
  mode = boot_arg->boot_mode &= 0x000000FF;

  printf("%s Check meta info from pre-loader: %x, %x, %d\n", META_STR_MOD_PREF, boot_arg->boot_mode, boot_arg->maggic_number, mode);
  
  if (boot_arg->maggic_number == BOOT_ARGUMENT_MAGIC)
  {
    if (mode == 1)
     {
      g_boot_mode = META_BOOT;
      return TRUE;
    }
    else if (mode == 5)
    {
      g_boot_mode = ADVMETA_BOOT;
      return TRUE;
    }
    else if (mode == 6)
    {
      g_boot_mode = ATE_FACTORY_BOOT;
      return TRUE;
    }
    else if (mode == 7)
    {
      g_boot_mode = ALARM_BOOT;
      return FALSE;
    }
    else
    {
      return FALSE;
    }
  }
#endif
	return FALSE;
}

#endif /* CFG_META_MODE */

