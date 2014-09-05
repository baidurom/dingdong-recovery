/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * Source code for parsing and processing edify script (dd-config)
 *
 */

#include <sys/stat.h>       //-- Filesystem Stats
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "../edify/expr.h"

#include "../dd_inter.h"
#include "../dd.h"

#define APARSE_MAXHISTORY 256

//* 
//* GLOBAL UI VARIABLES
//* 


static  CANVAS      dd_bg;                 //-- Saved CANVAS for background
static  CANVAS      dd_win_bg;             //-- Current drawed CANVAS for windows background
static  byte        dd_isbgredraw  		 = 0;
static  int         dd_minY          	 = 0;
static  int         dd_virkeys_home_X    = 0;
static  int         dd_virkeys_home_Y    = 0;
static  int         dd_virkeys_back_X    = 0;
static  int         dd_virkeys_back_Y    = 0;
static  int         dd_virkeysW     	 = 0;
static  int         dd_virkeysH    		 = 0;
static  int         dd_title_x      	 = 0;
static  int         dd_title_y      	 = 0;

//* 
//* MACROS
//* 
#define MIN_BAR_HEGIHT				       64
#define MAX_FILE_GETPROP_SIZE         	   65536
#define BANNER_HEIGTH_HOME  	 	       agh()*3/10
#define BANNER_HEIGTH_SUB        		   agh()*3/10
#define VIRTUAL_KEYS_HEIGHT				   agdp()*20
#define VIRTUAL_KEYS_WIDTH				   agw()/2

/************************************[ DD INSTALLER UI - LIBRARIES ]************************************/


#define _INITARGS() \
    int args_i =0; \
    va_list arg_ptr; \
    char **args = (char**)malloc(argc * sizeof(char *)); \
    va_start(arg_ptr, format); \
    args[0] = format; \
    for (args_i = 1; args_i < argc; args_i++) \
        args[args_i] = va_arg(arg_ptr, char*);

#define _FREEARGS() \
    va_end(arg_ptr); \
    free(args);

static pthread_mutex_t title_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t redraw_mutex = PTHREAD_MUTEX_INITIALIZER;

static int dd_draw_homebanner(int x, int y, int w, int h) {
	if (!h) return 0;
	char welcome[256];
    char message[512];
    PNGCANVAS ap;

    snprintf(welcome, 256, "<#dialogbg><b><@left><~banner.info.welcome></@></b></#>"
    		"<#dialogbg><b><@left><~banner.info.recoveryname></@></b></#>"
    		,
    		acfg()->rec_name);
    snprintf(message, 512,
    " <b><#dialogbg><~banner.info.device></#></b>  <b>%s</b>\n"
    " <b><#dialogbg><~banner.info.version></#></b>  <b>%s</b>\n"
    " <b><#dialogbg><~banner.info.author></#></b>  <b>%s</b>\n"
    ,
    acfg()->rec_device,
    acfg()->rec_version,
#ifdef DD_USE_AUTHOR_INNER
    DD_AUTHOR
#else
    acfg()->rec_author
#endif
    );
	if (!atheme_id_draw(24, &dd_win_bg, x, y, w, h)){
	    printf("Load home banner failed!\n");
	    return 0;
	}

	int titH     = ag_fontheight(1)*2;
	int txtH     = ag_fontheight(0)*5;
	int pad = agdp()*4;
    int imgE = 0;
    int imgW = 0;
    int imgH = 0;
    int imgX = 0;
    int imgY = 0;
    int titX = 0;
    int titY = 0;
    int txtX = 0;
    int txtY = 0;
    int tmaxW = 0;
    if (apng_load(&ap, BANNER_ICON)){
    	imgE = 1;
        imgW = min(ap.w, w/2);
        imgH = min(ap.h, h);
        if (imgW >= w/2) imgX = 0;
        else imgX = (w/2 - imgW)/2;
        if (imgH >= h) imgY = y;
        else imgY = (h - imgH)/2 + y;
        titX = txtX = w/2;
        tmaxW = agw() - titX - pad*2;
    } else {
    	printf("Can not load %s\n", BANNER_ICON);
    	titX = txtX = pad;
    	tmaxW = agw() - pad*2;
    }
    titY = (h - titH - txtH - 4*pad)/2  + y;
    txtY = titY + titH + 4*pad;
    if(imgE) {
        apng_draw_ex(&dd_win_bg, &ap, imgX, imgY, 0, 0, imgW, imgH);
        apng_close(&ap);
    }
    //ag_textf(&dd_win_bg, tmaxW, titX+1, titY+1, welcome, acfg()->textbg, 1);
    ag_text(&dd_win_bg, tmaxW, titX, titY, welcome, acfg()->textfg, 2);
    //ag_textf(&dd_win_bg, tmaxW, txtX+1, txtY+1, message, acfg()->titlefg, 0);
    ag_text(&dd_win_bg, tmaxW, txtX, txtY, message, acfg()->titlefg, 1);
	return h;
}

static int dd_draw_subbanner(int x, int y, int w, int h) {
	if (!h) return 0;
	if (!atheme_id_draw(25, &dd_win_bg, x, y, w, h)){
	    printf("Load submenu banner failed!\n");
	    return 0;
	}
	PNGCANVAS ap;
	int pad = agdp()*4;
    int imgE = 0;
    int imgW = 0;
    int imgH = 0;
    int imgX = 0;
    int imgY = 0;
    if (apng_load(&ap, SUBBANNER_ICON)){
    	imgE = 1;
        imgW = min(ap.w, w);
        imgH = min(ap.h, h);
        if (imgW >= w) imgX = 0;
        else imgX = (w - imgW)/2;
        if (imgH >= h) imgY = y;
        else imgY = (h - imgH)/2 + y;
    }
    if(imgE) {
        apng_draw_ex(&dd_win_bg, &ap, imgX, imgY, 0, 0, imgW, imgH);
        apng_close(&ap);
    }
	return h;
}

static void draw_virtualkeys(AWINDOWP win) {
	if (acfg()->virkeys_en) {
		// Show home virtual key
		acvirkeys(win, dd_virkeys_home_X, dd_virkeys_home_Y, dd_virkeysW, dd_virkeysH, 1, 7);
		// Show back virtual key
		acvirkeys(win, dd_virkeys_back_X, dd_virkeys_back_Y, dd_virkeysW, dd_virkeysH, 0, 8);
	}
}

STATUS dd_set_isbgredraw(int value)
{
    pthread_mutex_lock(&redraw_mutex);
    dd_isbgredraw = value;
    pthread_mutex_unlock(&redraw_mutex);
    return RET_OK;
}

