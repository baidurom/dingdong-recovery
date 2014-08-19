/* drivers/input/touchscreen/goodix_tool.c
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
 *        V1.2:2012/06/08,modify some warning.
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

//#define IC_TYPE_NAME        "GT813" //Default
#define GTP_ADDR_LENGTH     2
#define DATA_LENGTH_UINT    512
#define CMD_HEAD_LENGTH     (sizeof(st_cmd_head) - sizeof(u8*))
#define GOODIX_ENTRY_NAME   "goodix_tool"

#define GTP_INFO(fmt,arg...)           printk("<<-GTP-INFO->> "fmt"\n",##arg)
#define GTP_ERROR(fmt,arg...)          printk("<<-GTP-ERROR->> "fmt"\n",##arg)
#define GTP_DEBUG_ON
#ifdef GTP_DEBUG_ON
#define GTP_DEBUG(fmt,arg...)          printk("<<-GTP-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg)
#else
#define GTP_DEBUG(fmt,arg...)
#endif
#define MAX_TRANSACTION_LENGTH         8
#define I2C_DEVICE_ADDRESS_LEN         2
#define MAX_I2C_TRANSFER_SIZE          (MAX_TRANSACTION_LENGTH - I2C_DEVICE_ADDRESS_LEN)
#define MAX_I2C_MAX_TRANSFER_SIZE      8
#define GTP_DRIVER_VERSION "V1.0<2012/06.18>"

#define FAIL                    0
#define SUCCESS                 1
#define false                   0
#define true                    1

u16 show_len;
u16 total_len;
extern u16 gt818_show_len;
extern u16 gt818_total_len;
extern u16 gt818x_show_len;
extern u16 gt818x_total_len;
extern u8 chip_type;
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
#define UPDATE_FUNCTIONS
#ifdef UPDATE_FUNCTIONS
extern s32 gt818_enter_update_mode(struct i2c_client *client);
extern s32 gt818x_enter_update_mode(struct i2c_client *client);
extern void gt818_leave_update_mode(void);
extern void gt818x_leave_update_mode(void);
extern s32 gt818_update_proc(void *dir);
extern s32 gt818x_update_proc(void *dir);
#endif

#pragma pack(1)
typedef struct
{
    u8  wr;         //write read flag£¬0:R  1:W  2:PID 3:
    u8  flag;       //0:no need flag/int 1: need flag  2:need int
    u8 flag_addr[2];  //flag address
    u8  flag_val;   //flag val
    u8  flag_relation;  //flag_val:flag 0:not equal 1:equal 2:> 3:<
    u16 circle;     //polling cycle
    u8  times;      //plling times
    u8  retry;      //I2C retry times
    u16 delay;      //delay befor read or after write
    u16 data_len;   //data length
    u8  addr_len;   //address length
    u8  addr[2];    //address
    u8  res[3];     //reserved
    u8 *data;       //data pointer
} st_cmd_head;
#pragma pack()
st_cmd_head cmd_head;

static struct i2c_client *gt_client = NULL;

static struct proc_dir_entry *goodix_proc_entry;

static s32 goodix_tool_write(struct file *filp, const char __user *buff, unsigned long len, void *data);
static s32 goodix_tool_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static s32(*tool_i2c_read)(u8 *, u16);
static s32(*tool_i2c_write)(u8 *, u16);

s32 DATA_LENGTH = 0;
s8 IC_TYPE[16] = {0};

static int i2c_enable_commands(struct i2c_client *client, u16 addr)
{
    u8 retry;
    u8 txbuf[2] = {0};

    if (txbuf == NULL)
        return -1;

    txbuf[0] = (addr >> 8) & 0xFF;
    txbuf[1] = addr & 0xFF;

    client->addr = client->addr & I2C_MASK_FLAG;// | I2C_ENEXT_FLAG;

    retry = 0;

    while (i2c_master_send(client, &txbuf[0], I2C_DEVICE_ADDRESS_LEN) < 0)
    {
        retry++;

        if (retry == 5)
        {
            client->addr = client->addr & I2C_MASK_FLAG;
            //TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr, I2C_DEVICE_ADDRESS_LEN);
            return -1;
        }
    }

    return 0;
}


static int i2c_write_bytes(struct i2c_client *client, u16 addr, u8 *txbuf, int len)
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u16 left = len;
    u16 offset = 0;
    u8 retry = 0;

    struct i2c_msg msg =
    {
        .addr = client->addr & I2C_MASK_FLAG,
        .flags = 0,
        .buf = buffer
    };


    if (txbuf == NULL)
        return -1;

    TPD_DEBUG("i2c_write_bytes to device %02X address %04X len %d\n", client->addr, addr, len);

    while (left > 0)
    {
        retry = 0;

        //addr = addr + offset;

        buffer[0] = ((addr + offset) >> 8) & 0xFF;
        buffer[1] = (addr + offset) & 0xFF;

        if (left > MAX_I2C_TRANSFER_SIZE)
        {
            memcpy(&buffer[I2C_DEVICE_ADDRESS_LEN], &txbuf[offset], MAX_I2C_TRANSFER_SIZE);
            msg.len = MAX_TRANSACTION_LENGTH;
            left -= MAX_I2C_TRANSFER_SIZE;
            offset += MAX_I2C_TRANSFER_SIZE;
        }
        else
        {
            memcpy(&buffer[I2C_DEVICE_ADDRESS_LEN], &txbuf[offset], left);
            msg.len = left + I2C_DEVICE_ADDRESS_LEN;
            left = 0;
        }

        TPD_DEBUG("byte left %d offset %d\n", left, offset);

        while (i2c_transfer(client->adapter, &msg, 1) != 1)
        {
            retry++;

            if (retry == 5)
            {
                TPD_DEBUG("I2C write 0x%X%X length=%d failed\n", buffer[0], buffer[1], len);
                return -1;
            }
            else
                TPD_DEBUG("I2C write retry %d addr 0x%X%X\n", retry, buffer[0], buffer[1]);
        }
    }

    return 1;
}


static int i2c_read_bytes(struct i2c_client *client, u16 addr, u8 *rxbuf, int len)
{
    u8 retry;
    u8 addrBuffer[2] = {0};
    u16 left = len;
    u16 offset = 0;

    if (rxbuf == NULL)
        return -1;

    TPD_DEBUG("i2c_read_bytes to device %02X address %04X len %d\n", client->addr, addr, len);

    while (left > 0)
    {
        if (left > MAX_I2C_MAX_TRANSFER_SIZE)
        {
            //addr = addr + offset;

            addrBuffer[0] = ((addr + offset) >> 8) & 0xFF;
            addrBuffer[1] = (addr + offset) & 0xFF;

            client->addr = client->addr & I2C_MASK_FLAG;

            retry = 0;

            while (i2c_master_send(client, &addrBuffer[0], I2C_DEVICE_ADDRESS_LEN) < 0)
            {
                retry++;

                if (retry == 5)
                {
                    client->addr = client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, I2C_DEVICE_ADDRESS_LEN);
                    return -1;
                }
            }

            retry = 0;

            while (i2c_master_recv(client, &rxbuf[offset], MAX_I2C_MAX_TRANSFER_SIZE) < 0)
            {
                retry++;

                if (retry == 5)
                {
                    client->addr = client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, MAX_I2C_MAX_TRANSFER_SIZE);
                    return -1;
                }
            }

            left -= MAX_I2C_MAX_TRANSFER_SIZE;
            offset += MAX_I2C_MAX_TRANSFER_SIZE;
        }
        else
        {
            //addr = addr + offset;

            addrBuffer[0] = ((addr + offset) >> 8) & 0xFF;
            addrBuffer[1] = (addr + offset) & 0xFF;

            client->addr = client->addr & I2C_MASK_FLAG;

            retry = 0;

            while (i2c_master_send(client, &addrBuffer[0], I2C_DEVICE_ADDRESS_LEN) < 0)
            {
                retry++;

                if (retry == 5)
                {
                    client->addr = client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C write 0x%X length=%d failed\n", addr + offset, I2C_DEVICE_ADDRESS_LEN);
                    return -1;
                }
            }

            retry = 0;

            while (i2c_master_recv(client, &rxbuf[offset], left) < 0)
            {
                retry++;

                if (retry == 5)
                {
                    client->addr = client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, left);
                    return -1;
                }
            }

            offset += left;
            left = 0;
        }
    }

    client->addr = client->addr & I2C_MASK_FLAG;

    return 2;
}


static s32 tool_i2c_read_no_extra(u8 *buf, u16 len)
{
    return i2c_read_bytes(gt_client, buf[0] << 8 | buf[1], &buf[2], len);
}

static s32 tool_i2c_write_no_extra(u8 *buf, u16 len)
{
    int ret = 1;

    if (len <= I2C_DEVICE_ADDRESS_LEN)
    {
        if (i2c_enable_commands(gt_client, buf[0] << 8 | buf[1]))
        {
            ret = -1;
        }
    }
    else
    {
        ret = i2c_write_bytes(gt_client, buf[0] << 8 | buf[1], &buf[2], len - I2C_DEVICE_ADDRESS_LEN);
    }

    return ret;
}

static s32 tool_i2c_read_with_extra(u8 *buf, u16 len)
{
    s32 ret = -1;

    i2c_enable_commands(gt_client, 0x0fff);
    ret = tool_i2c_read_no_extra(buf, len);
    i2c_enable_commands(gt_client, 0x8000);

    return ret;
}

static s32 tool_i2c_write_with_extra(u8 *buf, u16 len)
{
    s32 ret = -1;

    i2c_enable_commands(gt_client, 0x0fff);
    ret = tool_i2c_write_no_extra(buf, len);
    i2c_enable_commands(gt_client, 0x8000);

    return ret;
}

static void register_i2c_func(void)
{
//    if (!strncmp(IC_TYPE, "GT818", 5) || !strncmp(IC_TYPE, "GT816", 5)
//        || !strncmp(IC_TYPE, "GT811", 5) || !strncmp(IC_TYPE, "GT818F", 6)
//        || !strncmp(IC_TYPE, "GT827", 5) || !strncmp(IC_TYPE,"GT828", 5)
//        || !strncmp(IC_TYPE, "GT813", 5))
    if (strncmp(IC_TYPE, "GT8110", 6) && strncmp(IC_TYPE, "GT8105", 6)
            && strncmp(IC_TYPE, "GT801", 5) && strncmp(IC_TYPE, "GT800", 5)
            && strncmp(IC_TYPE, "GT801PLUS", 9) && strncmp(IC_TYPE, "GT811", 5)
            && strncmp(IC_TYPE, "GTxxx", 5))
    {
        tool_i2c_read = tool_i2c_read_with_extra;
        tool_i2c_write = tool_i2c_write_with_extra;
        GTP_DEBUG("I2C function: with pre and end cmd!");
    }
    else
    {
        tool_i2c_read = tool_i2c_read_no_extra;
        tool_i2c_write = tool_i2c_write_no_extra;
        GTP_INFO("I2C function: without pre and end cmd!");
    }
}

static void unregister_i2c_func(void)
{
    tool_i2c_read = NULL;
    tool_i2c_write = NULL;
    GTP_INFO("I2C function: unregister i2c transfer function!");
}


s32 init_wr_node(struct i2c_client *client)
{
    s32 i;

    gt_client = client;
    memset(&cmd_head, 0, sizeof(cmd_head));
    cmd_head.data = NULL;

    i = 5;

    while ((!cmd_head.data) && i)
    {
        cmd_head.data = kzalloc(i * DATA_LENGTH_UINT, GFP_KERNEL);

        if (NULL != cmd_head.data)
        {
            break;
        }

        i--;
    }

    if (i)
    {
        DATA_LENGTH = i * DATA_LENGTH_UINT + GTP_ADDR_LENGTH;
        GTP_INFO("Applied memory size:%d.", DATA_LENGTH);
    }
    else
    {
        GTP_ERROR("Apply for memory failed.");
        return FAIL;
    }

    cmd_head.addr_len = 2;
    cmd_head.retry = 5;

    register_i2c_func();

    goodix_proc_entry = create_proc_entry(GOODIX_ENTRY_NAME, 0666, NULL);

    if (goodix_proc_entry == NULL)
    {
        GTP_ERROR("Couldn't create proc entry!");
        return FAIL;
    }
    else
    {
        GTP_INFO("Create proc entry success!");
        goodix_proc_entry->write_proc = goodix_tool_write;
        goodix_proc_entry->read_proc = goodix_tool_read;
    }

    return SUCCESS;
}

void uninit_wr_node(void)
{
    kfree(cmd_head.data);
    cmd_head.data = NULL;
    unregister_i2c_func();
    remove_proc_entry(GOODIX_ENTRY_NAME, NULL);
}

static u8 relation(u8 src, u8 dst, u8 rlt)
{
    u8 ret = 0;

    switch (rlt)
    {
        case 0:
            ret = (src != dst) ? true : false;
            break;

        case 1:
            ret = (src == dst) ? true : false;
            GTP_DEBUG("equal:src:0x%02x   dst:0x%02x   ret:%d\n", src, dst, (s32)ret);
            break;

        case 2:
            ret = (src > dst) ? true : false;
            break;

        case 3:
            ret = (src < dst) ? true : false;
            break;

        case 4:
            ret = (src & dst) ? true : false;
            break;

        case 5:
            ret = (!(src | dst)) ? true : false;
            break;

        default:
            ret = false;
            break;
    }

    return ret;
}

/*******************************************************
Function:
	Comfirm function.
Input:
  None.
Output:
	Return write length.
********************************************************/
static u8 comfirm(void)
{
    s32 i = 0;
    u8 buf[32];

//    memcpy(&buf[GTP_ADDR_LENGTH - cmd_head.addr_len], &cmd_head.flag_addr, cmd_head.addr_len);
//    memcpy(buf, &cmd_head.flag_addr, cmd_head.addr_len);//Modified by Scott, 2012-02-17
    memcpy(buf, cmd_head.flag_addr, cmd_head.addr_len);

    for (i = 0; i < cmd_head.times; i++)
    {
        if (tool_i2c_read(buf, 1) <= 0)
        {
            GTP_ERROR("Read flag data failed!");
            return FAIL;
        }

        if (true == relation(buf[GTP_ADDR_LENGTH], cmd_head.flag_val, cmd_head.flag_relation))
        {
            GTP_DEBUG("value at flag addr:0x%02x\n", buf[GTP_ADDR_LENGTH]);
            GTP_DEBUG("flag value:0x%02x\n", cmd_head.flag_val);
            break;
        }

        msleep(cmd_head.circle);
    }

    if (i >= cmd_head.times)
    {
        GTP_ERROR("Didn't get the flag to continue!");
        return FAIL;
    }

    return SUCCESS;
}

