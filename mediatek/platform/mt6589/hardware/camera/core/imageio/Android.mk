#
# libimageio
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(HAVE_AEE_FEATURE),yes)
	LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

#
LOCAL_SHARED_LIBRARIES := \
    libstlport \
    libcutils \
    libcamdrv \
    libm4u \

# libcam.utils: For CameraProfile APIs. Only used when non-LDVT.
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SHARED_LIBRARIES += libcam.utils
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
endif

#
LOCAL_STATIC_LIBRARIES := \
#    libimageio_plat_pipe \
#    libimageio_plat_drv \
#    libimageio_plat_pipe_mgr \

LOCAL_WHOLE_STATIC_LIBRARIES := \
    libimageio_plat_pipe \
    libimageio_plat_drv \
    
#
LOCAL_MODULE := libimageio

#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE_TAGS := optional

#
include $(BUILD_SHARED_LIBRARY) 

#
include $(call all-makefiles-under, $(LOCAL_PATH))