//* 
//* Redraw Window Background
//* 
void dd_redraw(){
  if (!dd_isbgredraw) return;
  ag_blank(&dd_bg);
  ag_rect(&dd_bg,0,0,agw(),agh(),0x0000);

  int elmP  = agdp()*4;
  int capH = agw()/10;
  if (capH  < MIN_BAR_HEGIHT)
	  capH = MIN_BAR_HEGIHT;
  dd_minY  = capH;
  PNGCANVAS ap;
  byte imgE = 0;
  int  imgW = 0;
  int  imgH = 0;
  int  imgX = 0;
  int  imgY = 0;
  //Calculate and draw logo & title location
  if (apng_load(&ap,TITLEBAR_LOGO)){
      imgE = 1;
      imgW = ap.w;
      imgH = ap.h;
      imgX = elmP;
      imgY = (capH-imgH)/2;
      dd_title_x = imgX + imgW + elmP;
      dd_title_y = (capH - ag_fontheight(1)) / 2;
  } else {
	  capH  = ag_fontheight(1) + (elmP*2);
	  dd_minY  = capH;
	  dd_title_x = elmP;
	  dd_title_y = elmP;
  }

  //-- Background
  if (!atheme_id_draw(0, &dd_bg, 0, 0, agw(),agh())){
    ag_roundgrad(&dd_bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  }

  //-- Titlebar
  if (!atheme_id_draw(1, &dd_bg, 0, 0, agw(),capH)){
    ag_roundgrad_ex(&dd_bg,0,0,agw(),capH,acfg()->titlebg,acfg()->titlebg_g,(acfg()->winroundsz*agdp())-2,1,1,0,0);
  }

  //-- Logo
  if(imgE){
      apng_draw(&dd_bg,&ap,imgX,imgY);
      apng_close(&ap);
  }

  //-- Virtual keys
  if (acfg()->virkeys_en) {
	  dd_virkeysW       = agw()/2;
	  dd_virkeysH       = elmP*5;
	  dd_virkeys_home_X = 0;
	  dd_virkeys_home_Y = agh()-dd_virkeysH;
	  dd_virkeys_back_X = agw()/2;
	  dd_virkeys_back_Y = agh()-dd_virkeysH;
  }
  dd_isbgredraw = 0;
}

//* 
//* Redraw Window Background
//*
void dd_redraw_otherbg(){
  if (!dd_isbgredraw) return;
  ag_blank(&dd_bg);
  ag_rect(&dd_bg,0,0,agw(),agh(),0x0000);

  int elmP  = agdp()*4;
  int capH = agw()/10;
  if (capH  < MIN_BAR_HEGIHT)
	  capH = MIN_BAR_HEGIHT;
  dd_minY  = capH;
  PNGCANVAS ap;
  byte imgE = 0;
  int  imgW = 0;
  int  imgH = 0;
  int  imgX = 0;
  int  imgY = 0;
  //Calculate and draw logo & title location
  if (apng_load(&ap,TITLEBAR_LOGO)){
      imgE = 1;
      imgW = ap.w;
      imgH = ap.h;
      imgX = elmP;
      imgY = (capH-imgH)/2;
      dd_title_x = imgX + imgW + elmP;
      dd_title_y = (capH - ag_fontheight(1)) / 2;
  } else {
	  capH  = ag_fontheight(1) + (elmP*2);
	  dd_minY  = capH;
	  dd_title_x = elmP;
	  dd_title_y = elmP;
  }

  //-- Background
  if (!atheme_id_draw(26, &dd_bg, 0, 0, agw(),agh())){
    ag_roundgrad(&dd_bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  }

  //-- Titlebar
  if (!atheme_id_draw(1, &dd_bg, 0, 0, agw(),capH)){
    ag_roundgrad_ex(&dd_bg,0,0,agw(),capH,acfg()->titlebg,acfg()->titlebg_g,(acfg()->winroundsz*agdp())-2,1,1,0,0);
  }

  //-- Logo
  if(imgE){
      apng_draw(&dd_bg,&ap,imgX,imgY);
      apng_close(&ap);
  }

  //-- Virtual keys
  if (acfg()->virkeys_en) {
	  dd_virkeysW       = VIRTUAL_KEYS_WIDTH;
	  dd_virkeysH       = VIRTUAL_KEYS_HEIGHT;
	  dd_virkeys_home_X = 0;
	  dd_virkeys_home_Y = agh()-dd_virkeysH;
	  dd_virkeys_back_X = agw()/2;
	  dd_virkeys_back_Y = agh()-dd_virkeysH;
  }
  dd_isbgredraw = 0;
}

//*
//* Redraw Window Background
//*
void dd_redraw_win(){
  if (!dd_isbgredraw) return;
  ag_blank(&dd_win_bg);
  ag_rect(&dd_win_bg,0,0,agw(),agh(),0x0000);
  //-- Background
  if (!atheme_id_draw(26, &dd_win_bg, 0, 0, agw(),agh())){
    ag_roundgrad(&dd_win_bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  }

  dd_isbgredraw = 0;
}

//*
//* Redraw Whole Window Background
//*
void dd_redraw_bg(){
  if (!dd_isbgredraw) return;
  ag_blank(&dd_bg);

  ag_rect(&dd_bg,0,0,agw(),agh(),0x0000);
  //-- Background
  if (!atheme_id_draw(26, &dd_bg, 0, 0, agw(),agh())){
    ag_roundgrad(&dd_bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  }

  dd_isbgredraw = 0;
}

void dd_redraw_bg_ex(CANVAS* bg){
  ag_blank(bg);
  ag_rect(bg,0,0,agw(),agh(),0x0000);
  //-- Background
  if (!atheme_id_draw(26, bg, 0, 0, agw(),agh())){
    ag_roundgrad(bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  }
}

//*
//* Init Window Background With New Title
//* 
int dd_setbg(char * titlev){
  char title[64];
  snprintf(title,64,"%s",titlev);
  dd_redraw();
  int elmP  = agdp()*4;
  int titW  = ag_txtwidth(title,1);
  pthread_mutex_lock(&title_mutex);
  ag_draw(&dd_win_bg,&dd_bg,0,0);
  ag_textf(&dd_win_bg,titW,((agw()/2)-(titW/2))+1,elmP+1,title,acfg()->titlebg_g,2);
  ag_text(&dd_win_bg,titW,(agw()/2)-(titW/2),elmP,title,acfg()->titlefg,2);
  pthread_mutex_unlock(&title_mutex);
  return 2*elmP + ag_fontheight(1);
}

static int read_from_file(const char* path, char* buf, size_t size) {
    int fd = open(path, O_RDONLY, 0);
    if (fd == -1) {
        LOGE("Could not open '%s'", path);
        return -1;
    }

    size_t count = read(fd, buf, size);
    if (count > 0) {
        count = (count < size) ? count : size - 1;
        while (count > 0 && buf[count-1] == '\n') count--;
        buf[count] = '\0';
    } else {
        buf[0] = '\0';
    }

    close(fd);
    return count;
}

static int read_int(const char* path) {
    const int SIZE = 128;
    char buf[SIZE];
    int value = 0;

    if (read_from_file(path, buf, SIZE) > 0) {
        value = atoi(buf);
    }
    return value;
}


static int _dd_setbg_title(CANVAS *win, CANVAS *bg, int isotherbg) {
  pthread_mutex_lock(&title_mutex);
  static char bg_title[64];
  static time_t timep;
  static struct tm *p;
  PNGCANVAS ap;

  if(isotherbg)
	  dd_redraw_otherbg();
  else
	  dd_redraw();

  if(acfg()->rec_version != NULL)
	  snprintf(bg_title, 64, "<~recovery.name> v%s", acfg()->rec_version);
  else
	  snprintf(bg_title, 64, "<~recovery.name> v%s", DD_VERSION);

  int  elmP = agdp()*4;
  int  chkW = agw();
  byte imgE = 0;
  int  imgW = 0;
  int  imgH = 0;
  int  imgX = 0;
  int  imgY = 0;
  int  txtX = 0;
  int  txtY = 0;
  char *battery = BATTERY_NORMAL;

  //Calculate and draw logo & title location
  txtX = dd_title_x;
  txtY = dd_title_y;

  int  titW = ag_txtwidth(bg_title,1);

  //Calculate and draw battery location
  char batt_capacity[10];
  int capacity = 0;
  struct stat st;
  if (strlen(acfg()->battery_path) != 0) {
	  if (stat(acfg()->battery_path, &st) >= 0) {
	  	  capacity = read_int(acfg()->battery_path);
	  	  if(capacity < 0)
	  		  capacity = 0;
	  	  else if(capacity > 100)
	  		  capacity = 100;
	      snprintf(batt_capacity, 10, "%2d%%", capacity);
	  } else {
	      dd_error("Get battery capacity error.\n");
	  	  snprintf(batt_capacity, 10, "%2d%%", 0);
	  }

  } else {
	  if (stat(BATTERY_CAPACITY_PATH, &st) >= 0) {
		  capacity = read_int(BATTERY_CAPACITY_PATH);
		  if(capacity < 0)
			  capacity = 0;
		  else if(capacity > 100)
			  capacity = 100;
	      snprintf(batt_capacity, 10, "%2d%%", capacity);
	  } else if (stat(BATTERY_CAPACITY_PATH_1, &st) >= 0) {
		  capacity = read_int(BATTERY_CAPACITY_PATH_1);
		  if(capacity < 0)
			  capacity = 0;
		  else if(capacity > 100)
			  capacity = 100;
	      snprintf(batt_capacity, 10, "%2d%%", capacity);
	  } else {
	      dd_error("Get battery capacity error.\n");
		  snprintf(batt_capacity, 10, "%2d%%", 0);
	  }
  }

  int battW = ag_txtwidth(batt_capacity,1);
  int battX = chkW-battW-elmP;
  int battY = (dd_minY - ag_fontheight(1)) / 2;

  if (capacity < 6)
  	  battery = BATTERY_VERYLOW;
  else if (capacity < 20)
	  battery = BATTERY_LOW;
  else if (capacity > 98)
	  battery = BATTERY_FULL;
  else
	  battery = BATTERY_NORMAL;

  if (apng_load(&ap,battery)){
      imgE = 1;
      imgW = ap.w;
      imgH = ap.h;
      imgX = battX - imgW - elmP;
      imgY = (dd_minY - imgH) / 2;
  }

  if(imgE){
      apng_draw(&dd_bg,&ap,imgX,imgY);
      apng_close(&ap);
  }

  ag_draw(win, bg,0,0);
  //draw title name
  ag_text(win,titW,txtX,txtY,bg_title,acfg()->textbg,1);
  //draw battery
  if (capacity < 20) {
	  ag_text(win,battW,battX-9,battY,batt_capacity,acfg()->battlow,1);
  } else {
	  ag_text(win,battW,battX-9,battY,batt_capacity,acfg()->textbg,1);
  }
  pthread_mutex_unlock(&title_mutex);
  return dd_minY;
}

int dd_setbg_title() {
    return _dd_setbg_title(&dd_win_bg, &dd_bg, 0);
}
int dd_setbg_title_otherbg() {
    return _dd_setbg_title(&dd_win_bg, &dd_bg, 1);
}
int dd_setbg_title_win(AWINDOWP win){
    return _dd_setbg_title(&win->c, win->bg, 0);
}

int dd_set_title(char * titlev){
  char title[4096];
  snprintf(title,4096,"%s",titlev);
  PNGCANVAS ap;
  int pad   = agdp()*4;
  int barH  = dd_minY;
  int imgE  = 0;
  int imgW  = 0;
  int imgH  = 0;
  int imgX  = 0;
  int imgY  = 0;
  int titW  = agw()-pad*2-dd_minY;
  int titH  = ag_txtheight(agw()-((pad*2)+dd_minY),title,1);  //text高
  if (titH > barH)
	  barH = titH + pad*2;
  int titX  = pad;
  int titY  = (barH - titH) / 2 + dd_minY;
  int tmaxW = agw() - pad*2;
  if (apng_load(&ap, DIR_ICON)){
  	  imgE  = 1;
      imgW  = min(ap.w, barH);
      imgH  = min(ap.h, barH);
      imgX  = pad;
      imgY  = (barH - imgH)/2 + dd_minY;
      titX  = imgX + imgW + pad;
      tmaxW = agw() - titX;
  }

  ag_rect(&dd_win_bg, 0, dd_minY, agw(), barH, acfg()->winbarbg);

  if(imgE){
      apng_draw(&dd_win_bg,&ap,imgX,imgY);
      apng_close(&ap);
  }

  ag_text(&dd_win_bg,titW,titX,titY,title,acfg()->wintitlefg,1);
  return dd_minY + barH;
}

int dd_set_title_icon(CANVAS *bg, char *icon, char * titlev){
  char title[4096];
  snprintf(title,4096,"%s",titlev);
  PNGCANVAS ap;
  int pad   = agdp()*4;
  int barH  = dd_minY;
  int imgE  = 0;
  int imgW  = 0;
  int imgH  = 0;
  int imgX  = 0;
  int imgY  = 0;
  int titW  = agw()-pad*2-dd_minY;
  int titH  = ag_txtheight(agw()-((pad*2)+dd_minY),title,1);  //text高
  if (titH > barH)
	  barH = titH + pad*2;
  int titX  = pad;
  int titY  = (barH - titH) / 2 + dd_minY;
  int tmaxW = agw() - pad*2;
  if (apng_load(&ap, icon)){
  	  imgE  = 1;
      imgW  = min(ap.w, barH);
      imgH  = min(ap.h, barH);
      imgX  = pad;
      imgY  = (barH - imgH)/2 + dd_minY;
      titX  = imgX + imgW + pad;
      tmaxW = agw() - titX;
  }

  ag_rect(bg, 0, dd_minY, agw(), barH, acfg()->winbarbg);

  if(imgE){
      apng_draw(bg,&ap,imgX,imgY);
      apng_close(&ap);
  }

  ag_text(bg,titW,titX,titY,title,acfg()->wintitlefg,1);
  return dd_minY + barH;
}
//* 
//* Draw Navigation Bar
//*
void dd_drawnav(CANVAS * bg,int x, int y, int w, int h){
  if (!atheme_id_draw(2, bg, x, y, w, h)){
    ag_roundgrad_ex(
      bg,x,y,w,h,
      acfg()->navbg,
      acfg()->navbg_g,
      (acfg()->winroundsz*agdp())-2,0,0,1,1
    );
  }
}

//* 
//* Read Strings From filesystem
//* 
char * dd_readfromfs(char * name){
  char* buffer = NULL;
  struct stat st;
  if (stat(name,&st) < 0) return NULL;
  if (st.st_size>MAX_FILE_GETPROP_SIZE) return NULL;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;
  FILE* f = fopen(name, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  buffer[st.st_size] = '\0';
  fclose(f);
  return buffer;
done:
  free(buffer);
  return NULL;
}

//* 
//* Write Strings into file
//* 
void dd_writetofs(char * name, char * value){
  FILE * fp = fopen(name,"wb");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}

//* 
//* Read Strings From Temporary File
//*
char * dd_readfromtmp(char * name){
  char path[256];
  snprintf(path,256,"%s/%s",DD_TMP,name);
  return dd_readfromfs(path);
  
}

//* 
//* Write Strings From Temporary File
//*
void dd_writetotmp(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/%s",DD_TMP,name);
  dd_writetofs(path,value);
}

//* 
//* Read Strings From ZIP
//* 
char * dd_readfromzip(char * name){
  AZMEM filedata;
  if (!az_readmem(&filedata,name,0)) return NULL;
  return (char *)filedata.data;
}

//* 
//* Parse PROP String
//* 
static char * dd_parsepropstring(char * bf,char *key){
  char* result = NULL;  
  if (bf==NULL) return result;
  char* buffer=strdup(bf);
  char* line = strtok(buffer, "\n");
  do {
      while (*line && isspace(*line)) ++line;
      if (*line == '\0' || *line == '#') continue;
      char* equal = strchr(line, '=');
      if (equal == NULL) goto done;

      char* key_end = equal-1;
      while (key_end > line && isspace(*key_end)) --key_end;
      key_end[1] = '\0';

      if (strcmp(key, line) != 0) continue;

      char* val_start = equal+1;
      while(*val_start && isspace(*val_start)) ++val_start;

      char* val_end = val_start + strlen(val_start)-1;
      while (val_end > val_start && isspace(*val_end)) --val_end;
      val_end[1] = '\0';

      result = strdup(val_start);
      break;
  } while ((line = strtok(NULL, "\n")));
  free(buffer);
done:
  
  return result;
}

//* 
//* Parse PROP Files
//* 
char * dd_parseprop(char * filename,char *key){
  char * buffer = dd_readfromfs(filename);
  char * result = dd_parsepropstring(buffer,key);
  free(buffer);
  return result;
}

//* 
//* Parse PROP from ZIP
//* 
char * dd_parsepropzip(char * filename,char *key){
  char * buffer = dd_readfromzip(filename);
  char * result = dd_parsepropstring(buffer,key);
  free(buffer);
  return result;
}

//* 
//* Read Variable
//* 
char * dd_getvar(char * name){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",DD_TMP,name);
  return dd_readfromfs(path);
}

//* 
//* Set Variable
//* 
void dd_setvar(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",DD_TMP,name);
  dd_writetofs(path,value);
}

//* 
//* Append Variable
//* 
void dd_appendvar(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",DD_TMP,name);
  FILE * fp = fopen(path,"ab");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}

//* 
//* Delete Variable
//* 
void dd_delvar(char * name){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",DD_TMP,name);
  unlink(path);
}

//* 
//* Prepend Variable
//* 
void dd_prependvar(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",DD_TMP,name);
  char * buf = dd_getvar(name);
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    if (buf!=NULL){
      fwrite(buf,1,strlen(buf),fp);
    }
    fclose(fp);
  }
  if (buf!=NULL){
    free(buf);
  }
}

//* 
//* Set Colorset From Prop String
//* 
void dd_setthemecolor(char * prop, char * key, color * cl){
  char * val = dd_parsepropstring(prop,key);
  if (val!=NULL){
    cl[0] = strtocolor(val);
    free(val);
  }
}
//* 
//* Set Drawing Config From Prop String
//* 
void dd_setthemeconfig(char * prop, char * key, byte * b){
  char * val = dd_parsepropstring(prop,key);
  if (val!=NULL){
    b[0] = (byte) min(atoi(val),255);
    free(val);
  }
}

/************************************[ DD EDIFY HANDLERS ]************************************/
//* 
//* loadtruefont
//*
STATUS dd_font(char *ttf_type, char *ttf_file, char *ttf_size){
  
  //-- This is Busy Function
  return_val_if_fail(ttf_type != NULL, RET_FAIL);
  return_val_if_fail(ttf_file != NULL, RET_FAIL);
  return_val_if_fail(ttf_size != NULL, RET_FAIL);
  ag_setbusy();
  
  //-- Get Arguments
  
  char zpath[256];
  snprintf(zpath,256,"%s/",DD_DIR);
  
  int size = atoi(ttf_size);
  if (ttf_type[0]=='0'){
    if (!ag_loadsmallfont(ttf_file, size, zpath))
    	printf("Load %s(%d) failed!\n", ttf_file, size);
  }
  else if (ttf_type[0]=='2'){
    if (!ag_loadbigfont(ttf_file, size, zpath))
    	printf("Load %s(%d) failed!\n", ttf_file, size);
  } else {
	if (!ag_loadmediumfont(ttf_file, size, zpath))
	    printf("Load %s(%d) failed!\n", ttf_file, size);
  }

  //-- Return
  return RET_OK;
  
}

//* 
//* set_theme
//* 
STATUS  dd_theme(char *theme) {
  return_val_if_fail(theme != NULL, RET_FAIL);
  
  //-- This is Busy Function
  ag_setbusy();
  
  if ((strcmp(theme,"")==0)||(strcmp(theme,"generic")==0)){
    //-- Background Should Be Redrawed
    dd_isbgredraw = 1;
    
    
    //-- Return
    return RET_OK;
  }

  //-- Parse The Prop
  char themename[256];
  snprintf(themename,256,"%s/themes/%s/theme.prop",DD_DIR,theme);
  snprintf(acfg()->themename,64,"%s",theme);
  char * propstr = dd_readfromzip(themename);
  if (propstr){
    int i=0;
    for (i=0;i<DD_THEME_CNT;i++){
      char * key = atheme_key(i);
      char * val = dd_parsepropstring(propstr,key);
      if (val!=NULL){
        if (strcmp(val,"")!=0){
          snprintf(themename,256,"themes/%s/%s",theme,val);
          atheme_create(key,themename);
        }
        free(val);
      }
    }

    dd_setthemecolor(propstr,  "color.winbg",              &acfg()->winbg);
    dd_setthemecolor(propstr,  "color.winbg_g",            &acfg()->winbg_g);
    dd_setthemecolor(propstr,  "color.winfg",              &acfg()->winfg);
    dd_setthemecolor(propstr,  "color.winfg_gray",         &acfg()->winfg_gray);
    dd_setthemecolor(propstr,  "color.dialogbg",           &acfg()->dialogbg);
    dd_setthemecolor(propstr,  "color.dialogbg_g",         &acfg()->dialogbg_g);
    dd_setthemecolor(propstr,  "color.dialogfg",           &acfg()->dialogfg);
    dd_setthemecolor(propstr,  "color.textbg",             &acfg()->textbg);
    dd_setthemecolor(propstr,  "color.textfg",             &acfg()->textfg);
    dd_setthemecolor(propstr,  "color.textfg_gray",        &acfg()->textfg_gray);
    dd_setthemecolor(propstr,  "color.controlbg",          &acfg()->controlbg);
    dd_setthemecolor(propstr,  "color.controlbg_g",        &acfg()->controlbg_g);
    dd_setthemecolor(propstr,  "color.controlfg",          &acfg()->controlfg);
    dd_setthemecolor(propstr,  "color.selectbg",           &acfg()->selectbg);
    dd_setthemecolor(propstr,  "color.selectbg_g",         &acfg()->selectbg_g);
    dd_setthemecolor(propstr,  "color.selectfg",           &acfg()->selectfg);
    dd_setthemecolor(propstr,  "color.titlebg",            &acfg()->titlebg);
    dd_setthemecolor(propstr,  "color.titlebg_g",          &acfg()->titlebg_g);
    dd_setthemecolor(propstr,  "color.titlefg",            &acfg()->titlefg);
    dd_setthemecolor(propstr,  "color.dlgtitlebg",         &acfg()->dlgtitlebg);
    dd_setthemecolor(propstr,  "color.dlgtitlebg_g",       &acfg()->dlgtitlebg_g);
    dd_setthemecolor(propstr,  "color.dlgtitlefg",         &acfg()->dlgtitlefg);
    dd_setthemecolor(propstr,  "color.scrollbar",          &acfg()->scrollbar);
    dd_setthemecolor(propstr,  "color.navbg",              &acfg()->navbg);
    dd_setthemecolor(propstr,  "color.navbg_g",            &acfg()->navbg_g);
    dd_setthemecolor(propstr,  "color.border",             &acfg()->border);
    dd_setthemecolor(propstr,  "color.border_g",           &acfg()->border_g);
    dd_setthemecolor(propstr,  "color.progressglow",       &acfg()->progressglow);
    dd_setthemecolor(propstr,  "color.battlow",            &acfg()->battlow);
    dd_setthemecolor(propstr,  "color.wintitlebg",         &acfg()->wintitlebg);
    dd_setthemecolor(propstr,  "color.wintitlefg",         &acfg()->wintitlefg);
    dd_setthemecolor(propstr,  "color.winbarbg",           &acfg()->winbarbg);

    dd_setthemeconfig(propstr, "config.roundsize",         &acfg()->roundsz);
    dd_setthemeconfig(propstr, "config.button_roundsize",  &acfg()->btnroundsz);
    dd_setthemeconfig(propstr, "config.window_roundsize",  &acfg()->winroundsz);
    dd_setthemeconfig(propstr, "config.transition_frame",  &acfg()->fadeframes);

    free(propstr);
  }
  else{
    memset(acfg()->themename, 0x00, 64);
  }

  //-- Background Should Be Redrawed
  dd_isbgredraw = 1;

  //-- Return
  return RET_OK;
}

char * dd_getprop(char *file, char *key) {
  return_null_if_fail(file != NULL); 
  return_null_if_fail(key != NULL); 
  //-- This is Busy Function
  ag_setbusy();

  //-- Parse The Prop
  char* result;
  char path[256];
  snprintf(path,256,"%s/%s",DD_DIR,file);
  result = dd_parseprop(path,key);
  return result;
}
char * dd_gettmpprop(char *file, char *key) {
  return_null_if_fail(file != NULL); 
  return_null_if_fail(key != NULL); 
  //-- This is Busy Function
  ag_setbusy();

  //-- Parse The Prop
  char* result;
  char path[256];
  snprintf(path,256,"%s/%s",DD_TMP,file);
  result = dd_parseprop(path,key);
  return result;
}

//* 
//* resread, readfile_dd
//* 
char * dd_resread(char *file) {
  
  return_null_if_fail(file != NULL);
  //-- This is Busy Function
  ag_setbusy();
  
    
  //-- Create Path Into Resource Dir
  char path[256];
  snprintf(path,256,"%s/%s",DD_DIR,file);
  
  //-- Read From Zip
  char * buf = dd_readfromzip(path);
  
  
  //-- Return
  return buf;
}


//* 
//* pleasewait
//* 
STATUS dd_pleasewait(char *message) {
  
  
  return_val_if_fail(message != NULL, RET_FAIL);
  
  //-- Set Busy Text
  char txt[32];
  snprintf(txt, 32, "%s", message);
  ag_setbusy_withtext(txt);
  
  //-- Return
  return RET_OK;
}

STATUS dd_busy_process(int isauto)
{
  //-- Set Busy before everythings ready
  ag_setbusy();
  dd_isbgredraw = 1;
  dd_redraw_win();
  PNGCANVAS ap;
  int  chkH  = agh();
  int  chkW  = agw();
  int  chkY  = 0;
  byte imgE  = 0;
  int  imgA  = 0;
  int  imgW  = 0;
  int  imgH  = 0;
  int  imgX  = 0;
  int  imgY  = 0;

  char *waitpic = PIC_WAIT;
  char *text ;
  if(!isauto)
	  text  = "<~default.wait>";
  else
	  text  = "<~default.wipe.wait>";

  chkH -= chkY;
  int big = 2;
  int txtW = ag_txtwidth(text, big);
  int txtH = ag_fontheight(big);

  if (apng_load(&ap,waitpic)){
    imgE = 1;
    imgW = ap.w;
    imgH = ap.h;
    imgX = (chkW - imgW) / 2;
    imgY = (chkH - imgH) / 3;
  }

  if(imgE){
      apng_draw(&dd_win_bg,&ap,imgX,imgY);
      apng_close(&ap);
  }
  int txtX = (agw()/2) - (txtW/2);
  int txtY = imgY + imgH + agdp() * 4;

  ag_textf(&dd_win_bg, txtW, txtX, txtY, text, acfg()->titlefg, big);

  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);

  //-- Dispatch Message
  aw_show(hWin);

  //-- Destroy Window
  aw_destroy(hWin);
  ag_sync();
  return RET_OK;
}



//* 
//* setcolor
//*
STATUS dd_setcolor(char *item, char *val) {
  return_val_if_fail(item != NULL, RET_FAIL);
  return_val_if_fail(val != NULL, RET_FAIL); 
  //-- This is Busy Function
  ag_setbusy();
  
  
  //-- Convert String into Color
  color cl = strtocolor(val);
  
  //-- Set Color Property
  if      (strcmp(item,"winbg") == 0)          acfg()->winbg=cl;
  else if (strcmp(item,"winbg_g") == 0)        acfg()->winbg_g=cl;
  else if (strcmp(item,"winfg") == 0)          acfg()->winfg=cl;
  else if (strcmp(item,"winfg_gray") == 0)     acfg()->winfg_gray=cl;
  else if (strcmp(item,"dialogbg") == 0)       acfg()->dialogbg=cl;
  else if (strcmp(item,"dialogbg_g") == 0)     acfg()->dialogbg_g=cl;
  else if (strcmp(item,"dialogfg") == 0)       acfg()->dialogfg=cl;
  else if (strcmp(item,"textbg") == 0)         acfg()->textbg=cl;
  else if (strcmp(item,"textfg") == 0)         acfg()->textfg=cl;
  else if (strcmp(item,"textfg_gray") == 0)    acfg()->textfg_gray=cl;
  else if (strcmp(item,"controlbg") == 0)      acfg()->controlbg=cl;
  else if (strcmp(item,"controlbg_g") == 0)    acfg()->controlbg_g=cl;
  else if (strcmp(item,"controlfg") == 0)      acfg()->controlfg=cl;
  else if (strcmp(item,"selectbg") == 0)       acfg()->selectbg=cl;
  else if (strcmp(item,"selectbg_g") == 0)     acfg()->selectbg_g=cl;
  else if (strcmp(item,"selectfg") == 0)       acfg()->selectfg=cl;
  else if (strcmp(item,"titlebg") == 0)        acfg()->titlebg=cl;
  else if (strcmp(item,"titlebg_g") == 0)      acfg()->titlebg_g=cl;
  else if (strcmp(item,"titlefg") == 0)        acfg()->titlefg=cl;
  else if (strcmp(item,"dlgtitlebg") == 0)     acfg()->dlgtitlebg=cl;
  else if (strcmp(item,"dlgtitlebg_g") == 0)   acfg()->dlgtitlebg_g=cl;
  else if (strcmp(item,"dlgtitlefg") == 0)     acfg()->dlgtitlefg=cl;
  else if (strcmp(item,"scrollbar") == 0)      acfg()->scrollbar=cl;
  else if (strcmp(item,"navbg") == 0)          acfg()->navbg=cl;
  else if (strcmp(item,"navbg_g") == 0)        acfg()->navbg_g=cl;
  else if (strcmp(item,"border") == 0)         acfg()->border=cl;
  else if (strcmp(item,"border_g") == 0)       acfg()->border_g=cl;
  else if (strcmp(item,"progressglow") == 0)   acfg()->progressglow=cl;
  else if (strcmp(item,"battlow") == 0)        acfg()->battlow=cl;
  else if (strcmp(item,"wintitlebg") == 0)     acfg()->wintitlebg=cl;
  else if (strcmp(item,"wintitlefg") == 0)     acfg()->wintitlefg=cl;
  else if (strcmp(item,"winbarbg") == 0)       acfg()->winbarbg=cl;
  
  //-- Background Should Be Redrawed
  dd_isbgredraw = 1;
  

  //-- Return
  return RET_OK;
}


//* 
//* ini_get
//*
char * dd_ini_get(char *item) {
  
  return_null_if_fail(item != NULL);
  //-- This is Busy Function
  ag_setbusy();
  
  
  //-- Convert Arguments
  char retval[128];
  memset(retval,0,128);
  
  //-- Set Property
  if      (strcmp(item,"roundsize") == 0)          snprintf(retval,128,"%i",acfg()->roundsz);
  else if (strcmp(item,"button_roundsize") == 0)   snprintf(retval,128,"%i",acfg()->btnroundsz);
  else if (strcmp(item,"window_roundsize") == 0)   snprintf(retval,128,"%i",acfg()->winroundsz);
  else if (strcmp(item,"transition_frame") == 0)   snprintf(retval,128,"%i",acfg()->fadeframes);

  else if (strcmp(item,"text_ok") == 0)            snprintf(retval,128,"%s",acfg()->text_ok);
  else if (strcmp(item,"text_next") == 0)          snprintf(retval,128,"%s",acfg()->text_next);
  else if (strcmp(item,"text_back") == 0)          snprintf(retval,128,"%s",acfg()->text_back);

  else if (strcmp(item,"text_yes") == 0)           snprintf(retval,128,"%s",acfg()->text_yes);
  else if (strcmp(item,"text_no") == 0)            snprintf(retval,128,"%s",acfg()->text_no);
  else if (strcmp(item,"text_about") == 0)         snprintf(retval,128,"%s",acfg()->text_about);
  else if (strcmp(item,"text_calibrating") == 0)   snprintf(retval,128,"%s",acfg()->text_calibrating);
  else if (strcmp(item,"text_quit") == 0)          snprintf(retval,128,"%s",acfg()->text_quit);
  else if (strcmp(item,"text_quit_msg") == 0)      snprintf(retval,128,"%s",acfg()->text_quit_msg);
    
  else if (strcmp(item,"rec_name") == 0)           snprintf(retval,128,"%s",acfg()->rec_name);
  else if (strcmp(item,"rec_version") == 0)        snprintf(retval,128,"%s",acfg()->rec_version);
  else if (strcmp(item,"rec_iteration") == 0)      snprintf(retval,128,"%s",acfg()->rec_iteration);
  else if (strcmp(item,"rec_author") == 0)         snprintf(retval,128,"%s",acfg()->rec_author);
  else if (strcmp(item,"rec_device") == 0)         snprintf(retval,128,"%s",acfg()->rec_device);
  else if (strcmp(item,"rec_date") == 0)           snprintf(retval,128,"%s",acfg()->rec_date);
  
  else if (strcmp(item,"customkeycode_up")==0)     snprintf(retval,128,"%i",acfg()->ckey_up);
  else if (strcmp(item,"customkeycode_down")==0)   snprintf(retval,128,"%i",acfg()->ckey_down);
  else if (strcmp(item,"customkeycode_select")==0) snprintf(retval,128,"%i",acfg()->ckey_select);
  else if (strcmp(item,"customkeycode_back") == 0) snprintf(retval,128,"%i",acfg()->ckey_back);
  else if (strcmp(item,"customkeycode_menu") == 0) snprintf(retval,128,"%i",acfg()->ckey_menu);
  else if (strcmp(item,"customkeycode_home") == 0) snprintf(retval,128,"%i",acfg()->ckey_home);
  else if (strcmp(item,"dp") == 0) snprintf(retval,128,"%i",agdp());
  
  //-- Return
  return strdup(retval);
}

//* 
//* viewbox
//*
STATUS dd_viewbox(int argc, char *format, ...) {

  //-- is plain textbox or agreement
  if (argc!=3) return dd_error("%s() expects 3 args (title,desc,ico), got %d", __FUNCTION__, argc);
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Init Background
  dd_setbg(args[0]);
  char text[1024];
  snprintf(text,1024,"%s",args[1]);
  
  //-- Init Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( dd_minY + bntH + (pad*4));
  int chkY        = dd_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  dd_drawnav(&dd_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,1);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&dd_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&dd_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,1);
  ag_text(&dd_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,1);
  
  color sepcl = ag_calculatealpha(acfg()->winbg,0x0000,80);
  color sepcb = ag_calculatealpha(acfg()->winbg,0xffff,127);
  ag_rect(&dd_win_bg,tifX,tifY+pad+txtH,chkW-((pad*2)+imgA),1,sepcl);
  ag_rect(&dd_win_bg,tifX,tifY+pad+txtH+1,chkW-((pad*2)+imgA),1,sepcb);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);
  ACONTROLP txtcb = NULL;
  
  
  //-- NEXT BUTTON
  ACONTROLP nxtbtn=acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,0,
    6
  );

  char save_var_name[256];
  if (argc==6){
    //-- Save Variable Name
    snprintf(save_var_name,256,"%s",args[5]);
  }
            
  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  //aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  byte is_checked     = 0;
      
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        ondispatch = 0;
      }
      break;
      case 5:{
          ondispatch      = 0;
      }
      break;
      case 4:{
        //-- EXIT
        ondispatch      = 0;
      }
      break;
    }
  }
  //-- Window
  aw_destroy(hWin);
  
  //-- Return
  
  if (is_checked) return RET_YES;
  return RET_NO;
}

