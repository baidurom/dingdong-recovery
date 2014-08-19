LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_pq.cpp 

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
        $(TOP)/mediatek/platform/mt6589/kernel/drivers/dispsys

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \

LOCAL_MODULE:= pq

include $(BUILD_EXECUTABLE)