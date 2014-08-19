/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/

//#include <config.h>
//#include <common.h>
//#include <version.h>
//#include <stdarg.h>
//#include <linux/types.h>
//#include <devices.h>
//#include <lcd.h>
//#include <video_fb.h>

//#include <video_fb.h>


//#include <asm/arch/mt65xx_typedefs.h>
//#include <asm/arch/mt65xx_disp_drv.h>
//#include <asm/arch/disp_drv.h>
//#include <asm/arch/lcd_drv.h>
//#include <asm/arch/mt65xx_logo.h>

#include <platform/mt_typedefs.h>
#include <platform/mt_disp_drv.h>
#include <platform/disp_drv.h>
#include <platform/lcd_drv.h>
#include <platform/mt_logo.h>
#include <platform/disp_drv_platform.h>

#include <target/board.h>

#include "lcm_drv.h"

//#include <cust_display.h>
#include <target/cust_display.h>

//#include <u-boot/zlib.h>
#include <lib/zlib.h>
#include <string.h>

extern unsigned long long get_ticks(void);

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static const UINT32 VOLTAGE_SCALE = 4;

typedef struct {
    UINT32 left, top, right, bottom;
} RECT;

static RECT bar_rect = {BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM};

static UINT32 bar_occupied_color = BAR_OCCUPIED_COLOR;
static UINT32 bar_empty_color    = BAR_EMPTY_COLOR;
static UINT32 bar_bg_color       = BAR_BG_COLOR;

static LOGO_CUST_IF *logo_cust_if = NULL;

// for new 


static int number_pic_width = NUMBER_RIGHT - NUMBER_LEFT;       //width
int number_pic_size = (NUMBER_RIGHT - NUMBER_LEFT)*(NUMBER_BOTTOM - NUMBER_TOP)*2;   //size
char number_pic_addr[(NUMBER_RIGHT - NUMBER_LEFT)*(NUMBER_BOTTOM - NUMBER_TOP)*2] = {0x0}; //addr
RECT number_location_rect = {NUMBER_LEFT,NUMBER_TOP,NUMBER_RIGHT,NUMBER_BOTTOM}; 
static int number_pic_start_0 = 4;


