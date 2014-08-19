/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/wireless.h>
#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>

#include "hardware_legacy/mtk_wifi.h"
#include "libwpa_client/wpa_ctrl.h"

#define LOG_TAG "WifiCommon"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

#define HALD_SUPPORT            
//#define WIFI_POWER_CONTROL      
//#define P2P_IOCTL               

/* PRIMARY refers to the connection on the primary interface
 * SECONDARY refers to an optional connection on a p2p interface
 *
 * For concurrency, we only support one active p2p connection and
 * one active STA connection at a time
 */
#define PRIMARY     0
#define SECONDARY   1
#define MAX_CONNS   2

#ifdef HALD_SUPPORT
#define HAL_DAEMON_CMD              "hal"
#define HAL_DAEMON_NAME             "hald"
#define HAL_DAEMON_CMD_LENGTH       255
#endif

#ifdef P2P_IOCTL
#define PRIV_CMD_P2P_MODE    28
#define IOCTL_SET_INT                   (SIOCIWFIRSTPRIV + 0)
#define IOCTL_GET_INT                   (SIOCIWFIRSTPRIV + 1)
#endif

#define WIFI_POWER_PATH                 "/dev/wmtWifi"

static struct wpa_ctrl *ctrl_conn[MAX_CONNS];
static struct wpa_ctrl *monitor_conn[MAX_CONNS];

/* socket pair used to exit from a blocking read */
static int exit_sockets[MAX_CONNS][2];

extern int do_dhcp();
extern int ifc_init();
extern void ifc_close();
extern char *dhcp_lasterror();
extern void get_dhcp_info();
extern int init_module(void *, unsigned long, const char *);
extern int delete_module(const char *, unsigned int);

static char primary_iface[PROPERTY_VALUE_MAX];
// TODO: use new ANDROID_SOCKET mechanism, once support for multiple
// sockets is in

#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG          ""
#endif
#ifndef WIFI_FIRMWARE_LOADER
#define WIFI_FIRMWARE_LOADER		""
#endif
#define WIFI_TEST_INTERFACE		"sta"
#define WIFI_INTERFACE          "wlan0"
#define WIFI_P2P_INTERFACE      "p2p0"
#define WIFI_AP_INTERFACE       "ap0"

#ifndef WIFI_DRIVER_FW_PATH_STA
#define WIFI_DRIVER_FW_PATH_STA		"STA"
#endif
#ifndef WIFI_DRIVER_FW_PATH_AP
#define WIFI_DRIVER_FW_PATH_AP		"AP"
#endif
#ifndef WIFI_DRIVER_FW_PATH_P2P
#define WIFI_DRIVER_FW_PATH_P2P		"STA+P2P"
#endif

#ifndef WIFI_DRIVER_FW_PATH_PARAM
#define WIFI_DRIVER_FW_PATH_PARAM	"/sys/module/wlan/parameters/fwpath"
#endif

#define WIFI_DRIVER_LOADER_DELAY	1000000
#define SUPP_CONNECT_POLLING_LOOP   60
#define SUPP_CONNECT_DELAY          50000

static const char IFACE_DIR[]           = "/data/misc/wpa_supplicant";
#ifdef WIFI_DRIVER_MODULE_PATH
static const char DRIVER_MODULE_NAME[]  = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME " ";
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = WIFI_DRIVER_MODULE_ARG;
#endif
static const char FIRMWARE_LOADER[]     = WIFI_FIRMWARE_LOADER;
static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";
static const char SUPPLICANT_NAME[]     = "wpa_supplicant";
static const char SUPP_PROP_NAME[]      = "init.svc.wpa_supplicant";
static const char P2P_SUPPLICANT_NAME[] = "p2p_supplicant";
static const char P2P_PROP_NAME[]       = "init.svc.p2p_supplicant";
static const char AP_DAEMON_NAME[]      = "ap_daemon";
static const char AP_PROP_NAME[]        = "init.svc.ap_daemon";
static const char SUPP_CONFIG_TEMPLATE[]= "/system/etc/wifi/wpa_supplicant.conf";
static const char P2P_CONFIG_TEMPLATE[] = "/system/etc/wifi/p2p_supplicant.conf";
static const char SUPP_CONFIG_FILE[]    = "/data/misc/wifi/wpa_supplicant.conf";
static const char P2P_CONFIG_FILE[]     = "/data/misc/wifi/p2p_supplicant.conf";
//static const char CONTROL_IFACE_PATH[]  = "/data/misc/wifi/sockets";
static const char CONTROL_IFACE_PATH[]  = "/data/misc/wpa_supplicant";
static const char MODULE_FILE[]         = "/proc/modules";

static const char SUPP_ENTROPY_FILE[]   = WIFI_ENTROPY_FILE;
static unsigned char dummy_key[21] = { 0x02, 0x11, 0xbe, 0x33, 0x43, 0x35,
                                       0x68, 0x47, 0x84, 0x99, 0xa9, 0x2b,
                                       0x1c, 0xd3, 0xee, 0xff, 0xf1, 0xe2,
                                       0xf3, 0xf4, 0xf5 };

/* Is either SUPPLICANT_NAME, P2P_SUPPLICANT_NAME or AP_DAEMON_NAME */
/* Default value is P2P_SUPPLICANT_NAME */
static char supplicant_name[PROPERTY_VALUE_MAX] = {"p2p_supplicant"};
/* Is either SUPP_PROP_NAME, P2P_PROP_NAME or AP_PROP_NAME */
/* Default value is P2P_PROP_NAME */
static char supplicant_prop_name[PROPERTY_KEY_MAX] = {"init.svc.p2p_supplicant"};

