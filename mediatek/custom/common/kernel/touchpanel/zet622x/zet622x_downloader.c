#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include "cust_gpio_usage.h"
#include "tpd.h"
#include "zet622x_fw.h"

#define ZET6221_DOWNLOADER_NAME "zet622x_downloader"

//#define TS_INT_GPIO		GPIO75
//#define TS_RST_GPIO		GPIO86
//#define High_Impendence_Mode

#define GPIO_LOW 	0
#define GPIO_HIGH 	1

#define debug_mode 0
#define DPRINTK(fmt,args...)	do { if (debug_mode) printk(KERN_EMERG "[%s][%d] "fmt"\n", __FUNCTION__, __LINE__, ##args);} while(0)

static unsigned char zeitec_zet622x_page[131] __initdata;
static unsigned char zeitec_zet622x_page_in[131] __initdata;

static u16 fb[8] = {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF}; 
static u8 ic_model = 0;	// 0:6221 / 1:6223
static u16 fb21[8] = {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF}; 
static u16 fb23[8] = {0x7BFC,0x7BFD,0x7BFE,0x7BFF,0x7C04,0x7C05,0x7C06,0x7C07};

extern s32 zet6221_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length);
extern s32 zet6221_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length);
extern u8 pc[];

//#define I2C_CTPM_ADDRESS        (0x76)

u8 zet622x_ts_sndpwd(struct i2c_client *client)
{
	u8 ts_sndpwd_cmd[3] = {0x20,0xC5,0x9D};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_sndpwd_cmd, 3);

	return ret;
}

u8 zet622x_ts_option(struct i2c_client *client)
{

	u8 ts_cmd[1] = {0x27};
	u8 ts_cmd_erase[1] = {0x28};
	u8 ts_in_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 ts_out_data[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ret;
	u16 model;
	int i;
	
	printk("\noption write : "); 

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	msleep(5);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\nread : "); 

	ret=zet6221_i2c_read_tsdata(client, ts_in_data, 16);

	msleep(1);

	for(i=0;i<16;i++)
	{
		printk("%02x ",ts_in_data[i]); 
	}
	printk("\n"); 

	model = 0x0;
	model = ts_in_data[7];
	model = (model << 8) | ts_in_data[6];
	
	switch(model) { 
        case 0xFFFF: 
        	ret = 1;
            ic_model = 0;
			for(i=0;i<8;i++)
			{
				fb[i]=fb21[i];
			}
			
#if defined(High_Impendence_Mode)

			if(ts_in_data[2] != 0xf1)
			{
			
				if(zet622x_ts_sfr(client)==0)
				{
					return 0;
				}
			
				ret=zet6221_i2c_write_tsdata(client, ts_cmd_erase, 1);
				msleep(50);
			
				for(i=2;i<18;i++)
				{
					ts_out_data[i]=ts_in_data[i-2];
				}
				ts_out_data[0] = 0x21;
				ts_out_data[1] = 0xc5;
				ts_out_data[4] = 0xf1;
			
				ret=zet6221_i2c_write_tsdata(client, ts_out_data, 18);
				msleep(50);
			
				//
				ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

				msleep(5);
	
				printk("%02x ",ts_cmd[0]); 
	
				printk("\n(2)read : "); 

				ret=zet6221_i2c_read_tsdata(client, ts_in_data, 16);

				msleep(1);

				for(i=0;i<16;i++)
				{
					printk("%02x ",ts_in_data[i]); 
				}
				printk("\n"); 
				
			}
									
#endif					
            break; 
        case 0x6231:  
        case 0x6223:
        	ret = 1;
			ic_model = 1;
			for(i=0;i<8;i++)
			{
				fb[i]=fb23[i];
			}
            break; 
        default: 
        	ret = 1;
			ic_model = 1;
			for(i=0;i<8;i++)
			{
				fb[i]=fb23[i];
			}
            break;         
            //ret=0; 
    } 

	return ret;
}

u8 zet622x_ts_sfr(struct i2c_client *client)
{

	u8 ts_cmd[1] = {0x2C};
	u8 ts_in_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 ts_cmd17[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ret;
	int i;
	
	printk("\nwrite : "); 

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	msleep(5);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\nread : "); 

	ret=zet6221_i2c_read_tsdata(client, ts_in_data, 16);

	msleep(1);

	for(i=0;i<16;i++)
	{
		ts_cmd17[i+1]=ts_in_data[i];
		printk("%02x ",ts_in_data[i]); 
	}
	printk("\n"); 

#if 1
	if(ts_in_data[14]!=0x3D && ts_in_data[14]!=0x7D)
	{
		return 0;
	}
#endif

	if(ts_in_data[14]!=0x3D)
	{
		ts_cmd17[15]=0x3D;
		
		ts_cmd17[0]=0x2B;	
		
		ret=zet6221_i2c_write_tsdata(client, ts_cmd17, 17);
	}
	
	return 1;
}

u8 zet622x_ts_masserase(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x24};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	return 1;
}

u8 zet622x_ts_pageerase(struct i2c_client *client,int npage)
{
	u8 ts_cmd[3] = {0x23,0x00,0x00};
	u8 len=0;
	int ret;

	switch(ic_model)
	{
		case 0: //6221
			ts_cmd[1]=npage;
			len=2;
			break;
		case 1: //6223
			ts_cmd[1]=npage & 0xff;
			ts_cmd[2]=npage >> 8;
			len=3;
			break;
	}

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, len);

	return 1;
}

