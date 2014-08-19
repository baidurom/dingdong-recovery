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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#ifdef HAVE_SELINUX
#include <selinux/label.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

/* for ANDROID_SOCKET_* */
#include <cutils/sockets.h>

#include <private/android_filesystem_config.h>

#include "init.h"
#include "log.h"
#include "util.h"

/*
 * android_name_to_id - returns the integer uid/gid associated with the given
 * name, or -1U on error.
 */
static unsigned int android_name_to_id(const char *name)
{
    const struct android_id_info *info = android_ids;
    unsigned int n;

    for (n = 0; n < android_id_count; n++) {
        if (!strcmp(info[n].name, name))
            return info[n].aid;
    }

    return -1U;
}

/*
 * decode_uid - decodes and returns the given string, which can be either the
 * numeric or name representation, into the integer uid or gid. Returns -1U on
 * error.
 */
unsigned int decode_uid(const char *s)
{
    unsigned int v;

    if (!s || *s == '\0')
        return -1U;
    if (isalpha(s[0]))
        return android_name_to_id(s);

    errno = 0;
    v = (unsigned int) strtoul(s, 0, 0);
    if (errno)
        return -1U;
    return v;
}

/*
 * create_socket - creates a Unix domain socket in ANDROID_SOCKET_DIR
 * ("/dev/socket") as dictated in init.rc. This socket is inherited by the
 * daemon. We communicate the file descriptor's value via the environment
 * variable ANDROID_SOCKET_ENV_PREFIX<name> ("ANDROID_SOCKET_foo").
 */
int create_socket(const char *name, int type, mode_t perm, uid_t uid, gid_t gid)
{
    struct sockaddr_un addr;
    int fd, ret;
#ifdef HAVE_SELINUX
    char *secon;
#endif

    fd = socket(PF_UNIX, type, 0);
    if (fd < 0) {
        ERROR("Failed to open socket '%s': %s\n", name, strerror(errno));
        return -1;
    }

    memset(&addr, 0 , sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), ANDROID_SOCKET_DIR"/%s",
             name);

    ret = unlink(addr.sun_path);
    if (ret != 0 && errno != ENOENT) {
        ERROR("Failed to unlink old socket '%s': %s\n", name, strerror(errno));
        goto out_close;
    }

#ifdef HAVE_SELINUX
    secon = NULL;
    if (sehandle) {
        ret = selabel_lookup(sehandle, &secon, addr.sun_path, S_IFSOCK);
        if (ret == 0)
            setfscreatecon(secon);
    }
#endif

    ret = bind(fd, (struct sockaddr *) &addr, sizeof (addr));
    if (ret) {
        ERROR("Failed to bind socket '%s': %s\n", name, strerror(errno));
        goto out_unlink;
    }

#ifdef HAVE_SELINUX
    setfscreatecon(NULL);
    freecon(secon);
#endif

    chown(addr.sun_path, uid, gid);
    chmod(addr.sun_path, perm);

    INFO("Created socket '%s' with mode '%o', user '%d', group '%d'\n",
         addr.sun_path, perm, uid, gid);

    return fd;

out_unlink:
    unlink(addr.sun_path);
out_close:
    close(fd);
    return -1;
}

/* reads a file, making sure it is terminated with \n \0 */
void *read_file(const char *fn, unsigned *_sz)
{
    char *data;
    int sz;
    int fd;
    struct stat sb;

    data = 0;
    fd = open(fn, O_RDONLY);
    if(fd < 0) return 0;

    // for security reasons, disallow world-writable
    // or group-writable files
    if (fstat(fd, &sb) < 0) {
        ERROR("fstat failed for '%s'\n", fn);
        goto oops;
    }
    if ((sb.st_mode & (S_IWGRP | S_IWOTH)) != 0) {
        ERROR("skipping insecure file '%s'\n", fn);
        goto oops;
    }

    sz = lseek(fd, 0, SEEK_END);
    if(sz < 0) goto oops;

    if(lseek(fd, 0, SEEK_SET) != 0) goto oops;

    data = (char*) malloc(sz + 2);
    if(data == 0) goto oops;

    if(read(fd, data, sz) != sz) goto oops;
    close(fd);
    data[sz] = '\n';
    data[sz+1] = 0;
    if(_sz) *_sz = sz;
    return data;

oops:
    close(fd);
    if(data != 0) free(data);
    return 0;
}

#define MAX_MTD_PARTITIONS 16