//* 
//* textbox, agreebox
//*
STATUS dd_textbox(char *title, char *desc, char* icon, char *message) {
  
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  dd_isbgredraw = 1;
  
  //-- Init Background
  dd_setbg_title();
  char text[256];
  snprintf(text,256,"%s",title);
  
  //-- Unchecked Alert Message
    
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw();
  int bntH        = agdp() * 20;
  int chkY  	  = dd_set_title_icon(&dd_win_bg, icon, text);
  int chkH 		  = agh() - chkY - dd_virkeysH;
  
  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);
  
  //-- Create Controls
  ACONTROLP txtbox;
  ACONTROLP agreecb;  
  txtbox = actext(hWin,0,chkY,chkW,chkH,message,1);

  draw_virtualkeys(hWin);
  
  //-- Dispatch Message
  aw_show(hWin);
//  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  byte onback = 0;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
    case 8:{
      //-- BACK
         onback = 1;
         ondispatch = 0;
    }
    break;
    case 7:{
      //-- BACK TO MAIN
         onback = 2;
         ondispatch = 0;
    }
    break;
      case 6:{
            ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
      
          ondispatch      = 0;
      }
      break;
      case 4:{
        //-- EXIT
        ondispatch      = 0;
      }
      break;
    }
  }
  
  int selindex;
  if (onback == 0)
      selindex = MENU_BACK;
  else if (onback == 2)
	  selindex = MENU_MAIN;
  else selindex = 0;
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  //return RET_OK;
  return selindex;
}

