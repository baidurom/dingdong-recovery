/*******************************************************************************************/


/*******************************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <asm/system.h>

#include <linux/proc_fs.h> 


#include <linux/dma-mapping.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov12830trulybmipiraw_Sensor.h"
#include "ov12830trulybmipiraw_Camera_Sensor_para.h"
#include "ov12830trulybmipiraw_CameraCustomized.h"
static DEFINE_SPINLOCK(ov12830trulybmipiraw_drv_lock);

#define OV12830TRULYB_DEBUG
//#define OV12830TRULYB_DEBUG_SOFIA

#ifdef OV12830TRULYB_DEBUG
	#define OV12830TRULYBDB(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[OV12830TRULYBRaw] ",  fmt, ##arg)
#else
	#define OV12830TRULYBDB(fmt, arg...)
#endif

#ifdef OV12830TRULYB_DEBUG_SOFIA
	#define OV12830TRULYBDBSOFIA(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[OV12830TRULYBRaw] ",  fmt, ##arg)
#else
	#define OV12830TRULYBDBSOFIA(fmt, arg...)
#endif

#define mDELAY(ms)  mdelay(ms)

kal_uint32 OV12830TRULYB_FeatureControl_PERIOD_PixelNum=OV12830TRULYB_PV_PERIOD_PIXEL_NUMS;
kal_uint32 OV12830TRULYB_FeatureControl_PERIOD_LineNum=OV12830TRULYB_PV_PERIOD_LINE_NUMS;
UINT16  ov12830trulybVIDEO_MODE_TARGET_FPS = 30;
MSDK_SENSOR_CONFIG_STRUCT OV12830TRULYBSensorConfigData;
MSDK_SCENARIO_ID_ENUM OV12830TRULYBCurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT OV12830TRULYBSensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT OV12830TRULYBSensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/

static OV12830TRULYB_PARA_STRUCT ov12830trulyb;

static struct OV12830TRULYB_SENSOR_STRUCT OV12830TRULYB_sensor=
{
        .i2c_write_id = 0x6c,
        .i2c_read_id  = 0x6d,
};
extern unsigned char IR_type;
extern bool otp_update_wb(unsigned short golden_rg, unsigned short golden_bg);
extern bool otp_update_lenc();
extern int otp_update_BLC();

extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
extern int iMultiWriteReg(u8 *pData, u16 lens, u16 i2cId);

#define OV12830TRULYB_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, OV12830TRULYB_sensor.i2c_write_id)

#define OV12830TRULYB_multi_write_cmos_sensor(pData, lens) iMultiWriteReg((u8*) pData, (u16) lens, OV12830TRULYB_sensor.i2c_write_id)


kal_uint16 OV12830TRULYB_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte, OV12830TRULYB_sensor.i2c_write_id);
    return get_byte;
}

#define Sleep(ms) mdelay(ms)
void OV12830TRULYB_write_shutter(kal_uint32 shutter)
{
#if 1
	kal_uint32 min_framelength = OV12830TRULYB_PV_PERIOD_PIXEL_NUMS, max_shutter=0;
	kal_uint32 extra_lines = 0;
	kal_uint32 line_length = 0;
	kal_uint32 frame_length = 0;
	unsigned long flags;

	OV12830TRULYBDBSOFIA("!!shutter=%d!!!!!\n", shutter);

	if(ov12830trulyb.OV12830TRULYBAutoFlickerMode == KAL_TRUE)
	{
		if ( SENSOR_MODE_PREVIEW == ov12830trulyb.sensorMode )  //(g_iOV12830TRULYB_Mode == OV12830TRULYB_MODE_PREVIEW)	//SXGA size output
		{
			line_length = OV12830TRULYB_PV_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
			max_shutter = OV12830TRULYB_PV_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines ;
		}
		else if( SENSOR_MODE_VIDEO == ov12830trulyb.sensorMode ) //add for video_6M setting
		{
			line_length = OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
			max_shutter = OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines ;
		}
		else
		{
			line_length = OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
			max_shutter = OV12830TRULYB_FULL_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines ;
		}

		switch(OV12830TRULYBCurrentScenarioId)
		{
        	case MSDK_SCENARIO_ID_CAMERA_ZSD:
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				OV12830TRULYBDBSOFIA("AutoFlickerMode!!! MSDK_SCENARIO_ID_CAMERA_ZSD  0!!\n");
				min_framelength = max_shutter;
				break;
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				if( ov12830trulybVIDEO_MODE_TARGET_FPS==30)
				{
					min_framelength = (OV12830TRULYBMIPI_VIDEO_CLK) /(OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/306*10 ;
				}
				else if( ov12830trulybVIDEO_MODE_TARGET_FPS==15)
				{
					min_framelength = (OV12830TRULYBMIPI_VIDEO_CLK) /(OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/148*10 ;
				}
				else
				{
					min_framelength = max_shutter;
				}
				break;
			default:
				min_framelength = (OV12830TRULYBMIPI_PREVIEW_CLK) /(OV12830TRULYB_PV_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/296*10 ;
    			break;
		}

		OV12830TRULYBDBSOFIA("AutoFlickerMode!!! min_framelength for AutoFlickerMode = %d (0x%x)\n",min_framelength,min_framelength);
		OV12830TRULYBDBSOFIA("max framerate(10 base) autofilker = %d\n",(OV12830TRULYBMIPI_PREVIEW_CLK)*10 /line_length/min_framelength);

		if (shutter < 3)
			shutter = 3;

		if (shutter > max_shutter-4)
			extra_lines = shutter - max_shutter + 4;
		else
			extra_lines = 0;

		if ( SENSOR_MODE_PREVIEW == ov12830trulyb.sensorMode )	//SXGA size output
		{
			frame_length = OV12830TRULYB_PV_PERIOD_LINE_NUMS+ ov12830trulyb.DummyLines + extra_lines ;
		}
		else if(SENSOR_MODE_VIDEO == ov12830trulyb.sensorMode)
		{
			frame_length = OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS+ ov12830trulyb.DummyLines + extra_lines ;
		}
		else				//QSXGA size output
		{
			frame_length = OV12830TRULYB_FULL_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines + extra_lines ;
		}
		OV12830TRULYBDBSOFIA("frame_length 0= %d\n",frame_length);

		if (frame_length < min_framelength)
		{
			//shutter = min_framelength - 4;

			switch(OV12830TRULYBCurrentScenarioId)
			{
        	case MSDK_SCENARIO_ID_CAMERA_ZSD:
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				extra_lines = min_framelength- (OV12830TRULYB_FULL_PERIOD_LINE_NUMS+ ov12830trulyb.DummyLines);
				break;
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				extra_lines = min_framelength- (OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS+ ov12830trulyb.DummyLines);
			default:
				extra_lines = min_framelength- (OV12830TRULYB_PV_PERIOD_LINE_NUMS+ ov12830trulyb.DummyLines);
    			break;
			}
			frame_length = min_framelength;
		}

		//Set total frame length
		OV12830TRULYB_write_cmos_sensor(0x380e, (frame_length >> 8) & 0xFF);
		OV12830TRULYB_write_cmos_sensor(0x380f, frame_length & 0xFF);
		spin_lock_irqsave(&ov12830trulybmipiraw_drv_lock,flags);
		ov12830trulyb.maxExposureLines = frame_length;
		OV12830TRULYB_FeatureControl_PERIOD_PixelNum = line_length;
		OV12830TRULYB_FeatureControl_PERIOD_LineNum = frame_length;
		spin_unlock_irqrestore(&ov12830trulybmipiraw_drv_lock,flags);

		//Set shutter (Coarse integration time, uint: lines.)
		OV12830TRULYB_write_cmos_sensor(0x3500, (shutter>>12) & 0x0F);
		OV12830TRULYB_write_cmos_sensor(0x3501, (shutter>>4) & 0xFF);
		OV12830TRULYB_write_cmos_sensor(0x3502, (shutter<<4) & 0xF0);	/* Don't use the fraction part. */
	
		//OV12830TRULYBDB("framerate(10 base) = %d\n",(OV12830TRULYBMIPI_PREVIEW_CLK)*10 /line_length/frame_length);
		//OV12830TRULYBDB("shutter=%d, extra_lines=%d, line_length=%d, frame_length=%d\n", shutter, extra_lines, line_length, frame_length);

	}
	else
	{
		if ( SENSOR_MODE_PREVIEW == ov12830trulyb.sensorMode )  //(g_iOV12830TRULYB_Mode == OV12830TRULYB_MODE_PREVIEW)	//SXGA size output
		{
			max_shutter = OV12830TRULYB_PV_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines ;
		}
		else if( SENSOR_MODE_VIDEO == ov12830trulyb.sensorMode ) //add for video_6M setting
		{
			max_shutter = OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines ;
		}
		else
		{
			max_shutter = OV12830TRULYB_FULL_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines ;
		}

		if (shutter < 3)
			shutter = 3;

		if (shutter > max_shutter-4)
			extra_lines = shutter - max_shutter + 4;
		else
			extra_lines = 0;

		if ( SENSOR_MODE_PREVIEW == ov12830trulyb.sensorMode )	//SXGA size output
		{
			line_length = OV12830TRULYB_PV_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
			frame_length = OV12830TRULYB_PV_PERIOD_LINE_NUMS+ ov12830trulyb.DummyLines + extra_lines ;
		}
		else if( SENSOR_MODE_VIDEO == ov12830trulyb.sensorMode )
		{
			line_length = OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
			frame_length = OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines + extra_lines ;
		}
		else				//QSXGA size output
		{
			line_length = OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
			frame_length = OV12830TRULYB_FULL_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines + extra_lines ;
		}

		ASSERT(line_length < OV12830TRULYB_MAX_LINE_LENGTH);		//0xCCCC
		ASSERT(frame_length < OV12830TRULYB_MAX_FRAME_LENGTH); 	//0xFFFF
		
		//Set total frame length
		OV12830TRULYB_write_cmos_sensor(0x380e, (frame_length >> 8) & 0xFF);
		OV12830TRULYB_write_cmos_sensor(0x380f, frame_length & 0xFF);
		spin_lock_irqsave(&ov12830trulybmipiraw_drv_lock,flags);
		ov12830trulyb.maxExposureLines = frame_length -4;
		OV12830TRULYB_FeatureControl_PERIOD_PixelNum = line_length;
		OV12830TRULYB_FeatureControl_PERIOD_LineNum = frame_length;
		spin_unlock_irqrestore(&ov12830trulybmipiraw_drv_lock,flags);

		//Set shutter (Coarse integration time, uint: lines.)
		OV12830TRULYB_write_cmos_sensor(0x3500, (shutter>>12) & 0x0F);
		OV12830TRULYB_write_cmos_sensor(0x3501, (shutter>>4) & 0xFF);
		OV12830TRULYB_write_cmos_sensor(0x3502, (shutter<<4) & 0xF0);	/* Don't use the fraction part. */

		//OV12830TRULYBDB("framerate(10 base) = %d\n",(OV12830TRULYBMIPI_PREVIEW_CLK)*10 /line_length/frame_length);

		//OV12830TRULYBDB("shutter=%d, extra_lines=%d, line_length=%d, frame_length=%d\n", shutter, extra_lines, line_length, frame_length);
	}
