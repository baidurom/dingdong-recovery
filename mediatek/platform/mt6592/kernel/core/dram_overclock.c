#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <asm/cacheflush.h>
#include <asm/outercache.h>
#include <asm/system.h>
#include <asm/delay.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_freqhopping.h>
//#include <mach/emi_bwl.h>
#include <mach/mt_typedefs.h>
#include <mach/memory.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <mach/dfe_drv.h>
#include <mach/mt6333.h>
#include <mach/mt_clkmgr.h>
#include <mach/dma.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include "mach/sync_write.h"
#include <mach/dma.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <asm/sizes.h>
#define MEM_TEST_SIZE 0x2000
//#define Voltage_Debug
#define DRAMC_READ_REG(offset)         ( \
                                        (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) |\
                                        (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) |\
                                        (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) \
                                        )   

enum
{
  LPDDR2 = 0,
  DDR3_16,
  DDR3_32,
  LPDDR3,
  mDDR,
};



#define Vcore_HV_1066  (0x40)   //1.1
#define Vcore_NV_1066  (0x30)   //1.0
#define Vcore_LV_1066  (0x20)  //0.9

#define Vmem_HV_1066 (0x60) //1.3
#define Vmem_NV_1066 (0x54) //1.22
#define Vmem_LV_1066 (0x47) //1.14

#define Vcore_HV_1333  (0x50)   //1.2
#define Vcore_NV_1333  (0x44)   //1.125
#define Vcore_LV_1333  (0x30)   //1.0

#define Vmem_HV_1333 (0x60) //1.3
#define Vmem_NV_1333 (0x54) //1.22
#define Vmem_LV_1333 (0x4A) //1.16


extern unsigned int get_ddr_type(void);
extern unsigned int DMA_TIMES_RECORDER;
extern unsigned int get_max_DRAM_size(void);
#if 1
#define DRAMC_WRITE_REG(val,offset)     do{ \
                                      (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) = (unsigned int)(val); \
                                      }while(0)

static int dram_clk;
static DEFINE_SPINLOCK(lock);

__attribute__ ((__section__ (".sram.func"))) int sram_set_dram(int clk)
{
   DRAMC_WRITE_REG(DRAMC_READ_REG(0x04) | (0x1<<26), 0x04);
   while ( (DRAMC_READ_REG(0x3b8) & (0x01<<16))==0);

    /* set ac timing */
    if(clk == 293) {
        DRAMC_WRITE_REG( 0x447844A4    , 0x0  );
        DRAMC_WRITE_REG( 0xA00642D1     , 0x7C );
        DRAMC_WRITE_REG( 0xBF0B0401     , 0x44 );
        DRAMC_WRITE_REG( 0x03406346     , 0x8  );
        DRAMC_WRITE_REG( 0xD1642742     , 0x1DC);
        DRAMC_WRITE_REG( 0x01000710     , 0x1E8);
        DRAMC_WRITE_REG( 0x000002F5     , 0x1f8 );
    }
    if(clk == 367)
     {
        DRAMC_WRITE_REG( 0x66AB46F7    , 0x0  );
        DRAMC_WRITE_REG( 0xF00487C3     , 0x4 );
        DRAMC_WRITE_REG( 0xBF020401     , 0x44 );
        DRAMC_WRITE_REG( 0x03406358     , 0x8  );
        DRAMC_WRITE_REG( 0xD1644E42     , 0x1DC);
        DRAMC_WRITE_REG( 0x11001640     , 0x1E8);
        DRAMC_WRITE_REG( 0x04002605     , 0x1f8 );
     }

    DRAMC_WRITE_REG(DRAMC_READ_REG(0x04) & ~(0x1<<26), 0x04);
    while ( (DRAMC_READ_REG(0x3b8) & (0x01<<16))==1);
    return 0;
}
static void enable_gpu(void)
{
/*
    enable_clock(MT_CG_MFG_HYD, "MFG");
    enable_clock(MT_CG_MFG_G3D, "MFG");
    enable_clock(MT_CG_MFG_MEM, "MFG");
    enable_clock(MT_CG_MFG_AXI, "MFG");
*/
}
static void disable_gpu(void)
{
/*
    disable_clock(MT_CG_MFG_AXI, "MFG");
    disable_clock(MT_CG_MFG_MEM, "MFG");
    disable_clock(MT_CG_MFG_G3D, "MFG");
    disable_clock(MT_CG_MFG_HYD, "MFG");
*/
}
static ssize_t dram_overclock_show(struct device_driver *driver, char *buf)
{
      extern unsigned int get_ddr_type(void)__attribute__((weak));
      int dram_type;
      if(get_ddr_type)
      {
        dram_type=get_ddr_type();
        if(dram_type==LPDDR2)
        {
          dram_type=2;
        }
        else if(dram_type==LPDDR3)
        {
          dram_type=3;
        }
        else if(dram_type==DDR3_16)
        {
          dram_type=4;
        }
        else if(dram_type==DDR3_32)
        {
          dram_type=5;
        }
        else
        {
            dram_type=-1;
            printk("We do not support this DRAM Type\n");
            ASSERT(0);
        }
        return snprintf(buf, PAGE_SIZE, "%d,%d\n",dram_type, mt_fh_get_dramc());
      }
      else
      {
        return  snprintf(buf,PAGE_SIZE,"do not support dram_overclock show\n");
        ASSERT(0);
      }
}
static ssize_t dram_overclock_store(struct device_driver *driver, const char *buf, size_t count)
{
    int clk, ret = 0;
    extern unsigned int get_ddr_type(void)__attribute__((weak));
    clk = simple_strtol(buf, 0, 10);
    dram_clk = mt_fh_get_dramc();
    if(clk == dram_clk) {
        printk(KERN_ERR "dram_clk:%d, is equal to user inpu clk:%d\n", dram_clk, clk);
        return count;
    }
    spin_lock(&lock);
    ret = sram_set_dram(clk);
    if(ret < 0)
        printk(KERN_ERR "dram overclock in sram failed:%d, clk:%d\n", ret, clk);
    spin_unlock(&lock);
    if(get_ddr_type){
      if(get_ddr_type()==LPDDR3)
      {
          mt6333_config_interface(0x6B, Vcore_NV_1333, 0x7F,0);  //1.1
          mt6333_config_interface(0x6C, Vcore_NV_1333, 0x7F,0);
      }
      else
      { 
          mt6333_config_interface(0x6B, Vcore_NV_1066, 0x7F,0);  //1.1
          mt6333_config_interface(0x6C, Vcore_NV_1066, 0x7F,0);
      }
    }
    else
    {
       printk("do not support get_ddr_type\n");
       ASSERT(0);
    }
    ret = mt_fh_dram_overclock(clk);
    if(ret < 0)
        printk(KERN_ERR "dram overclock failed:%d, clk:%d\n", ret, clk);
    printk(KERN_INFO "In %s pass, dram_clk:%d, clk:%d\n", __func__, dram_clk, clk);
    return count;

}
extern unsigned int RESERVED_MEM_SIZE_FOR_TEST_3D;
extern unsigned int FB_SIZE_EXTERN;
extern unsigned int get_max_DRAM_size (void);
static ssize_t ftm_dram_3d_show(struct device_driver *driver, char *buf)
{
    unsigned int pa_3d_base = PHYS_OFFSET + get_max_DRAM_size() - RESERVED_MEM_SIZE_FOR_TEST_3D - FB_SIZE_EXTERN;
    return snprintf(buf, PAGE_SIZE, "%u\n", pa_3d_base);
}
static ssize_t ftm_dram_3d_store(struct device_driver *driver, const char *buf, size_t count)
{
    return count;
}
static ssize_t ftm_dram_mtcmos_show(struct device_driver *driver, char *buf)
{
    return 0;
}
static ssize_t ftm_dram_mtcmos_store(struct device_driver *driver, const char *buf, size_t count)
{
    int enable;
    enable = simple_strtol(buf, 0, 10);
    if(enable == 1) {
        enable_gpu();
        printk(KERN_INFO "enable in %s, enable:%d\n", __func__, enable);
    } else if(enable == 0) {
        disable_gpu();
        printk(KERN_INFO "enable in %s, disable:%d\n", __func__, enable);
    } else
        printk(KERN_ERR "dram overclock failed:%s, enable:%d\n", __func__, enable);
    return count;
}

