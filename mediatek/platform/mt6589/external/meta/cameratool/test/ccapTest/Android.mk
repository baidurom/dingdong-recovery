#
# msdkCCAP Test 
#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES:= \
        ./AcdkCCAPTest.cpp
    

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/platform/mt6589/hardware/camera/inc/acdk \
    $(MTK_PATH_SOURCE)/platform/mt6589/hardware/camera/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/platform/mt6589/hardware/camera/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES:= liblog libcutils libacdk

LOCAL_MODULE:= ccaptest

ifneq (yes,$(strip $(MTK_EMULATOR_SUPPORT)))
include $(BUILD_EXECUTABLE)
endif
