#ifndef _MT_DCM_H
#define _MT_DCM_H

#include "mach/mt_reg_base.h"



//#define CAM_BASE                	0xF5004000//0x15004000

// APB Module usb2
#if 1 //92
#define	USB0_DCM					(USB_BASE+0x700)
#else
#define	PERI_USB0_DCM		    	(USB_BASE+0x700)
#endif

// APB Module msdc
#define MSDC0_IP_DCM				(MSDC_0_BASE + 0x00B4)

// APB Module msdc
#define MSDC1_IP_DCM				(MSDC_1_BASE + 0x00B4)

// APB Module msdc
#define MSDC2_IP_DCM				(MSDC_2_BASE + 0x00B4)

// APB Module msdc
#define MSDC3_IP_DCM				(MSDC_3_BASE + 0x00B4)

// APB Module pmic_wrap
#define PMIC_WRAP_DCM_EN			(PWRAP_BASE+0x13C)

// APB Module i2c
#define I2C0_I2CREG_HW_CG_EN		((I2C0_BASE+0x054))

// APB Module i2c
#define I2C1_I2CREG_HW_CG_EN		((I2C1_BASE+0x054))

// APB Module i2c
#define I2C2_I2CREG_HW_CG_EN		((I2C2_BASE+0x054))


//MJC
#define MJC_HW_DCM_DIS        	0xF7000010
#define MJC_HW_DCM_DIS_SET    	0xF7000014
#define MJC_HW_DCM_DIS_CLR    	0xF7000018


//CPUSYS_dcm
#if 1//92
#define L2C_SRAM_CTRL 			(MCUSYS_CFGREG_BASE + 0x0248) //0x10200248
#define CCI_CLK_CTRL 			(MCUSYS_CFGREG_BASE + 0x0270) //0x10200270
#else
#define CA7_MISC_CONFIG			(MCUSYS_CFGREG_BASE + 0x005C) // check 82
#define MCU_BIU_CON				(MCUSYS_CFGREG_BASE + 0x8000)
#endif

// AXI bus dcm
//TOPCKGen_dcm
#define DCM_CFG                 (INFRA_BASE + 0x0004)//DCM_CFG
//#define CLK_SCP_CFG_0			(INFRA_BASE + 0x0200)//82 Y,89 N
//#define CLK_SCP_CFG_1			(INFRA_BASE + 0x0204)//82 Y,89 N

//CA7 DCM
#if 1 //92
#define CA7_CKDIV1				(INFRACFG_AO_BASE + 0x008) //Only 82 support
#define INFRA_TOPCKGEN_DCMCTL   (INFRACFG_AO_BASE + 0x0010) //INFRA_TOPCKGEN_DCMCTL
#define INFRA_TOPCKGEN_DCMDBC   (INFRACFG_AO_BASE + 0x0014) //INFRA_TOPCKGEN_DCMDBC

#else
#define TOP_CKDIV1				(INFRACFG_AO_BASE + 0x008) //Only 82 support
#define TOP_DCMCTL              (INFRACFG_AO_BASE + 0x0010) //INFRA_TOPCKGEN_DCMCTL
#define TOP_DCMDBC              (INFRACFG_AO_BASE + 0x0014) //INFRA_TOPCKGEN_DCMDBC
#endif
//82 N,89 Y
//#define TOP_CA7DCMFSEL          (INFRA_DCM_BASE + 0x0018)//INFRA_TOPCKGEN_DCMFSEL

//infra dcm
#if 1 //92
#define INFRA_GLOBALCON_DCMCTL  (INFRACFG_AO_BASE + 0x0050) //INFRA_GLOBALCON_DCMCTL
#define INFRA_GLOBALCON_DCMDBC  (INFRACFG_AO_BASE + 0x0054) //INFRA_GLOBALCON_DCMDBC
#define INFRA_GLOBALCON_DCMSEL  (INFRACFG_AO_BASE + 0x0058) //INFRA_GLOBALCON_DCMFSEL
#else
#define INFRA_DCMCTL            (INFRACFG_AO_BASE + 0x0050) //INFRA_GLOBALCON_DCMCTL
#define INFRA_DCMDBC            (INFRACFG_AO_BASE + 0x0054) //INFRA_GLOBALCON_DCMDBC
#define INFRA_DCMFSEL           (INFRACFG_AO_BASE + 0x0058) //INFRA_GLOBALCON_DCMFSEL
#endif

