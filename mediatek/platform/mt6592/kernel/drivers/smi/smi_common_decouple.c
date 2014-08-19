#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <mach/mt_clkmgr.h>
#include <asm/io.h>

#include <mach/m4u.h>
#include <mach/mt_smi.h>
#include "smi_reg.h"
#include "smi_common.h"


#define SMI_LOG_TAG "SMI"

#define LARB0_PORT_NUM 10
#define LARB1_PORT_NUM 7
#define LARB2_PORT_NUM 19
#define LARB5_PORT_NUM 4

// SMI policies
#define SMI_ADJ_POLICY_DDR1333_720P 0
#define SMI_ADJ_POLICY_DDR1333_FHD 1
#define SMI_ADJ_POLICY_DDR1333_WUXGA 2
#define SMI_ADJ_POLICY_DDR1066_720P 3
#define SMI_ADJ_POLICY_DDR1066_FHD 4
#define SMI_ADJ_POLICY_DDR1066_WUXGA 5

// Pixel limitation selection for HWC
#define SF_HWC_PIXEL_MAX_NORMAL  (1920 * 1080 * 2 + 110 * 1080)
#define SF_HWC_PIXEL_MAX_VR  (1280 * 720 * 4)
#define SF_HWC_PIXEL_MAX_VP  (1920 * 1080 * 2 + 110 * 1080)
#define SF_HWC_PIXEL_MAX_ALWAYS_GPU (1280 * 720 * 2)
#define SF_HWC_PIXEL_MAX_ALWAYS_HWC (1920 * 1080 * 6)

#define SF_HWC_PIXEL_BYTE_MAX_NORMAL  (SF_HWC_PIXEL_MAX_NORMAL * 4 )
#define SF_HWC_PIXEL_BYTE_MAX_VR  (SF_HWC_PIXEL_MAX_VR * 4 )
#define SF_HWC_PIXEL_BYTE_MAX_VP  (SF_HWC_PIXEL_MAX_VP * 4 )


// GPU limitation selection
#define GPU_SMI_L1_LIMIT_0 (0)
#define GPU_SMI_L1_LIMIT_1 ((0x1 <<23) + (0x2<<18)+ (0x6 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_2 ((0x1 <<23) + (0x2<<18)+ (0x3 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_3 ((0x1 <<23) + (0x2<<18)+ (0x2 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_4 ((0x1 <<23) + (0x1<<18)+ (0x1 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)

#define GPU_SMI_L1_LIMIT_OPT_0 ((0x1 <<23) + (0x3<<18)+ (0x9 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_OPT_1 ((0x1 <<23) + (0x3<<18)+ (0x7 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_OPT_2 ((0x1 <<23) + (0x3<<18)+ (0x6 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_OPT_3 ((0x1 <<23) + (0x3<<18)+ (0x5 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)

#define GPU_SMI_L1_LIMIT_VP_GP_OPT ((0x1 <<23) + (0x2<<18)+ (0x1 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_VP_PP_OPT ((0x1 <<23) + (0x2<<18)+ (0x7 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)

#define GPU_SMI_L1_LIMIT_WFDVP_GP_OPT ((0x1 <<23) + (0x2<<18)+ (0x1 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_WFDVP_PP_OPT ((0x1 <<23) + (0x2<<18)+ (0x5 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)

#define GPU_SMI_L1_LIMIT_VR_GP_OPT ((0x1 <<23) + (0x2<<18)+ (0x2 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)
#define GPU_SMI_L1_LIMIT_VR_PP_OPT ((0x1 <<23) + (0x2<<18)+ (0x4 <<13)+ (0x1 << 12) + (0x0 << 11)+ 0x0)


#define SMIDBG(level, x...)            \
    do{                        \
    if (smi_debug_level >= (level))    \
    SMIMSG(x);            \
    } while (0)

#define SMI_REGISTER_DBG(level, x...)   \
    do{           \
    if (smi_debug_level >= (level)) \
    smi_dumpDebugMsg();   \
    } while (0)

typedef struct
{
    spinlock_t SMI_lock;
    unsigned long pu4ConcurrencyTable[SMI_BWC_SCEN_CNT];  //one bit represent one module
} SMI_struct;

extern unsigned int mt_get_emi_freq(void);
extern UINT32 DISP_GetScreenWidth(void);
extern UINT32 DISP_GetScreenHeight(void);

static SMI_struct g_SMIInfo;

static const unsigned int gLarbBaseAddr[SMI_LARB_NR] = 
{LARB0_BASE, LARB1_BASE, LARB2_BASE, SMI_ERROR_ADDR, SMI_ERROR_ADDR, LARB5_BASE}; 

// Structures for larb backup/ restore
static const unsigned int larb_port_num[SMI_LARB_NR]=
{LARB0_PORT_NUM, LARB1_PORT_NUM, LARB2_PORT_NUM, 0, 0, LARB5_PORT_NUM };

static unsigned short int larb0_port_backup[LARB0_PORT_NUM];
static unsigned short int larb1_port_backup[LARB1_PORT_NUM];
static unsigned short int larb2_port_backup[LARB2_PORT_NUM];
static unsigned short int larb5_port_backup[LARB5_PORT_NUM];

static unsigned short int * larb_port_backup[SMI_LARB_NR] = 
{ larb0_port_backup, larb1_port_backup, larb2_port_backup, NULL, NULL, larb5_port_backup };

// For IPO init
static int ipo_force_init = 0;
// To keep the HW's init value
static int is_default_value_saved = 0;

static unsigned int default_val_smi_l1arb[SMI_LARB_NR] = { 0 };

static unsigned int hwc_max_pixel_count = SF_HWC_PIXEL_MAX_NORMAL;

static unsigned int smi_adj_policy = SMI_ADJ_POLICY_DDR1066_WUXGA;

// For user space debugging
static unsigned int wifi_disp_transaction = 0;

static unsigned int mhl_disp_transaction = 0;

static unsigned int smi_debug_level = 0;

static unsigned int smi_tuning_mode = 0;

static unsigned int smi_pixel_count = 0;

static unsigned int smi_common_regs[3] = {0};

static int smi_common_regs_size = 3;

static unsigned int smi_common_larb_regs[6] = {0};

static unsigned int smi_profile = SMI_BWC_SCEN_NORMAL;

static int smi_common_larb_regs_size = 6;

char *smi_port_name[][19] = 
{
    {
        "disp_ovl",
            "disp_rdma1",
            "disp_rdma",
            "disp_wdma",
            "mm_cmdq",
            "mdp_rdma",
            "mdp_wdma",
            "mdp_rot_y",
            "mdp_rot_u",
            "mdp_rot_v",
    },
    {
        "vdec_mc",
            "vdec_pp",
            "vdec_avc_mv",
            "vdec_pred_rd",
            "vdec_pred_wr",
            "vdec_vld",
            "vdec_ppwrap",
    },
    {
        "cam_imgo",
            "cam_img2o",
            "cam_lsci",
            "cam_imgi",
            "cam_esfko",
            "cam_aao",
            "cam_lcei",
            "cam_lcso",
            "jpgenc_rdma",
            "jpgenc_bsdma",
            "venc_rd_comv",
            "venc_sv_comv",
            "venc_rcpu",
            "venc_rec_frm",
            "venc_ref_luma",
            "venc_ref_chroma",
            "venc_bsdma",
            "venc_cur_luma",
            "venc_cur_chroma",
    },
    {NULL},
    {NULL},
    {
        "mjc_mvr",
            "mjc_mvw",
            "mjc_rdma",
            "mjc_wdma",
    }
};

static void initSetting(void);
static void vpSetting(void);
static void swvpSetting(void);
static void vrSetting(void);
static void vencSetting(void);
static void set_sf_pixel_limitation(void);

static void backup_larb_smi(int index){

    int port_index = 0;
    unsigned short int *backup_ptr = NULL;
    unsigned int larb_base =  gLarbBaseAddr[index];
    unsigned int larb_offset = 0x200;
    int total_port_num = 0; 

    // boundary check for larb_port_num and larb_port_backup access
    if( index < 0 || index >=SMI_LARB_NR){
        return;
    }
    total_port_num = larb_port_num[index];
    backup_ptr = larb_port_backup[index];

    // boundary check for port value access
    if( total_port_num <= 0 || backup_ptr == NULL){
        return;
    }

    for(port_index = 0; port_index < total_port_num; port_index++){
        *backup_ptr = (unsigned short int)(M4U_ReadReg32(larb_base , larb_offset));
        backup_ptr++;
        larb_offset+=4;
    }
    SMIDBG(1, "Backup smi larb[%d]: 0x%x - 0x%x", index, 0x200, (larb_offset-4));
    return;
}   

static void restore_larb_smi(int index){

    int port_index = 0;
    unsigned short int *backup_ptr = NULL;
    unsigned int larb_base =  gLarbBaseAddr[index];
    unsigned int larb_offset = 0x200;
    unsigned int backup_value = 0;
    int total_port_num = 0; 

    // boundary check for larb_port_num and larb_port_backup access
    if(index < 0 || index >= SMI_LARB_NR){
        return;
    }
    total_port_num = larb_port_num[index];
    backup_ptr = larb_port_backup[index];

    // boundary check for port value access
    if( total_port_num <= 0 || backup_ptr == NULL){
        return;
    }
    for(port_index = 0; port_index < total_port_num; port_index++){
        backup_value = *backup_ptr;
        M4U_WriteReg32(larb_base , larb_offset, backup_value);
        backup_ptr++;
        larb_offset+=4;
    }
    SMIDBG(1, "Restored smi larb[%d]: 0x%x - 0x%x", index, 0x200, (larb_offset-4));
    return;
}


