#include <platform/mt_reg_base.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>
         
unsigned int gMutexID = 0;
         
//50us -> 1G needs 50000times
int disp_wait_timeout(BOOL flag, unsigned int timeout)
{
    unsigned int cnt=0;

    while(cnt<timeout)
    {
        if(flag)
        {
            return 0;
        }
        cnt++;
    }
    return -1;
}

int disp_path_get_mutex()
{
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 1);
    if(disp_wait_timeout(((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID))& 0x2) == 0x2), 1000*1000*500))
    {
        printf("[DDP] error! disp_path_get_mutex(), get mutex timeout! \n");
		disp_dump_reg(DISP_MODULE_CONFIG);
    } 

}

int disp_path_release_mutex()
{
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 0);
    if(disp_wait_timeout(((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID)) & 0x2) == 0), 1000*1000*500))
    {
        printf("[DDP] error! disp_path_release_mutex(), release mutex timeout! \n");
		disp_dump_reg(DISP_MODULE_CONFIG);
    }
}



int disp_path_config_layer(OVL_CONFIG_STRUCT* pOvlConfig)
{
	printf("%s, %d\n", __func__, __LINE__);
//    unsigned int reg_addr;
	printf("[DDP]disp_path_config_layer(), layer=%d, source=%d, fmt=%d, addr=0x%x, x=%d, y=%d \n\
	w=%d, h=%d, pitch=%d, keyEn=%d, key=%d, aen=%d, alpha=%d \n ", 
    pOvlConfig->layer,   // layer
    pOvlConfig->source,   // data source (0=memory)
    pOvlConfig->fmt, 
    pOvlConfig->addr, // addr 
    pOvlConfig->x,  // x
    pOvlConfig->y,  // y
    pOvlConfig->w, // width
    pOvlConfig->h, // height
    pOvlConfig->pitch, //pitch, pixel number
    pOvlConfig->keyEn,  //color key
    pOvlConfig->key,  //color key
    pOvlConfig->aen, // alpha enable
    pOvlConfig->alpha);	
//    disp_path_get_mutex();
	
    // config overlay
    OVLLayerSwitch(pOvlConfig->layer, pOvlConfig->layer_en);
    if(pOvlConfig->layer_en!=0)
    {
        OVLLayerConfig(pOvlConfig->layer,   // layer
                       pOvlConfig->source,   // data source (0=memory)
                       pOvlConfig->fmt, 
                       pOvlConfig->addr, // addr 
                       pOvlConfig->x,  // x
                       pOvlConfig->y,  // y
                       pOvlConfig->w, // width
                       pOvlConfig->h, // height
                       pOvlConfig->pitch, //pitch, pixel number
                       pOvlConfig->keyEn,  //color key
                       pOvlConfig->key,  //color key
                       pOvlConfig->aen, // alpha enable
                       pOvlConfig->alpha); // alpha
    }    
//    printf("[DDP]disp_path_config_layer() done, addr=0x%x \n", pOvlConfig->addr);

//    disp_path_release_mutex();
		
    return 0;
}

int disp_path_config_layer_addr(unsigned int layer, unsigned int addr)
{
    unsigned int reg_addr;

    printf("[DDP]disp_path_config_layer_addr(), layer=%d, addr=0x%x\n ", layer, addr);

//    disp_path_get_mutex();	
	
    switch(layer)
    {
        case 0:
			DISP_REG_SET(DISP_REG_OVL_L0_ADDR, addr);
			reg_addr = DISP_REG_OVL_L0_ADDR;
			break;
        case 1:
			DISP_REG_SET(DISP_REG_OVL_L1_ADDR, addr);
			reg_addr = DISP_REG_OVL_L1_ADDR;
			break;
        case 2:
			DISP_REG_SET(DISP_REG_OVL_L2_ADDR, addr);
			reg_addr = DISP_REG_OVL_L2_ADDR;
			break;
        case 3:
			DISP_REG_SET(DISP_REG_OVL_L3_ADDR, addr);
			reg_addr = DISP_REG_OVL_L3_ADDR;
			break;
	    default:
			printf("[DDP] error! error: unknow layer=%d \n", layer);
    }
    printf("[DDP]disp_path_config_layer_addr() done, addr=0x%x \n", DISP_REG_GET(reg_addr));

//    disp_path_release_mutex();
   
    return 0;
}

int disp_path_ddp_clock_on()
{    
    DISP_REG_SET(DISP_REG_CONFIG_CG_CLR0 , (1<<0)|(1<<4)|(1<<5)|(1<<8)|(1<<13)|(1<<14)|(1<<15));
    return 0;	
}

int disp_path_ddp_clock_off()
{    
    DISP_REG_SET(DISP_REG_CONFIG_CG_SET0, (1<<0)|(1<<4)|(1<<5)|(1<<8)|(1<<13)|(1<<14)|(1<<15));

    return 0;	
}


int disp_path_config(struct disp_path_config_struct* pConfig)
{
        ///> get mutex and set mout/sel
//        unsigned int gMutexID = 0;
        unsigned int mutex_mode;

        printf("[DDP]disp_path_config(), srcModule=%d, addr=0x%x, inFormat=%d, \n\
        pitch=%d, bgROI(%d,%d,%d,%d), bgColor=%d, outFormat=%d, dstModule=%d, dstAddr=0x%x,  \n",
            pConfig->srcModule,            
            pConfig->addr,  
            pConfig->inFormat,  
            pConfig->pitch, 
            pConfig->bgROI.x, 
            pConfig->bgROI.y, 
            pConfig->bgROI.width, 
            pConfig->bgROI.height, 
            pConfig->bgColor, 
            pConfig->outFormat,  
            pConfig->dstModule, 
            pConfig->dstAddr);

        if(pConfig->srcModule==DISP_MODULE_RDMA0 && pConfig->dstModule==DISP_MODULE_WDMA1)
        {
            printf("[DDP] error! rdma0 wdma1 can not enable together! \n");
            return -1;
        }

        
        switch(pConfig->dstModule)
        {
            case DISP_MODULE_DSI_VDO:
                mutex_mode = 1;
            break;

            case DISP_MODULE_DPI0:
                mutex_mode = 2;
            break;

            case DISP_MODULE_DBI:
			case DISP_MODULE_DSI_CMD:
			case DISP_MODULE_WDMA1:
                mutex_mode = 0;
            break;

            default:
               printf("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
        }

       
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 1);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 0);
        if(pConfig->srcModule==DISP_MODULE_RDMA0)
        {
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x80); //rdma0=7
        }
        else
        {
	        if(pConfig->dstModule==DISP_MODULE_WDMA1)
	        {
	            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x44); //ovl=2, wdma1=6
	        }
	        else
	        {
#if defined(MTK_AAL_SUPPORT)
	            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x284); //ovl=2, bls=9, rdma0=7
