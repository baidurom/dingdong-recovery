#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#include <mach/m4u.h> //for m4u
#include <asm/io.h>
#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h> //clock manage 
#include <mach/mt_irq.h>
#include <mach/sync_write.h>


#include "ddp_hal.h"
#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_path.h"
#include "ddp_debug.h"
#include "ddp_bls.h"
#include "ddp_rdma.h"
#include "ddp_wdma.h"
#include "ddp_ovl.h"
#include "ddp_debug.h"
#include "dsi_drv.h"

//#include <linux/mmprofile.h>

#include "ddp_slt.h"

struct REGION_SLT
{
	unsigned x;
	unsigned y;
	unsigned width;
	unsigned height;
};


//source and golden
extern unsigned char data_argb_64x64[16384];
extern unsigned char slt_data_rgb888_64x64_golden[12288];
extern unsigned char data_rgb888_64x64_golden2[12288];


//extern struct DDP_MMP_Events_t DDP_MMP_Events;//mmp
extern LCM_PARAMS *lcm_params;

unsigned int gSltBitbltVerify = 0;// 2 times verify 
unsigned int gMutexID_slt = 0;//use mutex0 
//restore mutex register
unsigned int reg_mutex_mod2 =0;
unsigned int reg_mutex_sof2 =0;
unsigned int reg_mout2 =0;


struct REGION_SLT region_slt = {0, 0, 64, 64}; //wdma output region

#define DDP_MOUT_NUM 1
#define DDP_SEL_OUT_NUM 1
#define SLT_LCD_WIDTH_BY_PIXEL 720
#define SLT_LCD_HEIGHT_BY_PIXEL 1280 

//for ovl->wdma0 only
int ddp_mutex_lock(void)
{
	unsigned int cnt = 0;

	// mutex module
	reg_mutex_mod2 = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID_slt));
	// mutex sof
	reg_mutex_sof2 = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID_slt));
	// ovl mout
	reg_mout2 = DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);

	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID_slt), 1);
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID_slt), 0);//single mode
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID_slt), (1<<3)|(1<<6)); //ovl, wdma0	
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID_slt), 0);
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID_slt), 1);
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID_slt),1);
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, DISP_REG_CONFIG_MUTEX_INTSTA & (~(1 << gMutexID_slt)));
	DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN , 0x0d0d);   // enable mutex 0,2,3's intr, other mutex will be used by MDP		

	while(((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID_slt))& DISP_INT_MUTEX_BIT_MASK) != DISP_INT_MUTEX_BIT_MASK))
	{
		cnt++;
		if(cnt>10000)
		{
			printk("[DDP] disp_path_get_mutex() timeout! \n");
			disp_dump_reg(DISP_MODULE_CONFIG);
			break;
		}
	}

	return 0;
}

int ddp_mutex_unlock(int id)
{
	unsigned int cnt = 0;

	DISP_REG_SET(DISP_REG_CONFIG_MUTEX(id), 0);
	while((DISP_REG_CONFIG_MUTEX(id) & DISP_INT_MUTEX_BIT_MASK) != 0)
	{
		cnt++;
		if(cnt>1000*1000*10)
		{
			printk("error: disp_mutex_unlock timeout! \n");
			disp_dump_reg(DISP_MODULE_CONFIG);
			return - 1;
		}
	}
    cnt=0;
	while(DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA)&(1<<id) != (1<<id))
	{
		cnt++;
		if(cnt>10000)
		{
			printk("[DDP] reg update timeout! \n");
			disp_dump_reg(DISP_MODULE_CONFIG);
			disp_dump_reg(DISP_MODULE_MUTEX);			
			break;
		}
	}
	return 0;
}

//for
void ddp_bitblt_wait(void)
{
    WDMAWait(0);
}

