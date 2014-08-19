# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2011. All rights reserved.
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

##############################################################
# Including Neccesary Files
#

include ../build/Makefile

$(call codebase-path,preloader,$(abspath $(CURDIR)/$(call to-root)))

##############################################################
# Secure Library Building Control
#

export CREATE_SEC_LIB := FALSE

##############################################################
# Variable Initialization
#

export D_ROOT           := $(CURDIR)
export W_ROOT           := $(subst /,\,$(CURDIR))
ALL                     ?= clean show_title

#
# Image Auth
#
CUSTOM_PATH             := ${MTK_ROOT_CUSTOM}/${MTK_PROJECT}/security
IMAGE_AUTH_CFG_FILE     := ${MTK_PATH_PLATFORM}/src/security/inc/KEY_IMAGE_AUTH.h
IMAGE_AUTH_CONFIG_PATH  := ${CUSTOM_PATH}/image_auth
IMAGE_AUTH_KEY          := ${IMAGE_AUTH_CONFIG_PATH}/IMG_AUTH_KEY.ini
IMAGE_AUTH_CFG          := ${IMAGE_AUTH_CONFIG_PATH}/IMG_AUTH_CFG.ini
IMAGE_AUTH_KEY_EXIST    := $(shell if [ -f $(IMAGE_AUTH_KEY) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(IMAGE_AUTH_KEY_EXIST)","TRUE")
ALL                     += img_auth_info
endif

#
# PL Version
#
PL_CFG_FILE		:= ${CUSTOM_PATH}/pl_auth/PL_CFG.ini
PL_CFG_HDR		:= ${MTK_PATH_PLATFORM}/src/security/inc/PL_CFG.h
PL_CFG_EXIST		:= $(shell if [ -f $(PL_CFG_FILE) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(PL_CFG_EXIST)","TRUE")
ALL			+= pl_cfg_info
endif

#   
# SML Encode
#
SML_ENCODE_CFG_FILE     := ${MTK_PATH_PLATFORM}/src/security/inc/KEY_SML_ENCODE.h
SML_CONFIG_PATH         := ${CUSTOM_PATH}/sml_auth
SML_ENCODE_KEY          := ${SML_CONFIG_PATH}/SML_ENCODE_KEY.ini
SML_ENCODE_CFG          := ${SML_CONFIG_PATH}/SML_ENCODE_CFG.ini
SML_ENCODE_KEY_EXIST    := $(shell if [ -f $(SML_ENCODE_KEY) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(SML_ENCODE_KEY_EXIST)","TRUE")
ALL                     += sml_encode_info
endif

#
# SML Auth
#
SML_AUTH_CFG_FILE       := ${MTK_PATH_PLATFORM}/src/security/inc/KEY_SML_AUTH.h
SML_AUTH_PATH           := ${CUSTOM_PATH}/sml_auth
SML_AUTH_KEY            := ${SML_CONFIG_PATH}/SML_AUTH_KEY.ini
SML_AUTH_CFG            := ${SML_CONFIG_PATH}/SML_AUTH_CFG.ini
SML_AUTH_KEY_EXIST      := $(shell if [ -f $(SML_AUTH_KEY) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(SML_AUTH_KEY_EXIST)","TRUE")
ALL                     += sml_auth_info
endif

#
# preloader extension
#
PLATFORM                := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
PRELOADER_EXT_BIN       := ${MTK_ROOT_CUSTOM}/${PLATFORM}/preloader/preloader_ext.bin
PRELOADER_EXT_BIN_EXIST := $(shell if [ -f $(PRELOADER_EXT_BIN) ]; then echo "TRUE"; else echo "FALSE"; fi;)

#
# Tool
#
TOOL_PATH               := ../build/tools
SIGN_TOOL               := ${TOOL_PATH}/SignTool/SignTool_PL
CIPHER_TOOL             := ${TOOL_PATH}/CipherTool/CipherTool 

PL_IMAGE_NAME           := preloader_$(MTK_PROJECT)
SECURITY_LIB            := $(MTK_PATH_PLATFORM)/src/SecLib.a
DA_VERIFY_LIB           := $(MTK_PATH_PLATFORM)/src/security/auth/DaVerifyLib.a
SEC_PLAT_LIB            := $(MTK_PATH_PLATFORM)/src/SecPlat.a
ALL                     += build_info project_info $(SUBDIRS)
COMMON_DIR_MK           := $(D_ROOT)/build/common-dir.mak
COMMON_FILE_MK          := $(D_ROOT)/build/common.mak
export COMMON_DIR_MK COMMON_FILE_MK

ifeq ($(CREATE_SEC_LIB),TRUE)
    ALL                 += $(SECURITY_LIB) $(DA_VERIFY_LIB) $(SEC_PLAT_LIB)
    SUBDIRS             := $(MTK_PATH_PLATFORM)/src/
else
    ALL                 += $(D_BIN)/$(PL_IMAGE_NAME).elf System.map
    SUBDIRS             := $(MTK_PATH_PLATFORM)/src/ $(MTK_PATH_CUSTOM)/
endif

PL_MTK_CDEFS            := $(call mtk.custom.generate-macros)
PL_MTK_ADEFS            := $(call mtk.custom.generate-macros)
export PL_MTK_CDEFS PL_MTK_ADEFS

include build/debug.in build/setting.in 
include $(MTK_PATH_PLATFORM)/makefile.mak

.PHONY: $(ALL)
all: $(ALL)

##############################################################
# ELF Generation
#

$(D_BIN)/$(PL_IMAGE_NAME).elf:
	$(LD) --gc-sections -Bstatic -T$(LDSCRIPT) \
	$(wildcard $(D_OBJ)/*) $(SECURITY_LIB) \
	$(shell if [ -f $(DA_VERIFY_LIB) ]; then echo $(DA_VERIFY_LIB); else echo ""; fi;) \
	$(shell if [ -f $(SEC_PLAT_LIB) ]; then echo $(SEC_PLAT_LIB); else echo ""; fi;) \
	-Map system.map -o $(D_BIN)/$(PL_IMAGE_NAME).elf
	@$(OBJCOPY) ${OBJCFLAGS} $(D_BIN)/$(PL_IMAGE_NAME).elf -O binary $(D_BIN)/$(PL_IMAGE_NAME).bin 
ifeq ("$(PRELOADER_EXT_BIN_EXIST)","TRUE")
	cat $(PRELOADER_EXT_BIN) >> $(D_BIN)/$(PL_IMAGE_NAME).bin
endif
	cat $(MTK_PATH_CUSTOM)/MTK_Loader_Info.tag >> $(D_BIN)/$(PL_IMAGE_NAME).bin
	@readelf -s $@ | awk -F':' '/Num/ {print $$2}' > report-codesize.txt
	@readelf -s $@ | awk -F':' '{if ($$1>0) print $$2}' | awk -F' ' '{if ($$2>0) print $$0}' | sort +0 -1 >> report-codesize.txt
	@cat report-codesize.txt | sed -r 's/[ ]+/,/g' > report-codesize.csv

##############################################################
# Security Library Generation
#

$(SECURITY_LIB):
	rm -rf $(SECURITY_LIB)
	@echo AR out/*
	$(AR) -r $(SECURITY_LIB) out/*
	@echo =================================================================
	@echo Security Library
	@echo '$(SECURITY_LIB)' built at
	@echo time : $(shell date )
	@echo =================================================================

$(DA_VERIFY_LIB):
	rm -rf $(DA_VERIFY_LIB)
	@echo AR out_da_verify/*
	$(AR) -r $(DA_VERIFY_LIB) out_da_verify/*
	@echo =================================================================
	@echo DA Verify Library
	@echo '$(DA_VERIFY_LIB)' built at
	@echo time : $(shell date )
	@echo =================================================================

$(SEC_PLAT_LIB):
	rm -rf $(SEC_PLAT_LIB)
	@echo AR out/*
	$(AR) -r $(SEC_PLAT_LIB) out_plat/*
	@echo =================================================================
	@echo Security Library
	@echo '$(SEC_PLAT_LIB)' built at
	@echo time : $(shell date )
	@echo =================================================================

##############################################################
# File for Debugging
#

System.map: $(D_BIN)/$(PL_IMAGE_NAME).elf
	@$(NM) $< | \
	grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
	sort > function.map

##############################################################
# Dump Configurations
#

show_title:
	@echo =================================================================
	@echo Building Configuration:
	@echo Project              = $(MTK_PROJECT)
	@echo Platform             = $(MTK_PLATFORM)
	@echo Buildspec            = buildspec.mak
	@echo Create SecLib        = $(CREATE_SEC_LIB)
	@echo Image Auth key exist = $(IMAGE_AUTH_KEY_EXIST)
	@echo SML Encode key exist = $(SML_ENCODE_KEY_EXIST)
	@echo SML Auth key exist   = $(SML_AUTH_KEY_EXIST)
	@echo Preloader EXT exist  = $(PRELOADER_EXT_BIN_EXIST)
	@echo SECRO AC support     = $(MTK_SEC_SECRO_AC_SUPPORT)
	@echo =================================================================

##############################################################
# Adding Build Time
#

build_info:		
	@echo // Auto generated. Build Time Information > $(MTK_PATH_PLATFORM)/src/init/inc/preloader.h		
	@echo '#'define BUILD_TIME '"'$(shell date +%Y%m%d-%H%M%S)'"' >> $(MTK_PATH_PLATFORM)/src/init/inc/preloader.h


##############################################################
# Adding Project Configuration
#

project_info:
	@echo // Auto generated. Import ProjectConfig.mk > $(MTK_PATH_PLATFORM)/src/security/inc/proj_cfg.h
	@echo '#'define CUSTOM_SUSBDL_CFG $(MTK_SEC_USBDL) >> $(MTK_PATH_PLATFORM)/src/security/inc/proj_cfg.h	
	@echo '#'define CUSTOM_SBOOT_CFG $(MTK_SEC_BOOT) >> $(MTK_PATH_PLATFORM)/src/security/inc/proj_cfg.h	
	@echo '#'define MTK_SEC_MODEM_AUTH $(MTK_SEC_MODEM_AUTH) >> $(MTK_PATH_PLATFORM)/src/security/inc/proj_cfg.h
        ifdef MTK_SEC_SECRO_AC_SUPPORT
	@echo '#'define MTK_SEC_SECRO_AC_SUPPORT $(MTK_SEC_SECRO_AC_SUPPORT) >> $(MTK_PATH_PLATFORM)/src/security/inc/proj_cfg.h
        endif		

##############################################################
# Generate Key Info File
#

img_auth_info:
	@touch $(IMAGE_AUTH_CFG_FILE)
	@chmod 777 $(IMAGE_AUTH_CFG_FILE)
	@./$(SIGN_TOOL) $(IMAGE_AUTH_KEY) $(IMAGE_AUTH_CFG) $(IMAGE_AUTH_CFG_FILE) IMG

sml_encode_info:	
	@touch $(SML_ENCODE_CFG_FILE)
	@chmod 777 $(SML_ENCODE_CFG_FILE)
	@./$(CIPHER_TOOL) GEN_HEADER $(SML_ENCODE_KEY) $(SML_ENCODE_CFG) $(SML_ENCODE_CFG_FILE) SML	

sml_auth_info:	
	@touch $(SML_AUTH_CFG_FILE)
	@chmod 777 $(SML_AUTH_CFG_FILE)
	@./$(SIGN_TOOL) $(SML_AUTH_KEY) $(SML_AUTH_CFG) $(SML_AUTH_CFG_FILE) SML

pl_cfg_info:
	@touch $(PL_CFG_HDR)
	@chmod 777 $(PL_CFG_HDR)
	@echo '#'define $(shell cat $(PL_CFG_FILE) | sed -s 's/=//g') > $(PL_CFG_HDR)

$(SUBDIRS):
	@$(MAKE) -C $@ --no-print-directory -s

##############################################################
# Clean 
#

clean:
	@rm -rf $(D_ROOT)/out
	@mkdir  $(D_ROOT)/out
	@rm -rf $(D_ROOT)/bin
	@mkdir  $(D_ROOT)/bin
	@rm -rf $(D_ROOT)/out_da_verify
	@mkdir  $(D_ROOT)/out_da_verify
	@rm -rf $(D_ROOT)/out_plat
	@mkdir  $(D_ROOT)/out_plat
	

##############################################################
# EMI Customization
#

emigen_files := \
    $(MTK_ROOT_CUSTOM)/$(MTK_PROJECT)/preloader/custom_emi.c \
    $(MTK_ROOT_CUSTOM)/$(MTK_PROJECT)/preloader/inc/custom_emi.h

$(ALL): prepare

custom-files := $(strip $(call mtk.custom.generate-rules,prepare,preloader,$(emigen_files)))
$(custom-files): $(emigen_files)
$(emigen_files):
	cd $(to-root); ./makeMtk $(FULL_PROJECT) emigen; cd -;
