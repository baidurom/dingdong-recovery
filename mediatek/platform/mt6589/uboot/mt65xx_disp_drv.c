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

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
//#include <devices.h>
#include <lcd.h>
#include <video_fb.h>

#include <asm/arch/mt65xx.h>
#include <asm/arch/disp_drv_platform.h>

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Export Functions - Display
// ---------------------------------------------------------------------------

static void  *fb_addr      = NULL;
static void  *logo_db_addr = NULL;
static UINT32 fb_size      = 0;


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static const DISP_DRIVER *disp_drv = NULL;

static LCD_IF_ID ctrl_if = LCD_IF_PARALLEL_0;

extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
static BOOL isLCMFound = FALSE;
extern LCM_DRIVER  *lcm_drv;
extern LCM_PARAMS *lcm_params;


UINT32 mt65xx_disp_get_vram_size(void)
{
    return DISP_GetVRamSize();
}

extern void disp_log_enable(int enable);
extern void dbi_log_enable(int enable);


#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

void mt65xx_disp_init(void *lcdbase)
{
	UINT32 boot_mode_addr = 0;
    fb_size = ALIGN_TO(CFG_DISPLAY_WIDTH, 32) * ALIGN_TO(CFG_DISPLAY_HEIGHT, 32) * CFG_DISPLAY_BPP / 8;
	boot_mode_addr = (void *)((UINT32)lcdbase + fb_size);
    logo_db_addr = (void *)((UINT32)lcdbase - 4 * 1024 * 1024);
//    fb_addr      = (void *)((UINT32)lcdbase + fb_size);
	fb_addr  =   lcdbase;

    ///for debug prupose
    disp_log_enable(1);
    dbi_log_enable(1);

    DISP_CHECK_RET(DISP_Init((UINT32)lcdbase, (UINT32)lcdbase, FALSE));

	memset((void*)lcdbase, 0, DISP_GetVRamSize());
    /* transparent front buffer for fb_console display */
#if 1 
    LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER, TRUE));
    LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER, (UINT32)boot_mode_addr));
    LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER, LCD_LAYER_FORMAT_RGB565));
	LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER, CFG_DISPLAY_WIDTH*2));
    LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER, 0, 0));
    LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT));
    LCD_CHECK_RET(LCD_LayerSetSourceColorKey(FB_LAYER, TRUE, 0x0));
#endif

    /* background buffer for uboot logo display */
    LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER - 1, TRUE));
    LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER - 1, (UINT32)fb_addr));
    LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER - 1, LCD_LAYER_FORMAT_RGB565));
    LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER - 1, 0, 0));
    LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER - 1, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT));
    LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER - 1, ALIGN_TO(CFG_DISPLAY_WIDTH, 32)*2));

    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3))
    {
	LCD_CHECK_RET(LCD_LayerSetRotation(FB_LAYER, LCD_LAYER_ROTATE_180));
	LCD_CHECK_RET(LCD_LayerSetRotation(FB_LAYER - 1, LCD_LAYER_ROTATE_180));
    }
}


void mt65xx_disp_power(BOOL on)
{
#ifndef CFG_MT6577_FPGA 
    if (on) {
        DISP_PowerEnable(TRUE);
        DISP_PanelEnable(TRUE);
    } else {
        DISP_PanelEnable(FALSE);
        DISP_PowerEnable(FALSE);
    }
#endif
}


void* mt65xx_get_logo_db_addr(void)
{
    return logo_db_addr;
}


void* mt65xx_get_fb_addr(void)
{
    return fb_addr;
}


UINT32 mt65xx_get_fb_size(void)
{
    return fb_size;
}


void mt65xx_disp_update(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    DISP_CHECK_RET(DISP_UpdateScreen(x, y, width, height));
}


void mt65xx_disp_wait_idle(void)
{
    LCD_CHECK_RET(LCD_WaitForNotBusy());
}

