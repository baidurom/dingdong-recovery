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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief
 
#define LOG_TAG "CamShotTest"


#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <common/hw/hwstddef.h>

//
#include <camshot/ICamShot.h>
#include <camshot/ISingleShot.h>

extern "C" {
#include <pthread.h>
}

//
#include <drv/imem_drv.h>
#include <mtkcam/hal/sensor_hal.h>

#include "mtkcam/hal/aaa_hal_base.h"
using namespace NS3A;

using namespace NSCamShot; 
using namespace NSCamHW; 

/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)

#define POSTVIEW_WIDTH  640
#define POSTVEIW_HEIGHT 480 

static  IMemDrv *g_pIMemDrv;
static SensorHal *g_pSensorHal = NULL; 

static pthread_t g_CliKeyThreadHandle; 
static MBOOL g_bIsCLITest = MTRUE;

static EImageFormat g_eImgFmt[] = {eImgFmt_YUY2, eImgFmt_NV21, eImgFmt_I420, eImgFmt_YV16, eImgFmt_JPEG} ; 

static IMEM_BUF_INFO g_rRawMem; 
static IMEM_BUF_INFO g_rYuvMem; 
static IMEM_BUF_INFO g_rPostViewMem; 
static IMEM_BUF_INFO g_rJpegMem; 

static ImgBufInfo g_rRawBufInfo; 
static ImgBufInfo g_rYuvBufInfo; 
static ImgBufInfo g_rPostViewBufInfo; 
static ImgBufInfo g_rJpegBufInfo; 

static MUINT32 u4CapCnt = 0; 
static MUINT32 g_u4Width = 1280; 
static MUINT32 g_u4Height = 960; 
static MUINT32 g_u4SensorWidth = 0; 
static MUINT32 g_u4SensorHeight = 0; 
static MUINT32 g_GetCheckSumValue = 0;

// For CRC
static MUINT32 u4CRC = 0;
static MUINT32 u4CRCRef = 0;

//3A object  
Hal3ABase *m_p3AHal;

/*******************************************************************************
*  Calculates the CRC-8 of the first len bits in data
********************************************************************************/
static const MUINT32 ACDK_CRC_Table[256]=
{
    0x0,        0x4C11DB7,  0x9823B6E,  0xD4326D9,  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,  0x54BF6A4,  0x808D07D,  0xCC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x315D626,  0x7D4CB91,  0xA97ED48,  0xE56F0FF,  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x29F3D35,  0x65E2082,  0xB1D065B,  0xFDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};  // Table of 8-bit remainders

