/* drivers/input/touchscreen/gt816_818_update.c
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
 * Version:1.2
 *        V1.0:2012/05/01,create file.
 *        V1.2:2012/06/08,update some statement.
 *
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
#include "gt818_firmware.h"             //      |
#endif                                  //      |
//-----------------------------------------------



//#ifdef GTP_AUTO_UPDATE

#define BIT_NVRAM_STROE        0
#define BIT_NVRAM_RECALL       1
#define BIT_NVRAM_LOCK         2
#define REG_NVRCS              0x1201

#define READ_FW_MSG_ADDR       0x0F7C
#define READ_MSK_VER_ADDR      0xC009

#define FW_HEAD_LENGTH         30
#define FILE_HEAD_LENGTH       100
#define IGNORE_LENGTH          100
#define FW_MSG_LENGTH          7
#define UPDATE_DATA_LENGTH     5000
#define GTP_ADDR_LENGTH        2

#define MAX_TRANSACTION_LENGTH          8
#define I2C_DEVICE_ADDRESS_LEN          2
#define MAX_I2C_TRANSFER_SIZE           (MAX_TRANSACTION_LENGTH - I2C_DEVICE_ADDRESS_LEN)

#define TPD_CONFIG_REG_BASE                 0x6A2
#define TPD_CONFIG_SIZE                     (106)

#define TPD_CHIP_VERSION_C_FIRMWARE_BASE    0x5A
#define TPD_CHIP_VERSION_D1_FIRMWARE_BASE   0x7A
#define TPD_CHIP_VERSION_E_FIRMWARE_BASE    0x9A
#define TPD_CHIP_VERSION_D2_FIRMWARE_BASE   0xBA
#define MAGIC_NUMBER_1                      0x4D454449
#define MAGIC_NUMBER_2                      0x4154454B

#define GTP_REG_VERSION       0x715

#define GTP_GPIO_OUTPUT(pin, level) do{\
                                      mt_set_gpio_mode(pin, GPIO_CTP_EINT_PIN_M_GPIO);\
                                      mt_set_gpio_dir(pin, GPIO_DIR_OUT);\
                                      mt_set_gpio_out(pin, level ? GPIO_OUT_ONE : GPIO_OUT_ZERO);\
                                      }while(0)

#define GTP_GPIO_AS_INT(pin)        do{\
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


enum
{
    TPD_GT818_VERSION_B,
    TPD_GT818_VERSION_C,
    TPD_GT818_VERSION_D1,
    TPD_GT818_VERSION_E,
    TPD_GT818_VERSION_D2
};

#define fail    0
#define success 1
#define false   0
#define true    1

u16 gt818_show_len;
u16 gt818_total_len;

static struct i2c_client *guitar_client = NULL;

extern u8 *cfg_data;

#pragma pack(1)
typedef struct
{
    s32 magic_number_1;
    s32 magic_number_2;
    u16 version;
    u16 length;
    u16 checksum;
} st_check_msg;
#pragma pack()

typedef struct
{
    u8 st_addr[2];
    u16 lenth;
} st_fw_head;

typedef struct
{
    u16 version;
    u8 fw_flag;
    loff_t gt_loc;
    struct file *file;
    st_check_msg  fw_msg;
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

static s32 gtp_i2c_pre_cmd(struct i2c_client *client)
{
    s32 ret = -1;
    u8 pre_cmd_data[2] = {0x0f, 0xff};

    ret = gtp_i2c_write(client, pre_cmd_data, 2);

    return ret;
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
    GTP_GPIO_OUTPUT(GPIO_CTP_RST_PIN, 1);
    msleep(ms);
    GTP_GPIO_OUTPUT(GPIO_CTP_RST_PIN, 0);
    msleep(ms);
    GTP_GPIO_OUTPUT(GPIO_CTP_RST_PIN, 1);
    msleep(50);

    return;
}

static s32  gtp_send_cfg(struct i2c_client *client)
{
    s32 ret = -1;
    u8 cfg_buf[256];

    cfg_buf[0] = (u8)(TPD_CONFIG_REG_BASE >> 8);
    cfg_buf[1] = (u8)TPD_CONFIG_REG_BASE;

    memcpy(&cfg_buf[2], cfg_data, TPD_CONFIG_SIZE);

    ret = gtp_i2c_write(client, cfg_buf, TPD_CONFIG_SIZE + GTP_ADDR_LENGTH);

    gtp_i2c_end_cmd(client);

    return ret;
}

static s32  gtp_read_version(struct i2c_client *client, u16 *version)
{
    s32 ret = -1;
    u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};

    ret = gtp_i2c_read(client, buf, 6);
    gtp_i2c_end_cmd(client);

    if (ret < 0)
    {
        TPD_DWN_ERROR("GTP read version failed");
        return ret;
    }

    if (version)
    {
        *version = (buf[5] << 8) | buf[4];
    }

    TPD_DWN_DMESG("IC VERSION:%02x%02x_%02x%02x\n", buf[3], buf[2], buf[5], buf[4]);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
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

#if 0
static u8 gup_clear_mix_flag(struct i2c_client *client)
{
    s32 i = 0;
    u8 buf[3];

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
#endif
#if 0
static u8 gup_get_ic_fw_msg(struct i2c_client *client)
{
    s32 ret = 0;
    s32 i = 0;
    u8 buf[32];

    if (fail == gup_clear_mix_flag(client))
    {
        return fail;
    }

    //Get the mask version in rom of IC
    if (fail == gup_get_ic_msg(client, READ_MSK_VER_ADDR, buf, 4))
    {
        TPD_DWN_ERROR("Read mask version failed!\n");
        return fail;
    }

    memcpy(update_msg.ic_fw_msg.msk_ver, &buf[GTP_ADDR_LENGTH], 4);
    TPD_DWN_DEBUG("IC The mask version in rom is %c%c%c%c.\n",
                  update_msg.ic_fw_msg.msk_ver[0], update_msg.ic_fw_msg.msk_ver[1],
                  update_msg.ic_fw_msg.msk_ver[2], update_msg.ic_fw_msg.msk_ver[3]);

    //Get the firmware msg in IC, include firmware version and checksum flag
    for (i = 0; i < 2; i++)
    {
        if (fail == gup_get_ic_msg(client, READ_FW_MSG_ADDR, buf, 4))
        {
            TPD_DWN_ERROR("Get firmware msg in IC error.\n");
            return fail;
        }

        update_msg.force_update = buf[GTP_ADDR_LENGTH];

        if (i == 0 && update_msg.force_update == 0xAA)
        {
            TPD_DWN_ERROR("The check sum in ic is error.\n");
            TPD_DWN_ERROR("IC will be reset.\n");
            TPD_DWN_ERROR("If the check sum is still error, ");
            TPD_DWN_ERROR("The IC will be updated by force.\n");

            gtp_reset_guitar(10);
            continue;
            //msleep(100);
        }

        break;
    }

    //ic_fw_msg.type = buf[GTP_ADDR_LENGTH + 1];
    update_msg.ic_fw_msg.version = buf[GTP_ADDR_LENGTH + 2] << 8 | buf[GTP_ADDR_LENGTH + 3];
    TPD_DWN_DEBUG("IC VID:0x%x", (int)update_msg.ic_fw_msg.version);
    TPD_DWN_DEBUG("IC force update:%x", update_msg.force_update);

    //Cuts the frequency
    buf[0] = 0x15;
    buf[1] = 0x22;
    buf[2] = 0x18;
    ret =  gtp_i2c_write(client, buf, 3);

    if (ret <= 0)
    {
        return fail;
    }

    gtp_i2c_end_cmd(client);

    //Get the pid at 0x4011 in nvram
    if (fail == gup_get_ic_msg(client, 0x4011, buf, 1))
    {
        TPD_DWN_ERROR("Read pid failed!\n");
        return fail;
    }

    update_msg.ic_fw_msg.type = buf[GTP_ADDR_LENGTH];

    TPD_DWN_DEBUG("IC PID:%x", update_msg.ic_fw_msg.type);

    return success;
}
#endif
/*
* Steps of reset guitar
*1. INT output low£¬delay 5ms
*2. RESET output low 100ms£¬transfer to input w/o pull-up or pull-down
*3. I2C access GUITAR
*4. delay 100ms to read register 0xff
*5. success until the return value of register 0xff is 0x55
*/
s32 gt818_enter_update_mode(struct i2c_client *client)
{
    int ret = 1;
    u8 retry;
    unsigned char inbuf[3] = {0, 0xff, 0};

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
            //step 4
            msleep(100);
            ret = gtp_i2c_read(client, inbuf, 3);

            if (ret > 0)
            {
                TPD_DWN_DEBUG("The value of 0x00ff is 0x%02x", inbuf[2]);

                //step 5
                if (inbuf[2] == 0x55)
                {
                    return success;
                }
            }
        }

        msleep(40);
    }

    TPD_DWN_ERROR(KERN_INFO"Detect address %0X", client->addr);

    return fail;
}

