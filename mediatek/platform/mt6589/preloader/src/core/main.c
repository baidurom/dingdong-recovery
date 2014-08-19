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
#include "download.h"
#include "meta.h"
#include "sec.h"
#include "part.h"

/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD "[BLDR]"

/*============================================================================*/
/* MACROS DEFINITIONS                                                         */
/*============================================================================*/
#define CMD_MATCH(cmd1,cmd2)  \
    (!strncmp((const char*)(cmd1->data), (cmd2), min(strlen(cmd2), cmd1->len)))

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/

/*============================================================================*/
/* INTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static void bldr_pre_process(void)
{
    /* enter preloader safe mode */
    platform_safe_mode(1, 0);

    /* essential hardware initialization. e.g. timer, pll, uart... */
    platform_pre_init();

    print("\n%s Build Time: %s\n", MOD, BUILD_TIME);    

    g_boot_mode = NORMAL_BOOT;

    /* hardware initialization */
    platform_init();

#if CFG_UART_TOOL_HANDSHAKE
    /* init uart handshake for sending 'ready' to tool and receiving handshake
     * pattern from tool in the background and we'll see the pattern later.
     * this can reduce the handshake time.
     */
    uart_handshake_init();
#endif   

    part_init();
    part_dump();

    /* init security library */
    sec_lib_init();
}

static void bldr_post_process(void)
{
    platform_post_init();
}

int bldr_load_part(char *name, blkdev_t *bdev, u32 *addr)
{
    part_t *part = part_get(name);

    if (NULL == part) {
        print("%s %s partition not found\n", MOD, name);
        return -1;
    }

    return part_load(bdev, part, addr, 0, 0);
}

static bool bldr_cmd_handler(struct bldr_command_handler *handler, 
    struct bldr_command *cmd, struct bldr_comport *comport)
{
    struct comport_ops *comm = comport->ops;
    u32 attr = handler->attr;

#if CFG_LEGACY_USB_DOWNLOAD
    /* "DOWNLOAD" */
    if (CMD_MATCH(cmd, DM_STR_DOWNLOAD_REQ) && (comport->type == COM_USB)) {
        comm->send((u8*)DM_STR_DOWNLOAD_ACK, strlen(DM_STR_DOWNLOAD_ACK));
        print("%s download mode detected!\n", MOD);
        download_handler();
        g_boot_mode = DOWNLOAD_BOOT;
        return TRUE;
    }
#endif

#if CFG_DT_MD_DOWNLOAD
    if (CMD_MATCH(cmd, SWITCH_MD_REQ)) {
        /* SWITCHMD */

        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        comm->send((u8*)SWITCH_MD_ACK, strlen(SWITCH_MD_ACK));
        platform_modem_download();
        return TRUE;
    }
#endif

    if (CMD_MATCH(cmd, ATCMD_PREFIX)) {
        /* "AT+XXX" */
        if (CMD_MATCH(cmd, ATCMD_NBOOT_REQ)) {
            /* return "AT+OK" to tool */
            comm->send((u8*)ATCMD_OK, strlen(ATCMD_OK));
            g_boot_mode = NORMAL_BOOT;
            g_boot_reason = BR_TOOL_BY_PASS_PWK;
        } else {
            /* return "AT+UNKONWN" to ack tool */
            comm->send((u8*)ATCMD_UNKNOWN, strlen(ATCMD_UNKNOWN));
            return FALSE;
        }
    } else if (CMD_MATCH(cmd, META_STR_REQ)) {
        para_t param;
        
        /* "METAMETA" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v1, sizeof(param.v1), 5)) {
            g_meta_com_id = param.v1.usb_type;
        }

        comm->send((u8*)META_STR_ACK, strlen(META_STR_ACK));
        g_boot_mode = META_BOOT;
    } else if (CMD_MATCH(cmd, FACTORY_STR_REQ)) {
        para_t param;

        /* "FACTFACT" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v1, sizeof(param.v1), 5)) {
            g_meta_com_id = param.v1.usb_type;
        }

        comm->send((u8*)FACTORY_STR_ACK, strlen(FACTORY_STR_ACK));
        g_boot_mode = FACTORY_BOOT;
    } else if (CMD_MATCH(cmd, META_ADV_REQ)) {
        /* "ADVEMETA" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;
        comm->send((u8*)META_ADV_ACK, strlen(META_ADV_ACK));
        g_boot_mode = ADVMETA_BOOT;
    } else if (CMD_MATCH(cmd, ATE_STR_REQ)) {
        para_t param;

        /* "FACTORYM" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v1, sizeof(param.v1), 5)) {
            g_meta_com_id = param.v1.usb_type;
        }

        comm->send((u8*)ATE_STR_ACK, strlen(ATE_STR_ACK));
        g_boot_mode = ATE_FACTORY_BOOT;
    } else if (CMD_MATCH(cmd, FB_STR_REQ)) {
	/* "FASTBOOT" */
	comm->send((u8 *)FB_STR_ACK, strlen(FB_STR_ACK));
	g_boot_mode = FASTBOOT;
    } else {
        print("%s unknown received: \'%s\'\n", MOD, cmd->data);
        return FALSE;
    }
    print("%s '%s' received!\n", MOD, cmd->data);
    return TRUE;

forbidden:
    comm->send((u8*)META_FORBIDDEN_ACK, strlen(META_FORBIDDEN_ACK));
    print("%s '%s' is forbidden!\n", MOD, cmd->data);
    return FALSE;
}

