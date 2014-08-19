#
# libimageio_plat_drv
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    $(call all-c-cpp-files-under, cam) \
    $(call all-c-cpp-files-under, cdp) \

# camutils: For CameraProfile APIs.
#
LOCAL_C_INCLUDES := $(TOP)/bionic \
    $(TOP)/external/stlport/stlport \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/imageio/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/imageio/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/m4u \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils \

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#ifeq ($(BUILD_MTK_LDVT),true)
#    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
#endif

#
LOCAL_MODULE := libimageio_plat_drv

#
include $(BUILD_STATIC_LIBRARY)

#
#include $(call all-makefiles-under, $(LOCAL_PATH))