/******************************************************************************
* 
*******************************************************************************/
static void allocMem(IMEM_BUF_INFO &memBuf) 
{
    if (g_pIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (g_pIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
}

/******************************************************************************
* 
*******************************************************************************/
static void deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (g_pIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if (g_pIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }        
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL reallocMem(IMEM_BUF_INFO & rMemBuf, MUINT32 const u4Size )
{       
    //
    deallocMem(rMemBuf); 
    rMemBuf.size = u4Size; 
    //
    allocMem(rMemBuf);     
    return MTRUE; 
}


/******************************************************************************
* 
*******************************************************************************/
static MBOOL allocImgMem(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, IMEM_BUF_INFO & rMem)
{
    //
    MY_LOGD("[allocImgMem], (format, width, height) = (0x%x, %d, %d)",  eFmt, u4Width, u4Height); 
#warning [TODO] buffer size 
    // 422 format
    MUINT32 u4BufSize = u4Width * u4Height * 2;
    // 420 format
    if (eImgFmt_YV12 == eFmt 
       || eImgFmt_NV21 == eFmt 
       || eImgFmt_NV12 == eFmt
       || eImgFmt_I420 == eFmt)
    {
        u4BufSize = u4Width * u4Height * 3 / 2; 
    }
    else if (eImgFmt_RGB888 == eFmt) 
    {
        u4BufSize = u4Width * u4Height * 3; 
    }  
    else if (eImgFmt_ARGB888 == eFmt) 
    {
        u4BufSize = u4Width * u4Height * 4; 
    }
    //
    if (0 == rMem.size) 
    {        
        rMem.size = (u4BufSize  + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
        allocMem(rMem); 
        MY_LOGD("[allocImgMem] (va, pa, size) = (0x%x, 0x%x, %d)",  rMem.virtAddr, rMem.phyAddr, rMem.size);  
    }
    else 
    {
        if (rMem.size < u4BufSize) 
        {          
            reallocMem(rMem, u4BufSize); 
            MY_LOGD("[allocImgMem] re-allocate (va, pa, size) = (0x%x, 0x%x, %d)", rMem.virtAddr, rMem.phyAddr, rMem.size);  
        }
    }  
    return MTRUE; 
}



/******************************************************************************
* save the buffer to the file 
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{ 
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, 0x%x, %d)", fname, buf, size); 
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true; 
}


/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    MY_LOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            MY_LOGE("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        readCnt += nr;
        cnt++;
    }
    MY_LOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}


/////////////////////////////////////////////////////////////////////////
//! Nucamera commands
/////////////////////////////////////////////////////////////////////////
typedef struct CLICmd_t
{
    //! Command string, include shortcut key
    const char *pucCmdStr;

    //! Help string, include functionality and parameter description
    const char *pucHelpStr;

    //! Handling function
    //! \param a_u4Argc  [IN] Number of arguments plus 1
    //! \param a_pprArgv [IN] Array of command and arguments, element 0 is
    //!                       command string
    //! \return error code
    //FIXME: return MRESULT is good?
    MUINT32 (*handleCmd)(const int argc, char** argv);

} CLICmd;



/******************************************************************************
* 
*******************************************************************************/
static MBOOL fgCamShotNotifyCb(MVOID* user, CamShotNotifyInfo const msg)
{
    return MTRUE;
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handleBayerDataCallback(CamShotDataInfo const msg) 
{
    MY_LOGD("handleBayerDataCallback"); 
    //char fileName[256] = {'\0'}; 
    //sprintf(fileName, "/data/bayer_%ux%u_%02d.raw", g_u4SensorWidth, g_u4SensorHeight, u4CapCnt); 
    //saveBufToFile(fileName, msg.puData, msg.u4Size); 

    // For CRC Check
    register MUINT32 crc_accum = 0;
    MUINT32 size = msg.u4Size;
    MUINT8 *out_buffer = msg.puData;

    MY_LOGD("[Camera_check_crc]\n");
    while (size-- > 0)
    {
        crc_accum = (crc_accum << 8) ^ ACDK_CRC_Table[(MUINT8)(crc_accum >> 24) ^ (*out_buffer++)];
    }

#if 0    
    if(u4CapCnt ==0)
    {
        u4CRCRef = ~crc_accum;
        printf("CRC_accum 0x%x\n", ~crc_accum);
    }
    else
#endif

    u4CRC = ~crc_accum;

    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handleYuvDataCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleYuvDataCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/yuv_%ux%u_%02d.yuv", g_u4Width, g_u4Height, u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 
    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handlePostViewCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handlePostViewCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/postview%02d.yuv", u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 
    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handleJpegCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleJpegCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/jpeg%02d.jpg", u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 
 
    memset(fileName, '\0', 256); 
    sprintf(fileName, "/data/thumb%02d.jpg", u4CapCnt); 
    saveBufToFile(fileName, reinterpret_cast<MUINT8*> (msg.ext1), msg.ext2); 
 
    return 0; 
}



/******************************************************************************
* 
*******************************************************************************/
static MBOOL fgCamShotDataCb(MVOID* user, CamShotDataInfo const msg)
{
    CamShotDataInfo rDataInfo = msg; 

    switch (rDataInfo.msgType) 
    {
        case ECamShot_DATA_MSG_BAYER:
            handleBayerDataCallback(msg); 
        break; 
        case ECamShot_DATA_MSG_YUV:
            handleYuvDataCallback(msg); 
        break; 
        case ECamShot_DATA_MSG_POSTVIEW:
            handlePostViewCallback(msg);             
        break; 
        case ECamShot_DATA_MSG_JPEG:
            handleJpegCallback(msg); 
        break; 
  
    }

    return MTRUE;
}


static MVOID allocateMem(MUINT32 u4EnableMsg)
{
#warning [TODO] allocate memory according to the format
    if (u4EnableMsg & ECamShot_DATA_MSG_BAYER) 
    {
        allocImgMem(eImgFmt_YUY2,g_u4SensorWidth ,g_u4SensorHeight , g_rRawMem); 
        
        g_rRawBufInfo.u4ImgWidth = g_u4SensorWidth;
        g_rRawBufInfo.u4ImgHeight = g_u4SensorHeight; 
        g_rRawBufInfo.eImgFmt = eImgFmt_BAYER8; 
        g_rRawBufInfo.u4Stride[0] = g_u4SensorWidth; 
        g_rRawBufInfo.u4BufSize = g_rRawMem.size; 
        g_rRawBufInfo.u4BufVA = g_rRawMem.virtAddr;
        g_rRawBufInfo.u4BufPA = g_rRawMem.phyAddr;
        g_rRawBufInfo.i4MemID = g_rRawMem.memID; 

    }
    
    if (u4EnableMsg & ECamShot_DATA_MSG_YUV) 
    {
        EImageFormat eFmt = eImgFmt_YUY2; 
        allocImgMem(eFmt, g_u4Width, g_u4Height, g_rYuvMem);
        g_rYuvBufInfo.u4ImgWidth = g_u4Width;
        g_rYuvBufInfo.u4ImgHeight = g_u4Height; 
        g_rYuvBufInfo.eImgFmt = eFmt; 
        g_rYuvBufInfo.u4Stride[0] = g_u4Width; 
        g_rYuvBufInfo.u4Stride[1] = g_u4Width ; 
        g_rYuvBufInfo.u4Stride[2] = 0; 
        g_rYuvBufInfo.u4BufSize = g_rYuvMem.size; 
        g_rYuvBufInfo.u4BufVA = g_rYuvMem.virtAddr;
        g_rYuvBufInfo.u4BufPA = g_rYuvMem.phyAddr;
        g_rYuvBufInfo.i4MemID = g_rYuvMem.memID; 
    }
   
    if (u4EnableMsg & ECamShot_DATA_MSG_POSTVIEW) 
    {
        //allocImgMem(eImgFmt_YUY2, 640, 480, g_rPostViewMem); 
        MUINT32 u4Width = 640; 
        MUINT32 u4Height = 480; 
        EImageFormat eFmt = eImgFmt_YUY2; 
        allocImgMem(eFmt, u4Width, u4Height, g_rPostViewMem); 
        g_rPostViewBufInfo.u4ImgWidth = u4Width; ;
        g_rPostViewBufInfo.u4ImgHeight = u4Height; ; 
        g_rPostViewBufInfo.eImgFmt = eFmt; 
        //g_rPostViewBufInfo.u4Stride[0] = (~31) & (31 + u4Width); 
        //g_rPostViewBufInfo.u4Stride[1] = (~15) & (15 + (u4Width >> 1)); 
        //g_rPostViewBufInfo.u4Stride[2] = (~15) & (15 + (u4Width >> 1)); 
        g_rPostViewBufInfo.u4Stride[0] = u4Width; 
        g_rPostViewBufInfo.u4Stride[1] = u4Width ; 
        g_rPostViewBufInfo.u4Stride[2] = u4Width ;

        g_rPostViewBufInfo.u4BufSize = g_rPostViewMem.size; 
        g_rPostViewBufInfo.u4BufVA = g_rPostViewMem.virtAddr;
        g_rPostViewBufInfo.u4BufPA = g_rPostViewMem.phyAddr;
        g_rPostViewBufInfo.i4MemID = g_rPostViewMem.memID; 
    }
    
    if (u4EnableMsg & ECamShot_DATA_MSG_JPEG) 
    {
        allocImgMem(eImgFmt_YUY2, g_u4Width/2, g_u4Height/2, g_rJpegMem);   
        g_rJpegBufInfo.u4ImgWidth = g_u4Width;
        g_rJpegBufInfo.u4ImgHeight = g_u4Height; 
        g_rJpegBufInfo.eImgFmt = eImgFmt_JPEG; 
        g_rJpegBufInfo.u4Stride[0] = g_u4Width; 
        g_rJpegBufInfo.u4BufSize = g_rJpegMem.size; 
        g_rJpegBufInfo.u4BufVA = g_rJpegMem.virtAddr;
        g_rJpegBufInfo.u4BufPA = g_rJpegMem.phyAddr;
        g_rJpegBufInfo.i4MemID = g_rJpegMem.memID; 
    }
}

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4Capture_Cmd(int argc, char** argv)
{
    return 0; 
}
/******************************************************************************
* 
*******************************************************************************/
#define ALLOCA_MEM    0
//static MUINT32 u4Capture_Cmd(int argc, char** argv)
static MUINT32 Capture_Test(void)
{
    MUINT32 u4Mode = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG; //
    MUINT32 u4EnableMsg = 0x1;    //raw output
    MUINT32 u4Rot = 0; 
    MUINT32 u4ShotCnt = 1; 
    u4CapCnt = 0; 
    g_u4Width = 640;
    g_u4Height = 480;
    //{"cap", "cap <mode:0:prv, 1:cap> <width> <height> <img:0x1:raw, 0x2:yuv, 0x4:postview, 0x8:jpeg> <rot:0, 1:90, 2:180, 3:270> <Count>", u4Capture_Cmd},

    /*if (argc == 1) 
    {
        u4Mode = atoi(argv[0]); 
    }
    else if (argc == 3) 
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
    }
    else if (argc == 4) 
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
        sscanf( argv[3],"%x", &u4EnableMsg); 
    }
    else if (argc == 5) 
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
        sscanf( argv[3],"%x", &u4EnableMsg); 
        u4Rot = atoi(argv[4]); 
    }
    else if (argc == 6)
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
        sscanf( argv[3],"%x", &u4EnableMsg); 
        u4Rot = atoi(argv[4]); 
        u4ShotCnt = atoi(argv[5]); 

    }*/

    MUINT32 cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE; 
    if (u4Mode == 0)  
    {
        cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE; 
    }
    else if (1 == u4Mode) 
    {
        cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;    
    } 

    g_pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                             cmd,
                             (int)&g_u4SensorWidth,
                             (int)&g_u4SensorHeight,
                             0
                            );    


    MY_LOGD("sensor width:%d, height:%d\n", g_u4SensorWidth, g_u4SensorHeight); 
    MY_LOGD("capture width:%d, height:%d\n, mode:%d, image:0x%x, count:%d\n", g_u4Width, g_u4Height, u4Mode, u4EnableMsg, u4ShotCnt); 

    //get sensor test pattern checksum value 
    g_pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                    SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,
                                    (int)&g_GetCheckSumValue,
                                    0,
                                    0);
    MY_LOGD("CheckValue 0x%x\n",g_GetCheckSumValue);
    // 3a
    if(m_p3AHal == NULL)
    {
        m_p3AHal = Hal3ABase::createInstance(SENSOR_DEV_MAIN);
    }
    //3AHal control
    m_p3AHal->setIspProfile(EIspProfile_NormalPreview);

    //3A scenario control
    m_p3AHal->sendCommand(ECmd_CameraPreviewStart);
    
    m_p3AHal->sendCommand(ECmd_PrecaptureStart);
    m_p3AHal->sendCommand(ECmd_PrecaptureEnd);
    m_p3AHal->sendCommand(ECmd_CameraPreviewEnd);

    //
    ISingleShot *pSingleShot = ISingleShot::createInstance(eShotMode_NormalShot, "testshot"); 
    // 
    pSingleShot->init(); 
 
    // 
    pSingleShot->enableDataMsg(u4EnableMsg ); 
    // set buffer 
    //
#if ALLOCA_MEM
    allocateMem(u4EnableMsg); 
    //
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_BAYER, g_rRawBufInfo); 
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_YUV, g_rYuvBufInfo); 
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_POSTVIEW, g_rPostViewBufInfo); 
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_JPEG, g_rJpegBufInfo); 
#endif 
   

    // shot param 
    ShotParam rShotParam(eImgFmt_YUY2,           //yuv format 
                         g_u4Width,              //picutre width 
                         g_u4Height,             //picture height
                         u4Rot * 90,             //picutre rotation 
                         0,                      //picutre flip 
                         eImgFmt_YV12,           //postview format 
                         800,                    //postview width 
                         480,                    //postview height 
                         0,                      //postview rotation 
                         0,                      //postview flip 
                         100                     //zoom   
                        );                                  
 
    // jpeg param 
    JpegParam rJpegParam(ThumbnailParam(160, 128, 100, MTRUE),
                         90,                     //Quality 
                         MTRUE                   //isSOI 
                        ); 
 
    // thumbnail param 
    ThumbnailParam rThumbnailParam(160,          // thumbnail width
                                   128,          // thumbnail height
                                   100,          // quality 
                                   MTRUE         // isSOI     
                                  ); 

    // sensor param 
    SensorParam rSensorParam(SENSOR_DEV_MAIN,                         //Device ID 
               u4Mode == 0 ? ACDK_SCENARIO_ID_CAMERA_PREVIEW : ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio 
                             10,                                       //bit depth 
                             MFALSE,                                   //bypass delay 
                             MFALSE,                                  //bypass scenario 
                             2                                        // Raw type 0: pure raw 1:pre-post raw 2: test pattern
                            );  

    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, NULL); 
    //     
    pSingleShot->setShotParam(rShotParam); 
    //
    pSingleShot->setJpegParam(rJpegParam); 
    // 
    // 
    for (MUINT32 i = 0 ; i < u4ShotCnt; i++) 
    {
        pSingleShot->startOne(rSensorParam); 
        
        u4CapCnt++; 
    }
    //
    pSingleShot->uninit(); 
    //
    pSingleShot->destroyInstance(); 
    
    if(m_p3AHal != NULL)
    {
        m_p3AHal->destroyInstance();
    }

    return 0; 
}


