#
# libacdk
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    $(call all-c-cpp-files-under, acdk) \
    $(call all-c-cpp-files-under, surfaceview) \
#    $(call all-c-cpp-files-under, cct)


#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
LOCAL_C_INCLUDES += \
    $(TOP)/bionic \
    $(TOP)/external/stlport/stlport \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/ \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/acdk/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/acdk/inc/cct \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/imageio/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/isp_tuning \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/ae_mgr \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/m4u \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(TOP)/mediatek/source/external/mhal/src/core/drv/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/common/camexif \

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#
LOCAL_STATIC_LIBRARIES := \
    libcct \

ifeq ($(BUILD_MTK_LDVT),true)
   LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif
#

#
LOCAL_SHARED_LIBRARIES := \
    libstlport \
    libcutils \
    libimageio \
    libcamdrv \
    libfeatureio \
    libm4u \
    libcameracustom  \
    libcam.camshot \
    libcam_camera_exif

#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE := libacdk

#
include $(BUILD_SHARED_LIBRARY)
#
include $(call all-makefiles-under, $(LOCAL_PATH))
