/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_M4U_LIB_H
#define _MTK_M4U_LIB_H

#include <linux/ioctl.h>

#define __PMEM_WRAP_LAYER_EN__




#define DEFAULT_PAGE_SIZE   0x1000                                  //4KB
#define MODULE_WITH_INDEPENDENT_PORT_ID  36

#define M4U_CLIENT_MODULE_NUM M4U_CLNTMOD_MAX
#define TOTAL_MVA_RANGE       0x40000000                              //total virtual address range: 1GB

#define ACCESS_TYPE_TRANSLATION_FAULT  0
#define ACCESS_TYPE_64K_PAGE           1
#define ACCESS_TYPE_4K_PAGE            2
#define ACCESS_TYPE_4K_EXTEND_PAGE     3

#define PT_TOTAL_ENTRY_NUM    (TOTAL_MVA_RANGE/DEFAULT_PAGE_SIZE)              //total page table entries
#define MODULE_MVA_RANGE      (TOTAL_MVA_RANGE/M4U_CLIENT_MODULE_NUM)     //the virtual address range per port
#define PT_MODULE_ENTRY_NUM   (MODULE_MVA_RANGE/DEFAULT_PAGE_SIZE)            //number of page table entries for each port
#define PT_MODULE_PA_SZ       (PT_MODULE_ENTRY_NUM*4)                      //the physical memory size of page table per port


#define M4U_GET_PTE_OFST_TO_PT_SA(MVA)    ((MVA >> 12) << 2)

#define SEGMENT_SIZE 16


//====================================
// about portid
//====================================
#define M4U_LARB0_PORTn(n)      ((n)+0)
#define M4U_LARB1_PORTn(n)      ((n)+10)
#define M4U_LARB2_PORTn(n)      ((n)+17)
#define M4U_LARB3_PORTn(n)      ((n)+29)
#define M4U_LARB4_PORTn(n)      ((n)+43)
#define M4U_LARB5_PORTn(n)      ((n)+53)

