#ifndef __seninf_reg_h__
#define __seninf_reg_h__


// ----------------- seninf_top Bit Field Definitions -------------------

//#define SENINF_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
//#define SENINF_REG(RegBase, RegName) (RegBase->RegName.Raw)

// New macro for read ISP registers.
#define SENINF_READ_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define SENINF_READ_REG(RegBase, RegName)              (RegBase->RegName.Raw)

// New macro for write ISP registers except CAM_CTL_EN1/CAM_CTL_EN2/CAM_DMA_EN/CAM_CTL_EN1_SET/
// CAM_CTL_EN2_SET/CAM_CTL_DMA_EN_SET/CAM_CTL_EN1_CLR/CAM_CTL_EN2_CLR/CAM_CTL_DMA_EN_CLR
// For CAM_CTL_EN1/CAM_CTL_EN2/CAM_DMA_EN/CAM_CTL_EN1_SET/CAM_CTL_EN2_SET/CAM_CTL_DMA_EN_SET/CAM_CTL_EN1_CLR/
// CAM_CTL_EN2_CLR/CAM_CTL_DMA_EN_CLR, use ISP_WRITE_ENABLE_BITS()/ISP_WRITE_ENABLE_REG() instead.
#define SENINF_WRITE_BITS(RegBase, RegName, FieldName, Value)              \
    do {                                                                \
        (RegBase->RegName.Bits.FieldName) = (Value);                    \
        dsb();                                                          \
    } while (0)

#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
        dsb();                                                          \
    } while (0)



//#define SENINF_BASE_HW     0x15008000
#define SENINF_BASE_HW     0x15008000
#define SENINF_BASE_RANGE  0x800
#define PACKING volatile
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6589_seninf_top.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD SENINF1_PCLK_SEL          : 1;
        FIELD SENINF2_PCLK_SEL          : 1;
        FIELD SENINF2_PCLK_EN           : 1;
        FIELD SENINF1_PCLK_EN           : 1;
        FIELD rsv_12                    : 4;
        FIELD SENINF_TOP_N3D_SW_RST     : 1;
        FIELD rsv_17                    : 14;
        FIELD SENINF_TOP_DBG_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CTRL;

/* end MT6589_seninf_top.xml*/

/* start MT6589_seninf.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_INTEN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_INTSTA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_FORMAT             : 4;
        FIELD SENINF_EN                 : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_SPARE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_DATA0              : 12;
        FIELD rsv_12                    : 4;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DATA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_INTEN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_INTSTA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DEBUG_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_FORMAT             : 4;
        FIELD SENINF_EN                 : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_SPARE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_DATA0              : 12;
        FIELD rsv_12                    : 4;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_DATA;

/* end MT6589_seninf.xml*/

/* start MT6589_csi2.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DELAY;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTEN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTSTA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_ECCDBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CRCDBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_SHORT_INFO;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNFSM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNMUX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_HSYNC_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CAL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_BIST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DELAY;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_INTEN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_INTSTA;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_ECCDBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CRCDBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_VER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_SHORT_INFO;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_LNFSM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_LNMUX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_HSYNC_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CAL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_VS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_BIST;

/* end MT6589_csi2.xml*/

/* start MT6589_seninf_scam.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD INTEN0                    : 1;
        FIELD INTEN1                    : 1;
        FIELD INTEN2                    : 1;
        FIELD INTEN3                    : 1;
        FIELD INTEN4                    : 1;
        FIELD INTEN5                    : 1;
        FIELD INTEN6                    : 1;
        FIELD rsv_7                     : 1;
        FIELD Cycle                     : 3;
        FIELD rsv_11                    : 1;
        FIELD Clock_inverse             : 1;
        FIELD rsv_13                    : 4;
        FIELD Continuous_mode           : 1;
        FIELD rsv_18                    : 2;
        FIELD Debug_mode                : 1;
        FIELD rsv_21                    : 3;
        FIELD CSD_NUM                   : 2;
        FIELD rsv_26                    : 2;
        FIELD Warning_mask              : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD Enable                    : 1;
        FIELD rsv_1                     : 15;
        FIELD Reset                     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD INT0                      : 1;
        FIELD INT1                      : 1;
        FIELD INT2                      : 1;
        FIELD INT3                      : 1;
        FIELD INT4                      : 1;
        FIELD INT5                      : 1;
        FIELD INT6                      : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD WIDTH                     : 12;
        FIELD rsv_12                    : 4;
        FIELD HEIGHT                    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DIS_GATED_CLK             : 1;
        FIELD Reserved                  : 31;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CFG2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LINE_ID                   : 16;
        FIELD PACKET_SIZE               : 16;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INFO0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD Reserved                  : 8;
        FIELD DATA_ID                   : 6;
        FIELD CRC_ON                    : 1;
        FIELD ACTIVE                    : 1;
        FIELD DATA_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INFO1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD FEND_CNT                  : 4;
        FIELD W_CRC_CNT                 : 4;
        FIELD W_SYNC_CNT                : 4;
        FIELD W_PID_CNT                 : 4;
        FIELD W_LID_CNT                 : 4;
        FIELD W_DID_CNT                 : 4;
        FIELD W_SIZE_CNT                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_STA;

/* end MT6589_seninf_scam.xml*/

