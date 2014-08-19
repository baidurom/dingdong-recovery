/*
 * Copyright (C) 2011 Ahmad Amarullah
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
 * DD UI: Menubox List Window Control
 *
 */
#include "../dd_inter.h"

/***************************[ MENU BOX ]**************************/
typedef struct{
  char title[64];
  char desc[128];
  PNGCANVAS * img;
  int  id;
  int  w;
  int  h;
  int  x;
  int  y;

  /* Title & Desc Size/Pos */
  int  th;
  int  dh;
  int  ty;
  int  dy;

  int  px;
  int  py;
} ACMAINMENUI, * ACMAINMENUIP;

typedef struct{
  byte      acheck_signature;
  CANVAS    client;
  CANVAS    control;
  CANVAS    control_focused;
  AKINETIC  akin;
  int       scrollY;
  int       maxScrollY;
  int       prevTouchY;
  int       invalidDrawItem;

  /* Client Size */
  int clientWidth;
  int clientHeigth;
  int clientTextW;
  int clientXA;
  int clientXB;
  int clientTextXA;
  int clientTextXB;
  int clientImgXA;
  int clientImgXB;
  int clientInv;
  int nextX;
  int nextY;

  /* Items */
  ACMAINMENUIP * items;
  int       itemn;
  int       touchedItem;
  int       focusedItem;
  int       draweditemn;
  int       selectedIndex;
  byte      touchmsg;
  /* Focus */
  byte      focused;
} ACMAINMENUD, * ACMAINMENUDP;

static int minpad       = 0;
static int pad          = 0;
static int imgMax       = 0;
static byte listFixed	= 0;

static void control_item_dump(ACMAINMENUD * d)
{
    dd_debug("**************ACMAINMENUD***************\n");
    dd_debug("itemn:%d,touchedItem:%d,focusedItem:%d,\n", d->itemn, d->touchedItem, d->focusedItem);
    dd_debug("drawedItemn:%d,selectIndex:%d,touchmsg:%x\n,", d->draweditemn, d->selectedIndex, d->touchmsg);
    dd_debug("******************END*******************\n");
}

void acblockmenu_ondestroy(void * x){
  ACONTROLP ctl= (ACONTROLP) x;
  ACMAINMENUDP d  = (ACMAINMENUDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_focused);
  if (d->itemn>0){
    int i;
    for (i=0;i<d->itemn;i++){
      if (d->items[i]->img!=NULL){
        apng_close(d->items[i]->img);
        free(d->items[i]->img);
        d->items[i]->img=NULL;
      }
      free(d->items[i]);
    }
    free(d->items);
    ag_ccanvas(&d->client);
  }
  free(ctl->d);
}
void acblockmenu_redrawitem(ACONTROLP ctl, int index){
	ACMAINMENUDP d = (ACMAINMENUDP) ctl->d;
	if (d->acheck_signature != 186) return;   //-- Not Valid Signature
	if ((index>=d->itemn)||(index<0)) return; //-- Not Valid Index

	ACMAINMENUIP p = d->items[index];
	CANVAS *  c = &d->client;

	//-- Cleanup Background
	ag_rect_block(c, p->x, p->y, p->w, p->h, acfg()->textbg);
	color txtcolor = acfg()->textfg;
	color graycolor= acfg()->textfg_gray;

	if (index==d->touchedItem){
		if (!atheme_draw("img.selection.push", c,p->x,p->y,p->w,p->h)){
			printf("load img.selection.push failed\n");
		}
	}
	else if ((index==d->focusedItem)&&(d->focused)){
	    if (!atheme_draw("img.selection", c,p->x,p->y,p->w,p->h)){
	    	printf("load img.selection failed\n");
	    }
	}

	//-- Not Last... Add Lateral & Vertical separation line
	color sepcl = ag_calculatealpha(acfg()->textbg,acfg()->textfg_gray,60);
	int lx = 0, ly = 0, lw = 0, lh = 0;
	int mod = d->itemn % 2;
	if ((!mod && (index < d->itemn-2)) || (mod && (index != d->itemn-1))) {
		ly = p->y + p->h + 1;
		lw = p->w + 1;
		lh = 2;
		if (index%2 == 0)  	  lx = p->x;
		else  				  lx = p->x - 1;
		ag_rect_block(c,lx,ly,lw,lh,sepcl);
	}

	// Draw Vertical separation line
	if (index%2 == 0) {
	  	lx = p->x + p->w;
	  	lw = 2;
  	  	ly = p->y - 1;
  	  	lh = p->h + 2;
	  	ag_rect_block(c,lx,ly,lw,lh,sepcl);
	}

	//-- Now Draw The Checkbox
	if (p->img!=NULL){
	    apng_draw_ex(c,p->img,p->px,p->py,0,0,p->img->w,p->img->h);
	}

	int txtX = 0;
	int txtY = p->ty;
	int dtxtX = 0;
	int dtxtY = p->dy;
	if (index%2 == 0)	  txtX = dtxtX = d->clientTextXA;
	else 				  txtX = dtxtX = d->clientTextXB;

	  //-- Now Draw The Text
	ag_text(c,d->clientTextW,txtX,txtY,p->title,txtcolor,1);
	ag_text(c,d->clientTextW,dtxtX,dtxtY,p->desc,graycolor,0);
}

