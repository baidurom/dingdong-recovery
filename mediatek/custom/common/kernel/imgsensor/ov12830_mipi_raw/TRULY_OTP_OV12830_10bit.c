/*
NOTE:
The modification is appended to initialization of image sensor. 
After sensor initialization, use the function
bool otp_update_wb(unsigned char golden_rg, unsigned char golden_bg)
and
bool otp_update_lenc(void)
and
int otp_update_BLC(void) 
then the calibration of AWB & LSC & BLC will be applied. 
After finishing the OTP written, we will provide you the typical value of golden sample.
*/
/*
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
	
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
	
#include "OV12830_Sensor.h"
#include "OV12830_Camera_Sensor_para.h"
#include "OV12830_CameraCustomized.h"
*/
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


extern kal_uint16 OV12830_write_cmos_sensor(kal_uint16 addr, kal_uint32 para);
extern kal_uint16 OV12830_read_cmos_sensor(kal_uint32 addr);

//#define SUPPORT_FLOATING
#define TRACE printk
#define OTP_DATA_ADDR         0x3D00
#define OTP_LOAD_ADDR         0x3D81
#define OTP_BANK_ADDR         0x3D84

#define LENC_START_ADDR       0x5800
#define LENC_REG_SIZE         62

#define OTP_LENC_GROUP_ADDR   0x3D00

#define OTP_WB_GROUP_ADDR     0x3D00
#define OTP_WB_GROUP_SIZE     16

#define GAIN_RH_ADDR          0x3400
#define GAIN_RL_ADDR          0x3401
#define GAIN_GH_ADDR          0x3402
#define GAIN_GL_ADDR          0x3403
#define GAIN_BH_ADDR          0x3404
#define GAIN_BL_ADDR          0x3405

#define GAIN_DEFAULT_VALUE    0x0400 // 1x gain

#define OTP_HDS_MID               0x0b
#define OTP_TRULY_MID               0x02
#define Module_IR               0x01


// R/G and B/G of current camera module
unsigned short rg_ratio = 0;
unsigned short bg_ratio = 0;
unsigned char otp_lenc_data[62];
unsigned char  IR_type;

// Enable OTP read function
void otp_read_enable(void)
{
	OV12830_write_cmos_sensor(OTP_LOAD_ADDR, 0x01);
	mdelay(15); // sleep > 10ms
}

// Disable OTP read function
void otp_read_disable(void)
{
	OV12830_write_cmos_sensor(OTP_LOAD_ADDR, 0x00);
	mdelay(15); // sleep > 10ms
}

void otp_read(unsigned short otp_addr, unsigned char* otp_data)
{
	otp_read_enable();
	*otp_data = OV12830_read_cmos_sensor(otp_addr);
	otp_read_disable();
}

/*******************************************************************************
* Function    :  otp_clear
* Description :  Clear OTP buffer 
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
void otp_clear(void)
{
	// After read/write operation, the OTP buffer should be cleared to avoid accident write
	unsigned char i;
	for (i=0; i<16; i++) 
	{
		OV12830_write_cmos_sensor(OTP_DATA_ADDR+i, 0x00);
	}
}

/*******************************************************************************
* Function    :  otp_check_wb_group
* Description :  Check OTP Space Availability
* Parameters  :  [in] index : index of otp group (0, 1, 2)
* Return      :  0, group index is empty
                 1, group index has invalid data
                 2, group index has valid data
                -1, group index error
*******************************************************************************/	
signed char otp_check_wb_group(unsigned char index)
{   
	unsigned char  flag;

    if (index > 2)
	{
		TRACE("OTP input wb group index %d error\n", index);
		return -1;
	}
		
	// select bank 1-3
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (index+1));

	otp_read(OTP_WB_GROUP_ADDR, &flag);
	otp_clear();

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	
	flag &= 0xc0;
	if (!flag)
	{
		TRACE("wb group %d is empty\n", index);
		return 0;
	}
	else if (flag == 0x40)
	{
		TRACE("wb group %d has valid data\n", index);
		return 2;
	}
	else
	{
		TRACE("wb group %d has invalid data\n", index);
		return 1;
	}
}

