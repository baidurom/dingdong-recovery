#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <pthread.h>

#include <sys/wait.h>
#include <sys/limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include <signal.h>
#include <sys/wait.h>

#include "cutils/properties.h"

#include "common.h"
#include "roots.h"

#include "extendedcommands.h"

// Return true if USB is connected.
int usb_connected() {
    int fd = open("/sys/class/android_usb/android0/state", O_RDONLY);
    if (fd < 0) {
        printf("failed to open /sys/class/android_usb/android0/state: %s\n",
               strerror(errno));
        return 0;
    }

    char buf;
    /* USB is connected if android_usb state is CONNECTED or CONFIGURED */
    int connected = (read(fd, &buf, 1) == 1) && (buf == 'C');
    if (close(fd) < 0) {
        printf("failed to close /sys/class/android_usb/android0/state: %s\n",
               strerror(errno));
    }
    return connected;
}

#define BATTERY_CAPACITY_FILE "/sys/class/power_supply/battery/capacity"
// get battery capacity, when lower than 10%, break the option
int get_battery_capacity(){
    FILE *fp;
    char btmp[5];
    int capacity = -1;
    if(NULL != (fp = fopen(BATTERY_CAPACITY_FILE, "r"))){
        if(fgets(btmp, sizeof(btmp), fp)){
            capacity = atoi(btmp);
        }
        printf("Current battery capacity %d%%\n", capacity);
        fclose(fp);
    }

    return capacity;
}

void write_fstab_root(char *path, FILE *file)
{
    Volume *vol = volume_for_path(path);
    if (vol == NULL) {
        //LOGW("Unable to get recovery.fstab info for %s during fstab generation!\n", path);
        return;
    }

    char device[200];
    strcpy(device, vol->device);

    fprintf(file, "%s ", device);
    fprintf(file, "%s ", path);
    // special case rfs cause auto will mount it as vfat on samsung.
    fprintf(file, "%s rw\n", vol->fs_type);
}

void create_fstab()
{
    struct stat info;
    if (creat("/etc/mtab", (S_IRWXU | S_IRWXG | S_IROTH)) < 0)
        LOGW("Unable to create /etc/mtab!\n");

    FILE *file = fopen("/etc/fstab", "w");
    if (file == NULL) {
        LOGW("Unable to create /etc/fstab!\n");
        return;
    }
    Volume *vol = volume_for_path("/boot");
    if (NULL != vol && strcmp(vol->fs_type, "mtd") != 0 && strcmp(vol->fs_type, "emmc") != 0 && strcmp(vol->fs_type, "bml") != 0)
         write_fstab_root("/boot", file);
    write_fstab_root("/cache", file);
    write_fstab_root("/data", file);
    write_fstab_root("/cust", file);
    write_fstab_root("/emmc", file);
    write_fstab_root("/system", file);
    write_fstab_root("/sdcard", file);
    write_fstab_root("/sdcard2", file);
    write_fstab_root("/recovery", file);
    fclose(file);
    LOGI("Completed outputting fstab.\n");
}

void process_volumes() {
    create_fstab();
}

static const int MAX_APP_NAME = 4096;
int remove_system_app(const char *dest_app_path, const char *app_list){
    char app_name[MAX_APP_NAME];
    char app_path[MAX_APP_NAME];
    char *tmp;
    int rtn = 0;
    int status = 0;

    printf("Delete application according to app.list\n");

    if (ensure_path_mounted(dest_app_path) != 0) {
        printf("Can't mount %s\n", dest_app_path);
        return 1;
    }

    if (ensure_path_mounted(app_list) != 0) {
        printf("Can't mount %s\n", app_list);
        return 1;
    }

    FILE *fp = fopen_path(app_list, "r");
    if(fp != NULL){
        memset(app_name, 0, sizeof(app_name));
        memset(app_path, 0, sizeof(app_path));
        while(fgets(app_name, sizeof(app_name), fp) != NULL){
            sprintf(app_path, "%s/%s", dest_app_path, app_name);
            tmp = strstr(app_path, "apk");
            tmp[3] = 0;

            status = unlink(app_path);
            if(status){
                printf("Delete: %s(%s) - FAILED\n", app_path, strerror(errno));
                continue;
            }else{
                printf("Delete: %s - SUCCEED\n", app_path);
            }

            sprintf(tmp, "odex");
            tmp[4] = 0;
            status = unlink(app_path);
            if(status){
                printf("Delete: %s(%s) - FAILED\n", app_path, strerror(errno));
            }else{
                printf("Delete: %s - SUCCEED\n", app_path);
            }
        }
        fclose(fp);
    } else {
        printf("Can't open application list %s (%s)\n", app_list, strerror(errno));
        rtn = 1;
    }

    if (ensure_path_unmounted(dest_app_path) != 0) {
        printf("Can't umount %s\n", dest_app_path);
    }

    return rtn;
}


#define MAX_NUM_USB_VOLUMES 3
#define LUN_FILE_EXPANDS    2

struct lun_node {
    const char *lun_file;
    struct lun_node *next;
};

static struct lun_node *lun_head = NULL;
static struct lun_node *lun_tail = NULL;

