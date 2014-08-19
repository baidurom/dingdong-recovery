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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "fdevent.h"
#include "adb.h"

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#if defined(EMULATOR_PROJECT)
	#define LCDC_CAPTURE   (0)
#else
	#define LCDC_CAPTURE   (1)
#endif

#if LCDC_CAPTURE
#include "mtkfb.h"
// TODO: include from <linux/fb.h>
#define FBIOLOCK_FB             0x4630
#define FBIOUNLOCK_FB           0x4631
#define FBIOLOCKED_IOCTL        0x4632
static void lcdc_screen_cap(void);
static int vinfoToPixelFormat(struct fb_var_screeninfo* vinfo, uint32_t* bytespp, uint32_t* f);
#endif
/* TODO:
** - sync with vsync to avoid tearing
*/
/* This version number defines the format of the fbinfo struct.
   It must match versioning in ddms where this data is consumed. */
#define DDMS_RAWIMAGE_VERSION 1
struct fbinfo {
    unsigned int version;
    unsigned int bpp;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned int red_offset;
    unsigned int red_length;
    unsigned int blue_offset;
    unsigned int blue_length;
    unsigned int green_offset;
    unsigned int green_length;
    unsigned int alpha_offset;
    unsigned int alpha_length;
} __attribute__((packed));

#define LOG_TAG "ddmscap"
#include <utils/Log.h>

void framebuffer_service(int fd, void *cookie)
{
    struct fbinfo fbinfo;
    unsigned int i;
    char buf[640];
    int fd_screencap,fb;
    int w, h, f;
    int fds[2];

    if (pipe(fds) < 0) goto done;

    pid_t pid = fork();
    if (pid < 0) goto done;

    if (pid == 0) {
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
    #if (0 == LCDC_CAPTURE)
        const char* command = "screencap";
        const char *args[2] = {command, NULL};
        execvp(command, (char**)args);
        exit(1);
    #else // #if (LCDC_CAPTURE)
        if ((fb=open("/system/bin/lcdc_screen_cap",O_RDONLY)) >= 0){
            close(fb);
            const char * command = "lcdc_screen_cap";
	    const char *args[2] = {command, NULL};
            execvp(command, (char**)args);
            exit(1);
    }
        else{
        lcdc_screen_cap();
        exit(0);
        }
    #endif
    }

    fd_screencap = fds[0];

    /* read w, h & format */
    if(readx(fd_screencap, &w, 4)) goto done;
    if(readx(fd_screencap, &h, 4)) goto done;
    if(readx(fd_screencap, &f, 4)) goto done;

    fbinfo.version = DDMS_RAWIMAGE_VERSION;
    /* see hardware/hardware.h */
    switch (f) {
        case 1: /* RGBA_8888 */
            fbinfo.bpp = 32;
            fbinfo.size = w * h * 4;
            fbinfo.width = w;
            fbinfo.height = h;
            fbinfo.red_offset = 0;
            fbinfo.red_length = 8;
            fbinfo.green_offset = 8;
            fbinfo.green_length = 8;
            fbinfo.blue_offset = 16;
            fbinfo.blue_length = 8;
            fbinfo.alpha_offset = 24;
            fbinfo.alpha_length = 8;
            break;
        case 2: /* RGBX_8888 */
            fbinfo.bpp = 32;
            fbinfo.size = w * h * 4;
            fbinfo.width = w;
            fbinfo.height = h;
            fbinfo.red_offset = 0;
            fbinfo.red_length = 8;
            fbinfo.green_offset = 8;
            fbinfo.green_length = 8;
            fbinfo.blue_offset = 16;
            fbinfo.blue_length = 8;
            fbinfo.alpha_offset = 24;
            fbinfo.alpha_length = 0;
            break;
        case 3: /* RGB_888 */
            fbinfo.bpp = 24;
            fbinfo.size = w * h * 3;
            fbinfo.width = w;
            fbinfo.height = h;
            fbinfo.red_offset = 0;
            fbinfo.red_length = 8;
            fbinfo.green_offset = 8;
            fbinfo.green_length = 8;
            fbinfo.blue_offset = 16;
            fbinfo.blue_length = 8;
            fbinfo.alpha_offset = 24;
            fbinfo.alpha_length = 0;
            break;
        case 4: /* RGB_565 */
            fbinfo.bpp = 16;
            fbinfo.size = w * h * 2;
            fbinfo.width = w;
            fbinfo.height = h;
            fbinfo.red_offset = 11;
            fbinfo.red_length = 5;
            fbinfo.green_offset = 5;
            fbinfo.green_length = 6;
            fbinfo.blue_offset = 0;
            fbinfo.blue_length = 5;
            fbinfo.alpha_offset = 0;
            fbinfo.alpha_length = 0;
            break;
        case 5: /* BGRA_8888 */
            fbinfo.bpp = 32;
            fbinfo.size = w * h * 4;
            fbinfo.width = w;
            fbinfo.height = h;
            fbinfo.red_offset = 16;
            fbinfo.red_length = 8;
            fbinfo.green_offset = 8;
            fbinfo.green_length = 8;
            fbinfo.blue_offset = 0;
            fbinfo.blue_length = 8;
            fbinfo.alpha_offset = 24;
            fbinfo.alpha_length = 8;
           break;
        default:
            goto done;
    }

    /* write header */
    if(writex(fd, &fbinfo, sizeof(fbinfo))) goto done;

    /* write data */
    for(i = 0; i < fbinfo.size; i += sizeof(buf)) {
      if(readx(fd_screencap, buf, sizeof(buf))) goto done;
      if(writex(fd, buf, sizeof(buf))) goto done;
    }
    if(readx(fd_screencap, buf, fbinfo.size % sizeof(buf))) goto done;
    if(writex(fd, buf, fbinfo.size % sizeof(buf))) goto done;

done:
    TEMP_FAILURE_RETRY(waitpid(pid, NULL, 0));

    close(fds[0]);
    close(fds[1]);
    close(fd);
}