void acblockmenu_redraw(ACONTROLP ctl){
  ACMAINMENUDP d = (ACMAINMENUDP) ctl->d;
  if (d->acheck_signature != 186) return; //-- Not Valid Signature
  if ((d->itemn>0)&&(d->draweditemn<d->itemn)) {
	int cw = agw();
	int ch = (d->clientHeigth+2)*5;
    ag_ccanvas(&d->client);
    ag_canvas(&d->client, cw, ch);
    ag_rect(&d->client,0,0,cw, ch,acfg()->textbg);

    //-- Set Values
    d->scrollY     = 0;
    d->maxScrollY  = 0;
    //d->maxScrollY  = d->nextY-(ctl->h-(agdp()*max(acfg()->roundsz,4)));
    if (d->maxScrollY<0) d->maxScrollY=0;

    //-- Draw Items
    int i;
    for (i=0;i<d->itemn;i++){
      acblockmenu_redrawitem(ctl,i);
    }
    d->draweditemn=d->itemn;
  }

}
int acblockmenu_getselectedindex(ACONTROLP ctl){
  ACMAINMENUDP d = (ACMAINMENUDP) ctl->d;
  if (d->acheck_signature != 186) return -1; //-- Not Valid Signature
  return d->selectedIndex < 0 ?0:d->selectedIndex;
}
//-- Add Item Into Control
byte acblockmenu_add(ACONTROLP ctl,char * title, char * desc, char * img){
  ACMAINMENUDP d = (ACMAINMENUDP) ctl->d;
  if (d->acheck_signature != 186) return 0; //-- Not Valid Signature
  //-- Allocating Memory For Item Data
  ACMAINMENUIP newip = (ACMAINMENUIP) malloc(sizeof(ACMAINMENUI));
  snprintf(newip->title,64,"%s",title);
  snprintf(newip->desc,128,"%s",desc);

  //-- Load Image
  newip->img      = (PNGCANVAS *) malloc(sizeof(PNGCANVAS));
  memset(newip->img,0,sizeof(PNGCANVAS));
  if (!apng_load(newip->img,img)){
    free(newip->img);
    newip->img=NULL;
  }

  newip->id 	  = d->itemn;
  newip->th       = ag_txtheight(d->clientTextW,newip->title,1) / 2;
  newip->dh       = ag_txtheight(d->clientTextW,newip->desc,0) / 2;
  newip->px		  = d->nextX + (d->clientWidth - newip->img->w)/2;
  newip->py		  = d->nextY + (d->clientHeigth - newip->img->h - newip->th - newip->dh - agdp()*6)/2 + 1;
  newip->ty       = newip->py + newip->img->h + agdp()*4;
  newip->dy       = newip->ty + newip->th + agdp()*2;
  newip->h  	  = d->clientHeigth;
  newip->w  	  = d->clientWidth;
  newip->x  	  = d->nextX;
  newip->y        = d->nextY;

  if (newip->x < newip->w) {
	  d->nextX    = d->clientXB;
  }
  else {
	  d->nextX    = d->clientXA;
	  d->nextY   += (newip->h + 3);
  }

  if (d->itemn>0){
    int i;
    ACMAINMENUIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(ACMAINMENUIP)*(d->itemn+1) );
    for (i=0;i<d->itemn;i++)
      d->items[i]=tmpitms[i];
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else{
    d->items    = malloc(sizeof(ACMAINMENUIP));
    d->items[0] = newip;
  }
  d->itemn++;
  return 1;
}

