/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include "gralloc_priv.h"
#include "gr.h"

// [MTK] {{{
#ifdef MTK_ION_SUPPORT
#include "gralloc_ion.h"
#endif
// [MTK] }}}

/*****************************************************************************/

struct gralloc_context_t {
    alloc_device_t  device;
    /* our private data here */

// [MTK] {{{
#ifdef MTK_ION_SUPPORT
    int ion_dev_fd;
#endif
// [MTK] }}}
};

// [MTK] {{{
struct gralloc_extra_t {
    extra_device_t device;
};

#define ALIGN(x,a)	(((x) + (a) - 1L) & ~((a) - 1L))
#define YUV_ALIGN	32
// [MTK] }}}

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle);

/*****************************************************************************/

int fb_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

static int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

extern int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr);

extern int gralloc_unlock(gralloc_module_t const* module, 
        buffer_handle_t handle);

extern int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

extern int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

/*****************************************************************************/

static struct hw_module_methods_t gralloc_module_methods = {
        open: gralloc_device_open
};

struct private_module_t HAL_MODULE_INFO_SYM = {
    base: {
        common: {
            tag: HARDWARE_MODULE_TAG,
            version_major: 1,
            version_minor: 0,
            id: GRALLOC_HARDWARE_MODULE_ID,
            name: "Graphics Memory Allocator Module",
            author: "The Android Open Source Project",
            methods: &gralloc_module_methods
        },
        registerBuffer: gralloc_register_buffer,
        unregisterBuffer: gralloc_unregister_buffer,
        lock: gralloc_lock,
        unlock: gralloc_unlock,
    },
    framebuffer: 0,
    flags: 0,
    numBuffers: 0,
    bufferMask: 0,
    lock: PTHREAD_MUTEX_INITIALIZER,
    currentBuffer: 0,
};

/*****************************************************************************/

static int gralloc_alloc_framebuffer_locked(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);

    // allocate the framebuffer
    if (m->framebuffer == NULL) {
        // initialize the framebuffer, the framebuffer is mapped once
        // and forever.
        int err = mapFrameBufferLocked(m);
        if (err < 0) {
            return err;
        }
    }

    const uint32_t bufferMask = m->bufferMask;
    const uint32_t numBuffers = m->numBuffers;
    const size_t bufferSize = m->finfo.line_length * m->info.yres;
    if (numBuffers == 1) {
        // If we have only one buffer, we never use page-flipping. Instead,
        // we return a regular buffer which will be memcpy'ed to the main
        // screen when post is called.
        int newUsage = (usage & ~GRALLOC_USAGE_HW_FB) | GRALLOC_USAGE_HW_2D;
        return gralloc_alloc_buffer(dev, bufferSize, newUsage, pHandle);
    }

    if (bufferMask >= ((1LU<<numBuffers)-1)) {
        // We ran out of buffers.
        return -ENOMEM;
    }

    // create a "fake" handles for it
    intptr_t vaddr = intptr_t(m->framebuffer->base);
    private_handle_t* hnd = new private_handle_t(dup(m->framebuffer->fd), size,
            private_handle_t::PRIV_FLAGS_FRAMEBUFFER);

    // find a free slot
    for (uint32_t i=0 ; i<numBuffers ; i++) {
        if ((bufferMask & (1LU<<i)) == 0) {
            m->bufferMask |= (1LU<<i);
            break;
        }
        vaddr += bufferSize;
    }
    
    hnd->base = vaddr;
    hnd->offset = vaddr - intptr_t(m->framebuffer->base);
    *pHandle = hnd;

    return 0;
}

