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
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mtk_key.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/mt6575_recovery.h>
#include <asm/arch/mt_rtc.h>
#include "mt65xx_partition.h"
//#include <cust_kpd.h> //for 83fpga uboot early porting 

#define MT65XX_RECOVERY_KEY 3 //for 83fpga uboot early porting 


extern int mboot_recovery_load_misc(unsigned char *misc_addr, unsigned int size);

#define MODULE_NAME "[RECOVERY]"

//#define LOG_VERBOSE

//#define MSG printf
#define MSG

#ifdef LOG_VERBOSE
static void dump_data(const char *data, int len) {
    int pos;
    for (pos = 0; pos < len; ) {
        MSG("%05x: %02x", pos, data[pos]);
        for (++pos; pos < len && (pos % 24) != 0; ++pos) {
            MSG(" %02x", data[pos]);
        }
        MSG("\n");
    }
}
#endif

BOOL recovery_check_key_trigger(void)
{
    //wait
    ulong begin = get_timer(0);
	printf("\n%s Check recovery boot\n",MODULE_NAME);
	printf("%s Wait 50ms for special keys\n",MODULE_NAME);
	
    while(get_timer(begin)<50)
    {    
    	if(mtk_detect_key(MT65XX_RECOVERY_KEY))
    	{	
    		printf("%s Detect cal key\n",MODULE_NAME);
    		printf("%s Enable recovery mode\n",MODULE_NAME);		
    		g_boot_mode = RECOVERY_BOOT;
    		//video_printf("%s : detect recovery mode !\n",MODULE_NAME);
    		return TRUE;
    	}
    }
		
    return FALSE;		
}

BOOL recovery_check_command_trigger(void)
{
	struct misc_message misc_msg;
	struct misc_message *pmisc_msg = &misc_msg;
	const unsigned int size = NAND_WRITE_SIZE * MISC_PAGES;
	unsigned char *pdata;
    	int ret;

	pdata = (uchar*)malloc(sizeof(uchar)*size);

	ret = mboot_recovery_load_misc(pdata, size);
	
    if (ret < 0)
    {
    	return FALSE;	
    }	
	
#ifdef LOG_VERBOSE
    MSG("\n--- get_bootloader_message ---\n");
    dump_data(pdata, size);
    MSG("\n");
#endif

#ifndef MTK_EMMC_SUPPORT //wschen 2012-01-12 eMMC did not need 2048 byte offset
	memcpy(pmisc_msg, &pdata[NAND_WRITE_SIZE * MISC_COMMAND_PAGE], sizeof(misc_msg));	
#else
	memcpy(pmisc_msg, pdata, sizeof(misc_msg));	
#endif
	MSG("Boot command: %.*s\n", sizeof(misc_msg.command), misc_msg.command);
	MSG("Boot status: %.*s\n", sizeof(misc_msg.status), misc_msg.status);
	MSG("Boot message\n\"%.20s\"\n", misc_msg.recovery);

	if(strcmp(misc_msg.command, "boot-recovery")==0)
	{	
	  g_boot_mode = RECOVERY_BOOT;
	}

	return TRUE;
}

/**********************************************************
 * Routine: recovery_detection
 *
 * Description: check recovery mode
 *
 * Notice: the recovery bits of RTC PDN1 are set as 0x10 only if
 *			(1) user trigger factory reset
 *
 **********************************************************/
BOOL recovery_detection(void)
{
	if ((DRV_Reg16(RTC_PDN1) & 0x0030) == 0x0010) {	/* factory data reset */
		g_boot_mode = RECOVERY_BOOT;
		return TRUE;
	}

    if(recovery_check_key_trigger())
    {
    	return TRUE;
    }
        
    
    #ifdef CFG_NAND_BOOT 
	recovery_check_command_trigger();
	#endif


	if (g_boot_mode == RECOVERY_BOOT)
	{	
	  return TRUE;
	}
	else
	{
	  return FALSE;
	}
	

}