#endif
}   /* write_OV12830TRULYB_shutter */

static kal_uint16 OV12830TRULYBReg2Gain(const kal_uint16 iReg)
{
    kal_uint8 iI;
    kal_uint16 iGain = ov12830trulyb.ispBaseGain;    // 1x-gain base

    // Range: 1x to 32x
    // Gain = (GAIN[9] + 1) *(GAIN[8] + 1) *(GAIN[7] + 1) * (GAIN[6] + 1) * (GAIN[5] + 1) * (GAIN[4] + 1) * (1 + GAIN[3:0] / 16)
    for (iI = 8; iI >= 4; iI--) {
        iGain *= (((iReg >> iI) & 0x01) + 1);
    }
    return iGain +  (iGain * (iReg & 0x0F)) / 16; //ov12830trulyb.realGain
}

static kal_uint16 OV12830TRULYBGain2Reg(const kal_uint16 Gain)
{
    kal_uint16 iReg = 0x0000;
	kal_uint16 iGain=Gain;
	if(iGain <  ov12830trulyb.ispBaseGain) 
	{
		iReg =0;
	}    
	else if (iGain < 2 * ov12830trulyb.ispBaseGain) 
	{
        iReg = 16 * iGain / ov12830trulyb.ispBaseGain - 16;
    }
	else if (iGain < 4 * ov12830trulyb.ispBaseGain) 
	{
        iReg |= 0x10;
        iReg |= (8 *iGain / ov12830trulyb.ispBaseGain - 16);
    }
	else if (iGain < 8 * ov12830trulyb.ispBaseGain) 
	{
        iReg |= 0x30;
        iReg |= (4 * iGain / ov12830trulyb.ispBaseGain - 16);
    }else if (iGain < 16 * ov12830trulyb.ispBaseGain) {
        iReg |= 0x70;
        iReg |= (2 * iGain /ov12830trulyb.ispBaseGain - 16);
    }else if(iGain < 32 * ov12830trulyb.ispBaseGain) {
        iReg |= 0xF0;
        iReg |= (iGain /ov12830trulyb.ispBaseGain - 16);
    }else if(iGain <= 62 * ov12830trulyb.ispBaseGain) {
    	iReg |= 0x1F0;
        iReg |= (iGain /ov12830trulyb.ispBaseGain/2 - 16);
    }
	else
	{
		OV12830TRULYBDB("out of range!\n");
	}
	OV12830TRULYBDBSOFIA("[OV12830TRULYBGain2Reg]: isp gain:%d,sensor gain:0x%x\n",iGain,iReg);

    return iReg;//ov12830trulyb. sensorGlobalGain

}

void write_OV12830TRULYB_gain(kal_uint16 gain)
{
	OV12830TRULYB_write_cmos_sensor(0x350a,(gain>>8));
	OV12830TRULYB_write_cmos_sensor(0x350b,(gain&0xff));
	return;
}
void OV12830TRULYB_SetGain(UINT16 iGain)
{
	unsigned long flags;
	spin_lock_irqsave(&ov12830trulybmipiraw_drv_lock,flags);
	ov12830trulyb.realGain = iGain;
	ov12830trulyb.sensorGlobalGain = OV12830TRULYBGain2Reg(iGain);
	spin_unlock_irqrestore(&ov12830trulybmipiraw_drv_lock,flags);
	write_OV12830TRULYB_gain(ov12830trulyb.sensorGlobalGain);
	OV12830TRULYBDB("[OV12830TRULYB_SetGain]ov12830trulyb.sensorGlobalGain=0x%x,ov12830trulyb.realGain=%d\n",ov12830trulyb.sensorGlobalGain,ov12830trulyb.realGain);

}   /*  OV12830TRULYB_SetGain_SetGain  */

kal_uint16 read_OV12830TRULYB_gain(void)
{
	kal_uint16 read_gain=0;
	read_gain=(((OV12830TRULYB_read_cmos_sensor(0x350a)&0x01) << 8) | OV12830TRULYB_read_cmos_sensor(0x350b));
	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.sensorGlobalGain = read_gain;
	ov12830trulyb.realGain = OV12830TRULYBReg2Gain(ov12830trulyb.sensorGlobalGain);
	spin_unlock(&ov12830trulybmipiraw_drv_lock);
	OV12830TRULYBDB("ov12830trulyb.sensorGlobalGain=0x%x,ov12830trulyb.realGain=%d\n",ov12830trulyb.sensorGlobalGain,ov12830trulyb.realGain);
	return ov12830trulyb.sensorGlobalGain;
}  /* read_OV12830TRULYB_gain */


void OV12830TRULYB_camera_para_to_sensor(void)
{}

void OV12830TRULYB_sensor_to_camera_para(void)
{}

kal_int32  OV12830TRULYB_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void OV12830TRULYB_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{}
void OV12830TRULYB_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{}
kal_bool OV12830TRULYB_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{    return KAL_TRUE;}

static void OV12830TRULYB_SetDummy( const kal_uint32 iPixels, const kal_uint32 iLines )
{
 	kal_uint32 line_length = 0;
	kal_uint32 frame_length = 0;

	if ( SENSOR_MODE_PREVIEW == ov12830trulyb.sensorMode )	//SXGA size output
	{
		line_length = OV12830TRULYB_PV_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV12830TRULYB_PV_PERIOD_LINE_NUMS + iLines;
	}
	else if( SENSOR_MODE_VIDEO== ov12830trulyb.sensorMode )
	{
		line_length = OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS + iLines;
	}
	else//QSXGA size output
	{
		line_length = OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV12830TRULYB_FULL_PERIOD_LINE_NUMS + iLines;
	}

	//if(ov12830trulyb.maxExposureLines > frame_length -4 )
	//	return;

	//ASSERT(line_length < OV12830TRULYB_MAX_LINE_LENGTH);		//0xCCCC
	//ASSERT(frame_length < OV12830TRULYB_MAX_FRAME_LENGTH);	//0xFFFF

	//Set total frame length
	OV12830TRULYB_write_cmos_sensor(0x380e, (frame_length >> 8) & 0xFF);
	OV12830TRULYB_write_cmos_sensor(0x380f, frame_length & 0xFF);

	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.maxExposureLines = frame_length -4;
	OV12830TRULYB_FeatureControl_PERIOD_PixelNum = line_length;
	OV12830TRULYB_FeatureControl_PERIOD_LineNum = frame_length;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);

	//Set total line length
	OV12830TRULYB_write_cmos_sensor(0x380c, (line_length >> 8) & 0xFF);
	OV12830TRULYB_write_cmos_sensor(0x380d, line_length & 0xFF);

}   /*  OV12830TRULYB_SetDummy */