UINT32 mt65xx_disp_get_lcd_time(void)
{
#if 0
	UINT32 time0, time1, lcd_time;
	mt65xx_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);

    LCD_CHECK_RET(LCD_WaitForNotBusy());

	time0 = gpt4_tick2time_us(gpt4_get_current_tick());
    LCD_CHECK_RET(LCD_StartTransfer(FALSE));

	if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode==CMD_MODE) {
		DSI_clk_HS_mode(1);
		DSI_CHECK_RET(DSI_EnableClk());
	}
	else if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode!=CMD_MODE) {
		DSI_clk_HS_mode(1);
		DPI_CHECK_RET(DPI_EnableClk());
		DSI_CHECK_RET(DSI_EnableClk());
	}
    LCD_CHECK_RET(LCD_WaitForNotBusy());

	time1 = gpt4_tick2time_us(gpt4_get_current_tick());

	lcd_time = time1 - time0;
	printf("lcd one %d \n", lcd_time);
	if(0 != lcd_time)	
		return (100000000/lcd_time);
	else
#endif
		return (6000);
}

int mt6573IDP_EnableDirectLink(void)
{
    return 0;   // dummy function
}

const char* mt65xx_disp_get_lcm_id(void)
{
    return DISP_GetLCMId();
}


void disp_get_fb_address(UINT32 *fbVirAddr, UINT32 *fbPhysAddr)
{
    *fbVirAddr = fb_addr;
    *fbPhysAddr = fb_addr;
}

// ---------------------------------------------------------------------------
//  Export Functions - Console
// ---------------------------------------------------------------------------

#ifdef CONFIG_CFB_CONSOLE

//  video_hw_init -- called by drv_video_init() for framebuffer console

extern UINT32 memory_size(void);

void *video_hw_init (void)
{
    static GraphicDevice s_mt65xx_gd;

	memset(&s_mt65xx_gd, 0, sizeof(GraphicDevice));

    s_mt65xx_gd.frameAdrs  = memory_size() - mt65xx_disp_get_vram_size() + fb_size;
    s_mt65xx_gd.winSizeX   = CFG_DISPLAY_WIDTH;
    s_mt65xx_gd.winSizeY   = CFG_DISPLAY_HEIGHT;
    s_mt65xx_gd.gdfIndex   = GDF_16BIT_565RGB;
    s_mt65xx_gd.gdfBytesPP = CFG_DISPLAY_BPP / 8;
    s_mt65xx_gd.memSize    = s_mt65xx_gd.winSizeX * s_mt65xx_gd.winSizeY * s_mt65xx_gd.gdfBytesPP;

    return &s_mt65xx_gd;
}


void video_set_lut(unsigned int index,  /* color number */
                   unsigned char r,     /* red */
                   unsigned char g,     /* green */
                   unsigned char b)     /* blue */
{
}

#endif  // CONFIG_CFB_CONSOLE
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#if 0
static void lcm_set_reset_pin(UINT32 value)
{
    LCD_SetResetSignal(value);
}

static void lcm_udelay(UINT32 us)
{
    udelay(us);
}

static void lcm_mdelay(UINT32 ms)
{
    udelay(1000 * ms);
}

static void lcm_send_cmd(UINT32 cmd)
{
	if(lcm_params == NULL)
		return;

    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_LOW,
                              cmd, lcm_params->dbi.cpu_write_bits));
}

static void lcm_send_data(UINT32 data)
{
	if(lcm_params == NULL)
		return;

    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_HIGH,
                              data, lcm_params->dbi.cpu_write_bits));
}

static UINT32 lcm_read_data(void)
{
    UINT32 data = 0;
	ASSERT(lcm_params != NULL);
 	if(lcm_params == NULL)
		return;
   
    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_ReadIF(ctrl_if, LCD_IF_A0_HIGH,
                             &data, lcm_params->dbi.cpu_write_bits));

    return data;
}