static void update_debug_info(void)
{
    // Dump the resgister settings to module paramters
    smi_common_regs[0] = M4U_ReadReg32(REG_SMI_M4U_TH , 0);
    smi_common_regs[1] = M4U_ReadReg32(REG_SMI_L1LEN,0);
    smi_common_regs[2] = M4U_ReadReg32(REG_SMI_READ_FIFO_TH , 0);

    smi_common_larb_regs[0] = M4U_ReadReg32(REG_SMI_L1ARB0, 0);
    smi_common_larb_regs[1] = M4U_ReadReg32(REG_SMI_L1ARB1, 0); 
    smi_common_larb_regs[2] = M4U_ReadReg32(REG_SMI_L1ARB2, 0);
    smi_common_larb_regs[3] = M4U_ReadReg32(REG_SMI_L1ARB3, 0);
    smi_common_larb_regs[4] = M4U_ReadReg32(REG_SMI_L1ARB4, 0); 
    smi_common_larb_regs[5] = M4U_ReadReg32(REG_SMI_L1ARB5, 0);


    SMIMSG("Current Setting: M4U_TH, L1LEN, READ_FIFO_TH = 0x%x, 0x%x, 0x%x\n", smi_common_regs[0], smi_common_regs[1], smi_common_regs[2] );
    SMIMSG("l1arb[0-2] = 0x%x,  0x%x, 0x%x\n" , smi_common_larb_regs[0], smi_common_larb_regs[1], smi_common_larb_regs[2]);
    SMIMSG("l1arb[3-5] = 0x%x,  0x%x, 0x%x\n" , smi_common_larb_regs[3], smi_common_larb_regs[4], smi_common_larb_regs[5]);

    SMI_REGISTER_DBG(2);

}
// Use this function to get base address of Larb resgister
// to support error checking
int get_larb_base_addr(int larb_id){
    if(larb_id > SMI_LARB_NR || larb_id < 0)
    {
        return SMI_ERROR_ADDR;
    }else{
        return gLarbBaseAddr[larb_id];
    }
}

int larb_clock_on(int larb_id) 
{

#ifndef CONFIG_MTK_FPGA
    char name[30];
    sprintf(name, "smi+%d", larb_id);  

    switch(larb_id)
    {
    case 0: 
        enable_clock(MT_CG_DISP0_SMI_COMMON, name);
        enable_clock(MT_CG_DISP0_SMI_LARB0, name);
        break;
    case 1:
        enable_clock(MT_CG_DISP0_SMI_COMMON, name);
        enable_clock(MT_CG_VDEC1_LARB, name);
        break;
    case 2: 
        enable_clock(MT_CG_DISP0_SMI_COMMON, name);
        enable_clock(MT_CG_IMAGE_LARB2_SMI, name);
        break;
        // TBC: need to confirm with CLK MRG's owner to check there is no deadlock or synchronization issue
    case 5: // Added MJC since MT6592
        enable_clock(MT_CG_DISP0_SMI_COMMON, name); 
        enable_clock(MT_CG_MJC_SMI_LARB, name);
    default: 
        break;
    }
#endif

    return 0;
}

int larb_clock_off(int larb_id) 
{
#ifndef CONFIG_MTK_FPGA

    char name[30];
    sprintf(name, "smi+%d", larb_id);


    switch(larb_id)
    {
    case 0: 
        disable_clock(MT_CG_DISP0_SMI_LARB0, name);
        disable_clock(MT_CG_DISP0_SMI_COMMON, name);
        break;
    case 1:
        disable_clock(MT_CG_VDEC1_LARB, name);
        disable_clock(MT_CG_DISP0_SMI_COMMON, name);
        break;
    case 2: 
        disable_clock(MT_CG_IMAGE_LARB2_SMI, name);
        disable_clock(MT_CG_DISP0_SMI_COMMON, name);
        break;
        // TBC: need to confirm with CLK MRG's owner to check there is no deadlock or synchronization issue
    case 5: // Added MJC since MT6592
        disable_clock(MT_CG_MJC_SMI_LARB, name);
        disable_clock(MT_CG_DISP0_SMI_COMMON, name);
    default: 
        break;
    }
#endif

    return 0;

}


#define LARB_BACKUP_REG_SIZE 128
static unsigned int* pLarbRegBackUp[SMI_LARB_NR];
static int g_bInited = 0;
int larb_reg_backup(int larb)
{
    unsigned int* pReg = NULL;
    unsigned int larb_base = SMI_ERROR_ADDR;
    int i = 0;

    SMIDBG(1, "+larb_reg_backup(), larb_idx=%d \n", larb);
    SMI_REGISTER_DBG(2);

    larb_base = get_larb_base_addr(larb);
    pReg = pLarbRegBackUp[larb];

    // If SMI can't find the corrosponded larb address, skip the backup for the larb
    if(larb_base == SMI_ERROR_ADDR ){
        SMIMSG("Can't find the base address for Larb%d\n", larb);
        return 0;
    }
    SMIDBG(1, "m4u part backup, larb_idx=%d \n", larb);

    *(pReg++) = M4U_ReadReg32(larb_base, SMI_LARB_CON);
    *(pReg++) = M4U_ReadReg32(larb_base, SMI_SHARE_EN);
    //*(pReg++) = M4U_ReadReg32(larb_base, SMI_ROUTE_SEL);

    for(i=0; i<3; i++)
    {
        *(pReg++) = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_START(i));
        *(pReg++) = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_END(i));
        *(pReg++) = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_GID(i));
    }
    SMI_REGISTER_DBG(2);
    // Backup SMI related registers
    SMIDBG(1, "+backup_larb_smi(), larb_idx=%d \n", larb);
    backup_larb_smi(larb);
    SMIDBG(1, "-backup_larb_smi(), larb_idx=%d \n", larb);
    if(0 == larb)
    {
        g_bInited = 0;
    }
    SMIDBG(1, "-larb_reg_backup(), larb_idx=%d \n", larb);
    return 0;
}

static int smi_larb_init(unsigned int larb, int force_init){

    unsigned int regval = 0;
    unsigned int regval1 = 0;
    unsigned int regval2 = 0;
    unsigned int larb_base = get_larb_base_addr(larb);

    //Clock manager enable LARB clock before call back restore already, it will be disabled after restore call back returns
    //Got to enable OSTD before engine starts
    regval = M4U_ReadReg32(larb_base , SMI_LARB_STAT);
    regval1 = M4U_ReadReg32(larb_base , SMI_LARB_MON_BUS_REQ0);
    regval2 = M4U_ReadReg32(larb_base , SMI_LARB_MON_BUS_REQ1);

    if(0 == regval)
    {
        SMIMSG("Init OSTD for larb_base: 0x%x\n" , larb_base);
        M4U_WriteReg32(larb_base , SMI_LARB_OSTD_CTRL_EN , 0xffffffff);
    }
    else
    {
        if( force_init == 1 || ipo_force_init == 1)
        {
            SMIMSG("Larb: 0x%x is busy : 0x%x , port:0x%x,0x%x , didn't set OSTD in force init mode\n" , larb_base , regval , regval1 , regval2);
            SMI_REGISTER_DBG(2);
        }
        else
        {
            SMIMSG("Larb: 0x%x is busy : 0x%x , port:0x%x,0x%x ,fail to set OSTD\n" , larb_base , regval , regval1 , regval2);
            smi_dumpDebugMsg();
            if(smi_debug_level >= 1){
                SMIERR("DISP_MDP LARB  0x%x OSTD cannot be set:0x%x,port:0x%x,0x%x\n" , larb_base , regval , regval1 , regval2);
            }else{
                dump_stack();
            }
        }
    }

    if(0 == g_bInited)
    {
        // smi_profile = SMI_BWC_SCEN_NORMAL;
        if(smi_profile == SMI_BWC_SCEN_VP){
            vpSetting();
        }else if(smi_profile == SMI_BWC_SCEN_SWDEC_VP){
            swvpSetting();
        }else if(smi_profile == SMI_BWC_SCEN_VR){
            vrSetting();
        }else if(smi_profile == SMI_BWC_SCEN_VR_SLOW){
            vrSetting();
        }else if(smi_profile == SMI_BWC_SCEN_VENC){
            vencSetting();
        }else{
            initSetting();
        }
        g_bInited = 1;
        SMIMSG("SMI init\n");
    }else{
        restore_larb_smi(larb);
    }
    return 0;
}

