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
#include <asm/arch/mtk_factory.h>

#define MODULE_NAME "[FACTORY]"

#define MT65XX_FACTORY_KEY 2 //for mt6589 fpga uboot early porting

BOOL factory_check_key_trigger(void)
{
	//wait
	ulong begin = get_timer(0);
	printf("\n%s Check factory boot\n",MODULE_NAME);
	printf("%s Wait 50ms for special keys\n",MODULE_NAME);
	
	/* If the boot reason is RESET, than we will NOT enter factory mode. */
	if(mtk_detect_pmic_just_rst())
	{
	  return false;
	}
	
    while(get_timer(begin)<50)
    {    
		if(mtk_detect_key(MT65XX_FACTORY_KEY))
		{	
			printf("%s Detect key\n",MODULE_NAME);
			printf("%s Enable factory mode\n",MODULE_NAME);		
			g_boot_mode = FACTORY_BOOT;
			//video_printf("%s : detect factory mode !\n",MODULE_NAME);
			return TRUE;
		}
	}
		
	return FALSE;		
}

BOOL factory_detection(void)
{
    if(factory_check_key_trigger())
    {
    	return TRUE;
    }
	
	return FALSE;
}