//* 
//* checkbox
//*
STATUS dd_checkbox(int argc, char *format, ...) {
  if (argc<7){
    return dd_error("%s() expects more than 7 args, got %d", __FUNCTION__, argc);
  }
  else if ((argc-4)%3!=0){
    return dd_error("%s() expects 4 args + 3 args per items, got %d", __FUNCTION__, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Variable Def
  int i;
  
  //-- Init Background
  dd_setbg(args[0]);
  
  //-- Init Strings
  char path[256];
  char text[256];
  snprintf(path,256,"%s/%s",DD_TMP,args[3]);
  snprintf(text,256,"%s",args[1]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( dd_minY + bntH + (pad*4));
  int chkY        = dd_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  dd_drawnav(&dd_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,1);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&dd_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&dd_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,1);
  ag_text(&dd_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,1);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);
  
  //-- Check Box
  ACONTROLP chk1  = accheck(hWin,pad,chkY,chkW,chkH);
  
  //-- NEXT BUTTON
  ACONTROLP nxtbtn=acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,1,
    6
  );
  
  //-- Populate Checkbox Items
  char propkey[64];
  int idx = 0;
  int group_id = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==2){
      if (accheck_addgroup(chk1,args[i],args[i+1])){
        group_id++;
        idx=0;
      }
    }
    else if (defchk!=3){
      idx++;
      snprintf(propkey,64,"item.%d.%d",group_id,idx);
      char * res = dd_parseprop(path,propkey);
      if (res!=NULL){
        defchk = (strcmp(res,"1")==0)?1:0;
        free(res);
      }
      accheck_add(chk1,args[i],args[i+1],defchk);
    }
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  //aw_setfocus(hWin,nxtbtn);
  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6: ondispatch = 0; break;
      case 5:{
        //-- BACK
          ondispatch      = 0;
      }
      break;
      case 4:{
        //-- EXIT
        ondispatch      = 0;
      }
      break;
    }
  }
  //-- Collecting Items:
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    int itemcnt = accheck_itemcount(chk1);
    for (i=0;i<itemcnt;i++) {
      if (!accheck_isgroup(chk1,i)){
        byte state = accheck_ischecked(chk1,i);
        snprintf(propkey,64,"item.%d.%d=%d\n",accheck_getgroup(chk1,i),accheck_getgroupid(chk1,i)+1,state);
        fwrite(propkey,1,strlen(propkey),fp);
      }
    }
    fclose(fp);
  }
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  return RET_OK;
}

