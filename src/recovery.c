/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  * See the License for the specific language governing permissions and * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "install.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"
#include "ddui/src/dd.h"
#include "dd_op.h"
#include "mtdutils/mounts.h"
#include "extendedcommands.h"

#include "nandroid.h"
static const struct option OPTIONS[] = {
  { "send_intent", required_argument, NULL, 's' },
  { "update_package", required_argument, NULL, 'u' },
  { "update_baidu_package", required_argument, NULL, 'b' },
  { "wipe_data", no_argument, NULL, 'w' },
  { "wipe_cache", no_argument, NULL, 'c' },
  { "show_text", no_argument, NULL, 't' },
  { "backup", required_argument, NULL, 'k' },
  { "restore", required_argument, NULL, 'e' },
  { NULL, 0, NULL, 0 },
};

static const char *APP_LIST_FILE = "/data/local/tmp/app.list";
static const char *BD_COMMAND_FILE = "/data/local/tmp/rom_flash.command";
static const char *COMMAND_FILE = "/cache/recovery/command";
static const char *INTENT_FILE = "/cache/recovery/intent";
static const char *LOG_FILE = "/cache/recovery/log";
static const char *LAST_LOG_FILE = "/cache/recovery/last_log";
static const char *LAST_INSTALL_FILE = "/cache/recovery/last_install";
static const char *BACKUP_PATH_CONFIG_CACHE = "/data/local/tmp/backup.config";
static const char *CACHE_ROOT = "/cache";
static const char *SDCARD_ROOT = "/sdcard";
static const char *TEMPORARY_DIR = "/tmp";
static const char *TEMPORARY_LOG_FILE = "/tmp/recovery.log";
static const char *TEMPORARY_INSTALL_FILE = "/tmp/last_install";
static const char *SIDELOAD_TEMP_DIR = "/tmp/sideload";

extern int __system(const char *command);


/*
 * The recovery tool communicates with the main system through /cache files.
 *   /cache/recovery/command - INPUT - command line for tool, one arg per line
 *   /cache/recovery/log - OUTPUT - combined log file from recovery run(s)
 *   /cache/recovery/intent - OUTPUT - intent that was passed in
 *
 * The arguments which may be supplied in the recovery.command file:
 *   --send_intent=anystring - write the text out to recovery.intent
 *   --update_package=path - verify install an OTA package file
 *   --wipe_data - erase user data (and cache), then reboot
 *   --wipe_cache - wipe cache (but not user data), then reboot
 *   --set_encrypted_filesystem=on|off - enables / diasables encrypted fs
 *
 * After completing, we remove /cache/recovery/command and reboot.
 * Arguments may also be supplied in the bootloader control block (BCB).
 * These important scenarios must be safely restartable at any point:
 *
 * FACTORY RESET
 * 1. user selects "factory reset"
 * 2. main system writes "--wipe_data" to /cache/recovery/command
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and "--wipe_data"
 *    -- after this, rebooting will restart the erase --
 * 5. erase_volume() reformats /data
 * 6. erase_volume() reformats /cache
 * 7. finish_recovery() erases BCB
 *    -- after this, rebooting will restart the main system --
 * 8. main() calls reboot() to boot main system
 *
 * OTA INSTALL
 * 1. main system downloads OTA package to /cache/some-filename.zip
 * 2. main system writes "--update_package=/cache/some-filename.zip"
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and "--update_package=..."
 *    -- after this, rebooting will attempt to reinstall the update --
 * 5. install_package() attempts to install the update
 *    NOTE: the package install must itself be restartable from any point
 * 6. finish_recovery() erases BCB
 *    -- after this, rebooting will (try to) restart the main system --
 * 7. ** if install failed **
 *    7a. prompt_and_wait() shows an error icon and waits for the user
 *    7b; the user reboots (pulling the battery, etc) into the main system
 * 8. main() calls maybe_install_firmware_update()
 *    ** if the update contained radio/hboot firmware **:
 *    8a. m_i_f_u() writes BCB with "boot-recovery" and "--wipe_cache"
 *        -- after this, rebooting will reformat cache & restart main system --
 *    8b. m_i_f_u() writes firmware image into raw cache partition
 *    8c. m_i_f_u() writes BCB with "update-radio/hboot" and "--wipe_cache"
 *        -- after this, rebooting will attempt to reinstall firmware --
 *    8d. bootloader tries to flash firmware
 *    8e. bootloader writes BCB with "boot-recovery" (keeping "--wipe_cache")
 *        -- after this, rebooting will reformat cache & restart main system --
 *    8f. erase_volume() reformats /cache
 *    8g. finish_recovery() erases BCB
 *        -- after this, rebooting will (try to) restart the main system --
 * 9. main() calls reboot() to boot main system
 */

