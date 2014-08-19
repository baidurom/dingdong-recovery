/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "platform.h"
#include "mmc_core.h"
#include "mtk_nand_core.h"
#include "addr_trans.h"

#define MMC_HOST_ID                 0
#define BUF_BLK_NUM                 4   /* 4 * 512bytes = 2KB */

/**************************************************************************
*  DEBUG CONTROL
**************************************************************************/

/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
extern struct nand_chip g_nand_chip;

static blkdev_t g_mmc_bdev;

#ifdef FEATURE_MMC_ADDR_TRANS
typedef enum {
     EMMC_PART_UNKNOWN = 0
    ,EMMC_PART_BOOT1
    ,EMMC_PART_BOOT2
    ,EMMC_PART_RPMB
    ,EMMC_PART_GP1
    ,EMMC_PART_GP2
    ,EMMC_PART_GP3
    ,EMMC_PART_GP4
    ,EMMC_PART_USER
    ,EMMC_PART_NUM
};

static addr_trans_info_t g_emmc_addr_trans[EMMC_PART_NUM];
static addr_trans_tbl_t g_addr_trans_tbl;
u64 g_emmc_size = 0;
static int mmc_switch_part(u32 part_id)
{
    int err = MMC_ERR_NONE;
    struct mmc_card *card;
    struct mmc_host *host;    
    u8 part = (u8)part_id;
    u8 cfg;
    u8 *ext_csd;

    card = mmc_get_card(MMC_HOST_ID);
    host = mmc_get_host(MMC_HOST_ID);
    
    if (!card) 
        return MMC_ERR_INVALID;

    ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_mmc(card) && ext_csd[EXT_CSD_REV] >= 3) {

        if (part_id == EMMC_PART_USER)
            part = EXT_CSD_PART_CFG_DEFT_PART;       

        cfg = card->raw_ext_csd[EXT_CSD_PART_CFG];

        /* already set to specific partition */
        if (part == (cfg & 0x7))
            return MMC_ERR_NONE;

        cfg = (cfg & ~0x7) | part;

        err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CFG, cfg);

        if (err == MMC_ERR_NONE) {
            err = mmc_read_ext_csd(host, card);
            if (err == MMC_ERR_NONE) {
                ext_csd = &card->raw_ext_csd[0];
                if (ext_csd[EXT_CSD_PART_CFG] != cfg)
                    err = MMC_ERR_FAILED;
            }
        }
    }
    return err;    
}

static int mmc_virt_to_phys(u32 virt_blknr, u32 *phys_blknr, u32 *part_id)
{
    int ret;
    virt_addr_t virt;
    phys_addr_t phys;

    virt.addr = virt_blknr;

    ret = virt_to_phys_addr(&g_addr_trans_tbl, &virt, &phys);

    if (phys.id == -1)
        phys.id = EMMC_PART_USER;

    *phys_blknr = (ret == 0) ? phys.addr : virt_blknr; /* in 512B unit */
    *part_id    = (ret == 0) ? phys.id : EMMC_PART_USER;

    return ret;
}

static int mmc_phys_to_virt(u32 phys_blknr, u32 part_id, u32 *virt_blknr)
{
    int ret;
    virt_addr_t virt;
    phys_addr_t phys;

    phys.addr = phys_blknr;
    phys.id   = part_id;

    ret = phys_to_virt_addr(&g_addr_trans_tbl, &phys, &virt);

    *virt_blknr = (ret == 0) ? virt.addr : phys_blknr; /* in 512B unit */

    return ret;
}