static const LCM_UTIL_FUNCS lcm_utils =
{
    .set_reset_pin      = lcm_set_reset_pin,
    .set_gpio_out       = mt_set_gpio_out,
    .udelay             = lcm_udelay,
    .mdelay             = lcm_mdelay,
    .send_cmd           = lcm_send_cmd,
    .send_data          = lcm_send_data,
    .read_data          = lcm_read_data,
    .dsi_set_cmdq		= DSI_set_cmdq,
	.dsi_set_cmdq_V2	= DSI_set_cmdq_V2,
	.dsi_write_cmd		= DSI_write_lcm_cmd,
	.dsi_write_regs 	= DSI_write_lcm_regs,
	.dsi_read_reg		= DSI_read_lcm_reg,
	.dsi_dcs_read_lcm_reg       = DSI_dcs_read_lcm_reg,
	.dsi_dcs_read_lcm_reg_v2    = DSI_dcs_read_lcm_reg_v2,
    /** FIXME: GPIO mode should not be configured in lcm driver
               REMOVE ME after GPIO customization is done    
    */
    .set_gpio_mode        = mt_set_gpio_mode,
    .set_gpio_dir         = mt_set_gpio_dir,
    .set_gpio_pull_enable = mt_set_gpio_pull_enable
};




static __inline LCD_IF_WIDTH to_lcd_if_width(LCM_DBI_DATA_WIDTH data_width)
{
    switch(data_width)
    {
    case LCM_DBI_DATA_WIDTH_8BITS  : return LCD_IF_WIDTH_8_BITS;
    case LCM_DBI_DATA_WIDTH_9BITS  : return LCD_IF_WIDTH_9_BITS;
    case LCM_DBI_DATA_WIDTH_16BITS : return LCD_IF_WIDTH_16_BITS;
    case LCM_DBI_DATA_WIDTH_18BITS : return LCD_IF_WIDTH_18_BITS;
    case LCM_DBI_DATA_WIDTH_24BITS : return LCD_IF_WIDTH_24_BITS;
    default : ASSERT(0);
    }
    return LCD_IF_WIDTH_18_BITS;
}


static void disp_drv_init_ctrl_if(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

	if(lcm_params == NULL)
		return;

    switch(lcm_params->ctrl)
    {
    case LCM_CTRL_NONE :
    case LCM_CTRL_GPIO : return;

    case LCM_CTRL_SERIAL_DBI :
        ASSERT(dbi->port <= 1);
        ctrl_if = LCD_IF_SERIAL_0 + dbi->port;
        LCD_ConfigSerialIF(ctrl_if,
									 (LCD_IF_SERIAL_BITS)dbi->data_width,
									 dbi->serial.sif_3wire,
									 dbi->serial.sif_sdi,
									 dbi->serial.sif_1st_pol,
									 dbi->serial.sif_sck_def,
									 dbi->serial.sif_div2,
									 dbi->serial.sif_hw_cs,
									 dbi->serial.css,
									 dbi->serial.csh,
									 dbi->serial.rd_1st,
									 dbi->serial.rd_2nd,
									 dbi->serial.wr_1st,
									 dbi->serial.wr_2nd);

        break;
        
    case LCM_CTRL_PARALLEL_DBI :
        ASSERT(dbi->port <= 2);
        ctrl_if = LCD_IF_PARALLEL_0 + dbi->port;
        LCD_ConfigParallelIF(ctrl_if,
                             (LCD_IF_PARALLEL_BITS)dbi->data_width,
                             (LCD_IF_PARALLEL_CLK_DIV)dbi->clock_freq,
                             dbi->parallel.write_setup,
                             dbi->parallel.write_hold,
                             dbi->parallel.write_wait,
                             dbi->parallel.read_setup,
							 dbi->parallel.read_hold,
                             dbi->parallel.read_latency,
                             dbi->parallel.wait_period,
							 dbi->parallel.cs_high_width);
        break;

    default : ASSERT(0);
    }

    LCD_CHECK_RET(LCD_SelectWriteIF(ctrl_if));

    LCD_CHECK_RET(LCD_ConfigIfFormat(dbi->data_format.color_order,
                                     dbi->data_format.trans_seq,
                                     dbi->data_format.padding,
                                     dbi->data_format.format,
                                     to_lcd_if_width(dbi->data_format.width)));
}


