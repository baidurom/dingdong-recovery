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


#define BACKUP_ALL            1
#define BACKUP_BOOT           2
#define BACKUP_SYSTEM         3
#define BACKUP_DATA           4
#define BACKUP_CACHE          5
#define BACKUP_RECOVERY       6

#define RESTORE_ALL           11
#define RESTORE_BOOT          12
#define RESTORE_SYSTEM        13
#define RESTORE_DATA          14
#define RESTORE_CACHE         15
#define RESTORE_RECOVERY      16

#define MANAGE_FULL           21
#define MANAGE_BOOT           22
#define MANAGE_SYSTEM         23
#define MANAGE_DATA           24
#define MANAGE_CACHE          25
#define MANAGE_ALL            26

#define BACKUP_RESTORE 		  51
#define BACKUP_MANAGE 		  61

static char *DEFAULT_BACKUP_PATH_EXTSD = "/sdcard/dingdong/recovery/backup";
static char *DEFAULT_BACKUP_PATH_INTSD = "/sdcard2/dingdong/recovery/backup";
char backup_root[PATH_MAX];
static int tomain = 0;

static struct _menuItem* p_current = NULL;
static struct _menuItem* backup_menu = NULL;
static struct _ddProgress_unit ddProgress_args = {
	.success_text = "",
	.fail_text = "",
	.path = NULL,
	.install_file = NULL,
	.wipe_cache = 0,
	.backup_restore_type = 0
};
static struct _ddProgress_unit *backup_arg = &ddProgress_args;
static struct _ddProgress_unit *restore_arg = &ddProgress_args;

static void save_backup_config(const char* config_file) {
    FILE *conf = fopen_path(config_file, "w");
    if (conf == NULL) {
    	printf("Can't open %s\n", config_file);
    } else {
    	printf("Save path into %s\n", config_file);
        fputs(backup_root, conf);
        fclose(conf);
    }
}

static STATUS exec_manage(char* path, menuItem* p)
{
    return_val_if_fail(p_current != NULL, RET_FAIL);
    ddOp_send(OP_BACKUP_MANAGE, 1, path);
    int result = ddOp_result_get_int();
    if(!result)
    	dd_alert(4, p->name, "<~backup_manage.success.alert>", NULL, acfg()->text_ok);
    else
    	dd_alert(4, p->name, "<~backup_manage.fail.alert>", NULL, acfg()->text_ok);
    return RET_OK;
}

static void restore_progress(struct _ddProgress_unit *r_args)
{
	int restore_partition = r_args->backup_restore_type;
    switch(restore_partition) {
        case RESTORE_ALL:
            ddOp_send(OP_RESTORE, 7, r_args->path, "1", "1", "1", "1", "0", "0");
            break;
        case RESTORE_BOOT:
            ddOp_send(OP_RESTORE, 7, r_args->path, "1", "0", "0", "0", "0", "0");
            break;
        case RESTORE_SYSTEM:
            ddOp_send(OP_RESTORE, 7, r_args->path, "0", "1", "0", "0", "0", "0");
           break;
        case RESTORE_DATA:
            ddOp_send(OP_RESTORE, 7, r_args->path, "0", "0", "1", "0", "0", "0");
            break;
        case RESTORE_CACHE:
            ddOp_send(OP_RESTORE, 7, r_args->path, "0", "0", "0", "1", "0", "0");
            break;
        default:
            dd_error("restore_partition = %d, should not be the value\n", restore_partition);
            ddOp_result_set(1, NULL);
            break;
    }
    ddProgress_set_progress(1);
    int result = ddOp_result_get_int();
    if(result)
    	aw_post(aw_msg(16, 0, 0, 0));
    else
    	aw_post(aw_msg(15, 0, 0, 0));
}

static STATUS exec_restore(char* path, menuItem* p)
{
    return_val_if_fail(p_current != NULL, RET_FAIL);
    //dd_busy_process();
    ddProgress_fun restore_fun;
    restore_fun = &restore_progress;
    strncpy(restore_arg->success_text, "<~restore.progress.success.info>", PATH_MAX);
    strncpy(restore_arg->fail_text, "<~restore.progress.fail.info>", PATH_MAX);
    restore_arg->path = path;

    ddProgress_init(restore_fun, restore_arg);
    dd_progress("@list.file", p->name, p->name, 1);
    return RET_OK;
}

