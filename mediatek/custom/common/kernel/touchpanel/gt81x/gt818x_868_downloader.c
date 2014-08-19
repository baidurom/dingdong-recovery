/* drivers/input/touchscreen/gt818x_update.c
 *
 * 2010 - 2012 Goodix Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Version:1.0
 * Release Date:2012/06/08
 * Revision record:
 *      V1.0:2012/06/08,create file.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>

#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "tpd.h"
#include <cust_eint.h>
#include <linux/jiffies.h>
#include "tpd_custom_gt818_818x_868.h"

#ifndef TPD_NO_GPIO
#include "cust_gpio_usage.h"
#endif

#define PACK_SIZE            64                    //update file package size
#define SEARCH_FILE_TIMES    50
#define UPDATE_FILE_PATH_2   "/data/goodix/_goodix_update_.bin"
#define UPDATE_FILE_PATH_1   "/sdcard/goodix/_goodix_update_.bin"

#define GUP_USE_HEADER_FILE     1

//-----------------------------------------------
//Do not change the order                       |
#define HEADER_UPDATE_DATA NULL         //      |
#if GUP_USE_HEADER_FILE                 //      |
#include "gt818x_868_firmware.h"            //      |
#endif                                  //      |
//-----------------------------------------------

//#ifdef GTP_AUTO_UPDATE

#define BIT_NVRAM_STROE        0
#define BIT_NVRAM_RECALL       1
#define BIT_NVRAM_LOCK         2
#define REG_NVRCS              0x1201

#define READ_FW_MSG_ADDR       0x715
#define READ_MSK_VER_ADDR      0x4014

#define FW_HEAD_LENGTH         30
#define FILE_HEAD_LENGTH       100
#define IGNORE_LENGTH          100
#define FW_MSG_LENGTH          7
#define UPDATE_DATA_LENGTH     5000

#define GTP_ADDR_LENGTH        2
#define MAX_I2C_TRANSFER_SIZE  6
#define fail                   0
#define success                1
#define false                  0
#define true                   1

#define GTP_GPIO_OUTPUT(pin, level) do{\
                                        mt_set_gpio_mode(pin, GPIO_CTP_EINT_PIN_M_GPIO);\
                                        mt_set_gpio_dir(pin, GPIO_DIR_OUT);\
                                        mt_set_gpio_out(pin, level);\
                                      }while(0)
#define GTP_GPIO_AS_INT(pin) do{\
                                mt_set_gpio_mode(pin, GPIO_CTP_EINT_PIN_M_EINT);\
                                mt_set_gpio_dir(pin, GPIO_DIR_IN);\
                                mt_set_gpio_pull_enable(pin, GPIO_PULL_DISABLE);\
                               }while(0)

#define TPD_DWN_DEBUG

#ifdef TPD_DWN_DEBUG
#undef TPD_DWN_DEBUG
#define TPD_DWN_DEBUG(a,arg...) printk("DWN-DEBUG" ": " a,##arg)
#else
#define TPD_DWN_DEBUG(arg...)
#endif

#define TPD_DWN_ERROR(a,arg...) printk("DWN-ERROR" ": " a,##arg)
#define TPD_DWN_DMESG(a,arg...) printk("DWN-DMESG" ": " a,##arg)

u16 gt818x_show_len;
u16 gt818x_total_len;
extern u8 chip_type;

static struct i2c_client *guitar_client = NULL;

static void gtp_reset_guitar(s32 ms);

#pragma pack(1)
typedef struct
{
    u8  type;          //chip type
    u16 version;       //FW version
    u8  msk_ver[4];    //mask version
    u8  st_addr[2];    //start addr
    u16 lenth;         //FW length
    u8  chk_sum[3];
    u8  force_update[6];//force_update flag,"GOODIX"
} st_fw_head;
#pragma pack()

typedef struct
{
    u8 force_update;
    u8 fw_flag;
    loff_t gt_loc;
    struct file *file;
    st_fw_head  ic_fw_msg;
    mm_segment_t old_fs;
} st_update_msg;

static st_update_msg update_msg;
//******************************************************************************
//static u8 __initdata inbuf[256];
//static u8 __initdata outbuf[256];

static s32 gtp_i2c_write(struct i2c_client *client, uint8_t *data, s32 len)
{

    u8  buf[256];
    u8  retry = 0;
    u16 left = len - GTP_ADDR_LENGTH;
    u16 offset = 0;
    u16 address = 0;

    struct i2c_msg msg =
    {
        .addr = client->addr,
        .flags = !I2C_M_RD,
    };


    if (data == NULL)
    {
        return -1;
    }

    address = (u16)(data[0] << 8) + (u16)data[1];



    msg.buf = buf;

    while (left > 0)
    {
        retry = 0;

        buf[0] = ((address + offset) >> 8) & 0xFF;
        buf[1] = (address + offset) & 0xFF;

        if (left > MAX_I2C_TRANSFER_SIZE)
        {
            memcpy(&buf[GTP_ADDR_LENGTH], &data[offset + GTP_ADDR_LENGTH], MAX_I2C_TRANSFER_SIZE);
            msg.len = MAX_I2C_TRANSFER_SIZE + GTP_ADDR_LENGTH;
            left -= MAX_I2C_TRANSFER_SIZE;
            offset += MAX_I2C_TRANSFER_SIZE;
        }
        else
        {
            memcpy(&buf[GTP_ADDR_LENGTH], &data[offset + GTP_ADDR_LENGTH], left);
            msg.len = left + GTP_ADDR_LENGTH;
            left = 0;
        }

        while (i2c_transfer(client->adapter, &msg, 1) != 1)
        {
            retry++;

            if (retry == 20)
            {
                return -1;
            }
        }
    }

    return 1;
}

static s32 gtp_i2c_read(struct i2c_client *client, uint8_t *data, s32 len)
{
    u8  buf[GTP_ADDR_LENGTH];
    u8  retry;
    u16 left = len - GTP_ADDR_LENGTH;
    u16 offset = 0;
    u16 address = 0;

    struct i2c_msg msg[2] =
    {
        {
            .addr = client->addr,
            .flags = !I2C_M_RD,
            .buf = buf,
            .len = GTP_ADDR_LENGTH,
        },
        {
            .addr = client->addr,
            .flags = I2C_M_RD,
        },
    };

    if (data == NULL)
    {
        return -1;
    }

    address = (u16)(data[0] << 8) + (u16)data[1];

    while (left > 0)
    {
        buf[0] = ((address + offset) >> 8) & 0xFF;
        buf[1] = (address + offset) & 0xFF;

        msg[1].flags = I2C_M_RD;
        msg[1].buf = &data[offset + GTP_ADDR_LENGTH];

        if (left > 8)
        {
            msg[1].len = 8;
            left -= 8;
            offset += 8;
        }
        else
        {
            msg[1].len = left;
            left = 0;
        }

        retry = 0;

        while (i2c_transfer(client->adapter, &msg[0], 2) != 2)
        {
            retry++;

            if (retry == 20)
            {
                return -1;
            }
        }

    }

    return 1;
}

static s32 gtp_i2c_end_cmd(struct i2c_client *client)
{

    s32 ret = -1;
    u8  end_cmd_data[2] = {0x80, 0x00};

    ret = gtp_i2c_write(client, end_cmd_data, 2);

    return ret;
}

static void gtp_reset_guitar(s32 ms)
{
    GTP_GPIO_OUTPUT(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(ms);
    GTP_GPIO_OUTPUT(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(ms);
    GTP_GPIO_OUTPUT(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(50);

    return;
}

static u8 gup_get_ic_msg(struct i2c_client *client, u16 addr, u8 *msg, s32 len)
{
    s32 i = 0;

    msg[0] = (addr >> 8) & 0xff;
    msg[1] = addr & 0xff;

    for (i = 0; i < 5; i++)
    {
        if (gtp_i2c_read(client, msg, GTP_ADDR_LENGTH + len) > 0)
        {
            break;
        }
    }

    gtp_i2c_end_cmd(client);

    if (i >= 5)
    {
        TPD_DWN_ERROR("Read data from 0x%02x%02x failed!\n", msg[0], msg[1]);
        return fail;
    }

    return success;
}

static u8 gup_clear_mix_flag(struct i2c_client *client)
{
    s32 i = 0;
    u8  buf[3];

    buf[0] = 0x14;
    buf[1] = 0x00;
    buf[2] = 0x8C;

    for (i = 0; i < 5; i++)
    {
        if (gtp_i2c_write(client, buf, 3) > 0)
        {
            break;
        }
    }

    gtp_i2c_end_cmd(client);

    if (i >= 5)
    {
        TPD_DWN_ERROR("Clear mix flag failed!\n");
        return fail;
    }

    return success;
}

static u8 gup_get_ic_fw_msg(struct i2c_client *client)
{
    s32 i = 0;
    u8  buf[32];

    if (fail == gup_clear_mix_flag(client))
    {
        return fail;
    }

    //Get the firmware msg in IC, include firmware version and checksum flag

    if (fail == gup_get_ic_msg(client, 0x717, buf, 2))
    {
        TPD_DWN_ERROR("Get firmware msg in IC error.\n");
        return fail;
    }

    update_msg.ic_fw_msg.version = buf[GTP_ADDR_LENGTH + 1] << 8 | buf[GTP_ADDR_LENGTH];

    TPD_DWN_DEBUG("IC VID:0x%x\n", (int)update_msg.ic_fw_msg.version);
    TPD_DWN_DEBUG("IC force update:0x%x\n", update_msg.force_update);

    //Get the pid at 0x4011 in nvram
    if (fail == gup_get_ic_msg(client, 0x4011, buf, 1))
    {
        TPD_DWN_ERROR("Read pid failed!\n");
        return fail;
    }

    update_msg.ic_fw_msg.type = buf[GTP_ADDR_LENGTH];

    //Get the mask version at 0x4014 in nvram
    if (fail == gup_get_ic_msg(client, READ_MSK_VER_ADDR, buf, 1))
    {
        TPD_DWN_ERROR("Read mask version failed!\n");
        return fail;
    }

    update_msg.ic_fw_msg.msk_ver[0] = buf[GTP_ADDR_LENGTH];

    TPD_DWN_DEBUG("IC PID:%x\n", update_msg.ic_fw_msg.type);
    TPD_DWN_DEBUG("IC MSK VER:%c\n", update_msg.ic_fw_msg.msk_ver[0]);

    for (i = 0; i < 2; i++)
    {
        if (fail == gup_get_ic_msg(client, 0x694, buf, 1))
        {
            TPD_DWN_ERROR("Get firmware msg in IC error.\n");
            return fail;
        }

        update_msg.force_update = buf[GTP_ADDR_LENGTH];

        if (i == 0 && update_msg.force_update == 0xAA)
        {
            TPD_DWN_DMESG("The check sum in ic is error.\n");
            TPD_DWN_DMESG("IC will be reset.\n");
            TPD_DWN_DMESG("If the check sum is still error, \n");
            TPD_DWN_DMESG("The IC will be updated by force.\n");
            
            if(GT818X == chip_type)
            {
                gtp_reset_guitar(10);
            }
            continue;
            //msleep(100);
        }

        break;
    }

    return success;
}

/*
* Steps of reset guitar
*1. INT output low£¬delay 5ms
*2. RESET output low 100ms£¬transfer to input w/o pull-up or pull-down
*3. I2C access GUITAR
*4. delay 100ms to read register 0xff
*5. success until the return value of register 0xff is 0x55
*/
s32 gt818x_enter_update_mode(struct i2c_client *client)
{
    int ret = 1;
    u8 retry;
    u8 inbuf[3] = {0, 0xff, 0};

    // step 1
    GTP_GPIO_OUTPUT(GPIO_CTP_EINT_PIN, 0);
    msleep(5);

    //step 2
	gtp_reset_guitar(100);

    for (retry = 0; retry < 20; retry++)
    {

        //step 3
        ret = gtp_i2c_write(client, inbuf, 2);   //Test I2C connection.

        if (ret > 0)
        {
            TPD_DWN_DEBUG("<Set update mode>I2C is OK!\n");
            if(GT868 == chip_type)
            {
                return success;
            }
            //step 4
            msleep(100);

            ret = gtp_i2c_read(client, inbuf, 3);

            if (ret > 0)
            {
                TPD_DWN_DEBUG("The value of 0x00ff is 0x%02x\n", inbuf[2]);

                //step 5
                if (inbuf[2] == 0x55)
                {
                    return success;
                }
            }
        }

        msleep(40);
    }

    TPD_DWN_ERROR(KERN_INFO"Detect address %0X\n", client->addr);

    return fail;
}

