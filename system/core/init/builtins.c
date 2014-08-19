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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <linux/kd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <linux/loop.h>
#include <cutils/partition_utils.h>
#include <sys/system_properties.h>
#include <fs_mgr.h>
#include <stdarg.h>

#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#include <selinux/label.h>
#endif

#include "init.h"
#include "keywords.h"
#include "property_service.h"
#include "devices.h"
#include "init_parser.h"
#include "util.h"
#include "log.h"

#include <private/android_filesystem_config.h>
#ifdef INIT_ENG_BUILD
#define printf(x...) NOTICE(x)
#endif
void add_environment(const char *name, const char *value);

extern int init_module(void *, unsigned long, const char *);

static int write_file(const char *path, const char *value)
{
    int fd, ret, len;

    fd = open(path, O_WRONLY|O_CREAT, 0622);

    if (fd < 0)
        return -errno;

    len = strlen(value);

    do {
        ret = write(fd, value, len);
    } while (ret < 0 && errno == EINTR);

    close(fd);
    if (ret < 0) {
        return -errno;
    } else {
        return 0;
    }
}


static int _chown(const char *path, unsigned int uid, unsigned int gid)
{
    int ret;

    struct stat p_statbuf;

    ret = lstat(path, &p_statbuf);
    if (ret < 0) {
        return -1;
    }

    if (S_ISLNK(p_statbuf.st_mode) == 1) {
        errno = EINVAL;
        return -1;
    }

    ret = chown(path, uid, gid);

    return ret;
}

static int _chmod(const char *path, mode_t mode)
{
    int ret;

    struct stat p_statbuf;

    ret = lstat(path, &p_statbuf);
    if( ret < 0) {
        return -1;
    }

    if (S_ISLNK(p_statbuf.st_mode) == 1) {
        errno = EINVAL;
        return -1;
    }

    ret = chmod(path, mode);

    return ret;
}

#ifdef HAVE_AEE_FEATURE
#define AEE_KE_INSMOD_PATH "/proc/aed/generate-kernel-notify"

static void raise_aed_ke(const char *fmt, ...)
{
    FILE *fp = fopen(AEE_KE_INSMOD_PATH, "w");
    if (fp != NULL) {
        char *msg = NULL;
        va_list ap;
        va_start(ap, fmt);
        if (vasprintf(&msg, fmt, ap) != -1) {
            fwrite(msg, strlen(msg) + 1, 1, fp);
        }
        fclose(fp);
        va_end(ap);
        free(msg);
    }
}
#endif

static int insmod(const char *filename, char *options)
{
    void *module;
    unsigned size;
    int ret;

    module = read_file(filename, &size);
    if (!module) {
#ifdef HAVE_AEE_FEATURE
        raise_aed_ke("W:insmod:Module \"%s\" not found.\n", filename);
#endif
        return -1;
    }

    ret = init_module(module, size, options);
#ifdef HAVE_AEE_FEATURE
    if (ret < 0) {
        const char *prop;

        prop = property_get("vold.post_fs_data_done");
        if (! prop) {
            prop = "notset";
        }       

        if (EEXIST == errno && *prop == '0') {
             printf("In encrypt phone, it will trigger_post_fs_data. Threefore skip insmod (File exists) error. \n");
        }
        else {
             raise_aed_ke("W:insmod:Failed to init module \"%s\" (%s), Check kernel log for more info.\n", filename, strerror(errno));
        }
    }
#endif

    free(module);

    return ret;
}

static int setkey(struct kbentry *kbe)
{
    int fd, ret;

    fd = open("/dev/tty0", O_RDWR | O_SYNC);
    if (fd < 0)
        return -1;

    ret = ioctl(fd, KDSKBENT, kbe);

    close(fd);
    return ret;
}