static const int MAX_ARG_LENGTH = 4096;
static const int MAX_ARGS = 100;

void display_print(const char* fmt, ...) {
    char buf[PATH_MAX];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, PATH_MAX, fmt, ap);
    va_end(ap);
    ddProgress_set_text(buf);
}

// open a given path, mounting partitions as necessary
FILE*
fopen_path(const char *path, const char *mode) {
    if (ensure_path_mounted(path) != 0) {
        LOGE("Can't mount %s\n", path);
        return NULL;
    }

    // When writing, try to create the containing directory, if necessary.
    // Use generous permissions, the system (init.rc) will reset them.
    if (strchr("wa", mode[0])) dirCreateHierarchy(path, 0777, NULL, 1);

    FILE *fp = fopen(path, mode);
    return fp;
}

// close a file, log an error if the error indicator is set
static void
check_and_fclose(FILE *fp, const char *name) {
    fflush(fp);
    if (ferror(fp)) LOGE("Error in %s\n(%s)\n", name, strerror(errno));
    fclose(fp);
}

// command line args come from, in decreasing precedence:
//   - the actual command line
//   - the bootloader control block (one per line, after "recovery")
//   - the contents of COMMAND_FILE (one per line)
static void
get_args(int *argc, char ***argv) {
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    get_bootloader_message(&boot);  // this may fail, leaving a zeroed structure

    if (boot.command[0] != 0 && boot.command[0] != 255) {
        LOGI("Boot command: %.*s\n", sizeof(boot.command), boot.command);
    }

    if (boot.status[0] != 0 && boot.status[0] != 255) {
        LOGI("Boot status: %.*s\n", sizeof(boot.status), boot.status);
    }

    // --- if arguments weren't supplied, look in the bootloader control block
    if (*argc <= 1) {
        boot.recovery[sizeof(boot.recovery) - 1] = '\0';  // Ensure termination
        const char *arg = strtok(boot.recovery, "\n");
        if (arg != NULL && !strcmp(arg, "recovery")) {
            *argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
            (*argv)[0] = strdup(arg);
            for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
                if ((arg = strtok(NULL, "\n")) == NULL) break;
                (*argv)[*argc] = strdup(arg);
            }
            LOGI("Got arguments from boot message\n");
        } else if (boot.recovery[0] != 0 && boot.recovery[0] != 255) {
            LOGE("Bad boot message\n\"%.20s\"\n", boot.recovery);
        }
    }

    // --- if that doesn't work, try the command file
    if (*argc <= 1) {
        FILE *fp = fopen_path(COMMAND_FILE, "r");
        if (fp != NULL) {
            char *argv0 = (*argv)[0];
            *argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
            (*argv)[0] = argv0;  // use the same program name

            char buf[MAX_ARG_LENGTH];
            for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
                if (!fgets(buf, sizeof(buf), fp)) break;
                (*argv)[*argc] = strdup(strtok(buf, "\r\n"));  // Strip newline.
            }

            check_and_fclose(fp, COMMAND_FILE);
            LOGI("Got arguments from %s\n", COMMAND_FILE);
        }
    } else {
        ensure_path_mounted("/cache");
    }

    if (*argc <= 1) {
        FILE *fp = fopen_path(BD_COMMAND_FILE, "r");
        if (fp != NULL) {
            char *argv0 = (*argv)[0];
            *argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
            (*argv)[0] = argv0;  // use the same program name

            char buf[MAX_ARG_LENGTH];
            for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
                if (!fgets(buf, sizeof(buf), fp)) break;
                (*argv)[*argc] = strdup(strtok(buf, "\r\n"));  // Strip newline.
            }

            check_and_fclose(fp, BD_COMMAND_FILE);
            LOGI("Got arguments from %s\n", BD_COMMAND_FILE);
        }
    }

    // --> write the arguments we have back into the bootloader control block
    // always boot into recovery after this (until finish_recovery() is called)
    strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
    strlcpy(boot.recovery, "recovery\n", sizeof(boot.recovery));
    int i;
    for (i = 1; i < *argc; ++i) {
        strlcat(boot.recovery, (*argv)[i], sizeof(boot.recovery));
        strlcat(boot.recovery, "\n", sizeof(boot.recovery));
    }
    set_bootloader_message(&boot);
}