static int bldr_handshake(struct bldr_command_handler *handler)
{
    boot_mode_t mode = 0;

    /* get mode type */
    mode = seclib_brom_meta_mode();
        
    switch (mode) {
    case NORMAL_BOOT:
        /* ------------------------- */
        /* security check            */
        /* ------------------------- */
        if (TRUE == seclib_sbc_enabled()) {
            handler->attr |= CMD_HNDL_ATTR_COM_FORBIDDEN;
            print("%s META DIS\n", MOD);
        }

        #if CFG_USB_TOOL_HANDSHAKE
        if (TRUE == usb_handshake(handler))
            g_meta_com_type = META_USB_COM;
        #endif
        #if CFG_UART_TOOL_HANDSHAKE
        if (TRUE == uart_handshake(handler))
            g_meta_com_type = META_UART_COM;
        #endif

        break;
    
    case META_BOOT:
        print("%s BR META BOOT\n", MOD);
        g_boot_mode = META_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    case FACTORY_BOOT:
        print("%s BR FACTORY BOOT\n", MOD);
        g_boot_mode = FACTORY_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    case ADVMETA_BOOT:
        print("%s BR ADVMETA BOOT\n", MOD);
        g_boot_mode = ADVMETA_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    case ATE_FACTORY_BOOT:
        print("%s BR ATE FACTORY BOOT\n", MOD);
        g_boot_mode = ATE_FACTORY_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    default:
        print("%s UNKNOWN MODE\n", MOD);
        break;
    }

    return 0;
}

/*============================================================================*/
/* GLOBAL FUNCTIONS                                                           */
/*============================================================================*/
void bldr_jump(u32 addr, u32 arg1, u32 arg2)
{
    platform_wdt_kick();

    /* disable preloader safe mode */
    platform_safe_mode(0, 0);

    print("\n%s jump to 0x%x\n", MOD, addr);
    print("%s <0x%x>=0x%x\n", MOD, addr, *(u32*)addr);
    print("%s <0x%x>=0x%x\n", MOD, addr + 4, *(u32*)(addr + 4));
    /* 2012/11/27
     * Sten
     * Remove MT6589 MCI downsizer workaround start*/
    *(volatile unsigned int*)(0x10001200) &= (~0x1);
     /* Remove MT6589 MCI downsizer workaround end*/
    jump(addr, arg1, arg2);
}

void main(void)
{
    struct bldr_command_handler handler;
    blkdev_t *bootdev;
    u32 addr = 0;
    char *name;

    bldr_pre_process();

    handler.priv = NULL;
    handler.attr = 0;
    handler.cb   = bldr_cmd_handler;

    bldr_handshake(&handler);

    if (NULL == (bootdev = blkdev_get(CFG_BOOT_DEV))) {
        print("%s can't find boot device(%d)\n", MOD, CFG_BOOT_DEV);
        goto error;
    }

#if CFG_LOAD_DSP_ROM
    /* DSP is no more available in MT6589/MT6583 */
#endif

#if CFG_LOAD_MD_FS
    addr = CFG_MD_FS_MEMADDR;
    if (bldr_load_part(PART_MD_FS, bootdev, &addr) != 0)
        goto error;
#endif

#if CFG_LOAD_MD_ROM
    if (platform_is_three_g()) {
        addr = CFG_MD_3G_ROM_MEMADDR;
        name = PART_MD_3G_ROM;
    } else {
        addr = CFG_MD_2G_ROM_MEMADDR;
        name = PART_MD_2G_ROM;
    }
    if (bldr_load_part(name, bootdev, &addr) != 0)
        goto error;
#endif

#if CFG_LOAD_AP_ROM
    addr = CFG_AP_ROM_MEMADDR;
    if (bldr_load_part(PART_AP_ROM, bootdev, &addr) != 0)
        goto error;
#endif

#if CFG_LOAD_UBOOT
    addr = CFG_UBOOT_MEMADDR;
    if (bldr_load_part(PART_UBOOT, bootdev, &addr) != 0)
        goto error;
#endif

    bldr_post_process();
    bldr_jump(addr, BOOT_ARGUMENT_ADDR, sizeof(boot_arg_t));

error:
    platform_error_handler();
}

