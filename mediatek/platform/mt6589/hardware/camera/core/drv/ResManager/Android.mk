#
# libcamdrv_resmanager
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    ResManager.cpp

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_ROOT)/hardware/camera/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc \

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
LOCAL_MODULE := libcamdrv_resmanager

#
include $(BUILD_STATIC_LIBRARY)

#
#include $(call all-makefiles-under, $(LOCAL_PATH))
