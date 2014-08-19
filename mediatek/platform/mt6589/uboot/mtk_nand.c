#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/errno.h>
#include "cust_nand.h"
#include <asm/io.h>
#include <asm/arch/mtk_nand.h>
#include "nand_device_list.h"
#include "mt65xx_partition.h"
#include <asm/arch/bmt.h>
//#include "nand_customer.h"
#include "partition_define.h"

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE (2)

#if defined(CONFIG_CMD_NAND)

#ifdef CFG_NAND_LEGACY
#include <linux/mtd/nand_legacy.h>
#else
//#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#endif

#define NAND_SECTOR_SIZE            (512)
#define NAND_SPARE_PER_SECTOR       (16)
#define CONFIG_MTD_NAND_MTK         1

#define CONFIG_SYS_NAND_QUIET_TEST	1
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_MTD_DEVICE           1

#define IO_WIDTH_8                  8
#define IO_WIDTH_16                 16

/*******************************************************************************
 * Macro definition 
 *******************************************************************************/

#define NFI_SET_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) | (value)))
#define NFI_SET_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) | (value)))
#define NFI_CLN_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) & (~(value))))
#define NFI_CLN_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) & (~(value))))

#define NFI_WAIT_STATE_DONE(state) do{;}while (__raw_readl(NFI_STA_REG32) & state)
#define NFI_WAIT_TO_READY()  do{;}while (!(__raw_readl(NFI_STA_REG32) & STA_BUSY2READY))

#define FIFO_PIO_READY(x)  (0x1 & x)
#define WAIT_NFI_PIO_READY(timeout) \
do {\
   while( (!FIFO_PIO_READY(DRV_Reg(NFI_PIO_DIRDY_REG16))) && (--timeout) );\
   if(timeout == 0)\
   {\
        MSG(ERR, "Error: FIFO_PIO_READY timeout at line=%d, file =%s\n", __LINE__, __FILE__);\
   }\
} while(0);

#define TIMEOUT_1   0x1fff
#define TIMEOUT_2   0x8ff
#define TIMEOUT_3   0xffff
#define TIMEOUT_4   5000        //PIO