static int __ifupdown(const char *interface, int up)
{
    struct ifreq ifr;
    int s, ret;

    strlcpy(ifr.ifr_name, interface, IFNAMSIZ);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        return -1;

    ret = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (ret < 0) {
        goto done;
    }

    if (up)
        ifr.ifr_flags |= IFF_UP;
    else
        ifr.ifr_flags &= ~IFF_UP;

    ret = ioctl(s, SIOCSIFFLAGS, &ifr);
    
done:
    close(s);
    return ret;
}

static void service_start_if_not_disabled(struct service *svc)
{
    if (!(svc->flags & SVC_DISABLED)) {
        service_start(svc, NULL);
    }
}

int do_chdir(int nargs, char **args)
{
    chdir(args[1]);
    return 0;
}

int do_chroot(int nargs, char **args)
{
    chroot(args[1]);
    return 0;
}

int do_class_start(int nargs, char **args)
{
        /* Starting a class does not start services
         * which are explicitly disabled.  They must
         * be started individually.
         */
    service_for_each_class(args[1], service_start_if_not_disabled);
    return 0;
}

int do_class_stop(int nargs, char **args)
{
    service_for_each_class(args[1], service_stop);
    return 0;
}

int do_class_reset(int nargs, char **args)
{
    service_for_each_class(args[1], service_reset);
    return 0;
}

int do_domainname(int nargs, char **args)
{
    return write_file("/proc/sys/kernel/domainname", args[1]);
}

int do_exec(int nargs, char **args)
{
    int i;
    int status;
    int child_return_val;
    pid_t pid;
    pid_t pid_super;
    pid_t pid_manual;

    for(i = 0; i < nargs; i++)
    {
        printf("argv[%d]= %s\n", i, args[i]);
    }
    pid = fork();
    switch (pid){
        case -1:
            printf("fork system call failed\n");
            break;
        case 0:  /* child */
            execl(args[1], args[1], args[2], args[3], args[4], NULL);
            printf("cannot run %s\n", args[1]);
            _exit(127);
            break;
        default: /* parent */
            /* wait child process end(for e2fsck complete) */
            while (pid != wait(&status));

            if (WIFEXITED(status)){
                child_return_val = WEXITSTATUS(status);
                if (strncmp(args[1], "/sbin/e2fsck", 12)){
                    if (child_return_val != 0){
                        printf("%s run failed\n", args[1]);
                    }
                    //printf("child_return_val = %s\n", child_return_val);
                } else {
                    switch (child_return_val){
                        case 127:
                            printf("execl error\n");
                            break;
                        case 0:
                        case 1:
                            /* e2fsck run ok */
                            printf("e2fsck run ok!\n");
                            break;
                        case 2:
                            /* e2fsck run ok, but need reboot the system */
                            printf("e2fsck need reboot!\n");
                            system("reboot");
                            break;
                        case 4:
                            printf("e2fsck: all say yes!\n");
                            pid_manual = fork();
                            switch (pid_manual){
                                case -1:
                                    printf("manual check:fork system call failed\n");
                                    break;
                                case 0: /* child */
                                    execl(args[1], args[1], "-y", args[3], NULL);
                                    printf("manual check:cannot run %s\n", args[1]);
                                    _exit(127);
                                    break;
                                default: /* parent */
                                    /* wait child process end(for e2fsck complete) */
                                    while (pid_manual != wait(&status));

                                    if (WIFEXITED(status)){
                                        child_return_val = WEXITSTATUS(status);
                                        if (child_return_val != 0 && child_return_val != 1){
                                            printf("superblock check:canot correct the bad fs\n");
                                        }
                                    }
                                    break;
                            } 
                            break;
                        case 8:
                        case 16:
                        case 32:
                        case 128:
                            printf("begin superblock check:child_return_val = %d\n", child_return_val);
                        default:
                            /* e2fsck need furture work */
                            pid_super = fork();
                            switch (pid_super){
                                case -1:
                                    printf("superblock check:fork system call failed\n");
                                    break;
                                case 0:  /* child */
                                    /* running /system/bin/e2fsck -p -b 16384 /dev/block/~ */
                                    execl(args[1], args[1], args[2], "-b 32768", args[3], NULL);
                                    printf("superblock check:cannot run %s\n", args[1]);
                                    _exit(127);
                                    break;
                                default: /* parent */
                                    /* wait child process end(for e2fsck complete) */
                                    while (pid_super != wait(&status));

                                    if (WIFEXITED(status)){
                                        child_return_val = WEXITSTATUS(status);
                                        if (child_return_val != 0 && child_return_val != 1){
                                            printf("superblock check:canot correct the bad fs\n");
                                        }
                                    }
                                    break;
                            }
                            break;
                    }
                }
            }
            break;
    }

    return 0;

}

