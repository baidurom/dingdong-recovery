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
#define LOG_TAG "SeninfDrvImp"
//
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <mt6589_sync_write.h>
#include <cutils/xlog.h>

//
#include "drv_types.h"
#include "isp_drv.h"
#include "camera_isp.h"
#include "isp_reg.h"
#include "seninf_reg.h"
#include "seninf_drv_imp.h"
#include "sensor_hal.h"
//
/******************************************************************************
*
*******************************************************************************/
#define SENINF_DEV_NAME     "/dev/mt6589-seninf"
#define ISP_DEV_NAME     	"/dev/camera-isp"

#define CAM_PLL_RANGE   (0x200)
#define SCAM_ENABLE     1   // 1: enable SCAM feature. 0. disable SCAM feature.

#define FPGA 0 //for FPGA

#define CAM_APCONFIG_RANGE 0x1000
#define CAM_MMSYS_RANGE 0x100
#define CAM_MIPIRX_CONFIG_RANGE 0x100
#define CAM_MIPIRX_ANALOG_RANGE 0x1000
#define CAM_MIPIPLL_RANGE 0x100
#define CAM_GPIO_RANGE 0x1000
/*******************************************************************************/
#define DEV_IOC_MAGIC       'd'
#define READ_DEV_DATA       _IOR(DEV_IOC_MAGIC,  1, unsigned int)


#define OPEN_DEVINFO_NODE_FAIL	0x1001
#define READ_DEVINFO_DATA_FAIL	0x1002	

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::createInstance()
{
    return SeninfDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrvImp::
getInstance()
{
    LOG_MSG("[getInstance] \n");
    static SeninfDrvImp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
SeninfDrvImp::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::SeninfDrvImp() :
    SeninfDrv()
{
    LOG_MSG("[SeninfDrvImp] \n");

    mUsers = 0;
    mfd = 0;
    tg1GrabWidth = 0;
    tg1GrabHeight = 0;
    tg2GrabWidth = 0;
    tg2GrabHeight = 0;

}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::~SeninfDrvImp()
{
    LOG_MSG("[~SeninfDrvImp] \n");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()  //kk test
{
    LOG_MSG("[init]: Entry count %d \n", mUsers);
    MBOOL result;
    MINT32 imgsys_cg_clr0 = 0x15000000;
    MINT32 pll_base_hw = 0x10000000;
    MINT32 mipiRx_config = 0x1500C000;
//    MINT32 mipiRx_analog = 0x10012800;
    MINT32 mipiRx_analog = 0x10012000;
    MINT32 gpio_base_addr = 0x10005000;
    MUINT32 temp=0,temp1=0;
	MINT32 fd = 0;    	
	MINT32 ret = 0;
	MUINT32 devinfo_data_res2=0,devinfo_data_res3=0;   //serve as index as well as readback data


    Mutex::Autolock lock(mLock);

    //
    if (mUsers > 0) {
        LOG_MSG("  Has inited \n");
        android_atomic_inc(&mUsers);
        return 0;
    }

    // Open isp driver
    mfd = open(ISP_DEV_NAME, O_RDWR);
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }


	// Access mpIspHwRegAddr
	m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv) {
        LOG_ERR("IspDrvImp::createInstance fail \n");
        return -2;
    }

    //
    result = m_pIspDrv->init();
    if ( MFALSE == result ) {
        LOG_ERR("pIspDrv->init() fail \n");
        return -3;
    }


	//get isp reg for TG module use
    mpIspHwRegAddr = (unsigned long*)m_pIspDrv->getRegAddr();

    if ( NULL == mpIspHwRegAddr ) {
        LOG_ERR("getRegAddr fail \n");
        return -4;
    }

    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned long *) mmap(0, SENINF_BASE_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, SENINF_BASE_HW);

    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -5;
    }

    // mmap pll reg
    mpPLLHwRegAddr = (unsigned long *) mmap(0, CAM_PLL_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, pll_base_hw);
    if (mpPLLHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(2), %d, %s \n", errno, strerror(errno));
        return -6;
    }


    // mmap seninf clear gating reg
    mpCAMMMSYSRegAddr = (unsigned long *) mmap(0, CAM_MMSYS_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, imgsys_cg_clr0);
    if (mpCAMMMSYSRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(3), %d, %s \n", errno, strerror(errno));
        return -7;
    }

    // mipi rx config address
    mpCSI2RxConfigRegAddr = (unsigned long *) mmap(0, CAM_MIPIRX_CONFIG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, mipiRx_config);
    if (mpCSI2RxConfigRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(4), %d, %s \n", errno, strerror(errno));
        return -8;
    }


    // mipi rx analog address
    mpCSI2RxAnalogRegStartAddr = (unsigned long *) mmap(0, CAM_MIPIRX_ANALOG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, mipiRx_analog);
    if (mpCSI2RxAnalogRegStartAddr == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s \n", errno, strerror(errno));
        return -9;
    }


    //gpio
    mpGpioHwRegAddr = (unsigned long *) mmap(0, CAM_GPIO_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, gpio_base_addr);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(6), %d, %s \n", errno, strerror(errno));
        return -10;
    }

    //mpSENINFClearGateRegAddr = mpCAMMMSYSRegAddr + (0x08/4);
    //mpSENINFSetGateRegAddr = mpCAMMMSYSRegAddr + (0x04/4);
    mpIPllCon0RegAddr = mpPLLHwRegAddr + (0x158 /4);


    mpCAMIODrvRegAddr = mpGpioHwRegAddr + (0x5B0 / 4);//GPIO210 -> CMMCLK

//
#ifdef USING_MTK_LDVT
    unsigned long *pCMMCLKReg = mpGpioHwRegAddr + (0xEA0 / 4);
    *pCMMCLKReg = ( (*pCMMCLKReg)&(~0x07) ) | 0x01;