static struct nand_ecclayout nand_oob_16 = {
    .eccbytes = 8,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
    .oobfree = {{1, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
    .eccbytes = 32,
    .eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
               40, 41, 42, 43, 44, 45, 46, 47,
               48, 49, 50, 51, 52, 53, 54, 55,
               56, 57, 58, 59, 60, 61, 62, 63},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
    .eccbytes = 64,
    .eccpos = {
               64, 65, 66, 67, 68, 69, 70, 71,
               72, 73, 74, 75, 76, 77, 78, 79,
               80, 81, 82, 83, 84, 85, 86, 86,
               88, 89, 90, 91, 92, 93, 94, 95,
               96, 97, 98, 99, 100, 101, 102, 103,
               104, 105, 106, 107, 108, 109, 110, 111,
               112, 113, 114, 115, 116, 117, 118, 119,
               120, 121, 122, 123, 124, 125, 126, 127},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

static bmt_struct *g_bmt = NULL;

#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
   do { \
      DRV_WriteReg(NFI_CMD_REG16,cmd);\
      while (DRV_Reg32(NFI_STA_REG32) & STA_CMD_STATE);\
      DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);\
      DRV_WriteReg32(NFI_ROWADDR_REG32, row_addr);\
      DRV_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
      while (DRV_Reg32(NFI_STA_REG32) & STA_ADDR_STATE);\
   }while(0);

#ifdef CFG_NAND_LEGACY
void nand_init(void)
{

}
#else

flashdev_info devinfo;

#define CHIPVER_ECO_1 (0x8a00)
#define CHIPVER_ECO_2 (0x8a01)

struct NAND_CMD g_kCMD;
u32 total_size;
static u32 g_u4ChipVer;
static u32 g_i4ErrNum;
static bool g_bInitDone;
bool get_device_info(u16 id, u32 ext_id, flashdev_info * pdevinfo)
{
    u32 index;
    for (index = 0; gen_FlashTable[index].id != 0; index++)
    {
        if (id == gen_FlashTable[index].id && ext_id == gen_FlashTable[index].ext_id)
        {
            pdevinfo->id = gen_FlashTable[index].id;
            pdevinfo->ext_id = gen_FlashTable[index].ext_id;
            pdevinfo->blocksize = gen_FlashTable[index].blocksize;
            pdevinfo->addr_cycle = gen_FlashTable[index].addr_cycle;
            pdevinfo->iowidth = gen_FlashTable[index].iowidth;
            pdevinfo->timmingsetting = gen_FlashTable[index].timmingsetting;
            pdevinfo->advancedmode = gen_FlashTable[index].advancedmode;
            pdevinfo->pagesize = gen_FlashTable[index].pagesize;
            pdevinfo->totalsize = gen_FlashTable[index].totalsize;
            memcpy(pdevinfo->devciename, gen_FlashTable[index].devciename, sizeof(pdevinfo->devciename));
            MSG(INIT, "MTK Table, ID: %x, EXT_ID: %x\n", id, ext_id);

            goto find;
        }
    }

  find:
    if (0 == pdevinfo->id)
    {
        MSG(INIT, "ID: %x not found\n", id);
        return false;
    } else
    {
        return true;
    }
}
void dump_nfi(void)
{
    printk("~~~~Dump NFI Register~~~~\n");
    printk("NFI_CNFG_REG16: 0x%x\n", DRV_Reg16(NFI_CNFG_REG16));
    printk("NFI_PAGEFMT_REG16: 0x%x\n", DRV_Reg16(NFI_PAGEFMT_REG16));
    printk("NFI_CON_REG16: 0x%x\n", DRV_Reg16(NFI_CON_REG16));
    printk("NFI_ACCCON_REG32: 0x%x\n", DRV_Reg32(NFI_ACCCON_REG32));
    printk("NFI_INTR_EN_REG16: 0x%x\n", DRV_Reg16(NFI_INTR_EN_REG16));
    printk("NFI_INTR_REG16: 0x%x\n", DRV_Reg16(NFI_INTR_REG16));
    printk("NFI_CMD_REG16: 0x%x\n", DRV_Reg16(NFI_CMD_REG16));
    printk("NFI_ADDRNOB_REG16: 0x%x\n", DRV_Reg16(NFI_ADDRNOB_REG16));
    printk("NFI_COLADDR_REG32: 0x%x\n", DRV_Reg32(NFI_COLADDR_REG32));
    printk("NFI_ROWADDR_REG32: 0x%x\n", DRV_Reg32(NFI_ROWADDR_REG32));
    printk("NFI_STRDATA_REG16: 0x%x\n", DRV_Reg16(NFI_STRDATA_REG16));
    printk("NFI_DATAW_REG32: 0x%x\n", DRV_Reg32(NFI_DATAW_REG32));
    printk("NFI_DATAR_REG32: 0x%x\n", DRV_Reg32(NFI_DATAR_REG32));
    printk("NFI_PIO_DIRDY_REG16: 0x%x\n", DRV_Reg16(NFI_PIO_DIRDY_REG16));
    printk("NFI_STA_REG32: 0x%x\n", DRV_Reg32(NFI_STA_REG32));
    printk("NFI_FIFOSTA_REG16: 0x%x\n", DRV_Reg16(NFI_FIFOSTA_REG16));
    printk("NFI_LOCKSTA_REG16: 0x%x\n", DRV_Reg16(NFI_LOCKSTA_REG16));
    printk("NFI_ADDRCNTR_REG16: 0x%x\n", DRV_Reg16(NFI_ADDRCNTR_REG16));
    printk("NFI_STRADDR_REG32: 0x%x\n", DRV_Reg32(NFI_STRADDR_REG32));
    printk("NFI_BYTELEN_REG16: 0x%x\n", DRV_Reg16(NFI_BYTELEN_REG16));
    printk("NFI_CSEL_REG16: 0x%x\n", DRV_Reg16(NFI_CSEL_REG16));
    printk("NFI_IOCON_REG16: 0x%x\n", DRV_Reg16(NFI_IOCON_REG16));
    printk("NFI_FDM0L_REG32: 0x%x\n", DRV_Reg32(NFI_FDM0L_REG32));
    printk("NFI_FDM0M_REG32: 0x%x\n", DRV_Reg32(NFI_FDM0M_REG32));
    printk("NFI_LOCK_REG16: 0x%x\n", DRV_Reg16(NFI_LOCK_REG16));
    printk("NFI_LOCKCON_REG32: 0x%x\n", DRV_Reg32(NFI_LOCKCON_REG32));
    printk("NFI_LOCKANOB_REG16: 0x%x\n", DRV_Reg16(NFI_LOCKANOB_REG16));
    printk("NFI_FIFODATA0_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA0_REG32));
    printk("NFI_FIFODATA1_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA1_REG32));
    printk("NFI_FIFODATA2_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA2_REG32));
    printk("NFI_FIFODATA3_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA3_REG32));
    printk("NFI_MASTERSTA_REG16: 0x%x\n", DRV_Reg16(NFI_MASTERSTA_REG16));
    printk("NFI clock register: 0xF1000010: %s\n", (DRV_Reg32((volatile u32 *)0xF1000010) & (0x1)) ? "Clock Disabled" : "Clock Enabled");
    printk("NFI clock:0xF100002C: %s\n", (DRV_Reg32((volatile u32 *)0xF100002C) & (0x1)) ? "133MHz" : "66.5MHz");

}
//-------------------------------------------------------------------------------
static void ECC_Config(void)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;

    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
    do
    {;
    }
    while (!DRV_Reg16(ECC_DECIDLE_REG16));

    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
    do
    {;
    }
    while (!DRV_Reg32(ECC_ENCIDLE_REG32));

    /* setup FDM register base */
    DRV_WriteReg32(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

    u4ENCODESize = (NAND_SECTOR_SIZE + 8) << 3;
    u4DECODESize = ((NAND_SECTOR_SIZE + 8) << 3) + 4 * 13;

    /* configure ECC decoder && encoder */

    DRV_WriteReg32(ECC_DECCNFG_REG32, ECC_CNFG_ECC4 | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));

    DRV_WriteReg32(ECC_ENCCNFG_REG32, ECC_CNFG_ECC4 | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));

#ifndef MANUAL_CORRECT
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif
}

//-------------------------------------------------------------------------------
static void ECC_Decode_Start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}

//-------------------------------------------------------------------------------
static void ECC_Decode_End(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

//-------------------------------------------------------------------------------
static void ECC_Encode_Start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

//-------------------------------------------------------------------------------
static void ECC_Encode_End(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

//-------------------------------------------------------------------------------
static bool mtk_nand_check_bch_error(u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
    bool bRet = true;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug0,u4ErrorNumDebug1, i, u4ErrNum;
    u32 timeout = 0xFFFF;

#ifdef MANUAL_CORRECT
    u32 au4ErrBitLoc[6];
    u32 u4ErrByteLoc, u4BitOffset;
    u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

    while (0 == (u2SectorDoneMask & DRV_Reg16(ECC_DECDONE_REG16)))
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }

#ifndef MANUAL_CORRECT
    u4ErrorNumDebug0 = DRV_Reg32(ECC_DECENUM0_REG32);
	u4ErrorNumDebug1=DRV_Reg32(ECC_DECENUM1_REG32);
    if (0 != (u4ErrorNumDebug0 & 0xFFFFF) ||0 != (u4ErrorNumDebug1 & 0xFFFFF) )
    {
        for (i = 0; i <= u4SecIndex; ++i)
        {
        	if(i<4){
                 u4ErrNum = DRV_Reg32(ECC_DECENUM0_REG32) >> (i *5);
        	}else{
        	    u4ErrNum = DRV_Reg32(ECC_DECENUM1_REG32) >> ((i-4) *5);
        	}
            u4ErrNum &= 0x1F;
            if (0x1F == u4ErrNum)
            {
                MSG(ERR, "In Uboot UnCorrectable at PageAddr=%d, Sector=%d\n", u4PageAddr, i);
                bRet = false;
            } else
            {
                MSG(ERR, " In Uboot Correct %d at PageAddr=%d, Sector=%d\n", u4ErrNum, u4PageAddr, i);
            }
        }
    }
#else
    memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug1 = DRV_Reg32(ECC_DECENUM1_REG32);
    u4ErrNum = DRV_Reg32(ECC_DECENUM1_REG32) >>( (u4SecIndex -4)*5);
    u4ErrNum &= 0x1F;
    if (u4ErrNum)
    {
        if (0x1F == u4ErrNum)
        {
            MSG(ERR, "UnCorrectable at PageAddr=%d\n", u4PageAddr);
            bRet = false;
        } else
        {
            for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i)
            {
                au4ErrBitLoc[i] = DRV_Reg32(ECC_DECEL0_REG32 + i);
                u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
                if (u4ErrBitLoc1th < 0x1000)
                {
                    u4ErrByteLoc = u4ErrBitLoc1th / 8;
                    u4BitOffset = u4ErrBitLoc1th % 8;
                    pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                } else
                {
                    MSG(ERR, "UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
                }

                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    } else
                    {
                        MSG(ERR, "UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
                    }
                }
            }
            bRet = true;
        }

        if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            bRet = false;
        }
    }
#endif

    return bRet;
}

//-------------------------------------------------------------------------------
static bool mtk_nand_RFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("mtk_nand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return false;
            }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------
static bool mtk_nand_WFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("mtk_nand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return false;
            }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------
static bool mtk_nand_status_ready(u32 u4Status)
{
    u32 timeout = 0xFFFF;
    while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    return true;
}

//-------------------------------------------------------------------------------
static bool mtk_nand_reset(void)
{
    int timeout = 0xFFFF;
    if (DRV_Reg16(NFI_MASTERSTA_REG16)) // master is busy
    {
        DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
        while (DRV_Reg16(NFI_MASTERSTA_REG16))
        {
            timeout--;
            if (!timeout)
            {
                MSG(FUC, "Wait for NFI_MASTERSTA timeout\n");
            }
        }
    }
    /* issue reset operation */
    DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

    return mtk_nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && mtk_nand_RFIFOValidSize(0) && mtk_nand_WFIFOValidSize(0);
}

//-------------------------------------------------------------------------------
static void mtk_nand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

//-------------------------------------------------------------------------------
static void mtk_nand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

//-------------------------------------------------------------------------------
static void mtk_nand_configure_fdm(u16 u2FDMSize)
{
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

//-------------------------------------------------------------------------------
static void mtk_nand_configure_lock(void)
{
    u32 u4WriteColNOB = 2;
    u32 u4WriteRowNOB = 3;
    u32 u4EraseColNOB = 0;
    u32 u4EraseRowNOB = 3;
    DRV_WriteReg16(NFI_LOCKANOB_REG16,
                   (u4WriteColNOB << PROG_CADD_NOB_SHIFT) | (u4WriteRowNOB << PROG_RADD_NOB_SHIFT) | (u4EraseColNOB << ERASE_CADD_NOB_SHIFT) | (u4EraseRowNOB << ERASE_RADD_NOB_SHIFT));

    // Workaround method for ECO1 mt6577    
    if (CHIPVER_ECO_1 == g_u4ChipVer)
    {
        int i;
        for (i = 0; i < 16; ++i)
        {
            DRV_WriteReg32(NFI_LOCK00ADD_REG32 + (i << 1), 0xFFFFFFFF);
            DRV_WriteReg32(NFI_LOCK00FMT_REG32 + (i << 1), 0xFFFFFFFF);
        }
        //DRV_WriteReg16(NFI_LOCKANOB_REG16, 0x0);
        DRV_WriteReg32(NFI_LOCKCON_REG32, 0xFFFFFFFF);
        DRV_WriteReg16(NFI_LOCK_REG16, NFI_LOCK_ON);
    }
}

//-------------------------------------------------------------------------------
static bool mtk_nand_set_command(u16 command)
{
    /* Write command to device */
    DRV_WriteReg16(NFI_CMD_REG16, command);
    return mtk_nand_status_ready(STA_CMD_STATE);
}

//-------------------------------------------------------------------------------
static bool mtk_nand_set_address(u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB, u16 u2RowNOB)
{
    /* fill cycle addr */
    DRV_WriteReg32(NFI_COLADDR_REG32, u4ColAddr);
    DRV_WriteReg32(NFI_ROWADDR_REG32, u4RowAddr);
    DRV_WriteReg16(NFI_ADDRNOB_REG16, u2ColNOB | (u2RowNOB << ADDR_ROW_NOB_SHIFT));
    return mtk_nand_status_ready(STA_ADDR_STATE);
}

//-------------------------------------------------------------------------------
static bool mtk_nand_check_RW_count(u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> 9;
    while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    return true;
}

//-------------------------------------------------------------------------------
static bool mtk_nand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, bool bFull, u8 * buf)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    bool bRet = false;
    u16 sec_num = 1 << (nand->page_shift - 9);
    u32 col_addr = u4ColAddr;
    if (nand->options & NAND_BUSWIDTH_16)
        col_addr >>= 1;
    u32 colnob = 2, rownob = devinfo.addr_cycle - 2;