#if LCDC_CAPTURE
static int vinfoToPixelFormat(struct fb_var_screeninfo* vinfo,
                              uint32_t* bytespp, uint32_t* f)
{

    switch (vinfo->bits_per_pixel) {
        case 16:
            *f = 4; //PIXEL_FORMAT_RGB_565
            *bytespp = 2;
            break;
        case 24:
            *f = 3; //PIXEL_FORMAT_RGB_888
            *bytespp = 3;
            break;
        case 32:
            // TODO: do better decoding of vinfo here
            //*f = PIXEL_FORMAT_RGBX_8888;
            *f = 5; //PIXEL_FORMAT_BGRA_8888
            *bytespp = 4;
            break;
        default:
            return -1;
    }
    return 0;
}

static void lcdc_screen_cap(void)
{
    int step = 0;
    int fd = dup(STDOUT_FILENO);
    unsigned long fb_lock[2]   = {MTKFB_LOCK_FRONT_BUFFER,   (unsigned long)NULL};
    unsigned long fb_unlock[2] = {MTKFB_UNLOCK_FRONT_BUFFER, (unsigned long)NULL};
    unsigned long fb_capture[2] = {MTKFB_CAPTURE_FRAMEBUFFER, (unsigned long)NULL};
    void *base = NULL, *base_align = NULL;
    int capture_buffer_size = 0, capture_buffer_size_align = 0;
    struct fb_var_screeninfo vinfo;
    int fb;
    uint32_t bytespp;
    uint32_t w, h, f;
    size_t size = 0;

    if (0 > (fb = open("/dev/graphics/fb0", O_RDONLY))) goto done;
    //LOGI("[DDMSCap]Open /dev/graphics/fb0\n");
    fcntl(fb, F_SETFD, FD_CLOEXEC);
    if(ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) goto done;
    //LOGI("[DDMSCap]FBIOGET_VSCREENINFO\n");
    if(ioctl(fb, FBIOLOCK_FB, NULL) < 0) goto done;
    //LOGI("[DDMSCap]FBIOLOCK_FB\n");
    ++ step; //1
    if(ioctl(fb, FBIOLOCKED_IOCTL, fb_lock) < 0) goto done;
    //LOGI("[DDMSCap]FBIOLOCKED_IOCTL\n");
    ++ step; //2

    if (vinfoToPixelFormat(&vinfo, &bytespp, &f) == 0) 
    {
        w = vinfo.xres;
        h = vinfo.yres;
        size = w * h * bytespp;
        ALOGI("[DDMSCap]screen_width = %d, screen_height = %d, bpp = %d, format = %d, size = %d\n", w, h, bytespp, f, size);
    }
    {
        capture_buffer_size = w * h * bytespp;
        capture_buffer_size_align = capture_buffer_size + 32; //for M4U 32-byte alignment
        base_align = malloc(capture_buffer_size_align);

        if(base_align == NULL)
        {
            ALOGE("[DDMSCap]pmem_alloc size 0x%08x failed", capture_buffer_size_align);
            goto done;
        }
        else
        {
             ALOGI("[DDMSCap]pmem_alloc size = 0x%08x, addr = 0x%08x", capture_buffer_size_align, base_align);
        }

        base = (void *)((unsigned long)base_align + 32 - ((unsigned long)base_align & 0x1F)); //for M4U 32-byte alignment
        ALOGI("[DDMSCap]pmem_alloc base = 0x%08x", base);
        fb_capture[1] = (unsigned long)&base;
        if(ioctl(fb, FBIOLOCKED_IOCTL, fb_capture) < 0)
        {
            ALOGE("[DDMSCap]ioctl of MTKFB_CAPTURE_FRAMEBUFFER fail\n");
            goto done;
        }

        if (step > 1) 
            ioctl(fb, FBIOLOCKED_IOCTL, fb_unlock);
        if (step > 0) 
            ioctl(fb, FBIOUNLOCK_FB, NULL);

        ++ step; //3
    }

    if (base) 
    {
        {
            write(fd, &w, 4);
            write(fd, &h, 4);
            write(fd, &f, 4);
            write(fd, base, size);
        }
    }
done:
    if (NULL != base_align)
        free(base_align);
    if (step < 3)
    {
        if (step > 1) 
            ioctl(fb, FBIOLOCKED_IOCTL, fb_unlock);
        if (step > 0) 
            ioctl(fb, FBIOUNLOCK_FB, NULL);
    }
    if(fb >= 0) close(fb);
    close(fd);
    return 0;
}
#endif //#if LCDC_CAPTURE