#define PATTERN1 0x5A5A5A5A
#define PATTERN2 0xA5A5A5A5
int Binning_DRAM_complex_mem_test (void)
{
    unsigned char *MEM8_BASE;
    unsigned short *MEM16_BASE;
    unsigned int *MEM32_BASE;
    unsigned int *MEM_BASE;
    unsigned char pattern8;
    unsigned short pattern16;
    unsigned int i, j, size, pattern32;
    unsigned int value;
    unsigned int len=MEM_TEST_SIZE;
    void *ptr;   
    ptr = vmalloc(PAGE_SIZE*2);
    MEM8_BASE=(unsigned char *)ptr;
    MEM16_BASE=(unsigned short *)ptr;
    MEM32_BASE=(unsigned int *)ptr;
    MEM_BASE=(unsigned int *)ptr;
    printk("Test DRAM start address 0x%x\n",(unsigned int)ptr);
    printk("Test DRAM SIZE 0x%x\n",MEM_TEST_SIZE);
    size = len >> 2;

    /* === Verify the tied bits (tied high) === */
    for (i = 0; i < size; i++)
    {
        MEM32_BASE[i] = 0;
    }

    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0)
        {
            vfree(ptr);
            return -1;
        }
        else
        {
            MEM32_BASE[i] = 0xffffffff;
        }
    }

    /* === Verify the tied bits (tied low) === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xffffffff)
        {
            vfree(ptr);
            return -2;
        }
        else
            MEM32_BASE[i] = 0x00;
    }

    /* === Verify pattern 1 (0x00~0xff) === */
    pattern8 = 0x00;
    for (i = 0; i < len; i++)
        MEM8_BASE[i] = pattern8++;
    pattern8 = 0x00;
    for (i = 0; i < len; i++)
    {
        if (MEM8_BASE[i] != pattern8++)
        { 
            vfree(ptr);
            return -3;
        }
    }

    /* === Verify pattern 2 (0x00~0xff) === */
    pattern8 = 0x00;
    for (i = j = 0; i < len; i += 2, j++)
    {
        if (MEM8_BASE[i] == pattern8)
            MEM16_BASE[j] = pattern8;
        if (MEM16_BASE[j] != pattern8)
        {
            vfree(ptr);
            return -4;
        }
        pattern8 += 2;
    }

    /* === Verify pattern 3 (0x00~0xffff) === */
    pattern16 = 0x00;
    for (i = 0; i < (len >> 1); i++)
        MEM16_BASE[i] = pattern16++;
    pattern16 = 0x00;
    for (i = 0; i < (len >> 1); i++)
    {
        if (MEM16_BASE[i] != pattern16++)                                                                                                    
        {
            vfree(ptr);
            return -5;
        }
    }

    /* === Verify pattern 4 (0x00~0xffffffff) === */
    pattern32 = 0x00;
    for (i = 0; i < (len >> 2); i++)
        MEM32_BASE[i] = pattern32++;
    pattern32 = 0x00;
    for (i = 0; i < (len >> 2); i++)
    {
        if (MEM32_BASE[i] != pattern32++)
        { 
            vfree(ptr);
            return -6;
        }
    }

    /* === Pattern 5: Filling memory range with 0x44332211 === */
    for (i = 0; i < size; i++)
        MEM32_BASE[i] = 0x44332211;

    /* === Read Check then Fill Memory with a5a5a5a5 Pattern === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0x44332211)
        {
            vfree(ptr);
            return -7;
        }
        else
        {
            MEM32_BASE[i] = 0xa5a5a5a5;
        }
    }

    /* === Read Check then Fill Memory with 00 Byte Pattern at offset 0h === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xa5a5a5a5)
        { 
            vfree(ptr);
            return -8;  
        }
        else                                                                                                                              
        {
            MEM8_BASE[i * 4] = 0x00;
        }
    }

    /* === Read Check then Fill Memory with 00 Byte Pattern at offset 2h === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xa5a5a500)
        {
            vfree(ptr);
            return -9;
        }
        else
        {
            MEM8_BASE[i * 4 + 2] = 0x00;
        }
    }

    /* === Read Check then Fill Memory with 00 Byte Pattern at offset 1h === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xa500a500)
        {
            vfree(ptr);
            return -10;
        }
        else
        {
            MEM8_BASE[i * 4 + 1] = 0x00;
        }
    }

    /* === Read Check then Fill Memory with 00 Byte Pattern at offset 3h === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xa5000000)
        {
            vfree(ptr);
            return -11;
        }
        else
        {
            MEM8_BASE[i * 4 + 3] = 0x00;
        }
    }

    /* === Read Check then Fill Memory with ffff Word Pattern at offset 1h == */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0x00000000)
        {
            vfree(ptr);
            return -12;
        }
        else
        {
            MEM16_BASE[i * 2 + 1] = 0xffff;
        }
    }


    /* === Read Check then Fill Memory with ffff Word Pattern at offset 0h == */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xffff0000)
        {
            vfree(ptr);
            return -13;
        }
        else
        {
            MEM16_BASE[i * 2] = 0xffff;
        }
    }
    /*===  Read Check === */
    for (i = 0; i < size; i++)
    {
        if (MEM32_BASE[i] != 0xffffffff)
        {
            vfree(ptr);
            return -14;
        }
    }


    /************************************************
    * Additional verification
    ************************************************/
    /* === stage 1 => write 0 === */

    for (i = 0; i < size; i++)
    {
        MEM_BASE[i] = PATTERN1;
    }


    /* === stage 2 => read 0, write 0xF === */
    for (i = 0; i < size; i++)
    {
        value = MEM_BASE[i];

        if (value != PATTERN1)
        {
            vfree(ptr);
            return -15;
        }
        MEM_BASE[i] = PATTERN2;
    }


    /* === stage 3 => read 0xF, write 0 === */
    for (i = 0; i < size; i++)
    {
        value = MEM_BASE[i];
        if (value != PATTERN2)
        {
            vfree(ptr);
            return -16;
        }
        MEM_BASE[i] = PATTERN1;
    }


    /* === stage 4 => read 0, write 0xF === */
    for (i = 0; i < size; i++)
    {
        value = MEM_BASE[i];
        if (value != PATTERN1)
        {
            vfree(ptr);
            return -17;
        }
        MEM_BASE[i] = PATTERN2;
    }


    /* === stage 5 => read 0xF, write 0 === */
    for (i = 0; i < size; i++)
    {
        value = MEM_BASE[i];
        if (value != PATTERN2)
        { 
            vfree(ptr);
            return -18;
        }
        MEM_BASE[i] = PATTERN1;
    }


    /* === stage 6 => read 0 === */
    for (i = 0; i < size; i++)
    {
        value = MEM_BASE[i];
        if (value != PATTERN1)
        {
            vfree(ptr);
            return -19;
        }
    }


    /* === 1/2/4-byte combination test === */
    i = (unsigned int) MEM_BASE;
    while (i < (unsigned int) MEM_BASE + (size << 2))
    {
        *((unsigned char *) i) = 0x78;
        i += 1;
        *((unsigned char *) i) = 0x56;
        i += 1;
        *((unsigned short *) i) = 0x1234;
        i += 2;
        *((unsigned int *) i) = 0x12345678;
        i += 4;
        *((unsigned short *) i) = 0x5678;
        i += 2;
        *((unsigned char *) i) = 0x34;
        i += 1;
        *((unsigned char *) i) = 0x12;
        i += 1;
        *((unsigned int *) i) = 0x12345678;
        i += 4;
        *((unsigned char *) i) = 0x78;
        i += 1;
        *((unsigned char *) i) = 0x56;
        i += 1;
        *((unsigned short *) i) = 0x1234;
        i += 2;
        *((unsigned int *) i) = 0x12345678;
        i += 4;
        *((unsigned short *) i) = 0x5678;
        i += 2;
        *((unsigned char *) i) = 0x34;
        i += 1;
        *((unsigned char *) i) = 0x12;
        i += 1;
        *((unsigned int *) i) = 0x12345678;
        i += 4;
    }
    for (i = 0; i < size; i++)
    {
        value = MEM_BASE[i];
        if (value != 0x12345678)
        {
            vfree(ptr);
            return -20;
        }
    }


    /* === Verify pattern 1 (0x00~0xff) === */
    pattern8 = 0x00;
    MEM8_BASE[0] = pattern8;
    for (i = 0; i < size * 4; i++)
    {
        unsigned char waddr8, raddr8;
        waddr8 = i + 1;
        raddr8 = i;
        if (i < size * 4 - 1)
            MEM8_BASE[waddr8] = pattern8 + 1;
        if (MEM8_BASE[raddr8] != pattern8)
        {
            vfree(ptr);
            return -21;
        }
        pattern8++;
    }


    /* === Verify pattern 2 (0x00~0xffff) === */
    pattern16 = 0x00;
    MEM16_BASE[0] = pattern16;
    for (i = 0; i < size * 2; i++)
    {
        if (i < size * 2 - 1)
            MEM16_BASE[i + 1] = pattern16 + 1;
        if (MEM16_BASE[i] != pattern16)
        {
            vfree(ptr);
            return -22;
        }
        pattern16++;
    }
    /* === Verify pattern 3 (0x00~0xffffffff) === */
    pattern32 = 0x00;
    MEM32_BASE[0] = pattern32;
    for (i = 0; i < size; i++)
    {
        if (i < size - 1)
            MEM32_BASE[i + 1] = pattern32 + 1;
        if (MEM32_BASE[i] != pattern32)
        {
            vfree(ptr);
            return -23;
        }
        pattern32++;
    }
    printk("complex R/W mem test pass\n");
    vfree(ptr);
    return 1;
}