static struct {
    char name[16];
    int number;
} mtd_part_map[MAX_MTD_PARTITIONS];

static int mtd_part_count = -1;

static void find_mtd_partitions(void)
{
    int fd;
    char buf[1024];
    char *pmtdbufp;
    ssize_t pmtdsize;
    int r;

    fd = open("/proc/mtd", O_RDONLY);
    if (fd < 0)
        return;

    buf[sizeof(buf) - 1] = '\0';
    pmtdsize = read(fd, buf, sizeof(buf) - 1);
    pmtdbufp = buf;
    while (pmtdsize > 0) {
        int mtdnum, mtdsize, mtderasesize;
        char mtdname[16];
        mtdname[0] = '\0';
        mtdnum = -1;
        r = sscanf(pmtdbufp, "mtd%d: %x %x %15s",
                   &mtdnum, &mtdsize, &mtderasesize, mtdname);
        if ((r == 4) && (mtdname[0] == '"')) {
            char *x = strchr(mtdname + 1, '"');
            if (x) {
                *x = 0;
            }
            INFO("mtd partition %d, %s\n", mtdnum, mtdname + 1);
            if (mtd_part_count < MAX_MTD_PARTITIONS) {
                strcpy(mtd_part_map[mtd_part_count].name, mtdname + 1);
                mtd_part_map[mtd_part_count].number = mtdnum;
                mtd_part_count++;
            } else {
                ERROR("too many mtd partitions\n");
            }
        }
        while (pmtdsize > 0 && *pmtdbufp != '\n') {
            pmtdbufp++;
            pmtdsize--;
        }
        if (pmtdsize > 0) {
            pmtdbufp++;
            pmtdsize--;
        }
    }
    close(fd);
}

int mtd_name_to_number(const char *name)
{
    int n;
    if (mtd_part_count < 0) {
        mtd_part_count = 0;
        find_mtd_partitions();
    }
    for (n = 0; n < mtd_part_count; n++) {
        if (!strcmp(name, mtd_part_map[n].name)) {
            return mtd_part_map[n].number;
        }
    }
    return -1;
}

/*
 * gettime() - returns the time in seconds of the system's monotonic clock or
 * zero on error.
 */
time_t gettime(void)
{
    struct timespec ts;
    int ret;

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret < 0) {
        ERROR("clock_gettime(CLOCK_MONOTONIC) failed: %s\n", strerror(errno));
        return 0;
    }

    return ts.tv_sec;
}

int mkdir_recursive(const char *pathname, mode_t mode)
{
    char buf[128];
    const char *slash;
    const char *p = pathname;
    int width;
    int ret;
    struct stat info;

    while ((slash = strchr(p, '/')) != NULL) {
        width = slash - pathname;
        p = slash + 1;
        if (width < 0)
            break;
        if (width == 0)
            continue;
        if ((unsigned int)width > sizeof(buf) - 1) {
            ERROR("path too long for mkdir_recursive\n");
            return -1;
        }
        memcpy(buf, pathname, width);
        buf[width] = 0;
        if (stat(buf, &info) != 0) {
            ret = mkdir(buf, mode);
            if (ret && errno != EEXIST)
                return ret;
        }
    }
    ret = mkdir(pathname, mode);
    if (ret && errno != EEXIST)
        return ret;
    return 0;
}

void sanitize(char *s)
{
    if (!s)
        return;
    while (isalnum(*s))
        s++;
    *s = 0;
}
void make_link(const char *oldpath, const char *newpath)
{
    int ret;
    char buf[256];
    char *slash;
    int width;

    slash = strrchr(newpath, '/');
    if (!slash)
        return;
    width = slash - newpath;
    if (width <= 0 || width > (int)sizeof(buf) - 1)
        return;
    memcpy(buf, newpath, width);
    buf[width] = 0;
    ret = mkdir_recursive(buf, 0755);
    if (ret)
        ERROR("Failed to create directory %s: %s (%d)\n", buf, strerror(errno), errno);

    ret = symlink(oldpath, newpath);
    if (ret && errno != EEXIST)
        ERROR("Failed to symlink %s to %s: %s (%d)\n", oldpath, newpath, strerror(errno), errno);
}

void remove_link(const char *oldpath, const char *newpath)
{
    char path[256];
    ssize_t ret;
    ret = readlink(newpath, path, sizeof(path) - 1);
    if (ret <= 0)
        return;
    path[ret] = 0;
    if (!strcmp(path, oldpath))
        unlink(newpath);
}

