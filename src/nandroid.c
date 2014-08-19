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

#include <sys/wait.h>
#include <sys/limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include <signal.h>
#include <sys/wait.h>
#include <libgen.h>

#include "bootloader.h"
#include "common.h"
#include "libcrecovery/common.h"
#include "cutils/properties.h"
#include "firmware.h"
#include "install.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"

#include <sys/vfs.h>

#include "nandroid.h"
#include "extendedcommands.h"
#include "mtdutils/mounts.h"
#include "ddui/src/dd.h"

#include "flashutils/flashutils.h"

#define DD_RECOVERY "dd_recovery"
extern char backup_root[PATH_MAX];
float curpos = 0;

static int print_and_error(const char *message){
	display_print("%s\n",message);
	return 1;
}

void nandroid_generate_timestamp_path(char* backup_path)
{
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    if (tmp == NULL)
    {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        sprintf(backup_path, "%s/%ld", backup_root, tp.tv_sec);
    }
    else
    {
    	char time[PATH_MAX];
        strftime(time, PATH_MAX, "%F.%H.%M.%S", tmp);
        snprintf(backup_path, PATH_MAX, "%s/%s", backup_root, time);
    }
}

static void ensure_directory(const char* dir) {
    char tmp[PATH_MAX];
    sprintf(tmp, "mkdir -p %s ; chmod 777 %s", dir, dir);
    __system(tmp);
}

static long delta_milliseconds(struct timeval from, struct timeval to) {
  long delta_sec = (to.tv_sec - from.tv_sec)*1000;
  long delta_usec = (to.tv_usec - from.tv_usec)/1000;
  return (delta_sec + delta_usec);
}

/*
 * How often nandroid updates text in ms
 */

#define NANDROID_UPDATE_INTERVAL 1000

static struct timeval lastupdate = (struct timeval) {0};
static int nandroid_files_total = 0;
static int nandroid_files_count = 0;

static void nandroid_callback(const char* filename) {
    if (filename == NULL)
        return;

    char tmp[PATH_MAX];
    strcpy(tmp, filename);
    if (tmp[strlen(tmp) - 1] == '\n')
        tmp[strlen(tmp) - 1] = '\0';
    LOGI("%s\n", tmp);
    ui_set_progress_info(tmp);

    if (nandroid_files_total != 0) {
        nandroid_files_count++;
        float progress_decimal = (float)((double)nandroid_files_count /
                                         (double)nandroid_files_total);
        ui_set_progress(progress_decimal);
    }
}

static void compute_directory_stats(const char* directory) {
    char tmp[PATH_MAX];
    char count_text[100];

    // reset file count if we ever return before setting it
    nandroid_files_count = 0;
    nandroid_files_total = 0;

    sprintf(tmp, "find %s | %s wc -l > /tmp/dircount", directory, strcmp(directory, "/data") == 0 && is_data_media() ? "grep -v /data/media |" : "");
    __system(tmp);

    FILE* f = fopen("/tmp/dircount", "r");
    if (f == NULL)
        return;

    if (fgets(count_text, sizeof(count_text), f) == NULL) {
        fclose(f);
        return;
    }

    size_t len = strlen(count_text);
    if (count_text[len - 1] == '\n')
        count_text[len - 1] = '\0';

    fclose(f);
    nandroid_files_total = atoi(count_text);
    ui_reset_progress();
    //ui_show_progress(1, 0);
}

static int nandroid_backup_bitfield = 0;
#define NANDROID_FIELD_DEDUPE_CLEARED_SPACE 1

typedef void (*file_event_callback)(const char* filename);
typedef int (*nandroid_backup_handler)(const char* backup_path, const char* backup_file_image, int callback);

static int mkyaffs2image_wrapper(const char* backup_path, const char* backup_file_image, int callback) {
    char backup_file_image_with_extension[PATH_MAX];
    sprintf(backup_file_image_with_extension, "%s.img", backup_file_image);
    gettimeofday(&lastupdate,NULL);
    return mkyaffs2image(backup_path, backup_file_image_with_extension, 0, callback ? nandroid_callback : NULL);
}

static int do_tar_compress(char* command, int callback) {
    char buf[PATH_MAX];

    FILE *fp = __popen(command, "r");
    if (fp == NULL) {
        ui_print("Unable to execute tar command!\n");
        return -1;
    }

    while (fgets(buf, PATH_MAX, fp) != NULL) {
        buf[PATH_MAX - 1] = '\0';
        if (callback)
            nandroid_callback(buf);
    }

    return __pclose(fp);
}

static int tar_compress_wrapper(const char* backup_path, const char* backup_file_image, int callback) {
    char tmp[PATH_MAX];
    sprintf(tmp, "cd $(dirname %s) ; touch %s.tar ; (tar cv %s $(basename %s) | split -a 1 -b 1000000000 /proc/self/fd/0 %s.tar.) 2> /proc/self/fd/1 ; exit $?", backup_path, backup_file_image, strcmp(backup_path, "/data") == 0 && is_data_media() ? "--exclude 'media'  --exclude 'share'" : "", backup_path, backup_file_image);

    return do_tar_compress(tmp, callback);
}

