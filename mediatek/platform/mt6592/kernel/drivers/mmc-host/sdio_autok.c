#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>

#include "mt_sd.h"
#include "sdio_autok.h"

#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
// auto-k test [s]
extern int curstg;
// auto-k test [e]
#endif

static U_AUTOK_INTERFACE_DATA g_autok_data[MAX_AUTOK_DAT_NUM];
static unsigned int  g_autoK_range_against_temper = 0;

static unsigned int g_test_write_pattern[TUNING_TEST_TIME*TUNING_DATA_NO];
static unsigned int g_test_read_pattern[TUNING_TEST_TIME];

S_AUTOK_CKGEN_DATA autok_ckg_data[SCALE_CKGEN_MSDC_DLY_SEL];
S_AUTOK_CMD_DLY autok_cmd_cmdrrdly[SCALE_CMD_RSP_DLY_SEL];
S_AUTOK_CMD_DLY autok_cmd_ckgdly[SCALE_CKGEN_MSDC_DLY_SEL];

/*************************************************************************
* FUNCTION
*  msdc_autok_read
*
* DESCRIPTION
*  This function for auto-K, read from sdio device
*
* PARAMETERS
*    host: msdc host manipulator pointer
*    u4Addr: sdio device address
*    u4Func: sdio device function
*    pBuffer: content read from device
*    u4Len: read data length
*    u4Cmd: transferred cmd (cmd52/cmd53)
*
* RETURN VALUES
*    error code: refer to errno.h
*************************************************************************/
int msdc_autok_read(struct msdc_host *host, unsigned int u4Addr, unsigned int u4Func, void *pBuffer, unsigned int u4Len, unsigned int u4Cmd)
{
    int ret = 0;
    u8 *value = (u8 *) pBuffer;
    struct sdio_func *sdioFunc;

    if((pBuffer==NULL) || (host==NULL))
    {
        printk("[%s] pBuffer = 0x%x, host = 0x%x\n", __func__, (unsigned int)pBuffer, (unsigned int)host);
        return -1;
    }
        
    if( ((u4Cmd == 53) && (u4Len < 4)) ||
        ((u4Cmd == 52) && (u4Len > 1)) )
    {
        printk("[%s] u4Cmd = %d, u4Len = %d\n", __func__, u4Cmd, u4Len);
        return -1;
    }

    sdioFunc = host->mmc->card->sdio_func[u4Func - 1];

    sdio_claim_host(sdioFunc);
    if(u4Cmd == 53)
        ret = sdio_readsb(sdioFunc, pBuffer, u4Addr, u4Len);
    else if(u4Cmd == 52)
        *value = sdio_readb(sdioFunc, u4Addr, &ret);
    else
    {
        printk("[%s] Doesn't support u4Cmd = %d\n", __func__, u4Cmd);
        ret = -1;
    }
    sdio_release_host(sdioFunc);
    
//    printk("Isaac: host->error = %d\n", host->error);

    return ret;
}

/*************************************************************************
* FUNCTION
*  msdc_autok_write
*
* DESCRIPTION
*  This function for auto-K, write to sdio device
*
* PARAMETERS
*    host: msdc host manipulator pointer
*    u4Addr: sdio device address
*    u4Func: sdio device function
*    pBuffer: content write to device
*    u4Len: write data length
*    u4Cmd: transferred cmd (cmd52/cmd53)
*
* RETURN VALUES
*    error code: refer to errno.h
*************************************************************************/
int msdc_autok_write(struct msdc_host *host, unsigned int u4Addr, unsigned int u4Func, void *pBuffer, unsigned int u4Len, unsigned int u4Cmd)
{
    int ret = 0;
    u8 *value = (u8 *) pBuffer;
    struct sdio_func *sdioFunc;

    if((pBuffer==NULL) || (host==NULL))
    {
        printk("[%s] pBuffer = 0x%x, host = 0x%x\n", __func__, (unsigned int)pBuffer, (unsigned int)host);
        return -1;
    }
        
    if( ((u4Cmd == 53) && (u4Len < 4)) ||
        ((u4Cmd == 52) && (u4Len > 1)) )
    {
        printk("[%s] u4Cmd = %d, u4Len = %d\n", __func__, u4Cmd, u4Len);
        return -1;
    }

    sdioFunc = host->mmc->card->sdio_func[u4Func - 1];

    sdio_claim_host(sdioFunc);
    if(u4Cmd == 53)
        ret = sdio_writesb(sdioFunc, u4Addr, pBuffer, u4Len);
    else if(u4Cmd == 52)
        sdio_writeb(sdioFunc, *value, u4Addr, &ret);
    else
    {
        printk("[%s] Doesn't support u4Cmd = %d\n", __func__, u4Cmd);
        ret = -1;
    }
    sdio_release_host(sdioFunc);
    
//    printk("Isaac: host->error = %d\n", host->error);

    return ret;
}