typedef enum
{
    M4U_PORT_VENC_RCPU             =  M4U_LARB0_PORTn(0)   ,
    M4U_PORT_VENC_REF_LUMA         =  M4U_LARB0_PORTn(1)   ,
    M4U_PORT_VENC_REF_CHROMA       =  M4U_LARB0_PORTn(2)   ,
    M4U_PORT_VENC_DB_READ          =  M4U_LARB0_PORTn(3)   ,
    M4U_PORT_VENC_DB_WRITE         =  M4U_LARB0_PORTn(4)   ,
    M4U_PORT_VENC_CUR_LUMA         =  M4U_LARB0_PORTn(5)   ,
    M4U_PORT_VENC_CUR_CHROMA       =  M4U_LARB0_PORTn(6)   ,
    M4U_PORT_VENC_RD_COMV          =  M4U_LARB0_PORTn(7)   ,
    M4U_PORT_VENC_SV_COMV          =  M4U_LARB0_PORTn(8)   ,
    M4U_PORT_VENC_BSDMA            =  M4U_LARB0_PORTn(9)   ,
                                                           
    M4U_PORT_HW_VDEC_MC_EXT        =  M4U_LARB1_PORTn(0)   ,
    M4U_PORT_HW_VDEC_PP_EXT        =  M4U_LARB1_PORTn(1)   ,
    M4U_PORT_HW_VDEC_AVC_MV_EXT    =  M4U_LARB1_PORTn(2)   ,
    M4U_PORT_HW_VDEC_PRED_RD_EXT   =  M4U_LARB1_PORTn(3)   ,
    M4U_PORT_HW_VDEC_PRED_WR_EXT   =  M4U_LARB1_PORTn(4)   ,
    M4U_PORT_HW_VDEC_VLD_EXT       =  M4U_LARB1_PORTn(5)   ,
    M4U_PORT_HW_VDEC_VLD2_EXT      =  M4U_LARB1_PORTn(6)   ,
                                                           
    M4U_PORT_ROT_EXT               =  M4U_LARB2_PORTn(0)   ,
    M4U_PORT_OVL_CH0               =  M4U_LARB2_PORTn(1)   ,
    M4U_PORT_OVL_CH1               =  M4U_LARB2_PORTn(2)   ,
    M4U_PORT_OVL_CH2               =  M4U_LARB2_PORTn(3)   ,
    M4U_PORT_OVL_CH3               =  M4U_LARB2_PORTn(4)   ,
    M4U_PORT_WDMA0                 =  M4U_LARB2_PORTn(5)   ,
    M4U_PORT_WDMA1                 =  M4U_LARB2_PORTn(6)   ,
    M4U_PORT_RDMA0                 =  M4U_LARB2_PORTn(7)   ,
    M4U_PORT_RDMA1                 =  M4U_LARB2_PORTn(8)   ,
    M4U_PORT_CMDQ                  =  M4U_LARB2_PORTn(9)   ,
    M4U_PORT_DBI                   =  M4U_LARB2_PORTn(10)  ,
    M4U_PORT_G2D                   =  M4U_LARB2_PORTn(11)  ,
                                                           
    M4U_PORT_JPGDEC_WDMA           =  M4U_LARB3_PORTn(0)   ,
    M4U_PORT_JPGENC_RDMA           =  M4U_LARB3_PORTn(1)   ,
    M4U_PORT_VIPI                  =  M4U_LARB3_PORTn(2)   ,
    M4U_PORT_IMGI                  =  M4U_LARB3_PORTn(3)   ,
    M4U_PORT_DISPO                 =  M4U_LARB3_PORTn(4)   ,
    M4U_PORT_DISPCO                =  M4U_LARB3_PORTn(5)   ,
    M4U_PORT_DISPVO                =  M4U_LARB3_PORTn(6)   ,
    M4U_PORT_VIDO                  =  M4U_LARB3_PORTn(7)   ,
    M4U_PORT_VIDCO                 =  M4U_LARB3_PORTn(8)   ,
    M4U_PORT_VIDVO                 =  M4U_LARB3_PORTn(9)   ,
    M4U_PORT_VIP2I                 =  M4U_LARB3_PORTn(10)  ,
    M4U_PORT_GDMA_SMI_WR           =  M4U_LARB3_PORTn(11)  ,
    M4U_PORT_JPGDEC_BSDMA          =  M4U_LARB3_PORTn(12)  ,
    M4U_PORT_JPGENC_BSDMA          =  M4U_LARB3_PORTn(13)  ,
                                                           
    M4U_PORT_GDMA_SMI_RD           =  M4U_LARB4_PORTn(0)   ,
    M4U_PORT_IMGCI                 =  M4U_LARB4_PORTn(1)   ,
    M4U_PORT_IMGO                  =  M4U_LARB4_PORTn(2)   ,
    M4U_PORT_IMG2O                 =  M4U_LARB4_PORTn(3)   ,
    M4U_PORT_LSCI                  =  M4U_LARB4_PORTn(4)   ,
    M4U_PORT_FLKI                  =  M4U_LARB4_PORTn(5)   ,
    M4U_PORT_LCEI                  =  M4U_LARB4_PORTn(6)   ,
    M4U_PORT_LCSO                  =  M4U_LARB4_PORTn(7)   ,
    M4U_PORT_ESFKO                 =  M4U_LARB4_PORTn(8)   ,
    M4U_PORT_AAO                   =  M4U_LARB4_PORTn(9)   ,
                                                           
    M4U_PORT_AUDIO                 =  M4U_LARB5_PORTn(0)   ,

    M4U_PORT_NUM,

    M4U_PORT_UNKNOWN         = 1000

} M4U_PORT_ID_ENUM;





typedef enum
{
    M4U_CLNTMOD_VENC    = 0,	//0
                             
    M4U_CLNTMOD_VDEC       ,
                             
    M4U_CLNTMOD_ROT        ,
    M4U_CLNTMOD_OVL        ,
    M4U_CLNTMOD_WDMA       ,
    M4U_CLNTMOD_RDMA       ,
    M4U_CLNTMOD_CMDQ       ,
    M4U_CLNTMOD_DBI        ,
    M4U_CLNTMOD_G2D        ,
                                 
    M4U_CLNTMOD_JPGDEC     ,
    M4U_CLNTMOD_JPGENC     ,
    M4U_CLNTMOD_VIP        ,
    M4U_CLNTMOD_DISP       ,
    M4U_CLNTMOD_VID        ,
    M4U_CLNTMOD_GDMA       ,
                           
    M4U_CLNTMOD_IMG        ,
    M4U_CLNTMOD_LSCI       ,
    M4U_CLNTMOD_FLKI       ,
    M4U_CLNTMOD_LCEI       ,
    M4U_CLNTMOD_LCSO       ,
    M4U_CLNTMOD_ESFKO      ,
    M4U_CLNTMOD_AAO        ,
                           
    M4U_CLNTMOD_AUDIO      ,

    M4U_CLNTMOD_LCDC_UI,
    
    M4U_CLNTMOD_UNKNOWN,
    M4U_CLNTMOD_MAX
} M4U_MODULE_ID_ENUM;