static int tar_gzip_compress_wrapper(const char* backup_path, const char* backup_file_image, int callback) {
    char tmp[PATH_MAX];
    sprintf(tmp, "cd /; tar czvf %s.tar.gz %s %s --exclude=data/data/com.google.android.music/files/*; exit $?",
            backup_file_image,
            backup_path,
            strcmp(backup_path, "/data") == 0 && is_data_media() ? "--exclude 'media' --exclude 'share'" : "");
    return do_tar_compress(tmp, callback);
}

static int tar_dump_wrapper(const char* backup_path, const char* backup_file_image, int callback) {
    char tmp[PATH_MAX];
    sprintf(tmp, "cd /; tar cvf %s %s %s --exclude=data/data/com.google.android.music/files/*; exit $?",
            backup_file_image,
            backup_path,
            strcmp(backup_path, "/data") == 0 && is_data_media() ? "--exclude 'media' --exclude 'share'" : "");
    return __system(tmp);
}

void nandroid_dedupe_gc(const char* blob_dir) {
    char backup_dir[PATH_MAX];
    strcpy(backup_dir, blob_dir);
    char *d = dirname(backup_dir);
    strcpy(backup_dir, d);
    strcat(backup_dir, "/backup");
    ui_print("Freeing space...\n");
    char tmp[PATH_MAX];
    sprintf(tmp, "dedupe gc %s $(find %s -name '*.dup')", blob_dir, backup_dir);
    __system(tmp);
    ui_print("Done freeing space.\n");
}

static int dedupe_compress_wrapper(const char* backup_path, const char* backup_file_image, int callback) {
    char tmp[PATH_MAX];
    char blob_dir[PATH_MAX];
    strcpy(blob_dir, backup_file_image);
    char *d = dirname(blob_dir);
    strcpy(blob_dir, d);
    d = dirname(blob_dir);
    strcpy(blob_dir, d);
    d = dirname(blob_dir);
    strcpy(blob_dir, d);
    strcat(blob_dir, "/blobs");
    ensure_directory(blob_dir);

    if (!(nandroid_backup_bitfield & NANDROID_FIELD_DEDUPE_CLEARED_SPACE)) {
        nandroid_backup_bitfield |= NANDROID_FIELD_DEDUPE_CLEARED_SPACE;
        nandroid_dedupe_gc(blob_dir);
    }

    sprintf(tmp, "dedupe c %s %s %s.dup %s", backup_path, blob_dir, backup_file_image, strcmp(backup_path, "/data") == 0 && is_data_media() ? "./media" : "");

    FILE *fp = __popen(tmp, "r");
    if (fp == NULL) {
        ui_print("Unable to execute dedupe.\n");
        return -1;
    }

    while (fgets(tmp, PATH_MAX, fp) != NULL) {
        tmp[PATH_MAX - 1] = '\0';
        if (callback)
            nandroid_callback(tmp);
    }

    return __pclose(fp);
}

static nandroid_backup_handler default_backup_handler = tar_compress_wrapper;
static char forced_backup_format[5] = "";
void nandroid_force_backup_format(const char* fmt) {
    strcpy(forced_backup_format, fmt);
}
static void refresh_default_backup_handler() {
    char fmt[5];
    if (strlen(forced_backup_format) > 0) {
        strcpy(fmt, forced_backup_format);
    }
    else {
        ensure_path_mounted("/sdcard");
        FILE* f = fopen(NANDROID_BACKUP_FORMAT_FILE, "r");
        if (NULL == f) {
            default_backup_handler = tar_compress_wrapper;
            return;
        }
        fread(fmt, 1, sizeof(fmt), f);
        fclose(f);
    }
    fmt[3] = '\0';
    if (0 == strcmp(fmt, "dup"))
        default_backup_handler = dedupe_compress_wrapper;
    else if (0 == strcmp(fmt, "tgz"))
        default_backup_handler = tar_gzip_compress_wrapper;
    else if (0 == strcmp(fmt, "tar"))
        default_backup_handler = tar_compress_wrapper;
    else
        default_backup_handler = tar_compress_wrapper;
}

unsigned nandroid_get_default_backup_format() {
    refresh_default_backup_handler();
    if (default_backup_handler == dedupe_compress_wrapper) {
        return NANDROID_BACKUP_FORMAT_DUP;
    } else if (default_backup_handler == tar_gzip_compress_wrapper) {
        return NANDROID_BACKUP_FORMAT_TGZ;
    } else {
        return NANDROID_BACKUP_FORMAT_TAR;
    }
}

static nandroid_backup_handler get_backup_handler(const char *backup_path) {
    Volume *v = volume_for_path(backup_path);
    if (v == NULL) {
        ui_print("Unable to find volume.\n");
        return NULL;
    }
    const MountedVolume *mv = find_mounted_volume_by_mount_point(v->mount_point);
    if (mv == NULL) {
        ui_print("Unable to find mounted volume: %s\n", v->mount_point);
        return NULL;
    }

    if (strcmp(backup_path, "/data") == 0 && is_data_media()) {
        return default_backup_handler;
    }

    if (strlen(forced_backup_format) > 0)
        return default_backup_handler;

    // cwr5, we prefer dedupe for everything except yaffs2
    if (strcmp("yaffs2", mv->filesystem) == 0) {
        return mkyaffs2image_wrapper;
    }

    return default_backup_handler;
}