#ifdef __MTK_BT_30_HS_PAL__
extern void pal_set_wlan_up();
extern void pal_send_wlan_on_event();
extern void pal_set_wlan_down();
extern void pal_send_wlan_off_event();
#endif

static int is_primary_interface(const char *ifname)
{
    //Treat NULL as primary interface to allow control
    //on STA without an interface
    if (ifname == NULL || !strncmp(ifname, primary_iface, strlen(primary_iface))) {
        return 1;
    }
    return 0;
}

static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        ALOGD("Unable to unload driver module \"%s\": %s\n",
             modname, strerror(errno));
    return ret;
}

int do_dhcp_request(int *ipaddr, int *gateway, int *mask,
                    int *dns1, int *dns2, int *server, int *lease) {
    /* For test driver, always report success */
    if (strcmp(primary_iface, WIFI_TEST_INTERFACE) == 0)
        return 0;

    if (ifc_init() < 0)
        return -1;

    if (do_dhcp(primary_iface) < 0) {
        ifc_close();
        return -1;
    }
    ifc_close();
    get_dhcp_info(ipaddr, gateway, mask, dns1, dns2, server, lease);
    return 0;
}

const char *get_dhcp_error_string() {
    return dhcp_lasterror();
}

#ifdef HALD_SUPPORT
int halDoMonitor(int sock)
{
    char *buffer = malloc(4096);

    while(1) {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 10;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        if ((rc = select(sock +1, &read_fds, NULL, NULL, &to)) < 0) {
            ALOGE("Error in select (%s)", strerror(errno));
            free(buffer);
            close(sock);
            return errno;
        } else if (!rc) {
            continue;
            ALOGE("[TIMEOUT]");
            close(sock);
            return ETIMEDOUT;
        } else if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, 4096);
            if ((rc = read(sock, buffer, 4096)) <= 0) {
                if (rc == 0) {
                    ALOGE("Lost connection to Hald - did it crash?");
                }
                else {
                    ALOGE("Error reading data (%s)", strerror(errno));
                }
                close(sock);
                free(buffer);
                if (rc == 0) {
                    return ECONNRESET;
                }
                return errno;
            }

            int offset = 0;
            int i = 0;

            for (i = 0; i < rc; i++) {
                if (buffer[i] == '\0') {
                    int code;
                    char tmp[4];

                    strncpy(tmp, buffer + offset, 3);
                    tmp[3] = '\0';
                    code = atoi(tmp);

                    ALOGD("Hal cmd response code: \"%d\"", code);
                    if (code >= 200 && code < 600) {
                        int ret = 0;

                        switch(code) {
                            /*the requested action did not take place.*/
                            case 400:
                            case 500:
                            case 501:
                                ret = -1;
                                break;
                            /*Requested action has been successfully completed*/
                            default:
                                ret = 0;
                                break;
                        }

                        close(sock);
                        free(buffer);
                        return ret;
                    }
                    offset = i + 1;
                }
            }
        }
    }
    close(sock);
    free(buffer);
    return 0;
}

int halDoCommand(const char *cmd)
{
    int sock;
    char *final_cmd;

    if ((sock = socket_local_client(HAL_DAEMON_NAME,
                                     ANDROID_SOCKET_NAMESPACE_RESERVED,
                                     SOCK_STREAM)) < 0) {
        ALOGE("Error connecting (%s)", strerror(errno));
        //exit(4);
        /*return error if hald is not existing*/
        return errno;
    }

    asprintf(&final_cmd, "%s %s", HAL_DAEMON_CMD, cmd);

    ALOGD("Hal cmd: \"%s\"", final_cmd);

    if (write(sock, final_cmd, strlen(final_cmd) + 1) < 0) {
        free(final_cmd);
	close(sock);
        ALOGE("Hal cmd error: \"%s\"", final_cmd);
        return errno;
    }
    free(final_cmd);
    return halDoMonitor(sock);
}
#endif

#ifdef WIFI_POWER_CONTROL
int wifi_set_power(int enable) {
    int sz;
    int fd = -1;
    const char buffer = (enable ? '1' : '0');

    fd = open(WIFI_POWER_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("Open \"%s\" failed", WIFI_POWER_PATH);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("Set \"%s\" [%c] failed", WIFI_POWER_PATH, buffer);
        goto out;
    }

out:
    if (fd >= 0) close(fd);
    return 0;
}
#else
int wifi_set_power(int enable) {
    if(enable) {
        ALOGD("wifi_set_power on!");
        return halDoCommand("load wifi");
    }
    else {
        ALOGD("wifi_set_power off!");
        return halDoCommand("unload wifi");
    }
}
#endif