//peri dcm
#define PERI_GLOBALCON_DCMCTL        (PERICFG_BASE + 0x0050) //PERI_GLOBALCON_DCMCTL
#define PERI_GLOBALCON_DCMDBC        (PERICFG_BASE + 0x0054) //PERI_GLOBALCON_DCMDBC
#define PERI_GLOBALCON_DCMFSEL       (PERICFG_BASE + 0x0058) //PERI_GLOBALCON_DCMFSEL


#define DRAMC_PD_CTRL           (DRAMC0_BASE + 0x01DC) //not found in 82 API.c

//m4u dcm
#define MMU_DCM					(SMI_MMU_TOP_BASE+0x5f0)//check 82

//smi_common dcm
//#define SMI_COMMON_DCM          0x10202300 //HW_DCM API_17


//Smi_common dcm
#define SMI_DCM_CONTROL				0xF4011300



// APB Module smi
//Smi_secure dcm

#if 1 //92
#define SMI_CON						(SMI1_BASE+0x010)//SMI_CON
#define SMI_CON_SET					(SMI1_BASE+0x014)//SMI_CON_SET
#define SMI_CON_CLR					(SMI1_BASE+0x018)//SMI_CON_CLR
#else
#define SMI_COMMON_AO_SMI_CON		(SMI1_BASE+0x010)//SMI_CON
#define SMI_COMMON_AO_SMI_CON_SET	(SMI1_BASE+0x014)//SMI_CON_SET
#define SMI_COMMON_AO_SMI_CON_CLR	(SMI1_BASE+0x018)//SMI_CON_CLR
#endif


// APB Module smi_larb
#if 1  //92
#define SMI_LARB0_STA        	(SMI_LARB0_BASE + 0x00)//SMI_LARB0_STAT
#define SMI_LARB0_CON        	(SMI_LARB0_BASE + 0x10)//SMI_LARB0_CON
#define SMI_LARB0_CON_SET       (SMI_LARB0_BASE + 0x14)//SMI_LARB0_CON_SET
#define SMI_LARB0_CON_CLR       (SMI_LARB0_BASE + 0x18)//SMI_LARB0_CON_CLR

#define SMI_LARB1_STAT        	(SMI_LARB1_BASE + 0x00)//SMI_LARB1_STAT
#define SMI_LARB1_CON        	(SMI_LARB1_BASE + 0x10)//SMI_LARB1_CON
#define SMI_LARB1_CON_SET       (SMI_LARB1_BASE + 0x14)//SMI_LARB1_CON_SET
#define SMI_LARB1_CON_CLR       (SMI_LARB1_BASE + 0x18)//SMI_LARB1_CON_CLR

#define SMI_LARB2_STAT        	(SMI_LARB2_BASE + 0x00)//SMI_LARB2_STAT
#define SMI_LARB2_CON        	(SMI_LARB2_BASE + 0x10)//SMI_LARB2_CON
#define SMI_LARB2_CON_SET       (SMI_LARB2_BASE + 0x14)//SMI_LARB2_CON_SET
#define SMI_LARB2_CON_CLR       (SMI_LARB2_BASE + 0x18)//SMI_LARB2_CON_CLR
#else
#define SMILARB0_DCM_STA        (SMI_LARB0_BASE + 0x00)//SMI_LARB0_STAT
#define SMILARB0_DCM_CON        (SMI_LARB0_BASE + 0x10)//SMI_LARB0_CON
#define SMILARB0_DCM_SET        (SMI_LARB0_BASE + 0x14)//SMI_LARB0_CON_SET
#define SMILARB0_DCM_CLR        (SMI_LARB0_BASE + 0x18)//SMI_LARB0_CON_CLR

