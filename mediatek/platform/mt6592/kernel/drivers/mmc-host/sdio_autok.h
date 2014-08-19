#ifndef MT6582_AUTOK_H
#define MT6582_AUTOK_H

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>

#include "mt_sd.h"

#define AUTOK_READ 0
#define AUTOK_WRITE 1

/*************************************************************************
*           AutoK Implementation
*************************************************************************/
//#define AUTOK_DEBUG
//#define USE_KERNEL_THREAD
//#define CHANGE_SCHED_POLICY
//#define SCHED_POLICY_INFO

#define AUTOK_VERSION_NO (0x62900010)
#define SDIO_AUTOK_ID (1)
#define AUTOK_TUNING_INACCURACY (1)
#define AUTOK_CMD_TIMES (5)
#define AUTOK_RDAT_TIMES (100)
#define AUTOK_WDAT_TIMES (10)
#define CMD_INTERNAL_DELAY_RANGE (6)
#define CKG_DELAY_RANGE (4)

#define TUNING_TEST_TIME (64)
#define DIV_CEIL_FUNC(_n,_d) ((_n)/(_d)+(((_n)%(_d)==0)?0:1))
#define ABS_DIFF(_a,_b)      (((_a)>=(_b))?((_a)-(_b)):((_b)-(_a)))
#define THRESHOLD_VAL(_v,_t) (((_v)>=(_t))?(_t):(_v))
#define FREQ_MHZ_2_PERIOD_CYCLE_IN_PS(_Mhz) (1000000L/(_Mhz))


/*CMD*/
#define SCALE_CMD_RSP_DLY_SEL         (32)
#define SCALE_CKGEN_MSDC_DLY_SEL      (32)
#define SCALE_PAD_TUNE_CMDRDLY        (32)
/*READ*/
#define SCALE_DATA_DRIVING            (8)
#define SCALE_INT_DAT_LATCH_CK_SEL    (8)
#if 1
#define SCALE_IOCON_RDSPL             (2)
#define SCALE_PAD_TUNE_DATRDDLY       (32)
#else
#define SCALE_IOCON_RD0SPL            (2)
#define SCALE_IOCON_RD1SPL            (2)
#define SCALE_IOCON_RD2SPL            (2)
#define SCALE_IOCON_RD3SPL            (2)
#define SCALE_DAT_RDDLY0_D0           (32)
#define SCALE_DAT_RDDLY0_D1           (32)
#define SCALE_DAT_RDDLY0_D2           (32)
#define SCALE_DAT_RDDLY0_D3           (32)
#endif
/*WRITE*/
#define SCALE_WRDAT_CRCS_TA_CNTR      (8)
#define SCALE_IOCON_WD0SPL            (2)
#define SCALE_PAD_TUNE_DATWRDLY       (32)



/*Following definition is provided by spec 3.0*/
#define MAX_DELAY_VARIATION_DUE_TO_TEMPERATURE_IN_PS (2600) /*-25degC~~+125degC*/
#define F208M_CYCLE_IN_PS (4808)


/*Following definition is provieded by designer PVT simulation result
  The data is used to calculating the auto-K stage1 range spec*/
#define MIN_CLK_GEN_DELAY_IN_PS (12123)
#define MAX_CLK_GEN_DELAY_IN_PS (32726)
#define MIN_PAD_DELAY_IN_PS (3111) /*bc_RCcbest : FF/LT/HV*/
#define MAX_PAD_DELAY_IN_PS (8618) /*wcl_RCcworst SS/LT/LV*/
#define SCALE_OF_CLK_GEN_2_PAD_DELAY (MIN_CLK_GEN_DELAY_IN_PS/MIN_PAD_DELAY_IN_PS)


#define MIN_SCORE_OF_CLK_GEN_IN_ONE_CYCLE(_periodCycle) \
       (DIV_CEIL_FUNC((SCALE_CKGEN_MSDC_DLY_SEL*(_periodCycle)), MAX_CLK_GEN_DELAY_IN_PS))

#define MIN_SCORE_OF_PAD_DELAY_IN_ONE_CYCLE(_periodCycle) \
       (DIV_CEIL_FUNC((SCALE_PAD_TUNE_CMDRDLY*(_periodCycle)), MAX_PAD_DELAY_IN_PS))

#define MAX_SCALE_OF_CLK_GEN_IN_ONE_CYCLE(_periodCycle) \
       (DIV_CEIL_FUNC((SCALE_CKGEN_MSDC_DLY_SEL*(_periodCycle)), MIN_CLK_GEN_DELAY_IN_PS))