int control_usb_storage_set_lun(Volume* vol, int enable, const char *lun_file) {
    const char *vol_device = enable ? vol->device : "";
    int fd;
    struct lun_node *node;

    // Verify that we have not already used this LUN file
    for(node = lun_head; node; node = node->next) {
        if (!strcmp(node->lun_file, lun_file)) {
            // Skip any LUN files that are already in use
            return -1;
        }
    }

    // Open a handle to the LUN file
    LOGI("Trying %s on LUN file %s\n", vol->device, lun_file);
    if ((fd = open(lun_file, O_WRONLY)) < 0) {
        LOGW("Unable to open ums lunfile %s (%s)\n", lun_file, strerror(errno));
        return -1;
    }

    // Write the volume path to the LUN file
    if ((write(fd, vol_device, strlen(vol_device) + 1) < 0) &&
       (!enable || !vol->device2 || (write(fd, vol->device2, strlen(vol->device2)) < 0))) {
        LOGW("Unable to write to ums lunfile %s (%s)\n", lun_file, strerror(errno));
        close(fd);
        return -1;
    } else {
        // Volume path to LUN association succeeded
        close(fd);

        // Save off a record of this lun_file being in use now
        node = (struct lun_node *)malloc(sizeof(struct lun_node));
        node->lun_file = strdup(lun_file);
        node->next = NULL;
        if (lun_head == NULL)
           lun_head = lun_tail = node;
        else {
           lun_tail->next = node;
           lun_tail = node;
        }

        LOGI("Successfully %sshared %s on LUN file %s\n", enable ? "" : "un", vol->device, lun_file);
        return 0;
    }
}

int control_usb_storage_for_lun(Volume* vol, int enable) {
    static const char* lun_files[] = {
        "/sys/devices/platform/usb_mass_storage/lun%d/file",
        "/sys/class/android_usb/android0/f_mass_storage/lun0/file",
        "/sys/class/android_usb/android0/f_mass_storage/lun1/file",
        NULL
    };

    if(!strncmp(vol->device, "/dev/null", 9)) {
    	LOGI("%s on %s, ignore...\n", vol->mount_point, vol->device);
    	return -1;
    }

    char value[PROPERTY_VALUE_MAX];
    if (enable) {
        property_get("sys.storage.ums_enabled", value, "");
        value[PROPERTY_VALUE_MAX - 1] = '\0';
        LOGI("sys.storage.ums_enabled = %s\n", value);
        if (strncmp("1", value, 1))
        	property_set("sys.storage.ums_enabled", "1");
    } else {
    	property_get("sys.storage.ums_enabled", value, "");
        value[PROPERTY_VALUE_MAX - 1] = '\0';
        LOGI("sys.storage.ums_enabled = %s\n", value);
        if (strncmp("0", value, 1))
        	property_set("sys.storage.ums_enabled", "0");
    }

    // If recovery.fstab specifies a LUN file, use it
    if (vol->lun) {
        return control_usb_storage_set_lun(vol, enable, vol->lun);
    }

    // Try to find a LUN for this volume
    //   - iterate through the lun file paths
    //   - expand any %d by LUN_FILE_EXPANDS
    int lun_num = 0;
    int i;
    for(i = 0; lun_files[i]; i++) {
        const char *lun_file = lun_files[i];
        for(lun_num = 0; lun_num < LUN_FILE_EXPANDS; lun_num++) {
            char formatted_lun_file[255];

            // Replace %d with the LUN number
            bzero(formatted_lun_file, 255);
            snprintf(formatted_lun_file, 254, lun_file, lun_num);

            // Attempt to use the LUN file
            if (control_usb_storage_set_lun(vol, enable, formatted_lun_file) == 0) {
                return 0;
            }
        }
    }

    // All LUNs were exhausted and none worked
    LOGW("Could not %sable %s on LUN %d\n", enable ? "en" : "dis", vol->device, lun_num);

    return -1;  // -1 failure, 0 success
}

int control_usb_storage(Volume **volumes, int enable) {
    int res = -1;
    int i;
    for(i = 0; i < MAX_NUM_USB_VOLUMES; i++) {
        Volume *volume = volumes[i];
        if (volume) {
            int vol_res = control_usb_storage_for_lun(volume, enable);
            if (vol_res == 0) res = 0; // if any one path succeeds, we return success
        }
    }

    // Release memory used by the LUN file linked list
    struct lun_node *node = lun_head;
    while(node) {
       struct lun_node *next = node->next;
       free((void *)node->lun_file);
       free(node);
       node = next;
    }
    lun_head = lun_tail = NULL;

    return res;  // -1 failure, 0 success
}

static void toggle_usb_storage(int enable) {
    // Build a list of Volume objects; some or all may not be valid
    Volume* volumes[MAX_NUM_USB_VOLUMES] = {
        volume_for_path("/sdcard"),
        volume_for_path("/sdcard2"),
    };

    // Enable USB storage
    if (enable) {
        if (control_usb_storage(volumes, 1))
            return;
    } else {
    	control_usb_storage(volumes, 0);
    }
}

void set_usb_storage_enable() {
	toggle_usb_storage(1);
}

void set_usb_storage_disable() {
	toggle_usb_storage(0);
}

#define APP_INFO_FILE "/proc/app_info"
int huawei_charge_detect()
{
    int charge_flag = 0;
    FILE *fp = fopen(APP_INFO_FILE, "r");
    if(fp!=NULL){
        char buf[128];
        while(fgets(buf,128,fp)!=NULL){
            if(strncmp(buf,"charge_flag",11)==0){
                char c = fgetc(fp);
                if(c=='1'){
                    printf("CHARGE-FLAG: %c\n", c);
                    charge_flag=  1;
                }
                break;
            }
        }
        fclose(fp);
    }

    /* when charge_mode enabled, let charge daemon handle everything */
    if(charge_flag == 1){
        property_set("service.huawei.poweroff.charge", "1");
        printf("charge mode enabled\n");
        return 0;
    }

    return 1;
}