#ifdef P2P_IOCTL
int wifi_set_p2p_mode(int enable, int mode) {
    struct iwreq wrq = {0};
    int i = 0, skfd = 0;
    int param[2];
    int ret;
    
    param[0] = enable;
    param[1] = mode;
    
    /* initialize socket */
    skfd = socket(PF_INET, SOCK_DGRAM, 0);
    
    wrq.u.data.pointer = &(param[0]);
    wrq.u.data.length = 2;
    wrq.u.mode = PRIV_CMD_P2P_MODE;
    memcpy(wrq.u.name + 4, param, sizeof(int) * 2);
    
    strncpy(wrq.ifr_name, WIFI_INTERFACE, IFNAMSIZ);

    /* do ioctl */
    ret = ioctl(skfd, IOCTL_SET_INT, &wrq);
    if (ret >= 0) {
        ALOGD("SET_P2P_MODE enable[%d], mode[%d] Success", enable, mode);
    } else {
        ALOGE("SET_P2P_MODE enable[%d], mode[%d] Failed", enable, mode);
        ALOGE("%s", strerror(errno));
    }
    close(skfd);
    
    return ret;
}
#else
int wifi_set_p2p_mode(int enable, int mode) {
    if(enable) {
        if(mode) {
            return halDoCommand("load hotspot");
        }
        else {
            return halDoCommand("load p2p");
        }
    }
    else {
        halDoCommand("unload p2p");
        halDoCommand("unload hotspot");
    }
    
    return 0;
}
#endif

int is_wifi_driver_loaded() {
    char driver_status[PROPERTY_VALUE_MAX];
#ifdef WIFI_DRIVER_MODULE_PATH
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];
#endif
   
    if (!property_get(DRIVER_PROP_NAME, driver_status, NULL)
            || strcmp(driver_status, "ok") != 0) {
        ALOGD("Check driver status: not loaded");
        return 0;  /* driver not loaded */
    }
#ifdef WIFI_DRIVER_MODULE_PATH
    /*
     * If the property says the driver is loaded, check to
     * make sure that the property setting isn't just left
     * over from a previous manual shutdown or a runtime
     * crash.
     */
    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        ALOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        property_set(DRIVER_PROP_NAME, "unloaded");
        ALOGD("Driver is not loaded.");
        return 0;
    }
    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            ALOGD("Driver is loaded.");
            return 1;
        }
    }
    fclose(proc);
    property_set(DRIVER_PROP_NAME, "unloaded");
    return 0;
#else
    ALOGD("Check driver status: loaded");
    return 1;
#endif
}

int wifi_load_driver()
{
#ifdef WIFI_DRIVER_MODULE_PATH
    char driver_status[PROPERTY_VALUE_MAX];
    int count = 100; /* wait at most 20 seconds for completion */

    if (is_wifi_driver_loaded()) {
        //Enable power HERE
        wifi_set_power(1);
        return 0;
    }

    if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0)
        return -1;

    if (strcmp(FIRMWARE_LOADER,"") == 0) {
        /* usleep(WIFI_DRIVER_LOADER_DELAY); */
        property_set(DRIVER_PROP_NAME, "ok");
    }
    else {
        property_set("ctl.start", FIRMWARE_LOADER);
    }
    sched_yield();
    while (count-- > 0) {
        if (property_get(DRIVER_PROP_NAME, driver_status, NULL)) {
            if (strcmp(driver_status, "ok") == 0) {
                //Enable power HERE
                wifi_set_power(1);
                return 0;
            }
            else if (strcmp(DRIVER_PROP_NAME, "failed") == 0) {
                wifi_unload_driver();
                return -1;
            }
        }
        usleep(200000);
    }
    property_set(DRIVER_PROP_NAME, "timeout");
    wifi_unload_driver();
    return -1;
#else
    ALOGD("wifi_load_driver");
    
    if(0 > wifi_set_power(1))
	return -1;
    
    property_set(DRIVER_PROP_NAME, "ok");
    return 0;
#endif
}

int wifi_unload_driver()
{
    //usleep(200000); /* allow to finish interface down */
    
    ALOGD("wifi_unload_driver");   
    
#ifdef WIFI_DRIVER_MODULE_PATH
    if (rmmod(DRIVER_MODULE_NAME) == 0) {
        int count = 20; /* wait at most 10 seconds for completion */
        while (count-- > 0) {
            if (!is_wifi_driver_loaded())
                break;
            usleep(500000);
        }
        usleep(500000); /* allow card removal */
        if (count) {
            return 0;
        }
        return -1;
    } else
        return -1;
#else
    //Disable P2P/AP 
    if (wifi_set_p2p_mode(0, 0) < 0) {
        //Failed
    }
    
    //Disable power HERE
    property_set(DRIVER_PROP_NAME, "unloaded");
    return wifi_set_power(0);;
#endif
}