int larb_reg_restore(int larb, int force_init)
{
    unsigned int larb_base = SMI_ERROR_ADDR;
    unsigned int regval = 0;
    int i = 0;
    unsigned int* pReg = NULL;

    larb_base = get_larb_base_addr(larb);

    // The larb assign doesn't exist
    if(larb_base == SMI_ERROR_ADDR ){
        SMIMSG("Can't find the base address for Larb%d\n", larb);
        return 0;
    }


    pReg = pLarbRegBackUp[larb];

    SMIDBG(1, "+larb_reg_restore(), larb_idx=%d \n", larb); 
    SMI_REGISTER_DBG(2);
    SMIDBG(1, "m4u part restore, larb_idx=%d \n", larb);
    //warning: larb_con is controlled by set/clr
    regval = *(pReg++);
    M4U_WriteReg32(larb_base, SMI_LARB_CON_CLR, ~(regval));
    M4U_WriteReg32(larb_base, SMI_LARB_CON_SET, (regval));

    M4U_WriteReg32(larb_base, SMI_SHARE_EN, *(pReg++) );
    //M4U_WriteReg32(larb_base, SMI_ROUTE_SEL, *(pReg++) );

    for(i=0; i<3; i++)
    {
        M4U_WriteReg32(larb_base, SMI_MAU_ENTR_START(i), *(pReg++));
        M4U_WriteReg32(larb_base, SMI_MAU_ENTR_END(i), *(pReg++));
        M4U_WriteReg32(larb_base, SMI_MAU_ENTR_GID(i), *(pReg++));
    }
    SMI_REGISTER_DBG(2);
    SMIDBG(1, "+smi_larb_init(), larb_idx=%d \n", larb);
    smi_larb_init(larb, force_init);
    SMIDBG(1, "-smi_larb_init(), larb_idx=%d \n", larb);
    SMIDBG(1, "-larb_reg_restore(), larb_idx=%d \n", larb);
    return 0;
}

// callback after larb clock is enabled
void on_larb_power_on(struct larb_monitor *h, int larb_idx)
{
    SMIDBG(1, "on_larb_power_on(), larb_idx=%d \n", larb_idx);
    SMI_REGISTER_DBG(2);
    SMIDBG(1, "+larb_reg_restore(), larb_idx=%d \n", larb_idx);
    larb_reg_restore(larb_idx, 0);

    SMIDBG(1, "-larb_reg_restore(), larb_idx=%d \n", larb_idx); 
    SMI_REGISTER_DBG(2);

    return;
}
// callback before larb clock is disabled
void on_larb_power_off(struct larb_monitor *h, int larb_idx)
{
    SMIDBG(1, "on_larb_power_off(), larb_idx=%d \n", larb_idx);
    SMI_REGISTER_DBG(2);
    SMIDBG(1, "+larb_reg_backup(), larb_idx=%d \n", larb_idx);
    larb_reg_backup(larb_idx);
    SMIDBG(1, "-larb_reg_backup(), larb_idx=%d \n", larb_idx);
    SMI_REGISTER_DBG(2);
}


static void vrSetting(void){
    M4U_WriteReg32(REG_SMI_M4U_TH , 0 , ((0x2 << 15) + (0x3 << 10) + (0x4 << 5) + 0x5));// 2 non-ultra write, 3 write command , 4 non-ultra read , 5 ultra read
    M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0xB);//Level 1 LARB, apply new outstanding control method, 1/4 bandwidth limiter overshoot control , enable warb channel
    M4U_WriteReg32(REG_SMI_READ_FIFO_TH , 0 ,  ((0x6<<11) + (0x8<<6) +0x3F));//total 8 commnads between smi common to M4U, 12 non ultra commands between smi common to M4U, 1 commnads can in write AXI slice for all LARBs

    M4U_WriteReg32(REG_SMI_L1ARB0 , 0 , 0xC26);//1111/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB1 , 0 , 0x943);//503/4096 maximum grant counts, soft limiter        
    M4U_WriteReg32(REG_SMI_L1ARB2 , 0 , 0xD4F);//1359/4096 maximum grant counts, soft limiter

    M4U_WriteReg32(REG_SMI_L1ARB3 , 0 , GPU_SMI_L1_LIMIT_1);
    M4U_WriteReg32(REG_SMI_L1ARB4 , 0 , GPU_SMI_L1_LIMIT_1);
    M4U_WriteReg32(REG_SMI_L1ARB5 , 0 , 0xAA8);//549/4096 maximum grant counts, hard limiter, 2 read 2 write outstanding limit

    M4U_WriteReg32(LARB0_BASE , 0x200 , 0x8); //DISP_OVL_0
    M4U_WriteReg32(LARB0_BASE , 0x204 , 0x8); //DISP_RDMA_1
    M4U_WriteReg32(LARB0_BASE , 0x208 , 0x8); //DISP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x20C , 0x1); //DISP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x210 , 0x1); //MM_CMDQ 
    M4U_WriteReg32(LARB0_BASE , 0x214 , 0x2); //MDP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x218 , 0x2); //MDP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x21C , 0x4); //MDP_ROT
    M4U_WriteReg32(LARB0_BASE , 0x220 , 0x2); //MDP_ROTCO
    M4U_WriteReg32(LARB0_BASE , 0x224 , 0x2); //MDP ROTVO

    M4U_WriteReg32(LARB1_BASE , 0x200 , 0x1); //HW_VDEC_MC_EXT
    M4U_WriteReg32(LARB1_BASE , 0x204 , 0x1); //HW_VDEC_PP_EXT
    M4U_WriteReg32(LARB1_BASE , 0x208 , 0x1); //HW_VDEC_AVC_MV-EXT
    M4U_WriteReg32(LARB1_BASE , 0x20C , 0x1); //HW_VDEC_PRED_RD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x210 , 0x1); //HW_VDEC_PRED_WR_EXT
    M4U_WriteReg32(LARB1_BASE , 0x214 , 0x1); //HW_VDEC_VLD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x218 , 0x1); //HW_VDEC_PP_INT

    M4U_WriteReg32(LARB2_BASE , 0x200 , 0x6);//CAM_IMGO
    M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
    M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
    M4U_WriteReg32(LARB2_BASE , 0x20C , 0x4);//CAM_IMGI
    M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
    M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
    M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
    M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
    M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
    M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
    M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
    M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
    M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
    M4U_WriteReg32(LARB2_BASE , 0x234 , 0x2);//VENC_REC_FRM   
    M4U_WriteReg32(LARB2_BASE , 0x238 , 0x4);//VENC_REF_LUMA  
    M4U_WriteReg32(LARB2_BASE , 0x23C , 0x2);//VENC_REF_CHROMA
    M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
    M4U_WriteReg32(LARB2_BASE , 0x248 , 0x2);//VENC_CUR_LUMA  
    M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA

    M4U_WriteReg32(LARB5_BASE , 0x200 , 0x1);//MJC_MVR
    M4U_WriteReg32(LARB5_BASE , 0x204 , 0x1);//MJC_MVW
    M4U_WriteReg32(LARB5_BASE , 0x208 , 0x1);//MJC_RDMA
    M4U_WriteReg32(LARB5_BASE , 0x20C , 0x1);//MJC_WDMA
}

static void vpSetting(void)
{
    M4U_WriteReg32(REG_SMI_M4U_TH , 0 , ((0x2 << 15) + (0x3 << 10) + (0x4 << 5) + 0x5));// 2 non-ultra write, 3 write command , 4 non-ultra read , 5 ultra read
    M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0x1B);//Level 1 LARB, apply new outstanding control method, 1/4 bandwidth limiter overshoot control , enable warb channel
    M4U_WriteReg32(REG_SMI_READ_FIFO_TH , 0 , 0x323F);//total 8 commnads between smi common to M4U, 12 non ultra commands between smi common to M4U, 1 commnads can in write AXI slice for all LARBs

    M4U_WriteReg32(REG_SMI_L1ARB0 , 0 , 0xC3A);//1111/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB1 , 0 , 0x9E8);//503/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB2 , 0 , 0x943);//353/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB3 , 0 , default_val_smi_l1arb[3]);
    M4U_WriteReg32(REG_SMI_L1ARB4 , 0 , default_val_smi_l1arb[4]);
    M4U_WriteReg32(REG_SMI_L1ARB5 , 0 , 0xAA8);//549/4096 maximum grant counts, hard limiter, 2 read 2 write outstanding limit

    M4U_WriteReg32(LARB0_BASE , 0x200 , 0x8); //DISP_OVL_0
    M4U_WriteReg32(LARB0_BASE , 0x204 , 0x8); //DISP_RDMA_1
    M4U_WriteReg32(LARB0_BASE , 0x208 , 0x8); //DISP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x20C , 0x2); //DISP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x210 , 0x1); //MM_CMDQ 
    M4U_WriteReg32(LARB0_BASE , 0x214 , 0x5); //MDP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x218 , 0x1); //MDP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x21C , 0x3); //MDP_ROT
    M4U_WriteReg32(LARB0_BASE , 0x220 , 0x1); //MDP_ROTCO
    M4U_WriteReg32(LARB0_BASE , 0x224 , 0x1); //MDP ROTVO

    M4U_WriteReg32(LARB1_BASE , 0x200 , 0x6); //HW_VDEC_MC_EXT
    M4U_WriteReg32(LARB1_BASE , 0x204 , 0x2); //HW_VDEC_PP_EXT
    M4U_WriteReg32(LARB1_BASE , 0x208 , 0x1); //HW_VDEC_AVC_MV-EXT
    M4U_WriteReg32(LARB1_BASE , 0x20C , 0x3); //HW_VDEC_PRED_RD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x210 , 0x3); //HW_VDEC_PRED_WR_EXT
    M4U_WriteReg32(LARB1_BASE , 0x214 , 0x1); //HW_VDEC_VLD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x218 , 0x1); //HW_VDEC_PP_INT

    if(wifi_disp_transaction == 1){
        M4U_WriteReg32(LARB2_BASE , 0x200 , 0x6);//CAM_IMGO
        M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
        M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
        M4U_WriteReg32(LARB2_BASE , 0x20C , 0x4);//CAM_IMGI
        M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
        M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
        M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
        M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
        M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
        M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
        M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
        M4U_WriteReg32(LARB2_BASE , 0x234 , 0x1);//VENC_REC_FRM   
        M4U_WriteReg32(LARB2_BASE , 0x238 , 0x2);//VENC_REF_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x23C , 0x1);//VENC_REF_CHROMA
        M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
        M4U_WriteReg32(LARB2_BASE , 0x248 , 0x1);//VENC_CUR_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA    
    }else{
        M4U_WriteReg32(LARB2_BASE , 0x200 , 0x1);//CAM_IMGO
        M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
        M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
        M4U_WriteReg32(LARB2_BASE , 0x20C , 0x1);//CAM_IMGI
        M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
        M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
        M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
        M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
        M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
        M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
        M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
        M4U_WriteReg32(LARB2_BASE , 0x234 , 0x1);//VENC_REC_FRM   
        M4U_WriteReg32(LARB2_BASE , 0x238 , 0x1);//VENC_REF_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x23C , 0x1);//VENC_REF_CHROMA
        M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
        M4U_WriteReg32(LARB2_BASE , 0x248 , 0x1);//VENC_CUR_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA
    }

    M4U_WriteReg32(LARB5_BASE , 0x200 , 0x1);//MJC_MVR
    M4U_WriteReg32(LARB5_BASE , 0x204 , 0x1);//MJC_MVW
    M4U_WriteReg32(LARB5_BASE , 0x208 , 0x7);//MJC_RDMA
    M4U_WriteReg32(LARB5_BASE , 0x20C , 0x5);//MJC_WDMA
}