static STATUS _backup_dir_show(char *path, menuItem* p, int operation)
{
    DIR* d;
    struct dirent* de;
    printf("Opening backup path: %s\n", path);
    d = opendir(path);
    if (!d)
    	return RET_FAIL;

    int d_size = 0;
    int d_alloc = 10;
    return_val_if_fail(backup_menu != NULL, RET_FAIL);
    char** dirs = malloc(d_alloc * sizeof(char*));
    char** dirs_desc = malloc(d_alloc * sizeof(char*));
    return_val_if_fail(dirs != NULL, RET_FAIL);
    return_val_if_fail(dirs_desc != NULL, RET_FAIL);
    int z_size = 0;
    int z_alloc = 10;
    char** zips = malloc(z_alloc * sizeof(char*));
    char** zips_desc=malloc(z_alloc * sizeof(char*));
    return_val_if_fail(zips != NULL, RET_FAIL);
    return_val_if_fail(zips_desc != NULL, RET_FAIL);
    //zips[0] = strdup("../");
    //zips_desc[0]=strdup("../");

    while ((de = readdir(d)) != NULL) {
        int name_len = strlen(de->d_name);
        char de_path[PATH_MAX];
        snprintf(de_path, PATH_MAX, "%s/%s", path, de->d_name);
        struct stat st ;
        assert_if_fail(stat(de_path, &st) == 0);
        if (de->d_type == DT_DIR) {
            //skip "." and ".." entries
            if (name_len == 1 && de->d_name[0] == '.') continue;
            if (name_len == 2 && de->d_name[0] == '.' && 
                    de->d_name[1] == '.') continue;
            if (d_size >= d_alloc) {
                d_alloc *= 2;
                dirs = realloc(dirs, d_alloc * sizeof(char*));
                dirs_desc = realloc(dirs_desc, d_alloc * sizeof(char*));
            }
            dirs[d_size] = malloc(name_len + 2);
            dirs_desc[d_size] = malloc(64);
            strcpy(dirs[d_size], de->d_name);
            dirs[d_size][name_len ] = '\0';
            time_t GMT_8 = st.st_mtime + 8*3600;
            snprintf(dirs_desc[d_size], 64, "%s" ,ctime(&GMT_8));
            ++d_size;
        } else if (de->d_type == DT_REG && name_len >= 4 &&
                  strncasecmp(de->d_name + (name_len - 4), ".zip", 4) == 0) {
            if (z_size >= z_alloc) {
                z_alloc *= 2;
                zips = realloc(zips, z_alloc * sizeof(char*));
                zips_desc = realloc(zips_desc, z_alloc * sizeof(char*));
            }
            zips[z_size] = strdup(de->d_name);
            zips_desc[z_size] = malloc(64);
            time_t GMT_8 = st.st_mtime + 8*3600;
            snprintf(zips_desc[z_size], 64, "%s%lldMB" ,ctime(&GMT_8), st.st_size/(uint64_t)(1024*1024));
            z_size++;
        }
    }
    closedir(d);


    // append dirs to the zips list
    if (d_size + z_size + 1 > z_alloc) {
        z_alloc = d_size + z_size + 1;
        zips = realloc(zips, z_alloc * sizeof(char*));
        zips_desc = realloc(zips_desc, z_alloc * sizeof(char*));
    }
    memcpy(zips + z_size, dirs, d_size * sizeof(char *));
    memcpy(zips_desc + z_size, dirs_desc, d_size * sizeof(char*));
    free(dirs);
    z_size += d_size;
    zips[z_size] = NULL;
    zips_desc[z_size] = NULL;

   int result;
   int chosen_item = -1;
   do {
	   if(tomain) {
		   result = MENU_MAIN;
		   break;
	   }
       chosen_item = dd_sdmenu(backup_menu->name, zips, zips_desc, z_size);

       if (chosen_item == MENU_MAIN) {
    	   //go to main menu
    	   tomain = 1;
    	   result = MENU_MAIN;
    	   break;
       }
       if ( chosen_item == -1) {
           //go up but continue browsing
           result = -1;
           break;
       }

       char * item = zips[chosen_item];
       if (!item){
           //go up but continue browsing
    	   result = -1;
          break;
       }
       int item_len = strlen(item);
       // select a zipfile
       // the status to the caller
       char new_path[PATH_MAX];
       strlcpy(new_path, path, PATH_MAX);
       strlcat(new_path, "/", PATH_MAX);
       strlcat(new_path, item, PATH_MAX);
       /*
        *nandroid_restore(backup_path, restore_boot, system, data, chache , sdext, wimax)
        */
       if (p_current != NULL && RET_YES == dd_confirm(3, p_current->name, p_current->desc, p_current->icon)) {
    	   switch (operation) {
    	   	   case BACKUP_RESTORE:
    	   	   {
    	   		   exec_restore(new_path, p);
    	   		   break;
    	   	   }
    	   	   case BACKUP_MANAGE:
    	   	   {
    	   		   exec_manage(new_path, p);
    	   		   result = -2;
    	   		   break;
    	   	   }
    	   	   default:
    	   	   {
    	   		   dd_error("Unknown operation: %d\n", operation);
    	   		   break;
    	   	   }
    	   }
       }
   } while(1);

   int i;
   for (i = 0; i < z_size; ++i) 
   {
       free(zips[i]);
       free(zips_desc[i]);
   }
   free(zips);
   return result;
}