int ddp_bitblt_verify(unsigned int dst_addr, unsigned int golden_addr,
                struct REGION_SLT region)
{
	unsigned int diff_cnt = 0;
	unsigned int t=0;
	unsigned int size = region.width*region.height*6/2;
	int result=0;
	for(t=0;t<size;t++)
	{
	    if( *((unsigned char*)dst_addr+t) != *((unsigned char*)golden_addr+t) &&
	    	  *((unsigned char*)dst_addr+t) != *((unsigned char*)data_rgb888_64x64_golden2+t))
	    {
	        diff_cnt++;
	        printk("n=%d, gold=0x%x, real=0x%x, gold2=0x%x \n", t, 
	            *((unsigned char*)golden_addr+t), 
	            *((unsigned char*)dst_addr+t),
	            *((unsigned char*)data_rgb888_64x64_golden2+t));
	    }
	}
	if(diff_cnt == 0)
	{
	    printk("ddp_bitblt_verify, wdma1 output result: success \n");
		// dump reg info for debug
		//disp_dump_reg(DISP_MODULE_CONFIG);
		//disp_dump_reg(DISP_MODULE_MUTEX);
		//disp_dump_reg(DISP_MODULE_OVL);
		//disp_dump_reg(DISP_MODULE_WDMA0);
	    result = 0;
	}
	else
	{
	    if(gSltBitbltVerify==1)
	    {
	    	printk("wdma1 output result: fail \n");
	        printk("detail: : fail, %d, %d, %%%d \n", diff_cnt, size, diff_cnt*100/size);  
			// dump reg info
			disp_dump_reg(DISP_MODULE_CONFIG);
			disp_dump_reg(DISP_MODULE_MUTEX);
			disp_dump_reg(DISP_MODULE_OVL);
			disp_dump_reg(DISP_MODULE_WDMA0);
	    }
		result = -1;
	   
	}   
	
	return result;

}