/* unit-test */
#if 0
void mmc_addr_trans_test(void)
{
    u32 i, virt_blknr, phys_blknr, part_id;

    virt_blknr = 0;
    for (i = 0; i < EMMC_PART_NUM; i++) {
        mmc_virt_to_phys(virt_blknr - 1, &phys_blknr, &part_id); 
        printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
            virt_blknr - 1, phys_blknr, part_id);
        mmc_virt_to_phys(virt_blknr, &phys_blknr, &part_id);
        printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
            virt_blknr, phys_blknr, part_id);
        mmc_virt_to_phys(virt_blknr + 1, &phys_blknr, &part_id);
        printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
            virt_blknr + 1, phys_blknr, part_id);        
        virt_blknr += g_emmc_addr_trans[i].len;
    }
    mmc_virt_to_phys(virt_blknr - 1, &phys_blknr, &part_id);
    printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
        virt_blknr - 1, phys_blknr, part_id);    
    mmc_virt_to_phys(virt_blknr, &phys_blknr, &part_id);
    printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
        virt_blknr, phys_blknr, part_id);
    mmc_virt_to_phys(virt_blknr + 1, &phys_blknr, &part_id);
    printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
        virt_blknr + 1, phys_blknr, part_id);

    phys_blknr = 0;
    for (i = 0; i < EMMC_PART_NUM; i++) {
        mmc_phys_to_virt(phys_blknr, i, &virt_blknr);
        mmc_phys_to_virt(phys_blknr + 1, i, &virt_blknr);

        mmc_phys_to_virt(phys_blknr + g_emmc_addr_trans[i].len, i, &virt_blknr);
        mmc_phys_to_virt(phys_blknr + g_emmc_addr_trans[i].len + 1, i, &virt_blknr);
    }
}
#endif

static int mmc_addr_trans_tbl_init(struct mmc_card *card, blkdev_t *bdev)
{
    u32 wpg_sz;
    u8 *ext_csd;

    memset(&g_addr_trans_tbl, 0, sizeof(addr_trans_tbl_t));

    ext_csd = &card->raw_ext_csd[0];
    bdev->offset = 0;

    if (mmc_card_mmc(card) && ext_csd[EXT_CSD_REV] >= 3) {
        u64 size[EMMC_PART_NUM];
        u32 i;

        if ((ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN)
            && (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] > 0)) {
            wpg_sz = 512 * 1024 * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 
                ext_csd[EXT_CSD_HC_WP_GPR_SIZE];
        } else {
            wpg_sz = card->csd.write_prot_grpsz;
        }
            
        size[EMMC_PART_BOOT1] = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
        size[EMMC_PART_BOOT2] = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
        size[EMMC_PART_RPMB]  = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
        size[EMMC_PART_GP1]   = ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP1_SIZE_MULT + 0];
        size[EMMC_PART_GP2]   = ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP2_SIZE_MULT + 0];
        size[EMMC_PART_GP3]   = ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP3_SIZE_MULT + 0];
        size[EMMC_PART_GP4]   = ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP4_SIZE_MULT + 0];
        size[EMMC_PART_USER]  = (u64)card->blklen * card->nblks;

        size[EMMC_PART_GP1] *= wpg_sz;
        size[EMMC_PART_GP2] *= wpg_sz;
        size[EMMC_PART_GP3] *= wpg_sz;
        size[EMMC_PART_GP4] *= wpg_sz;
		printf("*******EMMC_INFO*******\n");
		printf("eMMC partition size(1 block = 512Bytes):\n");
		printf("BOOT1:<%d> blocks\n",(u32)(size[EMMC_PART_BOOT1]/(u64)card->blklen));
		printf("BOOT2:<%d> blocks\n",(u32)(size[EMMC_PART_BOOT2]/(u64)card->blklen));
		printf("RPMB :<%d> blocks\n",(u32)(size[EMMC_PART_RPMB]/(u64)card->blklen));
		printf("GP1  :<%d> blocks\n",(u32)(size[EMMC_PART_GP1]/(u64)card->blklen));
		printf("GP2  :<%d> blocks\n",(u32)(size[EMMC_PART_GP2]/(u64)card->blklen));
		printf("GP3  :<%d> blocks\n",(u32)(size[EMMC_PART_GP3]/(u64)card->blklen));
		printf("GP4  :<%d> blocks\n",(u32)(size[EMMC_PART_GP4]/(u64)card->blklen));
		printf("USER :<%d> blocks\n",(u32)(size[EMMC_PART_USER]/(u64)card->blklen));
		printf("*******EMMC_INFO*******\n");

        for (i = EMMC_PART_BOOT1; i < EMMC_PART_NUM; i++) {
            g_emmc_addr_trans[i].id  = i;
            g_emmc_addr_trans[i].len = size[i] / 512; /* in 512B unit */
            	g_emmc_size += size[i];
        }

        /* determine user area offset */
        for (i = EMMC_PART_BOOT1; i < EMMC_PART_USER; i++) {
            bdev->offset += size[i];
        }
        bdev->offset /= bdev->blksz; /* in blksz unit */
        
        g_addr_trans_tbl.num  = EMMC_PART_NUM;
        g_addr_trans_tbl.info = &g_emmc_addr_trans[0];
    } else {
        g_addr_trans_tbl.num  = 0;
        g_addr_trans_tbl.info = NULL;
    }

    return 0;
}
#define mmc_virt_switch(vbn,pbn)    \
do{ u32 pid; \
    if (mmc_virt_to_phys(vbn, pbn, &pid) == MMC_ERR_NONE) \
        mmc_switch_part(pid); \
}while(0)
    