static ssize_t complex_mem_test_show(struct device_driver *driver, char *buf)
{
    int ret;
    ret=Binning_DRAM_complex_mem_test();
    if(ret>0)
    {
      return snprintf(buf, PAGE_SIZE, "MEM Test all pass\n");
    }
    else
    {
      return snprintf(buf, PAGE_SIZE, "MEM TEST failed %d \n", ret);
    }
}
static ssize_t complex_mem_test__store(struct device_driver *driver, const char *buf, size_t count)
{
    return count;
}



int DDR_1333=1;
int V_cond=1;
static ssize_t DFS_test_show(struct device_driver *driver, char *buf)
{
    int ret;
    int rank0_fine;
    int rank1_fine;
    if(DDR_1333==1)
    {
      
        ret=mt_fh_dram_overclock(266);
        if(ret<0)
        {
        return snprintf(buf, PAGE_SIZE, "hopping failed\n");
        }
        DDR_1333=0;        
        rank0_fine=DRAMC_READ_REG(0x374);
        rank1_fine=DRAMC_READ_REG(0x378);
       if(V_cond==2)
       {
        mt6333_config_interface(0x6B, Vcore_HV_1066, 0x7F,0);   //1.1
        mt6333_config_interface(0x6C, Vcore_HV_1066, 0x7F,0);   //1.1
        mt6333_config_interface(0x81, Vmem_HV_1066, 0x7F,0);
        mt6333_config_interface(0x82, Vmem_HV_1066, 0x7F,0);  //1.325

       }
       else if (V_cond==0)
       {
        mt6333_config_interface(0x6B, Vcore_LV_1066, 0x7F,0);  //0.9
        mt6333_config_interface(0x6C, Vcore_LV_1066, 0x7F,0);  
        mt6333_config_interface(0x81, Vmem_LV_1066, 0x7F,0); //1.14
        mt6333_config_interface(0x82, Vmem_LV_1066, 0x7F,0);

       }
       else if (V_cond==3)  //HVc_LVm
       {
        mt6333_config_interface(0x6B, Vcore_HV_1066, 0x7F,0);  //1.0
        mt6333_config_interface(0x6C, Vcore_HV_1066, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_LV_1066, 0x7F,0); //1.16
        mt6333_config_interface(0x82, Vmem_LV_1066, 0x7F,0);
       }
       else if (V_cond==4) //LVc_HVm
       {
        mt6333_config_interface(0x6B, Vcore_LV_1066, 0x7F,0);  //0.9
        mt6333_config_interface(0x6C, Vcore_LV_1066, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_HV_1066, 0x7F,0); //1.3
        mt6333_config_interface(0x82, Vmem_HV_1066, 0x7F,0);
       }
       else
       {
        mt6333_config_interface(0x6B, Vcore_NV_1066, 0x7F,0); //1.0
        mt6333_config_interface(0x6C, Vcore_NV_1066, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_NV_1066, 0x7F,0); //1.22
        mt6333_config_interface(0x82, Vmem_NV_1066, 0x7F,0);
       }
        return snprintf(buf, PAGE_SIZE, "hopping to 1066, rank0_fine=0x%x,rank1_fine=0x%x DFS_DMA_Times=%d\n",rank0_fine,rank1_fine,DMA_TIMES_RECORDER);
    }
    else
    { 

       if(V_cond==2)
       {
        mt6333_config_interface(0x6B, Vcore_HV_1333, 0x7F,0);   //1.2
        mt6333_config_interface(0x6C, Vcore_HV_1333, 0x7F,0);   //1.2
        mt6333_config_interface(0x81, Vmem_HV_1333, 0x7F,0); 
        mt6333_config_interface(0x82, Vmem_HV_1333, 0x7F,0);  //1.325
       }
       else if (V_cond==0)
       {
        mt6333_config_interface(0x6B, Vcore_LV_1333, 0x7F,0);  //1.0
        mt6333_config_interface(0x6C, Vcore_LV_1333, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_LV_1333, 0x7F,0); //1.16
        mt6333_config_interface(0x82, Vmem_LV_1333, 0x7F,0);
       }

       else if (V_cond==3)  //HVc_LVm
       {
        mt6333_config_interface(0x6B, Vcore_HV_1333, 0x7F,0);  //1.2
        mt6333_config_interface(0x6C, Vcore_HV_1333, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_LV_1333, 0x7F,0); //1.16
        mt6333_config_interface(0x82, Vmem_LV_1333, 0x7F,0);
       }
       else if (V_cond==4) //LVc_HVm
       {
        mt6333_config_interface(0x6B, Vcore_LV_1333, 0x7F,0);  //1.0
        mt6333_config_interface(0x6C, Vcore_LV_1333, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_HV_1333, 0x7F,0); //1.3
        mt6333_config_interface(0x82, Vmem_HV_1333, 0x7F,0);
       }
       else
       {
        mt6333_config_interface(0x6B, Vcore_NV_1333, 0x7F,0);  //1.1
        mt6333_config_interface(0x6C, Vcore_NV_1333, 0x7F,0);
        mt6333_config_interface(0x81, Vmem_NV_1333, 0x7F,0); //1.22
        mt6333_config_interface(0x82, Vmem_NV_1333, 0x7F,0);
       }
        ret=mt_fh_dram_overclock(333);
        if(ret<0)
        {
        return snprintf(buf, PAGE_SIZE, "hopping failed\n");
        }
        DDR_1333=1;
        rank0_fine=DRAMC_READ_REG(0x374);
        rank1_fine=DRAMC_READ_REG(0x378);
        return snprintf(buf, PAGE_SIZE, "hopping to 1333, rank0_fine=0x%x,rank1_fine=0x%x DFS_DMA_Times %d\n",rank0_fine,rank1_fine,DMA_TIMES_RECORDER);
    }
}