void OV12830TRULYBPreviewSetting(void)
{
	OV12830TRULYBDB("OV12830TRULYB_2112x1500_4Lane_30fps_Mclk26M_setting start \n");
	//@@_OV12830TRULYB_2112x1500_4lane_30fps_351SCLK_728Mbps/Lane
	// fps=38.7
	// vblanking = 601.5us
	// hblanking = 8.6us
	//line data period = 16.7us
	// line pixel num = 2816*10b	
	// bps/lane 672MHz bps

	OV12830TRULYB_write_cmos_sensor(0x0100,0x00);//	
	mDELAY(2);
	OV12830TRULYB_write_cmos_sensor(0x30b4,0x03);
	OV12830TRULYB_write_cmos_sensor(0x30b3,0x54);
	OV12830TRULYB_write_cmos_sensor(0x3106,0x21);
	OV12830TRULYB_write_cmos_sensor(0x3090,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3091,0x1C);
	OV12830TRULYB_write_cmos_sensor(0x3708,0xe6);
	OV12830TRULYB_write_cmos_sensor(0x3709,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3801,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3802,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3803,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3805,0x9f);
	OV12830TRULYB_write_cmos_sensor(0x3806,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x3807,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3808,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3809,0x40);
	OV12830TRULYB_write_cmos_sensor(0x380a,0x05);
	OV12830TRULYB_write_cmos_sensor(0x380b,0xdc);
	OV12830TRULYB_write_cmos_sensor(0x380c,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x380d,0x00);
	OV12830TRULYB_write_cmos_sensor(0x380e,0x07);
	OV12830TRULYB_write_cmos_sensor(0x380f,0x9D);
	OV12830TRULYB_write_cmos_sensor(0x3811,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3813,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3814,0x31);
	OV12830TRULYB_write_cmos_sensor(0x3815,0x31);
	OV12830TRULYB_write_cmos_sensor(0x3820,0x14);
	OV12830TRULYB_write_cmos_sensor(0x3821,0x0f);
	OV12830TRULYB_write_cmos_sensor(0x4004,0x08);
	OV12830TRULYB_write_cmos_sensor(0x4837,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x5002,0x00);
	mDELAY(2);
	OV12830TRULYB_write_cmos_sensor(0x0100,0x01);


}


void OV12830TRULYBVideoSetting(void)
{
	OV12830TRULYBDB("OV12830TRULYBVideoSetting/3M_16:9 exit :\n ");
	OV12830TRULYB_write_cmos_sensor(0x0100,0x00);//
	mDELAY(2);
	OV12830TRULYB_write_cmos_sensor(0x30b4,0x03);
	OV12830TRULYB_write_cmos_sensor(0x30b3,0x54);
	OV12830TRULYB_write_cmos_sensor(0x3106,0x21);
	OV12830TRULYB_write_cmos_sensor(0x3090,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3091,0x1C);
	OV12830TRULYB_write_cmos_sensor(0x3708,0xe6);
	OV12830TRULYB_write_cmos_sensor(0x3709,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3801,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3802,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3803,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3805,0x9f);
	OV12830TRULYB_write_cmos_sensor(0x3806,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x3807,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3808,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3809,0x40);
	OV12830TRULYB_write_cmos_sensor(0x380a,0x05);
	OV12830TRULYB_write_cmos_sensor(0x380b,0xdc);
	OV12830TRULYB_write_cmos_sensor(0x380c,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x380d,0x00);
	OV12830TRULYB_write_cmos_sensor(0x380e,0x07);
	OV12830TRULYB_write_cmos_sensor(0x380f,0x9D);
	OV12830TRULYB_write_cmos_sensor(0x3811,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3813,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3814,0x31);
	OV12830TRULYB_write_cmos_sensor(0x3815,0x31);
	OV12830TRULYB_write_cmos_sensor(0x3820,0x14);
	OV12830TRULYB_write_cmos_sensor(0x3821,0x0f);
	OV12830TRULYB_write_cmos_sensor(0x4004,0x08);
	OV12830TRULYB_write_cmos_sensor(0x4837,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x5002,0x00);
	mDELAY(2);
	OV12830TRULYB_write_cmos_sensor(0x0100,0x01);


	
}

void OV12830TRULYBCaptureSetting(void)
{
	
	// fps=18.8
	// vblanking = 1.22ms
	// hblanking = 5.1us
	//line data period = 17.26us
	// line pixel num = 4352*10b	
	// bps/lane 816MHz
	OV12830TRULYB_write_cmos_sensor(0x0100,0x00);
	//mDELAY(2);
	OV12830TRULYB_write_cmos_sensor(0x30b3,0x54);
	OV12830TRULYB_write_cmos_sensor(0x30b4,0x03);
	OV12830TRULYB_write_cmos_sensor(0x3090,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3091,0x11);
	OV12830TRULYB_write_cmos_sensor(0x3106,0x01);
	OV12830TRULYB_write_cmos_sensor(0x3708,0xe3);
	OV12830TRULYB_write_cmos_sensor(0x3709,0xc3);
	OV12830TRULYB_write_cmos_sensor(0x3801,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3802,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3803,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3805,0x9f);
	OV12830TRULYB_write_cmos_sensor(0x3806,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x3807,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3808,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3809,0x80);
	OV12830TRULYB_write_cmos_sensor(0x380a,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x380b,0xb8);
	OV12830TRULYB_write_cmos_sensor(0x380c,0x11);
	OV12830TRULYB_write_cmos_sensor(0x380d,0x00);
	OV12830TRULYB_write_cmos_sensor(0x380e,0x0c);
	OV12830TRULYB_write_cmos_sensor(0x380f,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3811,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3813,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3814,0x11);
	OV12830TRULYB_write_cmos_sensor(0x3815,0x11);
	OV12830TRULYB_write_cmos_sensor(0x3820,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3821,0x0e);
	OV12830TRULYB_write_cmos_sensor(0x3503,0x07);
	OV12830TRULYB_write_cmos_sensor(0x3500,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3501,0xbd);
	OV12830TRULYB_write_cmos_sensor(0x3502,0x40);
	OV12830TRULYB_write_cmos_sensor(0x350b,0x80);
	OV12830TRULYB_write_cmos_sensor(0x4004,0x08);
	OV12830TRULYB_write_cmos_sensor(0x4837,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x5002,0x00);
	OV12830TRULYB_write_cmos_sensor(0x0100,0x01);
		mDELAY(66);
	
}