typedef struct _M4U_RANGE_DES  //sequential entry range
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    unsigned int MVAStart;
    unsigned int MVAEnd;
    unsigned int entryCount;
} M4U_RANGE_DES_T;

typedef struct _M4U_MVA_SLOT
{
    unsigned int BaseAddr;      //slot MVA start address
    unsigned int Size;          //slot size
    unsigned int Offset;        //current offset of the slot
    unsigned int BufCnt;        //how many buffer has been allocated from this slot
} M4U_MVA_SLOT_T;

typedef enum
{
	M4U_DESC_MAIN_TLB=0,
	M4U_DESC_PRE_TLB_LSB,
	M4U_DESC_PRE_TLB_MSB
} M4U_DESC_TLB_SELECT_ENUM;


typedef enum
{
	RT_RANGE_HIGH_PRIORITY=0,
	SEQ_RANGE_LOW_PRIORITY=1
} M4U_RANGE_PRIORITY_ENUM;

typedef enum
{
	M4U_DMA_READ_WRITE = 0,
	M4U_DMA_READ = 1,
	M4U_DMA_WRITE = 2,
	M4U_DMA_NONE_OP = 3,

} M4U_DMA_DIR_ENUM;


typedef struct _M4U_PORT
{  
	M4U_PORT_ID_ENUM ePortID;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
	unsigned int Virtuality;						   
	unsigned int Security;
    unsigned int domain;            //domain : 0 1 2 3
	unsigned int Distance;
	unsigned int Direction;         //0:- 1:+
}M4U_PORT_STRUCT;

typedef enum
{
	ROTATE_0=0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270,
	ROTATE_HFLIP_0,
	ROTATE_HFLIP_90,
	ROTATE_HFLIP_180,
	ROTATE_HFLIP_270
} M4U_ROTATOR_ENUM;

typedef struct _M4U_PORT_ROTATOR
{  
	M4U_PORT_ID_ENUM ePortID;		   // hardware port ID, defined in M4U_PORT_ID_ENUM
	unsigned int Virtuality;						   
	unsigned int Security;
	// unsigned int Distance;      // will be caculated actomatically inside M4U driver
	// unsigned int Direction;
  unsigned int MVAStart; 
  unsigned int BufAddr;
  unsigned int BufSize;  
  M4U_ROTATOR_ENUM angle;	
}M4U_PORT_STRUCT_ROTATOR;

// module related:  alloc/dealloc MVA buffer
typedef struct _M4U_MOUDLE
{
	// MVA alloc / dealloc
	M4U_MODULE_ID_ENUM eModuleID;	// module ID used inside M4U driver, defined in M4U_PORT_MODULE_ID_ENUM
	unsigned int BufAddr;				// buffer virtual address
	unsigned int BufSize;				// buffer size in byte

	// TLB range invalidate or set uni-upadte range
	unsigned int MVAStart;						 // MVA start address
	unsigned int MVAEnd;							 // MVA end address
	M4U_RANGE_PRIORITY_ENUM ePriority;						 // range priority, 0:high, 1:normal
	unsigned int entryCount;

    // manually insert page entry
	unsigned int EntryMVA;						 // manual insert entry MVA
	unsigned int Lock;							 // manual insert lock or not
	int security;
        int cache_coherent;
}M4U_MOUDLE_STRUCT;

typedef struct _M4U_WRAP_DES
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    M4U_PORT_ID_ENUM ePortID;    
    unsigned int MVAStart;
    unsigned int MVAEnd;
} M4U_WRAP_DES_T;

typedef enum
{
    M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM = 0,  // optimized, recommand to use
    M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM = 1, // optimized, recommand to use
    M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM = 2,
    M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM = 3,
    M4U_NONE_OP = 4,
} M4U_CACHE_SYNC_ENUM;

