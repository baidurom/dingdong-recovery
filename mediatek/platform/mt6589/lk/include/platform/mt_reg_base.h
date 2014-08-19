#ifndef __MT6589_H__
#define __MT6589_H__

/* I/O mapping */
#define IO_PHYS            	0x10000000
#define IO_SIZE            	0x00100000


#define APCONFIG_BASE (IO_PHYS + 0x00026000)

/* IO register definitions */

#define TOP_RGU_BASE              (IO_PHYS + 0x00000000)
#define CONFIG_BASE               (IO_PHYS + 0x00001000)
#define SRAMROM_BASE              (IO_PHYS + 0x00002000)
#define DVFS_BASE                 (IO_PHYS + 0x00004000)
#define GPIO_BASE                 (IO_PHYS + 0x00005000)
#define SLEEP_BASE                (IO_PHYS + 0x00006000)
#define APMIXED_BASE              (IO_PHYS + 0x00007000)
#define PMIC_WRAP_BASE            (IO_PHYS + 0x0000F000)
#define MCUSYS_CFGREG_BASE        (IO_PHYS + 0x00200000)
#define INT_POL_CTL0              (MCUSYS_CFGREG_BASE + 0x100)
#define EMI_BASE                  (IO_PHYS + 0x00203000)
#define GPIO1_BASE                (IO_PHYS + 0x0020C000)
#define GIC_DIST_BASE             (IO_PHYS + 0x00211000)
#define GIC_CPU_BASE              (IO_PHYS + 0x00212000)
#define PERI_CON_BASE             (IO_PHYS + 0x01003000)
#define PERI_PWRAP_BRIDGE_BASE    (IO_PHYS + 0x01017000)
#define AUDIO_BASE                (IO_PHYS + 0x02071000)
#define MMSYS1_CONFIG_BASE        (IO_PHYS + 0x02080000)
#define MMSYS2_CONFG_BASE         (IO_PHYS + 0x020C0000)
#define APMIXED_BASE              (IO_PHYS + 0x00007000)

/**************************************************
 *                   F002 0000                    *
 **************************************************/
#define CIRQ_BASE        	(IO_PHYS + 0x00021000)
#define DMA_BASE         	(IO_PHYS + 0x00022000)

#define AP_DMA_BASE      	(IO_PHYS + 0x01000000)


#define UART1_BASE 		    (IO_PHYS + 0x01006000)
#define UART2_BASE 		    (IO_PHYS + 0x01007000)
#define UART3_BASE 		    (IO_PHYS + 0x01008000)
#define UART4_BASE 		    (IO_PHYS + 0x01009000)
#define APMCU_GPTIMER_BASE  (IO_PHYS + 0x00008000)

#define HDQ_BASE         	(IO_PHYS + 0x00027000)
#define KP_BASE         	(IO_PHYS + 0x00015000)
//#define RTC_BASE         	(IO_PHYS + 0x01003000)
#define RTC_BASE 			(0xE000)
#define SEJ_BASE         	(IO_PHYS + 0x0002D000)
#define IRDA_BASE       	(IO_PHYS + 0x0002F000) //(IO_PHYS + 0x00037000)
#define MSDC0_BASE          (IO_PHYS + 0x01230000)
#define MSDC1_BASE          (IO_PHYS + 0x01240000)
#define MSDC2_BASE          (IO_PHYS + 0x01250000)
#define MSDC3_BASE          (IO_PHYS + 0x01260000)
#define MSDC4_BASE          (IO_PHYS + 0x01270000)

#define MIXEDSYS0_BASE 		(IO_PHYS + 0x0002E000) //

/**************************************************
 *                   F003 0000                    *
 **************************************************/
