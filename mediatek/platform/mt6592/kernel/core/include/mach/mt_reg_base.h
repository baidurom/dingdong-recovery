/*
 * This file is generated automatically according to the design of silicon.
 * Don't modify it directly.
 */

#ifndef __MT_REG_BASE
#define __MT_REG_BASE

#if 1
// CCI400
 #define CCI400_BASE (0xF0390000)
//
// APB Module cksys
#define INFRA_BASE (0xF0000000)

// APB Module infracfg_ao
#define INFRACFG_AO_BASE (0xF0001000)

// APB Module fhctl
#define FHCTL_BASE (0xF0002000)

// APB Module pericfg
#define PERICFG_BASE (0xF0003000)

// APB Module dramc
#define DRAMC0_BASE (0xF0004000)

// APB Module gpio
#define GPIO_BASE (0xF0005000)

// APB Module sleep
#define SPM_BASE (0xF0006000)

// APB Module toprgu
#define TOPRGU_BASE (0xF0007000)
#define AP_RGU_BASE TOPRGU_BASE

// APB Module apxgpt
#define APMCU_GPTIMER_BASE (0xF0008000)

// APB Module rsvd
#define RSVD_BASE (0xF0009000)

// APB Module HACC
#define HACC_BASE (0xF000A000)

// APB Module ap_cirq_eint
#define AP_CIRQ_EINT (0xF000B000)

// APB Module ap_cirq_eint
#define EINT_BASE (0xF000B000)

// APB Module smi
#define SMI1_BASE (0xF000C000)

// APB Module pmic_wrap
#define PWRAP_BASE (0xF000D000)

// APB Module device_apc_ao
#define DEVAPC_AO_BASE (0xF000E000)

// APB Module ddrphy
#define DDRPHY_BASE (0xF000F000)

// APB Module vencpll
//#define VENCPLL_BASE (0xF000F000)   // 82
#define VENCPLL_BASE (0xF0209000)     // 82M

// APB Module mipi_tx_config
#define MIPI_CONFIG_BASE (0xF0010000)

// APB Module mipi_rx_ana
#define MIPI_RX_ANA_BASE (0xF0010800)

// APB Module kp
#define KP_BASE (0xF0011000)

// APB Module dbgapb
#define DEBUGTOP_BASE (0xF0100000)

// APB Module mcucfg
#define MCUSYS_CFGREG_BASE (0xF0200000)

// APB Module infracfg
#define INFRACFG_BASE (0xF0201000)

// APB Module sramrom
#define SRAMROM_BASE (0xF0202000)

// APB Module emi
#define EMI_BASE (0xF0203000)

// APB Module sys_cirq
#define SYS_CIRQ_BASE (0xF0204000)

// APB Module m4u
#define SMI_MMU_TOP_BASE (0xF0205000)

// APB Module nb_mmu
#define NB_MMU0_BASE (0xF0205200)

// APB Module nb_mmu
#define NB_MMU1_BASE (0xF0205800)

// APB Module efusec
#define EFUSEC_BASE (0xF0206000)

// APB Module device_apc
#define DEVAPC_BASE (0xF0207000)

// APB Module mcu_biu_cfg
#define MCU_BIU_BASE (0xF0208000)
#define BUS_DBG_TRACKER_BASE        (0xF0208000)

// APB Module apmixed
#define APMIXED_BASE    (0xF0209000)
#define APMIXEDSYS_BASE (0xF0209000)

// APB Module ccif
#define AP_CCIF_BASE (0xF020A000)

// APB Module ccif
#define MD_CCIF_BASE (0xF020B000)

// APB Module gpio1
#define GPIO1_BASE (0xF020C000)

// APB Module infra_mbist
#define INFRA_TOP_MBIST_CTRL_BASE (0xF020D000)

// APB Module dramc_conf_nao
#define DRAMC_NAO_BASE (0xF020E000)

// APB Module trng
#define TRNG_BASE (0xF020F000)

// APB Module ca9
#define CORTEXA7MP_BASE (0xF0210000)

// APB Module ap_dma
#define AP_DMA_BASE (0xF1000000)

// APB Module auxadc
#define AUXADC_BASE (0xF1001000)