static STATUS backup_manage_child_show(menuItem* p)
{
    p_current = p;
    ddOp_send(OP_MOUNT, 1, backup_root);
    if(ddOp_result_get_int()) {
    	dd_alert(4, p->name, "<~backup_path.setting.alert>", NULL, acfg()->text_ok);
    	return MENU_BACK;
    }

    char fullpath[PATH_MAX];
    switch(p->result) {
        case MANAGE_FULL:
            snprintf(fullpath,PATH_MAX, "%s/full", backup_root);
            break;
        case MANAGE_BOOT:
            snprintf(fullpath,PATH_MAX, "%s/boot", backup_root);
            break;
        case MANAGE_SYSTEM:
            snprintf(fullpath,PATH_MAX, "%s/system", backup_root);
            break;
        case MANAGE_DATA:
            snprintf(fullpath,PATH_MAX, "%s/data", backup_root);
            break;
        case MANAGE_CACHE:
            snprintf(fullpath,PATH_MAX, "%s/cache", backup_root);
            break;
        case MANAGE_ALL:
            snprintf(fullpath,PATH_MAX, "%s", backup_root);
            break;
        default:
        	dd_error("p->result = %d, should not be the value\n", p->result);
            return MENU_BACK;
    }
    int ret;
    tomain = 0;
    do {
    	ret = _backup_dir_show(fullpath, p, BACKUP_MANAGE);
    	if (ret == -1) return MENU_BACK;
    	else if(ret == -2) continue;
    	else if(ret == MENU_MAIN) return MENU_MAIN;
    	else break;
    } while(1);
    return MENU_BACK;
}

static STATUS restore_child_show(menuItem* p)
{
    p_current = p;
    ddOp_send(OP_MOUNT, 1, backup_root);
    if(ddOp_result_get_int()) {
    	dd_alert(4, p->name, "<~backup_path.setting.alert>", NULL, acfg()->text_ok);
    	return MENU_BACK;
    }

    memset(backup_arg, 0, sizeof(ddProgress_unit));
    char fullpath[PATH_MAX];
    switch(p->result) {
        case RESTORE_ALL:
            snprintf(fullpath,PATH_MAX, "%s/full", backup_root);
            backup_arg->backup_restore_type = RESTORE_ALL;
            break;
        case RESTORE_BOOT:
            snprintf(fullpath,PATH_MAX, "%s/boot", backup_root);
            backup_arg->backup_restore_type = RESTORE_BOOT;
            break;
        case RESTORE_SYSTEM:
            snprintf(fullpath,PATH_MAX, "%s/system", backup_root);
            backup_arg->backup_restore_type = RESTORE_SYSTEM;
            break;
        case RESTORE_DATA:
            snprintf(fullpath,PATH_MAX, "%s/data", backup_root);
            backup_arg->backup_restore_type = RESTORE_DATA;
            break;
        case RESTORE_CACHE:
            snprintf(fullpath,PATH_MAX, "%s/cache", backup_root);
            backup_arg->backup_restore_type = RESTORE_CACHE;
            break;
        default:
        	dd_error("p->result = %d, should not be the value\n", p->result);
            return MENU_BACK;
    }
    int ret;
    tomain = 0;
    ret = _backup_dir_show(fullpath, p, BACKUP_RESTORE);
    if(ret == MENU_MAIN) return MENU_MAIN;
    return MENU_BACK;
}