/*************************************************************************
* FUNCTION
*  msdc_autok_adjust_param
*
* DESCRIPTION
*  This function for auto-K, adjust msdc parameter
*
* PARAMETERS
*    host: msdc host manipulator pointer
*    param: enum of msdc parameter
*    value: value of msdc parameter
*    rw: AUTOK_READ/AUTOK_WRITE
*
* RETURN VALUES
*    error code: 0 success, 
*               -1 parameter input error
*               -2 read/write fail            
*               -3 else error
*************************************************************************/
int msdc_autok_adjust_param(struct msdc_host *host, enum AUTOK_PARAM param, u32 *value, int rw)
{
    u32 base = host->base;
    u32 reg = 0;
    u32 field = 0;

    switch (param)
    {
        case CMD_EDGE:
            if((rw == AUTOK_WRITE) && (*value > 1))
            {
                printk("[%s] Input value(%d) for CMD_EDGE is out of range, it should be [0~1]\n", __func__, *value);
                return -1;
            }
            reg = (u32)(MSDC_IOCON);
            field = (u32)(MSDC_IOCON_RSPL);
            break;
        case RDATA_EDGE:
            if((rw == AUTOK_WRITE) && (*value > 1))
            {
                printk("[%s] Input value(%d) for RDATA_EDGE is out of range, it should be [0~1]\n", __func__, *value);
                return -1;
            }
            reg = (u32)(MSDC_IOCON);
            field = (u32)(MSDC_IOCON_DSPL);
            break;
        case WDATA_EDGE:
            if((rw == AUTOK_WRITE) && (*value > 1))
            {
                printk("[%s] Input value(%d) for WDATA_EDGE is out of range, it should be [0~1]\n", __func__, *value);
                return -1;
            }
            reg = (u32)(MSDC_IOCON);
            field = (u32)(MSDC_IOCON_W_DSPL);
            break;
        case CLK_DRV:
            if((rw == AUTOK_WRITE) && (*value > 7))
            {
                printk("[%s] Input value(%d) for CLK_DRV is out of range, it should be [0~7]\n", __func__, *value);
                return -1;
            }
                
            if(host->id != 2)
            {
                printk("[%s] MSDC%d doesn't support AUTO K\n", __func__, host->id);
                return -1;
            }

            reg = (u32)(MSDC2_GPIO_CLK_BASE);
            field = (u32)(GPIO_MSDC_DRVN);
            break;
        case CMD_DRV:
            if((rw == AUTOK_WRITE) && (*value > 7))
            {
                printk("[%s] Input value(%d) for CMD_DRV is out of range, it should be [0~7]\n", __func__, *value);
                return -1;
            }
            
            if(host->id != 2)
            {
                printk("[%s] MSDC%d doesn't support on AUTO K\n", __func__, host->id);
                return -1;
            }

            reg = (u32)(MSDC2_GPIO_CMD_BASE);
            field = (u32)(GPIO_MSDC_DRVN);
            break;
        case DAT_DRV:
            if((rw == AUTOK_WRITE) && (*value > 7))
            {
                printk("[%s] Input value(%d) for DAT_DRV is out of range, it should be [0~7]\n", __func__, *value);
                return -1;
            }
            
            if(host->id != 2)
            {
                printk("[%s] MSDC%d doesn't support on AUTO K\n", __func__, host->id);
                return -1;
            }

            reg = (u32)(MSDC2_GPIO_DAT_BASE);
            field = (u32)(GPIO_MSDC_DRVN);
            break;
        case DAT0_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for DAT0_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_DAT_RDDLY0);
            field = (u32)(MSDC_DAT_RDDLY0_D0);
            break;
        case DAT1_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for DAT1_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_DAT_RDDLY0);
            field = (u32)(MSDC_DAT_RDDLY0_D1);
            break;
        case DAT2_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for DAT2_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_DAT_RDDLY0);
            field = (u32)(MSDC_DAT_RDDLY0_D2);
            break;
        case DAT3_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for DAT3_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_DAT_RDDLY0);
            field = (u32)(MSDC_DAT_RDDLY0_D3);
            break;
        case DAT_WRD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for DAT_WRD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PAD_TUNE);
            field = (u32)(MSDC_PAD_TUNE_DATWRDLY);
            break;
        case DAT_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for DAT_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PAD_TUNE);
            field = (u32)(MSDC_PAD_TUNE_DATRRDLY);
            break;
        case CMD_RESP_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for CMD_RESP_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PAD_TUNE);
            field = (u32)(MSDC_PAD_TUNE_CMDRRDLY);
            break;
        case CMD_RD_DLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PAD_TUNE);
            field = (u32)(MSDC_PAD_TUNE_CMDRDLY);
            break;
        case DATA_DLYLINE_SEL:
            if((rw == AUTOK_WRITE) && (*value > 1))
            {
                printk("[%s] Input value(%d) for DATA_DLYLINE_SEL is out of range, it should be [0~1]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_IOCON);
            field = (u32)(MSDC_IOCON_DDLSEL);
            break;
        case READ_DATA_SMPL_SEL:
            if((rw == AUTOK_WRITE) && (*value > 1))
            {
                printk("[%s] Input value(%d) for READ_DATA_SMPL_SEL is out of range, it should be [0~1]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_IOCON);
            field = (u32)(MSDC_IOCON_DSPLSEL);
            break;
        case WRITE_DATA_SMPL_SEL:
            if((rw == AUTOK_WRITE) && (*value > 1))
            {
                printk("[%s] Input value(%d) for WRITE_DATA_SMPL_SEL is out of range, it should be [0~1]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_IOCON);
            field = (u32)(MSDC_IOCON_WDSPLSEL);
            break;
        case INT_DAT_LATCH_CK:
            if((rw == AUTOK_WRITE) && (*value > 7))
            {
                printk("[%s] Input value(%d) for INT_DAT_LATCH_CK is out of range, it should be [0~7]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PATCH_BIT0);
            field = (u32)(MSDC_INT_DAT_LATCH_CK_SEL);
            break;
        case CKGEN_MSDC_DLY_SEL:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for CKGEN_MSDC_DLY_SEL is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PATCH_BIT0);
            field = (u32)(MSDC_CKGEN_MSDC_DLY_SEL);
            break;
        case CMD_RSP_TA_CNTR:
            if((rw == AUTOK_WRITE) && (*value > 7))
            {
                printk("[%s] Input value(%d) for CMD_RSP_TA_CNTR is out of range, it should be [0~7]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PATCH_BIT1);
            field = (u32)(MSDC_PATCH_BIT1_CMD_RSP);
            break;
        case WRDAT_CRCS_TA_CNTR:
            if((rw == AUTOK_WRITE) && (*value > 7))
            {
                printk("[%s] Input value(%d) for WRDAT_CRCS_TA_CNTR is out of range, it should be [0~7]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PATCH_BIT1);
            field = (u32)(MSDC_PATCH_BIT1_WRDAT_CRCS);
            break;
        case PAD_CLK_TXDLY:
            if((rw == AUTOK_WRITE) && (*value > 31))
            {
                printk("[%s] Input value(%d) for PAD_CLK_TXDLY is out of range, it should be [0~31]\n", __func__, *value);
                return -1;
            }
            
            reg = (u32)(MSDC_PAD_TUNE);
            field = (u32)(MSDC_PAD_TUNE_CLKTXDLY);
            break;
        default:
            printk("[%s] Value of [enum AUTOK_PARAM param] is wrong\n", __func__);
            return -1;
    }

    if(rw == AUTOK_READ)
        sdr_get_field(reg, field, *value);
    else if(rw == AUTOK_WRITE)
        sdr_set_field(reg, field, *value);
    else
    {
        printk("[%s] Value of [int rw] is wrong\n", __func__);
        return -1;
    }

    return 0;
}

static E_RESULT_TYPE errMapping(struct msdc_host *host)
{

    E_RESULT_TYPE res= E_RESULT_PASS;

    switch(host->error)
    {
        case REQ_CMD_EIO:
            res = E_RESULT_CMD_CRC;
            break;
        case REQ_CMD_TMO:
            res = E_RESULT_TO;
            break;
        default:
            res = E_RESULT_ERR;
            break;
    }

    return res;
}

static void containGen(void)
{
    unsigned int i,j;

    unsigned int *pData = g_test_write_pattern;

    for(j=0; j<TUNING_DATA_NO; j++) {
        for(i=0; i<TUNING_TEST_TIME; i++) {
            *pData = tuning_data[j];
            pData++;
        }
    }
}

static E_RESULT_TYPE autok_write_test(struct msdc_host *host)
{
    int i;
    E_RESULT_TYPE res = E_RESULT_PASS;
    unsigned int reg;
    unsigned char *data;

    /*use test mode to test write*/
    for(i=0; i<TUNING_DATA_NO; i++) {
        data = (unsigned char *)&tuning_data[i];
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR1, LTE_MODEM_FUNC, (void*)data, 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR1+1, LTE_MODEM_FUNC, (void*)(data+1), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR1+2, LTE_MODEM_FUNC, (void*)(data+2), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR1+3, LTE_MODEM_FUNC, (void*)(data+3), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }

        if(msdc_autok_write(host, SDIO_IP_WTMDR, LTE_MODEM_FUNC, (void*)&(g_test_write_pattern[i*TUNING_TEST_TIME]),(4*TUNING_TEST_TIME), CMD_53) != 0) {
            res = errMapping(host);
            goto end;
        }

        data = (unsigned char *)&reg;
        if(msdc_autok_read(host, SDIO_IP_WTMCR, LTE_MODEM_FUNC, (void*)data, 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_read(host, SDIO_IP_WTMCR+1, LTE_MODEM_FUNC, (void*)(data+1), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_read(host, SDIO_IP_WTMCR+2, LTE_MODEM_FUNC, (void*)(data+2), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_read(host, SDIO_IP_WTMCR+3, LTE_MODEM_FUNC, (void*)(data+3), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }

        if((reg & TEST_MODE_STATUS) == TEST_MODE_STATUS) {
            res = E_RESULT_ERR;
            goto end;
        }
    }
end:
    return res;
}

static E_RESULT_TYPE autok_read_test(struct msdc_host *host)
{
    int i;
    E_RESULT_TYPE res = E_RESULT_PASS;
    unsigned char *data;

    /*use test mode to test read*/
    for(i=0; i<TUNING_DATA_NO; i++) {
        data = (unsigned char *)&tuning_data[i];
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0, LTE_MODEM_FUNC, (void*)data, 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0+1, LTE_MODEM_FUNC, (void*)(data+1), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0+2, LTE_MODEM_FUNC, (void*)(data+2), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0+3, LTE_MODEM_FUNC, (void*)(data+3), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }

        if(msdc_autok_read(host, SDIO_IP_WTMDR, LTE_MODEM_FUNC, (void*)g_test_read_pattern, (4*TUNING_TEST_TIME), CMD_53) != 0) {
            res = errMapping(host);
            goto end;
        }

        if(memcmp(g_test_read_pattern, &g_test_write_pattern[i*TUNING_TEST_TIME], 4*TUNING_TEST_TIME) != 0) {
            res = E_RESULT_CMP_ERR;
            goto end;
        }
    }
end:
    return res;
}

static E_RESULT_TYPE autok_cmd_test(struct msdc_host *host)
{
    int i;
    E_RESULT_TYPE res = E_RESULT_PASS;
    unsigned char *data;

    /*use test mode to test read*/
    for(i=0; i<TUNING_DATA_NO; i++) {
        data = (unsigned char *)&tuning_data[i];
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0, LTE_MODEM_FUNC, (void*)data, 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0+1, LTE_MODEM_FUNC, (void*)(data+1), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0+2, LTE_MODEM_FUNC, (void*)(data+2), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_write(host, SDIO_IP_WTMDPCR0+3, LTE_MODEM_FUNC, (void*)(data+3), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }

        data = (unsigned char *)g_test_read_pattern;
        if(msdc_autok_read(host, SDIO_IP_WTMDPCR0, LTE_MODEM_FUNC, (void*)data, 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_read(host, SDIO_IP_WTMDPCR0+1, LTE_MODEM_FUNC, (void*)(data+1), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_read(host, SDIO_IP_WTMDPCR0+2, LTE_MODEM_FUNC, (void*)(data+2), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }
        if(msdc_autok_read(host, SDIO_IP_WTMDPCR0+3, LTE_MODEM_FUNC, (void*)(data+3), 1, CMD_52) != 0) {
            res = E_RESULT_CMD_CRC;
            goto end;
        }

        if(g_test_read_pattern[0] != tuning_data[i]) {
        #ifdef AUTOK_DEBUG
            printk("write: 0x%x read: 0x%x\r\n", tuning_data[i], g_test_read_pattern[0]);
        #endif
            res = E_RESULT_CMP_ERR;
            goto end;
        }
    }
end:
    return res;
}

static int autok_recovery(struct msdc_host *host)
{
    /*TODO need to do some SW recovery for next test*/

#if 0
    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
#endif

    return 0;
}


static void autok_select_range(unsigned int  result, unsigned int *sel)
{
    unsigned char start = 0;
    unsigned char end = 0;  // we need ten 0.
    unsigned char bit = 0;
    unsigned char max_start = 0;
    unsigned char max_end = 0;
    unsigned char max_score = 0;

    // maybe result is 0
    if (result == 0) {
        start = 0; end = 31;
        goto end;
    }

find:
    start = end = 0;
    while (bit < 32) {
        if (result & (1 << bit)) { // failed
            bit++; continue;
        }
        start = end = bit;
        bit++;
        break;
    }

    while (bit < 32) {
        if (result & (1 << bit)){ // failed
            bit++;
            if((end - start) > max_score) {
                max_score = end - start;
                max_start = start;
                max_end = end;
            }
            goto find;

        }
        else {
            end = bit;
            bit++;
        }
    }
end:
    if((end -start) > max_score) {
        max_score = end - start;
        max_start = start;
        max_end = end;
    }

#ifdef AUTOK_DEBUG
    printk("score<%d> choose bit<%d> from<0x%x>\r\n",(max_score+1),(max_end + max_start)/2, result);
#endif

    *sel = (max_end + max_start)/2;

}


static int autok_simple_score(unsigned int result)
{
    unsigned int  bit = 0;
    unsigned int  num = 0;
    unsigned int  old = 0;

    // maybe result is 0
    if (0 == result) {
    #ifdef AUTOK_DEBUG
        strcpy(g_tune_result_str,"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
    #endif
        return 32;
    }

    if (0xFFFFFFFF == result) {
    #ifdef AUTOK_DEBUG
        strcpy(g_tune_result_str,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    #endif
        return 0;
    }

    /* calc continue zero number */
    while (bit < 32) {
        if (result & (1 << bit)) { // failed
        #ifdef AUTOK_DEBUG
            g_tune_result_str[bit]='X';
        #endif
            bit++;
            if (old < num)
                old = num;
            num = 0;
            continue;
        }
        #ifdef AUTOK_DEBUG
        g_tune_result_str[bit]='O';
        #endif
        bit++;
        num++;
    }

    if (num > old)
        old = num;

    return old;
}

static int autok_check_score(unsigned int  result, unsigned int  *pNumOfzero, unsigned int *pFrtPosErr, unsigned int  *pPeriod)
{
    unsigned int  bit = 0;
    unsigned int  num = 0;
    unsigned int  old = 0;
    unsigned int  sndPosErr=0;
    E_AUTOK_PERIOD_STA sta=PERIOD_NONE;


    *pNumOfzero = 0;
    *pFrtPosErr = 0;
    // maybe result is 0
    if (0 == result) {
    #ifdef AUTOK_DEBUG
        strcpy(g_tune_result_str,"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
    #endif
        *pNumOfzero = 32;
        *pFrtPosErr = 32;
        return 32;
    }

    if (0xFFFFFFFF == result) {
    #ifdef AUTOK_DEBUG
        strcpy(g_tune_result_str,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    #endif
        return 0;
    }

    /* calc continue zero number */
    while (bit < 32) {
        if (result & (1 << bit)) { // failed
        #ifdef AUTOK_DEBUG
            g_tune_result_str[bit]='X';
        #endif
            switch(sta)
            {
                case PERIOD_NONE:
                    sta = PERIOD_L_FIRST_POS;
                    *pFrtPosErr = bit;
                    break;
                case PERIOD_L_FIRST_POS:
                    *pFrtPosErr = bit;
                    break;
                case PERIOD_L_FIRST_POS_DONE:
                    sta = PERIOD_L_SECOND_POS;
                    sndPosErr = bit;
                    break;
                case PERIOD_L_SECOND_POS:
                    sndPosErr = bit;
                    break;
                case PERIOD_F_FIRST_POS:
                    sta = PERIOD_F_FIRST_POS_DONE;
                    *pFrtPosErr = bit;
                    break;
                case PERIOD_F_SECOND_POS:
                    sta = PERIOD_DONE;
                    sndPosErr = bit;
                    break;
                default:
                    break;
            }
            bit++;
            if (num > old)
                old = num;
            num = 0;
            continue;
        }
    #ifdef AUTOK_DEBUG
        g_tune_result_str[bit]='O';
    #endif
        bit++;
        num++;
        *pNumOfzero=*pNumOfzero+1;
        switch(sta)
        {
            case PERIOD_NONE:
                sta = PERIOD_F_FIRST_POS;
                break;
            case PERIOD_F_FIRST_POS_DONE:
                sta = PERIOD_F_SECOND_POS;
                break;
            case PERIOD_L_FIRST_POS:
                sta = PERIOD_L_FIRST_POS_DONE;
                break;
            case PERIOD_L_SECOND_POS:
                sta = PERIOD_DONE;
                break;
            default:
                break;
        }

    }

    if (num > old)
        old = num;

    if(sta == PERIOD_DONE)
        *pPeriod = sndPosErr - *pFrtPosErr;
    else
        *pPeriod = 0;

    return old;
}


static E_RESULT_TYPE autok_tune_algorithm(struct msdc_host *host, E_AUTOK_TUNING_STAGE stg, U_AUTOK_INTERFACE_DATA *pAutoKData)
{
    unsigned int k,m,x;
    unsigned int sel=0;
    unsigned int max_score = 0;
    unsigned int max_numZero = 0;
    unsigned int score = 0;
    unsigned int stop = 0;
    unsigned int pass = 0;
    unsigned int regionFound = 0;
    unsigned int fstPass,sndPass;
    E_AUTOK_DATA_STA dataSta;
    E_AUTOK_ERR_STA err;
    S_AUTOK_CMD_DLY data;
    unsigned int autok_cmddly_stop_bit[SCALE_CMD_RSP_DLY_SEL];
    unsigned int pad_delay_period_cycle = 0;
    unsigned int clk_gen_delay_period_cycle = 0;
    unsigned int periodCycle = 0;
    unsigned int minPadCycleScore = 0;
    unsigned int minClkGenCycleScore = 0;
    unsigned char bTryFindPadCycle = 1;
    unsigned char bTryFindClkGenCycle = 1;
    unsigned int range_min, range_max;
    int cnt;
    E_RESULT_TYPE res;
    int reTuneCmd = 0;

    /* title */
#if 1//def AUTOK_DEBUG
    printk("=======autok_stg%d_tune=======\r\n",stg+1);
#endif
    memset(autok_cmd_cmdrrdly, 0, sizeof(S_AUTOK_CMD_DLY)*SCALE_CMD_RSP_DLY_SEL);
    memset(autok_cmd_ckgdly, 0, sizeof(S_AUTOK_CMD_DLY)*SCALE_CKGEN_MSDC_DLY_SEL);
    memset(autok_ckg_data, 0, sizeof(S_AUTOK_CKGEN_DATA)*SCALE_CKGEN_MSDC_DLY_SEL);
    memset(&data, 0, sizeof(S_AUTOK_CMD_DLY));
    periodCycle = FREQ_MHZ_2_PERIOD_CYCLE_IN_PS(host->mclk/1000000);
    clk_gen_delay_period_cycle = MAX_SCALE_OF_CLK_GEN_IN_ONE_CYCLE(periodCycle);
    clk_gen_delay_period_cycle = THRESHOLD_VAL(clk_gen_delay_period_cycle, SCALE_CKGEN_MSDC_DLY_SEL);
    minPadCycleScore = MIN_SCORE_OF_PAD_DELAY_IN_ONE_CYCLE(periodCycle);
    if(minPadCycleScore >= SCALE_PAD_TUNE_CMDRDLY) {
        bTryFindPadCycle = 0;
    }
    minClkGenCycleScore = MIN_SCORE_OF_CLK_GEN_IN_ONE_CYCLE(periodCycle);
    if(minClkGenCycleScore >= SCALE_CKGEN_MSDC_DLY_SEL) {
        bTryFindClkGenCycle = 0;
    }

    g_autoK_range_against_temper = MAX_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR;


#ifdef AUTOK_DEBUG
    printk("period=%d MaxCkgen period=%d MinCkgen score=%d MinPad score=%d \r\n", \
            periodCycle, clk_gen_delay_period_cycle, minClkGenCycleScore, minPadCycleScore);
#endif


    if(pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.bRange == 1) {
        if(pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.sel > pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.range)
            range_min = pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.sel - pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.range;
        else
            range_min = 0;

        range_max = pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.sel + pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.range + 1;
    }
    else {
        range_min = 0;
        range_max = clk_gen_delay_period_cycle;
    }
    k = range_min;
    range_max = THRESHOLD_VAL(range_max,SCALE_CKGEN_MSDC_DLY_SEL-1);

#ifdef AUTOK_DEBUG
    printk("ckg scan range from %d to %d\r\n",range_min, range_max);
#endif
    while(k <= range_max) {
        pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.sel = k;
        msdc_autok_adjust_param(host, CKGEN_MSDC_DLY_SEL, &k, MSDC_WRITE);
        /*Step1 find the simple cmd delay for clk_gen in internal delay=0*/
        autok_cmd_cmdrrdly[0].raw_data = 0;
        sel = 0;
        msdc_autok_adjust_param(host, CMD_RESP_RD_DLY, &sel, MSDC_WRITE);
        for (m = 0; m < SCALE_PAD_TUNE_CMDRDLY; m++) {
            msdc_autok_adjust_param(host, CMD_RD_DLY, &m, MSDC_WRITE);
            for (cnt = 0; cnt < AUTOK_CMD_TIMES; cnt++) {
                if (autok_cmd_test(host) != E_RESULT_PASS) {
                    // 0 means pass
                    autok_cmd_cmdrrdly[0].raw_data |= (1 << m);
                    break;
                }
            }
        }
        autok_cmd_cmdrrdly[0].score = autok_simple_score(autok_cmd_cmdrrdly[0].raw_data);

        if(autok_cmd_cmdrrdly[0].score < MIN_DATA_SCORE){
        #ifdef AUTOK_DEBUG
            printk("cmd score(%d) is less than min(%d)\r\n",autok_cmd_cmdrrdly[0].score,MIN_DATA_SCORE);
        #endif
            k++;
            continue;
        }
    #ifdef AUTOK_DEBUG
        printk("CKGEN_MSDC_DLY \t PAD_TUNE_CMDRDLY \r\n");
        printk("%02d \t %02d \t %s\r\n",k,autok_cmd_cmdrrdly[0].score,g_tune_result_str);
    #endif
        autok_select_range(autok_cmd_cmdrrdly[0].raw_data, &pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel);
        msdc_autok_adjust_param(host, CMD_RD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel, MSDC_WRITE);

        /*Step2 tune_read data*/
    #ifdef AUTOK_DEBUG
        /*Step1 find the internal delay for every clk_gen*/
        printk("CKGEN_MSDC_DLY \t PAD_TUNE_DATRDDLY \r\n");
    #endif
        data.raw_data = 0;
        reTuneCmd = 0;
        stop = 0;
        x = 0;
        dataSta = SEARCH_FIRST_PASS;
        fstPass = sndPass = 0;
        regionFound = 0;
        while(x < SCALE_PAD_TUNE_DATRDDLY && reTuneCmd == 0) {
        #ifdef AUTOK_DEBUG
            if(stop == 1) {
                data.raw_data  |= (1 << x);
                x++;
                continue;
            }
        #else
            if(stop == 1) {
                break;
            }
        #endif
            msdc_autok_adjust_param(host, DAT_RD_DLY, &x, MSDC_WRITE);
            for (m = 0; m < AUTOK_RDAT_TIMES; m++) {
                if ((res = autok_read_test(host)) != E_RESULT_PASS) {
                    //printk("curPos=%d Fail\r\n",x);
                    data.raw_data  |= (1 << x);
                    if(x> (SCALE_PAD_TUNE_DATRDDLY - MIN_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR )) {
                        /*we do not care the rest because the score is must less than criteria*/
                        stop = 1;
                    }
                    if (autok_recovery(host)) {
                    #ifdef AUTOK_DEBUG
                        printk("===tune read : error, fail to bring to tranfer status===\r\n");
                    #endif
                        return E_RESULT_ERR;
                    }
                    if (res == E_RESULT_CMD_CRC) {
                    #ifdef AUTOK_DEBUG
                        printk("[WARN] CMD CRC error in tuning read[%d %d], need to tune command again!!\r\n",x,m);
                    #endif
                        reTuneCmd = 1;
                    }
                    if(stop == 0) {
                        //printk("old state=%d\r\n",dataSta);
                        switch(dataSta) {
                            case SEARCH_FIRST_PASS:
                                x++;
                                if(x == sndPass) {
                                    m = AUTOK_WDAT_TIMES; /*just enter the pass condition later*/
                                }
                                break;
                            case SEARCH_SECOND_PASS:
                                /*ignore gears before x because the score is must less than criteria*/
                                x++;
                                dataSta = SEARCH_FIRST_PASS;
                                break;
                            case SEARCH_PASS_REGION:
                                dataSta = SEARCH_FIRST_PASS;
                                x++;
                                if(x == sndPass) {
                                    m = AUTOK_WDAT_TIMES; /*just enter the pass condition later*/
                                }
                                break;
                            case PASS_REGION_GET:
                                dataSta = SEARCH_FIRST_PASS;
                                fstPass = 0;
                                sndPass = 0;
                                x++;
                                break;
                            default:
                                break;

                        }
                        //printk("new state=%d\r\n",dataSta);
                    }
                    break;
                }
            }
            if(m == AUTOK_RDAT_TIMES) {
                //printk("curPos=%d Pass\r\n",x);
                /*Pass*/
                //printk("old state=%d\r\n",dataSta);
                switch(dataSta) {
                    case SEARCH_FIRST_PASS:
                        if(x > (SCALE_PAD_TUNE_DATRDDLY - MIN_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR )) {
                            /*we do not care the rest because the score is must less than criteria*/
                            stop = 1;
                            x++;
                        }
                        else {
                            fstPass = x;
                            sndPass = 0;
                            x+= (MIN_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR - 1);
                            dataSta = SEARCH_SECOND_PASS;
                        }
                        break;
                    case SEARCH_SECOND_PASS:
                        sndPass = x;
                        x= fstPass+1;
                        dataSta = SEARCH_PASS_REGION;
                        break;
                    case PASS_REGION_GET:
                        x++;
                    case SEARCH_PASS_REGION:
                        x++;
                        if(x == sndPass) {
                            dataSta = PASS_REGION_GET;
                            x++;
                            regionFound++;
                        }
                        break;
                        break;
                    default:
                        break;
                }
                //printk("new state=%d\r\n",dataSta);
            }
            //printk("curPos=%d sta=%d fstPos=%d sndPos=%d\r\n",x,dataSta,fstPass,sndPass);
        }

        if(reTuneCmd == 1) {
    #ifdef AUTOK_DEBUG
            return E_RESULT_ERR;
    #else
            k = range_min;
            continue;
    #endif
        }
        if(regionFound == 0) {
            k++;
            continue;
        }
        autok_ckg_data[k].readScore = autok_simple_score(data.raw_data);
        autok_select_range(data.raw_data, &pAutoKData[E_MSDC_PAD_TUNE_DATRRDLY].data.sel);
        autok_ckg_data[k].readPadSel = pAutoKData[E_MSDC_PAD_TUNE_DATRRDLY].data.sel;
        msdc_autok_adjust_param(host, DAT_RD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_DATRRDLY].data.sel, MSDC_WRITE);

    #ifdef AUTOK_DEBUG
        printk("%d \t %d \t %s\r\n", k, autok_ckg_data[k].readScore, g_tune_result_str);
    #endif

        /*Step3 tune_write data*/
    #ifdef AUTOK_DEBUG
        printk("CKGEN_MSDC_DLY \t PAD_TUNE_DATWDDLY \r\n");
    #endif
        data.raw_data = 0;
        reTuneCmd = 0;
        stop = 0;
        x = 0;
        while ( x < SCALE_PAD_TUNE_DATWRDLY && reTuneCmd == 0 && stop == 0) {
            msdc_autok_adjust_param(host, DAT_WRD_DLY, &x, MSDC_WRITE);
            for (m = 0; m < AUTOK_WDAT_TIMES; m++) {
                if ((res = autok_write_test(host)) != E_RESULT_PASS) {
                    data.raw_data|= (1 << x);
                    if(x> (SCALE_PAD_TUNE_DATRDDLY - MIN_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR )) {
                        /*we do not care the rest because the score is must less than criteria*/
                        stop = 1;
                    }
                    if (autok_recovery(host)) {
                    #ifdef AUTOK_DEBUG
                        printk("===tune write : error, fail to bring to tranfer status===\r\n");
                    #endif
                        return E_RESULT_ERR;
                    }
                    if (res == E_RESULT_CMD_CRC) {
                    #ifdef AUTOK_DEBUG
                        printk("[WARN] CMD CRC error in tuning write[%d %d], need to tune command again!!\r\n",x,m);
                    #endif
                        reTuneCmd = 1;
                    }
                    break;
                }
            }
            x++;
        }

        if(reTuneCmd == 1) {
     #ifdef AUTOK_DEBUG
            return E_RESULT_ERR;
     #else
            k = range_min;
            continue;
     #endif
        }
        autok_ckg_data[k].writeScore = autok_simple_score(data.raw_data);
        autok_select_range(data.raw_data, &pAutoKData[E_MSDC_PAD_TUNE_DATWRDLY].data.sel);
        autok_ckg_data[k].writePadSel = pAutoKData[E_MSDC_PAD_TUNE_DATWRDLY].data.sel;
        msdc_autok_adjust_param(host, DAT_WRD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_DATWRDLY].data.sel, MSDC_WRITE);

    #ifdef AUTOK_DEBUG
        printk("%d \t %d \t %s\r\n", k, autok_ckg_data[k].writeScore, g_tune_result_str);
    #endif



    #ifdef AUTOK_DEBUG
        printk("CKGEN_MSDC_DLY \t PAD_TUNE_CMDRRDLY \t PAD_TUNE_CMDRDLY \r\n");
    #endif

        pass = 0;
        max_score = 0;
        for (x=0; x<SCALE_CMD_RSP_DLY_SEL; x++){
            autok_cmd_cmdrrdly[x].raw_data = 0;
            autok_cmddly_stop_bit[x] = 0;
            stop = 0;
            err = ERR_NONE;
            msdc_autok_adjust_param(host, CMD_RESP_RD_DLY, &x, MSDC_WRITE);
            for (m = 0; m < SCALE_PAD_TUNE_CMDRDLY && stop == 0; m++) {
                msdc_autok_adjust_param(host, CMD_RD_DLY, &m, MSDC_WRITE);
                for (cnt = 0; cnt < AUTOK_CMD_TIMES; cnt++) {
                    if (autok_cmd_test(host) != E_RESULT_PASS) {
                        // 0 means pass
                        autok_cmd_cmdrrdly[x].raw_data |= (1 << m);
                        if(err == PASS_AFTER_ERR) {
                            /*If err occurs in the last position, just done!!*/
                            if(m != (SCALE_PAD_TUNE_CMDRDLY-1)) {
                                stop = 1;
                                autok_cmddly_stop_bit[x] = m;
                            }
                        }
                        else if(err == ERR_NONE) {
                            err = ERR_OCCURE;
                        }
                        break;
                    }
                }
                if( (cnt == AUTOK_CMD_TIMES)&& (err == ERR_OCCURE))
                    err = PASS_AFTER_ERR;
            }
            /*only find one pass region*/
            if(pass == 1 && stop == 1) {
                break;
            }
            else if(stop == 1) {
                continue;
            }

            pass = 1;

            autok_cmd_cmdrrdly[x].score = autok_check_score(autok_cmd_cmdrrdly[x].raw_data, \
                                                              &autok_cmd_cmdrrdly[x].numOfzero, \
                                                              &autok_cmd_cmdrrdly[x].fstPosErr, \
                                                              &autok_cmd_cmdrrdly[x].period);
        #ifdef AUTOK_DEBUG
            printk("%02d \t %02d \t %02d \t %s\r\n",k,x,autok_cmd_cmdrrdly[x].score,g_tune_result_str);
        #endif
            if(autok_cmd_cmdrrdly[x].score > max_score) {
                max_score = autok_cmd_cmdrrdly[x].score;
                max_numZero = autok_cmd_cmdrrdly[x].numOfzero;
                sel = x;
            }
            else if(autok_cmd_cmdrrdly[x].score == max_score) {
                if(autok_cmd_cmdrrdly[x].numOfzero > max_numZero) {
                    max_numZero = autok_cmd_cmdrrdly[x].numOfzero;
                    sel = x;
                }
            }
        }
        if(max_score == 0) {
        #ifdef AUTOK_DEBUG
            printk("[Warn] autok algorithm for tuning cmd internal delay need to scan more!!\r\n");
        #endif
            for (x=0; x<SCALE_CMD_RSP_DLY_SEL; x++){
                msdc_autok_adjust_param(host, CMD_RESP_RD_DLY, &x, MSDC_WRITE);
                for (m = autok_cmddly_stop_bit[x]+1; m < SCALE_PAD_TUNE_CMDRDLY; m++) {
                    msdc_autok_adjust_param(host, CMD_RD_DLY, &m, MSDC_WRITE);
                    for (cnt = 0; cnt < AUTOK_CMD_TIMES; cnt++) {
                        if (autok_cmd_test(host) != E_RESULT_PASS) {
                            // 0 means pass
                            autok_cmd_cmdrrdly[x].raw_data |= (1 << m);
                            break;
                        }
                    }
                }
                autok_cmd_cmdrrdly[x].score = autok_check_score(autok_cmd_cmdrrdly[x].raw_data, \
                                              &autok_cmd_cmdrrdly[x].numOfzero, \
                                              &autok_cmd_cmdrrdly[x].fstPosErr, \
                                              &autok_cmd_cmdrrdly[x].period);
            #ifdef AUTOK_DEBUG
                printk("%02d \t %02d \t %02d \t %s\r\n",k,x,autok_cmd_cmdrrdly[x].score,g_tune_result_str);
            #endif

                if(autok_cmd_cmdrrdly[x].score > max_score) {
                    max_score = autok_cmd_cmdrrdly[x].score;
                    max_numZero = autok_cmd_cmdrrdly[x].numOfzero;
                    sel = x;
                }
                else if(autok_cmd_cmdrrdly[x].score == max_score) {
                    if(autok_cmd_cmdrrdly[x].numOfzero > max_numZero) {
                        max_numZero = autok_cmd_cmdrrdly[x].numOfzero;
                        sel = x;
                    }
                }
            }
        }

        pAutoKData[E_MSDC_PAD_TUNE_CMDRRDLY].data.sel = sel;
        autok_ckg_data[k].interDelaySel = sel;
        autok_cmd_ckgdly[k] = autok_cmd_cmdrrdly[sel];

        #ifdef AUTOK_DEBUG
            printk("CMD internal delay %d score= %d numOfZero=%d fstPosErr=%d\r\n", \
                    sel,max_score,max_numZero,autok_cmd_ckgdly[k].fstPosErr);
        #endif

        /*pAutoKData[E_MSDC_PAD_TUNE_CMDRRDLY].data.bRange = 1;
          pAutoKData[E_MSDC_PAD_TUNE_CMDRRDLY].data.range_min = \
          ((sel-CMD_INTERNAL_DELAY_RANGE)>=0?(sel-CMD_INTERNAL_DELAY_RANGE):(sel+SCALE_CMD_RSP_DLY_SEL-CMD_INTERNAL_DELAY_RANGE));
          pAutoKData[E_MSDC_PAD_TUNE_CMDRRDLY].data.range_max = \
          ((sel+CMD_INTERNAL_DELAY_RANGE)<SCALE_CMD_RSP_DLY_SEL?(sel+CMD_INTERNAL_DELAY_RANGE):(sel+CMD_INTERNAL_DELAY_RANGE-SCALE_CMD_RSP_DLY_SEL));
          */
        msdc_autok_adjust_param(host, CMD_RESP_RD_DLY, &sel, MSDC_WRITE);

        if(k==range_min && stg == TUNING_STG1) {
            /*Step4 find sampling edge*/
            sel = 1;
            data.raw_data = 0;
            msdc_autok_adjust_param(host, CMD_EDGE, &sel, MSDC_WRITE);
            for (m = 0; m < SCALE_PAD_TUNE_CMDRDLY; m++) {
                msdc_autok_adjust_param(host, CMD_RD_DLY, &m, MSDC_WRITE);
                for (cnt = 0; cnt < AUTOK_CMD_TIMES; cnt++) {
                    if (autok_cmd_test(host) != E_RESULT_PASS) {
                        // 0 means pass
                        data.raw_data |= (1 << m);
                        break;
                    }
                }
            }
            data.score = autok_check_score(data.raw_data, \
                                          &data.numOfzero, \
                                          &data.fstPosErr, \
                                          &data.period);
            #ifdef AUTOK_DEBUG
                printk("Falling edge %s score=%d fstPosErr=%d\r\n",g_tune_result_str,data.score,data.fstPosErr);
            #endif
            sel = 0;
        #if 1
            if(data.fstPosErr < autok_cmd_ckgdly[0].fstPosErr) {
                sel = 1;
                autok_cmd_ckgdly[0] = data;
            }
        #else
            if(data.score > max_score) {
                sel = 1;
                autok_cmd_ckgdly[0] = data;
            }
            else if(data.score == max_score) {
                if(data.fstPosErr < autok_cmd_ckgdly[0].fstPosErr) {
                    sel = 1;
                    autok_cmd_ckgdly[0] = data;
                }
            }
        #endif

            if(sel != 1)
                msdc_autok_adjust_param(host, CMD_EDGE, &sel, MSDC_WRITE);

            pAutoKData[E_MSDC_IOCON_RSPL].data.sel = sel;

        }
        autok_ckg_data[k].cmdScore = autok_cmd_ckgdly[k].score;
        autok_select_range(autok_cmd_ckgdly[k].raw_data, &pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel);
        autok_ckg_data[k].cmdPadSel = pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel;
        msdc_autok_adjust_param(host, CMD_RD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel, MSDC_WRITE);


        if(bTryFindPadCycle == 1 && pad_delay_period_cycle == 0 && autok_cmd_ckgdly[k].period >= minPadCycleScore) {
            /*get period form data the pass criteria may change*/
            pad_delay_period_cycle = autok_cmd_ckgdly[k].period;
            clk_gen_delay_period_cycle = autok_cmd_ckgdly[k].period/SCALE_OF_CLK_GEN_2_PAD_DELAY;
            if(stg == TUNING_STG1) {
                range_max = clk_gen_delay_period_cycle;
            }
            g_autoK_range_against_temper = \
            USER_DEF_MAX_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR(autok_cmd_ckgdly[k].period, periodCycle);
        #ifdef AUTOK_DEBUG
            printk("period change by pad :pad cycle=%d ckg cycle=%d pass criteria=%d\r\n", \
                    pad_delay_period_cycle,clk_gen_delay_period_cycle, g_autoK_range_against_temper);
        #endif
            /*Check the old score if any one meet success criteria*/
            for(sel=range_min;sel<=k;sel++) {
                if(autok_ckg_data[sel].cmdScore >= g_autoK_range_against_temper && \
                   autok_ckg_data[sel].readScore >= g_autoK_range_against_temper && \
                   autok_ckg_data[sel].writeScore >= g_autoK_range_against_temper) {
                    if(sel!=k) {
                        goto APPLYSET;
                    }
                    goto END;
                }
            }
        }
        else {
            if(autok_ckg_data[k].cmdScore >= g_autoK_range_against_temper && \
               autok_ckg_data[k].readScore >= g_autoK_range_against_temper && \
               autok_ckg_data[k].writeScore >= g_autoK_range_against_temper)
                goto END;
        }
        k++;
    }

    if(bTryFindClkGenCycle == 1 && pad_delay_period_cycle == 0) {
        /*usually FF will hit this condition*/
        /*chk the ckg fstPosErr to find one cycle if possible*/
        if(range_max> minClkGenCycleScore) {
            for(x=0;x<range_max-minClkGenCycleScore && pad_delay_period_cycle==0;x++) {
                for(m=x+minClkGenCycleScore;m<=range_max;m++) {
                    if(ABS_DIFF(autok_cmd_ckgdly[x].fstPosErr, autok_cmd_ckgdly[m].fstPosErr) \
                        > AUTOK_TUNING_INACCURACY) {
                             continue;
                    }
                    clk_gen_delay_period_cycle = m-x;
                    pad_delay_period_cycle = (m-x)*DIV_CEIL_FUNC(MIN_CLK_GEN_DELAY_IN_PS, MIN_PAD_DELAY_IN_PS);
                    g_autoK_range_against_temper =  USER_DEF_MAX_SCORE_OF_PAD_DELAY_AGAINST_TEMP_VAR(pad_delay_period_cycle, periodCycle);
                #ifdef AUTOK_DEBUG
                    printk("period change by ckg:pad cycle=%d ckg cycle=%d pass criteria=%d\r\n", \
                            pad_delay_period_cycle,clk_gen_delay_period_cycle, g_autoK_range_against_temper);
                #endif
                    for(sel=range_min;sel<=range_max;sel++) {
                        if(autok_ckg_data[sel].cmdScore >= g_autoK_range_against_temper && \
                           autok_ckg_data[sel].readScore >= g_autoK_range_against_temper && \
                           autok_ckg_data[sel].writeScore >= g_autoK_range_against_temper) {
                            if(sel!=range_max) {
                                goto APPLYSET;
                            }
                            goto END;
                        }
                    }
                    break;

                }
            }
        }
    }

#ifdef AUTOK_DEBUG
    printk("Get the worst case!! Find the max score as parameter\r\n");
#endif
    /*From the total candidate, find the best as parameter*/
    max_score = 0;
    sel = 0;
    for(k=range_min;k<=range_max;k++) {
        if(autok_ckg_data[k].cmdScore < MIN_DATA_SCORE || \
           autok_ckg_data[k].readScore < MIN_DATA_SCORE || \
           autok_ckg_data[k].writeScore < MIN_DATA_SCORE)
            continue;
        score = autok_ckg_data[k].cmdScore + autok_ckg_data[k].readScore + autok_ckg_data[k].writeScore;
        if(score > max_score) {
            sel=k;
            max_score = score;
        }
    }
    if(max_score == 0) {
        printk("[ERROR] autok algorithm fail to find any suitable parameter!!\r\n");
        return E_RESULT_ERR;
    }

APPLYSET:
    pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.sel = sel;
    msdc_autok_adjust_param(host, CKGEN_MSDC_DLY_SEL, &pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.sel, MSDC_WRITE);

    pAutoKData[E_MSDC_PAD_TUNE_CMDRRDLY].data.sel = autok_ckg_data[sel].interDelaySel;
    msdc_autok_adjust_param(host, CMD_RESP_RD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_CMDRRDLY].data.sel, MSDC_WRITE);

    pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel = autok_ckg_data[sel].cmdPadSel;
    msdc_autok_adjust_param(host, CMD_RD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_CMDRDLY].data.sel, MSDC_WRITE);

    pAutoKData[E_MSDC_PAD_TUNE_DATRRDLY].data.sel = autok_ckg_data[sel].readPadSel;
    msdc_autok_adjust_param(host, DAT_RD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_DATRRDLY].data.sel, MSDC_WRITE);

    pAutoKData[E_MSDC_PAD_TUNE_DATWRDLY].data.sel = autok_ckg_data[sel].writePadSel;
    msdc_autok_adjust_param(host, DAT_WRD_DLY, &pAutoKData[E_MSDC_PAD_TUNE_DATWRDLY].data.sel, MSDC_WRITE);

END:
    if(stg == TUNING_STG1) {
        pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.bRange = 1;
        pAutoKData[E_MSDC_CKGEN_MSDC_DLY_SEL].data.range = DIV_CEIL_FUNC(clk_gen_delay_period_cycle, 2);
    }

#ifdef AUTOK_DEBUG
    for(k=range_min;k<=range_max;k++) {
        autok_simple_score(autok_cmd_ckgdly[k].raw_data);
        printk("%d \t %s\r\n", k, g_tune_result_str);
    }
#endif

#if 1//def AUTOK_DEBUG
    printk("[autok tuning] End of %d stg\r\n", stg);
#endif
    return E_RESULT_PASS;
}


static void autok_tuning_parameter_init(struct msdc_host *host, E_AUTOK_TUNING_STAGE stg, U_AUTOK_INTERFACE_DATA *pAutokData)
{
    unsigned int val=0;

    /* TODO :disable & clear all interrupt, and begin my test */
    //MSDC_RESET();
    //MSDC_CLR_FIFO();
    //MSDC_WRITE32(MSDC_INTEN, 0);
    //MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
    containGen();
    /* data delay using the one setting */
    msdc_autok_adjust_param(host, DATA_DLYLINE_SEL, &val, MSDC_WRITE);
    msdc_autok_adjust_param(host, DAT_RD_DLY, &val, MSDC_WRITE);
    msdc_autok_adjust_param(host, DAT_WRD_DLY, &val, MSDC_WRITE);

    /* data sampling use the one setting*/
    msdc_autok_adjust_param(host, READ_DATA_SMPL_SEL, &val, MSDC_WRITE);
    msdc_autok_adjust_param(host, WRITE_DATA_SMPL_SEL, &val, MSDC_WRITE);


    /* cmd response delay selection value */
    msdc_autok_adjust_param(host, CMD_RESP_RD_DLY, &val, MSDC_WRITE);

    /* cmd line delay selection value */
    msdc_autok_adjust_param(host, CMD_RD_DLY, &val, MSDC_WRITE);

    /* ckbuf in ckgen delay selection  for read tuning, 32 stages */
    msdc_autok_adjust_param(host, CKGEN_MSDC_DLY_SEL, &val, MSDC_WRITE);

    if(stg == TUNING_STG1) {
        /* cmd line with clock's rising or falling edge */
        msdc_autok_adjust_param(host, CMD_EDGE, &val, MSDC_WRITE);

        /* cmd response turn around period, just for UHS104 mode */
        msdc_autok_adjust_param(host, CMD_RSP_TA_CNTR, &val, MSDC_READ);
        pAutokData[E_MSDC_CMD_RSP_TA_CNTR].data.sel = val;
        /* read data latch clock selection */
        msdc_autok_adjust_param(host, INT_DAT_LATCH_CK, &val, MSDC_READ);
        pAutokData[E_MSDC_INT_DAT_LATCH_CK_SEL].data.sel = val;

        /* read sampling edge*/
        msdc_autok_adjust_param(host, RDATA_EDGE, &val, MSDC_READ);
        pAutokData[E_MSDC_IOCON_RDSPL].data.sel = val;

        /* write CRC turn around period, just for UHS104 mode */
        msdc_autok_adjust_param(host, WRDAT_CRCS_TA_CNTR, &val, MSDC_READ);
        pAutokData[E_MSDC_WRDAT_CRCS_TA_CNTR].data.sel = val;

        /* write CRC sampling edge*/
        msdc_autok_adjust_param(host, WDATA_EDGE, &val, MSDC_READ);
        pAutokData[E_MSDC_IOCON_WDSPL].data.sel = val;
    }
    else {
        /* cmd line with clock's rising or falling edge */
        msdc_autok_adjust_param(host, CMD_EDGE, &pAutokData[E_MSDC_IOCON_RSPL].data.sel, MSDC_WRITE);

        /* cmd response turn around period, just for UHS104 mode */
        msdc_autok_adjust_param(host, CMD_RSP_TA_CNTR, &pAutokData[E_MSDC_CMD_RSP_TA_CNTR].data.sel, MSDC_WRITE);

        /* read data latch clock selection */
        msdc_autok_adjust_param(host, INT_DAT_LATCH_CK, &pAutokData[E_MSDC_INT_DAT_LATCH_CK_SEL].data.sel, MSDC_WRITE);

        /* read sampling edge*/
        msdc_autok_adjust_param(host, RDATA_EDGE, &pAutokData[E_MSDC_IOCON_RDSPL].data.sel, MSDC_WRITE);

        /* write CRC turn around period, just for UHS104 mode */
        msdc_autok_adjust_param(host, WRDAT_CRCS_TA_CNTR, &pAutokData[E_MSDC_WRDAT_CRCS_TA_CNTR].data.sel, MSDC_WRITE);

        /* write CRC sampling edge*/
        msdc_autok_adjust_param(host, WDATA_EDGE, &pAutokData[E_MSDC_IOCON_WDSPL].data.sel, MSDC_WRITE);

    }

}

/*************************************************************************
* FUNCTION
*  msdc_autok_stg1_cal
*
* DESCRIPTION
*  This function for auto-K at stage1
*
* PARAMETERS
*    host: msdc host manipulator pointer
*
* RETURN VALUES
*    error code: 0 success,
*               -1 parameter input error
*               -2 else error
*************************************************************************/
int msdc_autok_stg1_cal(struct msdc_host *host)
{
    //E_AUTOK_STATE state = AUTOK_CMD;
    E_RESULT_TYPE res= E_RESULT_ERR;

    if(!host)
        return -1;
    memset(g_autok_data, 0, sizeof(U_AUTOK_INTERFACE_DATA)*MAX_AUTOK_DAT_NUM);

    autok_tuning_parameter_init(host, TUNING_STG1, g_autok_data);

    res = autok_tune_algorithm(host, TUNING_STG1, g_autok_data);

    g_autok_data[E_AUTOK_VERSION].version = AUTOK_VERSION_NO;
    g_autok_data[E_AUTOK_FREQ].freq = host->mclk;

#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT    
    // auto-k test [s]
    curstg++;
    // auto-k test [e]
#endif

    return -res;
}


/*************************************************************************
* FUNCTION
*  msdc_autok_stg1_result_get
*
* DESCRIPTION
*  This function for getting data from stage1 result
*
* PARAMETERS
*    ppData: pointer to pointer for getting autoK data at stage1
*    pLen: data length in number of byte
*
* RETURN VALUES
*    error code: 0 success,
*               -1 parameter input error
*               -2 else error
*************************************************************************/
int msdc_autok_stg1_data_get(void **ppData, int *pLen)
{
    if(ppData == NULL || pLen == NULL)
        return -1;

    *ppData = (void *)g_autok_data;
    *pLen = sizeof(U_AUTOK_INTERFACE_DATA)*(MAX_AUTOK_DAT_NUM);
    return 0;
}

/*************************************************************************
*************************************************************************/
#if 0
int msdc_autok_msdc_parameter_test(struct msdc_host *host)
{
    enum AUTOK_PARAM i;
    int err;
    int value;
    int ret;
    int defaultValue[TOTAL_PARAM_COUNT];
    int error = 0;
    
    for(i = CMD_EDGE; i < TOTAL_PARAM_COUNT; i++)
    {
        err = msdc_autok_adjust_param(host, i, &defaultValue[i], AUTOK_READ);
        if(err)
        {
            error = 1;
            printk("Isaac: msdc_autok_adjust_param(get default) read fail, autok_param = %d, ret = %d, err = %d\n", i, defaultValue[i], err);
        }
    }
    
    for(i = CMD_EDGE; i < TOTAL_PARAM_COUNT + 1; i++)
    {
        for(value = 0; value < 33; value++)
        {
            err = msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
            if(err)
            {
                error = 1;
                printk("Isaac: msdc_autok_adjust_param write fail, autok_param = %d, value = %d, err = %d\n", i, value, err);
            }
            err = msdc_autok_adjust_param(host, i, &ret, AUTOK_READ);
            if(err)
            {
                error = 1;
                printk("Isaac: msdc_autok_adjust_param read fail, autok_param = %d, ret = %d, err = %d\n", i, ret, err);
            }
            if(value != ret)
            {
                error = 1;
                printk("Isaac: msdc_autok_adjust_param rw fail, autok_param = %d, value = %d, ret = %d\n", i, value, ret);
            }
        }
    }
    
    for(i = CMD_EDGE; i < TOTAL_PARAM_COUNT; i++)
    {
        err = msdc_autok_adjust_param(host, i, &defaultValue[i], AUTOK_WRITE);
        if(err)
        {
            error = 1;
            printk("Isaac: msdc_autok_adjust_param(reset default) write fail, autok_param = %d, default value = %d, err = %d\n", i, defaultValue[i], err);
        }
    }
    
    if(error == 0)
        printk("Isaac: msdc_autok_adjust_param pass\n");
    return 0;
}

#define DEFAULT_WTMDPCR0 0xF0F0F0F0
#define DEFAULT_WTMDPCR1 0xF0F0F0F0
#define SDIO_IP_WTMDR       	(0x00B0)	
#define SDIO_IP_WTMCR       	(0x00B4)
#define SDIO_IP_WTMDPCR0    	(0x00B8)	
#define SDIO_IP_WTMDPCR1    	(0x00BC)	
#define TEST_MODE_FW_OWN    (0x1<<24)	
#define TEST_MODE_SELECT    (0x3)
#define TEST_MODE_STATUS    (0x1<<8)

unsigned char buff_kmemory_test_mode[65536];

int msdc_autok_rw_test(struct msdc_host *host)
{
    unsigned int WTMDR_val;
    unsigned int WTMCR_val;
    unsigned int WTMDPCR0_val;
    unsigned int WTMDPCR1_val;
    unsigned int *buffer_recv, *buffer_send;
    
    unsigned int comp_pattern0, comp_pattern1;
    unsigned int test_length;
    
    unsigned int i,j;
    
    msdc_autok_read(host, SDIO_IP_WTMCR, 1, &WTMCR_val, 4, 53);
    if(1 == (WTMCR_val & TEST_MODE_FW_OWN)>>24){
        printk("[%s]: The test mode is not controlable by host !!! \n", __func__);
        return -1;
    }
    
    // Default value Test
    msdc_autok_read(host, SDIO_IP_WTMCR, 1, &WTMCR_val, 4, 53);
    msdc_autok_read(host, SDIO_IP_WTMDPCR0, 1, &WTMDPCR0_val, 4, 53);
    msdc_autok_read(host, SDIO_IP_WTMDPCR1, 1, &WTMDPCR1_val, 4, 53);
    
    test_length = 1024;
    buffer_recv = (unsigned int *)&buff_kmemory_test_mode[0];
    buffer_send = (unsigned int *)&buff_kmemory_test_mode[2048];
    
    // Read pattern test

      //32-bit read pattern
    WTMCR_val &= (~TEST_MODE_SELECT);
    msdc_autok_write(host, SDIO_IP_WTMCR, 1, &WTMCR_val, 4, 53);
    
    comp_pattern0 = DEFAULT_WTMDPCR0;
    msdc_autok_write(host, SDIO_IP_WTMDPCR0, 1, &comp_pattern0, 4, 53);
    
    msdc_autok_read(host, SDIO_IP_WTMDR, 1, buffer_recv, test_length, 53);
    for(i=0; i<(test_length/4); i++){
        if(comp_pattern0 != *(buffer_recv+i) ){
            printk("[%s]:[ERR] Read addr = %x, Read pattern = %x, expect be %x !!! \n",__func__, i*4, *(buffer_recv+i), comp_pattern0);
            return -1;
        }
    }
    
    printk("[%s]: Test mode of 32-bit Read Pass !!! \n", __func__);
    
    // Write pattern test

      //32-bit write pattern
    WTMCR_val &= (~TEST_MODE_SELECT);
    msdc_autok_write(host, SDIO_IP_WTMCR, 1, &WTMCR_val, 4, 53);
    
    comp_pattern1 = 0x00000000;
    msdc_autok_write(host, SDIO_IP_WTMDPCR1, 1, &comp_pattern1, 4, 53);
    memset(buffer_send, 0, test_length);
    msdc_autok_write(host, SDIO_IP_WTMDR, 1, buffer_send, test_length, 53);
    
    msdc_autok_read(host, SDIO_IP_WTMCR, 1, &WTMCR_val, 4, 53);
    if(WTMCR_val & TEST_MODE_STATUS){
        printk("[%s]:[ERR] Device compare data fail with all 0 at 32-bit mode !!! \n", __func__);
        return -1;
    }
}
#endif

/*************************************************************************
*************************************************************************/

/*************************************************************************
* FUNCTION
*  msdc_autok_stg2_cal
*
* DESCRIPTION
*  This function for auto-K at stage2
*
* PARAMETERS
*    host: msdc_host pointer
*    pData: pointer for autoK data came from stage1
*    len: number of byte data
*
* RETURN VALUES
*    error code: 0 success,
*               -1 parameter input error
*               -2 else error
*************************************************************************/
int msdc_autok_stg2_cal(struct msdc_host *host, void *pData, int len)
{
    U_AUTOK_INTERFACE_DATA *pAutok;
    //E_AUTOK_STATE state = AUTOK_CMD;
    E_RESULT_TYPE res= E_RESULT_ERR;

    if(len != (sizeof(S_AUTOK_DATA)*MAX_AUTOK_DAT_NUM))
        return -1;

    if(pData == NULL || host == NULL)
        return -1;

    pAutok =(U_AUTOK_INTERFACE_DATA *)pData;

    if((pAutok + E_AUTOK_VERSION)->version != AUTOK_VERSION_NO)
    {
        printk("autoK version wrong = %d\r\n", pAutok->version);
        return -2;
    }

    if((pAutok + E_AUTOK_FREQ)->freq != host->mclk)
    {
        printk("Now operation freq(%d) not meet autok data(%d)\r\n", host->mclk, pAutok->freq);
        return -2;
    }

    autok_tuning_parameter_init(host, TUNING_STG2, pAutok);

    res = autok_tune_algorithm(host, TUNING_STG2, pAutok);

    return -res;
}





