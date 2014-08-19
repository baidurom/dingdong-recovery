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
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


#
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
#

###########################################################
## Standard rules for building an application package.
##
## Additional inputs from base_rules.make:
## LOCAL_PACKAGE_NAME: The name of the package; the directory
## will be called this.
##
## MODULE, MODULE_PATH, and MODULE_SUFFIX will
## be set for you.
###########################################################


# If this makefile is being read from within an inheritance,
# use the new values.
skip_definition:=
ifdef LOCAL_PACKAGE_OVERRIDES
  package_overridden := $(call set-inherited-package-variables)
  ifeq ($(strip $(package_overridden)),)
    skip_definition := true
  endif
endif

ifndef skip_definition

LOCAL_PACKAGE_NAME := $(strip $(LOCAL_PACKAGE_NAME))
ifeq ($(LOCAL_PACKAGE_NAME),)
$(error $(LOCAL_PATH): Package modules must define LOCAL_PACKAGE_NAME)
endif

ifneq ($(strip $(LOCAL_MODULE_SUFFIX)),)
$(error $(LOCAL_PATH): Package modules may not define LOCAL_MODULE_SUFFIX)
endif
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)

ifneq ($(strip $(LOCAL_MODULE)),)
$(error $(LOCAL_PATH): Package modules may not define LOCAL_MODULE)
endif
LOCAL_MODULE := $(LOCAL_PACKAGE_NAME)

ifeq ($(strip $(LOCAL_MANIFEST_FILE)),)
LOCAL_MANIFEST_FILE := AndroidManifest.xml
 ifneq ($(RESOURCE_OVERLAY_SUPPORT),)
   MANIFEST_OVERLAY_SUPPORT := $(RESOURCE_OVERLAY_SUPPORT)
 endif
else
# if manifest file specified in Android.mk,
# disable manifest overlay mechanism
MANIFEST_OVERLAY_SUPPORT :=
endif

# If you need to put the MANIFEST_FILE outside of LOCAL_PATH
# you can use FULL_MANIFEST_FILE
ifeq ($(strip $(LOCAL_FULL_MANIFEST_FILE)),)
LOCAL_FULL_MANIFEST_FILE := $(LOCAL_PATH)/$(LOCAL_MANIFEST_FILE)
endif

ifneq ($(strip $(LOCAL_MODULE_CLASS)),)
$(error $(LOCAL_PATH): Package modules may not set LOCAL_MODULE_CLASS)
endif
LOCAL_MODULE_CLASS := APPS

# Package LOCAL_MODULE_TAGS default to optional
LOCAL_MODULE_TAGS := $(strip $(LOCAL_MODULE_TAGS))
ifeq ($(LOCAL_MODULE_TAGS),)
LOCAL_MODULE_TAGS := optional
endif

ifeq ($(filter tests, $(LOCAL_MODULE_TAGS)),)
# Force localization check if it's not tagged as tests.
LOCAL_AAPT_FLAGS := $(LOCAL_AAPT_FLAGS) -z
endif

ifeq (,$(LOCAL_ASSET_DIR))
LOCAL_ASSET_DIR := $(LOCAL_PATH)/assets
#
# MediaTek assets overlay
#
ifneq ($(RESOURCE_OVERLAY_SUPPORT),)
  overlay_assets_dir := $(shell perl $(MTK_ROOT_BUILD)/tools/assets_overlay.pl \
                                     $(LOCAL_PATH) \
                                     $(LOCAL_ASSET_DIR) \
                                     $(TARGET_PRODUCT) \
                                     '$(PRODUCT_PACKAGE_OVERLAYS)'; \
                                if [ $$? != 0 ]; then echo ASSETS_OVERLAY_ERROR; fi \
                         )
  ifneq ($(filter ASSETS_OVERLAY_ERROR, $(overlay_assets_dir)),)
    $(error errors in assets overlay process)
  else
    LOCAL_ASSET_DIR := $(strip $(overlay_assets_dir))
  endif
endif
endif

ifeq (,$(LOCAL_RESOURCE_DIR))
  LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
endif
LOCAL_RESOURCE_DIR := \
  $(wildcard $(foreach dir, $(DEVICE_PACKAGE_OVERLAYS), \
    $(addprefix $(dir)/, $(LOCAL_RESOURCE_DIR)))) \
  $(wildcard $(foreach dir, $(PRODUCT_PACKAGE_OVERLAYS), \
    $(addprefix $(dir)/, $(LOCAL_RESOURCE_DIR)))) \
  $(LOCAL_RESOURCE_DIR)

