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

#ifndef __JPEG_HAL_H__
#define __JPEG_HAL_H__

#ifndef MTK_M4U_SUPPORT
  #define JPEG_ENC_USE_PMEM
  #define USE_PMEM
#else
  #include "m4u_lib.h"
#endif

#include "m4u_lib.h"



/*******************************************************************************
*
********************************************************************************/
#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef INT32
typedef int INT32;
#endif
 
#define JPEG_MAX_ENC_SIZE (12*1024*1024)




/*******************************************************************************
* class JpgEncHal
********************************************************************************/
class JpgEncHal {
public:
    JpgEncHal();
    virtual ~JpgEncHal();

    //enum SrcFormat {
    //    kRGB_565_Format,
    //    kRGB_888_Format,
    //    kARGB_8888_Format,
    //    kABGR_8888_Format,
    //    kYUY2_Pack_Format,      // YUYV
    //    kUYVY_Pack_Format,      // UYVY
    //    kYVU9_Planar_Format,    // YUV411, 4x4 sub sample U/V plane
    //    kYV16_Planar_Format,    // YUV422, 2x1 subsampled U/V planes
    //    kYV12_Planar_Format,    // YUV420, 2x2 subsampled U/V planes
    //    kNV12_Format,           // YUV420, 2x2 subsampled , interleaved U/V plane
    //    kNV21_Format,           // YUV420, 2x2 subsampled , interleaved V/U plane
    //    
    //    kSrcFormatCount
    //};

    //enum JPEG_ENC_RESULT {
    //  JPEG_ENC_RST_CFG_ERR,
    //  JPEG_ENC_RST_DONE,      
    //  JPEG_ENC_RST_ROW_DONE,
    //  JPEG_ENC_RST_HUFF_ERROR,
    //  JPEG_ENC_RST_DECODE_FAIL,
    //  JPEG_ENC_RST_BS_UNDERFLOW
    //  
    //};
    

    enum EncFormat {
        //kYUV_444_Format,
        //kYUV_422_Format,
        //kYUV_411_Format,
        //kYUV_420_Format,
        //kYUV_400_Format,
        
        kENC_YUY2_Format,           // YUYV
        kENC_UYVY_Format,           // UYVY
        kENC_NV12_Format,           // YUV420, 2x2 subsampled , interleaved U/V plane
        kENC_NV21_Format,           // YUV420, 2x2 subsampled , interleaved V/U plane

        
        kEncFormatCount
    };
    
    enum {
      JPEG_ENC_MEM_PHY,
      JPEG_ENC_MEM_PMEM,
      JPEG_ENC_MEM_M4U,
      JPEG_ENC_MEM_ION
      
    };
    
    
    
    bool lock();
    bool unlock();
    bool start(UINT32 *encSize);
    
    /* set image actual width, height and encode format */ 
    bool setEncSize(UINT32 width, UINT32 height, EncFormat encformat) ;
    
    /* get requirement of minimum source buffer size and stride after setEncSize */
    UINT32 getSrcBufMinSize()      { return fSrcMinBufferSize  ; };    
    UINT32 getSrcCbCrBufMinSize()  { return fSrcMinCbCrSize ; };
    UINT32 getSrcBufMinStride()    { return fSrcMinBufferStride  ; };    
    
    /* Set source buffer virtual address.
       The srcChromaAddr should be NULL in YUV422.
    */
    bool setSrcAddr(void *srcAddr, void *srcChromaAddr); 
    
    /* Set source size of buffer1(srcSize) and buffer2(srcSize2) and stride. 
       The buffer size and stride should be at least minimum buffer size and stride.
       The buffer1 and buffer2 share the buffer stride.
       Stride should be align to 32(YUV422) or 16 (YUV420).       
       */
    bool setSrcBufSize(UINT32 srcStride,UINT32 srcSize, UINT32 srcSize2); 
    
    /* set encoding quality , range should be [100:1] */
    bool setQuality(UINT32 quality) { if( quality > 100) return false ; else fQuality = quality; return true ;}
    
    /* set distination buffer virtual address and size */
    bool setDstAddr(void *dstAddr) { if(dstAddr == NULL) return false; 
                                     else fDstAddr = dstAddr; return true;}
                                     