#define MAX_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR \
       (DIV_CEIL_FUNC((SCALE_PAD_TUNE_CMDRDLY*MAX_DELAY_VARIATION_DUE_TO_TEMPERATURE_IN_PS), MIN_PAD_DELAY_IN_PS))

#define MIN_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR \
       (DIV_CEIL_FUNC((SCALE_PAD_TUNE_CMDRDLY*MAX_DELAY_VARIATION_DUE_TO_TEMPERATURE_IN_PS), MAX_PAD_DELAY_IN_PS))
       
#define USER_DEF_MAX_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR(_x,_y) \
       (DIV_CEIL_FUNC(((_x)*MAX_DELAY_VARIATION_DUE_TO_TEMPERATURE_IN_PS), (_y)))

#define MIN_DATA_SCORE (MIN_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR - AUTOK_TUNING_INACCURACY)

typedef enum
{
/*CMD*/
    E_MSDC_PAD_TUNE_CMDRRDLY = 0,
    E_MSDC_CMD_RSP_TA_CNTR,
    E_MSDC_IOCON_RSPL,
    E_MSDC_CKGEN_MSDC_DLY_SEL,
    E_MSDC_PAD_TUNE_CMDRDLY,
/*READ*/
    E_MSDC_INT_DAT_LATCH_CK_SEL,
#if 1
    E_MSDC_IOCON_RDSPL,
    E_MSDC_PAD_TUNE_DATRRDLY,
#else
    E_IOCON_RD0SPL,
    E_IOCON_RD1SPL,
    E_IOCON_RD2SPL,
    E_IOCON_RD3SPL,
    E_DAT_RDDLY0_D0,
    E_DAT_RDDLY0_D1,
    E_DAT_RDDLY0_D2,
    E_DAT_RDDLY0_D3,
#endif
/*WRITE*/
    E_MSDC_WRDAT_CRCS_TA_CNTR,
    E_MSDC_IOCON_WDSPL,
    E_MSDC_PAD_TUNE_DATWRDLY,
    E_AUTOK_VERSION,
    E_AUTOK_FREQ,
    E_AUTOK_PARM_MAX
}E_AUTOK_PARAM;

#define MAX_AUTOK_DAT_NUM (E_AUTOK_PARM_MAX)

typedef enum
{
    E_RESULT_PASS = 0,
    E_RESULT_CMD_CRC = 1,
    E_RESULT_W_CRC = 2,
    E_RESULT_R_CRC = 3,
    E_RESULT_ERR = 4,
    E_RESULT_START = 5,
    E_RESULT_PW_SMALL = 6,
    E_RESULT_KEEP_OLD = 7,
    E_RESULT_TO = 8,
    E_RESULT_CMP_ERR = 9,
    E_RESULT_MAX
}E_RESULT_TYPE;

typedef enum
{
    AUTOK_CMD = 0,
    AUTOK_DATA,
    AUTOK_FAIL,
    AUTOK_DONE
}E_AUTOK_STATE;

typedef enum
{
    ERR_NONE = 0,
    ERR_OCCURE,
    PASS_AFTER_ERR,
    ERR_MAX
}E_AUTOK_ERR_STA;

typedef enum
{
    PERIOD_NONE = 0,
    PERIOD_F_FIRST_POS,
    PERIOD_F_FIRST_POS_DONE,
    PERIOD_F_SECOND_POS,
    PERIOD_L_FIRST_POS,
    PERIOD_L_FIRST_POS_DONE,
    PERIOD_L_SECOND_POS,
    PERIOD_DONE,
    PERIOD_MAX,
}E_AUTOK_PERIOD_STA;

typedef struct
{
    unsigned int interDelaySel;
    unsigned int cmdScore;
    unsigned int cmdPadSel;
    unsigned int readScore;
    unsigned int readPadSel;
    unsigned int writeScore;
    unsigned int writePadSel;
}S_AUTOK_CKGEN_DATA;

typedef enum
{
    SEARCH_FIRST_PASS = 0,
    SEARCH_SECOND_PASS,
    SEARCH_PASS_REGION,
    PASS_REGION_GET,
    SEARCH_MAX
}E_AUTOK_DATA_STA;

typedef struct
{
    unsigned int raw_data;
    unsigned int score;
    unsigned int numOfzero;
    unsigned int fstPosErr;
    unsigned int period;
}S_AUTOK_CMD_DLY;