/*******************************************************
Function:
	Goodix tool write function.
Input:
  standard proc write function param.
Output:
	Return write length.
********************************************************/
static s32 goodix_tool_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
{
    u64 ret = 0;
//    GTP_DEBUG_FUNC();
//   GTP_DEBUG_ARRAY((u8*)buff, len);

    ret = copy_from_user(&cmd_head, buff, CMD_HEAD_LENGTH);

    if (ret)
    {
        GTP_ERROR("copy_from_user failed.");
    }

    GTP_DEBUG("wr  :0x%02x\n", cmd_head.wr);
    GTP_DEBUG("flag:0x%02x\n", cmd_head.flag);
    GTP_DEBUG("flag addr:0x%02x%02x\n", cmd_head.flag_addr[0], cmd_head.flag_addr[1]);
    GTP_DEBUG("flag val:0x%02x\n", cmd_head.flag_val);
    GTP_DEBUG("flag rel:0x%02x\n", cmd_head.flag_relation);
    GTP_DEBUG("circle  :%d\n", (s32)cmd_head.circle);
    GTP_DEBUG("times   :%d\n", (s32)cmd_head.times);
    GTP_DEBUG("retry   :%d\n", (s32)cmd_head.retry);
    GTP_DEBUG("delay   :%d\n", (s32)cmd_head.delay);
    GTP_DEBUG("data len:%d\n", (s32)cmd_head.data_len);
    GTP_DEBUG("addr len:%d\n", (s32)cmd_head.addr_len);
    GTP_DEBUG("addr:0x%02x%02x\n", cmd_head.addr[0], cmd_head.addr[1]);
    GTP_DEBUG("len:%d\n", (s32)len);
    GTP_DEBUG("buf[20]:0x%02x\n", buff[CMD_HEAD_LENGTH]);

    if (1 == cmd_head.wr)
    {
        //  copy_from_user(&cmd_head.data[cmd_head.addr_len], &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
        ret = copy_from_user(&cmd_head.data[GTP_ADDR_LENGTH], &buff[CMD_HEAD_LENGTH], cmd_head.data_len);

        if (ret)
        {
            GTP_ERROR("copy_from_user failed.");
        }

        memcpy(&cmd_head.data[GTP_ADDR_LENGTH - cmd_head.addr_len], cmd_head.addr, cmd_head.addr_len);

        // GTP_DEBUG_ARRAY(cmd_head.data, cmd_head.data_len + cmd_head.addr_len);
        //   GTP_DEBUG_ARRAY((u8*)&buff[CMD_HEAD_LENGTH], cmd_head.data_len);

        if (1 == cmd_head.flag)
        {
            if (FAIL == comfirm())
            {
                GTP_ERROR("[WRITE]Comfirm fail!");
                return FAIL;
            }
        }
        else if (2 == cmd_head.flag)
        {
            //Need interrupt!
        }

        if (tool_i2c_write(&cmd_head.data[GTP_ADDR_LENGTH - cmd_head.addr_len],
                           cmd_head.data_len + cmd_head.addr_len) <= 0)
        {
            GTP_ERROR("[WRITE]Write data failed!");
            return FAIL;
        }

        // GTP_DEBUG_ARRAY(&cmd_head.data[GTP_ADDR_LENGTH - cmd_head.addr_len],cmd_head.data_len + cmd_head.addr_len);
        if (cmd_head.delay)
        {
            msleep(cmd_head.delay);
        }

        return cmd_head.data_len + CMD_HEAD_LENGTH;
    }
    else if (3 == cmd_head.wr)  //Write ic type
    {
        memcpy(IC_TYPE, cmd_head.data, cmd_head.data_len);
        register_i2c_func();

        return cmd_head.data_len + CMD_HEAD_LENGTH;
    }
    else if (5 == cmd_head.wr)
    {
        //memcpy(IC_TYPE, cmd_head.data, cmd_head.data_len);

        return cmd_head.data_len + CMD_HEAD_LENGTH;
    }
    else if (7 == cmd_head.wr)//disable irq!
    {
        mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

        return CMD_HEAD_LENGTH;
    }
    else if (9 == cmd_head.wr) //enable irq!
    {
        mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

        return CMD_HEAD_LENGTH;
    }

#ifdef UPDATE_FUNCTIONS
    else if (11 == cmd_head.wr)//Enter update mode!
    {
        if (chip_type)
        {
            ret = gt818x_enter_update_mode(gt_client);
        }
        else
        {
            ret = gt818_enter_update_mode(gt_client);
        }

        if (FAIL == ret)
        {
            return ret;
        }
    }
    else if (13 == cmd_head.wr)//Leave update mode!
    {
        if (chip_type)
        {
            gt818x_leave_update_mode();
        }
        else
        {
            gt818_leave_update_mode();
        }
    }
    else if (15 == cmd_head.wr) //Update firmware!
    {
        memset(cmd_head.data, 0, cmd_head.data_len + 1);
        memcpy(cmd_head.data, &buff[CMD_HEAD_LENGTH], cmd_head.data_len);

        if (chip_type)
        {
            ret = gt818x_update_proc((void *)cmd_head.data);
        }
        else
        {
            ret = gt818_update_proc((void *)cmd_head.data);
        }

        if (FAIL == ret)
        {
            return ret;
        }

    }

#endif

    return CMD_HEAD_LENGTH;
}