int do_export(int nargs, char **args)
{
    add_environment(args[1], args[2]);
    return 0;
}

int do_hostname(int nargs, char **args)
{
    return write_file("/proc/sys/kernel/hostname", args[1]);
}

int do_ifup(int nargs, char **args)
{
    return __ifupdown(args[1], 1);
}


static int do_insmod_inner(int nargs, char **args, int opt_len)
{
    char options[opt_len + 1];
    int i;

    options[0] = '\0';
    if (nargs > 2) {
        strcpy(options, args[2]);
        for (i = 3; i < nargs; ++i) {
            strcat(options, " ");
            strcat(options, args[i]);
        }
    }

    return insmod(args[1], options);
}

int do_insmod(int nargs, char **args)
{
    int i;
    int size = 0;

    if (nargs > 2) {
        for (i = 2; i < nargs; ++i)
            size += strlen(args[i]) + 1;
    }

    return do_insmod_inner(nargs, args, size);
}

int do_mkdir(int nargs, char **args)
{
    mode_t mode = 0755;
    int ret;

    /* mkdir <path> [mode] [owner] [group] */

    if (nargs >= 3) {
        mode = strtoul(args[2], 0, 8);
    }

    ret = make_dir(args[1], mode);
    /* chmod in case the directory already exists */
    if (ret == -1 && errno == EEXIST) {
        ret = _chmod(args[1], mode);
    }
    if (ret == -1) {
        return -errno;
    }

    if (nargs >= 4) {
        uid_t uid = decode_uid(args[3]);
        gid_t gid = -1;

        if (nargs == 5) {
            gid = decode_uid(args[4]);
        }

        if (_chown(args[1], uid, gid) < 0) {
            return -errno;
        }

        /* chown may have cleared S_ISUID and S_ISGID, chmod again */
        if (mode & (S_ISUID | S_ISGID)) {
            ret = _chmod(args[1], mode);
            if (ret == -1) {
                return -errno;
            }
    }
    }

    return 0;
}

int do_mknod(int nargs, char **args)
{
    dev_t dev;
    int major;
    int minor;
    int mode;

    /* mknod <path> <type> <major> <minor> */

    if (nargs != 5) {
        return -1;
    }

    major = strtoul(args[3], 0, 0);
    minor = strtoul(args[4], 0, 0);
    dev = (major << 8) | minor;

    if (strcmp(args[2], "c") == 0) {
        mode = S_IFCHR;
    } else {
        mode = S_IFBLK;
    }

    if (mknod(args[1], mode, dev)) {
        ERROR("init: mknod failed");
        return -1;
    }

    return 0;
}


static struct {
    const char *name;
    unsigned flag;
} mount_flags[] = {
    { "noatime",    MS_NOATIME },
    { "noexec",     MS_NOEXEC },
    { "nosuid",     MS_NOSUID },
    { "nodev",      MS_NODEV },
    { "nodiratime", MS_NODIRATIME },
    { "ro",         MS_RDONLY },
    { "rw",         0 },
    { "remount",    MS_REMOUNT },
    { "bind",       MS_BIND },
    { "rec",        MS_REC },
    { "unbindable", MS_UNBINDABLE },
    { "private",    MS_PRIVATE },
    { "slave",      MS_SLAVE },
    { "shared",     MS_SHARED },
    { "defaults",   0 },
    { 0,            0 },
};