#define SMILARB1_DCM_STA        (SMI_LARB1_BASE + 0x00)//SMI_LARB1_STAT
#define SMILARB1_DCM_CON        (SMI_LARB1_BASE + 0x10)//SMI_LARB1_CON
#define SMILARB1_DCM_SET        (SMI_LARB1_BASE + 0x14)//SMI_LARB1_CON_SET
#define SMILARB1_DCM_CLR        (SMI_LARB1_BASE + 0x18)//SMI_LARB1_CON_CLR

#define SMILARB2_DCM_STA        (SMI_LARB3_BASE + 0x00)//SMI_LARB2_STAT
#define SMILARB2_DCM_CON        (SMI_LARB3_BASE + 0x10)//SMI_LARB2_CON
#define SMILARB2_DCM_SET        (SMI_LARB3_BASE + 0x14)//SMI_LARB2_CON_SET
#define SMILARB2_DCM_CLR        (SMI_LARB3_BASE + 0x18)//SMI_LARB2_CON_CLR
#endif

//MFG
//MFG_DCM
// APB Module mfg_top
#define MFG_DCM_CON_0            (G3D_CONFIG_BASE + 0x10) //MFG_DCM_CON_0
#if 1 //92
#define CAM_CTL_RAW_DCM_DIS         (CAM0_BASE + 0x190)//CAM_CTL_RAW_DCM_DIS
#define CAM_CTL_RGB_DCM_DIS         (CAM0_BASE + 0x194)//CAM_CTL_RGB_DCM_DIS
#define CAM_CTL_YUV_DCM_DIS         (CAM0_BASE + 0x198)//CAM_CTL_YUV_DCM_DIS
#define CAM_CTL_CDP_DCM_DIS         (CAM0_BASE + 0x19C)//CAM_CTL_CDP_DCM_DIS
#define CAM_CTL_DMA_DCM_DIS			(CAM0_BASE + 0x1B0)//CAM_CTL_DMA_DCM_DIS

#define CAM_CTL_RAW_DCM_STATUS     (CAM0_BASE + 0x1A0)//CAM_CTL_RAW_DCM_STATUS
#define CAM_CTL_RGB_DCM_STATUS     (CAM0_BASE + 0x1A4)//CAM_CTL_RGB_DCM_STATUS
#define CAM_CTL_YUV_DCM_STATUS     (CAM0_BASE + 0x1A8)//CAM_CTL_YUV_DCM_STATUS
#define CAM_CTL_CDP_DCM_STATUS     (CAM0_BASE + 0x1AC)//CAM_CTL_CDP_DCM_STATUS
#define CAM_CTL_DMA_DCM_STATUS     (CAM0_BASE + 0x1B4)//CAM_CTL_DMA_DCM_STATUS
#else
//smi_isp_dcm
#define CAM_CTL_RAW_DCM         (CAM_BASE + 0x190)//CAM_CTL_RAW_DCM_DIS
#define CAM_CTL_RGB_DCM         (CAM_BASE + 0x194)//CAM_CTL_RGB_DCM_DIS
#define CAM_CTL_YUV_DCM         (CAM_BASE + 0x198)//CAM_CTL_YUV_DCM_DIS
#define CAM_CTL_CDP_DCM         (CAM_BASE + 0x19C)//CAM_CTL_CDP_DCM_DIS
#define CAM_CTL_DMA_DCM			(CAM_BASE + 0x1B0)//CAM_CTL_DMA_DCM_DIS

