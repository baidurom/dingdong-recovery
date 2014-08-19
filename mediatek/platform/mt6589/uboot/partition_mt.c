#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mtk_nand.h>
#include "cust_nand.h"
#include "mt65xx_partition.h"
#include "pmt.h"

#define PMT 1

//common
//BLK_SIZE is 512, block_size is from flash is 128K
static u32 block_size;
static u32 page_size;
extern flashdev_info devinfo;
extern pt_resident lastest_part[PART_MAX_COUNT];
extern part_t mt6575_parts[];

extern int total_size;
extern struct NAND_CMD g_kCMD;
static pt_info pi;

//if used malloc func ,the pdata = (uchar*)malloc(sizeof(uchar)*size);
// in recovery_check_command_trigger will return 0
//static char *page_buf;  

unsigned char page_buf[4096+128];


#ifdef PMT
void get_part_tab_from_complier(void)
{
	int index=0;
	printf("get_pt_from_complier \n");
	while(mt6575_parts[index].flags!= PART_FLAG_END)
	{
    		
		memcpy(lastest_part[index].name,mt6575_parts[index].name,MAX_PARTITION_NAME_LEN);
		lastest_part[index].size = mt6575_parts[index].blknum*BLK_SIZE ;
		lastest_part[index].offset = mt6575_parts[index].startblk * BLK_SIZE;
		lastest_part[index].mask_flags =  mt6575_parts[index].flags;  //this flag in kernel should be fufilled even though in flash is 0.
		printf ("get_ptr  %s %lx \n",lastest_part[index].name,lastest_part[index].offset);
		index++;
	}
}

bool find_mirror_pt_from_bottom(int *start_addr,part_dev_t *dev)
{
	int mpt_locate;
	int mpt_start_addr;
	int current_start_addr=0;
	char pmt_spare[4];
	mpt_start_addr = total_size+block_size;
	//mpt_start_addr=MPT_LOCATION*block_size-page_size;
	for(mpt_locate=(block_size/page_size);mpt_locate>0;mpt_locate--)
	{
		memset(pmt_spare,0xFF,PT_SIG_SIZE);
		
		current_start_addr = mpt_start_addr+mpt_locate*page_size;
		if(!dev->read(dev,current_start_addr, page_buf,page_size))
		{
			printf ("find_mirror read  failed %x %x \n",current_start_addr,mpt_locate);
		}
		memcpy(&page_buf[page_size],g_kCMD.au1OOB,16);
		memcpy(pmt_spare,&page_buf[page_size] ,PT_SIG_SIZE);
		//need enhance must be the larget sequnce number
		
		if(is_valid_mpt(page_buf)&&is_valid_mpt(&pmt_spare))
		{
		      //if no pt, pt.has space is 0;
			pi.sequencenumber = page_buf[PT_SIG_SIZE+page_size];
			printf ("find_mirror find valid pt at %x sq %x \n",current_start_addr,pi.sequencenumber);
			break;
		}
		else
		{
			continue;
		}
	}
	if(mpt_locate==0)
	{
		printf ("no valid mirror page\n");
		pi.sequencenumber =  0;
		return FALSE;
	}
	else
	{
		*start_addr = current_start_addr;
		return TRUE;
	}
}
int load_exist_part_tab(u8 *buf,part_dev_t *dev)
{
	int pt_start_addr;
	int pt_cur_addr;
	int pt_locate;
	int reval=DM_ERR_OK;
	int mirror_address;
	char pmt_spare[PT_SIG_SIZE];

	block_size= devinfo.blocksize*1024;
	page_size = devinfo.pagesize;
	
	//page_buf = malloc(page_size);	 

	pt_start_addr = total_size;
	printf("load_pt from 0x%x \n",pt_start_addr);
	//pt_start_addr=PT_LOCATION*block_size;
	for(pt_locate=0;pt_locate<(block_size/page_size);pt_locate++)
	{
		pt_cur_addr = pt_start_addr+pt_locate*page_size;
		memset(pmt_spare,0xFF,PT_SIG_SIZE);

		if(!dev->read(dev,pt_cur_addr, page_buf,page_size))
		{
			printf ("load_pt read pt failded: %x\n",pt_cur_addr);
		}
             memcpy(&page_buf[page_size],g_kCMD.au1OOB,16);
		#if 0
		{
			int i;
			for(i=0;i<8;i++)
			{
				printf ("%x %x \n",page_buf[i],page_buf[2048+i]);
			}

		}
		#endif
		memcpy(pmt_spare,&page_buf[page_size] ,PT_SIG_SIZE); //skip bad block flag
		if(is_valid_pt(page_buf)&&is_valid_pt(pmt_spare))
		{
			pi.sequencenumber = page_buf[PT_SIG_SIZE+page_size];
			printf("load_pt find valid pt at %x sq %x \n",pt_start_addr,pi.sequencenumber);
			break;
		}
		else
		{
			continue;
		}
	}
	//for test 
	//pt_locate==(block_size/page_size);
	if(pt_locate==(block_size/page_size))
	{
		//first download or download is not compelte after erase or can not download last time
		printf ("load_pt find pt failed \n");
		pi.pt_has_space = 0; //or before download pt power lost
		
		if(!find_mirror_pt_from_bottom(&mirror_address,dev))
		{
			printf ("First time download \n");
			reval=ERR_NO_EXIST;
			return reval;
		}
		else
		{
			//used the last valid mirror pt, at lease one is valid.
			dev->read(dev,mirror_address, page_buf,page_size);
		}
	}
	memcpy(buf,&page_buf[PT_SIG_SIZE],sizeof(lastest_part));

	return reval;
}
void part_init_pmt(unsigned long totalblks,part_dev_t *dev)
{
	part_t *part = &mt6575_parts[0];
	unsigned long lastblk;
	int retval=0;
	int i=0;
	printf ("mt6575_part_init_pmt \n");

	if (!totalblks) return;

	/* updater the number of blks of first part. */
	if (totalblks <= part->blknum)
	part->blknum = totalblks;

	totalblks -= part->blknum;
	lastblk = part->startblk + part->blknum;

	while(totalblks) 
	{
		part++;
		if (!part->name)
		break;

		if (part->flags & PART_FLAG_LEFT || totalblks <= part->blknum)
		part->blknum = totalblks;

		part->startblk = lastblk;
		totalblks -= part->blknum;
		lastblk = part->startblk + part->blknum;
	}
	
	memset(&pi,0xFF,sizeof(pi));
	memset(&lastest_part,0,PART_MAX_COUNT*sizeof(pt_resident));
	retval=load_exist_part_tab((u8 *)&lastest_part,dev);
	if (retval==ERR_NO_EXIST) //first run preloader before dowload
	{
		//and valid mirror last download or first download 
		printf ("no pt \n");
		get_part_tab_from_complier(); //get from complier
	}
	else
	{
		printf ("Find pt \n");
		for(i=0;i<13;i++)
		{	
			printf ("partition %s size %lx %lx \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
		}
	}
}

#endif