int nandroid_backup_partition_extended(const char* backup_path, const char* mount_point, int umount_when_finished) {

    int ret = 0;
    char name[PATH_MAX];
    char tmp[PATH_MAX];
    strcpy(name, basename(mount_point));

    //struct stat file_info;
    //sprintf(tmp, "%s/.hidenandroidprogress", backup_root);
    //int callback = stat(tmp, &file_info) != 0;
    int callback = 1;

    display_print("<~backup.partition.info>: %s\n", name);
    if (0 != (ret = ensure_path_mounted(mount_point) != 0)) {
        ui_print("Can't mount %s!\n", mount_point);
        return ret;
    }
    compute_directory_stats(mount_point);
    scan_mounted_volumes();
    Volume *v = volume_for_path(mount_point);
    const MountedVolume *mv = NULL;
    if (v != NULL)
        mv = find_mounted_volume_by_mount_point(v->mount_point);

    if (strcmp(backup_path, "-") == 0)
        sprintf(tmp, "/proc/self/fd/1");
    else if (mv == NULL || mv->filesystem == NULL)
        sprintf(tmp, "%s/%s.auto", backup_path, name);
    else
        sprintf(tmp, "%s/%s.%s", backup_path, name, mv->filesystem);
    nandroid_backup_handler backup_handler = get_backup_handler(mount_point);

    if (backup_handler == NULL) {
        ui_print("Error finding an appropriate backup handler.\n");
        return -2;
    }
    ret = backup_handler(mount_point, tmp, callback);
    if (umount_when_finished && !is_data_media_volume_path(backup_path)) {
        ensure_path_unmounted(mount_point);
    }
    if (0 != ret) {
    	display_print("<~backup.partition.fail.info>: %s\n", name);
        return ret;
    }
    ui_print("Backup of %s completed.\n", name);
    return 0;
}

