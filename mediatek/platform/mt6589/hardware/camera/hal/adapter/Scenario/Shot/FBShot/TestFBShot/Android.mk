#
# camshottest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    test_fbshot.cpp \
    main.cpp \


#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
LOCAL_C_INCLUDES += $(TOP)/bionic
LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
# 
# camera Hardware 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/Scenario/Shot/FBShot
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/inc/Scenario
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/Scenario/Shot/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils

# vector
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libstlport \

# shot 
LOCAL_SHARED_LIBRARIES += \
    libcam.camshot \

# Imem
LOCAL_SHARED_LIBRARIES += \
    libcamdrv \

# Imem
LOCAL_SHARED_LIBRARIES += \
    libcam.camadapter \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := camfbshottest

#
LOCAL_MODULE_TAGS := eng

#
LOCAL_PRELINK_MODULE := false

#
include $(BUILD_EXECUTABLE)

#
#include $(call all-makefiles-under,$(LOCAL_PATH))