//* 
//* checkbox
//*
STATUS dd_checkbox_install(int argc, char *format, ...) {
  if (argc<7){
    return dd_error("%s() expects more than 7 args, got %d", __FUNCTION__, argc);
  }
  else if ((argc-4)%3!=0){
    return dd_error("%s() expects 4 args + 3 args per items, got %d", __FUNCTION__, argc);
  }

  //-- Set Busy before everythings ready
  ag_setbusy();

  //-- Get Arguments
  _INITARGS();

  //-- Variable Def
  int i;
  int rtn_val;

  //-- Init Background
  dd_setbg_title();

  //-- Init Strings
  char path[256];
  char text[256];
  snprintf(path,256,"%s/%s",DD_TMP,args[3]);
  snprintf(text,256,"%s",args[1]);

  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw();
  int bntH        = agdp() * 24;
  int chkY  	  = dd_set_title_icon(&dd_win_bg, args[2], text);
  int chkH        = agh() - (chkY + bntH + (pad*4) + dd_virkeysH);
  int btnY        = chkY + chkH + (pad*2);
  ag_rect(&dd_win_bg, 0, chkY + chkH, agw(), bntH + (pad*4), acfg()->titlefg);

  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);

  //-- Check Box
  ACONTROLP chk1  = accheck(hWin,0,chkY,chkW,chkH);

  //Show Next Button
  ACONTROLP nxtbtn=acbutton(
    hWin,
    pad+(chkW/2),btnY,(chkW/2)-(pad*2),bntH,"<~install.prepare.confirm.info>",1,
    6
  );
  // Show Dump Button
  ACONTROLP cnlbtn=acbutton(
    hWin,
    pad,btnY,(chkW/2)-(pad*2),bntH,"<~install.prepare.cancel.info>",1,
    8
  );

  draw_virtualkeys(hWin);

  //-- Populate Checkbox Items
  char propkey[64];
  int idx = 0;
  int group_id = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==2){
      if (accheck_addgroup(chk1,args[i],args[i+1])){
        group_id++;
        idx=0;
      }
    }
    else if (defchk!=3){
      idx++;
      snprintf(propkey,64,"item.%d.%d",group_id,idx);
      char * res = dd_parseprop(path,propkey);
      if (res!=NULL){
        defchk = (strcmp(res,"1")==0)?1:0;
        free(res);
      }
      accheck_add(chk1,args[i],args[i+1],defchk);
    }
  }

  //-- Release Arguments
  _FREEARGS();

  //-- Dispatch Message
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 8:
      {
    	  ondispatch = 0;
    	  rtn_val = RET_NO;
    	  break;
      }
      case 6:
      {
    	  ondispatch = 0;
    	  rtn_val = RET_YES;
    	  break;
      }
    }
  }
  //-- Collecting Items:
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    int itemcnt = accheck_itemcount(chk1);
    for (i=0;i<itemcnt;i++) {
      if (!accheck_isgroup(chk1,i)){
        byte state = accheck_ischecked(chk1,i);
        snprintf(propkey,64,"item.%d.%d=%d\n",accheck_getgroup(chk1,i),accheck_getgroupid(chk1,i)+1,state);
        fwrite(propkey,1,strlen(propkey),fp);
      }
    }
    fclose(fp);
  }

  //-- Destroy Window
  aw_destroy(hWin);

  //-- Finish
  return rtn_val;
}