#endif

    mpCSI2RxAnalogRegAddr = mpCSI2RxAnalogRegStartAddr + (0x800/4);
    // enable seninf cam & tg  clear gating
    //*(mpSENINFClearGateRegAddr) |= 0x000001FF;

       //set CMMCLK mode 1

       temp = *(mpGpioHwRegAddr + (0xEA0/4));
       mt65xx_reg_sync_writel((temp&0xFFF8),mpGpioHwRegAddr + (0xEA0/4) );
       temp = *(mpGpioHwRegAddr + (0xEA0/4));    
       mt65xx_reg_sync_writel((temp|= 0x0001),mpGpioHwRegAddr + (0xEA0/4) );        


    //MIPI analog parameter setting by eFuse  start
    /* =================================== */
    /* open devinfo driver                 */
    /* =================================== */    
    fd = open("/dev/devmap", O_RDONLY, 0);
    if (fd < 0)
    {
        ret = OPEN_DEVINFO_NODE_FAIL;
        LOG_MSG("[init]: open devmap fail = 0x%x \n", ret);

    }
    /* ----------------------------------- */
    /* Read Devinfo data                   */
    /* ----------------------------------- */ 
    devinfo_data_res2 = 9;
    devinfo_data_res3 = 14;

	if ((ret = ioctl(fd, READ_DEV_DATA, &devinfo_data_res2)) != 0)
	{	
		ret = READ_DEVINFO_DATA_FAIL;
		LOG_MSG("Get Devinfo Data Fail:%d\n", ret);
	}
	else
	{
		LOG_MSG("Get Devinfo Data:0x%x\n", devinfo_data_res2);
	}
	if ((ret = ioctl(fd, READ_DEV_DATA, &devinfo_data_res3)) != 0)
	{	
		ret = READ_DEVINFO_DATA_FAIL;
		LOG_MSG("Get Devinfo Data Fail:%d\n", ret);

	}
	else
	{
		LOG_MSG("Get Devinfo Data:0x%x\n", devinfo_data_res3);
	}

    if((devinfo_data_res2&0x00007800)!=0)
    {
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));
        temp = (temp>>12)&0xF;
        temp1 = ((devinfo_data_res2>>11)&0xF);
        temp = (((temp +temp1)&0xF)<<12);       
        LOG_MSG("MIPI_RX_ANA24[15:12]:%d\n", temp);
        mt65xx_reg_sync_writel(temp|0xFFFF0FFF, mpCSI2RxAnalogRegAddr + (0x24/4));    
    }

    if(((devinfo_data_res2&0x00008000)!=0) || ((devinfo_data_res3&0xE0000000)!=0))
    {
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));
        temp = (temp>>8)&0xF;
        temp1 = ((devinfo_data_res2>>12)&0x8)+((devinfo_data_res3>>29)&0x7);
        temp = (((temp +temp1)&0xF)<<8); 
        LOG_MSG("MIPI_RX_ANA24[11:8]:%d\n", temp);
        mt65xx_reg_sync_writel(temp|0xFFFFF0FF, mpCSI2RxAnalogRegAddr + (0x24/4));
    }

     if (fd > 0)
    {
        close(fd);
        fd = 0;
    }
    //MIPI analog parameter setting by eFuse  end


    android_atomic_inc(&mUsers);

    LOG_MSG("[init]: Exit count %d \n", mUsers);


    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    LOG_MSG("[uninit]: %d \n", mUsers);
    MBOOL result;
    unsigned int temp = 0;

    Mutex::Autolock lock(mLock);
    //
    if (mUsers <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);

    
    if (mUsers == 0) {
        // Last user
        setTg1CSI2(0, 0, 0, 0, 0, 0, 0, 0);   // disable CSI2
		setTg2CSI2(0, 0, 0, 0, 0, 0, 0, 0);   // disable CSI2
		setTg1PhaseCounter(0, 0, 0, 0, 0, 0, 0);
		setTg2PhaseCounter(0, 0, 0, 0, 0, 0, 0);        
        //set CMMCLK mode 0
        *(mpGpioHwRegAddr + (0xEA0/4)) &= 0xFFF8;   

        //disable MIPI RX analog
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFC, mpCSI2RxAnalogRegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalogRegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFF0, mpCSI2RxAnalogRegAddr);

        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x10/4));   

        temp = *(mpCSI2RxAnalogRegAddr + (0x14/4));//RG_CSI1_LDO_CORE_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x14/4));
        usleep(1);
        temp = *(mpCSI2RxAnalogRegAddr + (0x18/4));//RG_CSI1_LNRD0_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x18/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x1C/4));//RG_CSI1_LNRD1_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x1C/4));



	    // disable seninf cam & tg  clear gating
        //*(mpSENINFSetGateRegAddr) |= 0x000001FF;
        //
        mpIspHwRegAddr = NULL;
        if ( 0 != mpSeninfHwRegAddr ) {
            munmap(mpSeninfHwRegAddr, SENINF_BASE_RANGE);
            mpSeninfHwRegAddr = NULL;
        }
        // Disable Camera PLL
        if ( mpIPllCon0RegAddr ) {
            (*mpIPllCon0RegAddr) |= 0x01; //Power Down
        }
        if ( 0 != mpPLLHwRegAddr ) {
            munmap(mpPLLHwRegAddr, CAM_PLL_RANGE);
            mpPLLHwRegAddr = NULL;
        }
        if ( 0 != mpCAMAPConRegAddr ) {
            munmap(mpCAMAPConRegAddr, CAM_APCONFIG_RANGE);
            mpCAMAPConRegAddr = NULL;
        }
        if ( 0 != mpCAMMMSYSRegAddr ) {
            munmap(mpCAMMMSYSRegAddr, CAM_MMSYS_RANGE);
            mpCAMMMSYSRegAddr = NULL;
        }
        if ( 0 != mpCSI2RxConfigRegAddr ) {
            munmap(mpCSI2RxConfigRegAddr, CAM_MIPIRX_CONFIG_RANGE);
            mpCSI2RxConfigRegAddr = NULL;
        }
        if ( 0 != mpCSI2RxAnalogRegStartAddr ) {
            munmap(mpCSI2RxAnalogRegStartAddr, CAM_MIPIRX_ANALOG_RANGE);
            mpCSI2RxAnalogRegStartAddr = NULL;
        }
        if ( 0 != mpGpioHwRegAddr ) {
            munmap(mpGpioHwRegAddr, CAM_GPIO_RANGE);
            mpGpioHwRegAddr = NULL;
        }
        //
        result = m_pIspDrv->uninit();

        LOG_MSG("[uninit]: %d \n", mUsers);
        if ( MFALSE == result ) {
            LOG_ERR("pIspDrv->uninit() fail \n");
            return -3;
        }
        //
        if (mfd > 0) {
            close(mfd);
            mfd = -1;
        }
    }
    else {
        LOG_MSG("  Still users \n");
    }

    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::waitSeninf1Irq(int mode)
{
    int ret = 0;

    LOG_MSG("[waitIrq polling] 0x%x \n", mode);
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int sleepCount = 40;
    int sts;
    ret = -1;
    while (sleepCount-- > 0) {
        sts = SENINF_READ_REG(pSeninf, SENINF1_INTSTA);  // Not sure CTL_INT_STATUS or CTL_INT_EN
        if (sts & mode) {
            LOG_MSG("[waitIrq polling] Done: 0x%x \n", sts);
            ret = 0;
            break;
        }
        LOG_MSG("[waitIrq polling] Sleep... %d, 0x%x \n", sleepCount, sts);
        usleep(100 * 1000);
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::waitSeninf2Irq(int mode)
{
    int ret = 0;

    LOG_MSG("[waitIrq polling] 0x%x \n", mode);
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int sleepCount = 40;
    int sts;
    ret = -1;
    while (sleepCount-- > 0) {
        sts = SENINF_READ_REG(pSeninf, SENINF2_INTSTA);  // Not sure CTL_INT_STATUS or CTL_INT_EN
        if (sts & mode) {
            LOG_MSG("[waitIrq polling] Done: 0x%x \n", sts);
            ret = 0;
            break;
        }
        LOG_MSG("[waitIrq polling] Sleep... %d, 0x%x \n", sleepCount, sts);
        usleep(100 * 1000);
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1PhaseCounter(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;

    LOG_MSG("[setTg1PhaseCounter] pcEn(%d) clkPol(%d)\n",pcEn,clkPol);

    // Enable Camera PLL first
    if (mclkSel == CAM_PLL_48_GROUP) {
        //48MHz
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp&0xFFFF00FF, mpIPllCon0RegAddr);
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp|= 0x100, mpIPllCon0RegAddr);
   
    }
    else if (mclkSel == CAM_PLL_52_GROUP) {
        //104MHz
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp&0xFFFF00FF, mpIPllCon0RegAddr);
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp|0x200,mpIPllCon0RegAddr);
     
    }
    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt         

    //Seninf Top pclk clear gating
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN, 1);
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN, 1);

    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKRS, clkRiseEdge);
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL, clkFallEdge);
    //SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = clkCnt - 1;
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT, clkCnt);

    //TODO:remove later
	//SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = 0;  //FPGA
	//SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL) = 0;	//FPGA

	//SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL) = clkCnt >> 1;//fpga
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKFL_POL, (clkCnt & 0x1) ? 0 : 1);

    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKPOL, clkPol);
    // mclkSel, 0: 122.88MHz, (others: Camera PLL) 1: 120.3MHz, 2: 52MHz
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, TGCLK_SEL, 1);//force PLL due to ISP engine clock dynamic spread
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN, 1);//FPGA experiment
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, PCEN, pcEn);//FPGA experiment
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, PAD_PCLK_INV, padPclkInv);
	ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN, 1);
    // Wait 1ms for PLL stable
    usleep(1000);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2PhaseCounter(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;

    LOG_MSG("[setTg2PhaseCounter] \n");
    // Enable Camera PLL first
    if (mclkSel == CAM_PLL_48_GROUP) {
        //48MHz
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp&0xFFFF00FF, mpIPllCon0RegAddr);
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp|= 0x100, mpIPllCon0RegAddr);


    }
    else if (mclkSel == CAM_PLL_52_GROUP) {
        //104MHz
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp&0xFFFF00FF, mpIPllCon0RegAddr);
        temp = (*mpIPllCon0RegAddr);
        mt65xx_reg_sync_writel(temp|0x200,mpIPllCon0RegAddr);

    }
    //
        clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt  

    //Seninf Top pclk clear gating
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN, 1);
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN, 1);
    // TG phase counter register (0x83A0)
    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN, pcEn);    // TG phase counter enable control
    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, ADCLK_EN, 1);    // Enable sensor master clock (mclk) output to sensor. Note that to set sensor master clock driving setting,
    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKPOL, clkPol);    // Sensor master clock polarity control
    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, CAM_PCLK_INV, 0);    // Pixel clock inverse in CAM.
	SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, PAD_PCLK_INV, padPclkInv);    // Pixel clock inverse in PAD side

	SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKFL_POL, (clkCnt & 0x1) ? 0 : 1);    // Sensor clock falling edge polarity
	SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, TGCLK_SEL, 1);//force PLL due to ISP engine clock dynamic spread

    // TG Sensor clock divider (0x83A4)
	//SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKCNT) = clkCnt-1;       // Sensor master clock falling edge control
	SENINF_WRITE_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKCNT, clkCnt); 	  // Sensor master clock falling edge control
    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKRS, clkRiseEdge);  // Sensor master clock rising edge control
    //SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKFL ) = clkCnt >> 1;  // "Sensor master clock frequency divider controlSensor master clock will be ISP_clock/CLKCNT, where CLKCNT >=1."
    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKFL, clkFallEdge);//fpga
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN, 1);
    //ISP_BITS(pisp, CAM_CTL_SEL, TG_SEL) = 1;   //select TG2


    //Wait 1ms for PLL stable
    usleep(1000);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1GrabRange] \n");
    tg1GrabWidth = pixelEnd - pixelStart;
    tg1GrabHeight = lineEnd - lineStart;

    // TG Grab Win Setting
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_E, pixelEnd);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_S, pixelStart);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_E, lineEnd);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_S, lineStart);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg2GrabRange] \n");
    tg2GrabWidth = pixelEnd - pixelStart;
    tg2GrabHeight = lineEnd - lineStart;    

    // TG Grab Win Setting
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_E, pixelEnd);
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_S, pixelStart);
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_E, lineEnd);
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_S, lineStart);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1SensorModeCfg(
    unsigned long hsPol, unsigned long vsPol
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1SensorModeCfg] \n");

    // Sensor Mode Config
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, hsPol);
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, vsPol);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN, 1);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOT_MODE, 1);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2SensorModeCfg(
    unsigned long hsPol, unsigned long vsPol
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg2SensorModeCfg] \n");

    // Sensor Mode Config
    SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_HSYNC_POL, hsPol);
    SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_VSYNC_POL, vsPol);
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN, 1);
    ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, SOT_MODE, 1);
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1InputCfg(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel,
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb
)
{
    int ret = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1InputCfg] \n");
    LOG_MSG("inSrcTypeSel = % d \n",inSrcTypeSel);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL, padSel);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, inSrcTypeSel);
    ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, SEN_IN_LSB, 0x0);//no matter what kind of format, set 0
    ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TG1_FMT_CLR, 0x7);
    ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TG1_FMT_SET, inDataType);


	if (MIPI_SENSOR == inSrcTypeSel) {
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC, 0x0);
		if (JPEG_FMT == inDataType) {
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x18);//0x1B;
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1E);//0x1F;
			SENINF_WRITE_BITS(pSeninf, SENINF1_SPARE, SENINF_FIFO_FULL_SEL, 0x1);
			SENINF_WRITE_BITS(pSeninf, SENINF1_SPARE, SENINF_VCNT_SEL, 0x1);
			SENINF_WRITE_BITS(pSeninf, SENINF1_SPARE, SENINF_CRC_SEL, 0x2);
		}
		else {
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x1B);
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1F);
		}

	}
	else {
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC, 0x1);
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x1B);
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1F);
	}

	//One-pixel mode
	if ( JPEG_FMT != inDataType) {
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_PIX_SEL, 0);
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS, 0);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TWO_PIX_CLR, 1);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TWO_PIX_SET, 0);
		ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN, 0);

	}
	else {
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_PIX_SEL, 1);
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS, 1);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TWO_PIX_CLR, 1);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TWO_PIX_SET, 1);
		ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN, 1);
	}


	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_IRQ_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_SW_RST, 0x0);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_IRQ_SW_RST, 0x0);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2InputCfg(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel,
	TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb
)

