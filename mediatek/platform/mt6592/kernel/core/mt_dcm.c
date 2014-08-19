#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_dcm.h>
#include <mach/mt_clkmgr.h>


#define USING_XLOG

#ifdef USING_XLOG

#include <linux/xlog.h>
#define TAG     "Power/dcm"

#define MT6592_DCM_SETTING (1)

//#define DCM_ENABLE_DCM_CFG

#define dcm_err(fmt, args...)       \
    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
#define dcm_warn(fmt, args...)      \
    xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
#define dcm_info(fmt, args...)      \
    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define dcm_dbg(fmt, args...)       \
    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define dcm_ver(fmt, args...)       \
    xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

#else /* !USING_XLOG */

#define TAG     "[Power/dcm] "

#define dcm_err(fmt, args...)       \
    printk(KERN_ERR TAG);           \
    printk(KERN_CONT fmt, ##args)
#define dcm_warn(fmt, args...)      \
    printk(KERN_WARNING TAG);       \
    printk(KERN_CONT fmt, ##args)
#define dcm_info(fmt, args...)      \
    printk(KERN_NOTICE TAG);        \
    printk(KERN_CONT fmt, ##args)
#define dcm_dbg(fmt, args...)       \
    printk(KERN_INFO TAG);          \
    printk(KERN_CONT fmt, ##args)
#define dcm_ver(fmt, args...)       \
    printk(KERN_DEBUG TAG);         \
    printk(KERN_CONT fmt, ##args)

#endif


#define dcm_readl(addr)         DRV_Reg32(addr)


#define dcm_writel(addr, val)   mt65xx_reg_sync_writel((val), ((void *)addr))

#define dcm_setl(addr, val)     mt65xx_reg_sync_writel(dcm_readl(addr) | (val), ((void *)addr))

#define dcm_clrl(addr, val)     mt65xx_reg_sync_writel(dcm_readl(addr) & ~(val), ((void *)addr))


static DEFINE_MUTEX(dcm_lock);

static unsigned int dcm_sta = 0;

void dcm_dump_regs(unsigned int type)
{
   //volatile unsigned int dcm_cfg;

    mutex_lock(&dcm_lock);


    if (type & CPU_DCM) {
        volatile unsigned int l2c_sram_ctrl, cci_clk_ctrl;

        l2c_sram_ctrl = dcm_readl(L2C_SRAM_CTRL);
        cci_clk_ctrl  = dcm_readl(CCI_CLK_CTRL);

        dcm_info("[CPU_DCM]L2C_SRAM_CTRL(0x%08x)\n",  l2c_sram_ctrl);
        dcm_info("[CPU_DCM]CCI_CLK_CTRL (0x%08x)\n",  cci_clk_ctrl);
    }

#if 0
	if (type & TOPCKGEN_DCM) {
		volatile unsigned int dcm_cfg;
		dcm_cfg = dcm_readl(DCM_CFG);

        dcm_info("[IFR_DCM]DCM_CFG(0x%08x)\n", dcm_cfg);
    }
#endif

    if (type & IFR_DCM) {
        volatile unsigned int ca7_ckdiv1, infra_topckgen_dcmctl, infra_topckgen_dcmdbc;
        volatile unsigned int infra_globalcon_dcmctl,infra_globalcon_dcmdbc;
        volatile unsigned int infra_globalcon_dcmsel,dramc;
        ca7_ckdiv1 			   = dcm_readl(CA7_CKDIV1);
        infra_topckgen_dcmctl  = dcm_readl(INFRA_TOPCKGEN_DCMCTL);
        infra_topckgen_dcmdbc  = dcm_readl(INFRA_TOPCKGEN_DCMDBC);
        infra_globalcon_dcmctl = dcm_readl(INFRA_GLOBALCON_DCMCTL);
        infra_globalcon_dcmdbc = dcm_readl(INFRA_GLOBALCON_DCMDBC);
        infra_globalcon_dcmsel = dcm_readl(INFRA_GLOBALCON_DCMSEL);
        dramc 				   = dcm_readl(DRAMC_PD_CTRL);

        dcm_info("[IFR_DCM]CA7_CKDIV1(0x%08x)\n", ca7_ckdiv1);
        dcm_info("[IFR_DCM]INFRA_TOPCKGEN_DCMCTL (0x%08x)\n", infra_topckgen_dcmctl);
        dcm_info("[IFR_DCM]INFRA_TOPCKGEN_DCMDBC (0x%08x)\n", infra_topckgen_dcmdbc);
        dcm_info("[IFR_DCM]INFRA_GLOBALCON_DCMCTL(0x%08x)\n", infra_globalcon_dcmctl);
        dcm_info("[IFR_DCM]INFRA_GLOBALCON_DCMDBC(0x%08x)\n", infra_globalcon_dcmdbc);
        dcm_info("[IFR_DCM]INFRA_GLOBALCON_DCMSEL(0x%08x)\n", infra_globalcon_dcmsel);
        dcm_info("[IFR_DCM]DRAMC_PD_CTRL         (0x%08x)\n", dramc);

    }



    if (type & PER_DCM) {
        volatile unsigned int peri_globalcon_dcmctl;
        volatile unsigned int peri_globalcon_dcmdbc, peri_globalcon_dcmsel;
        volatile unsigned int msdc0_ip_dcm,msdc1_ip_dcm,msdc2_ip_dcm,msdc3_ip_dcm;
		volatile unsigned int usb0_dcm,pmic_wrap_dcm_en;
        volatile unsigned int i2c0_i2creg_hw_cg_en,i2c1_i2creg_hw_cg_en,i2c2_i2creg_hw_cg_en;

        peri_globalcon_dcmctl = dcm_readl(PERI_GLOBALCON_DCMCTL);
        peri_globalcon_dcmdbc = dcm_readl(PERI_GLOBALCON_DCMDBC);
        peri_globalcon_dcmsel = dcm_readl(PERI_GLOBALCON_DCMFSEL);

        msdc0_ip_dcm = dcm_readl(MSDC0_IP_DCM);
		msdc1_ip_dcm = dcm_readl(MSDC1_IP_DCM);
		msdc2_ip_dcm = dcm_readl(MSDC2_IP_DCM);
		msdc3_ip_dcm = dcm_readl(MSDC3_IP_DCM);

		usb0_dcm = dcm_readl(USB0_DCM);

		pmic_wrap_dcm_en = dcm_readl(PMIC_WRAP_DCM_EN);

		i2c0_i2creg_hw_cg_en = dcm_readl(I2C0_I2CREG_HW_CG_EN);
		i2c1_i2creg_hw_cg_en = dcm_readl(I2C1_I2CREG_HW_CG_EN);
		i2c2_i2creg_hw_cg_en = dcm_readl(I2C2_I2CREG_HW_CG_EN);

        dcm_info("[PER_DCM]PERI_GLOBALCON_DCMCTL  (0x%08x)\n", peri_globalcon_dcmctl);
        dcm_info("[PER_DCM]PERI_GLOBALCON_DCMDBC  (0x%08x)\n", peri_globalcon_dcmdbc);
        dcm_info("[PER_DCM]PERI_GLOBALCON_DCMFSEL (0x%08x)\n", peri_globalcon_dcmsel);

        dcm_info("[PER_DCM]MSDC0_IP_DCM (0x%08x)\n", msdc0_ip_dcm);
        dcm_info("[PER_DCM]MSDC1_IP_DCM (0x%08x)\n", msdc1_ip_dcm);
        dcm_info("[PER_DCM]MSDC2_IP_DCM (0x%08x)\n", msdc2_ip_dcm);
        dcm_info("[PER_DCM]MSDC3_IP_DCM (0x%08x)\n", msdc3_ip_dcm);

        dcm_info("[PER_DCM]USB0_DCM     (0x%08x)\n", usb0_dcm);

        dcm_info("[PER_DCM]PMIC_WRAP_DCM_EN (0x%08x)\n", pmic_wrap_dcm_en);

        dcm_info("[PER_DCM]I2C0_I2CREG_HW_CG_EN (0x%08x)\n", i2c0_i2creg_hw_cg_en);
        dcm_info("[PER_DCM]I2C1_I2CREG_HW_CG_EN (0x%08x)\n", i2c1_i2creg_hw_cg_en);
        dcm_info("[PER_DCM]I2C2_I2CREG_HW_CG_EN (0x%08x)\n", i2c2_i2creg_hw_cg_en);

    }

    if (type & SMI_DCM) {

        volatile unsigned int smi_dcm_control,smi_com_set,mmu_dcm;

		smi_dcm_control = dcm_readl(SMI_DCM_CONTROL);


        smi_com_set = dcm_readl(SMI_CON_SET);

        //m4u_dcm
        mmu_dcm = dcm_readl(MMU_DCM);

        dcm_info("[PER_DCM]SMI_DCM_CONTROL  (0x%08x)\n", smi_dcm_control);
        dcm_info("[PER_DCM]SMI_CON_SET 		(0x%08x)\n", smi_com_set);
        dcm_info("[PER_DCM]MMU_DCM 		    (0x%08x)\n", mmu_dcm);

		}


    if (type & MFG_DCM) {
        if (subsys_is_on(SYS_MFG)) {
            volatile unsigned int mfg0;
            mfg0 = dcm_readl(MFG_DCM_CON_0);
            dcm_info("[MFG_DCM]MFG_DCM_CON_0(0x%08x)\n",mfg0);
        } else {
            dcm_info("[MFG_DCM]subsy MFG is off\n");
        }
    }


    if (type & DIS_DCM) {
        if (subsys_is_on(SYS_DIS)) {
            volatile unsigned int dis0, dis_set0, dis_clr0,dis1, dis_set1, dis_clr1;
            volatile unsigned int smilarb0_dcm_sta,smilarb0_dcm_con,smilarb0_dcm_set,smilarb0_dcm_clr;
            dis0 	 = dcm_readl(MMSYS_HW_DCM_DIS0);
			dis_set0 = dcm_readl(MMSYS_HW_DCM_DIS_SET0);
			dis_clr0 = dcm_readl(MMSYS_HW_DCM_DIS_CLR0);


            dis1 	 = dcm_readl(MMSYS_HW_DCM_DIS1);
			dis_set1 = dcm_readl(MMSYS_HW_DCM_DIS_SET1);
			dis_clr1 = dcm_readl(MMSYS_HW_DCM_DIS_CLR1);
			smilarb0_dcm_sta = dcm_readl(SMI_LARB0_STA);
			smilarb0_dcm_con = dcm_readl(SMI_LARB0_CON);
            smilarb0_dcm_set = dcm_readl(SMI_LARB0_CON_SET);
            smilarb0_dcm_clr = dcm_readl(SMI_LARB0_CON_CLR);

			dcm_info("[DIS_DCM]MMSYS_HW_DCM_DIS0 	 (0x%08x)\n", dis0);
            dcm_info("[DIS_DCM]MMSYS_HW_DCM_DIS_SET0 (0x%08x)\n", dis_set0);
            dcm_info("[DIS_DCM]MMSYS_HW_DCM_DIS_CLR0 (0x%08x)\n", dis_clr0);
            dcm_info("[DIS_DCM]MMSYS_HW_DCM_DIS1     (0x%08x)\n", dis1);
            dcm_info("[DIS_DCM]MMSYS_HW_DCM_DIS_SET1 (0x%08x)\n", dis_set1);
            dcm_info("[DIS_DCM]MMSYS_HW_DCM_DIS_CLR1 (0x%08x)\n", dis_clr1);
            dcm_info("[DIS_DCM]SMI_LARB0_STA (0x%08x)\n", smilarb0_dcm_sta);
			dcm_info("[DIS_DCM]SMI_LARB0_CON (0x%08x)\n", smilarb0_dcm_con);
			dcm_info("[DIS_DCM]SMI_LARB0_CON_SET (0x%08x)\n", smilarb0_dcm_set);
			dcm_info("[DIS_DCM]SMI_LARB0_CON_CLR (0x%08x)\n", smilarb0_dcm_clr);
        } else {
            dcm_info("[DIS_DCM]subsys DIS is off\n");
        }
    }

    if (type & ISP_DCM) {
        if (subsys_is_on(SYS_ISP)) {
            volatile unsigned int cam_ctl_raw_dcm_dis, cam_ctl_rgb_dcm_dis, cam_ctl_yuv_dcm_dis;
            volatile unsigned int cam_ctl_cdp_dcm_dis,cam_ctl_dma_dcm_dis;
            volatile unsigned int venc_cg_ctrl,venc_ce,venc_clk_dcm_ctrl,jpgenc,smi_larb2_con_set;

			cam_ctl_raw_dcm_dis = dcm_readl(CAM_CTL_RAW_DCM_DIS);
			cam_ctl_rgb_dcm_dis = dcm_readl(CAM_CTL_RGB_DCM_DIS);
            cam_ctl_yuv_dcm_dis = dcm_readl(CAM_CTL_YUV_DCM_DIS);
            cam_ctl_cdp_dcm_dis = dcm_readl(CAM_CTL_CDP_DCM_DIS);
			cam_ctl_dma_dcm_dis = dcm_readl(CAM_CTL_DMA_DCM_DIS);

			venc_cg_ctrl = dcm_readl(VENC_CLK_CG_CTRL);
			venc_ce 	 = dcm_readl(VENC_CE);
            venc_clk_dcm_ctrl = dcm_readl(VENC_CLK_DCM_CTRL);
            jpgenc       = dcm_readl(JPGENC_DCM_CTRL);

            smi_larb2_con_set   = dcm_readl(SMI_LARB2_CON_SET);

            dcm_info("[ISP_DCM]CAM_CTL_RAW_DCM_DIS (0x%08x)\n", cam_ctl_raw_dcm_dis);
			dcm_info("[ISP_DCM]CAM_CTL_RGB_DCM_DIS (0x%08x)\n", cam_ctl_rgb_dcm_dis);
			dcm_info("[ISP_DCM]CAM_CTL_YUV_DCM_DIS (0x%08x)\n", cam_ctl_yuv_dcm_dis);
			dcm_info("[ISP_DCM]CAM_CTL_CDP_DCM_DIS (0x%08x)\n", cam_ctl_cdp_dcm_dis);
			dcm_info("[ISP_DCM]CAM_CTL_DMA_DCM_DIS (0x%08x)\n", cam_ctl_dma_dcm_dis);

            dcm_info("[ISP_DCM]VENC_CLK_CG_CTRL (0x%08x)\n", venc_cg_ctrl);
			dcm_info("[ISP_DCM]VENC_CE 			(0x%08x)\n", venc_ce);
			dcm_info("[ISP_DCM]VENC_CLK_DCM_CTRL (0x%08x)\n", venc_clk_dcm_ctrl);
			dcm_info("[ISP_DCM]JPGENC_DCM_CTRL  (0x%08x)\n", jpgenc);

			dcm_info("[ISP_DCM]SMI_LARB2_CON_SET  (0x%08x)\n", smi_larb2_con_set);
        } else {
            dcm_info("[ISP_DCM]subsys ISP is off\n");
        }
    }
			dcm_clrl   (VDEC_DCM_CON          , 0x00000001); //0x16000018

			dcm_setl   (SMI_LARB1_CON_SET     , 0x00008000); //0x16010014



    if (type & VDE_DCM) {
        if (subsys_is_on(SYS_VDE)) {
            volatile unsigned int vdec_dcm_con,smi_larb1_con_set;

            vdec_dcm_con = dcm_readl(VDEC_DCM_CON);
            smi_larb1_con_set = dcm_readl(SMI_LARB1_CON_SET);

			dcm_info("[VDE_DCM]VDEC_DCM_CON       (0x%08x)\n", vdec_dcm_con);
            dcm_info("[VDE_DCM]SMI_LARB1_CON_SET  (0x%08x)\n", smi_larb1_con_set);
        } else {
            dcm_info("[VDE_DCM]subsys VDE is off\n");
        }
    }

	if (type & MJC_DCM) {
        if (subsys_is_on(SYS_MJC)) {
			volatile unsigned int mjc_hw_dcm_dis,mjc_hw_dcm_dis_set,mjc_hw_dcm_dis_clr;
			mjc_hw_dcm_dis 	   = dcm_readl(MJC_HW_DCM_DIS);
	   		mjc_hw_dcm_dis_set = dcm_readl(MJC_HW_DCM_DIS_SET);
	        mjc_hw_dcm_dis_clr = dcm_readl(MJC_HW_DCM_DIS_CLR);

	        dcm_info("[MJC_DCM]MJC_HW_DCM_DIS    (0x%08x)\n", mjc_hw_dcm_dis);
			dcm_info("[MJC_DCM]MJC_HW_DCM_DIS_SET(0x%08x)\n", mjc_hw_dcm_dis_set);
			dcm_info("[MJC_DCM]MJC_HW_DCM_DIS_CLR(0x%08x)\n", mjc_hw_dcm_dis_clr);
   	 	}else {
            dcm_info("[SYS_MJC]subsys MJC is off\n");
        }
    }
    mutex_unlock(&dcm_lock);

}

void dcm_enable(unsigned int type)
{

#if 1

    dcm_info("[%s]type:0x%08x\n", __func__, type);

    mutex_lock(&dcm_lock);

    if (type & CPU_DCM) {
        dcm_info("[%s][CPU_DCM     ]=0x%08x\n", __func__,CPU_DCM);

		dcm_setl   (L2C_SRAM_CTRL         , 0x00000001); //0x10200248
		dcm_setl   (CCI_CLK_CTRL          , 0x00000100); //0x10200270

        dcm_sta |= CPU_DCM;
    }

#if 0 //because in 92 there is no register need to be set in TOPCKGEN
	if (type & TOPCKGEN_DCM) {
        dcm_info("[%s][TOPCKGEN_DCM]=0x%08x\n", __func__,TOPCKGEN_DCM);

        #ifdef DCM_ENABLE_DCM_CFG //AXI bus dcm, don't need to set by KL Tong
        //default value are all 0,use default value
        dcm_writel(DCM_CFG, 0xFFFFFF7F);//set bit0~bit4=0,bit7=0,bit8~bit14=0,bit15=0????
        #endif
    	dcm_sta |= TOPCKGEN_DCM;

    }
#endif

	//Infrasys_dcm
    if (type & IFR_DCM) {
        dcm_info("[%s][IFR_DCM     ]=0x%08x\n", __func__,IFR_DCM);

		dcm_clrl   (CA7_CKDIV1            , 0x0000001F); //0x10001008//5'h0,00xxx: 1/1
		dcm_setl   (INFRA_TOPCKGEN_DCMCTL , 0x00000007); //0x10001010//set bit0~bit2=1
		dcm_setl   (INFRA_TOPCKGEN_DCMDBC , 0x00000001); //0x10001014//set bit0=1, force to 26M
		dcm_setl   (INFRA_GLOBALCON_DCMCTL, 0x00000303); //0x10001050//set bit0,bit1,bit8,bit9=1,DCM debouncing counter=0
		dcm_setl   (INFRA_GLOBALCON_DCMDBC, 0x00000300); //0x10001054//set bit8,bit9=1 first
		dcm_clrl   (INFRA_GLOBALCON_DCMDBC, 0x0000007F); //0x10001054//then clear b0~b6

		#if 0  // divided most, save power,clear bit0~bit2
		dcm_clrl   (INFRA_GLOBALCON_DCMSEL, 0x001F0F07); //0x10001058   //clear bit0~bit2,clear bit8~bit11,clear bit16~bit20
        #else  //defaule value //div 1
//		dcm_writel (INFRA_GLOBALCON_DCMSEL, 0xFFF0F0F8); //ref 6582
		dcm_setl   (INFRA_GLOBALCON_DCMSEL, 0x00100000); //0x10001058,set INFRA_GLOBALCON_DCMSEL[20:16]=5'h10,1xxxx:1/1
		dcm_clrl   (INFRA_GLOBALCON_DCMSEL, 0x000F0F07); //0x10001058

		#endif

		dcm_setl   (DRAMC_PD_CTRL         , 0x03000000); //0x100041DC//set bit24,bit25=1

		dcm_sta |= IFR_DCM;

    }

    if (type & PER_DCM) {
        dcm_info("[%s][PER_DCM     ]=0x%08x\n", __func__,PER_DCM);

		dcm_clrl   (PERI_GLOBALCON_DCMCTL , 0x00001F00); //0x10003050//clear bit8~bit12=0
        dcm_setl   (PERI_GLOBALCON_DCMCTL , 0x000000F3); //0x10003050//set bit0,bit1,bit4~bit7=1

		dcm_setl   (PERI_GLOBALCON_DCMDBC , 0x000000F0); //0x10003054//set bit7=1,clear bit0~bit6=0x70

		dcm_clrl   (PERI_GLOBALCON_DCMFSEL, 0x001F0F07); //0x10003058//clear bit0~bit2,bit8~bit11,bit16~bit20

		//MSDC module
		dcm_clrl   (MSDC0_IP_DCM          , 0xFF800000); //0x112300B4//clear bit23~bit31=0
		dcm_clrl   (MSDC1_IP_DCM          , 0xFF800000); //0x112400B4//clear bit23~bit31=0
		dcm_clrl   (MSDC2_IP_DCM          , 0xFF800000); //0x112500B4//clear bit23~bit31=0
		dcm_clrl   (MSDC3_IP_DCM          , 0xFF800000); //0x112600B4//clear bit23~bit31=0

		//USB
		dcm_clrl   (USB0_DCM              , 0x00070000); //0x11200700//clear bit16~bit18=0

        //PMIC
		dcm_writel (PMIC_WRAP_DCM_EN      , 0x00000001); //0x1000D13C//set bit0=1

		//I2C
        dcm_setl   (I2C0_I2CREG_HW_CG_EN  , 0x00000001); //0x11007054//set bit0=1
		dcm_setl   (I2C1_I2CREG_HW_CG_EN  , 0x00000001); //0x11008054//set bit0=1
		dcm_setl   (I2C2_I2CREG_HW_CG_EN  , 0x00000001); //0x11009054//set bit0=1


        dcm_sta |= PER_DCM;

    }
    if (type & SMI_DCM) {

        dcm_info("[%s][SMI_DCM     ]=0x%08x\n", __func__,SMI_DCM);

		dcm_writel (SMI_DCM_CONTROL       , 0x00000001); //0x14011300//set bit 0=1
		//RU dcm_setl   (SMI_CON               , 0x00000004); //0x1000C010
		dcm_setl   (SMI_CON_SET           , 0x00000004); //0x1000C014

		//m4u_dcm
		dcm_setl   (MMU_DCM               , 0x00000001); //0x102055F0//set bit0=1

        dcm_sta |= SMI_DCM;

    }

    if (type & MFG_DCM) {
		dcm_info("[%s][MFG_DCM     ]=0x%08x,subsys_is_on(SYS_MFG)=%d\n", __func__,MFG_DCM,subsys_is_on(SYS_MFG));

        if (subsys_is_on(SYS_MFG)) {


			dcm_setl   (MFG_DCM_CON_0         , 0x0001C03F); //0x13000010
			dcm_clrl   (MFG_DCM_CON_0         , 0x00003F40); //0x13000010

            dcm_sta |= MFG_DCM;
        }
    }

    if (type & DIS_DCM) {
		dcm_info("[%s][DIS_DCM     ]=0x%08x,subsys_is_on(SYS_DIS)=%d\n", __func__,DIS_DCM,subsys_is_on(SYS_DIS));

        if (subsys_is_on(SYS_DIS)) {

			dcm_writel (MMSYS_HW_DCM_DIS0     , 0x00000000); //0x14000120
			dcm_writel (MMSYS_HW_DCM_DIS_SET0 , 0x00000000); //0x14000124
			dcm_writel (MMSYS_HW_DCM_DIS_CLR0 , 0xFFFFFFFF); //0x14000128

			dcm_writel (MMSYS_HW_DCM_DIS1     , 0x00000000); //0x1400012C
			dcm_writel (MMSYS_HW_DCM_DIS_SET1 , 0x00000000); //0x14000130
			dcm_writel (MMSYS_HW_DCM_DIS_CLR1 , 0xFFFFFFFF); //0x14000134

			dcm_setl   (SMI_LARB0_CON_SET     , 0x00008000); //0x14010014//set bit15=1


            dcm_sta |= DIS_DCM;
        }

    }

    if (type & ISP_DCM) {

        dcm_info("[%s][ISP_DCM     ]=0x%08x,subsys_is_on(SYS_ISP)=%d\n", __func__,ISP_DCM,subsys_is_on(SYS_ISP));

        if (subsys_is_on(SYS_ISP)) {

			dcm_clrl   (CAM_CTL_RAW_DCM_DIS   , 0x00007FFF); //0x15004190//set bit0~bit14=0
			dcm_clrl   (CAM_CTL_RGB_DCM_DIS   , 0x000001FF); //0x15004194//set bit0~bit8=0
			dcm_clrl   (CAM_CTL_YUV_DCM_DIS   , 0x0000000F); //0x15004198//set bit0~bit3=0
			dcm_clrl   (CAM_CTL_CDP_DCM_DIS   , 0x000001FF); //0x1500419C//set bit0~bit8=0
			dcm_clrl   (CAM_CTL_DMA_DCM_DIS   , 0x0000003F); //0x150041B0//set bit0~bit5=0

			dcm_setl   (VENC_CLK_CG_CTRL      , 0xFFFFFFFF); //0x15009094
			dcm_clrl   (VENC_CE               , 0x00000001); //0x150090EC//no need to set???
			dcm_setl   (VENC_CLK_DCM_CTRL     , 0x00000001); //0x150090F4
			dcm_clrl   (JPGENC_DCM_CTRL       , 0x00000001); //0x1500A300//clear bit0=0

			dcm_setl   (SMI_LARB2_CON_SET     , 0x00008000); //0x15001014//set bit15=1

            dcm_sta |= ISP_DCM;

        }

    }

    if (type & VDE_DCM) {

		dcm_info("[%s][VDE_DCM     ]=0x%08x,subsys_is_on(SYS_VDE)=%d\n", __func__,VDE_DCM,subsys_is_on(SYS_VDE));

        if (subsys_is_on(SYS_VDE)) {

			dcm_clrl   (VDEC_DCM_CON          , 0x00000001); //0x16000018

			dcm_setl   (SMI_LARB1_CON_SET     , 0x00008000); //0x16010014

            dcm_sta |= VDE_DCM;
        }

    }

    if (type & MJC_DCM) { //improve video record resloution
        if (subsys_is_on(SYS_MJC)) {
			dcm_writel (MJC_HW_DCM_DIS        , 0x00000000); //0x17000010
			dcm_writel (MJC_HW_DCM_DIS_SET    , 0x00000000); //0x17000014
			dcm_writel (MJC_HW_DCM_DIS_CLR    , 0x00000000); //0x17000018
			dcm_sta |= MJC_DCM;
       	}
	}


    mutex_unlock(&dcm_lock);
#endif
}

void dcm_disable(unsigned int type)
{
#if 1 //Jerry

//	volatile unsigned int temp;

    dcm_info("[%s]type:0x%08x\n", __func__, type);

    mutex_lock(&dcm_lock);
    //dcm_sta |= type & ALL_DCM;

    if (type & CPU_DCM) {

        dcm_info("[%s][CPU_DCM     ]=0x%08x\n", __func__,CPU_DCM);


		dcm_clrl   (L2C_SRAM_CTRL         , 0x00000001); //0x1020005C
		dcm_clrl   (CCI_CLK_CTRL          , 0x00000100); //0x10208000

        dcm_sta &= ~CPU_DCM;
    }

#if 0
	if (type & TOPCKGEN_DCM) {

        dcm_info("[%s][TOPCKGEN_DCM]=0x%08x\n", __func__,TOPCKGEN_DCM);
        #ifdef DCM_ENABLE_DCM_CFG //AXI bus dcm, don't need to set by KL Tong
        //default value are all 0,use default value
        dcm_clrl(DCM_CFG, (0x1 <<7 ));//set bit7=0
        #endif
        dcm_setl(CLK_SCP_CFG_0, 0x3FF);//set bit0~bit9=1,SCP control register 1
        dcm_setl(CLK_SCP_CFG_1, ((0x1 << 4) | 0x1));//set bit0=1 and bit4=1,SCP control register 1
    	dcm_sta &= ~TOPCKGEN_DCM;
    }
#endif

    if (type & PER_DCM) {
        dcm_info("[%s][PER_DCM     ]=0x%08x\n", __func__,PER_DCM);

		dcm_clrl   (PERI_GLOBALCON_DCMCTL , 0x00001FF3); //0x10003050

		dcm_setl   (PERI_GLOBALCON_DCMDBC , 0x000000F0); //0x10003054

		dcm_clrl   (PERI_GLOBALCON_DCMFSEL, 0x001F0F07); //0x10003058

		//MSDC module
		dcm_setl   (MSDC0_IP_DCM          , 0xFF800000); //0x112300B4
		dcm_setl   (MSDC1_IP_DCM          , 0xFF800000); //0x112400B4
		dcm_setl   (MSDC2_IP_DCM          , 0xFF800000); //0x112500B4

        //USB
		dcm_setl   (USB0_DCM              , 0x00070000); //0x11200700

		//PMIC
		dcm_clrl   (PMIC_WRAP_DCM_EN      , 0x00000001); //0x1000D13C

		//I2C
		dcm_clrl   (I2C0_I2CREG_HW_CG_EN  , 0x00000001); //0x11007054
		dcm_clrl   (I2C1_I2CREG_HW_CG_EN  , 0x00000001); //0x11008054
		dcm_clrl   (I2C2_I2CREG_HW_CG_EN  , 0x00000001); //0x11009054

        dcm_sta &= ~PER_DCM;
    }


	//Infrasys_dcm
    if (type & IFR_DCM) {
		dcm_info("[%s][IFR_DCM     ]=0x%08x\n", __func__,IFR_DCM);


		/*should off DRAMC first than off TOP_DCMCTL*/
		dcm_setl   (DRAMC_PD_CTRL         , 0x01000000); //0x100041DC
		dcm_clrl   (DRAMC_PD_CTRL         , 0x02000000); //0x100041DC


		//dcm_clrl   (INFRA_TOPCKGEN_DCMCTL , 0x00000007); //0x10001010
		dcm_clrl   (INFRA_TOPCKGEN_DCMCTL , 0x00000006); //0x10001010  ????????
		dcm_clrl   (INFRA_TOPCKGEN_DCMDBC , 0x00000001); //0x10001014  ????????
		dcm_clrl   (INFRA_GLOBALCON_DCMCTL, 0x00000303); //0x10001050


		dcm_sta &= ~IFR_DCM;

    }
    if (type & SMI_DCM) {
		dcm_info("[%s][SMI_DCM     ]=0x%08x\n", __func__,SMI_DCM);

		dcm_clrl   (SMI_DCM_CONTROL           , 0x00000001); //0x14011300
//?????		dcm_clrl   (SMI_CON               , 0x00000004); //0x1000C010 ??????

		dcm_setl   (SMI_CON_CLR           , 0x00000004); //0x1000C018

		//m4u_dcm
		dcm_clrl   (MMU_DCM               , 0x00000001); //0x102055F0

        dcm_sta &= ~SMI_DCM;
    }

    if (type & MFG_DCM) {

        dcm_info("[%s][MFG_DCM     ]=0x%08x\n", __func__,MFG_DCM);

		dcm_clrl   (MFG_DCM_CON_0         , 0x00008000); //0x13000010

        dcm_sta &= ~MFG_DCM;
    }

    if (type & DIS_DCM) {

		dcm_info("[%s][DIS_DCM     ]=0x%08x\n", __func__,DIS_DCM);

		dcm_writel (MMSYS_HW_DCM_DIS0     , 0xFFFFFFFF); //0x14000120
		dcm_writel (MMSYS_HW_DCM_DIS_SET0 , 0xFFFFFFFF); //0x14000124
		dcm_writel (MMSYS_HW_DCM_DIS_CLR0 , 0x00000000); //0x14000128


		dcm_writel (MMSYS_HW_DCM_DIS1     , 0xFFFFFFFF); //0x1400012C
		dcm_writel (MMSYS_HW_DCM_DIS_SET1 , 0xFFFFFFFF); //0x14000130
		dcm_writel (MMSYS_HW_DCM_DIS_CLR1 , 0x00000000); //0x14000134

		dcm_setl   (SMI_LARB0_CON_CLR     , 0x00008000); //0x14010018



        dcm_sta &= ~DIS_DCM;
    }

    if (type & ISP_DCM) {

		dcm_info("[%s][ISP_DCM     ]=0x%08x\n", __func__,ISP_DCM);

		dcm_setl   (CAM_CTL_RAW_DCM_DIS   , 0x00007FFF); //0x15004190
		dcm_setl   (CAM_CTL_RGB_DCM_DIS   , 0x000001FF); //0x15004194
		dcm_setl   (CAM_CTL_YUV_DCM_DIS   , 0x0000000F); //0x15004198
		dcm_setl   (CAM_CTL_CDP_DCM_DIS   , 0x000001FF); //0x1500419C
		dcm_setl   (CAM_CTL_DMA_DCM_DIS   , 0x0000003F); //0x150041B0

		dcm_setl   (JPGENC_DCM_CTRL       , 0x00000001); //0x1500A300


		dcm_clrl   (VENC_CLK_DCM_CTRL     , 0x00000001); //0x150090F4
		dcm_writel (VENC_CLK_CG_CTRL      , 0x00000000); //0x15009094

		dcm_setl   (SMI_LARB2_CON_CLR     , 0x00008000); //0x15001018


        dcm_sta &= ~ISP_DCM;
    }

    if (type & VDE_DCM) {

    	dcm_info("[%s][VDE_DCM     ]=0x%08x\n", __func__,VDE_DCM);

		dcm_setl   (VDEC_DCM_CON          , 0x00000001); //0x16000018

		//LARB1 „³ VDEC
		dcm_setl   (SMI_LARB1_CON_CLR     , 0x00008000); //0x16010018



        //dcm_writel(SMILARB1_DCM_SET, 0x3 << 15);

        dcm_sta &= ~VDE_DCM;
    }

	if (type & VDE_DCM) {
		dcm_writel (MJC_HW_DCM_DIS        , 0x0000000F); //0x17000010
		dcm_sta &= ~MJC_DCM;
	}

    mutex_unlock(&dcm_lock);
#endif
}

void bus_dcm_enable(void)
{
    dcm_writel(DCM_CFG, 0x1 << 7 | 0xF);
}

void bus_dcm_disable(void)
{
    dcm_clrl(DCM_CFG, 0x1 << 7);
}

static unsigned int infra_dcm = 0;
void disable_infra_dcm(void)
{
    infra_dcm = dcm_readl(INFRA_GLOBALCON_DCMCTL);
    dcm_clrl(INFRA_GLOBALCON_DCMCTL, 0x100);
}

void restore_infra_dcm(void)
{
    dcm_writel(INFRA_GLOBALCON_DCMCTL, infra_dcm);
}

static unsigned int peri_dcm = 0;
void disable_peri_dcm(void)
{
    peri_dcm = dcm_readl(PERI_GLOBALCON_DCMCTL);
    dcm_clrl(PERI_GLOBALCON_DCMCTL, 0x1);
}

void restore_peri_dcm(void)
{
    dcm_writel(PERI_GLOBALCON_DCMCTL, peri_dcm);
}

#define dcm_attr(_name)                         \
static struct kobj_attribute _name##_attr = {   \
    .attr = {                                   \
        .name = __stringify(_name),             \
        .mode = 0644,                           \
    },                                          \
    .show = _name##_show,                       \
    .store = _name##_store,                     \
}

static const char *dcm_name[NR_DCMS] = {
    "CPU_DCM",
    "IFR_DCM",
    "PER_DCM",
    "SMI_DCM",
    "MFG_DCM",
    "DIS_DCM",
    "ISP_DCM",
    "VDE_DCM",
    "MJC_DCM",
};

static ssize_t dcm_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int i;
    unsigned int sta;

    p += sprintf(p, "********** dcm_state dump **********\n");
    mutex_lock(&dcm_lock);

    for (i = 0; i < NR_DCMS; i++) {
        sta = dcm_sta & (0x1 << i);
        p += sprintf(p, "[%d][%s]%s\n", i, dcm_name[i], sta ? "on" : "off");
    }

    mutex_unlock(&dcm_lock);

    p += sprintf(p, "\n********** dcm_state help *********\n");
    p += sprintf(p, "enable dcm:    echo enable mask(dec) > /sys/power/dcm_state\n");
    p += sprintf(p, "disable dcm:   echo disable mask(dec) > /sys/power/dcm_state\n");
    p += sprintf(p, "dump reg:      echo dump mask(dec) > /sys/power/dcm_state\n");


    len = p - buf;
    return len;
}

static ssize_t dcm_state_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t n)
{
    char cmd[10];
    unsigned int mask;

    if (sscanf(buf, "%s %x", cmd, &mask) == 2) {
        mask &= ALL_DCM;

        /*
		Need to enable MM clock before setting Smi_secure register
		to avoid system crash while screen is off(screen off with USB cable)
		*/
        enable_mux(MT_MUX_MM, "DCM");

        if (!strcmp(cmd, "enable")) {
            dcm_dump_regs(mask);
            dcm_enable(mask);
            dcm_dump_regs(mask);
        } else if (!strcmp(cmd, "disable")) {
            dcm_dump_regs(mask);
            dcm_disable(mask);
            dcm_dump_regs(mask);
        } else if (!strcmp(cmd, "dump")) {
            dcm_dump_regs(mask);
        }

	    disable_mux(MT_MUX_MM, "DCM");

        return n;
    }

    return -EINVAL;
}
dcm_attr(dcm_state);


void mt_dcm_init(void)
{
    int err = 0;

    dcm_info("[%s]entry!!,ALL_DCM=%d\n", __func__,ALL_DCM);
    dcm_enable(ALL_DCM);
    //dcm_enable(ALL_DCM & (~IFR_DCM));

    err = sysfs_create_file(power_kobj, &dcm_state_attr.attr);

    if (err) {
        dcm_err("[%s]: fail to create sysfs\n", __func__);
    }
}
