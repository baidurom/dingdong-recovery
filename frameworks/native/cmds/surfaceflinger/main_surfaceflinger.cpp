/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include <binder/BinderService.h>
#include <SurfaceFlinger.h>

// [MTK] {{{
#include <private/android_filesystem_config.h>
#include <linux/rtpm_prio.h>
#include <sys/prctl.h>
#include <cutils/xlog.h>
// [MTK] }}}

using namespace android;

int main(int argc, char** argv) {
    // [MTK] {{{
    if (AID_ROOT == getuid()) {
        XLOGI("[%s] set surfaceflinger is in root user, adjust caps for its thread", __func__);
        if (-1 == prctl(PR_SET_KEEPCAPS, 1, 0, 0)) {
            XLOGW("    prctl failed: %s", strerror(errno));
        } else {
            __user_cap_header_struct hdr;
            __user_cap_data_struct data;

            hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps
            hdr.pid = 0;
            data.effective = ((1 << CAP_SYS_NICE) | (1 << CAP_SETUID) | (1 << CAP_SETGID));
            data.permitted = ((1 << CAP_SYS_NICE) | (1 << CAP_SETUID) | (1 << CAP_SETGID));
            data.inheritable = 0xffffffff;
            if (-1 == capset(&hdr, &data)) {
                XLOGW("    cap setting failed, %s", strerror(errno));
            }

            setgid(AID_SYSTEM);
            setuid(AID_SYSTEM);         // change user to system

            hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps again
            hdr.pid = 0;
            data.effective = (1 << CAP_SYS_NICE);
	        data.permitted = (1 << CAP_SYS_NICE);
            data.inheritable = 0xffffffff;
            if (-1 == capset(&hdr, &data)) {
                XLOGW("    cap re-setting failed, %s", strerror(errno));
            }
        }
    } else {
        XLOGI("[%s] surfaceflinger is not in root user", __func__);
    }
    // [MTK] }}}

    SurfaceFlinger::publishAndJoinThreadPool(true);
    // When SF is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);
    return 0;
}
