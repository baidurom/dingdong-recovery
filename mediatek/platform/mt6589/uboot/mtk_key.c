/*
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
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <cust_key.h>
#include <asm/arch/mtk_key.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/mtk_pmic_6320.h>

extern pmic_detect_powerkey(void);
extern pmic_detect_homekey(void);

#define KPD_UBOOT_TEST 0

#if KPD_UBOOT_TEST
void kpd_get_keymap_state(u16 state[])
{
	state[0] = *(volatile u16 *)KP_MEM1;
	state[1] = *(volatile u16 *)KP_MEM2;
	state[2] = *(volatile u16 *)KP_MEM3;
	state[3] = *(volatile u16 *)KP_MEM4;
	state[4] = *(volatile u16 *)KP_MEM5;
	if (1) {
		printf("register = %x %x %x %x %x\n",
		       state[0], state[1], state[2], state[3], state[4]);
	}
}

static u16 kpd_keymap_state[5] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0x00ff
};
#endif


void set_kpd_pmic_mode()
{
	*(volatile u16 *)(KP_BASE+0x1C) = 0x1;
	printf("kpd register for pmic set!\n");
	return;
}

bool mtk_detect_key(unsigned short key)	/* key: HW keycode */
{
	unsigned short idx, bit, din;

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
            dbg_print ("power key is pressed\n");
            return true;
        }
        return false;
    }
#else // check by PMIC
    if (key == 8)
    {                         /* Power key */
        if (1 == pmic_detect_powerkey())
        {
            //dbg_print ("power key is pressed\n");
            return true;
        }
        return false;
    }    
#endif

		if (key == MT65XX_PMIC_RST_KEY) 
		{
			if (1 == pmic_detect_homekey())
			{
				return true;
			}
			// return false; /* to support common EVB */
		}

	idx = key / 16;
	bit = key % 16;

	din = DRV_Reg16(KP_MEM1 + (idx << 2)) & (1U << bit);
	if (!din) {
		printf("key %d is pressed\n", key);
		return true;
	}

/***************************/
#if KPD_UBOOT_TEST
	int i, j;
	bool pressed;
	u16 new_state[5], change, mask;
	u16 hw_keycode, linux_keycode;
	u16 a;
	a = *(volatile u16 *)(KP_STA);
	printf("yucong debug stuts register value = 0x%x\n",a);

	do{
	kpd_get_keymap_state(new_state);
	for (i = 0; i < 5; i++) {
		change = new_state[i] ^ kpd_keymap_state[i];
		if (!change)
			continue;

		for (j = 0; j < 16; j++) {
			mask = 1U << j;
			if (!(change & mask))
				continue;

			hw_keycode = (i << 4) + j;
			/* bit is 1: not pressed, 0: pressed */
			pressed = !(new_state[i] & mask);
			if (1) {
				printf("(%s) HW keycode = %u\n",
				       pressed ? "pressed" : "released",
				       hw_keycode);
			}
			if(hw_keycode == 3)break;	
			continue;
		}
	}
	kpd_keymap_state[0] = new_state[0];
	kpd_keymap_state[1] = new_state[1];
	kpd_keymap_state[2] = new_state[2];
	kpd_keymap_state[3] = new_state[3];
	kpd_keymap_state[4] = new_state[4];
	printf("save new keymap state\n");
	}while(1);
#endif
/***************************/
	return false;
}

bool mtk_detect_pmic_just_rst()
{
	kal_uint8 just_rst;
	
	printf("detecting pmic just reset\n");
		pmic_read_interface(0x15, &just_rst, 0x01, 0x07);
		if(just_rst)
		{
			printf("Just recover form a reset\n"); 
			pmic_config_interface(0x22, 0x01, 0x01, 0x07);
			return TRUE;
		}
	return FALSE;
}