int ensure_entropy_file_exists()
{
    int ret;
    int destfd;

    ALOGV("ensure_entropy_file_exists");
    
    ret = access(SUPP_ENTROPY_FILE, R_OK|W_OK);
    if ((ret == 0) || (errno == EACCES)) {
        if ((ret != 0) &&
            (chmod(SUPP_ENTROPY_FILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
            ALOGE("Cannot set RW to \"%s\": %s", SUPP_ENTROPY_FILE, strerror(errno));
            return -1;
        }
        return 0;
    }
    destfd = TEMP_FAILURE_RETRY(open(SUPP_ENTROPY_FILE, O_CREAT|O_RDWR, 0660));
    if (destfd < 0) {
        ALOGE("Cannot create \"%s\": %s", SUPP_ENTROPY_FILE, strerror(errno));
        return -1;
    }

    if (TEMP_FAILURE_RETRY(write(destfd, dummy_key, sizeof(dummy_key))) != sizeof(dummy_key)) {
        ALOGE("Error writing \"%s\": %s", SUPP_ENTROPY_FILE, strerror(errno));
        close(destfd);
        return -1;
    }
    close(destfd);

    /* chmod is needed because open() didn't set permisions properly */
    if (chmod(SUPP_ENTROPY_FILE, 0660) < 0) {
        ALOGE("Error changing permissions of %s to 0660: %s",
             SUPP_ENTROPY_FILE, strerror(errno));
        unlink(SUPP_ENTROPY_FILE);
        return -1;
    }

    if (chown(SUPP_ENTROPY_FILE, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
             SUPP_ENTROPY_FILE, AID_WIFI, strerror(errno));
        unlink(SUPP_ENTROPY_FILE);
        return -1;
    }
    return 0;
}

int update_ctrl_interface(const char *config_file) {

    int srcfd, destfd;
    int nread;
    char ifc[PROPERTY_VALUE_MAX];
    char *pbuf;
    char *sptr;
    struct stat sb;
    
    ALOGV("update_ctrl_interface %s", config_file);

    if (stat(config_file, &sb) != 0)
        return -1;

    pbuf = malloc(sb.st_size + PROPERTY_VALUE_MAX);
    if (!pbuf)
        return 0;
    srcfd = TEMP_FAILURE_RETRY(open(config_file, O_RDONLY));
    if (srcfd < 0) {
        ALOGE("Cannot open \"%s\": %s", config_file, strerror(errno));
        free(pbuf);
        return 0;
    }
    nread = TEMP_FAILURE_RETRY(read(srcfd, pbuf, sb.st_size));
    close(srcfd);
    if (nread < 0) {
        ALOGE("Cannot read \"%s\": %s", config_file, strerror(errno));
        free(pbuf);
        return 0;
    }

#if 0    
    if (!strcmp(config_file, SUPP_CONFIG_FILE)) {
        property_get("wifi.interface", ifc, WIFI_TEST_INTERFACE);
    } else {
        strcpy(ifc, CONTROL_IFACE_PATH);
    }
#else
    strcpy(ifc, IFACE_DIR);
#endif    
    
    if ((sptr = strstr(pbuf, "ctrl_interface="))) {
        char *iptr = sptr + strlen("ctrl_interface=");
        int ilen = 0;
        int mlen = strlen(ifc);
        int nwrite;
        if (strncmp(ifc, iptr, mlen) != 0) {
            ALOGE("ctrl_interface != %s", ifc);
            while (((ilen + (iptr - pbuf)) < nread) && (iptr[ilen] != '\n'))
                ilen++;
            mlen = ((ilen >= mlen) ? ilen : mlen) + 1;
            memmove(iptr + mlen, iptr + ilen + 1, nread - (iptr + ilen + 1 - pbuf));
            memset(iptr, '\n', mlen);
            memcpy(iptr, ifc, strlen(ifc));
            destfd = TEMP_FAILURE_RETRY(open(config_file, O_RDWR, 0660));
            if (destfd < 0) {
                ALOGE("Cannot update \"%s\": %s", config_file, strerror(errno));
                free(pbuf);
                return -1;
            }
            TEMP_FAILURE_RETRY(write(destfd, pbuf, nread + mlen - ilen -1));
            close(destfd);
        }
    }
    free(pbuf);
    return 0;
}

int ensure_config_file_exists_common(const char *config_file, const char *config_template)
{
    char buf[2048];
    int srcfd, destfd;
    struct stat sb;
    int nread;
    int ret;

    ALOGV("ensure_config_file_exists: %s", config_file);
    
    ret = access(config_file, R_OK|W_OK);
    if ((ret == 0) || (errno == EACCES)) {
        if ((ret != 0) &&
            (chmod(config_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
            ALOGE("Cannot set RW to \"%s\": %s", config_file, strerror(errno));
            return -1;
        }
        /* return if filesize is at least 10 bytes */
        if (stat(config_file, &sb) == 0 && sb.st_size > 10) {
            return update_ctrl_interface(config_file);
        }
    } else if (errno != ENOENT) {
        ALOGE("Cannot access \"%s\": %s", config_file, strerror(errno));
        return -1;
    }

    srcfd = TEMP_FAILURE_RETRY(open(config_template, O_RDONLY));
    if (srcfd < 0) {
        ALOGE("Cannot open \"%s\": %s", config_template, strerror(errno));
        return -1;
    }

    destfd = TEMP_FAILURE_RETRY(open(config_file, O_CREAT|O_RDWR, 0660));
    if (destfd < 0) {
        close(srcfd);
        ALOGE("Cannot create \"%s\": %s", config_file, strerror(errno));
        return -1;
    }

    while ((nread = TEMP_FAILURE_RETRY(read(srcfd, buf, sizeof(buf)))) != 0) {
        if (nread < 0) {
            ALOGE("Error reading \"%s\": %s", config_template, strerror(errno));
            close(srcfd);
            close(destfd);
            unlink(config_file);
            return -1;
        }
        TEMP_FAILURE_RETRY(write(destfd, buf, nread));
    }

    close(destfd);
    close(srcfd);

    /* chmod is needed because open() didn't set permisions properly */
    if (chmod(config_file, 0660) < 0) {
        ALOGE("Error changing permissions of %s to 0660: %s",
             config_file, strerror(errno));
        unlink(config_file);
        return -1;
    }

    if (chown(config_file, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
             config_file, AID_WIFI, strerror(errno));
        unlink(config_file);
        return -1;
    }
    return update_ctrl_interface(config_file);
}

int ensure_config_file_exists(const char *config_file)
{
    if(strcmp(config_file, SUPP_CONFIG_FILE) == 0) {
        return ensure_config_file_exists_common(config_file, SUPP_CONFIG_TEMPLATE);
    }
    else if(strcmp(config_file, P2P_CONFIG_FILE) == 0) {
        return ensure_config_file_exists_common(config_file, P2P_CONFIG_TEMPLATE);
    }
    
    return -1;
}

/**
 * wifi_wpa_ctrl_cleanup() - Delete any local UNIX domain socket files that
 * may be left over from clients that were previously connected to
 * wpa_supplicant. This keeps these files from being orphaned in the
 * event of crashes that prevented them from being removed as part
 * of the normal orderly shutdown.
 */
void wifi_wpa_ctrl_cleanup(void)
{
    DIR *dir;
    struct dirent entry;
    struct dirent *result;
    size_t dirnamelen;
    size_t maxcopy;
    char pathname[PATH_MAX];
    char *namep;
    char *local_socket_dir = CONFIG_CTRL_IFACE_CLIENT_DIR;
    char *local_socket_prefix = CONFIG_CTRL_IFACE_CLIENT_PREFIX;

    ALOGV("wifi_wpa_ctrl_cleanup");
    
    if ((dir = opendir(local_socket_dir)) == NULL)
        return;

    dirnamelen = (size_t)snprintf(pathname, sizeof(pathname), "%s/", local_socket_dir);
    if (dirnamelen >= sizeof(pathname)) {
        closedir(dir);
        return;
    }
    namep = pathname + dirnamelen;
    maxcopy = PATH_MAX - dirnamelen;
    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
        if (strncmp(entry.d_name, local_socket_prefix, strlen(local_socket_prefix)) == 0) {
            if (strlcpy(namep, entry.d_name, maxcopy) < maxcopy) {
                unlink(pathname);
            }
        }
    }
    closedir(dir);
}

int wifi_start_supplicant(int p2p_supported)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 400; /* wait at most 20 seconds for completion */
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    const prop_info *pi;
    unsigned serial = 0, i;
#endif

    ALOGD("wifi_start_supplicant [%d]", p2p_supported);

    if (p2p_supported) {
    
        if(p2p_supported == 1) {
            /* start p2p_supplicant */
            strcpy(supplicant_name, P2P_SUPPLICANT_NAME);
            strcpy(supplicant_prop_name, P2P_PROP_NAME);
        }
        else {
            /* start ap daemon */
            strcpy(supplicant_name, AP_DAEMON_NAME);
            strcpy(supplicant_prop_name, AP_PROP_NAME);            
        }
        
        /* Ensure p2p config file is created */
        if (ensure_config_file_exists(P2P_CONFIG_FILE) < 0) {
            ALOGE("Failed to create a p2p config file");
            return -1;
        }

    } else {
        strcpy(supplicant_name, SUPPLICANT_NAME);
        strcpy(supplicant_prop_name, SUPP_PROP_NAME);
    }

    /* Check whether already running */
    if (property_get(supplicant_name, supp_status, NULL)
            && strcmp(supp_status, "running") == 0) {
#ifdef __MTK_BT_30_HS_PAL__	
        pal_set_wlan_up();
	    pal_send_wlan_on_event();
        ALOGD("[PAL] wifi_start_supplicant pass\n");
#endif
        return 0;
    }

    /* Before starting the daemon, make sure its config file exists */
    if (ensure_config_file_exists(SUPP_CONFIG_FILE) < 0) {
        ALOGE("Wi-Fi will not be enabled");
        return -1;
    }

    if (ensure_entropy_file_exists() < 0) {
        ALOGE("Wi-Fi entropy file was not created");
    }

    /* Clear out any stale socket files that might be left over. */
    wifi_wpa_ctrl_cleanup();

    /* Reset sockets used for exiting from hung state */
    for (i=0; i<MAX_CONNS; i++) {
        exit_sockets[i][0] = exit_sockets[i][1] = -1;
    }
    
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    /*
     * Get a reference to the status property, so we can distinguish
     * the case where it goes stopped => running => stopped (i.e.,
     * it start up, but fails right away) from the case in which
     * it starts in the stopped state and never manages to start
     * running at all.
     */
    pi = __system_property_find(supplicant_prop_name);
    if (pi != NULL) {
        serial = pi->serial;
    }
#endif
    property_get("wifi.interface", primary_iface, WIFI_TEST_INTERFACE);

    ALOGD("Start \"%s\"", supplicant_name);
    property_set("ctl.start", supplicant_name);
    sched_yield();

    while (count-- > 0) {
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
        if (pi == NULL) {
            pi = __system_property_find(supplicant_prop_name);
        }
        if (pi != NULL) {
            __system_property_read(pi, NULL, supp_status);
            if (strcmp(supp_status, "running") == 0) {
#ifdef __MTK_BT_30_HS_PAL__	
            pal_set_wlan_up();
	        pal_send_wlan_on_event();
            ALOGD("[PAL] wifi_start_supplicant pass\n");
#endif
                return 0;
            } else if (pi->serial != serial &&
                    strcmp(supp_status, "stopped") == 0) {
                return -1;
            }
        }
#else
        if (property_get(supplicant_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "running") == 0)
#ifdef __MTK_BT_30_HS_PAL__	
            pal_set_wlan_up();
	        pal_send_wlan_on_event();
            ALOGD("[PAL] wifi_start_supplicant pass\n");
#endif
                return 0;
        }
#endif
        usleep(50000);
    }
    return -1;
}

int wifi_ap_start_supplicant()
{
    int ret = 0;
    
    ALOGD("wifi_ap_start_supplicant");

    //Start ap daemon
    if (wifi_start_supplicant(2) < 0) {
        ret = -1;
    }
    
    return ret;
}

int wifi_stop_supplicant()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds for completion */
  
    ALOGD("Stop \"%s\"", supplicant_name);
  
    /* Check whether supplicant already stopped */
    if (property_get(supplicant_prop_name, supp_status, NULL)
        && strcmp(supp_status, "stopped") == 0) {
#ifdef __MTK_BT_30_HS_PAL__	
        pal_set_wlan_down();
        pal_send_wlan_off_event();    
        ALOGD("[PAL] wifi_stop_supplicant pass\n");
#endif
        return 0;
    }
   
    property_set("ctl.stop", supplicant_name);
    sched_yield();

    while (count-- > 0) {
        if (property_get(supplicant_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "stopped") == 0){
#ifdef __MTK_BT_30_HS_PAL__	
            pal_set_wlan_down();
            pal_send_wlan_off_event();    
            ALOGD("[PAL] wifi_stop_supplicant pass\n");
#endif
                return 0;
            }
        }
        usleep(100000);
    }
    return -1;
}