#define MTK_PARTITON_INFO   "/proc/dumchar_info"
#define BOOTIMG             "bootimg"
#define RECOVERYIMG         "recovery"
#define SYSTEMIMG           "android"
#define DATAIMG             "usrdata"
#define CACHEIMG            "cache"
static int get_partition_size(const char* root) {
    char buf[512];
    char p_name[32], p_addr[32], p_size[32], p_actname[64];
    unsigned int p_type;
	long partition_sz = 0;
    FILE *fp_info;
    char *substr, *stopstring;

    fp_info = fopen(MTK_PARTITON_INFO, "r");
    if (fp_info) {
    	if (fgets(buf, sizeof(buf), fp_info) != NULL) {
            while (fgets(buf, sizeof(buf), fp_info)) {
        		if (strcmp(root, "/boot") == 0) {
        			substr = strstr(buf, BOOTIMG);
        			if (!substr)
        				continue;
        		} else if (strcmp(root, "/recovery") == 0) {
        			substr = strstr(buf, RECOVERYIMG);
        			if (!substr)
        				continue;
        		} else if (strcmp(root, "/system") == 0) {
        			substr = strstr(buf, SYSTEMIMG);
        			if (!substr)
        				continue;
        		} else if (strcmp(root, "/data") == 0) {
        			substr = strstr(buf, DATAIMG);
        			if (!substr)
        				continue;
        		} else if (strcmp(root, "/cache") == 0) {
        			substr = strstr(buf, CACHEIMG);
        			if (!substr)
        				continue;
        		} else
        			continue;
                //printf("%s", buf);
                if (sscanf(buf, "%s %s %s %d %s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
					partition_sz = strtol(p_size, &stopstring, 16);
					printf("%s size: %ld\n", root, partition_sz);
                	break;
                }
				printf("Get nonthing\n");
            }
    	}
    	fclose(fp_info);
    }
    if (partition_sz > 0)
    	return partition_sz;
    else
    	return 0;
}
int nandroid_backup_partition(const char* backup_path, const char* root) {
    Volume *vol = volume_for_path(root);
    // make sure the volume exists before attempting anything...
    if (vol == NULL || vol->fs_type == NULL)
        return 0;

    // see if we need a raw backup (mtd)
    char tmp[PATH_MAX];
    int ret;
    if (strcmp(vol->fs_type, "mtd") == 0 ||
            strcmp(vol->fs_type, "bml") == 0 ||
            strcmp(vol->fs_type, "emmc") == 0) {
        const char* name = basename(root);
        if (strcmp(backup_path, "-") == 0)
            strcpy(tmp, "/proc/self/fd/1");
        else
            sprintf(tmp, "%s/%s.img", backup_path, name);

        int partition_size = get_partition_size(root);

        display_print("<~backup.partition.info>: %s\n", name);
        if (0 != (ret = backup_raw_partition(vol->fs_type, vol->device, tmp, partition_size))) {
        	display_print("<~backup.partition.fail.inf>: %s\n", name);
            return ret;
        }

        ui_print("Backup of %s image completed.\n", name);
        return 0;
    } else if(strcmp(vol->fs_type, "vfat") == 0){
    	// for u8860 etc.
    	display_print("<~backup.partition.info>: %s\n", vol->mount_point);
		sprintf(tmp, "mkdir -p /tmp/mvfat; mount -t vfat %s /tmp/mvfat; cp -f /tmp/mvfat/image/boot.img %s; umount -l /tmp/mvfat; exit $?",
				vol->device, backup_path);
        if(0 != __system(tmp)) {
        	display_print("<~backup.partition.fail.info>: %s", vol->mount_point);
        	return 1;
        }
        return 0;
    }

    return nandroid_backup_partition_extended(backup_path, root, 1);
}

int nandroid_advanced_backup(const char* backup_path, const char *root)
{
    if (ensure_path_mounted(backup_path) != 0) {
        return print_and_error("<~backup.mount.fail.info>\n");
    }
    display_print("\n--<~backup.backup.tip.info>--\n");
    display_print("<~backup.path.info>: %s\n", backup_path);

    Volume* volume = volume_for_path(backup_path);
    if (NULL == volume) {
      if (strstr(backup_path, "/sdcard") == backup_path && is_data_media())
          volume = volume_for_path("/data");
      else
          return print_and_error("<~backup.mount.fail.info>\n");
    }
    int ret;
    struct statfs s;
    if (NULL != volume) {
        if (0 != (ret = statfs(volume->mount_point, &s)))
            return print_and_error("<~backup.mount.fail.info>\n");
        uint64_t bavail = s.f_bavail;
        uint64_t bsize = s.f_bsize;
        uint64_t sdcard_free = bavail * bsize;
        uint64_t sdcard_free_mb = sdcard_free / (uint64_t)(1024 * 1024);
        if (sdcard_free_mb > 1024) {
        	float sdcard_free_gb = (float)sdcard_free_mb /(float) 1024;
        	display_print("<~backup.sd.cap.info>: %0.1fGB\n\n", sdcard_free_gb);
        } else
        	display_print("<~backup.sd.cap.info>: %lluMB\n\n", sdcard_free_mb);

        if (sdcard_free_mb < 600 && strcmp(root, "/boot")) {
        		display_print("<~backup.sd.stop.info>\n\n");
        		return 1;
        }
        if (sdcard_free_mb < 800)
        	display_print("<~backup.sd.alert.info>\n\n");
    }
    char tmp[PATH_MAX];
    sprintf(tmp, "mkdir -p %s", backup_path);
    __system(tmp);

    if (0 != (ret = nandroid_backup_partition(backup_path, root)))
        return ret;
    display_print("<~backup.md5.gen.info>");
    sprintf(tmp, "cd %s; md5sum *.* > /tmp/nandroid.md5; cat /tmp/nandroid.md5 > nandroid.md5; exit $?", backup_path);
    if (0 != (ret = __system(tmp))) {
    	display_print("<~backup.md5.error.info>");
        return ret;
    }

    sync();
    ui_print("\nBackup complete!\n");
    return 0;

}
int nandroid_backup(const char* backup_path)
{
    nandroid_backup_bitfield = 0;
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    refresh_default_backup_handler();

    if (ensure_path_mounted(backup_path) != 0) {
        return print_and_error("<~backup.mount.fail.info>\n");
    }

    display_print("\n--<~backup.backup.tip.info>--\n");
    display_print("<~backup.path.info>: %s\n\n", backup_path);

    Volume* volume;
    if (is_data_media_volume_path(backup_path))
        volume = volume_for_path("/data");
    else
        volume = volume_for_path(backup_path);
    if (NULL == volume)
        return print_and_error("<~backup.mount.fail.info>\n");
    int ret;
    struct statfs sfs;
    struct stat s;
    if (NULL != volume) {
        if (0 != (ret = statfs(volume->mount_point, &sfs)))
            return print_and_error("<~backup.mount.fail.info>\n");;
        uint64_t bavail = sfs.f_bavail;
        uint64_t bsize = sfs.f_bsize;
        uint64_t sdcard_free = bavail * bsize;
        uint64_t sdcard_free_mb = sdcard_free / (uint64_t)(1024 * 1024);
        if (sdcard_free_mb > 1024) {
        	float sdcard_free_gb = (float)sdcard_free_mb /(float) 1024;
        	display_print("<~backup.sd.cap.info>: %0.1fGB\n\n", sdcard_free_gb);
        } else
        	display_print("<~backup.sd.cap.info>: %lluMB\n\n", sdcard_free_mb);
        if (sdcard_free_mb < 600) {
            display_print("<~backup.sd.stop.info>\n\n");
            return 1;
        }
        if (sdcard_free_mb < 800)
        	display_print("<~backup.sd.alert.info>\n\n");
    }
    ui_set_progress(0.008);
    char tmp[PATH_MAX];
    ensure_directory(backup_path);

    if (0 != (ret = nandroid_backup_partition(backup_path, "/boot")))
        return ret;
    ui_set_progress(0.016);

    Volume *vol = volume_for_path("/wimax");
    if (vol != NULL && 0 == stat(vol->device, &s))
    {
        char serialno[PROPERTY_VALUE_MAX];
        ui_print("Backing up WiMAX...\n");
        serialno[0] = 0;
        property_get("ro.serialno", serialno, "");
        sprintf(tmp, "%s/wimax.%s.img", backup_path, serialno);
        int partition_size = get_partition_size("/wimax");
        ret = backup_raw_partition(vol->fs_type, vol->device, tmp, partition_size);
        if (0 != ret)
            return print_and_error("Error while dumping WiMAX image!\n");
    }
    if (0 != (ret = nandroid_backup_partition(backup_path, "/system")))
        return ret;
    ui_set_progress(0.527);

    if (0 != (ret = nandroid_backup_partition(backup_path, "/data")))
        return ret;

    if (has_datadata()) {
        if (0 != (ret = nandroid_backup_partition(backup_path, "/datadata")))
            return ret;
    }
    ui_set_progress(0.83);

    if (is_data_media() || 0 != stat("/sdcard/.android_secure", &s)) {
        ui_print("No .android_secure found. Skipping backup of applications on external storage.\n");
    }
    else {
        if (0 != (ret = nandroid_backup_partition_extended(backup_path, "/sdcard/.android_secure", 0)))
            return ret;
    }

    if (0 != (ret = nandroid_backup_partition_extended(backup_path, "/cache", 0)))
        return ret;
    ui_set_progress(0.914);

    vol = volume_for_path("/sd-ext");
    if (vol == NULL || 0 != stat(vol->device, &s))
    {
        LOGI("No sd-ext found. Skipping backup of sd-ext.\n");
    }
    else
    {
        if (0 != ensure_path_mounted("/sd-ext"))
            LOGI("Could not mount sd-ext. sd-ext backup may not be supported on this device. Skipping backup of sd-ext.\n");
        else if (0 != (ret = nandroid_backup_partition(backup_path, "/sd-ext")))
            return ret;
    }
    sync();

    display_print("<~backup.md5.gen.info>");
    sprintf(tmp, "cd %s; md5sum *.* > /tmp/nandroid.md5; cat /tmp/nandroid.md5 > nandroid.md5; exit $?", backup_path);
    if (0 != (ret = __system(tmp))) {
    	display_print("<~backup.md5.error.info>");
        return ret;
    }
    ui_set_progress(0.99);

    sync();
    ui_set_progress(1);
    ui_set_background(BACKGROUND_ICON_NONE);
    ui_reset_progress();
    ui_print("\nBackup complete!\n");
    return 0;
}

int nandroid_dump(const char* partition) {
    // silence our ui_print statements and other logging
    ui_set_log_stdout(0);

    nandroid_backup_bitfield = 0;
    refresh_default_backup_handler();

    // override our default to be the basic tar dumper
    default_backup_handler = tar_dump_wrapper;

    if (strcmp(partition, "boot") == 0) {
        return __system("dump_image boot /proc/self/fd/1 | cat");
    }

    if (strcmp(partition, "recovery") == 0) {
        return __system("dump_image recovery /proc/self/fd/1 | cat");
    }

    if (strcmp(partition, "data") == 0) {
        return nandroid_backup_partition("-", "/data");
    }

    if (strcmp(partition, "system") == 0) {
        return nandroid_backup_partition("-", "/system");
    }

    return 1;
}

typedef int (*nandroid_restore_handler)(const char* backup_file_image, const char* backup_path, int callback);

static int unyaffs_wrapper(const char* backup_file_image, const char* backup_path, int callback) {
    gettimeofday(&lastupdate,NULL);
    return unyaffs(backup_file_image, backup_path, callback ? nandroid_callback : NULL);
}

static int do_tar_extract(char* command, int callback) {
    char buf[PATH_MAX];

    FILE *fp = __popen(command, "r");
    if (fp == NULL) {
        ui_print("Unable to execute tar command.\n");
        return -1;
    }

    while (fgets(buf, PATH_MAX, fp) != NULL) {
        buf[PATH_MAX - 1] = '\0';
        if (callback)
            nandroid_callback(buf);
    }

    return __pclose(fp);
}

static int tar_gzip_extract_wrapper(const char* backup_file_image, const char* backup_path, int callback) {
    char tmp[PATH_MAX];
    sprintf(tmp, "cd /; tar xzvf %s; exit $?", backup_file_image);
    return do_tar_extract(tmp, callback);
}

static int tar_extract_wrapper(const char* backup_file_image, const char* backup_path, int callback) {
    char tmp[PATH_MAX];
    sprintf(tmp, "cd $(dirname %s) ; cat %s* | tar xv ; exit $?", backup_path, backup_file_image);
    return do_tar_extract(tmp, callback);
}

static int dedupe_extract_wrapper(const char* backup_file_image, const char* backup_path, int callback) {
    char tmp[PATH_MAX];
    char blob_dir[PATH_MAX];
    strcpy(blob_dir, backup_file_image);
    char *bd = dirname(blob_dir);
    strcpy(blob_dir, bd);
    bd = dirname(blob_dir);
    strcpy(blob_dir, bd);
    bd = dirname(blob_dir);
    sprintf(tmp, "dedupe x %s %s/blobs %s; exit $?", backup_file_image, bd, backup_path);

    char path[PATH_MAX];
    FILE *fp = __popen(tmp, "r");
    if (fp == NULL) {
        ui_print("Unable to execute dedupe.\n");
        return -1;
    }

    while (fgets(path, PATH_MAX, fp) != NULL) {
        if (callback)
            nandroid_callback(path);
    }

    return __pclose(fp);
}

static int tar_undump_wrapper(const char* backup_file_image, const char* backup_path, int callback) {
    char tmp[PATH_MAX];
    sprintf(tmp, "cd $(dirname %s) ; tar xv ", backup_path);

    return __system(tmp);
}

static nandroid_restore_handler get_restore_handler(const char *backup_path) {
    Volume *v = volume_for_path(backup_path);
    if (v == NULL) {
        ui_print("Unable to find volume.\n");
        return NULL;
    }
    scan_mounted_volumes();
    const MountedVolume *mv = find_mounted_volume_by_mount_point(v->mount_point);
    if (mv == NULL) {
        ui_print("Unable to find mounted volume: %s\n", v->mount_point);
        return NULL;
    }

    if (strcmp(backup_path, "/data") == 0 && is_data_media()) {
        return tar_extract_wrapper;
    }

    // cwr 5, we prefer tar for everything unless it is yaffs2
    char str[255];
    char* partition;
    property_get("ro.cwm.prefer_tar", str, "false");
    if (strcmp("true", str) != 0) {
        return unyaffs_wrapper;
    }

    if (strcmp("yaffs2", mv->filesystem) == 0) {
        return unyaffs_wrapper;
    }

    return tar_extract_wrapper;
}

int nandroid_restore_partition_extended(const char* backup_path, const char* mount_point, int umount_when_finished) {
    int ret = 0;
    char* name = basename(mount_point);

    nandroid_restore_handler restore_handler = NULL;
    const char *filesystems[] = { "yaffs2", "ext2", "ext3", "ext4", "vfat", "rfs", "f2fs", NULL };
    const char* backup_filesystem = NULL;
    Volume *vol = volume_for_path(mount_point);
    const char *device = NULL;
    if (vol != NULL)
        device = vol->device;

    char tmp[PATH_MAX];
    char tmp_hp[PATH_MAX];
    sprintf(tmp, "%s/%s.img", backup_path, name);
    struct stat file_info;
    if (strcmp(backup_path, "-") == 0) {
        if (vol)
            backup_filesystem = vol->fs_type;
        restore_handler = tar_extract_wrapper;
        strcpy(tmp, "/proc/self/fd/0");
    }
    else if (0 != (ret = stat(tmp, &file_info))) {
        // can't find the backup, it may be the new backup format?
        // iterate through the backup types
        const char *filesystem;
        int i = 0;
        while ((filesystem = filesystems[i]) != NULL) {
            sprintf(tmp, "%s/%s.%s.img", backup_path, name, filesystem);
            if (0 == (ret = stat(tmp, &file_info))) {
                backup_filesystem = filesystem;
                restore_handler = unyaffs_wrapper;
                break;
            }
            sprintf(tmp, "%s/%s.%s.tar", backup_path, name, filesystem);
            if (0 == (ret = stat(tmp, &file_info))) {
                backup_filesystem = filesystem;
                restore_handler = tar_extract_wrapper;
                break;
            }
            sprintf(tmp, "%s/%s.%s.tar.gz", backup_path, name, filesystem);
            if (0 == (ret = stat(tmp, &file_info))) {
                backup_filesystem = filesystem;
                restore_handler = tar_gzip_extract_wrapper;
                break;
            }
            sprintf(tmp, "%s/%s.%s.dup", backup_path, name, filesystem);
            if (0 == (ret = stat(tmp, &file_info))) {
                backup_filesystem = filesystem;
                restore_handler = dedupe_extract_wrapper;
                break;
            }
            i++;
        }

        if (backup_filesystem == NULL || restore_handler == NULL) {
        	display_print("%s.img <~restore.image.lack.info> %s.", name, mount_point);
            return 0;
        }
        else {
            printf("Found backup image: %s\n", tmp);
        }
    }

    // If the fs_type of this volume is "auto" or mount_point is /data
    // and is_data_media, let's revert
    // to using a rm -rf, rather than trying to do a
    // ext3/ext4/whatever format.
    // This is because some phones (like DroidX) will freak out if you
    // reformat the /system or /data partitions, and not boot due to
    // a locked bootloader.
    // Other devices, like the Galaxy Nexus, XOOM, and Galaxy Tab 10.1
    // have a /sdcard symlinked to /data/media.
    // Or of volume does not exist (.android_secure), just rm -rf.
    if (vol == NULL || 0 == strcmp(vol->fs_type, "auto"))
        backup_filesystem = NULL;
    if (0 == strcmp(vol->mount_point, "/data") && is_data_media())
        backup_filesystem = NULL;

    ensure_directory(mount_point);

    //sprintf(tmp_hp, "%s/.hidenandroidprogress", backup_root);
    //int callback = stat(tmp_hp, &file_info) != 0;
    int callback = 1;

    display_print("<~restore.partition.info>: %s", name);
    if (backup_filesystem == NULL) {
        if (0 != (ret = format_volume(mount_point))) {
        	display_print("<~restore.format.fail.info>: %s", name);
            return ret;
        }
    }
    else if (0 != (ret = format_device(device, mount_point, backup_filesystem))) {
        display_print("<~restore.format.fail.info>: %s", name);
        return ret;
    }

    curpos += 0.080;
    ui_set_progress(curpos);

    if (0 != (ret = ensure_path_mounted(mount_point))) {
    	display_print("<~restore.mount.fail.info>: %s", name);
        return ret;
    }

    if (restore_handler == NULL)
        restore_handler = get_restore_handler(mount_point);

    // override restore handler for undump
    if (strcmp(backup_path, "-") == 0) {
        restore_handler = tar_undump_wrapper;
    }

    if (restore_handler == NULL) {
        ui_print("Error finding an appropriate restore handler.\n");
        return -2;
    }

    if (0 != (ret = restore_handler(tmp, mount_point, callback))) {
    	display_print("<~restore.partition.fail.info>: %s", name);
        return ret;
    }

    if (umount_when_finished && !is_data_media_volume_path(backup_path)) {
        ensure_path_unmounted(mount_point);
    }

    return 0;
}

int nandroid_restore_partition(const char* backup_path, const char* root) {
    Volume *vol = volume_for_path(root);
    // make sure the volume exists...
    if (vol == NULL || vol->fs_type == NULL)
        return 0;

    // see if we need a raw restore (mtd)
    char tmp[PATH_MAX];
    if (strcmp(vol->fs_type, "mtd") == 0 ||
            strcmp(vol->fs_type, "bml") == 0 ||
            strcmp(vol->fs_type, "emmc") == 0) {
        int ret;
        const char* name = basename(root);
        ui_print("Erasing %s before restore...\n", name);
        if (0 != (ret = format_volume(root))) {
            ui_print("Error while erasing %s image!", name);
            return ret;
        }

        if (strcmp(backup_path, "-") == 0)
            strcpy(tmp, backup_path);
        else
            sprintf(tmp, "%s%s.img", backup_path, root);

        display_print("<~restore.partition.info>: %s", name);
        if (0 != (ret = restore_raw_partition(vol->fs_type, vol->device, tmp))) {
        	display_print("<~restore.partition.fail.info>: %s", name);
            return ret;
        }
        return 0;
    } else if (strcmp(vol->fs_type, "vfat") == 0) {
    	display_print("<~restore.partition.info>: %s", vol->mount_point);
        sprintf(tmp, "mkdir -p /tmp/mvfat; mount -t vfat %s /tmp/mvfat; cp -f %s/boot.img /tmp/mvfat/image/; umount -l /tmp/mvfat; exit $?",
        		vol->device, backup_path);
        if(0 != __system(tmp)) {
        	display_print("<~restore.partition.fail.info>: %s", vol->mount_point);
        	return 1;
        }
        return 0;
    }
    return nandroid_restore_partition_extended(backup_path, root, 1);
}

int nandroid_restore(const char* backup_path, int restore_boot, int restore_system, int restore_data, int restore_cache, int restore_sdext, int restore_wimax)
{
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    ui_show_indeterminate_progress();
    nandroid_files_total = 0;

    ui_set_progress(0);

    if (ensure_path_mounted(backup_path) != 0)
        return print_and_error("<~backup.mount.error.info>");

    int ret = get_battery_capacity();
    if(((ret >= 0) && (ret < LOW_BATTERY)) && !usb_connected()){
    	display_print("\n<~restore.battery.alert.info> %d%%(%d%%)\n", LOW_BATTERY, ret);
    	display_print("<~restore.battery.tip.info>\n");
    }

    ui_set_progress(0.012);

    display_print("\n--<~restore.restore.begin.info>--\n");
    display_print("<~restore.path.info>: %s\n", backup_path);
    display_print("\n*<~restore.restore.tip.info>*\n\n");

    char tmp[PATH_MAX];
    display_print("<~restore.md5.check.info>");
    sprintf(tmp, "cd %s && md5sum -c nandroid.md5", backup_path);
    if (0 != __system(tmp)) {
    	ddProgress_set_pause(1);
   		ret = dd_confirm(3, "<~restore.md5.check.fail.title>", "<~restore.md5.check.fail.info>\n\n<~restore.md5.check.fail.ask>", NULL);
		ddProgress_set_pause(0);
    	if (ret != RET_YES)
    		return print_and_error("\n<~restore.md5.error.info>\n");
    	else {
    		display_print("\n<~restore.md5.check.fail.continue>\n");
    	}
    }

    ui_set_progress(0.090);

    if (restore_boot && NULL != volume_for_path("/boot") && 0 != (ret = nandroid_restore_partition(backup_path, "/boot")))
        return ret;

    struct stat s;
    Volume *vol = volume_for_path("/wimax");
    if (restore_wimax && vol != NULL && 0 == stat(vol->device, &s))
    {
        char serialno[PROPERTY_VALUE_MAX];

        serialno[0] = 0;
        property_get("ro.serialno", serialno, "");
        sprintf(tmp, "%s/wimax.%s.img", backup_path, serialno);

        struct stat st;
        if (0 != stat(tmp, &st))
        {
            ui_print("WARNING: WiMAX partition exists, but nandroid\n");
            ui_print("         backup does not contain WiMAX image.\n");
            ui_print("         You should create a new backup to\n");
            ui_print("         protect your WiMAX keys.\n");
        }
        else
        {
            ui_print("Erasing WiMAX before restore...\n");
            if (0 != (ret = format_volume("/wimax")))
                return print_and_error("Error while formatting wimax!\n");
            ui_print("Restoring WiMAX image...\n");
            if (0 != (ret = restore_raw_partition(vol->fs_type, vol->device, tmp)))
                return ret;
        }
    }

    curpos = 0.138;
    ui_set_progress(curpos);

    if (restore_system && 0 != (ret = nandroid_restore_partition(backup_path, "/system")))
        return ret;

    curpos = 0.474;
    ui_set_progress(curpos);

    if (restore_data && 0 != (ret = nandroid_restore_partition(backup_path, "/data")))
        return ret;

    if (has_datadata()) {
        if (restore_data && 0 != (ret = nandroid_restore_partition(backup_path, "/datadata")))
            return ret;
    }

    curpos = 0.836;
    ui_set_progress(curpos);

    if (restore_data && 0 != (ret = nandroid_restore_partition_extended(backup_path, "/sdcard/.android_secure", 0)))
        return ret;

    if (restore_cache && 0 != (ret = nandroid_restore_partition_extended(backup_path, "/cache", 0)))
        return ret;

    if (restore_sdext && 0 != (ret = nandroid_restore_partition(backup_path, "/sd-ext")))
        return ret;

    curpos = 0.977;
    ui_set_progress(curpos);

    sync();

    curpos = 0.990;
    ui_set_progress(curpos);

    ui_set_background(BACKGROUND_ICON_NONE);
    ui_reset_progress();
    ui_print("\nRestore complete!\n");
    return 0;
}

int nandroid_backup_manage(const char* path) {
	char tmp[PATH_MAX];
	int ret = 0;
	sprintf(tmp, "busybox rm -rf %s", path);
	if(0 != (ret = __system(tmp))){
		//try again
		if(0 != (ret = __system(tmp)))
			return 1;
	}
	return 0;
}

int nandroid_undump(const char* partition) {
    nandroid_files_total = 0;

    int ret;

    if (strcmp(partition, "boot") == 0) {
        return __system("flash_image boot /proc/self/fd/0");
    }

    if (strcmp(partition, "recovery") == 0) {
        if(0 != (ret = nandroid_restore_partition("-", "/recovery")))
            return ret;
    }

    if (strcmp(partition, "system") == 0) {
        if(0 != (ret = nandroid_restore_partition("-", "/system")))
            return ret;
    }

    if (strcmp(partition, "data") == 0) {
        if(0 != (ret = nandroid_restore_partition("-", "/data")))
            return ret;
    }

    sync();
    return 0;
}

int nandroid_usage()
{
    printf("Usage: nandroid backup\n");
    printf("Usage: nandroid restore <directory>\n");
    printf("Usage: nandroid dump <partition>\n");
    printf("Usage: nandroid undump <partition>\n");
    return 1;
}

static int bu_usage() {
    printf("Usage: bu <fd> backup partition\n");
    printf("Usage: Prior to restore:\n");
    printf("Usage: echo -n <partition> > /tmp/ro.bu.restore\n");
    printf("Usage: bu <fd> restore\n");
    return 1;
}

int bu_main(int argc, char** argv) {
    load_volume_table();

    if (strcmp(argv[2], "backup") == 0) {
        if (argc != 4) {
            return bu_usage();
        }

        int fd = atoi(argv[1]);
        char* partition = argv[3];

        if (fd != STDOUT_FILENO) {
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // fprintf(stderr, "%d %d %s\n", fd, STDOUT_FILENO, argv[3]);
        int ret = nandroid_dump(partition);
        sleep(10);
        return ret;
    }
    else if (strcmp(argv[2], "restore") == 0) {
        if (argc != 3) {
            return bu_usage();
        }

        int fd = atoi(argv[1]);
        if (fd != STDIN_FILENO) {
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        char partition[100];
        FILE* f = fopen("/tmp/ro.bu.restore", "r");
        fread(partition, 1, sizeof(partition), f);
        fclose(f);

        // fprintf(stderr, "%d %d %s\n", fd, STDIN_FILENO, argv[3]);
        return nandroid_undump(partition);
    }

    return bu_usage();
}

int nandroid_main(int argc, char** argv)
{
    load_volume_table();
    char backup_path[PATH_MAX];

    if (argc > 3 || argc < 2)
        return nandroid_usage();

    if (strcmp("backup", argv[1]) == 0)
    {
        if (argc != 2)
            return nandroid_usage();

        nandroid_generate_timestamp_path(backup_path);
        return nandroid_backup(backup_path);
    }

    if (strcmp("restore", argv[1]) == 0)
    {
        if (argc != 3)
            return nandroid_usage();
        return nandroid_restore(argv[2], 1, 1, 1, 1, 1, 0);
    }

    if (strcmp("dump", argv[1]) == 0)
    {
        if (argc != 3)
            return nandroid_usage();
        return nandroid_dump(argv[2]);
    }

    if (strcmp("undump", argv[1]) == 0)
    {
        if (argc != 3)
            return nandroid_usage();
        return nandroid_undump(argv[2]);
    }

    return nandroid_usage();
}