static kal_uint8 ov12830trulyb_init[] = {
0x01,0x03,0x01,
0x30,0x01,0x06,
0x30,0x02,0x80,
0x30,0x11,0x41,
0x30,0x14,0x16,
0x30,0x15,0x0b,
0x30,0x22,0x03,
0x30,0x90,0x02,
0x30,0x91,0x1b,
0x30,0x92,0x00,
0x30,0x93,0x00,
0x30,0x98,0x03,
0x30,0x99,0x11,
0x30,0x9c,0x01,
0x30,0xb4,0x03,
0x30,0xb5,0x04,
0x33,0x04,0x28,
0x33,0x05,0x41,
0x33,0x06,0x30,
0x33,0x08,0x00,
0x33,0x09,0xc8,
0x33,0x0a,0x01,
0x33,0x0b,0x90,
0x33,0x0c,0x02,
0x33,0x0d,0x58,
0x33,0x0e,0x03,
0x33,0x0f,0x20,
0x33,0x00,0x00,
0x35,0x00,0x00,
0x35,0x03,0x07,
0x35,0x09,0x08,
0x35,0x0a,0x00,
0x35,0x0b,0x80,
0x36,0x02,0x18,
0x36,0x12,0x80,
0x36,0x20,0x64,//add to reduce temperature
0x36,0x21,0xb5,
0x36,0x22,0x09,//modify from 0b->09 for reduce temperature 
0x36,0x23,0x28,
0x36,0x31,0xb3,
0x36,0x34,0x04,
0x36,0x60,0x80,
0x36,0x62,0x10,
0x36,0x63,0xf0,
0x36,0x67,0x00,
0x36,0x6f,0x20,
0x36,0x80,0xb5,
0x36,0x82,0x00,
0x37,0x01,0x12,
0x37,0x02,0x88,
0x37,0x0b,0xa0,
0x37,0x0d,0x11,
0x37,0x0e,0x00,
0x37,0x1c,0x01,
0x37,0x1f,0x1b,
0x37,0x24,0x10,
0x37,0x26,0x00,
0x37,0x2a,0x09,
0x37,0x39,0xb0,
0x37,0x3c,0x40,
0x37,0x6b,0x44,
0x37,0x7b,0x44,
0x37,0x80,0x22,
0x37,0x81,0xc8,
0x37,0x83,0x31,
0x37,0x96,0x84,
0x37,0x9c,0x0c,
0x37,0xc5,0x00,
0x37,0xc6,0x00,
0x37,0xc7,0x00,
0x37,0xc9,0x00,
0x37,0xca,0x00,
0x37,0xcb,0x00,
0x37,0xcc,0x00,
0x37,0xcd,0x00,
0x37,0xce,0x10,
0x37,0xcf,0x00,
0x37,0xd0,0x00,
0x37,0xd1,0x00,
0x37,0xd2,0x00,
0x37,0xde,0x00,
0x37,0xdf,0x00,
0x38,0x00,0x00,
0x38,0x04,0x10,
0x38,0x10,0x00,
0x38,0x12,0x00,
0x38,0x29,0x0b,
0x38,0x2b,0x6a,
0x40,0x00,0x10,
0x40,0x01,0x06,
0x40,0x02,0x45,
0x40,0x05,0x18,
0x40,0x07,0x90,
0x40,0x08,0x24,
0x40,0x4e,0x37,
0x40,0x4f,0x8f,
0x40,0x58,0x40,//add for reduce temperature
0x41,0x00,0x2d,
0x41,0x01,0x22,
0x41,0x02,0x04,
0x41,0x04,0x5c,
0x41,0x09,0xa3,
0x41,0x0a,0x03,
0x43,0x00,0xff,
0x43,0x03,0x00,
0x43,0x04,0x08,
0x43,0x07,0x30,
0x43,0x11,0x04,
0x45,0x11,0x05,
0x48,0x16,0x52,
0x48,0x1f,0x30,
0x48,0x26,0x2c,
0x48,0x4b,0x05,  //SEL_MIPI_CTRL_4B  
0x4a,0x00,0xaa,
0x4a,0x03,0x01,
0x4a,0x05,0x08,
0x4d,0x01,0x19,
0x4d,0x02,0xfd,
0x4d,0x03,0xd1,
0x4d,0x04,0xff,
0x4d,0x05,0xff,
0x4d,0x07,0x04,  //Temperature monitor
0x50,0x00,0x06,
0x50,0x01,0x01,
0x50,0x03,0x21,
0x50,0x43,0x48,
0x50,0x13,0x80,
0x50,0x1f,0x00,
0x5e,0x00,0x00,
0x5a,0x01,0x00,
0x5a,0x02,0x00,
0x5a,0x03,0x00,
0x5a,0x04,0x10,
0x5a,0x05,0xa0,
0x5a,0x06,0x0c,
0x5a,0x07,0x78,
0x5a,0x08,0x00,
0x5e,0x00,0x00,
0x5e,0x01,0x41,
0x5e,0x11,0x30,
0x50,0x00,0x06,
0x50,0x01,0x01,
0x34,0x00,0x04,
0x34,0x01,0x00,
0x34,0x02,0x04,
0x34,0x03,0x00,
0x34,0x04,0x04,
0x34,0x05,0x00,
0x34,0x06,0x01,
0x40,0x09,0x10,
0x35,0x03,0x07,
0x35,0x00,0x00,
0x35,0x01,0x97,
0x35,0x02,0x00,
0x35,0x0b,0x80,
0x48,0x00,0x14,
0x30,0xb4,0x03,
0x30,0xb3,0x54,
0x31,0x06,0x21,
0x30,0x90,0x02,
0x30,0x91,0x1C,
0x37,0x08,0xe6,
0x37,0x09,0xc7,
0x38,0x01,0x00,
0x38,0x02,0x00,
0x38,0x03,0x00,
0x38,0x05,0x9f,
0x38,0x06,0x0b,
0x38,0x07,0xc7,
0x38,0x08,0x08,
0x38,0x09,0x40,
0x38,0x0a,0x05,
0x38,0x0b,0xdc,
0x38,0x0c,0x0b,
0x38,0x0d,0x00,
0x38,0x0e,0x07,
0x38,0x0f,0x9D,
0x38,0x11,0x08,
0x38,0x13,0x02,
0x38,0x14,0x31,
0x38,0x15,0x31,
0x38,0x20,0x14,
0x38,0x21,0x0f,
0x40,0x04,0x08,
0x48,0x37,0x0b,
0x50,0x02,0x00,
0x01,0x00,0x01};