int wait_for_file(const char *filename, int timeout)
{
    struct stat info;
    time_t timeout_time = gettime() + timeout;
    int ret = -1;

    while (gettime() < timeout_time && ((ret = stat(filename, &info)) < 0))
        usleep(10000);

    return ret;
}

void open_devnull_stdio(void)
{
    int fd;
    static const char *name = "/dev/__null__";
    if (mknod(name, S_IFCHR | 0600, (1 << 8) | 3) == 0) {
        fd = open(name, O_RDWR);
        unlink(name);
        if (fd >= 0) {
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
            if (fd > 2) {
                close(fd);
            }
            return;
        }
    }

    exit(1);
}

void get_hardware_name(char *hardware, unsigned int *revision)
{
    char data[1024];
    int fd, n;
    char *x, *hw, *rev;

    /* Hardware string was provided on kernel command line */
    if (hardware[0])
        return;

    fd = open("/proc/cpuinfo", O_RDONLY);
    if (fd < 0) return;

    n = read(fd, data, 1023);
    close(fd);
    if (n < 0) return;

    data[n] = 0;
    hw = strstr(data, "\nHardware");
    rev = strstr(data, "\nRevision");

    if (hw) {
        x = strstr(hw, ": ");
        if (x) {
            x += 2;
            n = 0;
            while (*x && *x != '\n') {
                if (!isspace(*x))
                    hardware[n++] = tolower(*x);
                x++;
                if (n == 31) break;
            }
            hardware[n] = 0;
        }
    }

    if (rev) {
        x = strstr(rev, ": ");
        if (x) {
            *revision = strtoul(x + 2, 0, 16);
        }
    }
}

void import_kernel_cmdline(int in_qemu,
                           void (*import_kernel_nv)(char *name, int in_qemu))
{
    char cmdline[1024];
    char *ptr;
    int fd;

    fd = open("/proc/cmdline", O_RDONLY);
    if (fd >= 0) {
        int n = read(fd, cmdline, 1023);
        if (n < 0) n = 0;

        /* get rid of trailing newline, it happens */
        if (n > 0 && cmdline[n-1] == '\n') n--;

        cmdline[n] = 0;
        close(fd);
    } else {
        cmdline[0] = 0;
    }

    ptr = cmdline;
    while (ptr && *ptr) {
        char *x = strchr(ptr, ' ');
        if (x != 0) *x++ = 0;
        import_kernel_nv(ptr, in_qemu);
        ptr = x;
    }
}

int make_dir(const char *path, mode_t mode)
{
    int rc;

#ifdef HAVE_SELINUX
    char *secontext = NULL;

    if (sehandle) {
        selabel_lookup(sehandle, &secontext, path, mode);
        setfscreatecon(secontext);
    }
#endif

    rc = mkdir(path, mode);

#ifdef HAVE_SELINUX
    if (secontext) {
        int save_errno = errno;
        freecon(secontext);
        setfscreatecon(NULL);
        errno = save_errno;
    }
#endif
    return rc;
}

int restorecon(const char *pathname)
{
#ifdef HAVE_SELINUX
    char *secontext = NULL;
    struct stat sb;
    int i;

    if (is_selinux_enabled() <= 0 || !sehandle)
        return 0;

    if (lstat(pathname, &sb) < 0)
        return -errno;
    if (selabel_lookup(sehandle, &secontext, pathname, sb.st_mode) < 0)
        return -errno;
    if (lsetfilecon(pathname, secontext) < 0) {
        freecon(secontext);
        return -errno;
    }
    freecon(secontext);
#endif
    return 0;
}
#ifdef MTK_NAND_UBIFS_SUPPORT
#define UBI_CTRL_DEV "/dev/ubi_ctrl"
#define UBI_SYS_PATH "/sys/class/ubi"
static int ubi_dev_read_int(int dev, const char *file, int def)
{
    int fd, val = def;
    char path[128], buf[64];

    sprintf(path, UBI_SYS_PATH "/ubi%d/%s", dev, file);
    wait_for_file(path, 5);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        return val;
    }

    if (read(fd, buf, 64) > 0) {
        val = atoi(buf);
    }

    close(fd);
    return val;
}

