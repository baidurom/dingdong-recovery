/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "TpipeDrv"
//
#include <utils/Errors.h>
//#include <cutils/pmem.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <sys/ioctl.h>

#include "drv_types.h"
#include "tpipe_drv_imp.h"

#include <cutils/properties.h>  // For property_get().
#include "CameraProfile.h"  // For CPTLog*() CameraProfile APIS.


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{TpipeDrv} "
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(tpipe_drv);
//EXTERN_DBG_LOG_VARIABLE(tpipe_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (tpipe_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (tpipe_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (tpipe_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (tpipe_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (tpipe_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (tpipe_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)



#define TDRI_ALIGNMENT_UP( _number_, _power_of_2_ ) (((((size_t)_number_) + (( 0x1 << (_power_of_2_) )-1)) >> (_power_of_2_) ) << (_power_of_2_))
#define TPIPE_DRV_INIT_MAX        5



class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
//        if  (0==mIdx)
//        {
//            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            LOG_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
        mi4LastUs = i4EndUs;

	    //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};


#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 1   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog((CPT_Event)G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr((CPT_Event)G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable((CPT_Event)EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr((CPT_Event)G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

#define ISP_SCENARIO_TPIPE_IC         0
#define ISP_SCENARIO_TPIPE_VR         1
#define ISP_SCENARIO_TPIPE_ZSD        2
#define ISP_SCENARIO_TPIPE_IP         3
#define ISP_SCENARIO_TPIPE_VEC        4
#define ISP_SCENARIO_TPIPE_RESERVE01  5
#define ISP_SCENARIO_TPIPE_N3D_IC     6
#define ISP_SCENARIO_TPIPE_N3D_VR     7
#define ISP_SCENARIO_TPIPE_MAX        8
//
#define ISP_TPIPE_SUB_MODE_RAW        0
#define ISP_TPIPE_SUB_MODE_YUV        1
#define ISP_TPIPE_SUB_MODE_RGB        2
#define ISP_TPIPE_SUB_MODE_JPG        3
#define ISP_TPIPE_SUB_MODE_MFB        4
#define ISP_TPIPE_SUB_MODE_VEC        0
#define ISP_TPIPE_SUB_MODE_RGB_LOAD   3
#define ISP_TPIPE_SUB_MODE_MAX        5


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
TpipeDrvImp::TpipeDrvImp():m_pImemDrv(NULL)
{
    LOG_DBG("TpipeDrvImp()");
    //
    GLOBAL_PROFILING_LOG_START(Event_TpipeDrv);	// Profiling Start.
    mInitCount = 0;
    fdConfigTpipeStruct = -1;
    fdConfigTdriInfo = -1;
    fdKeepTpipeInfo = -1;
}


//-----------------------------------------------------------------------------
TpipeDrvImp::~TpipeDrvImp()
{
    LOG_DBG("~TpipeDrvImp()");
    //
    GLOBAL_PROFILING_LOG_END(); 	// Profiling End.
}


//-----------------------------------------------------------------------------
TpipeDrv*
TpipeDrv::createInstance(void)
{
    DBG_LOG_CONFIG(drv, tpipe_drv);

    return TpipeDrvImp::getInstance();
}

//-----------------------------------------------------------------------------
TpipeDrv*
TpipeDrvImp::getInstance(void)
{
    static TpipeDrvImp Singleton;
    //
    LOG_DBG("&Singleton(0x%x)\n",&Singleton);
    //
    return &Singleton;
}

//-----------------------------------------------------------------------------
void
TpipeDrvImp::destroyInstance(void)
{
    LOG_DBG("");
}

//-----------------------------------------------------------------------------
MBOOL
TpipeDrvImp::init(void)
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("init TpieDrv");

    //
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    //
    LOG_INF("mInitCount(%d)",mInitCount);
    //
    if(mInitCount == 0) {
        int i;
        dbgBufIdx = 0;
        //
        //tdri data buffer
        tpipeAlgoStructSize = TDRI_ALIGNMENT_UP(sizeof(ISP_TPIPE_CONFIG_STRUCT), 2); // 4 alignment
        //
        if ( NULL == m_pImemDrv ) {
            m_pImemDrv = IMemDrv::createInstance();
            m_pImemDrv->init();
        } else {
            LOG_ERR("[Error] m_pImemDrv(0x%x) not equal to null\n",m_pImemDrv);
        }
        //
        // tpipe main working buffer
        m_tileDataInfo.size = tpipeAlgoStructSize;
        m_pImemDrv->allocVirtBuf(&m_tileDataInfo);
        fdConfigTpipeStruct = (MINT32)m_tileDataInfo.memID;
        pConfigTpipeStruct = (ISP_TPIPE_CONFIG_STRUCT *)m_tileDataInfo.virtAddr;
        memset(pConfigTpipeStruct,0x00,sizeof(ISP_TPIPE_CONFIG_STRUCT));
        LOG_DBG("pConfigTpipeStruct=0x%x tpipeAlgoStructSize=%d\n",pConfigTpipeStruct, tpipeAlgoStructSize);
        if(pConfigTpipeStruct == 0) {
            fdConfigTpipeStruct = -1;
            Result = MFALSE;
            LOG_ERR("can't alloc ISP_TPIPE_CONFIG_TOP_STRUCT\n");
            goto EXIT;
        } else if ((int)pConfigTpipeStruct & 0x03) {
            fdConfigTpipeStruct = -1;
            Result = MFALSE;
            LOG_ERR("pConfigTpipeStruct not 4 alignment pConfigTpipeStruct(0x%x) tpipeAlgoStructSize(%d)\n",pConfigTpipeStruct,tpipeAlgoStructSize);
            goto EXIT;
        }
        //
        // for tpipe debug
        for(i=0; i<TPIPE_DBG_BUFFER_NUM; i++){
            m_DbgTpipeInfo[i].size = tpipeAlgoStructSize;
            m_pImemDrv->allocVirtBuf(&m_DbgTpipeInfo[i]);
            fdDbgTpipeStruct[i] = (MINT32)m_DbgTpipeInfo[i].memID;
            pDbgTpipeStruct[i] = (ISP_TPIPE_CONFIG_STRUCT *)m_DbgTpipeInfo[i].virtAddr;
            memset(pDbgTpipeStruct[i],0x00,sizeof(ISP_TPIPE_CONFIG_STRUCT));
            LOG_DBG("pDbgTpipeStruct[%d]=0x%x tpipeAlgoStructSize=%d\n",i,pDbgTpipeStruct[i], tpipeAlgoStructSize);
            if(pDbgTpipeStruct[i] == 0) {
                fdDbgTpipeStruct[i] = -1;
                Result = MFALSE;
                LOG_ERR("can't alloc ISP_TPIPE_CONFIG_TOP_STRUCT\n");
                goto EXIT;
            } else if ((int)pDbgTpipeStruct[i] & 0x03) {
                fdDbgTpipeStruct[i] = -1;
                Result = MFALSE;
                LOG_ERR("pConfigTpipeStruct not 4 alignment pDbgTpipeStruct[%d]=0x%x tpipeAlgoStructSize(%d)\n",i,pDbgTpipeStruct[i],tpipeAlgoStructSize);
                goto EXIT;
            }
        }
        //
        // keep tpip info for turning function
        for(i =TPIPE_DRV_CQ01; i < TPIPE_DRV_CQ_NUM;i++) {
            m_KeepTpipeInfo[i].size = sizeof(TdriDrvCfg);
            m_pImemDrv->allocVirtBuf(&m_KeepTpipeInfo[i]);
            fdKeepTpipeInfo = (MINT32)m_KeepTpipeInfo[i].memID;
            pKeepTdriInfo[i] = (TdriDrvCfg *)m_KeepTpipeInfo[i].virtAddr;
            LOG_DBG("pKeepTdriInfo[%d]=0x%x tpipeAlgoStructSize=%d\n",i,pKeepTdriInfo[i], tpipeAlgoStructSize);
            if(pKeepTdriInfo[i] == 0) {
                fdKeepTpipeInfo = -1;
                Result = MFALSE;
                LOG_ERR("can't alloc ISP_TPIPE_CONFIG_TOP_STRUCT\n");
                goto EXIT;
            } else if ((int)pKeepTdriInfo[i] & 0x03) {
                fdKeepTpipeInfo = -1;
                Result = MFALSE;
                LOG_ERR("pConfigTpipeStruct not 4 alignment pKeepTdriInfo[%d](0x%x) tpipeAlgoStructSize(%d)\n",i,pKeepTdriInfo[i],tpipeAlgoStructSize);
                goto EXIT;
            }

            memset(pKeepTdriInfo[i],0x00,sizeof(TdriDrvCfg));
        }



        //tpipelib working buffer
        int tpipeWorkingSize;
        tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(TPIPE_DRV_MAX_TPIPE_NUM);
        tpipeWorkingSize = TDRI_ALIGNMENT_UP(tpipeWorkingSize, 2); // 4 alignment
        if ( NULL != m_pImemDrv ) {
            //
            m_WBInfo.size = tpipeWorkingSize;
            m_pImemDrv->allocVirtBuf(&m_WBInfo);
        } else {
            LOG_ERR("[Error] m_pImemDrv(0x%x) not equal to null\n",m_pImemDrv);
        }
    }

    //
    if(mInitCount >= TPIPE_DRV_INIT_MAX) {
        LOG_ERR("over max mInitCount(%d)",mInitCount);
        Result = MFALSE;
        goto EXIT;
    }
    //
    android_atomic_inc(&mInitCount);

    //
    EXIT:
    LOG_INF("X",mInitCount);
    //
    return Result;
}