static void OV12830TRULYB_Sensor_Init(void)
{

#if 0

	OV12830TRULYB_write_cmos_sensor(0x0103,0x01);
	OV12830TRULYB_write_cmos_sensor(0x3001,0x06);
	OV12830TRULYB_write_cmos_sensor(0x3002,0x80);
	OV12830TRULYB_write_cmos_sensor(0x3011,0x41);
	OV12830TRULYB_write_cmos_sensor(0x3014,0x16);
	OV12830TRULYB_write_cmos_sensor(0x3015,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x3022,0x03);
	OV12830TRULYB_write_cmos_sensor(0x3090,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3091,0x1b);
	OV12830TRULYB_write_cmos_sensor(0x3092,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3093,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3098,0x03);
	OV12830TRULYB_write_cmos_sensor(0x3099,0x11);
	OV12830TRULYB_write_cmos_sensor(0x309c,0x01);
	OV12830TRULYB_write_cmos_sensor(0x30b4,0x03);
	OV12830TRULYB_write_cmos_sensor(0x30b5,0x04);
	OV12830TRULYB_write_cmos_sensor(0x3304,0x28);
	OV12830TRULYB_write_cmos_sensor(0x3305,0x41);
	OV12830TRULYB_write_cmos_sensor(0x3306,0x30);
	OV12830TRULYB_write_cmos_sensor(0x3308,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3309,0xc8);
	OV12830TRULYB_write_cmos_sensor(0x330a,0x01);
	OV12830TRULYB_write_cmos_sensor(0x330b,0x90);
	OV12830TRULYB_write_cmos_sensor(0x330c,0x02);
	OV12830TRULYB_write_cmos_sensor(0x330d,0x58);
	OV12830TRULYB_write_cmos_sensor(0x330e,0x03);
	OV12830TRULYB_write_cmos_sensor(0x330f,0x20);
	OV12830TRULYB_write_cmos_sensor(0x3300,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3500,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3503,0x07);
	OV12830TRULYB_write_cmos_sensor(0x3509,0x08);
	OV12830TRULYB_write_cmos_sensor(0x350a,0x00);
	OV12830TRULYB_write_cmos_sensor(0x350b,0x80);
	OV12830TRULYB_write_cmos_sensor(0x3602,0x28);
	OV12830TRULYB_write_cmos_sensor(0x3612,0x80);
	OV12830TRULYB_write_cmos_sensor(0x3622,0x0f);
	OV12830TRULYB_write_cmos_sensor(0x3631,0xb3);
	OV12830TRULYB_write_cmos_sensor(0x3634,0x04);
	OV12830TRULYB_write_cmos_sensor(0x3660,0x80);
	OV12830TRULYB_write_cmos_sensor(0x3662,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3663,0xf0);
	OV12830TRULYB_write_cmos_sensor(0x3667,0x00);
	OV12830TRULYB_write_cmos_sensor(0x366f,0x20);
	OV12830TRULYB_write_cmos_sensor(0x3680,0xb5);
	OV12830TRULYB_write_cmos_sensor(0x3682,0x00);
	OV12830TRULYB_write_cmos_sensor(0x370b,0xa8);
	OV12830TRULYB_write_cmos_sensor(0x370d,0x11);
	OV12830TRULYB_write_cmos_sensor(0x370e,0x00);
	OV12830TRULYB_write_cmos_sensor(0x371c,0x01);
	OV12830TRULYB_write_cmos_sensor(0x3726,0x00);
	OV12830TRULYB_write_cmos_sensor(0x372a,0x09);
	OV12830TRULYB_write_cmos_sensor(0x3739,0x7c);
	OV12830TRULYB_write_cmos_sensor(0x373c,0x44);
	OV12830TRULYB_write_cmos_sensor(0x376b,0x44);
	OV12830TRULYB_write_cmos_sensor(0x377b,0x44);
	OV12830TRULYB_write_cmos_sensor(0x3780,0x22);
	OV12830TRULYB_write_cmos_sensor(0x3781,0x0c);
	OV12830TRULYB_write_cmos_sensor(0x3783,0x31);
	OV12830TRULYB_write_cmos_sensor(0x379c,0x0c);
	OV12830TRULYB_write_cmos_sensor(0x37c5,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37c6,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37c7,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37c9,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37ca,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37cb,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37cc,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37cd,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37ce,0x10);
	OV12830TRULYB_write_cmos_sensor(0x37cf,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37d0,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37d1,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37d2,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37de,0x00);
	OV12830TRULYB_write_cmos_sensor(0x37df,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3800,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3804,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3810,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3812,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3829,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x382b,0x6a);
	OV12830TRULYB_write_cmos_sensor(0x4000,0x10);
	OV12830TRULYB_write_cmos_sensor(0x4001,0x06);
	OV12830TRULYB_write_cmos_sensor(0x4002,0x45);
	OV12830TRULYB_write_cmos_sensor(0x4005,0x18);
	OV12830TRULYB_write_cmos_sensor(0x4008,0x24);
	OV12830TRULYB_write_cmos_sensor(0x4100,0x50);
	OV12830TRULYB_write_cmos_sensor(0x4101,0xb2);
	OV12830TRULYB_write_cmos_sensor(0x4102,0x34);
	OV12830TRULYB_write_cmos_sensor(0x4104,0xdc);
	OV12830TRULYB_write_cmos_sensor(0x4109,0x62);
	OV12830TRULYB_write_cmos_sensor(0x4300,0xff);
	OV12830TRULYB_write_cmos_sensor(0x4303,0x00);
	OV12830TRULYB_write_cmos_sensor(0x4304,0x08);
	OV12830TRULYB_write_cmos_sensor(0x4307,0x30);
	OV12830TRULYB_write_cmos_sensor(0x4311,0x04);
	OV12830TRULYB_write_cmos_sensor(0x4511,0x05);
	OV12830TRULYB_write_cmos_sensor(0x4816,0x52);
	OV12830TRULYB_write_cmos_sensor(0x481f,0x30);
	OV12830TRULYB_write_cmos_sensor(0x4826,0x2c);
	OV12830TRULYB_write_cmos_sensor(0x4a00,0xaa);
	OV12830TRULYB_write_cmos_sensor(0x4a03,0x01);
	OV12830TRULYB_write_cmos_sensor(0x4a05,0x08);
	OV12830TRULYB_write_cmos_sensor(0x4d01,0x71);
	OV12830TRULYB_write_cmos_sensor(0x4d02,0xfd);
	OV12830TRULYB_write_cmos_sensor(0x4d03,0xf5);
	OV12830TRULYB_write_cmos_sensor(0x4d04,0x0c);
	OV12830TRULYB_write_cmos_sensor(0x4d05,0xcc);
	OV12830TRULYB_write_cmos_sensor(0x5000,0x06);
	OV12830TRULYB_write_cmos_sensor(0x5001,0x01);
	OV12830TRULYB_write_cmos_sensor(0x5003,0x21);
	OV12830TRULYB_write_cmos_sensor(0x5043,0x48);
	OV12830TRULYB_write_cmos_sensor(0x5013,0x80);
	OV12830TRULYB_write_cmos_sensor(0x501f,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5e00,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5a01,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5a02,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5a03,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5a04,0x10);
	OV12830TRULYB_write_cmos_sensor(0x5a05,0xa0);
	OV12830TRULYB_write_cmos_sensor(0x5a06,0x0c);
	OV12830TRULYB_write_cmos_sensor(0x5a07,0x78);
	OV12830TRULYB_write_cmos_sensor(0x5a08,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5e00,0x00);
	OV12830TRULYB_write_cmos_sensor(0x5e01,0x41);
	OV12830TRULYB_write_cmos_sensor(0x5e11,0x30);
	OV12830TRULYB_write_cmos_sensor(0x5000,0x06);
	OV12830TRULYB_write_cmos_sensor(0x5001,0x01);
	OV12830TRULYB_write_cmos_sensor(0x3400,0x04);
	OV12830TRULYB_write_cmos_sensor(0x3401,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3402,0x04);
	OV12830TRULYB_write_cmos_sensor(0x3403,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3404,0x04);
	OV12830TRULYB_write_cmos_sensor(0x3405,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3406,0x01);
	OV12830TRULYB_write_cmos_sensor(0x4005,0x18);
	OV12830TRULYB_write_cmos_sensor(0x4009,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3503,0x07);
	OV12830TRULYB_write_cmos_sensor(0x3500,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3501,0x10);
	OV12830TRULYB_write_cmos_sensor(0x3502,0x40);
	OV12830TRULYB_write_cmos_sensor(0x350b,0x80);
	OV12830TRULYB_write_cmos_sensor(0x4800,0x14);
	OV12830TRULYB_write_cmos_sensor(0x30b4,0x03);
	OV12830TRULYB_write_cmos_sensor(0x30b3,0x54);
	OV12830TRULYB_write_cmos_sensor(0x3106,0x21);
	OV12830TRULYB_write_cmos_sensor(0x3090,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3091,0x1C);
	OV12830TRULYB_write_cmos_sensor(0x3708,0xe6);
	OV12830TRULYB_write_cmos_sensor(0x3709,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3801,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3802,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3803,0x00);
	OV12830TRULYB_write_cmos_sensor(0x3805,0x9f);
	OV12830TRULYB_write_cmos_sensor(0x3806,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x3807,0xc7);
	OV12830TRULYB_write_cmos_sensor(0x3808,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3809,0x40);
	OV12830TRULYB_write_cmos_sensor(0x380a,0x05);
	OV12830TRULYB_write_cmos_sensor(0x380b,0xdc);
	OV12830TRULYB_write_cmos_sensor(0x380c,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x380d,0x00);
	OV12830TRULYB_write_cmos_sensor(0x380e,0x07);
	OV12830TRULYB_write_cmos_sensor(0x380f,0x9D);
	OV12830TRULYB_write_cmos_sensor(0x3811,0x08);
	OV12830TRULYB_write_cmos_sensor(0x3813,0x02);
	OV12830TRULYB_write_cmos_sensor(0x3814,0x31);
	OV12830TRULYB_write_cmos_sensor(0x3815,0x31);
	OV12830TRULYB_write_cmos_sensor(0x3820,0x14);
	OV12830TRULYB_write_cmos_sensor(0x3821,0x0f);
	OV12830TRULYB_write_cmos_sensor(0x4004,0x02);
	OV12830TRULYB_write_cmos_sensor(0x4837,0x0b);
	OV12830TRULYB_write_cmos_sensor(0x5002,0x00);
	OV12830TRULYB_write_cmos_sensor(0x0100,0x01);

#else
    int totalCnt = 0, len = 0;
	int transfer_len, transac_len=3;
	kal_uint8* pBuf=NULL;
	dma_addr_t dmaHandle;
	pBuf = (kal_uint8*)kmalloc(1024, GFP_KERNEL);
	

    totalCnt = ARRAY_SIZE(ov12830trulyb_init);
	transfer_len = totalCnt / transac_len;
	len = (transfer_len<<8)|transac_len;    
	OV12830TRULYBDB("Total Count = %d, Len = 0x%x\n", totalCnt,len);    
	memcpy(pBuf, &ov12830trulyb_init, totalCnt );   
	dmaHandle = dma_map_single(NULL, pBuf, 1024, DMA_TO_DEVICE);	
	OV12830TRULYB_multi_write_cmos_sensor(dmaHandle, len); 

	dma_unmap_single(NULL, dmaHandle, 1024, DMA_TO_DEVICE);

	
#endif	

	
}   /*  OV12830TRULYB_Sensor_Init  */