    if (!mtk_nand_reset())
    {
        goto cleanup;
    }

    /* Enable HW ECC */
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    mtk_nand_set_mode(CNFG_OP_READ);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    if (bFull)
    {
#if USE_AHB_MODE
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
#else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif
        DRV_WriteReg32(NFI_STRADDR_REG32, buf);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    mtk_nand_set_autoformat(bFull);
    if (bFull)
        ECC_Decode_Start();

    if (!mtk_nand_set_command(NAND_CMD_READ0))
    {
        goto cleanup;
    }

    if (!mtk_nand_set_address(col_addr, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!mtk_nand_set_command(NAND_CMD_READSTART))
    {
        goto cleanup;
    }

    if (!mtk_nand_status_ready(STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = true;

  cleanup:
    return bRet;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u8 * buf)
{
    bool bRet = false;
    u16 sec_num = 1 << (nand->page_shift - 9);
    u32 colnob = 2, rownob = devinfo.addr_cycle - 2;
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    if (!mtk_nand_reset())
    {
        return false;
    }

    mtk_nand_set_mode(CNFG_OP_PRGM);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    mtk_nand_set_autoformat(true);

    ECC_Encode_Start();

    if (!mtk_nand_set_command(NAND_CMD_SEQIN))
    {
        goto cleanup;
    }

    if (!mtk_nand_set_address(0, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!mtk_nand_status_ready(STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = true;
  cleanup:

    return bRet;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_dma_transfer_data(u8 * pDataBuf, u32 u4Size)
{
    u32 timeout = 0xFFFF;
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }

    timeout = 0xFFFF;
    while ((u4Size >> 9) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12))
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }

    return true;
}

static bool mtk_nand_mcu_transfer_data(u8 * pDataBuf, u32 length)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32;

    if (length % 4)
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);
    pBuf32 = (u32 *) pDataBuf;

    if (length % 4)
    {
        for (i = 0; (i < length) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                *pDataBuf++ = DRV_Reg32(NFI_DATAR_REG32);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                return false;
            }
        }
    } else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                *pBuf32++ = DRV_Reg32(NFI_DATAR_REG32);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                return false;
            }
        }
    }
    return true;
}

