/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
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

#include <platform/pll.h>

unsigned int mtk_get_bus_freq(void)
{
    kal_uint32 mainpll_con0, mainpll_con1, main_diff;
    kal_uint32 clk_cfg_0, bus_clk, output_freq;

    clk_cfg_0 = DRV_Reg32(CLK_CFG_0);

    mainpll_con0 = DRV_Reg32(MAINPLL_CON0);
    mainpll_con1 = DRV_Reg32(MAINPLL_CON1);

    main_diff = ((mainpll_con1 >> 12) - 0x8009A) / 2;

    if ((mainpll_con0 & 0xFF) == 0x01)
    {
        output_freq = 1001 + (main_diff * 13); // Mhz
    }
    else // if (mainpll_con0 & 0xFF) == 0x41)
    {
        output_freq = 1001 + (main_diff * 13) / 2; // Mhz
    }

    if ((clk_cfg_0 & 0x7) == 1) // SYSPLL_D3 = MAINPLL / 3 / 2
    {
        bus_clk = ((output_freq * 1000) / 3) / 2;
    }
    else if ((clk_cfg_0 & 0x7) == 2) // SYSPLL_D4 = MAINPLL / 2 / 4
    {
        bus_clk = ((output_freq * 1000) / 2) / 4;
    }
    else if ((clk_cfg_0 & 0x7) == 3) // SYSPLL_D6 = MAINPLL /2 / 6
    {
        bus_clk = ((output_freq * 1000) / 2) / 6;
    }
    else if ((clk_cfg_0 & 0x7) == 4) // UNIVPLL_D5 = UNIVPLL / 5
    {
        bus_clk = (1248 * 1000) / 5;
    }
    else if ((clk_cfg_0 & 0x7) == 5) // UNIVPLL2_D2 = UNIVPLL / 3 / 2
    {
        bus_clk = (1248 * 1000) / 3 / 2;
    }
    else // CLKSQ
    {
        bus_clk = 26 * 1000;
    }

    return bus_clk; // Khz
}