static void swvpSetting(void)
{
    M4U_WriteReg32(REG_SMI_M4U_TH , 0 , ((0x2 << 15) + (0x3 << 10) + (0x4 << 5) + 0x5));// 2 non-ultra write, 3 write command , 4 non-ultra read , 5 ultra read
    M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0x1B);//Level 1 LARB, apply new outstanding control method, 1/4 bandwidth limiter overshoot control , enable warb channel
    M4U_WriteReg32(REG_SMI_READ_FIFO_TH , 0 , 0x323F);//total 8 commnads between smi common to M4U, 12 non ultra commands between smi common to M4U, 1 commnads can in write AXI slice for all LARBs

    M4U_WriteReg32(REG_SMI_L1ARB0 , 0 , 0xC3A);//1111/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB1 , 0 , 0x9E8);//503/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB2 , 0 , 0x943);//353/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB3 , 0 , default_val_smi_l1arb[3]);
    M4U_WriteReg32(REG_SMI_L1ARB4 , 0 , default_val_smi_l1arb[4]);
    M4U_WriteReg32(REG_SMI_L1ARB5 , 0 , 0xAA8);//549/4096 maximum grant counts, hard limiter, 2 read 2 write outstanding limit

    M4U_WriteReg32(LARB0_BASE , 0x200 , 0x4); //DISP_OVL_0
    M4U_WriteReg32(LARB0_BASE , 0x204 , 0x8); //DISP_RDMA_1
    M4U_WriteReg32(LARB0_BASE , 0x208 , 0x8); //DISP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x20C , 0x1); //DISP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x210 , 0x1); //MM_CMDQ 
    M4U_WriteReg32(LARB0_BASE , 0x214 , 0x1); //MDP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x218 , 0x1); //MDP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x21C , 0x1); //MDP_ROT
    M4U_WriteReg32(LARB0_BASE , 0x220 , 0x1); //MDP_ROTCO
    M4U_WriteReg32(LARB0_BASE , 0x224 , 0x1); //MDP ROTVO

    M4U_WriteReg32(LARB1_BASE , 0x200 , 0x6); //HW_VDEC_MC_EXT
    M4U_WriteReg32(LARB1_BASE , 0x204 , 0x2); //HW_VDEC_PP_EXT
    M4U_WriteReg32(LARB1_BASE , 0x208 , 0x1); //HW_VDEC_AVC_MV-EXT
    M4U_WriteReg32(LARB1_BASE , 0x20C , 0x3); //HW_VDEC_PRED_RD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x210 , 0x3); //HW_VDEC_PRED_WR_EXT
    M4U_WriteReg32(LARB1_BASE , 0x214 , 0x1); //HW_VDEC_VLD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x218 , 0x1); //HW_VDEC_PP_INT

    if(wifi_disp_transaction == 1){
        M4U_WriteReg32(LARB2_BASE , 0x200 , 0x6);//CAM_IMGO
        M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
        M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
        M4U_WriteReg32(LARB2_BASE , 0x20C , 0x4);//CAM_IMGI
        M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
        M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
        M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
        M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
        M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
        M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
        M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
        M4U_WriteReg32(LARB2_BASE , 0x234 , 0x1);//VENC_REC_FRM   
        M4U_WriteReg32(LARB2_BASE , 0x238 , 0x2);//VENC_REF_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x23C , 0x1);//VENC_REF_CHROMA
        M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
        M4U_WriteReg32(LARB2_BASE , 0x248 , 0x1);//VENC_CUR_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA    
    }else{
        M4U_WriteReg32(LARB2_BASE , 0x200 , 0x1);//CAM_IMGO
        M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
        M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
        M4U_WriteReg32(LARB2_BASE , 0x20C , 0x1);//CAM_IMGI
        M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
        M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
        M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
        M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
        M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
        M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
        M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
        M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
        M4U_WriteReg32(LARB2_BASE , 0x234 , 0x1);//VENC_REC_FRM   
        M4U_WriteReg32(LARB2_BASE , 0x238 , 0x1);//VENC_REF_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x23C , 0x1);//VENC_REF_CHROMA
        M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
        M4U_WriteReg32(LARB2_BASE , 0x248 , 0x1);//VENC_CUR_LUMA  
        M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA
    }

    M4U_WriteReg32(LARB5_BASE , 0x200 , 0x1);//MJC_MVR
    M4U_WriteReg32(LARB5_BASE , 0x204 , 0x1);//MJC_MVW
    M4U_WriteReg32(LARB5_BASE , 0x208 , 0x7);//MJC_RDMA
    M4U_WriteReg32(LARB5_BASE , 0x20C , 0x5);//MJC_WDMA
}

static void vencSetting(void)
{
    M4U_WriteReg32(REG_SMI_M4U_TH , 0 , ((0x2 << 15) + (0x3 << 10) + (0x4 << 5) + 0x5));// 2 non-ultra write, 3 write command , 4 non-ultra read , 5 ultra read
    M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0xB);//Level 1 LARB, apply new outstanding control method, 1/4 bandwidth limiter overshoot control , enable warb channel
    M4U_WriteReg32(REG_SMI_READ_FIFO_TH , 0 ,  ((0x6<<11) + (0x8<<6) +0x3F));//total 8 commnads between smi common to M4U, 12 non ultra commands between smi common to M4U, 1 commnads can in write AXI slice for all LARBs

    M4U_WriteReg32(REG_SMI_L1ARB0 , 0 , 0xC26);//1111/4096 maximum grant counts, soft limiter
    M4U_WriteReg32(REG_SMI_L1ARB1 , 0 , 0x9E8);//503/4096 maximum grant counts, soft limiter        
    M4U_WriteReg32(REG_SMI_L1ARB2 , 0 , 0xD4F);//1359/4096 maximum grant counts, soft limiter

    // GPU OSTD limittion: HIGH 
    M4U_WriteReg32(REG_SMI_L1ARB3 , 0 , default_val_smi_l1arb[3]);
    M4U_WriteReg32(REG_SMI_L1ARB4 , 0 , default_val_smi_l1arb[4]);
    M4U_WriteReg32(REG_SMI_L1ARB5 , 0 , 0xAA8);//549/4096 maximum grant counts, hard limiter, 2 read 2 write outstanding limit

    M4U_WriteReg32(LARB0_BASE , 0x200 , 0x8); //DISP_OVL_0
    M4U_WriteReg32(LARB0_BASE , 0x204 , 0x8); //DISP_RDMA_1
    M4U_WriteReg32(LARB0_BASE , 0x208 , 0x8); //DISP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x20C , 0x1); //DISP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x210 , 0x1); //MM_CMDQ 
    M4U_WriteReg32(LARB0_BASE , 0x214 , 0x1); //MDP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x218 , 0x1); //MDP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x21C , 0x1); //MDP_ROT
    M4U_WriteReg32(LARB0_BASE , 0x220 , 0x2); //MDP_ROTCO
    M4U_WriteReg32(LARB0_BASE , 0x224 , 0x1); //MDP ROTVO

    M4U_WriteReg32(LARB1_BASE , 0x200 , 0x3); //HW_VDEC_MC_EXT
    M4U_WriteReg32(LARB1_BASE , 0x204 , 0x1); //HW_VDEC_PP_EXT
    M4U_WriteReg32(LARB1_BASE , 0x208 , 0x1); //HW_VDEC_AVC_MV-EXT
    M4U_WriteReg32(LARB1_BASE , 0x20C , 0x1); //HW_VDEC_PRED_RD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x210 , 0x1); //HW_VDEC_PRED_WR_EXT
    M4U_WriteReg32(LARB1_BASE , 0x214 , 0x1); //HW_VDEC_VLD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x218 , 0x1); //HW_VDEC_PP_INT

    M4U_WriteReg32(LARB2_BASE , 0x200 , 0x6);//CAM_IMGO
    M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
    M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
    M4U_WriteReg32(LARB2_BASE , 0x20C , 0x4);//CAM_IMGI
    M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
    M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
    M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
    M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
    M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
    M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
    M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
    M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
    M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
    M4U_WriteReg32(LARB2_BASE , 0x234 , 0x1);//VENC_REC_FRM   
    M4U_WriteReg32(LARB2_BASE , 0x238 , 0x2);//VENC_REF_LUMA  
    M4U_WriteReg32(LARB2_BASE , 0x23C , 0x1);//VENC_REF_CHROMA
    M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
    M4U_WriteReg32(LARB2_BASE , 0x248 , 0x1);//VENC_CUR_LUMA  
    M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA

    M4U_WriteReg32(LARB5_BASE , 0x200 , 0x1);//MJC_MVR
    M4U_WriteReg32(LARB5_BASE , 0x204 , 0x1);//MJC_MVW
    M4U_WriteReg32(LARB5_BASE , 0x208 , 0x1);//MJC_RDMA
    M4U_WriteReg32(LARB5_BASE , 0x20C , 0x1);//MJC_WDMA
}