typedef struct _M4U_CACHE
{
    // MVA alloc / dealloc
    M4U_MODULE_ID_ENUM eModuleID;             // module ID used inside M4U driver, defined in M4U_MODULE_ID_ENUM
    M4U_CACHE_SYNC_ENUM eCacheSync;
    unsigned int BufAddr;                  // buffer virtual address
    unsigned int BufSize;                     // buffer size in byte
}M4U_CACHE_STRUCT;

typedef enum _M4U_STATUS
{
	M4U_STATUS_OK = 0,
	M4U_STATUS_INVALID_CMD,
	M4U_STATUS_INVALID_HANDLE,
	M4U_STATUS_NO_AVAILABLE_RANGE_REGS,
	M4U_STATUS_KERNEL_FAULT,
	M4U_STATUS_MVA_OVERFLOW,
	M4U_STATUS_INVALID_PARAM
} M4U_STATUS_ENUM;


//IOCTL commnad
#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_POWER_ON            _IOW(MTK_M4U_MAGICNO, 0, int)
#define MTK_M4U_T_POWER_OFF           _IOW(MTK_M4U_MAGICNO, 1, int)
#define MTK_M4U_T_DUMP_REG            _IOW(MTK_M4U_MAGICNO, 2, int)
#define MTK_M4U_T_DUMP_INFO           _IOW(MTK_M4U_MAGICNO, 3, int)
#define MTK_M4U_T_ALLOC_MVA           _IOWR(MTK_M4U_MAGICNO,4, int)
#define MTK_M4U_T_DEALLOC_MVA         _IOW(MTK_M4U_MAGICNO, 5, int)
#define MTK_M4U_T_INSERT_TLB_RANGE    _IOW(MTK_M4U_MAGICNO, 6, int)
#define MTK_M4U_T_INVALID_TLB_RANGE   _IOW(MTK_M4U_MAGICNO, 7, int)
#define MTK_M4U_T_INVALID_TLB_ALL     _IOW(MTK_M4U_MAGICNO, 8, int)
#define MTK_M4U_T_MANUAL_INSERT_ENTRY _IOW(MTK_M4U_MAGICNO, 9, int)
#define MTK_M4U_T_CACHE_SYNC          _IOW(MTK_M4U_MAGICNO, 10, int)
#define MTK_M4U_T_CONFIG_PORT         _IOW(MTK_M4U_MAGICNO, 11, int)
#define MTK_M4U_T_CONFIG_ASSERT       _IOW(MTK_M4U_MAGICNO, 12, int)
#define MTK_M4U_T_INSERT_WRAP_RANGE   _IOW(MTK_M4U_MAGICNO, 13, int)
#define MTK_M4U_T_MONITOR_START       _IOW(MTK_M4U_MAGICNO, 14, int)
#define MTK_M4U_T_MONITOR_STOP        _IOW(MTK_M4U_MAGICNO, 15, int)
#define MTK_M4U_T_RESET_MVA_RELEASE_TLB  _IOW(MTK_M4U_MAGICNO, 16, int)
#define MTK_M4U_T_CONFIG_PORT_ROTATOR _IOW(MTK_M4U_MAGICNO, 17, int)
#define MTK_M4U_T_QUERY_MVA           _IOW(MTK_M4U_MAGICNO, 18, int)
#define MTK_M4U_T_M4UDrv_CONSTRUCT    _IOW(MTK_M4U_MAGICNO, 19, int)
#define MTK_M4U_T_M4UDrv_DECONSTRUCT  _IOW(MTK_M4U_MAGICNO, 20, int)
#define MTK_M4U_T_DUMP_PAGETABLE      _IOW(MTK_M4U_MAGICNO, 21, int)
#define MTK_M4U_T_REGISTER_BUFFER     _IOW(MTK_M4U_MAGICNO, 22, int)
#define MTK_M4U_T_CACHE_FLUSH_ALL     _IOW(MTK_M4U_MAGICNO, 23, int)
#define MTK_M4U_T_REG_SET             _IOW(MTK_M4U_MAGICNO, 24, int)
#define MTK_M4U_T_REG_GET             _IOW(MTK_M4U_MAGICNO, 25, int)

class MTKM4UDrv
{
public:
    MTKM4UDrv(void);
    ~MTKM4UDrv(void);
    