// APB Module uart
#define UART1_BASE (0xF1002000)

// APB Module uart
#define UART2_BASE (0xF1003000)

// APB Module uart
#define UART3_BASE (0xF1004000)

// APB Module uart
#define UART4_BASE (0xF1005000)

// APB Module pwm
#define PWM_BASE (0xF1006000)

// APB Module i2c
#define I2C0_BASE (0xF1007000)

// APB Module i2c
#define I2C1_BASE (0xF1008000)

// APB Module i2c
#define I2C2_BASE (0xF1009000)

// APB Module spi
#define SPI0_BASE (0xF100A000)
#define SPI1_BASE (0xF100A000)   // no more SPI1

// APB Module therm_ctrl
#define THERMAL_BASE (0xF100B000)

// APB Module btif
#define BTIF_BASE (0xF100C000)

// APB Module nfi
#define NFI_BASE (0xF100D000)

// APB Module nfiecc_16bit
#define NFIECC_BASE (0xF100E000)

// APB Module nli_arb
#define NLI_ARB_BASE (0xF100F000)

// APB Module peri_pwrap_bridge
#define PERI_PWRAP_BRIDGE_BASE (0xF1017000)   // no more existed in 82M

// APB Module usb2
#define USB_BASE (0xF1200000)

// APB Module usb_sif
#define USB_SIF_BASE (0xF1210000)

// APB Module msdc
#define MSDC_0_BASE (0xF1230000)

// APB Module msdc
#define MSDC_1_BASE (0xF1240000)

// APB Module msdc
#define MSDC_2_BASE (0xF1250000)

// APB Module wcn_ahb
#define WCN_AHB_BASE (0xF1260000)

// APB Module msdc
#define MSDC_3_BASE  (0xF1260000)


// APB Module mfg_top
#define G3D_CONFIG_BASE (0xF3000000)

// APB Module mali
#define MALI_BASE (0xF3040000)

// APB Module mali_tb_cmd
#define MALI_TB_BASE (0xF307f000)

// APB Module mmsys_config
#define DISPSYS_BASE (0xF4000000)
#define MMSYS_CONFIG_BASE           0xF4000000

// APB Module mdp_rdma
#define MDP_RDMA_BASE (0xF4001000)

// APB Module mdp_rsz
#define MDP_RSZ0_BASE (0xF4002000)

// APB Module mdp_rsz
#define MDP_RSZ1_BASE (0xF4003000)

// APB Module disp_wdma
#define MDP_WDMA_BASE (0xF4004000)
// APB Module disp_wdma
#define WDMA1_BASE (0xF4004000)

// APB Module mdp_wrot
#define MDP_WROT_BASE (0xF4005000)

// APB Module mdp_tdshp
#define MDP_TDSHP_BASE (0xF4006000)

// APB Module ovl
#define DISP_OVL_BASE (0xF4007000)
// APB Module ovl
#define OVL0_BASE (0xF4007000)

// APB Module ovl
//#define OVL1_BASE (0xF4007000)

// APB Module disp_rdma
#define DISP_RDMA0_BASE (0xF4008000)
// APB Module disp_rdma
#define R_DMA0_BASE (0xF4008000)

#define DISP_UFOE_BASE              0xF4013000

// APB Module disp_wdma
#define DISP_WDMA_BASE (0xF4009000)
// APB Module disp_wdma
#define WDMA0_BASE (0xF4009000)

// APB Module disp_bls
#define DISP_BLS_BASE (0xF400A000)

// APB Module disp_color_config
#define DISP_COLOR_BASE (0xF400B000)

// APB Module dsi
#define DSI_BASE (0xF400C000)

// APB Module disp_dpi
#define DPI_BASE (0xF400D000)

// APB Module disp_mutex
#define MMSYS_MUTEX_BASE (0xF400E000)

// APB Module mm_cmdq
#define MMSYS_CMDQ_BASE (0xF400F000)

// APB Module smi_larb
#define SMI_LARB0_BASE (0xF4010000)

// APB Module smi
#define SMI_BASE (0xF4011000)

// APB module RDMA1
#define DISP_RDMA1_BASE             0xF4012000
#define R_DMA1_BASE                 0xF4012000