static bool mtk_nand_read_page_data(u8 * buf, u32 length)
{
#if USE_AHB_MODE
    return mtk_nand_dma_transfer_data(buf, length);
#else
    return mtk_nand_mcu_transfer_data(buf, length);
#endif
}

//-----------------------------------------------------------------------------
static bool mtk_nand_dma_write_data(u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

    if ((unsigned int)buf % 16)
    {
        //printf("Un-16-aligned address\n");
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    } else
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
    {
        timeout--;
        if (0 == timeout)
        {
            printf("wait write AHB done timeout\n");
            dump_nfi();
            return FALSE;
        }
    }
    return true;
}

static bool mtk_nand_mcu_write_data(const u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32 = (u32 *) buf;
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);

    if ((u32) buf % 4 || length % 4)
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    if ((u32) buf % 4 || length % 4)
    {
        for (i = 0; (i < (length)) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                DRV_WriteReg32(NFI_DATAW_REG32, *buf++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printk("[%s] nand mcu write timeout\n", __FUNCTION__);
                dump_nfi();
                return false;
            }
        }
    } else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                DRV_WriteReg32(NFI_DATAW_REG32, *pBuf32++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printk("[%s] nand mcu write timeout\n", __FUNCTION__);
                dump_nfi();
                return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_write_page_data(u8 * buf, u32 length)
{
#if USE_AHB_MODE
    return mtk_nand_dma_write_data(buf, length);
#else
    return mtk_nand_mcu_write_data(buf, length);
#endif
}

static void mtk_nand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        *pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i << 1));
        *pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i << 1));
    }
}

//-----------------------------------------------------------------------------
static void mtk_nand_write_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        DRV_WriteReg32(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
        DRV_WriteReg32(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
    }
}

//-----------------------------------------------------------------------------
static void mtk_nand_stop_read(void)
{
    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
    ECC_Decode_End();
}

//-----------------------------------------------------------------------------
static void mtk_nand_stop_write(void)
{
    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
    ECC_Encode_End();
}

//-----------------------------------------------------------------------------

static bool mtk_nand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;
    while ((dec_mask != DRV_Reg(ECC_DECDONE_REG16)) && timeout > 0)
        timeout--;
    if (timeout == 0)
    {
        dump_nfi();
        MSG(ERR, "ECC_DECDONE: timeout\n");
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------

int mtk_nand_exec_read_page_hw(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = true;
    u32 u4SecNum = u4PageSize >> 9;

    if (mtk_nand_ready_for_read(nand, u4RowAddr, 0, true, pPageBuf))
    {
        if (!mtk_nand_read_page_data(pPageBuf, u4PageSize))
        {
            dump_nfi();
            bRet = false;
        }
        if (!mtk_nand_status_ready(STA_NAND_BUSY))
        {
            dump_nfi();
            bRet = false;
        }

        if (!mtk_nand_check_dececc_done(u4SecNum))
        {
            dump_nfi();
            bRet = false;
        }

        mtk_nand_read_fdm_data(pFDMBuf, u4SecNum);

        if (!mtk_nand_check_bch_error(pPageBuf, u4SecNum - 1, u4RowAddr))
        {
            g_i4ErrNum++;
        }
        mtk_nand_stop_read();
    }

    return bRet;
}

static bool mtk_nand_exec_read_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    int block = u4RowAddr / page_per_block;
    int page_in_block = u4RowAddr % page_per_block;
    int mapped_block;
    int i, start, len, offset;
    struct nand_oobfree *free;
    u8 oob[0x80];

    mapped_block = get_mapping_block_index(block);

    if (!mtk_nand_exec_read_page_hw(nand, (mapped_block * page_per_block + page_in_block), u4PageSize, pPageBuf, oob))
        return false;

    offset = 0;
    free = nand->ecc.layout->oobfree;
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && free[i].length; i++)
    {
        start = free[i].offset;
        len = free[i].length;
        memcpy(pFDMBuf + offset, oob + start, len);
        offset += len;
    }

    return false;
}

