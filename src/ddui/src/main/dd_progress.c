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
 * Installer Proccess
 *
 */

#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"

static float     ai_progress_pos     = 0;
static float     ai_progress_fract   = 0;
static int       ai_progress_fract_n = 0;
static int       ai_progress_fract_c = 0;
static long      ai_progress_fract_l = 0;
static byte      ai_run              = 0;
static int       ai_progani_pos      = 0;
static int       ai_progress_w       = 0;
static int       ai_prog_x           = 0;
static int       ai_prog_y           = 0;
static int       ai_prog_w           = 0;
static int       ai_prog_h           = 0;
static int       ai_prog_r           = 0;
static int       ai_prog_ox          = 0;
static int       ai_prog_oy          = 0;
static int       ai_prog_ow          = 0;
static int       ai_prog_oh          = 0;
static int       ai_prog_or          = 0;
static CANVAS *  ai_bg               = NULL;
static CANVAS *  ai_cv               = NULL;
static char      ai_progress_text[64];
static char      ai_progress_info[101];
static AWINDOWP  ai_win;
static ACONTROLP ai_buftxt;
static int		 ai_animation		 = 0;
static int		 ai_an_h			 = 0;
static CANVAS    ai_an;

static pthread_mutex_t ai_progress_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ai_canvas_mutex_progress = PTHREAD_MUTEX_INITIALIZER;

void ai_canvas_lock_progress()
{
    pthread_mutex_lock(&ai_canvas_mutex_progress);
}
void ai_canvas_unlock_progress()
{
    pthread_mutex_unlock(&ai_canvas_mutex_progress);
}


#define DD_INSTALL_LOG "/tmp/recovery.log"
#define DD_INSTALL_LOG_TMP "/tmp/installinfo.log"
static struct _ddProgress dd_progress_struct ={
    .start_pfun = NULL,
    .args = NULL
};
static struct _ddProgress* pdd_progress = &dd_progress_struct;