static int gralloc_alloc_framebuffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);
    pthread_mutex_lock(&m->lock);
    int err = gralloc_alloc_framebuffer_locked(dev, size, usage, pHandle);
    pthread_mutex_unlock(&m->lock);
    return err;
}

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
// [MTK] {{{
#ifdef MTK_ION_SUPPORT
    int ret =0;
    int dev_fd =0 ;
    struct ion_handle *ion_hnd;
    struct ion_allocation_data *alloc_data;
    int ion_map_fd;
	
    size = roundUpToPageSize(size);

    //dev_fd = gralloc_ion_open();//;reinterpret_cast<gralloc_context_t*>(dev)->ion_dev_fd;
    dev_fd = reinterpret_cast<gralloc_context_t*>(dev)->ion_dev_fd;
    alloc_data = (ion_allocation_data *)malloc (sizeof (ion_allocation_data));
    ret = gralloc_ion_alloc(dev_fd, size, 4, alloc_data);
	
    if (ret < 0) {
        ALOGE("gralloc_alloc_buffer alloc fail :%x\n", ret);
        free(alloc_data);
        return ret;
    }

    ion_hnd = alloc_data->handle;

    ret = gralloc_ion_share(dev_fd, ion_hnd, &ion_map_fd);
	
    if (ret < 0) {
        ALOGE("gralloc_alloc_buffer fail fail :%x\n", ret);		
        free(alloc_data);
        return ret;
    }
	
    private_handle_t* hnd = new private_handle_t(ion_map_fd, size, 0); //dev_fd  is wrong, remember to correct it . [xxxxx]
    gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(dev->common.module);

    hnd->ion_alloc_data = alloc_data;
    hnd->ion_dev_fd = dev_fd; 

    ret = mapBuffer(module, hnd);
    if (ret == 0) {
        *pHandle = hnd;
    }
    hnd->fd = ion_map_fd;

    //LOGE_IF(ret, "gralloc alloc failed err=%s", strerror(-ret));

    return ret;
	
#else // MTK_ION_SUPPORT
// [MTK] }}}

    int err = 0;
    int fd = -1;

    size = roundUpToPageSize(size);
    
    fd = ashmem_create_region("gralloc-buffer", size);
    if (fd < 0) {
        ALOGE("couldn't create ashmem (%s)", strerror(-errno));
        err = -errno;
    }

    if (err == 0) {
        private_handle_t* hnd = new private_handle_t(fd, size, 0);
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
        err = mapBuffer(module, hnd);
        if (err == 0) {
            *pHandle = hnd;
        }
    }
    
    ALOGE_IF(err, "gralloc failed err=%s", strerror(-err));
    
    return err;

// [MTK] {{{
#endif
// [MTK] }}}
}

/*****************************************************************************/

static int gralloc_alloc(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
    if (!pHandle || !pStride)
        return -EINVAL;

    size_t size, stride;

    int align = 4;
    int bpp = 0;

    // [MTK] {{{
    int is_yuv = 0;
    int r_shift = 0;
    int extra_line = 0;
    // [MTK] }}}

    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            bpp = 4;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bpp = 3;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
            bpp = 2;
            break;

        // [MTK] {{{
        case HAL_PIXEL_FORMAT_YV12:
            is_yuv = 1;
            bpp = 3;
            r_shift = 1;
            extra_line = 0;
            break;
        case HAL_PIXEL_FORMAT_I420:
            is_yuv = 1;
            bpp = 3;
            r_shift = 1;
            extra_line = 1;
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            is_yuv = 1;
            bpp = 3;
            r_shift = 1;
            extra_line = 0;
            break;
        // [MTK] }}}

        default:
            return -EINVAL;
    }

    // [MTK] {{{
    if (is_yuv) {
        size_t bpr= 0;
        if ((format == HAL_PIXEL_FORMAT_NV12_BLK) ||
            (format == HAL_PIXEL_FORMAT_NV12_BLK_FCM )) {
            int y_w_stride = ALIGN(w, 16);
            int y_h_stride = ALIGN(h, 32);

            stride = y_w_stride;
            // 512 bytes alignment is done later in alloc function.
            size = (y_w_stride*y_h_stride*bpp) >> r_shift;
        } else {
            stride = ALIGN(w,YUV_ALIGN);
            bpr = (stride *bpp) >> r_shift;
            size = bpr * (h + extra_line);
        }
    } else {
        size_t bpr = (w*bpp + (align-1)) & ~(align-1);
        size = bpr * h;
        stride = bpr / bpp;
    }
    // [MTK] }}}

    int err;
    if (usage & GRALLOC_USAGE_HW_FB) {
        err = gralloc_alloc_framebuffer(dev, size, usage, pHandle);
    } else {
        err = gralloc_alloc_buffer(dev, size, usage, pHandle);
    }

    if (err < 0) {
        return err;
    }

    *pStride = stride;
    return 0;
}

