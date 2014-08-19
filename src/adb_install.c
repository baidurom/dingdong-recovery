/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#include "minui/minui.h"
#include "cutils/properties.h"
#include "install.h"
#include "common.h"
#include "recovery_ui.h"
#include "adb_install.h"
#include "minadbd/adb.h"
#include "ddui/src/dd.h"

static int finished = 0;

static void
set_usb_driver(int enabled) {
    int fd = open("/sys/class/android_usb/android0/enable", O_WRONLY);
    if (fd < 0) {
        ui_print("failed to open driver control: %s\n", strerror(errno));
        return;
    }

    int status;
    if (enabled > 0) {
        status = write(fd, "1", 1);
    } else {
        status = write(fd, "0", 1);
    }

    if (status < 0) {
        ui_print("failed to set driver control: %s\n", strerror(errno));
    }

    if (close(fd) < 0) {
        ui_print("failed to close driver control: %s\n", strerror(errno));
    }
}

static void
stop_adbd() {
    property_set("ctl.stop", "adbd");
    set_usb_driver(0);
}


static void
maybe_restart_adbd() {
    char value[PROPERTY_VALUE_MAX+1];
    int len = property_get("ro.debuggable", value, NULL);
    if (len == 1 && value[0] == '1') {
        ui_print("Restarting adbd...\n");
        set_usb_driver(1);
        property_set("ctl.start", "adbd");
    }
}

struct sideload_waiter_data {
    pid_t child;
};

void *adb_sideload_thread(void* v) {
    struct sideload_waiter_data* data = (struct sideload_waiter_data*)v;

    int status;
    waitpid(data->child, &status, 0);
    printf("sideload process finished\n");

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    	printf("status %d\n", WEXITSTATUS(status));
    }

    printf("sideload thread finished\n");
    return NULL;
}

int
apply_from_adb() {

    stop_adbd();
    set_usb_driver(1);

    printf("\n\nSideload started ...\nNow send the package you want to apply\n"
                  "to the device with \"adb sideload <filename>\"...\n\n");

    struct sideload_waiter_data data;
    if ((data.child = fork()) == 0) {
    	execl("/sbin/recovery", "recovery", "adbd", NULL);
    	_exit(-1);
    }
    pthread_t sideload_thread;
    pthread_create(&sideload_thread, NULL, &adb_sideload_thread, &data);

    int ret = dd_confirm(3, "<~install.sideload.wait.title>",
    		"<~install.sideload.start.tip>\n\n<~install.sideload.cmd.tip>\n\n<~install.sideload.finish.tip>",
    		NULL);

    set_usb_driver(0);
    maybe_restart_adbd();

    // kill the child
	kill(data.child, SIGTERM);
	pthread_join(sideload_thread, NULL);

    if (ret != RET_YES) {
    	printf("Cancel sideload flash\n");
    	return 1;
    } else {
    	printf("Continue sideload flash\n");
    }

    struct stat st;
    if (stat(ADB_SIDELOAD_FILENAME, &st) != 0) {
        if (errno == ENOENT) {
        	printf("<~install.sideload.nopackage.alert>\n\n");
        } else {
        	printf("<~install.sideload.zip.error>\n\n");
        }
        return -1;
    }

    return 0;
}
