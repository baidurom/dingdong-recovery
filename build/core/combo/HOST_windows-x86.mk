# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.


#
# Copyright (C) 2006 The Android Open Source Project
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
#

# Configuration for Linux on x86.
# Included by combo/select.make

# right now we get these from the environment, but we should
# pick them from the tree somewhere
TOOLS_PREFIX := #prebuilt/windows/host/bin/
TOOLS_EXE_SUFFIX := .exe

# Settings to use MinGW has a cross-compiler under Linux
ifneq ($(findstring Linux,$(UNAME)),)
ifneq ($(strip $(USE_MINGW)),)
HOST_ACP_UNAVAILABLE := true
TOOLS_EXE_SUFFIX :=
HOST_GLOBAL_CFLAGS += -DUSE_MINGW
ifneq ($(strip $(BUILD_HOST_64bit)),)
TOOLS_PREFIX := /usr/bin/amd64-mingw32msvc-
HOST_C_INCLUDES += /usr/lib/gcc/amd64-mingw32msvc/4.4.2/include
HOST_GLOBAL_LD_DIRS += -L/usr/amd64-mingw32msvc/lib
else
TOOLS_PREFIX := /usr/bin/i586-mingw32msvc-
HOST_C_INCLUDES += /usr/lib/gcc/i586-mingw32msvc/3.4.4/include
HOST_GLOBAL_LD_DIRS += -L/usr/i586-mingw32msvc/lib
endif # BUILD_HOST_64bit
endif # USE_MINGW
endif # Linux

HOST_CC := $(TOOLS_PREFIX)gcc$(TOOLS_EXE_SUFFIX)
HOST_CXX := $(TOOLS_PREFIX)g++$(TOOLS_EXE_SUFFIX)
HOST_AR := $(TOOLS_PREFIX)ar$(TOOLS_EXE_SUFFIX)

HOST_GLOBAL_CFLAGS += -include $(call select-android-config-h,windows)
HOST_GLOBAL_LDFLAGS += --enable-stdcall-fixup
ifneq ($(strip $(BUILD_HOST_static)),)
# Statically-linked binaries are desirable for sandboxed environment
HOST_GLOBAL_LDFLAGS += -static
endif # BUILD_HOST_static

# when building under Cygwin, ensure that we use Mingw compilation by default.
# you can disable this (i.e. to generate Cygwin executables) by defining the
# USE_CYGWIN variable in your environment, e.g.:
#
#   export USE_CYGWIN=1
#
# note that the -mno-cygwin flags are not needed when cross-compiling the
# Windows host tools on Linux
#
ifneq ($(findstring CYGWIN,$(UNAME)),)
ifeq ($(strip $(USE_CYGWIN)),)
HOST_GLOBAL_CFLAGS += -mno-cygwin
HOST_GLOBAL_LDFLAGS += -mno-cygwin -mconsole
endif
endif

HOST_SHLIB_SUFFIX := .dll
HOST_EXECUTABLE_SUFFIX := .exe

# $(1): The file to check
# TODO: find out what format cygwin's stat(1) uses
define get-file-size
999999999
endef