void gt818_leave_update_mode(void)
{
    GTP_GPIO_AS_INT(GPIO_CTP_EINT_PIN);
}

/*******************************************************
Function:
	Check chip version function.

Input:
  sw_ver:vendor id.

Output:
	Executive outcomes.
*********************************************************/
static s16 gup_check_version(u16 sw_ver)
{
    if ((sw_ver & 0xff) < TPD_CHIP_VERSION_C_FIRMWARE_BASE)
    {
        return TPD_GT818_VERSION_B;
    }
    else if ((sw_ver & 0xff) < TPD_CHIP_VERSION_D1_FIRMWARE_BASE)
    {
        return TPD_GT818_VERSION_C;
    }
    else if ((sw_ver & 0xff) < TPD_CHIP_VERSION_E_FIRMWARE_BASE)
    {
        return TPD_GT818_VERSION_D1;
    }
    else if ((sw_ver & 0xff) < TPD_CHIP_VERSION_D2_FIRMWARE_BASE)
    {
        return TPD_GT818_VERSION_E;
    }
    else
    {
        return TPD_GT818_VERSION_D2;
    }
}

static s32 gup_comfirm_version(struct i2c_client *client)
{
    s32 i = 0;
    s32 count = 0;
    u16 version;
    u16 version_tmp;

    gtp_read_version(client, &version);
    gtp_read_version(client, &version_tmp);

    for (i = 0; i < 15; i++)
    {
        if (version != version_tmp)
        {
            count = 0;
        }
        else
        {
            count ++;

            if (count > 10)
            {
                update_msg.version = version;

                return success;
            }
        }

        version = version_tmp;
        gtp_read_version(client, &version_tmp);
    }

    return fail;
}

