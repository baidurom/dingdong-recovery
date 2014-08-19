#
# native_svc
# 
# Include this file in a product makefile to include the binary files
#
LOCAL_PATH := $(my-dir)
$(shell cp -a $(LOCAL_PATH)/upgraded $(PRODUCT_OUT)/system/bin/upgraded)
