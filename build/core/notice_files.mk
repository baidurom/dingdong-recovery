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


###########################################################
## Track NOTICE files
###########################################################

notice_file:=$(strip $(wildcard $(LOCAL_PATH)/NOTICE))

ifeq ($(LOCAL_MODULE_CLASS),NONE)
  # We ignore NOTICE files for modules of type NONE.
  notice_file :=
endif

ifdef notice_file

# This relies on the name of the directory in PRODUCT_OUT matching where
# it's installed on the target - i.e. system, data, etc.  This does
# not work for root and isn't exact, but it's probably good enough for
# compliance.
# Includes the leading slash
ifdef LOCAL_INSTALLED_MODULE
  module_installed_filename := $(patsubst $(PRODUCT_OUT)%,%,$(LOCAL_INSTALLED_MODULE))
else
  # This module isn't installable
  ifeq ($(LOCAL_MODULE_CLASS),STATIC_LIBRARIES)
    # Stick the static libraries with the dynamic libraries.
    # We can't use xxx_OUT_STATIC_LIBRARIES because it points into
    # device-obj or host-obj.
    module_installed_filename := \
        $(patsubst $(PRODUCT_OUT)%,%,$($(my_prefix)OUT_SHARED_LIBRARIES))/$(notdir $(LOCAL_BUILT_MODULE))
  else
    ifeq ($(LOCAL_MODULE_CLASS),JAVA_LIBRARIES)
      # Stick the static java libraries with the regular java libraries.
      module_leaf := $(notdir $(LOCAL_BUILT_MODULE))
      # javalib.jar is the default name for the build module (and isn't meaningful)
      # If that's what we have, substitute the module name instead.  These files
      # aren't included on the device, so this name is synthetic anyway.
      ifeq ($(module_leaf),javalib.jar)
        module_leaf := $(LOCAL_MODULE).jar
      endif
      module_installed_filename := \
          $(patsubst $(PRODUCT_OUT)%,%,$($(my_prefix)OUT_JAVA_LIBRARIES))/$(module_leaf)
    else
      $(error Cannot determine where to install NOTICE file for $(LOCAL_MODULE))
    endif # JAVA_LIBRARIES
  endif # STATIC_LIBRARIES
endif

# In case it's actually a host file
module_installed_filename := $(patsubst $(HOST_OUT)%,%,$(module_installed_filename))

installed_notice_file := $($(my_prefix)OUT_NOTICE_FILES)/src/$(module_installed_filename).txt

$(installed_notice_file): PRIVATE_INSTALLED_MODULE := $(module_installed_filename)

$(installed_notice_file): $(notice_file)
	@echo Notice file: $< -- $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cat $< >> $@

ifdef LOCAL_INSTALLED_MODULE
# Make LOCAL_INSTALLED_MODULE depend on NOTICE files if they exist
# libraries so they get installed along with it.  Make it an order-only
# dependency so we don't re-install a module when the NOTICE changes.
$(LOCAL_INSTALLED_MODULE): | $(installed_notice_file)
endif

else
# NOTICE file does not exist
installed_notice_file :=
endif

# Create a predictable, phony target to build this notice file.
# Define it even if the notice file doesn't exist so that other
# modules can depend on it.
notice_target := NOTICE-$(if \
    $(LOCAL_IS_HOST_MODULE),HOST,TARGET)-$(LOCAL_MODULE_CLASS)-$(LOCAL_MODULE)
.PHONY: $(notice_target)
$(notice_target): $(installed_notice_file)
