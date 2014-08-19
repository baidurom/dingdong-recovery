LOCAL_PATH := $(call my-dir)
BB_PATH := $(LOCAL_PATH)

# Bionic Branches Switches (CM7/AOSP/ICS)
BIONIC_ICS := true


# Make a static library for regex.
include $(CLEAR_VARS)
LOCAL_SRC_FILES := android/regex/regex.c
LOCAL_C_INCLUDES := $(BB_PATH)/android/regex
LOCAL_CFLAGS := -Wno-sign-compare
LOCAL_MODULE := libclearsilverregex
include $(BUILD_STATIC_LIBRARY)

# Make a static library for RPC library (coming from uClibc).
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(shell cat $(BB_PATH)/android/librpc.sources)
LOCAL_C_INCLUDES := $(BB_PATH)/android/librpc
LOCAL_MODULE := libuclibcrpc
LOCAL_CFLAGS += -fno-strict-aliasing
include $(BUILD_STATIC_LIBRARY)


LOCAL_PATH := $(BB_PATH)
include $(CLEAR_VARS)

# Each profile require a compressed usage/config, outside the source tree for git history
# We keep the uncompressed headers in local include-<profile> to track config changes.
# TODO: generate includes in out/

# BB_INCLUDES_OUT := $(TARGET_OUT_INTERMEDIATES)/include
# $(BB_INCLUDES_OUT):
#	mkdir -p $(ANDROID_BUILD_TOP)/$(BB_INCLUDES_OUT)

# Execute make clean, make prepare and copy profiles required for normal & static lib (recovery)

