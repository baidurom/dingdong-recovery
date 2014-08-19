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


# include MediaTek's extension of sdk-addon.make
include $(BUILD_SYSTEM_MTK_EXTENSION)/sdk-addon.mk

# If they didn't define PRODUCT_SDK_ADDON_NAME, then we won't define
# any of these rules.
addon_name := $(strip $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_NAME))
ifneq ($(addon_name),)

addon_dir_leaf := $(addon_name)-$(FILE_NAME_TAG)-$(INTERNAL_SDK_HOST_OS_NAME)

intermediates := $(HOST_OUT_INTERMEDIATES)/SDK_ADDON/$(addon_name)_intermediates
full_target := $(HOST_OUT_SDK_ADDON)/$(addon_dir_leaf).zip
staging := $(intermediates)/$(addon_dir_leaf)

ifeq ($(TARGET_PRODUCT),banyan_addon)
  mtk_sdk_api_addon_version := $(strip $(last_released_sdk_version)).$(strip $(last_released_mtk_sdk_version))
  mtk_sdk_toolset_version := $(strip $(MTK_SDK_TOOLSET_VERSION))
  ifeq ($(BUILD_MTK_SDK),all)
    full_target := $(HOST_OUT_SDK_ADDON)/mtk_sdk_api_addon-$(mtk_sdk_api_addon_version).zip \
                   $(HOST_OUT_SDK_ADDON)/mtk_sdk_toolset-$(mtk_sdk_toolset_version).zip
  endif

  ifeq ($(BUILD_MTK_SDK),api)
    full_target := $(HOST_OUT_SDK_ADDON)/mtk_sdk_api_addon-$(mtk_sdk_api_addon_version).zip
  endif

  ifeq ($(BUILD_MTK_SDK),toolset)
    full_target := $(HOST_OUT_SDK_ADDON)/mtk_sdk_toolset-$(mtk_sdk_toolset_version).zip
  endif
$(full_target): PRIVATE_MTK_SDK_API_ADDON_ZIP := $(HOST_OUT_SDK_ADDON)/mtk_sdk_api_addon-$(mtk_sdk_api_addon_version).zip
$(full_target): PRIVATE_MTK_SDK_TOOLSET_ZIP := $(HOST_OUT_SDK_ADDON)/mtk_sdk_toolset-$(mtk_sdk_toolset_version).zip
endif


sdk_addon_deps :=
files_to_copy :=

define stub-addon-jar-file
$(subst .jar,_stub-addon.jar,$(1))
endef

define stub-addon-jar
$(call stub-addon-jar-file,$(1)): $(1) | mkstubs
	$(info Stubbing addon jar using $(PRODUCT_SDK_ADDON_STUB_DEFS))
	$(hide) java -jar $(call module-installed-files,mkstubs) $(if $(hide),,--v) \
		"$$<" "$$@" @$(PRODUCT_SDK_ADDON_STUB_DEFS)
endef

# Files that are built and then copied into the sdk-addon
ifneq ($(strip $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_COPY_MODULES)),)
$(foreach cf,$(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_COPY_MODULES), \
  $(eval _src := $(call module-stubs-files,$(call word-colon,1,$(cf)))) \
  $(eval $(call stub-addon-jar,$(_src))) \
  $(eval _src := $(call stub-addon-jar-file,$(_src))) \
  $(if $(_src),,$(eval $(error Unknown or unlinkable module: $(call word-colon,1,$(cf)). Requested by $(INTERNAL_PRODUCT)))) \
  $(eval _dest := $(call word-colon,2,$(cf))) \
  $(eval files_to_copy += $(_src):$(_dest)) \
 )
endif

ifneq ($(strip $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_COPY_HOST_OUT)),)
$(foreach cf,$(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_COPY_HOST_OUT), \
  $(eval _src := $(call append-path,$(HOST_OUT),$(call word-colon,1,$(cf)))) \
  $(eval _dest := $(call word-colon,2,$(cf))) \
  $(eval files_to_copy += $(_src):$(_dest)) \
 )
endif
# Files that are copied directly into the sdk-addon
files_to_copy += $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_COPY_FILES)

# All SDK add-ons have these files
ifneq ($(strip $(BUILD_MTK_SDK)),toolset)
files_to_copy += \
        $(BUILT_SYSTEMIMAGE):images/$(TARGET_CPU_ABI)/system.img \
        $(BUILT_USERDATAIMAGE_TARGET):images/$(TARGET_CPU_ABI)/userdata.img \
        $(BUILT_RAMDISK_TARGET):images/$(TARGET_CPU_ABI)/ramdisk.img \
        $(PRODUCT_OUT)/system/build.prop:images/$(TARGET_CPU_ABI)/build.prop \
        $(target_notice_file_txt):images/$(TARGET_CPU_ABI)/NOTICE.txt