/*******************************************************************************
* Function    :  otp_read_wb_group
* Description :  Read group value and store it in OTP Struct 
* Parameters  :  [in] index : index of otp group (0, 1, 2)
* Return      :  group index (0, 1, 2)
                 -1, error
*******************************************************************************/	
signed char otp_read_wb_group(signed char index)
{
	unsigned char  mid, AWB_light_LSB, rg_ratio_MSB, bg_ratio_MSB;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_wb_group(index) == 2)
			{
				TRACE("read wb from group %d\n", index);
				break;
			}
		}

		if (index > 2)
		{
			TRACE("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_wb_group(index) != 2)
		{
			TRACE("read wb from group %d failed\n", index);
			return -1;
		}
	}

	// select bank 1-3
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (index+1));

	otp_read(OTP_WB_GROUP_ADDR+1, &mid);
        printk("-------qingzhan the mid is %x---------\n",mid);
	if ((mid != OTP_TRULY_MID)&(mid != OTP_HDS_MID))
	{
		return -1;
	}
	otp_read(OTP_WB_GROUP_ADDR+11, &IR_type);
	if (IR_type == Module_IR)
	{
		TRACE("the type of IR is blue IR\n");
	}
	else if (IR_type== 0)
		
  {
		TRACE("the type of IR is simple IR\n");
	}
	otp_read(OTP_WB_GROUP_ADDR+6,  &rg_ratio_MSB);
	otp_read(OTP_WB_GROUP_ADDR+7,  &bg_ratio_MSB);
	otp_read(OTP_WB_GROUP_ADDR+10, &AWB_light_LSB);	
	otp_clear();
	rg_ratio = (rg_ratio_MSB<<2) | ((AWB_light_LSB & 0xC0)>>6);
	bg_ratio = (bg_ratio_MSB<<2) | ((AWB_light_LSB & 0x30)>>4);
	TRACE("read wb finished\n");
	return index;
}