/*
   0:LV
   1:NV
   2:HV
   3:HVc_LVm
   4:LVc_HVm
*/
static ssize_t DFS_test_store(struct device_driver *driver, const char *buf, size_t count)
{
    int status;
    status = simple_strtol(buf, 0, 10);
    if (status==4)
    {
      V_cond=4;
    }
    else if(status==3)
    {
      V_cond=3;
    }
    else if(status==2)
    {
      V_cond=2;
    }
    else if(status==0)
    {
      V_cond=0;
    }
    else
    {
      V_cond=1;
    }
    printk("V_cond 0x%x\n",V_cond);
    return count;
}
extern int DFS_Detection(void);
static ssize_t DFS_status_show(struct device_driver *driver, char *buf)
{
    int ret;
    int coarse_v_e0;
    int coarse_v_124;
    ret=DFS_Detection();
    coarse_v_e0=DRAMC_READ_REG(0xe0);
    coarse_v_124=DRAMC_READ_REG(0x124);
  if(ret>0)
    {
      return snprintf(buf, PAGE_SIZE, " DFS could be enable, 0xE0=0x%x,0x124=0x%x \n",coarse_v_e0,coarse_v_124);
    }
    else
    {
      return snprintf(buf, PAGE_SIZE, "DFS could not be enable, 0xE0=0x%x,0x124=0x%x \n",coarse_v_e0,coarse_v_124);
    }
}
static ssize_t DFS_status_store(struct device_driver *driver, const char *buf, size_t count)
{
    return count;
}
#if 0
int irq_dma_handler(void)
{
  return 0;
}
#endif
int DFS_APDMA_Enable(void);
int DFS_APDMA_Init(void);
int DFS_APDMA_early_init(void);
#ifdef APDMA_TEST
static ssize_t DFS_APDMA_TEST_show(struct device_driver *driver, char *buf)
{
    int ret;
    printk("[Before  enable_clock]addr:0x%x, value:%x\n",(0xF0003018),*((volatile int *)0xF0003018));
    enable_clock(MT_CG_PERI_AP_DMA,"APDMA_MODULE");
    printk("[after  enable_clock]addr:0x%x, value:%x\n",(0xF0003018),*((volatile int *)0xF0003018));
    DFS_APDMA_early_init();
    ret=DFS_APDMA_Init();
    ret=DFS_APDMA_Enable();
    disable_clock(MT_CG_PERI_AP_DMA,"APDMA_MODULE");

    return snprintf(buf, PAGE_SIZE, "DFS APDMA ret 0x%x \n",ret);
}
static ssize_t DFS_APDMA_TEST_store(struct device_driver *driver, const char *buf, size_t count)
{
    return count;
}
#endif