static void backup_progress(struct _ddProgress_unit *b_args)
{
	int backup_partition = b_args->backup_restore_type;
    switch(backup_partition) {
        case BACKUP_ALL:
            ddOp_send(OP_BACKUP, 1, b_args->path);
            break;
        case BACKUP_BOOT:
            ddOp_send(OP_ADVANCED_BACKUP, 2, b_args->path, "/boot");
            break;
        case BACKUP_SYSTEM:
            ddOp_send(OP_ADVANCED_BACKUP, 2, b_args->path, "/system");
            break;
        case BACKUP_DATA:
            ddOp_send(OP_ADVANCED_BACKUP, 2, b_args->path, "/data");
            break;
        case BACKUP_CACHE:
            ddOp_send(OP_ADVANCED_BACKUP, 2,b_args->path, "/cache");
            break;
        default:
            dd_error("backup_partition = %d, should not be the value\n", backup_partition);
            ddOp_result_set(1, NULL);
            break;
    }
    int result = ddOp_result_get_int();
    if(result)
    	aw_post(aw_msg(16, 0, 0, 0));
    else
    	aw_post(aw_msg(15, 0, 0, 0));
}

static STATUS backup_child_show(menuItem* p)
{
    p_current = p;
    ddOp_send(OP_MOUNT, 1, backup_root);
    if(ddOp_result_get_int()) {
    	dd_alert(4, p->name, "<~backup_path.setting.alert>", NULL, acfg()->text_ok);
    	return MENU_BACK;
    }
    printf("Backup path: %s\n", backup_root);

    if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
        //dd_busy_process();
        memset(backup_arg, 0, sizeof(ddProgress_unit));
        char fullpath[PATH_MAX];
        char partition[PATH_MAX];
        ddProgress_fun bakcup_fun;

        switch(p->result) {
            case BACKUP_ALL:
            	strncpy(partition, "full", PATH_MAX);
            	backup_arg->backup_restore_type = BACKUP_ALL;
                break;
            case BACKUP_BOOT:
            	strncpy(partition, "boot", PATH_MAX);
            	backup_arg->backup_restore_type = BACKUP_BOOT;
                break;
            case BACKUP_SYSTEM:
            	strncpy(partition, "system", PATH_MAX);
            	backup_arg->backup_restore_type = BACKUP_SYSTEM;
                break;
            case BACKUP_DATA:
            	strncpy(partition, "data", PATH_MAX);
            	backup_arg->backup_restore_type = BACKUP_DATA;
                break;
            case BACKUP_CACHE:
            	strncpy(partition, "cache", PATH_MAX);
            	backup_arg->backup_restore_type = BACKUP_CACHE;
                break;
            default:
                dd_error("p->result = %d, should not be the value\n", p->result);
                return MENU_BACK;
        }

        time_t timep = time(NULL);
        struct tm *time_tmp = gmtime(&timep);
        if (time_tmp == NULL)
        {
            struct timeval tp;
            gettimeofday(&tp, NULL);
            snprintf(fullpath, PATH_MAX, "%s/%s/%ld", backup_root, partition, tp.tv_sec);
        }
        else
        {// display as follow format: 2012-01-12-05-20
            snprintf(fullpath, PATH_MAX, "%s/%s/%d-%02d-%02d-%02d-%02d",
            		backup_root,
            		partition,
            		time_tmp->tm_year+1900,
            		time_tmp->tm_mon+1,
            		time_tmp->tm_mday,
            		time_tmp->tm_hour+8,
            		time_tmp->tm_min
            		);
        }

        bakcup_fun = &backup_progress;
        strncpy(backup_arg->success_text, "<~backup.progress.success.info>", PATH_MAX);
        strncpy(backup_arg->fail_text, "<~backup.progress.fail.info>", PATH_MAX);
        backup_arg->path = fullpath;

        ddProgress_init(bakcup_fun, backup_arg);
        dd_progress("@list.file", p->name, p->name, 1);
    }
    return MENU_BACK;
}