static int gralloc_free(alloc_device_t* dev,
        buffer_handle_t handle)
{
// [MTK] {{{
#ifdef MTK_ION_SUPPORT
    int ret = 0;
    int dev_fd = 0;
    gralloc_context_t * ion_ctx;

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(handle);
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        // free this buffer
        private_module_t* m = reinterpret_cast<private_module_t*>(
                dev->common.module);
        const size_t bufferSize = m->finfo.line_length * m->info.yres;
        int index = (hnd->base - m->framebuffer->base) / bufferSize;
        m->bufferMask &= ~(1<<index); 
    } else { 
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
        terminateBuffer(module, const_cast<private_handle_t*>(hnd));
    }

    ion_ctx = reinterpret_cast<gralloc_context_t*>(dev);
    ret = gralloc_ion_free(ion_ctx->ion_dev_fd, hnd->ion_alloc_data->handle);
    if (ret < 0) {
        ALOGE("gralloc_free ion_free fail :%x\n", ret);
        goto exit;
    }

    close(hnd->fd);
    delete hnd;

exit:	
    return ret;

#else // MTK_ION_SUPPORT
// [MTK] }}}

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(handle);
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        // free this buffer
        private_module_t* m = reinterpret_cast<private_module_t*>(
                dev->common.module);
        const size_t bufferSize = m->finfo.line_length * m->info.yres;
        int index = (hnd->base - m->framebuffer->base) / bufferSize;
        m->bufferMask &= ~(1<<index); 
    } else { 
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
        terminateBuffer(module, const_cast<private_handle_t*>(hnd));
    }

    close(hnd->fd);
    delete hnd;
    return 0;

// [MTK] {{{
#endif
// [MTK] }}}
}


// [MTK] {{{
/*****************************************************************************/

static int gralloc_getIonFd(extra_device_t* dev,
        buffer_handle_t handle, int *idx, int *num)
{
    // TODO
    return -EINVAL;
}

static int gralloc_extra_close(struct hw_device_t *dev)
{
    gralloc_extra_t* ctx = reinterpret_cast<gralloc_extra_t*>(dev);
    if (ctx) {
        free(ctx);
    }
    return 0;
}
// [MTK] }}}

/*****************************************************************************/

static int gralloc_close(struct hw_device_t *dev)
{
    gralloc_context_t* ctx = reinterpret_cast<gralloc_context_t*>(dev);
    if (ctx) {
        /* TODO: keep a list of all buffer_handle_t created, and free them
         * all here.
         */

// [MTK] {{{
#ifdef MTK_ION_SUPPORT
        gralloc_ion_close(ctx->ion_dev_fd);
#endif
// [MTK] }}}

        free(ctx);
    }
    return 0;
}

int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, GRALLOC_HARDWARE_GPU0)) {
        gralloc_context_t *dev;
        dev = (gralloc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = gralloc_close;

        dev->device.alloc   = gralloc_alloc;
        dev->device.free    = gralloc_free;

        *device = &dev->device.common;
        status = 0;

// [MTK] {{{
#ifdef MTK_ION_SUPPORT
        dev->ion_dev_fd = gralloc_ion_open();
#endif
    } else if (!strcmp(name, GRALLOC_HARDWARE_EXTRA)) {
        gralloc_extra_t *dev;
        dev = (gralloc_extra_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = gralloc_extra_close;

        dev->device.getIonFd = gralloc_getIonFd;

        *device = &dev->device.common;
        status = 0;
// [MTK] }}}
    } else {
        status = fb_device_open(module, name, device);
    }
    return status;
}