#define I2C0_BASE         	(IO_PHYS + 0x0100D000) 
#define I2C1_BASE         	(IO_PHYS + 0x0100E000)
#define I2C2_BASE         	(IO_PHYS + 0x0100F000) 
#define I2C3_BASE         	(IO_PHYS + 0x01010000)
#define I2C4_BASE         	(IO_PHYS + 0x01011000)
#define I2C5_BASE         	(IO_PHYS + 0x01012000)
#define I2C6_BASE         	(IO_PHYS + 0x01013000)
#define NFI_BASE         	(IO_PHYS + 0x01001000)
#define NFIECC_BASE	        (IO_PHYS + 0x01002000)
#define PWM_BASE         	(IO_PHYS + 0x0100B000)
#define SIM_BASE 	      	(IO_PHYS + 0x00033000)
#define CCIF_BASE        	(IO_PHYS + 0x00036000)
#define AMCONFG_BASE     	(IO_PHYS + 0x00039000)
#define AP2MD_BASE	     	(IO_PHYS + 0x0003A000)
#define APVFE_BASE	     	(IO_PHYS + 0x0003B000)
#define APSLP_BASE	     	(IO_PHYS + 0x0003C000)
#define AUXADC_BASE	     	(IO_PHYS + 0x0003D000) // (IO_PHYS + 0x00034000)

/**************************************************
 *                   F004 0000                    *
 **************************************************/
#define CSDBG_BASE			(IO_PHYS + 0x00040000)

/**************************************************
 *                   F006 0000                    *
 **************************************************/
#define PLL_BASE        	(IO_PHYS + 0x00060000) // same
#define DSI_PHY_BASE            (IO_PHYS + 0x00060B00)
#define PMU_BASE        	(IO_PHYS + 0x00061300) // Jau leave the define for short

/**************************************************
 *                   F007 0000                    *
 **************************************************/
/**************************************************
 *                   F008 0000                    *
 **************************************************/
#define GMC1_BASE			(IO_PHYS + 0x00080000)
#define G2D_BASE			(IO_PHYS + 0x00081000)
#define GCMQ_BASE			(IO_PHYS + 0x00082000)
#define GIFDEC_BASE			(IO_PHYS + 0x00083000)
#define IMGDMA_BASE			(IO_PHYS + 0x00084000)
#define PNGDEC_BASE			(IO_PHYS + 0x00085000)
#define MTVSPI_BASE			(IO_PHYS + 0x00087000)
#define TVCON_BASE			(IO_PHYS + 0x00088000)
#define TVENC_BASE			(IO_PHYS + 0x00089000)
#define CAM_BASE			(IO_PHYS + 0x0008A000)
#define CAM_ISP_BASE		        (IO_PHYS + 0x0008B000)
#define BLS_BASE			(IO_PHYS + 0x0008C000)
#define CRZ_BASE			(IO_PHYS + 0x0008D000)
#define DRZ_BASE			(IO_PHYS + 0x0008E000)
#define ASM_BASE			(IO_PHYS + 0x0008F000)

/**************************************************
 *                   F009 0000                    *
 **************************************************/
#define WT_BASE				 (IO_PHYS + 0x00090000)
#define IMG_BASE			 (IO_PHYS + 0x00091000)
#define MMSYS1_CONFIG_BASE             (IO_PHYS + 0x02080000)

/**************************************************
 *                   F00A 0000                    *
 **************************************************/
#define GMC2_BASE			 (IO_PHYS + 0x000A0000)
#define JPEG_BASE			 (IO_PHYS + 0x000A1000)
#define M3D_BASE			 (IO_PHYS + 0x000A2000)
#define PRZ_BASE			 (IO_PHYS + 0x000A3000)
#define IMGDMA1_BASE		         (IO_PHYS + 0x000A4000)
#define MP4_DEBLK_BASE		         (IO_PHYS + 0x000A5000)
#define FAKE_ENG2_BASE		         (IO_PHYS + 0x000A6000)
#define GRAPH2SYS_BASE		         (IO_PHYS + 0x000A7000)

/**************************************************
 *                   F00C 0000                    *
 **************************************************/
 #define MP4_BASE			 (IO_PHYS + 0x000C0000)
 #define H264_BASE			 (IO_PHYS + 0x000C1000)

/**************************************************
 *                   F010 0000                    *
 **************************************************/
#define USB0_BASE           (IO_PHYS + 0x01200000)
#define USB1_BASE           (IO_PHYS + 0x01210000)
#define USBSIF_BASE         (IO_PHYS + 0x01220000)
#define USB_BASE            (USB0_BASE)


/**************************************************
 *                   F012 0000                    *
 **************************************************/