extern char backup_root[PATH_MAX];

static void
copy_backup_config_file(const char* source, const char* destination) {
    FILE *conf = fopen_path(destination, "w");
    if (conf == NULL) {
        LOGE("Can't open %s\n", destination);
    } else {
    	//LOGI("Copy %s to %s\n", source, destination);
        FILE *tmpconf = fopen(source, "r");
        if (tmpconf != NULL) {
            char buf[4096];
            fgets(buf, sizeof(buf), tmpconf);
            fputs(buf, conf);
            check_and_fclose(tmpconf, source);
        }
        check_and_fclose(conf, destination);
    }
}

static void
get_backup_path_last() {
	printf("Loading saved backup path configure file\n");
	FILE *fp = fopen_path(BACKUP_PATH_CONFIG_CACHE, "r");
	if (fp != NULL) {
        char buf[MAX_ARG_LENGTH];
        if (!fgets(backup_root, PATH_MAX, fp))
            printf("Loading backup path failed\n");
        copy_backup_config_file(BACKUP_PATH_CONFIG_CACHE, BACKUP_PATH_CONFIG);
    	check_and_fclose(fp, BACKUP_PATH_CONFIG_CACHE);
    	printf("Get backup path: %s\n", backup_root);
	} else {
		printf("%s not found.\n", BACKUP_PATH_CONFIG_CACHE);
	}
}

static void
set_sdcard_update_bootloader_message() {
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
    strlcpy(boot.recovery, "recovery\n", sizeof(boot.recovery));
    set_bootloader_message(&boot);
}

// How much of the temp log we have copied to the copy in cache.
static long tmplog_offset = 0;

static void
copy_log_file(const char* source, const char* destination, int append) {
    FILE *log = fopen_path(destination, append ? "a" : "w");
    if (log == NULL) {
        LOGE("Can't open %s\n", destination);
    } else {
        FILE *tmplog = fopen(source, "r");
        if (tmplog != NULL) {
            if (append) {
                fseek(tmplog, tmplog_offset, SEEK_SET);  // Since last write
            }
            char buf[4096];
            while (fgets(buf, sizeof(buf), tmplog)) fputs(buf, log);
            if (append) {
                tmplog_offset = ftell(tmplog);
            }
            check_and_fclose(tmplog, source);
        }
        check_and_fclose(log, destination);
    }
}


// clear the recovery command and prepare to boot a (hopefully working) system,
// copy our log file to cache as well (for the system to read), and
// record any intent we were asked to communicate back to the system.
// this function is idempotent: call it as many times as you like.
static void
finish_recovery(const char *send_intent) {
    // By this point, we're ready to return to the main system...
    if (send_intent != NULL) {
        FILE *fp = fopen_path(INTENT_FILE, "w");
        if (fp == NULL) {
            LOGE("Can't open %s\n", INTENT_FILE);
        } else {
            fputs(send_intent, fp);
            check_and_fclose(fp, INTENT_FILE);
        }
    }

    // Copy logs to cache so the system can find out what happened.
    copy_log_file(TEMPORARY_LOG_FILE, LOG_FILE, true);
    copy_log_file(TEMPORARY_LOG_FILE, LAST_LOG_FILE, false);
    copy_log_file(TEMPORARY_INSTALL_FILE, LAST_INSTALL_FILE, false);
    copy_log_file(VERSION_TMP_PATH, VERSION_DATA_PATH, false);
    copy_backup_config_file(BACKUP_PATH_CONFIG, BACKUP_PATH_CONFIG_CACHE);
    chmod(LOG_FILE, 0600);
    chown(LOG_FILE, 1000, 1000);   // system user
    chmod(LAST_LOG_FILE, 0640);
    chmod(LAST_INSTALL_FILE, 0644);
    chmod(BACKUP_PATH_CONFIG_CACHE, 0644);
    chmod(VERSION_DATA_PATH, 0644);

    // Reset to mormal system boot so recovery won't cycle indefinitely.
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    set_bootloader_message(&boot);

    // Remove the command file, so recovery won't repeat indefinitely.
    if (ensure_path_mounted(COMMAND_FILE) != 0 ||
        (unlink(COMMAND_FILE) && errno != ENOENT)) {
        LOGW("Can't unlink %s\n", COMMAND_FILE);
    }

    ensure_path_unmounted(CACHE_ROOT);
    sync();  // For good measure.
}