//-------------------------------------------------------------------------------
static bool mtk_nand_exec_write_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = true;
    u32 u4SecNum = u4PageSize >> 9;
    if (mtk_nand_ready_for_write(nand, u4RowAddr, pPageBuf))
    {
        mtk_nand_write_fdm_data(pFDMBuf, u4SecNum);
        if (!mtk_nand_write_page_data(pPageBuf, u4PageSize))
        {

            bRet = false;
        }
        if (!mtk_nand_check_RW_count(u4PageSize))
        {
            bRet = false;
        }
        mtk_nand_stop_write();
        mtk_nand_set_command(NAND_CMD_PAGEPROG);
        while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
    }
    return bRet;
}

#define OOB_AVAIL_PER_SECTOR 8
static bool mtk_nand_read_oob_raw(struct nand_chip *chip, u32 page_addr, u32 length, u8 * buf)
{
    u32 sector = 0;
    u32 col_addr = 0;

    if (length > 32 || length % OOB_AVAIL_PER_SECTOR || !buf)
    {
        printf("[%s] invalid parameter, length: %d, buf: %p\n", __FUNCTION__, length, buf);
        return false;
    }

    while (length > 0)
    {
        col_addr = NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + NAND_SPARE_PER_SECTOR);
        if (!mtk_nand_ready_for_read(chip, page_addr, col_addr, false, NULL))
            return false;
        if (!mtk_nand_mcu_transfer_data(buf, length))
            return false;
        NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
        sector++;
        length -= 8;
    }

    return true;
}

bool nand_block_bad_hw(struct nand_chip * nand, u32 offset)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 page_addr = offset >> nand->page_shift;

    u8 oob_buf[OOB_AVAIL_PER_SECTOR];
    memset(oob_buf, 0, OOB_AVAIL_PER_SECTOR);

    page_addr &= ~(page_per_block - 1);

    if (!mtk_nand_read_oob_raw(nand, page_addr, OOB_AVAIL_PER_SECTOR, oob_buf))
    {
        printf("mtk_nand_read_oob_raw return fail\n");
    }

    if (oob_buf[0] != 0xff)
    {
        printf("Bad block detect at block 0x%x, oob_buf[0] is %x\n", page_addr / page_per_block, oob_buf[0]);
        return true;
    }

    return false;
}

static bool nand_block_bad(struct nand_chip *nand, u32 page_addr)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    int block = page_addr / page_per_block;
    int mapped_block = get_mapping_block_index(block);

    return nand_block_bad_hw(nand, mapped_block << nand->phys_erase_shift);
}

#if 1
static int mtk_nand_part_write(part_dev_t * dev, uchar * src, ulong dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    uint8_t res;                // *bbt = nand->bbt;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (nand->chipsize >> nand->phys_erase_shift) << 1;
    u32 u4BlkAddr = (dst >> nand->phys_erase_shift) << 1;
    u32 u4ColAddr = dst & (u4PageSize - 1);
    u32 u4RowAddr = dst >> nand->page_shift;
    u32 u4RowEnd;
    u32 u4WriteLen = 0;
    int i4Len;

    int k = 0;
/*
	MSG(OPS, "|--------------------------|\n");
	MSG(OPS, "start addr = 0x%08x\n", src);
	MSG(OPS, "dest addr = 0x%08x\n", dst);
	MSG(OPS, "size = %d\n", size);
	MSG(OPS, "u4BlkAddr = 0x%08x\n", (u4BlkAddr/2));
	MSG(OPS, "u4ColAddr = 0x%08x\n", u4ColAddr);
	MSG(OPS, "u4RowAddr = 0x%08x\n", u4RowAddr);
	MSG(OPS, "|--------------------------|\n");
*/
    for (k = 0; k < sizeof(g_kCMD.au1OOB); k++)
        *(g_kCMD.au1OOB + k) = 0xFF;

    while ((size > u4WriteLen) && (u4BlkAddr < u4BlkEnd))
    {
        if (!u4ColAddr)
        {
            MSG(OPS, "Erase the block of 0x%08x\n", u4BlkAddr);
            mtk_nand_reset();
            mtk_nand_set_mode(CNFG_OP_ERASE);
            mtk_nand_set_command(NAND_CMD_ERASE1);
            mtk_nand_set_address(0, u4RowAddr, 0, 3);
            mtk_nand_set_command(NAND_CMD_ERASE2);
            while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
        }

        res = nand_block_bad(nand, ((u4BlkAddr >> 1) * u4PageNumPerBlock));

        if (!res)
        {
            u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd; u4RowAddr++)
            {
                i4Len = min(size - u4WriteLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && (i4Len == u4PageSize))
                {
                    mtk_nand_exec_write_page(nand, u4RowAddr, u4PageSize, src + u4WriteLen, g_kCMD.au1OOB);
                } else
                {
                    memcpy(nand->buffers->databuf + u4ColAddr, src + u4WriteLen, i4Len);
                    mtk_nand_exec_write_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                }
                u4WriteLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        } else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
        u4BlkAddr += 2;
    }

    return (int)u4WriteLen;

}
#endif