//#define LCD_BASE           	(IO_PHYS + 0x020A1000)
#define LCD_BASE           	(IO_PHYS + 0x0400C000)
#define MIPI_CONFIG_BASE            (IO_PHYS+0x00012000)
#define APMIXEDSYS_BASE             (IO_PHYS+0x00007000)



/* disp subsys register */
#define DISPSYS_BASE          (IO_PHYS + 0x04000000)
#define ROT_BASE              (IO_PHYS + 0x04001000)
#define SCL_BASE              (IO_PHYS + 0x04002000)
#define OVL_BASE              (IO_PHYS + 0x04003000)
#define WDMA0_BASE            (IO_PHYS + 0x04004000)
#define WDMA1_BASE            (IO_PHYS + 0x04005000)
#define RDMA0_BASE            (IO_PHYS + 0x04006000)
#define RDMA1_BASE            (IO_PHYS + 0x04007000)
#define BLS_BASE              (IO_PHYS + 0x04008000)
#define GAMMA_BASE            (IO_PHYS + 0x04009000)
#define COLOR_BASE            (IO_PHYS + 0x0400A000)
#define TDSHP_BASE            (IO_PHYS + 0x0400B000)
#define LCD_BASE              (IO_PHYS + 0x0400C000)
#define DSI_BASE              (IO_PHYS + 0x0400D000)
#define DPI_BASE              (IO_PHYS + 0x0400E000)
#define SMILARB2_BASE         (IO_PHYS + 0x04010000)
#define DISP_MUTEX_BASE       (IO_PHYS + 0x04011000)
#define DISP_CMDQ_BASE        (IO_PHYS + 0x04012000)

#define MIPI_CONFG_BASE 	(IO_PHYS + 0x020A3000)


#define INFRA_SYS_CFG_BASE	(IO_PHYS + 0x00001000)

#define APMIXED_BASE		(IO_PHYS + 0x00007000)

#define MMSYS2_CONFG_BASE   (IO_PHYS + 0x020C0000)

#define SMI_LARB0_BASE      (IO_PHYS + 0x02081000)
#define SMI_LARB1_BASE      (IO_PHYS + 0x02082000)
#define SMI_LARB2_BASE      (IO_PHYS + 0x02083000)
#define SMI_LARB3_BASE      (IO_PHYS + 0x020C1000)

/* hardware version register */
#define VER_BASE 0xF8000000
#define APHW_CODE           (VER_BASE)
#define APHW_SUBCODE        (VER_BASE + 0x04)
#define APHW_VER            (VER_BASE + 0x08)
#define APSW_VER            (VER_BASE + 0x0C)

/* EMI Registers */
#define EMI_CON0 					(EMI_BASE+0x0000) /* Bank 0 configuration */
#define EMI_CON1 					(EMI_BASE+0x0004) /* Bank 1 configuration */
#define EMI_CON2 					(EMI_BASE+0x0008) /* Bank 2 configuration */
#define EMI_CON3 					(EMI_BASE+0x000C) /* Bank 3 configuration */
#define EMI_CON4 					(EMI_BASE+0x0010) /* Boot Mapping config  */
#define	EMI_CON5 					(EMI_BASE+0x0014)
#define SDRAM_MODE 					(EMI_BASE+0x0020)
#define SDRAM_COMD 					(EMI_BASE+0x0024)
#define SDRAM_SET 					(EMI_BASE+0x0028)
#define SDRAM_BASE					0x00000000

/* AHB MCU-DSP SHARE RAM */
#define MCU_DSP_SHARE_RAM_BASE		(0xA0000000)
#define MCU_DSP_SHARE_RAM_SIZE      (0x01A0)        // 208*2
#define IDMA_PORT_BASE              (0xA2000000)



//----------------------------------------------------------------------------
/* Powerdown module watch dog timer registers */
#define REG_RW_WATCHDOG_EN  		0x0100      // Watchdog Timer Control Register
#define REG_RW_WATCHDOG_TMR 		0x0104      // Watchdog Timer Register
#define REG_RW_WAKE_RSTCNT  		0x0108      // Wakeup Reset Counter Register

#define PLL_CON5_REG            (MIXEDSYS0_BASE+0x114)



