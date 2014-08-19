# Copyright 2005 The Android Open Source Project
ifneq ($(TARGET_PROVIDES_INIT),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	builtins.c \
	init.c \
	devices.c \
	property_service.c \
	property_patch.c \
	util.c \
	parser.c \
	logo.c \
	keychords.c \
	signal_handler.c \
	init_parser.c \
	ueventd.c \
	ueventd_parser.c \
	watchdogd.c

ifeq ($(strip $(INIT_BOOTCHART)),true)
LOCAL_SRC_FILES += bootchart.c
LOCAL_CFLAGS    += -DBOOTCHART=1
endif

ifeq ($(strip $(MTK_NAND_UBIFS_SUPPORT)),yes)
LOCAL_CFLAGS += -DMTK_NAND_UBIFS_SUPPORT
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DALLOW_LOCAL_PROP_OVERRIDE=1 -DINIT_ENG_BUILD
endif

LOCAL_MODULE:= init

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

LOCAL_STATIC_LIBRARIES := libfs_mgr libcutils libc

ifeq ($(HAVE_SELINUX),true)
LOCAL_STATIC_LIBRARIES += libselinux
LOCAL_C_INCLUDES += external/libselinux/include
LOCAL_CFLAGS += -DHAVE_SELINUX
endif

ifeq ($(PARTIAL_BUILD),true)
LOCAL_CFLAGS += -DPARTIAL_BUILD
endif
ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
LOCAL_STATIC_LIBRARIES += libaed_static
endif

ifeq ($(NO_INIT_PERMISSION_CHECK),yes)
LOCAL_CFLAGS    += -DNO_INIT_PERMISSION_CHECK
endif

ifeq ($(TARGET_NO_FACTORYIMAGE),true)
LOCAL_CFLAGS += -DUSE_BUILT_IN_FACTORY
endif

#INF@MTK, add for KPOC mode {
ifeq ($(MTK_KERNEL_POWER_OFF_CHARGING),yes)
LOCAL_CFLAGS += -DMTK_KERNEL_POWER_OFF_CHARGING_SUPPORT
endif

include $(BUILD_EXECUTABLE)

# Make a symlink from /sbin/ueventd and /sbin/watchdogd to /init
SYMLINKS := \
	$(TARGET_ROOT_OUT)/sbin/ueventd \
	$(TARGET_ROOT_OUT)/sbin/watchdogd

$(SYMLINKS): INIT_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> ../$(INIT_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf ../$(INIT_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)

endif
