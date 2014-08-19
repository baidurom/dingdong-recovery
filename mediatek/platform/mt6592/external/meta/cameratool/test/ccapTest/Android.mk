#
# msdkCCAP Test 
#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES:= \
        ./AcdkCCAPTest.cpp
    

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/acdk/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/inc/acdk \
    $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES:= liblog libcutils libacdk

LOCAL_MODULE:= ccaptest

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include

# End of common part ---------------------------------------
#

ifneq (yes,$(strip $(MTK_EMULATOR_SUPPORT)))
include $(BUILD_EXECUTABLE)
endif