static void disp_drv_set_driving_current(LCM_PARAMS *lcm)
{
	LCD_Set_DrivingCurrent(lcm);
}

static void disp_drv_init_io_pad(LCM_PARAMS *lcm)
{
	LCD_Init_IO_pad(lcm);
}

extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
extern void init_dsi(void);
LCM_DRIVER *disp_drv_get_lcm_driver(const char* lcm_name)
{
	LCM_DRIVER *lcm = NULL;
	printf("[LCM Auto Detect], we have %d lcm drivers built in\n", lcm_count);

	if(lcm_count ==1)
	{
		// we need to verify whether the lcm is connected
		// even there is only one lcm type defined
		lcm = lcm_driver_list[0];
		lcm->set_util_funcs(&lcm_utils);
		lcm->get_params(&s_lcm_params);

		lcm_params = &s_lcm_params;
		lcm_drv = lcm;
		isLCMFound = TRUE;
        
        printf("[LCM Specified]\t[%s]\n", (lcm->name==NULL)?"unknown":lcm->name);

		goto done;
	}
	else
	{
		int i;

		for(i = 0;i < lcm_count;i++)
		{
			lcm_params = &s_lcm_params;
			lcm = lcm_driver_list[i];

			printf("[LCM Auto Detect] [%d] - [%s]\t", 
				i, 
				(lcm->name==NULL)?"unknown":lcm->name);

			lcm->set_util_funcs(&lcm_utils);
			memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
			lcm->get_params(lcm_params);

			disp_drv_init_ctrl_if();
			disp_drv_set_driving_current(lcm_params);
			disp_drv_init_io_pad(lcm_params);

			if(lcm_name != NULL)
			{
				if(!strcmp(lcm_name,lcm->name))
				{
					printf("\t\t[success]\n");
					isLCMFound = TRUE;
					lcm_drv = lcm;

					goto done;
				}
				else
				{
					printf("\t\t[fail]\n");
				}
			}
			else 
			{
				if(LCM_TYPE_DSI == lcm_params->type){
					init_dsi();
				}
				if(lcm->compare_id != NULL && lcm->compare_id())
				{
					printf("\t\t[success]\n");
					isLCMFound = TRUE;
					lcm_drv = lcm;

					goto done;
				}
				else
				{
					printf("\t\t[fail]\n");
					if(LCM_TYPE_DSI == lcm_params->type)
						DSI_Deinit();
				}
			}
		}
	}

done:
	return lcm;
}


static void disp_dump_lcm_parameters(LCM_PARAMS *lcm_params)
{
	unsigned char *LCM_TYPE_NAME[] = {"DBI", "DPI", "DSI"};
	unsigned char *LCM_CTRL_NAME[] = {"NONE", "SERIAL", "PARALLEL", "GPIO"};
	printf("[LCM Auto Detect] LCM TYPE: %s\n", LCM_TYPE_NAME[lcm_params->type]);
	printf("[LCM Auto Detect] LCM INTERFACE: %s\n", LCM_CTRL_NAME[lcm_params->ctrl]);
	printf("[LCM Auto Detect] LCM resolution: %d x %d\n", lcm_params->width, lcm_params->height);

	return;
}
static BOOL disp_drv_init_context(void)
{
	LCD_STATUS ret;
	if (disp_drv != NULL && lcm_drv != NULL){
		return TRUE;
	}

	DISP_DetectDevice();

	disp_drv_init_ctrl_if();
	disp_drv_set_driving_current(NULL);

	switch(lcm_params->type)
	{
		case LCM_TYPE_DBI : disp_drv = DISP_GetDriverDBI(); break;
		case LCM_TYPE_DPI : disp_drv = DISP_GetDriverDPI(); break;
		case LCM_TYPE_DSI : disp_drv = DISP_GetDriverDSI(); break;
		default : ASSERT(0);
	}

	if (!disp_drv) return FALSE;

	return TRUE;
}

BOOL DISP_IsLcmFound(void)
{
	return isLCMFound;
}