all_assets := $(call find-subdir-assets,$(LOCAL_ASSET_DIR))
all_assets := $(addprefix $(LOCAL_ASSET_DIR)/,$(patsubst assets/%,%,$(all_assets)))

all_resources := $(strip \
    $(foreach dir, $(LOCAL_RESOURCE_DIR), \
      $(addprefix $(dir)/, \
        $(patsubst res/%,%, \
          $(call find-subdir-assets,$(dir)) \
         ) \
       ) \
     ) \
    $(strip $(LOCAL_GENERATED_RESOURCES)))

all_res_assets := $(strip $(all_assets) $(all_resources))

package_expected_intermediates_COMMON := $(call local-intermediates-dir,COMMON)
# If no assets or resources were found, clear the directory variables so
# we don't try to build them.
ifeq (,$(all_assets))
LOCAL_ASSET_DIR:=
endif
ifeq (,$(all_resources))
LOCAL_RESOURCE_DIR:=
R_file_stamp :=
else
# Make sure that R_file_stamp inherits the proper PRIVATE vars.
# If R.stamp moves, be sure to update the framework makefile,
# which has intimate knowledge of its location.
R_file_stamp := $(package_expected_intermediates_COMMON)/src/R.stamp
LOCAL_INTERMEDIATE_TARGETS += $(R_file_stamp)
endif

LOCAL_BUILT_MODULE_STEM := package.apk

LOCAL_PROGUARD_ENABLED:=$(strip $(LOCAL_PROGUARD_ENABLED))
ifndef LOCAL_PROGUARD_ENABLED
ifneq ($(filter user userdebug, $(TARGET_BUILD_VARIANT)),)
    # turn on Proguard by default for user & userdebug build
    LOCAL_PROGUARD_ENABLED :=
endif
endif
ifeq ($(LOCAL_PROGUARD_ENABLED),disabled)
    # the package explicitly request to disable proguard.
    LOCAL_PROGUARD_ENABLED :=
endif
proguard_options_file :=
ifneq ($(LOCAL_PROGUARD_ENABLED),custom)
ifneq ($(all_resources),)
    proguard_options_file := $(package_expected_intermediates_COMMON)/proguard_options
endif # all_resources
endif # !custom
LOCAL_PROGUARD_FLAGS := $(addprefix -include ,$(proguard_options_file)) $(LOCAL_PROGUARD_FLAGS)

ifneq (true,$(WITH_DEXPREOPT))
LOCAL_DEX_PREOPT :=
else
ifeq (,$(TARGET_BUILD_APPS))
ifneq (,$(LOCAL_SRC_FILES))
ifndef LOCAL_DEX_PREOPT
LOCAL_DEX_PREOPT := true
endif
endif
endif
endif
ifeq (false,$(LOCAL_DEX_PREOPT))
LOCAL_DEX_PREOPT :=
endif

ifeq (true,$(EMMA_INSTRUMENT))
ifndef LOCAL_EMMA_INSTRUMENT
# No emma for test apks.
ifeq (,$(filer tests,$(LOCAL_MODULE_TAGS))$(LOCAL_INSTRUMENTATION_FOR))
LOCAL_EMMA_INSTRUMENT := true
endif # No test apk
endif # LOCAL_EMMA_INSTRUMENT is not set
else
LOCAL_EMMA_INSTRUMENT := false
endif # EMMA_INSTRUMENT is true

ifeq (true,$(LOCAL_EMMA_INSTRUMENT))
ifeq (true,$(EMMA_INSTRUMENT_STATIC))
LOCAL_STATIC_JAVA_LIBRARIES += emma
else
ifdef LOCAL_SDK_VERSION
ifdef TARGET_BUILD_APPS
# In unbundled build merge the emma library into the apk.
LOCAL_STATIC_JAVA_LIBRARIES += emma
else
# If build against the SDK in full build, core.jar is not used,
# we have to use prebiult emma.jar to make Proguard happy;
# Otherwise emma classes are included in core.jar.
LOCAL_PROGUARD_FLAGS += -libraryjars $(EMMA_JAR)
endif # full build
endif # LOCAL_SDK_VERSION
endif # EMMA_INSTRUMENT_STATIC
endif # LOCAL_EMMA_INSTRUMENT

#################################
include $(BUILD_SYSTEM)/java.mk
#################################

LOCAL_SDK_RES_VERSION:=$(strip $(LOCAL_SDK_RES_VERSION))
ifeq ($(LOCAL_SDK_RES_VERSION),)
  LOCAL_SDK_RES_VERSION:=$(LOCAL_SDK_VERSION)
