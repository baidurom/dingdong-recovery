# Copyright (C) 2008 The Android Open Source Project
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

ifeq ($(TARGET_DEVICE),Coolpad8297)
include $(call all-named-subdir-makefiles, libaudio)

include device/coolpad/Coolpad8297/AndroidBoard.mk

# linchunliang@baidu.com: add to symlink original init - begin
# Make a symlink from /sbin/ueventd to /init
SYMLINKS := $(TARGET_ROOT_OUT)/sbin/ueventd
$(SYMLINKS): INIT_BINARY := init
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> ../$(INIT_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf ../$(INIT_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.init.INSTALLED := \
    $(ALL_MODULES.init.INSTALLED) $(SYMLINKS)
# add end
#
LOCAL_PATH := $(call my-dir)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif # only include this for Coolpad8297 - F1