//Make sure clock is on
static void initSetting(void)
{
    //SMIMSG("Current Setting: Normal");
    //M4U_WriteReg32(REG_SMI_M4U_TH , 0 , ((0x2 << 15) + (0x3 << 10) + (0x4 << 5) + 0x5));// 2 non-ultra write, 3 write command , 4 non-ultra read , 5 ultra read
    //M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0xB);//Level 1 LARB, apply new outstanding control method, 1/4 bandwidth limiter overshoot control , enable warb channel
    //M4U_WriteReg32(REG_SMI_READ_FIFO_TH , 0 , ((0x6 << 11) + (0x8 << 6) + 0x3F) );//total 8 commnads between smi common to M4U, 12 non ultra commands between smi common to M4U, 1 commnads can in write AXI slice for all LARBs
    hwc_max_pixel_count = SF_HWC_PIXEL_MAX_NORMAL;

    SMIMSG("Current Setting: GPU - new");
    M4U_WriteReg32(REG_SMI_M4U_TH , 0 , ((0x3 << 15) + (0x4 << 10) + (0x4 << 5) + 0x5));// 2 non-ultra write, 3 write command , 4 non-ultra read , 5 ultra read
    M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0xB);//Level 1 LARB, apply new outstanding control method, 1/4 bandwidth limiter overshoot control , enable warb channel
    M4U_WriteReg32(REG_SMI_READ_FIFO_TH , 0 , ((0x7 << 11) + (0x8 << 6) + 0x3F) );//total 8 commnads between smi common to M4U, 12 non ultra commands between smi common to M4U, 1 commnads can in write AXI slice for all LARBs

    if(!is_default_value_saved){
        SMIMSG("Save default config:\n");
        default_val_smi_l1arb[0] = M4U_ReadReg32(REG_SMI_L1ARB0, 0);
        default_val_smi_l1arb[1] = M4U_ReadReg32(REG_SMI_L1ARB1, 0);
        default_val_smi_l1arb[2] = M4U_ReadReg32(REG_SMI_L1ARB2, 0);
        default_val_smi_l1arb[3] = M4U_ReadReg32(REG_SMI_L1ARB3, 0);
        default_val_smi_l1arb[4] = M4U_ReadReg32(REG_SMI_L1ARB4, 0);
        default_val_smi_l1arb[5] = M4U_ReadReg32(REG_SMI_L1ARB5, 0);
        SMIMSG("l1arb[0-2]= 0x%x,  0x%x, 0x%x\n" , default_val_smi_l1arb[0], default_val_smi_l1arb[1], default_val_smi_l1arb[2]);
        SMIMSG("l1arb[3-5]= 0x%x,  0x%x, 0x%x\n" , default_val_smi_l1arb[3], default_val_smi_l1arb[4], default_val_smi_l1arb[5]);

        is_default_value_saved = 1;
    }

    // Keep the HW's init setting in REG_SMI_L1ARB0 ~ REG_SMI_L1ARB5
    M4U_WriteReg32(REG_SMI_L1ARB0 , 0 , default_val_smi_l1arb[0]);
    M4U_WriteReg32(REG_SMI_L1ARB1 , 0 , default_val_smi_l1arb[1]);
    M4U_WriteReg32(REG_SMI_L1ARB2 , 0 , default_val_smi_l1arb[2]);

    // Original - no GPU OSTD limitation
    M4U_WriteReg32(REG_SMI_L1ARB3 , 0 , default_val_smi_l1arb[3]);
    M4U_WriteReg32(REG_SMI_L1ARB4 , 0 , default_val_smi_l1arb[4]);

    M4U_WriteReg32(REG_SMI_L1ARB5 , 0 , default_val_smi_l1arb[5]);

    M4U_WriteReg32(LARB0_BASE , 0x200 , 0x8); //DISP_OVL_0
    M4U_WriteReg32(LARB0_BASE , 0x204 , 0x8); //DISP_RDMA_1
    M4U_WriteReg32(LARB0_BASE , 0x208 , 0x8); //DISP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x20C , 0x2); //DISP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x210 , 0x1); //MM_CMDQ 
    M4U_WriteReg32(LARB0_BASE , 0x214 , 0x5); //MDP_RDMA
    M4U_WriteReg32(LARB0_BASE , 0x218 , 0x1); //MDP_WDMA
    M4U_WriteReg32(LARB0_BASE , 0x21C , 0x3); //MDP_ROT
    M4U_WriteReg32(LARB0_BASE , 0x220 , 0x1); //MDP_ROTCO
    M4U_WriteReg32(LARB0_BASE , 0x224 , 0x1); //MDP ROTVO

    M4U_WriteReg32(LARB1_BASE , 0x200 , 0x1); //HW_VDEC_MC_EXT
    M4U_WriteReg32(LARB1_BASE , 0x204 , 0x1); //HW_VDEC_PP_EXT
    M4U_WriteReg32(LARB1_BASE , 0x208 , 0x1); //HW_VDEC_AVC_MV-EXT
    M4U_WriteReg32(LARB1_BASE , 0x20C , 0x1); //HW_VDEC_PRED_RD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x210 , 0x1); //HW_VDEC_PRED_WR_EXT
    M4U_WriteReg32(LARB1_BASE , 0x214 , 0x1); //HW_VDEC_VLD_EXT
    M4U_WriteReg32(LARB1_BASE , 0x218 , 0x1); //HW_VDEC_PP_INT

    M4U_WriteReg32(LARB2_BASE , 0x200 , 0x1);//CAM_IMGO
    M4U_WriteReg32(LARB2_BASE , 0x204 , 0x1);//CAM_IMG2O
    M4U_WriteReg32(LARB2_BASE , 0x208 , 0x1);//CAM_LSCI
    M4U_WriteReg32(LARB2_BASE , 0x20C , 0x1);//CAM_IMGI
    M4U_WriteReg32(LARB2_BASE , 0x210 , 0x1);//CAM_ESFKO
    M4U_WriteReg32(LARB2_BASE , 0x214 , 0x1);//CAM_AAO
    M4U_WriteReg32(LARB2_BASE , 0x218 , 0x1);//CAM_LCEI
    M4U_WriteReg32(LARB2_BASE , 0x21C , 0x1);//CAM_LCSO
    M4U_WriteReg32(LARB2_BASE , 0x220 , 0x1);//JPGENC_RDMA       
    M4U_WriteReg32(LARB2_BASE , 0x224 , 0x1);//JPGENC_BSDMA      
    M4U_WriteReg32(LARB2_BASE , 0x228 , 0x1);//VENC_SV_COMV   
    M4U_WriteReg32(LARB2_BASE , 0x22C , 0x1);//VENC_RD_COMV   
    M4U_WriteReg32(LARB2_BASE , 0x230 , 0x1);//VENC_RCPU      
    M4U_WriteReg32(LARB2_BASE , 0x234 , 0x1);//VENC_REC_FRM   
    M4U_WriteReg32(LARB2_BASE , 0x238 , 0x1);//VENC_REF_LUMA  
    M4U_WriteReg32(LARB2_BASE , 0x23C , 0x1);//VENC_REF_CHROMA
    M4U_WriteReg32(LARB2_BASE , 0x244 , 0x1);//VENC_BSDMA     
    M4U_WriteReg32(LARB2_BASE , 0x248 , 0x1);//VENC_CUR_LUMA  
    M4U_WriteReg32(LARB2_BASE , 0x24C , 0x1);//VENC_CUR_CHROMA

    M4U_WriteReg32(LARB5_BASE , 0x200 , 0x1);//MJC_MVR
    M4U_WriteReg32(LARB5_BASE , 0x204 , 0x1);//MJC_MVW
    M4U_WriteReg32(LARB5_BASE , 0x208 , 0x1);//MJC_RDMA
    M4U_WriteReg32(LARB5_BASE , 0x20C , 0x1);//MJC_WDMA

}