    M4U_STATUS_ENUM m4u_power_on(M4U_MODULE_ID_ENUM eModuleID);
    M4U_STATUS_ENUM m4u_power_off(M4U_MODULE_ID_ENUM eModuleID);
    M4U_STATUS_ENUM m4u_alloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
		                          const unsigned int BufAddr, 
		                          const unsigned int BufSize, 
		                          int security,
		                          int cache_coherent,
		                          unsigned int *pRetMVABuf);

    M4U_STATUS_ENUM m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
		                          const unsigned int BufAddr, 
		                          const unsigned int BufSize, 
		                          const unsigned int MVAStart);

    M4U_STATUS_ENUM m4u_insert_wrapped_range(M4U_MODULE_ID_ENUM eModuleID, 
                  M4U_PORT_ID_ENUM portID, 
								  const unsigned int MVAStart, 
								  const unsigned int MVAEnd); //0:disable, 1~4 is valid
								  		                            
    M4U_STATUS_ENUM m4u_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID, 
		                          unsigned int MVAStart, 
		                          const unsigned int MVAEnd, 
		                          M4U_RANGE_PRIORITY_ENUM ePriority,
		                          unsigned int entryCount);	
		                                
    M4U_STATUS_ENUM m4u_invalid_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
		                          unsigned int MVAStart, 
		                          unsigned int MVAEnd);
		                                  
    M4U_STATUS_ENUM m4u_manual_insert_entry(M4U_MODULE_ID_ENUM eModuleID,
		                          unsigned int EntryMVA, 
		                          bool Lock);	
    M4U_STATUS_ENUM m4u_invalid_tlb_all(M4U_MODULE_ID_ENUM eModuleID);
    M4U_STATUS_ENUM m4u_config_port(M4U_PORT_STRUCT* pM4uPort);

    M4U_STATUS_ENUM m4u_config_port_rotator(M4U_PORT_STRUCT_ROTATOR* pM4uPort);
        
    M4U_STATUS_ENUM m4u_cache_sync(M4U_MODULE_ID_ENUM eModuleID,
		                          M4U_CACHE_SYNC_ENUM eCacheSync,
		                          unsigned int BufAddr, 
		                          unsigned int BufSize);
		                          
    M4U_STATUS_ENUM m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID);
    
    ///> ------- helper function
    M4U_STATUS_ENUM m4u_dump_reg(M4U_MODULE_ID_ENUM eModuleID);
    M4U_STATUS_ENUM m4u_dump_info(M4U_MODULE_ID_ENUM eModuleID);
    M4U_STATUS_ENUM m4u_monitor_start(M4U_PORT_ID_ENUM PortID);
    M4U_STATUS_ENUM m4u_monitor_stop(M4U_PORT_ID_ENUM PortID);	

private:		                          		                          
    int mFileDescriptor;
    #ifdef __PMEM_WRAP_LAYER_EN__
        static bool mUseM4U[M4U_CLNTMOD_MAX];
    #endif
public:
	
    // used for those looply used buffer
    // will check link list for mva rather than re-build pagetable by get_user_pages()
    // if can not find the VA in link list, will call m4u_alloc_mva() internally		  
    M4U_STATUS_ENUM m4u_query_mva(M4U_MODULE_ID_ENUM eModuleID, 
		                          const unsigned int BufAddr, 
		                          const unsigned int BufSize, 
		                          unsigned int *pRetMVABuf);
    M4U_STATUS_ENUM m4u_dump_pagetable(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int MVAStart);

    M4U_STATUS_ENUM m4u_register_buffer(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize,
								  int security,
								  int cache_coherent,
								  unsigned int *pRetMVAAddr);

    M4U_STATUS_ENUM m4u_cache_flush_all(M4U_MODULE_ID_ENUM eModuleID);
    unsigned int m4u_get_reg(unsigned int addr);
    unsigned int m4u_set_reg(unsigned int addr, unsigned int val);



    
#ifdef __PMEM_WRAP_LAYER_EN__
    bool m4u_enable_m4u_func(M4U_MODULE_ID_ENUM eModuleID);
    bool m4u_disable_m4u_func(M4U_MODULE_ID_ENUM eModuleID);
    bool m4u_print_m4u_enable_status();
    bool m4u_check_m4u_en(M4U_MODULE_ID_ENUM eModuleID);
#endif    

};

#endif	/* __M4U_H_ */