int line_pic_size = (TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*2;
char line_pic_addr[(TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*2] = {0x0};
RECT battery_rect = {CAPACITY_LEFT,CAPACITY_TOP,CAPACITY_RIGHT,CAPACITY_BOTTOM};


int percent_pic_size = (PERCENT_RIGHT - PERCENT_LEFT)*(PERCENT_BOTTOM - PERCENT_TOP)*2;
char percent_pic_addr[(PERCENT_RIGHT - PERCENT_LEFT)*(PERCENT_BOTTOM - PERCENT_TOP)*2] = {0x0};
RECT percent_location_rect = {PERCENT_LEFT,PERCENT_TOP,PERCENT_RIGHT,PERCENT_BOTTOM};

static int top_animation_height = TOP_ANIMATION_BOTTOM - TOP_ANIMATION_TOP;
int top_animation_size = (TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*(TOP_ANIMATION_BOTTOM - TOP_ANIMATION_TOP)*2;
char top_animation_addr[(TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*(TOP_ANIMATION_BOTTOM - TOP_ANIMATION_TOP)*2] = {0x0};
RECT top_animation_rect = {TOP_ANIMATION_LEFT,TOP_ANIMATION_TOP,TOP_ANIMATION_RIGHT,TOP_ANIMATION_BOTTOM};

int charging_low_index = 0;
int charging_animation_index = 0;

int capacity_zero = 0;
int capacity_zero_index = 0;
//for new


void show_animation_number(UINT32 index,UINT32 number_position);
void show_animation_line(UINT32 index,UINT32 capacity_grids);
void show_animation(UINT32 index, RECT rect, char* addr);


bool mt_logo_decompress(void *in, void *out, int inlen, int outlen)
{
    int ret;
    z_stream strm;

    memset(&strm, 0, sizeof(z_stream));
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = inlen; 
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = outlen;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void mt_logo_get_custom_if(void)
{
    if(logo_cust_if == NULL)
    {
        logo_cust_if = (LOGO_CUST_IF *)LOGO_GetCustomIF();
    }
}



static void show_logo(UINT32 index)
{
	UINT32 logonum;
    UINT32 logolen;
	UINT32 inaddr;
    void  *fb_addr = mt_get_fb_addr();
    void  *fb_tempaddr = mt_get_tempfb_addr();
    UINT32 fb_size = mt_get_fb_size();
    void  *db_addr = mt_get_logo_db_addr();

	unsigned int *pinfo = (unsigned int*)db_addr;
    logonum = pinfo[0];
	
	ASSERT(index < logonum);

	if(index < logonum)
		logolen = pinfo[3+index] - pinfo[2+index];
	else
		logolen = pinfo[1] - pinfo[2+index];

	inaddr = (unsigned int)db_addr+pinfo[2+index];
    printf("show_logo, in_addr=0x%08x, fb_addr=0x%08x, logolen=%d, ticks=%d\n", 
                inaddr, fb_addr, logolen, get_ticks());
//    mt_logo_decompress((void*)inaddr, (void*)fb_addr + 2 * fb_size, logolen, fb_size); 
#if 1
	{
		unsigned short *d;
		unsigned int j,k;
		int t;
		if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
		{
			unsigned int l;
			unsigned short *s;
			unsigned int width = CFG_DISPLAY_WIDTH;
			unsigned int height = CFG_DISPLAY_HEIGHT;
			mt_logo_decompress((void*)inaddr, fb_tempaddr, logolen, fb_size);
			s = fb_tempaddr;
			for (j=0; j<width; j++){
		  		for (k=0, l=height-1; k<height; k++, l--)
		    	{
					d = fb_addr + ((ALIGN_TO(width, MTK_FB_ALIGNMENT) * l + j) << 1);
					*d = *s++;
		    	}
			}
		}
		else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
		{
			unsigned int l;
			unsigned short *s;
			unsigned int width = CFG_DISPLAY_WIDTH;
			unsigned int height = CFG_DISPLAY_HEIGHT;
			mt_logo_decompress((void*)inaddr, fb_tempaddr, logolen, fb_size);
			s = fb_tempaddr;
			for (t=width - 1; t>=0; t--){
		  		for (k=0, l=0; k<height; k++, l++)
		    	{
					d = fb_addr + ((ALIGN_TO(width, MTK_FB_ALIGNMENT) * l + t) << 1);
					*d = *s++;
		    	}
			}
		}
		else
#endif		
		{
			if(0 != CFG_DISPLAY_WIDTH % 32){
				unsigned short *s;
				unsigned short *d;
				unsigned int width = CFG_DISPLAY_WIDTH;
				unsigned int height = CFG_DISPLAY_HEIGHT;
				mt_logo_decompress((void*)inaddr, fb_tempaddr, logolen, fb_size);
				s = fb_tempaddr;
				d = fb_addr;
				for (j=0;j < height; j++){
		    		{
						memcpy(d, s, width * 2);
						d += ALIGN_TO(width, MTK_FB_ALIGNMENT);
						s += width;
		    		}
				}
			}
			else{
				mt_logo_decompress((void*)inaddr, (void*)fb_addr, logolen, fb_size); 
			}
		}
	}
    printf("ticks=%d\n", get_ticks());
}


void cust_show_battery_capacity(UINT32 capacity)
{
	DISP_PROG_BAR_DIRECT direct = DISP_VERTICAL_PROG_BAR;
    UINT32 capacity_grids = 0;
 
    if (capacity > 100) capacity = 100;

    capacity_grids = (capacity * VOLTAGE_SCALE) / 100;

    show_logo(1);

    // Fill Occupied Color
    
    mt_disp_draw_prog_bar(direct,
                              bar_rect.left + 1, bar_rect.top + 1,
                              bar_rect.right, bar_rect.bottom,
                              bar_occupied_color, bar_bg_color,
                              0, VOLTAGE_SCALE, capacity_grids);
    
    // Fill Empty Color
    
    mt_disp_draw_prog_bar(direct,
                              bar_rect.left + 1, bar_rect.top + 1,
                              bar_rect.right, bar_rect.bottom,
                              bar_empty_color, bar_bg_color,
                              capacity_grids, VOLTAGE_SCALE,
                              VOLTAGE_SCALE - capacity_grids);

    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
}

void cust_show_battery_capacity_new(UINT32 capacity)
{

        printf("[ChargingAnimation]capacity : %d\n", capacity);

        if (capacity <= 0) 
          {
             //show_logo(2);
             //mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);	
             //return;

            //capacity_zero ++;
            // if (capacity_zero > 10) capacity_zero = 2;
            // if (capacity_zero > 1)
            //   {
                  show_logo(29+capacity_zero_index);
                  show_animation_number(number_pic_start_0,1);
                  show_animation(14, percent_location_rect, percent_pic_addr);
                  if(++capacity_zero_index > 1)  capacity_zero_index = 0;
                  mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);	
                  return;
           //    }               
          }
        //else 
        //  {
        //     capacity_zero = 0;
        //  }

        
        if (capacity < 10 && capacity > 0) 
          {
             charging_low_index ++ ;
             show_logo(25 + charging_low_index);
             if (charging_low_index >= 9) charging_low_index = 0;
             show_animation_number(number_pic_start_0+capacity,1);
             show_animation(14, percent_location_rect, percent_pic_addr);
             mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);	
             return;
          }

        if (capacity >= 100) 
          {
             show_logo(37); // battery 100
             mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);	
             return;
          }

	UINT32 capacity_grids = 0;     
	//if (capacity > 100) capacity = 100;
	capacity_grids = battery_rect.bottom - (battery_rect.bottom - battery_rect.top) * (capacity - 10) / 90;
	printf("[ChargingAnimation]capacity : %d\n", capacity);
        printf("[ChargingAnimation]capacity_grids : %d\n", capacity_grids);
        show_logo(35);  // show background
        show_animation_line(36,capacity_grids); // show battery capacity line
        
        top_animation_rect.bottom = capacity_grids;
        top_animation_rect.top = capacity_grids - top_animation_height;
        if (capacity <= 90) 
          {
            charging_animation_index++;
            printf("[ChargingAnimation]top_animation : left = %d, top = %d, right = %d, bottom = %d\n",  top_animation_rect.left, top_animation_rect.top,  top_animation_rect.right, top_animation_rect.bottom);
            show_animation(15 + charging_animation_index, top_animation_rect, top_animation_addr); // show top animation
            if (charging_animation_index >= 9) charging_animation_index = 0;
         }

	if (capacity < 10)
	{
		show_animation_number(number_pic_start_0+capacity,1);
	}
	
	if ((capacity >= 10)&&(capacity < 100))
	{
		show_animation_number(number_pic_start_0+(capacity/10),0);
		show_animation_number(number_pic_start_0+(capacity%10),1);	
	}
        show_animation(14, percent_location_rect, percent_pic_addr);

        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
}

static void fill_rect_flow(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, char *addr)
{
    void * fb_addr = mt_get_fb_addr();
    const UINT32 WIDTH = ALIGN_TO(CFG_DISPLAY_WIDTH,MTK_FB_ALIGNMENT);
    const UINT32 HEIGHT = CFG_DISPLAY_HEIGHT;
    UINT16 *pLine = (UINT16 *)fb_addr + top * WIDTH + left;	
    UINT32 x, y;
    INT32 i = 0;
    INT32 t = 0;
    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
	{
    printf("[ChargingAnimation]fill_rect_flow : MTK_LCM_PHYSICAL_ROTATION = 270\n");
	unsigned int l;
        UINT16 *d = fb_addr;
	UINT16 *pLine2 = (UINT16*)addr;	
		for (x=top; x<bottom; x++) {
	  		for (y=left, l= HEIGHT - left; y<right; y++, l--)
	    	        {
				d = fb_addr + ((WIDTH * l + x) << 1);
				*d = pLine2[i++];
	    	        }
		}
	}
    else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
	{
   printf("[ChargingAnimation]fill_rect_flow : MTK_LCM_PHYSICAL_ROTATION = 90\n");
        unsigned int l;
        UINT16 *d = fb_addr;
        UINT16 *pLine2 = (UINT16*)addr;
		for (t=WIDTH - top + 1; t > (INT32)(WIDTH - bottom); t--) {
			for (y=left, l=left; y<right; y++, l++)
		   	{
				d = fb_addr + ((WIDTH * l + t) << 1);
				*d = pLine2[i++];
		   	}
		}
	}
   else 
        {
	UINT16 *pLine2 = (UINT16*)addr;
           for (y = top; y < bottom; ++ y) {
              UINT16 *pPixel = pLine;
              for (x = left; x < right; ++ x) {
		*pPixel++ = pLine2[i++];
              }
              pLine += WIDTH;
           }
        }
}

static void fill_rect_flow2(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, char *addr)
{
    void * fb_addr = mt_get_fb_addr();
    const UINT32 WIDTH = ALIGN_TO(CFG_DISPLAY_WIDTH,MTK_FB_ALIGNMENT);

    UINT16 *pLine = (UINT16 *)fb_addr + top * WIDTH + left;
	
    UINT32 x, y;
	INT32 i = 0;
	UINT16 *pLine2 = (UINT16*)addr;
    for (y = top; y < bottom; ++ y) {
        UINT16 *pPixel = pLine;
        for (x = left; x < right; ++ x) {
	*pPixel++ = pLine2[i++];
        }
        pLine += WIDTH;
    }
}

static void fill_line_flow2(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, char *addr)
{
    void * fb_addr = mt_get_fb_addr();
    const UINT32 WIDTH = ALIGN_TO(CFG_DISPLAY_WIDTH,MTK_FB_ALIGNMENT);

    UINT16 *pLine = (UINT16 *)fb_addr + top * WIDTH + left;
	
    UINT32 x, y;
	INT32 i = 0;

	UINT16 *pLine2 = (UINT16*)addr;
    for (y = top; y < bottom; ++ y) {
        UINT16 *pPixel = pLine;
        for (x = left; x < right; ++ x) {;
	*pPixel++ = pLine2[i++];
        }
        i = 0;
        pLine += WIDTH;
    }
}

static void fill_line_flow(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, char *addr)
{
    void * fb_addr = mt_get_fb_addr();
    const UINT32 WIDTH = ALIGN_TO(CFG_DISPLAY_WIDTH,MTK_FB_ALIGNMENT);
    const UINT32 HEIGHT = CFG_DISPLAY_HEIGHT;
    UINT16 *pLine = (UINT16 *)fb_addr + top * WIDTH + left;
    UINT16 *pLine2 = (UINT16*)addr;		
    UINT32 x, y;
    INT32 i = 0;
    INT32 t = 0;
    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
	{
	unsigned int l;
        UINT16 *d = fb_addr;
		for (x=top; x<bottom; x++) {
	  		for (y=left, l= HEIGHT - left; y<right; y++, l--)
	    	        {
				d = fb_addr + ((WIDTH * l + x) << 1);
				*d = pLine2[i++];
	    	        }
                    i = 0;
		}
	}
    else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
	{
        unsigned int l;
        UINT16 *d = fb_addr;
		for (t=WIDTH - top + 1; t > (INT32)(WIDTH - bottom); t--) {
			for (y=left, l=left; y<right; y++, l++)
		   	{
				d = fb_addr + ((WIDTH * l + t) << 1);
				*d = pLine2[i++];
		   	}
                    i = 0;
		}
	}
   else 
        {
           for (y = top; y < bottom; ++ y) {
              UINT16 *pPixel = pLine;
              for (x = left; x < right; ++ x) {
		*pPixel++ = pLine2[i++];
              }
              pLine += WIDTH;
              i = 0;
           }
        }
}


void show_animation(UINT32 index, RECT rect, char* addr){
	        UINT32 logonum;
		UINT32 logolen;
		UINT32 inaddr;
                void  *db_addr = mt_get_logo_db_addr();
	
		unsigned int *pinfo = (unsigned int*)db_addr;
		logonum = pinfo[0];
		
		printf("[ChargingAnimation]show_animation : index = %d, logonum = %d\n", index, logonum);
		//ASSERT(index < logonum);

		printf("[ChargingAnimation]show_animation :pinfo[0] = %d, pinfo[1] = %d, pinfo[2]= %d, pinfo[3] = %d\n",pinfo[0] , pinfo[1],pinfo[2],pinfo[3]);
		printf("[ChargingAnimation]show_animation :pinfo[2+index] = %d, pinfo[1] = %d\n",pinfo[2+index] , pinfo[3+index]);
		if(index < logonum)
			logolen = pinfo[3+index] - pinfo[2+index];
		else
			logolen = pinfo[1] - pinfo[2+index];
	
		inaddr = (unsigned int)db_addr+pinfo[2+index];
		printf("[ChargingAnimation]show_animation: in_addr=0x%08x, dest_addr=0x%08x, logolen=%d, ticks=%d\n", 
					inaddr, logolen, logolen, get_ticks());
	
		mt_logo_decompress((void*)inaddr, (void*)addr, logolen, (rect.right-rect.left)*(rect.bottom-rect.top)*2);
                printf("[ChargingAnimation]show_animation : rect right = %d\n", rect.right);
                printf("[ChargingAnimation]show_animation : rect top = %d\n", rect.top);
                printf("[ChargingAnimation]show_animation : rect size = %d\n", (rect.right-rect.left)*(rect.bottom-rect.top)*2);

		fill_rect_flow(rect.left,rect.top,rect.right,rect.bottom,addr);
		printf("ticks=%d\n", get_ticks());
		
	}

// number_position: 0~1st number, 1~2nd number ,2~%
void show_animation_number(UINT32 index,UINT32 number_position){
	UINT32 logonum;
	UINT32 logolen;
	UINT32 inaddr;
        void  *db_addr = mt_get_logo_db_addr();

	unsigned int *pinfo = (unsigned int*)db_addr;
	logonum = pinfo[0];
	
	printf("[ChargingAnimation]show_animation_number :index= %d, logonum = %d\n", index, logonum);
	//ASSERT(index < logonum);

	if(index < logonum)
		logolen = pinfo[3+index] - pinfo[2+index];
	else
		logolen = pinfo[1] - pinfo[2+index];

	inaddr = (unsigned int)db_addr+pinfo[2+index];
	printf("[ChargingAnimation]show_animation_number, in_addr=0x%08x, dest_addr=0x%08x, logolen=%d, ticks=%d\n", 
				inaddr, logolen, logolen, get_ticks());

	//windows draw default 160 180,
	mt_logo_decompress((void*)inaddr, (void*)number_pic_addr, logolen, number_pic_size);

	fill_rect_flow(number_location_rect.left+ number_pic_width*number_position,
						number_location_rect.top,
						number_location_rect.right+number_pic_width*number_position,
						number_location_rect.bottom,number_pic_addr);

	//mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
}


void show_animation_line(UINT32 index,UINT32 capacity_grids){
	UINT32 logonum;
	UINT32 logolen;
	UINT32 inaddr;
        void  *db_addr = mt_get_logo_db_addr();

	unsigned int *pinfo = (unsigned int*)db_addr;
	logonum = pinfo[0];
	
	printf("[ChargingAnimation]show_animation_line :index= %d, logonum = %d\n", index, logonum);
	//ASSERT(index < logonum);

	if(index < logonum)
		logolen = pinfo[3+index] - pinfo[2+index];
	else
		logolen = pinfo[1] - pinfo[2+index];

	inaddr = (unsigned int)db_addr+pinfo[2+index];
	printf("[ChargingAnimation]show_animation_line, in_addr=0x%08x, dest_addr=0x%08x, logolen=%d, ticks=%d\n", 
				inaddr, logolen, logolen, get_ticks());

	//windows draw default 160 180,
	mt_logo_decompress((void*)inaddr, (void*)line_pic_addr, logolen, line_pic_size);
        printf("[ChargingAnimation]show_animation_line :line_pic_size= %d, line_pic_addr size = %d\n", line_pic_size, sizeof(line_pic_addr));

	fill_line_flow(battery_rect.left, capacity_grids, battery_rect.right, battery_rect.bottom, line_pic_addr);
	
}


void mt_disp_show_boot_logo(void)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->show_boot_logo)
    {
    	logo_cust_if->show_boot_logo();
    }
    else
    {
        show_logo(0);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

    return;
}

void mt_disp_enter_charging_state(void)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->enter_charging_state)
    {
    	logo_cust_if->enter_charging_state();
    }
    else
    {

    }

    return;
}