static int
erase_volume(const char *volume) {
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    ui_show_indeterminate_progress();
    ui_print("Formatting %s...\n", volume);

    ensure_path_unmounted(volume);

    if (strcmp(volume, "/cache") == 0) {
        // Any part of the log we'd copied to cache is now gone.
        // Reset the pointer so we copy from the beginning of the temp
        // log.
        tmplog_offset = 0;
    }

    return format_volume(volume);
}

static char*
copy_sideloaded_package(const char* original_path) {
  if (ensure_path_mounted(original_path) != 0) {
    LOGE("Can't mount %s\n", original_path);
    return NULL;
  }

  if (ensure_path_mounted(SIDELOAD_TEMP_DIR) != 0) {
    LOGE("Can't mount %s\n", SIDELOAD_TEMP_DIR);
    return NULL;
  }

  if (mkdir(SIDELOAD_TEMP_DIR, 0700) != 0) {
    if (errno != EEXIST) {
      LOGE("Can't mkdir %s (%s)\n", SIDELOAD_TEMP_DIR, strerror(errno));
      return NULL;
    }
  }

  // verify that SIDELOAD_TEMP_DIR is exactly what we expect: a
  // directory, owned by root, readable and writable only by root.
  struct stat st;
  if (stat(SIDELOAD_TEMP_DIR, &st) != 0) {
    LOGE("failed to stat %s (%s)\n", SIDELOAD_TEMP_DIR, strerror(errno));
    return NULL;
  }
  if (!S_ISDIR(st.st_mode)) {
    LOGE("%s isn't a directory\n", SIDELOAD_TEMP_DIR);
    return NULL;
  }
  if ((st.st_mode & 0777) != 0700) {
    LOGE("%s has perms %o\n", SIDELOAD_TEMP_DIR, st.st_mode);
    return NULL;
  }
  if (st.st_uid != 0) {
    LOGE("%s owned by %lu; not root\n", SIDELOAD_TEMP_DIR, st.st_uid);
    return NULL;
  }

  char copy_path[PATH_MAX];
  strcpy(copy_path, SIDELOAD_TEMP_DIR);
  strcat(copy_path, "/package.zip");

  char* buffer = malloc(BUFSIZ);
  if (buffer == NULL) {
    LOGE("Failed to allocate buffer\n");
    return NULL;
  }

  size_t read;
  FILE* fin = fopen(original_path, "rb");
  if (fin == NULL) {
    LOGE("Failed to open %s (%s)\n", original_path, strerror(errno));
    return NULL;
  }
  FILE* fout = fopen(copy_path, "wb");
  if (fout == NULL) {
    LOGE("Failed to open %s (%s)\n", copy_path, strerror(errno));
    return NULL;
  }

  while ((read = fread(buffer, 1, BUFSIZ, fin)) > 0) {
    if (fwrite(buffer, 1, read, fout) != read) {
      LOGE("Short write of %s (%s)\n", copy_path, strerror(errno));
      return NULL;
    }
  }

  free(buffer);

  if (fclose(fout) != 0) {
    LOGE("Failed to close %s (%s)\n", copy_path, strerror(errno));
    return NULL;
  }

  if (fclose(fin) != 0) {
    LOGE("Failed to close %s (%s)\n", original_path, strerror(errno));
    return NULL;
  }

  // "adb push" is happy to overwrite read-only files when it's
  // running as root, but we'll try anyway.
  if (chmod(copy_path, 0400) != 0) {
    LOGE("Failed to chmod %s (%s)\n", copy_path, strerror(errno));
    return NULL;
  }

  return strdup(copy_path);
}
/*
 * for register operation
 */
