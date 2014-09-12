#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>

#define byte              unsigned char
#define dword             unsigned int
#define word              unsigned short
#define color             unsigned short
typedef int u32;

//##                                                   ##//
//##               LIST OF DEFINITIONS                 ##//
//##                                                   ##//
//#######################################################//
//
// Common Data Type
//

//
// DD Main Configurations

#define DD_AUTHOR        		"Dingdong"

#ifdef DD_USE_AUTHOR_INNER
#undef DD_USE_AUTHOR_INNER
#endif

#define DD_NAME        "叮咚Recovery"
//rec_version
#define DD_VERSION     "1.1.0"
//rec_iteration
#define DD_ITERATION   "0"
//rec date
#define DD_BUILD       "2014-01-01"

#define DD_BUILD_CN    "Baiyi Mobile"
#define DD_BUILD_L     "Baiyi Mobile"
#define DD_BUILD_A     "Baiyi Mobile"
#define DD_BUILD_URL   "http://www.baidu.com"
#define DD_COPY        "(c) 2014 by baidu developers"

//-- Temporary Dir - Move from /tmp/dingdong-data to /tmp/dingdong symlink to /tmp/dingdong-data for backward compatibility
#define DD_SYSTMP      "/tmp"
//#define DD_SYSTMP      "/data"
#define DD_TMP         "/tmp/dingdong"
#define DD_TMP_S       "/tmp/dingdong-data"

#define DD_DIR         "/res"
#define DD_FRAMEBUFFER "/dev/graphics/fb0"

#define DD_THEME_CNT   		27
#define DD_ANIMATION_FRAM	4
#define MAIN_IMG_MAX		128

#define DIR_ICON  						  "@list.dir"
#define FILE_ICON 						  "@list.file"
#define BACK_ICON						  "@list.back"
#define ZIP_ICON						  "@list.zip"
#define BANNER_ICON						  "@banner.main.icon"
#define SUBBANNER_ICON					  "@banner.sub.icon"
#define PIC_WAIT 						  "@common.wait"
#define PIC_WELCOME 					  "@common.welcome"
#define ANIMATION_FLASH 				  "@animation/icon_installing_overlay"
#define ICON_ENABLE  					  "@switch.enable"
#define ICON_DISABLE 					  "@switch.disable"
#define TITLEBAR_LOGO   				  "@titlebar.logo"
#define BATTERY_LOW     				  "@titlebar.battery.low"
#define BATTERY_NORMAL  				  "@titlebar.battery.normal"
#define BATTERY_VERYLOW     			  "@titlebar.battery.verylow"
#define BATTERY_FULL  					  "@titlebar.battery.full"
#define MOUNT_ICON  					  "@main.mount"
#define BACKUP_ICON  					  "@main.backup"
#define SD_ICON  						  "@main.sd"
#define ADVANCED_ICON  					  "@main.advanced"
#define POWER_ICON  					  "@main.power"
#define WIPE_ICON  					  	  "@main.wipe"
#define ITEM_ICON  					  	  "@menu.item"
#define VIRTULAKEYS_BACK_ICON  			  "@virtualkeys.back"
#define VIRTULAKEYS_HOME_ICON  			  "@virtualkeys.home"

// DD Canvas Structure
//
typedef struct{
	int     w;       // Width
	int     h;       // Height
	int     sz;      // Data Size
	color * data;    // Data 
} CANVAS;

//
// DD Assosiative Array Structure
//
typedef struct{
  char * key;
  char * val;
} AARRAY_ITEM, * AARRAY_ITEMP;

typedef struct{
  int length;
  AARRAY_ITEMP items;
} AARRAY, * AARRAYP;

AARRAYP   aarray_create();
char *    aarray_get(AARRAYP a, char * key);
byte      aarray_set(AARRAYP a, char * key, char * val);
byte      aarray_del(AARRAYP a, char * key);
byte      aarray_free(AARRAYP a);

//
// DD PNG Canvas Structure
//
typedef struct {
  int     w;       // Width
  int     h;       // Height
  int     s;       // Buffer Size
  byte    c;       // Channels
  byte *  r;       // Red Channel
  byte *  g;       // Green Channel
  byte *  b;       // Blue Channel
  byte *  a;       // Alpha Channel
} PNGCANVAS, * PNGCANVASP;


