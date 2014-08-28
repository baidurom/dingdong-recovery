#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"
/*
 *_sd_show_dir show file system on screen
 *return MENU_BACK when pree backmenu
 */
#define SD_MAX_PATH 256
static const char *INSTALL_FILE = "/tmp/last_install";
#define ADB_SIDELOAD_PACKAGE "/tmp/update.zip"

//callback function , success return 0, non-zero fail
int file_install(char *file_name, int file_len, void *data)
{
    return_val_if_fail(file_name != NULL, RET_FAIL);
    return_val_if_fail(strlen(file_name) <= file_len, RET_INVALID_ARG);
    return_val_if_fail(data != NULL, RET_FAIL);

    struct _menuItem *p = (pmenuItem)data;
    if (RET_YES == dd_checkbox_install(
    		16,
    		p->name,
    		"<~install.config.title>",
    		p->icon,
    		INSTALL_CONF_FILE,   //Will be saved in /tmp/dingdong/install.conf
    		"<~install.config.group.setting>",      "",                                      "2",    //-- Group 1. key = "item.1.x"
    		"<~install.config.wipe.before>",	    "<~install.config.wipe.before.desc>",	 "0",    //-- item.1.1 = Un-Selected by default
    		"<~install.config.wipe.after>",	        "<~install.config.wipe.after.desc>",     "0",    //-- item.1.2 = Un-Selected by default
    		"<~install.config.rmzip>",	            "<~install.config.rmzip.desc>",	         "0"     //-- item.1.3 = Un-Selected by default
    		)) {
    	install_start(file_name, 0, INSTALL_FILE, 1, 0);
        return 0;
    }
    else return -1;
}
//callback funtion file filter, if access ,return 0; others return -1
int file_filter(char *file, int file_len)
{
    return_val_if_fail(file != NULL, RET_FAIL);
    int len = strlen(file);
    return_val_if_fail(len <= file_len, RET_INVALID_ARG);
    if (len >= 4 && strncasecmp(file + len -4, ".zip", 4) == 0)
        return 0;
    return -1;
     
}

static STATUS sd_menu_show(menuItem *p)
{
    //ensure_mount sdcard path
    ddOp_send(OP_MOUNT, 1, "/sdcard");
    int result = ddOp_result_get_int();
    int ret = 0;
    if(!result) {
        do {
            ret = file_scan("/sdcard", sizeof("/sdcard"), p->name, strlen(p->name), &file_install, (void *)p, &file_filter);
            if (ret == -1) return MENU_BACK;
            else if(ret == -2) continue;
            else if(ret == MENU_MAIN) return MENU_MAIN;
            else break;
        } while(1);
        return ret;
    }
    dd_alert(4, p->name, "<~sd.mount.error>", NULL, acfg()->text_ok);
    return ret;
}

static STATUS sdint_menu_show(menuItem *p)
{
    //ensure_mount sdcard path
    ddOp_send(OP_MOUNT, 1, "/sdcard2");\
    int result = ddOp_result_get_int();
    int ret = 0;
    if(!result) {

        do {
            ret = file_scan("/sdcard2", sizeof("/sdcard2"), p->name, strlen(p->name), &file_install, (void *)p, &file_filter);
            if (ret == -1) return MENU_BACK;
            else if(ret == -2) continue;
            else if(ret == MENU_MAIN) return MENU_MAIN;
            else break;
        } while(1);
        return ret;
    }
    dd_alert(4, p->name, "<~sd.mount.error>", NULL, acfg()->text_ok);
    return ret;
}