    /* set bitstream buffer size , should at least 624 bytes */
    bool setDstSize(UINT32 size) { if(size<624)return false;
                                   else fDstSize = size; return true ;}

    /* set Normal/Exif mode, 1:Normal,0:Exif, default is Normal mode */
    void enableSOI(bool b) { fIsAddSOI = b; }
    
    
    void setIonMode(bool ionEn) { if( ionEn ) fMemType = JPEG_ENC_MEM_ION; 
                                         else fMemType = fMemTypeDefault ;      }
    
    void setSrcFD( INT32 srcFD, INT32 srcFD2 ) { fSrcFD = srcFD; fSrcFD2 = srcFD2; }
    
    void setDstFD( INT32 dstFD ) { fDstFD = dstFD ; }
    
    //bool setSrcAddrPA( UINT32 srcAddrPA, UINT32 srcChromaAddrPA); 
    
    //bool setDstAddrPA( UINT32 dstAddrPA){ if(dstAddrPA == NULL) return false; 
    //                                      else fDstAddrPA = dstAddrPA; return true; 
    //                                   } 
     void setDRI( INT32 dri ) { fDRI = dri ; } 

     
private:
  
    bool allocPMEM();  
    bool alloc_m4u();

    bool free_m4u();
    
    bool alloc_ion();
    bool free_ion();
    bool islock;
    
    
    MTKM4UDrv *pM4uDrv ;
    M4U_MODULE_ID_ENUM fm4uJpegID ; 
    
    UINT32 fMemType ;
    UINT32 fMemTypeDefault ;
    
    UINT32 fSrcWidth;
    UINT32 fSrcHeight;
    UINT32 fDstWidth;
    UINT32 fDstHeight;
    UINT32 fQuality;
    UINT32 fROIX;
    UINT32 fROIY;
    UINT32 fROIWidth;
    UINT32 fROIHeight;
    
    UINT32 fSrcMinBufferSize ;
    UINT32 fSrcMinCbCrSize ;        
    UINT32 fSrcMinBufferStride;
    UINT32 fSrcMinCbCrStride;
    
    UINT32 fEncSrcBufSize  ;
    UINT32 fSrcBufStride;
    UINT32 fSrcBufHeight;
    
    UINT32 fEncCbCrBufSize ;
    UINT32 fSrcCbCrBufStride;
    UINT32 fSrcCbCrBufHeight;
    
    //SrcFormat fSrcFormat;
    EncFormat fEncFormat;
    
    void *fSrcAddr;
    void *fSrcChromaAddr;

    //UINT32 fEncDstBufSize  ;

    
    void *fDstAddr;
    int fDstSize;
    bool fIsAddSOI;
    
    

    UINT32 fSrcAddrPA ;
    UINT32 fSrcChromaAddrPA;
    UINT32 fDstAddrPA ;
    
    UINT32 fDstM4uPA;    
    UINT32 fSrcM4uPA;    
    UINT32 fSrcChromaM4uPA;    
    UINT32 fIsSrc2p;
    
    //ION
    
    bool fIonEn ;
    //bool fSrcIonEn ;
    //bool fDstIonEn ;
    INT32 fSrcFD;
    INT32 fSrcFD2;
    INT32 fDstFD ;
    
    UINT32 fSrcIonPA       ;
    UINT32 fSrcChromaIonPA ;
    UINT32 fDstIonPA       ;

    void* fSrcIonVA       ;
    void* fSrcChromaIonVA ;
    void* fDstIonVA       ;

    void* fSrcIonHdle       ;
    void* fSrcChromaIonHdle ;
    void* fDstIonHdle       ;


    
    INT32 fIonDevFD ;
    UINT32 fDRI ;
    
    

    

#if 1 //def JPEG_ENC_USE_PMEM
    
    unsigned char *fEncSrcPmemVA      ;
    unsigned char *fEncSrcCbCrPmemVA  ;
    unsigned char *fEncDstPmemVA      ;
    
    UINT32 fEncSrcPmemPA      ;
    UINT32 fEncSrcCbCrPmemPA  ;
    UINT32 fEncDstPmemPA      ;
    
    int fEncSrcPmemFD      ;
    int fEncSrcCbCrPmemFD  ;
    int fEncDstPmemFD      ;
#endif
    int encID;
    unsigned long fResTable;
};

#endif 