int m4u_ddp_test(unsigned int srcPa,unsigned int dst,unsigned int dstPa)
{
   	struct REGION_SLT region = {0, 0, 64, 64}; 
	int woutputfmt = eRGB888;//wdma out fmt
	int ovl_in_fmt = eARGB8888;//ovl in fmt
	int bpp =0; 
	int result =0;
	unsigned int layer = 0;
	int i=0;
	
    switch (ovl_in_fmt) 
	{
        case eARGB8888:
            bpp = 4;
            break;
        case eRGB888:
            bpp = 3;
            break;
        case eRGB565:
            bpp = 2;
            break;
        default:
            printk("error\n"); 
			return -1;// invalid input format
    }
   
	DISP_REG_SET(DISP_REG_CONFIG_MMSYS_CG_CLR0,0xffffffff);//clock

	gSltBitbltVerify = 0;//print dump log

    printk("[SLT_DDP] START TEST \n");
	for(i=0;i<2;i++)
	{
	    ddp_mutex_lock();
		// wdma0  config mout/msel to creat a compelte path
		DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1<<1);   
		// config OVL
		OVLStop();
		OVLReset();
		OVLROI(region.width, // width
			  region.height, // height
			  255);// background B

		for(layer=0;layer<4;layer++)
		{
			OVLLayerSwitch(layer, 1);
			OVLLayerConfig(layer,   // layer
						OVL_LAYER_SOURCE_MEM,  // data source (0=memory)
						ovl_in_fmt, 
						srcPa, 	// addr
						region.x, 	// x
						region.y, 	// y
						region.width*bpp,//pitch, pixel number
						region.x, 	// x
						region.y, 	// y
						region.width, // width
						region.height,// height
						0,
						0xff020100,	// color key
						false,		// alpha enable
						0x00);		// alpha		                          
		}				  
		OVLStart();

		// config wdma
		WDMAReset(0);
		WDMAConfig(0, 
				  WDMA_INPUT_FORMAT_ARGB, 
				  region.width, 
				  region.height, 
				  0, 
				  0, 
				  region.width, 
				  region.height, 
				  woutputfmt, 
				  (unsigned int)dstPa, 
				  region.width, 
				  1, 
				  0);	
		if(woutputfmt==eYUV_420_3P)
		{
		   WDMAConfigUV(0, 
			   dstPa+region.width*region.height, 
			   dstPa+region.width*region.height*5/4, 
			   region.width);
		} 
		WDMAStart(0);
		printk("before release mutex dump-------------------------------------------------------------------\n");		
		//disp_dump_reg(DISP_MODULE_CONFIG);
		//disp_dump_reg(DISP_MODULE_MUTEX);
		//disp_dump_reg(DISP_MODULE_OVL);
		//disp_dump_reg(DISP_MODULE_WDMA0); 

		if(ddp_mutex_unlock(gMutexID_slt))
		{
		   result = -1;
		   printk("exit2\n");
		   break;
		}

		printk("after release mutex dump-------------------------------------------------------------------\n");

		ddp_bitblt_wait();
#if 0
		{
			MMP_MetaDataBitmap_t Bitmap;
			Bitmap.data1 = 0;
			Bitmap.data2 = 0;
			Bitmap.width = region.width;
			Bitmap.height =region.height;
			Bitmap.format = MMProfileBitmapRGB888;
			Bitmap.start_pos = 0;
			Bitmap.bpp = 24;
			Bitmap.pitch = 64*3;
			Bitmap.data_size = Bitmap.pitch * Bitmap.height;
			Bitmap.down_sample_x = 1;
			Bitmap.down_sample_y = 1;
			Bitmap.pData = pDst;
			MMProfileLogMetaBitmap(DDP_MMP_Events.Debug2, MMProfileFlagPulse, &Bitmap);
		}
#endif		
		printk("Verify start i %d\n",i);
			
		result = ddp_bitblt_verify(dst, (unsigned int)slt_data_rgb888_64x64_golden, region);

		gSltBitbltVerify=i+1;
		
		//disable path 
		//mutex
		DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID_slt), reg_mutex_mod2);
		DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID_slt), reg_mutex_sof2);		  
		// ovl mout
		DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, reg_mout2);	
		
        if(result == 0)
        {
			break;
       	}
	}
	
	return result;
}