#define DATA_MNT_POINT "/data"

/* mount <type> <device> <path> <flags ...> <options> */
int do_mount(int nargs, char **args)
{
    char tmp[64];
    char *source, *target, *type;
    char *options = NULL;
    unsigned flags = 0;
    int n, i;
    int wait = 0;
    //add for power loss test
    struct stat stbuf; 

    for (n = 4; n < nargs; n++) {
        for (i = 0; mount_flags[i].name; i++) {
            if (!strcmp(args[n], mount_flags[i].name)) {
                flags |= mount_flags[i].flag;
                break;
            }
        }

        if (!mount_flags[i].name) {
            if (!strcmp(args[n], "wait"))
                wait = 1;
            /* if our last argument isn't a flag, wolf it up as an option string */
            else if (n + 1 == nargs)
                options = args[n];
        }
    }

    type = args[1];
    source = args[2];
    target = args[3];

    if (!strncmp(source, "mtd@", 4)) {
        n = mtd_name_to_number(source + 4);
        if (n < 0) {
            return -1;
        }

        sprintf(tmp, "/dev/block/mtdblock%d", n);

        if (wait)
            wait_for_file(tmp, COMMAND_RETRY_TIMEOUT);
        if (mount(tmp, target, type, flags, options) < 0) {
            return -1;
        }

        goto exit_success;
#ifdef MTK_NAND_UBIFS_SUPPORT
    } else if (!strncmp(source, "ubi@", 4)) {
        n = ubi_attach_mtd(source + 4);
        if (n < 0) {
            return -1;
        }

        sprintf(tmp, "/dev/ubi%d_0", n);

        if (wait)
            wait_for_file(tmp, COMMAND_RETRY_TIMEOUT);
        if (mount(tmp, target, type, flags, options) < 0) {
            ubi_detach_dev(n);
            return -1;
        }
        goto exit_success;
#endif
    } else if (!strncmp(source, "loop@", 5)) {
        int mode, loop, fd;
        struct loop_info info;

        mode = (flags & MS_RDONLY) ? O_RDONLY : O_RDWR;
        fd = open(source + 5, mode);
        if (fd < 0) {
            return -1;
        }

        for (n = 0; ; n++) {
            sprintf(tmp, "/dev/block/loop%d", n);
            loop = open(tmp, mode);
            if (loop < 0) {
                return -1;
            }

            /* if it is a blank loop device */
            if (ioctl(loop, LOOP_GET_STATUS, &info) < 0 && errno == ENXIO) {
                /* if it becomes our loop device */
                if (ioctl(loop, LOOP_SET_FD, fd) >= 0) {
                    close(fd);

                    if (mount(tmp, target, type, flags, options) < 0) {
                        ioctl(loop, LOOP_CLR_FD, 0);
                        close(loop);
                        return -1;
                    }

                    close(loop);
                    goto exit_success;
                }
            }

            close(loop);
        }

        close(fd);
        ERROR("out of loopback devices");
        return -1;
    } else {

#ifdef MTK_EMMC_SUPPORT
         struct phone_encrypt_state ps;  
         if (!strcmp(target, DATA_MNT_POINT)) {
             if (misc_get_phone_encrypt_state(&ps) < 0) {
                 printf("Failed to get encrypted status in MISC\n");
             }
             else {
                 printf("Success: get encrypted status: 0x%x in MISC\n", ps.state);
             }  
         }
#endif

        if (wait)
            wait_for_file(source, COMMAND_RETRY_TIMEOUT);
        if (mount(source, target, type, flags, options) < 0) {

		/*auto-format the partition when the partition is empty or mount fail*/
  		if(strcmp(target,"/data")&& strcmp(target,"/cache")&& strcmp(target,"/system")&& strcmp(target,"/system/secro")){
        				int ret,status;
        				pid_t pid;
        				printf("mount  %s  to %s fail, it may be a empty partition\n",source,target);
        				pid = fork();
        				if(pid<0){
        						printf("create process fail\n");	
        						return -1;
        				}else if(pid ==0){
        						printf("create process to generate image\n");
        						execl("/system/bin/make_ext4fs","-w",source,NULL);
        						printf("can not run /system/bin/make_ext4fs\n");
        						return -1;	
        				}else{
        					 while (pid != waitpid(pid,&status,0));

        						if(status!=0){
        								printf("make_ext4fs failed on %s\n",target);
        								return -1;
        						}else{
        								printf("make_ext4fs on %s sucess!!\n",target);	
        						}
        						if (mount(source, target, type, flags, options) < 0){
							  	printf("re-mount %s fail\n",target);
							  	return -1;	
        				  		}
        						
        				}
		}

            if (!strcmp(target, DATA_MNT_POINT)) {
                int fd;
                if ((fd = open(source, O_RDONLY)) < 0) {
                     printf("Mount /data fail because source(%s) doesn't exist.", source);
                     return -1;
                }
            }

            if (!strcmp(target, DATA_MNT_POINT) && !partition_wiped(source)) {
                if (fs_mgr_do_tmpfs_mount(DATA_MNT_POINT)) {
                    return -1;
                }

                /* Set the property that triggers the framework to do a minimal
                           * startup and ask the user for a password
                           */
                property_set("ro.crypto.state", "encrypted");
                property_set("vold.decrypt", "1");
            } else {
                return -1;
            }
        }
#ifdef MTK_EMMC_SUPPORT
        else {
             if (!strcmp(target, DATA_MNT_POINT)) {
                    if (ps.state == PHONE_ENCRYPTED) {
                        ps.state = PHONE_UNCRYPTED;
                        if (misc_set_phone_encrypt_state(&ps) < 0) {
                            printf("Failed to set encrypted status to 0x%x in MISC\n", ps.state);
                        }
                        else {
                            printf("Success: Set encrypted status to 0x%x in MISC\n", ps.state);
                        }
                    }
             }
        }
#endif

        if (!strcmp(target, DATA_MNT_POINT)) {
            char fs_flags[32];

            /* Save the original mount options */
            property_set("ro.crypto.fs_type", type);
            property_set("ro.crypto.fs_real_blkdev", source);
            property_set("ro.crypto.fs_mnt_point", target);
            if (options) {
                property_set("ro.crypto.fs_options", options);
            }
            snprintf(fs_flags, sizeof(fs_flags), "0x%8.8x", flags);
            property_set("ro.crypto.fs_flags", fs_flags);
        }
        if (!strncmp(type, "ext4", 4)){
            if (!strncmp(target, "/data", 5)){
               printf("delete lost-found in data dir\n");
               system("/system/bin/rm -r /data/lost+found/*");
               
               if (stat("/data/data", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/data file\n");
                   system("/system/bin/rm -r /data/data");    
               }

               if (stat("/data/system", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/system file\n");
                   system("/system/bin/rm -r /data/system");    
               }

               if (stat("/data/misc", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/misc file\n");
                   system("/system/bin/rm -r /data/misc");    
               }

               if (stat("/data/local", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/local file\n");
                   system("/system/bin/rm -r /data/local");    
               }

               if (stat("/data/app-private", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/app-private file\n");
                   system("/system/bin/rm -r /data/app-private");    
               }

               if (stat("/data/dalvik-cache", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/dalvik-cache file\n");
                   system("/system/bin/rm -r /data/dalvik-cache");    
               }


               if (stat("/data/property", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/property file\n");
                   system("/system/bin/rm -r /data/property");    
               } 

               if (stat("/data/mvg_root", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/mvg_root file\n");
                   system("/system/bin/rm -r /data/mvg_root");    
               }

               if (stat("/data/anr", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/anr file\n");
                   system("/system/bin/rm -r /data/anr");    
               }

               if (stat("/data/app", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/app file\n");
                   system("/system/bin/rm -r /data/app");    
               }

               if (stat("/data/nvram", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/nvram file\n");
                   system("/system/bin/rm -r /data/nvram");    
               }

               if (stat("/data/secure", &stbuf) < 0){
                   printf("stat syscall fail\n");
               }
               if (S_ISREG(stbuf.st_mode)){
                   printf("delete /data/secure file\n");
                   system("/system/bin/rm -r /data/secure");    
               }
            }

            if (!strncmp(target, "/cache", 6)){
               printf("delete lost-found in cache dir\n");
               system("/system/bin/rm -r /cache/lost+found/*");
            }
        }        
    }

exit_success:
    /* If not running encrypted, then set the property saying we are
     * unencrypted, and also trigger the action for a nonencrypted system.
     */
    if (!strcmp(target, DATA_MNT_POINT)) {
        const char *prop;

        prop = property_get("ro.crypto.state");
        if (! prop) {
            prop = "notset";
        }
        if (strcmp(prop, "encrypted")) {
            property_set("ro.crypto.state", "unencrypted");
            action_for_each_trigger("nonencrypted", action_add_queue_tail);
        }
    }

    return 0;
}

int do_mount_all(int nargs, char **args)
{
    pid_t pid;
    int ret = -1;
    int child_ret = -1;
    int status;
    const char *prop;

    if (nargs != 2) {
        return -1;
    }

    /*
     * Call fs_mgr_mount_all() to mount all filesystems.  We fork(2) and
     * do the call in the child to provide protection to the main init
     * process if anything goes wrong (crash or memory leak), and wait for
     * the child to finish in the parent.
     */
    pid = fork();
    if (pid > 0) {
        /* Parent.  Wait for the child to return */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        } else {
            ret = -1;
        }
    } else if (pid == 0) {
        /* child, call fs_mgr_mount_all() */
        klog_set_level(6);  /* So we can see what fs_mgr_mount_all() does */
        child_ret = fs_mgr_mount_all(args[1]);
        if (child_ret == -1) {
            ERROR("fs_mgr_mount_all returned an error\n");
        }
        exit(child_ret);
    } else {
        /* fork failed, return an error */
        return -1;
    }

    /* ret is 1 if the device is encrypted, 0 if not, and -1 on error */
    if (ret == 1) {
        property_set("ro.crypto.state", "encrypted");
        property_set("vold.decrypt", "1");
    } else if (ret == 0) {
        property_set("ro.crypto.state", "unencrypted");
        /* If fs_mgr determined this is an unencrypted device, then trigger
         * that action.
         */
        action_for_each_trigger("nonencrypted", action_add_queue_tail);
    }

    return ret;
}

int do_setcon(int nargs, char **args) {
#ifdef HAVE_SELINUX
    if (is_selinux_enabled() <= 0)
        return 0;
    if (setcon(args[1]) < 0) {
        return -errno;
    }
#endif
    return 0;
}

int do_setenforce(int nargs, char **args) {
#ifdef HAVE_SELINUX
    if (is_selinux_enabled() <= 0)
        return 0;
    if (security_setenforce(atoi(args[1])) < 0) {
        return -errno;
    }
#endif
    return 0;
}

int do_setkey(int nargs, char **args)
{
    struct kbentry kbe;
    kbe.kb_table = strtoul(args[1], 0, 0);
    kbe.kb_index = strtoul(args[2], 0, 0);
    kbe.kb_value = strtoul(args[3], 0, 0);
    return setkey(&kbe);
}

int do_setprop(int nargs, char **args)
{
    const char *name = args[1];
    const char *value = args[2];
    char prop_val[PROP_VALUE_MAX];
    int ret;

    ret = expand_props(prop_val, value, sizeof(prop_val));
    if (ret) {
        ERROR("cannot expand '%s' while assigning to '%s'\n", value, name);
        return -EINVAL;
    }
    property_set(name, prop_val);
    return 0;
}

int do_setrlimit(int nargs, char **args)
{
    struct rlimit limit;
    int resource;
    resource = atoi(args[1]);
    limit.rlim_cur = atoi(args[2]);
    limit.rlim_max = atoi(args[3]);
    return setrlimit(resource, &limit);
}

int do_start(int nargs, char **args)
{
    struct service *svc;
    svc = service_find_by_name(args[1]);
    if (svc) {
        service_start(svc, NULL);
    }
    return 0;
}

int do_stop(int nargs, char **args)
{
    struct service *svc;
    svc = service_find_by_name(args[1]);
    if (svc) {
        service_stop(svc);
    }
    return 0;
}

int do_restart(int nargs, char **args)
{
    struct service *svc;
    svc = service_find_by_name(args[1]);
    if (svc) {
        service_stop(svc);
        service_start(svc, NULL);
    }
    return 0;
}

int do_trigger(int nargs, char **args)
{
    action_for_each_trigger(args[1], action_add_queue_tail);
    return 0;
}

int do_symlink(int nargs, char **args)
{
    return symlink(args[1], args[2]);
}

int do_rm(int nargs, char **args)
{
    return unlink(args[1]);
}

int do_rmdir(int nargs, char **args)
{
    return rmdir(args[1]);
}

int do_sysclktz(int nargs, char **args)
{
    struct timezone tz;

    if (nargs != 2)
        return -1;

    memset(&tz, 0, sizeof(tz));
    tz.tz_minuteswest = atoi(args[1]);   
    if (settimeofday(NULL, &tz))
        return -1;
    return 0;
}

int do_write(int nargs, char **args)
{
    const char *path = args[1];
    const char *value = args[2];
    char prop_val[PROP_VALUE_MAX];
    int ret;

    ret = expand_props(prop_val, value, sizeof(prop_val));
    if (ret) {
        ERROR("cannot expand '%s' while writing to '%s'\n", value, path);
        return -EINVAL;
    }
    return write_file(path, prop_val);
}

int do_copy(int nargs, char **args)
{
    char *buffer = NULL;
    int rc = 0;
    int fd1 = -1, fd2 = -1;
    struct stat info;
    int brtw, brtr;
    char *p;

    if (nargs != 3)
        return -1;

    if (stat(args[1], &info) < 0) 
        return -1;

    if ((fd1 = open(args[1], O_RDONLY)) < 0) 
        goto out_err;

    if ((fd2 = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0660)) < 0)
        goto out_err;

    if (!(buffer = malloc(info.st_size)))
        goto out_err;

    p = buffer;
    brtr = info.st_size;
    while(brtr) {
        rc = read(fd1, p, brtr);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtr -= rc;
    }

    p = buffer;
    brtw = info.st_size;
    while(brtw) {
        rc = write(fd2, p, brtw);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtw -= rc;
    }

    rc = 0;
    goto out;
out_err:
    rc = -1;
out:
    if (buffer)
        free(buffer);
    if (fd1 >= 0)
        close(fd1);
    if (fd2 >= 0)
        close(fd2);
    return rc;
}

int do_chown(int nargs, char **args) {
#if 0
    //tempararily disable this code section
    /* GID is optional. */
    if (nargs == 3) {
        if (_chown(args[2], decode_uid(args[1]), -1) < 0)
            return -errno;
    } else if (nargs == 4) {
        if (_chown(args[3], decode_uid(args[1]), decode_uid(args[2])) < 0)
            return -errno;
    } else {
        return -1;
    }
    return 0;
#else
    char tmp[64];
    char *target;
    int n;
    switch (nargs){
        case 3:
            target = args[2];
            break;
        case 4:
            target = args[3];
            break;
        default:
            ERROR("invalid args num: %d, It should be 3 or 4\n", nargs);
            return -1;
    }

    if (!strncmp(target, "mtd@", 4)) {
        n = mtd_name_to_number(target + 4);
            if (n < 0) {
            return -1;
        }
        sprintf(tmp, "/dev/mtd/mtd%d", n);
        target = tmp;

    }

    //ERROR("do_chown debug: target:%s\n",target);
    /* GID is optional. */
    if (nargs == 3) {
    if (_chown(target, decode_uid(args[1]), -1) < 0)
            return -errno;
    } else if (nargs == 4) {
        if (_chown(target, decode_uid(args[1]), decode_uid(args[2])))
            return -errno;
    } else {
        return -1;
    }
    return 0;
#endif
}

static mode_t get_mode(const char *s) {
    mode_t mode = 0;
    while (*s) {
        if (*s >= '0' && *s <= '7') {
            mode = (mode<<3) | (*s-'0');
        } else {
            return -1;
        }
        s++;
    }
    return mode;
}

int do_chmod(int nargs, char **args) {
#if 0
    //tempararily disable this code section
    mode_t mode = get_mode(args[1]);
    if (_chmod(args[2], mode) < 0) {
        return -errno;
    }
    return 0;
#else
    char tmp[64];
    char *target;
    int n;
    target = args[2];

    if (!strncmp(target, "mtd@", 4)) {
        n = mtd_name_to_number(target + 4);
            if (n < 0) {
            return -1;
        }
        sprintf(tmp, "/dev/mtd/mtd%d", n);
        target = tmp;

    }

    //ERROR("do_chmod debug: target:%s\n",target);

    mode_t mode = get_mode(args[1]);
    if (_chmod(target, mode) < 0) {
        return -errno;
    }
    return 0;
#endif
}

int do_restorecon(int nargs, char **args) {
    int i;

    for (i = 1; i < nargs; i++) {
        if (restorecon(args[i]) < 0)
            return -errno;
        }
    return 0;
}

int do_setsebool(int nargs, char **args) {
#ifdef HAVE_SELINUX
    SELboolean *b = alloca(nargs * sizeof(SELboolean));
    char *v;
    int i;

    if (is_selinux_enabled() <= 0)
        return 0;

    for (i = 1; i < nargs; i++) {
        char *name = args[i];
        v = strchr(name, '=');
        if (!v) {
            ERROR("setsebool: argument %s had no =\n", name);
            return -EINVAL;
        }
        *v++ = 0;
        b[i-1].name = name;
        if (!strcmp(v, "1") || !strcasecmp(v, "true") || !strcasecmp(v, "on"))
            b[i-1].value = 1;
        else if (!strcmp(v, "0") || !strcasecmp(v, "false") || !strcasecmp(v, "off"))
            b[i-1].value = 0;
        else {
            ERROR("setsebool: invalid value %s\n", v);
            return -EINVAL;
        }
    }

    if (security_set_boolean_list(nargs - 1, b, 0) < 0)
        return -errno;
#endif
    return 0;
}

int do_loglevel(int nargs, char **args) {
    if (nargs == 2) {
        klog_set_level(atoi(args[1]));
        return 0;
    }
    return -1;
}

int do_load_persist_props(int nargs, char **args) {
    if (nargs == 1) {
        load_persist_props();
        return 0;
    }
    return -1;
}

int do_wait(int nargs, char **args)
{
    if (nargs == 2) {
        return wait_for_file(args[1], COMMAND_RETRY_TIMEOUT);
    } else if (nargs == 3) {
        return wait_for_file(args[1], atoi(args[2]));
    } else
    return -1;
}