/* MTK EMI freq. definition */
#define EMI_52MHZ                   52000000
#define EMI_58_5MHZ                 58500000
#define EMI_104MHZ                  104000000
#define EMI_117MHZ                  117000000
#define EMI_130MHZ                  130000000

/* MTK clock definitions */
#define CPU_468MHZ_EMI_117MHZ       1
#define CPU_234MHZ_EMI_117MHZ       2
#define CPU_416MHZ_EMI_104MHZ       3
#define CPU_208MHZ_EMI_104MHZ       4
#define CPU_468MHZ_EMI_58_5MHZ      5
#define CPU_234MHZ_EMI_58_5MHZ      6
#define CPU_416MHZ_EMI_52MHZ        7
#define CPU_208MHZ_EMI_52MHZ        8
#define CPU_390MHZ_EMI_130MHZ       9

/* MTK storage type */
#define DEV_UNKNOWN                 0
#define DEV_NAND                    1
#define DEV_NOR                     2
#define DEV_INAND                   3
#define DEV_MOVINAND                4
#define DEV_ENAND                   5
#define DEV_MMC_SD                  6

/* MT storage boot type definitions */
#define NON_BOOTABLE                0
#define RAW_BOOT                    1
#define FAT_BOOT                    2

#define CONFIG_STACKSIZE	    (128*1024)	  /* regular stack */

// xuecheng, define this because we use zlib for boot logo compression
#define CONFIG_ZLIB 	1

// =======================================================================
// UBOOT DEBUG CONTROL
// =======================================================================
#define UBOOT_DEBUG_TRACER			(0)

/*MTK Memory layout configuration*/
#define MAX_NR_BANK    4

#define DRAM_PHY_ADDR   0x80000000

#if 0
#if defined (MODEM_3G)
 #define RIL_SIZE		0x1600000
#elif defined (MODEM_2G)
 #define RIL_SIZE		0x0A00000
#else
 #define RIL_SIZE		0x1600000
#endif
#endif
#define RIL_SIZE	0x0

#define CFG_RAMDISK_LOAD_ADDR           (DRAM_PHY_ADDR + RIL_SIZE + 0x4000000)
#define CFG_BOOTIMG_LOAD_ADDR           (DRAM_PHY_ADDR + RIL_SIZE + 0x8000)
#define CFG_BOOTARGS_ADDR               (DRAM_PHY_ADDR + RIL_SIZE + 0x100)

/*Command passing to Kernel */
#ifdef MACH_FPGA
#define COMMANDLINE_TO_KERNEL  "console=tty0 console=ttyMT0,921600n1 root=/dev/ram"
#else
#define COMMANDLINE_TO_KERNEL  "console=tty0 console=ttyMT3,921600n1 root=/dev/ram"
#endif
#define CFG_FACTORY_NAME	"factory.img"
/* Vanzo:lubaoquan on: Tue, 02 Apr 2013 22:13:43 +0800
 * TODO: disable lk text menu in user build
#define HAVE_LK_TEXT_MENU
 */
#ifndef USER_BUILD
#define HAVE_LK_TEXT_MENU
#endif

// End of Vanzo: lubaoquan

#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
#define EFUSE_CTR_BASE		0xC1019000	/* IO_PHYS + 0x01019000 */
#define PDN_CTR			0xC0001020	/* IO_PHYS + 0x00001020 */
#define SERIAL_KEY_HI		(EFUSE_CTR_BASE + 0x0144)
#define SERIAL_KEY_LO		(EFUSE_CTR_BASE + 0x0140)
#endif

//ALPS00427972, implement the analog register formula
//Add here for eFuse, chip version checking -> analog register calibration
#define M_HW_RES3	                    0x10009170
//#define M_HW_RES3_PHY                   IO_PHYS+M_HW_RES3
#define RG_USB20_TERM_VREF_SEL_MASK     0xE000      //0b 1110,0000,0000,0000     15~13
#define RG_USB20_CLKREF_REF_MASK        0x1C00      //0b 0001,1100,0000,0000     12~10
#define RG_USB20_VRT_VREF_SEL_MASK      0x0380      //0b 0000,0011,1000,0000     9~7
//ALPS00427972, implement the analog register formula

#endif