// DO NOT REMOVE the function, it is used by MHL driver
// Dynamic Adjustment for SMI profiles
void smi_dynamic_adj_hint_mhl(int mhl_enable)
{
    if(mhl_enable == 1){
        mhl_disp_transaction = 1;
        SMIDBG(1, "MHL enabling detected");
    }else{
        mhl_disp_transaction = 0;
        SMIDBG(1, "MHL disabling detected");
    }
    set_sf_pixel_limitation();

}

// DO NOT REMOVE the function, it is used by DISP driver
// Dynamic Adjustment for SMI profiles
void smi_dynamic_adj_hint(unsigned int dsi2smi_total_pixel)
{
    // Since we have OVL decoupling now, we don't need to adjust the bandwidth limitation by frame
    return;

} 

// Set hwc_max_pixel_count according to the current profile
static void set_sf_pixel_limitation(){
    // Since we have OVL decoupling now, we always enable HWC
    hwc_max_pixel_count = SF_HWC_PIXEL_MAX_ALWAYS_HWC;
}

// Fake mode check, e.g. WFD
static int fake_mode_handling(MTK_SMI_BWC_CONFIG* p_conf , unsigned long * pu4LocalCnt){
    if(p_conf->scenario == SMI_BWC_SCEN_WFD){
        if(p_conf->b_on_off){
            wifi_disp_transaction = 1;
            set_sf_pixel_limitation();
            SMIMSG("Enable WFD in profile: %d\n" , smi_profile);
        }else{
            wifi_disp_transaction = 0;
            set_sf_pixel_limitation();
            SMIMSG("Disable WFD in profile: %d\n" , smi_profile);
        }

        return 1;
    }else{
        return 0;
    }
}

static int smi_bwc_config( MTK_SMI_BWC_CONFIG* p_conf , unsigned long * pu4LocalCnt)
{
    int i;
    int result = 0;
    unsigned long u4Concurrency = 0;
    MTK_SMI_BWC_SCEN eFinalScen;
    static MTK_SMI_BWC_SCEN ePreviousFinalScen = SMI_BWC_SCEN_CNT;

    if(smi_tuning_mode == 1){
        SMIMSG("Doesn't change profile in tunning mode");
        return 0;
    }

    spin_lock(&g_SMIInfo.SMI_lock);
    result = fake_mode_handling(p_conf, pu4LocalCnt);
    spin_unlock(&g_SMIInfo.SMI_lock);

    // Fake mode is not a real SMI profile, so we need to return here
    if( result == 1){
        return 0;
    }

    if((SMI_BWC_SCEN_CNT <= p_conf->scenario) || (0 > p_conf->scenario))
    {
        SMIERR("Incorrect SMI BWC config : 0x%x, how could this be...\n" , p_conf->scenario);
        return -1;
    }
    //Debug - S
    //SMIMSG("SMI setTo%d,%s,%d\n" , p_conf->scenario , (p_conf->b_on_off ? "on" : "off") , ePreviousFinalScen);
    //Debug - E

    spin_lock(&g_SMIInfo.SMI_lock);

    if(p_conf->b_on_off)
    {
        //turn on certain scenario
        g_SMIInfo.pu4ConcurrencyTable[p_conf->scenario] += 1;

        if(NULL != pu4LocalCnt)
        {
            pu4LocalCnt[p_conf->scenario] += 1;
        }
    }
    else
    {
        //turn off certain scenario
        if(0 == g_SMIInfo.pu4ConcurrencyTable[p_conf->scenario])
        {
            SMIMSG("Too many turning off for global SMI profile:%d,%d\n" , p_conf->scenario , g_SMIInfo.pu4ConcurrencyTable[p_conf->scenario]);
        }
        else
        {
            g_SMIInfo.pu4ConcurrencyTable[p_conf->scenario] -= 1;
        }

        if(NULL != pu4LocalCnt)
        {
            if(0 == pu4LocalCnt[p_conf->scenario])
            {
                SMIMSG("Process : %s did too many turning off for local SMI profile:%d,%d\n" , current->comm ,p_conf->scenario , pu4LocalCnt[p_conf->scenario]);
            }
            else
            {
                pu4LocalCnt[p_conf->scenario] -= 1;
            }
        }
    }

    for(i=0 ; i < SMI_BWC_SCEN_CNT ; i++)
    {
        if(g_SMIInfo.pu4ConcurrencyTable[i])
        {
            u4Concurrency |= (1 << i);
        }
    }


    if((1 << SMI_BWC_SCEN_MM_GPU) & u4Concurrency)
    {
        eFinalScen = SMI_BWC_SCEN_MM_GPU;
    }
    else if((1 << SMI_BWC_SCEN_VR_SLOW) & u4Concurrency)
    {
        eFinalScen = SMI_BWC_SCEN_VR_SLOW;
    }
    else if((1 << SMI_BWC_SCEN_VR) & u4Concurrency)
    {
        eFinalScen = SMI_BWC_SCEN_VR;
    }
    else if((1 << SMI_BWC_SCEN_VP) & u4Concurrency)
    {
        eFinalScen = SMI_BWC_SCEN_VP;
    }
    else if((1 << SMI_BWC_SCEN_SWDEC_VP) & u4Concurrency)
    {
        eFinalScen = SMI_BWC_SCEN_SWDEC_VP;
    }
    else if((1<< SMI_BWC_SCEN_VENC) & u4Concurrency)
    {
        eFinalScen = SMI_BWC_SCEN_VENC;
    }
    else
    {
        eFinalScen = SMI_BWC_SCEN_NORMAL;
    }

    if(ePreviousFinalScen == eFinalScen)
    {
        SMIMSG("Scen equal%d,don't change\n" , eFinalScen);
        spin_unlock(&g_SMIInfo.SMI_lock);
        return 0;
    }
    else
    {
        ePreviousFinalScen = eFinalScen;
    }

    /*turn on larb clock*/
    for( i=0 ; i < SMI_LARB_NR ; i++){
        larb_clock_on(i);
    }

    /*Bandwidth Limiter*/
    switch( eFinalScen )
    {
    case SMI_BWC_SCEN_VP:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_VP");
        smi_profile = SMI_BWC_SCEN_VP;
        set_sf_pixel_limitation();
        vpSetting();
        break;
    case SMI_BWC_SCEN_SWDEC_VP:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_SCEN_SWDEC_VP");
        smi_profile = SMI_BWC_SCEN_SWDEC_VP;
        set_sf_pixel_limitation();
        swvpSetting();
        break;
    case SMI_BWC_SCEN_VR:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_VR");
        smi_profile = SMI_BWC_SCEN_VR;
        set_sf_pixel_limitation();
        vrSetting();
        break;
    case SMI_BWC_SCEN_VR_SLOW:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_VR");
        smi_profile = SMI_BWC_SCEN_VR_SLOW;
        // Set 30 FPS here!!
        set_sf_pixel_limitation();
        vrSetting();
        break; 
    case SMI_BWC_SCEN_VENC:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_SCEN_VENC");
        smi_profile = SMI_BWC_SCEN_VENC;
        set_sf_pixel_limitation();
        vencSetting();
        break;
    case SMI_BWC_SCEN_NORMAL:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_SCEN_NORMAL");
        smi_profile = SMI_BWC_SCEN_NORMAL;
        set_sf_pixel_limitation();
        initSetting();
        break;
    case SMI_BWC_SCEN_MM_GPU:
        SMIMSG( "[SMI_PROFILE] : %s\n", "SMI_BWC_SCEN_MM_GPU");
        smi_profile = SMI_BWC_SCEN_MM_GPU;
        set_sf_pixel_limitation();
        initSetting();
    default:
        break;
    }
    update_debug_info();

    /*turn off larb clock*/    
    for(i = 0 ; i < SMI_LARB_NR ; i++){
        larb_clock_off(i);
    }

    spin_unlock(&g_SMIInfo.SMI_lock);

    SMIMSG("ScenTo:%d,turn %s,Curr Scen:%d,%d,%d,%d\n" , p_conf->scenario , (p_conf->b_on_off ? "on" : "off") , eFinalScen , 
        g_SMIInfo.pu4ConcurrencyTable[SMI_BWC_SCEN_NORMAL] , g_SMIInfo.pu4ConcurrencyTable[SMI_BWC_SCEN_VR] , g_SMIInfo.pu4ConcurrencyTable[SMI_BWC_SCEN_VP]);

    //Debug usage - S
    //smi_dumpDebugMsg();
    //SMIMSG("Config:%d,%d,%d\n" , eFinalScen , g_SMIInfo.pu4ConcurrencyTable[SMI_BWC_SCEN_NORMAL] , (NULL == pu4LocalCnt ? (-1) : pu4LocalCnt[p_conf->scenario]));
    //Debug usage - E

    return 0;

}



struct larb_monitor larb_monitor_handler =
{
    .level = LARB_MONITOR_LEVEL_HIGH,
    .backup = on_larb_power_off,
    .restore = on_larb_power_on    
};