//ovl->wdma0 64*64 argb8888 -> rgb888
int slt_test_stream1(void)
{     
    int result = 0;
	int i=0;
	unsigned int layer = 0;
	unsigned int* pSrc =0;
	unsigned char* pSrcPa =0;
	unsigned int* pDst =0;
	unsigned char* pDstPa =0;
	
	struct REGION_SLT region = {0, 0, 64, 64}; 
    int woutputfmt = eRGB888;//wdma out fmt
	int ovl_in_fmt = eARGB8888;//ovl in fmt
	int bpp =0;
	pSrc= dma_alloc_coherent(NULL, 64*64*4, (dma_addr_t *)&pSrcPa, GFP_KERNEL);
	if(pSrc==0 || pSrcPa==0)
	{
		printk("dma_alloc_coherent error!  dma memory not available.\n");
		return -1;
	}
	else
	{
		printk("[ddp] pSrc=0x%x, pSrcPa=0x%x \n", (unsigned int)pSrc, (unsigned int)pSrcPa);
	}
	memset((void*)pSrc,0,64*64*4);
	memcpy((void*)pSrc, data_argb_64x64, 64*64*4);

	pDst= dma_alloc_coherent(NULL, 64*64*4, (dma_addr_t *)&pDstPa, GFP_KERNEL);//size >= 64*64*3
	if(pDst==0 || pDstPa==0)
	{
		printk("dma_alloc_coherent error!  dma memory not available.\n");
		return -1;
	}
	else
	{
		printk("[ddp] pDst=0x%x, pDstPa=0x%x \n",(unsigned int) pDst, (unsigned int)pDstPa);
	}
	memset((void*)pDst, 0, 64*64*4);
    
#if 1
	// config port to physical
	{
		M4U_PORT_STRUCT sPort;
		sPort.ePortID = DISP_OVL_0;
		sPort.Virtuality = 0;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
		
		sPort.ePortID = DISP_WDMA;
		sPort.Virtuality = 0;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
	  }
#endif

    switch (ovl_in_fmt) 
	{
        case eARGB8888:
            bpp = 4;
            break;
        case eRGB888:
            bpp = 3;
            break;
        case eRGB565:
            bpp = 2;
            break;
        default:
            printk("error\n"); 
			return -1;// invalid input format
    }
   
	DISP_REG_SET(DISP_REG_CONFIG_MMSYS_CG_CLR0,0xffffffff);//clock

	gSltBitbltVerify = 0;//print dump log

    printk("[SLT_DDP] START TEST \n");
	for(i=0;i<2;i++)
	{
	    ddp_mutex_lock();
		// wdma0  config mout/msel to creat a compelte path
		DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1<<1);   
		// config OVL
		OVLStop();
		OVLReset();
		OVLROI(region.width, // width
			  region.height, // height
			  255);// background B

		for(layer=0;layer<4;layer++)
		{
			OVLLayerSwitch(layer, 1);
			OVLLayerConfig(layer,   // layer
						OVL_LAYER_SOURCE_MEM,  // data source (0=memory)
						ovl_in_fmt, 
						(unsigned int)pSrcPa, 	// addr
						region.x, 	// x
						region.y, 	// y
						region.width*bpp,//pitch, pixel number
						region.x, 	// x
						region.y, 	// y
						region.width, // width
						region.height,// height
						0,
						0xff020100,	// color key
						false,		// alpha enable
						0x00);		// alpha		                          
		}				  
		OVLStart();

		// config wdma
		WDMAReset(0);
		WDMAConfig(0, 
				  WDMA_INPUT_FORMAT_ARGB, 
				  region.width, 
				  region.height, 
				  0, 
				  0, 
				  region.width, 
				  region.height, 
				  woutputfmt, 
				  (unsigned int)pDstPa, 
				  region.width, 
				  1, 
				  0);	
		if(woutputfmt==eYUV_420_3P)
		{
		   WDMAConfigUV(0, 
			   (unsigned int)(pDstPa+region.width*region.height), 
			   (unsigned int)(pDstPa+region.width*region.height*5/4), 
			   region.width);
		} 
		WDMAStart(0);
		//printk("before release mutex dump-------------------------------------------------------------------\n");		
		//disp_dump_reg(DISP_MODULE_CONFIG);
		//disp_dump_reg(DISP_MODULE_MUTEX);
		//disp_dump_reg(DISP_MODULE_OVL);
		//disp_dump_reg(DISP_MODULE_WDMA0); 

		if(ddp_mutex_unlock(gMutexID_slt))
		{
		   result = -1;
		   printk("exit2\n");
		   break;
		}

		//printk("after release mutex dump-------------------------------------------------------------------\n");

		ddp_bitblt_wait();
#if 0
		{
			MMP_MetaDataBitmap_t Bitmap;
			Bitmap.data1 = 0;
			Bitmap.data2 = 0;
			Bitmap.width = region.width;
			Bitmap.height =region.height;
			Bitmap.format = MMProfileBitmapRGB888;
			Bitmap.start_pos = 0;
			Bitmap.bpp = 24;
			Bitmap.pitch = 64*3;
			Bitmap.data_size = Bitmap.pitch * Bitmap.height;
			Bitmap.down_sample_x = 1;
			Bitmap.down_sample_y = 1;
			Bitmap.pData = pDst;
			MMProfileLogMetaBitmap(DDP_MMP_Events.Debug2, MMProfileFlagPulse, &Bitmap);
		}
#endif		
		//printk("Verify start i %d\n",i);
			
		result = ddp_bitblt_verify((unsigned int)pDst, slt_data_rgb888_64x64_golden, region);

		gSltBitbltVerify=i+1;
		
		//disable path 
		//mutex
		DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID_slt), reg_mutex_mod2);
		DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID_slt), reg_mutex_sof2);		  
		// ovl mout
		DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, reg_mout2);	
		
        if(result == 0)
        {
			break;
       	}
	}
	
	if(i==2)
	{
	   printk("result: fail, 2nd bitblt fail!!\n");
	}

	//dealloc memory
	if(pSrc!=0 && pSrcPa!=0)
	{
		dma_free_coherent(NULL, 64*64*4, pSrc, (dma_addr_t)&pSrcPa);
	}
	if(pDst!=0 && pDstPa!=0)
	{
		dma_free_coherent(NULL, 64*64*4, pDst, (dma_addr_t)&pDstPa);   
	}
	
