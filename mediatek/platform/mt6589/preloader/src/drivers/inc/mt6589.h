/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2011. All rights reserved.
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

#ifndef MT6589_H
#define MT6589_H

/*=======================================================================*/
/* Constant Definitions                                                  */
/*=======================================================================*/

#define IO_PHYS            	(0x10000000)
#define IO_SIZE            	(0x00100000)

#define VER_BASE            (0x08000000)

/*=======================================================================*/
/* Register Bases                                                        */
/*=======================================================================*/
#define RGU_BASE            (IO_PHYS + 0x0)
#define CONFIG_BASE         (IO_PHYS + 0x1000)
#define SRAMROM_BASE        (IO_PHYS + 0x2000)
#define DRAMC0_BASE         (IO_PHYS + 0x4000)
#define DDRPHY_BASE        (IO_PHYS + 0x11000)
#define DRAMC_NAO_BASE      (IO_PHYS + 0x20F000)
#define GPIO_BASE           (IO_PHYS + 0x00005000)
#define GPIO1_BASE          (IO_PHYS + 0x0020C000)
#define PLL_BASE            (IO_PHYS + 0x7000)
#define GPT_BASE            (IO_PHYS + 0x8000)

#define DEM_BASE            (IO_PHYS + 0x0011A000)
#define EMI_BASE            (IO_PHYS + 0x00203000)

#define PERI_CON_BASE       (IO_PHYS + 0x01003000)  /* CHECKME & FIXME */

//#define RTC_BASE            (IO_PHYS + 0x01003000)  /* CHECKME & FIXME (new design) */
#define RTC_BASE            0xE000

#define NFI_BASE            (IO_PHYS + 0x01001000)
#define NFIECC_BASE         (IO_PHYS + 0x01002000)
#define UART0_BASE          (IO_PHYS + 0x01006000)
#define UART1_BASE          (IO_PHYS + 0x01007000)
#define UART2_BASE          (IO_PHYS + 0x01008000)
#define UART3_BASE          (IO_PHYS + 0x01009000)

#define KPD_BASE            (IO_PHYS + 0x00015000)  /* CHECKME & FIXME (new design) */

#define I2C0_BASE           (IO_PHYS + 0x0100D000)
#define I2C1_BASE           (IO_PHYS + 0x0100E000)   
#define I2C2_BASE           (IO_PHYS + 0x0100F000)
#define I2C3_BASE           (IO_PHYS + 0x01010000)
#define I2C4_BASE           (IO_PHYS + 0x01011000)
#define I2C5_BASE           (IO_PHYS + 0x01012000)
#define I2C6_BASE           (IO_PHYS + 0x01013000)

#define SPI_BASE            (IO_PHYS + 0x01016000)
#define PWRAP_BRIDGE_BASE   (IO_PHYS + 0x01021000)

#define USB0_BASE           (IO_PHYS + 0x01200000)
#define USB1_BASE           (IO_PHYS + 0x01210000)
#define USBSIF_BASE         (IO_PHYS + 0x01220000)
#define USB_BASE            (USB0_BASE)

#define MSDC0_BASE          (IO_PHYS + 0x01230000)
#define MSDC1_BASE          (IO_PHYS + 0x01240000)
#define MSDC2_BASE          (IO_PHYS + 0x01250000)
#define MSDC3_BASE          (IO_PHYS + 0x01260000)
#define MSDC4_BASE          (IO_PHYS + 0x01270000)

#define APHW_CODE           (VER_BASE)
#define APHW_SUBCODE        (VER_BASE + 0x04)
#define APHW_VER            (VER_BASE + 0x08)
#define APSW_VER            (VER_BASE + 0x0C)

#define AMCONFG_BASE        (0xFFFFFFFF)            /* CHECKME & FIXME */

/*=======================================================================*/
/* USB download control                                                  */
/*=======================================================================*/
#define SRAMROM_USBDL       (SRAMROM_BASE + 0x0050)

#define USBDL_BIT_EN        (0x00000001) /* 1: download bit enabled */
#define USBDL_PL            (0x00000002) /* 0: usbdl by brom; 1: usbdl by bootloader */
#define USBDL_TIMEOUT_MASK  (0x0000FFFC) /* 14-bit timeout: 0x0000~0x3FFE: second; 0x3FFFF: no timeout */
#define USBDL_TIMEOUT_MAX   (USBDL_TIMEOUT_MASK >> 2) /* maximum timeout indicates no timeout */

/*=======================================================================*/
/* NAND Control                                                          */	
/*=======================================================================*/
#define NAND_PAGE_SIZE                  (2048)  // (Bytes)
#define NAND_BLOCK_BLKS                 (64)    // 64 nand pages = 128KB
#define NAND_PAGE_SHIFT                 (9)
#define NAND_LARGE_PAGE                 (11)    // large page
#define NAND_SMALL_PAGE                 (9)     // small page
#define NAND_BUS_WIDTH_8                (8)
#define NAND_BUS_WIDTH_16               (16)
#define NAND_FDM_SIZE                   (8)
#define NAND_ECC_SW                     (0)
#define NAND_ECC_HW                     (1)

#define NFI_MAX_FDM_SIZE                (8)
#define NFI_MAX_FDM_SEC_NUM             (8)
#define NFI_MAX_LOCK_CHANNEL            (16)

#define ECC_MAX_CORRECTABLE_BITS        (12)
#define ECC_MAX_PARITY_SIZE             (20)    /* in bytes */

#define ECC_ERR_LOCATION_MASK           (0x1FFF)
#define ECC_ERR_LOCATION_SHIFT          (16)

#define NAND_FFBUF_SIZE                 (2048+64)

#endif