//*
//* selectbox
//*
STATUS dd_selectbox(int argc, char *format, ...) {
  if (argc<7) {
    return dd_error("%s() expects more than 7 args, got %d", __FUNCTION__, argc);
  }
  else if ((argc-4)%3!=0){
    return dd_error("%s() expects 4 args + 3 args per items, got %d", __FUNCTION__, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Variable Def
  int i;
  
  //-- Init Background
  dd_setbg_title();
  
  //-- Init Strings
  char path[256];
  char text[256];
  snprintf(path,256,"%s/%s",DD_TMP,args[3]);
  snprintf(text,256,"%s",args[1]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( dd_minY + bntH + (pad*4));
  int chkY        = dd_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  dd_drawnav(&dd_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,1);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&dd_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }

  //-- Draw Text
  ag_textf(&dd_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,1);
  ag_text(&dd_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,1);

  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;

  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);

  //-- Check Box
  ACONTROLP opt1  = acopt(hWin,pad,chkY,chkW,chkH);

  //-- NEXT BUTTON
  ACONTROLP nxtbtn = acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,1,
    6
  );

  char propkey[64];

  //-- Populate Checkbox Items
  int group_id = 0;
  int idx      = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==2){
      if (acopt_addgroup(opt1,args[i],args[i+1])){
        group_id++;
        idx      = 0;
      }
    }
    else if (defchk!=3){
      idx++;
      snprintf(propkey,64,"selected.%d",group_id);
      char * savedsel = dd_parseprop(path,propkey);

      snprintf(propkey,64,"%d",idx);
      if (savedsel!=NULL){
        defchk = (strcmp(savedsel,propkey)==0)?1:0;
        free(savedsel);
      }
      acopt_add(opt1,args[i],args[i+1],defchk);
    }
  }

  //-- Release Arguments
  _FREEARGS();

  //-- Dispatch Message
  aw_show(hWin);
  //aw_setfocus(hWin,nxtbtn);
  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
       
          ondispatch      = 0;
      }
      break;
      case 4:{
        //-- EXIT
        ondispatch      = 0;
      }
      break;
    }
  }
  
  //-- Collecting Items:
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    for (i=0;i<=group_id;i++){
      int selidx   = acopt_getselectedindex(opt1,i);
      if (selidx!=-1){
        int selindex = acopt_getgroupid(opt1,selidx)+1;
        snprintf(propkey,64,"selected.%d=%d\n",i,selindex);
        fwrite(propkey,1,strlen(propkey),fp);
      }
    }
    fclose(fp);
  }
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  return RET_OK;
}

STATUS dd_loadwelcome() {
    ag_setbusy();
    dd_isbgredraw=1;
    dd_redraw_bg();
    PNGCANVAS ap;
    int chkH = agh();
    int chkW = agw();
    byte imgE = 0;
    int  imgW = 0;
    int  imgH = 0;
    int  imgX = 0;
    int  imgY = 0;
    char *welcome = PIC_WELCOME;

    if (apng_load(&ap,welcome)){
        imgE = 1;
        imgW = ap.w;
        imgH = ap.h;
        imgX = (chkW - imgW) / 2;
        imgY = (chkH - imgH)*2 / 5;
    }

    if(imgE){
        apng_draw(&dd_bg,&ap,imgX,imgY);
        apng_close(&ap);
    }

    AWINDOWP hWin   = aw(&dd_bg);
	aw_show(hWin);
    usleep(1500000); //2s
    aw_destroy(hWin);
    ag_sync();

    return RET_OK;
}


//*
//* menubox
//*
STATUS dd_mainmenu(char *title_name, char **item, char **item_icon, char **item_icon_append, int item_cnt, int icon_enable) {
  //-- Set Busy before everythings ready
  return_val_if_fail(title_name != NULL, RET_FAIL);
  return_val_if_fail(item_cnt >= 0, RET_FAIL);
  ag_setbusy();
  dd_isbgredraw=1;
  //-- Get Arguments

  //-- Variable Def
  int i;
  
  //-- Init Background
  
  //-- Init Strings

  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkH        = agh();
  int chkW        = agw();
  
  //-- Draw Navigation Bar
  int chkY= dd_setbg_title();
  chkH -= chkY;
  
  int banH = BANNER_HEIGTH_SUB;
  if (dd_draw_subbanner(0, chkY, agw(), banH))
	  chkY += banH;

  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);

  //-- Check Box
  ACONTROLP title = actitle_text(hWin, 0, chkY, chkW, &chkH, title_name, 2);
  chkY += chkH;
  chkH = agh() - chkY - dd_virkeysH;

  ACONTROLP menu1  = acmenu(hWin,0,chkY,chkW,chkH,6);

  //-- Populate Checkbox Items
  for (i=0;i<item_cnt;i++) {
    if (item[i] != NULL && strcmp(item[i],"")!=0){
      if (item_icon != NULL && item_icon_append != NULL)
           acmenu_add(menu1, item[i], item_icon[i], item_icon_append[i]);
      else {
          if (item_icon != NULL)
              acmenu_add(menu1, item[i], item_icon[i], NULL);
          else {
              if (item_icon_append != NULL)
                  acmenu_add(menu1, item[i], ITEM_ICON, item_icon_append[i]);
              else 
                  acmenu_add(menu1, item[i], ITEM_ICON, NULL);
          }
      }
    }
  }
  draw_virtualkeys(hWin);
  
  //-- Dispatch Message
  aw_show(hWin);
  byte ondispatch = 1;
  byte onback = 0;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
    case 8:{
      //-- BACK
         onback = 1;
         ondispatch = 0;
    }
    break;
    case 7:{
      //-- BACK TO MAIN
         onback = 2;
         ondispatch = 0;
    }
    break;
      case 6:{
          ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
          onback = 1;
          ondispatch  = 0;
      }
      break;
      case 4:{
        //-- EXIT
        onback = 1;
        ondispatch = 0;
      }
      break;
    }
  }
  
  int selindex;
  if (onback == 0)
      selindex = acmenu_getselectedindex(menu1)+1;
  else if (onback == 2)
	  selindex = MENU_MAIN;
  else selindex = 0;
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  return selindex;
}

//*
//* menubox
//*
STATUS dd_mainmenu_block(
		char *title_name,
		char **item,
		char **item_icon,
		char **item_icon_append,
		int item_cnt,
		int icon_enable,
		char **desc) {
  //-- Set Busy before everythings ready
  return_val_if_fail(title_name != NULL, RET_FAIL);
  return_val_if_fail(item_cnt >= 0, RET_FAIL);
  ag_setbusy();
  dd_isbgredraw=1;
  //-- Get Arguments

  //-- Variable Def
  int i;

  //-- Init Background

  //-- Init Strings

  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkH        = agh();
  int chkW        = agw();
  int bntH        = agdp() * 20;
  int banH		  = 0;

  //-- Draw Navigation Bar
  int chkY= dd_setbg_title();
  chkH -= chkY;
  banH = BANNER_HEIGTH_HOME;

  if (dd_draw_homebanner(0, chkY, agw(), banH))
	  chkY += banH;

  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);

  //-- Check Box
  chkH = agh() - chkY - dd_virkeysH;
  ACONTROLP menu1  = acblockmenu(hWin,0,chkY,chkW,chkH, 6, 2);
  //-- Populate Checkbox Items
  for (i=0;i<item_cnt;i++) {
    if (item[i] != NULL && strcmp(item[i],"")!=0){
      if (item_icon != NULL)
    	  acblockmenu_add(menu1, item[i], desc[i], item_icon[i]);
      else
    	  acblockmenu_add(menu1, item[i], desc[i], NULL);
    }
  }

  draw_virtualkeys(hWin);

  //-- Dispatch Message
  aw_show(hWin);
  byte ondispatch = 1;
  byte onback = 0;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
		case 8:{
		  //-- BACK
			 onback = 1;
			 ondispatch = 0;
		}
		break;
		case 7:{
		  //-- BACK TO MAIN
			 onback = 2;
			 ondispatch = 0;
		}
		break;
		case 6:{
			ondispatch = 0;
		}
		break;
		case 5:{
		  //-- BACK
			onback = 1;
			ondispatch  = 0;
		}
		break;
		case 4:{
		  //-- EXIT
		  onback = 1;
		  ondispatch = 0;
		}
		break;
    }
  }

  int selindex;
  if (onback == 0)
      selindex = acblockmenu_getselectedindex(menu1)+1;
  else if (onback == 2)
	  selindex = MENU_MAIN;
  else selindex = 0;

  //-- Destroy Window
  aw_destroy(hWin);

  //-- Finish
  return selindex;
}