#else
#define mmc_virt_to_phys(vbn,pbn,pid)       do{}while(0)
#define mmc_phys_to_virt(pbn,pid,vbn)       do{}while(0)
#define mmc_virt_switch(vbn,pbn)            do{}while(0)
#define mmc_addr_trans_tbl_init(card,bdev)  do{}while(0)
#endif

static int mmc_bread(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{    
    struct mmc_host *host = (struct mmc_host*)bdev->priv;

    mmc_virt_switch(blknr, (u32*)&blknr);
    
    return mmc_block_read(host->id, (unsigned long)blknr, blks, (unsigned long*)buf);
}

static int mmc_bwrite(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    struct mmc_host *host = (struct mmc_host*)bdev->priv;    

    mmc_virt_switch(blknr, (u32*)&blknr);
    
    return mmc_block_write(host->id, (unsigned long)blknr, blks, (unsigned long*)buf);
}

// ==========================================================
// MMC Common Interface - Init
// ==========================================================
u32 mmc_init_device(void)
{
    int ret;
    struct mmc_card *card;

    if (!blkdev_get(BOOTDEV_SDMMC)) {

        ret = mmc_init(MMC_HOST_ID);
        if (ret != 0) {
            printf("[SD0] init card failed\n");
            return ret;
        }

        memset(&g_mmc_bdev, 0, sizeof(blkdev_t));

        card = mmc_get_card(MMC_HOST_ID);

        g_mmc_bdev.blksz   = MMC_BLOCK_SIZE;
        g_mmc_bdev.erasesz = MMC_BLOCK_SIZE;
        g_mmc_bdev.blks    = card->nblks;
        g_mmc_bdev.bread   = mmc_bread;
        g_mmc_bdev.bwrite  = mmc_bwrite;
        g_mmc_bdev.type    = BOOTDEV_SDMMC;
        g_mmc_bdev.blkbuf  = (u8*)STORAGE_BUFFER_ADDR;
        g_mmc_bdev.priv    = (void*)mmc_get_host(MMC_HOST_ID);

#ifdef FEATURE_MMC_ADDR_TRANS
        mmc_addr_trans_tbl_init(card, &g_mmc_bdev);
#endif

        blkdev_register(&g_mmc_bdev);
    }
    return 0;
}

u32 mmc_get_device_id(u8 *id, u32 len,u32 *fw_len)
{
    u8 buf[16]; /* CID = 128 bits */
    struct mmc_card *card;
	u8 buf_sanid[512];

    if (0 != mmc_init_device())
        return -1;

    card = mmc_get_card(MMC_HOST_ID);
    
    buf[0] = (card->raw_cid[0] >> 24) & 0xFF; /* Manufacturer ID */
    buf[1] = (card->raw_cid[0] >> 16) & 0xFF; /* Reserved(6)+Card/BGA(2) */
    buf[2] = (card->raw_cid[0] >> 8 ) & 0xFF; /* OEM/Application ID */
    buf[3] = (card->raw_cid[0] >> 0 ) & 0xFF; /* Product name [0] */
    buf[4] = (card->raw_cid[1] >> 24) & 0xFF; /* Product name [1] */
    buf[5] = (card->raw_cid[1] >> 16) & 0xFF; /* Product name [2] */
    buf[6] = (card->raw_cid[1] >> 8 ) & 0xFF; /* Product name [3] */
    buf[7] = (card->raw_cid[1] >> 0 ) & 0xFF; /* Product name [4] */
    buf[8] = (card->raw_cid[2] >> 24) & 0xFF; /* Product name [5] */
    buf[9] = (card->raw_cid[2] >> 16) & 0xFF; /* Product revision */
	buf[10] =(card->raw_cid[2] >> 8 ) & 0xFF; /* Serial Number [0] */
	buf[11] =(card->raw_cid[2] >> 0 ) & 0xFF; /* Serial Number [1] */
	buf[12] =(card->raw_cid[3] >> 24) & 0xFF; /* Serial Number [2] */
	buf[13] =(card->raw_cid[3] >> 16) & 0xFF; /* Serial Number [3] */
	buf[14] =(card->raw_cid[3] >> 8 ) & 0xFF; /* Manufacturer date */
	buf[15] =(card->raw_cid[3] >> 0 ) & 0xFF; /* CRC7 + stuff bit*/
	*fw_len = 1;
	if(buf[0] == 0x45){
		if (0 == mmc_get_sandisk_fwid(MMC_HOST_ID,buf_sanid)){
			*fw_len = 6;
		}	
	}
	len = len > 16 ? 16 : len;
    memcpy(id, buf, len);
	if(*fw_len == 6)
		memcpy(id+len,buf_sanid+32,6);
	else
		*(id+len) = buf[9];
    return 0;
}

#if CFG_LEGACY_USB_DOWNLOAD
// ==========================================================
// MMC Common Interface - Correct R/W Address
// ==========================================================
u32 mmc_find_safe_block (u32 offset)
{
    return offset;
}


// ==========================================================
// MMC Common Interface - Read Function
// ==========================================================
u32 mmc_read_data (u8 * buf, u32 offset)
{
    unsigned long blknr;
    u32 blks;
    int ret;

    blknr = offset / 512;
    blks = BUF_BLK_NUM;
    mmc_virt_switch(blknr, (u32*)&blknr);
    ret = mmc_block_read(MMC_HOST_ID, blknr, blks, (unsigned long *)buf);
    if (ret != MMC_ERR_NONE) 
    {
        printf("[SD0] block read 0x%x failed\n", offset);
    }

    return offset;
}

// ==========================================================
// MMC Common Interface - Write Function
// ==========================================================
u32 mmc_write_data (u8 * buf, u32 offset)
{
    unsigned long blknr;
    u32 blks;
    int ret;

    blknr = offset / 512;
    blks = BUF_BLK_NUM; /* 2K bytes */
    mmc_virt_switch(blknr, (u32*)&blknr);
    ret = mmc_block_write(MMC_HOST_ID, blknr, blks, (unsigned long *)buf);
    if (ret != MMC_ERR_NONE) 
    {
        printf("[SD0] block write 0x%x failed\n", offset);
    }    
    return offset;
}

// ==========================================================
// MMC Common Interface - Erase Function
// ==========================================================
bool mmc_erase_data (u32 offset, u32 offset_limit, u32 size)
{
    /* Notice that the block size is different with different emmc.
    * Thus, it could overwrite other partitions while erasing data.
    * Don't implement it if you don't know the block size of emmc.
    */
    return TRUE;
}

// ==========================================================
// MMC Common Interface - Check If Device Is Ready To Use
// ==========================================================
void mmc_wait_ready (void)
{
    return;
}

// ==========================================================
// MMC Common Interface - Checksum Calculation Body
// ==========================================================
u32 mmc_chksum_body (u32 chksm, char *buf, u32 pktsz)
{
    u32 i;

    // TODO : Correct It    
    /* checksum algorithm body, simply exclusive or */
    for (i = 0; i < pktsz; i++)
        chksm ^= buf[i];

    return chksm;
}

// ==========================================================
// MMC Common Interface - Checksum Calculation
// ==========================================================
u32 mmc_chksum_per_file (u32 mmc_offset, u32 img_size)
{
    u32 now = 0, i = 0, chksm = 0, start_block = 0, total = 0;
    INT32 cnt;
    bool ret = TRUE;

    // TODO : Correct it. Don't use nand page size
    u32 start = mmc_offset;
    u32 pagesz = g_nand_chip.page_size;
    u32 pktsz = pagesz + g_nand_chip.oobsize;
    u8 *buf = (u8 *)STORAGE_BUFFER_ADDR;

    // clean the buffer
    memset (buf, 0x0, STORAGE_BUFFER_ADDR);

    // calculate the number of page
    total = img_size / pagesz;
    if (img_size % pagesz != 0)
    {    
        total++;
    }

    // check the starting block is safe
    start_block = mmc_find_safe_block (start);
    if (start_block != start)
    {     
        start = start_block;
    }

    // copy data from msdc to memory
    for (cnt = total, now = start; cnt >= 0; cnt--, now += pagesz)
    {
        /* read a packet */
        mmc_read_data (buf, now);
        chksm = mmc_chksum_body (chksm, buf, pktsz);
    }

    return chksm;
}
#endif