// Should include kernel header include/mtd/ubi-user.h
#include <linux/types.h>
#include <asm/ioctl.h>
#define UBI_CTRL_IOC_MAGIC 'o'
#define UBI_IOC_MAGIC 'o'
#define UBI_VOL_NUM_AUTO (-1)
#define UBI_DEV_NUM_AUTO (-1)
#define UBI_IOCATT _IOW(UBI_CTRL_IOC_MAGIC, 64, struct ubi_attach_req)
#define UBI_IOCDET _IOW(UBI_CTRL_IOC_MAGIC, 65, __s32)
#define UBI_IOCMKVOL _IOW(UBI_IOC_MAGIC, 0, struct ubi_mkvol_req)
#define UBI_MAX_VOLUME_NAME 127

struct ubi_attach_req {
	__s32 ubi_num;
	__s32 mtd_num;
	__s32 vid_hdr_offset;
	__s8 padding[12];
};

struct ubi_mkvol_req {
	__s32 vol_id;
	__s32 alignment;
	__s64 bytes;
	__s8 vol_type;
	__s8 padding1;
	__s16 name_len;
	__s8 padding2[4];
	char name[UBI_MAX_VOLUME_NAME + 1];
} __packed;

enum {
	UBI_DYNAMIC_VOLUME = 3,
	UBI_STATIC_VOLUME  = 4,
};

// Should include kernel header include/mtd/ubi-user.h

int ubi_attach_mtd(const char *name)
{
    int ret;
    int mtd_num, ubi_num;
    int ubi_ctrl, ubi_dev;
    int vols, avail_lebs, leb_size;
    char path[128];
    struct ubi_attach_req attach_req;
    struct ubi_mkvol_req mkvol_req;
   
    mtd_num = mtd_name_to_number(name);
    if (mtd_num == -1) {
        return -1;
    }

    for (ubi_num = 0; ubi_num < 4; ubi_num++)
    {
      sprintf(path, "/sys/class/ubi/ubi%d/mtd_num", ubi_num);
      ubi_dev = open(path, O_RDONLY);
      if (ubi_dev != -1)
      {
        ret = read(ubi_dev, path, sizeof(path));
        close(ubi_dev);
        if (ret > 0 && mtd_num == atoi(path))
          return ubi_num;
      }
    }

    ubi_ctrl = open(UBI_CTRL_DEV, O_RDONLY);
    if (ubi_ctrl == -1) {
        return -1;
    }

    memset(&attach_req, 0, sizeof(struct ubi_attach_req));
    attach_req.ubi_num = UBI_DEV_NUM_AUTO;
    attach_req.mtd_num = mtd_num;
    attach_req.vid_hdr_offset = 4096;  // for page and VID is 4096, but sub-page is 2048  (KF94G16Q4X-AEB)

    ret = ioctl(ubi_ctrl, UBI_IOCATT, &attach_req);
    if (ret == -1) {
        close(ubi_ctrl);
        return -1;
    }

    ubi_num = attach_req.ubi_num;
   
    vols = ubi_dev_read_int(ubi_num, "volumes_count", -1);
    if (vols == 0) {
        sprintf(path, "/dev/ubi%d", ubi_num);
        ubi_dev = open(path, O_RDONLY);
        if (ubi_dev == -1) {
            close(ubi_ctrl);
            return ubi_num;
        }
       
        avail_lebs = ubi_dev_read_int(ubi_num, "avail_eraseblocks", 0);
        leb_size = ubi_dev_read_int(ubi_num, "eraseblock_size", 0);

        memset(&mkvol_req, 0, sizeof(struct ubi_mkvol_req));
        mkvol_req.vol_id = UBI_VOL_NUM_AUTO;
        mkvol_req.alignment = 1;
        mkvol_req.bytes = (long long)avail_lebs * leb_size;
        mkvol_req.vol_type = UBI_DYNAMIC_VOLUME;
        ret = snprintf(mkvol_req.name, UBI_MAX_VOLUME_NAME + 1, "%s", name);
        mkvol_req.name_len = ret;
        ioctl(ubi_dev, UBI_IOCMKVOL, &mkvol_req);
        close(ubi_dev);
    }

    close(ubi_ctrl);
    return ubi_num;
}

int ubi_detach_dev(int dev)
{
    int ret, ubi_ctrl;

    ubi_ctrl = open(UBI_CTRL_DEV, O_RDONLY);
    if (ubi_ctrl == -1) {
        return -1;
    }

    ret = ioctl(ubi_ctrl, UBI_IOCDET, &dev);
    close(ubi_ctrl);
    return ret;
}
#endif