#define return_op_result_if_fail(p) if (!(p)) \
    {LOGE("function %s(line %d) " #p "\n", __FUNCTION__, __LINE__); \
    op_result.ret = -1; return &op_result;}
#define return_op_ok(val, str) op_result.ret = val; \
    if (str != NULL) \
        strncpy(op_result.result, str, OP_RESULT_LEN); \
     else op_result.result[0] = '\0'; \
     return &op_result

extern int global_signature_switch;
//OP_SIGNATURE, signature setting
static opResult* op_signature(int argc, char* argv[])
{
	//0printf("Current status: %d,Command: %s %s\n", global_signature_switch, argc, argv[0], argv[1]);
	return_op_result_if_fail(argc == 2);
    return_op_result_if_fail(argv != NULL);
    int result = 0;
    if (strncmp(argv[0], "get", 3) == 0) {
    	result = global_signature_switch;
    } else if (strncmp(argv[0], "set", 3) == 0) {
    	global_signature_switch = atoi(argv[1]);
    	result = global_signature_switch;
    } else {
    	result = -1;
    	return ddOp_result_set(result, "failed");
    }
    return ddOp_result_set(result, "done");
}

//OP_MOUNT, mount recovery.fstab
static opResult* op_mount(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = ensure_path_mounted(argv[0]);
    if (result == 0)
        return ddOp_result_set(result, "mounted");
    return ddOp_result_set(result, "fail");
}

//OP_ISMOUNT, mount recovery.fstab
static opResult* op_ismount(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = is_path_mounted(argv[0]);
    return ddOp_result_set(result, NULL);
}
//OP_MOUNT, mount recovery.fstab
static opResult* op_unmount(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = ensure_path_unmounted(argv[0]);
    if (result == 0)
        return ddOp_result_set(result, "ok");
    return ddOp_result_set(result, "fail");
}
//OP_WIPE ,wipe "/data" | "cache" "dalvik-cache"
static opResult* op_wipe(int argc, char *argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = 0;
    if (!strcmp(argv[0], "dalvik-cache"))
    {
        ensure_path_mounted("/data");
        ensure_path_mounted("/cache");
        __system("rm -r /data/dalvik-cache");
        __system("rm -r /cache/dalvik-cache");
        result = 0;
    }
    else {
        result = erase_volume(argv[0]);
    }
    assert_ui_if_fail(result == 0);
    return ddOp_result_set(result, "ok");
}
//OP_WIPE ,format "/data" | "/cache" | "/system" "/sdcard"
static opResult* op_format(int argc, char *argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = format_volume(argv[0]);
    assert_ui_if_fail(result == 0);
    return ddOp_result_set(result, "ok");
}
//OP_REBOOT, reboot, 0, NULL | reboot, 1, "recovery" | bootloader |
static opResult * op_reboot(int argc, char *argv[])
{
    return_op_result_if_fail(argc == 1);
    finish_recovery(NULL);
    if(strstr(argv[0], "reboot") != NULL) {
        android_reboot(ANDROID_RB_RESTART, 0, 0);

    } else if(strstr(argv[0], "poweroff") != NULL) {
        android_reboot(ANDROID_RB_POWEROFF, 0, 0);
    }

    else android_reboot(ANDROID_RB_RESTART2, 0, argv[0]);

    return ddOp_result_set(0, NULL);
}
//OP_INSTALL install path, wipe_cache, install_file
static opResult* op_install(int argc, char *argv[])
{
    return_op_result_if_fail(argc == 3);
    return_op_result_if_fail(argv != NULL);
    int status = install_package(argv[0], argv[1], TEMPORARY_INSTALL_FILE);
    return ddOp_result_set(status, NULL);
}
/*package nandroid_restore(const char* backup_path, restore_boot, restore_system, restore_data,
 *                         restore_cache, restore_sdext, restore_wimax);
 *OP_RESTORE
 *op_restore(argc, argv[0],...,argv[6])
 */
static opResult* op_restore(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 7);
    return_op_result_if_fail(argv != NULL);
    int result = nandroid_restore(argv[0], atoi(argv[1]), atoi(argv[2]), atoi(argv[3]),
                                  atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    return ddOp_result_set(result, NULL);
}
/*
 *nandroid_backup(backup_path);
 *
 *OP_BACKUP 
 *op_backup(argc, NULL);
 */
static opResult* op_backup(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 1);
    int result ;
    result = nandroid_backup(argv[0]);
    if (result) {
        char cmd[PATH_MAX];
        sprintf(cmd, "busybox rm -rf %s", argv[0]);
        __system(cmd);
    }
    return ddOp_result_set(result, NULL);
}