void gt818x_leave_update_mode(void)
{
    GTP_GPIO_AS_INT(GPIO_CTP_EINT_PIN);
}

static u8 gup_load_update_file(struct i2c_client *client, st_fw_head *fw_head, u8 *data, u8 *path)
{
    u8 mask_num = 0;
    int ret = 0;
    int i = 0;
    u8 buf[FW_HEAD_LENGTH];

    if (path)
    {
        TPD_DWN_DEBUG("File path:%s, %d\n", path, strlen(path));
        update_msg.file = filp_open(path, O_RDONLY, 0644);

        if (IS_ERR(update_msg.file))
        {
            TPD_DWN_ERROR("Open update file(%s) error!\n", path);
            return fail;
        }
    }
    else
    {
        //Begin to search update file
        for (i = 0; i < SEARCH_FILE_TIMES; i++)
        {
            update_msg.file = filp_open(UPDATE_FILE_PATH_1, O_RDWR, 0666);

            if (IS_ERR(update_msg.file))
            {
                update_msg.file = filp_open(UPDATE_FILE_PATH_2, O_RDWR, 0666);//O_RDWR

                if (IS_ERR(update_msg.file))
                {
                    TPD_DWN_DEBUG("%3d:Searching file...\n", i);
                    msleep(3000);
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (i >= SEARCH_FILE_TIMES)
        {
            TPD_DWN_ERROR("Can't find update file.\n");
            return fail;
        }

        TPD_DWN_DEBUG("Find the update file.\n");
    }

    update_msg.old_fs = get_fs();
    set_fs(KERNEL_DS);

    update_msg.file->f_pos = IGNORE_LENGTH;

    //Make sure the file is the right file.(By check the "Guitar" flag)
    ret = update_msg.file->f_op->read(update_msg.file, (char *)&buf, 6, &update_msg.file->f_pos);

    if (ret < 0)
    {
        TPD_DWN_ERROR("Read \"Guitar\" flag error.\n");
        goto load_failed;
    }

    if (memcmp(buf, "Guitar", 6))
    {
        TPD_DWN_ERROR("The flag is %s.Not equal!\n"
                      "The update file is incorrect!\n", buf);
        goto load_failed;
    }

    TPD_DWN_DEBUG("The file flag is :%s.\n", buf);

    //Get the total number of masks
    update_msg.file->f_pos++; //ignore one byte.
    ret = update_msg.file->f_op->read(update_msg.file, &mask_num, 1, &update_msg.file->f_pos);

    if (ret < 0)
    {
        TPD_DWN_ERROR("Didn't get the mask number from the file.\n");
        goto load_failed;
    }

    TPD_DWN_DEBUG("FILE The total number of masks is:%d.\n", mask_num);
    update_msg.file->f_pos = FILE_HEAD_LENGTH + IGNORE_LENGTH;

    //Get the correct nvram data
    //The correct conditions:
    //1. the same product id
    //2. the same mask id
    //3. the nvram version in update file is greater than the nvram version in ic
    //or force update flag is marked or the check sum in ic is wrong
    update_msg.gt_loc = -1;

    for (i = 0; i < mask_num; i++)
    {
        ret = update_msg.file->f_op->read(update_msg.file, (char *)buf, FW_HEAD_LENGTH, &update_msg.file->f_pos);

        if (ret < 0)
        {
            TPD_DWN_ERROR("Read update file head error.\n");
            goto load_failed;
        }

        memcpy(fw_head, buf, sizeof(st_fw_head));
        fw_head->version = buf[1] << 8 | buf[2];
        fw_head->lenth = buf[9] << 8 | buf[10];
        TPD_DWN_DEBUG("No.%d firmware\n", i);
        TPD_DWN_DEBUG("FILE PID:%x\n", fw_head->type);
        TPD_DWN_DEBUG("FILE VID:0x%04x\n", fw_head->version);
        TPD_DWN_DEBUG("FILE mask version:%c.\n", fw_head->msk_ver[0]);
        TPD_DWN_DEBUG("FILE start address:0x%02x%02x.\n", fw_head->st_addr[0], fw_head->st_addr[1]);
        TPD_DWN_DEBUG("FILE length:%d\n", (int)fw_head->lenth);
        TPD_DWN_DEBUG("FILE force update flag:%s\n", fw_head->force_update);
        TPD_DWN_DEBUG("FILE chksum:0x%02x%02x%02x\n", fw_head->chk_sum[0],
                      fw_head->chk_sum[1], fw_head->chk_sum[2]);
        TPD_DWN_DEBUG("IC mask version:%c.\n", update_msg.ic_fw_msg.msk_ver[0]);
        TPD_DWN_DEBUG("IC PID:%x\n", update_msg.ic_fw_msg.type);
        TPD_DWN_DEBUG("IC VID:0x%04x\n", update_msg.ic_fw_msg.version);

        //First two conditions
        if (update_msg.ic_fw_msg.type == fw_head->type
                && update_msg.ic_fw_msg.msk_ver[0] == fw_head->msk_ver[0])
        {
            u16 fw_version = ((fw_head->version << 8) & 0xff00) | ((fw_head->version >> 8) & 0xff);
            u16 ic_version = ((update_msg.ic_fw_msg.version << 8) & 0xff00) | ((update_msg.ic_fw_msg.version >> 8) & 0xff);

            TPD_DWN_DEBUG("Get the same mask version and same pid.\n");

            //The third condition
            if ((fw_version > ic_version)
                    || !memcmp(fw_head->force_update, "GOODIX", 6)
                    || (update_msg.force_update == 0xAA))
            {
                // TPD_DWN_DEBUG("FILE read position:%d", file->f_pos);
                // file->f_pos = FW_HEAD_LENGTH + FILE_HEAD_LENGTH + IGNORE_LENGTH;
                if (!memcmp(fw_head->force_update, "GOODIX", 6))
                {
                    update_msg.gt_loc = update_msg.file->f_pos - FW_HEAD_LENGTH + sizeof(st_fw_head) - sizeof(fw_head->force_update);
                }

                ret = update_msg.file->f_op->read(update_msg.file, (char *)data, fw_head->lenth, &update_msg.file->f_pos);

                if (ret <= 0)
                {
                    TPD_DWN_ERROR("Read firmware data in file error.\n");
                    goto load_failed;
                }

                // set_fs(ts->old_fs);
                //  filp_close(ts->file, NULL);
                TPD_DWN_DEBUG("Load data from file successfully.\n");
                return success;
            }

            TPD_DWN_ERROR("Don't meet the third condition.\n");
            goto load_failed;
        }

        update_msg.file->f_pos += UPDATE_DATA_LENGTH;
    }

load_failed:
    set_fs(update_msg.old_fs);
    filp_close(update_msg.file, NULL);
    return fail;
}

static u8 gup_load_update_header(struct i2c_client *client, st_fw_head *fw_head, u8 **data)
{
    const u8 *pos;
    int i = 0;
    u8 mask_num = 0;

    pos = HEADER_UPDATE_DATA;

    pos += IGNORE_LENGTH;

    TPD_DWN_DMESG("The file flag is :%c%c%c%c%c%c.\n", pos[0], pos[1], pos[2], pos[3], pos[4], pos[5]);

    if (memcmp((u8 *)pos, "Guitar", 6))
    {
        TPD_DWN_ERROR("The update Header is incorrect!\n");

        return fail;
    }

    pos += 7;
    mask_num = pos[0];
    TPD_DWN_DMESG("Header The total number of masks is:%d.\n", mask_num);

    pos = HEADER_UPDATE_DATA;
    pos += FILE_HEAD_LENGTH + IGNORE_LENGTH;

    // update_msg.gt_loc = -1;
    for (i = 0; i < mask_num; i++)
    {
        memcpy(fw_head, pos, sizeof(st_fw_head));
        fw_head->version = pos[1] << 8 | pos[2];
        fw_head->lenth = pos[9] << 8 | pos[10];
        TPD_DWN_DMESG("No.%d firmware\n", i);
        TPD_DWN_DMESG("Header PID:%x\n", fw_head->type);
        TPD_DWN_DMESG("Header VID:0x%x\n", fw_head->version);
        TPD_DWN_DMESG("Header mask version:%c.\n", fw_head->msk_ver[0]);
        TPD_DWN_DMESG("Header start address:0x%02x%02x.\n", fw_head->st_addr[0], fw_head->st_addr[1]);
        TPD_DWN_DMESG("Header length:%d\n", (int)fw_head->lenth);
        TPD_DWN_DMESG("Header force update flag:%s\n", fw_head->force_update);
        TPD_DWN_DMESG("Header chksum:0x%02x%02x%02x\n", fw_head->chk_sum[0],
                      fw_head->chk_sum[1], fw_head->chk_sum[2]);

        pos += FW_HEAD_LENGTH;

        //First two conditions
        if (update_msg.ic_fw_msg.type == fw_head->type
                && update_msg.ic_fw_msg.msk_ver[0] == fw_head->msk_ver[0])
        {
            u16 fw_version = ((fw_head->version << 8) & 0xff00) | ((fw_head->version >> 8) & 0xff);
            u16 ic_version = ((update_msg.ic_fw_msg.version << 8) & 0xff00) | ((update_msg.ic_fw_msg.version >> 8) & 0xff);

            TPD_DWN_DMESG("Get the same mask version and same pid.\n");

            //The third condition
            /*if (fw_head->version > update_msg.ic_fw_msg.version
                || !memcmp(fw_head->force_update, "GOODIX", 6)
                || update_msg.force_update == 0xAA)    */
            if ((fw_version > ic_version)
                    || (update_msg.force_update == 0xAA))
            {

                *data = (u8 *)pos;

                TPD_DWN_DMESG("Load data from Header successfully.\n");
                return success;
            }

            TPD_DWN_ERROR("Don't meet the third condition.\n");
            return fail;
        }

        pos += UPDATE_DATA_LENGTH;
    }

    return fail;
}

static u8 gup_nvram_store(struct i2c_client *client)
{
    int ret;
    int i;
    u8 inbuf[3] = {REG_NVRCS >> 8, REG_NVRCS & 0xff, 0x18};

    ret = gtp_i2c_read(client, inbuf, 3);

    if (ret < 0)
    {
        return fail;
    }

    if ((inbuf[2] & BIT_NVRAM_LOCK) == BIT_NVRAM_LOCK)
    {
        return fail;
    }

    inbuf[2] = 0x18;
    inbuf[2] |= (1 << BIT_NVRAM_STROE);      //store command

    for (i = 0 ; i < 300 ; i++)
    {
        ret = gtp_i2c_write(client, inbuf, 3);

        if (ret > 0)
            return success;
    }

    return fail;
}

static u8 gup_nvram_recall(struct i2c_client *client)
{
    int ret;
    u8 inbuf[3] = {REG_NVRCS >> 8, REG_NVRCS & 0xff, 0};

    ret = gtp_i2c_read(client, inbuf, 3);

    if (ret < 0)
    {
        return fail;
    }

    if ((inbuf[2]&BIT_NVRAM_LOCK) == BIT_NVRAM_LOCK)
    {
        return fail;
    }

    inbuf[2] = (1 << BIT_NVRAM_RECALL);          //recall command
    ret = gtp_i2c_write(client , inbuf, 3);

    if (ret <= 0)
    {
        return fail;
    }

    return success;
}

static u8 gup_update_nvram(struct i2c_client *client, st_fw_head *fw_head, u8 *nvram)
{
    int length = 0;
    int ret = 0;
    int write_bytes = 0;
    int retry = 0;
    int i = 0;
    int comp = 0;
    u16 st_addr = 0;
    u8 w_buf[PACK_SIZE + GTP_ADDR_LENGTH];
    u8 r_buf[PACK_SIZE + GTP_ADDR_LENGTH];

    if (fw_head->lenth > PACK_SIZE)
    {
        write_bytes = PACK_SIZE;
    }
    else
    {
        write_bytes = fw_head->lenth;
    }

    st_addr = (fw_head->st_addr[0] << 8) | (fw_head->st_addr[1] & 0xff);
    memcpy(&w_buf[2], &nvram[length], write_bytes);
    TPD_DWN_DEBUG("Total length:%d", (int)fw_head->lenth);

    while (length < fw_head->lenth)
    {
        w_buf[0] = st_addr >> 8;
        w_buf[1] = st_addr & 0xff;
        TPD_DWN_DEBUG("Write address:0x%02x%02x\tlength:%d\n", w_buf[0], w_buf[1], write_bytes);
        ret =  gtp_i2c_write(client, w_buf, GTP_ADDR_LENGTH + write_bytes);

        if (ret <= 0)
        {
            if (retry++ > 10)
            {
                TPD_DWN_ERROR("Write the same address 10 times.Give up!\n");
                return fail;
            }

            TPD_DWN_ERROR("Write error![gup_update_nvram]\n");
            continue;
        }
        else
        {
            r_buf[0] = w_buf[0];
            r_buf[1] = w_buf[1];

            for (i = 0; i < 10; i++)
            {
                ret = gtp_i2c_read(client, r_buf, GTP_ADDR_LENGTH + write_bytes);

                if (ret <= 0)
                {
                    continue;
                }

                break;
            }

            if (i >= 10)
            {
                TPD_DWN_ERROR("Read error! Can't check the nvram data.\n");
                return fail;
            }

#if 0

            if (fail == gup_nvram_store(ts))
            {
                TPD_DWN_DEBUG("Store nvram failed.\n");
                //continue;
            }

            return fail;
#endif

            if (memcmp(r_buf, w_buf, GTP_ADDR_LENGTH + write_bytes))
            {
                if (comp ++ > 10)
                {
                    TPD_DWN_ERROR("Compare error!\n");
                    return fail;
                }

                TPD_DWN_ERROR("Updating nvram: Not equal!\n");

                continue;
                //return fail;
            }
        }

        comp = 0;
        retry = 0;
        length += PACK_SIZE;
        st_addr += PACK_SIZE;

        if ((length + PACK_SIZE) > fw_head->lenth)
        {
            write_bytes = fw_head->lenth - length;
        }

        memcpy(&w_buf[2], &nvram[length], write_bytes);
    }

    return success;
}

static u8 gup_update_firmware(struct i2c_client *client, st_fw_head *fw_head, u8 *nvram)
{
    s32 retry;
    s32 i, j;
    s32 ret;
    s32 right_count = 0;
    u32 status = 0;
    u16 check_sum = 0;
    u8  buf[150];
    u8  tmp[150];

    //Cuts the frequency
    buf[0] = 0x15;
    buf[1] = 0x22;
    buf[2] = 0x18;
    ret =  gtp_i2c_write(client, buf, 3);

    if (ret <= 0)
    {
        return fail;
    }

    gup_get_ic_msg(client, 0x1522, buf, 1);
    TPD_DWN_DEBUG("IC OSC_CAL:0x%02x.\n", buf[2]);

    gup_clear_mix_flag(client);

    for (retry = 0; retry < 10; retry++)
    {
        if (!(status & 0x01))
        {
            tmp[0] = 0x4f;
            tmp[1] = 0x70;
            gtp_i2c_read(client, tmp, 130);

            for (i = 0; i < 50; i++)
            {
                buf[0] = 0x4f;
                buf[1] = 0x70;
                ret = gtp_i2c_read(client, buf, 130);

                if (ret <= 0)
                {
                    continue;
                }
                else
                {
                    TPD_DWN_DEBUG("Read solidified config successfully!\n");
                    ret = !memcmp(tmp, buf, 130);
                    memcpy(tmp, buf, 130);

                    if (true == ret)
                    {
                        if (right_count++ < 3)
                        {
                            continue;
                        }
                        else
                        {
                            for (j = 0; j < 128; j++)
                            {
                                check_sum += buf[j + 2];
                            }

                            TPD_DWN_DEBUG("check sum of solidified config is %04x\n", check_sum);
                            status |= 0x01;
                            break;
                        }
                    }
                    else
                    {
                        right_count = 0;
                    }

                    if (i >= 40)
                    {
                        TPD_DWN_ERROR("Compare solidified config failed!\n");
                        return fail;
                    }
                }
            }
        }

        //Write the 2nd part (nvram)
        if (!(status & 0x02))
        {
            if (fail == gup_update_nvram(client, fw_head, nvram))
            {
                continue;
            }
            else
            {
                TPD_DWN_DEBUG("Update nvram successfully!\n");
                status |= 0x02;
                msleep(1);
            }
        }

        //Write the 3rd part (check sum)
        if (1)
        {
            u32 sum = 0;
            sum |= fw_head->chk_sum[0] << 16;
            sum |= fw_head->chk_sum[1] << 8;
            sum |= fw_head->chk_sum[2];
            sum += check_sum;

            fw_head->chk_sum[0] = sum >> 16;
            fw_head->chk_sum[1] = sum >> 8;
            fw_head->chk_sum[2] = sum;

            TPD_DWN_DEBUG("FILE chksum after addition:0x%02x%02x%02x\n", fw_head->chk_sum[0],
                          fw_head->chk_sum[1], fw_head->chk_sum[2]);
            buf[0] = 0x4f;
            buf[1] = 0xf3;
            memcpy(&buf[2], fw_head->chk_sum, sizeof(fw_head->chk_sum));
            ret = gtp_i2c_write(client, buf, 5);

            if (ret <= 0)
            {
                continue;
            }
            else
            {
                TPD_DWN_DEBUG("Update check sum successfully!\n");
                break;
            }
        }
    }

    if (retry >= 10)
    {
        return fail;
    }
    else
    {
        return success;
    }

    /*
        else
        {
            for (retry = 0; retry < 10; retry++)
            {
                buf[0] = 0x00;
                buf[1] = 0xff;
                buf[2] = 0x44;
                ret = gtp_i2c_write(client, buf, 3);
                if (ret > 0)
                {
                    break;
                }
            }

            if (retry >= 10)
            {
                TPD_DWN_ERROR("Write address at 0x00ff error!\n");
                return fail;
            }
            msleep(10);
        }

        for (retry = 0; retry < 30; retry++)
        {
            msleep(1);
            if (fail == gup_get_ic_msg(client, 0x00ff, buf, 1))
            {
                TPD_DWN_ERROR("Read address at 0x00ff error!\t retry:%d", retry);
                continue;
            }

            if (0xcc == buf[GTP_ADDR_LENGTH])
            {
                return success;
            }
            else
            {
                TPD_DWN_ERROR("The value of 0x00ff: 0x%02x!\t retry:%d", buf[GTP_ADDR_LENGTH], retry);
                continue;
            }
        }

        TPD_DWN_ERROR("The value of 0x00ff error.\n");
        return fail;
    */
}

s32 gt818x_update_proc(void *dir)
{
    s32 ret;
    u32 retry = 100;
    u32 i = 0;
    u8 *data = NULL;
    u8 *ic_nvram = NULL;
    st_fw_head fw_head;
    u8 buf[32];

    gt818x_show_len = 20;
    gt818x_total_len = 100;

    if (!(HEADER_UPDATE_DATA != NULL && dir == NULL))
    {
        data = kzalloc(UPDATE_DATA_LENGTH, GFP_KERNEL);

        if (NULL == data)
        {
            TPD_DWN_ERROR("data failed apply for memory.\n");
            return fail;
        }
    }

    ic_nvram = kzalloc(UPDATE_DATA_LENGTH, GFP_KERNEL);

    if (NULL == ic_nvram)
    {
        TPD_DWN_ERROR("ic_nvram failed apply for memory.\n");
        goto app_mem_failed;
    }

    TPD_DWN_DEBUG("Apply for memory successfully.memory size: %d.\n", UPDATE_DATA_LENGTH);

    if (dir != NULL)
    {
        gup_get_ic_fw_msg(guitar_client);
        if(GT818X == chip_type)
        {
            gtp_reset_guitar(10);
        }
    }
    else
    {
        msleep(1000);
    }

    TPD_DWN_DEBUG("Updating...\n");

    if (!(HEADER_UPDATE_DATA != NULL && dir == NULL))
    {
        if (fail == gup_load_update_file(guitar_client, &fw_head, data, (u8 *)dir))
        {
            TPD_DWN_ERROR("Load file data failed!\n");
            goto load_failed;
        }
    }
    else
    {
        if (fail == gup_load_update_header(guitar_client, &fw_head, &data))
        {
            TPD_DWN_ERROR("Load file data failed!\n");
            goto load_failed;
        }
    }

    TPD_DWN_DEBUG("Load file data successfully!\n");

    gt818x_show_len = 40;
    gt818x_total_len = 100;

    if(GT818X == chip_type)
    {
        for (i = 0; i < 5; i++)
        {
            if (fail == gt818x_enter_update_mode(guitar_client))
            {
                TPD_DWN_ERROR("Next try![Enter update mode]\n");
                continue;
            }
            else
            {
                TPD_DWN_DEBUG("Set update mode successfully.\n");
                break;
            }
        }
    }
    else
    {
        buf[0] = 0x00;
        buf[1] = 0xff;
        buf[2] = 0x00;
        for(retry=0; retry<20; retry++)
        {
            ret = gtp_i2c_write(guitar_client, buf, 2);   //Test I2C connection.
            if (ret > 0)
            {
                TPD_DWN_DEBUG("<Set update mode>I2C is OK!");
                msleep(100);
                ret = gtp_i2c_read(guitar_client, buf, 3);
                if (ret > 0)
                {
                    TPD_DWN_DEBUG("The value of 0x00ff is 0x%02x", buf[2]);
                    if(buf[2] == 0x55)
                    {
                        TPD_DWN_DEBUG("GT868 Wait unlock nvram successfully!");
                        break;
                    }
                }  
            }
        }
        if (i >= 5)
        {
            TPD_DWN_ERROR("GT868 Wait unlock nvram failed.");
            return fail;
        }
    }

    if (i >= 5)
    {
        TPD_DWN_ERROR("Set update mode failed.\n");
        return fail;
    }

    retry = 0;

    while (retry++ < 5)
    {
        if (fail == gup_update_firmware(guitar_client, &fw_head, data))
        {
            TPD_DWN_ERROR("Update firmware failed.\n");
            continue;
        }

        TPD_DWN_DEBUG("Update firmware successfully.\n");

        //while(1)  // simulation store operation failed
        if (fail == gup_nvram_store(guitar_client))
        {
            TPD_DWN_ERROR("Store nvram failed.\n");
            continue;
        }

        msleep(100);

        if (fail == gup_get_ic_msg(guitar_client, 0x1201, buf, 1))
        {
            TPD_DWN_ERROR("Read NVRCS failed.(Store)\n");
            continue;
        }

        if (buf[GTP_ADDR_LENGTH] & 0x01)
        {
            TPD_DWN_ERROR("Check NVRCS(0x%02x) failed.(Store)\n", buf[GTP_ADDR_LENGTH]);
            continue;
        }

        TPD_DWN_DEBUG("Store nvram successfully.\n");

        if (fail == gup_nvram_recall(guitar_client))
        {
            TPD_DWN_ERROR("Recall nvram failed.\n");
            continue;
        }

        msleep(5);

        if (fail == gup_get_ic_msg(guitar_client, 0x1201, buf, 1))
        {
            TPD_DWN_ERROR("Read NVRCS failed.(Recall)\n");
            continue;
        }

        if (buf[GTP_ADDR_LENGTH] & 0x02)
        {
            TPD_DWN_ERROR("Check NVRCS(0x%02x) failed.(Recall)\n", buf[GTP_ADDR_LENGTH]);
            continue;
        }

        TPD_DWN_DEBUG("Recall nvram successfully.\n");

        ic_nvram[0] = fw_head.st_addr[0];
        ic_nvram[1] = fw_head.st_addr[1];

        for (i = 0; i < 10; i++)
        {
            ret = gtp_i2c_read(guitar_client, ic_nvram, GTP_ADDR_LENGTH + fw_head.lenth);

            if (ret <= 0)
            {
                continue;
            }

            break;
        }

        if (i >= 10)
        {
            TPD_DWN_ERROR("Read nvram failed!\n");
            continue;
        }

        TPD_DWN_DEBUG("Read nvram successfully!\n");

        if (memcmp(data, &ic_nvram[2], fw_head.lenth))
        {
            TPD_DWN_ERROR("Nvram not equal!\n");
            continue;
        }

        TPD_DWN_DEBUG("Check nvram by byte successfully!\n");

        //      if (HEADER_UPDATE_DATA == NULL || dir != NULL)
        if (HEADER_UPDATE_DATA == NULL && dir == NULL)
        {
            if (update_msg.gt_loc > 0)
            {
                TPD_DWN_DEBUG("Location:%d, Ret:%d.\n", (s32)update_msg.gt_loc, (s32)ret);
                memset(buf, 0, sizeof(buf));
                ret = update_msg.file->f_op->write(update_msg.file, buf, 6, &update_msg.gt_loc);

                if (ret < 0)
                {
                    TPD_DWN_ERROR("Didn't clear the focre update flag in file.\n");
                }
                else
                {
                    TPD_DWN_DEBUG("Clear the focre update flag in file.Location:%d, Ret:%d.\n", (s32)update_msg.gt_loc, (s32)ret);
                }
            }
        }

        TPD_DWN_DEBUG("Update successfully!\n");
        break;
    }

    if (!(HEADER_UPDATE_DATA != NULL && dir == NULL))
    {
        set_fs(update_msg.old_fs);
        filp_close(update_msg.file, NULL);
    }

    gt818x_leave_update_mode();
    TPD_DWN_DEBUG("Leave update mode!\n");

    //Reset guitar
    TPD_DWN_DEBUG("Reset IC and send config!\n");
    gtp_reset_guitar(10);

    msleep(50);

load_failed:
    kfree(ic_nvram);
app_mem_failed:

    if (!(HEADER_UPDATE_DATA != NULL && dir == NULL))
    {
        kfree(data);
    }

    if (retry < 5)
    {
        gt818x_show_len = 100;
        return success;
    }

    gt818x_show_len = 200;
    TPD_DWN_ERROR("Update failed!\n");
    return fail;
}

u8 gt818x_868_downloader(struct i2c_client *client)
{
    u8 flag = 0;
    struct task_struct *thread = NULL;
    s32 retry = 0;

    TPD_DWN_DMESG("Ready to run update thread.\n");

    guitar_client = client;

    update_msg.fw_flag = gup_get_ic_fw_msg(guitar_client);

    if (fail == update_msg.fw_flag)
    {
        TPD_DWN_DEBUG("Try get ic msg in update mode.\n");

        for (retry = 0; retry < 5; retry++)
        {
            if (success == gt818x_enter_update_mode(client))
            {
                break;
            }
        }

        if (retry >= 5)
        {
            update_msg.fw_flag = fail;
        }
        else
        {
            TPD_DWN_DEBUG("Get ic msg in update mode.\n");
            update_msg.fw_flag = gup_get_ic_fw_msg(client);
            update_msg.ic_fw_msg.version = 0xfff0;

            if (update_msg.force_update == 0xAA)
            {
                flag = 0xff;
            }
        }

        gt818x_leave_update_mode();
    }
    
    if(GT818X == chip_type)
    {
        gtp_reset_guitar(10);
    }

    if (success == update_msg.fw_flag)
    {
        update_msg.gt_loc = -1;
        thread = kthread_run(gt818x_update_proc, (void *)NULL, "guitar_update");

        if (IS_ERR(thread))
        {
            TPD_DWN_ERROR("Failed to create update thread.\n");
            return -1;
        }
    }

    return 0;
}
//#endif   //endif GTP_AUTO_UPDATE
//******************************End of firmware update surpport*******************************
