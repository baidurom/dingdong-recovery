/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>


#include <cutils/log.h>

#ifdef MTK_ION_SUPPORT

#include <linux/ion_drv.h>
#include "gralloc_ion.h"
#define LOG_TAG "gralloc_ion"

//for ion device open
int gralloc_ion_open()
{
    int fd = 0;

    fd = open("/dev/ion", O_RDONLY);
    if (fd < 0) {
        ALOGE("open /dev/ion failed! : %x", fd);
    }

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_open..%x", fd);
#endif

    return fd;
}
//for ion device device
int gralloc_ion_close(int fd)
{
#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_close..");
#endif
    return close(fd);
}

int gralloc_ion_ioctl(int fd, int req, void *arg)
{
    int ret = 0;

    ret = ioctl(fd, req, arg);
    if (ret < 0) {
        ALOGE("gralloc_ion_ioctl: %d failed with code %d: %s",
              req, ret, strerror(errno));
        ret = -errno;         
    }

exit:
    return ret;
}

int gralloc_ion_alloc(int dev_fd, int size, int align, struct ion_allocation_data *alloc_data)
{
    int ret = 0;

    alloc_data->len = size;
    alloc_data->align = align;
    alloc_data->flags = 0;//(1 << ION_HEAP_IDX_MULTIMEDIA;//ION_HEAP_ID_MULTIMEDIA);
    alloc_data->heap_mask = ION_HEAP_MULTIMEDIA_MASK;

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_alloc..");
#endif

    ret = gralloc_ion_ioctl(dev_fd, ION_IOC_ALLOC, (void *)alloc_data);
    if (ret < 0) {
        ALOGE("%s: ion_alloc fail: %d", __func__, ret);
    }

exit:
    return ret;
}

int gralloc_ion_free(int fd, struct ion_handle *handle)
{
    int ret = 0;
    struct ion_handle_data handle_data; 
    
#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_free..");
#endif

    handle_data.handle = handle;
    ret = gralloc_ion_ioctl(fd, ION_IOC_FREE, (void *)&handle_data);

    return ret;
}

int gralloc_ion_map(int fd, struct ion_handle *handle, int *map_fd)
{
    int ret = 0;
    struct ion_fd_data fd_data;

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_map..");
#endif

    fd_data.handle = handle;

    ret = gralloc_ion_ioctl(fd, ION_IOC_MAP, &fd_data);
    if (ret < 0) {
        ALOGE("%s: ion mapfail: %d", __func__, ret);
        goto exit;
    }

    *map_fd = fd_data.fd;

    if (*map_fd < 0) {
        ALOGE("%s: get wrong fd: %x", __func__, *map_fd);
        ret = -EINVAL;
        goto exit;
    }

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_map..: fd:%x, handle: %x", fd_data.fd, fd_data.handle);
#endif

exit:
    return ret;
}

int gralloc_ion_unmap(int fd, int size,void *vaddr)
{
    int ret = 0;

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_unmap..");
#endif

    if (munmap(vaddr, size) < 0) {
        ret = -1;
        ALOGE("Could not unmap %s", strerror(errno));
    }

    close (fd);
    return ret;
}

int gralloc_ion_share(int fd, struct ion_handle *handle, int *share_fd)
{
    int ret = 0;
    int map_fd;
    struct ion_fd_data fd_data;

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_share..");
#endif

    fd_data.handle = handle;
    ret = gralloc_ion_ioctl(fd, ION_IOC_SHARE, (void*)&fd_data);

    if (ret < 0) {
        ALOGE("%s: ion share fail: %x", __func__, ret);
        goto exit;
    }

    *share_fd = fd_data.fd;
    if (*share_fd < 0) {
        ALOGE("%s: ion share_fd <0 : %x", __func__, ret);
        ret = -EINVAL;
        goto exit;
    }

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_share..: fd:%x, handle: %x", fd_data.fd, fd_data.handle);
#endif

exit:
    return ret;
}

int gralloc_ion_import(int fd, int share_fd, struct ion_handle **handle)
//int gralloc_ion_import(int fd, int share_fd, struct ion_handle *handle)
{
    int ret = 0;
    struct ion_fd_data fd_data;

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_import..:%x", share_fd);
#endif

    fd_data.fd = share_fd,

    ret = gralloc_ion_ioctl(fd, ION_IOC_IMPORT, &fd_data);
    if (ret < 0) {
        ALOGE("map ioctl returned negative fd");
        goto exit;
    }
    *handle = fd_data.handle;

exit:
     return ret;
}

/*
  * fd : device fd 
  * handle: alloc_data.handle
*/
int gralloc_ion_sync(int fd, struct ion_handle *handle)
{
    int ret = 0;
    struct ion_custom_data custom_data;
    struct ion_sys_data    sys_data;

#ifdef MTK_GRALLOC_ION_DBG
    ALOGE("gralloc_ion_sync enter..");
#endif

    custom_data.cmd = ION_CMD_SYSTEM;
    custom_data.arg = (unsigned long)&sys_data;
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = (ion_handle *)handle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;

    ret = gralloc_ion_ioctl(fd, ION_IOC_CUSTOM, (void*)&custom_data);
    if (ret < 0) {
        ALOGE("sync ioctl fail");
    }

exit:
    return ret;
}

#endif // MTK_ION_SUPPORT