#else
	            // Elsa: de-couple BLS from OVL stream
	            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x84); //ovl=2, rdma0=7
#endif
	        }
        }		
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), mutex_mode);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, (1 << gMutexID));
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN, (1 << gMutexID));        

		// disp_path_get_mutex();

        ///> config config reg
        switch(pConfig->dstModule)
        {
            case DISP_MODULE_DSI:
			case DISP_MODULE_DSI_VDO:
			case DISP_MODULE_DSI_CMD:
                DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x2);  // ovl_mout output to bls
                DISP_REG_SET(DISP_REG_CONFIG_BLS_SEL, 0);        // bls_sel from overlay
                DISP_REG_SET(DISP_REG_CONFIG_RDMA0_OUT_SEL, 0);  // rdma0_mout to dsi0
            break;

            case DISP_MODULE_DPI0:
				        printf("DISI_MODULE_DPI0\n");
                DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x2);   // ovl_mout output to bls
                DISP_REG_SET(DISP_REG_CONFIG_BLS_SEL, 0);         // bls_sel from overlay
                DISP_REG_SET(DISP_REG_CONFIG_RDMA0_OUT_SEL, 0x2); // rdma0_mout to dpi0    
                DISP_REG_SET(DISP_REG_CONFIG_DPI0_SEL, 0);        // dpi0_sel from rdma0
            break;

            case DISP_MODULE_DBI:
                DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x2);   // ovl_mout output to bls
                DISP_REG_SET(DISP_REG_CONFIG_BLS_SEL, 0);         // bls_sel from overlay
                DISP_REG_SET(DISP_REG_CONFIG_RDMA0_OUT_SEL, 0x1); // rdma0_mout to dbi    
                DISP_REG_SET(DISP_REG_CONFIG_DBI_SEL, 0);         // dbi_sel from rdma0            
            break;

            case DISP_MODULE_WDMA1:
                DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x1);   // ovl_mout output to wdma1
            break;

            default:
               printf("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
        }    
        
        ///> config engines
        if(pConfig->srcModule!=DISP_MODULE_RDMA0)
        {            // config OVL
            OVLStop();
            // OVLReset();
            OVLROI(pConfig->bgROI.width, // width
                   pConfig->bgROI.height, // height
                   pConfig->bgColor);// background B

            OVLLayerSwitch(pConfig->ovl_config.layer, pConfig->ovl_config.layer_en);
            if(pConfig->ovl_config.layer_en!=0)
            {
                OVLLayerConfig(pConfig->ovl_config.layer,   // layer
	                           pConfig->ovl_config.source,   // data source (0=memory)
	                           pConfig->ovl_config.fmt, 
	                           pConfig->ovl_config.addr, // addr 
	                           pConfig->ovl_config.x,  // x
	                           pConfig->ovl_config.y,  // y
	                           pConfig->ovl_config.w, // width
	                           pConfig->ovl_config.h, // height
	                           pConfig->ovl_config.pitch,
	                           pConfig->ovl_config.keyEn,  //color key
	                           pConfig->ovl_config.key,  //color key
	                           pConfig->ovl_config.aen, // alpha enable
	                           pConfig->ovl_config.alpha); // alpha
			}

			OVLStart();

            if(pConfig->dstModule==DISP_MODULE_WDMA1)  //1. mem->ovl->wdma1->mem
            {
                WDMAReset(1);
                WDMAConfig(1, 
                           WDMA_INPUT_FORMAT_ARGB, 
                           pConfig->srcROI.width, 
                           pConfig->srcROI.height, 
                           0, 
                           0, 
                           pConfig->srcROI.width, 
                           pConfig->srcROI.height, 
                           pConfig->outFormat, 
                           pConfig->dstAddr, 
                           pConfig->srcROI.width, 
                           1, 
                           0);      
                WDMAStart(1);
            }
            else    //2. ovl->bls->rdma0->lcd
            {
                
#if defined(MTK_AAL_SUPPORT)                
                disp_bls_init(pConfig->srcROI.width, pConfig->srcROI.height);
#endif
                
                ///config RDMA
                RDMAStop(0);
                RDMAReset(0);
                RDMAConfig(0, 
                           RDMA_MODE_DIRECT_LINK,       ///direct link mode
                           RDMA_INPUT_FORMAT_RGB888,    // inputFormat
                           NULL,                        // address
                           pConfig->outFormat,          // output format
                           pConfig->pitch,              // pitch
                           pConfig->srcROI.width,       // width
                           pConfig->srcROI.height,      // height
                           0,                           //byte swap
                           0);                          // is RGB swap        
                           
                RDMAStart(0);
            }
        }
        else  //3. mem->rdma->lcd
        {
            ///config RDMA
            RDMAStop(0);
            RDMAReset(0);
            RDMAConfig(0, 
                       RDMA_MODE_MEMORY,       ///direct link mode
                       pConfig->inFormat,      // inputFormat
                       pConfig->addr,          // address
                       pConfig->outFormat,     // output format
                       pConfig->pitch,          //                                         
                       pConfig->srcROI.width,
                       pConfig->srcROI.height,
                       0,                       //byte swap    
                       0);                      // is RGB swap          
            RDMAStart(0);
        }

        
        disp_dump_reg(DISP_MODULE_OVL);
        disp_dump_reg(DISP_MODULE_WDMA1);
        disp_dump_reg(DISP_MODULE_DPI0);
        disp_dump_reg(DISP_MODULE_RDMA0);
        disp_dump_reg(DISP_MODULE_CONFIG);