BOOL DISP_SelectDevice(const char* lcm_name)
{
	LCD_STATUS ret;

	ret = LCD_Init();
	printf("ret of LCD_Init() = %d\n", ret);

	lcm_drv = disp_drv_get_lcm_driver(lcm_name);
	if (NULL == lcm_drv)
	{
		printf("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	disp_dump_lcm_parameters(lcm_params);
	return disp_drv_init_context();
}

BOOL DISP_DetectDevice(void)
{
	LCD_STATUS ret;

	ret = LCD_Init();

	lcm_drv = disp_drv_get_lcm_driver(NULL);
	if (NULL == lcm_drv)
	{
		printf("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	disp_dump_lcm_parameters(lcm_params);
	return true;
}

// ---------------------------------------------------------------------------
//  DISP Driver Implementations
// ---------------------------------------------------------------------------

DISP_STATUS DISP_Init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    if (!disp_drv_init_context()) {
        return DISP_STATUS_NOT_IMPLEMENTED;
    }

    /* power on LCD before config its registers*/
    LCD_CHECK_RET(LCD_Init());

    disp_drv_init_ctrl_if();
//    disp_drv_set_io_driving_current();
    
    return (disp_drv->init) ?
           (disp_drv->init(fbVA, fbPA, isLcmInited)) :
           DISP_STATUS_NOT_IMPLEMENTED;
}


DISP_STATUS DISP_Deinit(void)
{
    DISP_CHECK_RET(DISP_PanelEnable(FALSE));
    DISP_CHECK_RET(DISP_PowerEnable(FALSE));
   
    return DISP_STATUS_OK;
}

// -----

DISP_STATUS DISP_PowerEnable(BOOL enable)
{
    static BOOL s_enabled = FALSE;

	if (enable != s_enabled)
		s_enabled = enable;
	else
		return ;

    disp_drv_init_context();
       
    return (disp_drv->enable_power) ?
           (disp_drv->enable_power(enable)) :
           DISP_STATUS_NOT_IMPLEMENTED;
}


DISP_STATUS DISP_PanelEnable(BOOL enable)
{

    static BOOL s_enabled = FALSE;

    disp_drv_init_context();

    if (!lcm_drv->suspend || !lcm_drv->resume) {
        return DISP_STATUS_NOT_IMPLEMENTED;
    }

	if (enable && !s_enabled) {
		s_enabled = TRUE;

		if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		{		
			DSI_SetMode(CMD_MODE);
		}

		lcm_drv->resume();

		if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		{
			//DSI_clk_HS_mode(1);
			DSI_SetMode(lcm_params->dsi.mode);
			
			//DPI_CHECK_RET(DPI_EnableClk());
			//DSI_CHECK_RET(DSI_EnableClk());
		}
	}
	else if (!enable && s_enabled)
	{
		LCD_CHECK_RET(LCD_WaitForNotBusy());
		if(lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE)
			DSI_CHECK_RET(DSI_WaitForNotBusy());
		s_enabled = FALSE;

		if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE)
		{		
			DPI_CHECK_RET(DPI_DisableClk());
			udelay(200*1000);
			DSI_Reset();
			DSI_clk_HS_mode(0);
			DSI_SetMode(CMD_MODE);
		}

		lcm_drv->suspend();
	}

    return DISP_STATUS_OK;
}

DISP_STATUS DISP_SetBacklight(UINT32 level)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	disp_drv_init_context();

	LCD_WaitForNotBusy();

	if (!lcm_drv->set_backlight) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	lcm_drv->set_backlight(level);

End:
	return ret;
}


// -----

DISP_STATUS DISP_SetFrameBufferAddr(UINT32 fbPhysAddr)
{
    disp_drv_init_context();
        
    return (disp_drv->set_fb_addr) ?
           (disp_drv->set_fb_addr(fbPhysAddr)) :
           DISP_STATUS_NOT_IMPLEMENTED;
}

// -----

static BOOL is_overlaying = FALSE;

DISP_STATUS DISP_EnterOverlayMode(void)
{
    if (is_overlaying) {
        return DISP_STATUS_ALREADY_SET;
    } else {
        is_overlaying = TRUE;
    }

    return DISP_STATUS_OK;
}


DISP_STATUS DISP_LeaveOverlayMode(void)
{
    if (!is_overlaying) {
        return DISP_STATUS_ALREADY_SET;
    } else {
        is_overlaying = FALSE;
    }

    return DISP_STATUS_OK;
}

// -----
#if 0
static volatile int direct_link_layer = -1;

DISP_STATUS DISP_EnableDirectLinkMode(UINT32 layer)
{
    if (layer != direct_link_layer) {
        LCD_CHECK_RET(LCD_LayerSetTriggerMode(layer, LCD_HW_TRIGGER_DIRECT_COUPLE));
        LCD_CHECK_RET(LCD_LayerSetHwTriggerSrc(layer, LCD_HW_TRIGGER_SRC_IBW2));
        LCD_CHECK_RET(LCD_EnableHwTrigger(TRUE));
        LCD_CHECK_RET(LCD_StartTransfer(FALSE));
        direct_link_layer = layer;
    }

    return DISP_STATUS_OK;
}



DISP_STATUS DISP_DisableDirectLinkMode(UINT32 layer)
{
    if (layer == direct_link_layer) {
        LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));
        direct_link_layer = -1;
    }
    LCD_CHECK_RET(LCD_LayerSetTriggerMode(layer, LCD_SW_TRIGGER));

    return DISP_STATUS_OK;
}
#endif