// Detect the LCD size and emi frequency
static void detect_adj_policy(void)
{
    // EMI CLK detect
    unsigned int clk_read = 0;
    // Resolution Detect
    unsigned int width = 0;
    unsigned int height = 0;

    // Read HW configuration
    clk_read = mt_get_emi_freq();
    width = DISP_GetScreenWidth();
    height = DISP_GetScreenHeight();

    // Policy determination
    if( clk_read >= 300000) {
        // DDR 1333
        if( (width * height) <= (1280 * 720)){
            smi_adj_policy = SMI_ADJ_POLICY_DDR1333_720P;
        }else if ( (width * height) <= (1920 * 1080)){
            smi_adj_policy = SMI_ADJ_POLICY_DDR1333_FHD;
        }else{
            smi_adj_policy = SMI_ADJ_POLICY_DDR1333_WUXGA;
        }
    }else{
        // DDR 1066
        if( (width * height) <= (1280 * 720)){
            smi_adj_policy = SMI_ADJ_POLICY_DDR1066_720P;
        }else if ( (width * height) <= (1920 * 1080)){
            smi_adj_policy = SMI_ADJ_POLICY_DDR1066_FHD;
        }else{
            smi_adj_policy = SMI_ADJ_POLICY_DDR1066_WUXGA;
        }
    }
    SMIMSG("EMI clk = %d, LCD w= %d, h = %d\n", clk_read, width, height);
}

int smi_common_init(void)
{
    int i;

    detect_adj_policy();

    for( i=0 ; i < SMI_LARB_NR ; i++)
    {
        pLarbRegBackUp[i] = (unsigned int*)kmalloc(LARB_BACKUP_REG_SIZE, GFP_KERNEL|__GFP_ZERO);
        if(pLarbRegBackUp[i]==NULL)
        {
            SMIERR("pLarbRegBackUp kmalloc fail %d \n", i);
        }  
    }

    /** make sure all larb power is on before we register callback func.
    then, when larb power is first off, default register value will be backed up.
    **/     

    for( i=0 ; i < SMI_LARB_NR ; i++)
    {
        larb_clock_on(i);
    } 



#ifndef CONFIG_MTK_FPGA
    SMIMSG("Register SMI larb monitor handlers\n");
    register_larb_monitor(&larb_monitor_handler);
#endif


    for( i=0 ; i < SMI_LARB_NR ; i++)
    {
        larb_clock_off(i);
    }

    return 0;
}

static int smi_open(struct inode *inode, struct file *file)
{
    file->private_data = kmalloc(SMI_BWC_SCEN_CNT*sizeof(unsigned long) , GFP_ATOMIC);

    if(NULL == file->private_data)
    {
        SMIMSG("Not enough entry for DDP open operation\n");
        return -ENOMEM;
    }

    memset(file->private_data , 0 , SMI_BWC_SCEN_CNT*sizeof(unsigned long));

    return 0;
}

static int smi_release(struct inode *inode, struct file *file)
{

#if 0
    unsigned long u4Index = 0 ;
    unsigned long u4AssignCnt = 0;
    unsigned long * pu4Cnt = (unsigned long *)file->private_data;
    MTK_SMI_BWC_CONFIG config;

    for(; u4Index < SMI_BWC_SCEN_CNT ; u4Index += 1)
    {
        if(pu4Cnt[u4Index])
        {
            SMIMSG("Process:%s does not turn off BWC properly , force turn off %d\n" , current->comm , u4Index);
            u4AssignCnt = pu4Cnt[u4Index];
            config.b_on_off = 0;
            config.scenario = (MTK_SMI_BWC_SCEN)u4Index;
            do
            {
                smi_bwc_config( &config , pu4Cnt);
            }
            while(0 < u4AssignCnt);
        }
    }
#endif

    if(NULL != file->private_data)
    {
        kfree(file->private_data);
        file->private_data = NULL;
    }

    return 0;
}

static long smi_ioctl( struct file * pFile,
                      unsigned int cmd,
                      unsigned long param)
{
    int ret = 0;

    //    unsigned long * pu4Cnt = (unsigned long *)pFile->private_data;

    switch (cmd)
    {
    case MTK_CONFIG_MM_MAU:
        {
            MTK_MAU_CONFIG b;
            if(copy_from_user(&b, (void __user *)param, sizeof(b)))
            {
                SMIERR("copy_from_user failed!");
                ret = -EFAULT;
            } else {
                mau_config(&b);
            }
            return ret;
        }

#ifdef __MAU_SPC_ENABLE__
    case MTK_IOC_SPC_CONFIG :
        {
            MTK_SPC_CONFIG cfg;
            ret = copy_from_user(&cfg, (void*)param , sizeof(MTK_SPC_CONFIG));
            if(ret)
            {
                SMIMSG(" SPC_CONFIG, copy_from_user failed: %d\n", ret);
                return -EFAULT;
            }  

            spc_config(&cfg);

        }
        break;

    case MTK_IOC_SPC_DUMP_REG :
        spc_dump_reg();
        break;


    case MTK_IOC_SPC_DUMP_STA :
        spc_status_check();        
        break;

    case MTK_IOC_SPC_CMD :
        spc_test(param);
        break;
#endif
    case MTK_IOC_SMI_BWC_REGISTER_SET:
        {
            MTK_SMI_BWC_REGISTER_SET cfg;
            if(smi_tuning_mode != 1){
                SMIMSG("Only support MTK_IOC_SMI_BWC_REGISTER_SET in tuning mode");
                return 0;
            }
            ret = copy_from_user(&cfg, (void*)param , sizeof(MTK_SMI_BWC_REGISTER_SET));
            if(ret)
            {
                SMIMSG(" MTK_IOC_SMI_BWC_REGISTER_SET, copy_to_user failed: %d\n", ret);
                return -EFAULT;
            }  
            // Set the address to the value assigned by user space program
            if(((unsigned int *)cfg.address) != NULL){
                M4U_WriteReg32(cfg.address , 0 , cfg.value);
                SMIMSG("[Tunning] ADDR = 0x%x, VALUE = 0x%x\n", cfg.address, cfg.value);
            }
            break;  
        }
    case MTK_IOC_SMI_BWC_REGISTER_GET:
        {
            MTK_SMI_BWC_REGISTER_GET cfg;
            unsigned int value_read = 0;

            if(smi_tuning_mode != 1){
                SMIMSG("Only support MTK_IOC_SMI_BWC_REGISTER_SET in tuning mode");
                return 0;
            }
            ret = copy_from_user(&cfg, (void*)param , sizeof(MTK_SMI_BWC_REGISTER_GET));

            if(ret)
            {
                SMIMSG(" MTK_IOC_SMI_BWC_REGISTER_GET, copy_to_user failed: %d\n", ret);
                return -EFAULT;
            }  

            value_read = M4U_ReadReg32(cfg.address, 0);

            if(((unsigned int *)cfg.return_address) != NULL){
                ret = copy_to_user((void*)cfg.return_address, (void*)&value_read, sizeof(unsigned int));

                if(ret)
                {
                    SMIMSG(" MTK_IOC_SMI_REGISTER_GET, copy_to_user failed: %d\n", ret);
                    return -EFAULT;
                }
            }
            SMIMSG("[Tunning] ADDR = 0x%x, VALUE = 0x%x\n", cfg.address, value_read);
            break;
        }
    case MTK_IOC_SMI_BWC_STATE:

        {
            MTK_SMI_BWC_STATE cfg;
            ret = copy_from_user(&cfg, (void*)param , sizeof(MTK_IOC_SMI_BWC_STATE));

            if(ret)
            {
                SMIMSG(" MTK_IOC_SMI_BWC_STATE, copy_to_user failed: %d\n", ret);
                return -EFAULT;
            }  

            if(cfg.hwc_max_pixel != NULL){
                ret = copy_to_user((void*)cfg.hwc_max_pixel, (void*)&hwc_max_pixel_count, sizeof(unsigned int));

                if(ret)
                {
                    SMIMSG(" SMI_BWC_CONFIG, copy_to_user failed: %d\n", ret);
                    return -EFAULT;
                }
            }  
            SMIDBG(4, "HWC MAX=%d \n", hwc_max_pixel_count);
        }
        break;
    case MTK_IOC_SMI_BWC_CONFIG:
        {
            MTK_SMI_BWC_CONFIG cfg;
            ret = copy_from_user(&cfg, (void*)param , sizeof(MTK_SMI_BWC_CONFIG));
            if(ret)
            {
                SMIMSG(" SMI_BWC_CONFIG, copy_from_user failed: %d\n", ret);
                return -EFAULT;
            }  

            //                ret = smi_bwc_config( &cfg , pu4Cnt);
            ret = smi_bwc_config( &cfg , NULL);

        }
        break;

    default:
        return -1;
    }

    return ret;
}


static const struct file_operations smiFops =
{
    .owner = THIS_MODULE,
    .open = smi_open,
    .release = smi_release,
    .unlocked_ioctl = smi_ioctl,
};

