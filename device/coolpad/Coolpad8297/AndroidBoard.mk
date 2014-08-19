# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
TARGET_RECOVERY_SCREEN_TYPE := "XHDPI"

ADDITIONAL_DEFAULT_PROPERTIES += ro.mount.fs=EXT4

#
# Boot files
#
#ifeq ($(TARGET_PREBUILT_KERNEL),)
TARGET_PREBUILT_KERNEL := $(LOCAL_PATH)/official_jb/boot/kernel
#endif

file := $(INSTALLED_KERNEL_TARGET)
ALL_PREBUILT += $(file)
$(file): $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(transform-prebuilt-to-target)
file :=

include $(CLEAR_VARS)
LOCAL_MODULE       := init
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := official_jb/boot/root/$(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_ROOT_OUT)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fstab
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := official_jb/boot/root/$(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_ROOT_OUT)
include $(BUILD_PREBUILT)

ifneq ($(wildcard $(LOCAL_PATH)/static_library),)
PREBUILT_STATIC_LIBS := $(shell cd $(LOCAL_PATH) 2>&1 > /dev/null; ls static_library/*.a; cd - 2>&1 > /dev/null)
$(foreach x, $(PREBUILT_STATIC_LIBS), \
	$(warning x:$(x)) \
	$(eval include $(CLEAR_VARS)) \
	$(eval LOCAL_PREBUILT_LIBS:=$(x)) \
	$(eval include $(BUILD_MULTI_PREBUILT)))

$(foreach x, $(PREBUILT_STATIC_LIBS), \
	$(warning x:$(x)) \
	$(eval include $(CLEAR_VARS)) \
	$(eval LOCAL_PREBUILT_LIBS:=$(x)) \
	$(eval include $(BUILD_HOST_PREBUILT)))
endif

SECRO_IMG_BUILD_DISABLE := true