/*******************************************************
Function:
	Goodix tool read function.
Input:
  standard proc read function param.
Output:
	Return read length.
********************************************************/
static s32 goodix_tool_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
//    GTP_DEBUG_FUNC();

    if (cmd_head.wr % 2)
    {
        return FAIL;
    }
    else if (!cmd_head.wr)
    {
        u16 len = 0;
        s16 data_len = 0;
        u16 loc = 0;

        if (1 == cmd_head.flag)
        {
            if (FAIL == comfirm())
            {
                GTP_ERROR("[READ]Comfirm fail!");
                return FAIL;
            }
        }
        else if (2 == cmd_head.flag)
        {
            //Need interrupt!
        }

        memcpy(cmd_head.data, cmd_head.addr, cmd_head.addr_len);

        GTP_DEBUG("[CMD HEAD DATA] ADDR:0x%02x%02x\n", cmd_head.data[0], cmd_head.data[1]);
        GTP_DEBUG("[CMD HEAD ADDR] ADDR:0x%02x%02x\n", cmd_head.addr[0], cmd_head.addr[1]);

        if (cmd_head.delay)
        {
            msleep(cmd_head.delay);
        }

        data_len = cmd_head.data_len;

        while (data_len > 0)
        {
            if (data_len > DATA_LENGTH)
            {
                len = DATA_LENGTH;
            }
            else
            {
                len = data_len;
            }

            data_len -= DATA_LENGTH;

            if (tool_i2c_read(cmd_head.data, len) <= 0)
            {
                GTP_ERROR("[READ]Read data failed!");
                return FAIL;
            }

            memcpy(&page[loc], &cmd_head.data[GTP_ADDR_LENGTH], len);
            loc += len;

            //GTP_DEBUG_ARRAY(&cmd_head.data[GTP_ADDR_LENGTH], len);
            //    GTP_DEBUG_ARRAY(page, len);
        }
    }
    else if (2 == cmd_head.wr)
    {
        //    memcpy(page, "gt8", cmd_head.data_len);
        // memcpy(page, "GT818", 5);
        //  page[5] = 0;

        GTP_DEBUG("Return ic type:%s len:%d\n", page, (s32)cmd_head.data_len);
        return cmd_head.data_len;
        //return sizeof(IC_TYPE_NAME);
    }
    else if (4 == cmd_head.wr)
    {
        if (chip_type)
        {
            show_len = gt818x_show_len;
            total_len = gt818x_total_len;
        }
        else
        {
            show_len = gt818_show_len;
            total_len = gt818_total_len;
        }

        page[0] = show_len >> 8;
        page[1] = show_len & 0xff;
        page[2] = total_len >> 8;
        page[3] = total_len & 0xff;

        return cmd_head.data_len;
    }
    else if (6 == cmd_head.wr)
    {
        //Read error code!
    }
    else if (8 == cmd_head.wr)  //Read driver version
    {
        // memcpy(page, GTP_DRIVER_VERSION, strlen(GTP_DRIVER_VERSION));
        s32 tmp_len;
        tmp_len = strlen(GTP_DRIVER_VERSION);
        memcpy(page, GTP_DRIVER_VERSION, tmp_len);
        page[tmp_len] = 0;
    }

    return cmd_head.data_len;
}
