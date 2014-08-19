#
# libcamdrv_res_mgr
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    res_mgr_drv.cpp

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/src/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_ROOT)/hardware/bwc/inc \
    $(TOP)/$(MTK_ROOT)/kernel/drivers/hdmitx/

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
LOCAL_MODULE := libcamdrv_res_mgr

#
include $(BUILD_STATIC_LIBRARY)

#
#include $(call all-makefiles-under, $(LOCAL_PATH))