{
    int ret = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg2InputCfg] \n");
    LOG_MSG("inSrcTypeSel = % d \n",inSrcTypeSel);
	SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, PAD2CAM_DATA_SEL, padSel);
	SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_SRC_SEL, inSrcTypeSel);
    ISP_WRITE_BITS(pisp, CAM_TG2_PATH_CFG, SEN_IN_LSB, 0x0);//no matter what kind of format, set 0
    ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TG2_FMT_CLR, 0x7);
    ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TG2_FMT_SET, inDataType);

	if (MIPI_SENSOR == inSrcTypeSel) {
		ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, SOF_SRC, 0x0);
		if (JPEG_FMT == inDataType ) {
			SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, FIFO_FLUSH_EN, 0x18);//0x1B;
			SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, FIFO_PUSH_EN, 0x3E);//0x1F;

		}
		else {

			SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, FIFO_FLUSH_EN, 0x1B);
			SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, FIFO_PUSH_EN, 0x1F);
		}
	}
	else {
		ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, SOF_SRC, 0x1);
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x1B);
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1F);
	}

	//One-pixel mode
	if ( JPEG_FMT != inDataType) {
		SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_PIX_SEL, 0);
		ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, DBL_DATA_BUS, 0);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TWO_PIX2_CLR, 1);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TWO_PIX2_SET, 0);
	    ISP_WRITE_BITS(pisp, CAM_TG2_PATH_CFG, JPGINF_EN, 0);

	}
	else {
		SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_PIX_SEL, 1);
		ISP_WRITE_BITS(pisp, CAM_TG2_SEN_MODE, DBL_DATA_BUS, 1);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TWO_PIX2_CLR, 1);
		ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TWO_PIX2_SET, 1);
		ISP_WRITE_BITS(pisp, CAM_TG2_PATH_CFG, JPGINF_EN, 1);
	}
	SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_MUX_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_IRQ_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_MUX_SW_RST, 0x0);
	SENINF_WRITE_BITS(pSeninf, SENINF2_CTRL, SENINF_IRQ_SW_RST, 0x0);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1ViewFinderMode(
    unsigned long spMode, unsigned long spDelay
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1ViewFinderMode] \n");
    //
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SPDELAY_MODE, 1);
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SINGLE_MODE, spMode);
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SP_DELAY, spDelay);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2ViewFinderMode(
    unsigned long spMode, unsigned long spDelay
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg2ViewFinderMode] \n");
    //
    ISP_WRITE_BITS(pisp, CAM_TG2_VF_CON, SPDELAY_MODE, 1);
    ISP_WRITE_BITS(pisp, CAM_TG2_VF_CON, SINGLE_MODE, spMode);
    ISP_WRITE_BITS(pisp, CAM_TG2_VF_CON, SP_DELAY, spDelay);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::sendCommand(int cmd, int arg1, int arg2, int arg3)
{
    int ret = 0;

    //LOG_MSG("[sendCommand] cmd: 0x%x \n", cmd);
    switch (cmd) {
    case CMD_SET_DEVICE:
        mDevice = arg1;
        break;
        
    case CMD_GET_SENINF_ADDR:
        //LOG_MSG("  CMD_GET_ISP_ADDR: 0x%x \n", (int) mpIspHwRegAddr);
        *(int *) arg1 = (int) mpSeninfHwRegAddr;
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned long SeninfDrvImp::readReg(unsigned long addr)
{
    int ret;
    reg_t reg[2];
    int val = 0xFFFFFFFF;

    LOG_MSG("[readReg] addr: 0x%08x \n", (int) addr);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = readRegs(reg, 1);
    if (ret < 0) {
    }
    else {
        val = reg[0].val;
    }

    return val;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::writeReg(unsigned long addr, unsigned long val)
{
    int ret;
    reg_t reg[2];

    LOG_MSG("[writeReg] addr/val: 0x%08x/0x%08x \n", (int) addr, (int) val);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = writeRegs(reg, 1);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::readRegs(reg_t *pregs, int count)
{
    MBOOL result = MTRUE;
    result = m_pIspDrv->readRegs( (ISP_DRV_REG_IO_STRUCT*) pregs, count);
    if ( MFALSE == result ) {
        LOG_ERR("MT_ISP_IOC_G_READ_REG err \n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::writeRegs(reg_t *pregs, int count)
{
    MBOOL result = MTRUE;
    result = m_pIspDrv->writeRegs( (ISP_DRV_REG_IO_STRUCT*) pregs, count);
    if ( MFALSE == result ) {
        LOG_ERR("MT_ISP_IOC_S_WRITE_REG err \n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::holdReg(bool isHold)
{
    int ret;
    int hold = isHold;

    //LOG_MSG("[holdReg]");

    ret = ioctl(mfd, ISP_HOLD_REG, &hold);
    if (ret < 0) {
        LOG_ERR("ISP_HOLD_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::dumpReg()
{
    int ret;

    LOG_MSG("[dumpReg] \n");

    ret = ioctl(mfd, ISP_DUMP_REG, NULL);
    if (ret < 0) {
        LOG_ERR("ISP_DUMP_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::initTg1CSI2(bool csi2_en)
{
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int ret = 0;
    unsigned int temp = 0;

    //E2 ECO for LS/LE 
    temp = SENINF_READ_BITS(pSeninf, SENINF1_CSI2_INTSTA, CSI2_SPARE) & 0x7;
    SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_INTSTA, CSI2_SPARE, (temp|0x4));


    
   LOG_MSG("[initCSI2]:enable = %d\n", (int) csi2_en);
	if(csi2_en == 0) {
        //disable mipi BG
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x24/4));
        // disable mipi pin
        temp = *(mpGpioHwRegAddr + (0x1C0/4));//GPI*_IES = 1 for Parallel CAM
        mt65xx_reg_sync_writel(temp|=0xFFE0, mpGpioHwRegAddr + (0x1C0/4));
        temp = *(mpGpioHwRegAddr + (0x1D0/4));//GPI*_IES = 1 for Parallel CAM
        mt65xx_reg_sync_writel(temp|=0x0001, mpGpioHwRegAddr + (0x1D0/4));

        temp = *(mpCSI2RxAnalogRegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_sync_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x00));
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_sync_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_sync_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//data lane 2 input select hi-Z
        mt65xx_reg_sync_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//data lane 3 input select hi-Z
        mt65xx_reg_sync_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x10/4));

        /* parallel pad select to mode 1*/
        temp = *(mpGpioHwRegAddr + (0xE70/4)) ;
        mt65xx_reg_sync_writel(temp&0x003F, mpGpioHwRegAddr + (0xE70/4));
        temp = *(mpGpioHwRegAddr + (0xE80/4));
        mt65xx_reg_sync_writel(temp&0x0000, mpGpioHwRegAddr + (0xE80/4));
        temp = *(mpGpioHwRegAddr + (0xE90/4));
        mt65xx_reg_sync_writel(temp&0xF000, mpGpioHwRegAddr + (0xE90/4));
        temp = *(mpGpioHwRegAddr + (0xE70/4));
        mt65xx_reg_sync_writel(temp|0x1240, mpGpioHwRegAddr + (0xE70/4));
        temp = *(mpGpioHwRegAddr + (0xE80/4));
        mt65xx_reg_sync_writel(temp|0x1249, mpGpioHwRegAddr + (0xE80/4));
        temp = *(mpGpioHwRegAddr + (0xE90/4));
        mt65xx_reg_sync_writel(temp|0x0249, mpGpioHwRegAddr + (0xE90/4));

	}
	else {
         // enable mipi lane

        temp = *(mpCSI2RxAnalogRegAddr + (0x00));//clock lane input select mipi
        mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x00));
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//data lane 0 input select mipi
        mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//data lane 1 input select mipi
        mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0c/4));//data lane 2 input select mipi
        mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//data lane 3 input select mipi
        mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x10/4));        


        /* parallel pad select to mode 0*/
        temp = *(mpGpioHwRegAddr + (0xE70/4));
        mt65xx_reg_sync_writel(temp&0x003F, mpGpioHwRegAddr + (0xE70/4));
        temp = *(mpGpioHwRegAddr + (0xE80/4));
        mt65xx_reg_sync_writel(temp&0x0000, mpGpioHwRegAddr + (0xE80/4));
        temp = *(mpGpioHwRegAddr + (0xE90/4));
        mt65xx_reg_sync_writel(temp&0xF000, mpGpioHwRegAddr + (0xE90/4));


        // enable mipi ap
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x24/4));
        usleep(30);
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalogRegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr);
        usleep(1);

        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x10/4));        
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LNC_HSRXDB_EN, 1);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN0_HSRXDB_EN, 1);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN1_HSRXDB_EN, 1);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN2_HSRXDB_EN, 1);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN3_HSRXDB_EN, 1);
       temp = *(mpCSI2RxConfigRegAddr + (0x38/4));//MIPI_RX_HW_CAL_START
       mt65xx_reg_sync_writel(temp|0x00000004, mpCSI2RxConfigRegAddr + (0x38/4));        
       LOG_MSG("[initCSI2]:CSI0 calibration start !\n");
        usleep(100);       
       while(!((*(mpCSI2RxConfigRegAddr + (0x44/4)) & 0x10001) && (*(mpCSI2RxConfigRegAddr + (0x48/4)) & 0x101))){}
       LOG_MSG("[initCSI2]:CSI0 calibration end !\n");

       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LNC_HSRXDB_EN, 0);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN0_HSRXDB_EN, 0);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN1_HSRXDB_EN, 0);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN2_HSRXDB_EN, 0);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN3_HSRXDB_EN, 0);
	}

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::initTg2CSI2(bool csi2_en)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int ret = 0;
    unsigned int temp = 0;

    //E2 ECO for LS/LE 
    temp = SENINF_READ_BITS(pSeninf, SENINF2_CSI2_INTSTA, CSI2_SPARE) & 0x7;
    SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_INTSTA, CSI2_SPARE, (temp|0x4));
    
    LOG_MSG("[initCSI2]:%d\n", (int) csi2_en);
     if(csi2_en == 0) {      // disable mipi ap config and csi2 clear gating
        //disable mipi BG
       temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
       mt65xx_reg_sync_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x24/4));               
        // disable mipi pin
       temp = *(mpGpioHwRegAddr + (0x1C0/4)) ;//GPI*_IES = 1 for Parallel CAM
       mt65xx_reg_sync_writel(temp|0xFFE0, mpGpioHwRegAddr + (0x1C0/4));               
       temp = *(mpGpioHwRegAddr + (0x1D0/4));//GPI*_IES = 1 for Parallel CAM
       mt65xx_reg_sync_writel(temp|0x0001, mpGpioHwRegAddr + (0x1D0/4));                      

       temp = *(mpCSI2RxAnalogRegAddr + (0x14/4));//clock lane  input select hi-Z
       mt65xx_reg_sync_writel(temp&0xFFFFFFE7, mpCSI2RxAnalogRegAddr + (0x14/4));                      
       temp = *(mpCSI2RxAnalogRegAddr + (0x18/4)) ;//data lane 1 input select hi-Z
       mt65xx_reg_sync_writel(temp&0xFFFFFFE7,mpCSI2RxAnalogRegAddr + (0x18/4));                      
       temp = *(mpCSI2RxAnalogRegAddr + (0x1c/4));//data lane 2 input select hi-Z
       mt65xx_reg_sync_writel(temp&0xFFFFFFE7, mpCSI2RxAnalogRegAddr + (0x1c/4));                      

       /* parallel pad select to mode 1*/
       temp = *(mpGpioHwRegAddr + (0xE70/4)) ;
       mt65xx_reg_sync_writel(temp&0x003F, mpGpioHwRegAddr + (0xE70/4));
       temp = *(mpGpioHwRegAddr + (0xE80/4));
       mt65xx_reg_sync_writel(temp&0x0000, mpGpioHwRegAddr + (0xE80/4));
       temp = *(mpGpioHwRegAddr + (0xE90/4));
       mt65xx_reg_sync_writel(temp&0xF000, mpGpioHwRegAddr + (0xE90/4));
       temp = *(mpGpioHwRegAddr + (0xE70/4));
       mt65xx_reg_sync_writel(temp|0x1240, mpGpioHwRegAddr + (0xE70/4));
       temp = *(mpGpioHwRegAddr + (0xE80/4));
       mt65xx_reg_sync_writel(temp|0x1249, mpGpioHwRegAddr + (0xE80/4));
       temp = *(mpGpioHwRegAddr + (0xE90/4));
       mt65xx_reg_sync_writel(temp|0x0249, mpGpioHwRegAddr + (0xE90/4));
       
       /*Tg2PadSel 0:CM2_2X (DPI) 1:CAM1 2:CM2_2X (NAND) */
       mt65xx_reg_sync_writel(0x01, mpCAMMMSYSRegAddr + (0x34/4));

     }
     else {
          // enable mipi pin
          temp = *(mpGpioHwRegAddr + (0x1C0/4));//GPI*_IES = 0 for MIPI CAM
          mt65xx_reg_sync_writel(temp&0x0000001F, mpGpioHwRegAddr + (0x1C0/4));
          temp = *(mpGpioHwRegAddr + (0x1D0/4));//GPI*_IES = 0 for MIPI CAM
          mt65xx_reg_sync_writel(temp&0x0000FFFE, mpGpioHwRegAddr + (0x1D0/4));
        if(mDevice & SENSOR_DEV_MAIN_2) {
             temp = *(mpCSI2RxAnalogRegAddr + (0x14/4));//clock lane  input select mipi
             mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x14/4));
             temp = *(mpCSI2RxAnalogRegAddr + (0x18/4));//data lane 1 input select mipi
             mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x18/4));
             temp = *(mpCSI2RxAnalogRegAddr + (0x1c/4));//data lane 2 input select mipi
             mt65xx_reg_sync_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x1c/4));
        }
        else if (mDevice & SENSOR_DEV_SUB) {
            temp = *(mpCSI2RxAnalogRegAddr + (0x14/4));//clock lane  input select mipi
            mt65xx_reg_sync_writel(temp|0x00000010, mpCSI2RxAnalogRegAddr + (0x14/4));
            temp = *(mpCSI2RxAnalogRegAddr + (0x18/4));//data lane 1 input select mipi
            mt65xx_reg_sync_writel(temp|0x00000010, mpCSI2RxAnalogRegAddr + (0x18/4));
            temp = *(mpCSI2RxAnalogRegAddr + (0x1c/4));//data lane 2 input select mipi
            mt65xx_reg_sync_writel(temp|0x00000010, mpCSI2RxAnalogRegAddr + (0x1c/4));
        }            

       /* parallel pad select to mode 0*/
       temp = *(mpGpioHwRegAddr + (0xE70/4));
       mt65xx_reg_sync_writel(temp&0x003F, mpGpioHwRegAddr + (0xE70/4));
       temp = *(mpGpioHwRegAddr + (0xE80/4));
       mt65xx_reg_sync_writel(temp&0x0000, mpGpioHwRegAddr + (0xE80/4));
       temp = *(mpGpioHwRegAddr + (0xE90/4));
       mt65xx_reg_sync_writel(temp&0xF000, mpGpioHwRegAddr + (0xE90/4));


        // enable mipi ap
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x24/4));
        usleep(30);
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));// RG_CSI1_LDO_CORE_EN
        mt65xx_reg_sync_writel(temp|0x00000002, mpCSI2RxAnalogRegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x14/4));//RG_CSI1_LDO_CORE_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x14/4));
        usleep(1);
        temp = *(mpCSI2RxAnalogRegAddr + (0x18/4));//RG_CSI1_LNRD0_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x18/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x1C/4));//RG_CSI1_LNRD1_LDO_OUT_EN
        mt65xx_reg_sync_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x1C/4));
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DBG, LNC_HSRXDB_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DBG, LN0_HSRXDB_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DBG, LN1_HSRXDB_EN, 1);
         temp = *(mpCSI2RxConfigRegAddr + (0x38/4));//MIPI_RX_HW_CAL_START
         mt65xx_reg_sync_writel(temp|0x00000004, mpCSI2RxConfigRegAddr + (0x38/4));
        LOG_MSG("[initCSI2]:CSI1 calibration start !\n");
        usleep(1000);
        while(!(*(mpCSI2RxConfigRegAddr + (0x4C/4)) & 0x10001)) {}
        LOG_MSG("[initCSI2]:CSI1 calibration end !\n");
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DBG, LNC_HSRXDB_EN, 0);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DBG, LN0_HSRXDB_EN, 0);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DBG, LN1_HSRXDB_EN, 0);
     }


    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlane_num, unsigned long csi2_en,
    unsigned long dataheaderOrder, unsigned long dataFlow
)
{
    int ret = 0,temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    if(csi2_en == 1) {  // enable CSI2
        LOG_MSG("[configTg1CSI2]:DataTermDelay:%d SettleDelay:%d ClkTermDelay:%d VsyncType:%d dlane_num:%d CSI2 enable:%d HeaderOrder:%d DataFlow:%d\n",
        	(int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlane_num, (int) csi2_en, (int)dataheaderOrder, (int)dataFlow);

        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, 0);    // disable CSI2 first
        temp = *(mpSeninfHwRegAddr + (0x104/4));
        temp = (temp & 0x0)|((dataSettleDelay&0xFF)<<16);
        LOG_MSG("[configTg2CSI2]:mt65xx_reg_sync_writel SENINF1_CSI2_DELAY = 0x%x",temp);
        mt65xx_reg_sync_writel(temp, mpSeninfHwRegAddr + (0x104/4));
        //SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DELAY, LP2HS_CLK_TERM_DELAY, clkTermDelay);
        //SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DELAY, LP2HS_DATA_TERM_DELAY, dataTermDelay);
        //SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DELAY, LP2HS_DATA_SETTLE_DELAY, dataSettleDelay);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_INTEN, CRC_ERR_IRQ_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_INTEN, ECC_ERR_IRQ_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_INTEN, ECC_CORRECT_IRQ_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_INTSTA, CSI2_IRQ_CLR_SEL, 1);     // write clear

		#if (defined(FPGA))
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL,(dataFlow<<17) | (vsyncType <<13) | (0 << 10) | (1<<5) | (1<<4) | (((1<<dlane_num)-1)<<1) | (csi2_en<<0));
		#else
        	SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL,(dataFlow<<17) | (vsyncType <<13) | (1 << 10) | (dataheaderOrder<<5) | (1<<4) | (((1<<dlane_num)-1)<<1) | (csi2_en<<0));
		#endif
    }
    else {   // disable CSI2
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, 0);
    }

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlane_num, unsigned long csi2_en,
    unsigned long dataheaderOrder, unsigned long dataFlow
)
{
    int ret = 0, temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    if(csi2_en == 1) {  // enable CSI2
        LOG_MSG("[configTg2CSI2]:DataTermDelay:%d SettleDelay:%d ClkTermDelay:%d VsyncType:%d 2Lane:%d CSI2 enable:%d HeaderOrder:%d DataFlow:%d\n",
        	(int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlane_num, (int) csi2_en, (int)dataheaderOrder, (int)dataFlow);

        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_CTRL, CSI2_EN, 0);    // disable CSI2 first
        temp = *(mpSeninfHwRegAddr + (0x184/4));
        temp = (temp & 0x0)|((dataSettleDelay&0xFF)<<16);
         LOG_MSG("[configTg2CSI2]:mt65xx_reg_sync_writel SENINF2_CSI2_DELAY = 0x%x",temp);
        mt65xx_reg_sync_writel(temp, mpSeninfHwRegAddr + (0x184/4));        
        //SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DELAY, LP2HS_CLK_TERM_DELAY, clkTermDelay);
        //SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DELAY, LP2HS_DATA_TERM_DELAY, dataTermDelay);
        //SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_DELAY, LP2HS_DATA_SETTLE_DELAY, dataSettleDelay);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_INTEN, CRC_ERR_IRQ_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_INTEN, ECC_ERR_IRQ_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_INTEN, ECC_CORRECT_IRQ_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_INTSTA, CSI2_IRQ_CLR_SEL, 1);     // write clear

        SENINF_WRITE_REG(pSeninf, SENINF2_CSI2_CTRL,(dataFlow<<17) | (vsyncType <<13) | (1 << 10) | (dataheaderOrder<<5) | (1<<4) | (1<<dlane_num) | (csi2_en<<0));
    }
    else {   // disable CSI2
        SENINF_WRITE_BITS(pSeninf, SENINF2_CSI2_CTRL, CSI2_EN, 0);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;
    unsigned int temp = 0;

    if(mpCAMIODrvRegAddr != NULL) {
        temp = *(mpCAMIODrvRegAddr);
        mt65xx_reg_sync_writel(temp&0xFFFFFF0F, mpCAMIODrvRegAddr);
        temp = *(mpCAMIODrvRegAddr) ;   // CLK  CAM1  CAM
        mt65xx_reg_sync_writel(temp|ioDrivingCurrent, mpCAMIODrvRegAddr);
    }
//    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(mpCAMIODrvRegAddr)));

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;

    if(mpCAMIODrvRegAddr != NULL) {
//        *(mpCAMIODrvRegAddr) &= 0x000FFFFF;
//        *(mpCAMIODrvRegAddr) |= ((ioDrivingCurrent<<28) |(ioDrivingCurrent<<24) | (ioDrivingCurrent<<20));   // CLK  CAM1  CAM
    }
//    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(mpCAMIODrvRegAddr)));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1MCLKEn(bool isEn)
{
    int ret = 0;

	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN, isEn);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2MCLKEn(bool isEn)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SENINF_TG2_PH_CNT, ADCLK_EN, isEn);

    return ret;
}