//echo text when install failed
void ddProgress_rebuildtxt(int cx,int cy,int cw,int ch){
  char* buffer = NULL;
  struct stat st;
  if (stat(DD_INSTALL_LOG,&st) < 0) return;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;
  FILE* f = fopen(DD_INSTALL_LOG, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  buffer[st.st_size] = '\0';
  fclose(f);
done:
  actext_rebuild(
    ai_buftxt,
    cx,cy,cw,ch,
    ((buffer!=NULL)?buffer:""),
    0,1);
  free(buffer);

}

static void *dd_funthread(void *cookie){
    if (pdd_progress->start_pfun != NULL)
    {
    	pdd_progress->start_pfun(pdd_progress->args);
    }
    else
    	printf("pdd_install->pfun is NULL, force return\n");
    return NULL;
}

void ddProgress_actionsavelog(char * name){
  char* buffer = NULL;
  struct stat st;
  if (stat(DD_INSTALL_LOG,&st) < 0) return;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;

  FILE* f = fopen(DD_INSTALL_LOG, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  buffer[st.st_size] = '\0';
  fclose(f);

  f = fopen(name, "wb");
  if (f == NULL)
  {
      dd_error("%s open failed!\n", name);
      goto done;
  }
  fprintf(f,"%s", buffer);
  fclose(f);
done:
  if (buffer!=NULL) free(buffer);
}

void ddProgress_dump_logs(){
  char dumpname[256];
  char msgtext[256];
  struct stat st;

  snprintf(dumpname,255,"%s/recovery.log",RECOVERY_PATH);
  ddOp_send(OP_MOUNT, 1, RECOVERY_PATH);
  int result = ddOp_result_get_int();
  if(result) {
	  if (acfg()->sd_int == 1)  {
		  snprintf(dumpname,255,"%s/recovery.log",RECOVERY_PATH_INTER);
		  ddOp_send(OP_MOUNT, 1, RECOVERY_PATH_INTER);
		  result = ddOp_result_get_int();
		  if(result) {
		      aw_alert(ai_win, "<~progress.savelog.button>", "<~backup_path.setting.alert>", NULL, NULL);
		      return;
		  }
		  if (stat(RECOVERY_PATH_INTER, &st) != 0)
			  mkdir(RECOVERY_PATH_INTER, 0755);
	  }
	  else
		  aw_alert(ai_win, "<~progress.savelog.button>", "<~backup_path.setting.alert>", NULL, NULL);
  } else {
	  if (stat(RECOVERY_PATH, &st) != 0)
		  mkdir(RECOVERY_PATH, 0755);
  }

  snprintf(msgtext,255,"<~progress.savelog.tip>\n\n<#080>%s</#>\n\n<~progress.savelog.alert>",dumpname);

  byte res = aw_confirm(
    ai_win,
    "<~progress.savelog.button>",
    msgtext,
    NULL,
    NULL,
    NULL
  );

  if (res){
	ddProgress_actionsavelog(dumpname);
    aw_alert(
      ai_win,
      "<~progress.savelog.button>",
      "<~progress.savelog.success>",
      NULL,
      NULL
    );
  }

}


static void *ac_progressthread(void *cookie){
  //-- COLORS
  dword hl1 = ag_calchighlight(acfg()->selectbg,acfg()->selectbg_g);
  byte sg_r = ag_r(acfg()->progressglow);
  byte sg_g = ag_g(acfg()->progressglow);
  byte sg_b = ag_b(acfg()->progressglow);
  sg_r = min(sg_r*1.4,255);
  sg_g = min(sg_g*1.4,255);
  sg_b = min(sg_b*1.4,255);

  PNGCANVAS ap[DD_ANIMATION_FRAM];
  char *text ;
  text  = "<~update.wait>";
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  imgX       = 0;
  int  imgY       = 0;
  int  txtX 	  = 0;
  int  txtY 	  = 0;
  int  txtW 	  = ag_txtwidth(text, 2);
  int  i;
  int gInstallingFrame = 0;
  if (ai_animation) {
	  //-- Load Icon
	  for(i=0; i<DD_ANIMATION_FRAM; i++) {
		  char filename[40];
		  sprintf(filename, ANIMATION_FLASH"%02d", i+1);
		  if (apng_load(&ap[i],filename)){
			imgE  = 1;
		  }
	  }
	  if (imgE) {
		  imgW  = ap[0].w;
		  imgH  = ap[0].h;
		  imgX  = (agw() - imgW) / 2;
		  imgY  = (ai_an_h - imgH) / 3;
		  imgA  = imgW;
		  txtX  = (agw()/2) - (txtW/2);
		  txtY  = imgY + imgH + agdp() * 8;
	  }
  }
while(ai_run){
    //-- CALCULATE PROGRESS BY TIME
    pthread_mutex_lock(&ai_progress_mutex);
    if(ai_progress_fract_n<0){
      long curtick  = alib_tick();
      int  targetc  = abs(ai_progress_fract_n);
      long  tickdiff = curtick - ai_progress_fract_l;
      if (tickdiff>0){
        long diffms          = tickdiff*10;
        ai_progress_fract_l  = curtick;
        ai_progress_fract_n += diffms;
        if (ai_progress_fract_n>=0){
          diffms-=ai_progress_fract_n;
          ai_progress_fract_n = 0;
        }
        float curradd        = ai_progress_fract*diffms;
        ai_progress_pos     += curradd;
      }
    }

    //-- Safe Progress
    if (ai_progress_pos>1) ai_progress_pos=1.0;
    if (ai_progress_pos<0) ai_progress_pos=0.0;
    int prog_g = ai_prog_w; //-(ai_prog_r*2);
    int prog_w = round(ai_prog_w*ai_progress_pos);

    pthread_mutex_unlock(&ai_progress_mutex);


    //-- Percent Text
    float prog_percent = 100 * ai_progress_pos;
    char prog_percent_str[12];
    snprintf(prog_percent_str,12,"%0.1f%%",prog_percent);
    int ptxt_p = agdp()*5;
    int ptxt_y = ai_prog_oy-(ptxt_p+(ag_fontheight(0)*2));
    int ptxt_w = ag_txtwidth(prog_percent_str,1);
    int ptxt_x = (ai_prog_ox+ai_prog_ow)-(ptxt_w+ai_prog_or);
    int ptx1_x = ai_prog_ox+ai_prog_or;
    int ptx1_w = agw()-(agw()/3);

    if (ai_progress_w<prog_w){
      int diff       = ceil((prog_w-ai_progress_w)*0.1);
      ai_progress_w +=diff;
      if (ai_progress_w>prog_w) ai_progress_w=prog_w;
    }
    else if (ai_progress_w>prog_w){
      int diff       = ceil((ai_progress_w-prog_w)*0.1);
      ai_progress_w -=diff;
      if (ai_progress_w<prog_w) ai_progress_w=prog_w;
    }
    int issmall = -1;
    if (ai_progress_w<(ai_prog_r*2)){
      issmall = ai_progress_w;
      ai_progress_w = (ai_prog_r*2);
    }

    ai_canvas_lock_progress();
    ag_draw_ex(ai_cv,ai_bg,0,ptxt_y,0,ptxt_y,agw(),agh()-ptxt_y);
    int curr_prog_w = round(ai_prog_ow*ai_progress_pos);
    if (!atheme_draw("img.prograss.fill",ai_cv,ai_prog_ox,ai_prog_oy,curr_prog_w,ai_prog_oh)){
      ag_roundgrad(ai_cv,ai_prog_x,ai_prog_y,ai_progress_w,ai_prog_h,acfg()->selectbg,acfg()->selectbg_g,ai_prog_r);
      ag_roundgrad_ex(ai_cv,ai_prog_x,ai_prog_y,ai_progress_w,ceil((ai_prog_h)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_r,2,2,0,0);
      if (issmall>=0){
        ag_draw_ex(ai_cv,ai_bg,ai_prog_x+issmall,ai_prog_oy,ai_prog_x+issmall,ai_prog_oy,(ai_prog_r*2),ai_prog_oh);
      }
    }
    if (ai_animation) {
        ag_texts (ai_cv,ptx1_w,ptx1_x  ,ptxt_y  ,ai_progress_text,acfg()->titlefg,1);
        ag_texts (ai_cv,ai_prog_w-(ai_prog_or*2),ptx1_x  ,ptxt_y+ag_fontheight(1)+agdp(),ai_progress_info,acfg()->titlefg,0);
        ag_texts (ai_cv,ptxt_w,ptxt_x,ptxt_y,prog_percent_str,acfg()->titlefg,1);
    } else {
        ag_texts (ai_cv,ptx1_w,ptx1_x  ,ptxt_y  ,ai_progress_text,acfg()->winfg,1);
        ag_texts (ai_cv,ai_prog_w-(ai_prog_or*2),ptx1_x  ,ptxt_y+ag_fontheight(1)+agdp(),ai_progress_info,acfg()->winfg_gray,0);
    	ag_texts (ai_cv,ptxt_w,ptxt_x,ptxt_y,prog_percent_str,acfg()->winfg,1);
    }

    prog_g = ai_prog_w-(ai_prog_r*2);

    if (++ai_progani_pos>60) ai_progani_pos=0;
    int x    = ai_progani_pos;
    int hpos = prog_g/2;
    int vpos = ((prog_g+hpos)*x) / 60;
    int hhpos= prog_g/4;
    int hph  = ai_prog_h/2;
    int xx;
    int  sgmp = agdp()*40;

    if ((vpos>0)&&(hpos>0)){
      for (xx=0;xx<prog_g;xx++){
        int alp     = 255;
        float alx   = 1.0;
        int vn = (vpos-xx)-hhpos;
        if ((vn>0)){
          if (vn<hhpos){
            alp = (((hhpos-vn) * 255) / hhpos);
          }
          else if (vn<hpos){
            alp = (((vn-hhpos) * 255) / hhpos);
          }
        }
        if (xx<sgmp){
          alx = 1.0-(((float) (sgmp-xx)) / sgmp);
        }
        else if (xx>prog_g-sgmp){
          alx = 1.0-(((float) (xx-(prog_g-sgmp))) / sgmp);
        }
        int alpha = min(max(alx * (255-alp),0),255);

        int anix = ai_prog_x+ai_prog_r+xx;
        int yy;
        byte er = 0;
        byte eg = 0;
        byte eb = 0;
        for (yy=0;yy<ai_prog_oh;yy++){
          color * ic = agxy(ai_cv,anix,ai_prog_oy+yy);
          byte  l  = alpha*(0.5+((((float) yy+1)/((float) ai_prog_oh))*0.5));
          byte  ralpha = 255 - l;
          byte r = (byte) (((((int) ag_r(ic[0])) * ralpha) + (((int) sg_r) * l)) >> 8);
          byte g = (byte) (((((int) ag_g(ic[0])) * ralpha) + (((int) sg_g) * l)) >> 8);
          byte b = (byte) (((((int) ag_b(ic[0])) * ralpha) + (((int) sg_b) * l)) >> 8);
          r  = min(r+er,255);
          g  = min(g+eg,255);
          b  = min(b+eb,255);
          byte nr  = ag_close_r(r);
          byte ng  = ag_close_g(g);
          byte nb  = ag_close_b(b);
          er = r-nr;
          eg = g-ng;
          eb = b-nb;
          ic[0]=ag_rgb(nr,ng,nb);
        }
      }
    }

	  if (ai_animation && imgE){
		  i = (i + 1) % DD_ANIMATION_FRAM;
		  ag_canvas(&ai_an,agw(),agh());
		  dd_redraw_bg_ex(&ai_an);
		  apng_draw(&ai_an,&ap[i],imgX,imgY);
		  ag_textf(&ai_an, txtW, txtX, txtY, text, acfg()->titlefg, 2);
		  ag_draw_ex(ai_cv,&ai_an,0,0,0,0,agw(),ai_an_h);
	  }

    aw_draw(ai_win);
    ai_canvas_unlock_progress();
    if (ai_animation && imgE)
    	usleep(1000);
    else
    	usleep(160);
  }

  for(i=0; i<DD_ANIMATION_FRAM; i++) {
	  apng_close(&ap[i]);
  }
  return NULL;
}

void dd_init_progress(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph
){
  //-- Calculate Progress Location&Size

  pthread_mutex_lock(&ai_progress_mutex);
  ai_prog_oh = agdp()*6;  //进度条高度
  ai_prog_oy = 0;  //进度条起始y
  ai_prog_ox = px; //进度条起始x
  ai_prog_ow = pw; //进度条宽度
  if (ai_prog_oh>ph) ai_prog_oh=ph;
  else{
    ai_prog_oy = (ph/2)-(ai_prog_oh/2);
  }
  ai_prog_oy += py;
  ai_prog_or = ai_prog_oh/2;

  //-- Draw Progress Holder Into BG
  dword hl1 = ag_calchighlight(acfg()->controlbg,acfg()->controlbg_g);
  ag_rect(bg, 0, py-agdp()*16, agw(), ph+agdp()*24, acfg()->titlefg);

  if (!atheme_draw("img.progress",bg,px,ai_prog_oy,pw,ai_prog_oh)){
    ag_roundgrad(bg,px,ai_prog_oy,pw,ai_prog_oh,acfg()->border,acfg()->border_g,ai_prog_or);
    ag_roundgrad(bg,px+1,ai_prog_oy+1,pw-2,ai_prog_oh-2,
      ag_calculatealpha(acfg()->controlbg,0xffff,180),
      ag_calculatealpha(acfg()->controlbg_g,0xffff,160), ai_prog_or-1);
    ag_roundgrad(bg,px+2,ai_prog_oy+2,pw-4,ai_prog_oh-4,acfg()->controlbg,acfg()->controlbg_g,ai_prog_or-2);
    ag_roundgrad_ex(bg,px+2,ai_prog_oy+2,pw-4,ceil((ai_prog_oh-4)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_or-2,2,2,0,0);
  }

  //-- Calculate Progress Value Locations
  int hlfdp  = ceil(((float) agdp())/2);
  ai_prog_x = px+(hlfdp+1);
  ai_prog_y = ai_prog_oy+(hlfdp+1);
  ai_prog_h = ai_prog_oh-((hlfdp*2)+2);
  ai_prog_w = pw-((hlfdp*2)+2);
  ai_prog_r = ai_prog_or-(1+hlfdp);
  snprintf(ai_progress_text,63,"<~progress.running.info>");
  snprintf(ai_progress_info,100,"");
  pthread_mutex_unlock(&ai_progress_mutex);
  return ;
}

void dd_init_progress_animation(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph
){
	  //-- Calculate Progress Location&Size

	  pthread_mutex_lock(&ai_progress_mutex);
	  ai_prog_oh = agdp()*6;  //进度条高度
	  ai_prog_oy = 0;  //进度条起始y
	  ai_prog_ox = px + agdp()*10; //进度条起始x
	  ai_prog_ow = pw - agdp()*20; //进度条宽度
	  if (ai_prog_oh>ph) ai_prog_oh=ph;
	  else{
	    ai_prog_oy = (ph/2)-(ai_prog_oh/2);
	  }
	  ai_prog_oy += py;
	  ai_prog_or = ai_prog_oh/2;

	  //-- Draw Progress Holder Into BG
	  dword hl1 = ag_calchighlight(acfg()->controlbg,acfg()->controlbg_g);

	  if (!atheme_draw("img.progress",bg,ai_prog_ox,ai_prog_oy,ai_prog_ow,ai_prog_oh)){
	    ag_roundgrad(bg,ai_prog_ox,ai_prog_oy,ai_prog_ow,ai_prog_oh,acfg()->border,acfg()->border_g,ai_prog_or);
	    ag_roundgrad(bg,ai_prog_ox+1,ai_prog_oy+1,ai_prog_ow-2,ai_prog_oh-2,
	      ag_calculatealpha(acfg()->controlbg,0xffff,180),
	      ag_calculatealpha(acfg()->controlbg_g,0xffff,160), ai_prog_or-1);
	    ag_roundgrad(bg,ai_prog_ox+2,ai_prog_oy+2,ai_prog_ow-4,ai_prog_oh-4,acfg()->controlbg,acfg()->controlbg_g,ai_prog_or-2);
	    ag_roundgrad_ex(bg,ai_prog_ox+2,ai_prog_oy+2,ai_prog_ow-4,ceil((ai_prog_oh-4)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_or-2,2,2,0,0);
	  }

	  //-- Calculate Progress Value Locations
	  int hlfdp  = ceil(((float) agdp())/2);
	  ai_prog_x = ai_prog_ox+(hlfdp+1);
	  ai_prog_y = ai_prog_oy+(hlfdp+1);
	  ai_prog_h = ai_prog_oh-((hlfdp*2)+2);
	  ai_prog_w = ai_prog_ow-((hlfdp*2)+2);
	  ai_prog_r = ai_prog_or-(1+hlfdp);
	  snprintf(ai_progress_text,63,"<b><~progress.installing.info></b>");
	  snprintf(ai_progress_info,100,"");
	  pthread_mutex_unlock(&ai_progress_mutex);
	  return ;
}

int dd_start_progress(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph,
  CANVAS * cvf, int imgY, int chkFY, int chkFH,
  int echo
){
  int ai_return_status = 0;
  //-- Save Canvases
  ai_bg = bg;

  ddProgress_reset_progress();
  ai_canvas_lock_progress();
  dd_init_progress(bg,cx,cy,cw,ch,px,py,pw,ph);
  AWINDOWP hWin     = aw(bg);
  ai_win            = hWin;
  ai_cv             = &hWin->c;
  ai_progress_pos   = 0.0;
  ai_progress_w     = 0;
  ai_run            = 1;
  ai_buftxt         = actext(hWin,0,cy,agw(),ch-(agdp()*6),NULL,1);
  aw_set_on_dialog(1);
  ai_canvas_unlock_progress();

  aw_show(hWin);


  pthread_t threadProgress, threadOperating;
  pthread_create(&threadProgress, NULL, ac_progressthread, NULL);
  pthread_create(&threadOperating, NULL, dd_funthread, NULL);
  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 16:{
        //install failed
        ddProgress_set_text(pdd_progress->args->fail_text);
        ondispatch = 0;
        ai_return_status = -1;

      }
         break;
      case 15:{
        //install ok
        ddProgress_set_text(pdd_progress->args->success_text);
        ai_return_status = 0;
        ondispatch = 0;
      }
      break;
    }
  }
  ai_run = 0;
  hWin->isActived = 0;
  pthread_join(threadProgress,NULL);
  pthread_join(threadOperating,NULL);
  pthread_detach(threadProgress);
  pthread_detach(threadOperating);
  if (ai_return_status == -1 || 1 == echo)
  {
      int pad = agdp() * 4;

      ai_canvas_lock_progress();
      //dd_drawnav(bg, 0, py, agw(), ph+pad);
      ag_rect(bg, 0, py-pad*6, agw(), ph+pad*8, acfg()->titlefg);
      ag_draw_ex(bg, cvf, 0, imgY, 0, 0, cvf->w, cvf->h);
      ag_draw(&hWin->c, bg, 0, 0);
      ai_canvas_unlock_progress();
      //Show Next Button
      ACONTROLP nxtbtn=acbutton(
        hWin,
        pad+(agdp()*2)+(cw/2),py,(cw/2)-(agdp()*2),ph,acfg()->text_next,1,
        11
      );
      // Show Dump Button
      acbutton(
        hWin,
        pad,py,(cw/2)-(agdp()*2),ph,"<~progress.savelog.button>",1,
        12
      );
      aw_show(hWin);
      //aw_setfocus(hWin,nxtbtn);
      ondispatch = 1;
      while(ondispatch){
          dword msg = aw_dispatch(hWin);
          switch(aw_gm(msg))
          {
          case 12:
        	  ddProgress_dump_logs();
              break;
          case 11:
              ondispatch = 0;
              break;
        }
      }
  }
  aw_set_on_dialog(0);
  aw_destroy(hWin);

  return WEXITSTATUS(ai_return_status);
}

int dd_start_progress_animation(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph,
  CANVAS * cvf, int chkFX, int chkFY, int chkFH,
  int echo
){
  int ai_return_status = 0;
  //-- Save Canvases
  ai_bg = bg;

  ddProgress_reset_progress();
  ai_canvas_lock_progress();
  dd_init_progress_animation(bg,cx,cy,cw,ch,px,py,pw,ph);
  AWINDOWP hWin     = aw(bg);
  AWINDOWP fWin     = aw(cvf);
  ai_win            = hWin;
  ai_cv             = &hWin->c;
  ai_progress_pos   = 0.0;
  ai_progress_w     = 0;
  ai_run            = 1;
  ai_animation		= 1;
  ai_an_h			= agh() - ph*2;
  ai_buftxt         = actext(fWin,cx,cy+(agdp()*5),cw,ch-(agdp()*15),NULL,1);
  aw_set_on_dialog(1);
  ai_canvas_unlock_progress();

  aw_show(hWin);

  pthread_t threadOperating, threadProgress;
  pthread_create(&threadProgress, NULL, ac_progressthread, NULL);
  pthread_create(&threadOperating, NULL, dd_funthread, NULL);

  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 16:{
        //install failed
        ddProgress_set_text(pdd_progress->args->fail_text);
        ondispatch = 0;
        ai_return_status = -1;

      }
         break;
      case 15:{
        //install ok
        ddProgress_set_text(pdd_progress->args->success_text);
        ai_return_status = 0;
        ondispatch = 0;
      }
      break;
    }
  }
  ai_run = 0;
  hWin->isActived = 0;
  ai_win = fWin;
  pthread_join(threadOperating,NULL);
  pthread_join(threadProgress,NULL);
  pthread_detach(threadOperating);
  pthread_detach(threadProgress);

  if (ai_return_status == -1 || 1 == echo)
  {
	  ai_animation = 0;
      int pad = agdp() * 4;
      ai_canvas_lock_progress();
      ag_draw_ex(bg, cvf, 0, 0, 0, chkFY, cvf->w, cvf->h);
      ag_rect(bg, 0, py-pad*6, agw(), ph+pad*8, acfg()->titlefg);
      ag_draw(&fWin->c, bg, 0, 0);
      ai_canvas_unlock_progress();
      //Show Next Button
      ACONTROLP nxtbtn=acbutton(
        fWin,
        pad+(cw/2),py,(cw/2)-(pad*2),ph,acfg()->text_next,1,
        11
      );
      // Show Dump Button
      acbutton(
        fWin,
        pad,py,(cw/2)-(pad*2),ph,"<~progress.savelog.button>",1,
        12
      );

      aw_show(fWin);
      //aw_setfocus(fWin,nxtbtn);
      ondispatch = 1;
      while(ondispatch){
          dword msg = aw_dispatch(fWin);
          switch(aw_gm(msg))
          {
          case 12:
        	  ddProgress_dump_logs();
              break;
          case 11:
              ondispatch = 0;
              break;
        }
      }
  }
  aw_set_on_dialog(0);
  aw_destroy(hWin);
  aw_destroy(fWin);

  return WEXITSTATUS(ai_return_status);
}

STATUS ddProgress_init(ddProgress_fun start_fun, struct _ddProgress_unit *ddProgress_args)
{
    pdd_progress->start_pfun = start_fun;
    pdd_progress->args = ddProgress_args;
    return RET_OK;
}

void ddProgress_show_progress(float portion, int seconds)
{
    pthread_mutex_lock(&ai_progress_mutex);
    float progsize = portion;
    ai_progress_fract_n = abs(seconds);
    ai_progress_fract_c = 0;
    ai_progress_fract_l = alib_tick();
    if (ai_progress_fract_n>0)
      ai_progress_fract = progsize/ai_progress_fract_n;
    else if(ai_progress_fract_n<0)
      ai_progress_fract = progsize/abs(ai_progress_fract_n);
    else{
      ai_progress_fract = 0;
      ai_progress_pos   += progsize;
    }
    pthread_mutex_unlock(&ai_progress_mutex);
    return ;
}

void ddProgress_set_progress(float fraction)
{
    pthread_mutex_lock(&ai_progress_mutex);
    ai_progress_fract   = 0;
    ai_progress_fract_n = 0;
    ai_progress_fract_c = 0;
    ai_progress_pos     = fraction ;
    pthread_mutex_unlock(&ai_progress_mutex);
    return ;
}
void ddProgress_reset_progress()
{
    pthread_mutex_lock(&ai_progress_mutex);
    ai_progani_pos      = 0;
    ai_progress_pos     = 0;
    ai_progress_fract   = 0;
    ai_progress_fract_n = 0;
    ai_progress_fract_c = 0;
    ai_progress_fract_l = 0;
    ai_progress_w     = 0;
    ai_prog_x         = 0;
    ai_prog_y         = 0;
    ai_prog_w         = 0;
    ai_prog_h         = 0;
    ai_prog_r         = 0;
    ai_prog_ox        = 0;
    ai_prog_oy        = 0;
    ai_prog_ow        = 0;
    ai_prog_oh        = 0;
    ai_prog_or        = 0;
    pthread_mutex_unlock(&ai_progress_mutex);
    return ;
}

void ddProgress_set_text(char *str)
{
    ai_canvas_lock_progress();
    actext_appendtxt(ai_buftxt, str);
    ai_canvas_unlock_progress();
    return ;
}

int ddProgress_set_pause(int status)
{
	if (status == 0)
		pthread_mutex_unlock(&ai_progress_mutex);
	else
		pthread_mutex_lock(&ai_progress_mutex);
    return status;
}

//echo text with progress item
void ddProgress_set_info(char* file_name)
{

    char *filename = file_name;
    snprintf(ai_progress_info,100,"%s",filename);
    return ;
}