static u8 gup_load_update_file(struct i2c_client *client, st_fw_head *fw_head, u8 *data, u8 *path)
{
    //u8 mask_num = 0;
    u16 checksum = 0;
    int ret = 0;
    int i = 0;
    //u8 buf[FW_HEAD_LENGTH];

    if (path)
    {
        TPD_DWN_DEBUG("File path:%s, %d", path, strlen(path));
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

    //Read firmware head message in file
    ret = update_msg.file->f_op->read(update_msg.file, (u8 *)&update_msg.fw_msg,
                                      sizeof(st_check_msg), &update_msg.file->f_pos);

    if (ret < 0)
    {
        TPD_DWN_ERROR("Read firmware body error.\n");
        goto load_failed;
    }

    //1. Compare magic numbers
    if ((update_msg.fw_msg.magic_number_1 != MAGIC_NUMBER_1)
            || (update_msg.fw_msg.magic_number_2 != MAGIC_NUMBER_2))
    {
        TPD_DWN_ERROR("Magic number not match, Exit.\n");
        TPD_DWN_DMESG("[FW]magic number:0x%08x&0x%08x\n", update_msg.fw_msg.magic_number_1, update_msg.fw_msg.magic_number_2);
        TPD_DWN_DMESG("[DV]magic number:0x%08x&0x%08x\n", MAGIC_NUMBER_1, MAGIC_NUMBER_2);
        goto load_failed;
    }

    //2. Compare version
    if (1)
    {
        u16 fw_version = ((update_msg.fw_msg.version << 8) & 0xff00) | ((update_msg.fw_msg.version >> 8) & 0xff);
        u16 ic_version = ((update_msg.version << 8) & 0xff00) | ((update_msg.version >> 8) & 0xff);

        if (ic_version >= fw_version)
        {
            TPD_DWN_ERROR("The firmware version is lower than current.\n");
            goto load_failed;
        }
    }

    //3. Check the versions
    if (gup_check_version(update_msg.version) != gup_check_version(update_msg.fw_msg.version))
    {
        TPD_DWN_ERROR("Chip version not match,Exit.\n");
        goto load_failed;
    }

    //Read firmware body message in file
    ret = update_msg.file->f_op->read(update_msg.file, data, update_msg.fw_msg.length,
                                      &update_msg.file->f_pos);

    if (ret < 0)
    {
        TPD_DWN_ERROR("Read firmware head error.\n");
        goto load_failed;
    }

    //.4 Check check sum
    for (i = 0; i < update_msg.fw_msg.length; i++)
    {
        checksum += data[i];
    }

    if (checksum != update_msg.fw_msg.checksum)
    {
        TPD_DWN_ERROR("Checksum not match,Exit.\n");
        goto load_failed;
    }

    TPD_DWN_DEBUG("Load data successfully!\n");

    fw_head->lenth = update_msg.fw_msg.length;
    fw_head->st_addr[0] = 0x41;
    fw_head->st_addr[1] = 0x00;

    set_fs(update_msg.old_fs);
    filp_close(update_msg.file, NULL);
    return success;

load_failed:
    set_fs(update_msg.old_fs);
    filp_close(update_msg.file, NULL);
    return fail;
}

static u8 gup_load_update_header(struct i2c_client *client, st_fw_head *fw_head, u8 **data)
{
    u16 checksum = 0;
//    s32 ret = -1;
    s32 i = 0;
    u8 *pos = NULL;

    pos = HEADER_UPDATE_DATA;

    //Read firmware head message in file
    memcpy((u8 *)&update_msg.fw_msg, pos, sizeof(st_check_msg));

    //1. Compare magic numbers
    if ((update_msg.fw_msg.magic_number_1 != MAGIC_NUMBER_1)
            || (update_msg.fw_msg.magic_number_2 != MAGIC_NUMBER_2))
    {
        TPD_DWN_ERROR("Magic number not match, Exit.\n");
        TPD_DWN_DMESG("[FW]magic number:0x%08x&0x%08x", update_msg.fw_msg.magic_number_1, update_msg.fw_msg.magic_number_2);
        TPD_DWN_DMESG("[DV]magic number:0x%08x&0x%08x", MAGIC_NUMBER_1, MAGIC_NUMBER_2);
        goto load_failed;
    }

    TPD_DWN_DMESG("Magic number match.\n");

    //2. Compare version
    if (1)
    {
        u16 fw_version = ((update_msg.fw_msg.version << 8) & 0xff00) | ((update_msg.fw_msg.version >> 8) & 0xff);
        u16 ic_version = ((update_msg.version << 8) & 0xff00) | ((update_msg.version >> 8) & 0xff);

        if (ic_version >= fw_version)
        {
            TPD_DWN_ERROR("The firmware version is lower than current.\n");
            goto load_failed;
        }
    }

    //3. Check the versions
    if (gup_check_version(update_msg.version) != gup_check_version(update_msg.fw_msg.version))
    {
        TPD_DWN_ERROR("Chip version not match,Exit.\n");
        goto load_failed;
    }

    TPD_DWN_DMESG("Version match.\n");

    //Read firmware body message in file
    *data = pos + sizeof(st_check_msg);

    //.4 Check check sum
    for (i = 0; i < update_msg.fw_msg.length; i++)
    {
        checksum += (*data)[i];
    }

    if (checksum != update_msg.fw_msg.checksum)
    {
        TPD_DWN_ERROR("Checksum not match,Exit.\n");
        goto load_failed;
    }

    TPD_DWN_DMESG("Check sum match.\n");

    TPD_DWN_DEBUG("Load data successfully!\n");

    fw_head->lenth = update_msg.fw_msg.length;
    fw_head->st_addr[0] = 0x41;
    fw_head->st_addr[1] = 0x00;
    return success;

load_failed:
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

    // gup_clear_mix_flag(ts);
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
                TPD_DWN_ERROR("Store nvram failed.\n");
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
//    s32 i, j;
//    s32 ret;
//    s32 right_count = 0;
    u32 status = 0;
//    u16 check_sum = 0;
//    u8 buf[150];
//    u8  tmp[150];
#if 0
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
#endif

    for (retry = 0; retry < 10; retry++)
    {
#if 0

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

                            TPD_DWN_DEBUG("check sum of solidified config is %04x", check_sum);
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
                        TPD_DWN_DEBUG("Compare solidified config failed!\n");
                        return fail;
                    }
                }
            }
        }