u8 zet622x_ts_resetmcu(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x29};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	return 1;
}

u8 zet622x_ts_hwcmd(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0xB9};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	return 1;
}

u8 zet622x_ts_version()
{	
	int i;
		
	printk("pc: ");
	for(i=0;i<8;i++)
		printk("%02x ",pc[i]);
	printk("\n");
	
	printk("src: ");
	for(i=0;i<8;i++)
		printk("%02x ",zeitec_zet622x_firmware[fb[i]]);
	printk("\n");
	
	for(i=0;i<8;i++)
		if(pc[i]!=zeitec_zet622x_firmware[fb[i]])
			return 0;
			
	return 1;
}

//support compatible
int __init zet622x_downloader( struct i2c_client *client )
{
	int BufLen=0;
	int BufPage=0;
	int BufIndex=0;
	int ret;
	int i;
	
	int nowBufLen=0;
	int nowBufPage=0;
	int nowBufIndex=0;
	int retryCount=0;
	
	int i2cLength=0;
	int bufOffset=0;
	
begin_download:
	
	//reset mcu
	//gpio_direction_output(TS_RST_GPIO, GPIO_LOW);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(20);


	//send password
	ret=zet622x_ts_sndpwd(client);
	msleep(200);
	
	if(ret<=0)
		return ret;
		
	ret=zet622x_ts_option(client);
	msleep(200);
	
	if(ret<=0)
		return ret;

/*****compare version*******/

	//0~3
	memset(zeitec_zet622x_page_in,0x00,131);
	
	switch(ic_model)
	{
		case 0: //6221
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[0] >> 7);//(fb[0]/128);
			
			i2cLength=2;
			break;
		case 1: //6223
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[0] >> 7) & 0xff; //(fb[0]/128);
			zeitec_zet622x_page_in[2]=(fb[0] >> 7) >> 8; //(fb[0]/128);
			
			i2cLength=3;
			break;
	}
	
	ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page_in, i2cLength);

	if(ret<=0)
		return ret;
	
	zeitec_zet622x_page_in[0]=0x0;
	zeitec_zet622x_page_in[1]=0x0;
	zeitec_zet622x_page_in[2]=0x0;

	ret=zet6221_i2c_read_tsdata(client, zeitec_zet622x_page_in, 128);


	if(ret<=0)
		return ret;
	
	printk("page=%d ",(fb[0] >> 7));//(fb[0]/128));
	for(i=0;i<4;i++)
	{
		pc[i]=zeitec_zet622x_page_in[(fb[i] & 0x7f)];//[(fb[i]%128)];
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));//(fb[i]%128));
	}
	printk("\n");
	
	/*
	printk("page=%d ",(fb[0] >> 7));
	for(i=0;i<4;i++)
	{
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));
	}
	printk("\n");
	*/
	
	//4~7
	memset(zeitec_zet622x_page_in,0x00,131);
	
	switch(ic_model)
	{
		case 0: //6221
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[4] >> 7);//(fb[4]/128);
			
			i2cLength=2;
			break;
		case 1: //6223
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[4] >> 7) & 0xff; //(fb[4]/128);
			zeitec_zet622x_page_in[2]=(fb[4] >> 7) >> 8; //(fb[4]/128);
			
			i2cLength=3;
			break;
	}
	
	ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page_in, i2cLength);

	if(ret<=0)
		return ret;
	
	zeitec_zet622x_page_in[0]=0x0;
	zeitec_zet622x_page_in[1]=0x0;
	zeitec_zet622x_page_in[2]=0x0;

	ret=zet6221_i2c_read_tsdata(client, zeitec_zet622x_page_in, 128);


	printk("page=%d ",(fb[4] >> 7)); //(fb[4]/128));
	for(i=4;i<8;i++)
	{
		pc[i]=zeitec_zet622x_page_in[(fb[i] & 0x7f)]; //[(fb[i]%128)];
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));  //(fb[i]%128));
	}
	printk("\n");
	
	if(ret<=0)
		return ret;
	

	if(zet622x_ts_version()!=0)
		goto exit_download;
		