//-----------------------------------------------------------------------------
MBOOL
TpipeDrvImp::uninit(void)
{
    MBOOL Result = MTRUE;
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("Uninit TpieDrv");
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_INF("mInitCount(%d)",mInitCount);
    //
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0) {
        goto EXIT;
    }
    //
    if(mInitCount == 0) {
        int i;

        //release tdri data buffer
        //m_tileDataInfo is filled by alloc
        if ( m_pImemDrv->freeVirtBuf(&m_tileDataInfo) ) {
            LOG_ERR("ERROR:m_pImemDrv->freeVirtBuf");
            Result = MFALSE;
            goto EXIT;
        }
        m_tileDataInfo.size = 0;
        m_tileDataInfo.memID = -1;
        m_tileDataInfo.virtAddr = NULL;


        //release keep tpipe info
        for(i =TPIPE_DRV_CQ01; i < TPIPE_DRV_CQ_NUM;i++) {
            if ( m_pImemDrv->freeVirtBuf(&m_KeepTpipeInfo[i]) ) {
                LOG_ERR("ERROR:m_pImemDrv->freeVirtBuf(m_KeepTpipeInfo)");
                Result = MFALSE;
                goto EXIT;
            }
            m_KeepTpipeInfo[i].size = 0;
            m_KeepTpipeInfo[i].memID = -1;
            m_KeepTpipeInfo[i].virtAddr = NULL;
        }

        //release tpipe debug table
        for(i=0; i<TPIPE_DBG_BUFFER_NUM; i++){
            if ( m_pImemDrv->freeVirtBuf(&m_DbgTpipeInfo[i]) ) {
                LOG_ERR("ERROR:m_pImemDrv->freeVirtBuf(m_DbgTpipeInfo)");
                Result = MFALSE;
                goto EXIT;
            }
            m_DbgTpipeInfo[i].size = 0;
            m_DbgTpipeInfo[i].memID = -1;
            m_DbgTpipeInfo[i].virtAddr = NULL;
        }


        //release tpipelib working buffer
        //bufInfo is filled by alloc
        if ( m_pImemDrv->freeVirtBuf(&m_WBInfo) ) {
            LOG_ERR("ERROR:m_pImemDrv->freeVirtBuf");
            Result = MFALSE;
            goto EXIT;
        }
        m_WBInfo.size = 0;
        m_WBInfo.memID = -1;
        m_WBInfo.virtAddr = NULL;

        m_pImemDrv->uninit();
        m_pImemDrv->destroyInstance();
        m_pImemDrv = NULL;
    }
    //
    EXIT:
    LOG_INF("X",mInitCount);
    return Result;
}