//-------------------------------------------------------------------------------   
static int mtk_nand_part_read(part_dev_t * dev, ulong source, uchar * dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    uint8_t res;                // *bbt = nand->bbt;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (nand->chipsize >> nand->phys_erase_shift);  // << 1;
    u32 u4BlkAddr = (source >> nand->phys_erase_shift); // << 1;
    u32 u4ColAddr = source & (u4PageSize - 1);
    u32 u4RowAddr = source >> nand->page_shift;
    u32 u4RowEnd;
    u32 u4ReadLen = 0;
    int i4Len;
/*
	MSG(OPS, "|--------------------------|\n");
	MSG(OPS, "start addr = 0x%08x\n", source);
	MSG(OPS, "dest addr = 0x%08x\n", dst);
	MSG(OPS, "size = %d\n", size);
	MSG(OPS, "u4BlkAddr = 0x%08x\n", (u4BlkAddr/2));
	MSG(OPS, "u4ColAddr = 0x%08x\n", u4ColAddr);
	MSG(OPS, "u4RowAddr = 0x%08x\n", u4RowAddr);
	MSG(OPS, "|--------------------------|\n");	
*/
    while ((size > u4ReadLen) && (u4BlkAddr < u4BlkEnd))
    {
        res = nand_block_bad(nand, (u4BlkAddr * u4PageNumPerBlock));

        if (!res)
        {
            u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd; u4RowAddr++)
            {
                i4Len = min(size - u4ReadLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && (i4Len == u4PageSize))
                {
                    mtk_nand_exec_read_page(nand, u4RowAddr, u4PageSize, dst + u4ReadLen, g_kCMD.au1OOB);
                } else
                {
                    mtk_nand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(dst + u4ReadLen, nand->buffers->databuf + u4ColAddr, i4Len);
                }
                u4ReadLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        } else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
        u4BlkAddr++;            //  += 2;
    }
    return (int)u4ReadLen;
}

//-------------------------------------------------------------------------------
#if 0
static void mtk_nand_release_device(struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;

    MSG_FUNC_ENTRY();

    /* De-select the NAND device */
    this->select_chip(mtd, -1);
}
#endif
//-------------------------------------------------------------------------------
static void mtk_nand_command_bp(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
    struct nand_chip *nand = mtd->priv;
    u32 timeout;

    switch (command)
    {
      case NAND_CMD_SEQIN:
          /* Reset g_kCMD */
          if (g_kCMD.u4RowAddr != page_addr)
          {
              memset(g_kCMD.au1OOB, 0xFF, sizeof(g_kCMD.au1OOB));
              g_kCMD.pDataBuf = NULL;
          }
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column;
          break;

      case NAND_CMD_PAGEPROG:
          if (g_kCMD.pDataBuf || (0xFF != g_kCMD.au1OOB[0]))
          {
              u8 *pDataBuf = g_kCMD.pDataBuf ? g_kCMD.pDataBuf : nand->buffers->databuf;
              mtk_nand_exec_write_page(nand, g_kCMD.u4RowAddr, mtd->writesize, pDataBuf, g_kCMD.au1OOB);
              g_kCMD.u4RowAddr = (u32) - 1;
              g_kCMD.u4OOBRowAddr = (u32) - 1;
          }
          break;

      case NAND_CMD_READOOB:
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column + mtd->writesize;
          g_i4ErrNum = 0;
          break;

      case NAND_CMD_READ0:
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column;
          g_i4ErrNum = 0;
          break;

      case NAND_CMD_ERASE1:
          mtk_nand_reset();
          mtk_nand_set_mode(CNFG_OP_ERASE);
          mtk_nand_set_command(NAND_CMD_ERASE1);
          mtk_nand_set_address(0, page_addr, 0, devinfo.addr_cycle - 2);
          break;

      case NAND_CMD_ERASE2:
          mtk_nand_set_command(NAND_CMD_ERASE2);
          while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
          break;

      case NAND_CMD_STATUS:
          NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
          mtk_nand_reset();
          mtk_nand_set_mode(CNFG_OP_SRD);
          mtk_nand_set_command(NAND_CMD_STATUS);
          NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK);
          DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NFI_NOB_SHIFT));
          break;

      case NAND_CMD_RESET:
          mtk_nand_reset();
          break;

      case NAND_CMD_READID:
          /* Issue NAND chip reset command */
          NFI_ISSUE_COMMAND(NAND_CMD_RESET, 0, 0, 0, 0);

          timeout = TIMEOUT_4;

          while (timeout)
              timeout--;

          mtk_nand_reset();
          /* Disable HW ECC */
          NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

          /* Disable 16-bit I/O */
          NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);

          NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
          //NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
          mtk_nand_set_mode(CNFG_OP_SRD);
          mtk_nand_set_command(NAND_CMD_READID);
          mtk_nand_set_address(0, 0, 1, 0);
          DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD);
          while (DRV_Reg32(NFI_STA_REG32) & STA_DATAR_STATE) ;
          break;
      default:
          printf("[ERR] mtk_nand_command_bp : unknow command %d\n", command);
          BUG();
          break;
    }
}