endif

full_android_manifest := $(LOCAL_FULL_MANIFEST_FILE)
#
# MediaTek manifest file overlay
#
ifneq ($(MANIFEST_OVERLAY_SUPPORT),)
  overlay_manifest_file := \
    $(foreach mf, $(PRODUCT_PACKAGE_OVERLAYS), \
      $(if $(wildcard $(mf)/$(LOCAL_PATH)/$(LOCAL_MANIFEST_FILE)), \
        $(mf)/$(LOCAL_PATH)/$(LOCAL_MANIFEST_FILE) \
        , \
       ) \
     )

  ifneq ($(strip $(overlay_manifest_file)),)
    full_android_manifest := $(word 1,$(overlay_manifest_file))
  endif
endif

$(LOCAL_INTERMEDIATE_TARGETS): \
    PRIVATE_ANDROID_MANIFEST := $(full_android_manifest)
ifneq (,$(filter-out current, $(LOCAL_SDK_VERSION)))
$(LOCAL_INTERMEDIATE_TARGETS): \
    PRIVATE_DEFAULT_APP_TARGET_SDK := $(LOCAL_SDK_VERSION)
else
$(LOCAL_INTERMEDIATE_TARGETS): \
    PRIVATE_DEFAULT_APP_TARGET_SDK := $(DEFAULT_APP_TARGET_SDK)
endif

ifneq ($(all_resources),)

# Since we don't know where the real R.java file is going to end up,
# we need to use another file to stand in its place.  We'll just
# copy the generated file to src/R.stamp, which means it will
# have the same contents and timestamp as the actual file.
#
# At the same time, this will copy the R.java file to a central
# 'R' directory to make it easier to add the files to an IDE.
#
#TODO: use PRIVATE_SOURCE_INTERMEDIATES_DIR instead of
#      $(intermediates.COMMON)/src
ifneq ($(package_expected_intermediates_COMMON),$(intermediates.COMMON))
  $(error $(LOCAL_MODULE): internal error: expected intermediates.COMMON "$(package_expected_intermediates_COMMON)" != intermediates.COMMON "$(intermediates.COMMON)")
endif

$(R_file_stamp): PRIVATE_RESOURCE_PUBLICS_OUTPUT := \
			$(intermediates.COMMON)/public_resources.xml
$(R_file_stamp): PRIVATE_PROGUARD_OPTIONS_FILE := $(proguard_options_file)
$(R_file_stamp): $(all_res_assets) $(full_android_manifest) $(RenderScript_file_stamp) $(AAPT) | $(ACP)
	@echo "target R.java/Manifest.java: $(PRIVATE_MODULE) ($@)"
	@rm -f $@
	$(create-resource-java-files)
	$(hide) for GENERATED_MANIFEST_FILE in `find $(PRIVATE_SOURCE_INTERMEDIATES_DIR) \
					-name Manifest.java 2> /dev/null`; do \
		dir=`awk '/package/{gsub(/\./,"/",$$2);gsub(/;/,"",$$2);print $$2;exit}' $$GENERATED_MANIFEST_FILE`; \
		mkdir -p $(TARGET_COMMON_OUT_ROOT)/R/$$dir; \
		$(ACP) -fp $$GENERATED_MANIFEST_FILE $(TARGET_COMMON_OUT_ROOT)/R/$$dir; \
	done;
	$(hide) for GENERATED_R_FILE in `find $(PRIVATE_SOURCE_INTERMEDIATES_DIR) \
					-name R.java 2> /dev/null`; do \
		dir=`awk '/package/{gsub(/\./,"/",$$2);gsub(/;/,"",$$2);print $$2;exit}' $$GENERATED_R_FILE`; \
		mkdir -p $(TARGET_COMMON_OUT_ROOT)/R/$$dir; \
		$(ACP) -fp $$GENERATED_R_FILE $(TARGET_COMMON_OUT_ROOT)/R/$$dir \
			|| exit 31; \
		$(ACP) -fp $$GENERATED_R_FILE $@ || exit 32; \
	done; \

$(proguard_options_file): $(R_file_stamp)