typedef struct
{
    unsigned int sel;
    unsigned int bRange;
    unsigned int range;
}S_AUTOK_DATA;

typedef enum
{
    TUNING_STG1 = 0,
    TUNING_STG2,
    TUNING_STG_MAX
}E_AUTOK_TUNING_STAGE;


typedef union
{
    unsigned int version;
    unsigned int freq;
    S_AUTOK_DATA data;
}U_AUTOK_INTERFACE_DATA;


#ifdef AUTOK_DEBUG
static char g_tune_result_str[33];
static char *g_tune_name_str[E_AUTOK_VERSION] = {
    "CMD_RSP_DLY_SEL",
    "CMD_RSP_TA_CNTR",
    "IOCON_RSPL",
    "CKGEN_MSDC_DLY_SEL",
    "PAD_TUNE_CMDRDLY",
    "INT_DAT_LATCH_CK_SEL",
    "IOCON_RDSPL",
    "PAD_TUNE_DATRRDLY",
    "WRDAT_CRCS_TA_CNTR",
    "IOCON_WD0SPL",
    "PAD_TUNE_DATWRDLY",
};

#endif

static const unsigned int tuning_data[] =       {0xAA55AA55,0xAA558080,0x807F8080,
                                                 0x807F7F7F,0x807F7F7F,0x404040BF,
                                                 0xBFBF40BF,0xBFBF2020,0x20DF2020,
                                                 0x20DFDFDF,0x101010EF,0xEFEF10EF,
                                                 0xEFEF0808,0x08F70808,0x08F7F7F7,
                                                 0x040404FB,0xFBFB04FB,0xFBFB0202,
                                                 0x02FD0202,0x02FDFDFD,0x010101FE,
                                                 0xFEFE01FE,0xFEFE0000,0x00FF0000,
                                                 0x00FFFFFF,0x000000FF,0xFFFF00FF,
                                                 0xFFFF0000,0xFF0FFF00,0xFFCCC3CC,
                                                 0xC33CCCFF,0xFEFFFEEF,0xFFDFFFDD,
                                                 0xFFFBFFFB,0xBFFF7FFF,0x77F7BDEF,
                                                 0xFFF0FFF0,0x0FFCCC3C,0xCC33CCCF,
                                                 0xFFEFFFEE,0xFFFDFFFD,0xDFFFBFFF,
                                                 0xBBFFF7FF,0xF77F7BDE};


#define TUNING_DATA_NO  (sizeof(tuning_data)/sizeof(unsigned int))


/*TODO: just remove those def and include the correct one header*/
#define SDIO_IP_WTMDR       	(0x00B0)
#define SDIO_IP_WTMCR       	(0x00B4)
#define SDIO_IP_WTMDPCR0    	(0x00B8)
#define SDIO_IP_WTMDPCR1    	(0x00BC)
#define SDIO_IP_WPLRCR      	(0x00D4)
#define TEST_MODE_STATUS        (0x100)