MBOOL TpipeDrvImp::
updateImageIO(
    TdriDrvCfg *pDst,
    TdriDrvCfg* pSrc
)
{
    pDst->top.scenario = pSrc->top.scenario;
    pDst->top.mode = pSrc->top.mode;
    pDst->top.debug_sel = pSrc->top.debug_sel;
    pDst->top.pixel_id = pSrc->top.pixel_id;
    pDst->top.cam_in_fmt = pSrc->top.cam_in_fmt;
    pDst->top.unp_en = pSrc->top.unp_en;
    pDst->top.c02_en = pSrc->top.c02_en;
    pDst->top.c24_en = pSrc->top.c24_en;
    pDst->top.c42_en = pSrc->top.c42_en;
    pDst->top.fe_sel = pSrc->top.fe_sel;
    pDst->top.fe_en = pSrc->top.fe_en;
    pDst->top.cfa_en = pSrc->top.cfa_en; // switch(on/off) control by FeatureIO
    pDst->top.mfb_en = pSrc->top.mfb_en; // switch(on/off) control by FeatureIO
    pDst->top.nr3d_en = pSrc->top.nr3d_en;
    pDst->top.prz_opt_sel = pSrc->top.prz_opt_sel;
    pDst->top.nr3d_dma_sel = pSrc->top.nr3d_dma_sel;

    // mfb need a initial value
    if(pDst->top.mfb_en && (pDst->tuningFunc.mfb.bld_mode==0)) {
        ::memcpy((char*)&pDst->tuningFunc.mfb, (char*)&pSrc->tuningFunc.mfb, sizeof(TdriMfbCfg));
    }

    pDst->top.prz_en = pSrc->top.prz_en;
    pDst->top.disp_vid_sel = pSrc->top.disp_vid_sel;
    pDst->top.g2g2_en = pSrc->top.g2g2_en;
    ::memcpy((char*)&pDst->tdriCfg, (char*)&pSrc->tdriCfg, sizeof(TdriDMACfg));
    //
    pDst->top.imgi_en = pSrc->top.imgi_en;
    ::memcpy((char*)&pDst->imgi, (char*)&pSrc->imgi, sizeof(TdriRingInDMACfg));

    pDst->top.vipi_en = pSrc->top.vipi_en;
    ::memcpy((char*)&pDst->vipi, (char*)&pSrc->vipi, sizeof(TdriRingInDMACfg));

    pDst->top.vip2i_en = pSrc->top.vip2i_en;
    ::memcpy((char*)&pDst->vip2i, (char*)&pSrc->vip2i, sizeof(TdriRingInDMACfg));

    pDst->top.flki_en = pSrc->top.flki_en;
    ::memcpy((char*)&pDst->flki, (char*)&pSrc->flki, sizeof(TdriFlkiCfg));


    if(pDst->top.scenario==ISP_SCENARIO_TPIPE_IP && pDst->top.mode==ISP_TPIPE_SUB_MODE_MFB){
        pDst->top.imgci_en = pSrc->top.imgci_en;
        ::memcpy((char*)&pDst->imgci_stride, (char*)&pSrc->imgci_stride, sizeof(int));

        pDst->top.lcei_en = pSrc->top.lcei_en;
        ::memcpy((char*)&pDst->lcei_stride, (char*)&pSrc->lcei_stride, sizeof(int));

        pDst->top.lsci_en = pSrc->top.lsci_en;
        ::memcpy((char*)&pDst->lsci_stride, (char*)&pSrc->lsci_stride, sizeof(int));
    }

    pDst->top.imgo_en = pSrc->top.imgo_en;
    ::memcpy((char*)&pDst->imgo, (char*)&pSrc->imgo, sizeof(TdriImgoCfg));

    pDst->top.img2o_en = pSrc->top.img2o_en;
    ::memcpy((char*)&pDst->img2o, (char*)&pSrc->img2o, sizeof(TdriImg2oCfg));
	pDst->img2o.img2o_xsize = 0;  //kk test
	pDst->img2o.img2o_ysize = 0;

    pDst->top.esfko_en = pSrc->top.esfko_en;
    ::memcpy((char*)&pDst->esfko, (char*)&pSrc->esfko, sizeof(TdriEsfkoCfg));

    pDst->top.aao_en = pSrc->top.aao_en;
    ::memcpy((char*)&pDst->aao, (char*)&pSrc->aao, sizeof(TdriAaoCfg));

    pDst->top.lcso_en = pSrc->top.lcso_en;
    ::memcpy((char*)&pDst->lcso, (char*)&pSrc->lcso, sizeof(TdriLcsoCfg));

    pDst->top.vido_en = pSrc->top.vido_en;
    ::memcpy((char*)&pDst->vido, (char*)&pSrc->vido, sizeof(TdriRingOutDMACfg));

    pDst->top.dispo_en = pSrc->top.dispo_en;
    ::memcpy((char*)&pDst->dispo, (char*)&pSrc->dispo, sizeof(TdriRingOutDMACfg));

    pDst->top.cdrz_en = pSrc->top.cdrz_en;
    ::memcpy((char*)&pDst->cdrz, (char*)&pSrc->cdrz, sizeof(TdriCdrzCfg));

    pDst->top.curz_en = pSrc->top.curz_en;
    ::memcpy((char*)&pDst->curz, (char*)&pSrc->curz, sizeof(TdriCurzCfg));

    pDst->top.prz_en = pSrc->top.prz_en;
    ::memcpy((char*)&pDst->prz, (char*)&pSrc->prz, sizeof(TdriPrzCfg));

    pDst->top.fe_en = pSrc->top.fe_en;
    ::memcpy((char*)&pDst->fe, (char*)&pSrc->fe, sizeof(TdriFeCfg));

    ::memcpy((char*)&pDst->tdriPerformCfg, (char*)&pSrc->tdriPerformCfg, sizeof(TdriPerformCfg));

    return MTRUE;
}


MBOOL TpipeDrvImp::
updateFeatureIO(
    TdriDrvCfg* pDst,
    TdriDrvCfg *pSrc,
    int partUpdateFlag
)
{

    LOG_DBG("partUpdateFlag=0x%x scenario(%d)\n",partUpdateFlag,pDst->top.scenario);

    if(partUpdateFlag & TPIPE_DRV_UPDATE_BNR) {
        pDst->top.bnr_en = pSrc->top.bnr_en;
        ::memcpy((char*)&pDst->tuningFunc.bnr, (char*)&pSrc->tuningFunc.bnr, sizeof(TdriBnrCfg));

        if(pDst->top.scenario!=ISP_SCENARIO_TPIPE_IP || pDst->top.mode!=ISP_TPIPE_SUB_MODE_MFB){
            pDst->top.imgci_en = pSrc->top.imgci_en;
            pDst->imgci_stride = pSrc->imgci_stride;
        }
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_LSC) {
        pDst->top.lsc_en = pSrc->top.lsc_en;
        ::memcpy((char*)&pDst->tuningFunc.lsc, (char*)&pSrc->tuningFunc.lsc, sizeof(TdriLscCfg));

        if(pDst->top.scenario!=ISP_SCENARIO_TPIPE_IP || pDst->top.mode!=ISP_TPIPE_SUB_MODE_MFB){
            pDst->top.lsci_en = pSrc->top.lsci_en;
            pDst->lsci_stride = pSrc->lsci_stride;
            LOG_DBG("[lsci]lsciEn(%d) stride(%d)\n",pDst->top.lsci_en,pDst->lsci_stride);
        }
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_MFB) { // mfb top control by ImageIO
        ::memcpy((char*)&pDst->tuningFunc.mfb, (char*)&pSrc->tuningFunc.mfb, sizeof(TdriMfbCfg));
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_CFA){  // cfa top control by ImageIO
        ::memcpy((char*)&pDst->tuningFunc.cfa, (char*)&pSrc->tuningFunc.cfa, sizeof(TdriCfaCfg));
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_NBC) {
        pDst->top.nbc_en = pSrc->top.nbc_en;
        ::memcpy((char*)&pDst->tuningFunc.nbc, (char*)&pSrc->tuningFunc.nbc, sizeof(TdriNbcCfg));
        LOG_DBG("[nbc]eny(%d) enc(%d) iirMode(%d) scale(%d)\n",
                pDst->tuningFunc.nbc.anr_eny,pDst->tuningFunc.nbc.anr_enc,
                pDst->tuningFunc.nbc.anr_iir_mode,pDst->tuningFunc.nbc.anr_scale_mode);
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_SEEE) {
        pDst->top.seee_en = pSrc->top.seee_en;
        ::memcpy((char*)&pDst->tuningFunc.seee, (char*)&pSrc->tuningFunc.seee, sizeof(TdriSeeeCfg));
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_LCE) {
        pDst->top.lce_en = pSrc->top.lce_en;
        ::memcpy((char*)&pDst->tuningFunc.lce, (char*)&pSrc->tuningFunc.lce, sizeof(TdriLceCfg));

        if(pDst->top.scenario!=ISP_SCENARIO_TPIPE_IP || pDst->top.mode!=ISP_TPIPE_SUB_MODE_MFB){
            pDst->top.lcei_en = pSrc->top.lcei_en;
            pDst->lcei_stride = pSrc->lcei_stride;
        }
    }
    if(partUpdateFlag & TPIPE_DRV_UPDATE_NR3D) {
        pDst->top.nr3d_en = pSrc->top.nr3d_en;
    }

    return MTRUE;
}