struct _menuItem* advanced_backup_ui_init()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~advanced_backup.name>");
    menuItem_set_show(p, &common_menu_show);
    menuNode_init(p);
    //backup boot
    struct _menuItem* temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_backup.boot.name>");
    menuItem_set_desc(temp, "<~advanced_backup.confirm.alert>");
    menuItem_set_result(temp, BACKUP_BOOT);
    menuItem_set_show(temp, &backup_child_show);
    //backup system
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_backup.system.name>");
    menuItem_set_desc(temp, "<~advanced_backup.confirm.alert>");
    menuItem_set_result(temp, BACKUP_SYSTEM);
    menuItem_set_show(temp, &backup_child_show);
    //backup data
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_backup.data.name>");
    menuItem_set_desc(temp, "<~advanced_backup.confirm.alert>");
    menuItem_set_result(temp, BACKUP_DATA);
    menuItem_set_show(temp, &backup_child_show);
    //backup cache
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_backup.cache.name>");
    menuItem_set_desc(temp, "<~advanced_backup.confirm.alert>");
    menuItem_set_result(temp, BACKUP_CACHE);
    menuItem_set_show(temp, &backup_child_show);
    return p;
}
struct _menuItem* advanced_restore_ui_init()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~advanced_restore.name>");
    menuItem_set_show(p, &common_menu_show);
    menuNode_init(p);
    //restore boot
    struct _menuItem* temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_restore.boot.name>");
    menuItem_set_desc(temp, "<~advanced_restore.confirm.alert>");
    menuItem_set_result(temp, RESTORE_BOOT);
    menuItem_set_show(temp, &restore_child_show);
    //restore system
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_restore.system.name>");
    menuItem_set_desc(temp, "<~advanced_restore.confirm.alert>");
    menuItem_set_result(temp, RESTORE_SYSTEM);
    menuItem_set_show(temp, &restore_child_show);
    //restore data
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_restore.data.name>");
    menuItem_set_desc(temp, "<~advanced_restore.confirm.alert>");
    menuItem_set_result(temp, RESTORE_DATA);
    menuItem_set_show(temp, &restore_child_show);
    //restore cache
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~advanced_restore.cache.name>");
    menuItem_set_desc(temp, "<~advanced_restore.confirm.alert>");
    menuItem_set_result(temp, RESTORE_CACHE);
    menuItem_set_show(temp, &restore_child_show);
    return p;
}

struct _menuItem* backup_manage_ui_init()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~backup_manage.name>");
    menuItem_set_show(p, &common_menu_show);
    menuNode_init(p);
    //full manage
    struct _menuItem* temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_manage.full.name>");
    menuItem_set_desc(temp, "<~backup_manage.confirm.alert>");
    menuItem_set_result(temp, MANAGE_FULL);
    menuItem_set_show(temp, &backup_manage_child_show);
    //boot manage
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_manage.boot.name>");
    menuItem_set_desc(temp, "<~backup_manage.confirm.alert>");
    menuItem_set_result(temp, MANAGE_BOOT);
    menuItem_set_show(temp, &backup_manage_child_show);
    //system manage
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_manage.system.name>");
    menuItem_set_desc(temp, "<~backup_manage.confirm.alert>");
    menuItem_set_result(temp, MANAGE_SYSTEM);
    menuItem_set_show(temp, &backup_manage_child_show);
    //data manage
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_manage.data.name>");
    menuItem_set_desc(temp, "<~backup_manage.confirm.alert>");
    menuItem_set_result(temp, MANAGE_DATA);
    menuItem_set_show(temp, &backup_manage_child_show);
    //cache manage
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_manage.cache.name>");
    menuItem_set_desc(temp, "<~backup_manage.confirm.alert>");
    menuItem_set_result(temp, MANAGE_CACHE);
    menuItem_set_show(temp, &backup_manage_child_show);
    //all manage
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_manage.all.name>");
    menuItem_set_desc(temp, "<~backup_manage.confirm.alert>");
    menuItem_set_result(temp, MANAGE_ALL);
    menuItem_set_show(temp, &backup_manage_child_show);
    return p;
}

static STATUS backup_path_checkout_show(menuItem* p)
{
	char path_name[PATH_MAX];
	snprintf(path_name, PATH_MAX, "<~backup_path.current.desc>: %s", backup_root);
    dd_alert(4, p->name, path_name, NULL, acfg()->text_ok);
    return MENU_BACK;
}