int wifi_ap_stop_supplicant()
{
    int ret = 0;
    char buffer[32];
    size_t len = 32;
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds for completion */    
    
    ALOGD("wifi_ap_stop_supplicant");
    
#if 0
    if(wifi_stop_supplicant() < 0) {
        ret = -1;
    }    
#else    
    //Stop p2p_supplicant
    wifi_connect_to_supplicant(WIFI_AP_INTERFACE);
    if(wifi_command(WIFI_AP_INTERFACE, "TERMINATE", buffer, &len) < 0) {
        if(wifi_stop_supplicant() < 0) {
            ret = -1;
        }
    }
    else {
        while (count-- > 0) {
            if (property_get(supplicant_prop_name, supp_status, NULL)) {
                if (strcmp(supp_status, "stopped") == 0) {
                    break;
                }
            }
            usleep(100000);
        }    
        if(count <= 0) {
            ret = -1;
        }
    }
    wifi_close_supplicant_connection(WIFI_AP_INTERFACE);
#endif
    
    return ret;
}

int wifi_connect_on_socket_path(int index, const char *path)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    
    /* Make sure supplicant is running */
    if (!property_get(supplicant_prop_name, supp_status, NULL)
            || strcmp(supp_status, "running") != 0) {
        ALOGV("Supplicant not running, cannot connect");
        return -1;
    }

    ctrl_conn[index] = wpa_ctrl_open(path);
    if (ctrl_conn[index] == NULL) {
        ALOGV("Unable to open connection to supplicant on \"%s\": %s",
             path, strerror(errno));
        return -1;
    }
    monitor_conn[index] = wpa_ctrl_open(path);
    if (monitor_conn[index] == NULL) {
        wpa_ctrl_close(ctrl_conn[index]);
        ctrl_conn[index] = NULL;
        return -1;
    }
    if (wpa_ctrl_attach(monitor_conn[index]) != 0) {
        wpa_ctrl_close(monitor_conn[index]);
        wpa_ctrl_close(ctrl_conn[index]);
        ctrl_conn[index] = monitor_conn[index] = NULL;
        return -1;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets[index]) == -1) {
        wpa_ctrl_close(monitor_conn[index]);
        wpa_ctrl_close(ctrl_conn[index]);
        ctrl_conn[index] = monitor_conn[index] = NULL;
        return -1;
    }

    return 0;
}

