#ifndef _ISP_REG_H_
#define _ISP_REG_H_



// ------------------------- General Definitions --------------------------
//#define ISP_BASE_HW     0x15004000
#define ISP_BASE_HW     0x15000000
//#define ISP_BASE_RANGE  0x10000 
#define ISP_BASE_RANGE  0x6100 
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;
// ---------------------- CAM Bit Field Definitions -----------------------
/* start MT6589_000_cam_ctl.xml*/
typedef volatile union _CAM_REG_CTL_START_
{
    volatile struct
    {
        FIELD PASS2_START               : 1;
        FIELD PASS2B_START              : 1;
        FIELD rsv_2                     : 1;
        FIELD FMT_START                 : 1;
        FIELD PASS2C_START              : 1;
        FIELD CQ0_START                 : 1;
        FIELD CQ0B_START                : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_START;

typedef volatile union _CAM_REG_CTL_EN1_
{
    volatile struct
    {
        FIELD TG1_EN                    : 1;
        FIELD TG2_EN                    : 1;
        FIELD BIN_EN                    : 1;
        FIELD OB_EN                     : 1;
        FIELD rsv_4                     : 1;
        FIELD LSC_EN                    : 1;
        FIELD rsv_6                     : 1;
        FIELD BNR_EN                    : 1;
        FIELD rsv_8                     : 1;
        FIELD HRZ_EN                    : 1;
        FIELD rsv_10                    : 1;
        FIELD PGN_EN                    : 1;
        FIELD PAK_EN                    : 1;
        FIELD PAK2_EN                   : 1;
        FIELD rsv_14                    : 1;
        FIELD SGG_EN                    : 1;
        FIELD AF_EN                     : 1;
        FIELD FLK_EN                    : 1;
        FIELD AA_EN                     : 1;
        FIELD LCS_EN                    : 1;
        FIELD UNP_EN                    : 1;
        FIELD CFA_EN                    : 1;
        FIELD CCL_EN                    : 1;
        FIELD G2G_EN                    : 1;
        FIELD DGM_EN                    : 1;
        FIELD LCE_EN                    : 1;
        FIELD GGM_EN                    : 1;
        FIELD C02_EN                    : 1;
        FIELD MFB_EN                    : 1;
        FIELD C24_EN                    : 1;
        FIELD CAM_EN                    : 1;
        FIELD BIN2_EN                   : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN1;

typedef volatile union _CAM_REG_CTL_EN2_
{
    volatile struct
    {
        FIELD G2C_EN                    : 1;
        FIELD C42_EN                    : 1;
        FIELD NBC_EN                    : 1;
        FIELD PCA_EN                    : 1;
        FIELD SEEE_EN                   : 1;
        FIELD NR3D_EN                   : 1;
        FIELD rsv_6                     : 8;
        FIELD CQ0C_EN                   : 1;
        FIELD CQ0B_EN                   : 1;
        FIELD EIS_EN                    : 1;
        FIELD CDRZ_EN                   : 1;
        FIELD CURZ_EN                   : 1;
        FIELD rsv_19                    : 2;
        FIELD PRZ_EN                    : 1;
        FIELD rsv_22                    : 1;
        FIELD UV_CRSA_EN                : 1;
        FIELD FE_EN                     : 1;
        FIELD GDMA_EN                   : 1;
        FIELD FMT_EN                    : 1;
        FIELD CQ1_EN                    : 1;
        FIELD CQ2_EN                    : 1;
        FIELD CQ3_EN                    : 1;
        FIELD G2G2_EN                   : 1;
        FIELD CQ0_EN                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN2;

typedef volatile union _CAM_REG_CTL_DMA_EN_
{
    volatile struct
    {
        FIELD IMGO_EN                   : 1;
        FIELD LSCI_EN                   : 1;
        FIELD rsv_2                     : 1;
        FIELD ESFKO_EN                  : 1;
        FIELD rsv_4                     : 1;
        FIELD AAO_EN                    : 1;
        FIELD LCSO_EN                   : 1;
        FIELD IMGI_EN                   : 1;
        FIELD IMGCI_EN                  : 1;
        FIELD rsv_9                     : 1;
        FIELD IMG2O_EN                  : 1;
        FIELD FLKI_EN                   : 1;
        FIELD LCEI_EN                   : 1;
        FIELD VIPI_EN                   : 1;
        FIELD VIP2I_EN                  : 1;
        FIELD rsv_15                    : 4;
        FIELD VIDO_EN                   : 1;
        FIELD rsv_20                    : 1;
        FIELD DISPO_EN                  : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_EN;

typedef volatile union _CAM_REG_CTL_FMT_SEL_
{
    volatile struct
    {
        FIELD SCENARIO                  : 3;
        FIELD rsv_3                     : 1;
        FIELD SUB_MODE                  : 3;
        FIELD rsv_7                     : 1;
        FIELD CAM_IN_FMT                : 4;
        FIELD CAM_OUT_FMT               : 4;
        FIELD TG1_FMT                   : 3;
        FIELD rsv_19                    : 1;
        FIELD TG2_FMT                   : 3;
        FIELD rsv_23                    : 1;
        FIELD TWO_PIX                   : 1;
        FIELD TWO_PIX2                  : 1;
        FIELD TG1_SW                    : 2;
        FIELD TG2_SW                    : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FMT_SEL;

typedef volatile union _CAM_REG_CTL_SEL_
{
    volatile struct
    {
        FIELD CRZ_PRZ_MRG               : 1;
        FIELD GDMA_LINK                 : 1;
        FIELD CQ0_CONT                  : 1;
        FIELD CQ0B_CONT                 : 1;
        FIELD PASS1_DB_EN               : 1;
        FIELD PASS2_DB_EN               : 1;
        FIELD TDR_SEL                   : 1;
        FIELD CQ0C_IMGO_SEL             : 1;
        FIELD TG_SEL                    : 1;
        FIELD NR3D_DMA_SEL              : 1;
        FIELD STEREO_RGB_SEL            : 1;
        FIELD CQ0B_SEL                  : 1;
        FIELD DBG_SEL                   : 3;
        FIELD EIS_SEL                   : 1;
        FIELD EIS_RAW_SEL               : 1;
        FIELD CQ0_MODE                  : 1;
        FIELD CQ0_VR_SNAP_MODE          : 2;
        FIELD PRZ_OPT_SEL               : 2;
        FIELD CQ0C_IMG2O_SEL            : 1;
        FIELD DISP_VID_SEL              : 1;
        FIELD FE_SEL                    : 1;
        FIELD CQ0B_MODE                 : 1;
        FIELD CQ0B_VR_SNAP_MODE         : 2;
        FIELD CURZ_BORROW               : 1;
        FIELD RAW_PASS1                 : 1;
        FIELD JPG_PASS1                 : 1;
        FIELD CQ1_INT_SEL               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL;

typedef volatile union _CAM_REG_CTL_PIX_ID_
{
    volatile struct
    {
        FIELD PIX_ID                    : 2;
        FIELD TG_PIX_ID                 : 2;
        FIELD TG_PIX_ID_EN              : 1;
        FIELD BPC_TPIPE_EDGE_SEL         : 1;
        FIELD BPC_TPIPE_EDGE_SEL_EN      : 1;
        FIELD GDMA_IN_FMT_EN            : 1;
        FIELD GDMA_IN_FMT               : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_PIX_ID;

typedef volatile union _CAM_REG_CTL_INT_EN_
{
    volatile struct
    {
        FIELD VS1_EN                    : 1;
        FIELD TG1_EN1                   : 1;
        FIELD TG1_EN2                   : 1;
        FIELD EXPDON1_EN                : 1;
        FIELD TG1_ERR_EN                : 1;
        FIELD VS2_EN                    : 1;
        FIELD TG2_EN1                   : 1;
        FIELD TG2_EN2                   : 1;
        FIELD EXPDON2_EN                : 1;
        FIELD TG2_ERR_EN                : 1;
        FIELD PASS1_DON_EN              : 1;
        FIELD PASS1_TG2_DON_EN          : 1;
        FIELD SOF1_INT_EN               : 1;
        FIELD CQ_ERR_EN                 : 1;
        FIELD PASS2_DON_EN              : 1;
        FIELD TPIPE_DON_EN               : 1;
        FIELD AF_DON_EN                 : 1;
        FIELD FLK_DON_EN                : 1;
        FIELD FMT_DON_EN                : 1;
        FIELD CQ_DON_EN                 : 1;
        FIELD IMGO_ERR_EN               : 1;
        FIELD AAO_ERR_EN                : 1;
        FIELD LCSO_ERR_EN               : 1;
        FIELD IMG2O_ERR_EN              : 1;
        FIELD ESFKO_ERR_EN              : 1;
        FIELD FLK_ERR_EN                : 1;
        FIELD LSC_ERR_EN                : 1;
        FIELD DROP1_INT_EN              : 1;
        FIELD BPC_ERR_EN                : 1;
        FIELD LCE_ERR_EN                : 1;
        FIELD DMA_ERR_EN                : 1;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_EN;

typedef volatile union _CAM_REG_CTL_INT_STATUS_
{
    volatile struct
    {
        FIELD VS1_ST                    : 1;
        FIELD TG1_ST1                   : 1;
        FIELD TG1_ST2                   : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD VS2_ST                    : 1;
        FIELD TG2_ST1                   : 1;
        FIELD TG2_ST2                   : 1;
        FIELD EXPDON2_ST                : 1;
        FIELD TG2_ERR_ST                : 1;
        FIELD PASS1_TG1__DON_ST         : 1;
        FIELD PASS1_TG2_DON_ST          : 1;
        FIELD SOF1_INT_ST               : 1;
        FIELD CQ_ERR_ST                 : 1;
        FIELD PASS2_DON_ST              : 1;
        FIELD TPIPE_DON_ST               : 1;
        FIELD AF_DON_ST                 : 1;
        FIELD FLK_DON_ST                : 1;
        FIELD FMT_DON_ST                : 1;
        FIELD CQ_DON_ST                 : 1;
        FIELD IMGO_ERR_ST               : 1;
        FIELD AAO_ERR_ST                : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD IMG2O_ERR_ST              : 1;
        FIELD ESFKO_ERR_ST              : 1;
        FIELD FLK_ERR_ST                : 1;
        FIELD LSC_ERR_ST                : 1;
        FIELD DROP1_INT_ST              : 1;
        FIELD BPC_ERR_ST                : 1;
        FIELD LCE_ERR_ST                : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_STATUS;

typedef volatile union _CAM_REG_CTL_DMA_INT_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST              : 1;
        FIELD IMG2O_DONE_ST             : 1;
        FIELD AAO_DONE_ST               : 1;
        FIELD LCSO_DONE_ST              : 1;
        FIELD ESFKO_DONE_ST             : 1;
        FIELD DISPO_DONE_ST             : 1;
        FIELD VIDO_DONE_ST              : 1;
        FIELD CQ0_VR_SNAP_ST            : 1;
        FIELD CQ0_ERR_ST                : 1;
        FIELD CQ0_DONE_ST               : 1;
        FIELD SOF2_INT_ST               : 1;
        FIELD DROP2_INT_ST              : 1;
        FIELD TG1_GBERR_ST              : 1;
        FIELD TG2_GBERR_ST              : 1;
        FIELD CQ0C_DONE_ST              : 1;
        FIELD CQ0B_DONE_ST              : 1;
        FIELD IMGO_DONE_EN              : 1;
        FIELD IMG2O_DONE_EN             : 1;
        FIELD AAO_DONE_EN               : 1;
        FIELD LCSO_DONE_EN              : 1;
        FIELD ESFKO_DONE_EN             : 1;
        FIELD DISPO_DONE_EN             : 1;
        FIELD VIDO_DONE_EN              : 1;
        FIELD CQ0_VR_SNAP_EN            : 1;
        FIELD CQ0_ERR_EN                : 1;
        FIELD CQ0_DONE_EN               : 1;
        FIELD SOF2_INT_EN               : 1;
        FIELD DROP2_INT_EN              : 1;
        FIELD TG1_GBERR_EN              : 1;
        FIELD TG2_GBERR_EN              : 1;
        FIELD CQ0C_DONE_EN              : 1;
        FIELD CQ0B_DONE_EN              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_INT;

typedef volatile union _CAM_REG_CTL_INTB_EN_
{
    volatile struct
    {
        FIELD rsv_0                     : 13;
        FIELD CQ_ERR_EN                 : 1;
        FIELD PASS2B_DON_EN             : 1;
        FIELD TPIPE_DON_EN               : 1;
        FIELD rsv_16                    : 3;
        FIELD CQ_DON_EN                 : 1;
        FIELD IMGO_ERR_EN               : 1;
        FIELD rsv_21                    : 1;
        FIELD LCSO_ERR_EN               : 1;
        FIELD IMG2O_ERR_EN              : 1;
        FIELD rsv_24                    : 2;
        FIELD LSC_ERR_EN                : 1;
        FIELD rsv_27                    : 1;
        FIELD BPC_ERR_EN                : 1;
        FIELD LCE_ERR_EN                : 1;
        FIELD DMA_ERR_EN                : 1;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INTB_EN;

typedef volatile union _CAM_REG_CTL_INTB_STATUS_
{
    volatile struct
    {
        FIELD rsv_0                     : 13;
        FIELD CQ_ERR_ST                 : 1;
        FIELD PASS2_DON_ST              : 1;
        FIELD TPIPE_DON_ST               : 1;
        FIELD rsv_16                    : 3;
        FIELD CQ_DON_ST                 : 1;
        FIELD IMGO_ERR_ST               : 1;
        FIELD rsv_21                    : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD IMG2O_ERR_ST              : 1;
        FIELD rsv_24                    : 2;
        FIELD LSC_ERR_ST                : 1;
        FIELD rsv_27                    : 2;
        FIELD LCE_ERR_ST                : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INTB_STATUS;

typedef volatile union _CAM_REG_CTL_DMAB_INT_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST              : 1;
        FIELD IMG2O_DONE_ST             : 1;
        FIELD AAO_DONE_ST               : 1;
        FIELD LCSO_DONE_ST              : 1;
        FIELD ESFKO_DONE_ST             : 1;
        FIELD DISPO_DONE_ST             : 1;
        FIELD VIDO_DONE_ST              : 1;
        FIELD rsv_7                     : 9;
        FIELD IMGO_DONE_EN              : 1;
        FIELD IMG2O_DONE_EN             : 1;
        FIELD AAO_DONE_EN               : 1;
        FIELD LCSO_DONE_EN              : 1;
        FIELD ESFKO_DONE_EN             : 1;
        FIELD DISPO_DONE_EN             : 1;
        FIELD VIDO_DONE_EN              : 1;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMAB_INT;

typedef volatile union _CAM_REG_CTL_INTC_EN_
{
    volatile struct
    {
        FIELD rsv_0                     : 13;
        FIELD CQ_ERR_EN                 : 1;
        FIELD PASS2C_DON_EN             : 1;
        FIELD TPIPE_DON_EN               : 1;
        FIELD rsv_16                    : 3;
        FIELD CQ_DON_EN                 : 1;
        FIELD IMGO_ERR_EN               : 1;
        FIELD rsv_21                    : 1;
        FIELD LCSO_ERR_EN               : 1;
        FIELD IMG2O_ERR_EN              : 1;
        FIELD rsv_24                    : 2;
        FIELD LSC_ERR_EN                : 1;
        FIELD rsv_27                    : 1;
        FIELD BPC_ERR_EN                : 1;
        FIELD LCE_ERR_EN                : 1;
        FIELD DMA_ERR_EN                : 1;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INTC_EN;

typedef volatile union _CAM_REG_CTL_INTC_STATUS_
{
    volatile struct
    {
        FIELD rsv_0                     : 13;
        FIELD CQ_ERR_ST                 : 1;
        FIELD PASS2_DON_ST              : 1;
        FIELD TPIPE_DON_ST               : 1;
        FIELD rsv_16                    : 3;
        FIELD CQ_DON_ST                 : 1;
        FIELD IMGO_ERR_ST               : 1;
        FIELD rsv_21                    : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD IMG2O_ERR_ST              : 1;
        FIELD rsv_24                    : 2;
        FIELD LSC_ERR_ST                : 1;
        FIELD rsv_27                    : 1;
        FIELD BPC_ERR_ST                : 1;
        FIELD LCE_ERR_ST                : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INTC_STATUS;

typedef volatile union _CAM_REG_CTL_DMAC_INT_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST              : 1;
        FIELD IMG2O_DONE_ST             : 1;
        FIELD AAO_DONE_ST               : 1;
        FIELD LCSO_DONE_ST              : 1;
        FIELD ESFKO_DONE_ST             : 1;
        FIELD DISPO_DONE_ST             : 1;
        FIELD VIDO_DONE_ST              : 1;
        FIELD rsv_7                     : 9;
        FIELD IMGO_DONE_EN              : 1;
        FIELD IMG2O_DONE_EN             : 1;
        FIELD AAO_DONE_EN               : 1;
        FIELD LCSO_DONE_EN              : 1;
        FIELD ESFKO_DONE_EN             : 1;
        FIELD DISPO_DONE_EN             : 1;
        FIELD VIDO_DONE_EN              : 1;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMAC_INT;

typedef volatile union _CAM_REG_CTL_INT_STATUSX_
{
    volatile struct
    {
        FIELD VS1_ST                    : 1;
        FIELD TG1_ST1                   : 1;
        FIELD TG1_ST2                   : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD VS2_ST                    : 1;
        FIELD TG2_ST1                   : 1;
        FIELD TG2_ST2                   : 1;
        FIELD EXPDON2_ST                : 1;
        FIELD TG2_ERR_ST                : 1;
        FIELD PASS1_TG1__DON_ST         : 1;
        FIELD PASS1_TG2_DON_ST          : 1;
        FIELD rsv_12                    : 1;
        FIELD CQ_ERR_ST                 : 1;
        FIELD PASS2_DON_ST              : 1;
        FIELD TPIPE_DON_ST               : 1;
        FIELD AF_DON_ST                 : 1;
        FIELD FLK_DON_ST                : 1;
        FIELD FMT_DON_ST                : 1;
        FIELD CQ_DON_ST                 : 1;
        FIELD IMGO_ERR_ST               : 1;
        FIELD AAO_ERR_ST                : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD IMG2O_ERR_ST              : 1;
        FIELD ESFKO_ERR_ST              : 1;
        FIELD FLK_ERR_ST                : 1;
        FIELD LSC_ERR_ST                : 1;
        FIELD rsv_27                    : 1;
        FIELD BPC_ERR_ST                : 1;
        FIELD LCE_ERR_ST                : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_STATUSX;

typedef volatile union _CAM_REG_CTL_DMA_INTX_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST              : 1;
        FIELD IMG2O_DONE_ST             : 1;
        FIELD AAO_DONE_ST               : 1;
        FIELD LCSO_DONE_ST              : 1;
        FIELD ESFKO_DONE_ST             : 1;
        FIELD DISPO_DONE_ST             : 1;
        FIELD VIDO_DONE_ST              : 1;
        FIELD CQ0_VR_SNAP_ST            : 1;
        FIELD CQ0_ERR_ST                : 1;
        FIELD CQ0_DONE_ST               : 1;
        FIELD CQ_ERR_ST                 : 1;
        FIELD BUF_OVL_ST                : 1;
        FIELD TG1_GBERR_ST              : 1;
        FIELD TG2_GBERR_ST              : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_INTX;

typedef volatile union _CAM_REG_CTL_TPIPE_
{
    volatile struct
    {
        FIELD TPIPE_EDGE                 : 4;
        FIELD TPIPE_IRQ                  : 1;
        FIELD LAST_TPIPE                 : 1;
        FIELD IMGO_CROP_EN              : 1;
        FIELD IMG2O_CROP_EN             : 1;
        FIELD TPIPE_WIDTH                : 10;
        FIELD rsv_18                    : 2;
        FIELD TPIPE_HEIGHT               : 11;
        FIELD LCSO_CROP_EN              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TPIPE;

typedef volatile union _CAM_REG_CTL_TCM_EN_
{
    volatile struct
    {
        FIELD TCM_CTL_EN                : 1;
        FIELD TCM_IMGI_EN               : 1;
        FIELD TCM_IMGCI_EN              : 1;
        FIELD TCM_VIPI_EN               : 1;
        FIELD TCM_VIP2I_EN              : 1;
        FIELD TCM_FLKI_EN               : 1;
        FIELD TCM_LCEI_EN               : 1;
        FIELD TCM_LSCI_EN               : 1;
        FIELD TCM_IMGO_EN               : 1;
        FIELD TCM_IMG2O_EN              : 1;
        FIELD TCM_LCSO_EN               : 1;
        FIELD TCM_ESFKO_EN              : 1;
        FIELD TCM_AAO_EN                : 1;
        FIELD TCM_DISPO_EN              : 1;
        FIELD TCM_VIDO_EN               : 1;
        FIELD TCM_PRZ_EN                : 1;
        FIELD TCM_CDRZ_EN               : 1;
        FIELD TCM_CURZ_EN               : 1;
        FIELD TCM_BNR_EN                : 1;
        FIELD TCM_LCE_EN                : 1;
        FIELD TCM_UNP_EN                : 1;
        FIELD TCM_C02_EN                : 1;
        FIELD TCM_MFB_EN                : 1;
        FIELD TCM_LSC_EN                : 1;
        FIELD TCM_NR3D_EN               : 1;
        FIELD TCM_FE_EN                 : 1;
        FIELD rsv_26                    : 1;
        FIELD TCM_LOAD_EN               : 1;
        FIELD CQ_APB_2T                 : 1;
        FIELD TPIPE_STALL_EN             : 1;
        FIELD TPIPE_LINK                 : 1;
        FIELD TDR_EN                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TCM_EN;

typedef volatile union _CAM_REG_CTL_SRAM_CFG_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD RAW_SOF_DBG               : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SRAM_CFG;

typedef volatile union _CAM_REG_CTL_SW_CTL_
{
    volatile struct
    {
        FIELD SW_RST_Trig               : 1;
        FIELD SW_RST_ST                 : 1;
        FIELD HW_RST                    : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SW_CTL;

typedef volatile union _CAM_REG_CTL_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE0;

typedef volatile union _CAM_REG_CTL_SPARE1_
{
    volatile struct
    {
        FIELD CTL_SPARE1                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE1;

typedef volatile union _CAM_REG_CTL_SPARE2_
{
    volatile struct
    {
        FIELD CTL_SPARE2                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE2;

typedef volatile union _CAM_REG_CTL_SPARE3_
{
    volatile struct
    {
        FIELD CON_SCENARIO              : 3;
        FIELD REV1                      : 1;
        FIELD CON_SCENARIO_EN           : 1;
        FIELD CON_SUB_MODE_EN           : 1;
        FIELD EIS_DB_LD_SEL             : 1;
        FIELD INT_MRG                   : 1;
        FIELD CON_SUB_MODE              : 3;
        FIELD CTL_SPARE3                : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE3;

typedef volatile union _CAM_REG_CTL_MUX_SEL_
{
    volatile struct
    {
        FIELD PAK_SEL                   : 2;
        FIELD UNP_SEL                   : 1;
        FIELD AA_SEL                    : 1;
        FIELD LCS_SEL                   : 2;
        FIELD SGG_SEL                   : 2;
        FIELD BIN_SEL                   : 1;
        FIELD rsv_9                     : 1;
        FIELD C02_SEL                   : 2;
        FIELD G2G_SEL                   : 1;
        FIELD GGM_SEL                   : 1;
        FIELD YUV_SEL                   : 1;
        FIELD CDP_SEL                   : 1;
        FIELD NR3O_SEL                  : 1;
        FIELD rsv_17                    : 1;
        FIELD PAK_SEL_EN                : 1;
        FIELD UNP_SEL_EN                : 1;
        FIELD AA_SEL_EN                 : 1;
        FIELD LCS_SEL_EN                : 1;
        FIELD SGG_SEL_EN                : 1;
        FIELD BIN_SEL_EN                : 1;
        FIELD SOF_SEL_EN                : 1;
        FIELD C02_SEL_EN                : 1;
        FIELD G2G_SEL_EN                : 1;
        FIELD GGM_SEL_EN                : 1;
        FIELD YUV_SEL_EN                : 1;
        FIELD CDP_SEL_EN                : 1;
        FIELD NR3O_SEL_EN               : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_MUX_SEL;

typedef volatile union _CAM_REG_CTL_MUX_SEL2_
{
    volatile struct
    {
        FIELD CCL_SEL                   : 2;
        FIELD BIN_OUT_SEL               : 2;
        FIELD IMGO_MUX                  : 1;
        FIELD FLKI_SOF_SEL              : 1;
        FIELD IMG2O_MUX                 : 1;
        FIELD IMGCI_SOF_SEL             : 1;
        FIELD PASS1_DONE_MUX            : 5;
        FIELD PASS2_DONE_MUX            : 5;
        FIELD CCL_SEL_EN                : 1;
        FIELD BIN_OUT_SEL_EN            : 1;
        FIELD IMGO_MUX_EN               : 1;
        FIELD IMG2O_MUX_EN              : 1;
        FIELD IMGCI_SOF_SEL_EN          : 1;
        FIELD FLKI_SOF_SEL_EN           : 1;
        FIELD LCEI_SOF_SEL_EN           : 1;
        FIELD IMGI_MUX_EN               : 1;
        FIELD IMGCI_MUX_EN              : 1;
        FIELD LCEI_SOF_SEL              : 1;
        FIELD LSCI_SOF_SEL              : 1;
        FIELD LSCI_SOF_SEL_EN           : 1;
        FIELD PASS1_DONE_MUX_EN         : 1;
        FIELD PASS2_DONE_MUX_EN         : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_MUX_SEL2;

typedef volatile union _CAM_REG_CTL_SRAM_MUX_CFG_
{
    volatile struct
    {
        FIELD SRAM_MUX_SCENARIO         : 3;
        FIELD rsv_3                     : 1;
        FIELD SRAM_MUX_MODE             : 3;
        FIELD SRAM_MUX_TPIPE             : 1;
        FIELD SRAM_MUX_SET_EN           : 1;
        FIELD rsv_9                     : 1;
        FIELD IMGO_SOF_SEL              : 1;
        FIELD LCSO_SOF_SEL              : 1;
        FIELD ESFKO_SOF_SEL             : 1;
        FIELD AAO_SOF_SEL               : 1;
        FIELD RGB_SOF_SEL               : 1;
        FIELD rsv_15                    : 1;
        FIELD LCSO_SOF_SEL_EN           : 1;
        FIELD ESFKO_SOF_SEL_EN          : 1;
        FIELD AAO_SOF_SEL_EN            : 1;
        FIELD RGB_SOF_SEL_EN            : 1;
        FIELD GDMA_SEL_EN               : 1;
        FIELD GDMA_SYNC_EN              : 1;
        FIELD GDMA_REPEAT               : 1;
        FIELD GDMA_IMGCI_SEL            : 1;
        FIELD PREGAIN_SEL               : 1;
        FIELD CAM_OUT_FMT2_EN           : 1;
        FIELD CAM_OUT_FMT2              : 2;
        FIELD SGG_HRZ_SEL               : 1;
        FIELD rsv_29                    : 1;
        FIELD IMG2O_SOF_SEL             : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SRAM_MUX_CFG;

typedef volatile union _CAM_REG_CTL_EN1_SET_
{
    volatile struct
    {
        FIELD TG1_EN_SET                : 1;
        FIELD TG2_EN_SET                : 1;
        FIELD BIN_EN_SET                : 1;
        FIELD OB_EN_SET                 : 1;
        FIELD rsv_4                     : 1;
        FIELD LSC_EN_SET                : 1;
        FIELD rsv_6                     : 1;
        FIELD BNR_EN_SET                : 1;
        FIELD rsv_8                     : 1;
        FIELD HRZ_EN_SET                : 1;
        FIELD rsv_10                    : 1;
        FIELD PGN_EN_SET                : 1;
        FIELD PAK_EN_SET                : 1;
        FIELD PAK2_EN_SET               : 1;
        FIELD rsv_14                    : 1;
        FIELD SGG_EN_SET                : 1;
        FIELD AF_EN_SET                 : 1;
        FIELD FLK_EN_SET                : 1;
        FIELD AA_EN_SET                 : 1;
        FIELD LCS_EN_SET                : 1;
        FIELD UNP_EN_SET                : 1;
        FIELD CFA_EN_SET                : 1;
        FIELD CCL_EN_SET                : 1;
        FIELD G2G_EN_SET                : 1;
        FIELD DGM_EN_SET                : 1;
        FIELD LCE_EN_SET                : 1;
        FIELD GGM_EN_SET                : 1;
        FIELD C02_EN_SET                : 1;
        FIELD MFB_EN_SET                : 1;
        FIELD C24_EN_SET                : 1;
        FIELD CAM_EN_SET                : 1;
        FIELD BIN2_EN_SET               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN1_SET;

typedef volatile union _CAM_REG_CTL_EN1_CLR_
{
    volatile struct
    {
        FIELD TG1_EN_CLR                : 1;
        FIELD TG2_EN_CLR                : 1;
        FIELD BIN_EN_CLR                : 1;
        FIELD OB_EN_CLR                 : 1;
        FIELD rsv_4                     : 1;
        FIELD LSC_EN_CLR                : 1;
        FIELD rsv_6                     : 1;
        FIELD BNR_EN_CLR                : 1;
        FIELD rsv_8                     : 1;
        FIELD HRZ_EN_CLR                : 1;
        FIELD rsv_10                    : 1;
        FIELD PGN_EN_CLR                : 1;
        FIELD PAK_EN_CLR                : 1;
        FIELD PAK2_EN_CLR               : 1;
        FIELD rsv_14                    : 1;
        FIELD SGG_EN_CLR                : 1;
        FIELD AF_EN_CLR                 : 1;
        FIELD FLK_EN_CLR                : 1;
        FIELD AA_EN_CLR                 : 1;
        FIELD LCS_EN_CLR                : 1;
        FIELD UNP_EN_CLR                : 1;
        FIELD CFA_EN_CLR                : 1;
        FIELD CCL_EN_CLR                : 1;
        FIELD G2G_EN_CLR                : 1;
        FIELD DGM_EN_CLR                : 1;
        FIELD LCE_EN_CLR                : 1;
        FIELD GGM_EN_CLR                : 1;
        FIELD C02_EN_CLR                : 1;
        FIELD MFB_EN_CLR                : 1;
        FIELD C24_EN_CLR                : 1;
        FIELD CAM_EN_CLR                : 1;
        FIELD BIN2_EN_CLR               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN1_CLR;

typedef volatile union _CAM_REG_CTL_EN2_SET_
{
    volatile struct
    {
        FIELD G2C_EN_SET                : 1;
        FIELD C42_EN_SET                : 1;
        FIELD NBC_EN_SET                : 1;
        FIELD PCA_EN_SET                : 1;
        FIELD SEEE_EN_SET               : 1;
        FIELD NR3D_EN_SET               : 1;
        FIELD rsv_6                     : 8;
        FIELD CQ0C_EN_SET               : 1;
        FIELD CQ0B_EN_SET               : 1;
        FIELD EIS_EN_SET                : 1;
        FIELD CDRZ_EN_SET               : 1;
        FIELD CURZ_EN_SET               : 1;
        FIELD rsv_19                    : 2;
        FIELD PRZ_EN_SET                : 1;
        FIELD rsv_22                    : 1;
        FIELD UV_CRSA_EN_SET            : 1;
        FIELD FE_EN_SET                 : 1;
        FIELD GDMA_EN_SET               : 1;
        FIELD FMT_EN_SET                : 1;
        FIELD CQ1_EN_SET                : 1;
        FIELD CQ2_EN_SET                : 1;
        FIELD CQ3_EN_SET                : 1;
        FIELD G2G2_EN_SET               : 1;
        FIELD CQ0_EN_SET                : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN2_SET;

typedef volatile union _CAM_REG_CTL_EN2_CLR_
{
    volatile struct
    {
        FIELD G2C_EN_CLR                : 1;
        FIELD C42_EN_CLR                : 1;
        FIELD NBC_EN_CLR                : 1;
        FIELD PCA_EN_CLR                : 1;
        FIELD SEEE_EN_CLR               : 1;
        FIELD NR3D_EN_CLR               : 1;
        FIELD rsv_6                     : 8;
        FIELD CQ0C_EN_CLR               : 1;
        FIELD CQ0B_EN_CLR               : 1;
        FIELD EIS_EN_CLR                : 1;
        FIELD CDRZ_EN_CLR               : 1;
        FIELD CURZ_EN_CLR               : 1;
        FIELD rsv_19                    : 2;
        FIELD PRZ_EN_CLR                : 1;
        FIELD rsv_22                    : 1;
        FIELD UV_CRSA_EN_CLR            : 1;
        FIELD FE_EN_CLR                 : 1;
        FIELD GDMA_EN_CLR               : 1;
        FIELD FMT_EN_CLR                : 1;
        FIELD CQ1_EN_CLR                : 1;
        FIELD CQ2_EN_CLR                : 1;
        FIELD CQ3_EN_CLR                : 1;
        FIELD G2G2_EN_CLR               : 1;
        FIELD CQ0_EN_CLR                : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN2_CLR;

typedef volatile union _CAM_REG_CTL_DMA_EN_SET_
{
    volatile struct
    {
        FIELD IMGO_EN_SET               : 1;
        FIELD LSCI_EN_SET               : 1;
        FIELD rsv_2                     : 1;
        FIELD ESFKO_EN_SET              : 1;
        FIELD rsv_4                     : 1;
        FIELD AAO_EN_SET                : 1;
        FIELD LCSO_EN_SET               : 1;
        FIELD IMGI_EN_SET               : 1;
        FIELD IMGCI_EN_SET              : 1;
        FIELD rsv_9                     : 1;
        FIELD IMG2O_EN_SET              : 1;
        FIELD FLKI_EN_SET               : 1;
        FIELD LCEI_EN_SET               : 1;
        FIELD VIPI_EN_SET               : 1;
        FIELD VIP2I_EN_SET              : 1;
        FIELD rsv_15                    : 4;
        FIELD VIDO_EN_SET               : 1;
        FIELD rsv_20                    : 1;
        FIELD DISPO_EN_SET              : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_EN_SET;

typedef volatile union _CAM_REG_CTL_DMA_EN_CLR_
{
    volatile struct
    {
        FIELD IMGO_EN_CLR               : 1;
        FIELD LSCI_EN_CLR               : 1;
        FIELD rsv_2                     : 1;
        FIELD ESFKO_EN_CLR              : 1;
        FIELD rsv_4                     : 1;
        FIELD AAO_EN_CLR                : 1;
        FIELD LCSO_EN_CLR               : 1;
        FIELD IMGI_EN_CLR               : 1;
        FIELD IMGCI_EN_CLR              : 1;
        FIELD rsv_9                     : 1;
        FIELD IMG2O_EN_CLR              : 1;
        FIELD FLKI_EN_CLR               : 1;
        FIELD LCEI_EN_CLR               : 1;
        FIELD VIPI_EN_CLR               : 1;
        FIELD VIP2I_EN_CLR              : 1;
        FIELD rsv_15                    : 4;
        FIELD VIDO_EN_CLR               : 1;
        FIELD rsv_20                    : 1;
        FIELD DISPO_EN_CLR              : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_EN_CLR;

typedef volatile union _CAM_REG_CTL_FMT_SEL_SET_
{
    volatile struct
    {
        FIELD SCENARIO_SET              : 3;
        FIELD rsv_3                     : 1;
        FIELD SUB_MODE_SET              : 3;
        FIELD rsv_7                     : 1;
        FIELD CAM_IN_FMT_SET            : 4;
        FIELD CAM_OUT_FMT_SET           : 4;
        FIELD TG1_FMT_SET               : 3;
        FIELD rsv_19                    : 1;
        FIELD TG2_FMT_SET               : 3;
        FIELD rsv_23                    : 1;
        FIELD TWO_PIX_SET               : 1;
        FIELD TWO_PIX2_SET              : 1;
        FIELD TG1_SW_SET                : 2;
        FIELD TG2_SW_SET                : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FMT_SEL_SET;

typedef volatile union _CAM_REG_CTL_FMT_SEL_CLR_
{
    volatile struct
    {
        FIELD SCENARIO_CLR              : 3;
        FIELD rsv_3                     : 1;
        FIELD SUB_MODE_CLR              : 3;
        FIELD rsv_7                     : 1;
        FIELD CAM_IN_FMT_CLR            : 4;
        FIELD CAM_OUT_FMT_CLR           : 4;
        FIELD TG1_FMT_CLR               : 3;
        FIELD rsv_19                    : 1;
        FIELD TG2_FMT_CLR               : 3;
        FIELD rsv_23                    : 1;
        FIELD TWO_PIX_CLR               : 1;
        FIELD TWO_PIX2_CLR              : 1;
        FIELD TG1_SW_CLR                : 2;
        FIELD TG2_SW_CLR                : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FMT_SEL_CLR;

typedef volatile union _CAM_REG_CTL_SEL_SET_
{
    volatile struct
    {
        FIELD CRZ_PRZ_MRG_SET           : 1;
        FIELD GDMA_LINK_SET             : 1;
        FIELD CQ0_CONT_SET              : 1;
        FIELD CQ0B_CONT_SET             : 1;
        FIELD PASS1_DB_EN_SET           : 1;
        FIELD PASS2_DB_EN_SET           : 1;
        FIELD TDR_SEL_SET               : 1;
        FIELD CQ0C_IMGO_SEL_SET         : 1;
        FIELD TG_SEL_SET                : 1;
        FIELD NR3D_DMA_SEL_SET          : 1;
        FIELD STEREO_RGB_SEL_SET        : 1;
        FIELD CQ0B_SEL_SET              : 1;
        FIELD DBG_SEL_SET               : 3;
        FIELD EIS_SEL_SET               : 1;
        FIELD EIS_RAW_SEL_SET           : 1;
        FIELD CQ0_MODE_SET              : 1;
        FIELD CQ0_VR_SNAP_MODE_SET      : 2;
        FIELD PRZ_OPT_SEL_SET           : 2;
        FIELD CQ0C_IMG2O_SEL_SET        : 1;
        FIELD DISP_VID_SEL_SET          : 1;
        FIELD FE_SEL_SET                : 1;
        FIELD CQ0B_MODE_SET             : 1;
        FIELD CQ0B_VR_SNAP_MODE_SET     : 2;
        FIELD CURZ_BORROW_SET           : 1;
        FIELD RAW_PASS1_SET             : 1;
        FIELD JPG_PASS1_SET             : 1;
        FIELD CQ1_INT_SEL_SET           : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_SET;

typedef volatile union _CAM_REG_CTL_SEL_CLR_
{
    volatile struct
    {
        FIELD CRZ_PRZ_MRG_CLR           : 1;
        FIELD GDMA_LINK_CLR             : 1;
        FIELD CQ0_CONT_CLR              : 1;
        FIELD CQ0B_CONT_CLR             : 1;
        FIELD PASS1_DB_EN_CLR           : 1;
        FIELD PASS2_DB_EN_CLR           : 1;
        FIELD TDR_SEL_CLR               : 1;
        FIELD CQ0C_IMGO_SEL_CLR         : 1;
        FIELD TG_SEL_CLR                : 1;
        FIELD NR3D_DMA_SEL_CLR          : 1;
        FIELD STEREO_RGB_SEL_CLR        : 1;
        FIELD CQ0B_SEL_CLR              : 1;
        FIELD DBG_SEL_CLR               : 3;
        FIELD EIS_SEL_CLR               : 1;
        FIELD EIS_RAW_SEL_CLR           : 1;
        FIELD CQ0_MODE_CLR              : 1;
        FIELD CQ0_VR_SNAP_MODE_CLR      : 2;
        FIELD PRZ_OPT_SEL_CLR           : 2;
        FIELD CQ0C_IMG2O_SEL_CLR        : 1;
        FIELD DISP_VID_SEL_CLR          : 1;
        FIELD FE_SEL_CLR                : 1;
        FIELD CQ0B_MODE_CLR             : 1;
        FIELD CQ0B_VR_SNAP_MODE_CLR     : 2;
        FIELD CURZ_BORROW_CLR           : 1;
        FIELD RAW_PASS1_CLR             : 1;
        FIELD JPG_PASS1_CLR             : 1;
        FIELD CQ1_INT_SEL_CLR           : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_CLR;

typedef volatile union _CAM_REG_CTL_CQ0_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ0_BASEADDR          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ1_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ1_BASEADDR          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ1_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ2_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ2_BASEADDR          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ2_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ3_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ3_BASEADDR          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ3_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0B_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ0B_BASEADDR         : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0B_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0C_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ0C_BASEADDR         : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0C_BASEADDR;

typedef volatile union _CAM_REG_CTL_MUX_SEL_SET_
{
    volatile struct
    {
        FIELD PAK_SEL_SET               : 2;
        FIELD UNP_SEL_SET               : 1;
        FIELD AA_SEL_SET                : 1;
        FIELD LCS_SEL_SET               : 2;
        FIELD SGG_SEL_SET               : 2;
        FIELD BIN_SEL_SET               : 1;
        FIELD rsv_9                     : 1;
        FIELD C02_SEL_SET               : 2;
        FIELD G2G_SEL_SET               : 1;
        FIELD GGM_SEL_SET               : 1;
        FIELD YUV_SEL_SET               : 1;
        FIELD CDP_SEL_SET               : 1;
        FIELD NR3O_SEL_SET              : 1;
        FIELD rsv_17                    : 1;
        FIELD PAK_SEL_EN_SET            : 1;
        FIELD UNP_SEL_EN_SET            : 1;
        FIELD AA_SEL_EN_SET             : 1;
        FIELD LCS_SEL_EN_SET            : 1;
        FIELD SGG_SEL_EN_SET            : 1;
        FIELD BIN_SEL_EN_SET            : 1;
        FIELD SOF_SEL_EN_SET            : 1;
        FIELD C02_SEL_EN_SET            : 1;
        FIELD G2G_SEL_EN_SET            : 1;
        FIELD GGM_SEL_EN_SET            : 1;
        FIELD YUV_SEL_EN_SET            : 1;
        FIELD CDP_SEL_EN_SET            : 1;
        FIELD NR3O_SEL_EN_SET           : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_MUX_SEL_SET;

typedef volatile union _CAM_REG_CTL_MUX_SEL_CLR_
{
    volatile struct
    {
        FIELD PAK_SEL_CLR               : 2;
        FIELD UNP_SEL_CLR               : 1;
        FIELD AA_SEL_CLR                : 1;
        FIELD LCS_SEL_CLR               : 2;
        FIELD SGG_SEL_CLR               : 2;
        FIELD BIN_SEL_CLR               : 1;
        FIELD rsv_9                     : 1;
        FIELD C02_SEL_CLR               : 2;
        FIELD G2G_SEL_CLR               : 1;
        FIELD GGM_SEL_CLR               : 1;
        FIELD YUV_SEL_CLR               : 1;
        FIELD CDP_SEL_CLR               : 1;
        FIELD NR3O_SEL_CLR              : 1;
        FIELD rsv_17                    : 1;
        FIELD PAK_SEL_EN_CLR            : 1;
        FIELD UNP_SEL_EN_CLR            : 1;
        FIELD AA_SEL_EN_CLR             : 1;
        FIELD LCS_SEL_EN_CLR            : 1;
        FIELD SGG_SEL_EN_CLR            : 1;
        FIELD BIN_SEL_EN_CLR            : 1;
        FIELD SOF_SEL_EN_CLR            : 1;
        FIELD C02_SEL_EN_CLR            : 1;
        FIELD G2G_SEL_EN_CLR            : 1;
        FIELD GGM_SEL_EN_CLR            : 1;
        FIELD YUV_SEL_EN_CLR            : 1;
        FIELD CDP_SEL_EN_CLR            : 1;
        FIELD NR3O_SEL_EN_CLR           : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_MUX_SEL_CLR;

typedef volatile union _CAM_REG_CTL_MUX_SEL2_SET_
{
    volatile struct
    {
        FIELD CCL_SEL_SET               : 2;
        FIELD BIN_OUT_SEL_SET           : 2;
        FIELD IMGO_MUX_SET              : 1;
        FIELD FLKI_SOF_SEL_SET          : 1;
        FIELD IMG2O_MUX_SET             : 1;
        FIELD IMGCI_SOF_SEL_SET         : 1;
        FIELD PASS1_DONE_MUX_SET        : 5;
        FIELD PASS2_DONE_MUX_SET        : 5;
        FIELD CCL_SEL_EN_SET            : 1;
        FIELD BIN_OUT_SEL_EN_SET        : 1;
        FIELD IMGO_MUX_EN_SET           : 1;
        FIELD IMG2O_MUX_EN_SET          : 1;
        FIELD IMGCI_SOF_SEL_EN_SET      : 1;
        FIELD FLKI_SOF_SEL_EN_SET       : 1;
        FIELD LCEI_SOF_SEL_EN_SET       : 1;
        FIELD IMGI_MUX_EN_SET           : 1;
        FIELD IMGCI_MUX_EN_SET          : 1;
        FIELD LCEI_SOF_SEL_SET          : 1;
        FIELD LSCI_SOF_SEL_SET          : 1;
        FIELD LSCI_SOF_SEL_EN_SET       : 1;
        FIELD PASS1_DONE_MUX_EN_SET     : 1;
        FIELD PASS2_DONE_MUX_EN_SET     : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_MUX_SEL2_SET;

typedef volatile union _CAM_REG_CTL_MUX_SEL2_CLR_
{
    volatile struct
    {
        FIELD CCL_SEL_CLR               : 2;
        FIELD BIN_OUT_SEL_CLR           : 2;
        FIELD IMGO_MUX_CLR              : 1;
        FIELD FLKI_SOF_SEL_CLR          : 1;
        FIELD IMG2O_MUX_CLR             : 1;
        FIELD IMGCI_SOF_SEL_CLR         : 1;
        FIELD PASS1_DONE_MUX_CLR        : 5;
        FIELD PASS2_DONE_MUX_CLR        : 5;
        FIELD CCL_SEL_EN_CLR            : 1;
        FIELD BIN_OUT_SEL_EN_CLR        : 1;
        FIELD IMGO_MUX_EN_CLR           : 1;
        FIELD IMG2O_MUX_EN_CLR          : 1;
        FIELD IMGCI_SOF_SEL_EN_CLR      : 1;
        FIELD FLKI_SOF_SEL_EN_CLR       : 1;
        FIELD LCEI_SOF_SEL_EN_CLR       : 1;
        FIELD IMGI_MUX_EN_CLR           : 1;
        FIELD IMGCI_MUX_EN_CLR          : 1;
        FIELD LCEI_SOF_SEL_CLR          : 1;
        FIELD LSCI_SOF_SEL_CLR          : 1;
        FIELD LSCI_SOF_SEL_EN_CLR       : 1;
        FIELD PASS1_DONE_MUX_EN_CLR     : 1;
        FIELD PASS2_DONE_MUX_EN_CLR     : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_MUX_SEL2_CLR;

typedef volatile union _CAM_REG_CTL_SRAM_MUX_CFG_SET_
{
    volatile struct
    {
        FIELD SRAM_MUX_SCENARIO_SET     : 3;
        FIELD rsv_3                     : 1;
        FIELD SRAM_MUX_MODE_SET         : 3;
        FIELD SRAM_MUX_TPIPE_SET         : 1;
        FIELD SRAM_MUX_SET_EN_SET       : 1;
        FIELD rsv_9                     : 1;
        FIELD IMGO_SOF_SEL_SET          : 1;
        FIELD LCSO_SOF_SEL_SET          : 1;
        FIELD ESFKO_SOF_SEL_SET         : 1;
        FIELD AAO_SOF_SEL_SET           : 1;
        FIELD RGB_SOF_SEL_SET           : 1;
        FIELD rsv_15                    : 1;
        FIELD LCSO_SOF_SEL_EN_SET       : 1;
        FIELD ESFKO_SOF_SEL_EN_SET      : 1;
        FIELD AAO_SOF_SEL_EN_SET        : 1;
        FIELD RGB_SOF_SEL_EN_SET        : 1;
        FIELD GDMA_SEL_EN_SET           : 1;
        FIELD GDMA_SYNC_EN_SET          : 1;
        FIELD GDMA_REPEAT_SET           : 1;
        FIELD GDMA_IMGCI_SEL_SET        : 1;
        FIELD PREGAIN_SEL_SET           : 1;
        FIELD CAM_OUT_FMT2_EN_SET       : 1;
        FIELD CAM_OUT_FMT2_SET          : 2;
        FIELD SGG_HRZ_SEL_SET           : 1;
        FIELD rsv_29                    : 1;
        FIELD IMG2O_SOF_SEL_SET         : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SRAM_MUX_CFG_SET;

typedef volatile union _CAM_REG_CTL_SRAM_MUX_CFG_CLR_
{
    volatile struct
    {
        FIELD SRAM_MUX_SCENARIO_CLR     : 3;
        FIELD rsv_3                     : 1;
        FIELD SRAM_MUX_MODE_CLR         : 3;
        FIELD SRAM_MUX_TPIPE_CLR         : 1;
        FIELD SRAM_MUX_SET_EN_CLR       : 1;
        FIELD rsv_9                     : 1;
        FIELD IMGO_SOF_SEL_CLR          : 1;
        FIELD LCSO_SOF_SEL_CLR          : 1;
        FIELD ESFKO_SOF_SEL_CLR         : 1;
        FIELD AAO_SOF_SEL_CLR           : 1;
        FIELD RGB_SOF_SEL_CLR           : 1;
        FIELD rsv_15                    : 1;
        FIELD LCSO_SOF_SEL_EN_CLR       : 1;
        FIELD ESFKO_SOF_SEL_EN_CLR      : 1;
        FIELD AAO_SOF_SEL_EN_CLR        : 1;
        FIELD RGB_SOF_SEL_EN_CLR        : 1;
        FIELD GDMA_SEL_EN_CLR           : 1;
        FIELD GDMA_SYNC_EN_CLR          : 1;
        FIELD GDMA_REPEAT_CLR           : 1;
        FIELD GDMA_IMGCI_SEL_CLR        : 1;
        FIELD PREGAIN_SEL_CLR           : 1;
        FIELD CAM_OUT_FMT2_EN_CLR       : 1;
        FIELD CAM_OUT_FMT2_CLR          : 2;
        FIELD SGG_HRZ_SEL_CLR           : 1;
        FIELD rsv_29                    : 1;
        FIELD IMG2O_SOF_SEL_CLR         : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SRAM_MUX_CFG_CLR;

typedef volatile union _CAM_REG_CTL_PIX_ID_SET_
{
    volatile struct
    {
        FIELD PIX_ID_SET                : 2;
        FIELD TG_PIX_ID_SET             : 2;
        FIELD TG_PIX_ID_EN_SET          : 1;
        FIELD BPC_TPIPE_EDGE_SEL_SET     : 1;
        FIELD BPC_TPIPE_EDGE_SEL_EN_SET  : 1;
        FIELD GDMA_IN_FMT_EN_SET        : 1;
        FIELD GDMA_IN_FMT_SET           : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_PIX_ID_SET;

typedef volatile union _CAM_REG_CTL_PIX_ID_CLR_
{
    volatile struct
    {
        FIELD PIX_ID_CLR                : 2;
        FIELD TG_PIX_ID_CLR             : 2;
        FIELD TG_PIX_ID_EN_CLR          : 1;
        FIELD BPC_TPIPE_EDGE_SEL_CLR     : 1;
        FIELD BPC_TPIPE_EDGE_SEL_EN_CLR  : 1;
        FIELD GDMA_IN_FMT_EN_CLR        : 1;
        FIELD GDMA_IN_FMT_CLR           : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_PIX_ID_CLR;

typedef volatile union _CAM_REG_CTL_SPARE0_SET_
{
    volatile struct
    {
        FIELD SPARE0_SET                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE0_SET;

typedef volatile union _CAM_REG_CTL_SPARE0_CLR_
{
    volatile struct
    {
        FIELD SPARE0_CLR                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE0_CLR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0_BASEADDR      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0B_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0B_BASEADDR     : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0B_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0C_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0C_BASEADDR     : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0C_BASEADDR;

typedef volatile union _CAM_REG_CTL_IMGO_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD DROP_INT_EN               : 1;
        FIELD rsv_5                     : 6;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGO_FBC;

typedef volatile union _CAM_REG_CTL_IMG2O_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD DROP_INT_EN               : 1;
        FIELD rsv_5                     : 6;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMG2O_FBC;

typedef volatile union _CAM_REG_CTL_FBC_INT_
{
    volatile struct
    {
        FIELD IMGO_DROP_INT             : 1;
        FIELD IMG2O_DROP_INT            : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FBC_INT;

typedef volatile union _CAM_REG_CTL_IMG2O_SIZE_
{
    volatile struct
    {
        FIELD IMG2O_YSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD IMG2O_XSIZE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMG2O_SIZE;

typedef volatile union _CAM_REG_CTL_IMGI_SIZE_
{
    volatile struct
    {
        FIELD IMGI_YSIZE                : 13;
        FIELD rsv_13                    : 3;
        FIELD IMGI_XSIZE                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGI_SIZE;

typedef volatile union _CAM_REG_CTL_VIDO_SIZE_
{
    volatile struct
    {
        FIELD VIDO_YSIZE                : 13;
        FIELD rsv_13                    : 3;
        FIELD VIDO_XSIZE                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_VIDO_SIZE;

typedef volatile union _CAM_REG_CTL_DISPO_SIZE_
{
    volatile struct
    {
        FIELD DISPO_YSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD DISPO_XSIZE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DISPO_SIZE;

typedef volatile union _CAM_REG_CTL_IMGO_SIZE_
{
    volatile struct
    {
        FIELD IMGO_YSIZE                : 13;
        FIELD rsv_13                    : 3;
        FIELD IMGO_XSIZE                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGO_SIZE;

typedef volatile union _CAM_REG_CTL_CLK_EN_
{
    volatile struct
    {
        FIELD RAW_DP_CK_EN              : 1;
        FIELD rsv_1                     : 1;
        FIELD DIP_DP_CK_EN              : 1;
        FIELD MFB_DP_CK_EN              : 1;
        FIELD FMT_DP_CK_EN              : 1;
        FIELD CDRZ_DP_CK_EN             : 1;
        FIELD CURZ_DP_CK_EN             : 1;
        FIELD PRZ_DP_CK_EN              : 1;
        FIELD rsv_8                     : 1;
        FIELD DISPO_DP_CK_EN            : 1;
        FIELD DISPO_SMI_CK_EN           : 1;
        FIELD VIDO_DP_CK_EN             : 1;
        FIELD VIDO_SMI_CK_EN            : 1;
        FIELD rsv_13                    : 2;
        FIELD DMA_DP_CK_EN              : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CLK_EN;

typedef volatile union _CAM_REG_CTL_DBG_SET_
{
    volatile struct
    {
        FIELD DEBUG_MOD_SEL             : 8;
        FIELD DEBUG_SEL                 : 4;
        FIELD DEBUG_TOP_SEL             : 4;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DBG_SET;

typedef volatile union _CAM_REG_CTL_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DBG_PORT;

typedef volatile union _CAM_REG_CTL_IMGI_CHECK_
{
    volatile struct
    {
        FIELD CTL_IMGI_CHECK            : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGI_CHECK;

typedef volatile union _CAM_REG_CTL_IMGO_CHECK_
{
    volatile struct
    {
        FIELD CTL_IMGO_CHECK            : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGO_CHECK;

typedef volatile union _CAM_REG_CTL_DATE_CODE_
{
    volatile struct
    {
        FIELD CTL_DATE_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DATE_CODE;

typedef volatile union _CAM_REG_CTL_PROJ_CODE_
{
    volatile struct
    {
        FIELD CTL_PROJ_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_PROJ_CODE;

typedef volatile union _CAM_REG_CTL_RAW_DCM_DIS_
{
    volatile struct
    {
        FIELD BIN_DCM_DIS               : 1;
        FIELD PAK_DCM_DIS               : 1;
        FIELD BIN2_DCM_DIS              : 1;
        FIELD PAK2_DCM_DIS              : 1;
        FIELD UNP_DCM_DIS               : 1;
        FIELD OB_DCM_DIS                : 1;
        FIELD BPC_DCM_DIS               : 1;
        FIELD LSC_DCM_DIS               : 1;
        FIELD HRZ_DCM_DIS               : 1;
        FIELD PGN_DCM_DIS               : 1;
        FIELD AEAWB_DCM_DIS             : 1;
        FIELD SGG_DCM_DIS               : 1;
        FIELD AF_DCM_DIS                : 1;
        FIELD FLK_DCM_DIS               : 1;
        FIELD LCS_DCM_DIS               : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_DCM_DIS;

typedef volatile union _CAM_REG_CTL_RGB_DCM_DIS_
{
    volatile struct
    {
        FIELD CFA_DCM_DIS               : 1;
        FIELD CCL_DCM_DIS               : 1;
        FIELD MFB_DCM_DIS               : 1;
        FIELD C02_DCM_DIS               : 1;
        FIELD C24_DCM_DIS               : 1;
        FIELD G2G_DCM_DIS               : 1;
        FIELD DGM_DCM_DIS               : 1;
        FIELD LCE_DCM_DIS               : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RGB_DCM_DIS;

typedef volatile union _CAM_REG_CTL_YUV_DCM_DIS_
{
    volatile struct
    {
        FIELD G2C_DCM_DIS               : 1;
        FIELD C42_DCM_DIS               : 1;
        FIELD NBC_DCM_DIS               : 1;
        FIELD SEEE_DCM_DIS              : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_YUV_DCM_DIS;

typedef volatile union _CAM_REG_CTL_CDP_DCM_DIS_
{
    volatile struct
    {
        FIELD CDRZ_DCM_DIS              : 1;
        FIELD EIS_DCM_DIS               : 1;
        FIELD NR3D_DCM_DIS              : 1;
        FIELD FE_DCM_DIS                : 1;
        FIELD CURZ_DCM_DIS              : 1;
        FIELD PRZ_DCM_DIS               : 1;
        FIELD G2G2_DCM_DIS              : 1;
        FIELD CRSP_DCM_DIS              : 1;
        FIELD PRSP_DCM_DIS              : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CDP_DCM_DIS;

typedef volatile union _CAM_REG_CTL_RAW_DCM_STATUS_
{
    volatile struct
    {
        FIELD BIN_DCM_ST                : 1;
        FIELD PAK_DCM_ST                : 1;
        FIELD BIN2_DCM_ST               : 1;
        FIELD PAK2_DCM_ST               : 1;
        FIELD UNP_DCM_ST                : 1;
        FIELD OB_DCM_ST                 : 1;
        FIELD BPC_DCM_ST                : 1;
        FIELD LSC_DCM_ST                : 1;
        FIELD HRZ_DCM_ST                : 1;
        FIELD PGN_DCM_ST                : 1;
        FIELD AEAWB_DCM_ST              : 1;
        FIELD SGG_DCM_ST                : 1;
        FIELD AF_DCM_ST                 : 1;
        FIELD FLK_DCM_ST                : 1;
        FIELD LCS_DCM_ST                : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_RGB_DCM_STATUS_
{
    volatile struct
    {
        FIELD CFA_DCM_ST                : 1;
        FIELD CCL_DCM_ST                : 1;
        FIELD MFB_DCM_ST                : 1;
        FIELD C02_DCM_ST                : 1;
        FIELD C24_DCM_ST                : 1;
        FIELD G2G_DCM_ST                : 1;
        FIELD DGM_DCM_ST                : 1;
        FIELD LCE_DCM_ST                : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RGB_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_YUV_DCM_STATUS_
{
    volatile struct
    {
        FIELD G2C_DCM_ST                : 1;
        FIELD C42_DCM_ST                : 1;
        FIELD NBC_DCM_ST                : 1;
        FIELD SEEE_DCM_ST               : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_YUV_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_CDP_DCM_STATUS_
{
    volatile struct
    {
        FIELD CDRZ_DCM_ST               : 1;
        FIELD EIS_DCM_ST                : 1;
        FIELD NR3D_DCM_ST               : 1;
        FIELD FE_DCM_ST                 : 1;
        FIELD CURZ_DCM_ST               : 1;
        FIELD PRZ_DCM_ST                : 1;
        FIELD G2G2_DCM_ST               : 1;
        FIELD CRSP_DCM_ST               : 1;
        FIELD PRSP_DCM_ST               : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CDP_DCM_STATUS;

/* end MT6589_000_cam_ctl.xml*/

/* start MT6589_100_dma.xml*/
typedef volatile union _CAM_REG_DMA_SOFT_RSTSTAT_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD IMGI_SOFT_RST_STAT        : 1;
        FIELD IMGCI_SOFT_RST_STAT       : 1;
        FIELD rsv_3                     : 1;
        FIELD LSCI_SOFT_RST_STAT        : 1;
        FIELD FLKI_SOFT_RST_STAT        : 1;
        FIELD LCEI_SOFT_RST_STAT        : 1;
        FIELD VIPI_SOFT_RST_STAT        : 1;
        FIELD VIP2I_SOFT_RST_STAT       : 1;
        FIELD rsv_9                     : 7;
        FIELD IMGO_SOFT_RST_STAT        : 1;
        FIELD IMG2O_SOFT_RST_STAT       : 1;
        FIELD LCSO_SOFT_RST_STAT        : 1;
        FIELD ESFKO_SOFT_RST_STAT       : 1;
        FIELD AAO_SOFT_RST_STAT         : 1;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_SOFT_RSTSTAT;

typedef volatile union _CAM_REG_TDRI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TDRI_BASE_ADDR;

typedef volatile union _CAM_REG_TDRI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TDRI_OFST_ADDR;

typedef volatile union _CAM_REG_TDRI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TDRI_XSIZE;

typedef volatile union _CAM_REG_CQ0I_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CQ0I_BASE_ADDR;

typedef volatile union _CAM_REG_CQ0I_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CQ0I_XSIZE;

typedef volatile union _CAM_REG_IMGI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_BASE_ADDR;

typedef volatile union _CAM_REG_IMGI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_OFST_ADDR;

typedef volatile union _CAM_REG_IMGI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_XSIZE;

typedef volatile union _CAM_REG_IMGI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_YSIZE;

typedef volatile union _CAM_REG_IMGI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 15;
        FIELD rsv_15                    : 1;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 6;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_STRIDE;

typedef volatile union _CAM_REG_IMGI_RING_BUF_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD RING_SIZE                 : 20;
        FIELD rsv_28                    : 3;
        FIELD EN                        : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_RING_BUF;

typedef volatile union _CAM_REG_IMGI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_CON;

typedef volatile union _CAM_REG_IMGI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_CON2;

typedef volatile union _CAM_REG_IMGCI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_BASE_ADDR;

typedef volatile union _CAM_REG_IMGCI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_OFST_ADDR;

typedef volatile union _CAM_REG_IMGCI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_XSIZE;

typedef volatile union _CAM_REG_IMGCI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_YSIZE;

typedef volatile union _CAM_REG_IMGCI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 2;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 6;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_STRIDE;

typedef volatile union _CAM_REG_IMGCI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_CON;

typedef volatile union _CAM_REG_IMGCI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_CON2;

typedef volatile union _CAM_REG_LSCI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_BASE_ADDR;

typedef volatile union _CAM_REG_LSCI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_OFST_ADDR;

typedef volatile union _CAM_REG_LSCI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_XSIZE;

typedef volatile union _CAM_REG_LSCI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_YSIZE;

typedef volatile union _CAM_REG_LSCI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 3;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 6;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_STRIDE;

typedef volatile union _CAM_REG_LSCI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_CON;

typedef volatile union _CAM_REG_LSCI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_CON2;

typedef volatile union _CAM_REG_FLKI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_BASE_ADDR;

typedef volatile union _CAM_REG_FLKI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_OFST_ADDR;

typedef volatile union _CAM_REG_FLKI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_XSIZE;

typedef volatile union _CAM_REG_FLKI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_YSIZE;

typedef volatile union _CAM_REG_FLKI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 13;
        FIELD rsv_13                    : 3;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 2;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_STRIDE;

typedef volatile union _CAM_REG_FLKI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_CON;

typedef volatile union _CAM_REG_FLKI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_CON2;

typedef volatile union _CAM_REG_LCEI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_BASE_ADDR;

typedef volatile union _CAM_REG_LCEI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_OFST_ADDR;

typedef volatile union _CAM_REG_LCEI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_XSIZE;

typedef volatile union _CAM_REG_LCEI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_YSIZE;

typedef volatile union _CAM_REG_LCEI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 13;
        FIELD rsv_13                    : 3;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 2;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 6;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_STRIDE;

typedef volatile union _CAM_REG_LCEI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_CON;

typedef volatile union _CAM_REG_LCEI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_CON2;

typedef volatile union _CAM_REG_VIPI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_BASE_ADDR;

typedef volatile union _CAM_REG_VIPI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_OFST_ADDR;

typedef volatile union _CAM_REG_VIPI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_XSIZE;

typedef volatile union _CAM_REG_VIPI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_YSIZE;

typedef volatile union _CAM_REG_VIPI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 2;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 6;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_STRIDE;

typedef volatile union _CAM_REG_VIPI_RING_BUF_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD RING_SIZE                 : 20;
        FIELD rsv_28                    : 3;
        FIELD EN                        : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_RING_BUF;

typedef volatile union _CAM_REG_VIPI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_CON;

typedef volatile union _CAM_REG_VIPI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_CON2;

typedef volatile union _CAM_REG_VIP2I_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_BASE_ADDR;

typedef volatile union _CAM_REG_VIP2I_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_OFST_ADDR;

typedef volatile union _CAM_REG_VIP2I_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_XSIZE;

typedef volatile union _CAM_REG_VIP2I_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_YSIZE;

typedef volatile union _CAM_REG_VIP2I_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 6;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_STRIDE;

typedef volatile union _CAM_REG_VIP2I_RING_BUF_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD RING_SIZE                 : 20;
        FIELD rsv_28                    : 3;
        FIELD EN                        : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_RING_BUF;

typedef volatile union _CAM_REG_VIP2I_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_CON;

typedef volatile union _CAM_REG_VIP2I_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_CON2;

typedef volatile union _CAM_REG_IMGO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_BASE_ADDR;

typedef volatile union _CAM_REG_IMGO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_OFST_ADDR;

typedef volatile union _CAM_REG_IMGO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_XSIZE;

typedef volatile union _CAM_REG_IMGO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_YSIZE;

typedef volatile union _CAM_REG_IMGO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_STRIDE;

typedef volatile union _CAM_REG_IMGO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CON;

typedef volatile union _CAM_REG_IMGO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CON2;

typedef volatile union _CAM_REG_IMGO_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CROP;

typedef volatile union _CAM_REG_IMG2O_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_BASE_ADDR;

typedef volatile union _CAM_REG_IMG2O_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_OFST_ADDR;

typedef volatile union _CAM_REG_IMG2O_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_XSIZE;

typedef volatile union _CAM_REG_IMG2O_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_YSIZE;

typedef volatile union _CAM_REG_IMG2O_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_STRIDE;

typedef volatile union _CAM_REG_IMG2O_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_CON;

typedef volatile union _CAM_REG_IMG2O_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_CON2;

typedef volatile union _CAM_REG_IMG2O_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_CROP;

typedef volatile union _CAM_REG_LCSO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_BASE_ADDR;

typedef volatile union _CAM_REG_LCSO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_OFST_ADDR;

typedef volatile union _CAM_REG_LCSO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_XSIZE;

typedef volatile union _CAM_REG_LCSO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_YSIZE;

typedef volatile union _CAM_REG_LCSO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_STRIDE;

typedef volatile union _CAM_REG_LCSO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_CON;

typedef volatile union _CAM_REG_LCSO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_CON2;

typedef volatile union _CAM_REG_EISO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_EISO_BASE_ADDR;

typedef volatile union _CAM_REG_EISO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 10;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_EISO_XSIZE;

typedef volatile union _CAM_REG_AFO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_BASE_ADDR;

typedef volatile union _CAM_REG_AFO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 10;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_XSIZE;

typedef volatile union _CAM_REG_ESFKO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_BASE_ADDR;

typedef volatile union _CAM_REG_ESFKO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_XSIZE;

typedef volatile union _CAM_REG_ESFKO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_OFST_ADDR;

typedef volatile union _CAM_REG_ESFKO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_YSIZE;

typedef volatile union _CAM_REG_ESFKO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_STRIDE;

typedef volatile union _CAM_REG_ESFKO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_CON;

typedef volatile union _CAM_REG_ESFKO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_CON2;

typedef volatile union _CAM_REG_AAO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_BASE_ADDR;

typedef volatile union _CAM_REG_AAO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_OFST_ADDR;

typedef volatile union _CAM_REG_AAO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_XSIZE;

typedef volatile union _CAM_REG_AAO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_YSIZE;

typedef volatile union _CAM_REG_AAO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_STRIDE;

typedef volatile union _CAM_REG_AAO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_CON;

typedef volatile union _CAM_REG_AAO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_CON2;

typedef volatile union _CAM_REG_DMA_ERR_CTRL_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD IMGI_ERR                  : 1;
        FIELD IMGCI_ERR                 : 1;
        FIELD rsv_3                     : 1;
        FIELD LSCI_ERR                  : 1;
        FIELD FLKI_ERR                  : 1;
        FIELD LCEI_ERR                  : 1;
        FIELD VIPI_ERR                  : 1;
        FIELD VIP2I_ERR                 : 1;
        FIELD IMGO_ERR                  : 1;
        FIELD IMG2O_ERR                 : 1;
        FIELD LCSO_ERR                  : 1;
        FIELD ESFKO_ERR                 : 1;
        FIELD AAO_ERR                   : 1;
        FIELD rsv_14                    : 17;
        FIELD ERR_CLR_MD                : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_ERR_CTRL;

typedef volatile union _CAM_REG_IMGI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_ERR_STAT;

typedef volatile union _CAM_REG_IMGCI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGCI_ERR_STAT;

typedef volatile union _CAM_REG_LSCI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_ERR_STAT;

typedef volatile union _CAM_REG_FLKI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLKI_ERR_STAT;

typedef volatile union _CAM_REG_LCEI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_ERR_STAT;

typedef volatile union _CAM_REG_VIPI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_ERR_STAT;

typedef volatile union _CAM_REG_VIP2I_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_ERR_STAT;

typedef volatile union _CAM_REG_IMGO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_ERR_STAT;

typedef volatile union _CAM_REG_IMG2O_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_ERR_STAT;

typedef volatile union _CAM_REG_LCSO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_ERR_STAT;

typedef volatile union _CAM_REG_ESFKO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_ERR_STAT;

typedef volatile union _CAM_REG_AAO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_ERR_STAT;

typedef volatile union _CAM_REG_DMA_DEBUG_ADDR_
{
    volatile struct
    {
        FIELD DEBUG_ADDR                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_DEBUG_ADDR;

typedef volatile union _CAM_REG_DMA_RSV1_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV1;

typedef volatile union _CAM_REG_DMA_RSV2_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV2;

typedef volatile union _CAM_REG_DMA_RSV3_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV3;

typedef volatile union _CAM_REG_DMA_RSV4_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV4;

typedef volatile union _CAM_REG_DMA_RSV5_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV5;

typedef volatile union _CAM_REG_DMA_RSV6_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV6;

/* end MT6589_100_dma.xml*/

/* start MT6589_201_raw_tg.xml*/
typedef volatile union _CAM_REG_TG_SEN_MODE_
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_MODE;

typedef volatile union _CAM_REG_TG_VF_CON_
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_VF_CON;

typedef volatile union _CAM_REG_TG_SEN_GRAB_PXL_
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_GRAB_PXL;

typedef volatile union _CAM_REG_TG_SEN_GRAB_LIN_
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_GRAB_LIN;

typedef volatile union _CAM_REG_TG_PATH_CFG_
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_PATH_CFG;

typedef volatile union _CAM_REG_TG_MEMIN_CTL_
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MEMIN_CTL;

typedef volatile union _CAM_REG_TG_INT1_
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INT1;

typedef volatile union _CAM_REG_TG_INT2_
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INT2;

typedef volatile union _CAM_REG_TG_SOF_CNT_
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SOF_CNT;

typedef volatile union _CAM_REG_TG_SOT_CNT_
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SOT_CNT;

typedef volatile union _CAM_REG_TG_EOT_CNT_
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_EOT_CNT;

typedef volatile union _CAM_REG_TG_ERR_CTL_
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_ERR_CTL;

typedef volatile union _CAM_REG_TG_DAT_NO_
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_DAT_NO;

typedef volatile union _CAM_REG_TG_FRM_CNT_ST_
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FRM_CNT_ST;

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FRMSIZE_ST;

typedef volatile union _CAM_REG_TG_INTER_ST_
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INTER_ST;

typedef volatile union _CAM_REG_TG_FLASHA_CTL_
{
    volatile struct
    {
        FIELD FLASHA_EN                 : 1;
        FIELD FLASH_EN                  : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHA_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHA_END_FRM            : 3;
        FIELD rsv_11                    : 1;
        FIELD FLASH_POL                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_CTL;

typedef volatile union _CAM_REG_TG_FLASHA_LINE_CNT_
{
    volatile struct
    {
        FIELD FLASHA_LUNIT_NO           : 20;
        FIELD FLASHA_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_LINE_CNT;

typedef volatile union _CAM_REG_TG_FLASHA_POS_
{
    volatile struct
    {
        FIELD FLASHA_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHA_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_POS;

typedef volatile union _CAM_REG_TG_FLASHB_CTL_
{
    volatile struct
    {
        FIELD FLASHB_EN                 : 1;
        FIELD FLASHB_TRIG_SRC           : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHB_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHB_START_FRM          : 4;
        FIELD FLASHB_CONT_FRM           : 3;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_CTL;

typedef volatile union _CAM_REG_TG_FLASHB_LINE_CNT_
{
    volatile struct
    {
        FIELD FLASHB_LUNIT_NO           : 20;
        FIELD FLASHB_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_LINE_CNT;

typedef volatile union _CAM_REG_TG_FLASHB_POS_
{
    volatile struct
    {
        FIELD FLASHB_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHB_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_POS;

typedef volatile union _CAM_REG_TG_FLASHB_POS1_
{
    volatile struct
    {
        FIELD FLASHB_CYC_CNT            : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_POS1;

typedef volatile union _CAM_REG_TG_GSCTRL_CTL_
{
    volatile struct
    {
        FIELD GSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD GSCTRL_POL                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_GSCTRL_CTL;

typedef volatile union _CAM_REG_TG_GSCTRL_TIME_
{
    volatile struct
    {
        FIELD GS_EPTIME                 : 23;
        FIELD rsv_23                    : 1;
        FIELD GSMS_TIMEU                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_GSCTRL_TIME;

typedef volatile union _CAM_REG_TG_MS_PHASE_
{
    volatile struct
    {
        FIELD MSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD MSCTRL_VSPOL              : 1;
        FIELD MSCTRL_OPEN_TRSRC         : 1;
        FIELD rsv_6                     : 2;
        FIELD MSCTRL_TRSRC              : 2;
        FIELD rsv_10                    : 6;
        FIELD MSCP1_PH0                 : 1;
        FIELD MSCP1_PH1                 : 1;
        FIELD MSCP1_PH2                 : 1;
        FIELD rsv_19                    : 1;
        FIELD MSOP1_PH0                 : 1;
        FIELD MSOP1_PH1                 : 1;
        FIELD rsv_22                    : 1;
        FIELD MSP1_RST                  : 1;
        FIELD MSCP2_PH0                 : 1;
        FIELD MSCP2_PH1                 : 1;
        FIELD MSCP2_PH2                 : 1;
        FIELD rsv_27                    : 1;
        FIELD MSOP2_PH0                 : 1;
        FIELD MSOP2_PH1                 : 1;
        FIELD rsv_30                    : 1;
        FIELD MSP2_RST                  : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_PHASE;

typedef volatile union _CAM_REG_TG_MS_CL_TIME_
{
    volatile struct
    {
        FIELD MS_TCLOSE                 : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_CL_TIME;

typedef volatile union _CAM_REG_TG_MS_OP_TIME_
{
    volatile struct
    {
        FIELD MS_TOPEN                  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_OP_TIME;

typedef volatile union _CAM_REG_TG_MS_CLPH_TIME_
{
    volatile struct
    {
        FIELD MS_CL_T1                  : 16;
        FIELD MS_CL_T2                  : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_CLPH_TIME;

typedef volatile union _CAM_REG_TG_MS_OPPH_TIME_
{
    volatile struct
    {
        FIELD MS_OP_T3                  : 16;
        FIELD MS_OP_T4                  : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_OPPH_TIME;

typedef volatile union _CAM_REG_TG2_SEN_MODE_
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_MODE;

typedef volatile union _CAM_REG_TG2_VF_CON_
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_VF_CON;

typedef volatile union _CAM_REG_TG2_SEN_GRAB_PXL_
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_GRAB_PXL;

typedef volatile union _CAM_REG_TG2_SEN_GRAB_LIN_
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_GRAB_LIN;

typedef volatile union _CAM_REG_TG2_PATH_CFG_
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_PATH_CFG;

typedef volatile union _CAM_REG_TG2_MEMIN_CTL_
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_MEMIN_CTL;

typedef volatile union _CAM_REG_TG2_INT1_
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INT1;

typedef volatile union _CAM_REG_TG2_INT2_
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INT2;

typedef volatile union _CAM_REG_TG2_SOF_CNT_
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SOF_CNT;

typedef volatile union _CAM_REG_TG2_SOT_CNT_
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SOT_CNT;

typedef volatile union _CAM_REG_TG2_EOT_CNT_
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_EOT_CNT;

typedef volatile union _CAM_REG_TG2_ERR_CTL_
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_ERR_CTL;

typedef volatile union _CAM_REG_TG2_DAT_NO_
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_DAT_NO;

typedef volatile union _CAM_REG_TG2_FRM_CNT_ST_
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_FRM_CNT_ST;

typedef volatile union _CAM_REG_TG2_FRMSIZE_ST_
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_FRMSIZE_ST;

typedef volatile union _CAM_REG_TG2_INTER_ST_
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INTER_ST;

/* end MT6589_201_raw_tg.xml*/

/* start MT6589_202_raw_bin.xml*/
typedef volatile union _CAM_REG_BIN_SIZE_
{
    volatile struct
    {
        FIELD BIN_IN_H                  : 13;
        FIELD rsv_13                    : 3;
        FIELD BIN_IN_V                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BIN_SIZE;

typedef volatile union _CAM_REG_BIN_MODE_
{
    volatile struct
    {
        FIELD BIN_MODE                  : 2;
        FIELD rsv_2                     : 2;
        FIELD BIN_ENABLE                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_BIN_MODE;

typedef volatile union _CAM_REG_BIN2_SIZE_
{
    volatile struct
    {
        FIELD BIN_IN_H                  : 13;
        FIELD rsv_13                    : 3;
        FIELD BIN_IN_V                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BIN2_SIZE;

typedef volatile union _CAM_REG_BIN2_MODE_
{
    volatile struct
    {
        FIELD BIN_MODE                  : 2;
        FIELD rsv_2                     : 2;
        FIELD BIN_ENABLE                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_BIN2_MODE;

/* end MT6589_202_raw_bin.xml*/

/* start MT6589_203_raw_obc.xml*/
typedef volatile union _CAM_REG_OBC_OFFST0_
{
    volatile struct
    {
        FIELD OBOFFSET0                 : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST0;

typedef volatile union _CAM_REG_OBC_OFFST1_
{
    volatile struct
    {
        FIELD OBOFFSET1                 : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST1;

typedef volatile union _CAM_REG_OBC_OFFST2_
{
    volatile struct
    {
        FIELD OBOFFSET2                 : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST2;

typedef volatile union _CAM_REG_OBC_OFFST3_
{
    volatile struct
    {
        FIELD OBOFFSET3                 : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST3;

typedef volatile union _CAM_REG_OBC_GAIN0_
{
    volatile struct
    {
        FIELD OBGAIN0                   : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN0;

typedef volatile union _CAM_REG_OBC_GAIN1_
{
    volatile struct
    {
        FIELD OBGAIN1                   : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN1;

typedef volatile union _CAM_REG_OBC_GAIN2_
{
    volatile struct
    {
        FIELD OBGAIN2                   : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN2;

typedef volatile union _CAM_REG_OBC_GAIN3_
{
    volatile struct
    {
        FIELD OBGAIN3                   : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN3;

/* end MT6589_203_raw_obc.xml*/

/* start MT6589_204_raw_lsc.xml*/
typedef volatile union _CAM_REG_LSC_CTL1_
{
    volatile struct
    {
        FIELD SDBLK_YOFST               : 6;
        FIELD rsv_6                     : 10;
        FIELD SDBLK_XOFST               : 6;
        FIELD rsv_22                    : 2;
        FIELD SD_COEFRD_MODE            : 1;
        FIELD rsv_25                    : 3;
        FIELD SD_ULTRA_MODE             : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_CTL1;

typedef volatile union _CAM_REG_LSC_CTL2_
{
    volatile struct
    {
        FIELD SDBLK_WIDTH               : 12;
        FIELD SDBLK_XNUM                : 5;
        FIELD LSC_OFFLINE               : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_CTL2;

typedef volatile union _CAM_REG_LSC_CTL3_
{
    volatile struct
    {
        FIELD SDBLK_HEIGHT              : 12;
        FIELD SDBLK_YNUM                : 5;
        FIELD LSC_SPARE                 : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_CTL3;

typedef volatile union _CAM_REG_LSC_LBLOCK_
{
    volatile struct
    {
        FIELD SDBLK_lHEIGHT             : 12;
        FIELD rsv_12                    : 4;
        FIELD SDBLK_lWIDTH              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_LBLOCK;

typedef volatile union _CAM_REG_LSC_RATIO_
{
    volatile struct
    {
        FIELD RATIO11                   : 6;
        FIELD rsv_6                     : 2;
        FIELD RATIO10                   : 6;
        FIELD rsv_14                    : 2;
        FIELD RATIO01                   : 6;
        FIELD rsv_22                    : 2;
        FIELD RATIO00                   : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_RATIO;

typedef volatile union _CAM_REG_LSC_TPIPE_OFST_
{
    volatile struct
    {
        FIELD SD_TPIPE_YOFST             : 12;
        FIELD rsv_12                    : 4;
        FIELD SD_TPIPE_XOFST             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_TPIPE_OFST;

typedef volatile union _CAM_REG_LSC_TPIPE_SIZE_
{
    volatile struct
    {
        FIELD SD_TPIPE_YSIZE             : 13;
        FIELD rsv_13                    : 3;
        FIELD SD_TPIPE_XSIZE             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_TPIPE_SIZE;

typedef volatile union _CAM_REG_LSC_GAIN_TH_
{
    volatile struct
    {
        FIELD SDBLK_GAIN_TH2            : 9;
        FIELD rsv_9                     : 1;
        FIELD SDBLK_GAIN_TH1            : 9;
        FIELD rsv_19                    : 1;
        FIELD SDBLK_GAIN_TH0            : 9;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_GAIN_TH;

/* end MT6589_204_raw_lsc.xml*/

/* start MT6589_207_raw_hrz.xml*/
typedef volatile union _CAM_REG_HRZ_RES_
{
    volatile struct
    {
        FIELD HRZ_RESIZE                : 18;
        FIELD HRZ_INPUT_HEIGHT          : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_HRZ_RES;

typedef volatile union _CAM_REG_HRZ_OUT_
{
    volatile struct
    {
        FIELD HRZ_OUTSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD HRZ_BPCTH                 : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_HRZ_OUT;

/* end MT6589_207_raw_hrz.xml*/

/* start MT6589_2091_raw_awb.xml*/
typedef volatile union _CAM_REG_AWB_WIN_ORG_
{
    volatile struct
    {
        FIELD AWB_W_ORIGIN_X            : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_ORIGIN_Y            : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_ORG;

typedef volatile union _CAM_REG_AWB_WIN_SIZE_
{
    volatile struct
    {
        FIELD AWB_W_SIZE_X            : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_SIZE_Y            : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_SIZE;

typedef volatile union _CAM_REG_AWB_WIN_PITCH_
{
    volatile struct
    {
        FIELD AWB_W_PITCH_X             : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_PITCH_Y             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_PITCH;

typedef volatile union _CAM_REG_AWB_WIN_NUM_
{
    volatile struct
    {
        FIELD AWB_W_NUM_X             : 8;
        FIELD rsv_8                     : 8;
        FIELD AWB_W_NUM_Y             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_NUM;

typedef volatile union _CAM_REG_AWB_RAWPREGAIN1_0_
{
    volatile struct
    {
        FIELD AWB_GAIN1_R             : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_GAIN1_G             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_RAWPREGAIN1_0;

typedef volatile union _CAM_REG_AWB_RAWPREGAIN1_1_
{
    volatile struct
    {
        FIELD AWB_GAIN1_B             : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_RAWPREGAIN1_1;

typedef volatile union _CAM_REG_AWB_RAWLIMIT1_0_
{
    volatile struct
    {
        FIELD AWB_LIMIT1_R               : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_LIMIT1_G               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_RAWLIMIT1_0;

typedef volatile union _CAM_REG_AWB_RAWLIMIT1_1_
{
    volatile struct
    {
        FIELD AWB_LIMIT1_B               : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_RAWLIMIT1_1;

typedef volatile union _CAM_REG_AWB_LOW_THR_
{
    volatile struct
    {
        FIELD AWB_LOW_THR0              : 8;
        FIELD AWB_LOW_THR1              : 8;
        FIELD AWB_LOW_THR2              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_LOW_THR;

typedef volatile union _CAM_REG_AWB_HI_THR_
{
    volatile struct
    {
        FIELD AWB_HI_THR0               : 8;
        FIELD AWB_HI_THR1               : 8;
        FIELD AWB_HI_THR2               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_HI_THR;

typedef volatile union _CAM_REG_AWB_PIXEL_CNT0_
{
    volatile struct
    {
        FIELD PIXEL_CNT0                : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_PIXEL_CNT0;

typedef volatile union _CAM_REG_AWB_PIXEL_CNT1_
{
    volatile struct
    {
        FIELD PIXEL_CNT1                : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_PIXEL_CNT1;

typedef volatile union _CAM_REG_AWB_PIXEL_CNT2_
{
    volatile struct
    {
        FIELD PIXEL_CNT2                : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_PIXEL_CNT2;

typedef volatile union _CAM_REG_AWB_ERR_THR_
{
    volatile struct
    {
        FIELD AWB_ERR_THR               : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_ERR_THR;

typedef volatile union _CAM_REG_AWB_ROT_
{
    volatile struct
    {
        FIELD AWB_C                   : 9;
        FIELD rsv_9                     : 7;
        FIELD AWB_S                   : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_ROT;

typedef volatile union _CAM_REG_AWB_L0_X_
{
    volatile struct
    {
        FIELD AWB_L0_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L0_X;

typedef volatile union _CAM_REG_AWB_L0_Y_
{
    volatile struct
    {
        FIELD AWB_L0_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L0_Y;

typedef volatile union _CAM_REG_AWB_L1_X_
{
    volatile struct
    {
        FIELD AWB_L1_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L1_X;

typedef volatile union _CAM_REG_AWB_L1_Y_
{
    volatile struct
    {
        FIELD AWB_L1_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L1_Y;

typedef volatile union _CAM_REG_AWB_L2_X_
{
    volatile struct
    {
        FIELD AWB_L2_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L2_X;

typedef volatile union _CAM_REG_AWB_L2_Y_
{
    volatile struct
    {
        FIELD AWB_L2_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L2_Y;

typedef volatile union _CAM_REG_AWB_L3_X_
{
    volatile struct
    {
        FIELD AWB_L3_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L3_X;

typedef volatile union _CAM_REG_AWB_L3_Y_
{
    volatile struct
    {
        FIELD AWB_L3_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L3_Y;

typedef volatile union _CAM_REG_AWB_L4_X_
{
    volatile struct
    {
        FIELD AWB_L4_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L4_X;

typedef volatile union _CAM_REG_AWB_L4_Y_
{
    volatile struct
    {
        FIELD AWB_L4_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L4_Y;

typedef volatile union _CAM_REG_AWB_L5_X_
{
    volatile struct
    {
        FIELD AWB_L5_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L5_X;

typedef volatile union _CAM_REG_AWB_L5_Y_
{
    volatile struct
    {
        FIELD AWB_L5_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L5_Y;

typedef volatile union _CAM_REG_AWB_L6_X_
{
    volatile struct
    {
        FIELD AWB_L6_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L6_X;

typedef volatile union _CAM_REG_AWB_L6_Y_
{
    volatile struct
    {
        FIELD AWB_L6_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L6_Y;

typedef volatile union _CAM_REG_AWB_L7_X_
{
    volatile struct
    {
        FIELD AWB_L7_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L7_X;

typedef volatile union _CAM_REG_AWB_L7_Y_
{
    volatile struct
    {
        FIELD AWB_L7_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L7_Y;

typedef volatile union _CAM_REG_AWB_L8_X_
{
    volatile struct
    {
        FIELD AWB_L8_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L8_X;

typedef volatile union _CAM_REG_AWB_L8_Y_
{
    volatile struct
    {
        FIELD AWB_L8_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L8_Y;

typedef volatile union _CAM_REG_AWB_L9_X_
{
    volatile struct
    {
        FIELD AWB_L9_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L9_X;

typedef volatile union _CAM_REG_AWB_L9_Y_
{
    volatile struct
    {
        FIELD AWB_L9_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L9_Y;

typedef volatile union _CAM_REG_AWB_SPARE_
{
    volatile struct
    {
        FIELD AWB_SPARE                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_SPARE;

/* end MT6589_2091_raw_awb.xml*/

/* start MT6589_2092_raw_ae.xml*/
typedef volatile union _CAM_REG_AE_HST_CTL_
{
    volatile struct
    {
        FIELD AE_HST0_EN                : 1;
        FIELD AE_HST1_EN                : 1;
        FIELD AE_HST2_EN                : 1;
        FIELD AE_HST3_EN                : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST_CTL;

typedef volatile union _CAM_REG_AE_RAWPREGAIN2_0_
{
    volatile struct
    {
        FIELD AE_GAIN2_R             : 12;
        FIELD rsv_12                    : 4;
        FIELD AE_GAIN2_G             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RAWPREGAIN2_0;

typedef volatile union _CAM_REG_AE_RAWPREGAIN2_1_
{
    volatile struct
    {
        FIELD AE_GAIN2_B             : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RAWPREGAIN2_1;

typedef volatile union _CAM_REG_AE_RAWLIMIT2_0_
{
    volatile struct
    {
        FIELD AE_LIMIT2_R               : 12;
        FIELD rsv_12                    : 4;
        FIELD AE_LIMIT2_G               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RAWLIMIT2_0;

typedef volatile union _CAM_REG_AE_RAWLIMIT2_1_
{
    volatile struct
    {
        FIELD AE_LIMIT2_B               : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RAWLIMIT2_1;

typedef volatile union _CAM_REG_AE_MATRIX_COEF0_
{
    volatile struct
    {
        FIELD RC_CNV00                  : 11;
        FIELD rsv_11                    : 5;
        FIELD RC_CNV01                  : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_MATRIX_COEF0;

typedef volatile union _CAM_REG_AE_MATRIX_COEF1_
{
    volatile struct
    {
        FIELD RC_CNV02                  : 11;
        FIELD rsv_11                    : 5;
        FIELD RC_CNV10                  : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_MATRIX_COEF1;

typedef volatile union _CAM_REG_AE_MATRIX_COEF2_
{
    volatile struct
    {
        FIELD RC_CNV11                  : 11;
        FIELD rsv_11                    : 5;
        FIELD RC_CNV12                  : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_MATRIX_COEF2;

typedef volatile union _CAM_REG_AE_MATRIX_COEF3_
{
    volatile struct
    {
        FIELD RC_CNV20                  : 11;
        FIELD rsv_11                    : 5;
        FIELD RC_CNV21                  : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_MATRIX_COEF3;

typedef volatile union _CAM_REG_AE_MATRIX_COEF4_
{
    volatile struct
    {
        FIELD RC_CNV22                  : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_ACC                : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_MATRIX_COEF4;

typedef volatile union _CAM_REG_AE_YGAMMA_0_
{
    volatile struct
    {
        FIELD Y_GMR1                    : 8;
        FIELD Y_GMR2                    : 8;
        FIELD Y_GMR3                    : 8;
        FIELD Y_GMR4                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_YGAMMA_0;

typedef volatile union _CAM_REG_AE_YGAMMA_1_
{
    volatile struct
    {
        FIELD Y_GMR5                    : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_YGAMMA_1;

typedef volatile union _CAM_REG_AE_HST_SET_
{
    volatile struct
    {
        FIELD AE_HST0_BIN               : 2;
        FIELD rsv_2                     : 2;
        FIELD AE_HST1_BIN               : 2;
        FIELD rsv_6                     : 2;
        FIELD AE_HST2_BIN               : 2;
        FIELD rsv_10                    : 2;
        FIELD AE_HST3_BIN               : 2;
        FIELD rsv_14                    : 2;
        FIELD AE_HST0_COLOR             : 3;
        FIELD rsv_19                    : 1;
        FIELD AE_HST1_COLOR             : 3;
        FIELD rsv_23                    : 1;
        FIELD AE_HST2_COLOR             : 3;
        FIELD rsv_27                    : 1;
        FIELD AE_HST3_COLOR             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST_SET;

typedef volatile union _CAM_REG_AE_HST0_RNG_
{
    volatile struct
    {
        FIELD AE_HST0_X_LOW             : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_HST0_X_HI              : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_HST0_Y_LOW             : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_HST0_Y_HI              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST0_RNG;

typedef volatile union _CAM_REG_AE_HST1_RNG_
{
    volatile struct
    {
        FIELD AE_HST1_X_LOW             : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_HST1_X_HI              : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_HST1_Y_LOW             : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_HST1_Y_HI              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST1_RNG;

typedef volatile union _CAM_REG_AE_HST2_RNG_
{
    volatile struct
    {
        FIELD AE_HST2_X_LOW             : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_HST2_X_HI              : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_HST2_Y_LOW             : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_HST2_Y_HI              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST2_RNG;

typedef volatile union _CAM_REG_AE_HST3_RNG_
{
    volatile struct
    {
        FIELD AE_HST3_X_LOW             : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_HST3_X_HI              : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_HST3_Y_LOW             : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_HST3_Y_HI              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST3_RNG;

typedef volatile union _CAM_REG_AE_SPARE_
{
    volatile struct
    {
        FIELD AE_SPARE                  : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_SPARE;

/* end MT6589_2092_raw_ae.xml*/

/* start MT6589_210_raw_sgg.xml*/
typedef volatile union _CAM_REG_SGG_PGN_
{
    volatile struct
    {
        FIELD SGG_GAIN                  : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG_PGN;

typedef volatile union _CAM_REG_SGG_GMR_
{
    volatile struct
    {
        FIELD SGG_GMR1                  : 8;
        FIELD SGG_GMR2                  : 8;
        FIELD SGG_GMR3                  : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG_GMR;

/* end MT6589_210_raw_sgg.xml*/

/* start MT6589_211_raw_af.xml*/
typedef volatile union _CAM_REG_AF_CON_
{
    volatile struct
    {
        FIELD AF_DECI_1                 : 2;
        FIELD AF_ZIGZAG                 : 1;
        FIELD AF_ODD                    : 1;
        FIELD rsv_4                     : 24;
        FIELD AF_IIR_KEEP               : 1;
        FIELD RESERVED                  : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_CON;

typedef volatile union _CAM_REG_AF_WINX01_
{
    volatile struct
    {
        FIELD AF_WINX0                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINX1                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINX01;

typedef volatile union _CAM_REG_AF_WINX23_
{
    volatile struct
    {
        FIELD AF_WINX2                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINX3                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINX23;

typedef volatile union _CAM_REG_AF_WINX45_
{
    volatile struct
    {
        FIELD AF_WINX4                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINX5                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINX45;

typedef volatile union _CAM_REG_AF_WINY01_
{
    volatile struct
    {
        FIELD AF_WINY0                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINY1                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINY01;

typedef volatile union _CAM_REG_AF_WINY23_
{
    volatile struct
    {
        FIELD AF_WINY2                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINY3                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINY23;

typedef volatile union _CAM_REG_AF_WINY45_
{
    volatile struct
    {
        FIELD AF_WINY4                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINY5                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINY45;

typedef volatile union _CAM_REG_AF_SIZE_
{
    volatile struct
    {
        FIELD AF_XSIZE                  : 9;
        FIELD rsv_9                     : 7;
        FIELD AF_YSIZE                  : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_SIZE;

typedef volatile union _CAM_REG_AF_FILT0_
{
    volatile struct
    {
        FIELD AF_FILT_F0                : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FILT0;

typedef volatile union _CAM_REG_AF_FILT1_P14_
{
    volatile struct
    {
        FIELD AF_FILT1_P1               : 8;
        FIELD AF_FILT1_P2               : 8;
        FIELD AF_FILT1_P3               : 8;
        FIELD AF_FILT1_P4               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FILT1_P14;

typedef volatile union _CAM_REG_AF_FILT1_P58_
{
    volatile struct
    {
        FIELD AF_FILT1_P5               : 8;
        FIELD AF_FILT1_P6               : 8;
        FIELD AF_FILT1_P7               : 8;
        FIELD AF_FILT1_P8               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FILT1_P58;

typedef volatile union _CAM_REG_AF_FILT1_P912_
{
    volatile struct
    {
        FIELD AF_FILT1_P9               : 8;
        FIELD AF_FILT1_P10              : 8;
        FIELD AF_FILT1_P11              : 8;
        FIELD AF_FILT1_P12              : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FILT1_P912;

typedef volatile union _CAM_REG_AF_TH_
{
    volatile struct
    {
        FIELD AF_TH0                    : 8;
        FIELD AF_TH1                    : 8;
        FIELD AF_TH2                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_TH;

typedef volatile union _CAM_REG_AF_WIN_E_
{
    volatile struct
    {
        FIELD AF_WINXE                  : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINYE                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WIN_E;

typedef volatile union _CAM_REG_AF_SIZE_E_
{
    volatile struct
    {
        FIELD AF_SIZE_XE                : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_SIZE_YE                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_SIZE_E;

typedef volatile union _CAM_REG_AF_TH_E_
{
    volatile struct
    {
        FIELD AF_TH0EX                  : 8;
        FIELD AF_TH1EX                  : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_TH_E;

typedef volatile union _CAM_REG_AF_IN_SIZE_
{
    volatile struct
    {
        FIELD AF_IN_HSIZE               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_IN_SIZE;

/* end MT6589_211_raw_af.xml*/

/* start MT6589_212_raw_flk.xml*/
typedef volatile union _CAM_REG_FLK_CON_
{
    volatile struct
    {
        FIELD FLK_MODE                  : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_CON;

typedef volatile union _CAM_REG_FLK_SOFST_
{
    volatile struct
    {
        FIELD FLK_SOFST_X               : 12;
        FIELD rsv_12                    : 4;
        FIELD FLK_SOFST_Y               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_SOFST;

typedef volatile union _CAM_REG_FLK_WSIZE_
{
    volatile struct
    {
        FIELD FLK_WSIZE_X               : 12;
        FIELD rsv_12                    : 4;
        FIELD FLK_WSIZE_Y               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_WSIZE;

typedef volatile union _CAM_REG_FLK_WNUM_
{
    volatile struct
    {
        FIELD FLK_WNUM_X                : 3;
        FIELD rsv_3                     : 1;
        FIELD FLK_WNUM_Y                : 3;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_WNUM;

/* end MT6589_212_raw_flk.xml*/

/* start MT6589_213_raw_lcs.xml*/
typedef volatile union _CAM_REG_LCS_CON_
{
    volatile struct
    {
        FIELD LCS_LOG2                  : 1;
        FIELD rsv_1                     : 15;
        FIELD LCS_OUT_WIDTH             : 7;
        FIELD rsv_23                    : 1;
        FIELD LCS_OUT_HEIGHT            : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_CON;

typedef volatile union _CAM_REG_LCS_ST_
{
    volatile struct
    {
        FIELD LCS_START_J               : 13;
        FIELD rsv_13                    : 3;
        FIELD LCS_START_I               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_ST;

typedef volatile union _CAM_REG_LCS_AWS_
{
    volatile struct
    {
        FIELD LCS_IN_WIDTH              : 13;
        FIELD rsv_13                    : 3;
        FIELD LCS_IN_HEIGHT             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_AWS;

typedef volatile union _CAM_REG_LCS_FLARE_
{
    volatile struct
    {
        FIELD LCS_FLARE_OFFSET          : 7;
        FIELD LCS_FLARE_GAIN            : 9;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_FLARE;

typedef volatile union _CAM_REG_LCS_LRZ_HDR_
{
    volatile struct
    {
        FIELD LCS_RATIO_X               : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_LRZ_HDR;

typedef volatile union _CAM_REG_LCS_LRZ_VDR_
{
    volatile struct
    {
        FIELD LCS_RATIO_Y               : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_LRZ_VDR;

/* end MT6589_213_raw_lcs.xml*/

/* start MT6589_216_raw_bpc.xml*/
typedef volatile union _CAM_REG_BPC_CON_
{
    volatile struct
    {
        FIELD BPC_ENABLE                : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_TABLE_ENABLE          : 1;
        FIELD BPC_TABLE_END_MODE        : 1;
        FIELD rsv_6                     : 2;
        FIELD BPC_HF_MODE               : 1;
        FIELD BPC_THR_MODE              : 1;
        FIELD rsv_10                    : 2;
        FIELD BPC_DETECTION_MODE        : 2;
        FIELD rsv_14                    : 2;
        FIELD BPC_CORRECTION_MODE       : 2;
        FIELD rsv_18                    : 2;
        FIELD BPC_OPT_ENABLE : 1;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CON;

typedef volatile union _CAM_REG_BPC_CD1_1_
{
    volatile struct
    {
        FIELD BPC_CON_DARK_G_THR_LA     : 8;
        FIELD BPC_CON_DARK_G_THR_LB     : 8;
        FIELD BPC_CON_DARK_G_THR_LC     : 8;
        FIELD BPC_CON_DARK_G_LA         : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD1_1;

typedef volatile union _CAM_REG_BPC_CD1_2_
{
    volatile struct
    {
        FIELD BPC_CON_DARK_G_LB         : 8;
        FIELD BPC_CON_BRIGHT_G_THR_LA   : 8;
        FIELD BPC_CON_BRIGHT_G_THR_LB   : 8;
        FIELD BPC_CON_BRIGHT_G_THR_LC   : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD1_2;

typedef volatile union _CAM_REG_BPC_CD1_3_
{
    volatile struct
    {
        FIELD BPC_CON_BRIGHT_G_LA       : 8;
        FIELD BPC_CON_BRIGHT_G_LB       : 8;
        FIELD BPC_CON_DARK_RB_THR_LA    : 8;
        FIELD BPC_CON_DARK_RB_THR_LB    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD1_3;

typedef volatile union _CAM_REG_BPC_CD1_4_
{
    volatile struct
    {
        FIELD BPC_CON_DARK_RB_THR_LC    : 8;
        FIELD BPC_CON_DARK_RB_LA        : 8;
        FIELD BPC_CON_DARK_RB_LB        : 8;
        FIELD BPC_CON_BRIGHT_RB_THR_LA  : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD1_4;

typedef volatile union _CAM_REG_BPC_CD1_5_
{
    volatile struct
    {
        FIELD BPC_CON_BRIGHT_RB_THR_LB  : 8;
        FIELD BPC_CON_BRIGHT_RB_THR_LC  : 8;
        FIELD BPC_CON_BRIGHT_RB_LA      : 8;
        FIELD BPC_CON_BRIGHT_RB_LB      : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD1_5;

typedef volatile union _CAM_REG_BPC_CD1_6_
{
    volatile struct
    {
        FIELD BPC_CON_CLAMP_VAR_G_LB    : 6;
        FIELD BPC_CON_CLAMP_VAR_G_UB    : 10;
        FIELD BPC_CON_CLAMP_VAR_RB_LB   : 6;
        FIELD BPC_CON_CLAMP_VAR_RB_UB   : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD1_6;

typedef volatile union _CAM_REG_BPC_CD2_1_
{
    volatile struct
    {
        FIELD BPC_HF_OFFSET_G           : 12;
        FIELD BPC_HF_SEG_G              : 12;
        FIELD BPC_HF_SLOPE_A_G          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD2_1;

typedef volatile union _CAM_REG_BPC_CD2_2_
{
    volatile struct
    {
        FIELD BPC_HF_OFFSET_RB          : 12;
        FIELD BPC_HF_SEG_RB             : 12;
        FIELD BPC_HF_SLOPE_A_RB         : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD2_2;

typedef volatile union _CAM_REG_BPC_CD2_3_
{
    volatile struct
    {
        FIELD BPC_HF_VAR_THR            : 10;
        FIELD rsv_10                    : 2;
        FIELD BPC_HF_DIFF_THR           : 4;
        FIELD BPC_CON_AVG_OFFSET        : 5;
        FIELD rsv_21                    : 3;
        FIELD BPC_HF_OFFSET_ALL         : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD2_3;

typedef volatile union _CAM_REG_BPC_CD0_
{
    volatile struct
    {
        FIELD BPC_IN_RANGE_NUM          : 4;
        FIELD rsv_4                     : 4;
        FIELD BPC_HF_SLOPE_B_G          : 6;
        FIELD rsv_14                    : 2;
        FIELD BPC_HF_SLOPE_B_RB         : 6;
        FIELD rsv_22                    : 2;
        FIELD BPC_CON_EXTRA_THR         : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CD0;

typedef volatile union _CAM_REG_BPC_DET_
{
    volatile struct
    {
        FIELD BPC_STRONG_BP_D_THR_LA    : 8;
        FIELD BPC_STRONG_BP_B_THR_LA    : 8;
        FIELD BPC_STRONG_BP_D_THR_LB    : 8;
        FIELD BPC_STRONG_BP_B_THR_LB    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_DET;

typedef volatile union _CAM_REG_BPC_COR_
{
    volatile struct
    {
        FIELD BPC_CR2_MAX       : 8;
        FIELD BPC_CR2_THR       : 8;
        FIELD BPC_CR1         : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_COR;

typedef volatile union _CAM_REG_BPC_TBLI1_
{
    volatile struct
    {
        FIELD BPC_XOFFSET               : 13;
        FIELD rsv_13                    : 3;
        FIELD BPC_YOFFSET               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TBLI1;

typedef volatile union _CAM_REG_BPC_TBLI2_
{
    volatile struct
    {
        FIELD BPC_XSIZE                 : 13;
        FIELD rsv_13                    : 3;
        FIELD BPC_YSIZE                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TBLI2;

/* end MT6589_216_raw_bpc.xml*/

/* start MT6589_216_raw_ct.xml*/
typedef volatile union _CAM_REG_NR1_CON_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD NR1_CT_EN                 : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR1_CON;

typedef volatile union _CAM_REG_NR1_CT_CON_
{
    volatile struct
    {
        FIELD NR1_CT_MD                 : 2;
        FIELD NR1_CT_MD2                : 2;
        FIELD NR1_CT_THRD               : 10;
        FIELD rsv_14                    : 2;
        FIELD NR1_MBND                  : 10;
        FIELD rsv_26                    : 2;
        FIELD NR1_CT_SLOPE              : 2;
        FIELD NR1_CT_DIV                : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR1_CT_CON;

typedef volatile union _CAM_REG_BNR_RSV1_
{
    volatile struct
    {
        FIELD RSV1                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BNR_RSV1;

typedef volatile union _CAM_REG_BNR_RSV2_
{
    volatile struct
    {
        FIELD RSV2                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BNR_RSV2;

/* end MT6589_216_raw_ct.xml*/

/* start MT6589_217_raw_pgn.xml*/
typedef volatile union _CAM_REG_PGN_SATU01_
{
    volatile struct
    {
        FIELD PGN_CH0_SATU              : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_CH1_SATU              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_SATU01;

typedef volatile union _CAM_REG_PGN_SATU23_
{
    volatile struct
    {
        FIELD PGN_CH2_SATU              : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_CH3_SATU              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_SATU23;

typedef volatile union _CAM_REG_PGN_GAIN01_
{
    volatile struct
    {
        FIELD PGN_CH0_GAIN              : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_CH1_GAIN              : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_GAIN01;

typedef volatile union _CAM_REG_PGN_GAIN23_
{
    volatile struct
    {
        FIELD PGN_CH2_GAIN              : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_CH3_GAIN              : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_GAIN23;

typedef volatile union _CAM_REG_PGN_OFFS01_
{
    volatile struct
    {
        FIELD PGN_CH0_OFFS              : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_CH1_OFFS              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_OFFS01;

typedef volatile union _CAM_REG_PGN_OFFS23_
{
    volatile struct
    {
        FIELD PGN_CH2_OFFS              : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_CH3_OFFS              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_OFFS23;

/* end MT6589_217_raw_pgn.xml*/

/* start MT6589_300_rgb_cfa.xml*/
typedef volatile union _CAM_REG_CFA_BYPASS_
{
    volatile struct
    {
        FIELD BAYER_BYPASS              : 1;
        FIELD BAYER_DEBUG_MODE          : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_BYPASS;

typedef volatile union _CAM_REG_CFA_ED_F_
{
    volatile struct
    {
        FIELD BAYER_FLAT_DET_MODE       : 1;
        FIELD BAYER_STEP_DET_MODE       : 1;
        FIELD rsv_2                     : 6;
        FIELD BAYER_FLAT_TH             : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_ED_F;

typedef volatile union _CAM_REG_CFA_ED_NYQ_
{
    volatile struct
    {
        FIELD BAYER_NYQ_TH              : 8;
        FIELD BAYER_NYQ_TH2             : 8;
        FIELD BAYER_NYQ_TH3             : 8;
        FIELD BAYER_HF_NYQ_GAIN         : 2;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_ED_NYQ;

typedef volatile union _CAM_REG_CFA_ED_STEP_
{
    volatile struct
    {
        FIELD BAYER_STEP_TH             : 8;
        FIELD BAYER_STEP2_TH            : 8;
        FIELD BAYER_STEP3_TH            : 8;
        FIELD rsv_24                    : 7;
        FIELD BAYER_RB_MODE             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_ED_STEP;

typedef volatile union _CAM_REG_CFA_RGB_HF_
{
    volatile struct
    {
        FIELD BAYER_HF_CORE_G           : 5;
        FIELD BAYER_RB_ROUGH_F          : 5;
        FIELD BAYER_RB_ROUGH_D          : 5;
        FIELD BAYER_G_ROUGH_F           : 5;
        FIELD BAYER_G_ROUGH_D           : 5;
        FIELD BAYER_RB_MODE_F           : 2;
        FIELD BAYER_RB_MODE_D           : 2;
        FIELD BAYER_RB_MODE_HV          : 2;
        FIELD BAYER_SSG_MODE            : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_RGB_HF;

typedef volatile union _CAM_REG_CFA_BW_
{
    volatile struct
    {
        FIELD BAYER_BD_TH               : 8;
        FIELD BAYER_WD_TH               : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_BW;

typedef volatile union _CAM_REG_CFA_F1_ACT_
{
    volatile struct
    {
        FIELD BAYER_F1_TH1              : 9;
        FIELD BAYER_F1_TH2              : 9;
        FIELD BAYER_F1_SLOPE1           : 2;
        FIELD BAYER_F1_SLOPE2           : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F1_ACT;

typedef volatile union _CAM_REG_CFA_F2_ACT_
{
    volatile struct
    {
        FIELD BAYER_F2_TH1              : 9;
        FIELD BAYER_F2_TH2              : 9;
        FIELD BAYER_F2_SLOPE1           : 2;
        FIELD BAYER_F2_SLOPE2           : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F2_ACT;

typedef volatile union _CAM_REG_CFA_F3_ACT_
{
    volatile struct
    {
        FIELD BAYER_F3_TH1              : 9;
        FIELD BAYER_F3_TH2              : 9;
        FIELD BAYER_F3_SLOPE1           : 2;
        FIELD BAYER_F3_SLOPE2           : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F3_ACT;

typedef volatile union _CAM_REG_CFA_F4_ACT_
{
    volatile struct
    {
        FIELD BAYER_F4_TH1              : 9;
        FIELD BAYER_F4_TH2              : 9;
        FIELD BAYER_F4_SLOPE1           : 2;
        FIELD BAYER_F4_SLOPE2           : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F4_ACT;

typedef volatile union _CAM_REG_CFA_F1_L_
{
    volatile struct
    {
        FIELD BAYER_F1_L_LUT_Y0         : 5;
        FIELD BAYER_F1_L_LUT_Y1         : 5;
        FIELD BAYER_F1_L_LUT_Y2         : 5;
        FIELD BAYER_F1_L_LUT_Y3         : 5;
        FIELD BAYER_F1_L_LUT_Y4         : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F1_L;

typedef volatile union _CAM_REG_CFA_F2_L_
{
    volatile struct
    {
        FIELD BAYER_F2_L_LUT_Y0         : 5;
        FIELD BAYER_F2_L_LUT_Y1         : 5;
        FIELD BAYER_F2_L_LUT_Y2         : 5;
        FIELD BAYER_F2_L_LUT_Y3         : 5;
        FIELD BAYER_F2_L_LUT_Y4         : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F2_L;

typedef volatile union _CAM_REG_CFA_F3_L_
{
    volatile struct
    {
        FIELD BAYER_F3_L_LUT_Y0         : 5;
        FIELD BAYER_F3_L_LUT_Y1         : 5;
        FIELD BAYER_F3_L_LUT_Y2         : 5;
        FIELD BAYER_F3_L_LUT_Y3         : 5;
        FIELD BAYER_F3_L_LUT_Y4         : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F3_L;

typedef volatile union _CAM_REG_CFA_F4_L_
{
    volatile struct
    {
        FIELD BAYER_F4_L_LUT_Y0         : 5;
        FIELD BAYER_F4_L_LUT_Y1         : 5;
        FIELD BAYER_F4_L_LUT_Y2         : 5;
        FIELD BAYER_F4_L_LUT_Y3         : 5;
        FIELD BAYER_F4_L_LUT_Y4         : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_F4_L;

typedef volatile union _CAM_REG_CFA_HF_RB_
{
    volatile struct
    {
        FIELD BAYER_RB_DIFF_TH          : 10;
        FIELD BAYER_HF_CLIP             : 9;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_HF_RB;

typedef volatile union _CAM_REG_CFA_HF_GAIN_
{
    volatile struct
    {
        FIELD HF_GLOBAL_GAIN            : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_HF_GAIN;

typedef volatile union _CAM_REG_CFA_HF_COMP_
{
    volatile struct
    {
        FIELD HF_LSC_GAIN0              : 4;
        FIELD HF_LSC_GAIN1              : 4;
        FIELD HF_LSC_GAIN2              : 4;
        FIELD HF_LSC_GAIN3              : 4;
        FIELD HF_UNDER_TH               : 8;
        FIELD HF_UNDER_ACT_TH           : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_HF_COMP;

typedef volatile union _CAM_REG_CFA_HF_CORING_TH_
{
    volatile struct
    {
        FIELD HF_CORING_TH              : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_HF_CORING_TH;

typedef volatile union _CAM_REG_CFA_ACT_LUT_
{
    volatile struct
    {
        FIELD BAYER_ACT_LUT_Y0          : 5;
        FIELD BAYER_ACT_LUT_Y1          : 5;
        FIELD BAYER_ACT_LUT_Y2          : 5;
        FIELD BAYER_ACT_LUT_Y3          : 5;
        FIELD BAYER_ACT_LUT_Y4          : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_ACT_LUT;

typedef volatile union _CAM_REG_CFA_SPARE_
{
    volatile struct
    {
        FIELD CFA_SPARE                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_SPARE;

typedef volatile union _CAM_REG_CFA_BB_
{
    volatile struct
    {
        FIELD BAYER_BB_TH1              : 9;
        FIELD rsv_9                     : 3;
        FIELD BAYER_BB_TH2              : 9;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CFA_BB;

/* end MT6589_300_rgb_cfa.xml*/

/* start MT6589_301_rgb_ccl.xml*/
typedef volatile union _CAM_REG_CCL_GTC_
{
    volatile struct
    {
        FIELD CCL_TC                    : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCL_GTC;

typedef volatile union _CAM_REG_CCL_ADC_
{
    volatile struct
    {
        FIELD CCL_D_TH1                 : 12;
        FIELD rsv_12                    : 4;
        FIELD CCL_D_SLOPE               : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCL_ADC;

typedef volatile union _CAM_REG_CCL_BAC_
{
    volatile struct
    {
        FIELD CCL_B_OFFSET              : 12;
        FIELD CCL_B_SLOPE               : 4;
        FIELD CCL_B_DIFF                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCL_BAC;

/* end MT6589_301_rgb_ccl.xml*/

/* start MT6589_302_rgb_g2g.xml*/
typedef volatile union _CAM_REG_G2G_CONV0A_
{
    volatile struct
    {
        FIELD G2G_CNV_00                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2G_CNV_01                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CONV0A;

typedef volatile union _CAM_REG_G2G_CONV0B_
{
    volatile struct
    {
        FIELD G2G_CNV_02                : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CONV0B;

typedef volatile union _CAM_REG_G2G_CONV1A_
{
    volatile struct
    {
        FIELD G2G_CNV_10                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2G_CNV_11                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CONV1A;

typedef volatile union _CAM_REG_G2G_CONV1B_
{
    volatile struct
    {
        FIELD G2G_CNV_12                : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CONV1B;

typedef volatile union _CAM_REG_G2G_CONV2A_
{
    volatile struct
    {
        FIELD G2G_CNV_20                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2G_CNV_21                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CONV2A;

typedef volatile union _CAM_REG_G2G_CONV2B_
{
    volatile struct
    {
        FIELD G2G_CNV_22                : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CONV2B;

typedef volatile union _CAM_REG_G2G_ACC_
{
    volatile struct
    {
        FIELD G2G_ACC                   : 4;
        FIELD G2G_MOFFS_R               : 1;
        FIELD G2G_POFFS_R               : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_ACC;

/* end MT6589_302_rgb_g2g.xml*/

/* start MT6589_304_raw_unp.xml*/
typedef volatile union _CAM_REG_UNP_OFST_
{
    volatile struct
    {
        FIELD UNP_ST_OFST               : 4;
        FIELD UNP_END_OFST              : 4;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_UNP_OFST;

/* end MT6589_304_raw_unp.xml*/

/* start MT6589_305_rgb_c02.xml*/
typedef volatile union _CAM_REG_C02_CON_
{
    volatile struct
    {
        FIELD C02_MODE                  : 1;
        FIELD C02_OFFSET_Y              : 1;
        FIELD rsv_2                     : 14;
        FIELD C02_OUTPUT_YSIZE          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_C02_CON;

/* end MT6589_305_rgb_c02.xml*/

/* start MT6589_306_rgb_mfb.xml*/
typedef volatile union _CAM_REG_MFB_CON_
{
    volatile struct
    {
        FIELD BLD_MODE                  : 2;
        FIELD rsv_2                     : 2;
        FIELD BLD_DB_EN            : 1;
        FIELD rsv_5                     : 3;
        FIELD BLD_XDIST                 : 4;
        FIELD BLD_YDIST                 : 4;
        FIELD BLD_TH_E               : 8;
        FIELD BLD_MM_TH                 : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_CON;

typedef volatile union _CAM_REG_MFB_BRZ_
{
    volatile struct
    {
        FIELD BLD_OUTPUT_XSIZE          : 13;
        FIELD rsv_13                    : 2;
        FIELD BLD_OFFSET_X              : 1;
        FIELD BLD_OUTPUT_YSIZE          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_BRZ;

typedef volatile union _CAM_REG_MFB_LL_
{
    volatile struct
    {
        FIELD BLD_MAX_WEIGHT            : 3;
        FIELD rsv_3                     : 1;
        FIELD RESERVED                  : 4;
        FIELD BLD_LL_Y_DT               : 8;
        FIELD BLD_LL_Y_TH1              : 8;
        FIELD BLD_LL_Y_TH2              : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL;

/* end MT6589_306_rgb_mfb.xml*/

/* start MT6589_307_rgb_lce.xml*/
typedef volatile union _CAM_REG_LCE_CON_
{
    volatile struct
    {
        FIELD LCE_GLOBAL                : 1;
        FIELD LCE_GLOBAL_VHALF          : 7;
        FIELD LCE_LINK_MODE             : 2;
        FIELD rsv_10                    : 18;
        FIELD RESERVED                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_CON;

typedef volatile union _CAM_REG_LCE_ZR_
{
    volatile struct
    {
        FIELD LCE_BC_MAG_KUBNX          : 15;
        FIELD rsv_15                    : 1;
        FIELD LCE_BC_MAG_KUBNY          : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_ZR;

typedef volatile union _CAM_REG_LCE_QUA_
{
    volatile struct
    {
        FIELD LCE_PA                    : 7;
        FIELD LCE_PB                    : 9;
        FIELD LCE_BA                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_QUA;

typedef volatile union _CAM_REG_LCE_THR_
{
    volatile struct
    {
        FIELD LCE_ANR_THR1              : 6;
        FIELD rsv_6                     : 2;
        FIELD LCE_ANR_THR2              : 6;
        FIELD rsv_14                    : 2;
        FIELD LCE_ANR_THR3              : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_THR;

typedef volatile union _CAM_REG_LCE_HSI_
{
    volatile struct
    {
        FIELD LCE_HS_IN1                : 12;
        FIELD rsv_12                    : 4;
        FIELD LCE_HS_IN2                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_HSI;

typedef volatile union _CAM_REG_LCE_HSO_
{
    volatile struct
    {
        FIELD LCE_HS_OUT1               : 12;
        FIELD rsv_12                    : 4;
        FIELD LCE_HS_OUT2               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_HSO;

typedef volatile union _CAM_REG_LCE_HSS1_
{
    volatile struct
    {
        FIELD LCE_HS_SLOPE1             : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_HSS1;

typedef volatile union _CAM_REG_LCE_HSS2_
{
    volatile struct
    {
        FIELD LCE_HS_SLOPE2             : 12;
        FIELD rsv_12                    : 4;
        FIELD LCE_HS_SLOPE3             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_HSS2;

typedef volatile union _CAM_REG_LCE_SLM_LB_
{
    volatile struct
    {
        FIELD LCE_SLM_LB1               : 7;
        FIELD rsv_7                     : 1;
        FIELD LCE_SLM_LB2               : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_SLM_LB;

typedef volatile union _CAM_REG_LCE_SLM_SIZE_
{
    volatile struct
    {
        FIELD LCE_SLM_WIDTH             : 7;
        FIELD rsv_7                     : 1;
        FIELD LCE_SLM_HEIGHT            : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_SLM_SIZE;

typedef volatile union _CAM_REG_LCE_OFST_
{
    volatile struct
    {
        FIELD LCE_OFFSET_X              : 15;
        FIELD rsv_15                    : 1;
        FIELD LCE_OFFSET_Y              : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_OFST;

typedef volatile union _CAM_REG_LCE_BIAS_
{
    volatile struct
    {
        FIELD LCE_BIAS_X                : 3;
        FIELD rsv_3                     : 5;
        FIELD LCE_BIAS_Y                : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_BIAS;

typedef volatile union _CAM_REG_LCE_TPIPE_SIZE_
{
    volatile struct
    {
        FIELD LCE_TPIPE_XSIZE            : 13;
        FIELD rsv_13                    : 3;
        FIELD LCE_TPIPE_YSIZE            : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_TPIPE_SIZE;

/* end MT6589_307_rgb_lce.xml*/

/* start MT6589_400_yuv_g2c.xml*/
typedef volatile union _CAM_REG_G2C_CONV_0A_
{
    volatile struct
    {
        FIELD G2C_CNV00                 : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV01                 : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_0A;

typedef volatile union _CAM_REG_G2C_CONV_0B_
{
    volatile struct
    {
        FIELD G2C_CNV02                 : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_YOFFSET11             : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_0B;

typedef volatile union _CAM_REG_G2C_CONV_1A_
{
    volatile struct
    {
        FIELD G2C_CNV10                 : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV11                 : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_1A;

typedef volatile union _CAM_REG_G2C_CONV_1B_
{
    volatile struct
    {
        FIELD G2C_CNV12                 : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_UOFFSET10             : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_1B;

typedef volatile union _CAM_REG_G2C_CONV_2A_
{
    volatile struct
    {
        FIELD G2C_CNV20                 : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV21                 : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_2A;

typedef volatile union _CAM_REG_G2C_CONV_2B_
{
    volatile struct
    {
        FIELD G2C_CNV22                 : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_VOFFSET10             : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_2B;

/* end MT6589_400_yuv_g2c.xml*/

/* start MT6589_401_yuv_c42.xml*/
typedef volatile union _CAM_REG_C42_CON_
{
    volatile struct
    {
        FIELD C42_FILT_DIS              : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_C42_CON;

/* end MT6589_401_yuv_c42.xml*/

/* start MT6589_402_yuv_nbc.xml*/
typedef volatile union _CAM_REG_ANR_CON1_
{
    volatile struct
    {
        FIELD ANR_ENC                   : 1;
        FIELD ANR_ENY                   : 1;
        FIELD rsv_2                     : 2;
        FIELD ANR_SCALE_MODE            : 2;
        FIELD rsv_6                     : 2;
        FIELD ANR_FLT_MODE              : 3;
        FIELD rsv_11                    : 1;
        FIELD ANR_MODE                  : 1;
        FIELD rsv_13                    : 3;
        FIELD ANR_Y_LUMA_SCALE          : 4;
        FIELD rsv_20                    : 4;
        FIELD ANR_LCE_LINK              : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CON1;

typedef volatile union _CAM_REG_ANR_CON2_
{
    volatile struct
    {
        FIELD ANR_GNY                   : 6;
        FIELD rsv_6                     : 2;
        FIELD ANR_GNC                   : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CON2;

typedef volatile union _CAM_REG_ANR_CON3_
{
    volatile struct
    {
        FIELD ANR_IMPL_MODE             : 2;
        FIELD rsv_2                     : 2;
        FIELD ANR_C_MEDIAN_EN           : 2;
        FIELD rsv_6                     : 2;
        FIELD ANR_C_SM_EDGE             : 1;
        FIELD rsv_9                     : 3;
        FIELD ANR_QEC               : 1;
        FIELD rsv_13                    : 3;
        FIELD ANR_QEC_VAL           : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CON3;

typedef volatile union _CAM_REG_ANR_YAD1_
{
    volatile struct
    {
        FIELD ANR_CEN_GAIN_LO_TH        : 5;
        FIELD rsv_5                     : 3;
        FIELD ANR_CEN_GAIN_HI_TH        : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_K_LO_TH           : 4;
        FIELD rsv_20                    : 4;
        FIELD ANR_K_HI_TH           : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_YAD1;

typedef volatile union _CAM_REG_ANR_YAD2_
{
    volatile struct
    {
        FIELD ANR_PTY_VGAIN             : 4;
        FIELD rsv_4                     : 4;
        FIELD ANR_PTY_GAIN_TH           : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_YAD2;

typedef volatile union _CAM_REG_ANR_4LUT1_
{
    volatile struct
    {
        FIELD ANR_Y_CPX1                : 8;
        FIELD ANR_Y_CPX2                : 8;
        FIELD ANR_Y_CPX3                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_4LUT1;

typedef volatile union _CAM_REG_ANR_4LUT2_
{
    volatile struct
    {
        FIELD ANR_Y_SCALE_CPY0          : 5;
        FIELD rsv_5                     : 3;
        FIELD ANR_Y_SCALE_CPY1          : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_Y_SCALE_CPY2          : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_Y_SCALE_CPY3          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_4LUT2;

typedef volatile union _CAM_REG_ANR_4LUT3_
{
    volatile struct
    {
        FIELD ANR_Y_SCALE_SP0           : 5;
        FIELD rsv_5                     : 3;
        FIELD ANR_Y_SCALE_SP1           : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_Y_SCALE_SP2           : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_Y_SCALE_SP3           : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_4LUT3;

typedef volatile union _CAM_REG_ANR_PTY_
{
    volatile struct
    {
        FIELD ANR_PTY1                  : 8;
        FIELD ANR_PTY2                  : 8;
        FIELD ANR_PTY3                  : 8;
        FIELD ANR_PTY4                  : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_PTY;

typedef volatile union _CAM_REG_ANR_CAD_
{
    volatile struct
    {
        FIELD ANR_PTC_VGAIN             : 4;
        FIELD rsv_4                     : 4;
        FIELD ANR_PTC_GAIN_TH           : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_C_L_DIFF_TH           : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CAD;

typedef volatile union _CAM_REG_ANR_PTC_
{
    volatile struct
    {
        FIELD ANR_PTC1                  : 8;
        FIELD ANR_PTC2                  : 8;
        FIELD ANR_PTC3                  : 8;
        FIELD ANR_PTC4                  : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_PTC;

typedef volatile union _CAM_REG_ANR_LCE1_
{
    volatile struct
    {
        FIELD ANR_LCE_C_GAIN            : 4;
        FIELD ANR_LCE_SCALE_GAIN        : 3;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_LCE1;

typedef volatile union _CAM_REG_ANR_LCE2_
{
    volatile struct
    {
        FIELD ANR_LCE_GAIN0             : 6;
        FIELD rsv_6                     : 2;
        FIELD ANR_LCE_GAIN1             : 6;
        FIELD rsv_14                    : 2;
        FIELD ANR_LCE_GAIN2             : 6;
        FIELD rsv_22                    : 2;
        FIELD ANR_LCE_GAIN3             : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_LCE2;

typedef volatile union _CAM_REG_ANR_HP1_
{
    volatile struct
    {
        FIELD ANR_HP_A                  : 8;
        FIELD ANR_HP_B                  : 6;
        FIELD rsv_14                    : 2;
        FIELD ANR_HP_C                  : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_HP_D                  : 4;
        FIELD ANR_HP_E                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_HP1;

typedef volatile union _CAM_REG_ANR_HP2_
{
    volatile struct
    {
        FIELD ANR_HP_S1                 : 4;
        FIELD ANR_HP_S2                 : 4;
        FIELD ANR_HP_X1                 : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_HP_F                  : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_HP2;

typedef volatile union _CAM_REG_ANR_HP3_
{
    volatile struct
    {
        FIELD ANR_HP_Y_GAIN_CLIP        : 7;
        FIELD rsv_7                     : 1;
        FIELD ANR_HP_Y_SP               : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_HP_Y_LO               : 8;
        FIELD ANR_HP_CLIP               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_HP3;

typedef volatile union _CAM_REG_ANR_ACTY_
{
    volatile struct
    {
        FIELD ANR_ACT_TH_Y              : 8;
        FIELD ANR_ACT_BLD_BASE_Y        : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_ACT_SLANT_Y           : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_ACT_BLD_TH_Y          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_ACTY;

typedef volatile union _CAM_REG_ANR_ACTC_
{
    volatile struct
    {
        FIELD ANR_ACT_TH_C              : 8;
        FIELD ANR_ACT_BLD_BASE_C        : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_ACT_SLANT_C           : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_ACT_BLD_TH_C          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_ACTC;

typedef volatile union _CAM_REG_ANR_RSV1_
{
    volatile struct
    {
        FIELD RSV1                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_RSV1;

typedef volatile union _CAM_REG_ANR_RSV2_
{
    volatile struct
    {
        FIELD RSV2                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_RSV2;

typedef volatile union _CAM_REG_CCR_CON_
{
    volatile struct
    {
        FIELD CCR_EN                    : 1;
        FIELD rsv_1                     : 7;
        FIELD CCR_UV_GAIN_MODE          : 1;
        FIELD rsv_9                     : 15;
        FIELD CCR_Y_CPX3                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_CON;

typedef volatile union _CAM_REG_CCR_YLUT_
{
    volatile struct
    {
        FIELD CCR_Y_CPX1                : 8;
        FIELD CCR_Y_CPX2                : 8;
        FIELD CCR_Y_SP1                 : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPY1                : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_YLUT;

typedef volatile union _CAM_REG_CCR_UVLUT_
{
    volatile struct
    {
        FIELD CCR_UV_X1                 : 8;
        FIELD CCR_UV_X2                 : 8;
        FIELD CCR_UV_GAIN_SP1           : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_UV_GAIN1              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_UVLUT;

typedef volatile union _CAM_REG_CCR_YLUT2_
{
    volatile struct
    {
        FIELD CCR_Y_SP0                 : 7;
        FIELD rsv_7                     : 1;
        FIELD CCR_Y_SP2                 : 7;
        FIELD rsv_15                    : 1;
        FIELD CCR_Y_CPY0                : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPY2                : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_YLUT2;

/* end MT6589_402_yuv_nbc.xml*/

/* start MT6589_403_yuv_seee.xml*/
typedef volatile union _CAM_REG_SEEE_SRK_CTRL_
{
    volatile struct
    {
        FIELD USM_OVER_SHRINK_EN        : 1;
        FIELD rsv_1                     : 27;
        FIELD RESERVED                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SRK_CTRL;

typedef volatile union _CAM_REG_SEEE_CLIP_CTRL_
{
    volatile struct
    {
        FIELD USM_OVER_CLIP_EN          : 1;
        FIELD USM_CLIP_OVER             : 7;
        FIELD USM_CLIP_UNDER            : 7;
        FIELD rsv_15                    : 1;
        FIELD USM_CLIP                  : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_CLIP_CTRL;

typedef volatile union _CAM_REG_SEEE_HP_CTRL1_
{
    volatile struct
    {
        FIELD USM_HP_TH                 : 8;
        FIELD USM_HP_AMP                : 3;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_HP_CTRL1;

typedef volatile union _CAM_REG_SEEE_HP_CTRL2_
{
    volatile struct
    {
        FIELD USM_HP_A               : 8;
        FIELD USM_HP_B               : 6;
        FIELD USM_HP_C               : 5;
        FIELD USM_HP_D               : 4;
        FIELD USM_HP_E               : 4;
        FIELD USM_HP_F               : 3;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_HP_CTRL2;

typedef volatile union _CAM_REG_SEEE_ED_CTRL1_
{
    volatile struct
    {
        FIELD USM_ED_X1                 : 8;
        FIELD USM_ED_S1                 : 8;
        FIELD USM_ED_Y1                 : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL1;

typedef volatile union _CAM_REG_SEEE_ED_CTRL2_
{
    volatile struct
    {
        FIELD USM_ED_X2                 : 8;
        FIELD USM_ED_S2                 : 8;
        FIELD USM_ED_Y2                 : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL2;

typedef volatile union _CAM_REG_SEEE_ED_CTRL3_
{
    volatile struct
    {
        FIELD USM_ED_X3                 : 8;
        FIELD USM_ED_S3                 : 8;
        FIELD USM_ED_Y3                 : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL3;

typedef volatile union _CAM_REG_SEEE_ED_CTRL4_
{
    volatile struct
    {
        FIELD USM_ED_X4                 : 8;
        FIELD USM_ED_S4                 : 8;
        FIELD USM_ED_Y4                 : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL4;

typedef volatile union _CAM_REG_SEEE_ED_CTRL5_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD USM_ED_S5                 : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL5;

typedef volatile union _CAM_REG_SEEE_ED_CTRL6_
{
    volatile struct
    {
        FIELD USM_ED_TH_OVER            : 8;
        FIELD USM_ED_TH_UNDER           : 8;
        FIELD USM_ED_TH_MIN             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL6;

typedef volatile union _CAM_REG_SEEE_ED_CTRL7_
{
    volatile struct
    {
        FIELD USM_ED_DIAG_AMP           : 3;
        FIELD USM_ED_AMP                : 6;
        FIELD USM_ED_LV                 : 3;
        FIELD USM_ED_FIL_HP_EN          : 1;
        FIELD EE_ED_FIL2_EN        : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_ED_CTRL7;

typedef volatile union _CAM_REG_SEEE_EDGE_CTRL_
{
    volatile struct
    {
        FIELD SE_EDGE                   : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EDGE_CTRL;

typedef volatile union _CAM_REG_SEEE_Y_CTRL_
{
    volatile struct
    {
        FIELD SE_Y                      : 1;
        FIELD SE_Y_CONST                : 8;
        FIELD rsv_9                     : 7;
        FIELD SE_YOUT_QBIT              : 3;
        FIELD rsv_19                    : 1;
        FIELD SE_COUT_QBIT              : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_Y_CTRL;

typedef volatile union _CAM_REG_SEEE_EDGE_CTRL1_
{
    volatile struct
    {
        FIELD SE_HEDGE_SEL              : 1;
        FIELD SE_EGAIN_HA               : 4;
        FIELD SE_EGAIN_HB               : 5;
        FIELD SE_EGAIN_HC               : 5;
        FIELD rsv_15                    : 1;
        FIELD SE_VEDGE_SEL              : 1;
        FIELD SE_EGAIN_VA               : 4;
        FIELD SE_EGAIN_VB               : 5;
        FIELD SE_EGAIN_VC               : 5;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EDGE_CTRL1;

typedef volatile union _CAM_REG_SEEE_EDGE_CTRL2_
{
    volatile struct
    {
        FIELD SE_THRE_RT                : 5;
        FIELD SE_EMBOSS1                : 1;
        FIELD SE_EMBOSS2                : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EDGE_CTRL2;

typedef volatile union _CAM_REG_SEEE_EDGE_CTRL3_
{
    volatile struct
    {
        FIELD SE_ONLYC                  : 1;
        FIELD SE_CORE_CON               : 7;
        FIELD SE_ETH_CON                : 8;
        FIELD SE_TOP_SLOPE              : 1;
        FIELD SE_OILEN                  : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EDGE_CTRL3;

typedef volatile union _CAM_REG_SEEE_SPECIAL_CTRL_
{
    volatile struct
    {
        FIELD SE_SPECIPONLY             : 2;
        FIELD SE_SPECIABS               : 1;
        FIELD SE_SPECIINV               : 1;
        FIELD SE_SPECIGAIN              : 2;
        FIELD SE_KNEESEL                : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SPECIAL_CTRL;

typedef volatile union _CAM_REG_SEEE_CORE_CTRL1_
{
    volatile struct
    {
        FIELD SE_COREH                  : 7;
        FIELD SE_SUP                    : 2;
        FIELD SE_ETH3                   : 8;
        FIELD SE_SDN                    : 2;
        FIELD SE_COREH2                 : 6;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_CORE_CTRL1;

typedef volatile union _CAM_REG_SEEE_CORE_CTRL2_
{
    volatile struct
    {
        FIELD SE_E_TH1_V                : 7;
        FIELD SE_SUP_V                  : 2;
        FIELD SE_E_TH3_V                : 8;
        FIELD SE_SDN_V                  : 2;
        FIELD SE_HALF_V                 : 6;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_CORE_CTRL2;

typedef volatile union _CAM_REG_SEEE_EE_LINK1_
{
    volatile struct
    {
        FIELD EE_LCE_X1_1               : 8;
        FIELD EE_LCE_S1_1               : 8;
        FIELD EE_LCE_S2_1               : 8;
        FIELD EE_LCE_LINK               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EE_LINK1;

typedef volatile union _CAM_REG_SEEE_EE_LINK2_
{
    volatile struct
    {
        FIELD EE_LCE_X1_2               : 8;
        FIELD EE_LCE_S1_2               : 8;
        FIELD EE_LCE_S2_2               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EE_LINK2;

typedef volatile union _CAM_REG_SEEE_EE_LINK3_
{
    volatile struct
    {
        FIELD EE_LCE_X1_3               : 8;
        FIELD EE_LCE_S1_3               : 8;
        FIELD EE_LCE_S2_3               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EE_LINK3;

typedef volatile union _CAM_REG_SEEE_EE_LINK4_
{
    volatile struct
    {
        FIELD EE_LCE_THO_1              : 8;
        FIELD EE_LCE_THU_1              : 8;
        FIELD EE_LCE_THO_2              : 8;
        FIELD EE_LCE_THU_2              : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EE_LINK4;

typedef volatile union _CAM_REG_SEEE_EE_LINK5_
{
    volatile struct
    {
        FIELD EE_LCE_THO_3              : 8;
        FIELD EE_LCE_THU_3              : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EE_LINK5;

/* end MT6589_403_yuv_seee.xml*/

/* start MT6589_500_cdp_cdrz.xml*/
typedef volatile union _CAM_REG_CDRZ_CONTROL_
{
    volatile struct
    {
        FIELD CDRZ_HORIZONTAL_EN        : 1;
        FIELD CDRZ_Vertical_EN          : 1;
        FIELD rsv_2                     : 2;
        FIELD CDRZ_Vertical_First       : 1;
        FIELD CDRZ_Horizontal_Algorithm : 2;
        FIELD CDRZ_Vertical_Algorithm   : 2;
        FIELD CDRZ_Dering_en            : 1;
        FIELD CDRZ_Truncation_Bit_H     : 3;
        FIELD CDRZ_Truncation_Bit_V     : 3;
        FIELD CDRZ_Horizontal_Table_Select : 5;
        FIELD CDRZ_Vertical_Table_Select : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CONTROL;

typedef volatile union _CAM_REG_CDRZ_INPUT_IMAGE_
{
    volatile struct
    {
        FIELD CDRZ_Input_Image_W        : 13;
        FIELD rsv_13                    : 3;
        FIELD CDRZ_Input_Image_H        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_INPUT_IMAGE;

typedef volatile union _CAM_REG_CDRZ_OUTPUT_IMAGE_
{
    volatile struct
    {
        FIELD CDRZ_Output_Image_W       : 13;
        FIELD rsv_13                    : 3;
        FIELD CDRZ_Output_Image_H       : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_OUTPUT_IMAGE;

typedef volatile union _CAM_REG_CDRZ_HORIZONTAL_COEFF_STEP_
{
    volatile struct
    {
        FIELD CDRZ_Horizontal_Coeff_Step : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_HORIZONTAL_COEFF_STEP;

typedef volatile union _CAM_REG_CDRZ_VERTICAL_COEFF_STEP_
{
    volatile struct
    {
        FIELD CDRZ_Vertical_Coeff_Step  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_VERTICAL_COEFF_STEP;

typedef volatile union _CAM_REG_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Luma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Luma_Horizontal_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Luma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Luma_Vertical_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Horizontal_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Vertical_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CDRZ_DERING_1_
{
    volatile struct
    {
        FIELD CDRZ_Dering_Threshold_1V  : 4;
        FIELD rsv_4                     : 12;
        FIELD CDRZ_Dering_Threshold_1H  : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_DERING_1;

typedef volatile union _CAM_REG_CDRZ_DERING_2_
{
    volatile struct
    {
        FIELD CDRZ_Dering_Threshold_2V  : 9;
        FIELD rsv_9                     : 7;
        FIELD CDRZ_Dering_Threshold_2H  : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_DERING_2;

/* end MT6589_500_cdp_cdrz.xml*/

/* start MT6589_501_cdp_curz.xml*/
typedef volatile union _CAM_REG_CURZ_CONTROL_
{
    volatile struct
    {
        FIELD CURZ_HORIZONTAL_EN        : 1;
        FIELD CURZ_Vertical_EN          : 1;
        FIELD rsv_2                     : 2;
        FIELD CURZ_RSV_1                : 5;
        FIELD CURZ_Dering_en            : 1;
        FIELD rsv_10                    : 6;
        FIELD CURZ_Horizontal_Table_Select : 5;
        FIELD CURZ_Vertical_Table_Select : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_CONTROL;

typedef volatile union _CAM_REG_CURZ_INPUT_IMAGE_
{
    volatile struct
    {
        FIELD CURZ_Input_Image_W        : 13;
        FIELD rsv_13                    : 3;
        FIELD CURZ_Input_Image_H        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_INPUT_IMAGE;

typedef volatile union _CAM_REG_CURZ_OUTPUT_IMAGE_
{
    volatile struct
    {
        FIELD CURZ_Output_Image_W       : 13;
        FIELD rsv_13                    : 3;
        FIELD CURZ_Output_Image_H       : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_OUTPUT_IMAGE;

typedef volatile union _CAM_REG_CURZ_HORIZONTAL_COEFF_STEP_
{
    volatile struct
    {
        FIELD CURZ_Horizontal_Coeff_Step : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_HORIZONTAL_COEFF_STEP;

typedef volatile union _CAM_REG_CURZ_VERTICAL_COEFF_STEP_
{
    volatile struct
    {
        FIELD CURZ_Vertical_Coeff_Step  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_VERTICAL_COEFF_STEP;

typedef volatile union _CAM_REG_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Luma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Luma_Horizontal_Subpixel_Offset : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CURZ_LUMA_VERTICAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Luma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_LUMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Luma_Vertical_Subpixel_Offset : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Chroma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Chroma_Horizontal_Subpixel_Offset : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Chroma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD CURZ_Chroma_Vertical_Subpixel_Offset : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_CURZ_DERING_1_
{
    volatile struct
    {
        FIELD CURZ_Dering_Threshold_1V  : 4;
        FIELD rsv_4                     : 12;
        FIELD CURZ_Dering_Threshold_1H  : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_DERING_1;

typedef volatile union _CAM_REG_CURZ_DERING_2_
{
    volatile struct
    {
        FIELD CURZ_Dering_Threshold_2V  : 9;
        FIELD rsv_9                     : 7;
        FIELD CURZ_Dering_Threshold_2H  : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CURZ_DERING_2;

/* end MT6589_501_cdp_curz.xml*/

/* start MT6589_502_cdp_prz.xml*/
typedef volatile union _CAM_REG_PRZ_CONTROL_
{
    volatile struct
    {
        FIELD PRZ_HORIZONTAL_EN         : 1;
        FIELD PRZ_Vertical_EN           : 1;
        FIELD rsv_2                     : 2;
        FIELD PRZ_Vertical_First        : 1;
        FIELD PRZ_Horizontal_Algorithm  : 2;
        FIELD PRZ_Vertical_Algorithm    : 2;
        FIELD PRZ_Dering_en             : 1;
        FIELD PRZ_Truncation_Bit_H      : 3;
        FIELD PRZ_Truncation_Bit_V      : 3;
        FIELD PRZ_Horizontal_Table_Select : 5;
        FIELD PRZ_Vertical_Table_Select : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_CONTROL;

typedef volatile union _CAM_REG_PRZ_INPUT_IMAGE_
{
    volatile struct
    {
        FIELD PRZ_Input_Image_W         : 13;
        FIELD rsv_13                    : 3;
        FIELD PRZ_Input_Image_H         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_INPUT_IMAGE;

typedef volatile union _CAM_REG_PRZ_OUTPUT_IMAGE_
{
    volatile struct
    {
        FIELD PRZ_Output_Image_W        : 13;
        FIELD rsv_13                    : 3;
        FIELD PRZ_Output_Image_H        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_OUTPUT_IMAGE;

typedef volatile union _CAM_REG_PRZ_HORIZONTAL_COEFF_STEP_
{
    volatile struct
    {
        FIELD PRZ_Horizontal_Coeff_Step : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_HORIZONTAL_COEFF_STEP;

typedef volatile union _CAM_REG_PRZ_VERTICAL_COEFF_STEP_
{
    volatile struct
    {
        FIELD PRZ_Vertical_Coeff_Step   : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_VERTICAL_COEFF_STEP;

typedef volatile union _CAM_REG_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Luma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Luma_Horizontal_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_PRZ_LUMA_VERTICAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Luma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_LUMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Luma_Vertical_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Chroma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Chroma_Horizontal_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Chroma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union _CAM_REG_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET_
{
    volatile struct
    {
        FIELD PRZ_Chroma_Vertical_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union _CAM_REG_PRZ_DERING_1_
{
    volatile struct
    {
        FIELD PRZ_Dering_Threshold_1V   : 4;
        FIELD rsv_4                     : 12;
        FIELD PRZ_Dering_Threshold_1H   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_DERING_1;

typedef volatile union _CAM_REG_PRZ_DERING_2_
{
    volatile struct
    {
        FIELD PRZ_Dering_Threshold_2V   : 9;
        FIELD rsv_9                     : 7;
        FIELD PRZ_Dering_Threshold_2H   : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_PRZ_DERING_2;

/* end MT6589_502_cdp_prz.xml*/

/* start MT6589_504_cdp_fe.xml*/
typedef volatile union _CAM_REG_FE_CTRL_
{
    volatile struct
    {
        FIELD FEM_HARRIS_TPIPE_MODE      : 2;
        FIELD FEM_PARAM          : 3;
        FIELD FEM_LPF_MODE              : 1;
        FIELD FEM_TH_G           : 8;
        FIELD FEM_TH_C    : 8;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_CTRL;

typedef volatile union _CAM_REG_FE_IDX_CTRL_
{
    volatile struct
    {
        FIELD FEM_XIDX                  : 13;
        FIELD rsv_13                    : 3;
        FIELD FEM_YIDX                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_IDX_CTRL;

typedef volatile union _CAM_REG_FE_CROP_CTRL1_
{
    volatile struct
    {
        FIELD FEM_START_X               : 11;
        FIELD rsv_11                    : 5;
        FIELD FEM_START_Y               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_CROP_CTRL1;

typedef volatile union _CAM_REG_FE_CROP_CTRL2_
{
    volatile struct
    {
        FIELD FEM_IN_WIDTH              : 11;
        FIELD rsv_11                    : 5;
        FIELD FEM_IN_HEIGHT             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_CROP_CTRL2;

/* end MT6589_504_cdp_fe.xml*/

/* start MT6589_508_cdp_vido.xml*/
typedef volatile union _CAM_REG_VIDO_CTRL_
{
    volatile struct
    {
        FIELD VIDO_FORMAT_1             : 3;
        FIELD rsv_3                     : 1;
        FIELD VIDO_FORMAT_2             : 1;
        FIELD rsv_5                     : 1;
        FIELD VIDO_FORMAT_3             : 2;
        FIELD VIDO_FORMAT_SEQ           : 2;
        FIELD rsv_10                    : 2;
        FIELD VIDO_CROP_EN              : 1;
        FIELD VIDO_PRI_EN               : 1;
        FIELD VIDO_PREULTRA_EN          : 1;
        FIELD rsv_15                    : 1;
        FIELD VIDO_PADDING_MODE         : 1;
        FIELD rsv_17                    : 3;
        FIELD VIDO_ROTATION             : 2;
        FIELD rsv_22                    : 2;
        FIELD VIDO_FLIP                 : 1;
        FIELD rsv_25                    : 1;
        FIELD VIDO_SOF_RESET_EN         : 1;
        FIELD rsv_27                    : 1;
        FIELD VIDO_UV_XSEL              : 2;
        FIELD VIDO_UV_YSEL              : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_CTRL;

typedef volatile union _CAM_REG_VIDO_DMA_PERF_
{
    volatile struct
    {
        FIELD VIDO_FIFO_PRI_LOW_THR     : 12;
        FIELD VIDO_FIFO_PRI_THR         : 12;
        FIELD VIDO_MAX_BURST_LEN        : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_DMA_PERF;

typedef volatile union _CAM_REG_VIDO_MAIN_BUF_SIZE_
{
    volatile struct
    {
        FIELD VIDO_FIFO_MODE            : 1;
        FIELD rsv_1                     : 3;
        FIELD VIDO_DOUBLE_BUF_MODE      : 1;
        FIELD rsv_5                     : 3;
        FIELD VIDO_MAIN_BUF_LINE_NUM    : 7;
        FIELD rsv_15                    : 1;
        FIELD VIDO_MAIN_BLK_WIDTH       : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_MAIN_BUF_SIZE;

typedef volatile union _CAM_REG_VIDO_BUF_SIZE_
{
    volatile struct
    {
        FIELD VIDO_MAIN_BUF_LINE_SIZE   : 13;
        FIELD rsv_13                    : 3;
        FIELD VIDO_SUB_BUF_LINE_SIZE    : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BUF_SIZE;

typedef volatile union _CAM_REG_VIDO_SOFT_RST_
{
    volatile struct
    {
        FIELD VIDO_SOFT_RST             : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_SOFT_RST;

typedef volatile union _CAM_REG_VIDO_SOFT_RST_STAT_
{
    volatile struct
    {
        FIELD VIDO_SOFT_RST_STAT        : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_SOFT_RST_STAT;

typedef volatile union _CAM_REG_VIDO_INT_EN_
{
    volatile struct
    {
        FIELD VIDO_INT_1_EN             : 1;
        FIELD rsv_1                     : 30;
        FIELD VIDO_INT_WC_EN            : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_INT_EN;

typedef volatile union _CAM_REG_VIDO_INT_
{
    volatile struct
    {
        FIELD VIDO_INT_1                : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_INT;

typedef volatile union _CAM_REG_VIDO_CROP_OFST_
{
    volatile struct
    {
        FIELD VIDO_CROP_OFST_X          : 13;
        FIELD rsv_13                    : 3;
        FIELD VIDO_CROP_OFST_Y          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_CROP_OFST;

typedef volatile union _CAM_REG_VIDO_TAR_SIZE_
{
    volatile struct
    {
        FIELD VIDO_XSIZE                : 13;
        FIELD rsv_13                    : 3;
        FIELD VIDO_YSIZE                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_TAR_SIZE;

typedef volatile union _CAM_REG_VIDO_BASE_ADDR_
{
    volatile struct
    {
        FIELD VIDO_BASE_ADDR            : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BASE_ADDR;

typedef volatile union _CAM_REG_VIDO_OFST_ADDR_
{
    volatile struct
    {
        FIELD VIDO_OFST_ADDR            : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_OFST_ADDR;

typedef volatile union _CAM_REG_VIDO_STRIDE_
{
    volatile struct
    {
        FIELD VIDO_STRIDE               : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_STRIDE;

typedef volatile union _CAM_REG_VIDO_BASE_ADDR_C_
{
    volatile struct
    {
        FIELD VIDO_BASE_ADDR_C          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BASE_ADDR_C;

typedef volatile union _CAM_REG_VIDO_OFST_ADDR_C_
{
    volatile struct
    {
        FIELD VIDO_OFST_ADDR_C          : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_OFST_ADDR_C;

typedef volatile union _CAM_REG_VIDO_STRIDE_C_
{
    volatile struct
    {
        FIELD VIDO_STRIDE_C             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_STRIDE_C;

typedef volatile union _CAM_REG_VIDO_BUF_BASE_ADDR0_
{
    volatile struct
    {
        FIELD VIDO_BUF_BASE_ADDR0       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BUF_BASE_ADDR0;

typedef volatile union _CAM_REG_VIDO_BUF_BASE_ADDR1_
{
    volatile struct
    {
        FIELD VIDO_BUF_BASE_ADDR1       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BUF_BASE_ADDR1;

typedef volatile union _CAM_REG_VIDO_BUF_BASE_ADDR2_
{
    volatile struct
    {
        FIELD VIDO_BUF_BASE_ADDR2       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BUF_BASE_ADDR2;

typedef volatile union _CAM_REG_VIDO_BUF_BASE_ADDR3_
{
    volatile struct
    {
        FIELD VIDO_BUF_BASE_ADDR3       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BUF_BASE_ADDR3;

typedef volatile union _CAM_REG_VIDO_PADDING_
{
    volatile struct
    {
        FIELD VIDO_PADDING_Y            : 8;
        FIELD VIDO_PADDING_U            : 8;
        FIELD VIDO_PADDING_V            : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_PADDING;

typedef volatile union _CAM_REG_VIDO_DITHER_
{
    volatile struct
    {
        FIELD VIDO_DIT_EN               : 1;
        FIELD VIDO_DIT_MODE             : 2;
        FIELD rsv_3                     : 13;
        FIELD VIDO_DIT_INIT_X           : 4;
        FIELD VIDO_DIT_INIT_Y           : 4;
        FIELD VIDO_XRGB_DUMMY           : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_DITHER;

typedef volatile union _CAM_REG_VIDO_DIT_SEED_R_
{
    volatile struct
    {
        FIELD VIDO_DIT_SEED_R           : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_DIT_SEED_R;

typedef volatile union _CAM_REG_VIDO_DIT_SEED_G_
{
    volatile struct
    {
        FIELD VIDO_DIT_SEED_G           : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_DIT_SEED_G;

typedef volatile union _CAM_REG_VIDO_DIT_SEED_B_
{
    volatile struct
    {
        FIELD VIDO_DIT_SEED_B           : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_DIT_SEED_B;

typedef volatile union _CAM_REG_VIDO_BASE_ADDR_V_
{
    volatile struct
    {
        FIELD VIDO_BASE_ADDR_V          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_BASE_ADDR_V;

typedef volatile union _CAM_REG_VIDO_OFST_ADDR_V_
{
    volatile struct
    {
        FIELD VIDO_OFST_ADDR_V          : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_OFST_ADDR_V;

typedef volatile union _CAM_REG_VIDO_STRIDE_V_
{
    volatile struct
    {
        FIELD VIDO_STRIDE_V             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_STRIDE_V;

typedef volatile union _CAM_REG_VIDO_RSV_1_
{
    volatile struct
    {
        FIELD VIDO_RSV_1                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_RSV_1;

typedef volatile union _CAM_REG_VIDO_DMA_PREULTRA_
{
    volatile struct
    {
        FIELD VIDO_FIFO_PREULTRA_LOW_THR : 12;
        FIELD VIDO_FIFO_PREULTRA_THR    : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIDO_DMA_PREULTRA;

/* end MT6589_508_cdp_vido.xml*/

/* start MT6589_509_cdp_dispo.xml*/
typedef volatile union _CAM_REG_DISPO_CTRL_
{
    volatile struct
    {
        FIELD DISPO_FORMAT_1            : 3;
        FIELD rsv_3                     : 1;
        FIELD DISPO_FORMAT_2            : 1;
        FIELD rsv_5                     : 1;
        FIELD DISPO_FORMAT_3            : 2;
        FIELD DISPO_FORMAT_SEQ          : 2;
        FIELD rsv_10                    : 2;
        FIELD DISPO_CROP_EN             : 1;
        FIELD DISPO_PRI_EN              : 1;
        FIELD DISPO_PREULTRA_EN         : 1;
        FIELD rsv_15                    : 1;
        FIELD DISPO_PADDING_MODE        : 1;
        FIELD rsv_17                    : 3;
        FIELD DISPO_ROTATION            : 2;
        FIELD rsv_22                    : 2;
        FIELD DISPO_FLIP                : 1;
        FIELD rsv_25                    : 1;
        FIELD DISPO_SOF_RESET_EN        : 1;
        FIELD rsv_27                    : 1;
        FIELD DISPO_UV_XSEL             : 2;
        FIELD DISPO_UV_YSEL             : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_CTRL;

typedef volatile union _CAM_REG_DISPO_DMA_PERF_
{
    volatile struct
    {
        FIELD DISPO_FIFO_PRI_LOW_THR    : 12;
        FIELD DISPO_FIFO_PRI_THR        : 12;
        FIELD DISPO_MAX_BURST_LEN       : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_DMA_PERF;

typedef volatile union _CAM_REG_DISPO_MAIN_BUF_SIZE_
{
    volatile struct
    {
        FIELD DISPO_FIFO_MODE           : 1;
        FIELD rsv_1                     : 3;
        FIELD DISPO_DOUBLE_BUF_MODE     : 1;
        FIELD rsv_5                     : 3;
        FIELD DISPO_MAIN_BUF_LINE_NUM   : 7;
        FIELD rsv_15                    : 1;
        FIELD DISPO_MAIN_BLK_WIDTH      : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_MAIN_BUF_SIZE;

typedef volatile union _CAM_REG_DISPO_BUF_SIZE_
{
    volatile struct
    {
        FIELD DISPO_MAIN_BUF_LINE_SIZE  : 13;
        FIELD rsv_13                    : 3;
        FIELD DISPO_SUB_BUF_LINE_SIZE   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BUF_SIZE;

typedef volatile union _CAM_REG_DISPO_SOFT_RST_
{
    volatile struct
    {
        FIELD DISPO_SOFT_RST            : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_SOFT_RST;

typedef volatile union _CAM_REG_DISPO_SOFT_RST_STAT_
{
    volatile struct
    {
        FIELD DISPO_SOFT_RST_STAT       : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_SOFT_RST_STAT;

typedef volatile union _CAM_REG_DISPO_INT_EN_
{
    volatile struct
    {
        FIELD DISPO_INT_1_EN            : 1;
        FIELD rsv_1                     : 30;
        FIELD DISPO_INT_WC_EN           : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_INT_EN;

typedef volatile union _CAM_REG_DISPO_INT_
{
    volatile struct
    {
        FIELD DISPO_INT_1               : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_INT;

typedef volatile union _CAM_REG_DISPO_CROP_OFST_
{
    volatile struct
    {
        FIELD DISPO_CROP_OFST_X         : 13;
        FIELD rsv_13                    : 3;
        FIELD DISPO_CROP_OFST_Y         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_CROP_OFST;

typedef volatile union _CAM_REG_DISPO_TAR_SIZE_
{
    volatile struct
    {
        FIELD DISPO_XSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD DISPO_YSIZE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_TAR_SIZE;

typedef volatile union _CAM_REG_DISPO_BASE_ADDR_
{
    volatile struct
    {
        FIELD DISPO_BASE_ADDR           : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BASE_ADDR;

typedef volatile union _CAM_REG_DISPO_OFST_ADDR_
{
    volatile struct
    {
        FIELD DISPO_OFST_ADDR           : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_OFST_ADDR;

typedef volatile union _CAM_REG_DISPO_STRIDE_
{
    volatile struct
    {
        FIELD DISPO_STRIDE              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_STRIDE;

typedef volatile union _CAM_REG_DISPO_BASE_ADDR_C_
{
    volatile struct
    {
        FIELD DISPO_BASE_ADDR_C         : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BASE_ADDR_C;

typedef volatile union _CAM_REG_DISPO_OFST_ADDR_C_
{
    volatile struct
    {
        FIELD DISPO_OFST_ADDR_C         : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_OFST_ADDR_C;

typedef volatile union _CAM_REG_DISPO_STRIDE_C_
{
    volatile struct
    {
        FIELD DISPO_STRIDE_C            : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_STRIDE_C;

typedef volatile union _CAM_REG_DISPO_BUF_BASE_ADDR0_
{
    volatile struct
    {
        FIELD DISPO_BUF_BASE_ADDR0      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BUF_BASE_ADDR0;

typedef volatile union _CAM_REG_DISPO_BUF_BASE_ADDR1_
{
    volatile struct
    {
        FIELD DISPO_BUF_BASE_ADDR1      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BUF_BASE_ADDR1;

typedef volatile union _CAM_REG_DISPO_BUF_BASE_ADDR2_
{
    volatile struct
    {
        FIELD DISPO_BUF_BASE_ADDR2      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BUF_BASE_ADDR2;

typedef volatile union _CAM_REG_DISPO_BUF_BASE_ADDR3_
{
    volatile struct
    {
        FIELD DISPO_BUF_BASE_ADDR3      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BUF_BASE_ADDR3;

typedef volatile union _CAM_REG_DISPO_PADDING_
{
    volatile struct
    {
        FIELD DISPO_PADDING_Y           : 8;
        FIELD DISPO_PADDING_U           : 8;
        FIELD DISPO_PADDING_V           : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_PADDING;

typedef volatile union _CAM_REG_DISPO_DITHER_
{
    volatile struct
    {
        FIELD DISPO_DIT_EN              : 1;
        FIELD DISPO_DIT_MODE            : 2;
        FIELD rsv_3                     : 13;
        FIELD DISPO_DIT_INIT_X          : 4;
        FIELD DISPO_DIT_INIT_Y          : 4;
        FIELD DISPO_XRGB_DUMMY          : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_DITHER;

typedef volatile union _CAM_REG_DISPO_DIT_SEED_R_
{
    volatile struct
    {
        FIELD DISPO_DIT_SEED_R          : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_DIT_SEED_R;

typedef volatile union _CAM_REG_DISPO_DIT_SEED_G_
{
    volatile struct
    {
        FIELD DISPO_DIT_SEED_G          : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_DIT_SEED_G;

typedef volatile union _CAM_REG_DISPO_DIT_SEED_B_
{
    volatile struct
    {
        FIELD DISPO_DIT_SEED_B          : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_DIT_SEED_B;

typedef volatile union _CAM_REG_DISPO_BASE_ADDR_V_
{
    volatile struct
    {
        FIELD DISPO_BASE_ADDR_V         : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_BASE_ADDR_V;

typedef volatile union _CAM_REG_DISPO_OFST_ADDR_V_
{
    volatile struct
    {
        FIELD DISPO_OFST_ADDR_V         : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_OFST_ADDR_V;

typedef volatile union _CAM_REG_DISPO_STRIDE_V_
{
    volatile struct
    {
        FIELD DISPO_STRIDE_V            : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_STRIDE_V;

typedef volatile union _CAM_REG_DISPO_RSV_1_
{
    volatile struct
    {
        FIELD DISPO_RSV_1               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_RSV_1;

typedef volatile union _CAM_REG_DISPO_DMA_PREULTRA_
{
    volatile struct
    {
        FIELD DISPO_FIFO_PREULTRA_LOW_THR : 12;
        FIELD DISPO_FIFO_PREULTRA_THR   : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DISPO_DMA_PREULTRA;

/* end MT6589_509_cdp_dispo.xml*/

/* start MT6589_510_cdp_eis.xml*/
typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL1_
{
    volatile struct
    {
        FIELD PREP_DS_IIR_H             : 3;
        FIELD PREP_DS_IIR_V             : 3;
        FIELD rsv_6                     : 2;
        FIELD ME_NUM_HRP                : 5;
        FIELD ME_AD_CLIP                : 4;
        FIELD ME_AD_KNEE                : 4;
        FIELD ME_NUM_VRP                : 4;
        FIELD ME_NUM_HWIN               : 3;
        FIELD ME_NUM_VWIN               : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_PREP_ME_CTRL1;

typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL2_
{
    volatile struct
    {
        FIELD PREP_GAIN_H               : 2;
        FIELD PREP_GAIN_IIR_H           : 3;
        FIELD PREP_GAIN_IIR_V           : 3;
        FIELD PREP_FIR_H                : 6;
        FIELD EIS_WRP_EN                : 1;
        FIELD EIS_FIRST_FRM             : 1;
        FIELD EIS_SPARE                 : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_PREP_ME_CTRL2;

typedef volatile union _CAM_REG_EIS_LMV_TH_
{
    volatile struct
    {
        FIELD LMV_TH_Y_SURROUND         : 8;
        FIELD LMV_TH_Y_CENTER           : 8;
        FIELD LMV_TH_X_SOURROUND        : 8;
        FIELD LMV_TH_X_CENTER           : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_LMV_TH;

typedef volatile union _CAM_REG_EIS_FL_OFFSET_
{
    volatile struct
    {
        FIELD FL_OFFSET_V               : 12;
        FIELD rsv_12                    : 4;
        FIELD FL_OFFSET_H               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_FL_OFFSET;

typedef volatile union _CAM_REG_EIS_MB_OFFSET_
{
    volatile struct
    {
        FIELD MB_OFFSET_V               : 12;
        FIELD rsv_12                    : 4;
        FIELD MB_OFFSET_H               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_MB_OFFSET;

typedef volatile union _CAM_REG_EIS_MB_INTERVAL_
{
    volatile struct
    {
        FIELD MB_INTERVAL_V             : 12;
        FIELD rsv_12                    : 4;
        FIELD MB_INTERVAL_H             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_MB_INTERVAL;

typedef volatile union _CAM_REG_EIS_GMV_
{
    volatile struct
    {
        FIELD GMV_Y                     : 16;
        FIELD GMV_X                     : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_GMV;

typedef volatile union _CAM_REG_EIS_ERR_CTRL_
{
    volatile struct
    {
        FIELD ERR_STATUS                : 4;
        FIELD CHK_SUM_EN                : 1;
        FIELD rsv_5                     : 3;
        FIELD CHK_SUM_OUT               : 8;
        FIELD ERR_MASK                  : 4;
        FIELD rsv_20                    : 11;
        FIELD CLEAR_ERR                 : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_ERR_CTRL;

typedef volatile union _CAM_REG_EIS_IMAGE_CTRL_
{
    volatile struct
    {
        FIELD HEIGHT                    : 13;
        FIELD rsv_13                    : 3;
        FIELD WIDTH                     : 13;
        FIELD rsv_29                    : 2;
        FIELD PIPE_MODE                 : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_IMAGE_CTRL;

/* end MT6589_510_cdp_eis.xml*/

/* start MT6589_601_rgb_dgm.xml*/
typedef volatile union _CAM_REG_DGM_B2_
{
    volatile struct
    {
        FIELD DGM_B1                    : 8;
        FIELD rsv_8                     : 8;
        FIELD DGM_B2                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DGM_B2;

typedef volatile union _CAM_REG_DGM_B4_
{
    volatile struct
    {
        FIELD DGM_B3                    : 8;
        FIELD rsv_8                     : 8;
        FIELD DGM_B4                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DGM_B4;

typedef volatile union _CAM_REG_DGM_B6_
{
    volatile struct
    {
        FIELD DGM_B5                    : 8;
        FIELD rsv_8                     : 8;
        FIELD DGM_B6                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DGM_B6;

typedef volatile union _CAM_REG_DGM_B8_
{
    volatile struct
    {
        FIELD DGM_B7                    : 8;
        FIELD rsv_8                     : 8;
        FIELD DGM_B8                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DGM_B8;

typedef volatile union _CAM_REG_DGM_B10_
{
    volatile struct
    {
        FIELD DGM_B9                    : 8;
        FIELD rsv_8                     : 8;
        FIELD DGM_B10                   : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DGM_B10;

typedef volatile union _CAM_REG_DGM_B11_
{
    volatile struct
    {
        FIELD DGM_B11                   : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_DGM_B11;

/* end MT6589_601_rgb_dgm.xml*/

/* start MT6589_602_gdma_ctl.xml*/
typedef volatile union _CAM_REG_GDMA_IOTYPE_
{
    volatile struct
    {
        FIELD JPEGMD                    : 1;
        FIELD rsv_1                     : 2;
        FIELD YUVMD                     : 1;
        FIELD rsv_4                     : 1;
        FIELD YONLY                     : 1;
        FIELD rsv_6                     : 2;
        FIELD CBCR_C                    : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_IOTYPE;

typedef volatile union _CAM_REG_GDMA_MCUMODE_
{
    volatile struct
    {
        FIELD FST_MCUVFAC_C             : 5;
        FIELD FST_MCUVFAC_Y             : 6;
        FIELD rsv_11                    : 1;
        FIELD LST_MCUVFAC_C             : 5;
        FIELD LST_MCUVFAC_Y             : 6;
        FIELD rsv_23                    : 1;
        FIELD MCUVFAC_C                 : 3;
        FIELD rsv_27                    : 1;
        FIELD MCUVFAC_Y                 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_MCUMODE;

typedef volatile union _CAM_REG_GDMA_SRCOFSTY_
{
    volatile struct
    {
        FIELD SRCOFST                   : 16;
        FIELD FSTOFST                   : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_SRCOFSTY;

typedef volatile union _CAM_REG_GDMA_SRCOFSTC_
{
    volatile struct
    {
        FIELD SRCOFST                   : 16;
        FIELD FSTOFST                   : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_SRCOFSTC;

typedef volatile union _CAM_REG_GDMA_YSRCB1_
{
    volatile struct
    {
        FIELD SRCYBASE1                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_YSRCB1;

typedef volatile union _CAM_REG_GDMA_YSRCB2_
{
    volatile struct
    {
        FIELD SRCYBASE2                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_YSRCB2;

typedef volatile union _CAM_REG_GDMA_CBSRCB1_
{
    volatile struct
    {
        FIELD SRCCBBASE1                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_CBSRCB1;

typedef volatile union _CAM_REG_GDMA_CBSRCB2_
{
    volatile struct
    {
        FIELD SRCCBBASE2                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_CBSRCB2;

typedef volatile union _CAM_REG_GDMA_CRSRCB1_
{
    volatile struct
    {
        FIELD SRCCRBASE1                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_CRSRCB1;

typedef volatile union _CAM_REG_GDMA_CRSRCB2_
{
    volatile struct
    {
        FIELD SRCCRBASE2                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_CRSRCB2;

typedef volatile union _CAM_REG_GDMA_YSRCSZ_
{
    volatile struct
    {
        FIELD SRC_HEIGHTY               : 16;
        FIELD SRC_WIDTHY                : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_YSRCSZ;

typedef volatile union _CAM_REG_GDMA_SPARE_
{
    volatile struct
    {
        FIELD GDMA_SPARE                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_GDMA_SPARE;

/* end MT6589_602_gdma_ctl.xml*/

/* start MT6589_603_gdma_fmt.xml*/
typedef volatile union _CAM_REG_FMT_IOTYPE_
{
    volatile struct
    {
        FIELD rsv_0                     : 2;
        FIELD TOPFLD                    : 1;
        FIELD rsv_3                     : 1;
        FIELD INTER                     : 1;
        FIELD rsv_5                     : 3;
        FIELD FIX_256K_WEN              : 1;
        FIELD FIX_256K_REN              : 1;
        FIELD FIX_BL_EN                 : 1;
        FIELD FIX_MCU_EN                : 1;
        FIELD rsv_12                    : 5;
        FIELD OUTMD                     : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_IOTYPE;

typedef volatile union _CAM_REG_FMT_MCUMODE_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD MCUVFAC_C                 : 2;
        FIELD rsv_6                     : 2;
        FIELD MCUVFAC_Y                 : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_MCUMODE;

typedef volatile union _CAM_REG_FMT_WEBPMODE_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD WEBPVFAC_CB               : 2;
        FIELD rsv_6                     : 2;
        FIELD WEBPVFAC_Y                : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_WEBPMODE;

typedef volatile union _CAM_REG_FMT_ULTRAMODE_
{
    volatile struct
    {
        FIELD RD_ULTRA_EN               : 1;
        FIELD WR_ULTRA_EN               : 1;
        FIELD rsv_2                     : 2;
        FIELD RD_REQ_CFG                : 4;
        FIELD WR_REQ_CFG                : 4;
        FIELD RD_WAIT_CFG               : 10;
        FIELD WR_WAIT_CFG               : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_ULTRAMODE;

typedef volatile union _CAM_REG_FMT_MIFMODE_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD BST_LIM                   : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_MIFMODE;

typedef volatile union _CAM_REG_FMT_SRCBUFL_
{
    volatile struct
    {
        FIELD YSRCBUFL                  : 12;
        FIELD CSRCBUFL                  : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_SRCBUFL;

typedef volatile union _CAM_REG_FMT_WINTFON_
{
    volatile struct
    {
        FIELD WINTFON                   : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_WINTFON;

typedef volatile union _CAM_REG_FMT_TGBUFL_
{
    volatile struct
    {
        FIELD YTGBUFL                   : 12;
        FIELD CTGBUFL                   : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_TGBUFL;

typedef volatile union _CAM_REG_FMT_YSRCB1_
{
    volatile struct
    {
        FIELD SRCYBASE1                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_YSRCB1;

typedef volatile union _CAM_REG_FMT_CBSRCB1_
{
    volatile struct
    {
        FIELD SRCCBBASE1                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_CBSRCB1;

typedef volatile union _CAM_REG_FMT_PREULTRAMODE_
{
    volatile struct
    {
        FIELD RD_ULTRA_EN               : 1;
        FIELD WR_ULTRA_EN               : 1;
        FIELD rsv_2                     : 2;
        FIELD RD_REQ_CFG                : 4;
        FIELD WR_REQ_CFG                : 4;
        FIELD RD_WAIT_CFG               : 10;
        FIELD WR_WAIT_CFG               : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_PREULTRAMODE;

typedef volatile union _CAM_REG_FMT_YTGB1_
{
    volatile struct
    {
        FIELD TGYBASE1                  : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_YTGB1;

typedef volatile union _CAM_REG_FMT_CBTGB1_
{
    volatile struct
    {
        FIELD TGCBBASE1                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_CBTGB1;

typedef volatile union _CAM_REG_FMT_CRTGB1_
{
    volatile struct
    {
        FIELD TGCRBASE1                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_CRTGB1;

typedef volatile union _CAM_REG_FMT_YTGB2_
{
    volatile struct
    {
        FIELD TGYBASE2                  : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_YTGB2;

typedef volatile union _CAM_REG_FMT_CBTGB2_
{
    volatile struct
    {
        FIELD TGCBBASE2                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_CBTGB2;

typedef volatile union _CAM_REG_FMT_CRTGB2_
{
    volatile struct
    {
        FIELD TGCRBASE2                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_CRTGB2;

typedef volatile union _CAM_REG_FMT_YSRCSZ_
{
    volatile struct
    {
        FIELD SRC_HEIGHTY               : 16;
        FIELD SRC_WIDTHY                : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_YSRCSZ;

typedef volatile union _CAM_REG_FMT_RANGE_
{
    volatile struct
    {
        FIELD RANGE_MAP_EN              : 1;
        FIELD rsv_1                     : 3;
        FIELD RANGE_RED_EN              : 1;
        FIELD rsv_5                     : 3;
        FIELD RANGE_MAPCB               : 5;
        FIELD rsv_13                    : 3;
        FIELD RANGE_MAPY                : 5;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_RANGE;

typedef volatile union _CAM_REG_FMT_INTEN_
{
    volatile struct
    {
        FIELD DONE_ST                   : 1;
        FIELD rsv_1                     : 3;
        FIELD INT_EN                    : 1;
        FIELD rsv_5                     : 3;
        FIELD INT_WCLR_EN               : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_FMT_INTEN;

/* end MT6589_603_gdma_fmt.xml*/

/* start MT6589_604_cdp_g2g2.xml*/
typedef volatile union _CAM_REG_G2G2_CONV0A_
{
    volatile struct
    {
        FIELD G2G2_CNV_00               : 11;
        FIELD rsv_11                    : 5;
        FIELD G2G2_CNV_01               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_CONV0A;

typedef volatile union _CAM_REG_G2G2_CONV0B_
{
    volatile struct
    {
        FIELD G2G2_CNV_02               : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_CONV0B;

typedef volatile union _CAM_REG_G2G2_CONV1A_
{
    volatile struct
    {
        FIELD G2G2_CNV_10               : 11;
        FIELD rsv_11                    : 5;
        FIELD G2G2_CNV_11               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_CONV1A;

typedef volatile union _CAM_REG_G2G2_CONV1B_
{
    volatile struct
    {
        FIELD G2G2_CNV_12               : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_CONV1B;

typedef volatile union _CAM_REG_G2G2_CONV2A_
{
    volatile struct
    {
        FIELD G2G2_CNV_20               : 11;
        FIELD rsv_11                    : 5;
        FIELD G2G2_CNV_21               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_CONV2A;

typedef volatile union _CAM_REG_G2G2_CONV2B_
{
    volatile struct
    {
        FIELD G2G2_CNV_22               : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_CONV2B;

typedef volatile union _CAM_REG_G2G2_ACC_
{
    volatile struct
    {
        FIELD G2G2_ACC                  : 4;
        FIELD G2G2_MOFFS_R              : 1;
        FIELD G2G2_POFFS_R              : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G2_ACC;

/* end MT6589_604_cdp_g2g2.xml*/

/* start MT6589_605_cdp_3dnr.xml*/
typedef volatile union _CAM_REG_NR3D_BLEND_
{
    volatile struct
    {
        FIELD NR3D_GAIN        : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_ROUND_Y     : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_ROUND_U     : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_ROUND_V     : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_BLEND;

typedef volatile union _CAM_REG_NR3D_SKIP_KEY_
{
    volatile struct
    {
        FIELD NR3D_SKIP_KEY_Y           : 8;
        FIELD NR3D_SKIP_KEY_U           : 8;
        FIELD NR3D_SKIP_KEY_V           : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_SKIP_KEY;

typedef volatile union _CAM_REG_NR3D_FBCNT_OFF_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD NR3D_FBCNT_XOFF           : 13;
        FIELD rsv_14                    : 2;
        FIELD NR3D_FBCNT_YOFF           : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_FBCNT_OFF;

typedef volatile union _CAM_REG_NR3D_FBCNT_SIZ_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD NR3D_FBCNT_XSIZ           : 13;
        FIELD rsv_14                    : 2;
        FIELD NR3D_FBCNT_YSIZ           : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_FBCNT_SIZ;

typedef volatile union _CAM_REG_NR3D_FB_COUNT_
{
    volatile struct
    {
        FIELD NR3D_FB_COUNT             : 28;
        FIELD NR3D_LIMIT_OUTSIDE_COUNT_TH : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_FB_COUNT;

typedef volatile union _CAM_REG_NR3D_LIMIT_CPX_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_CPX1           : 8;
        FIELD NR3D_LIMIT_CPX2           : 8;
        FIELD NR3D_LIMIT_CPX3           : 8;
        FIELD NR3D_LIMIT_CPX4           : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_CPX;

typedef volatile union _CAM_REG_NR3D_LIMIT_Y_CON1_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_Y0             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_Y0_TH          : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LIMIT_Y1             : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LIMIT_Y1_TH          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_Y_CON1;

typedef volatile union _CAM_REG_NR3D_LIMIT_Y_CON2_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_Y2             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_Y2_TH          : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LIMIT_Y3             : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LIMIT_Y3_TH          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_Y_CON2;

typedef volatile union _CAM_REG_NR3D_LIMIT_Y_CON3_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_Y4             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_Y4_TH          : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_Y_CON3;

typedef volatile union _CAM_REG_NR3D_LIMIT_U_CON1_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_U0             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_U0_TH          : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LIMIT_U1             : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LIMIT_U1_TH          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_U_CON1;

typedef volatile union _CAM_REG_NR3D_LIMIT_U_CON2_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_U2             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_U2_TH          : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LIMIT_U3             : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LIMIT_U3_TH          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_U_CON2;

typedef volatile union _CAM_REG_NR3D_LIMIT_U_CON3_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_U4             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_U4_TH          : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_U_CON3;

typedef volatile union _CAM_REG_NR3D_LIMIT_V_CON1_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_V0             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_V0_TH          : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LIMIT_V1             : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LIMIT_V1_TH          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_V_CON1;

typedef volatile union _CAM_REG_NR3D_LIMIT_V_CON2_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_V2             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_V2_TH          : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LIMIT_V3             : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LIMIT_V3_TH          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_V_CON2;

typedef volatile union _CAM_REG_NR3D_LIMIT_V_CON3_
{
    volatile struct
    {
        FIELD NR3D_LIMIT_V4             : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LIMIT_V4_TH          : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LIMIT_V_CON3;

/* end MT6589_605_cdp_3dnr.xml*/

/* start MT6589_700_rgb_ggm.xml*/
typedef volatile union _CAM_REG_GGM_RB_GMT_
{
    volatile struct
    {
        FIELD R_GAMMA                   : 16;
        FIELD B_GAMMA                   : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GGM_RB_GMT;

typedef volatile union _CAM_REG_GGM_G_GMT_
{
    volatile struct
    {
        FIELD G_GAMMA                   : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GGM_G_GMT;

typedef volatile union _CAM_REG_GGM_CTRL_
{
    volatile struct
    {
        FIELD GAMMA_EN                  : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_GGM_CTRL;

/* end MT6589_700_rgb_ggm.xml*/

/* start MT6589_701_yuv_pca.xml*/
typedef volatile union _CAM_REG_PCA_TBL_
{
    volatile struct
    {
        FIELD PCA_LUMA_GAIN             : 8;
        FIELD PCA_SAT_GAIN              : 8;
        FIELD PCA_HUE_SHIFT             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_PCA_TBL;

typedef volatile union _CAM_REG_PCA_CON1_
{
    volatile struct
    {
        FIELD PCA_LUT_360               : 1;
        FIELD rsv_1                     : 3;
        FIELD RSV                       : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_PCA_CON1;

typedef volatile union _CAM_REG_PCA_CON2_
{
    volatile struct
    {
        FIELD PCA_C_TH             : 5;
        FIELD rsv_5                     : 3;
        FIELD RSV                       : 2;
        FIELD rsv_10                    : 2;
        FIELD PCA_S_TH_EN             : 1;
        FIELD rsv_13                    : 3;
        FIELD PCA_S_TH                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_PCA_CON2;

/* end MT6589_701_yuv_pca.xml*/

/* start MT6589_cam_verif.xml*/
typedef volatile union _CAM_REG_TPIPE_RING_CON1_
{
    volatile struct
    {
        FIELD TPIPE_IRQ_MODE             : 8;
        FIELD RING_BUFFER_MCU_NO        : 8;
        FIELD RING_BUFFER_MCU_Y_SIZE    : 8;
        FIELD EIS_FE_ONE_SEL            : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_TPIPE_RING_CON1;

/* end MT6589_cam_verif.xml*/

// ----------------- CAM  Grouping Definitions -------------------
// ----------------- CAM Register Definition -------------------
typedef volatile struct _isp_reg_t_
{
    UINT32                          rsv_0000[4096];           // 0000..3FFC
    CAM_REG_CTL_START               CAM_CTL_START;            // 4000 (start MT6589_000_cam_ctl.xml)
    CAM_REG_CTL_EN1                 CAM_CTL_EN1;              // 4004
    CAM_REG_CTL_EN2                 CAM_CTL_EN2;              // 4008
    CAM_REG_CTL_DMA_EN              CAM_CTL_DMA_EN;           // 400C
    CAM_REG_CTL_FMT_SEL             CAM_CTL_FMT_SEL;          // 4010
    UINT32                          rsv_4014;                 // 4014
    CAM_REG_CTL_SEL                 CAM_CTL_SEL;              // 4018
    CAM_REG_CTL_PIX_ID              CAM_CTL_PIX_ID;           // 401C
    CAM_REG_CTL_INT_EN              CAM_CTL_INT_EN;           // 4020
    CAM_REG_CTL_INT_STATUS          CAM_CTL_INT_STATUS;       // 4024
    CAM_REG_CTL_DMA_INT             CAM_CTL_DMA_INT;          // 4028
    CAM_REG_CTL_INTB_EN             CAM_CTL_INTB_EN;          // 402C
    CAM_REG_CTL_INTB_STATUS         CAM_CTL_INTB_STATUS;      // 4030
    CAM_REG_CTL_DMAB_INT            CAM_CTL_DMAB_INT;         // 4034
    CAM_REG_CTL_INTC_EN             CAM_CTL_INTC_EN;          // 4038
    CAM_REG_CTL_INTC_STATUS         CAM_CTL_INTC_STATUS;      // 403C
    CAM_REG_CTL_DMAC_INT            CAM_CTL_DMAC_INT;         // 4040
    CAM_REG_CTL_INT_STATUSX         CAM_CTL_INT_STATUSX;      // 4044
    CAM_REG_CTL_DMA_INTX            CAM_CTL_DMA_INTX;         // 4048
    UINT32                          rsv_404C;                 // 404C
    CAM_REG_CTL_TPIPE                CAM_CTL_TPIPE;             // 4050
    CAM_REG_CTL_TCM_EN              CAM_CTL_TCM_EN;           // 4054
    CAM_REG_CTL_SRAM_CFG            CAM_CTL_SRAM_CFG;         // 4058
    CAM_REG_CTL_SW_CTL              CAM_CTL_SW_CTL;           // 405C
    CAM_REG_CTL_SPARE0              CAM_CTL_SPARE0;           // 4060
    CAM_REG_CTL_SPARE1              CAM_CTL_SPARE1;           // 4064
    CAM_REG_CTL_SPARE2              CAM_CTL_SPARE2;           // 4068
    CAM_REG_CTL_SPARE3              CAM_CTL_SPARE3;           // 406C
    UINT32                          rsv_4070;                 // 4070
    CAM_REG_CTL_MUX_SEL             CAM_CTL_MUX_SEL;          // 4074
    CAM_REG_CTL_MUX_SEL2            CAM_CTL_MUX_SEL2;         // 4078
    CAM_REG_CTL_SRAM_MUX_CFG        CAM_CTL_SRAM_MUX_CFG;     // 407C
    CAM_REG_CTL_EN1_SET             CAM_CTL_EN1_SET;          // 4080
    CAM_REG_CTL_EN1_CLR             CAM_CTL_EN1_CLR;          // 4084
    CAM_REG_CTL_EN2_SET             CAM_CTL_EN2_SET;          // 4088
    CAM_REG_CTL_EN2_CLR             CAM_CTL_EN2_CLR;          // 408C
    CAM_REG_CTL_DMA_EN_SET          CAM_CTL_DMA_EN_SET;       // 4090
    CAM_REG_CTL_DMA_EN_CLR          CAM_CTL_DMA_EN_CLR;       // 4094
    CAM_REG_CTL_FMT_SEL_SET         CAM_CTL_FMT_SEL_SET;      // 4098
    CAM_REG_CTL_FMT_SEL_CLR         CAM_CTL_FMT_SEL_CLR;      // 409C
    CAM_REG_CTL_SEL_SET             CAM_CTL_SEL_SET;          // 40A0
    CAM_REG_CTL_SEL_CLR             CAM_CTL_SEL_CLR;          // 40A4
    CAM_REG_CTL_CQ0_BASEADDR        CAM_CTL_CQ0_BASEADDR;     // 40A8
    CAM_REG_CTL_CQ1_BASEADDR        CAM_CTL_CQ1_BASEADDR;     // 40AC
    CAM_REG_CTL_CQ2_BASEADDR        CAM_CTL_CQ2_BASEADDR;     // 40B0
    CAM_REG_CTL_CQ3_BASEADDR        CAM_CTL_CQ3_BASEADDR;     // 40B4
    CAM_REG_CTL_CQ0B_BASEADDR       CAM_CTL_CQ0B_BASEADDR;    // 40B8
    CAM_REG_CTL_CQ0C_BASEADDR       CAM_CTL_CQ0C_BASEADDR;    // 40BC
    CAM_REG_CTL_MUX_SEL_SET         CAM_CTL_MUX_SEL_SET;      // 40C0
    CAM_REG_CTL_MUX_SEL_CLR         CAM_CTL_MUX_SEL_CLR;      // 40C4
    CAM_REG_CTL_MUX_SEL2_SET        CAM_CTL_MUX_SEL2_SET;     // 40C8
    CAM_REG_CTL_MUX_SEL2_CLR        CAM_CTL_MUX_SEL2_CLR;     // 40CC
    CAM_REG_CTL_SRAM_MUX_CFG_SET    CAM_CTL_SRAM_MUX_CFG_SET; // 40D0
    CAM_REG_CTL_SRAM_MUX_CFG_CLR    CAM_CTL_SRAM_MUX_CFG_CLR; // 40D4
    CAM_REG_CTL_PIX_ID_SET          CAM_CTL_PIX_ID_SET;       // 40D8
    CAM_REG_CTL_PIX_ID_CLR          CAM_CTL_PIX_ID_CLR;       // 40DC
    CAM_REG_CTL_SPARE0_SET          CAM_CTL_SPARE0_SET;       // 40E0
    CAM_REG_CTL_SPARE0_CLR          CAM_CTL_SPARE0_CLR;       // 40E4
    CAM_REG_CTL_CUR_CQ0_BASEADDR    CAM_CTL_CUR_CQ0_BASEADDR; // 40E8
    CAM_REG_CTL_CUR_CQ0B_BASEADDR   CAM_CTL_CUR_CQ0B_BASEADDR; // 40EC
    CAM_REG_CTL_CUR_CQ0C_BASEADDR   CAM_CTL_CUR_CQ0C_BASEADDR; // 40F0
    CAM_REG_CTL_IMGO_FBC            CAM_CTL_IMGO_FBC;         // 40F4
    CAM_REG_CTL_IMG2O_FBC           CAM_CTL_IMG2O_FBC;        // 40F8
    CAM_REG_CTL_FBC_INT             CAM_CTL_FBC_INT;          // 40FC
    UINT32                          rsv_4100[14];             // 4100..4134
    CAM_REG_CTL_IMG2O_SIZE          CAM_CTL_IMG2O_SIZE;       // 4138
    CAM_REG_CTL_IMGI_SIZE           CAM_CTL_IMGI_SIZE;        // 413C
    UINT32                          rsv_4140;                 // 4140
    CAM_REG_CTL_VIDO_SIZE           CAM_CTL_VIDO_SIZE;        // 4144
    CAM_REG_CTL_DISPO_SIZE          CAM_CTL_DISPO_SIZE;       // 4148
    CAM_REG_CTL_IMGO_SIZE           CAM_CTL_IMGO_SIZE;        // 414C
    CAM_REG_CTL_CLK_EN              CAM_CTL_CLK_EN;           // 4150
    UINT32                          rsv_4154[3];              // 4154..415C
    CAM_REG_CTL_DBG_SET             CAM_CTL_DBG_SET;          // 4160
    CAM_REG_CTL_DBG_PORT            CAM_CTL_DBG_PORT;         // 4164
    CAM_REG_CTL_IMGI_CHECK          CAM_CTL_IMGI_CHECK;       // 4168
    CAM_REG_CTL_IMGO_CHECK          CAM_CTL_IMGO_CHECK;       // 416C
    UINT32                          rsv_4170[4];              // 4170..417C
    CAM_REG_CTL_DATE_CODE           CAM_CTL_DATE_CODE;        // 4180
    CAM_REG_CTL_PROJ_CODE           CAM_CTL_PROJ_CODE;        // 4184
    UINT32                          rsv_4188[2];              // 4188..418C
    CAM_REG_CTL_RAW_DCM_DIS         CAM_CTL_RAW_DCM_DIS;      // 4190
    CAM_REG_CTL_RGB_DCM_DIS         CAM_CTL_RGB_DCM_DIS;      // 4194
    CAM_REG_CTL_YUV_DCM_DIS         CAM_CTL_YUV_DCM_DIS;      // 4198
    CAM_REG_CTL_CDP_DCM_DIS         CAM_CTL_CDP_DCM_DIS;      // 419C
    CAM_REG_CTL_RAW_DCM_STATUS      CAM_CTL_RAW_DCM_STATUS;   // 41A0
    CAM_REG_CTL_RGB_DCM_STATUS      CAM_CTL_RGB_DCM_STATUS;   // 41A4
    CAM_REG_CTL_YUV_DCM_STATUS      CAM_CTL_YUV_DCM_STATUS;   // 41A8
    CAM_REG_CTL_CDP_DCM_STATUS      CAM_CTL_CDP_DCM_STATUS;   // 41AC
    UINT32                          rsv_41B0[20];             // 41B0..41FC
    CAM_REG_DMA_SOFT_RSTSTAT        CAM_DMA_SOFT_RSTSTAT;     // 4200 (start MT6589_100_dma.xml)
    CAM_REG_TDRI_BASE_ADDR          CAM_TDRI_BASE_ADDR;       // 4204
    CAM_REG_TDRI_OFST_ADDR          CAM_TDRI_OFST_ADDR;       // 4208
    CAM_REG_TDRI_XSIZE              CAM_TDRI_XSIZE;           // 420C
    CAM_REG_CQ0I_BASE_ADDR          CAM_CQ0I_BASE_ADDR;       // 4210
    CAM_REG_CQ0I_XSIZE              CAM_CQ0I_XSIZE;           // 4214
    UINT32                          rsv_4218[6];              // 4218..422C
    CAM_REG_IMGI_BASE_ADDR          CAM_IMGI_BASE_ADDR;       // 4230
    CAM_REG_IMGI_OFST_ADDR          CAM_IMGI_OFST_ADDR;       // 4234
    CAM_REG_IMGI_XSIZE              CAM_IMGI_XSIZE;           // 4238
    CAM_REG_IMGI_YSIZE              CAM_IMGI_YSIZE;           // 423C
    CAM_REG_IMGI_STRIDE             CAM_IMGI_STRIDE;          // 4240
    CAM_REG_IMGI_RING_BUF           CAM_IMGI_RING_BUF;        // 4244
    CAM_REG_IMGI_CON                CAM_IMGI_CON;             // 4248
    CAM_REG_IMGI_CON2               CAM_IMGI_CON2;            // 424C
    CAM_REG_IMGCI_BASE_ADDR         CAM_IMGCI_BASE_ADDR;      // 4250
    CAM_REG_IMGCI_OFST_ADDR         CAM_IMGCI_OFST_ADDR;      // 4254
    CAM_REG_IMGCI_XSIZE             CAM_IMGCI_XSIZE;          // 4258
    CAM_REG_IMGCI_YSIZE             CAM_IMGCI_YSIZE;          // 425C
    CAM_REG_IMGCI_STRIDE            CAM_IMGCI_STRIDE;         // 4260
    CAM_REG_IMGCI_CON               CAM_IMGCI_CON;            // 4264
    CAM_REG_IMGCI_CON2              CAM_IMGCI_CON2;           // 4268
    CAM_REG_LSCI_BASE_ADDR          CAM_LSCI_BASE_ADDR;       // 426C
    CAM_REG_LSCI_OFST_ADDR          CAM_LSCI_OFST_ADDR;       // 4270
    CAM_REG_LSCI_XSIZE              CAM_LSCI_XSIZE;           // 4274
    CAM_REG_LSCI_YSIZE              CAM_LSCI_YSIZE;           // 4278
    CAM_REG_LSCI_STRIDE             CAM_LSCI_STRIDE;          // 427C
    CAM_REG_LSCI_CON                CAM_LSCI_CON;             // 4280
    CAM_REG_LSCI_CON2               CAM_LSCI_CON2;            // 4284
    CAM_REG_FLKI_BASE_ADDR          CAM_FLKI_BASE_ADDR;       // 4288
    CAM_REG_FLKI_OFST_ADDR          CAM_FLKI_OFST_ADDR;       // 428C
    CAM_REG_FLKI_XSIZE              CAM_FLKI_XSIZE;           // 4290
    CAM_REG_FLKI_YSIZE              CAM_FLKI_YSIZE;           // 4294
    CAM_REG_FLKI_STRIDE             CAM_FLKI_STRIDE;          // 4298
    CAM_REG_FLKI_CON                CAM_FLKI_CON;             // 429C
    CAM_REG_FLKI_CON2               CAM_FLKI_CON2;            // 42A0
    CAM_REG_LCEI_BASE_ADDR          CAM_LCEI_BASE_ADDR;       // 42A4
    CAM_REG_LCEI_OFST_ADDR          CAM_LCEI_OFST_ADDR;       // 42A8
    CAM_REG_LCEI_XSIZE              CAM_LCEI_XSIZE;           // 42AC
    CAM_REG_LCEI_YSIZE              CAM_LCEI_YSIZE;           // 42B0
    CAM_REG_LCEI_STRIDE             CAM_LCEI_STRIDE;          // 42B4
    CAM_REG_LCEI_CON                CAM_LCEI_CON;             // 42B8
    CAM_REG_LCEI_CON2               CAM_LCEI_CON2;            // 42BC
    CAM_REG_VIPI_BASE_ADDR          CAM_VIPI_BASE_ADDR;       // 42C0
    CAM_REG_VIPI_OFST_ADDR          CAM_VIPI_OFST_ADDR;       // 42C4
    CAM_REG_VIPI_XSIZE              CAM_VIPI_XSIZE;           // 42C8
    CAM_REG_VIPI_YSIZE              CAM_VIPI_YSIZE;           // 42CC
    CAM_REG_VIPI_STRIDE             CAM_VIPI_STRIDE;          // 42D0
    CAM_REG_VIPI_RING_BUF           CAM_VIPI_RING_BUF;        // 42D4
    CAM_REG_VIPI_CON                CAM_VIPI_CON;             // 42D8
    CAM_REG_VIPI_CON2               CAM_VIPI_CON2;            // 42DC
    CAM_REG_VIP2I_BASE_ADDR         CAM_VIP2I_BASE_ADDR;      // 42E0
    CAM_REG_VIP2I_OFST_ADDR         CAM_VIP2I_OFST_ADDR;      // 42E4
    CAM_REG_VIP2I_XSIZE             CAM_VIP2I_XSIZE;          // 42E8
    CAM_REG_VIP2I_YSIZE             CAM_VIP2I_YSIZE;          // 42EC
    CAM_REG_VIP2I_STRIDE            CAM_VIP2I_STRIDE;         // 42F0
    CAM_REG_VIP2I_RING_BUF          CAM_VIP2I_RING_BUF;       // 42F4
    CAM_REG_VIP2I_CON               CAM_VIP2I_CON;            // 42F8
    CAM_REG_VIP2I_CON2              CAM_VIP2I_CON2;           // 42FC
    CAM_REG_IMGO_BASE_ADDR          CAM_IMGO_BASE_ADDR;       // 4300
    CAM_REG_IMGO_OFST_ADDR          CAM_IMGO_OFST_ADDR;       // 4304
    CAM_REG_IMGO_XSIZE              CAM_IMGO_XSIZE;           // 4308
    CAM_REG_IMGO_YSIZE              CAM_IMGO_YSIZE;           // 430C
    CAM_REG_IMGO_STRIDE             CAM_IMGO_STRIDE;          // 4310
    CAM_REG_IMGO_CON                CAM_IMGO_CON;             // 4314
    CAM_REG_IMGO_CON2               CAM_IMGO_CON2;            // 4318
    CAM_REG_IMGO_CROP               CAM_IMGO_CROP;            // 431C
    CAM_REG_IMG2O_BASE_ADDR         CAM_IMG2O_BASE_ADDR;      // 4320
    CAM_REG_IMG2O_OFST_ADDR         CAM_IMG2O_OFST_ADDR;      // 4324
    CAM_REG_IMG2O_XSIZE             CAM_IMG2O_XSIZE;          // 4328
    CAM_REG_IMG2O_YSIZE             CAM_IMG2O_YSIZE;          // 432C
    CAM_REG_IMG2O_STRIDE            CAM_IMG2O_STRIDE;         // 4330
    CAM_REG_IMG2O_CON               CAM_IMG2O_CON;            // 4334
    CAM_REG_IMG2O_CON2              CAM_IMG2O_CON2;           // 4338
    CAM_REG_IMG2O_CROP              CAM_IMG2O_CROP;           // 433C
    CAM_REG_LCSO_BASE_ADDR          CAM_LCSO_BASE_ADDR;       // 4340
    CAM_REG_LCSO_OFST_ADDR          CAM_LCSO_OFST_ADDR;       // 4344
    CAM_REG_LCSO_XSIZE              CAM_LCSO_XSIZE;           // 4348
    CAM_REG_LCSO_YSIZE              CAM_LCSO_YSIZE;           // 434C
    CAM_REG_LCSO_STRIDE             CAM_LCSO_STRIDE;          // 4350
    CAM_REG_LCSO_CON                CAM_LCSO_CON;             // 4354
    CAM_REG_LCSO_CON2               CAM_LCSO_CON2;            // 4358
    CAM_REG_EISO_BASE_ADDR          CAM_EISO_BASE_ADDR;       // 435C
    CAM_REG_EISO_XSIZE              CAM_EISO_XSIZE;           // 4360
    CAM_REG_AFO_BASE_ADDR           CAM_AFO_BASE_ADDR;        // 4364
    CAM_REG_AFO_XSIZE               CAM_AFO_XSIZE;            // 4368
    CAM_REG_ESFKO_BASE_ADDR         CAM_ESFKO_BASE_ADDR;      // 436C
    CAM_REG_ESFKO_XSIZE             CAM_ESFKO_XSIZE;          // 4370
    CAM_REG_ESFKO_OFST_ADDR         CAM_ESFKO_OFST_ADDR;      // 4374
    CAM_REG_ESFKO_YSIZE             CAM_ESFKO_YSIZE;          // 4378
    CAM_REG_ESFKO_STRIDE            CAM_ESFKO_STRIDE;         // 437C
    CAM_REG_ESFKO_CON               CAM_ESFKO_CON;            // 4380
    CAM_REG_ESFKO_CON2              CAM_ESFKO_CON2;           // 4384
    CAM_REG_AAO_BASE_ADDR           CAM_AAO_BASE_ADDR;        // 4388
    CAM_REG_AAO_OFST_ADDR           CAM_AAO_OFST_ADDR;        // 438C
    CAM_REG_AAO_XSIZE               CAM_AAO_XSIZE;            // 4390
    CAM_REG_AAO_YSIZE               CAM_AAO_YSIZE;            // 4394
    CAM_REG_AAO_STRIDE              CAM_AAO_STRIDE;           // 4398
    CAM_REG_AAO_CON                 CAM_AAO_CON;              // 439C
    CAM_REG_AAO_CON2                CAM_AAO_CON2;             // 43A0
    CAM_REG_DMA_ERR_CTRL            CAM_DMA_ERR_CTRL;         // 43A4
    CAM_REG_IMGI_ERR_STAT           CAM_IMGI_ERR_STAT;        // 43A8
    CAM_REG_IMGCI_ERR_STAT          CAM_IMGCI_ERR_STAT;       // 43AC
    CAM_REG_LSCI_ERR_STAT           CAM_LSCI_ERR_STAT;        // 43B0
    CAM_REG_FLKI_ERR_STAT           CAM_FLKI_ERR_STAT;        // 43B4
    CAM_REG_LCEI_ERR_STAT           CAM_LCEI_ERR_STAT;        // 43B8
    CAM_REG_VIPI_ERR_STAT           CAM_VIPI_ERR_STAT;        // 43BC
    CAM_REG_VIP2I_ERR_STAT          CAM_VIP2I_ERR_STAT;       // 43C0
    CAM_REG_IMGO_ERR_STAT           CAM_IMGO_ERR_STAT;        // 43C4
    CAM_REG_IMG2O_ERR_STAT          CAM_IMG2O_ERR_STAT;       // 43C8
    CAM_REG_LCSO_ERR_STAT           CAM_LCSO_ERR_STAT;        // 43CC
    CAM_REG_ESFKO_ERR_STAT          CAM_ESFKO_ERR_STAT;       // 43D0
    CAM_REG_AAO_ERR_STAT            CAM_AAO_ERR_STAT;         // 43D4
    CAM_REG_DMA_DEBUG_ADDR          CAM_DMA_DEBUG_ADDR;       // 43D8
    CAM_REG_DMA_RSV1                CAM_DMA_RSV1;             // 43DC
    CAM_REG_DMA_RSV2                CAM_DMA_RSV2;             // 43E0
    CAM_REG_DMA_RSV3                CAM_DMA_RSV3;             // 43E4
    CAM_REG_DMA_RSV4                CAM_DMA_RSV4;             // 43E8
    CAM_REG_DMA_RSV5                CAM_DMA_RSV5;             // 43EC
    CAM_REG_DMA_RSV6                CAM_DMA_RSV6;             // 43F0
    UINT32                          rsv_43F4[7];              // 43F4..440C
    CAM_REG_TG_SEN_MODE             CAM_TG_SEN_MODE;          // 4410 (start MT6589_201_raw_tg.xml)
    CAM_REG_TG_VF_CON               CAM_TG_VF_CON;            // 4414
    CAM_REG_TG_SEN_GRAB_PXL         CAM_TG_SEN_GRAB_PXL;      // 4418
    CAM_REG_TG_SEN_GRAB_LIN         CAM_TG_SEN_GRAB_LIN;      // 441C
    CAM_REG_TG_PATH_CFG             CAM_TG_PATH_CFG;          // 4420
    CAM_REG_TG_MEMIN_CTL            CAM_TG_MEMIN_CTL;         // 4424
    CAM_REG_TG_INT1                 CAM_TG_INT1;              // 4428
    CAM_REG_TG_INT2                 CAM_TG_INT2;              // 442C
    CAM_REG_TG_SOF_CNT              CAM_TG_SOF_CNT;           // 4430
    CAM_REG_TG_SOT_CNT              CAM_TG_SOT_CNT;           // 4434
    CAM_REG_TG_EOT_CNT              CAM_TG_EOT_CNT;           // 4438
    CAM_REG_TG_ERR_CTL              CAM_TG_ERR_CTL;           // 443C
    CAM_REG_TG_DAT_NO               CAM_TG_DAT_NO;            // 4440
    CAM_REG_TG_FRM_CNT_ST           CAM_TG_FRM_CNT_ST;        // 4444
    CAM_REG_TG_FRMSIZE_ST           CAM_TG_FRMSIZE_ST;        // 4448
    CAM_REG_TG_INTER_ST             CAM_TG_INTER_ST;          // 444C
    UINT32                          rsv_4450[4];              // 4450..445C
    CAM_REG_TG_FLASHA_CTL           CAM_TG_FLASHA_CTL;        // 4460
    CAM_REG_TG_FLASHA_LINE_CNT      CAM_TG_FLASHA_LINE_CNT;   // 4464
    CAM_REG_TG_FLASHA_POS           CAM_TG_FLASHA_POS;        // 4468
    CAM_REG_TG_FLASHB_CTL           CAM_TG_FLASHB_CTL;        // 446C
    CAM_REG_TG_FLASHB_LINE_CNT      CAM_TG_FLASHB_LINE_CNT;   // 4470
    CAM_REG_TG_FLASHB_POS           CAM_TG_FLASHB_POS;        // 4474
    CAM_REG_TG_FLASHB_POS1          CAM_TG_FLASHB_POS1;       // 4478
    CAM_REG_TG_GSCTRL_CTL           CAM_TG_GSCTRL_CTL;        // 447C
    CAM_REG_TG_GSCTRL_TIME          CAM_TG_GSCTRL_TIME;       // 4480
    CAM_REG_TG_MS_PHASE             CAM_TG_MS_PHASE;          // 4484
    CAM_REG_TG_MS_CL_TIME           CAM_TG_MS_CL_TIME;        // 4488
    CAM_REG_TG_MS_OP_TIME           CAM_TG_MS_OP_TIME;        // 448C
    CAM_REG_TG_MS_CLPH_TIME         CAM_TG_MS_CLPH_TIME;      // 4490
    CAM_REG_TG_MS_OPPH_TIME         CAM_TG_MS_OPPH_TIME;      // 4494
    UINT32                          rsv_4498[6];              // 4498..44AC
    CAM_REG_TG2_SEN_MODE            CAM_TG2_SEN_MODE;         // 44B0
    CAM_REG_TG2_VF_CON              CAM_TG2_VF_CON;           // 44B4
    CAM_REG_TG2_SEN_GRAB_PXL        CAM_TG2_SEN_GRAB_PXL;     // 44B8
    CAM_REG_TG2_SEN_GRAB_LIN        CAM_TG2_SEN_GRAB_LIN;     // 44BC
    CAM_REG_TG2_PATH_CFG            CAM_TG2_PATH_CFG;         // 44C0
    CAM_REG_TG2_MEMIN_CTL           CAM_TG2_MEMIN_CTL;        // 44C4
    CAM_REG_TG2_INT1                CAM_TG2_INT1;             // 44C8
    CAM_REG_TG2_INT2                CAM_TG2_INT2;             // 44CC
    CAM_REG_TG2_SOF_CNT             CAM_TG2_SOF_CNT;          // 44D0
    CAM_REG_TG2_SOT_CNT             CAM_TG2_SOT_CNT;          // 44D4
    CAM_REG_TG2_EOT_CNT             CAM_TG2_EOT_CNT;          // 44D8
    CAM_REG_TG2_ERR_CTL             CAM_TG2_ERR_CTL;          // 44DC
    CAM_REG_TG2_DAT_NO              CAM_TG2_DAT_NO;           // 44E0
    CAM_REG_TG2_FRM_CNT_ST          CAM_TG2_FRM_CNT_ST;       // 44E4
    CAM_REG_TG2_FRMSIZE_ST          CAM_TG2_FRMSIZE_ST;       // 44E8
    CAM_REG_TG2_INTER_ST            CAM_TG2_INTER_ST;         // 44EC
    CAM_REG_BIN_SIZE                CAM_BIN_SIZE;             // 44F0 (start MT6589_202_raw_bin.xml)
    CAM_REG_BIN_MODE                CAM_BIN_MODE;             // 44F4
    CAM_REG_BIN2_SIZE               CAM_BIN2_SIZE;            // 44F8
    CAM_REG_BIN2_MODE               CAM_BIN2_MODE;            // 44FC
    CAM_REG_OBC_OFFST0              CAM_OBC_OFFST0;           // 4500 (start MT6589_203_raw_obc.xml)
    CAM_REG_OBC_OFFST1              CAM_OBC_OFFST1;           // 4504
    CAM_REG_OBC_OFFST2              CAM_OBC_OFFST2;           // 4508
    CAM_REG_OBC_OFFST3              CAM_OBC_OFFST3;           // 450C
    CAM_REG_OBC_GAIN0               CAM_OBC_GAIN0;            // 4510
    CAM_REG_OBC_GAIN1               CAM_OBC_GAIN1;            // 4514
    CAM_REG_OBC_GAIN2               CAM_OBC_GAIN2;            // 4518
    CAM_REG_OBC_GAIN3               CAM_OBC_GAIN3;            // 451C
    UINT32                          rsv_4520[4];              // 4520..452C
    CAM_REG_LSC_CTL1                CAM_LSC_CTL1;             // 4530 (start MT6589_204_raw_lsc.xml)
    CAM_REG_LSC_CTL2                CAM_LSC_CTL2;             // 4534
    CAM_REG_LSC_CTL3                CAM_LSC_CTL3;             // 4538
    CAM_REG_LSC_LBLOCK              CAM_LSC_LBLOCK;           // 453C
    CAM_REG_LSC_RATIO               CAM_LSC_RATIO;            // 4540
    CAM_REG_LSC_TPIPE_OFST           CAM_LSC_TPIPE_OFST;        // 4544
    CAM_REG_LSC_TPIPE_SIZE           CAM_LSC_TPIPE_SIZE;        // 4548
    CAM_REG_LSC_GAIN_TH             CAM_LSC_GAIN_TH;          // 454C
    UINT32                          rsv_4550[12];             // 4550..457C
    CAM_REG_HRZ_RES                 CAM_HRZ_RES;              // 4580 (start MT6589_207_raw_hrz.xml)
    CAM_REG_HRZ_OUT                 CAM_HRZ_OUT;              // 4584
    UINT32                          rsv_4588[10];             // 4588..45AC
    CAM_REG_AWB_WIN_ORG             CAM_AWB_WIN_ORG;          // 45B0 (start MT6589_2091_raw_awb.xml)
    CAM_REG_AWB_WIN_SIZE            CAM_AWB_WIN_SIZE;         // 45B4
    CAM_REG_AWB_WIN_PITCH           CAM_AWB_WIN_PITCH;        // 45B8
    CAM_REG_AWB_WIN_NUM             CAM_AWB_WIN_NUM;          // 45BC
    CAM_REG_AWB_RAWPREGAIN1_0       CAM_AWB_RAWPREGAIN1_0;    // 45C0
    CAM_REG_AWB_RAWPREGAIN1_1       CAM_AWB_RAWPREGAIN1_1;    // 45C4
    CAM_REG_AWB_RAWLIMIT1_0         CAM_AWB_RAWLIMIT1_0;      // 45C8
    CAM_REG_AWB_RAWLIMIT1_1         CAM_AWB_RAWLIMIT1_1;      // 45CC
    CAM_REG_AWB_LOW_THR             CAM_AWB_LOW_THR;          // 45D0
    CAM_REG_AWB_HI_THR              CAM_AWB_HI_THR;           // 45D4
    CAM_REG_AWB_PIXEL_CNT0          CAM_AWB_PIXEL_CNT0;       // 45D8
    CAM_REG_AWB_PIXEL_CNT1          CAM_AWB_PIXEL_CNT1;       // 45DC
    CAM_REG_AWB_PIXEL_CNT2          CAM_AWB_PIXEL_CNT2;       // 45E0
    CAM_REG_AWB_ERR_THR             CAM_AWB_ERR_THR;          // 45E4
    CAM_REG_AWB_ROT                 CAM_AWB_ROT;              // 45E8
    CAM_REG_AWB_L0_X                CAM_AWB_L0_X;             // 45EC
    CAM_REG_AWB_L0_Y                CAM_AWB_L0_Y;             // 45F0
    CAM_REG_AWB_L1_X                CAM_AWB_L1_X;             // 45F4
    CAM_REG_AWB_L1_Y                CAM_AWB_L1_Y;             // 45F8
    CAM_REG_AWB_L2_X                CAM_AWB_L2_X;             // 45FC
    CAM_REG_AWB_L2_Y                CAM_AWB_L2_Y;             // 4600
    CAM_REG_AWB_L3_X                CAM_AWB_L3_X;             // 4604
    CAM_REG_AWB_L3_Y                CAM_AWB_L3_Y;             // 4608
    CAM_REG_AWB_L4_X                CAM_AWB_L4_X;             // 460C
    CAM_REG_AWB_L4_Y                CAM_AWB_L4_Y;             // 4610
    CAM_REG_AWB_L5_X                CAM_AWB_L5_X;             // 4614
    CAM_REG_AWB_L5_Y                CAM_AWB_L5_Y;             // 4618
    CAM_REG_AWB_L6_X                CAM_AWB_L6_X;             // 461C
    CAM_REG_AWB_L6_Y                CAM_AWB_L6_Y;             // 4620
    CAM_REG_AWB_L7_X                CAM_AWB_L7_X;             // 4624
    CAM_REG_AWB_L7_Y                CAM_AWB_L7_Y;             // 4628
    CAM_REG_AWB_L8_X                CAM_AWB_L8_X;             // 462C
    CAM_REG_AWB_L8_Y                CAM_AWB_L8_Y;             // 4630
    CAM_REG_AWB_L9_X                CAM_AWB_L9_X;             // 4634
    CAM_REG_AWB_L9_Y                CAM_AWB_L9_Y;             // 4638
    CAM_REG_AWB_SPARE               CAM_AWB_SPARE;            // 463C
    UINT32                          rsv_4640[4];              // 4640..464C
    CAM_REG_AE_HST_CTL              CAM_AE_HST_CTL;           // 4650 (start MT6589_2092_raw_ae.xml)
    CAM_REG_AE_RAWPREGAIN2_0        CAM_AE_RAWPREGAIN2_0;     // 4654
    CAM_REG_AE_RAWPREGAIN2_1        CAM_AE_RAWPREGAIN2_1;     // 4658
    CAM_REG_AE_RAWLIMIT2_0          CAM_AE_RAWLIMIT2_0;       // 465C
    CAM_REG_AE_RAWLIMIT2_1          CAM_AE_RAWLIMIT2_1;       // 4660
    CAM_REG_AE_MATRIX_COEF0         CAM_AE_MATRIX_COEF0;      // 4664
    CAM_REG_AE_MATRIX_COEF1         CAM_AE_MATRIX_COEF1;      // 4668
    CAM_REG_AE_MATRIX_COEF2         CAM_AE_MATRIX_COEF2;      // 466C
    CAM_REG_AE_MATRIX_COEF3         CAM_AE_MATRIX_COEF3;      // 4670
    CAM_REG_AE_MATRIX_COEF4         CAM_AE_MATRIX_COEF4;      // 4674
    CAM_REG_AE_YGAMMA_0             CAM_AE_YGAMMA_0;          // 4678
    CAM_REG_AE_YGAMMA_1             CAM_AE_YGAMMA_1;          // 467C
    CAM_REG_AE_HST_SET              CAM_AE_HST_SET;           // 4680
    CAM_REG_AE_HST0_RNG             CAM_AE_HST0_RNG;          // 4684
    CAM_REG_AE_HST1_RNG             CAM_AE_HST1_RNG;          // 4688
    CAM_REG_AE_HST2_RNG             CAM_AE_HST2_RNG;          // 468C
    CAM_REG_AE_HST3_RNG             CAM_AE_HST3_RNG;          // 4690
    CAM_REG_AE_SPARE                CAM_AE_SPARE;             // 4694
    UINT32                          rsv_4698[2];              // 4698..469C
    CAM_REG_SGG_PGN                 CAM_SGG_PGN;              // 46A0 (start MT6589_210_raw_sgg.xml)
    CAM_REG_SGG_GMR                 CAM_SGG_GMR;              // 46A4
    UINT32                          rsv_46A8[2];              // 46A8..46AC
    CAM_REG_AF_CON                  CAM_AF_CON;               // 46B0 (start MT6589_211_raw_af.xml)
    CAM_REG_AF_WINX01               CAM_AF_WINX01;            // 46B4
    CAM_REG_AF_WINX23               CAM_AF_WINX23;            // 46B8
    CAM_REG_AF_WINX45               CAM_AF_WINX45;            // 46BC
    CAM_REG_AF_WINY01               CAM_AF_WINY01;            // 46C0
    CAM_REG_AF_WINY23               CAM_AF_WINY23;            // 46C4
    CAM_REG_AF_WINY45               CAM_AF_WINY45;            // 46C8
    CAM_REG_AF_SIZE                 CAM_AF_SIZE;              // 46CC
    CAM_REG_AF_FILT0                CAM_AF_FILT0;             // 46D0
    CAM_REG_AF_FILT1_P14            CAM_AF_FILT1_P14;         // 46D4
    CAM_REG_AF_FILT1_P58            CAM_AF_FILT1_P58;         // 46D8
    CAM_REG_AF_FILT1_P912           CAM_AF_FILT1_P912;        // 46DC
    CAM_REG_AF_TH                   CAM_AF_TH;                // 46E0
    CAM_REG_AF_WIN_E                CAM_AF_WIN_E;             // 46E4
    CAM_REG_AF_SIZE_E               CAM_AF_SIZE_E;            // 46E8
    CAM_REG_AF_TH_E                 CAM_AF_TH_E;              // 46EC
    CAM_REG_AF_IN_SIZE              CAM_AF_IN_SIZE;           // 46F0
    UINT32                          rsv_46F4[31];             // 46F4..476C
    CAM_REG_FLK_CON                 CAM_FLK_CON;              // 4770 (start MT6589_212_raw_flk.xml)
    CAM_REG_FLK_SOFST               CAM_FLK_SOFST;            // 4774
    CAM_REG_FLK_WSIZE               CAM_FLK_WSIZE;            // 4778
    CAM_REG_FLK_WNUM                CAM_FLK_WNUM;             // 477C
    CAM_REG_LCS_CON                 CAM_LCS_CON;              // 4780 (start MT6589_213_raw_lcs.xml)
    CAM_REG_LCS_ST                  CAM_LCS_ST;               // 4784
    CAM_REG_LCS_AWS                 CAM_LCS_AWS;              // 4788
    CAM_REG_LCS_FLARE               CAM_LCS_FLARE;            // 478C
    CAM_REG_LCS_LRZ_HDR             CAM_LCS_LRZ_HDR;          // 4790
    CAM_REG_LCS_LRZ_VDR             CAM_LCS_LRZ_VDR;          // 4794
    UINT32                          rsv_4798[26];             // 4798..47FC
    CAM_REG_BPC_CON                 CAM_BPC_CON;              // 4800 (start MT6589_216_raw_bpc.xml)
    CAM_REG_BPC_CD1_1               CAM_BPC_CD1_1;            // 4804
    CAM_REG_BPC_CD1_2               CAM_BPC_CD1_2;            // 4808
    CAM_REG_BPC_CD1_3               CAM_BPC_CD1_3;            // 480C
    CAM_REG_BPC_CD1_4               CAM_BPC_CD1_4;            // 4810
    CAM_REG_BPC_CD1_5               CAM_BPC_CD1_5;            // 4814
    CAM_REG_BPC_CD1_6               CAM_BPC_CD1_6;            // 4818
    CAM_REG_BPC_CD2_1               CAM_BPC_CD2_1;            // 481C
    CAM_REG_BPC_CD2_2               CAM_BPC_CD2_2;            // 4820
    CAM_REG_BPC_CD2_3               CAM_BPC_CD2_3;            // 4824
    CAM_REG_BPC_CD0                 CAM_BPC_CD0;              // 4828
    CAM_REG_BPC_DET                 CAM_BPC_DET;              // 482C
    CAM_REG_BPC_COR                 CAM_BPC_COR;              // 4830
    UINT32                          rsv_4834;                 // 4834
    CAM_REG_BPC_TBLI1               CAM_BPC_TBLI1;            // 4838
    CAM_REG_BPC_TBLI2               CAM_BPC_TBLI2;            // 483C
    CAM_REG_NR1_CON                 CAM_NR1_CON;              // 4840 (start MT6589_216_raw_ct.xml)
    CAM_REG_NR1_CT_CON              CAM_NR1_CT_CON;           // 4844
    CAM_REG_BNR_RSV1                CAM_BNR_RSV1;             // 4848
    CAM_REG_BNR_RSV2                CAM_BNR_RSV2;             // 484C
    UINT32                          rsv_4850[12];             // 4850..487C
    CAM_REG_PGN_SATU01              CAM_PGN_SATU01;           // 4880 (start MT6589_217_raw_pgn.xml)
    CAM_REG_PGN_SATU23              CAM_PGN_SATU23;           // 4884
    CAM_REG_PGN_GAIN01              CAM_PGN_GAIN01;           // 4888
    CAM_REG_PGN_GAIN23              CAM_PGN_GAIN23;           // 488C
    CAM_REG_PGN_OFFS01              CAM_PGN_OFFS01;           // 4890
    CAM_REG_PGN_OFFS23              CAM_PGN_OFFS23;           // 4894
    UINT32                          rsv_4898[2];              // 4898..489C
    CAM_REG_CFA_BYPASS              CAM_CFA_BYPASS;           // 48A0 (start MT6589_300_rgb_cfa.xml)
    CAM_REG_CFA_ED_F                CAM_CFA_ED_F;             // 48A4
    CAM_REG_CFA_ED_NYQ              CAM_CFA_ED_NYQ;           // 48A8
    CAM_REG_CFA_ED_STEP             CAM_CFA_ED_STEP;          // 48AC
    CAM_REG_CFA_RGB_HF              CAM_CFA_RGB_HF;           // 48B0
    CAM_REG_CFA_BW                  CAM_CFA_BW;               // 48B4
    CAM_REG_CFA_F1_ACT              CAM_CFA_F1_ACT;           // 48B8
    CAM_REG_CFA_F2_ACT              CAM_CFA_F2_ACT;           // 48BC
    CAM_REG_CFA_F3_ACT              CAM_CFA_F3_ACT;           // 48C0
    CAM_REG_CFA_F4_ACT              CAM_CFA_F4_ACT;           // 48C4
    CAM_REG_CFA_F1_L                CAM_CFA_F1_L;             // 48C8
    CAM_REG_CFA_F2_L                CAM_CFA_F2_L;             // 48CC
    CAM_REG_CFA_F3_L                CAM_CFA_F3_L;             // 48D0
    CAM_REG_CFA_F4_L                CAM_CFA_F4_L;             // 48D4
    CAM_REG_CFA_HF_RB               CAM_CFA_HF_RB;            // 48D8
    CAM_REG_CFA_HF_GAIN             CAM_CFA_HF_GAIN;          // 48DC
    CAM_REG_CFA_HF_COMP             CAM_CFA_HF_COMP;          // 48E0
    CAM_REG_CFA_HF_CORING_TH        CAM_CFA_HF_CORING_TH;     // 48E4
    CAM_REG_CFA_ACT_LUT             CAM_CFA_ACT_LUT;          // 48E8
    UINT32                          rsv_48EC;                 // 48EC
    CAM_REG_CFA_SPARE               CAM_CFA_SPARE;            // 48F0
    CAM_REG_CFA_BB                  CAM_CFA_BB;               // 48F4
    UINT32                          rsv_48F8[6];              // 48F8..490C
    CAM_REG_CCL_GTC                 CAM_CCL_GTC;              // 4910 (start MT6589_301_rgb_ccl.xml)
    CAM_REG_CCL_ADC                 CAM_CCL_ADC;              // 4914
    CAM_REG_CCL_BAC                 CAM_CCL_BAC;              // 4918
    UINT32                          rsv_491C;                 // 491C
    CAM_REG_G2G_CONV0A              CAM_G2G_CONV0A;           // 4920 (start MT6589_302_rgb_g2g.xml)
    CAM_REG_G2G_CONV0B              CAM_G2G_CONV0B;           // 4924
    CAM_REG_G2G_CONV1A              CAM_G2G_CONV1A;           // 4928
    CAM_REG_G2G_CONV1B              CAM_G2G_CONV1B;           // 492C
    CAM_REG_G2G_CONV2A              CAM_G2G_CONV2A;           // 4930
    CAM_REG_G2G_CONV2B              CAM_G2G_CONV2B;           // 4934
    CAM_REG_G2G_ACC                 CAM_G2G_ACC;              // 4938
    UINT32                          rsv_493C[3];              // 493C..4944
    CAM_REG_UNP_OFST                CAM_UNP_OFST;             // 4948 (start MT6589_304_raw_unp.xml)
    UINT32                          rsv_494C;                 // 494C
    CAM_REG_C02_CON                 CAM_C02_CON;              // 4950 (start MT6589_305_rgb_c02.xml)
    UINT32                          rsv_4954[3];              // 4954..495C
    CAM_REG_MFB_CON                 CAM_MFB_CON;              // 4960 (start MT6589_306_rgb_mfb.xml)
    CAM_REG_MFB_BRZ                 CAM_MFB_BRZ;              // 4964
    CAM_REG_MFB_LL                  CAM_MFB_LL;               // 4968
    UINT32                          rsv_496C[21];             // 496C..49BC
    CAM_REG_LCE_CON                 CAM_LCE_CON;              // 49C0 (start MT6589_307_rgb_lce.xml)
    CAM_REG_LCE_ZR                  CAM_LCE_ZR;               // 49C4
    CAM_REG_LCE_QUA                 CAM_LCE_QUA;              // 49C8
    CAM_REG_LCE_THR                 CAM_LCE_THR;              // 49CC
    CAM_REG_LCE_HSI                 CAM_LCE_HSI;              // 49D0
    CAM_REG_LCE_HSO                 CAM_LCE_HSO;              // 49D4
    CAM_REG_LCE_HSS1                CAM_LCE_HSS1;             // 49D8
    CAM_REG_LCE_HSS2                CAM_LCE_HSS2;             // 49DC
    CAM_REG_LCE_SLM_LB              CAM_LCE_SLM_LB;           // 49E0
    CAM_REG_LCE_SLM_SIZE            CAM_LCE_SLM_SIZE;         // 49E4
    CAM_REG_LCE_OFST                CAM_LCE_OFST;             // 49E8
    CAM_REG_LCE_BIAS                CAM_LCE_BIAS;             // 49EC
    CAM_REG_LCE_TPIPE_SIZE           CAM_LCE_TPIPE_SIZE;        // 49F0
    UINT32                          rsv_49F4[3];              // 49F4..49FC
    CAM_REG_G2C_CONV_0A             CAM_G2C_CONV_0A;          // 4A00 (start MT6589_400_yuv_g2c.xml)
    CAM_REG_G2C_CONV_0B             CAM_G2C_CONV_0B;          // 4A04
    CAM_REG_G2C_CONV_1A             CAM_G2C_CONV_1A;          // 4A08
    CAM_REG_G2C_CONV_1B             CAM_G2C_CONV_1B;          // 4A0C
    CAM_REG_G2C_CONV_2A             CAM_G2C_CONV_2A;          // 4A10
    CAM_REG_G2C_CONV_2B             CAM_G2C_CONV_2B;          // 4A14
    UINT32                          rsv_4A18;                 // 4A18
    CAM_REG_C42_CON                 CAM_C42_CON;              // 4A1C (start MT6589_401_yuv_c42.xml)
    CAM_REG_ANR_CON1                CAM_ANR_CON1;             // 4A20 (start MT6589_402_yuv_nbc.xml)
    CAM_REG_ANR_CON2                CAM_ANR_CON2;             // 4A24
    CAM_REG_ANR_CON3                CAM_ANR_CON3;             // 4A28
    CAM_REG_ANR_YAD1                CAM_ANR_YAD1;             // 4A2C
    CAM_REG_ANR_YAD2                CAM_ANR_YAD2;             // 4A30
    CAM_REG_ANR_4LUT1               CAM_ANR_4LUT1;            // 4A34
    CAM_REG_ANR_4LUT2               CAM_ANR_4LUT2;            // 4A38
    CAM_REG_ANR_4LUT3               CAM_ANR_4LUT3;            // 4A3C
    CAM_REG_ANR_PTY                 CAM_ANR_PTY;              // 4A40
    CAM_REG_ANR_CAD                 CAM_ANR_CAD;              // 4A44
    CAM_REG_ANR_PTC                 CAM_ANR_PTC;              // 4A48
    CAM_REG_ANR_LCE1                CAM_ANR_LCE1;             // 4A4C
    CAM_REG_ANR_LCE2                CAM_ANR_LCE2;             // 4A50
    CAM_REG_ANR_HP1                 CAM_ANR_HP1;              // 4A54
    CAM_REG_ANR_HP2                 CAM_ANR_HP2;              // 4A58
    CAM_REG_ANR_HP3                 CAM_ANR_HP3;              // 4A5C
    CAM_REG_ANR_ACTY                CAM_ANR_ACTY;             // 4A60
    CAM_REG_ANR_ACTC                CAM_ANR_ACTC;             // 4A64
    CAM_REG_ANR_RSV1                CAM_ANR_RSV1;             // 4A68
    CAM_REG_ANR_RSV2                CAM_ANR_RSV2;             // 4A6C
    UINT32                          rsv_4A70[8];              // 4A70..4A8C
    CAM_REG_CCR_CON                 CAM_CCR_CON;              // 4A90
    CAM_REG_CCR_YLUT                CAM_CCR_YLUT;             // 4A94
    CAM_REG_CCR_UVLUT               CAM_CCR_UVLUT;            // 4A98
    CAM_REG_CCR_YLUT2               CAM_CCR_YLUT2;            // 4A9C
    CAM_REG_SEEE_SRK_CTRL           CAM_SEEE_SRK_CTRL;        // 4AA0 (start MT6589_403_yuv_seee.xml)
    CAM_REG_SEEE_CLIP_CTRL          CAM_SEEE_CLIP_CTRL;       // 4AA4
    CAM_REG_SEEE_HP_CTRL1           CAM_SEEE_HP_CTRL1;        // 4AA8
    CAM_REG_SEEE_HP_CTRL2           CAM_SEEE_HP_CTRL2;        // 4AAC
    CAM_REG_SEEE_ED_CTRL1           CAM_SEEE_ED_CTRL1;        // 4AB0
    CAM_REG_SEEE_ED_CTRL2           CAM_SEEE_ED_CTRL2;        // 4AB4
    CAM_REG_SEEE_ED_CTRL3           CAM_SEEE_ED_CTRL3;        // 4AB8
    CAM_REG_SEEE_ED_CTRL4           CAM_SEEE_ED_CTRL4;        // 4ABC
    CAM_REG_SEEE_ED_CTRL5           CAM_SEEE_ED_CTRL5;        // 4AC0
    CAM_REG_SEEE_ED_CTRL6           CAM_SEEE_ED_CTRL6;        // 4AC4
    CAM_REG_SEEE_ED_CTRL7           CAM_SEEE_ED_CTRL7;        // 4AC8
    CAM_REG_SEEE_EDGE_CTRL          CAM_SEEE_EDGE_CTRL;       // 4ACC
    CAM_REG_SEEE_Y_CTRL             CAM_SEEE_Y_CTRL;          // 4AD0
    CAM_REG_SEEE_EDGE_CTRL1         CAM_SEEE_EDGE_CTRL1;      // 4AD4
    CAM_REG_SEEE_EDGE_CTRL2         CAM_SEEE_EDGE_CTRL2;      // 4AD8
    CAM_REG_SEEE_EDGE_CTRL3         CAM_SEEE_EDGE_CTRL3;      // 4ADC
    CAM_REG_SEEE_SPECIAL_CTRL       CAM_SEEE_SPECIAL_CTRL;    // 4AE0
    CAM_REG_SEEE_CORE_CTRL1         CAM_SEEE_CORE_CTRL1;      // 4AE4
    CAM_REG_SEEE_CORE_CTRL2         CAM_SEEE_CORE_CTRL2;      // 4AE8
    CAM_REG_SEEE_EE_LINK1           CAM_SEEE_EE_LINK1;        // 4AEC
    CAM_REG_SEEE_EE_LINK2           CAM_SEEE_EE_LINK2;        // 4AF0
    CAM_REG_SEEE_EE_LINK3           CAM_SEEE_EE_LINK3;        // 4AF4
    CAM_REG_SEEE_EE_LINK4           CAM_SEEE_EE_LINK4;        // 4AF8
    CAM_REG_SEEE_EE_LINK5           CAM_SEEE_EE_LINK5;        // 4AFC
    CAM_REG_CDRZ_CONTROL            CAM_CDRZ_CONTROL;         // 4B00 (start MT6589_500_cdp_cdrz.xml)
    CAM_REG_CDRZ_INPUT_IMAGE        CAM_CDRZ_INPUT_IMAGE;     // 4B04
    CAM_REG_CDRZ_OUTPUT_IMAGE       CAM_CDRZ_OUTPUT_IMAGE;    // 4B08
    CAM_REG_CDRZ_HORIZONTAL_COEFF_STEP CAM_CDRZ_HORIZONTAL_COEFF_STEP; // 4B0C
    CAM_REG_CDRZ_VERTICAL_COEFF_STEP CAM_CDRZ_VERTICAL_COEFF_STEP; // 4B10
    CAM_REG_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET; // 4B14
    CAM_REG_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET; // 4B18
    CAM_REG_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET; // 4B1C
    CAM_REG_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET; // 4B20
    CAM_REG_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET; // 4B24
    CAM_REG_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET; // 4B28
    CAM_REG_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET; // 4B2C
    CAM_REG_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET; // 4B30
    CAM_REG_CDRZ_DERING_1           CAM_CDRZ_DERING_1;        // 4B34
    CAM_REG_CDRZ_DERING_2           CAM_CDRZ_DERING_2;        // 4B38
    UINT32                          rsv_4B3C;                 // 4B3C
    CAM_REG_CURZ_CONTROL            CAM_CURZ_CONTROL;         // 4B40 (start MT6589_501_cdp_curz.xml)
    CAM_REG_CURZ_INPUT_IMAGE        CAM_CURZ_INPUT_IMAGE;     // 4B44
    CAM_REG_CURZ_OUTPUT_IMAGE       CAM_CURZ_OUTPUT_IMAGE;    // 4B48
    CAM_REG_CURZ_HORIZONTAL_COEFF_STEP CAM_CURZ_HORIZONTAL_COEFF_STEP; // 4B4C
    CAM_REG_CURZ_VERTICAL_COEFF_STEP CAM_CURZ_VERTICAL_COEFF_STEP; // 4B50
    CAM_REG_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET; // 4B54
    CAM_REG_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET; // 4B58
    CAM_REG_CURZ_LUMA_VERTICAL_INTEGER_OFFSET CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET; // 4B5C
    CAM_REG_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET; // 4B60
    CAM_REG_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET; // 4B64
    CAM_REG_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET; // 4B68
    CAM_REG_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET; // 4B6C
    CAM_REG_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET; // 4B70
    CAM_REG_CURZ_DERING_1           CAM_CURZ_DERING_1;        // 4B74
    CAM_REG_CURZ_DERING_2           CAM_CURZ_DERING_2;        // 4B78
    UINT32                          rsv_4B7C[9];              // 4B7C..4B9C
    CAM_REG_PRZ_CONTROL             CAM_PRZ_CONTROL;          // 4BA0 (start MT6589_502_cdp_prz.xml)
    CAM_REG_PRZ_INPUT_IMAGE         CAM_PRZ_INPUT_IMAGE;      // 4BA4
    CAM_REG_PRZ_OUTPUT_IMAGE        CAM_PRZ_OUTPUT_IMAGE;     // 4BA8
    CAM_REG_PRZ_HORIZONTAL_COEFF_STEP CAM_PRZ_HORIZONTAL_COEFF_STEP; // 4BAC
    CAM_REG_PRZ_VERTICAL_COEFF_STEP CAM_PRZ_VERTICAL_COEFF_STEP; // 4BB0
    CAM_REG_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET; // 4BB4
    CAM_REG_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET; // 4BB8
    CAM_REG_PRZ_LUMA_VERTICAL_INTEGER_OFFSET CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET; // 4BBC
    CAM_REG_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET; // 4BC0
    CAM_REG_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET; // 4BC4
    CAM_REG_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET; // 4BC8
    CAM_REG_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET; // 4BCC
    CAM_REG_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET; // 4BD0
    CAM_REG_PRZ_DERING_1            CAM_PRZ_DERING_1;         // 4BD4
    CAM_REG_PRZ_DERING_2            CAM_PRZ_DERING_2;         // 4BD8
    UINT32                          rsv_4BDC[17];             // 4BDC..4C1C
    CAM_REG_FE_CTRL                 CAM_FE_CTRL;              // 4C20 (start MT6589_504_cdp_fe.xml)
    CAM_REG_FE_IDX_CTRL             CAM_FE_IDX_CTRL;          // 4C24
    CAM_REG_FE_CROP_CTRL1           CAM_FE_CROP_CTRL1;        // 4C28
    CAM_REG_FE_CROP_CTRL2           CAM_FE_CROP_CTRL2;        // 4C2C
    UINT32                          rsv_4C30[36];             // 4C30..4CBC
    CAM_REG_VIDO_CTRL               CAM_VIDO_CTRL;            // 4CC0 (start MT6589_508_cdp_vido.xml)
    CAM_REG_VIDO_DMA_PERF           CAM_VIDO_DMA_PERF;        // 4CC4
    CAM_REG_VIDO_MAIN_BUF_SIZE      CAM_VIDO_MAIN_BUF_SIZE;   // 4CC8
    CAM_REG_VIDO_BUF_SIZE           CAM_VIDO_BUF_SIZE;        // 4CCC
    CAM_REG_VIDO_SOFT_RST           CAM_VIDO_SOFT_RST;        // 4CD0
    CAM_REG_VIDO_SOFT_RST_STAT      CAM_VIDO_SOFT_RST_STAT;   // 4CD4
    CAM_REG_VIDO_INT_EN             CAM_VIDO_INT_EN;          // 4CD8
    CAM_REG_VIDO_INT                CAM_VIDO_INT;             // 4CDC
    CAM_REG_VIDO_CROP_OFST          CAM_VIDO_CROP_OFST;       // 4CE0
    CAM_REG_VIDO_TAR_SIZE           CAM_VIDO_TAR_SIZE;        // 4CE4
    CAM_REG_VIDO_BASE_ADDR          CAM_VIDO_BASE_ADDR;       // 4CE8
    CAM_REG_VIDO_OFST_ADDR          CAM_VIDO_OFST_ADDR;       // 4CEC
    CAM_REG_VIDO_STRIDE             CAM_VIDO_STRIDE;          // 4CF0
    CAM_REG_VIDO_BASE_ADDR_C        CAM_VIDO_BASE_ADDR_C;     // 4CF4
    CAM_REG_VIDO_OFST_ADDR_C        CAM_VIDO_OFST_ADDR_C;     // 4CF8
    CAM_REG_VIDO_STRIDE_C           CAM_VIDO_STRIDE_C;        // 4CFC
    CAM_REG_VIDO_BUF_BASE_ADDR0     CAM_VIDO_BUF_BASE_ADDR0;  // 4D00
    CAM_REG_VIDO_BUF_BASE_ADDR1     CAM_VIDO_BUF_BASE_ADDR1;  // 4D04
    CAM_REG_VIDO_BUF_BASE_ADDR2     CAM_VIDO_BUF_BASE_ADDR2;  // 4D08
    CAM_REG_VIDO_BUF_BASE_ADDR3     CAM_VIDO_BUF_BASE_ADDR3;  // 4D0C
    CAM_REG_VIDO_PADDING            CAM_VIDO_PADDING;         // 4D10
    CAM_REG_VIDO_DITHER             CAM_VIDO_DITHER;          // 4D14
    CAM_REG_VIDO_DIT_SEED_R         CAM_VIDO_DIT_SEED_R;      // 4D18
    CAM_REG_VIDO_DIT_SEED_G         CAM_VIDO_DIT_SEED_G;      // 4D1C
    CAM_REG_VIDO_DIT_SEED_B         CAM_VIDO_DIT_SEED_B;      // 4D20
    CAM_REG_VIDO_BASE_ADDR_V        CAM_VIDO_BASE_ADDR_V;     // 4D24
    CAM_REG_VIDO_OFST_ADDR_V        CAM_VIDO_OFST_ADDR_V;     // 4D28
    CAM_REG_VIDO_STRIDE_V           CAM_VIDO_STRIDE_V;        // 4D2C
    CAM_REG_VIDO_RSV_1              CAM_VIDO_RSV_1;           // 4D30
    CAM_REG_VIDO_DMA_PREULTRA       CAM_VIDO_DMA_PREULTRA;    // 4D34
    UINT32                          rsv_4D38[2];              // 4D38..4D3C
    CAM_REG_DISPO_CTRL              CAM_DISPO_CTRL;           // 4D40 (start MT6589_509_cdp_dispo.xml)
    CAM_REG_DISPO_DMA_PERF          CAM_DISPO_DMA_PERF;       // 4D44
    CAM_REG_DISPO_MAIN_BUF_SIZE     CAM_DISPO_MAIN_BUF_SIZE;  // 4D48
    CAM_REG_DISPO_BUF_SIZE          CAM_DISPO_BUF_SIZE;       // 4D4C
    CAM_REG_DISPO_SOFT_RST          CAM_DISPO_SOFT_RST;       // 4D50
    CAM_REG_DISPO_SOFT_RST_STAT     CAM_DISPO_SOFT_RST_STAT;  // 4D54
    CAM_REG_DISPO_INT_EN            CAM_DISPO_INT_EN;         // 4D58
    CAM_REG_DISPO_INT               CAM_DISPO_INT;            // 4D5C
    CAM_REG_DISPO_CROP_OFST         CAM_DISPO_CROP_OFST;      // 4D60
    CAM_REG_DISPO_TAR_SIZE          CAM_DISPO_TAR_SIZE;       // 4D64
    CAM_REG_DISPO_BASE_ADDR         CAM_DISPO_BASE_ADDR;      // 4D68
    CAM_REG_DISPO_OFST_ADDR         CAM_DISPO_OFST_ADDR;      // 4D6C
    CAM_REG_DISPO_STRIDE            CAM_DISPO_STRIDE;         // 4D70
    CAM_REG_DISPO_BASE_ADDR_C       CAM_DISPO_BASE_ADDR_C;    // 4D74
    CAM_REG_DISPO_OFST_ADDR_C       CAM_DISPO_OFST_ADDR_C;    // 4D78
    CAM_REG_DISPO_STRIDE_C          CAM_DISPO_STRIDE_C;       // 4D7C
    CAM_REG_DISPO_BUF_BASE_ADDR0    CAM_DISPO_BUF_BASE_ADDR0; // 4D80
    CAM_REG_DISPO_BUF_BASE_ADDR1    CAM_DISPO_BUF_BASE_ADDR1; // 4D84
    CAM_REG_DISPO_BUF_BASE_ADDR2    CAM_DISPO_BUF_BASE_ADDR2; // 4D88
    CAM_REG_DISPO_BUF_BASE_ADDR3    CAM_DISPO_BUF_BASE_ADDR3; // 4D8C
    CAM_REG_DISPO_PADDING           CAM_DISPO_PADDING;        // 4D90
    CAM_REG_DISPO_DITHER            CAM_DISPO_DITHER;         // 4D94
    CAM_REG_DISPO_DIT_SEED_R        CAM_DISPO_DIT_SEED_R;     // 4D98
    CAM_REG_DISPO_DIT_SEED_G        CAM_DISPO_DIT_SEED_G;     // 4D9C
    CAM_REG_DISPO_DIT_SEED_B        CAM_DISPO_DIT_SEED_B;     // 4DA0
    CAM_REG_DISPO_BASE_ADDR_V       CAM_DISPO_BASE_ADDR_V;    // 4DA4
    CAM_REG_DISPO_OFST_ADDR_V       CAM_DISPO_OFST_ADDR_V;    // 4DA8
    CAM_REG_DISPO_STRIDE_V          CAM_DISPO_STRIDE_V;       // 4DAC
    CAM_REG_DISPO_RSV_1             CAM_DISPO_RSV_1;          // 4DB0
    CAM_REG_DISPO_DMA_PREULTRA      CAM_DISPO_DMA_PREULTRA;   // 4DB4
    UINT32                          rsv_4DB8[2];              // 4DB8..4DBC
    CAM_REG_EIS_PREP_ME_CTRL1       CAM_EIS_PREP_ME_CTRL1;    // 4DC0 (start MT6589_510_cdp_eis.xml)
    CAM_REG_EIS_PREP_ME_CTRL2       CAM_EIS_PREP_ME_CTRL2;    // 4DC4
    CAM_REG_EIS_LMV_TH              CAM_EIS_LMV_TH;           // 4DC8
    CAM_REG_EIS_FL_OFFSET           CAM_EIS_FL_OFFSET;        // 4DCC
    CAM_REG_EIS_MB_OFFSET           CAM_EIS_MB_OFFSET;        // 4DD0
    CAM_REG_EIS_MB_INTERVAL         CAM_EIS_MB_INTERVAL;      // 4DD4
    CAM_REG_EIS_GMV                 CAM_EIS_GMV;              // 4DD8
    CAM_REG_EIS_ERR_CTRL            CAM_EIS_ERR_CTRL;         // 4DDC
    CAM_REG_EIS_IMAGE_CTRL          CAM_EIS_IMAGE_CTRL;       // 4DE0
    UINT32                          rsv_4DE4[19];             // 4DE4..4E2C
    CAM_REG_DGM_B2                  CAM_DGM_B2;               // 4E30 (start MT6589_601_rgb_dgm.xml)
    CAM_REG_DGM_B4                  CAM_DGM_B4;               // 4E34
    CAM_REG_DGM_B6                  CAM_DGM_B6;               // 4E38
    CAM_REG_DGM_B8                  CAM_DGM_B8;               // 4E3C
    CAM_REG_DGM_B10                 CAM_DGM_B10;              // 4E40
    CAM_REG_DGM_B11                 CAM_DGM_B11;              // 4E44
    UINT32                          rsv_4E48[2];              // 4E48..4E4C
    CAM_REG_GDMA_IOTYPE             CAM_GDMA_IOTYPE;          // 4E50 (start MT6589_602_gdma_ctl.xml)
    UINT32                          rsv_4E54;                 // 4E54
    CAM_REG_GDMA_MCUMODE            CAM_GDMA_MCUMODE;         // 4E58
    UINT32                          rsv_4E5C;                 // 4E5C
    CAM_REG_GDMA_SRCOFSTY           CAM_GDMA_SRCOFSTY;        // 4E60
    CAM_REG_GDMA_SRCOFSTC           CAM_GDMA_SRCOFSTC;        // 4E64
    CAM_REG_GDMA_YSRCB1             CAM_GDMA_YSRCB1;          // 4E68
    CAM_REG_GDMA_YSRCB2             CAM_GDMA_YSRCB2;          // 4E6C
    CAM_REG_GDMA_CBSRCB1            CAM_GDMA_CBSRCB1;         // 4E70
    CAM_REG_GDMA_CBSRCB2            CAM_GDMA_CBSRCB2;         // 4E74
    CAM_REG_GDMA_CRSRCB1            CAM_GDMA_CRSRCB1;         // 4E78
    CAM_REG_GDMA_CRSRCB2            CAM_GDMA_CRSRCB2;         // 4E7C
    CAM_REG_GDMA_YSRCSZ             CAM_GDMA_YSRCSZ;          // 4E80
    CAM_REG_GDMA_SPARE              CAM_GDMA_SPARE;           // 4E84
    UINT32                          rsv_4E88[2];              // 4E88..4E8C
    CAM_REG_FMT_IOTYPE              CAM_FMT_IOTYPE;           // 4E90 (start MT6589_603_gdma_fmt.xml)
    CAM_REG_FMT_MCUMODE             CAM_FMT_MCUMODE;          // 4E94
    CAM_REG_FMT_WEBPMODE            CAM_FMT_WEBPMODE;         // 4E98
    CAM_REG_FMT_ULTRAMODE           CAM_FMT_ULTRAMODE;        // 4E9C
    CAM_REG_FMT_MIFMODE             CAM_FMT_MIFMODE;          // 4EA0
    CAM_REG_FMT_SRCBUFL             CAM_FMT_SRCBUFL;          // 4EA4
    CAM_REG_FMT_WINTFON             CAM_FMT_WINTFON;          // 4EA8
    CAM_REG_FMT_TGBUFL              CAM_FMT_TGBUFL;           // 4EAC
    CAM_REG_FMT_YSRCB1              CAM_FMT_YSRCB1;           // 4EB0
    CAM_REG_FMT_CBSRCB1             CAM_FMT_CBSRCB1;          // 4EB4
    CAM_REG_FMT_PREULTRAMODE        CAM_FMT_PREULTRAMODE;     // 4EB8
    CAM_REG_FMT_YTGB1               CAM_FMT_YTGB1;            // 4EBC
    CAM_REG_FMT_CBTGB1              CAM_FMT_CBTGB1;           // 4EC0
    CAM_REG_FMT_CRTGB1              CAM_FMT_CRTGB1;           // 4EC4
    CAM_REG_FMT_YTGB2               CAM_FMT_YTGB2;            // 4EC8
    CAM_REG_FMT_CBTGB2              CAM_FMT_CBTGB2;           // 4ECC
    CAM_REG_FMT_CRTGB2              CAM_FMT_CRTGB2;           // 4ED0
    CAM_REG_FMT_YSRCSZ              CAM_FMT_YSRCSZ;           // 4ED4
    CAM_REG_FMT_RANGE               CAM_FMT_RANGE;            // 4ED8
    CAM_REG_FMT_INTEN               CAM_FMT_INTEN;            // 4EDC
    CAM_REG_G2G2_CONV0A             CAM_G2G2_CONV0A;          // 4EE0 (start MT6589_604_cdp_g2g2.xml)
    CAM_REG_G2G2_CONV0B             CAM_G2G2_CONV0B;          // 4EE4
    CAM_REG_G2G2_CONV1A             CAM_G2G2_CONV1A;          // 4EE8
    CAM_REG_G2G2_CONV1B             CAM_G2G2_CONV1B;          // 4EEC
    CAM_REG_G2G2_CONV2A             CAM_G2G2_CONV2A;          // 4EF0
    CAM_REG_G2G2_CONV2B             CAM_G2G2_CONV2B;          // 4EF4
    CAM_REG_G2G2_ACC                CAM_G2G2_ACC;             // 4EF8
    UINT32                          rsv_4EFC;                 // 4EFC
    CAM_REG_NR3D_BLEND              CAM_NR3D_BLEND;           // 4F00 (start MT6589_605_cdp_3dnr.xml)
    CAM_REG_NR3D_SKIP_KEY           CAM_NR3D_SKIP_KEY;        // 4F04
    CAM_REG_NR3D_FBCNT_OFF          CAM_NR3D_FBCNT_OFF;       // 4F08
    CAM_REG_NR3D_FBCNT_SIZ          CAM_NR3D_FBCNT_SIZ;       // 4F0C
    CAM_REG_NR3D_FB_COUNT           CAM_NR3D_FB_COUNT;        // 4F10
    CAM_REG_NR3D_LIMIT_CPX          CAM_NR3D_LIMIT_CPX;       // 4F14
    CAM_REG_NR3D_LIMIT_Y_CON1       CAM_NR3D_LIMIT_Y_CON1;    // 4F18
    CAM_REG_NR3D_LIMIT_Y_CON2       CAM_NR3D_LIMIT_Y_CON2;    // 4F1C
    CAM_REG_NR3D_LIMIT_Y_CON3       CAM_NR3D_LIMIT_Y_CON3;    // 4F20
    CAM_REG_NR3D_LIMIT_U_CON1       CAM_NR3D_LIMIT_U_CON1;    // 4F24
    CAM_REG_NR3D_LIMIT_U_CON2       CAM_NR3D_LIMIT_U_CON2;    // 4F28
    CAM_REG_NR3D_LIMIT_U_CON3       CAM_NR3D_LIMIT_U_CON3;    // 4F2C
    CAM_REG_NR3D_LIMIT_V_CON1       CAM_NR3D_LIMIT_V_CON1;    // 4F30
    CAM_REG_NR3D_LIMIT_V_CON2       CAM_NR3D_LIMIT_V_CON2;    // 4F34
    CAM_REG_NR3D_LIMIT_V_CON3       CAM_NR3D_LIMIT_V_CON3;    // 4F38
    UINT32                          rsv_4F3C[49];             // 4F3C..4FFC
    CAM_REG_GGM_RB_GMT              CAM_GGM_RB_GMT[144];      // 5000..523C
    UINT32                          rsv_5240[48];             // 5240..52FC
    CAM_REG_GGM_G_GMT               CAM_GGM_G_GMT[144];       // 5300..553C
    UINT32                          rsv_5540[48];             // 5540..55FC
    CAM_REG_GGM_CTRL                CAM_GGM_CTRL;             // 5600
    UINT32                          rsv_5604[127];            // 5604..57FC
    CAM_REG_PCA_TBL                 CAM_PCA_TBL[360];         // 5800..5D9C
    UINT32                          rsv_5DA0[24];             // 5DA0..5DFC
    CAM_REG_PCA_CON1                CAM_PCA_CON1;             // 5E00
    CAM_REG_PCA_CON2                CAM_PCA_CON2;             // 5E04
    UINT32                          rsv_5E08[126];            // 5E08..5FFC
    CAM_REG_TPIPE_RING_CON1          CAM_TPIPE_RING_CON1;       // 6000 (start MT6589_cam_verif.xml)
}isp_reg_t;

#endif // _ISP_REG_H_
