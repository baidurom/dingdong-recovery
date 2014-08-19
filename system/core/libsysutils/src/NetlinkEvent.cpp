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
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "NetlinkEvent"
#include <cutils/log.h>

#include <sysutils/NetlinkEvent.h>
#include <cutils/properties.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter_ipv4/ipt_ULOG.h>
/* From kernel's net/netfilter/xt_quota2.c */
const int QLOG_NL_EVENT  = 112;

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>

const int NetlinkEvent::NlActionUnknown = 0;
const int NetlinkEvent::NlActionAdd = 1;
const int NetlinkEvent::NlActionRemove = 2;
const int NetlinkEvent::NlActionChange = 3;
const int NetlinkEvent::NlActionLinkUp = 4;
const int NetlinkEvent::NlActionLinkDown = 5;
const int NetlinkEvent::NlActionIPv6Enable = 6;
const int NetlinkEvent::NlActionIPv6Disable = 7;

NetlinkEvent::NetlinkEvent() {
    mAction = NlActionUnknown;
    memset(mParams, 0, sizeof(mParams));
    mPath = NULL;
    mSubsystem = NULL;
}

NetlinkEvent::~NetlinkEvent() {
    int i;
    if (mPath)
        free(mPath);
    if (mSubsystem)
        free(mSubsystem);
    for (i = 0; i < NL_PARAMS_MAX; i++) {
        if (!mParams[i])
            break;
        free(mParams[i]);
    }
}

void NetlinkEvent::dump() {
    int i;
	SLOGD("NL action '%d'\n", mAction);
	SLOGD("NL subsystem '%s'\n", mSubsystem);

    for (i = 0; i < NL_PARAMS_MAX; i++) {
        if (!mParams[i])
            break;
        SLOGD("NL param '%s'\n", mParams[i]);
    }
}

/*
 * Parse an binary message from a NETLINK_ROUTE netlink socket.
 */