/* Establishes the control and monitor socket connections on the interface */
int wifi_connect_to_supplicant(const char *ifname)
{
    char path[256];
    int count = SUPP_CONNECT_POLLING_LOOP;
    int ret = 0;
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    ALOGD("[%s]wifi_connect_to_supplicant", ifname);
    
    if(!strcmp(ifname, WIFI_AP_INTERFACE) && 
      !(!property_get(AP_PROP_NAME, supp_status, NULL)
            || strcmp(supp_status, "running") != 0)) {
        strcpy(supplicant_name, AP_DAEMON_NAME);
        strcpy(supplicant_prop_name, AP_PROP_NAME);
        strcpy(primary_iface, WIFI_INTERFACE);
    }
                   
    while (count-- > 0) { 
        if (is_primary_interface(ifname)) {
            if (access(IFACE_DIR, F_OK) == 0) {
                snprintf(path, sizeof(path), "%s/%s", IFACE_DIR, primary_iface);
            } else {
                strlcpy(path, primary_iface, sizeof(path));
            }
            ret = wifi_connect_on_socket_path(PRIMARY, path);
        } else {
            sprintf(path, "%s/%s", CONTROL_IFACE_PATH, ifname);
            ret = wifi_connect_on_socket_path(SECONDARY, path);
        }
        
        if(ret == 0) {
            ALOGV("[%s]wifi_connect_to_supplicant OK", ifname);
            break;
        }
        ALOGV("[%s]wifi_connect_to_supplicant Retry, sleep %dus", ifname, SUPP_CONNECT_DELAY);
        usleep(SUPP_CONNECT_DELAY);
    }
    
    if(count <= 0) {
        ALOGE("[%s]wifi_connect_to_supplicant Failed!!", ifname);
    }
   
    return ret;
}