#ifdef Voltage_Debug
static ssize_t DFS_Voltage_show(struct device_driver *driver, char *buf)
{
    int ret_val = 0;
    unsigned int OldVcore1 = 0;
    unsigned int OldVcore2 = 0;
    unsigned int OldVmem1 = 0;
    unsigned int OldVmem2 = 0;

    printk("[PMIC]pmic_voltage_read : \r\n");
    ret_val=mt6333_read_interface(0x6B,&OldVcore1,0x7F,0);
    ret_val=mt6333_read_interface(0x81,&OldVmem1, 0x7F,0);
    printk("[Vcore]0x6B=0x%x,\r\n[Vmem] 0x81=0x%x \r\n", OldVcore1,OldVmem1);
    return snprintf(buf, PAGE_SIZE,"[Vcore]0x6B=0x%x,\r\n[Vmem] 0x81=0x%x \r\n", OldVcore1,OldVmem1);
}
static ssize_t DFS_Voltage_store(struct device_driver *driver, const char *buf, size_t count)
{
    int status;
    int vcore;
    int vmem;
    int voltage;
    voltage = simple_strtol(buf, 0, 16);
    vcore= voltage >> 8;
    vmem = voltage & 0xFF;
    mt6333_config_interface(0x6B, vcore, 0x7F,0);
    mt6333_config_interface(0x6C, vcore, 0x7F,0);
    mt6333_config_interface(0x81,vmem, 0x7F,0);
    mt6333_config_interface(0x82,vmem, 0x7F,0);
    return count;
}
#endif