//-----------------------------------------------------------------------------
static void mtk_nand_select_chip(struct mtd_info *mtd, int chip)
{
    u32 busw = 0;

    if (chip == -1 && false == g_bInitDone)
    {
        struct nand_chip *nand = mtd->priv;
        if (nand->page_shift == 12)
        {
            NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_SPARE_16 | PAGEFMT_4K);
            nand->cmdfunc = mtk_nand_command_bp;
        }

        else if (nand->page_shift == 11)
        {
            NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_SPARE_16 | PAGEFMT_2K);
            nand->cmdfunc = mtk_nand_command_bp;
        } else if (nand->page_shift == 9)
        {
            NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_SPARE_16 | PAGEFMT_512);

        }

        busw = devinfo.iowidth;

        if (IO_WIDTH_16 == busw)
        {
            MSG(FUC, "Setting the NFI PAGEFMT to enable %d bit I/O\n", busw);
            NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        } else if (IO_WIDTH_8 == busw)
        {
            MSG(FUC, "Setting the NFI PAGEFMT to enable %d bit I/O\n", busw);
        } else
        {
            MSG(FUC, "Setting NFI_BUS_WIDTH (%d) is error, please check the NAND setting in UBOOT\n", busw);
            BUG();
        }
        g_bInitDone = true;
    }

    switch (chip)
    {
      case 0:
      case 1:
          DRV_WriteReg16(NFI_CSEL_REG16, chip);
          break;
    }
}

//-----------------------------------------------------------------------------
static u_char mtk_nand_read_byte(struct mtd_info *mtd)
{
    /* Check the PIO bit is ready or not */
    unsigned int timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY(timeout);
    return DRV_Reg32(NFI_DATAR_REG32);
}

//-----------------------------------------------------------------------------
static void mtk_nand_read_buf(struct mtd_info *mtd, u_char * buf, int len)
{
    struct nand_chip *nand = (struct nand_chip *)mtd->priv;
    struct NAND_CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = mtd->writesize;

    if (u4ColAddr < u4PageSize)
    {
        if ((u4ColAddr == 0) && (len >= u4PageSize))
        {
            mtk_nand_exec_read_page(nand, pkCMD->u4RowAddr, u4PageSize, buf, pkCMD->au1OOB);
            if (len > u4PageSize)
            {
                u32 u4Size = min(len - u4PageSize, sizeof(pkCMD->au1OOB));
                memcpy(buf + u4PageSize, pkCMD->au1OOB, u4Size);
            }
        } else
        {
            mtk_nand_exec_read_page(nand, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
            memcpy(buf, nand->buffers->databuf + u4ColAddr, len);
        }
        pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
    } else
    {
        u32 u4Offset = u4ColAddr - u4PageSize;
        u32 u4Size = min(len - u4PageSize - u4Offset, sizeof(pkCMD->au1OOB));
        if (pkCMD->u4OOBRowAddr != pkCMD->u4RowAddr)
        {
            mtk_nand_exec_read_page(nand, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
            pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
        }
        memcpy(buf, pkCMD->au1OOB + u4Offset, u4Size);
    }
    pkCMD->u4ColAddr += len;
}

//-----------------------------------------------------------------------------
static void mtk_nand_write_buf(struct mtd_info *mtd, const u_char * buf, int len)
{
    struct NAND_CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = mtd->writesize;
    u32 i;

    if (u4ColAddr >= u4PageSize)
    {
        u8 *pOOB = pkCMD->au1OOB;
        u32 u4Size = min(len, sizeof(pkCMD->au1OOB));
        for (i = 0; i < u4Size; i++)
        {
            pOOB[i] &= buf[i];
        }
    } else
    {
        pkCMD->pDataBuf = (u8 *) buf;
    }
    pkCMD->u4ColAddr += len;
}

//-----------------------------------------------------------------------------
static int mtk_nand_verify_buf(struct mtd_info *mtd, const u_char * buf, int len)
{
    return 0;                   /* FIXME. Always return success */
}

//-----------------------------------------------------------------------------
static int mtk_nand_dev_ready(struct mtd_info *mtd)
{
    return !(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);
}

#if 0
/******************************************************************************
 * mtk_read_multi_page_cache
 *
 * DESCRIPTION:
 *   read multi page data using cache read
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf, int page, int len
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   Only available for NAND flash support cache read.
 *   Read main data only.
 *
 *****************************************************************************/
static int mtk_read_multi_page_cache(struct mtd_info *mtd, struct nand_chip *chip, uint8_t * buf, int page, int len)
{
    // mtk_nand_ready_for_read(chip, page, 0, true);

    while (len > 0)
    {
        mtk_nand_set_mode(CNFG_OP_CUST);
        DRV_WriteReg16(NFI_CON_REG16, 8 << CON_NFI_SEC_SHIFT);
        ECC_Decode_Start();

        if (len > mtd->writesize)   // remained more than one page
            mtk_nand_set_command(0x31); // TODO: add cache read command
        else
            mtk_nand_set_command(0x3f); // last page remained

        mtk_nand_status_ready(STA_NAND_BUSY);

#ifdef USE_AHB_MODE
        mtk_nand_dma_transfer_data(buf, mtd->writesize);
#else
        mtk_nand_mcu_transfer_data(buf, mtd->writesize);
#endif
        // get ecc error info

        ECC_Decode_End();

        len -= mtd->writesize;
        buf += mtd->writesize;
        mtk_nand_reset();
    }
}
#endif

//-----------------------------------------------------------------------------
#if 0
static void mtk_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
    //1 No need to implement
    return;
}
#endif
//-----------------------------------------------------------------------------
static int mtk_nand_calculate_ecc(struct mtd_info *mtd, const u_char * dat, u_char * ecc_code)
{
    memset(ecc_code, 0xFF, 32);
    return 0;
}

//-----------------------------------------------------------------------------
static int mtk_nand_correct_data(struct mtd_info *mtd, u_char * dat, u_char * read_ecc, u_char * calc_ecc)
{
    return g_i4ErrNum;
}

static void mtk_nand_hwctl(struct mtd_info *mtd, int mode)
{
}

bool getflashid(u16 * id)
{
    u8 maf_id = 0;
    u8 dev_id = 0;
    struct mtd_info *mtd = NULL;

    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

    /* Set default NFI access timing control */
    DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    mtk_nand_command_bp(mtd, NAND_CMD_READID, 0, 0);
    maf_id = mtk_nand_read_byte(mtd);
    dev_id = mtk_nand_read_byte(mtd);

    if (maf_id == 0 || dev_id == 0)
    {
        return false;
    }
    *id = (maf_id << 8) | dev_id;
    return true;
}

//-----------------------------------------------------------------------------
static int mtk_nand_init(struct nand_chip *this)
{

    g_bInitDone = false;
    g_u4ChipVer = DRV_Reg32(CONFIG_BASE /*HW_VER */ );
    g_kCMD.u4OOBRowAddr = (u32) - 1;

    /* Note: EVB NAND  is mounted on CS0, but FPGA is CS1 ? */
    DRV_WriteReg16(NFI_CSEL_REG16, NFI_DEFAULT_CS);

    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

    /* Set default NFI access timing control */
    DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    mtk_nand_reset();

    /* Initilize interrupt. Clear interrupt, read clear. */
    DRV_Reg16(NFI_INTR_REG16);

    /* Interrupt arise when read data or program data to/from AHB is done. */
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);

    DRV_WriteReg16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    ECC_Config();
    mtk_nand_configure_fdm(8);
    mtk_nand_configure_lock();

    return 0;
}

