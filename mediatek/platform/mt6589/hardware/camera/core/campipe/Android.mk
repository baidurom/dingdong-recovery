#
# libcam.campipe
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    PipeImp.cpp \
    $(call all-c-cpp-files-under, CamIOPipe) \
    $(call all-c-cpp-files-under, CdpPipe) \
    $(call all-c-cpp-files-under, PostProcPipe) \
    $(call all-c-cpp-files-under, SamplePipe) \
    $(call all-c-cpp-files-under, utils) \

#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
LOCAL_C_INCLUDES += $(TOP)/bionic
LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
# 
# camera Hardware 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/base/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach


#
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libstlport \
    libimageio \
    libcamdrv \


#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libcampipe_plat_pipe_mgr \

#
LOCAL_MODULE := libcam.campipe

#
LOCAL_MODULE_TAGS := optional

#
include $(BUILD_SHARED_LIBRARY)

#
include $(call all-makefiles-under,$(LOCAL_PATH))