UINT32 OV12830TRULYBOpen(void)
{
	volatile signed int i;
	kal_uint16 sensor_id = 0;
	OV12830TRULYBDB("OV12830TRULYBOpen enter :\n ");
	OV12830TRULYB_write_cmos_sensor(0x0103,0x01);// Reset sensor
    mDELAY(2);

	for(i=0;i<3;i++)
	{
		sensor_id = (OV12830TRULYB_read_cmos_sensor(0x300A)<<8)|OV12830TRULYB_read_cmos_sensor(0x300B);
		OV12830TRULYBDB("OOV12830TRULYB READ ID :%x",sensor_id);
		if(sensor_id != OV12830_SENSOR_ID)
			return ERROR_SENSOR_CONNECT_FAIL;
		else break;
	}
	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.sensorMode = SENSOR_MODE_INIT;
	ov12830trulyb.OV12830TRULYBAutoFlickerMode = KAL_FALSE;
	ov12830trulyb.OV12830TRULYBVideoMode = KAL_FALSE;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);
	OV12830TRULYB_Sensor_Init();
        /*add by qingzhan */
        OV12830TRULYB_write_cmos_sensor(0x0100,0x01);// wake up//add for ensure can enter otp
        mdelay(50);
        otp_update_wb(0x137, 0x141);
        otp_update_lenc();
        otp_update_BLC();
        /*end*/
	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.DummyLines= 0;
	ov12830trulyb.DummyPixels= 0;
	ov12830trulyb.shutter = 0x4EA;
	ov12830trulyb.pvShutter = 0x4EA;
	ov12830trulyb.maxExposureLines =OV12830TRULYB_PV_PERIOD_LINE_NUMS -4;
	ov12830trulyb.ispBaseGain = BASEGAIN;//0x40
	ov12830trulyb.sensorGlobalGain = 0x1f;//sensor gain read from 0x350a 0x350b; 0x1f as 3.875x
	ov12830trulyb.pvGain = 0x1f;
	ov12830trulyb.realGain = OV12830TRULYBReg2Gain(0x1f);//ispBaseGain as 1x
	spin_unlock(&ov12830trulybmipiraw_drv_lock);

	OV12830TRULYBDB("OV12830TRULYBOpen exit :\n ");

    return ERROR_NONE;
}
extern bool camera_pdn_reverse;
static bool Sensorid_has_get_trulyb = 0;
UINT32 OV12830TRULYBGetSensorID(UINT32 *sensorID)
{
    int  retry = 1;
    unsigned int mid = 0;
    unsigned int  truly_truly_IR_type;
   if(Sensorid_has_get_trulyb== 1)
    {
        *sensorID = OV12830TRULYB_SENSOR_ID;
        camera_pdn_reverse = 1;
        return;
    }
    mt_set_gpio_mode(GPIO_CAMERA_CMPDN_PIN, GPIO_CAMERA_CMPDN_PIN_M_GPIO);
    mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN, GPIO_OUT_ONE);
    mDELAY(10);

    OV12830TRULYBDB("OV12830TRULYBGetSensorID enter :\n ");
    OV12830TRULYB_write_cmos_sensor(0x0103,0x01);// Reset sensor
    mDELAY(10);

    // check if sensor ID correct
    do {
        *sensorID = (OV12830TRULYB_read_cmos_sensor(0x300A)<<8)|OV12830TRULYB_read_cmos_sensor(0x300B);
        if (*sensorID == OV12830_SENSOR_ID)
        {
            OV12830TRULYBDB("Sensor ID = 0x%04x\n", *sensorID);
            camera_pdn_reverse = 1;
            Sensorid_has_get_trulyb= 1;
            break;
        }
        OV12830TRULYBDB("Read Sensor ID Fail = 0x%04x\n", *sensorID);
        retry--;
    } while (retry > 0);

    if (*sensorID != OV12830_SENSOR_ID) {
        OV12830TRULYB_sensor.i2c_write_id = 0x20;
        OV12830TRULYB_write_cmos_sensor(0x0103,0x01);// Reset sensor
        mDELAY(10);
        // check if sensor ID correct
        do {
            *sensorID = (OV12830TRULYB_read_cmos_sensor(0x300A)<<8)|OV12830TRULYB_read_cmos_sensor(0x300B);
            if (*sensorID == OV12830_SENSOR_ID)
            {
                OV12830TRULYBDB("Sensor ID = 0x%04x\n", *sensorID);
                camera_pdn_reverse = 1;
                Sensorid_has_get_trulyb = 1;
                break;
            }
			OV12830TRULYBDB("Read Sensor ID Fail = 0x%04x\n", *sensorID);
            retry--;
        } while (retry > 0);
    }

    if (*sensorID != OV12830_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    /*
    otp_read(OTP_WB_GROUP_ADDR+1, &mid);
    printk("-----mid is %d------\n",mid);
    if (mid != TRULY_OTP_MID)
    {
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    otp_read(OTP_WB_GROUP_ADDR+11, &truly_IR_type);
    printk("-----truly_IR_type is %d------\n",truly_IR_type);
     */
    //otp_read_wb_group(-1);
    if (IR_type == 0x01)
    {
        *sensorID = OV12830TRULYB_SENSOR_ID;
    }
    else
    {
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    *sensorID = OV12830TRULYB_SENSOR_ID;
    OV12830TRULYBDB("OV12830TRULYBGetSensorID end :\n ");
    return ERROR_NONE;
}


void OV12830TRULYB_SetShutter(kal_uint32 iShutter)
{
	if(MSDK_SCENARIO_ID_CAMERA_ZSD == OV12830TRULYBCurrentScenarioId )
	{
		//OV12830TRULYBDB("always UPDATE SHUTTER when ov12830trulyb.sensorMode == SENSOR_MODE_CAPTURE\n");
	}
	else{
		if(ov12830trulyb.sensorMode == SENSOR_MODE_CAPTURE)
		{
			//OV12830TRULYBDB("capture!!DONT UPDATE SHUTTER!!\n");
			//return;
		}
	}
	if(ov12830trulyb.shutter == iShutter)
		return;
   spin_lock(&ov12830trulybmipiraw_drv_lock);
   ov12830trulyb.shutter= iShutter;
   spin_unlock(&ov12830trulybmipiraw_drv_lock);
   OV12830TRULYB_write_shutter(iShutter);
   return;
}   /*  OV12830TRULYB_SetShutter   */

UINT32 OV12830TRULYB_read_shutter(void)
{

	kal_uint16 temp_reg1, temp_reg2 ,temp_reg3;
	UINT32 shutter =0;
	temp_reg1 = OV12830TRULYB_read_cmos_sensor(0x3500);    // AEC[b19~b16]
	temp_reg2 = OV12830TRULYB_read_cmos_sensor(0x3501);    // AEC[b15~b8]
	temp_reg3 = OV12830TRULYB_read_cmos_sensor(0x3502);    // AEC[b7~b0]
	shutter  = (temp_reg1 <<12)| (temp_reg2<<4)|(temp_reg3>>4);

	return shutter;
}

void OV12830TRULYB_NightMode(kal_bool bEnable)
{}


UINT32 OV12830TRULYBClose(void)
{    return ERROR_NONE;}

void OV12830TRULYBSetFlipMirror(kal_int32 imgMirror)
{
	kal_int16 mirror=0,flip=0;
        if(0 == strncmp(VANZO_MAIN_CAM_ROTATION, "180", 3))
            imgMirror= IMAGE_HV_MIRROR;
        else
            ;//imgMirror = IMAGE_HV_MIRROR;
	mirror= OV12830TRULYB_read_cmos_sensor(0x3820);
	flip  = OV12830TRULYB_read_cmos_sensor(0x3821);
    switch (imgMirror)
    {
        case IMAGE_H_MIRROR://IMAGE_NORMAL:
            OV12830TRULYB_write_cmos_sensor(0x3820, (mirror & (0xBD)));//Set normal
            OV12830TRULYB_write_cmos_sensor(0x3821, (flip & (0xF9)));	//Set normal
            break;
        case IMAGE_NORMAL://IMAGE_V_MIRROR:
            OV12830TRULYB_write_cmos_sensor(0x3820, (mirror & (0xBD)));//Set flip
            OV12830TRULYB_write_cmos_sensor(0x3821, (flip | (0x06)));	//Set flip
            break;
        case IMAGE_HV_MIRROR://IMAGE_H_MIRROR:
            OV12830TRULYB_write_cmos_sensor(0x3820, (mirror | (0x42)));	//Set mirror
            OV12830TRULYB_write_cmos_sensor(0x3821, (flip & (0xF9)));	//Set mirror
            break;
        case IMAGE_V_MIRROR://IMAGE_HV_MIRROR:
            OV12830TRULYB_write_cmos_sensor(0x3820, (mirror | (0x42)));	//Set mirror & flip
            OV12830TRULYB_write_cmos_sensor(0x3821, (flip | (0x06)));	//Set mirror & flip
            break;
    }
}

UINT32 OV12830TRULYBPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	OV12830TRULYBDB("OV12830TRULYBPreview enter:");

	// preview size
	if(ov12830trulyb.sensorMode == SENSOR_MODE_PREVIEW)
	{
		// do nothing
		// FOR CCT PREVIEW
	}
	else
	{
		//OV12830TRULYBDB("OV12830TRULYBPreview setting!!\n");
		OV12830TRULYB_Sensor_Init();
		OV12830TRULYBPreviewSetting();
	}
	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.sensorMode = SENSOR_MODE_PREVIEW; // Need set preview setting after capture mode
	ov12830trulyb.DummyPixels = 0;//define dummy pixels and lines
	ov12830trulyb.DummyLines = 0 ;
	OV12830TRULYB_FeatureControl_PERIOD_PixelNum=OV12830TRULYB_PV_PERIOD_PIXEL_NUMS+ ov12830trulyb.DummyPixels;
	OV12830TRULYB_FeatureControl_PERIOD_LineNum=OV12830TRULYB_PV_PERIOD_LINE_NUMS+ov12830trulyb.DummyLines;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);

	//OV12830TRULYB_write_shutter(ov12830trulyb.shutter);
	//write_OV12830TRULYB_gain(ov12830trulyb.pvGain);
	//set mirror & flip
	//OV12830TRULYBDB("[OV12830TRULYBPreview] mirror&flip: %d \n",sensor_config_data->SensorImageMirror);
	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.imgMirror = sensor_config_data->SensorImageMirror;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);
	OV12830TRULYBSetFlipMirror(sensor_config_data->SensorImageMirror);
	OV12830TRULYBDB("OV12830TRULYBPreview exit:\n");
    return ERROR_NONE;
}	/* OV12830TRULYBPreview() */