endif

#Build the windows emulator 
$(shell rm -f external/qemu/objs/emulator)
windows_intermediate_emulator :=  \
        $(HOST_OUT_EXECUTABLES)/emulator.exe  \
        $(HOST_OUT_EXECUTABLES)/emulator-arm.exe  \
        $(HOST_OUT_EXECUTABLES)/emulator-x86.exe
windows_emulator := external/qemu/objs/emulator-arm.exe  \
                    external/qemu/objs/emulator-x86.exe
emulator_setup := external/qemu/objs/emulator.exe
$(emulator_setup):
	$(hide) (cd external/qemu ; \
                 ./android-configure.sh --mingw --static; \
                 make clean ; make -j1)
$(windows_emulator):$(emulator_setup)

$(windows_intermediate_emulator):$(windows_emulator) $(emulator_setup)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp $(addprefix external/qemu/objs/,$(notdir $@)) $@

# Generate rules to copy the requested files
$(foreach cf,$(files_to_copy), \
  $(eval _src := $(call word-colon,1,$(cf))) \
  $(eval _dest := $(call append-path,$(staging),$(call word-colon,2,$(cf)))) \
  $(eval $(call copy-one-file,$(_src),$(_dest))) \
  $(eval sdk_addon_deps += $(_dest)) \
 )

# We don't know about all of the docs files, so depend on the timestamps for
# them, and record the directories, and the packaging rule will just copy the
# whole thing.
ifneq ($(BUILD_MTK_SDK),toolset)
# MTK SDK toolset package NOT require docs, so ONLY build docs for MTK SDK API package
doc_modules := $(strip $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SDK_ADDON_DOC_MODULES))
sdk_addon_deps += $(foreach dm, $(doc_modules), $(call doc-timestamp-for, $(dm)))
$(full_target): PRIVATE_DOCS_DIRS := $(addprefix $(OUT_DOCS)/, $(doc_modules))
endif

$(full_target): PRIVATE_STAGING_DIR := $(staging)

$(full_target): $(sdk_addon_deps) | $(ACP)
	@echo Packaging SDK Addon: $@
	@$(if $(filter $(PRIVATE_MTK_SDK_TOOLSET_ZIP),$@),, \
           mkdir -p $(PRIVATE_STAGING_DIR)/docs; \
           for d in $(PRIVATE_DOCS_DIRS); do \
             $(ACP) -r $$d $(PRIVATE_STAGING_DIR)/docs ; \
           done ; \
         )
	$(hide) mkdir -p $(dir $@)
ifneq ($(filter all api toolset,$(BUILD_MTK_SDK)),)
  ifneq ($(filter all api,$(BUILD_MTK_SDK)),)
	$(hide) ( F=$$(pwd)/$(PRIVATE_MTK_SDK_API_ADDON_ZIP) ; rm -f $$F ; cd $(PRIVATE_STAGING_DIR) && zip -rq $$F * -x tools/\* tools/ )
  endif
  ifneq ($(filter all toolset,$(BUILD_MTK_SDK)),)
	$(hide) ( F=$$(pwd)/$(PRIVATE_MTK_SDK_TOOLSET_ZIP) ; rm -f $$F ; cd $(PRIVATE_STAGING_DIR)/tools && zip -rq $$F * )
  endif
else
	$(hide) ( F=$$(pwd)/$@ ; rm -f $$F ; cd $(PRIVATE_STAGING_DIR)/.. && zip -rq $$F * )
endif

.PHONY: sdk_addon
sdk_addon: $(full_target)

ifneq ($(sdk_repo_goal),)
# If we're building the sdk_repo, keep the name of the addon zip
# around so that development/build/tools/sdk_repo.mk can dist it
# at the appropriate location.
ADDON_SDK_ZIP := $(full_target)
else
# When not building an sdk_repo, just dist the addon zip file
# as-is.
$(call dist-for-goals, sdk_addon, $(full_target))
endif

else # addon_name
ifneq ($(filter sdk_addon,$(MAKECMDGOALS)),)
$(error Trying to build sdk_addon, but product '$(INTERNAL_PRODUCT)' does not define one)
endif
endif # addon_name