//
MBOOL TpipeDrvImp::
runDbgTpipeMain(void)
{
    int i;

    for(i=0; i<TPIPE_DBG_BUFFER_NUM; i++){
        LOG_INF("X:runDbgTpipeMain(%d)",i);
        pDbgTpipeStruct[i]->sw.log_en = 1;
        tpipe_print_platform_config((const ISP_TPIPE_CONFIG_STRUCT *)pDbgTpipeStruct[i]);
        LOG_INF("E(%d)",i);
    }
    //
    return MTRUE;
}
//
MBOOL TpipeDrvImp::
runTpipeMain(
    TdriDrvCfg* pTdriInfo
)
{
    ISP_TPIPE_DESCRIPTOR_STRUCT tpipeDescriptor;
    int tpipeWorkingSize, fdWorkingBuffer;
    char *pWorkingBuffer;
    MBOOL ret = MTRUE;
    int tpipeRet;
    int en1, en2, dma;
    unsigned int i;
    unsigned int used_word_no;
    unsigned int total_word_no;
    unsigned int config_no_per_tpipe;
    unsigned int used_tpipe_no;
    unsigned int total_tpipe_no;
    unsigned int horizontal_tpipe_no;
    unsigned int curr_horizontal_tpipe_no;
    unsigned int curr_vertical_tpipe_no;

    LOCAL_PROFILING_LOG_AUTO_START(Event_TpipeDrv);

    LOG_DBG("Start to Config Tpipe Mode");

    /* config top_en */
    pConfigTpipeStruct->top.scenario = pTdriInfo->top.scenario;
    pConfigTpipeStruct->top.mode = pTdriInfo->top.mode;
    pConfigTpipeStruct->top.debug_sel = pTdriInfo->top.debug_sel;
    pConfigTpipeStruct->top.pixel_id = pTdriInfo->top.pixel_id;
    pConfigTpipeStruct->top.cam_in_fmt = pTdriInfo->top.cam_in_fmt;
    pConfigTpipeStruct->top.imgi_en = pTdriInfo->top.imgi_en;
    pConfigTpipeStruct->top.imgci_en = pTdriInfo->top.imgci_en;
    pConfigTpipeStruct->top.vipi_en = pTdriInfo->top.vipi_en;
    pConfigTpipeStruct->top.vip2i_en = pTdriInfo->top.vip2i_en;
    pConfigTpipeStruct->top.flki_en = pTdriInfo->top.flki_en;
    pConfigTpipeStruct->top.lce_en = pTdriInfo->top.lce_en;
    pConfigTpipeStruct->top.lcei_en = pTdriInfo->top.lcei_en;
    pConfigTpipeStruct->top.lsci_en = pTdriInfo->top.lsci_en;
    pConfigTpipeStruct->top.unp_en = pTdriInfo->top.unp_en;
    pConfigTpipeStruct->top.bnr_en = pTdriInfo->top.bnr_en;
    pConfigTpipeStruct->top.lsc_en = pTdriInfo->top.lsc_en;
    pConfigTpipeStruct->top.mfb_en = pTdriInfo->top.mfb_en;
    pConfigTpipeStruct->top.c02_en = pTdriInfo->top.c02_en;
    pConfigTpipeStruct->top.c24_en = pTdriInfo->top.c24_en;
    pConfigTpipeStruct->top.cfa_en = pTdriInfo->top.cfa_en;
    pConfigTpipeStruct->top.c42_en = pTdriInfo->top.c42_en;
    pConfigTpipeStruct->top.nbc_en = pTdriInfo->top.nbc_en;
    pConfigTpipeStruct->top.seee_en = pTdriInfo->top.seee_en;
    pConfigTpipeStruct->top.imgo_en = pTdriInfo->top.imgo_en;
    pConfigTpipeStruct->top.img2o_en = pTdriInfo->top.img2o_en;
    pConfigTpipeStruct->top.esfko_en = pTdriInfo->top.esfko_en;
    pConfigTpipeStruct->top.aao_en = pTdriInfo->top.aao_en;
    pConfigTpipeStruct->top.lcso_en = pTdriInfo->top.lcso_en;
    pConfigTpipeStruct->top.cdrz_en = pTdriInfo->top.cdrz_en;
    pConfigTpipeStruct->top.curz_en = pTdriInfo->top.curz_en;
    pConfigTpipeStruct->top.fe_sel = pTdriInfo->top.fe_sel;
    pConfigTpipeStruct->top.fe_en = pTdriInfo->top.fe_en;
    pConfigTpipeStruct->top.prz_en = pTdriInfo->top.prz_en;
    pConfigTpipeStruct->top.disp_vid_sel = pTdriInfo->top.disp_vid_sel;
    pConfigTpipeStruct->top.g2g2_en = pTdriInfo->top.g2g2_en;
    pConfigTpipeStruct->top.vido_en = pTdriInfo->top.vido_en;
    pConfigTpipeStruct->top.dispo_en = pTdriInfo->top.dispo_en;
    pConfigTpipeStruct->top.nr3d_en = pTdriInfo->top.nr3d_en;
    pConfigTpipeStruct->top.prz_opt_sel = pTdriInfo->top.prz_opt_sel;
    pConfigTpipeStruct->top.nr3d_dma_sel = pTdriInfo->top.nr3d_dma_sel;

    LOG_DBG("[Top]scenario(%d) mode(%d) debug_sel(%d) pixel_id(%d) cam_in_fmt(%d) cfa(%d) dispo_en(%d)\n", \
         pTdriInfo->top.scenario,pTdriInfo->top.mode,pTdriInfo->top.debug_sel, \
         pTdriInfo->top.pixel_id,pTdriInfo->top.cam_in_fmt,pConfigTpipeStruct->top.cfa_en, \
         pTdriInfo->top.dispo_en);

    /* config dma */
    pConfigTpipeStruct->imgi.imgi_stride = pTdriInfo->imgi.stride;
    pConfigTpipeStruct->imgi.imgi_ring_en = pTdriInfo->imgi.ring_en;
    pConfigTpipeStruct->imgi.imgi_ring_size = pTdriInfo->imgi.ring_size;
    pConfigTpipeStruct->imgci.imgci_stride = pTdriInfo->imgci_stride;
    pConfigTpipeStruct->vipi.vipi_stride = pTdriInfo->vipi.stride;
    pConfigTpipeStruct->vipi.vipi_ring_en = pTdriInfo->vipi.ring_en;
    pConfigTpipeStruct->vipi.vipi_ring_size = pTdriInfo->vipi.ring_size;
    pConfigTpipeStruct->vipi.vipi_xsize = 0;  //kk test
    pConfigTpipeStruct->vipi.vipi_ysize = 0;
    pConfigTpipeStruct->vip2i.vip2i_stride = pTdriInfo->vip2i.stride;
    pConfigTpipeStruct->vip2i.vip2i_ring_en = pTdriInfo->vip2i.ring_en;
    pConfigTpipeStruct->vip2i.vip2i_ring_size = pTdriInfo->vip2i.ring_size;

    LOG_DBG("[Imgi]stride(%d) ring(%d) ring_size(%d)",
         pConfigTpipeStruct->imgi.imgi_stride,pConfigTpipeStruct->imgi.imgi_ring_en,pConfigTpipeStruct->imgi.imgi_ring_size);

    ::memcpy( (char*)&pConfigTpipeStruct->imgo, (char*)&pTdriInfo->imgo, sizeof(ISP_TPIPE_CONFIG_IMGO_STRUCT)); /* IMGO */
    ::memcpy( (char*)&pConfigTpipeStruct->cdrz, (char*)&pTdriInfo->cdrz, sizeof(ISP_TPIPE_CONFIG_CDRZ_STRUCT)); /* CDRZ */
    ::memcpy( (char*)&pConfigTpipeStruct->curz, (char*)&pTdriInfo->curz, sizeof(ISP_TPIPE_CONFIG_CURZ_STRUCT)); /* CURZ */
    ::memcpy( (char*)&pConfigTpipeStruct->img2o, (char*)&pTdriInfo->img2o, sizeof(ISP_TPIPE_CONFIG_IMG2O_STRUCT)); /* IMG2O */
    ::memcpy( (char*)&pConfigTpipeStruct->prz, (char*)&pTdriInfo->prz, sizeof(ISP_TPIPE_CONFIG_PRZ_STRUCT)); /* PRZ */
	pConfigTpipeStruct->prz.prz_input_crop_width = 0;  //kk test
	pConfigTpipeStruct->prz.prz_input_crop_height = 0;

    /* vido */
    pConfigTpipeStruct->vido.vido_rotation = pTdriInfo->vido.rotation;
    pConfigTpipeStruct->vido.vido_flip = pTdriInfo->vido.flip;
    pConfigTpipeStruct->vido.vido_format_1 = pTdriInfo->vido.format_1;
    pConfigTpipeStruct->vido.vido_format_3 = pTdriInfo->vido.format_3;
    pConfigTpipeStruct->vido.vido_stride = pTdriInfo->vido.stride;
    pConfigTpipeStruct->vido.vido_stride_c = pTdriInfo->vido.stride_c;
    pConfigTpipeStruct->vido.vido_stride_v = pTdriInfo->vido.stride_v;
    pConfigTpipeStruct->vido.vido_crop_en = pTdriInfo->vido.crop_en;

    LOG_DBG("[Vido]rotation(%d) flip(%d) format_1(%d) format_3(%d) stride(%d) stride_c(%d) stride_v(%d) crop_en(%d)",
         pTdriInfo->vido.rotation, pTdriInfo->vido.flip, pTdriInfo->vido.format_1,
         pTdriInfo->vido.format_3,pTdriInfo->vido.stride,pTdriInfo->vido.stride_c,
         pTdriInfo->vido.stride_v,pTdriInfo->vido.crop_en);


    /* dispo */
    pConfigTpipeStruct->dispo.dispo_rotation = pTdriInfo->dispo.rotation;
    pConfigTpipeStruct->dispo.dispo_flip = pTdriInfo->dispo.flip;
    pConfigTpipeStruct->dispo.dispo_format_1 = pTdriInfo->dispo.format_1;
    pConfigTpipeStruct->dispo.dispo_format_3 = pTdriInfo->dispo.format_3;
    pConfigTpipeStruct->dispo.dispo_stride = pTdriInfo->dispo.stride;
    pConfigTpipeStruct->dispo.dispo_stride_c = pTdriInfo->dispo.stride_c;
    pConfigTpipeStruct->dispo.dispo_stride_v = pTdriInfo->dispo.stride_v;
    pConfigTpipeStruct->dispo.dispo_crop_en = pTdriInfo->dispo.crop_en;

    pConfigTpipeStruct->dispo.dispo_xsize = 0;  //kk test
    pConfigTpipeStruct->dispo.dispo_ysize = 0;
    LOG_DBG("[Dispo]rotation(%d) flip(%d) format_1(%d) format_3(%d) stride(%d) stride_c(%d) stride_v(%d) crop_en(%d)",
         pTdriInfo->dispo.rotation, pTdriInfo->dispo.flip, pTdriInfo->dispo.format_1,
         pTdriInfo->dispo.format_3,pTdriInfo->dispo.stride,pTdriInfo->dispo.stride_c,
         pTdriInfo->dispo.stride_v, pTdriInfo->dispo.crop_en);


    ::memcpy( (char*)&pConfigTpipeStruct->aao, (char*)&pTdriInfo->aao, sizeof(TdriAaoCfg));    /* AAO */
    ::memcpy( (char*)&pConfigTpipeStruct->lcso, (char*)&pTdriInfo->lcso, sizeof(TdriLcsoCfg)); /* LCSO */
    ::memcpy( (char*)&pConfigTpipeStruct->esfko, (char*)&pTdriInfo->esfko, sizeof(TdriEsfkoCfg)); /* ESFKO */
    ::memcpy( (char*)&pConfigTpipeStruct->flki, (char*)&pTdriInfo->flki, sizeof(TdriFlkiCfg)); /* FLKI */
    ::memcpy( (char*)&pConfigTpipeStruct->fe, (char*)&pTdriInfo->fe, sizeof(TdriFeCfg)); /* FE */


    /* tuning */
    pConfigTpipeStruct->lcei.lcei_stride = pTdriInfo->lcei_stride;
    pConfigTpipeStruct->lsci.lsci_stride = pTdriInfo->lsci_stride;
    ::memcpy( (char*)&pConfigTpipeStruct->bnr, (char*)&pTdriInfo->tuningFunc.bnr, sizeof(TdriBnrCfg)); /* BNR */
    ::memcpy( (char*)&pConfigTpipeStruct->lsc, (char*)&pTdriInfo->tuningFunc.lsc, sizeof(TdriLscCfg)); /* LSC */
    ::memcpy( (char*)&pConfigTpipeStruct->lce, (char*)&pTdriInfo->tuningFunc.lce, sizeof(TdriLceCfg)); /* LCE */
    ::memcpy( (char*)&pConfigTpipeStruct->nbc, (char*)&pTdriInfo->tuningFunc.nbc, sizeof(TdriNbcCfg)); /* NBC */
    ::memcpy( (char*)&pConfigTpipeStruct->seee, (char*)&pTdriInfo->tuningFunc.seee, sizeof(TdriSeeeCfg)); /* SEEE */
    ::memcpy( (char*)&pConfigTpipeStruct->mfb, (char*)&pTdriInfo->tuningFunc.mfb, sizeof(TdriMfbCfg));    /* MFB */
    ::memcpy( (char*)&pConfigTpipeStruct->cfa, (char*)&pTdriInfo->tuningFunc.cfa, sizeof(TdriCfaCfg)); /* CFA */

    LOG_DBG("nbc_sca(%d)-(%d)\n",pConfigTpipeStruct->nbc.anr_scale_mode, pTdriInfo->tuningFunc.nbc.anr_scale_mode);


    /* software tpipe setting */
    if(tpipe_drv_DbgLogEnable_VERBOSE==true)
        pConfigTpipeStruct->sw.log_en = 1;
    else
        pConfigTpipeStruct->sw.log_en = 0;

    pConfigTpipeStruct->sw.src_width = pTdriInfo->tdriCfg.srcWidth;
    pConfigTpipeStruct->sw.src_height = pTdriInfo->tdriCfg.srcHeight;
    pConfigTpipeStruct->sw.ring_buffer_mcu_no = pTdriInfo->tdriCfg.ringBufferMcuRowNo;
    pConfigTpipeStruct->sw.ring_buffer_mcu_y_size = pTdriInfo->tdriCfg.ringBufferMcuHeight;
    pConfigTpipeStruct->sw.tpipe_width = pTdriInfo->tdriPerformCfg.tpipeWidth;
    pConfigTpipeStruct->sw.tpipe_height = pTdriInfo->tdriPerformCfg.tpipeHeight;
    pConfigTpipeStruct->sw.tpipe_irq_mode = pTdriInfo->tdriPerformCfg.irqMode;
    pConfigTpipeStruct->sw.vr_3dnr_x_offset = 0;  //kk test
    pConfigTpipeStruct->sw.vr_3dnr_y_offset = 0;
    pConfigTpipeStruct->sw.zsd_3dnr_rdma_x_offset = 0;
    pConfigTpipeStruct->sw.zsd_3dnr_rdma_y_offset = 0;
    pConfigTpipeStruct->sw.zsd_3dnr_wdma_x_offset = 0;
    pConfigTpipeStruct->sw.zsd_3dnr_wdma_y_offset = 0;

    LOG_DBG("[Tdri]log_en(%d) srcWidth(%d) srcHeight(%d) isRunSegment(%d) tpipeW(%d) tpipeH(%d) irqMode(%d)\n", \
            pConfigTpipeStruct->sw.log_en, pTdriInfo->tdriCfg.srcWidth,pTdriInfo->tdriCfg.srcHeight, \
            pTdriInfo->tdriCfg.isRunSegment, pConfigTpipeStruct->sw.tpipe_width, \
            pConfigTpipeStruct->sw.tpipe_height,pConfigTpipeStruct->sw.tpipe_irq_mode);



    tpipeWorkingSize = (int)m_WBInfo.size;
    fdWorkingBuffer = (MINT32)m_WBInfo.memID;
    pWorkingBuffer = (char *)m_WBInfo.virtAddr;


    if(pWorkingBuffer == 0) {
        fdWorkingBuffer = -1;
        ret = MFALSE;
        LOG_ERR("can't alloc tpipe working buffer\n");
        goto EXIT;
    } else if ((int)pWorkingBuffer & 0x03) {
        fdWorkingBuffer = -1;
        ret = MFALSE;
        LOG_ERR("pWorkingBuffer not 4 alignment\n");
        goto EXIT;
    }
    LOG_DBG("tpipeWorkingSize=%d pWorkingBuffer=0x%x",tpipeWorkingSize, pWorkingBuffer);

    en1 = TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.lce_en,LCE)|TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.unp_en,UNP)|
            TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.bnr_en,BNR)|TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.lsc_en,LSC)|
            TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.mfb_en,MFB)|TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.c02_en,C02)|
            TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.c24_en,C24)|TPIPE_CTL_BIT_EN1(pConfigTpipeStruct->top.cfa_en,CFA);
            ;
    en2 = TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.c42_en,C42)|TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.nbc_en,NBC)|
            TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.seee_en,SEEE)|TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.cdrz_en,CDRZ)|
            TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.curz_en,CURZ)|TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.prz_en,PRZ)|
            TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.g2g2_en,G2G2)|TPIPE_CTL_BIT_EN2(pConfigTpipeStruct->top.nr3d_en,NR3D);

    dma = TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.imgi_en,IMGI)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.imgci_en,IMGCI)|
            TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.vipi_en,VIPI)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.vip2i_en,VIP2I)|
            TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.flki_en,FLKI)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.lcei_en,LCEI)|
            TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.lsci_en,LSCI)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.imgo_en,IMGO)|
            TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.img2o_en,IMG2O)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.esfko_en,ESFKO)|
            TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.aao_en,AAO)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.lcso_en,LCSO)|
            TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.vido_en,VIDO)|TPIPE_CTL_BIT_DMA(pConfigTpipeStruct->top.dispo_en,DISPO);


    LOG_INF("[Top]bnr(%d),lsc(%d),lsci(%d),lce(%d),lcei(%d),nbc(%d),seee(%d),cfa(%d)--en1(0x%08x),en2(0x%08x),dma(0x%08x)",
            pConfigTpipeStruct->top.bnr_en,pConfigTpipeStruct->top.lsc_en,
            pConfigTpipeStruct->top.lsci_en,pConfigTpipeStruct->top.lce_en,
            pConfigTpipeStruct->top.lcei_en,pConfigTpipeStruct->top.nbc_en,
            pConfigTpipeStruct->top.seee_en,pConfigTpipeStruct->top.cfa_en,
            en1,en2,dma);


    LOCAL_PROFILING_LOG_PRINT("run tpipe main");
    //
    /* for tpipe debug */
    memcpy(pDbgTpipeStruct[dbgBufIdx%TPIPE_DBG_BUFFER_NUM],pConfigTpipeStruct,sizeof(ISP_TPIPE_CONFIG_STRUCT));
    dbgBufIdx++;
    //
    tpipeRet = tpipe_main_platform((const ISP_TPIPE_CONFIG_STRUCT *)pConfigTpipeStruct, &tpipeDescriptor, pWorkingBuffer, tpipeWorkingSize);
    //
    LOCAL_PROFILING_LOG_PRINT("end tpipe main");
    //
    LOG_DBG("per_tpipe(%d)",tpipeDescriptor.config_no_per_tpipe);


    LOG_DBG("used_word_no(%d) total_word_no(%d) config_no_per_tpipe(%d) used_tpipe_no(%d) tpipeRet(%d)",tpipeDescriptor.used_word_no,tpipeDescriptor.total_word_no,tpipeDescriptor.config_no_per_tpipe,tpipeDescriptor.used_tpipe_no,tpipeRet);
    LOG_DBG("total_tpipe_no(%d) horizontal_tpipe_no(%d) curr_horizontal_tpipe_no(%d) curr_vertical_tpipe_no(%d)",tpipeDescriptor.total_tpipe_no,tpipeDescriptor.horizontal_tpipe_no,tpipeDescriptor.curr_horizontal_tpipe_no,tpipeDescriptor.curr_vertical_tpipe_no);
    LOG_DBG("updateType(%d) isRunSegment(%d)\n",pTdriInfo->updateTdri.updateType,pTdriInfo->tdriCfg.isRunSegment);

    if(pTdriInfo->imgi.ring_en) {  // special check and set for ring jpeg
        int *pRingConfNum;
        int *pRingConfVerNum;
        char *pRingConfBuf;
        int *pRingErrorControl;
        pRingConfNum = (int *)pTdriInfo->tdriCfg.ringConfNumVa;
        pRingConfVerNum = (int *)pTdriInfo->tdriCfg.ringConfVerNumVa;
        pRingConfBuf = (char *)pTdriInfo->tdriCfg.ringConfBufVa;
        pRingErrorControl = (int *)pTdriInfo->tdriCfg.ringErrorControlVa;

        if(pRingConfNum==NULL){
            LOG_ERR("memory point(used_tpipe_no) = NULL\n");
            ret = MFALSE;
            goto EXIT;
        }

        if(pRingConfVerNum==NULL){
            LOG_ERR("memory point(curr_vertical_tpipe_no) = NULL\n");
            ret = MFALSE;
            goto EXIT;
        }

        if(pRingConfBuf==NULL){
            LOG_ERR("memory point(pRingConfBuf) = NULL\n");
            ret = MFALSE;
            goto EXIT;
        }

        if(pRingErrorControl==NULL){
            LOG_ERR("memory point(pRingErrorControl) = NULL\n");
            ret = MFALSE;
            goto EXIT;
        }
        *pRingErrorControl = tpipeRet;
        *pRingConfNum = tpipeDescriptor.used_tpipe_no;
        *pRingConfVerNum = tpipeDescriptor.curr_vertical_tpipe_no;
        memcpy(pRingConfBuf, &tpipeDescriptor.tpipe_info[0], sizeof(ISP_TPIPE_INFORMATION_STRUCT)*(*pRingConfNum));

        for(i=0;i<tpipeDescriptor.used_tpipe_no;i++){
            LOG_VRB("(%02d) start_no(%04d) end_no(%04d) stop_f(%04d) offset(%04d)",
                i,
                tpipeDescriptor.tpipe_info[i].mcu_buffer_start_no,
                tpipeDescriptor.tpipe_info[i].mcu_buffer_end_no,
                tpipeDescriptor.tpipe_info[i].tpipe_stop_flag,
                tpipeDescriptor.tpipe_info[i].dump_offset_no);
        }
    } else if (pTdriInfo->tdriCfg.isRunSegment) {
        int *pSetSimpleConfIdxNumVa;
        int isGetOffset = 0;
        int currSimpleConfIdx = 0;
        int currOffset = 0;
        unsigned int *pSimpleSegConfBuf;

        pSetSimpleConfIdxNumVa = (int *)pTdriInfo->tdriCfg.setSimpleConfIdxNumVa;
        pSimpleSegConfBuf = (unsigned int *)pTdriInfo->tdriCfg.segSimpleConfBufVa;

        if(pSetSimpleConfIdxNumVa==NULL){
            LOG_ERR("memory point(used_tpipe_no) = NULL\n");
            ret = MFALSE;
            goto EXIT;
        }
        if(pSimpleSegConfBuf==NULL){
            LOG_ERR("memory point(pRingConfBuf) = NULL\n");
            ret = MFALSE;
            goto EXIT;
        }
        *pSetSimpleConfIdxNumVa = tpipeDescriptor.used_tpipe_no;

        LOG_DBG("Advanced configuration table\n");
        for(i=0;i<tpipeDescriptor.used_tpipe_no;i++){
            LOG_VRB("(%02d) start_no(%04d) end_no(%04d) stop_f(%04d) offset(%04d)",
               i,
               tpipeDescriptor.tpipe_info[i].mcu_buffer_start_no,
               tpipeDescriptor.tpipe_info[i].mcu_buffer_end_no,
               tpipeDescriptor.tpipe_info[i].tpipe_stop_flag,
               tpipeDescriptor.tpipe_info[i].dump_offset_no);
        }


        // transfer advanced configuration table to simple table
        currSimpleConfIdx = 0;
        for(i=0;i<tpipeDescriptor.used_tpipe_no;i++){
            if (tpipeDescriptor.tpipe_info[i].tpipe_stop_flag == 1) {
                if(isGetOffset) {
                    pSimpleSegConfBuf[currSimpleConfIdx] = currOffset;
                } else {
                    pSimpleSegConfBuf[currSimpleConfIdx] = tpipeDescriptor.tpipe_info[i].dump_offset_no * sizeof(int);
                }
                currSimpleConfIdx++;
                isGetOffset = 0;
            } else if(tpipeDescriptor.tpipe_info[i].tpipe_stop_flag==0 && isGetOffset==0) { // get previous non-stop idx for tpipe base address
                isGetOffset = 1;
                currOffset = tpipeDescriptor.tpipe_info[i].dump_offset_no * sizeof(int);
            }
        }
        *pSetSimpleConfIdxNumVa = currSimpleConfIdx;
        //
        LOG_DBG("Simple configuration table\n");
        for(i=0;i<*pSetSimpleConfIdxNumVa;i++)
            LOG_VRB("i(%02d) offset(%04d)",i,pSimpleSegConfBuf[i]);

    } else if(tpipeRet != ISP_TPIPE_MESSAGE_OK) {
        ret = MFALSE;
        LOG_ERR("tpipe parameter error(%d)\n",tpipeRet);
        goto EXIT;
    }

    //
    if(tpipeDescriptor.used_word_no==0) {
        LOG_ERR("tpipe table number error\n");
        ret = MFALSE;
        goto EXIT;
    }

    for(i=0;i<tpipeDescriptor.used_word_no;i++){
        LOG_DBG("i(%02d) 0x%08x",i,*(tpipeDescriptor.tpipe_config+i));
    }

    pTdriInfo->tdriCfg.tpipeTabSize = tpipeDescriptor.used_word_no*sizeof(int);
    memcpy((unsigned char*)(pTdriInfo->tdriCfg.baseVa), \
          (unsigned char*)tpipeDescriptor.tpipe_config, \
            pTdriInfo->tdriCfg.tpipeTabSize);

    LOG_DBG("tpipeTableVA(0x%x)",pTdriInfo->tdriCfg.baseVa);

    if(tpipeDescriptor.used_word_no >= 10) {  // for debug
        LOG_INF("size(%d)||[0-4]%08X-%08X-%08X-%08X-%08X||[%d-%d]%08X-%08X-%08X-%08X-%08X",
            pTdriInfo->tdriCfg.tpipeTabSize,
            *(tpipeDescriptor.tpipe_config+0),*(tpipeDescriptor.tpipe_config+1),
            *(tpipeDescriptor.tpipe_config+2),*(tpipeDescriptor.tpipe_config+3),*(tpipeDescriptor.tpipe_config+4),
            (tpipeDescriptor.used_word_no-5),(tpipeDescriptor.used_word_no-1),
            *(tpipeDescriptor.tpipe_config+tpipeDescriptor.used_word_no-5),
            *(tpipeDescriptor.tpipe_config+tpipeDescriptor.used_word_no-4),
            *(tpipeDescriptor.tpipe_config+tpipeDescriptor.used_word_no-3),
            *(tpipeDescriptor.tpipe_config+tpipeDescriptor.used_word_no-2),
            *(tpipeDescriptor.tpipe_config+tpipeDescriptor.used_word_no-1));
    }