UINT32 OV12830TRULYBVideo(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	OV12830TRULYBDB("OV12830TRULYBVideo enter:");
	if(ov12830trulyb.sensorMode == SENSOR_MODE_VIDEO)
	{
		// do nothing
	}
	else
		OV12830TRULYBVideoSetting();
	
	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.sensorMode = SENSOR_MODE_VIDEO;
	OV12830TRULYB_FeatureControl_PERIOD_PixelNum=OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS+ ov12830trulyb.DummyPixels;
	OV12830TRULYB_FeatureControl_PERIOD_LineNum=OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS+ov12830trulyb.DummyLines;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);

	//OV12830TRULYB_write_shutter(ov12830trulyb.shutter);
	//write_OV12830TRULYB_gain(ov12830trulyb.pvGain);

	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.imgMirror = sensor_config_data->SensorImageMirror;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);
	OV12830TRULYBSetFlipMirror(sensor_config_data->SensorImageMirror);

	OV12830TRULYBDBSOFIA("[OV12830TRULYBVideo]frame_len=%x\n", ((OV12830TRULYB_read_cmos_sensor(0x380e)<<8)+OV12830TRULYB_read_cmos_sensor(0x380f)));

	OV12830TRULYBDB("OV12830TRULYBVideo exit:\n");
    return ERROR_NONE;
}


UINT32 OV12830TRULYBCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
 	kal_uint32 shutter = ov12830trulyb.shutter;
	kal_uint32 temp_data;
	if( SENSOR_MODE_CAPTURE== ov12830trulyb.sensorMode)
	{
		OV12830TRULYBDB("OV12830TRULYBCapture BusrtShot!!!\n");
	}else{
		OV12830TRULYBDB("OV12830TRULYBCapture enter:\n");
		//Record Preview shutter & gain
		shutter=OV12830TRULYB_read_shutter();
		temp_data =  read_OV12830TRULYB_gain();
		spin_lock(&ov12830trulybmipiraw_drv_lock);
		ov12830trulyb.pvShutter =shutter;
		ov12830trulyb.sensorGlobalGain = temp_data;
		ov12830trulyb.pvGain =ov12830trulyb.sensorGlobalGain;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);

		OV12830TRULYBDB("[OV12830TRULYBCapture]ov12830trulyb.shutter=%d, read_pv_shutter=%d, read_pv_gain = 0x%x\n",ov12830trulyb.shutter, shutter,ov12830trulyb.sensorGlobalGain);

		// Full size setting
		OV12830TRULYBCaptureSetting();

		spin_lock(&ov12830trulybmipiraw_drv_lock);
		ov12830trulyb.sensorMode = SENSOR_MODE_CAPTURE;
		ov12830trulyb.imgMirror = sensor_config_data->SensorImageMirror;
		ov12830trulyb.DummyPixels = 0;//define dummy pixels and lines                                                                                                         
		ov12830trulyb.DummyLines = 0 ;    
		OV12830TRULYB_FeatureControl_PERIOD_PixelNum = OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels;
		OV12830TRULYB_FeatureControl_PERIOD_LineNum = OV12830TRULYB_FULL_PERIOD_LINE_NUMS + ov12830trulyb.DummyLines;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);

		//OV12830TRULYBDB("[OV12830TRULYBCapture] mirror&flip: %d\n",sensor_config_data->SensorImageMirror);
		OV12830TRULYBSetFlipMirror(sensor_config_data->SensorImageMirror);

	    if(OV12830TRULYBCurrentScenarioId==MSDK_SCENARIO_ID_CAMERA_ZSD)
	    {
			OV12830TRULYBDB("OV12830TRULYBCapture exit ZSD!!\n");
			return ERROR_NONE;
	    }
		OV12830TRULYBDB("OV12830TRULYBCapture exit:\n");
	}

    return ERROR_NONE;
}	/* OV12830TRULYBCapture() */

UINT32 OV12830TRULYBGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{

    OV12830TRULYBDB("OV12830TRULYBGetResolution!!\n");
	pSensorResolution->SensorPreviewWidth	= OV12830TRULYB_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight	= OV12830TRULYB_IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorFullWidth		= OV12830TRULYB_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight		= OV12830TRULYB_IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorVideoWidth		= OV12830TRULYB_IMAGE_SENSOR_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight    = OV12830TRULYB_IMAGE_SENSOR_VIDEO_HEIGHT;
    return ERROR_NONE;
}   /* OV12830TRULYBGetResolution() */

UINT32 OV12830TRULYBGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	pSensorInfo->SensorPreviewResolutionX= OV12830TRULYB_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY= OV12830TRULYB_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX= OV12830TRULYB_IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY= OV12830TRULYB_IMAGE_SENSOR_FULL_HEIGHT;

	spin_lock(&ov12830trulybmipiraw_drv_lock);
	ov12830trulyb.imgMirror = pSensorConfigData->SensorImageMirror ;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);

    pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_B;
    pSensorInfo->SensorClockPolarity =SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;

    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;

    pSensorInfo->CaptureDelayFrame = 2;
    pSensorInfo->PreviewDelayFrame = 1;
    pSensorInfo->VideoDelayFrame = 2;

    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
    pSensorInfo->AEShutDelayFrame = 0;//0;		    /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0 ;//0;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = OV12830TRULYB_PV_X_START;
            pSensorInfo->SensorGrabStartY = OV12830TRULYB_PV_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = OV12830TRULYB_VIDEO_X_START;
            pSensorInfo->SensorGrabStartY = OV12830TRULYB_VIDEO_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = OV12830TRULYB_FULL_X_START;	//2*OV12830TRULYB_IMAGE_SENSOR_PV_STARTX;
            pSensorInfo->SensorGrabStartY = OV12830TRULYB_FULL_Y_START;	//2*OV12830TRULYB_IMAGE_SENSOR_PV_STARTY;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        default:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = OV12830TRULYB_PV_X_START;
            pSensorInfo->SensorGrabStartY = OV12830TRULYB_PV_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
    }

    memcpy(pSensorConfigData, &OV12830TRULYBSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* OV12830TRULYBGetInfo() */


UINT32 OV12830TRULYBControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
		spin_lock(&ov12830trulybmipiraw_drv_lock);
		OV12830TRULYBCurrentScenarioId = ScenarioId;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);
		OV12830TRULYBDB("OV12830TRULYBCurrentScenarioId=%d\n",OV12830TRULYBCurrentScenarioId);
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            OV12830TRULYBPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			OV12830TRULYBVideo(pImageWindow, pSensorConfigData);
			break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            OV12830TRULYBCapture(pImageWindow, pSensorConfigData);
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
} /* OV12830TRULYBControl() */