/*****compare version*******/

proc_sfr:
	//sfr
	if(zet622x_ts_sfr(client)==0)
	{
		//comment out to disable sfr checking loop
		return 0;
#if 0

		//gpio_direction_output(TS_RST_GPIO, GPIO_HIGH);
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(20);
		
		//gpio_direction_output(TS_RST_GPIO, GPIO_LOW);
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(20);
		
		//gpio_direction_output(TS_RST_GPIO, GPIO_HIGH);
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(20);
		goto begin_download;
		
#endif

	}
	msleep(20);
	
	//comment out to enable download procedure
	//return 1;
	
	//erase
	if(BufLen==0)
	{
		//mass erase
		//DPRINTK( "mass erase\n");
		zet622x_ts_masserase(client);
		msleep(200);

		BufLen=sizeof(zeitec_zet622x_firmware)/sizeof(char);
	}else
	{
		zet622x_ts_pageerase(client,BufPage);
		msleep(200);
	}
	
	
	while(BufLen>0)
	{
download_page:

		memset(zeitec_zet622x_page,0x00,131);
		
		DPRINTK( "Start: write page%d\n",BufPage);
		nowBufIndex=BufIndex;
		nowBufLen=BufLen;
		nowBufPage=BufPage;
		
		switch(ic_model)
		{
			case 0: //6221
				bufOffset = 2;
				i2cLength=130;
				
				zeitec_zet622x_page[0]=0x22;
				zeitec_zet622x_page[1]=BufPage;				
				break;
			case 1: //6223
				bufOffset = 3;
				i2cLength=131;
				
				zeitec_zet622x_page[0]=0x22;
				zeitec_zet622x_page[1]=BufPage & 0xff;
				zeitec_zet622x_page[2]=BufPage >> 8;
				break;
		}
		
		if(BufLen>128)
		{
			for(i=0;i<128;i++)
			{
				zeitec_zet622x_page[i+bufOffset]=zeitec_zet622x_firmware[BufIndex];
				BufIndex+=1;
			}

			BufLen-=128;
		}
		else
		{
			for(i=0;i<BufLen;i++)
			{
				zeitec_zet622x_page[i+bufOffset]=zeitec_zet622x_firmware[BufIndex];
				BufIndex+=1;
			}

			BufLen=0;
		}
//		DPRINTK( "End: write page%d\n",BufPage);

		ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page, i2cLength);

		msleep(50);
		
#if 1

		memset(zeitec_zet622x_page_in,0x00,131);
		switch(ic_model)
		{
			case 0: //6221
				zeitec_zet622x_page_in[0]=0x25;
				zeitec_zet622x_page_in[1]=BufPage;
				
				i2cLength=2;
				break;
			case 1: //6223
				zeitec_zet622x_page_in[0]=0x25;
				zeitec_zet622x_page_in[1]=BufPage & 0xff;
				zeitec_zet622x_page_in[2]=BufPage >> 8;

				i2cLength=3;
				break;
		}		
		
		ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page_in, i2cLength);

		zeitec_zet622x_page_in[0]=0x0;
		zeitec_zet622x_page_in[1]=0x0;
		zeitec_zet622x_page_in[2]=0x0;
		
		ret=zet6221_i2c_read_tsdata(client, zeitec_zet622x_page_in, 128);

		
		for(i=0;i<128;i++)
		{
			if(i < nowBufLen)
			{
				if(zeitec_zet622x_page[i+bufOffset]!=zeitec_zet622x_page_in[i])
				{
					BufIndex=nowBufIndex;
					BufLen=nowBufLen;
					BufPage=nowBufPage;
				
					if(retryCount < 5)
					{
						retryCount++;
						goto download_page;
					}else
					{
						//BufIndex=0;
						//BufLen=0;
						//BufPage=0;
						retryCount=0;
						
						//gpio_direction_output(TS_RST_GPIO, GPIO_HIGH);
						mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
						mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
						mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
						msleep(20);
		
						//gpio_direction_output(TS_RST_GPIO, GPIO_LOW);
						mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
						mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
						mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
						msleep(20);
		
						//gpio_direction_output(TS_RST_GPIO, GPIO_HIGH);
						mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
						mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
						mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
						msleep(20);
						
						goto begin_download;
					}

				}
			}
		}
		
#endif
		retryCount=0;
		BufPage+=1;
	}

exit_download:

	zet622x_ts_resetmcu(client);
	msleep(100);

	//gpio_direction_output(TS_RST_GPIO, GPIO_HIGH);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(200);

	return 1;

}