DRIVER_ATTR(emi_clk_test, 0664, dram_overclock_show, dram_overclock_store);
DRIVER_ATTR(emi_clk_3d_test, 0664, ftm_dram_3d_show, ftm_dram_3d_store);
DRIVER_ATTR(emi_clk_mtcmos_test, 0664, ftm_dram_mtcmos_show, ftm_dram_mtcmos_store);
DRIVER_ATTR(emi_clk_mem_test, 0664, complex_mem_test_show, complex_mem_test__store);
DRIVER_ATTR(emi_DFS_test, 0664, DFS_test_show, DFS_test_store);
DRIVER_ATTR(emi_DFS_status, 0664, DFS_status_show, DFS_status_store);
#ifdef APDMA_TEST
DRIVER_ATTR(emi_DFS_APDMA_test, 0664, DFS_APDMA_TEST_show, DFS_APDMA_TEST_store);
#endif
#ifdef Voltage_Debug
DRIVER_ATTR(emi_DRAM_Voltage, 0664, DFS_Voltage_show,DFS_Voltage_store);
#endif
static struct device_driver dram_overclock_drv =
{
    .name = "emi_clk_test",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

extern char __ssram_text, _sram_start, __esram_text;
int __init dram_overclock_init(void)
{
    int ret;
    unsigned char * dst = &__ssram_text;
    unsigned char * src =  &_sram_start;
    
    ret = driver_register(&dram_overclock_drv);
    if (ret) {
        printk(KERN_ERR "fail to create the dram_overclock driver\n");
        return ret;
    }
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_clk_test);
    if (ret) {
        printk(KERN_ERR "fail to create the dram_overclock sysfs files\n");
        return ret;
    }
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_clk_3d_test);
    if (ret) {
        printk(KERN_ERR "fail to create the ftm_dram_3d_drv sysfs files\n");
        return ret;
    }
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_clk_mtcmos_test);
    if (ret) {
        printk(KERN_ERR "fail to create the ftm_dram_mtcmos_drv sysfs files\n");
        return ret;
    }
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_clk_mem_test);
    if (ret) {
        printk(KERN_ERR "fail to create the ftm_dram_mtcmos_drv sysfs files\n");
        return ret;
    }
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_DFS_test);
    if (ret) {
        printk(KERN_ERR "fail to create the DFS sysfs files\n");
        return ret;
    }
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_DFS_status);
    if (ret) {
        printk(KERN_ERR "fail to create the DFS sysfs files\n");
        return ret;
    }
#ifdef APDMA_TEST
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_DFS_APDMA_test);
    if (ret) {
        printk(KERN_ERR "fail to create the DFS sysfs files\n");
        return ret;
    }
#endif
#ifdef Voltage_Debug
    ret = driver_create_file(&dram_overclock_drv, &driver_attr_emi_DRAM_Voltage);
    if (ret) {
        printk(KERN_ERR "fail to create the DFS sysfs files\n");
        return ret;
    }
#endif
    for (dst = &__ssram_text ; dst < (unsigned char *)&__esram_text ; dst++,src++) {
        *dst = *src;
    }
   
    return 0;
}

#endif

volatile unsigned char *dst_array_v;
volatile unsigned char *src_array_v;
volatile unsigned int dst_array_p;
volatile unsigned int src_array_p;
volatile int DMA_done;
int channel;

//#define APDMAREG_DUMP
#define DMA_BASE_CH(n)     IOMEM((AP_DMA_BASE + 0x0080 * (n + 1)))
#define DMA_SRC(base)           IOMEM((base + 0x001C))
#define DMA_DST(base)           IOMEM((base + 0x0020))
#define DMA_LEN1(base)          IOMEM((base + 0x0024))
#define DMA_GLOBAL_GSEC_EN IOMEM((AP_DMA_BASE + 0x0014))
#define DMA_INT_EN(base)        IOMEM((base + 0x0004))
#define DMA_CON(base)           IOMEM((base + 0x0018))
#define DMA_START(base)         IOMEM((base + 0x0008))
#define DMA_INT_FLAG(base) IOMEM((base + 0x0000))