int wifi_send_command(int index, const char *cmd, char *reply, size_t *reply_len)
{
    int ret;

    if (ctrl_conn[index] == NULL) {
        ALOGV("Not connected to wpa_supplicant - \"%s\" command dropped.\n", cmd);
        return -1;
    }
    ret = wpa_ctrl_request(ctrl_conn[index], cmd, strlen(cmd), reply, reply_len, NULL);
    if (ret == -2) {
        //ALOGD("'%s' command timed out.\n", cmd);
        /* unblocks the monitor receive socket for termination */
        TEMP_FAILURE_RETRY(write(exit_sockets[index][0], "T", 1));
        return -2;
    } else if (ret < 0 || strncmp(reply, "FAIL", 4) == 0) {
        //ALOGD("'%s' command failed.\n", cmd);
        return -1;
    }
    if (strncmp(cmd, "PING", 4) == 0) {
        reply[*reply_len] = '\0';
    }
    return 0;
}

int wifi_ctrl_recv(int index, char *reply, size_t *reply_len)
{
    int res;
    int ctrlfd = wpa_ctrl_get_fd(monitor_conn[index]);
    struct pollfd rfds[2];
    
    memset(rfds, 0, 2 * sizeof(struct pollfd));
    rfds[0].fd = ctrlfd;
    rfds[0].events |= POLLIN;
    rfds[1].fd = exit_sockets[index][1];
    rfds[1].events |= POLLIN;
    res = TEMP_FAILURE_RETRY(poll(rfds, 2, -1));
    if (res < 0) {
        ALOGE("Error poll = %d", res);
        return res;
    }
    /* Error handling */
    if ((rfds[1].revents & POLLIN) || (monitor_conn[index] == NULL)) {
        return -2;
    } else if (rfds[0].revents & POLLIN) { 
        return wpa_ctrl_recv(monitor_conn[index], reply, reply_len);
    } else {
        return -2;
    }
    return 0;
}

int wifi_wait_on_socket(int index, char *buf, size_t buflen)
{
    size_t nread = buflen - 1;
    int fd;
    fd_set rfds;
    int result;
    struct timeval tval;
    struct timeval *tptr;

    if (monitor_conn[index] == NULL) {
        ALOGD("Connection closed\n");
        strncpy(buf, WPA_EVENT_TERMINATING " - connection closed", buflen-1);
        buf[buflen-1] = '\0';
        return strlen(buf);
    }

    result = wifi_ctrl_recv(index, buf, &nread);

    /* Terminate reception on exit socket */
    if (result == -2) {
        strncpy(buf, WPA_EVENT_TERMINATING " - connection closed", buflen-1);
        buf[buflen-1] = '\0';
        return strlen(buf);
    }

    if (result < 0) {
        ALOGD("wifi_ctrl_recv failed: %s\n", strerror(errno));
        strncpy(buf, WPA_EVENT_TERMINATING " - recv error", buflen-1);
        buf[buflen-1] = '\0';
        return strlen(buf);
    }
    buf[nread] = '\0';
    /* Check for EOF on the socket */
    if (result == 0 && nread == 0) {
        /* Fabricate an event to pass up */
        ALOGD("Received EOF on supplicant socket\n");
        strncpy(buf, WPA_EVENT_TERMINATING " - signal 0 received", buflen-1);
        buf[buflen-1] = '\0';
        return strlen(buf);
    }
    /*
     * Events strings are in the format
     *
     *     <N>CTRL-EVENT-XXX 
     *
     * where N is the message level in numerical form (0=VERBOSE, 1=DEBUG,
     * etc.) and XXX is the event name. The level information is not useful
     * to us, so strip it off.
     */
    if (buf[0] == '<') {
        char *match = strchr(buf, '>');
        if (match != NULL) {
            nread -= (match+1-buf);
            memmove(buf, match+1, nread+1);
        }
    }

    return nread;
}

int wifi_wait_for_event(const char *ifname, char *buf, size_t buflen)
{
    int ret = 0;
    
    //ALOGD("[%s]wifi_wait_for_event", ifname);
    if (is_primary_interface(ifname)) {
        ret = wifi_wait_on_socket(PRIMARY, buf, buflen);
    } else {
        ret = wifi_wait_on_socket(SECONDARY, buf, buflen);
    }
    
    ALOGD("[%s]GET \"%s\"", ifname, buf);
    
    return ret;
}

void wifi_close_sockets(int index)
{
    if (ctrl_conn[index] != NULL) {
        wpa_ctrl_close(ctrl_conn[index]);
        ctrl_conn[index] = NULL;
    }

    if (monitor_conn[index] != NULL) {
        wpa_ctrl_close(monitor_conn[index]);
        monitor_conn[index] = NULL;
    }

    if (exit_sockets[index][0] >= 0) {
        close(exit_sockets[index][0]);
        exit_sockets[index][0] = -1;
    }

    if (exit_sockets[index][1] >= 0) {
        close(exit_sockets[index][1]);
        exit_sockets[index][1] = -1;
    }
}