static opResult* op_advanced_backup(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 2);
    return_op_result_if_fail(argv != NULL);
    int result = nandroid_advanced_backup(argv[0], argv[1]);
    if (result) {
        char cmd[PATH_MAX];
        sprintf(cmd, "busybox rm -rf %s", argv[0]);
        __system(cmd);
    }

    return ddOp_result_set(result, NULL);
}

//OP_BACKUP_MANAGE, manage backup files
static opResult* op_backup_manage(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = nandroid_backup_manage(argv[0]);
    if (result == 0)
        return ddOp_result_set(result, "success");
    return ddOp_result_set(result, "fail");
}

static opResult* op_system(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 1);
    return_op_result_if_fail(argv != NULL);
    int result = __system(argv[0]);
    assert_if_fail(result == 0);
    return ddOp_result_set(result, NULL);
}
//copy_log_file(src_file, dst_file, append);
static opResult* op_copy(int argc, char* argv[])
{
    return_op_result_if_fail(argc == 2);
    return_op_result_if_fail(argv != NULL);
    copy_log_file(argv[0], argv[1], false);
    return ddOp_result_set(0, NULL);
}

static opResult* op_sideload(int argc, char* argv[])
{
	int result = apply_from_adb();
    return ddOp_result_set(result, NULL);
}

//OP_TOGGLE toggle usb storage
opResult* op_toggle_usb(int argc, char *argv[])
{
    assert_ui_if_fail(argc == 1);
    assert_ui_if_fail(argv[0] != NULL);
    int op_type = atoi(argv[0]);
    int result = 0;

    if (op_type == 0)
    {
    	set_usb_storage_disable();
        ensure_path_unmounted("/sdcard");
        ensure_path_unmounted("/sdcard2");
        return ddOp_result_set(result, "ok");
    }
    //wait for usb connected
    //while (is_usb_connected()) ;
    if (is_usb_connected())
    {
        set_usb_storage_enable();
        return ddOp_result_set(result, "mounted");
    }
    LOGE("USB not connected\n");
    set_usb_storage_disable();
    ensure_path_unmounted("/sdcard");
    ensure_path_unmounted("/sdcard2");
    return ddOp_result_set(result, "ok");
}


static void
print_property(const char *key, const char *name, void *cookie) {
    printf("%s=%s\n", key, name);
}

