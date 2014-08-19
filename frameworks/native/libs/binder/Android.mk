# Copyright (C) 2009 The Android Open Source Project
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

# we have the common sources, plus some device-specific stuff
sources := \
    Binder.cpp \
    BpBinder.cpp \
    IInterface.cpp \
    IMemory.cpp \
    IPCThreadState.cpp \
    IPermissionController.cpp \
    IServiceManager.cpp \
    MemoryDealer.cpp \
    MemoryBase.cpp \
    MemoryHeapBase.cpp \
    Parcel.cpp \
    PermissionCache.cpp \
    ProcessState.cpp \
    Static.cpp

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
ifeq ($(TARGET_BUILD_VARIANT),eng)
ifeq ($(filter banyan_addon banyan_addon_x86,$(TARGET_PRODUCT)),)
ifeq ($(MTK_INTERNAL),yes)
# mtk80143: enable FP and ARM build for debug15 memory debugging
LOCAL_CFLAGS += \
		-fno-omit-frame-pointer \
		-mapcs \
		-D_MTK_ENG_
LOCAL_ARM_MODE := arm
endif
endif
endif
LOCAL_LDLIBS += -lpthread
LOCAL_MODULE := libbinder
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils
LOCAL_SRC_FILES := $(sources)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
ifeq ($(TARGET_BUILD_VARIANT),eng)
ifeq ($(filter banyan_addon banyan_addon_x86,$(TARGET_PRODUCT)),)
ifeq ($(MTK_INTERNAL),yes)
# mtk80143: enable FP and ARM build for debug15 memory debugging
LOCAL_CFLAGS += \
		-fno-omit-frame-pointer \
		-mapcs
LOCAL_ARM_MODE := arm
endif
endif
endif
LOCAL_LDLIBS += -lpthread
LOCAL_MODULE := libbinder
LOCAL_SRC_FILES := $(sources)
include $(BUILD_STATIC_LIBRARY)
