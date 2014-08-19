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

# Select a combo based on the compiler being used.
#
# Inputs:
#	combo_target -- prefix for final variables (HOST_ or TARGET_)
#

# Build a target string like "linux-arm" or "darwin-x86".
combo_os_arch := $($(combo_target)OS)-$($(combo_target)ARCH)

# Set reasonable defaults for the various variables

$(combo_target)CC := $(CC)
$(combo_target)CXX := $(CXX)
$(combo_target)AR := $(AR)
$(combo_target)STRIP := $(STRIP)

$(combo_target)BINDER_MINI := 0

$(combo_target)HAVE_EXCEPTIONS := 0
$(combo_target)HAVE_UNIX_FILE_PATH := 1
$(combo_target)HAVE_WINDOWS_FILE_PATH := 0
$(combo_target)HAVE_RTTI := 1
$(combo_target)HAVE_CALL_STACKS := 1
$(combo_target)HAVE_64BIT_IO := 1
$(combo_target)HAVE_CLOCK_TIMERS := 1
$(combo_target)HAVE_PTHREAD_RWLOCK := 1
$(combo_target)HAVE_STRNLEN := 1
$(combo_target)HAVE_STRERROR_R_STRRET := 1
$(combo_target)HAVE_STRLCPY := 0
$(combo_target)HAVE_STRLCAT := 0
$(combo_target)HAVE_KERNEL_MODULES := 0

$(combo_target)GLOBAL_CFLAGS := -fno-exceptions -Wno-multichar
$(combo_target)RELEASE_CFLAGS := -O2 -g -fno-strict-aliasing
$(combo_target)GLOBAL_LDFLAGS :=
$(combo_target)GLOBAL_ARFLAGS := crsP

$(combo_target)EXECUTABLE_SUFFIX :=
$(combo_target)SHLIB_SUFFIX := .so
$(combo_target)JNILIB_SUFFIX := $($(combo_target)SHLIB_SUFFIX)
$(combo_target)STATIC_LIB_SUFFIX := .a

# Now include the combo for this specific target.
include $(BUILD_COMBOS)/$(combo_target)$(combo_os_arch).mk

ifneq ($(USE_CCACHE),)
  # The default check uses size and modification time, causing false misses
  # since the mtime depends when the repo was checked out
  export CCACHE_COMPILERCHECK := content

  # See man page, optimizations to get more cache hits
  # implies that __DATE__ and __TIME__ are not critical for functionality.
  # Ignore include file modification time since it will depend on when
  # the repo was checked out
  export CCACHE_SLOPPINESS := time_macros,include_file_mtime,file_macro

  # Turn all preprocessor absolute paths into relative paths.
  # Fixes absolute paths in preprocessed source due to use of -g.
  # We don't really use system headers much so the rootdir is
  # fine; ensures these paths are relative for all Android trees
  # on a workstation.
  export CCACHE_BASEDIR := /

  CCACHE_HOST_TAG := $(HOST_PREBUILT_TAG)
  # If we are cross-compiling Windows binaries on Linux
  # then use the linux ccache binary instead.
  ifeq ($(HOST_OS)-$(BUILD_OS),windows-linux)
    CCACHE_HOST_TAG := linux-$(BUILD_ARCH)
  endif
  ccache := prebuilts/misc/$(CCACHE_HOST_TAG)/ccache/ccache
  ifneq ($(ANDROID_CCACHE),)
     ccache := $(ANDROID_CCACHE)
  endif
  # Check that the executable is here.
  ccache := $(strip $(wildcard $(ccache)))
  ifdef ccache
    # prepend ccache if necessary
    ifneq ($(ccache),$(firstword $($(combo_target)CC)))
      $(combo_target)CC := $(ccache) $($(combo_target)CC)
    endif
    ifneq ($(ccache),$(firstword $($(combo_target)CXX)))
      $(combo_target)CXX := $(ccache) $($(combo_target)CXX)
    endif
    ccache =
  endif
endif