// -----

extern int mt65xxIDP_EnableDirectLink(void);

DISP_STATUS DISP_UpdateScreen(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{

    LCD_CHECK_RET(LCD_WaitForNotBusy());

    if ((lcm_drv->update) &&
	   ((lcm_params->type==LCM_TYPE_DBI) || ((lcm_params->type==LCM_TYPE_DSI) && (lcm_params->dsi.mode==CMD_MODE))))
		{
        lcm_drv->update(x, y, width, height);
    }	

    LCD_CHECK_RET(LCD_SetRoiWindow(x, y, width, height));
    LCD_CHECK_RET(LCD_FBSetStartCoord(x, y));

    LCD_CHECK_RET(LCD_StartTransfer(FALSE));

	if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode==CMD_MODE) {
		DSI_clk_HS_mode(1);
		DSI_CHECK_RET(DSI_EnableClk());
	}
	else if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode!=CMD_MODE) {
		DSI_clk_HS_mode(1);
		DPI_CHECK_RET(DPI_EnableClk());
		DSI_CHECK_RET(DSI_EnableClk());
	}
	
    return DISP_STATUS_OK;
}


// ---------------------------------------------------------------------------
//  Retrieve Information
// ---------------------------------------------------------------------------

UINT32 DISP_GetScreenWidth(void)
{
    disp_drv_init_context();
    return lcm_params->width;
}


UINT32 DISP_GetScreenHeight(void)
{
    disp_drv_init_context();
    return lcm_params->height;
}


UINT32 DISP_GetScreenBpp(void)
{
    return 32;  // ARGB8888
}


UINT32 DISP_GetPages(void)
{
    //return 4;
    return 2;   // Double Buffers
}


#define ALIGN_TO_POW_OF_2(x, n)  \
    (((x) + ((n) - 1)) & ~((n) - 1))

UINT32 DISP_GetVRamSize(void)
{
    // Use a local static variable to cache the calculated vram size
    //    
    static UINT32 vramSize = 0;

    if (0 == vramSize)
    {
        disp_drv_init_context();

        vramSize = disp_drv->get_vram_size();
        
        // Align vramSize to 1MB
        //
        vramSize = ALIGN_TO_POW_OF_2(vramSize, 0x100000);

        //printf("DISP_GetVRamSize: %u bytes\n", vramSize);
    }

    return vramSize;
}


PANEL_COLOR_FORMAT DISP_GetPanelColorFormat(void)
{
    disp_drv_init_context();
        
    return (disp_drv->get_panel_color_format) ?
           (disp_drv->get_panel_color_format()) :
           DISP_STATUS_NOT_IMPLEMENTED;
}
#endif