static STATUS backup_path_external_show(menuItem* p)
{
	char path_name[PATH_MAX];
    ddOp_send(OP_MOUNT, 1, "/sdcard");
    int result = ddOp_result_get_int();
    if (!result) {
    	if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
			snprintf(backup_root, PATH_MAX, "%s", DEFAULT_BACKUP_PATH_EXTSD);
			save_backup_config(BACKUP_PATH_CONFIG);
			snprintf(path_name, PATH_MAX, "<~backup_path.current.desc>: %s", backup_root);
			dd_alert(4, p->name, path_name, NULL, acfg()->text_ok);
	    }
	} else {
    	dd_alert(4, p->name, "<~sd.mount.error>", NULL, acfg()->text_ok);
    }
	return MENU_BACK;
}

static STATUS backup_path_internal_show(menuItem* p)
{
	char path_name[PATH_MAX];
    ddOp_send(OP_MOUNT, 1, "/sdcard2");
    int result = ddOp_result_get_int();
    if (!result) {
    	if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
			snprintf(backup_root, PATH_MAX, "%s", DEFAULT_BACKUP_PATH_INTSD);
			save_backup_config(BACKUP_PATH_CONFIG);
			snprintf(path_name, PATH_MAX, "<~backup_path.current.desc>: %s", backup_root);
			dd_alert(4, p->name, path_name, NULL, acfg()->text_ok);
	    }
	} else {
    	dd_alert(4, p->name, "<~sd.mount.error>", NULL, acfg()->text_ok);
    }
	return MENU_BACK;
}

struct _menuItem* backup_path_setting()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~backup_path.setting.name>");
    menuItem_set_show(p, &common_menu_show);
    menuNode_init(p);

    //checkout current backup path
    struct _menuItem* temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup_path.checkout.name>");
    menuItem_set_show(temp, &backup_path_checkout_show);

    //backup path on external sdcard
    temp = common_ui_init();
    menuNode_add(p, temp);
    if(acfg()->sd_ext == 0 && acfg()->sd_int == 1)
    	menuItem_set_name(temp, "<~backup_path.internalsd.name>");
    else
    	menuItem_set_name(temp, "<~backup_path.externalsd.name>");
    menuItem_set_desc(temp, "<~backup_path.change.alert>");
    menuItem_set_show(temp, &backup_path_external_show);

    if (acfg()->sd_ext == 1 && acfg()->sd_int == 1) {
	    //backup path on external sdcard
	    temp = common_ui_init();
	    menuNode_add(p, temp);
	    menuItem_set_name(temp, "<~backup_path.internalsd.name>");
	    menuItem_set_desc(temp, "<~backup_path.change.alert>");
	    menuItem_set_show(temp, &backup_path_internal_show);
	}

	return p;
}

struct _menuItem* backup_ui_init()
{
	// Initialize the backup path
	if(strcmp(backup_root, "")==0){
		ddOp_send(OP_MOUNT, 1, "/sdcard");
		if(strstr(ddOp_result_get_string(), "mounted") != NULL)
			snprintf(backup_root, PATH_MAX, "%s", DEFAULT_BACKUP_PATH_EXTSD);
		else
			snprintf(backup_root, PATH_MAX, "%s", DEFAULT_BACKUP_PATH_INTSD);
		printf("Backup path not set, initializing: %s\n", backup_root);
		save_backup_config(BACKUP_PATH_CONFIG);
	}

    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~backup.name>");
    menuItem_set_title(p, "<~backup.title>");
    menuItem_set_desc(p, "<~backup.title.desc>");
    menuItem_set_icon(p, "@main.backup");
    menuItem_set_show(p, &common_menu_show);
    menuNode_init(p);
    backup_menu = p;
    //backup & restore root path setting
    struct _menuItem* temp = backup_path_setting();
    menuNode_add(p, temp);
    //backup
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup.backup.name>");
    menuItem_set_desc(temp, "<~backup.confirm.alert>");
    menuItem_set_result(temp, BACKUP_ALL);
    menuItem_set_show(temp, &backup_child_show);
    //restore
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~backup.restore.name>");
    menuItem_set_desc(temp, "<~restore.confirm.alert>");
    menuItem_set_result(temp, RESTORE_ALL);
    menuItem_set_show(temp, &restore_child_show);
    //advanced backup
    temp = advanced_backup_ui_init();
    menuNode_add(p, temp);
    //advanced restore
    temp = advanced_restore_ui_init();
    menuNode_add(p, temp);
    //manage backup files
    temp = backup_manage_ui_init();
    menuNode_add(p, temp);
    return p;
}