#if 1
	// config port to physical
	{
		M4U_PORT_STRUCT sPort;
		sPort.ePortID = DISP_OVL_0;
		sPort.Virtuality = 1;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
		
		sPort.ePortID = DISP_WDMA;
		sPort.Virtuality = 1;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
	}
#endif

	return result;
}


int slt_test_stream2(void)
{
	int result = 0;
	unsigned int* pSrc =0;
	unsigned char* pSrcPa =0;
	unsigned int* pDst =0;
	unsigned char* pDstPa =0;
	printk("slt_test_stream2\n");
	pSrc= dma_alloc_coherent(NULL, 64*64*4, (dma_addr_t *)&pSrcPa, GFP_KERNEL);
	if(pSrc==0 || pSrcPa==0)
	{
		printk("dma_alloc_coherent error!  dma memory not available.\n");
		return -1;
	}
	else
	{
		printk("[ddp] pSrc=0x%x, pSrcPa=0x%x \n", (unsigned int)pSrc, (unsigned int)pSrcPa);
	}
	memset((void*)pSrc,0,64*64*4);
	memcpy((void*)pSrc, data_argb_64x64, 64*64*4);

	pDst= dma_alloc_coherent(NULL, 64*64*4, (dma_addr_t *)&pDstPa, GFP_KERNEL);//size >= 64*64*3
	if(pDst==0 || pDstPa==0)
	{
		printk("dma_alloc_coherent error!  dma memory not available.\n");
		return -1;
	}
	else
	{
		printk("[ddp] pDst=0x%x, pDstPa=0x%x \n",(unsigned int) pDst, (unsigned int)pDstPa);
	}
	memset((void*)pDst, 0, 64*64*4);
	
#if 1
	// config port to physical
	{
		M4U_PORT_STRUCT sPort;
		sPort.ePortID = DISP_OVL_0;
		sPort.Virtuality = 0;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
		
		sPort.ePortID = DISP_WDMA;
		sPort.Virtuality = 0;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
	 }
#endif

	result = m4u_ddp_test((unsigned int)pSrcPa,(unsigned int)pDst,(unsigned int)pDstPa);

	if(result!=0)
	{
	   printk("result: fail, 2nd bitblt fail!!\n");
	}

	//dealloc memory
	if(pSrc!=0 && pSrcPa!=0)
	{
		dma_free_coherent(NULL, 64*64*4, pSrc, (dma_addr_t)&pSrcPa);
	}
	if(pDst!=0 && pDstPa!=0)
	{
		dma_free_coherent(NULL, 64*64*4, pDst, (dma_addr_t)&pDstPa);   
	}
		
#if 1
	// config port to physical
	{
		M4U_PORT_STRUCT sPort;
		sPort.ePortID = DISP_OVL_0;
		sPort.Virtuality = 1;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
		
		sPort.ePortID = DISP_WDMA;
		sPort.Virtuality = 1;					   
		sPort.Security = 0;
		sPort.Distance = 1;
		sPort.Direction = 0;
		m4u_config_port(&sPort);
	}
#endif

	return result;

}