#define DMA_GDMA_LEN_MAX_MASK   (0x000FFFFF)
#define DMA_GSEC_EN_BIT         (0x00000001)
#define DMA_INT_EN_BIT          (0x00000001)
#define DMA_INT_FLAG_CLR_BIT (0x00000000)
#define DRAM_BASE             (0x80000000)
#define BUFF_LEN   0x100
#define IOREMAP_ALIGMENT 0x1000
//#define TEMP_SENSOR
//#define APDMA_TEST
int DFS_APDMA_early_init(void)
{
   #ifdef APDMA_TEST
   src_array_p=DRAM_BASE+get_max_DRAM_size()/2-IOREMAP_ALIGMENT;
   dst_array_p=DRAM_BASE+get_max_DRAM_size()/2;
   src_array_v = ioremap(src_array_p,0x1000)+IOREMAP_ALIGMENT-BUFF_LEN/2;
   dst_array_v = ioremap(dst_array_p,0x1000)+BUFF_LEN/2;
   #else
   src_array_p=DRAM_BASE+get_max_DRAM_size()/2-BUFF_LEN/2;
   dst_array_p=DRAM_BASE+get_max_DRAM_size()/2+BUFF_LEN/2;
   #endif
   channel = DFS_APDMA_CHANNEL;
   enable_clock(MT_CG_PERI_AP_DMA,"APDMA_MODULE");
   mt_reset_gdma_conf(channel);
   disable_clock(MT_CG_PERI_AP_DMA,"APDMA_MODULE");
   return 1;
}


int DFS_APDMA_Init(void)
{
    writel(((~DMA_GSEC_EN_BIT)&readl(DMA_GLOBAL_GSEC_EN)), DMA_GLOBAL_GSEC_EN);
    return 1;
}
int DFS_APDMA_Enable(void)
{

    while(readl(DMA_START(DMA_BASE_CH(channel)))& 0x1);
    writel(src_array_p, DMA_SRC(DMA_BASE_CH(channel)));
    writel(dst_array_p, DMA_DST(DMA_BASE_CH(channel)));
    writel(BUFF_LEN , DMA_LEN1(DMA_BASE_CH(channel)));
    writel(DMA_CON_BURST_8BEAT, DMA_CON(DMA_BASE_CH(channel)));    
#ifdef APDMAREG_DUMP
   int i;
   printk("Channel 0x%x\n",channel);
    for (i=0;i<16;i++)
   {
     printk("[Before]addr:0x%x, value:%x\n",(0xF1000080+i*4),*((volatile int *)(0xF1000080+i*4)));
   }
     printk("[Before]addr:0x%x, value:%x\n",(0xF10000D0),*((volatile int *)(0xF10000D0)));
     printk("[Infra] addr:0x%x, value:%x\n",(0xF0001050),*((volatile int *)(0xF0001050)));
     printk("[Infra] addr:0x%x, value:%x\n",(0xF0000040),*((volatile int *)(0xF0000040)));
     printk("[Infra] addr:0x%x, value:%x\n",(0xF000305c),*((volatile int *)(0xF000305c)));
    for(i=0;i<32;i++)
   {
    printk("[Before]addr:0x%x, value:%x\n",(0xF1000000+i*4),*((volatile int *)(0xF1000000+i*4)));
   }                          
     
#ifdef APDMA_TEST
    for(i = 0; i < BUFF_LEN/sizeof(unsigned int); i++) {
                dst_array_v[i] = 0;
                src_array_v[i] = i;
        }

#endif
#endif

 mt_reg_sync_writel(0x1,DMA_START(DMA_BASE_CH(channel)));

#ifdef APDMAREG_DUMP
    for (i=0;i<15;i++)
   {
     printk("[AFTER]addr:0x%x, value:%x\n",(0xF1000080+i*4),*((volatile int *)(0xF1000080+i*4)));
   }
   
     printk("[AFTER]addr:0x%x, value:%x\n",(0xF10000D0),*((volatile int *)(0xF10000D0)));
    for(i=0;i<31;i++)
   {
    printk("[AFTER]addr:0x%x, value:%x\n",(0xF1000000+i*4),*((volatile int *)(0xF1000000+i*4)));
   }

#ifdef APDMA_TEST
        for(i = 0; i < BUFF_LEN/sizeof(unsigned int); i++){
                if(dst_array_v[i] != src_array_v[i]){
                        printk("DMA ERROR at Address %x\n", (unsigned int)&dst_array_v[i]);
                        ASSERT(0);
                }
        }
        printk("Channe0 DFS DMA TEST PASS\n");
#endif
#endif
        return 1;     
}

int DFS_APDMA_Init_2(void)
{
#ifdef TEMP_SENSOR
   unsigned val1;
    val1 = (DRAMC_READ_REG(0x1E8) & (0xff00ffff));      // Clear REFRCNT
    DRAMC_WRITE_REG(val1,0x1E8);
    dsb();
#endif

   return 1;
}


