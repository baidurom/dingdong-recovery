#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"

#define BUILD_PROP "/system/build.prop"

static STATUS advanced_battary_show(struct _menuItem* p)
{
    if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
        ddOp_send(OP_MOUNT, 1, "/data");
        unlink("/data/system/batterystats.bin");
        ddOp_send(OP_UNMOUNT, 1, "/data");
        dd_printf("Battery Stats wiped.\n");
        dd_alert(4, p->name, "<~advanced.battery.wipe.info>", NULL, acfg()->text_ok);
    }
    return MENU_BACK;
}
static STATUS advanced_dalvik_show(struct _menuItem* p)
{
    if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
        ddOp_send(OP_WIPE, 1, "dalvik-cache");
        dd_printf("dalvik-cache Stats wiped.\n");
        dd_alert(4, p->name, "<~advanced.dalvik.wipe.info>", NULL, acfg()->text_ok);
    }
    return MENU_BACK;
}
static STATUS advanced_permission_show(struct _menuItem* p)
{
	if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
		dd_busy_process(0);
	    ddOp_send(OP_MOUNT, 1, "/system");
	    ddOp_send(OP_MOUNT, 1, "/data");
	    ddOp_send(OP_SYSTEM, 1, "fix_permissions");
	    dd_alert(4, p->name, "<~advanced.permission.fixed.info>", NULL, acfg()->text_ok);
	}
    return MENU_BACK;
}
static STATUS advanced_copylog_show(struct _menuItem* p)
{
    char desc[512];
    char file_name[PATH_MAX];
    struct stat st;
    time_t timep;
    struct tm *time_tm;
    time(&timep);
    time_tm = gmtime(&timep);

    ddOp_send(OP_MOUNT, 1, "/sdcard");
    int result = ddOp_result_get_int();
    if(!result) {
        snprintf(file_name, PATH_MAX - 1, "%s/recovery-%02d%02d%02d-%02d%02d.log",
        	   RECOVERY_PATH,
               time_tm->tm_year+1900,
               time_tm->tm_mon+1,
               time_tm->tm_mday,
               time_tm->tm_hour+8,
               time_tm->tm_min
               );
        snprintf(desc, 511, "%s%s?",p->desc, file_name);
        if (RET_YES == dd_confirm(3, p->name, desc, p->icon)) {
        	if (stat(RECOVERY_PATH, &st) != 0)
        	{
        		mkdir(RECOVERY_PATH, 0755);
        	}

        	ddOp_send(OP_COPY, 2, DD_LOG_FILE, file_name);
        	int result = ddOp_result_get_int();
        	if (!result)
        		dd_alert(4, p->name, "<~advanced.log.copy.info>", NULL, acfg()->text_ok);
        	else
        		dd_alert(4, p->name, "<~advanced.log.copy.alert>", NULL, acfg()->text_ok);
        }
    } else {
        ddOp_send(OP_MOUNT, 1, "/sdcard2");
        result = ddOp_result_get_int();
        if(!result) {
            snprintf(file_name, PATH_MAX - 1, "%s/recovery-%02d%02d%02d-%02d%02d.log", RECOVERY_PATH_INTER,
                   time_tm->tm_year, time_tm->tm_mon + 1, time_tm->tm_mday,
                   time_tm->tm_hour, time_tm->tm_min);
            snprintf(desc, 511, "%s%s?",p->desc, file_name);
            if (RET_YES == dd_confirm(3, p->name, desc, p->icon)) {
            	if (stat(RECOVERY_PATH_INTER, &st) != 0)
            	{
            		mkdir(RECOVERY_PATH_INTER, 0755);
            	}

            	ddOp_send(OP_COPY, 2, DD_LOG_FILE, file_name);
            	int result = ddOp_result_get_int();
            	if (!result)
            		dd_alert(4, p->name, "<~advanced.log.copy.info>", NULL, acfg()->text_ok);
            	else
            		dd_alert(4, p->name, "<~advanced.log.copy.alert>", NULL, acfg()->text_ok);
            }
        } else {
        	dd_alert(4, p->name, "<~sd.mount.error>", NULL, acfg()->text_ok);
        }
    }
    return MENU_BACK;
}

#define MOUNT_LOG "/tmp/storageinfo"
STATUS advanced_mount_show(struct _menuItem* p)
{
    ddOp_send(OP_MOUNT, 1, "/data");
    ddOp_send(OP_MOUNT, 1, "/cache");
    ddOp_send(OP_MOUNT, 1, "/system");
    ddOp_send(OP_MOUNT, 1, "/sdcard");
    if(acfg()->sd_int == 1)
    	ddOp_send(OP_MOUNT, 1, "/sdcard2");
    ddOp_send(OP_SYSTEM, 1, "storageinfo");
    int ret;
    ret = dd_textbox(p->name, p->title_name, p->icon, dd_readfromfs(MOUNT_LOG));
    if (ret == MENU_MAIN) return MENU_MAIN;
    return MENU_BACK;
}

STATUS advanced_checklog_show(struct _menuItem* p)
{
    char file_name[PATH_MAX];
    snprintf(file_name, PATH_MAX, "%s", DD_LOG_FILE);
    int ret;
    ret = dd_textbox(p->name, p->title_name, p->icon, dd_readfromfs(file_name));
    if (ret == MENU_MAIN) return MENU_MAIN;
    return MENU_BACK;
}

struct _menuItem* advanced_ui_init()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~advanced.name>");
    menuItem_set_title(p, "<~advanced.title>");
    menuItem_set_desc(p, "<~advanced.title.desc>");
    menuItem_set_icon(p, "@main.advanced");
    menuNode_init(p);

    struct _menuItem *temp;
    //batarry wipe
    temp = common_ui_init();
    menuItem_set_name(temp, "<~advanced.battary.name>");
    //menuItem_set_icon(temp, "@advanced.battery");
    menuItem_set_show(temp, &advanced_battary_show);
    menuNode_add(p, temp);

    //fix permission
    temp = common_ui_init();
    menuItem_set_name(temp, "<~advanced.dalvik.name>");
    //menuItem_set_icon(temp, "@advanced.permission");
    menuItem_set_show(temp, &advanced_dalvik_show);
    menuNode_add(p, temp);

    //fix permission
    temp = common_ui_init();
    menuItem_set_name(temp, "<~advanced.permission.name>");
    //menuItem_set_icon(temp, "@advanced.permission");
    menuItem_set_show(temp, &advanced_permission_show);
    menuNode_add(p, temp);

    //copy log
    temp = common_ui_init();
    menuItem_set_name(temp, "<~advanced.log.name>");
    menuItem_set_show(temp, &advanced_copylog_show);
    //menuItem_set_icon(temp, "@advanced.log");
    menuItem_set_desc(temp, "<~advanced.log.desc>");
    menuNode_add(p, temp);

    //mount
    temp = common_ui_init();
    menuItem_set_name(temp, "<~advanced.mount.name>");
    menuItem_set_icon(temp, "@advanced.mount");
    menuItem_set_show(temp, &advanced_mount_show);
    menuNode_add(p, temp);
    //log
    temp = common_ui_init();
    menuItem_set_name(temp, "<~advanced.logcheck.name>");
    menuItem_set_icon(temp, "@advanced.log");
    menuItem_set_show(temp, &advanced_checklog_show);
    menuNode_add(p, temp);
    return p;
}