static STATUS sideload_install_show(menuItem *p)
{
	ddOp_send(OP_SIDELOAD, 1, "sideload");
	int result_sideload = ddOp_result_get_int();
	printf("sideload return: %d\n", result_sideload);
	if(result_sideload < 0)
		dd_alert(4, p->name, "<~install.sideload.zip.error>", NULL, acfg()->text_ok);
	else if (result_sideload > 0)
		dd_alert(4, p->name, "<~install.sideload.exit.tip>", NULL, acfg()->text_ok);
	else {
		printf("Begin to install %s\n", ADB_SIDELOAD_PACKAGE);
		file_install(ADB_SIDELOAD_PACKAGE, PATH_MAX, (void *)p);
	}
	remove(ADB_SIDELOAD_PACKAGE);
    return 0;
}

static STATUS cache_menu_show(menuItem *p)
{
    //ensure_mounte cache path
    struct _opResult* result = ddOp_send(OP_MOUNT, 1, "/cache");
    int ret;
    do {
        ret = file_scan("/cache", sizeof("/cache"), p->name, strlen(p->name), &file_install, (void *)p, &file_filter);
        if (ret == -1) return MENU_BACK;
        else if(ret == -2) continue;
        else if(ret == MENU_MAIN) return MENU_MAIN;
        else break;
    } while(1);
    return ret;
}

static STATUS signature_setting_show(menuItem* p)
{
	ddOp_send(OP_SIGNATURE, 2, "get", "0");
	int status = ddOp_result_get_int();
	char newstat[2];
	char tmp[MENU_LEN];

	if( status == 0) {
		strncpy(newstat, "1", 2);
		menuItem_set_desc(p, "<~signature.setting.alert>: <~signature.setting.alert.off>");
	} else {
		strncpy(newstat, "0", 2);
		menuItem_set_desc(p, "<~signature.setting.alert>: <~signature.setting.alert.on>");
	}

    if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
        ddOp_send(OP_SIGNATURE, 2, "set", newstat);
        status = ddOp_result_get_int();
        if( status == 1)
        	dd_alert(4, p->name, "<~signature.setting.on>", NULL, acfg()->text_ok);
        else
        	dd_alert(4, p->name, "<~signature.setting.off>", NULL, acfg()->text_ok);
    }
    return MENU_BACK;
}

struct _menuItem * sd_ui_init()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~sd.name>");
    menuItem_set_title(p, "<~sd.title.name>");
    menuItem_set_desc(p, "<~sd.title.desc>");
    menuItem_set_icon(p, "@main.sd");

    p->result = 0;
    menuNode_init(p);

    //install from /sdcard
    struct _menuItem* temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    menuItem_set_icon(temp, "@sd.choose");
    if(acfg()->sd_ext == 0 && acfg()->sd_int == 1)
    	menuItem_set_name(temp, "<~sdint.install.name>");
    else
    	menuItem_set_name(temp, "<~sd.install.name>");
    menuItem_set_desc(temp, "<~sd.install.alert>");
    menuItem_set_show(temp, &sd_menu_show);
    menuNode_add(p, temp);

    if (acfg()->sd_ext != 0 && acfg()->sd_int != 0)
    {
        //install from /sdcard2 (internal SD)
        temp = common_ui_init();
        return_null_if_fail(temp != NULL);
        menuItem_set_icon(temp, "@sd.choose");
        menuItem_set_name(temp, "<~sdint.install.name>");
        menuItem_set_desc(temp, "<~sd.install.alert>");
        menuItem_set_show(temp, &sdint_menu_show);
        menuNode_add(p, temp);
    }

    //install from /cache
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    menuItem_set_icon(temp, "@sd.choose");
    menuItem_set_name(temp, "<~cache.install.name>");
    menuItem_set_show(temp, &cache_menu_show);
    menuNode_add(p, temp);

    //sideload mode
    if (acfg()->enable_sideload != 0) {
        temp = common_ui_init();
        return_null_if_fail(temp != NULL);
        menuItem_set_name(temp, "<~sideload.install.name>");
        menuItem_set_icon(temp, "@sd.choose");
        menuItem_set_show(temp, &sideload_install_show);
        menuNode_add(p, temp);
    }

    return p;

}