STATUS dd_menubox(char *title_name, char **item, int item_cnt) {
  return_val_if_fail(item_cnt >= 1, RET_FAIL); 
  return_val_if_fail(title_name != NULL, RET_FAIL);
  return_val_if_fail(item != NULL, RET_FAIL);
  //-- Set Busy before everythings ready
  ag_setbusy();
  dd_isbgredraw = 1;
  
  
  //-- Variable Def
  int i;
  
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkH        = agh();
  int chkY        = dd_setbg_title();
  int chkW        = agw();
  
  //-- Draw Navigation Bar
  chkH -= chkY;

  int banH = BANNER_HEIGTH_SUB;
  if (dd_draw_subbanner(0, chkY, agw(), banH))
	  chkY += banH;

  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);
  ACONTROLP title = actitle_text(hWin, 0, chkY, chkW, &chkH, title_name, 2);
  chkY += chkH;
  chkH = agh() - chkY - dd_virkeysH;
  //-- Check Box
  ACONTROLP menu1  = acmenu(hWin,0,chkY,chkW,chkH,6);

  //-- Populate Checkbox Items
  for (i = 0; i < item_cnt; i++) {
    if (item[i] != NULL && strcmp(item[i],"")!=0)
      acmenu_add(menu1,item[i], ITEM_ICON, NULL);
  }
  int bntY = chkY + chkH + pad/2;
  int bntW = (agw()/2 - pad*4);

  draw_virtualkeys(hWin);

  //-- Dispatch Message
  aw_show(hWin);
  byte ondispatch = 1;
  byte onback = 0;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    //printf("aw_gm(msg): %d\n", aw_gm(msg));
    switch (aw_gm(msg)){
      case 8:{
        //-- BACK
           onback = 1;
           ondispatch = 0;
      }
      break;
      case 7:{
        //-- BACK TO MAIN
           onback = 2;
           ondispatch = 0;
      }
      break;
      case 6:{
           ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
          onback = 1; 
          ondispatch = 0;
      }
      break;
      case 4:{
        //-- EXIT
        onback = 1;
        ondispatch = 0;
      }
      break;
    }
  }
  
  int selindex;
  if (onback == 0)
      selindex = acmenu_getselectedindex(menu1)+1;
  else if (onback == 2)
	  selindex = MENU_MAIN;
  else
	  selindex = 0;
  
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  return selindex;
}

/*
 *print sd file system,
 *menu titlname, use set_bg print title 
 *menu name, 
 *
 *
 */
/*
 *
 *dd_get_title "DD recovery  batt:n% time:21:30"
 *
 */
STATUS dd_sdmenu(char *title_name, char **item, char **item_sub, int item_cnt) {

  //-- Set Busy before everythings ready
  ag_setbusy();
  dd_isbgredraw = 1;

  //-- Get Arguments

  //-- Variable Def
  int i;
  dd_setbg_title(); 
  //-- Init Background

  //-- Init Strings

  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkY        = dd_set_title(title_name);
  int chkH        = agh() - chkY - dd_virkeysH;
  int chkW        = agw();

  //-- Draw Navigation Bar


  //-- Create Window
  AWINDOWP hWin   = aw(&dd_win_bg);
  draw_virtualkeys(hWin);
  //-- Check Box
  ACONTROLP menu1  = acsdmenu(hWin,0,chkY,chkW,chkH,6);

  if (item_cnt == 0)
	  acsdmenu_add(menu1, "<~sd.list.empty>","<~sd.list.empty.desc>", DIR_ICON);
  //-- Populate Checkbox Items
  for (i=0;i<item_cnt;i++) {
    int item_len = strlen(item[i]);
    if (strcmp(item[i],"") != 0)
    {
      if(item[i][item_len - 1] == '/')
          acsdmenu_add(menu1, item[i], item_sub[i], DIR_ICON);
      else if(item_len > 4 && !strncmp(&item[i][item_len - 4], ".zip", 4))
          acsdmenu_add(menu1, item[i], item_sub[i], ZIP_ICON);
      else
    	  acsdmenu_add(menu1, item[i], item_sub[i], FILE_ICON);
    }
  }
  
  //-- Dispatch Message
  aw_show(hWin);
  byte ondispatch = 1;
  byte onback = 0;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 8:{
       //-- BACK
        onback     = 1;
        ondispatch = 0;
      }
      break;
      case 7:{
       //-- BACK TO MAIN
        onback     = 2;
        ondispatch = 0;
      }
      break;
      case 6:{
        ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
        ondispatch = 0;
      }
      break;
      case 4:{
        //-- EXIT
        ondispatch = 0;
      }
      break;
    }
  }
  
  int selindex;
  if (onback == 0)
      selindex = acsdmenu_getselectedindex(menu1);
  else if (onback == 2)
	  selindex = MENU_MAIN;
  else
	  selindex = -1;
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  return selindex;
}


//* 
//* alert
//*

