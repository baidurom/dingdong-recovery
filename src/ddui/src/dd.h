#ifndef DD_H_
#define DD_H_
#include <dirent.h>
//add in ui operation
//
//
#include "utils.h"
typedef int STATUS;
#define RET_OK 0
#define RET_FAIL -1
#define RET_INVALID_ARG -1
#define RET_YES 1
#define RET_NO 0
#define RET_NULL 0

#define BATTERY_CAPACITY_PATH 			  "/sys/class/power_supply/batterys/capacity"
#define BATTERY_CAPACITY_PATH_1 	      "/sys/class/power_supply/battery/capacity"
#define DD_LOG_FILE 					  "/tmp/recovery.log"
#define RECOVERY_PATH					  "/sdcard/dingdong/recovery"
#define RECOVERY_PATH_INTER				  "/sdcard2/dingdong/recovery"
#define INSTALL_CONF_FILE				  "install.prop"
#define BACKUP_PATH_CONFIG				  "/tmp/backup.config"
#define VERSION_TMP_PATH				  "/tmp/.ddrec.conf"
#define VERSION_DATA_PATH				  "/data/local/tmp/.ddrec.conf"

//MAX_MENU_
#define ITEM_COUNT 999
#define MENU_BACK ITEM_COUNT
#define MENU_MAIN MENU_BACK + 1
#define MENU_QUIT MENU_MAIN + 1

#define ANIMATION_TYPE_UPDATE  1
#define ANIMATION_TYPE_BACKUP  2
#define ANIMATION_TYPE_RESTORE 3

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


#define MENU_LEN 64 //international, ~ or direct print string;
typedef struct _menuItem {
    char name[MENU_LEN];
    char title_name[MENU_LEN];
    char icon[MENU_LEN];
    char desc[MENU_LEN];
    int  result;

    void *data;

    struct _menuItem *child;
    struct _menuItem *nextSilbing;
    struct _menuItem *parent;
    //

    //method
    STATUS (*show)(struct _menuItem *p);

    int (*get_child_count)(struct _menuItem *p);
    struct _menuItem * (*get_child_by_index)(struct _menuItem *p, int index);

}menuItem, *pmenuItem;
typedef STATUS (*menuItemFunction)(struct _menuItem *p);
//dd_stack.c

#ifdef DD_STACK
#define MAX_STACK_SIZE 64
typedef pmenuItem dataType;
typedef struct _ddStack{
    dataType data[MAX_STACK_SIZE];
    int alloc;
    int top;
}ddStack, pddStack;
STATUS ddStack_init();
//judge empty
STATUS ddStack_isEmpty();
STATUS ddStack_isFull();
STATUS ddStack_push(dataType d);
dataType ddStack_pop();
dataType ddStack_getTop();
STATUS ddStack_disp();
#endif
//back_ui.c
//
struct _menuItem * back_ui_init();
//dd_ui.c

void dd_redraw();
void dd_redraw_otherbg();
void dd_redraw_bg();
void dd_redraw_win();
void dd_redraw_bg_ex(CANVAS* bg);
int dd_setbg(char * titlev);
char * dd_readfromfs(char * name);
void dd_writetofs(char * name, char * value);
char * dd_readfromtmp(char * name);
void dd_writetotmp(char * name, char * value);
char * dd_readfromzip(char * name);
char * dd_parseprop(char * filename,char *key);
char * dd_parsepropzip(char * filename,char *key);
char * dd_getvar(char * name);
void dd_setvar(char * name, char * value);
void dd_appendvar(char * name, char * value);
void dd_delvar(char * name);
void dd_prependvar(char * name, char * value);
STATUS dd_font(char *ttf_type, char *ttf_file, char *ttf_size);
char * dd_getprop(char *file, char *key);
char * dd_gettmpprop(char *file, char *key);
char * dd_resread(char *file);
STATUS dd_pleasewait(char *message);
STATUS dd_setcolor(char *item, char *val);
//* 
//* ini_get
//*
char * dd_ini_get(char *item);
//* 
//* ini_set
//*
STATUS dd_ini_set(char *item, char *value);
//* 
//* viewbox
//*
STATUS dd_viewbox(int argc, char *format, ...);
//* 
//* textbox, agreebox
//*
STATUS dd_textbox(char *title, char *desc, char *icon, char *message); 
//* 
//* checkbox
//*
STATUS dd_checkbox(int argc, char *format, ...);
STATUS dd_checkbox_install(int argc, char *format, ...);