#define CAM_CTL_RAW_DCM_STA     (CAM_BASE + 0x1A0)//CAM_CTL_RAW_DCM_STATUS
#define CAM_CTL_RGB_DCM_STA     (CAM_BASE + 0x1A4)//CAM_CTL_RGB_DCM_STATUS
#define CAM_CTL_YUV_DCM_STA     (CAM_BASE + 0x1A8)//CAM_CTL_YUV_DCM_STATUS
#define CAM_CTL_CDP_DCM_STA     (CAM_BASE + 0x1AC)//CAM_CTL_CDP_DCM_STATUS
#define CAM_CTL_DMA_DCM_STA     (CAM_BASE + 0x1B4)//CAM_CTL_DMA_DCM_STATUS
#endif

//#define JPGDEC_DCM_CTRL         0x15009300 //not found in 82 API.c
#define JPGENC_DCM_CTRL         (JPGENC_BASE + 0x300) //not found in 82 API.c

//#define SMI_ISP_COMMON_DCMCON   0x15003010  	//82 N 89 Y
//#define SMI_ISP_COMMON_DCMSET   0x15003014	//82 N 89 Y
//#define SMI_ISP_COMMON_DCMCLR   0x15003018	//82 N 89 Y

//display sys
//mmsys_dcm
// APB Module mmsys_config
#define MMSYS_HW_DCM_DIS0        (DISPSYS_BASE + 0x120)//MMSYS_HW_DCM_DIS0
#define MMSYS_HW_DCM_DIS_SET0    (DISPSYS_BASE + 0x124)//MMSYS_HW_DCM_DIS_SET0
#define MMSYS_HW_DCM_DIS_CLR0    (DISPSYS_BASE + 0x128)//MMSYS_HW_DCM_DIS_CLR0

#define MMSYS_HW_DCM_DIS1        (DISPSYS_BASE + 0x12C)//MMSYS_HW_DCM_DIS1
#define MMSYS_HW_DCM_DIS_SET1    (DISPSYS_BASE + 0x130)//MMSYS_HW_DCM_DIS_SET1
#define MMSYS_HW_DCM_DIS_CLR1    (DISPSYS_BASE + 0x134)//MMSYS_HW_DCM_DIS_CLR1

//venc sys
#define VENC_CE                 (VENC_BASE + 0xEC)//not found in 82 API.c
#define VENC_CLK_DCM_CTRL       (VENC_BASE + 0xF4)//not found in 82 API.c
#define VENC_CLK_CG_CTRL        (VENC_BASE + 0x94)//not found in 82 API.c
//#define VENC_MP4_DCM_CTRL       0x170026F0

//vdec
//VDEC_dcm
#define VDEC_DCM_CON            (VDEC_GCON_BASE + 0x18)// found in 82 API.c,VDECSYS




#define CPU_DCM                 (1U << 0)
#define IFR_DCM                 (1U << 1)
#define PER_DCM                 (1U << 2)
#define SMI_DCM                 (1U << 3)
#define MFG_DCM                 (1U << 4)
#define DIS_DCM                 (1U << 5)
#define ISP_DCM                 (1U << 6)
#define VDE_DCM                 (1U << 7)
//#define SMILARB_DCM				(1U << 8)
//#define TOPCKGEN_DCM			(1U << 8)
#define MJC_DCM					(1U << 8)
//#define ALL_DCM                 (CPU_DCM|IFR_DCM|PER_DCM|SMI_DCM|MFG_DCM|DIS_DCM|ISP_DCM|VDE_DCM|TOPCKGEN_DCM)
#define ALL_DCM                 (CPU_DCM|IFR_DCM|PER_DCM|SMI_DCM|MFG_DCM|DIS_DCM|ISP_DCM|VDE_DCM|MJC_DCM)
#define NR_DCMS                 (0x9)


//extern void dcm_get_status(unsigned int type);
extern void dcm_enable(unsigned int type);
extern void dcm_disable(unsigned int type);

extern void bus_dcm_enable(void);
extern void bus_dcm_disable(void);

extern void disable_infra_dcm(void);
extern void restore_infra_dcm(void);

extern void disable_peri_dcm(void);
extern void restore_peri_dcm(void);

extern void mt_dcm_init(void);

#endif