UINT32 OV12830TRULYBSetVideoMode(UINT16 u2FrameRate)
{

    kal_uint32 MIN_Frame_length =0,frameRate=0,extralines=0;
    OV12830TRULYBDB("[OV12830TRULYBSetVideoMode] frame rate = %d\n", u2FrameRate);

	spin_lock(&ov12830trulybmipiraw_drv_lock);
	 ov12830trulybVIDEO_MODE_TARGET_FPS=u2FrameRate;
	spin_unlock(&ov12830trulybmipiraw_drv_lock);

	if(u2FrameRate==0)
	{
		OV12830TRULYBDB("Disable Video Mode or dynimac fps\n");
		return KAL_TRUE;
	}
	if(u2FrameRate >30 || u2FrameRate <5)
	    OV12830TRULYBDB("error frame rate seting\n");

    if(ov12830trulyb.sensorMode == SENSOR_MODE_VIDEO)//video ScenarioId recording
    {
    	if(ov12830trulyb.OV12830TRULYBAutoFlickerMode == KAL_TRUE)
    	{
    		if (u2FrameRate==30)
				frameRate= 306;
			else if(u2FrameRate==15)
				frameRate= 148;
			else
				frameRate=u2FrameRate*10;

			MIN_Frame_length = (OV12830TRULYBMIPI_VIDEO_CLK)/(OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/frameRate*10;
    	}
		else
			MIN_Frame_length = (OV12830TRULYBMIPI_VIDEO_CLK) /(OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/u2FrameRate;

		if((MIN_Frame_length <=OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS))
		{
			MIN_Frame_length = OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS;
			OV12830TRULYBDB("[OV12830TRULYBSetVideoMode]current fps = %d\n", (OV12830TRULYBMIPI_PREVIEW_CLK)  /(OV12830TRULYB_PV_PERIOD_PIXEL_NUMS)/OV12830TRULYB_PV_PERIOD_LINE_NUMS);
		}
		//OV12830TRULYBDB("[OV12830TRULYBSetVideoMode]current fps (10 base)= %d\n", (OV12830TRULYBMIPI_PREVIEW_CLK)*10/(OV12830TRULYB_PV_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/MIN_Frame_length);
		extralines = MIN_Frame_length - OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS;

		spin_lock(&ov12830trulybmipiraw_drv_lock);
		ov12830trulyb.DummyPixels = 0;//define dummy pixels and lines
		ov12830trulyb.DummyLines = extralines ;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);
		
		OV12830TRULYB_SetDummy(ov12830trulyb.DummyPixels,extralines);
    }
	else if(ov12830trulyb.sensorMode == SENSOR_MODE_CAPTURE)
	{
		OV12830TRULYBDB("-------[OV12830TRULYBSetVideoMode]ZSD???---------\n");
		if(ov12830trulyb.OV12830TRULYBAutoFlickerMode == KAL_TRUE)
    	{
    		if (u2FrameRate==15)
			    frameRate= 148;
			else
				frameRate=u2FrameRate*10;

			MIN_Frame_length = (OV12830TRULYBMIPI_CAPTURE_CLK) /(OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/frameRate*10;
    	}
		else
			MIN_Frame_length = (OV12830TRULYBMIPI_CAPTURE_CLK) /(OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/u2FrameRate;

		if((MIN_Frame_length <=OV12830TRULYB_FULL_PERIOD_LINE_NUMS))
		{
			MIN_Frame_length = OV12830TRULYB_FULL_PERIOD_LINE_NUMS;
			//OV12830TRULYBDB("[OV12830TRULYBSetVideoMode]current fps = %d\n", (OV12830TRULYBMIPI_CAPTURE_CLK) /(OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS)/OV12830TRULYB_FULL_PERIOD_LINE_NUMS);

		}
		//OV12830TRULYBDB("[OV12830TRULYBSetVideoMode]current fps (10 base)= %d\n", (OV12830TRULYBMIPI_CAPTURE_CLK)*10/(OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS + ov12830trulyb.DummyPixels)/MIN_Frame_length);
		extralines = MIN_Frame_length - OV12830TRULYB_FULL_PERIOD_LINE_NUMS;

		spin_lock(&ov12830trulybmipiraw_drv_lock);
		ov12830trulyb.DummyPixels = 0;//define dummy pixels and lines
		ov12830trulyb.DummyLines = extralines ;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);

		OV12830TRULYB_SetDummy(ov12830trulyb.DummyPixels,extralines);
	}
	OV12830TRULYBDB("[OV12830TRULYBSetVideoMode]MIN_Frame_length=%d,ov12830trulyb.DummyLines=%d\n",MIN_Frame_length,ov12830trulyb.DummyLines);

    return KAL_TRUE;
}

UINT32 OV12830TRULYBSetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	//return ERROR_NONE;
    //OV12830TRULYBDB("[OV12830TRULYBSetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);
	if(bEnable) {   // enable auto flicker
		spin_lock(&ov12830trulybmipiraw_drv_lock);
		ov12830trulyb.OV12830TRULYBAutoFlickerMode = KAL_TRUE;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);
    } else {
    	spin_lock(&ov12830trulybmipiraw_drv_lock);
        ov12830trulyb.OV12830TRULYBAutoFlickerMode = KAL_FALSE;
		spin_unlock(&ov12830trulybmipiraw_drv_lock);
        OV12830TRULYBDB("Disable Auto flicker\n");
    }

    return ERROR_NONE;
}

UINT32 OV12830TRULYBSetTestPatternMode(kal_bool bEnable)
{
    OV12830TRULYBDB("[OV12830TRULYBSetTestPatternMode] Test pattern enable:%d\n", bEnable);

    return ERROR_NONE;
}


UINT32 OV12830TRULYBMIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	OV12830TRULYBDB("OV12830TRULYBMIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = OV12830TRULYBMIPI_PREVIEW_CLK;
			lineLength = OV12830TRULYB_PV_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV12830TRULYB_PV_PERIOD_LINE_NUMS;
			ov12830trulyb.sensorMode = SENSOR_MODE_PREVIEW;
			OV12830TRULYB_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = OV12830TRULYBMIPI_VIDEO_CLK; 
			lineLength = OV12830TRULYB_VIDEO_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV12830TRULYB_VIDEO_PERIOD_LINE_NUMS;
			ov12830trulyb.sensorMode = SENSOR_MODE_VIDEO;
			OV12830TRULYB_SetDummy(0, dummyLine);			
			break;			
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = OV12830TRULYBMIPI_CAPTURE_CLK;
			lineLength = OV12830TRULYB_FULL_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV12830TRULYB_FULL_PERIOD_LINE_NUMS;
			ov12830trulyb.sensorMode = SENSOR_MODE_CAPTURE;
			OV12830TRULYB_SetDummy(0, dummyLine);			
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
			break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			break;		
		default:
			break;
	}	
	return ERROR_NONE;
}


UINT32 OV12830TRULYBMIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 150;
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			 *pframeRate = 300;
			break;		
		default:
			break;
	}

	return ERROR_NONE;
}

UINT32 OV12830TRULYBFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++= OV12830TRULYB_IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16= OV12830TRULYB_IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
				*pFeatureReturnPara16++= OV12830TRULYB_FeatureControl_PERIOD_PixelNum;
				*pFeatureReturnPara16= OV12830TRULYB_FeatureControl_PERIOD_LineNum;
				*pFeatureParaLen=4;
				break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			switch(OV12830TRULYBCurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
					*pFeatureReturnPara32 = OV12830TRULYBMIPI_PREVIEW_CLK;
					*pFeatureParaLen=4;
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*pFeatureReturnPara32 = OV12830TRULYBMIPI_VIDEO_CLK;
					*pFeatureParaLen=4;
					break;
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara32 = OV12830TRULYBMIPI_CAPTURE_CLK;
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara32 = OV12830TRULYBMIPI_CAPTURE_CLK;
					*pFeatureParaLen=4;
					break;
			}
		      break;

        case SENSOR_FEATURE_SET_ESHUTTER:
            OV12830TRULYB_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            OV12830TRULYB_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            OV12830TRULYB_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            //OV12830TRULYB_isp_master_clock=*pFeatureData32;
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            OV12830TRULYB_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = OV12830TRULYB_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&ov12830trulybmipiraw_drv_lock);
                OV12830TRULYBSensorCCT[i].Addr=*pFeatureData32++;
                OV12830TRULYBSensorCCT[i].Para=*pFeatureData32++;
				spin_unlock(&ov12830trulybmipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=OV12830TRULYBSensorCCT[i].Addr;
                *pFeatureData32++=OV12830TRULYBSensorCCT[i].Para;
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&ov12830trulybmipiraw_drv_lock);
                OV12830TRULYBSensorReg[i].Addr=*pFeatureData32++;
                OV12830TRULYBSensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&ov12830trulybmipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=OV12830TRULYBSensorReg[i].Addr;
                *pFeatureData32++=OV12830TRULYBSensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=OV12830TRULYB_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, OV12830TRULYBSensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, OV12830TRULYBSensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &OV12830TRULYBSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            OV12830TRULYB_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            OV12830TRULYB_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=OV12830TRULYB_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            OV12830TRULYB_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            OV12830TRULYB_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            OV12830TRULYB_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            OV12830TRULYBSetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV12830TRULYBGetSensorID(pFeatureReturnPara32);
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            OV12830TRULYBSetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));
	        break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            OV12830TRULYBSetTestPatternMode((BOOL)*pFeatureData16);
            break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			OV12830TRULYBMIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			OV12830TRULYBMIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
        default:
            break;
    }
    return ERROR_NONE;
}	/* OV12830TRULYBFeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncOV12830TRULYB=
{
    OV12830TRULYBOpen,
    OV12830TRULYBGetInfo,
    OV12830TRULYBGetResolution,
    OV12830TRULYBFeatureControl,
    OV12830TRULYBControl,
    OV12830TRULYBClose
};

UINT32 OV12830TRULYB_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncOV12830TRULYB;

    return ERROR_NONE;
}   /* SensorInit() */