//
// DD PNG Font Canvas Structure
//
typedef struct {
  byte    loaded;    // Font is Loaded 
  int     fx[96];    // Font X Positions
  byte    fw[96];    // Font Width
  byte    fh;        // Font Height
  int     w;         // Png Width
  int     h;         // Png Height
  int     s;         // Buffer Size
  byte    c;         // Channels
  byte *  d;         // Fonts Alpha Channel
} PNGFONTS;

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef DEBUG
#define dd_debug(fmt...) printf("%s(line: %d):", __FUNCTION__, __LINE__);printf(fmt)
#else
#define dd_debug(fmt...) do{}while(0)
#endif
#ifndef dd_printf
#define dd_printf printf
#endif
#ifndef dd_error
#define dd_error(fmt...) printf("%s(line: %d):", __FUNCTION__, __LINE__);printf(fmt)
#endif
#ifndef return_val_if_fail
#define return_val_if_fail(p, val) \
	if (!(p)) { \
		dd_printf("%s(line %d): failed, cause %s return %d\n", __FUNCTION__, __LINE__, #p,  val);return val;}
#endif
#ifndef return_null_if_fail
#define return_null_if_fail(p) \
	if (!(p)) { \
		dd_printf("%s(line %d) " #p " \n",__FUNCTION__, __LINE__);return NULL;}
#endif
#ifndef assert_if_fail
#define assert_if_fail(p) \
	if (!(p)) { \
		dd_printf("%s(line %d) " #p " \n", __FUNCTION__, __LINE__);}
#endif
// Customization Functions
//
//
// DD Main Configuration Structure
//
typedef struct  {
  // Colors
  color winbg;                // Window Background
  color winbg_g;              // Window Background Gradient
  color winfg;                // Window Foreground
  color winfg_gray;           // Window Foreground
  color dialogbg;             // Dialog Background
  color dialogbg_g;           // Dialog Background Gradient
  color dialogfg;             // Dialog Foreground
  color textbg;               // Text / List Background
  color textfg;               // Text / List Font Color
  color textfg_gray;          // List Grayed Font Color ( List Description )
  color controlbg;            // Control/Button Background
  color controlbg_g;          // Control/Button Background Gradient
  color controlfg;            // Control/Button Font Color
  color selectbg;             // Selected Item/Control Background
  color selectbg_g;           // Selected Item/Control Background Gradient
  color selectfg;             // Selected Item/Control Font Color
  color titlebg;              // Title Background
  color titlebg_g;            // Title Background Gradient
  color titlefg;              // Title Font Color
  color dlgtitlebg;           // Dialog Title Background
  color dlgtitlebg_g;         // Dialog Title Background Gradient
  color dlgtitlefg;           // Dialog Title Font Color
  color navbg;                // Scrollbar Color
  color navbg_g;              // Navigation Bar Background
  color scrollbar;            // Navigation Bar Background Gradient
  color border;               // Border Color
  color border_g;             // Border Color Gradient
  color progressglow;         // Progress Bar Glow Color
  color battlow;              // Battery Low Color
  color wintitlebg;			  // Windows Title Text Background Color
  color wintitlefg;			  // Windows Title Text Foreground Color
  color winbarbg;			  // Windows Title Background Color
  
  // Property
  byte  roundsz;              // Control Rounded Size
  byte  btnroundsz;           // Button Control Rounded Size
  byte  winroundsz;           // Window Rounded Size
  
  // Transition
  byte  fadeframes;           // Number of Frame used for Fade Transition
  
  // Common Text
  char  text_ok[64];          // OK
  char  text_next[64];        // Next >
  char  text_back[64];        // < Back
  char  text_menumain[64];        // Next >
  char  text_menuback[64];        // < Back
  
  char  text_yes[64];         // Yes
  char  text_no[64];          // No
  char  text_about[64];       // About
  char  text_calibrating[64]; // Calibration Tools
  char  text_quit[64];        // Quit
  char  text_quit_msg[128];   // Quit Message
  char  brightness_path[PATH_MAX]; //brightness_path
  char  battery_path[PATH_MAX]; //battery_path
  char  lun_file[PATH_MAX]; //mass_storage path
  char  reboot_cmd[128];
  char  bootloader_cmd[128];
  
  // ROM Text
  char rec_name[128];          // Recovery Name
  char rec_version[128];       // Recovery Version
  char rec_iteration[128];     // Recovery Iteration
  char rec_author[128];        // Recovery Author
  char rec_device[128];        // Recovery Device Name
  char rec_date[128];          // Recovery Date
  
  // CUSTOM KEY
  int enable_usb;
  int enable_sideload;
  int enable_bootloader;
  int ckey_up;
  int ckey_down;
  int ckey_select;
  int ckey_back;
  int ckey_menu;
  int ckey_home;
  int virkeys_en;
  int root;
  u32 input_filter;
  u32 sd_int;
  u32 sd_ext;
  
  // THEME
  PNGCANVASP theme[DD_THEME_CNT];
  byte       theme_9p[DD_THEME_CNT];
  char themename[64];
} AC_CONFIG;

AC_CONFIG * acfg();           // Get Config Structure
void        acfg_init();      // Set Default Config
void acfg_init_ex(byte themeonly);
#endif