EXIT:
    return ret;

}


MBOOL TpipeDrvImp::
configTdriPara(
    TdriDrvCfg* pTdriInfo
)
{
    Mutex::Autolock lock(mLock); // acquires a lock on m_mutex

    TPIPE_DRV_UPDATE_TYPE updateType = pTdriInfo->updateTdri.updateType;
    int   partUpdateFlag = pTdriInfo->updateTdri.partUpdateFlag;
    TdriDrvCfg *pKeepedTdriInfo;  // tpipe data be saved
    TdriDrvCfg tdriInfo;
    int ret = MTRUE;

    LOG_DBG("updateType(%d) partUpdateFlag(0x%x) baseVa(0x%x) pTdriInfo(0x%x)\n",
                updateType,partUpdateFlag,pTdriInfo->tdriCfg.baseVa,pTdriInfo);

    if(updateType == TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ1_PARTIAL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE) {
        //
        pKeepedTdriInfo = pKeepTdriInfo[TPIPE_DRV_CQ01];
        pKeepedTdriInfo->updateTdri.updateType = pTdriInfo->updateTdri.updateType;
        pKeepedTdriInfo->tdriCfg.baseVa = pTdriInfo->tdriCfg.baseVa;

    } else if(updateType == TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ2_PARTIAL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE) {
        //
        pKeepedTdriInfo = pKeepTdriInfo[TPIPE_DRV_CQ02];
        pKeepedTdriInfo->updateTdri.updateType = pTdriInfo->updateTdri.updateType;
        pKeepedTdriInfo->tdriCfg.baseVa = pTdriInfo->tdriCfg.baseVa;
    } else if(updateType == TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ3_PARTIAL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ3_TURNING_SAVE) {
        //
        pKeepedTdriInfo = pKeepTdriInfo[TPIPE_DRV_CQ03];
        pKeepedTdriInfo->updateTdri.updateType = pTdriInfo->updateTdri.updateType;
        pKeepedTdriInfo->tdriCfg.baseVa = pTdriInfo->tdriCfg.baseVa;
    } else if(updateType == TPIPE_DRV_UPDATE_TYPE_FULL) {
        /* do nothing */
    }
    else {
        pKeepedTdriInfo = pKeepTdriInfo[TPIPE_DRV_CQ01];
        pKeepedTdriInfo->updateTdri.updateType = pTdriInfo->updateTdri.updateType;
        pKeepedTdriInfo->tdriCfg.baseVa = pTdriInfo->tdriCfg.baseVa;
        LOG_ERR("updateType(0x%x) error\n",updateType);
    }

    if(updateType == TPIPE_DRV_UPDATE_TYPE_CQ1_PARTIAL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ2_PARTIAL_SAVE ||
            updateType == TPIPE_DRV_UPDATE_TYPE_CQ3_PARTIAL_SAVE) { /* without update CDRZ, CURZ, PRZ, FE */

        if(partUpdateFlag & TPIPE_DRV_UPDATE_IMGI) {
            pKeepedTdriInfo->top.imgi_en = pTdriInfo->top.imgi_en;
            ::memcpy((char*)&pKeepedTdriInfo->imgi, (char*)&pTdriInfo->imgi, sizeof(TdriRingInDMACfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_IMGCI) {
            pKeepedTdriInfo->top.imgci_en = pTdriInfo->top.imgci_en;
            ::memcpy((char*)&pKeepedTdriInfo->imgci_stride, (char*)&pTdriInfo->imgci_stride, sizeof(int));
        }

        if(partUpdateFlag & TPIPE_DRV_UPDATE_VIPI) {
            pKeepedTdriInfo->top.vipi_en = pTdriInfo->top.vipi_en;
            ::memcpy((char*)&pKeepedTdriInfo->vipi, (char*)&pTdriInfo->vipi, sizeof(TdriRingInDMACfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_VIP2I) {
            pKeepedTdriInfo->top.vip2i_en = pTdriInfo->top.vip2i_en;
            ::memcpy((char*)&pKeepedTdriInfo->vip2i, (char*)&pTdriInfo->vip2i, sizeof(TdriRingInDMACfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_FLKI) {
            pKeepedTdriInfo->top.flki_en = pTdriInfo->top.flki_en;
            ::memcpy((char*)&pKeepedTdriInfo->flki, (char*)&pTdriInfo->flki, sizeof(TdriFlkiCfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_LCEI) {
            pKeepedTdriInfo->top.lcei_en = pTdriInfo->top.lcei_en;
            ::memcpy((char*)&pKeepedTdriInfo->lcei_stride, (char*)&pTdriInfo->lcei_stride, sizeof(int));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_LSCI) {
            pKeepedTdriInfo->top.lsci_en = pTdriInfo->top.lsci_en;
            ::memcpy((char*)&pKeepedTdriInfo->lsci_stride, (char*)&pTdriInfo->lsci_stride, sizeof(int));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_IMGO) {
            pKeepedTdriInfo->top.imgo_en = pTdriInfo->top.imgo_en;
            ::memcpy((char*)&pKeepedTdriInfo->imgo, (char*)&pTdriInfo->imgo, sizeof(TdriImgoCfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_IMG2O) {
            pKeepedTdriInfo->top.img2o_en = pTdriInfo->top.img2o_en;
            ::memcpy((char*)&pKeepedTdriInfo->img2o, (char*)&pTdriInfo->img2o, sizeof(TdriImg2oCfg));
			pKeepedTdriInfo->img2o.img2o_xsize = 0;  //kk test
			pKeepedTdriInfo->img2o.img2o_ysize = 0;
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_ESFKO) {
            pKeepedTdriInfo->top.esfko_en = pTdriInfo->top.esfko_en;
            ::memcpy((char*)&pKeepedTdriInfo->esfko, (char*)&pTdriInfo->esfko, sizeof(TdriEsfkoCfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_AAO) {
            pKeepedTdriInfo->top.aao_en = pTdriInfo->top.aao_en;
            ::memcpy((char*)&pKeepedTdriInfo->aao, (char*)&pTdriInfo->aao, sizeof(TdriAaoCfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_LCSO) {
            pKeepedTdriInfo->top.lcso_en = pTdriInfo->top.lcso_en;
            ::memcpy((char*)&pKeepedTdriInfo->lcso, (char*)&pTdriInfo->lcso, sizeof(TdriLcsoCfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_VIDO) {
            pKeepedTdriInfo->top.vido_en = pTdriInfo->top.vido_en;
            ::memcpy((char*)&pKeepedTdriInfo->vido, (char*)&pTdriInfo->vido, sizeof(TdriRingOutDMACfg));
        }
        if(partUpdateFlag & TPIPE_DRV_UPDATE_DISPO) {
            pKeepedTdriInfo->top.dispo_en = pTdriInfo->top.dispo_en;
            ::memcpy((char*)&pKeepedTdriInfo->dispo, (char*)&pTdriInfo->dispo, sizeof(TdriRingOutDMACfg));
        }

        ::memcpy((char*)&tdriInfo, (char*)pKeepedTdriInfo, sizeof(TdriDrvCfg));
    } else if(updateType == TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE ||
              updateType == TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE ||
              updateType == TPIPE_DRV_UPDATE_TYPE_CQ3_TURNING_SAVE) {

        updateFeatureIO(pKeepedTdriInfo, pTdriInfo, partUpdateFlag);
        //
        ::memcpy((char*)&tdriInfo, (char*)pKeepedTdriInfo, sizeof(TdriDrvCfg));
        //
    } else if (updateType == TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE ||
               updateType == TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE ||
               updateType == TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE) {

        updateImageIO(pKeepedTdriInfo, pTdriInfo);

        ::memcpy((char*)&tdriInfo, (char*)pKeepedTdriInfo, sizeof(TdriDrvCfg));
    } else if(updateType == TPIPE_DRV_UPDATE_TYPE_FULL) {

        ::memcpy((char*)&tdriInfo, (char*)pTdriInfo, sizeof(TdriDrvCfg));

    } else {
        LOG_ERR("Not support this updateType(%d)\n",updateType);
        ret = MFALSE;
        goto EXIT;
    }


    if(updateType != TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE &&
              updateType != TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE &&
              updateType != TPIPE_DRV_UPDATE_TYPE_CQ3_TURNING_SAVE) {


        runTpipeMain((TdriDrvCfg*)&tdriInfo);
    }

EXIT:
    return ret;

}







//-----------------------------------------------------------------------------