//		disp_path_release_mutex();
		
        return 0;
}

unsigned int dbg_log = 1;
unsigned int irq_log = 0;
#define DISP_WRN(string, args...) if(dbg_log) printf("[DSS]"string,##args)
#define DISP_MSG(string, args...) printf("[DSS]"string,##args)
#define DISP_ERR(string, args...) printf("[DSS]error:"string,##args)
#define DISP_IRQ(string, args...) if(irq_log) printf("[DSS]"string,##args)

int disp_dump_reg(DISP_MODULE_ENUM module)           
{
    unsigned int index;    
    switch(module)
    {        
        case DISP_MODULE_ROT: 
            DISP_MSG("===== DISP ROT Reg Dump: ============\n");    
            DISP_MSG("(+000)ROT_EN                   =0x%x\n", DISP_REG_GET(DISP_REG_ROT_EN                   ));
            DISP_MSG("(+008)ROT_RESET                =0x%x\n", DISP_REG_GET(DISP_REG_ROT_RESET                ));
            DISP_MSG("(+010)ROT_INTERRUPT_ENABLE     =0x%x\n", DISP_REG_GET(DISP_REG_ROT_INTERRUPT_ENABLE       ));
            DISP_MSG("(+018)ROT_INTERRUPT_STATUS     =0x%x\n", DISP_REG_GET(DISP_REG_ROT_INTERRUPT_STATUS       ));
            DISP_MSG("(+020)ROT_CON                  =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CON                   ));
            DISP_MSG("(+028)ROT_GMCIF_CON            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_GMCIF_CON            ));
            DISP_MSG("(+030)ROT_SRC_CON              =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SRC_CON               ));
            DISP_MSG("(+040)ROT_SRC_BASE_0           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SRC_BASE_0           ));
            DISP_MSG("(+048)ROT_SRC_BASE_1           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SRC_BASE_1           ));
            DISP_MSG("(+050)ROT_SRC_BASE_2           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SRC_BASE_2           ));
            DISP_MSG("(+060)ROT_MF_BKGD_SIZE_IN_BYTE =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MF_BKGD_SIZE_IN_BYTE ));
            DISP_MSG("(+070)ROT_MF_SRC_SIZE          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MF_SRC_SIZE           ));
            DISP_MSG("(+078)ROT_MF_CLIP_SIZE         =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MF_CLIP_SIZE           ));
            DISP_MSG("(+080)ROT_MF_OFFSET_1          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MF_OFFSET_1           ));
            DISP_MSG("(+088)ROT_MF_PAR               =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MF_PAR               ));
            DISP_MSG("(+090)ROT_SF_BKGD_SIZE_IN_BYTE =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SF_BKGD_SIZE_IN_BYTE ));
            DISP_MSG("(+0B8)ROT_SF_PAR               =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SF_PAR               ));
            DISP_MSG("(+0C0)ROT_MB_DEPTH             =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MB_DEPTH               ));
            DISP_MSG("(+0C8)ROT_MB_BASE              =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MB_BASE               ));
            DISP_MSG("(+0D0)ROT_MB_CON               =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MB_CON               ));
            DISP_MSG("(+0D8)ROT_SB_DEPTH             =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SB_DEPTH               ));
            DISP_MSG("(+0E0)ROT_SB_BASE              =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SB_BASE               ));
            DISP_MSG("(+0E8)ROT_SB_CON               =0x%x\n", DISP_REG_GET(DISP_REG_ROT_SB_CON               ));
            DISP_MSG("(+0F0)ROT_VC1_RANGE            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_VC1_RANGE            ));
            DISP_MSG("(+200)ROT_TRANSFORM_0          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_0           ));
            DISP_MSG("(+208)ROT_TRANSFORM_1          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_1           ));
            DISP_MSG("(+210)ROT_TRANSFORM_2          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_2           ));
            DISP_MSG("(+218)ROT_TRANSFORM_3          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_3           ));
            DISP_MSG("(+220)ROT_TRANSFORM_4          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_4           ));
            DISP_MSG("(+228)ROT_TRANSFORM_5          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_5           ));
            DISP_MSG("(+230)ROT_TRANSFORM_6          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_6           ));
            DISP_MSG("(+238)ROT_TRANSFORM_7          =0x%x\n", DISP_REG_GET(DISP_REG_ROT_TRANSFORM_7           ));
            DISP_MSG("(+240)ROT_RESV_DUMMY_0         =0x%x\n", DISP_REG_GET(DISP_REG_ROT_RESV_DUMMY_0           ));
            DISP_MSG("(+300)ROT_CHKS_EXTR            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_EXTR            ));
            DISP_MSG("(+308)ROT_CHKS_INTW            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_INTW            ));
            DISP_MSG("(+310)ROT_CHKS_INTR            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_INTR            ));
            DISP_MSG("(+318)ROT_CHKS_ROTO            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_ROTO            ));
            DISP_MSG("(+320)ROT_CHKS_SRIY            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_SRIY            ));
            DISP_MSG("(+328)ROT_CHKS_SRIU            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_SRIU            ));
            DISP_MSG("(+330)ROT_CHKS_SRIV            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_SRIV            ));
            DISP_MSG("(+338)ROT_CHKS_SROY            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_SROY            ));
            DISP_MSG("(+340)ROT_CHKS_SROU            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_SROU            ));
            DISP_MSG("(+348)ROT_CHKS_SROV            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_SROV            ));
            DISP_MSG("(+350)ROT_CHKS_VUPI            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_VUPI            ));
            DISP_MSG("(+358)ROT_CHKS_VUPO            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_CHKS_VUPO            ));
            DISP_MSG("(+380)ROT_DEBUG_CON            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_DEBUG_CON            ));
            DISP_MSG("(+400)ROT_MON_STA_0            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_0            ));
            DISP_MSG("(+408)ROT_MON_STA_1            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_1            ));
            DISP_MSG("(+410)ROT_MON_STA_2            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_2            ));
            DISP_MSG("(+418)ROT_MON_STA_3            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_3            ));
            DISP_MSG("(+420)ROT_MON_STA_4            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_4            ));
            DISP_MSG("(+428)ROT_MON_STA_5            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_5            ));
            DISP_MSG("(+430)ROT_MON_STA_6            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_6            ));
            DISP_MSG("(+438)ROT_MON_STA_7            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_7            ));
            DISP_MSG("(+440)ROT_MON_STA_8            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_8            ));
            DISP_MSG("(+448)ROT_MON_STA_9            =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_9            ));
            DISP_MSG("(+450)ROT_MON_STA_10           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_10           ));
            DISP_MSG("(+458)ROT_MON_STA_11           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_11           ));
            DISP_MSG("(+460)ROT_MON_STA_12           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_12           ));
            DISP_MSG("(+468)ROT_MON_STA_13           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_13           ));
            DISP_MSG("(+470)ROT_MON_STA_14           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_14           ));
            DISP_MSG("(+478)ROT_MON_STA_15           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_15           ));
            DISP_MSG("(+480)ROT_MON_STA_16           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_16           ));
            DISP_MSG("(+488)ROT_MON_STA_17           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_17           ));
            DISP_MSG("(+490)ROT_MON_STA_18           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_18           ));
            DISP_MSG("(+498)ROT_MON_STA_19           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_19           ));
            DISP_MSG("(+4A0)ROT_MON_STA_20           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_20           ));
            DISP_MSG("(+4A8)ROT_MON_STA_21           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_21           ));
            DISP_MSG("(+4B0)ROT_MON_STA_22           =0x%x\n", DISP_REG_GET(DISP_REG_ROT_MON_STA_22           ));
            break;
        
        case DISP_MODULE_SCL: 
            DISP_MSG("===== DISP SCL Reg Dump: ============\n");       
            DISP_MSG("(+000)SCL_CTRL       =0x%x \n", DISP_REG_GET(DISP_REG_SCL_CTRL       ));
            DISP_MSG("(+004)SCL_INTEN      =0x%x \n", DISP_REG_GET(DISP_REG_SCL_INTEN      ));
            DISP_MSG("(+008)SCL_INTSTA     =0x%x \n", DISP_REG_GET(DISP_REG_SCL_INTSTA     ));
            DISP_MSG("(+00C)SCL_STATUS     =0x%x \n", DISP_REG_GET(DISP_REG_SCL_STATUS     ));
            DISP_MSG("(+010)SCL_CFG        =0x%x \n", DISP_REG_GET(DISP_REG_SCL_CFG        ));
            DISP_MSG("(+018)SCL_INP_CHKSUM =0x%x \n", DISP_REG_GET(DISP_REG_SCL_INP_CHKSUM ));
            DISP_MSG("(+01C)SCL_OUTP_CHKSUM=0x%x \n", DISP_REG_GET(DISP_REG_SCL_OUTP_CHKSUM));
            DISP_MSG("(+020)SCL_HRZ_CFG    =0x%x \n", DISP_REG_GET(DISP_REG_SCL_HRZ_CFG    ));
            DISP_MSG("(+024)SCL_HRZ_SIZE   =0x%x \n", DISP_REG_GET(DISP_REG_SCL_HRZ_SIZE   ));
            DISP_MSG("(+028)SCL_HRZ_FACTOR =0x%x \n", DISP_REG_GET(DISP_REG_SCL_HRZ_FACTOR ));
            DISP_MSG("(+02C)SCL_HRZ_OFFSET =0x%x \n", DISP_REG_GET(DISP_REG_SCL_HRZ_OFFSET ));
            DISP_MSG("(+040)SCL_VRZ_CFG    =0x%x \n", DISP_REG_GET(DISP_REG_SCL_VRZ_CFG    ));
            DISP_MSG("(+044)SCL_VRZ_SIZE   =0x%x \n", DISP_REG_GET(DISP_REG_SCL_VRZ_SIZE   ));
            DISP_MSG("(+048)SCL_VRZ_FACTOR =0x%x \n", DISP_REG_GET(DISP_REG_SCL_VRZ_FACTOR ));
            DISP_MSG("(+04C)SCL_VRZ_OFFSET =0x%x \n", DISP_REG_GET(DISP_REG_SCL_VRZ_OFFSET ));
            DISP_MSG("(+060)SCL_EXT_COEF   =0x%x \n", DISP_REG_GET(DISP_REG_SCL_EXT_COEF   ));
            DISP_MSG("(+064)SCL_PEAK_CFG   =0x%x \n", DISP_REG_GET(DISP_REG_SCL_PEAK_CFG   ));
            break;
             
        case DISP_MODULE_OVL: 
            DISP_MSG("===== DISP OVL Reg Dump: ============\n");           
            DISP_MSG("(000)OVL_STA                    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_STA                     ));
            DISP_MSG("(004)OVL_INTEN                  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_INTEN                 ));
            DISP_MSG("(008)OVL_INTSTA                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_INTSTA                 ));
            DISP_MSG("(00C)OVL_EN                     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_EN                     ));
            DISP_MSG("(010)OVL_TRIG                   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_TRIG                  ));
            DISP_MSG("(014)OVL_RST                    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RST                     ));
            DISP_MSG("(020)OVL_ROI_SIZE               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ROI_SIZE              ));
            DISP_MSG("(024)OVL_DATAPATH_CON           =0x%x \n", DISP_REG_GET(DISP_REG_OVL_DATAPATH_CON          ));
            DISP_MSG("(028)OVL_ROI_BGCLR              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ROI_BGCLR             ));
            DISP_MSG("(02C)OVL_SRC_CON                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_SRC_CON                 ));
            DISP_MSG("(030)OVL_L0_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_CON                 ));
            DISP_MSG("(034)OVL_L0_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_SRCKEY             ));
            DISP_MSG("(038)OVL_L0_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE             ));
            DISP_MSG("(03C)OVL_L0_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_OFFSET             ));
            DISP_MSG("(040)OVL_L0_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_ADDR                 ));
            DISP_MSG("(044)OVL_L0_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_PITCH              ));
            DISP_MSG("(0C0)OVL_RDMA0_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_CTRL             ));
            DISP_MSG("(0C4)OVL_RDMA0_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_START_TRIG  ));
            DISP_MSG("(0C8)OVL_RDMA0_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING ));
            DISP_MSG("(0CC)OVL_RDMA0_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_SLOW_CON     ));
            DISP_MSG("(0D0)OVL_RDMA0_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_FIFO_CTRL         ));
            DISP_MSG("(050)OVL_L1_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_CON                 ));
            DISP_MSG("(054)OVL_L1_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_SRCKEY             ));
            DISP_MSG("(058)OVL_L1_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE             ));
            DISP_MSG("(05C)OVL_L1_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_OFFSET             ));
            DISP_MSG("(060)OVL_L1_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_ADDR                 ));
            DISP_MSG("(064)OVL_L1_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_PITCH              ));
            DISP_MSG("(0E0)OVL_RDMA1_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_CTRL             ));
            DISP_MSG("(0E4)OVL_RDMA1_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_START_TRIG  ));
            DISP_MSG("(0E8)OVL_RDMA1_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING ));
            DISP_MSG("(0EC)OVL_RDMA1_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_SLOW_CON     ));
            DISP_MSG("(0F0)OVL_RDMA1_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_FIFO_CTRL         ));
            DISP_MSG("(070)OVL_L2_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_CON                 ));
            DISP_MSG("(074)OVL_L2_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_SRCKEY             ));
            DISP_MSG("(078)OVL_L2_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE             ));
            DISP_MSG("(07C)OVL_L2_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_OFFSET             ));
            DISP_MSG("(080)OVL_L2_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_ADDR                 ));
            DISP_MSG("(084)OVL_L2_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_PITCH              ));
            DISP_MSG("(100)OVL_RDMA2_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_CTRL             ));
            DISP_MSG("(104)OVL_RDMA2_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_START_TRIG  ));
            DISP_MSG("(108)OVL_RDMA2_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING ));
            DISP_MSG("(10C)OVL_RDMA2_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_SLOW_CON     ));
            DISP_MSG("(110)OVL_RDMA2_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_FIFO_CTRL         ));
            DISP_MSG("(090)OVL_L3_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_CON                 ));
            DISP_MSG("(094)OVL_L3_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_SRCKEY             ));
            DISP_MSG("(098)OVL_L3_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE             ));
            DISP_MSG("(09C)OVL_L3_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_OFFSET             ));
            DISP_MSG("(0A0)OVL_L3_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_ADDR                 ));
            DISP_MSG("(0A4)OVL_L3_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_PITCH              ));
            DISP_MSG("(120)OVL_RDMA3_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_CTRL             ));
            DISP_MSG("(124)OVL_RDMA3_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_START_TRIG  ));
            DISP_MSG("(128)OVL_RDMA3_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING ));
            DISP_MSG("(12C)OVL_RDMA3_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_SLOW_CON     ));
            DISP_MSG("(130)OVL_RDMA3_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_FIFO_CTRL         ));
            DISP_MSG("(1C4)OVL_DEBUG_MON_SEL          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_DEBUG_MON_SEL         ));
            DISP_MSG("(1C4)OVL_RDMA0_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2));
            DISP_MSG("(1C8)OVL_RDMA1_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2));
            DISP_MSG("(1CC)OVL_RDMA2_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2));
            DISP_MSG("(1D0)OVL_RDMA3_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2));
            DISP_MSG("(240)OVL_FLOW_CTRL_DBG          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG         ));
            DISP_MSG("(244)OVL_ADDCON_DBG             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG             ));
            DISP_MSG("(248)OVL_OUTMUX_DBG             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_OUTMUX_DBG             ));
            DISP_MSG("(24C)OVL_RDMA0_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_DBG             ));
            DISP_MSG("(250)OVL_RDMA1_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_DBG             ));
            DISP_MSG("(254)OVL_RDMA2_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_DBG             ));
            DISP_MSG("(258)OVL_RDMA3_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_DBG             ));
            break;
             

        case DISP_MODULE_COLOR:  
        break;                 
        case DISP_MODULE_TDSHP:  
        break;                 
        case DISP_MODULE_BLS:      
        break;
                 
        case DISP_MODULE_WDMA0:  
        case DISP_MODULE_WDMA1: 
            if(DISP_MODULE_WDMA0==module)
            {
                index = 0;
            }
            else if(DISP_MODULE_WDMA1==module)
            {
                index = 1;
            }  
            DISP_MSG("===== DISP WDMA%d Reg Dump: ============\n", index);      
            DISP_MSG("(000)WDMA_INTEN         =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_INTEN        +0x1000*index) );
            DISP_MSG("(004)WDMA_INTSTA        =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_INTSTA       +0x1000*index) );
            DISP_MSG("(008)WDMA_EN            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_EN           +0x1000*index) );
            DISP_MSG("(00C)WDMA_RST           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_RST          +0x1000*index) );
            DISP_MSG("(010)WDMA_SMI_CON       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_SMI_CON      +0x1000*index) );
            DISP_MSG("(014)WDMA_CFG           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CFG          +0x1000*index) );
            DISP_MSG("(018)WDMA_SRC_SIZE      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE     +0x1000*index) );
            DISP_MSG("(01C)WDMA_CLIP_SIZE     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE    +0x1000*index) );
            DISP_MSG("(020)WDMA_CLIP_COORD    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD   +0x1000*index) );
            DISP_MSG("(024)WDMA_DST_ADDR      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR     +0x1000*index) );
            DISP_MSG("(028)WDMA_DST_W_IN_BYTE =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE+0x1000*index) );
            DISP_MSG("(02C)WDMA_ALPHA         =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_ALPHA        +0x1000*index) );
            DISP_MSG("(030)WDMA_BUF_ADDR      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_ADDR     +0x1000*index) );
            DISP_MSG("(034)WDMA_STA           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_STA          +0x1000*index) );
            DISP_MSG("(038)WDMA_BUF_CON1      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON1     +0x1000*index) );
            DISP_MSG("(03C)WDMA_BUF_CON2      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON2     +0x1000*index) );
            DISP_MSG("(040)WDMA_C00           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C00          +0x1000*index) );
            DISP_MSG("(044)WDMA_C02           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C02          +0x1000*index) );
            DISP_MSG("(048)WDMA_C10           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C10          +0x1000*index) );
            DISP_MSG("(04C)WDMA_C12           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C12          +0x1000*index) );
            DISP_MSG("(050)WDMA_C20           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C20          +0x1000*index) );
            DISP_MSG("(054)WDMA_C22           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C22          +0x1000*index) );
            DISP_MSG("(058)WDMA_PRE_ADD0      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD0     +0x1000*index) );
            DISP_MSG("(05C)WDMA_PRE_ADD2      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD2     +0x1000*index) );
            DISP_MSG("(060)WDMA_POST_ADD0     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD0    +0x1000*index) );
            DISP_MSG("(064)WDMA_POST_ADD2     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD2    +0x1000*index) );
            DISP_MSG("(070)WDMA_DST_U_ADDR    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_U_ADDR   +0x1000*index) );
            DISP_MSG("(074)WDMA_DST_V_ADDR    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_V_ADDR   +0x1000*index) );
            DISP_MSG("(078)WDMA_DST_UV_PITCH  =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH +0x1000*index) );
            DISP_MSG("(090)WDMA_DITHER_CON    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DITHER_CON   +0x1000*index) );
            DISP_MSG("(0A0)WDMA_FLOW_CTRL_DBG =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG+0x1000*index) );
            DISP_MSG("(0A4)WDMA_EXEC_DBG      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG     +0x1000*index) );
            DISP_MSG("(0A8)WDMA_CLIP_DBG      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_DBG     +0x1000*index) );
            break;      
           
        case DISP_MODULE_RDMA0:  
        case DISP_MODULE_RDMA1:  
            if(DISP_MODULE_RDMA0==module)
            {
                index = 0;
            }
            else if(DISP_MODULE_RDMA1==module)
            {
                index = 1;
            } 
            DISP_MSG("===== DISP RDMA%d Reg Dump: ======== \n", index);
            DISP_MSG("(000)RDMA_INT_ENABLE        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE       +0x1000*index));
            DISP_MSG("(004)RDMA_INT_STATUS        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_INT_STATUS       +0x1000*index));
            DISP_MSG("(010)RDMA_GLOBAL_CON        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON       +0x1000*index));
            DISP_MSG("(014)RDMA_SIZE_CON_0        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0       +0x1000*index));
            DISP_MSG("(018)RDMA_SIZE_CON_1        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1       +0x1000*index));
            DISP_MSG("(024)RDMA_MEM_CON           =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_CON          +0x1000*index));
            DISP_MSG("(028)RDMA_MEM_START_ADDR    =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR   +0x1000*index));
            DISP_MSG("(02C)RDMA_MEM_SRC_PITCH     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH    +0x1000*index));
            DISP_MSG("(030)RDMA_MEM_GMC_SETTING_0 =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0+0x1000*index));
            DISP_MSG("(034)RDMA_MEM_SLOW_CON      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON     +0x1000*index));
            DISP_MSG("(030)RDMA_MEM_GMC_SETTING_1 =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1+0x1000*index));
            DISP_MSG("(040)RDMA_FIFO_CON          =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_FIFO_CON         +0x1000*index));
            DISP_MSG("(054)RDMA_CF_00             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_00            +0x1000*index));
            DISP_MSG("(058)RDMA_CF_01             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_01            +0x1000*index));
            DISP_MSG("(05C)RDMA_CF_02             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_02            +0x1000*index));
            DISP_MSG("(060)RDMA_CF_10             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_10            +0x1000*index));
            DISP_MSG("(064)RDMA_CF_11             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_11            +0x1000*index));
            DISP_MSG("(068)RDMA_CF_12             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_12            +0x1000*index));
            DISP_MSG("(06C)RDMA_CF_20             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_20            +0x1000*index));
            DISP_MSG("(070)RDMA_CF_21             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_21            +0x1000*index));
            DISP_MSG("(074)RDMA_CF_22             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_22            +0x1000*index));
            DISP_MSG("(078)RDMA_CF_PRE_ADD0       =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD0      +0x1000*index));
            DISP_MSG("(07C)RDMA_CF_PRE_ADD1       =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD1      +0x1000*index));
            DISP_MSG("(080)RDMA_CF_PRE_ADD2       =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD2      +0x1000*index));
            DISP_MSG("(084)RDMA_CF_POST_ADD0      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD0     +0x1000*index));
            DISP_MSG("(088)RDMA_CF_POST_ADD1      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD1     +0x1000*index));
            DISP_MSG("(08C)RDMA_CF_POST_ADD2      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD2     +0x1000*index));      
            DISP_MSG("(090)RDMA_DUMMY             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_DUMMY            +0x1000*index));
            DISP_MSG("(094)RDMA_DEBUG_OUT_SEL     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL    +0x1000*index));
            break;
    
        case DISP_MODULE_CMDQ:
            DISP_MSG("===== DISP CMDQ Reg Dump: ============\n");
            DISP_MSG("(0x10)CMDQ_IRQ_FLAG          =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_IRQ_FLAG        )); 
            DISP_MSG("(0x20)CMDQ_LOADED_THR        =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_LOADED_THR      )); 
            DISP_MSG("(0x30)CMDQ_THR_SLOT_CYCLES   =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_THR_SLOT_CYCLES )); 
            DISP_MSG("(0x40)CMDQ_BUS_CTRL          =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_BUS_CTRL        ));                                               
            DISP_MSG("(0x50)CMDQ_ABORT             =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_ABORT           )); 
            for(index=0;index<CMDQ_THREAD_NUM;index++)
            {
                DISP_MSG("(0x%x)CMDQ_THRx_RESET%d                =0x%x \n", (0x100 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_RESET(index)               )); 
                DISP_MSG("(0x%x)CMDQ_THRx_EN%d                   =0x%x \n", (0x104 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(index)                  )); 
                DISP_MSG("(0x%x)CMDQ_THRx_SUSPEND%d              =0x%x \n", (0x108 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_SUSPEND(index)             )); 
                DISP_MSG("(0x%x)CMDQ_THRx_STATUS%d               =0x%x \n", (0x10c + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(index)              )); 
                DISP_MSG("(0x%x)CMDQ_THRx_IRQ_FLAG%d             =0x%x \n", (0x110 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG(index)            )); 
                DISP_MSG("(0x%x)CMDQ_THRx_IRQ_FLAG_EN%d          =0x%x \n", (0x114 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(index)         )); 
                DISP_MSG("(0x%x)CMDQ_THRx_SECURITY%d             =0x%x \n", (0x118 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_SECURITY(index)            )); 
                DISP_MSG("(0x%x)CMDQ_THRx_PC%d                   =0x%x \n", (0x120 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(index)                  )); 
                DISP_MSG("(0x%x)CMDQ_THRx_END_ADDR%d             =0x%x \n", (0x124 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(index)            )); 
                DISP_MSG("(0x%x)CMDQ_THRx_EXEC_CMDS_CNT%d        =0x%x \n", (0x128 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(index)       )); 
                DISP_MSG("(0x%x)CMDQ_THRx_WAIT_EVENTS0%d         =0x%x \n", (0x130 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS0(index)        )); 
                DISP_MSG("(0x%x)CMDQ_THRx_WAIT_EVENTS1%d         =0x%x \n", (0x134 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS1(index)        )); 
                DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS0%d     =0x%x \n", (0x140 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS0(index)    )); 
                DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS1%d     =0x%x \n", (0x144 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS1(index)    )); 
                DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS0_CLR%d =0x%x \n", (0x148 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS0_CLR(index))); 
                DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS1_CLR%d =0x%x \n", (0x14c + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS1_CLR(index))); 
                DISP_MSG("(0x%x)CMDQ_THRx_INSTN_TIMEOUT_CYCLES%d =0x%x \n", (0x150 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(index))); 
            }
            break;

                 
        case DISP_MODULE_GAMMA:  
        break;                 
        case DISP_MODULE_DBI:      
        break;                 
        case DISP_MODULE_DSI:
		case DISP_MODULE_DSI_VDO:
		case DISP_MODULE_DSI_CMD:
        break;                 
        case DISP_MODULE_DPI1:  
        break;
                         
        case DISP_MODULE_DPI0:   
            /*
            DISP_MSG("===== DISP DPI0 Reg Dump: ============\n");
            DISP_MSG("(0x00)DPI0_EN_REG               = 0x%x \n", *DPI0_EN_REG               ); 
            DISP_MSG("(0x04)DPI0_RST_REG              = 0x%x \n", *DPI0_RST_REG              );
            DISP_MSG("(0x08)DPI0_INTEN_REG            = 0x%x \n", *DPI0_INTEN_REG            ); 
            DISP_MSG("(0x0C)DPI0_INTSTA_REG           = 0x%x \n", *DPI0_INTSTA_REG           );    
            DISP_MSG("(0x10)DPI0_CON_REG              = 0x%x \n", *DPI0_CON_REG              ); 
            DISP_MSG("(0x14)DPI0_CLKCON_REG           = 0x%x \n", *DPI0_CLKCON_REG           ); 
            DISP_MSG("(0x18)DPI0_SIZE_REG             = 0x%x \n", *DPI0_SIZE_REG             ); 
            DISP_MSG("(0x1C)DPI0_TGEN_HWIDTH_REG      = 0x%x \n", *DPI0_TGEN_HWIDTH_REG      ); 
            DISP_MSG("(0x20)DPI0_TGEN_HPORCH_REG      = 0x%x \n", *DPI0_TGEN_HPORCH_REG      );    
            DISP_MSG("(0x24)DPI0_TGEN_VWIDTH_LODD_REG = 0x%x \n", *DPI0_TGEN_VWIDTH_LODD_REG );    
            DISP_MSG("(0x28)DPI0_TGEN_VPORCH_LODD_REG = 0x%x \n", *DPI0_TGEN_VPORCH_LODD_REG );    
            DISP_MSG("(0x2C)DPI0_TGEN_WWIDTH_LEVEN_REG= 0x%x \n", *DPI0_TGEN_WWIDTH_LEVEN_REG); 
            DISP_MSG("(0x30)DPI0_TGEN_VPORCH_LEVEN_REG= 0x%x \n", *DPI0_TGEN_VPORCH_LEVEN_REG); 
            DISP_MSG("(0x34)DPI0_TGEN_VWIDTH_RODD_REG = 0x%x \n", *DPI0_TGEN_VWIDTH_RODD_REG ); 
            DISP_MSG("(0x38)DPI0_TGEN_VPORCH_RODD_REG = 0x%x \n", *DPI0_TGEN_VPORCH_RODD_REG ); 
            DISP_MSG("(0x50)DPI0_BG_HCNTL_REG         = 0x%x \n", *DPI0_BG_HCNTL_REG         ); 
            DISP_MSG("(0x54)DPI0_BG_VCNTL_REG         = 0x%x \n", *DPI0_BG_VCNTL_REG         );    
            DISP_MSG("(0x60)DPI0_STATUS_REG           = 0x%x \n", *DPI0_STATUS_REG           );
            DISP_MSG("(0x64)DPI0_MATRIX_COEFF_SET0_REG= 0x%x \n", *DPI0_MATRIX_COEFF_SET0_REG); 
            DISP_MSG("(0x68)DPI0_MATRIX_COEFF_SET1_REG= 0x%x \n", *DPI0_MATRIX_COEFF_SET1_REG); 
            DISP_MSG("(0x6C)DPI0_MATRIX_COEFF_SET2_REG= 0x%x \n", *DPI0_MATRIX_COEFF_SET2_REG); 
            DISP_MSG("(0X8C)DPI0_Y_LIMIT_REG          = 0x%x \n", *DPI0_Y_LIMIT_REG          ); 
            DISP_MSG("(0x90)DPI0_C_LIMIT_REG          = 0x%x \n", *DPI0_C_LIMIT_REG          );
            */
            break;


        case DISP_MODULE_CONFIG:
            DISP_MSG("(0x020)CFG_SCL_MOUT_EN    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_SCL_MOUT_EN     ));
            DISP_MSG("(0x024)CFG_OVL_MOUT_EN    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_OVL_MOUT_EN     ));
            DISP_MSG("(0x028)CFG_COLOR_MOUT_EN  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_COLOR_MOUT_EN   ));
            DISP_MSG("(0x02C)CFG_TDSHP_MOUT_EN  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TDSHP_MOUT_EN   ));
            DISP_MSG("(0x030)CFG_MOUT_RST       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MOUT_RST        ));
            DISP_MSG("(0x034)CFG_RDMA0_OUT_SEL  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RDMA0_OUT_SEL   ));
            DISP_MSG("(0x038)CFG_RDMA1_OUT_SEL  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RDMA1_OUT_SEL   ));
            DISP_MSG("(0x03C)CFG_OVL_PQ_OUT_SEL =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_OVL_PQ_OUT_SEL  ));
            DISP_MSG("(0x040)CFG_WDMA0_SEL      =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WDMA0_SEL       ));
            DISP_MSG("(0x044)CFG_OVL_SEL        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_OVL_SEL         ));
            DISP_MSG("(0x048)CFG_OVL_PQ_IN_SEL  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_OVL_PQ_IN_SEL   ));
            DISP_MSG("(0x04C)CFG_COLOR_SEL      =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_COLOR_SEL       ));
            DISP_MSG("(0x050)CFG_TDSHP_SEL      =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TDSHP_SEL       ));
            DISP_MSG("(0x054)CFG_BLS_SEL        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_BLS_SEL         ));
            DISP_MSG("(0x058)CFG_DBI_SEL        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DBI_SEL         ));
            DISP_MSG("(0x05C)CFG_DPI0_SEL       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DPI0_SEL        ));
            DISP_MSG("(0x60))CFG_MISC           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MISC            ));
            DISP_MSG("(0x70))CFG_PATH_DEBUG0    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_PATH_DEBUG0     ));
            DISP_MSG("(0x74))CFG_PATH_DEBUG1    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_PATH_DEBUG1     ));
            DISP_MSG("(0x78))CFG_PATH_DEBUG2    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_PATH_DEBUG2     ));
            DISP_MSG("(0x7C))CFG_PATH_DEBUG3    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_PATH_DEBUG3     ));
            DISP_MSG("(0x80))CFG_PATH_DEBUG4    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_PATH_DEBUG4     ));
            DISP_MSG("(0x100)CFG_CG_CON0        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CON0         ));
            DISP_MSG("(0x104)CFG_CG_SET0        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_SET0         ));
            DISP_MSG("(0x108)CFG_CG_CLR0        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CLR0         ));
            DISP_MSG("(0x110)CFG_CG_CON1        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CON1         ));
            DISP_MSG("(0x114)CFG_CG_SET1        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_SET1         ));
            DISP_MSG("(0x118)CFG_CG_CLR1        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CLR1         ));
            DISP_MSG("(0x120)CFG_HW_DCM_EN0     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN0      ));
            DISP_MSG("(0x124)CFG_HW_DCM_EN_SET0 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_SET0  ));
            DISP_MSG("(0x128)CFG_HW_DCM_EN_CLR0 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_CLR0  ));
            DISP_MSG("(0x130)CFG_HW_DCM_EN1     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN1      ));
            DISP_MSG("(0x134)CFG_HW_DCM_EN_SET1 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_SET1  ));
            DISP_MSG("(0x138)CFG_HW_DCM_EN_CLR1 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_CLR1  ));
            DISP_MSG("(0x800)CFG_MBIST_DONE0    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_DONE0     ));
            DISP_MSG("(0x804)CFG_MBIST_DONE1    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_DONE1     ));
            DISP_MSG("(0x808)CFG_MBIST_FAIL0    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL0     ));
            DISP_MSG("(0x80C)CFG_MBIST_FAIL1    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL1     ));
            DISP_MSG("(0x810)CFG_MBIST_FAIL2    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL2     ));
            DISP_MSG("(0x814)CFG_MBIST_HOLDB0   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_HOLDB0    ));
            DISP_MSG("(0x818)CFG_MBIST_HOLDB1   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_HOLDB1    ));
            DISP_MSG("(0x81C)CFG_MBIST_MODE0    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_MODE0     ));
            DISP_MSG("(0x820)CFG_MBIST_MODE1    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_MODE1     ));
            DISP_MSG("(0x824)CFG_MBIST_BSEL0    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL0     ));
            DISP_MSG("(0x828)CFG_MBIST_BSEL1    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL1     ));
            DISP_MSG("(0x82C)CFG_MBIST_BSEL2    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL2     ));
            DISP_MSG("(0x830)CFG_MBIST_BSEL3    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL3     ));
            DISP_MSG("(0x834)CFG_MBIST_CON      =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_CON       ));
            DISP_MSG("(0x870)CFG_DEBUG_OUT_SEL  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DEBUG_OUT_SEL   ));
            DISP_MSG("(0x874)CFG_TEST_CLK_SEL   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TEST_CLK_SEL    ));
            DISP_MSG("(0x880)CFG_DUMMY          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DUMMY           ));
            DISP_MSG("((0x0) CFG_MUTEX_INTEN    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN     )); 
            DISP_MSG("((0x4) CFG_MUTEX_INTSTA   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA    )); 
            DISP_MSG("((0x8) CFG_REG_UPD_TIMEOUT=0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_REG_UPD_TIMEOUT )); 
            DISP_MSG("((0xC) CFG_REG_COMMIT     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT      )); 
            for(index=0;index<5;index++)
            {
                DISP_MSG("(0x%x)CFG_MUTEX_EN(%d)  =0x%x \n", 0x20 + (0x20 * index), index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_EN(index)  )); 
                DISP_MSG("(0x%x)CFG_MUTEX(%d)     =0x%x \n", 0x24 + (0x20 * index), index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX(index)     )); 
                DISP_MSG("(0x%x)CFG_MUTEX_RST(%d) =0x%x \n", 0x28 + (0x20 * index), index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_RST(index) )); 
                DISP_MSG("(0x%x)CFG_MUTEX_MOD(%d) =0x%x \n", 0x2C + (0x20 * index), index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(index) )); 
                DISP_MSG("(0x%x)CFG_MUTEX_SOF(%d) =0x%x \n", 0x30 + (0x20 * index), index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(index) )); 
            }
            break;
        
        default:
        	  DISP_MSG("disp_dump_reg() invalid module id=%d \n", module);

    }

    return 0;
}