enum AUTOK_PARAM {
    CMD_EDGE,                       // command response sample selection (MSDC_SMPL_RISING, MSDC_SMPL_FALLING)
    RDATA_EDGE,                     // read data sample selection (MSDC_SMPL_RISING, MSDC_SMPL_FALLING)
    WDATA_EDGE,                    // write data sample selection (MSDC_SMPL_RISING, MSDC_SMPL_FALLING)
    CLK_DRV,                            // clock driving
    CMD_DRV,                          // command driving
    DAT_DRV,                           // data driving
    DAT0_RD_DLY,                   // DAT0 Pad RX Delay Line Control (for MSDC RD), Total 32 stages
    DAT1_RD_DLY,                   // DAT1 Pad RX Delay Line Control (for MSDC RD), Total 32 stages
    DAT2_RD_DLY,                   // DAT2 Pad RX Delay Line Control (for MSDC RD), Total 32 stages
    DAT3_RD_DLY,                   // DAT3 Pad RX Delay Line Control (for MSDC RD), Total 32 stages
    DAT_WRD_DLY,                  // Write Data Status Internal Delay Line Control. This register is used to fine-tune write status phase latched by MSDC internal clock. Total 32 stages
    DAT_RD_DLY,                  // Rx  Delay Line Control. Total 32 stages
    CMD_RESP_RD_DLY,          // CMD Response Internal Delay Line Control. This register is used to fine-tune response phase  latched by MSDC internal clock. Total 32 stages
    CMD_RD_DLY,                    // CMD Pad RX Delay Line Control. This register is used to fine-tune CMD pad macro respose latch timing. Total 32 stages
    DATA_DLYLINE_SEL,           // Data line delay line fine tune selection. 1'b0: All data line share one delay selection value indicated by PAD_TUNE.PAD_DAT_RD_RXDLY. 1'b1: Each data line has its own delay selection value indicated by Data line (x): DAT_RD_DLY(x).DAT0_RD_DLY
    READ_DATA_SMPL_SEL,     // Data line rising/falling latch  fine tune selection in read transaction. 1'b0: All data line share one value indicated by MSDC_IOCON.R_D_SMPL. 1'b1: Each data line has its own  selection value indicated by Data line (x): MSDC_IOCON.R_D(x)_SMPL
    WRITE_DATA_SMPL_SEL,   // Data line rising/falling latch  fine tune selection in write transaction. 1'b0: All data line share one value indicated by MSDC_IOCON.W_D_SMPL. 1'b1: Each data line has its own  selection value indicated by Data line (x): MSDC_IOCON.W_D(x)_SMPL
    INT_DAT_LATCH_CK,          // Internal MSDC clock phase selection. Total 8 stages, each stage can delay 1 clock period of msdc_src_ck
    CKGEN_MSDC_DLY_SEL,    // CKBUF in CKGEN Delay Selection. Total 32 stages
    CMD_RSP_TA_CNTR,          // CMD response turn around period. The turn around cycle = CMD_RSP_TA_CNTR + 2, Only for USH104 mode, this register should be set to 0 in non-UHS104 mode
    WRDAT_CRCS_TA_CNTR,   // Write data and CRC status turn around period. The turn around cycle = WRDAT_CRCS_TA_CNTR + 2, Only for USH104 mode,  this register should be set to 0 in non-UHS104 mode
    PAD_CLK_TXDLY,                // CLK Pad TX Delay Control. This register is used to add delay to CLK phase. Total 32 stages
    TOTAL_PARAM_COUNT
};

struct sdio_autok_params
{
    u32 cmd_edge;
    u32 rdata_edge;
    u32 wdata_edge;
    u32 clk_drv;
    u32 cmd_drv;
    u32 dat_drv;
    u32 dat0_rd_dly;
    u32 dat1_rd_dly;
    u32 dat2_rd_dly;
    u32 dat3_rd_dly;
    u32 dat_wrd_dly;
    u32 cmd_resp_rd_dly;
    u32 cmd_rd_dly;
    u32 int_dat_latch_ck;
    u32 ckgen_msdc_dly_sel;
    u32 cmd_rsp_ta_cntr;
    u32 wrdat_crcs_ta_cntr;
    u32 pad_clk_txdly;
};

#ifdef USE_KERNEL_THREAD
struct sdio_autok_thread_data
{
	struct msdc_host *host;
	char autok_stage1_result[256];
	int len;
	char stage;
};
#else   // USE_KERNEL_THREAD
struct sdio_autok_workqueue_data
{
	struct delayed_work autok_delayed_work;
	struct msdc_host *host;
	char autok_stage1_result[256];
	int len;
	char stage;
};
#endif  // USE_KERNEL_THREAD

#define LTE_MODEM_FUNC (1)
#define CMD_52         (52)
#define CMD_53         (53)

#define REQ_CMD_EIO    (0x1 << 0)
#define REQ_CMD_TMO    (0x1 << 1)
#define REQ_DAT_ERR    (0x1 << 2)

#define MSDC_READ      (0)
#define MSDC_WRITE     (1)

int msdc_autok_read(struct msdc_host *host, unsigned int u4Addr, unsigned int u4Func, void *pBuffer, unsigned int u4Len, unsigned int u4Cmd);
int msdc_autok_write(struct msdc_host *host, unsigned int u4Addr, unsigned int u4Func, void *pBuffer, unsigned int u4Len, unsigned int u4Cmd);
int msdc_autok_adjust_param(struct msdc_host *host, enum AUTOK_PARAM param, u32 *value, int rw);
int msdc_autok_stg1_cal(struct msdc_host *host);
int msdc_autok_stg1_data_get(void **ppData, int *pLen);
int msdc_autok_stg2_cal(struct msdc_host *host, void *pData, int len);

#endif /* end of MT6582_AUTOK_H */