int DFS_APDMA_END(void)
{
#ifdef TEMP_SENSOR
   unsigned val1;
#endif

     while(readl(DMA_START(DMA_BASE_CH(channel))));

#ifdef TEMP_SENSOR
    val1 = (DRAMC_READ_REG(0x1E8) | (0x00010000));      // Enable temp sensor.
    DRAMC_WRITE_REG(val1,0x1E8);
    dsb();
#endif

  return 1 ;
}

int DFS_APDMA_free(void)
{
  mt_free_gdma(channel);
  return 1 ;
}

/*
 * XXX: Reserved memory in low memory must be 1MB aligned.
 *     This is because the Linux kernel always use 1MB section to map low memory.
 *
 *    We Reserved the memory regien which could cross rank for APDMA to do dummy read.
 *    
 */

void DFS_Reserved_Memory(int DFS_status)
{
  printk("DFS statuse from preload,lk, 0x%d\n",DFS_status);
  unsigned long high_memory_phys;
  high_memory_phys=virt_to_phys(high_memory);
  unsigned long DFS_dummy_read_center_address;
  DFS_dummy_read_center_address=DRAM_BASE+get_max_DRAM_size()/2;
  if(DFS_status==1)
  {
     /*For DFS Purpose, we remove this memory block for Dummy read/write by APDMA.*/
      printk("[DFS Check]DRAM SIZE:0x%x\n",get_max_DRAM_size());
      printk("[DFS Check]DRAM Dummy read  from:0x%x to 0x%x\n",(DFS_dummy_read_center_address-BUFF_LEN/2),(DFS_dummy_read_center_address+BUFF_LEN/2));
      printk("[DFS Check]DRAM Dummy read center address:0x%x\n",DFS_dummy_read_center_address);
      printk("[DFS Check]High Memory start address 0x%x\n",high_memory_phys);
      if((DFS_dummy_read_center_address - SZ_4K) >= high_memory_phys){
        printk("[DFS Check]DFS Dummy read reserved 0x%x to 0x%x\n",DFS_dummy_read_center_address-SZ_4K,DFS_dummy_read_center_address+SZ_4K);
        memblock_reserve(DFS_dummy_read_center_address-SZ_4K,SZ_4K*2);
        memblock_free(DFS_dummy_read_center_address-SZ_4K, SZ_4K*2);
        memblock_remove(DFS_dummy_read_center_address-SZ_4K,SZ_4K*2);
      }
      else
      {
        printk("[DFS Check]DFS Dummy read reserved 0x%x to 0x%x\n",DFS_dummy_read_center_address-SZ_1M,DFS_dummy_read_center_address+SZ_1M);
        memblock_reserve(DFS_dummy_read_center_address-SZ_1M,SZ_1M*2);
        memblock_free(DFS_dummy_read_center_address-SZ_1M,SZ_1M*2);
        memblock_remove(DFS_dummy_read_center_address-SZ_1M,SZ_1M*2);
      }
  }
  else
  {
    printk("This DRAM could not do DFS\n");
  }
  return;
}
int DFS_Detection(void)
{
   unsigned int DRAM_Type;
   extern unsigned int get_ddr_type(void)__attribute__((weak));
   if(get_ddr_type){
      DRAM_Type=get_ddr_type();
      DRAM_Type=LPDDR3;
      printk("dummy regs 0x%x\n",*((unsigned int *)(DRAMC0_BASE + 0xf4)));
      printk("check 0x%x\n",(0x1 <<15));
      if(DRAM_Type == LPDDR3)
      {
         if(*((unsigned int *)(DRAMC0_BASE + 0xf4))&(0x1 <<15))
         {
           printk("[LPDDR3] DFS could be enable \n");
           return 1;
         }
         else
         {
           printk("[LPDDR3] DFS could not be enable\n");
           return -1;
         }
      }
      else
      {
         printk("[LPDDR2] do not support DFS\n");
         return -1;
      }
   }
   else
   {
       printk("do not support get_ddr_type\n");
       ASSERT(0);
       return -1;
   }
}
int configure_mrw_pasr(int segment)
{
   unsigned int ddr_type=0;
   extern unsigned int get_ddr_type(void)__attribute__((weak));
   if(get_ddr_type){
      ddr_type=get_ddr_type(); 
      if(ddr_type == LPDDR3 || ddr_type == LPDDR2)
      {
         ndelay(4*10);
         *((volatile unsigned *) (DRAMC0_BASE + 0x0088)) = 0x00000011 | segment << 16;
         ndelay(4*10);
         *((volatile unsigned *) (DRAMC0_BASE + 0x01e8)) |= 0x04000000;
         *((volatile unsigned *) (DRAMC0_BASE + 0x01e4)) |= 0x00000001;
         ndelay(5*10);
         while(*((volatile unsigned *) (DRAMC_NAO_BASE + 0x03b8)) & 0x00000001);
         ndelay(1*10);
         *((volatile unsigned *) (DRAMC0_BASE + 0x01e4)) &= ~(0x00000001);
         *((volatile unsigned *) (DRAMC0_BASE + 0x01e8)) &= ~(0x04000000);
      }
      else if(ddr_type==DDR3_16||ddr_type==DDR3_32)
      {
         printk("do not support DDR3 PASR\n");
         ASSERT(0);
         return -1;
      }
   }
   else
   {

       printk("do not support get_ddr_type\n");
       ASSERT(0);
       return -1;
   }
   return 0;
}

arch_initcall(dram_overclock_init);