ifdef LOCAL_EXPORT_PACKAGE_RESOURCES
# Put this module's resources into a PRODUCT-agnositc package that
# other packages can use to build their own PRODUCT-agnostic R.java (etc.)
# files.
resource_export_package := $(intermediates.COMMON)/package-export.apk
$(R_file_stamp): $(resource_export_package)
$(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/package-export.apk:$(call intermediates-dir-for,APPS,framework-res,,COMMON)/src/R.stamp 
$(call intermediates-dir-for,APPS,framework-res-yi2,,COMMON)/package-export.apk:$(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/src/R.stamp $(call intermediates-dir-for,APPS,framework-res,,COMMON)/src/R.stamp
$(call intermediates-dir-for,APPS,framework-res-yi3,,COMMON)/package-export.apk:$(call intermediates-dir-for,APPS,framework-res-yi2,,COMMON)/src/R.stamp $(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/src/R.stamp $(call intermediates-dir-for,APPS,framework-res,,COMMON)/src/R.stamp
$(call intermediates-dir-for,APPS,framework-res-yi,,COMMON)/package-export.apk:$(call intermediates-dir-for,APPS,framework-res-yi3,,COMMON)/src/R.stamp $(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/src/R.stamp $(call intermediates-dir-for,APPS,framework-res,,COMMON)/src/R.stamp 

# add-assets-to-package looks at PRODUCT_AAPT_CONFIG, but this target
# can't know anything about PRODUCT.  Clear it out just for this target.
$(resource_export_package): PRIVATE_PRODUCT_AAPT_CONFIG :=
$(resource_export_package): PRIVATE_PRODUCT_AAPT_PREF_CONFIG :=
$(resource_export_package): $(all_res_assets) $(full_android_manifest) $(RenderScript_file_stamp) $(AAPT)
	@echo "target Export Resources: $(PRIVATE_MODULE) ($@)"
	$(create-empty-package)
	$(add-assets-to-package)
endif

# Other modules should depend on the BUILT module if
# they want to use this module's R.java file.
$(LOCAL_BUILT_MODULE): $(R_file_stamp)

ifneq ($(full_classes_jar),)
# If full_classes_jar is non-empty, we're building sources.
# If we're building sources, the initial javac step (which
# produces full_classes_compiled_jar) needs to ensure the
# R.java and Manifest.java files have been generated first.
$(full_classes_compiled_jar): $(R_file_stamp)
endif

endif  # all_resources

ifeq ($(LOCAL_NO_STANDARD_LIBRARIES),true)
# We need to explicitly clear this var so that we don't
# inherit the value from whomever caused us to be built.
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_AAPT_INCLUDES :=
else
# Most packages should link against the resources defined by framework-res.
# Even if they don't have their own resources, they may use framework
# resources.
ifneq ($(filter-out current,$(LOCAL_SDK_RES_VERSION))$(if $(TARGET_BUILD_APPS),$(filter current,$(LOCAL_SDK_RES_VERSION))),)
# for released sdk versions, the platform resources were built into android.jar.
framework_res_package_export := \
    $(HISTORICAL_SDK_VERSIONS_ROOT)/$(LOCAL_SDK_RES_VERSION)/android.jar
framework_res_package_export_deps := $(framework_res_package_export)
else # LOCAL_SDK_RES_VERSION
framework_res_package_export := \
    $(call intermediates-dir-for,APPS,framework-res,,COMMON)/package-export.apk
ifeq ($(LOCAL_USE_YI_RES),true)
LOCAL_NO_MTKRES := true
framework_res_package_export += \
        $(call intermediates-dir-for,APPS,framework-res-yi,,COMMON)/package-export.apk
endif
ifneq ($(LOCAL_NO_MTKRES),true)
framework_res_package_export += \
	$(call intermediates-dir-for,APPS,mediatek-res,,COMMON)/package-export.apk
endif
ifeq ($(LOCAL_USE_YI2_RES),true)
framework_res_package_export += \
        $(call intermediates-dir-for,APPS,framework-res-yi2,,COMMON)/package-export.apk
else
ifeq ($(LOCAL_USE_YI3_RES),true)
framework_res_package_export += \
        $(call intermediates-dir-for,APPS,framework-res-yi2,,COMMON)/package-export.apk \
        $(call intermediates-dir-for,APPS,framework-res-yi3,,COMMON)/package-export.apk
else
ifeq ($(LOCAL_USE_YI4_RES),true)
framework_res_package_export += \
        $(call intermediates-dir-for,APPS,framework-res-yi2,,COMMON)/package-export.apk \
        $(call intermediates-dir-for,APPS,framework-res-yi3,,COMMON)/package-export.apk \
        $(call intermediates-dir-for,APPS,framework-res-yi4,,COMMON)/package-export.apk
endif # LOCAL_USE_YI4_RES
endif # LOCAL_USE_YI3_RES
endif # LOCAL_USE_YI2_RES

# We can't depend directly on the export.apk file; it won't get its
# PRIVATE_ vars set up correctly if we do.  Instead, depend on the
# corresponding R.stamp file, which lists the export.apk as a dependency.
framework_res_package_export_deps := \
    $(foreach pkg, $(framework_res_package_export), $(dir $(pkg))src/R.stamp)
endif # LOCAL_SDK_RES_VERSION
$(R_file_stamp): $(framework_res_package_export_deps)
$(LOCAL_INTERMEDIATE_TARGETS): \
    PRIVATE_AAPT_INCLUDES := $(framework_res_package_export)
endif # LOCAL_NO_STANDARD_LIBRARIES

ifneq ($(full_classes_jar),)
$(LOCAL_BUILT_MODULE): PRIVATE_DEX_FILE := $(built_dex)
$(LOCAL_BUILT_MODULE): $(built_dex)
endif # full_classes_jar


# Get the list of jni libraries to be included in the apk file.

so_suffix := $($(my_prefix)SHLIB_SUFFIX)

jni_shared_libraries := \
    $(addprefix $($(my_prefix)OUT_INTERMEDIATE_LIBRARIES)/, \
      $(addsuffix $(so_suffix), \
        $(LOCAL_JNI_SHARED_LIBRARIES)))

# App explicitly requires the prebuilt NDK libstlport_shared.so.
# libstlport_shared.so should never go to the system image.
# Instead it should be packaged into the apk.
ifeq (stlport_shared,$(LOCAL_NDK_STL_VARIANT))
ifndef LOCAL_SDK_VERSION
$(error LOCAL_SDK_VERSION has to be defined together with LOCAL_NDK_STL_VARIANT, \
    LOCAL_PACKAGE_NAME=$(LOCAL_PACKAGE_NAME))
endif
jni_shared_libraries += \
    $(HISTORICAL_NDK_VERSIONS_ROOT)/current/sources/cxx-stl/stlport/libs/$(TARGET_CPU_ABI)/libstlport_shared.so
endif

# Set the abi directory used by the local JNI shared libraries.
# (Doesn't change how the local shared libraries are compiled, just
# sets where they are stored in the apk.)

ifeq ($(LOCAL_JNI_SHARED_LIBRARIES_ABI),)
    jni_shared_libraries_abi := $(TARGET_CPU_ABI)
else
    jni_shared_libraries_abi := $(LOCAL_JNI_SHARED_LIBRARIES_ABI)
endif

# Pick a key to sign the package with.  If this package hasn't specified
# an explicit certificate, use the default.
# Secure release builds will have their packages signed after the fact,
# so it's ok for these private keys to be in the clear.
ifeq ($(LOCAL_CERTIFICATE),)
  ifeq ($(MTK_SIGNATURE_CUSTOMIZATION),yes)
    LOCAL_CERTIFICATE := releasekey
  else
    LOCAL_CERTIFICATE := $(DEFAULT_SYSTEM_DEV_CERTIFICATE)
  endif
    
endif

ifeq ($(LOCAL_CERTIFICATE),EXTERNAL)
  # The special value "EXTERNAL" means that we will sign it with the
  # default devkey, apply predexopt, but then expect the final .apk
  # (after dexopting) to be signed by an outside tool.
  ifeq ($(MTK_SIGNATURE_CUSTOMIZATION),yes)
    LOCAL_CERTIFICATE := releasekey
  else
  LOCAL_CERTIFICATE := $(DEFAULT_SYSTEM_DEV_CERTIFICATE)
  endif
  PACKAGES.$(LOCAL_PACKAGE_NAME).EXTERNAL_KEY := 1
endif

# If this is not an absolute certificate, assign it to a generic one.
ifeq ($(dir $(strip $(LOCAL_CERTIFICATE))),./)
  ifeq ($(MTK_SIGNATURE_CUSTOMIZATION),yes)
    ifeq ($(MTK_INTERNAL),yes)
      LOCAL_CERTIFICATE := $(SRC_TARGET_DIR)/product/security/common/$(LOCAL_CERTIFICATE)
    else
      LOCAL_CERTIFICATE := $(SRC_TARGET_DIR)/product/security/$(TARGET_PRODUCT)/$(LOCAL_CERTIFICATE)
    endif
  else
    LOCAL_CERTIFICATE := $(dir $(DEFAULT_SYSTEM_DEV_CERTIFICATE))$(LOCAL_CERTIFICATE)
  endif
endif
private_key := $(LOCAL_CERTIFICATE).pk8
certificate := $(LOCAL_CERTIFICATE).x509.pem

$(LOCAL_BUILT_MODULE): $(private_key) $(certificate) $(SIGNAPK_JAR)
$(LOCAL_BUILT_MODULE): PRIVATE_PRIVATE_KEY := $(private_key)
$(LOCAL_BUILT_MODULE): PRIVATE_CERTIFICATE := $(certificate)

PACKAGES.$(LOCAL_PACKAGE_NAME).PRIVATE_KEY := $(private_key)
PACKAGES.$(LOCAL_PACKAGE_NAME).CERTIFICATE := $(certificate)

# Define the rule to build the actual package.
$(LOCAL_BUILT_MODULE): $(AAPT) | $(ZIPALIGN)
ifdef LOCAL_DEX_PREOPT
# Make sure the boot jars get dexpreopt-ed first
$(LOCAL_BUILT_MODULE): $(DEXPREOPT_BOOT_ODEXS) | $(DEXPREOPT) $(DEXOPT)
endif
$(LOCAL_BUILT_MODULE): PRIVATE_JNI_SHARED_LIBRARIES := $(jni_shared_libraries)
$(LOCAL_BUILT_MODULE): PRIVATE_JNI_SHARED_LIBRARIES_ABI := $(jni_shared_libraries_abi)
ifneq ($(TARGET_BUILD_APPS),)
    # Include all resources for unbundled apps.
    LOCAL_AAPT_INCLUDE_ALL_RESOURCES := true
endif
ifeq ($(LOCAL_AAPT_INCLUDE_ALL_RESOURCES),true)
    $(LOCAL_BUILT_MODULE): PRIVATE_PRODUCT_AAPT_CONFIG :=
    $(LOCAL_BUILT_MODULE): PRIVATE_PRODUCT_AAPT_PREF_CONFIG :=
else
    $(LOCAL_BUILT_MODULE): PRIVATE_PRODUCT_AAPT_CONFIG := $(PRODUCT_AAPT_CONFIG)
    $(LOCAL_BUILT_MODULE): PRIVATE_PRODUCT_AAPT_PREF_CONFIG := $(PRODUCT_AAPT_PREF_CONFIG)
endif
$(LOCAL_BUILT_MODULE): $(all_res_assets) $(jni_shared_libraries) $(full_android_manifest)
	@echo "target Package: $(PRIVATE_MODULE) ($@)"
	$(create-empty-package)
	$(add-assets-to-package)
ifneq ($(jni_shared_libraries),)
	$(add-jni-shared-libs-to-package)
endif
ifneq ($(full_classes_jar),)
	$(add-dex-to-package)
endif
	$(add-carried-java-resources)
ifneq ($(extra_jar_args),)
	$(add-java-resources-to-package)
endif
	$(sign-package)
	@# Alignment must happen after all other zip operations.
	$(align-package)
ifdef LOCAL_DEX_PREOPT
	$(hide) rm -f $(patsubst %.apk,%.odex,$@)
	$(call dexpreopt-one-file,$@,$(patsubst %.apk,%.odex,$@))
ifneq (nostripping,$(LOCAL_DEX_PREOPT))
	$(call dexpreopt-remove-classes.dex,$@)
endif

built_odex := $(basename $(LOCAL_BUILT_MODULE)).odex
$(built_odex): $(LOCAL_BUILT_MODULE)
endif

# Save information about this package
PACKAGES.$(LOCAL_PACKAGE_NAME).OVERRIDES := $(strip $(LOCAL_OVERRIDES_PACKAGES))
PACKAGES.$(LOCAL_PACKAGE_NAME).RESOURCE_FILES := $(all_resources)

PACKAGES := $(PACKAGES) $(LOCAL_PACKAGE_NAME)

# Lint phony targets
.PHONY: lint-$(LOCAL_PACKAGE_NAME)
lint-$(LOCAL_PACKAGE_NAME): PRIVATE_PATH := $(LOCAL_PATH)
lint-$(LOCAL_PACKAGE_NAME): PRIVATE_LINT_FLAGS := $(LOCAL_LINT_FLAGS)
lint-$(LOCAL_PACKAGE_NAME) :
	@echo lint $(PRIVATE_PATH)
	$(LINT) $(PRIVATE_LINT_FLAGS) $(PRIVATE_PATH)

lintall : lint-$(LOCAL_PACKAGE_NAME)

endif # skip_definition
