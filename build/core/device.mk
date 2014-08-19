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
#

_device_var_list := \
    DEVICE_NAME \
    DEVICE_BOARD \
    DEVICE_REGION

define dump-device
$(info ==== $(1) ====)\
$(foreach v,$(_device_var_list),\
$(info DEVICES.$(1).$(v) := $(DEVICES.$(1).$(v))))\
$(info --------)
endef

define dump-devices
$(foreach p,$(DEVICES),$(call dump-device,$(p)))
endef

#
# $(1): device to inherit
#
define inherit-device
  $(foreach v,$(_device_var_list), \
      $(eval $(v) := $($(v)) $(INHERIT_TAG)$(strip $(1))))
endef

#
# $(1): device makefile list
#
#TODO: check to make sure that devices have all the necessary vars defined
define import-devices
$(call import-nodes,DEVICES,$(1),$(_device_var_list))
endef


#
# $(1): short device name like "sooner"
#
define _resolve-short-device-name
  $(eval dn := $(strip $(1)))
  $(eval d := \
      $(foreach d,$(DEVICES), \
          $(if $(filter $(dn),$(DEVICES.$(d).DEVICE_NAME)), \
            $(d) \
       )) \
   )
  $(eval d := $(sort $(d)))
  $(if $(filter 1,$(words $(d))), \
    $(d), \
    $(if $(filter 0,$(words $(d))), \
      $(error No matches for device "$(dn)"), \
      $(error Device "$(dn)" ambiguous: matches $(d)) \
    ) \
  )
endef

#
# $(1): short device name like "sooner"
#
define resolve-short-device-name
$(strip $(call _resolve-short-device-name,$(1)))
endef
