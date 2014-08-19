LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

$(shell rm -f $(LOCAL_PATH)/custom)
$(call config-custom-folder,custom:hal/aal)

LOCAL_SRC_FILES := \
    cust_aal_main.cpp 

LOCAL_SRC_FILES += \
    custom/cust_aal.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc,$(MTK_PLATFORM))/hardware/aal/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(call lc,$(MTK_PLATFORM))/kernel/drivers/dispsys \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc

LOCAL_MODULE:= libaal_cust

include $(BUILD_SHARED_LIBRARY)