int
main(int argc, char **argv) {

	if (argc == 2 && strcmp(argv[1], "adbd") == 0) {
		adb_main();
		return 0;
	}

    if (strcmp(basename(argv[0]), "recovery") != 0)
    {
        return busybox_driver(argc, argv);
    }

    umask(0);

    time_t start = time(NULL);

    mkdir(TEMPORARY_DIR, "0777");
    freopen(TEMPORARY_LOG_FILE, "a", stdout); setbuf(stdout, NULL);
    freopen(TEMPORARY_LOG_FILE, "a", stderr); setbuf(stderr, NULL);
    printf("Starting recovery on %s", ctime(&start));
    if(!huawei_charge_detect())
    	return 0;

    load_volume_table();
    process_volumes();

    get_backup_path_last();

    //ddOp init
    ddOp_init(15);
    ddOp_register(OP_SIGNATURE, &op_signature);
    ddOp_register(OP_MOUNT, &op_mount);
    ddOp_register(OP_ISMOUNT, &op_ismount);
    ddOp_register(OP_UNMOUNT, &op_unmount);
    ddOp_register(OP_REBOOT, &op_reboot);
    ddOp_register(OP_INSTALL, &op_install);
    ddOp_register(OP_WIPE, &op_wipe);
    ddOp_register(OP_TOGGLE, &op_toggle_usb);
    ddOp_register(OP_FORMAT, &op_format);
    ddOp_register(OP_RESTORE, &op_restore);
    ddOp_register(OP_BACKUP, &op_backup);
    ddOp_register(OP_ADVANCED_BACKUP, &op_advanced_backup);
    ddOp_register(OP_BACKUP_MANAGE, &op_backup_manage);
    ddOp_register(OP_SYSTEM, &op_system);
    ddOp_register(OP_COPY, &op_copy);
    ddOp_register(OP_SIDELOAD, &op_sideload);

    device_ui_init();
    get_args(&argc, &argv);


    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    set_bootloader_message(&boot);

    int previous_runs = 0;
    const char *send_intent = NULL;
    const char *update_package = NULL;
    const char *update_baidu_package = NULL;
    const char *backup_cmd = NULL;
    const char *restore_cmd = NULL;

    int wipe_data = 0, wipe_cache = 0;

    int arg;
    while ((arg = getopt_long(argc, argv, "", OPTIONS, NULL)) != -1) {
        switch (arg) {
        case 'p': previous_runs = atoi(optarg); break;
        case 's': send_intent = optarg; break;
        case 'u': update_package = optarg; break;
        case 'b': update_baidu_package = optarg; break;
        case 'w': wipe_data = wipe_cache = 1; break;
        case 'c': wipe_cache = 1; break;
        case 'k': backup_cmd = optarg; break;
        case 'e': restore_cmd = optarg; break;
        //case 't': ui_show_text(1); break;
        case '?':
            LOGE("Invalid command argument\n");
            continue;
        }
    }

    device_recovery_start();

    printf("Command:");
    for (arg = 0; arg < argc; arg++) {
        printf(" \"%s\"", argv[arg]);
    }
    printf("\n");
    property_list(print_property, NULL);
    printf("\n");

    if(update_baidu_package){
        // ui_show_text(1); // open this for debug
		ui_print("Install package:%s\n", update_baidu_package);
        int ip_res = install_start(update_baidu_package, wipe_cache, TEMPORARY_INSTALL_FILE, 0, 1);
        sync();

        struct stat info;
        int rm_res;
        if(INSTALL_SUCCESS == ip_res) {
            if((0 == ensure_path_mounted(APP_LIST_FILE)) && (0 == stat(APP_LIST_FILE, &info))) {
                rm_res = remove_system_app("/system/app", APP_LIST_FILE);
            } else {
                rm_res = -1;
            }

            if(rm_res > 0) {
            	LOGE("Apps delete failed!\n");
            } else if(rm_res == 0) {
            	LOGI("Apps delete succeed\n");
            } else {
            	LOGI("Keep all the apps\n");
            }
        }

        if(ip_res == INSTALL_SUCCESS && wipe_data){
        	dd_busy_process(1);
			erase_volume("/data");
			erase_volume("/cache");
		}

        // clear the command file
        if (ensure_path_mounted(BD_COMMAND_FILE) != 0 ||
            (unlink(BD_COMMAND_FILE) && errno != ENOENT)) {
            LOGE("Can't unlink %s\n", BD_COMMAND_FILE);
        }

        // clear the package
        if (ensure_path_mounted(update_baidu_package) != 0 ||
            (unlink(update_baidu_package) && errno != ENOENT)) {
            LOGE("Can't unlink %s\n", update_baidu_package);
        }

        // clear the app.list
        if (ensure_path_mounted(APP_LIST_FILE) != 0 ||
            (unlink(APP_LIST_FILE) && errno != ENOENT)) {
            LOGE("Can't unlink %s\n", APP_LIST_FILE);
        }

        // get ready to boot the main system when flash done
        finish_recovery(send_intent);
        if(acfg()->reboot_cmd != NULL) {
        	LOGI("Rebooting with recovery_done...\n");
        	android_reboot(ANDROID_RB_RESTART2, 0, acfg()->reboot_cmd);
        } else {
        	LOGI("Rebooting...\n");
        	android_reboot(ANDROID_RB_RESTART, 0, 0);
        }

            return EXIT_SUCCESS;
    }

    if(backup_cmd) {
    	printf("Get backup cmd:%s\n", backup_cmd);
    	exec_backup_auto(backup_cmd);
    	finish_recovery(send_intent);
        if(acfg()->reboot_cmd != NULL) {
        	printf("Rebooting with recovery_done...\n");
        	android_reboot(ANDROID_RB_RESTART2, 0, acfg()->reboot_cmd);
        } else {
        	printf("Rebooting...\n");
        	android_reboot(ANDROID_RB_RESTART, 0, 0);
        }
        return EXIT_SUCCESS;
    }

    if(restore_cmd) {
    	ui_print("Get restore cmd:%s\n", restore_cmd);
    	exec_restore_auto(restore_cmd);
    	finish_recovery(send_intent);
        if(acfg()->reboot_cmd != NULL) {
        	printf("Rebooting with recovery_done...\n");
        	android_reboot(ANDROID_RB_RESTART2, 0, acfg()->reboot_cmd);
        } else {
        	printf("Rebooting...\n");
        	android_reboot(ANDROID_RB_RESTART, 0, 0);
        }
        return EXIT_SUCCESS;
    }

    if (update_package) {
        // For backwards compatibility on the cache partition only, if
        // we're given an old 'root' path "CACHE:foo", change it to
        // "/cache/foo".
        if (strncmp(update_package, "CACHE:", 6) == 0) {
            int len = strlen(update_package) + 10;
            char* modified_path = malloc(len);
            strlcpy(modified_path, "/cache/", len);
            strlcat(modified_path, update_package+6, len);
            printf("(replacing path \"%s\" with \"%s\")\n",
                   update_package, modified_path);
            update_package = modified_path;
        }
    }

    printf("\n");

    int status = INSTALL_SUCCESS;

    if (update_package != NULL) {
    	LOGI("Install package: %s\n", update_package);
        if (wipe_cache) erase_volume("/cache");
        if (strcmp("continuous-update", basename(update_package))
			&& strcmp("continuous-update-wipe", basename(update_package))) {
            status = install_start(update_package, wipe_cache, TEMPORARY_INSTALL_FILE, 0, 1);
 			if(status == INSTALL_SUCCESS && wipe_cache){
 				dd_busy_process(1);
				if (erase_volume("/cache")) {
                    LOGE("Cache wipe (requested by package) failed.");
                }
			}
			if(status == INSTALL_SUCCESS && wipe_data){
				dd_busy_process(1);
				if (erase_volume("/data")) {
                    LOGE("Data wipe (requested by package) failed.");
                }
				if (erase_volume("/cache")) {
                    LOGE("Cache wipe (requested by package) failed.");
                }
			}
            if (status != INSTALL_SUCCESS) ui_print("Installation aborted.\n");
        }  else {
            ensure_path_mounted(update_package);
			if(0 == strcmp("continuous-update-wipe", basename(update_package)))
				wipe_data = 1;
            char update_file_path[PATH_MAX];
            FILE *fp = fopen(update_package, "r");
            if (NULL == fp) {
                status = INSTALL_ERROR;
            } else {
                while (fgets(update_file_path, PATH_MAX, fp)) {
                    update_file_path[strlen(update_file_path) - 1] = 0;
                    LOGI("Install package: %s\n", update_file_path);
                    ui_reset_progress();
                    status = install_start(update_file_path, wipe_cache, TEMPORARY_INSTALL_FILE, 0, 1);
                    if (INSTALL_SUCCESS != status) {
                        break;
                    }
					if ((INSTALL_SUCCESS == status) && wipe_data){
						dd_busy_process(1);
						erase_volume("/data");
						erase_volume("/cache");
					}
                }
            }
        }
        if (status != INSTALL_SUCCESS) LOGE("Installation aborted.\n");
    } else if (wipe_data) {
    	dd_busy_process(1);
        if (device_wipe_data()) status = INSTALL_ERROR;
        if (erase_volume("/data")) status = INSTALL_ERROR;
        if (wipe_cache && erase_volume("/cache")) status = INSTALL_ERROR;
        if (status != INSTALL_SUCCESS) LOGE("Data wipe failed.\n");
    } else if (wipe_cache) {
    	dd_busy_process(1);
        if (wipe_cache && erase_volume("/cache")) status = INSTALL_ERROR;
        if (status != INSTALL_SUCCESS) LOGE("Cache wipe failed.\n");
    } else {
        status = INSTALL_ERROR;  // No command specified
    }
    if (status != INSTALL_SUCCESS) device_main_ui_show();//show menu
    device_main_ui_release();
    // Otherwise, get ready to boot the main system...
    finish_recovery(send_intent);
    if(acfg()->reboot_cmd != NULL) {
    	LOGI("Rebooting with recovery_done...\n");
    	android_reboot(ANDROID_RB_RESTART2, 0, acfg()->reboot_cmd);
    } else {
    	LOGI("Rebooting...\n");
    	android_reboot(ANDROID_RB_RESTART, 0, 0);
    }

    return EXIT_SUCCESS;
}
