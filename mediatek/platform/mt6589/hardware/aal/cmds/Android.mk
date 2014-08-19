
ifeq ($(MTK_AAL_SUPPORT),yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_aal.cpp 

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libbinder \
    libaal \

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/aal/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/protect/platform/mt6589/hardware/aal/lib \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/mt6589/kernel/drivers/dispsys \

LOCAL_MODULE:= aal

include $(BUILD_EXECUTABLE)
endif