static struct cdev * pSmiDev = NULL;
static dev_t smiDevNo = MKDEV(MTK_SMI_MAJOR_NUMBER,0);
static inline int smi_register(void)
{
    if (alloc_chrdev_region(&smiDevNo, 0, 1,"MTK_SMI")){
        SMIERR("Allocate device No. failed");
        return -EAGAIN;
    }
    //Allocate driver
    pSmiDev = cdev_alloc();

    if (NULL == pSmiDev) {
        unregister_chrdev_region(smiDevNo, 1);
        SMIERR("Allocate mem for kobject failed");
        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(pSmiDev, &smiFops);
    pSmiDev->owner = THIS_MODULE;

    //Add to system
    if (cdev_add(pSmiDev, smiDevNo, 1)) {
        SMIERR("Attatch file operation failed");
        unregister_chrdev_region(smiDevNo, 1);
        return -EAGAIN;
    }
    return 0;
}


static struct class *pSmiClass = NULL;
static int smi_probe(struct platform_device *pdev)
{
    struct device* smiDevice = NULL;

    if (NULL == pdev) {
        SMIERR("platform data missed");
        return -ENXIO;
    }

    if (smi_register()) {
        dev_err(&pdev->dev,"register char failed\n");
        return -EAGAIN;
    }

    pSmiClass = class_create(THIS_MODULE, "MTK_SMI");
    if (IS_ERR(pSmiClass)) {
        int ret = PTR_ERR(pSmiClass);
        SMIERR("Unable to create class, err = %d", ret);
        return ret;
    }
    smiDevice = device_create(pSmiClass, NULL, smiDevNo, NULL, "MTK_SMI");

    smi_common_init();

    mau_init();

#ifdef __MAU_SPC_ENABLE__

    MTK_SPC_Init(&(pdev->dev));

#endif

    SMI_DBG_Init();
#if 0
    //init mau to monitor mva 0~0x2ffff & 0x40000000~0xffffffff
    {
        MTK_MAU_CONFIG cfg;
        int i;
        for( i=0 ; i < SMI_LARB_NR ; i++)
        {
            cfg.larb = i;
            cfg.entry = 0;
            cfg.port_msk = 0xffffffff;
            cfg.virt = 1;
            cfg.monitor_read = 1;
            cfg.monitor_write = 1;
            cfg.start = 0;
            cfg.end = 0x2ffff;
            mau_config(&cfg);

            cfg.entry = 1;
            cfg.start = 0x40000000;
            cfg.end = 0xffffffff;
            mau_config(&cfg);
        }
    }
#endif

    return 0;
}



static int smi_remove(struct platform_device *pdev)
{
    cdev_del(pSmiDev);
    unregister_chrdev_region(smiDevNo, 1);
    device_destroy(pSmiClass, smiDevNo);
    class_destroy(pSmiClass);
    return 0;
}


static int smi_suspend(struct device *device)
{
    return 0;
}

static int smi_resume(struct device *device)
{
    return 0;
}


int SMI_common_pm_restore_noirq(struct device *device)
{

    printk("SMI_common_pm_restore_noirq, do nothing\n");
    return 0;
}

struct dev_pm_ops SMI_common_helper_pm_ops = {
    .suspend  = smi_suspend,
    .resume   = smi_resume,
    .restore_noirq = SMI_common_pm_restore_noirq,
};


static struct platform_driver smiDrv = {
    .probe     = smi_probe,
    .remove  = smi_remove,
    .driver     = {
        .name      = "MTK_SMI",
#ifdef CONFIG_PM
        .pm     = &SMI_common_helper_pm_ops,
#endif
        .owner     = THIS_MODULE,
    }
};


// Earily suspend call backs ---------------------------------
#ifdef CONFIG_HAS_EARLYSUSPEND
static void SMI_common_early_suspend(struct early_suspend *h)
{
    SMIMSG("[%s]\n", __func__);
    return;
}

static void SMI_common_late_resume(struct early_suspend *h)
{
    // This is a workaround for IPOH block issue. Somebody write REG_SMI_L1LEN
    // before SMI late_resume, need to fix it as soon as possible 
    SMIMSG("[%s] reinit REG_SMI_L1LEN reg.\n", __func__);
    M4U_WriteReg32(REG_SMI_L1LEN , 0 , 0x1B);
    return;
}

static struct early_suspend mt_SMI_common_early_suspend_handler =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
    .suspend = SMI_common_early_suspend,
    .resume  = SMI_common_late_resume,
};
#endif //CONFIG_HAS_EARLYSUSPEND
//-------------------------------------------------------------


static int __init smi_init(void)
{
    spin_lock_init(&g_SMIInfo.SMI_lock);

    memset(g_SMIInfo.pu4ConcurrencyTable , 0 , SMI_BWC_SCEN_CNT*sizeof(unsigned long));

    if(platform_driver_register(&smiDrv)){
        SMIERR("failed to register MAU driver");
        return -ENODEV;
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&mt_SMI_common_early_suspend_handler);
#endif

    return 0;
}

static void __exit smi_exit(void)
{
    platform_driver_unregister(&smiDrv);

}

void smi_dumpDebugMsg(void)
{
    unsigned int u4Index;
    unsigned int u4Base;
    unsigned int u4Offset;

    //SMI COMMON dump

    SMIMSG("===EMI related reg dump===\n");
    SMIMSG("+0x%x=0x%x \n" , 0xF0203140 , M4U_ReadReg32(0xF0203140 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF0203144 , M4U_ReadReg32(0xF0203144 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF0203078 , M4U_ReadReg32(0xF0203078 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF02030D0 , M4U_ReadReg32(0xF02030D0 , 0));

    SMIMSG("+0x%x=0x%x \n" , 0xF0203060 , M4U_ReadReg32(0xF0203060 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF02030E8 , M4U_ReadReg32(0xF02030E8 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF0203158 , M4U_ReadReg32(0xF0203158 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF0203150 , M4U_ReadReg32(0xF0203150 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF0203154 , M4U_ReadReg32(0xF0203154 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF0203148 , M4U_ReadReg32(0xF0203148 , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF020314C , M4U_ReadReg32(0xF020314C , 0));
    SMIMSG("+0x%x=0x%x \n" , 0xF020314C , M4U_ReadReg32(0xF010180c , 0));
    SMIMSG("===SMI common reg dump===\n");

    u4Base = SMI_COMMON_EXT_BASE;
    u4Offset = 0x400;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x404;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));

    u4Offset = 0x234;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x200;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x204;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x208;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x20C;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x210;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x214;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x218;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x230;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));

    // For VA and PA check:
    // 0x1000C5C0 , 0x1000C5C4, 0x1000C5C8, 0x1000C5CC, 0x1000C5D0

    u4Base = SMI_COMMON_AO_BASE;
    u4Offset = 0x5C0;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x5C4;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x5C8;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x5CC;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
    u4Offset = 0x5D0;
    SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));



    //SMI LARB dump
    for( u4Index=0 ; u4Index < SMI_LARB_NR ; u4Index++)
    {
        if(0 == u4Index)
        {
            //            if((0 == clock_is_on(MT_CG_DISP0_SMI_LARB0)) || (0 == clock_is_on(MT_CG_DISP0_SMI_COMMON)))
            if(0x3 & M4U_ReadReg32(0xF4000000 , 0x100))
            {
                SMIMSG("===SMI%d is off===\n" , u4Index);
                continue;
            }
        }
        else if(1 == u4Index)
        {
            //            if(0 == clock_is_on(MT_CG_VDEC1_LARB))
            if(0x1 & M4U_ReadReg32(0xF6000000 , 0x4))
            {
                SMIMSG("===SMI%d is off===\n" , u4Index);
                continue;
            }
        }
        else if(2 == u4Index)
        {
            //            if(0 == clock_is_on(MT_CG_IMAGE_LARB2_SMI))
            if(0x1 & M4U_ReadReg32(0xF5000000 , 0))
            {
                SMIMSG("===SMI%d is off===\n" , u4Index);
                continue;
            }
        }

        u4Base = get_larb_base_addr(u4Index);
        if(u4Base == SMI_ERROR_ADDR )
        {
            SMIMSG("Didn't support reg dump for Larb%d\n", u4Index);
            continue;
        }else{
            SMIMSG("===SMI%d reg dump===\n" , u4Index);    
            u4Offset = 0;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x10;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));

            // Add 0x60, 0x64, 0x8c
            u4Offset = 0x60;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x64;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x8c;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));

            u4Offset = 0x450;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x454;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x600;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x604;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x610;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            u4Offset = 0x614;
            SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            for(u4Offset = 0x200; u4Offset < 0x244 ; u4Offset += 4)
            {
                SMIMSG("+0x%x=0x%x \n" , u4Offset , M4U_ReadReg32(u4Base , u4Offset));
            }
        }
    }

}

module_init(smi_init);
module_exit(smi_exit);

// For user space debugging

module_param_named(tuning_mode, smi_tuning_mode, uint, S_IRUGO | S_IWUSR);

module_param_named(wifi_disp_transaction, wifi_disp_transaction, uint, S_IRUGO | S_IWUSR);

module_param_named(debug_level, smi_debug_level, uint, S_IRUGO | S_IWUSR);

module_param_named(pixel_count, smi_pixel_count, uint, S_IRUSR);

module_param_named(hwc_max_pixel_count, hwc_max_pixel_count, uint, S_IRUGO | S_IWUSR);

module_param_named(smi_profile, smi_profile, uint, S_IRUSR);  

module_param_named(smi_adj_policy, smi_adj_policy, uint, S_IRUSR);  

module_param_array_named(smi_common_fifo, smi_common_regs, uint, &smi_common_regs_size,
                         S_IRUSR);
module_param_array_named(smi_common_limiters, smi_common_larb_regs, uint, &smi_common_larb_regs_size,
                         S_IRUSR);

MODULE_DESCRIPTION("MTK SMI driver");
MODULE_AUTHOR("K_zhang<k.zhang@mediatek.com>");
MODULE_LICENSE("GPL");