/////////////////////////////////////////////////////////////////////////
//
//!  The cli command for the manucalibration
//!
/////////////////////////////////////////////////////////////////////////
static CLICmd g_rTest_Cmds[] =
{
    {"cap", "cap <mode:0:prv, 1:cap> <width> <height> <img:0x1:raw, 0x2:yuv, 0x4:postview, 0x8:jpeg> <rot:0, 1:90, 2:180, 3:270> <Count>", u4Capture_Cmd},
    {NULL, NULL, NULL}
};


/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param sig: The input arguments 
/////////////////////////////////////////////////////////////////////////
static void thread_exit_handler(int sig)
{ 
    printf("This signal is %d \n", sig);
    pthread_exit(0);
}    

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
static void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;
    
    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


//  vHelp () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
static void vHelp()
{
    printf("\n***********************************************************\n");
    printf("* CamShot SingleShot CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n"); 
    printf("===========================================================\n");    

    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");

    int i = 0; 
    for (i = 0; ; i++)
    {
        if (NULL == g_rTest_Cmds[i].pucCmdStr) 
        {
            break; 
        } 
        printf("%s    [%s]\n", g_rTest_Cmds[i].pucCmdStr, 
                               g_rTest_Cmds[i].pucHelpStr);        
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param a_pArg: The input arguments 
/////////////////////////////////////////////////////////////////////////
static void* cliKeyThread (void *a_pArg)
{
    char urCmds[256] = {0}; 

    //! ************************************************
    //! Set the signal for kill self thread 
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal 
    //! to kill self 
    //! ************************************************    
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions)); 
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0; 
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);
    
    while (1)
    {
        printf("Input Cmd#"); 
        fgets(urCmds, 256, stdin);

        //remove the '\n' 
        urCmds[strlen(urCmds)-1] = '\0';         
        char *pCmds = &urCmds[0];         
        //remove the space in the front of the string 
        vSkipSpace(&pCmds); 
       
        //Ignore blank command 
        if (*pCmds == '\0')
        {
            continue; 
        }
        
        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        char  *pucStrToken, *pucCmdToken;
        char  *pucArgValues[25];
        
        pucStrToken = (char *)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] =(char*) pucStrToken;
            pucStrToken = (char*)strtok (NULL, " ");                
        }

        if (u4ArgCount == 0)
        {
            continue; 
        }
        
        pucCmdToken = (char*) pucArgValues[0]; 

        //parse the command 
        if ((strcmp((char *)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp(); 
        }
        else if ((strcmp((char *)pucCmdToken, "exit") == 0) || 
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            printf("Exit From CLI\n"); 
            g_bIsCLITest = MFALSE; 
            break; 
        }
        else
        {
            MBOOL bIsFoundCmd = MFALSE;
            for (MUINT32 u4CmdIndex = 0; ; u4CmdIndex++)
            {
                if(NULL == g_rTest_Cmds[u4CmdIndex].pucCmdStr)
                {
                    break; 
                }  
                if (strcmp((char *)pucCmdToken, g_rTest_Cmds[u4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE; 
                    g_rTest_Cmds[u4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);                     
                    break;
                }                
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n"); 
            }            
        }        
            
    }

    return 0; 
}




/*******************************************************************************
*  Main Function 
********************************************************************************/
int main_acdksingleshot(int argc, char** argv)
{
    //printf("SingleShot Test \n");     

    //vHelp(); 
    printf("[CAM : VSS Test]\n");  
    g_pIMemDrv =  IMemDrv::createInstance(); 
    if (NULL == g_pIMemDrv)
    {
        MY_LOGE("g_pIMemDrv is NULL"); 
        return 0; 
    }
    g_pIMemDrv->init(); 


    // init sensor first 
    g_pSensorHal = SensorHal:: createInstance();

    if (NULL == g_pSensorHal) 
    {
        MY_LOGE("pSensorHal is NULL"); 
        return 0; 
    }
    // search sensor 
    g_pSensorHal->searchSensor();

    //
    // (1). init main sensor  
    //
    g_pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                    SENSOR_CMD_SET_SENSOR_DEV,
                                    0,
                                    0,
                                    0); 

    //
    g_pSensorHal->init(); 

    //
    Capture_Test();
    
    //u4CRCRef = g_GetCheckSumValue;

    if(g_GetCheckSumValue == u4CRC)
    {
        printf("Cam Test Pass\n");
    }
    else
    {
        printf("Cam Test Fail\n");
    }
    
    g_bIsCLITest = MFALSE;

    //pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL); 

    //!***************************************************
    //! Main thread wait for exit 
    //!***************************************************    
    while (g_bIsCLITest== MTRUE)
    {
        usleep(100000); 
    }

    //
    //
    g_pSensorHal->uninit(); 
    //
    g_pSensorHal->destroyInstance(); 
    //
    if (0 != g_rRawMem.size)
    {
        deallocMem(g_rRawMem); 
    }

    if (0 != g_rYuvMem.size)
    {
        deallocMem(g_rYuvMem); 
    }

    if (0 != g_rPostViewMem.size)
    {
        deallocMem(g_rPostViewMem); 
    }

    if (0 != g_rJpegMem.size)
    {
        deallocMem(g_rJpegMem); 
    }

    g_pIMemDrv->uninit(); 
    g_pIMemDrv->destroyInstance(); 

    printf("[Test END]\n");


    return 0; 
}