/* start MT6589_seninf_tg.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_PH_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_SEN_CK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CTL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CLK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_PH_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_SEN_CK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_CTL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_CLK;

/* end MT6589_seninf_tg.xml*/

/* start MT6589_seninf_ccir656.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_REV_0             : 1;
        FIELD CCIR656_REV_1             : 1;
        FIELD CCIR656_HS_POL            : 1;
        FIELD CCIR656_VS_POL            : 1;
        FIELD CCIR656_PT_EN             : 1;
        FIELD CCIR656_EN                : 1;
        FIELD rsv_6                     : 2;
        FIELD CCIR656_DBG_SEL           : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_CTL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_HS_START          : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_HS_END            : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_HTOTAL         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HACTIVE        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_HWIDTH         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HSTART         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_VTOTAL         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VACTIVE        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_VWIDTH         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VSTART         : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_TYPE           : 8;
        FIELD rsv_8                     : 8;
        FIELD CCIR656_PT_COLOR_BAR_TH   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_Y              : 8;
        FIELD CCIR656_PT_CB             : 8;
        FIELD CCIR656_PT_CR             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_BD_Y           : 8;
        FIELD CCIR656_PT_BD_CB          : 8;
        FIELD CCIR656_PT_BD_CR          : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_IN_FIELD          : 1;
        FIELD CCIR656_IN_VS             : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_STATUS;

/* end MT6589_seninf_ccir656.xml*/

/* start MT6589_seninf_n3d.xml*/
typedef PACKING union
{
    PACKING struct
    {
        FIELD MODE                      : 2;
        FIELD I2C1_EN                   : 1;
        FIELD I2C2_EN                   : 1;
        FIELD I2C1_INT_EN               : 1;
        FIELD I2C2_INT_EN               : 1;
        FIELD N3D_EN                    : 1;
        FIELD W1CLR                     : 1;
        FIELD DIFF_EN                   : 1;
        FIELD DDBG_SEL                  : 3;
        FIELD MODE1_DBG                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_CTL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD N3D_POS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_POS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD I2CA_TRIG                 : 1;
        FIELD I2CB_TRIG                 : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_TRIG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD I2C1_INT                  : 1;
        FIELD I2C2_INT                  : 1;
        FIELD DIFF_INT                  : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_INT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD N3D_CNT0                  : 32;
    } Bits;
    UINT32 Raw;
} REG_CNT0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD N3D_CNT1                  : 32;
    } Bits;
    UINT32 Raw;
} REG_CNT1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD N3D_DBG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD N3D_DIFF_THR              : 32;
    } Bits;
    UINT32 Raw;
} REG_DIFF_THR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD N3D_DIFF_CNT              : 32;
    } Bits;
    UINT32 Raw;
} REG_DIFF_CNT;

/* end MT6589_seninf_n3d.xml*/