KERNEL_MODULES_DIR ?= /system/lib/modules
BUSYBOX_CONFIG := minimal full
$(BUSYBOX_CONFIG):
	@echo -e ${CL_PFX}"prepare config for busybox $@ profile"${CL_RST}
	@cd $(BB_PATH) && make clean
	@cd $(BB_PATH) && git clean -f -- ./include-$@/
	cp $(BB_PATH)/.config-$@ $(BB_PATH)/.config
	cd $(BB_PATH) && make prepare
	@#cp $(BB_PATH)/.config $(BB_PATH)/.config-$@
	@mkdir -p $(BB_PATH)/include-$@
	cp $(BB_PATH)/include/*.h $(BB_PATH)/include-$@/
	@rm $(BB_PATH)/include/usage_compressed.h
	@rm $(BB_PATH)/include/autoconf.h
	@rm -f $(BB_PATH)/.config-old

busybox_prepare: $(BUSYBOX_CONFIG)
LOCAL_MODULE := busybox_prepare
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_STATIC_LIBRARY)


LOCAL_PATH := $(BB_PATH)
include $(CLEAR_VARS)

KERNEL_MODULES_DIR ?= /system/lib/modules

SUBMAKE := make -s -C $(BB_PATH) CC=$(CC)

BUSYBOX_SRC_FILES = $(shell cat $(BB_PATH)/busybox-$(BUSYBOX_CONFIG).sources) \
	libbb/android.c

ifeq ($(TARGET_ARCH),arm)
	BUSYBOX_SRC_FILES += \
	android/libc/arch-arm/syscalls/adjtimex.S \
	android/libc/arch-arm/syscalls/getsid.S \
	android/libc/arch-arm/syscalls/stime.S \
	android/libc/arch-arm/syscalls/swapon.S \
	android/libc/arch-arm/syscalls/swapoff.S \
	android/libc/arch-arm/syscalls/sysinfo.S
endif

ifeq ($(TARGET_ARCH),mips)
	BUSYBOX_SRC_FILES += \
	android/libc/arch-mips/syscalls/adjtimex.S \
	android/libc/arch-mips/syscalls/getsid.S \
	android/libc/arch-mips/syscalls/stime.S \
	android/libc/arch-mips/syscalls/swapon.S \
	android/libc/arch-mips/syscalls/swapoff.S \
	android/libc/arch-mips/syscalls/sysinfo.S
endif

BUSYBOX_C_INCLUDES = \
	$(BB_PATH)/include-$(BUSYBOX_CONFIG) \
	$(BB_PATH)/include $(BB_PATH)/libbb \
	bionic/libc/private \
	bionic/libm/include \
	bionic/libm \
	libc/kernel/common \
	$(BB_PATH)/android/regex \
	$(BB_PATH)/android/librpc

BUSYBOX_CFLAGS = \
	-Werror=implicit \
	-DNDEBUG \
	-DANDROID \
	-fno-strict-aliasing \
	-include include-$(BUSYBOX_CONFIG)/autoconf.h \
	-D'CONFIG_DEFAULT_MODULES_DIR="$(KERNEL_MODULES_DIR)"' \
	-D'BB_VER="$(strip $(shell $(SUBMAKE) kernelversion)) $(BUSYBOX_SUFFIX)"' -DBB_BT=AUTOCONF_TIMESTAMP

# to handle differences in ICS (ipv6)
ifeq ($(BIONIC_ICS),true)
BUSYBOX_CFLAGS += -DBIONIC_ICS
endif


# Build the static lib for the recovery tool

BUSYBOX_CONFIG:=minimal
BUSYBOX_SUFFIX:=static
LOCAL_SRC_FILES := $(BUSYBOX_SRC_FILES)
LOCAL_C_INCLUDES := $(BUSYBOX_C_INCLUDES)
LOCAL_CFLAGS := -Dmain=busybox_driver $(BUSYBOX_CFLAGS)
LOCAL_CFLAGS += \
  -Dgetusershell=busybox_getusershell \
  -Dsetusershell=busybox_setusershell \
  -Dendusershell=busybox_endusershell \
  -Dttyname_r=busybox_ttyname_r \
  -Dgetmntent=busybox_getmntent \
  -Dgetmntent_r=busybox_getmntent_r \
  -Dgenerate_uuid=busybox_generate_uuid
LOCAL_MODULE := libbusybox
LOCAL_MODULE_TAGS := eng debug
LOCAL_STATIC_LIBRARIES := libcutils libc libm
$(LOCAL_MODULE): busybox_prepare
include $(BUILD_STATIC_LIBRARY)


# Bionic Busybox /system/xbin

LOCAL_PATH := $(BB_PATH)
include $(CLEAR_VARS)

BUSYBOX_CONFIG:=full
BUSYBOX_SUFFIX:=bionic
LOCAL_SRC_FILES := $(BUSYBOX_SRC_FILES)
ifeq ($(BIONIC_ICS),true)
LOCAL_SRC_FILES += android/libc/__set_errno.c
endif
LOCAL_C_INCLUDES := $(BUSYBOX_C_INCLUDES)
LOCAL_CFLAGS := $(BUSYBOX_CFLAGS)
LOCAL_LDFLAGS += -Wl,--no-fatal-warnings
LOCAL_MODULE := busybox
LOCAL_MODULE_TAGS := eng debug
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_SHARED_LIBRARIES := libc libcutils libm
LOCAL_STATIC_LIBRARIES := libclearsilverregex libuclibcrpc
$(LOCAL_MODULE): busybox_prepare
include $(BUILD_EXECUTABLE)

BUSYBOX_LINKS := $(shell cat $(BB_PATH)/busybox-$(BUSYBOX_CONFIG).links)
# nc is provided by external/netcat
exclude := nc
SYMLINKS := $(addprefix $(TARGET_OUT_OPTIONAL_EXECUTABLES)/,$(filter-out $(exclude),$(notdir $(BUSYBOX_LINKS))))
$(SYMLINKS): BUSYBOX_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Symlink: $@ -> $(BUSYBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(BUSYBOX_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)


# Static Busybox

LOCAL_PATH := $(BB_PATH)
include $(CLEAR_VARS)

BUSYBOX_CONFIG:=full
BUSYBOX_SUFFIX:=static
LOCAL_SRC_FILES := $(BUSYBOX_SRC_FILES)
LOCAL_C_INCLUDES := $(BUSYBOX_C_INCLUDES)
LOCAL_CFLAGS := $(BUSYBOX_CFLAGS)
LOCAL_CFLAGS += \
  -Dgetusershell=busybox_getusershell \
  -Dsetusershell=busybox_setusershell \
  -Dendusershell=busybox_endusershell \
  -Dttyname_r=busybox_ttyname_r \
  -Dgetmntent=busybox_getmntent \
  -Dgetmntent_r=busybox_getmntent_r \
  -Dgenerate_uuid=busybox_generate_uuid
LOCAL_LDFLAGS += -Wl,--no-fatal-warnings
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := static_busybox
LOCAL_MODULE_STEM := busybox
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libclearsilverregex libc libcutils libm libuclibcrpc
LOCAL_MODULE_CLASS := UTILITY_EXECUTABLES
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/utilities
LOCAL_UNSTRIPPED_PATH := $(PRODUCT_OUT)/symbols/utilities
$(LOCAL_MODULE): busybox_prepare
include $(BUILD_EXECUTABLE)