//* 
//* selectbox
//*
STATUS dd_selectbox(int argc, char *format, ...);

STATUS dd_loadwelcome();
//* menubox
//*
STATUS dd_mainmenu(char *title_name, char **item, char **item_icon, char **item_icon_append, int item_cnt, int icon_enable);
STATUS dd_mainmenu_block(char *title_name, char **item, char **item_icon, char **item_icon_append, int item_cnt, int icon_enable, char **desc);
STATUS dd_menubox(char *title_name, char **item, int item_cnt);
STATUS dd_sdmenu(char *title_name, char **item, char **item_sub, int item_cnt); 
STATUS dd_busy_process(int isauto);
//* 
//* alert
//*
STATUS dd_alert(int argc, char *format, ...);
//* 
//* confirm
//*
STATUS dd_confirm(int argc, char *format, ...);
//* 
//* textdialog
//*
STATUS dd_textdialog(int argc, char *format, ...);
//* 
//* loadlang
//*
STATUS dd_loadlang(char * name);
//* 
//* lang
//*
char * dd_lang(char *name);
STATUS dd_install(int echo);
STATUS dd_progress(char * icon, char *txt, char *fin_txt, int echo);
int dd_gen_verfile();
int dd_ui_init();
int dd_ui_config(const char * file);
STATUS dd_ui_start();
STATUS dd_ui_end();
int dd_lang_init();


#define assert_ui_if_fail(p) if(!(p)) { \
    dd_alert(2, "<~alert.result>", "<~alert.desc>");}

//common_ui.c 
int common_get_child_count(menuItem *p);
struct _menuItem * common_get_child_by_index(struct _menuItem *p, int index);
STATUS common_ui_show(menuItem *p);
STATUS common_menu_show(menuItem *p);
STATUS menu_default_init(struct _menuItem *p);
struct _menuItem *common_ui_init();
STATUS menuItem_set_name(struct _menuItem *p, const char* name);
STATUS menuItem_set_icon(struct _menuItem *p, const char* name);
STATUS menuItem_set_title(struct _menuItem *p, const char* name);
STATUS menuItem_set_desc(struct _menuItem *p, const char* name);
STATUS menuItem_set_result(struct _menuItem *p, const int result);
STATUS menuItem_set_show(struct _menuItem *p, menuItemFunction fun);

struct _menuItem* sd_ui_init();
struct _menuItem* power_ui_init();
struct _menuItem* reboot_ui_init();
struct _menuItem* mount_ui_init();
struct _menuItem* power_ui_init();
struct _menuItem* back_ui_init();
struct _menuItem* wipe_ui_init();
struct _menuItem* backup_ui_init();
struct _menuItem* tool_ui_init();
struct _menuItem* info_ui_init();

extern struct _menuItem* g_main_menu;
extern struct _menuItem* g_root_menu;
STATUS main_ui_init();
STATUS main_ui_show();
STATUS main_ui_release();
//for re draw screen
STATUS dd_set_isbgredraw(int value);

typedef struct _ddProgress_unit {
	char success_text[PATH_MAX];
	char fail_text[PATH_MAX];
	char *path;
	char *install_file;
	int wipe_cache;
    int backup_restore_type;
}ddProgress_unit, *pddProgress_unit;
typedef void (*ddProgress_fun)();
typedef struct _ddProgress {
    ddProgress_fun start_pfun;
    struct _ddProgress_unit *args;
}ddProgress, *pddProgress;
STATUS ddProgress_init(ddProgress_fun start_fun, struct _ddProgress_unit *ddProgress_args);
void ddProgress_show_progress(float portion, int seconds);
void ddProgress_set_progress(float fraction);
void ddProgress_set_text(char *str);
int  ddProgress_set_pause(int status);
void ddProgress_set_info(char* file_name);
void ddProgress_reset_progress();
int ddProgress_set_op_type(int type);

STATUS exec_backup_auto(char* path);
STATUS exec_restore_auto(char* path);

//menuNode operation
struct _menuItem * menuNode_init(struct _menuItem *node);
STATUS menuNode_add(struct _menuItem *parent, struct _menuItem *child);
STATUS menuNode_delete(struct _menuItem *parent, struct _menuItem *child);

#endif // __DD_H__