#ifdef SUPPORT_FLOATING //Use this if support floating point values
/*******************************************************************************
* Function    :  otp_apply_wb
* Description :  Calcualte and apply R, G, B gain to module
* Parameters  :  [in] golden_rg : R/G of golden camera module
                 [in] golden_bg : B/G of golden camera module
* Return      :  1, success; 0, fail
*******************************************************************************/	
bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;

	double ratio_r, ratio_g, ratio_b;
	double cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		TRACE("golden_rg / golden_bg can not be zero\n");
		return 0;
	}

	// Calcualte R, G, B gain of current module from R/G, B/G of golden module
        // and R/G, B/G of current module
	cmp_rg = 1.0 * rg_ratio / golden_rg;
	cmp_bg = 1.0 * bg_ratio / golden_bg;

	if ((cmp_rg<1) && (cmp_bg<1))
	{
		// R/G < R/G golden, B/G < B/G golden
		ratio_g = 1;
		ratio_r = 1 / cmp_rg;
		ratio_b = 1 / cmp_bg;
	}
	else if (cmp_rg > cmp_bg)
	{
		// R/G >= R/G golden, B/G < B/G golden
		// R/G >= R/G golden, B/G >= B/G golden
		ratio_r = 1;
		ratio_g = cmp_rg;
		ratio_b = cmp_rg / cmp_bg;
	}
	else
	{
		// B/G >= B/G golden, R/G < R/G golden
		// B/G >= B/G golden, R/G >= R/G golden
		ratio_b = 1;
		ratio_g = cmp_bg;
		ratio_r = cmp_bg / cmp_rg;
	}

	// write sensor wb gain to registers
	// 0x0400 = 1x gain
	if (ratio_r != 1)
	{
		gain_r = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_r);
		OV12830_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV12830_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1)
	{
		gain_g = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_g);
		OV12830_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV12830_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1)
	{
		gain_b = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_b);
		OV12830_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV12830_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	TRACE("cmp_rg=%f, cmp_bg=%f\n", cmp_rg, cmp_bg);
	TRACE("ratio_r=%f, ratio_g=%f, ratio_b=%f\n", ratio_r, ratio_g, ratio_b);
	TRACE("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}

#else //Use this if not support floating point values

#define OTP_MULTIPLE_FAC	10000
bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;

	unsigned short ratio_r, ratio_g, ratio_b;
	unsigned short cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		TRACE("golden_rg / golden_bg can not be zero\n");
		return 0;
	}

	// Calcualte R, G, B gain of current module from R/G, B/G of golden module
    // and R/G, B/G of current module
	cmp_rg = OTP_MULTIPLE_FAC * rg_ratio / golden_rg;
	cmp_bg = OTP_MULTIPLE_FAC * bg_ratio / golden_bg;

	if ((cmp_rg < 1 * OTP_MULTIPLE_FAC) && (cmp_bg < 1 * OTP_MULTIPLE_FAC))
	{
		// R/G < R/G golden, B/G < B/G golden
		ratio_g = 1 * OTP_MULTIPLE_FAC;
		ratio_r = 1 * OTP_MULTIPLE_FAC * OTP_MULTIPLE_FAC / cmp_rg;
		ratio_b = 1 * OTP_MULTIPLE_FAC * OTP_MULTIPLE_FAC / cmp_bg;
	}
	else if (cmp_rg > cmp_bg)
	{
		// R/G >= R/G golden, B/G < B/G golden
		// R/G >= R/G golden, B/G >= B/G golden
		ratio_r = 1 * OTP_MULTIPLE_FAC;
		ratio_g = cmp_rg;
		ratio_b = OTP_MULTIPLE_FAC * cmp_rg / cmp_bg;
	}
	else
	{
		// B/G >= B/G golden, R/G < R/G golden
		// B/G >= B/G golden, R/G >= R/G golden
		ratio_b = 1 * OTP_MULTIPLE_FAC;
		ratio_g = cmp_bg;
		ratio_r = OTP_MULTIPLE_FAC * cmp_bg / cmp_rg;
	}

	// write sensor wb gain to registers
	// 0x0400 = 1x gain
	if (ratio_r != 1 * OTP_MULTIPLE_FAC)
	{
		gain_r = GAIN_DEFAULT_VALUE * ratio_r / OTP_MULTIPLE_FAC;
		OV12830_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV12830_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1 * OTP_MULTIPLE_FAC)
	{
		gain_g = GAIN_DEFAULT_VALUE * ratio_g / OTP_MULTIPLE_FAC;
		OV12830_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV12830_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1 * OTP_MULTIPLE_FAC)
	{
		gain_b = GAIN_DEFAULT_VALUE * ratio_b / OTP_MULTIPLE_FAC;
		OV12830_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV12830_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	TRACE("cmp_rg=%d, cmp_bg=%d\n", cmp_rg, cmp_bg);
	TRACE("ratio_r=%d, ratio_g=%d, ratio_b=%d\n", ratio_r, ratio_g, ratio_b);
	TRACE("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}
#endif /* SUPPORT_FLOATING */

/*******************************************************************************
* Function    :  otp_update_wb
* Description :  Update white balance settings from OTP
* Parameters  :  [in] golden_rg : R/G of golden camera module
                 [in] golden_bg : B/G of golden camera module
* Return      :  1, success; 0, fail
*******************************************************************************/	
bool otp_update_wb(unsigned short golden_rg, unsigned short golden_bg) 
{
	TRACE("start wb update\n");

	if (otp_read_wb_group(-1) != -1)
	{
		if (otp_apply_wb(golden_rg, golden_bg) == 1)
		{
			TRACE("wb update finished\n");
			return 1;
		}
	}

	TRACE("wb update failed\n");
	return 0;
}

/*******************************************************************************
* Function    :  otp_check_lenc_group
* Description :  Check OTP Space Availability
* Parameters  :  [in] BYTE index : index of otp group (0, 1, 2)
* Return      :  0, group index is empty
                 1, group index has invalid data
                 2, group index has valid data
                -1, group index error
*******************************************************************************/	
signed char otp_check_lenc_group(BYTE index)
{   
	unsigned char  flag;
	unsigned char  bank;

    if (index > 2)
	{
		TRACE("OTP input lenc group index %d error\n", index);
		return -1;
	}
		
	// select bank: index*4 + 4
	bank = 0xc0 | (index*4 + 4);
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_read(OTP_LENC_GROUP_ADDR, &flag);
	otp_clear();

	flag &= 0xc0;

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	if (!flag)
	{
		TRACE("lenc group %d is empty\n", index);
		return 0;
	}
	else if (flag == 0x40)
	{
		TRACE("lenc group %d has valid data\n", index);
		return 2;
	}
	else
	{
		TRACE("lenc group %d has invalid data\n", index);
		return 1;
	}
}

/*******************************************************************************
* Function    :  otp_read_lenc_group
* Description :  Read group value and store it in OTP Struct 
* Parameters  :  [in] int index : index of otp group (0, 1, 2)
* Return      :  group index (0, 1, 2)
                 -1, error
*******************************************************************************/	
signed char otp_read_lenc_group(int index)
{
	unsigned short otp_addr;
	unsigned char  bank;
	unsigned char  i;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_lenc_group(index) == 2)
			{
				TRACE("read lenc from group %d\n", index);
				break;
			}
		}

		if (index > 2)
		{
			TRACE("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_lenc_group(index) != 2) 
		{
			TRACE("read lenc from group %d failed\n", index);
			return -1;
		}
	}

	// select bank: index*4 + 4
	bank = 0xc0 | (index*4 + 4);
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR+1;

	otp_read_enable();
	for (i=0; i<15; i++) 
	{
		otp_lenc_data[i] = OV12830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();

	// select next bank
	bank++;
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=15; i<31; i++) 
	{
		otp_lenc_data[i] = OV12830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	// select next bank
	bank++;
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=31; i<47; i++) 
	{
		otp_lenc_data[i] = OV12830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	// select next bank
	bank++;
	OV12830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=47; i<62; i++) 
	{
		otp_lenc_data[i] = OV12830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	TRACE("read lenc finished\n");
	return index;
}

/*******************************************************************************
* Function    :  otp_apply_lenc
* Description :  Apply lens correction setting to module
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
void otp_apply_lenc(void)
{
	// write lens correction setting to registers
	TRACE("apply lenc setting\n");

	unsigned char i;


	for (i=0; i<LENC_REG_SIZE; i++)
	{
		OV12830_write_cmos_sensor(LENC_START_ADDR+i, otp_lenc_data[i]);
		TRACE("0x%x, 0x%x\n", LENC_START_ADDR+i, otp_lenc_data[i]);
	}
}

/*******************************************************************************
* Function    :  otp_update_lenc
* Description :  Get lens correction setting from otp, then apply to module
* Parameters  :  none
* Return      :  1, success; 0, fail
*******************************************************************************/	
bool otp_update_lenc(void) 
{
	TRACE("start lenc update\n");

	if (otp_read_lenc_group(-1) != -1)
	{
		otp_apply_lenc();
		TRACE("lenc update finished\n");
		return 1;
	}

	TRACE("lenc update failed\n");
	return 0;
}

/*******************************************************************************
* Function    :  otp_update_BLC
* Description :  Get BLC value from otp, then apply to module
* Parameters  :  none
* Return      :  0, set to 0x20; 1, use data from 0x3D0A; 2,use data from 0x3D0B
*******************************************************************************/
int otp_update_BLC(void) 
{
	unsigned char  k;
	unsigned char  temp;
	//select bank 31
	OV12830_write_cmos_sensor(0x3d84, 0xdf);
	otp_read(0x3d0b, &k);
	if(k!=0)
	{
		if((k>=0x15) && (k<=0x40))
		{
			//auto load mode
			temp = OV12830_read_cmos_sensor(0x4008);
			temp &=0xfb;
   			OV12830_write_cmos_sensor(0x4008, temp);
   			mdelay(20); 
    			temp = OV12830_read_cmos_sensor(0x4000);
			temp &=0xf7;
   			OV12830_write_cmos_sensor(0x4000, temp);
   			mdelay(20); 
   			TRACE("BLC value from 0x3d0b: %x\n",k);
   			return 2;
   		}
	}
	
	otp_read(0x3d0a, &k);
	
	if((k>=0x10) && (k<=0x40))
	{
		//manual load mode
		OV12830_write_cmos_sensor(0x4006, k);
		temp = OV12830_read_cmos_sensor(0x4008);
		temp &=0xfb;
   		OV12830_write_cmos_sensor(0x4008, temp);
   		mdelay(20); 
    		temp = OV12830_read_cmos_sensor(0x4000);
		temp |=0x08;
   		OV12830_write_cmos_sensor(0x4000, temp);
   		mdelay(20); 
   		TRACE("BLC value from 0x3d0a:%x\n",k);
   		return 1;	
	}
	else
	{
		//set to default
		OV12830_write_cmos_sensor(0x4006, 0x20);
		temp = OV12830_read_cmos_sensor(0x4008);
		temp &=0xfb;
   		OV12830_write_cmos_sensor(0x4008, temp);
   		mdelay(20); 
    		temp = OV12830_read_cmos_sensor(0x4000);
		temp |=0x08;
   		OV12830_write_cmos_sensor(0x4000, temp);
   		mdelay(20); 
   		TRACE("BLC value is set to 0x20\n");
	}
	
	return 0; 	
}
