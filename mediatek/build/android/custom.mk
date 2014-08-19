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


include mediatek/build/Makefile
$(call codebase-path)

.PHONY: mtk-custom-files
*: mtk-custom-folder
mtk-custom-folder: mtk-custom-files
mtk-custom-files := $(strip $(call mtk.custom.generate-rules,mtk-custom-files))
custom-files: ;

mtk-custom-folder := 

#ifneq (true,$(LOCAL_GENERATE_CUSTOM_FOLDER))
#ifeq (,$(filter $(mtk-custom-folder),$(LOCAL_PATH)))
#mtk-custom-folder += $(LOCAL_PATH)
#_ := $(if $(LOCAL_PATH),$(LOCAL_PATH),$(call my-dir))
#$(foreach t,$(MTK_PATH_PLATFORM) $(MTK_PATH_SOURCE) $(MTK_PATH_CUSTOM),\
#  $(eval _ := $(subst $(t),,$(_))))
#_ := $(subst / ,/,$(foreach t,$(subst /, ,$(if $(LOCAL_PATH)\
#     ,$(LOCAL_PATH),$(call my-dir))),../))$(MTK_ROOT_CUSTOM_OUT)/$(_)
#mtk-custom-folder: $(LOCAL_PATH)/custom
#$(LOCAL_PATH)/custom: PRIVATE_CUSTOM_PATH := $(_)
#$(LOCAL_PATH)/custom: $(mtk-custom-files)
#        @echo making $@
#        @if [ ! -L $@ ]; then rm -f $@; ln -s $(PRIVATE_CUSTOM_PATH) $@; fi
#endif
#endif

define _generate-custom-folder
mtk-custom-folder: $(1)
$(1): PRIVATE_CUSTOM_PATH := $(2)
$(1): PRIVATE_DISPLAY_CUSTOM_PATH := $(patsubst $(CURDIR)/%,%,$(abspath $(LOCAL_PATH)/$(2)))
$(1): $$(mtk-custom-files) FORCE
	@echo "Custom: $$(@) -> $$(PRIVATE_DISPLAY_CUSTOM_PATH)"
	@rm -rf $$@
	@ln -s $$(PRIVATE_CUSTOM_PATH) $$@
$(eval $(shell rm -rf $(1); ln -s $(2) $(1)))
endef

define generate-custom-folder
$(if $(filter $(shell readlink -f $(1)),$(abspath $(LOCAL_PATH)/$(2))),,$(eval $(call _generate-custom-folder,$1,$2)))
endef

# workaround: manually run gen_java_global_definition
#$(eval result := $(shell ./makeMtk $(MTK_PROJECT) gen_java_global_definition))

define config-custom-folder
$(foreach t,$1,\
    $(eval _t := $(subst :, ,$(if $(filter all,$(t)),custom:,$(t)))) \
    $(eval _1 := $(LOCAL_PATH)/$(word 1,$(_t))) \
    $(if $(filter $(mtk-custom-folder),$(LOCAL_PATH)/$(_1)), \
        $(warning "$(LOCAL_PATH)/$(_1)" already existed!), \
        $(eval _2 := $(call to-root,$(LOCAL_PATH))/$(MTK_PATH_CUSTOM)/$(word 2,$(_t))) \
        $(eval $(call generate-custom-folder,$(_1),$(_2))) \
        $(eval mtk-custom-folder += $(LOCAL_PATH)/$(_1)) \
     ) \
)
endef