int SeninfDrvImp::setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount,
			unsigned long startLine, unsigned long startPixel, unsigned long  flashPol)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN, 0x0);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_POL, flashPol);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_END_FRM, endFrame);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_STARTPNT, startPoint);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT_NO, unitCount);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT, lineUnit);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_PXL, startPixel);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_LINE, startLine);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN, 0x1);

	return ret;

}


int SeninfDrvImp::setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN, 0x0);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_CONT_FRM, contiFrm);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_START_FRM, startFrame);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_STARTPNT, 0x0);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_TRIG_SRC, 0x0);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT_NO, unitCount);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT, lineUnit);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_PXL, startPixel);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_LINE, startLine);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN, 0x1);

	return ret;
}

int SeninfDrvImp::setFlashEn(bool flashEn)
{
	int ret = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_EN, flashEn);

	return ret;

}



int SeninfDrvImp::setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	if ((hsStart > 4095) || (hsEnd > 4095))
	{
		LOG_ERR("CCIR656 HSTART or HEND value err \n");
		ret = -1;
	}

	SENINF_WRITE_BITS(pSeninf, CCIR656_CTL, CCIR656_VS_POL, vsPol);
	SENINF_WRITE_BITS(pSeninf, CCIR656_CTL, CCIR656_HS_POL, hsPol);
	SENINF_WRITE_BITS(pSeninf, CCIR656_H, CCIR656_HS_END, hsEnd);
	SENINF_WRITE_BITS(pSeninf, CCIR656_H, CCIR656_HS_START, hsStart);

	return ret;
}


