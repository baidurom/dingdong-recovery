/* Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
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
 * DD UI: Button Window Control
 *
 */
#include "../dd_inter.h"

/***************************[ BUTTON ]**************************/
typedef struct{
  CANVAS    control;
  CANVAS    control_pushed;
  CANVAS    control_focused;
  byte      touchmsg;
  byte      focused;
  byte      pushed;
} ACBUTTOND, * ACBUTTONDP;
dword acvirkeys_oninput(void * x,int action,ATEV * atev){
  ACONTROLP ctl  = (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;

  dword msg = 0;
  switch (action){
    case ATEV_MOUSEDN:
      {
        vibrate(30);
        d->pushed=1;
        msg=aw_msg(0,1,0,0);
        ctl->ondraw(ctl);
      }
      break;
    case ATEV_MOUSEUP:
      {
        d->pushed=0;
        if (aw_touchoncontrol(ctl,atev->x,atev->y))
          msg=aw_msg(d->touchmsg,1,0,0);
        else
          msg=aw_msg(0,1,0,0);
        ctl->ondraw(ctl);
      }
      break;
    case ATEV_SELECT:
      {
        if (atev->d){
          vibrate(30);
          d->pushed=1;
          msg=aw_msg(0,1,0,0);
          ctl->ondraw(ctl);
        }
        else{
          d->pushed=0;
          msg=aw_msg(d->touchmsg,1,0,0);
          ctl->ondraw(ctl);
        }
      }
      break;
  }
  return msg;
}
void acvirkeys_ondraw(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  CANVAS *  pc = &ctl->win->c;

  ag_draw(pc,&d->control,ctl->x,ctl->y);
}
void acvirkeys_ondestroy(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  ag_ccanvas(&d->control);
  free(ctl->d);
}
byte acvirkeys_onfocus(void * x){
  return 1;
}
void acvirkeys_onblur(void * x){

}
ACONTROLP acvirkeys(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  byte home,
  byte touchmsg
){
  CANVAS *  c = win->bg;
  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*16) w=agdp()*16;
  PNGCANVAS ap;
  int pad = agdp()*4;
  int imgE = 0;
  int imgW = 0;
  int imgH = 0;
  int imgX = 0;
  int imgY = 0;

  //-- Initializing Button Data
  ACBUTTONDP d = (ACBUTTONDP) malloc(sizeof(ACBUTTOND));
  memset(d,0,sizeof(ACBUTTOND));

  //-- Save Touch Message & Set Stats
  d->touchmsg  = touchmsg;
  d->focused   = 0;
  d->pushed    = 0;

  //-- Initializing Canvas
  ag_canvas(&d->control,w,h);

  //-- Draw Reset Control
  dword hl1 = ag_calchighlight(acfg()->controlbg,acfg()->controlbg_g);
  ag_draw_ex(&d->control,&win->c,0,0,x,y,w,h);

  if (home) {
	  if (apng_load(&ap, VIRTULAKEYS_HOME_ICON)){
	  	  imgE = 1;
	  	  imgW = ap.w;
	  	  imgH = ap.h;
	  }
	  if(imgE) {
		  apng_stretch(&d->control, &ap, 0, 0, w, h, 0, 0, imgW, imgH);
	      apng_close(&ap);
	  } else
		  printf("img.virtualkeys.home draw failed\n");
  } else {
	  if (apng_load(&ap, VIRTULAKEYS_BACK_ICON)){
	  	  imgE = 1;
	  	  imgW = ap.w;
	  	  imgH = ap.h;
	  }
	  if(imgE) {
		  apng_stretch(&d->control, &ap, 0, 0, w, h, 0, 0, imgW, imgH);
	      apng_close(&ap);
	  } else
		  printf("img.virtualkeys.home draw failed\n");
  }

  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy= &acvirkeys_ondestroy;
  ctl->oninput  = &acvirkeys_oninput;
  ctl->ondraw   = &acvirkeys_ondraw;
  ctl->onblur   = &acvirkeys_onblur;
  ctl->onfocus  = &acvirkeys_onfocus;
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  ctl->d        = (void *) d;
  aw_add(win,ctl);
  return ctl;
}