// ----------------- seninf_top  Grouping Definitions -------------------
// ----------------- seninf_top Register Definition -------------------
typedef volatile struct
{
    //UINT32                          rsv_0000[8192];           // 0000..7FFC
    REG_SENINF_TOP_CTRL             SENINF_TOP_CTRL;          // 8000 (start MT6589_seninf_top.xml)
    UINT32                          rsv_8004[3];              // 8004..800C
    REG_SENINF1_CTRL                SENINF1_CTRL;             // 8010 (start MT6589_seninf.xml)
    REG_SENINF1_INTEN               SENINF1_INTEN;            // 8014
    REG_SENINF1_INTSTA              SENINF1_INTSTA;           // 8018
    REG_SENINF1_SIZE                SENINF1_SIZE;             // 801C
    REG_SENINF1_DEBUG_1             SENINF1_DEBUG_1;          // 8020
    REG_SENINF1_DEBUG_2             SENINF1_DEBUG_2;          // 8024
    REG_SENINF1_DEBUG_3             SENINF1_DEBUG_3;          // 8028
    REG_SENINF1_DEBUG_4             SENINF1_DEBUG_4;          // 802C
    REG_SENINF1_DEBUG_5             SENINF1_DEBUG_5;          // 8030
    REG_SENINF1_DEBUG_6             SENINF1_DEBUG_6;          // 8034
    REG_SENINF1_DEBUG_7             SENINF1_DEBUG_7;          // 8038
    REG_SENINF1_SPARE               SENINF1_SPARE;            // 803C
    REG_SENINF1_DATA                SENINF1_DATA;             // 8040
    UINT32                          rsv_8044[19];             // 8044..808C
    REG_SENINF2_CTRL                SENINF2_CTRL;             // 8090
    REG_SENINF2_INTEN               SENINF2_INTEN;            // 8094
    REG_SENINF2_INTSTA              SENINF2_INTSTA;           // 8098
    REG_SENINF2_SIZE                SENINF2_SIZE;             // 809C
    REG_SENINF2_DEBUG_1             SENINF2_DEBUG_1;          // 80A0
    REG_SENINF2_DEBUG_2             SENINF2_DEBUG_2;          // 80A4
    REG_SENINF2_DEBUG_3             SENINF2_DEBUG_3;          // 80A8
    REG_SENINF2_DEBUG_4             SENINF2_DEBUG_4;          // 80AC
    REG_SENINF2_DEBUG_5             SENINF2_DEBUG_5;          // 80B0
    REG_SENINF2_DEBUG_6             SENINF2_DEBUG_6;          // 80B4
    REG_SENINF2_DEBUG_7             SENINF2_DEBUG_7;          // 80B8
    REG_SENINF2_SPARE               SENINF2_SPARE;            // 80BC
    REG_SENINF2_DATA                SENINF2_DATA;             // 80C0
    UINT32                          rsv_80C4[15];             // 80C4..80FC
    REG_SENINF1_CSI2_CTRL           SENINF1_CSI2_CTRL;        // 8100 (start MT6589_csi2.xml)
    REG_SENINF1_CSI2_DELAY          SENINF1_CSI2_DELAY;       // 8104
    REG_SENINF1_CSI2_INTEN          SENINF1_CSI2_INTEN;       // 8108
    REG_SENINF1_CSI2_INTSTA         SENINF1_CSI2_INTSTA;      // 810C
    REG_SENINF1_CSI2_ECCDBG         SENINF1_CSI2_ECCDBG;      // 8110
    REG_SENINF1_CSI2_CRCDBG         SENINF1_CSI2_CRCDBG;      // 8114
    REG_SENINF1_CSI2_DBG            SENINF1_CSI2_DBG;         // 8118
    REG_SENINF1_CSI2_VER            SENINF1_CSI2_VER;         // 811C
    REG_SENINF1_CSI2_SHORT_INFO     SENINF1_CSI2_SHORT_INFO;  // 8120
    REG_SENINF1_CSI2_LNFSM          SENINF1_CSI2_LNFSM;       // 8124
    REG_SENINF1_CSI2_LNMUX          SENINF1_CSI2_LNMUX;       // 8128
    REG_SENINF1_CSI2_HSYNC_CNT      SENINF1_CSI2_HSYNC_CNT;   // 812C
    REG_SENINF1_CSI2_CAL            SENINF1_CSI2_CAL;         // 8130
    REG_SENINF1_CSI2_DS             SENINF1_CSI2_DS;          // 8134
    REG_SENINF1_CSI2_VS             SENINF1_CSI2_VS;          // 8138
    REG_SENINF1_CSI2_BIST           SENINF1_CSI2_BIST;        // 813C
    UINT32                          rsv_8140[16];             // 8140..817C
    REG_SENINF2_CSI2_CTRL           SENINF2_CSI2_CTRL;        // 8180
    REG_SENINF2_CSI2_DELAY          SENINF2_CSI2_DELAY;       // 8184
    REG_SENINF2_CSI2_INTEN          SENINF2_CSI2_INTEN;       // 8188
    REG_SENINF2_CSI2_INTSTA         SENINF2_CSI2_INTSTA;      // 818C
    REG_SENINF2_CSI2_ECCDBG         SENINF2_CSI2_ECCDBG;      // 8190
    REG_SENINF2_CSI2_CRCDBG         SENINF2_CSI2_CRCDBG;      // 8194
    REG_SENINF2_CSI2_DBG            SENINF2_CSI2_DBG;         // 8198
    REG_SENINF2_CSI2_VER            SENINF2_CSI2_VER;         // 819C
    REG_SENINF2_CSI2_SHORT_INFO     SENINF2_CSI2_SHORT_INFO;  // 81A0
    REG_SENINF2_CSI2_LNFSM          SENINF2_CSI2_LNFSM;       // 81A4
    REG_SENINF2_CSI2_LNMUX          SENINF2_CSI2_LNMUX;       // 81A8
    REG_SENINF2_CSI2_HSYNC_CNT      SENINF2_CSI2_HSYNC_CNT;   // 81AC
    REG_SENINF2_CSI2_CAL            SENINF2_CSI2_CAL;         // 81B0
    REG_SENINF2_CSI2_DS             SENINF2_CSI2_DS;          // 81B4
    REG_SENINF2_CSI2_VS             SENINF2_CSI2_VS;          // 81B8
    REG_SENINF2_CSI2_BIST           SENINF2_CSI2_BIST;        // 81BC
    UINT32                          rsv_81C0[16];             // 81C0..81FC
    REG_SCAM1_CFG                   SCAM1_CFG;                // 8200 (start MT6589_seninf_scam.xml)
    REG_SCAM1_CON                   SCAM1_CON;                // 8204
    UINT32                          rsv_8208;                 // 8208
    REG_SCAM1_INT                   SCAM1_INT;                // 820C
    REG_SCAM1_SIZE                  SCAM1_SIZE;               // 8210
    UINT32                          rsv_8214[3];              // 8214..821C
    REG_SCAM1_CFG2                  SCAM1_CFG2;               // 8220
    UINT32                          rsv_8224[3];              // 8224..822C
    REG_SCAM1_INFO0                 SCAM1_INFO0;              // 8230
    REG_SCAM1_INFO1                 SCAM1_INFO1;              // 8234
    UINT32                          rsv_8238[2];              // 8238..823C
    REG_SCAM1_STA                   SCAM1_STA;                // 8240
    UINT32                          rsv_8244[47];             // 8244..82FC
    SENINF_REG_TG1_PH_CNT           SENINF_TG1_PH_CNT;        // 8300 (start MT6589_seninf_tg.xml)
    SENINF_REG_TG1_SEN_CK           SENINF_TG1_SEN_CK;        // 8304
    SENINF_REG_TG1_TM_CTL           SENINF_TG1_TM_CTL;        // 8308
    SENINF_REG_TG1_TM_SIZE          SENINF_TG1_TM_SIZE;       // 830C
    SENINF_REG_TG1_TM_CLK           SENINF_TG1_TM_CLK;        // 8310
    UINT32                          rsv_8314[35];             // 8314..839C
    SENINF_REG_TG2_PH_CNT           SENINF_TG2_PH_CNT;        // 83A0
    SENINF_REG_TG2_SEN_CK           SENINF_TG2_SEN_CK;        // 83A4
    SENINF_REG_TG2_TM_CTL           SENINF_TG2_TM_CTL;        // 83A8
    SENINF_REG_TG2_TM_SIZE          SENINF_TG2_TM_SIZE;       // 83AC
    SENINF_REG_TG2_TM_CLK           SENINF_TG2_TM_CLK;        // 83B0
    UINT32                          rsv_83B4[19];             // 83B4..83FC
    REG_CCIR656_CTL                 CCIR656_CTL;              // 8400 (start MT6589_seninf_ccir656.xml)
    REG_CCIR656_H                   CCIR656_H;                // 8404
    REG_CCIR656_PTGEN_H_1           CCIR656_PTGEN_H_1;        // 8408
    REG_CCIR656_PTGEN_H_2           CCIR656_PTGEN_H_2;        // 840C
    REG_CCIR656_PTGEN_V_1           CCIR656_PTGEN_V_1;        // 8410
    REG_CCIR656_PTGEN_V_2           CCIR656_PTGEN_V_2;        // 8414
    REG_CCIR656_PTGEN_CTL1          CCIR656_PTGEN_CTL1;       // 8418
    REG_CCIR656_PTGEN_CTL2          CCIR656_PTGEN_CTL2;       // 841C
    REG_CCIR656_PTGEN_CTL3          CCIR656_PTGEN_CTL3;       // 8420
    REG_CCIR656_STATUS              CCIR656_STATUS;           // 8424
    UINT32                          rsv_8428[54];             // 8428..84FC
    REG_CTL                         N3D_CTL;                  // 8500 (start MT6589_seninf_n3d.xml)
    REG_POS                         N3D_POS;                  // 8504
    REG_TRIG                        N3D_TRIG;                 // 8508
    REG_INT                         N3D_INT;                  // 850C
    REG_CNT0                        N3D_CNT0;                 // 8510
    REG_CNT1                        N3D_CNT1;                 // 8514
    REG_DBG                         N3D_DBG;                  // 8518
    REG_DIFF_THR                    N3D_DIFF_THR;             // 851C
    REG_DIFF_CNT                    N3D_DIFF_CNT;             // 8520
}seninf_reg_t;

#undef PACKING
#endif // __seninf_reg_h__