//-----------------------------------------------------------------------------
int board_nand_init(struct nand_chip *nand)
{
    int res = mtk_nand_init(nand);
    int busw = IO_WIDTH_8;
    int id, nand_maf_id, nand_dev_id;
    u8 ext_id1, ext_id2, ext_id3;
    u32 ext_id;
    struct mtd_info *mtd = NULL;
    nand->select_chip = mtk_nand_select_chip;
    nand->cmdfunc = mtk_nand_command_bp;
    nand->read_byte = mtk_nand_read_byte;
    nand->write_buf = mtk_nand_write_buf;
    nand->dev_ready = mtk_nand_dev_ready;
    //nand->enable_hwecc    = mtk_nand_enable_hwecc;
    nand->ecc.calculate = mtk_nand_calculate_ecc;
    nand->ecc.correct = mtk_nand_correct_data;
    nand->verify_buf = mtk_nand_verify_buf;
    nand->read_buf = mtk_nand_read_buf;
    //nand->block_markbad = mtk_nand_default_block_markbad;

    nand->ecc.mode = NAND_ECC_HW;
    nand->ecc.hwctl = mtk_nand_hwctl;
    nand->ecc.layout = &nand_oob_64;
    nand->ecc.size = 512;
    nand->options = NAND_NO_AUTOINCR;

    mtk_nand_command_bp(mtd, NAND_CMD_READID, 0, 0);
    nand_maf_id = mtk_nand_read_byte(mtd);
    nand_dev_id = mtk_nand_read_byte(mtd);
    id = nand_dev_id | (nand_maf_id << 8);

    ext_id1 = mtk_nand_read_byte(mtd);
    ext_id2 = mtk_nand_read_byte(mtd);
    ext_id3 = mtk_nand_read_byte(mtd);
    ext_id = ext_id1 << 16 | ext_id2 << 8 | ext_id3;

    if (get_device_info(id, ext_id, &devinfo))
    {
        busw = devinfo.iowidth;
        DRV_WriteReg32(NFI_ACCCON_REG32, devinfo.timmingsetting);

        if (devinfo.pagesize == 4096)
            nand->ecc.layout = &nand_oob_128;
        else if (devinfo.pagesize == 2048)
            nand->ecc.layout = &nand_oob_64;
        else if (devinfo.pagesize == 512)
            nand->ecc.layout = &nand_oob_16;
    } else
    {
        MSG(INIT, "No NAND device found!\n");
    }

    if (IO_WIDTH_16 == busw)
    {
        MSG(FUC, "Setting the MTD option to enable %d bit I/O\n", busw);
        nand->options |= NAND_BUSWIDTH_16;
    } else if (IO_WIDTH_8 == busw)
    {
        MSG(FUC, "Setting the MTD option to enable %d bit I/O\n", busw);
    } else
    {
        MSG(FUC, "Setting NFI_BUS_WIDTH (%d) is error, please check the NAND setting in UBOOT\n", busw);
        BUG();
    }

    return res;

}
#endif
//-----------------------------------------------------------------------------
#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
extern int mt6575_part_register_device(part_dev_t * dev);
void board_nand_select_device(struct nand_chip *nand, int chip)
{
    static part_dev_t dev;
    mtk_nand_select_chip(NULL, chip);

    nand->chipsize -= BMT_POOL_SIZE << nand->phys_erase_shift;
    total_size = nand->chipsize - (PMT_POOL_SIZE << nand->phys_erase_shift);
    if (!g_bmt)
    {
        if (!(g_bmt = init_bmt(nand, BMT_POOL_SIZE)))
        {
            MSG(INIT, "Error: init bmt failed\n");
            return;
        }
    }

    dev.id = 0;
    dev.init = 1;
    dev.blkdev = (block_dev_desc_t *) nand;
    dev.read = mtk_nand_part_read;
    dev.write = mtk_nand_part_write;
    MSG(FUC, "board_nand_select_device before register\n");
    mt6575_part_register_device(&dev);

    return;
}
#endif
//-----------------------------------------------------------------------------

#endif