STATUS dd_alert(int argc, char *format, ...) {
  if ((argc<2)||(argc>4)) {
    return dd_error("%s() expects 2-4 args (title, text, [icon, ok text]), got %d", __FUNCTION__, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Show Alert
  aw_alert(
    NULL,
    args[0],
    args[1],
    NULL,
    (argc>3)?args[3]:NULL
  );
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return RET_OK;
}

//* 
//* confirm
//*
STATUS dd_confirm(int argc, char *format, ...) {
  if ((argc<2)||(argc>5)) {
    return dd_error("%s() expects 2-4 args (title, text, [icon, yes text, no text]), got %d", __FUNCTION__, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Show Confirm
  byte res = aw_confirm(
    NULL,
    args[0],
    args[1],
    NULL,
    (argc>3)?args[3]:NULL,
    (argc>4)?args[4]:NULL
  );
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (res) return RET_YES;
  return RET_NO;
}

//* 
//* textdialog
//*
STATUS dd_textdialog(int argc, char *format, ...){
   return_val_if_fail(argc > 1, RET_FAIL);
  _INITARGS(); 
  //-- Set Busy before everythings ready
  ag_setbusy();
  //-- Show Text Dialog
  aw_textdialog(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:NULL
  );
 
  _FREEARGS(); 
  //-- Return
  return RET_OK;
}

//* 
//* loadlang
//*
static void dd_langloadsave(char * dest, int max, char * key){
  char * val = alang_get(key);
  if (val!=NULL) snprintf(dest,max,"%s",val);
}

STATUS dd_loadlang(char * name)
{
  ag_setbusy();
  
  
  //-- Load Language Data
  char path[256];
  snprintf(path,256,"%s/%s",DD_DIR,name);
  byte res = alang_load(path);
  
  //-- Replace Text
  if (res){
    acfg_reset_text();
    dd_langloadsave(acfg()->text_ok, 64, "text.ok");
    dd_langloadsave(acfg()->text_next, 64, "text.next");
    dd_langloadsave(acfg()->text_back, 64, "text.back");
    dd_langloadsave(acfg()->text_menumain, 64, "text.menumain");
    dd_langloadsave(acfg()->text_menuback, 64, "text.menuback");
    dd_langloadsave(acfg()->text_yes, 64, "text.yes");
    dd_langloadsave(acfg()->text_no, 64, "text.no");
    dd_langloadsave(acfg()->text_about, 64, "text.about");
    dd_langloadsave(acfg()->text_calibrating, 64, "text.calibrating");
    dd_langloadsave(acfg()->text_quit, 64, "text.quit");
    dd_langloadsave(acfg()->text_quit_msg, 128, "text.quit.msg");
  }
  
  return res; 
}

//* 
//* lang
//*
  
char * dd_lang(char *name){
  //-- Set Busy before everythings ready
  ag_setbusy();

  char * out = alang_get(name);

  return out;
}

STATUS dd_progress(char * icon, char *txt, char *fin_txt, int echo) {
  //-- Set Busy before everythings ready
  ag_setbusy();

  dd_isbgredraw = 1;

  //-- Init Background
  dd_setbg_title();

  //-- Init Strings
  char text[256];                   //-- Text When Installing
  char finish_text[256];            //-- Text After Installing
  snprintf(text,256,"%s", txt);
  snprintf(finish_text,256,"%s", fin_txt);

  //-- Drawing Data
  int pad         = agdp() * 4;  //边缘宽度
  int chkW        = agw() - (pad*2); //日志框宽度
  int bntH        = agdp() * 24;  //按钮高度
  int chkY  	  = dd_set_title_icon(&dd_win_bg, icon, text);
  int chkH        = agh() - (chkY + bntH + (pad*2));//日志框高度=屏幕高-电池栏高-按钮栏高-按钮栏边缘高
  int btnY        = chkY + chkH + pad; //按钮纵向起始Y

  //-- Finished Text Canvas
  CANVAS cvf;
  ag_canvas(&cvf,agw(),chkY - dd_minY);
  ag_draw_ex(&cvf,&dd_win_bg,0,0,0,dd_minY,agw(),cvf.h);
  int chkFY  	  = dd_set_title_icon(&cvf, icon, finish_text);
  int chkFH  	  = chkH; //计算结束日志框高度

  //-- Start Installer Proccess
  //if ret_status == 0 && echo = 1, install sucesss, make succes dialog
  //if ret_status != 0, install failed , list error log dialog
  //ret_status == -1; install failed
  int ret_status = dd_start_progress(
    &dd_win_bg,
    pad,chkY,chkW,chkH,
    pad,btnY,chkW,bntH,
    &cvf, dd_minY, chkFY, chkFH,
    echo
  );

  //-- Release Finished Canvas
  ag_ccanvas(&cvf);


  //-- Installer Not Return OK
  return ret_status;
}

STATUS dd_progress_animation(char * icon, char *txt, char *fin_txt, int echo, int type) {
  //-- Set Busy before everythings ready
  ag_setbusy();

  dd_isbgredraw = 1;
  dd_redraw_bg();
  dd_redraw_win();

  //-- Drawing Data
  int pad         = agdp() * 4;  //边缘宽度
  int chkW        = agw(); //日志框宽度
  int btnH        = agdp() * 24;  //按钮高度
  int btnW        = chkW - pad*2;  //按钮宽度
  int chkH        = agh() - btnH;//日志框高度=屏幕高-按钮栏高
  int chkX        = 0; //日志框纵向起始Y
  int chkY        = 0; //日志框纵向起始Y
  int btnX        = pad; //按钮纵向起始Y
  int btnY        = chkH - pad; //按钮纵向起始Y

  //-- Finished Text Canvas
  CANVAS cvf;
  ag_canvas(&cvf,agw(),agh());
  ag_draw(&cvf,&dd_win_bg,0,0);

  int chkFH       = agh() - btnH;//日志框高度=屏幕高-按钮栏高
  int chkFX       = 0; //日志框纵向起始Y
  int chkFY       = 0; //日志框纵向起始Y

  //-- Start Installer Proccess
  int ret_status = dd_start_progress_animation(
    &dd_bg,
    chkX,chkY,chkW,chkH,
    btnX,btnY,btnW,btnH,
    &cvf, chkFX, chkFY, chkFH,
    echo, type
  );

  //-- Release Finished Canvas
  ag_ccanvas(&cvf);

  //-- Installer Not Return OK
  return ret_status;
}

#if 0
static int time_echo_enable = 1;
static pthread_t time_thread_t;
static void *time_echo_thread(void *cookie){
    while(time_echo_enable)
    {
        dd_setbg_title();
        ag_draw(NULL, &dd_win_bg, 0, 0);
        ag_sync_fade(0);
        //interval can't be small
        sleep(10);
    }
    return NULL;
}
#endif

#define DD_INITARGS() \
          char** args = ReadVarArgs(state, argc, argv); \
          if (args==NULL) return NULL;

#define DD_FREEARGS() \
          int freearg_i; \
          for (freearg_i=0;freearg_i<argc;++freearg_i) free(args[freearg_i]); \
          free(args);

Value* DD_INI_SET(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 2) {
    dd_error("%s() expects 2 args(config name, config value in string), got %d", name, argc);
    return StringValue(strdup(""));
  }

  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  DD_INITARGS();
  //-- Convert Arguments
  byte valint = (byte) min(atoi(args[1]),255);
  int  valkey = (int) atoi(args[1]);
  
  //-- Set Property
  if      (strcmp(args[0],"roundsize") == 0)          acfg()->roundsz=valint;
  else if (strcmp(args[0],"button_roundsize") == 0)   acfg()->btnroundsz=valint;
  else if (strcmp(args[0],"window_roundsize") == 0)   acfg()->winroundsz=valint;
  else if (strcmp(args[0],"transition_frame") == 0)   acfg()->fadeframes=valint;

  else if (strcmp(args[0],"text_ok") == 0)            snprintf(acfg()->text_ok,64,"%s", args[1]);
  else if (strcmp(args[0],"text_next") == 0)          snprintf(acfg()->text_next,64,"%s", args[1]);
  else if (strcmp(args[0],"text_back") == 0)          snprintf(acfg()->text_back,64,"%s", args[1]);

  else if (strcmp(args[0],"text_yes") == 0)           snprintf(acfg()->text_yes,64,"%s", args[1]);
  else if (strcmp(args[0],"text_no") == 0)            snprintf(acfg()->text_no,64,"%s", args[1]);
  else if (strcmp(args[0],"text_about") == 0)         snprintf(acfg()->text_about,64, "%s", args[1]);
  else if (strcmp(args[0],"text_calibrating") == 0)   snprintf(acfg()->text_calibrating,64,"%s", args[1]);
  else if (strcmp(args[0],"text_quit") == 0)          snprintf(acfg()->text_quit,64,"%s", args[1]);
  else if (strcmp(args[0],"text_quit_msg") == 0)      snprintf(acfg()->text_quit_msg,128,"%s", args[1]);
    
  else if (strcmp(args[0],"rec_name") == 0)           snprintf(acfg()->rec_name,128,"%s", args[1]);
  else if (strcmp(args[0],"rec_version") == 0)        snprintf(acfg()->rec_version,128,"%s", args[1]);
  else if (strcmp(args[0],"rec_iteration") == 0)      snprintf(acfg()->rec_iteration,128,"%s", args[1]);
  else if (strcmp(args[0],"rec_author") == 0)         snprintf(acfg()->rec_author,128,"%s", args[1]);
  else if (strcmp(args[0],"rec_device") == 0)         snprintf(acfg()->rec_device,128,"%s", args[1]);
  else if (strcmp(args[0],"rec_date") == 0)           snprintf(acfg()->rec_date,128,"%s", args[1]);
  else if (strcmp(args[0],"brightness_path") == 0)    snprintf(acfg()->brightness_path, PATH_MAX, "%s", args[1]);
  else if (strcmp(args[0],"battery_path") == 0)    	  snprintf(acfg()->battery_path, PATH_MAX, "%s", args[1]);
  else if (strcmp(args[0],"lun_file") == 0)     	  snprintf(acfg()->lun_file, PATH_MAX, "%s", args[1]);
  else if (strcmp(args[0],"reboot_cmd") == 0)         snprintf(acfg()->reboot_cmd,128,"%s", args[1]);
  else if (strcmp(args[0],"bootloader_cmd") == 0)     snprintf(acfg()->bootloader_cmd,128,"%s", args[1]);
  
  else if (strcmp(args[0],"enable_usb") == 0)    	  acfg()->enable_usb=valkey;
  else if (strcmp(args[0],"enable_sideload") == 0)    acfg()->enable_sideload=valkey;
  else if (strcmp(args[0],"enable_reboot_bootloader") == 0)  acfg()->enable_bootloader=valkey;
  else if (strcmp(args[0],"support_root") == 0)    	  acfg()->root=valkey;
  else if (strcmp(args[0],"customkeycode_up")==0)     acfg()->ckey_up=valkey;
  else if (strcmp(args[0],"customkeycode_down")==0)   acfg()->ckey_down=valkey;
  else if (strcmp(args[0],"customkeycode_select")==0) acfg()->ckey_select=valkey;
  else if (strcmp(args[0],"customkeycode_back") == 0) acfg()->ckey_back=valkey;
  else if (strcmp(args[0],"customkeycode_menu") == 0) acfg()->ckey_menu=valkey;
  else if (strcmp(args[0],"customkeycode_home") == 0) acfg()->ckey_home=valkey;
  else if (strcmp(args[0],"virtualkeys_enable") == 0) acfg()->virkeys_en = valkey;
  //add for input event filter
  else if (strcmp(args[0], "input_filter")  == 0) {
     acfg()->input_filter = valkey;
	 dd_debug("input is 0x%x\n", acfg()->input_filter);
  }
    
  //-- Force Color Space  
  else if (strcmp(args[0],"force_colorspace") == 0){
	  if(strcmp(args[1], "rgb565") == 0)
		  ag_changcolor_565();
	  else
		  ag_changcolor(args[1][0], args[1][1], args[1][2], args[1][3]);
  }

  else if (strcmp(args[0],"dp") == 0){
	  set_agdp(valint);
  }
  else if (strcmp(args[0], "internal_sdcard")== 0) {
	  acfg()->sd_int=valint;
  }
  else if (strcmp(args[0], "external_sdcard")== 0) {
      acfg()->sd_ext=valint;
  } else
	  dd_debug("Unknown item: %s\n", args[1]);
  
  dd_isbgredraw = 1;
  //-- Release Arguments
  DD_FREEARGS();

  //-- Return
  return StringValue(strdup(""));
}

Value* DD_CALIBRATE(const char* name, State* state, int argc, Expr* argv[]) {
  //-- Return
  return StringValue(strdup(""));
}

int dd_gen_verfile()
{
	char version[64];
	FILE * fp = fopen(VERSION_TMP_PATH,"wb");
	if (fp!=NULL){
   		snprintf(version,64,"[version]:%s\n[iteration]:%s",acfg()->rec_version, acfg()->rec_iteration);
   		printf("Recovery version: %s\n", acfg()->rec_version);
	    fwrite(version,1,strlen(version),fp);
	    fclose(fp);
	    return 0;
	}
	return 1;

}

static void dd_ui_register()
{
//todo
    //--CONFIG FUNCTIONS
    //
    RegisterFunction("ini_set",   DD_INI_SET);       //-- SET INI CONFIGURATION
    RegisterFunction("calibrate", DD_CALIBRATE);
    //RegisterFunction("calibtool", DD_CALIB);
}
int dd_ui_init()
{
    acfg_init();
    //register function
    RegisterBuiltins();
    dd_ui_register();
    FinishRegistration();
	return 0;
}
//read config file if exist and then execute it
int dd_ui_config(const char *file)
{
    //if file exist 
    return_val_if_fail(file != NULL, RET_FAIL);
    struct stat file_stat;
    if (stat(file, &file_stat) < 0)
    {
        dd_printf("stat file error, file is not exist\n");
        return -1;
    }
    char *script_data = dd_readfromfs(file);
    return_val_if_fail(script_data != NULL, RET_FAIL);

   //--PARSE CONFIG SCRIPT
   Expr* root;
   int error_count = 0;
   yy_scan_string(script_data); 
   int error = yyparse(&root, &error_count);
   if (error != 0 || error_count > 0) {
       dd_printf("read file %s failed!\n", file);
       goto config_fail;
   }
   //--- EVALUATE CONFIG SCRIPT 
   State state;
   state.cookie = NULL;
   state.script = script_data;
   state.errmsg = NULL;
   char* result = Evaluate(&state, root);
   if (result == NULL) {
       if (state.errmsg == NULL) {
           dd_printf("script aborted\n");
       } else {
           free(state.errmsg);
       }
   } else {
       dd_printf("script read!\n");
       free(result);
   }
config_fail:
   if (script_data != NULL) free(script_data);
   return -1;
}

STATUS dd_ui_start()
{
    int i = 0;
    for (i = 0; i < DD_THEME_CNT; i++)
    {
        acfg()->theme[i] = NULL;
        acfg()->theme_9p[i] = 0;
    }
    ag_canvas(&dd_win_bg, agw(), agh());
    ag_canvas(&dd_bg, agw(), agh());
    dd_theme("dingdong");
    alang_release();
    return RET_OK;
}
STATUS dd_ui_end()
{
    ag_ccanvas(&dd_win_bg);
    ag_ccanvas(&dd_bg);
    alang_release();
    atheme_releaseall();
    return RET_OK;
}
