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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "mtdutils/mounts.h"
#include "mtdutils/mtdutils.h"
#include "cutils/properties.h"
#include "roots.h"
#include "ddui/src/dd.h"
#include "dd_op.h"


#ifndef BOARD_USB_CONFIG_FILE
/*
 * Available state: DISCONNECTED, CONFIGURED, CONNECTED
 */
#define BOARD_USB_CONFIG_FILE "/sys/class/android_usb/android0/state"

// USB_STATE_CONFIGURED
#define BOARD_USB_CONFIG_FILE1 "/sys/devices/platform/msm_hsusb/gadget/usb_state"
#endif

int is_usb_connected()
{
    char state[255];
    char sel_config_file = 0;
    int fd = open(BOARD_USB_CONFIG_FILE, O_RDONLY);

    if (fd < 0) 
    {
		fd = open(BOARD_USB_CONFIG_FILE1, O_RDONLY);
		if (fd < 0) 
		{
			LOGE("Unable to open usb_configuration state file(%s)\n",strerror(errno));
			close(fd);
			return 0;
		}
		else
		{
			sel_config_file = 1;
		}
    }
    else
    {
		sel_config_file = 0;
	}

    if (read(fd, state, sizeof(state)) < 0) 
    {
        LOGE("Unable to read usb_configuration state file(%s)\n", strerror(errno));
        close(fd);
        return 0;
    }
    state[254] = '\0';
    LOGI("%s: state=%s\n", __func__, state);
    close(fd);
    
	return state[10*sel_config_file] == 'C'; 
}
