#ifndef __GRALLOC_ION_H__
#define __GRALLOC_ION_H__

#include <linux/ion_drv.h>

int gralloc_ion_open();
int gralloc_ion_close(int fd);
int gralloc_ion_alloc(int fd, int size, int align, struct ion_allocation_data *alloc_data);
int gralloc_ion_free(int fd, struct ion_handle *handle);
int gralloc_ion_map(int fd, struct ion_handle *handle, int *map_fd);
int gralloc_ion_unmap(int fd, int size,void *vaddr);
int gralloc_ion_share(int fd, struct ion_handle *handle, int *share_fd);
int gralloc_ion_import(int fd, int share_fd, struct ion_handle **handle);
//int gralloc_ion_import(int fd, int share_fd, struct ion_handle *handle);

int gralloc_ion_sync(int fd, struct ion_handle *handle);
int gralloc_ion_ioctl(int fd, int req, void *arg);



#endif /*__GRALLOC_ION_H__*/