void acblockmenu_ondraw(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACMAINMENUDP   d  = (ACMAINMENUDP) ctl->d;
  CANVAS *    pc = &ctl->win->c;
  acblockmenu_redraw(ctl);
  if (d->invalidDrawItem!=-1){
    d->touchedItem = d->invalidDrawItem;
    acblockmenu_redrawitem(ctl,d->invalidDrawItem);
    d->invalidDrawItem=-1;
  }

  //-- Init Device Pixel Size
  int minpadding = max(acfg()->roundsz,4);
  int agdp3 = (agdp()*minpadding);
  int agdp6 = (agdp()*(minpadding*2));
  int agdpX = agdp6;

  if (d->focused){
    ag_draw(pc,&d->control_focused,ctl->x,ctl->y);
    ag_draw_ex(pc,&d->client,ctl->x,ctl->y,0,d->scrollY+agdp(),ctl->w,ctl->h);
  }
  else{
    ag_draw(pc,&d->control,ctl->x,ctl->y);
    ag_draw_ex(pc,&d->client,ctl->x,ctl->y,0,d->scrollY+1,ctl->w,ctl->h);
  }

  if (d->maxScrollY>0){
    //-- Glow
    int i;
    byte isST=(d->scrollY>0)?1:0;
    byte isSB=(d->scrollY<d->maxScrollY)?1:0;
    int add_t_y = 1;
    if (d->focused)
      add_t_y = agdp();
    for (i=0;i<agdpX;i++){
      byte alph = 255-round((((float) (i+1))/ ((float) agdpX))*230);
      if (isST)
        ag_rectopa(pc,ctl->x+agdp3,ctl->y+i+add_t_y,ctl->w-agdpX,1,acfg()->textbg,alph);
      if (isSB)
        ag_rectopa(pc,ctl->x+agdp3,((ctl->y+ctl->h)-(add_t_y))-(i+1),ctl->w-agdpX,1,acfg()->textbg,alph);
    }

    //-- Scrollbar
    int newh = ctl->h - agdp6;
    float scrdif    = ((float) newh) / ((float) d->client.h);
    int  scrollbarH = round(scrdif * newh);
    int  scrollbarY = round(scrdif * d->scrollY) + agdp3;
    if (d->scrollY<0){
      scrollbarY = agdp3;
      int alp = (1.0 - (((float) abs(d->scrollY)) / (((float) ctl->h)/4))) * 255;
      if (alp<0) alp = 0;
      ag_rectopa(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar, alp);
    }
    else if (d->scrollY>d->maxScrollY){
      scrollbarY = round(scrdif * d->maxScrollY) + agdp3;
      int alp = (1.0 - (((float) abs(d->scrollY-d->maxScrollY)) / (((float) ctl->h)/4))) * 255;
      if (alp<0) alp = 0;
      ag_rectopa(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar, alp);
    }
    else{
      ag_rect(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar);
    }
  }
}
dword acblockmenu_oninput(void * x,int action,ATEV * atev){
  ACONTROLP ctl= (ACONTROLP) x;
  ACMAINMENUDP d  = (ACMAINMENUDP) ctl->d;
  dword msg = 0;

  switch (action){
    case ATEV_MOUSEDN:
      {
        d->prevTouchY  = atev->y;
        akinetic_downhandler(&d->akin,atev->y);

        int touchposx = atev->x - ctl->x;
        int touchposy = atev->y - ctl->y + d->scrollY;
        int i;
        for (i=0;i<d->itemn;i++){
          if (((touchposx>=d->items[i]->x)&&(touchposx<d->items[i]->x+d->items[i]->w))
           && ((touchposy>=d->items[i]->y)&&(touchposy<d->items[i]->y+d->items[i]->h))){
            ac_regpushwait(
              ctl,&d->prevTouchY,&d->invalidDrawItem,i
            );
            break;
          }
        }
      }
      break;
    case ATEV_MOUSEUP:
      {
        byte retmsgx = 0;
        if ((d->prevTouchY!=-50)&&(abs(d->prevTouchY-atev->y)<agdp()*5)){
          d->prevTouchY=-50;
          int touchposx = atev->x - ctl->x;
          int touchposy = atev->y - ctl->y + d->scrollY;
          int i;
#ifdef DEBUG
          control_item_dump(d);
#endif
          for (i=0;i<d->itemn;i++){
        	  if (((touchposx>=d->items[i]->x)&&(touchposx<d->items[i]->x+d->items[i]->w))
        	             && ((touchposy>=d->items[i]->y)&&(touchposy<d->items[i]->y+d->items[i]->h))){
                  if ((d->touchedItem != -1)&&(d->touchedItem!=i)){
                      int tmptouch=d->touchedItem;
                      d->touchedItem = -1;
                      acblockmenu_redrawitem(ctl,tmptouch);
                    }
                    if ((d->selectedIndex != -1)&&(d->selectedIndex!=i)){
                      int tmpsidx=d->selectedIndex;
                      d->selectedIndex = -1;
                      acblockmenu_redrawitem(ctl,tmpsidx);
                    }

                    int prevfocus = d->focusedItem;
                    d->focusedItem= i;
                    d->touchedItem   = i;
                    d->selectedIndex = i;
                    if ((prevfocus!=-1)&&(prevfocus!=i)){
                  	  acblockmenu_redrawitem(ctl,prevfocus);
                    }

                    acblockmenu_redrawitem(ctl,i);
                    ctl->ondraw(ctl);
                    aw_draw(ctl->win);
                    vibrate(30);
                    retmsgx = d->touchmsg;
                    msg=aw_msg(retmsgx,1,0,0);
                    break;
            }
          }
          if ((d->scrollY<0)||(d->scrollY>d->maxScrollY)){
            ac_regbounce(ctl,&d->scrollY,d->maxScrollY);
          }
        }
        else{
          if(!listFixed) {
              if (akinetic_uphandler(&d->akin,atev->y)){
                ac_regfling(ctl,&d->akin,&d->scrollY,d->maxScrollY);
              }
              else if ((d->scrollY<0)||(d->scrollY>d->maxScrollY)){
                ac_regbounce(ctl,&d->scrollY,d->maxScrollY);
              }
          }
        }
#if 1
        if (d->touchedItem != -1){
          usleep(5);
          int tmptouch=d->touchedItem;
          d->touchedItem = -1;
          acblockmenu_redrawitem(ctl,tmptouch);
          ctl->ondraw(ctl);
          msg=aw_msg(retmsgx,1,0,0);
        }
#endif
      }
      break;

    case ATEV_MOUSEMV:
      {
    	  if(!listFixed){
    	        byte allowscroll=1;
    	        if (atev->y!=0){
    	          if (d->prevTouchY!=-50){
    	            if (abs(d->prevTouchY-atev->y)>=agdp()*5){
    	              d->prevTouchY=-50;
    	              if (d->touchedItem != -1){
    	                int tmptouch=d->touchedItem;
    	                d->touchedItem = -1;
    	                acblockmenu_redrawitem(ctl,tmptouch);
    	                ctl->ondraw(ctl);
    	                aw_draw(ctl->win);
    	              }
    	            }
    	            else
    	              allowscroll=0;
    	          }
    	          if (allowscroll){
    	            int mv = akinetic_movehandler(&d->akin,atev->y);
    	            if (mv!=0){
    	              if ((d->scrollY<0)&&(mv<0)){
    	                float dumpsz = 0.6-(0.6*(((float) abs(d->scrollY))/(ctl->h/4)));
    	                d->scrollY+=floor(mv*dumpsz);
    	              }
    	              else if ((d->scrollY>d->maxScrollY)&&(mv>0)){
    	                float dumpsz = 0.6-(0.6*(((float) abs(d->scrollY-d->maxScrollY))/(ctl->h/4)));
    	                d->scrollY+=floor(mv*dumpsz);
    	              }
    	              else
    	                d->scrollY+=mv;

    	              if (d->scrollY<0-(ctl->h/4)) d->scrollY=0-(ctl->h/4);
    	              if (d->scrollY>d->maxScrollY+(ctl->h/4)) d->scrollY=d->maxScrollY+(ctl->h/4);
    	              msg=aw_msg(0,1,0,0);
    	              ctl->ondraw(ctl);
    	            }
    	          }
    	        }
    	  }
      }
      break;

      case ATEV_SELECT:
      {
        if ((d->focusedItem>-1)&&(d->draweditemn>0)){
          if (atev->d){
            if ((d->touchedItem != -1)&&(d->touchedItem!=d->focusedItem)){
              int tmptouch=d->touchedItem;
              d->touchedItem = -1;
              acblockmenu_redrawitem(ctl,tmptouch);
            }
            vibrate(30);
            d->touchedItem=d->focusedItem;
            acblockmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(0,1,0,0);
          }
          else{
            if ((d->touchedItem != -1)&&(d->touchedItem!=d->focusedItem)){
              int tmptouch=d->touchedItem;
              d->touchedItem = -1;
              acblockmenu_redrawitem(ctl,tmptouch);
            }
            if ((d->selectedIndex != -1)&&(d->selectedIndex!=d->focusedItem)){
              int tmpsidx=d->selectedIndex;
              d->selectedIndex = -1;
              acblockmenu_redrawitem(ctl,tmpsidx);
            }
            d->selectedIndex = d->focusedItem;
            d->touchedItem=-1;
            acblockmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(d->touchmsg,1,0,0);
          }
        }
      }
      break;
      case ATEV_DOWN:
        {
          if ((d->focusedItem<d->itemn-1)&&(d->draweditemn>0)){
            int prevfocus = d->focusedItem;
            d->focusedItem++;
            acblockmenu_redrawitem(ctl,prevfocus);
            acblockmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(0,1,1,0);

            int reqY = d->items[d->focusedItem]->y - round((ctl->h/2) - (d->items[d->focusedItem]->h/2));
            ac_regscrollto(
              ctl,
              &d->scrollY,
              d->maxScrollY,
              reqY,
              &d->focusedItem,
              d->focusedItem
            );
          }
        }
      break;
      case ATEV_UP:
        {
          if ((d->focusedItem>0)&&(d->draweditemn>0)){
            int prevfocus = d->focusedItem;
            d->focusedItem--;
            acblockmenu_redrawitem(ctl,prevfocus);
            acblockmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(0,1,1,0);

            int reqY = d->items[d->focusedItem]->y - round((ctl->h/2) - (d->items[d->focusedItem]->h/2));
            ac_regscrollto(
              ctl,
              &d->scrollY,
              d->maxScrollY,
              reqY,
              &d->focusedItem,
              d->focusedItem
            );
          }
        }
      break;
  }
  return msg;
}
byte acblockmenu_onfocus(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACMAINMENUDP   d  = (ACMAINMENUDP) ctl->d;
  d->focused=1;

  if ((d->focusedItem==-1)&&(d->itemn>0)){
    //d->focusedItem=0;
  }
  if ((d->focusedItem!=-1)&&(d->draweditemn>0)){
	  acblockmenu_redrawitem(ctl,d->focusedItem);
  }
  ctl->ondraw(ctl);
  return 1;
}
void acblockmenu_onblur(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACMAINMENUDP   d  = (ACMAINMENUDP) ctl->d;
  d->focused=0;
  if ((d->focusedItem!=-1)&&(d->draweditemn>0)){
	  acblockmenu_redrawitem(ctl,d->focusedItem);
  }
  ctl->ondraw(ctl);
}
ACONTROLP acblockmenu(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  byte touchmsg,
  byte fixed
){
  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*20) w=agdp()*20;
  listFixed = fixed;

  //-- Initializing Text Data
  ACMAINMENUDP d        = (ACMAINMENUDP) malloc(sizeof(ACMAINMENUD));
  memset(d,0,sizeof(ACMAINMENUD));

  //-- Set Signature
  d->acheck_signature = 186;

  //-- Initializing Canvas
  ag_canvas(&d->control,w,h);
  ag_canvas(&d->control_focused,w,h);

  minpad = max(acfg()->roundsz,4);
  pad    = agdp() * 4;
  imgMax = MAIN_IMG_MAX;

  //-- Initializing Client Size
  d->clientInv    = minpad;
  d->clientWidth  = w/2-1;
  d->clientHeigth = h/3-2;
  d->clientXA  = 0;
  d->clientXB  = w/2 + 1;
  d->clientImgXA  = d->clientWidth/2 + d->clientXA;
  d->clientImgXB  = d->clientWidth/2 + d->clientXB;
  d->clientTextW  = d->clientWidth;
  d->clientTextXA = d->clientXA;
  d->clientTextXB = d->clientXB;
  d->client.data=NULL;
  //-- Draw Control
  ag_draw_ex(&d->control,&win->c,0,0,x,y,w,h);
  //-- Draw Focused Control
  ag_draw_ex(&d->control_focused,&win->c,0,0,x,y,w,h);
  //-- Set Scroll Value
  d->scrollY     = 0;
  d->maxScrollY  = 0;
  d->prevTouchY  =-50;
  d->invalidDrawItem = -1;
  //-- Set Data Values
  d->items       = NULL;
  d->itemn       = 0;
  d->touchedItem = -1;
  d->focusedItem = -1;
 // d->nextX       = minpad;
 // d->nextY       = minpad;
  d->nextX       = 0;
  d->nextY       = 1;
  d->draweditemn = 0;
  d->selectedIndex = -1;
  d->touchmsg    = touchmsg;

  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy= &acblockmenu_ondestroy;
  ctl->oninput  = &acblockmenu_oninput;
  ctl->ondraw   = &acblockmenu_ondraw;
  ctl->onblur   = &acblockmenu_onblur;
  ctl->onfocus  = &acblockmenu_onfocus;
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
