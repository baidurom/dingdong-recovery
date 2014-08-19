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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "property_service.h"
#include "log.h"


int patch_lcd_density(void)
{
    const char FB_DEV[] = "/dev/graphics/fb0";
    const char LCD_DENSITY_PROP[] = "ro.sf.lcd_density";
    char value[10];
    
    struct fb_var_screeninfo vinfo;
    unsigned int pixels;
    unsigned int default_density = 160;
    int fd = -1;
    int ret = 0;

    /* check if lcd_density has been defined */
    if (property_get(LCD_DENSITY_PROP)) goto done;

    if ((fd = open(FB_DEV, O_RDONLY)) < 0) {
        ERROR("[ERROR] failed to open %s", FB_DEV);
        ret = -1;
        goto done;
    }

    if ((ret = ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) < 0) {
        ERROR("[ERROR] failed to get fb_var_screeninfo");
        goto done;
    }

    pixels = vinfo.xres * vinfo.yres;
/* Vanzo:Kern on: Thu, 07 Mar 2013 14:44:48 +0800
 * add fhd support
    if (pixels <= 240 * 432) default_density = 120;         // <= WQVGA432
    else if ((pixels >= 480 * 800) && (pixels <=1024 * 600)) default_density = 240;    // >= WVGA854
	else if (pixels > 1024 * 600) default_density = 320;
 */
        if (pixels >= 1920 * 1080) default_density = 480;
        else if (pixels > 960 * 540) default_density = 320;
        else if (pixels >= 480 * 800) default_density = 240;
        else if (pixels <= 240 * 432) default_density = 120;
// End of Vanzo:Kern

    
    sprintf(value, "%d", default_density);
    if ((ret = property_set(LCD_DENSITY_PROP, value)) < 0) {
        ERROR("[ERROR] failed to set property %s = %s", LCD_DENSITY_PROP, value);
        goto done;
    }

done:
    close(fd);
    return ret;
}


int patch_properties(void)
{
    int ret = 0;

    if ((ret = patch_lcd_density()) < 0) {
        ERROR("[ERROR] patch lcd_density property failed");
        return ret;
    }

    return ret;
}