void wifi_close_supplicant_connection(const char *ifname)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds to ensure init has stopped stupplicant */

    ALOGD("[%s]wifi_close_supplicant_connection", ifname);
    
    if (is_primary_interface(ifname)) {
        wifi_close_sockets(PRIMARY);
    } else {
        /* p2p socket termination needs unblocking the monitor socket
         * STA connection does not need it since supplicant gets shutdown
         */
        TEMP_FAILURE_RETRY(write(exit_sockets[SECONDARY][0], "T", 1));
        wifi_close_sockets(SECONDARY);
        //closing p2p connection does not need a wait on
        //supplicant stop
        return;
    }

    while (count-- > 0) {
        if (property_get(supplicant_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "stopped") == 0)
                return;
        }
        usleep(100000);
    }
}

int wifi_command(const char *ifname, const char *command, char *reply, size_t *reply_len)
{
    int ret = 0;
    
    // printf WLAN native command in advance
    ALOGD("[%s] SET \"%s\"",ifname, command);
    if (is_primary_interface(ifname)) {
        ret = wifi_send_command(PRIMARY, command, reply, reply_len);
    } else {
        ret = wifi_send_command(SECONDARY, command, reply, reply_len);
    }
    
    if (ret == 0) {
        //ALOGD("[%s]SET \"%s\"",ifname, command);
    }
    else if (ret == -2) {
        ALOGD("[%s]SET \"%s\" Timeout",ifname, command);
    }
    else {
        ALOGD("[%s]SET \"%s\" Failed",ifname, command);
    }
    
    return ret;
}

const char *wifi_get_fw_path(int fw_type)
{
    ALOGV("wifi_get_fw_path [%d]", fw_type);
    switch (fw_type) {
    case WIFI_GET_FW_PATH_STA:
        return WIFI_DRIVER_FW_PATH_STA;
    case WIFI_GET_FW_PATH_AP:
        return WIFI_DRIVER_FW_PATH_AP;
    case WIFI_GET_FW_PATH_P2P:
        return WIFI_DRIVER_FW_PATH_P2P;
    }
    return NULL;
}

int wifi_change_fw_path(const char *fwpath)
{
    ALOGD("wifi_change_fw_path [%s]", fwpath);
    
    //Error handling, ensure driver is running
    //wifi_set_power(1);  
   
    if((strcmp(fwpath, WIFI_DRIVER_FW_PATH_STA) == 0) || 
       (strcmp(fwpath, WIFI_DRIVER_FW_PATH_P2P) == 0)) {
        return wifi_set_p2p_mode(1, 0);
    }
    else if(strcmp(fwpath, WIFI_DRIVER_FW_PATH_AP) == 0){
        return wifi_set_p2p_mode(1, 1);
    }
    
    return 0;
}

int wifi_get_own_addr(char *mac) 
{
    struct ifreq ifr= {0};
    int driver_loaded = 0;
    int skfd = 0;
    int ret, read_cnt = 0;
    unsigned char *own_addr;
    
    if (is_wifi_driver_loaded()) {
        driver_loaded = 1;   
    } else {     
        wifi_load_driver();
    }
    
    /* initialize socket */
    skfd = socket(PF_INET, SOCK_DGRAM, 0);
    
    strncpy(ifr.ifr_name, WIFI_INTERFACE, IFNAMSIZ);
    ifr.ifr_name[strlen(WIFI_INTERFACE)] = '\0';

    /* do ioctl */
    while((ret = ioctl(skfd, SIOCGIFHWADDR, &ifr)) < 0) {
        usleep(20000);                                          // wait for 20 ms
        read_cnt++;
        if (read_cnt > 100) {
            ALOGE("Netowrk %s UP timeout(2 seconds)", WIFI_INTERFACE);
            break;    
        }
    };    
    if (ret >= 0) {
        own_addr = ifr.ifr_hwaddr.sa_data;
        *own_addr |= 0x02;                                  // change to P2P mac address
        sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
        *own_addr, *(own_addr + 1), *(own_addr + 2), *(own_addr + 3), *(own_addr + 4), *(own_addr + 5));  
        if (read_cnt) {
            ALOGD("Network UP takes %d ms", read_cnt * 20);
        }
        ALOGD("GET_MAC_ADDR Success MAC %s", mac);
    } else {
        ALOGE("GET_MAC_ADDR Failed %s", strerror(errno));
    }
    
    close(skfd);

    if (driver_loaded == 0) {
        wifi_unload_driver();
    }
    return ret;

}
int wifi_build_cred(char *cred, size_t *cred_len)
{
    unsigned char ssid[36];
    size_t ssid_len;
    unsigned char passphrase[12];
    unsigned char *p;
    
    memcpy(ssid, P2P_WILDCARD_SSID, P2P_WILDCARD_SSID_LEN);
    wpa_ctrl_randum((char *) &ssid[P2P_WILDCARD_SSID_LEN], 2);
    // Add ssid postfix
    // memcpy(&ssid[P2P_WILDCARD_SSID_LEN + 2], ssid_postfix, ssid_postfix_len);
    ssid_len = P2P_WILDCARD_SSID_LEN + 2;
    ssid[ssid_len] = '\0';
    wpa_ctrl_randum(passphrase, 8);
    passphrase[8] = '\0';
    p = cred;
    p += sprintf(p, "ssid=%s", ssid);
    p += sprintf(p, " auth_type=0x0020");
    p += sprintf(p, " encr_type=0x0008");
    p += sprintf(p, " psk=%s", passphrase);
    *cred_len = strlen(cred);
    ALOGD("Build Credential %s", cred);
    return 0;
}