bool NetlinkEvent::parseBinaryNetlinkMessage(char *buffer, int size) {
    size_t sz = size;
    const struct nlmsghdr *nh = (struct nlmsghdr *) buffer;

    while (NLMSG_OK(nh, sz) && (nh->nlmsg_type != NLMSG_DONE)) {

        if (nh->nlmsg_type == RTM_NEWLINK) {
            int len = nh->nlmsg_len - sizeof(*nh);
            struct ifinfomsg *ifi;

            if (sizeof(*ifi) > (size_t) len) {
                SLOGE("Got a short RTM_NEWLINK message\n");
                continue;
            }

            ifi = (ifinfomsg *)NLMSG_DATA(nh);
            if ((ifi->ifi_flags & IFF_LOOPBACK) != 0) {
                continue;
            }

            struct rtattr *rta = (struct rtattr *)
              ((char *) ifi + NLMSG_ALIGN(sizeof(*ifi)));
            len = NLMSG_PAYLOAD(nh, sizeof(*ifi));

            while(RTA_OK(rta, len)) {
                switch(rta->rta_type) {
                case IFLA_IFNAME:
                    char buffer[16 + IFNAMSIZ];
                    snprintf(buffer, sizeof(buffer), "INTERFACE=%s",
                             (char *) RTA_DATA(rta));
                    mParams[0] = strdup(buffer);
                    mAction = (ifi->ifi_flags & IFF_LOWER_UP) ?
                      NlActionLinkUp : NlActionLinkDown;
                    mSubsystem = strdup("net");

					/*mtk80842 for IPv6 tethering*/					
					if(mAction == NlActionLinkDown){	
						char prefix_prop_name[PROPERTY_KEY_MAX];	
						char plen_prop_name[PROPERTY_KEY_MAX];	
						char prop_value[PROPERTY_VALUE_MAX] = {'\0'};	
						snprintf(prefix_prop_name, sizeof(prefix_prop_name), 
							"net.ipv6.%s.prefix", (char *) RTA_DATA(rta));	
						
						if (property_get("net.ipv6.tether", prop_value, NULL)) {	
							if(0 == strcmp(prop_value, ((char *)RTA_DATA(rta)))){		
								if (property_get(prefix_prop_name, prop_value, NULL)) {	
							        property_set("net.ipv6.lastprefix", prop_value);
									SLOGD("set last prefix as %s\n", prop_value);
							    }
							} else	{
								SLOGW("%s is not a tether interface\n", (char *)RTA_DATA(rta));
							}	
						}

						if (property_get(prefix_prop_name, prop_value, NULL)) {	
							property_set(prefix_prop_name, "");	
						}	
						snprintf(plen_prop_name, sizeof(plen_prop_name), 
							"net.ipv6.%s.plen", (char *) RTA_DATA(rta));	
						if (property_get(plen_prop_name, prop_value, NULL)) {	
							property_set(plen_prop_name, "");	
							}					
						}
					
                    break;
                }

                rta = RTA_NEXT(rta, len);
            }

        } else if (nh->nlmsg_type == QLOG_NL_EVENT) {
            char *devname;
            ulog_packet_msg_t *pm;
            size_t len = nh->nlmsg_len - sizeof(*nh);
            if (sizeof(*pm) > len) {
                SLOGE("Got a short QLOG message\n");
                continue;
            }
            pm = (ulog_packet_msg_t *)NLMSG_DATA(nh);
            devname = pm->indev_name[0] ? pm->indev_name : pm->outdev_name;
            asprintf(&mParams[0], "ALERT_NAME=%s", pm->prefix);
            asprintf(&mParams[1], "INTERFACE=%s", devname);
            mSubsystem = strdup("qlog");
            mAction = NlActionChange;

        } else if(nh->nlmsg_type == RTM_NEWPREFIX){

				struct prefixmsg *prefix = (prefixmsg *)NLMSG_DATA(nh);
				int len = nh->nlmsg_len;
				struct rtattr * tb[RTA_MAX+1];
				char if_name[IFNAMSIZ] = "";
				
				if (nh->nlmsg_type != RTM_NEWPREFIX) {
					SLOGE("Not a prefix: %08x %08x %08x\n",
						nh->nlmsg_len, nh->nlmsg_type, nh->nlmsg_flags);
					continue;
				}
			
				len -= NLMSG_LENGTH(sizeof(*prefix));
				if (len < 0) {
					SLOGE("BUG: wrong nlmsg len %d\n", len);
					continue;
				}
					
				if (prefix->prefix_family != AF_INET6) {
					SLOGE("wrong family %d\n", prefix->prefix_family);
					continue;
				}
				if (prefix->prefix_type != 3 /*prefix opt*/) {
					SLOGE( "wrong ND type %d\n", prefix->prefix_type);
					continue;
				}
				if_indextoname(prefix->prefix_ifindex, if_name);
				
				{ 
					int max = RTA_MAX;
				    struct rtattr *rta = RTM_RTA(prefix);
					memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
					while (RTA_OK(rta, len)) {
						if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
							tb[rta->rta_type] = rta;
						rta = RTA_NEXT(rta,len);
					}
					if (len)
						SLOGE("!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
				}
				if (tb[PREFIX_ADDRESS] && (0 == strncmp(if_name, "ccmni", 2))) {
					struct in6_addr *pfx;
					char abuf[256];
					char prefix_prop_name[PROPERTY_KEY_MAX];
					char plen_prop_name[PROPERTY_KEY_MAX];
					char prefix_value[PROPERTY_VALUE_MAX] = {'\0'};
					char plen_value[4]; 
					
					pfx = (struct in6_addr *)RTA_DATA(tb[PREFIX_ADDRESS]);
			
					memset(abuf, '\0', sizeof(abuf));
					const char* addrStr = inet_ntop(AF_INET6, pfx, abuf, sizeof(abuf));

					snprintf(prefix_prop_name, sizeof(prefix_prop_name), 
						"net.ipv6.%s.prefix", if_name);
					property_get(prefix_prop_name, prefix_value, NULL);
					if(NULL != addrStr && strcmp(addrStr, prefix_value)){
						SLOGI("%s new prefix: %s, len=%d\n", if_name, addrStr, prefix->prefix_len);  

						property_set(prefix_prop_name, addrStr);
						snprintf(plen_prop_name, sizeof(plen_prop_name), 
								"net.ipv6.%s.plen", if_name);
						snprintf(plen_value, sizeof(plen_value), 
								"%d", prefix->prefix_len);						
						property_set(plen_prop_name, plen_value);
						{
			                char buffer[16 + IFNAMSIZ];
			                snprintf(buffer, sizeof(buffer), "INTERFACE=%s", if_name);
			                mParams[0] = strdup(buffer);							
						    mAction = NlActionIPv6Enable;
							mSubsystem = strdup("net");
						}
					} else {
						SLOGD("get an exist prefix: = %s\n", addrStr);
					} 					
				}else{
					SLOGD("ignore prefix of %s\n", if_name);
				}
	/*		
				if (prefix->prefix_flags & IF_PREFIX_ONLINK)
					;
				if (prefix->prefix_flags & IF_PREFIX_AUTOCONF)
					;
	*/	
        } else {
                SLOGD("Unexpected netlink message. type=0x%x\n", nh->nlmsg_type);
        }
        nh = NLMSG_NEXT(nh, size);
    }

    return true;
}

/* If the string between 'str' and 'end' begins with 'prefixlen' characters
 * from the 'prefix' array, then return 'str + prefixlen', otherwise return
 * NULL.
 */
static const char*
has_prefix(const char* str, const char* end, const char* prefix, size_t prefixlen)
{
    if ((end-str) >= (ptrdiff_t)prefixlen && !memcmp(str, prefix, prefixlen))
        return str + prefixlen;
    else
        return NULL;
}

/* Same as strlen(x) for constant string literals ONLY */
#define CONST_STRLEN(x)  (sizeof(x)-1)

/* Convenience macro to call has_prefix with a constant string literal  */
#define HAS_CONST_PREFIX(str,end,prefix)  has_prefix((str),(end),prefix,CONST_STRLEN(prefix))


/*
 * Parse an ASCII-formatted message from a NETLINK_KOBJECT_UEVENT
 * netlink socket.
 */
bool NetlinkEvent::parseAsciiNetlinkMessage(char *buffer, int size) {
    const char *s = buffer;
    const char *end;
    int param_idx = 0;
    int i;
    int first = 1;

    if (size == 0)
        return false;

    /* Ensure the buffer is zero-terminated, the code below depends on this */
    buffer[size-1] = '\0';

    end = s + size;
    while (s < end) {
        if (first) {
            const char *p;
            /* buffer is 0-terminated, no need to check p < end */
            for (p = s; *p != '@'; p++) {
                if (!*p) { /* no '@', should not happen */
                    return false;
                }
            }
            mPath = strdup(p+1);
            first = 0;
        } else {
            const char* a;
            if ((a = HAS_CONST_PREFIX(s, end, "ACTION=")) != NULL) {
                if (!strcmp(a, "add"))
                    mAction = NlActionAdd;
                else if (!strcmp(a, "remove"))
                    mAction = NlActionRemove;
                else if (!strcmp(a, "change"))
                    mAction = NlActionChange;
            } else if ((a = HAS_CONST_PREFIX(s, end, "SEQNUM=")) != NULL) {
                mSeq = atoi(a);
            } else if ((a = HAS_CONST_PREFIX(s, end, "SUBSYSTEM=")) != NULL) {
                mSubsystem = strdup(a);
            } else if (param_idx < NL_PARAMS_MAX) {
                mParams[param_idx++] = strdup(s);
            }
        }
        s += strlen(s) + 1;
    }
    return true;
}

bool NetlinkEvent::decode(char *buffer, int size, int format) {
    if (format == NetlinkListener::NETLINK_FORMAT_BINARY) {
        return parseBinaryNetlinkMessage(buffer, size);
    } else {
        return parseAsciiNetlinkMessage(buffer, size);
    }
}

const char *NetlinkEvent::findParam(const char *paramName) {
    size_t len = strlen(paramName);
    for (int i = 0; i < NL_PARAMS_MAX && mParams[i] != NULL; ++i) {
        const char *ptr = mParams[i] + len;
        if (!strncmp(mParams[i], paramName, len) && *ptr == '=')
            return ++ptr;
    }

    SLOGE("NetlinkEvent::FindParam(): Parameter '%s' not found", paramName);
    return NULL;
}
