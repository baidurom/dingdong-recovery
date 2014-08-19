#define MOD_NO_IN_1_DEVAPC                  16
#define MAX_TIMEOUT                         100
 

 
 typedef struct {
     const char      *device_name;
     bool            forbidden;
 } DEVICE_INFO;
 
  
  static DEVICE_INFO D_APC0_Devices[] = {
      {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",     FALSE},
     {"INFRA_AO_INFRASYS_CONFIG_REGS",           TRUE},
     {"INFRA_AO_FHCTL",                          TRUE},
     {"INFRA_AO_PERISYS_CONFIG_REGS",            TRUE},
     {"INFRA_AO_DRAM_CONTROLLER",                TRUE},
     {"INFRA_AO_GPIO_CONTROLLER",                FALSE},
     {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          TRUE},
     {"INFRA_AO_TOP_LEVEL_RESET_GENERATOR",      TRUE},
     {"INFRA_AO_GPT",                            TRUE},
     {"INFRA_AO_RSVD",                           TRUE},
     {"INFRA_AO_SEJ",                            TRUE},
     {"INFRA_AO_APMCU_EINT_CONTROLLER",          TRUE},
     {"INFRA_AO_SMI_CONTROL_REG1",               TRUE},
     {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          FALSE},
     {"INFRA_AO_DEVICE_APC_AO",                  TRUE},
     {"INFRA_AO_DDRPHY_CONTROL_REG",             TRUE},
     {"INFRA_AO_MIPI_CONTROL_REG",               TRUE},
     {"INFRA_AO_KPAD_CONTROL_REG",               TRUE},
     {"INFRASYS_MCUSYS_CONFIG_REG",              TRUE},
     {"INFRASYS_CONTROL_REG",                    TRUE},
     {"INFRASYS_BOOTROM/SRAM",                   TRUE},
     {"INFRASYS_EMI_BUS_INTERFACE",              FALSE},
     {"INFRASYS_SYSTEM_CIRQ",                    TRUE},
     {"INFRASYS_M4U_CONFIGURATION",              TRUE},
     {"INFRASYS_EFUSEC",                         FALSE},
     {"INFRASYS_DEVICE_APC_MONITOR",             TRUE},
     {"INFRASYS_MCU_BIU_CONFIGURATION",          TRUE},
     {"INFRASYS_AP_MIXED_CONTROL_REG",           TRUE},
     {"INFRASYS_CA7_AP_CCIF",                    FALSE},
     {"INFRASYS_CA7_MD_CCIF",                    FALSE},
     {"INFRASYS_GPIO1_CONTROLLER",               FALSE},
     {"INFRASYS_MBIST_CONTROL_REG",              TRUE},
     {"INFRASYS_DRAMC_NAO_REGION_REG",           TRUE},
     {"INFRASYS_TRNG",                           TRUE},
     {"DEGBUG CORESIGHT",                        TRUE},
     {"DMA",                                     TRUE},
     {"AUXADC",                                  FALSE},
     {"UART0",                                   TRUE},
     {"UART1",                                   TRUE},
     {"UART2",                                   TRUE},
     {"UART3",                                   TRUE},
     {"PWM",                                     TRUE},
     {"I2C0",                                    TRUE},
     {"I2C1",                                    TRUE},
     {"I2C2",                                    TRUE},
     {"SPI0",                                    TRUE},
     {"PTP",                                     TRUE},
     {"BTIF",                                    TRUE},
     {"NFI",                                     TRUE},
     {"NFI_ECC",                                 TRUE},
     {"NLI_ARBITER",                             TRUE},
     {"USB2.0",                                  FALSE},
     {"USBSIF",                                  FALSE},
     {"AUDIO",                                   FALSE},
     {"MSDC0",                                   TRUE},
     {"MSDC1",                                   TRUE},
     {"MSDC2",                                   TRUE},
     {"MSDC3",                                   TRUE},
     {"WCN_AHB_SLAVE",                           FALSE},
     {"MD_PERIPHERALS",                          TRUE},
     {"IMGSYS_CONFIG",                           TRUE},
     {"IMGSYS_SMI_LARB2",                        TRUE},
     {"IMGSYS_CAM0",                             TRUE},
     {"IMGSYS_CAM1",                             TRUE},
     {"IMGSYS_SENINF",                           TRUE},
     {"IMGSYS_VENC",                             TRUE},
     {"IMGSYS_JPGENC",                           TRUE},
     {"IMGSYS_MIPI_RX",                          TRUE},
     {"G3D_CONFIG",                              TRUE},
     {"MALI",                                    TRUE},
     {"MMSYS_CONFIG",                            TRUE},
     {"MDP_RDMA",                                TRUE},
     {"MDP_RSZ0",                                TRUE},
     {"MDP_RSZ1",                                TRUE},
     {"MDP_WDMA",                                TRUE},
     {"MDP_WROT",                                TRUE},
     {"MDP_TDSHP",                               TRUE},
     {"DISP_OVL",                                TRUE},
     {"DISP_RDMA",                               TRUE},
     {"DISP_WDMA",                               TRUE},
     {"DISP_BLS",                                TRUE},
     {"DISP_COLOR",                              TRUE},
     {"DSI",                                     TRUE},
     {"DPI",                                     TRUE},
     {"MM_MUTEX",                                TRUE},
     {"MM_CMDQ",                                 TRUE},
     {"SMI_LARB0",                               TRUE},
     {"SMI_COMMON",                              TRUE},
     {"VDECSYS_CONFIGURATION",                   TRUE},
     {"VDECSYS_SMI_LARB1",                       TRUE},
     {"VDEC",                                    TRUE},
     {"MJC",                                     TRUE},
     {NULL,                                      FALSE},
  };

 
 
 
#define SET_SINGLE_MODULE(apcnum, domnum, index, module, permission_control)     \
 {                                                                               \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_APC_##index) & ~(0x3 << (2 * module)), DEVAPC##apcnum##_D##domnum##_APC_##index); \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_APC_##index) | (permission_control << (2 * module)),DEVAPC##apcnum##_D##domnum##_APC_##index); \
 }                                                                               \
 
#define UNMASK_SINGLE_MODULE_IRQ(apcnum, domnum, module_index)                  \
 {                                                                               \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_VIO_MASK) & ~(module_index),      \
         DEVAPC##apcnum##_D##domnum##_VIO_MASK);                                 \
 }                                                                               \
 
#define CLEAR_SINGLE_VIO_STA(apcnum, domnum, module_index)                     \
 {                                                                               \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_VIO_STA) | (module_index),        \
         DEVAPC##apcnum##_D##domnum##_VIO_STA);                                  \
 }                                                                               \

 