int SeninfDrvImp::setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	SENINF_WRITE_BITS(pSeninf, N3D_CTL, N3D_EN, n3dEn);
	SENINF_WRITE_BITS(pSeninf, N3D_CTL, I2C1_EN, i2c1En);
	SENINF_WRITE_BITS(pSeninf, N3D_CTL, I2C2_EN, i2c2En);
	SENINF_WRITE_BITS(pSeninf, N3D_CTL, MODE, n3dMode);

	return ret;
}


int SeninfDrvImp::setN3DI2CPos(unsigned long n3dPos)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	SENINF_WRITE_BITS(pSeninf, N3D_POS, N3D_POS, n3dPos);

	return ret;
}


int SeninfDrvImp::setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	SENINF_WRITE_BITS(pSeninf, N3D_TRIG, I2CA_TRIG, i2c1TrigOn);
	SENINF_WRITE_BITS(pSeninf, N3D_TRIG, I2CB_TRIG, i2c2TrigOn);

	return ret;

}

int SeninfDrvImp::checkSeninf1Input()
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;       
    
    temp = SENINF_READ_REG(pSeninf,SENINF1_DEBUG_4);
    LOG_MSG("[checkSeninf1Input]:size = 0x%x",temp);        
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;
        
    if( (tempW >= tg1GrabWidth) && (tempH >= tg1GrabHeight)  ) {
        ret = 0;
    }
    else {
        ret = 1;
    }

    return ret;

}

int SeninfDrvImp::checkSeninf2Input()
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;
    
    temp = SENINF_READ_REG(pSeninf,SENINF2_DEBUG_4);
    LOG_MSG("[checkSeninf2Input]:size = 0x%x",temp);
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;
        
    if( (tempW >= tg2GrabWidth) && (tempH >= tg2GrabHeight)  ) {
        ret = 0;
    }
    else {
        ret = 1;
    }

    return ret;

}