void mt_disp_show_battery_full(void)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->show_battery_full)
    {
    	logo_cust_if->show_battery_full();
    }
    else
    {
        #ifdef ANIMATION_NEW
        cust_show_battery_capacity_new(100);
        #else
        cust_show_battery_capacity(100);
        #endif
    }

    return;
}

void mt_disp_show_battery_capacity(UINT32 capacity)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->show_battery_capacity)
    {
    	logo_cust_if->show_battery_capacity(capacity);
    }
    else
    {   
        #ifdef ANIMATION_NEW
        cust_show_battery_capacity_new(capacity);
        #else
        cust_show_battery_capacity(capacity);
        #endif
    }

    return;
}

void mt_disp_show_charger_ov_logo(void)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->show_boot_logo)
    {
    	logo_cust_if->show_boot_logo();
    }
    else
    {
        show_logo(3);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

    return;
}

void mt_disp_show_low_battery(void)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->show_boot_logo)
    {
    	logo_cust_if->show_boot_logo();
    }
    else
    {
        show_logo(2);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

    return;
}

void mt_disp_fill_rect(UINT32 left, UINT32 top,
                           UINT32 right, UINT32 bottom,
                           UINT32 color)
{
    void * fb_addr = mt_get_fb_addr();
    const UINT32 WIDTH = ALIGN_TO(CFG_DISPLAY_WIDTH, MTK_FB_ALIGNMENT);
	const UINT32 HEIGHT = CFG_DISPLAY_HEIGHT;
    const UINT16 COLOR = (UINT16)color;

    UINT16 *pLine;
    UINT32 x, y;
    INT32 t;
	pLine = (UINT16 *)fb_addr + top * WIDTH + left;
#if 1
	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
	{
		unsigned int l;
        UINT16 *d = fb_addr;
		
		for (x=top; x<bottom; x++){
	  		for (y=left, l= HEIGHT - left; y<right; y++, l--)
	    	{
				d = fb_addr + ((WIDTH * l + x) << 1);
				*d = COLOR;
	    	}
		}
	}
	else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
	{
		unsigned int l;
        UINT16 *d = fb_addr;
		for (t=WIDTH - top + 1; t > (INT32)(WIDTH - bottom); t--){
			for (y=left, l=left; y<right; y++, l++)
		   	{
				d = fb_addr + ((WIDTH * l + t) << 1);
				*d = COLOR;
		   	}
		}
	}
	else
#endif
	{
    	for (y = top; y < bottom; ++ y) {
        	UINT16 *pPixel = pLine;
        	for (x = left; x < right; ++ x) {
            	*pPixel++ = COLOR;
        	}
        	pLine += WIDTH;
    	}
	}
}