#define IMGSYS_CONFIG_BASE          0xF5000000

// APB Module smi_larb
#define SMI_LARB2_BASE              (0xF5001000)

#define CAM0_BASE                   0xF5004000
#define CAM1_BASE                   0xF5005000
#define SENINF_BASE                 0xF5008000

#define VENC_BASE                   0xF5009000
#define VENC_TOP_BASE               0xF5009000
#define JPGENC_BASE                 0xF500A000
#define MIPI_RX_CONFIG_BASE         0xF500C000
// APB Module fake_eng
#define FAKE_ENG_BASE (0xF5002000)

// APB Module mmu
//#define SMI_LARB4_MMU_BASE (0xF5002800)


// APB Module vdecsys_config
#define VDEC_GCON_BASE (0xF6000000)

// APB Module smi_larb
#define SMI_LARB1_BASE (0xF6010000)

// APB Module mmu
//#define SMI_LARB1_MMU_BASE (0xF6010800)

// APB Module vdtop
#define VDEC_BASE (0xF6020000)

// APB Module vdtop
#define VDTOP_BASE (0xF6020000)

// APB Module vld
#define VLD_BASE (0xF6021000)

// APB Module vld_top
#define VLD_TOP_BASE (0xF6021800)

// APB Module mc
#define MC_BASE (0xF6022000)

// APB Module avc_vld
#define AVC_VLD_BASE (0xF6023000)

// APB Module avc_mv
#define AVC_MV_BASE (0xF6024000)

// APB Module vdec_pp
#define VDEC_PP_BASE (0xF6025000)

// APB Module vp8_vld
#define VP8_VLD_BASE (0xF6026800)

// APB Module vp6
#define VP6_BASE (0xF6027000)

// APB Module vld2
#define VLD2_BASE (0xF6027800)

// APB Module mc_vmmu
#define MC_VMMU_BASE (0xF6028000)

// APB Module pp_vmmu
#define PP_VMMU_BASE (0xF6029000)

// APB Module MJCSYS
#define MJCSYS_CONFIG_BASE (0xF7000000)

// APB Module imgsys
#define IMGSYS_CONFG_BASE (0xF5000000)

// APB Module cam
#define CAMINF_BASE (0xF5000000)

// APB Module csi2
#define CSI2_BASE (0xF5000000)

// APB Module seninf_tg
#define SENINF_TG_BASE (0xF5000000)

// APB Module seninf_top
#define SENINF_TOP_BASE (0xF5000000)


// APB Module scam
#define SCAM_BASE (0xF5008000)

// APB Module ncsi2
#define NCSI2_BASE (0xF5008000)

// APB Module ccir656
#define CCIR656_BASE (0xF5000000)

// APB Module n3d_ctl
#define N3D_CTL_BASE (0xF5000000)

// APB Module fdvt
#define FDVT_BASE (0xF500B000)

// APB Module audiosys
#define AUDIO_BASE (0xF1221000)
#define AUDIO_REG_BASE (0xF1220000)

// CONNSYS
#define CONN_BTSYS_PKV_BASE (0xF8000000)
#define CONN_BTSYS_TIMCON_BASE (0xF8010000)
#define CONN_BTSYS_RF_CONTROL_BASE (0xF8020000)
#define CONN_BTSYS_MODEM_BASE (0xF8030000)
#define CONN_BTSYS_BT_CONFIG_BASE (0xF8040000)
#define CONN_MCU_CONFIG_BASE (0xF8070000)
#define CONN_TOP_CR_BASE (0xF80B0000)
#define CONN_HIF_CR_BASE (0xF80F0000)

/*
 * Addresses below are added manually.
 * They cannot be mapped via IO_VIRT_TO_PHYS().
 */

#define GIC_CPU_BASE (CORTEXA7MP_BASE + 0x2000)
#define GIC_DIST_BASE (CORTEXA7MP_BASE + 0x1000)
#define SYSRAM_BASE 0xF2000000  /* L2 cache shared RAM mapped to 0x00200000*/
#define DEVINFO_BASE 0xFA000000 /* mapped to 0x08000000*/
#define INTER_SRAM 0xF9000000   /* mapped to 0x00100000*/


#endif
#endif