#endif

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
                break;
            }
        }

#if 0

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

            TPD_DWN_DEBUG("FILE chksum after addition:0x%02x%02x%02x", fw_head->chk_sum[0],
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

#endif
    }

    if (retry >= 10)
    {
        return fail;
    }
    else
    {
        return success;
#if 0

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
            TPD_DWN_ERROR("Read address at 0x00ff error!\t retry:%d\n", retry);
            continue;
        }

        if (0xcc == buf[GTP_ADDR_LENGTH])
        {
            return success;
        }
        else
        {
            TPD_DWN_ERROR("The value of 0x00ff: 0x%02x!\t retry:%d\n", buf[GTP_ADDR_LENGTH], retry);
            continue;
        }

#endif
    }

    TPD_DWN_ERROR("The value of 0x00ff error.\n");
    return fail;
}

s32 gt818_update_proc(void *dir)
{
    s32 ret;
    u32 retry = 100;
    u32 i = 0;
    // struct goodix_ts_data* ts = NULL;
    u8 *data = NULL;
    u8 *ic_nvram = NULL;
    st_fw_head fw_head;
    u8 buf[32];

    gt818_show_len = 20;
    gt818_total_len = 100;

    // ts = i2c_get_clientdata(guitar_client);

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
        //    gup_get_ic_fw_msg(guitar_client);
        //    gtp_reset_guitar(10);
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

    gt818_show_len = 40;
    gt818_total_len = 100;

    //  ts->enter_update = 1;

    for (i = 0; i < 5; i++)
    {
        if (fail == gt818_enter_update_mode(guitar_client))
        {
            TPD_DWN_ERROR("Next try![Enter update mode]");
            continue;
        }
        else
        {
            TPD_DWN_DEBUG("Set update mode successfully.\n");
            break;
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
            TPD_DWN_ERROR("Read NVRCS failed.(Store)");
            continue;
        }

        if (buf[GTP_ADDR_LENGTH] & 0x01)
        {
            TPD_DWN_ERROR("Check NVRCS(0x%02x) failed.(Store)", buf[GTP_ADDR_LENGTH]);
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
            TPD_DWN_ERROR("Read NVRCS failed.(Recall)");
            continue;
        }

        if (buf[GTP_ADDR_LENGTH] & 0x02)
        {
            TPD_DWN_ERROR("Check NVRCS(0x%02x) failed.(Recall)", buf[GTP_ADDR_LENGTH]);
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

    gt818_leave_update_mode();
    TPD_DWN_DEBUG("Leave update mode!\n");

    //Reset guitar
    TPD_DWN_DEBUG("Reset IC and send config!\n");
    gtp_reset_guitar(10);
    msleep(50);
    ret = gtp_send_cfg(guitar_client);

    if (ret < 0)
    {
        TPD_DWN_ERROR("Send config data failed.\n");
    }

    msleep(50);
    //  ts->enter_update = 0;

load_failed:
    kfree(ic_nvram);
app_mem_failed:

    if (!(HEADER_UPDATE_DATA != NULL && dir == NULL))
    {
        kfree(data);
    }

    if (retry < 5)
    {
        gt818_show_len = 100;
        return success;
    }

    gt818_show_len = 200;
    TPD_DWN_ERROR("Update failed!\n");
    return fail;
}

u8 gt818_downloader(struct i2c_client *client)
{
    struct task_struct *thread = NULL;

    if (fail == gup_comfirm_version(client))
    {
        TPD_DWN_ERROR("Comfirm version fail.\n");
        TPD_DWN_ERROR("Update thread won't be created.\n");

        return -1;
    }

    TPD_DWN_DEBUG("Comfirm version successfully!\n");
    guitar_client = client;

    thread = kthread_run(gt818_update_proc, (void *)NULL, "guitar_update");

    if (IS_ERR(thread))
    {
        TPD_DWN_ERROR("Failed to create update thread.\n");
        return -1;
    }

    return 0;
}


//#endif   //endif GTP_AUTO_UPDATE
//******************************End of firmware update surpport*******************************