void mt_disp_draw_prog_bar(DISP_PROG_BAR_DIRECT direct,
                               UINT32 left, UINT32 top,
                               UINT32 right, UINT32 bottom,
                               UINT32 fgColor, UINT32 bgColor,
                               UINT32 start_div, UINT32 total_div,
                               UINT32 occupied_div)
{
    const UINT32 PADDING = 3;
    UINT32 div_size  = (bottom - top) / total_div;
    UINT32 draw_size = div_size - PADDING;
    
    UINT32 i;

    if (DISP_HORIZONTAL_PROG_BAR == direct) 
	{
		div_size = (right - left) / total_div;
		draw_size = div_size - PADDING;
    	for (i = start_div; i < start_div + occupied_div; ++ i)
    	{
			UINT32 draw_left = left + div_size * i + PADDING;
			UINT32 draw_right = draw_left + draw_size;

        	// fill one division of the progress bar
        	mt_disp_fill_rect(draw_left, top, draw_right, bottom, fgColor);
		}
    }
	else if(DISP_VERTICAL_PROG_BAR == direct)
	{
		div_size  = (bottom - top) / total_div;
    	draw_size = div_size - PADDING;
 
    	for (i = start_div; i < start_div + occupied_div; ++ i)
    	{
        	UINT32 draw_bottom = bottom - div_size * i - PADDING;
        	UINT32 draw_top    = draw_bottom - draw_size;

        	// fill one division of the progress bar
        	mt_disp_fill_rect(left, draw_top, right, draw_bottom, fgColor);
    	}
	}
	else
	{
		printf("[ChargingAnimation]unimplement, line %d\n", __LINE__);
	}
}
