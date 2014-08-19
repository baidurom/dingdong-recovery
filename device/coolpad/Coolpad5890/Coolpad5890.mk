
# This device is hdpi.

PRODUCT_PROPERTY_OVERRIDES += \
	   ro.baidu.build.hardware=Coolpad5890 \
       ro.baidu.build.hardware.version=1.0 \
       dalvik.vm.heapstartsize=5m \
       dalvik.vm.heapgrowthlimit=48m \
       dalvik.vm.heapsize=128m \
       persist.sys.usb.config=mass_storage,adb

$(call inherit-product, device/qcom/common/common.mk)

#add more msm7627a packages
PRODUCT_PACKAGES += init.rc
PRODUCT_PACKAGES += ueventd.rc
PRODUCT_PACKAGES += init.msm7627a.rc
PRODUCT_PACKAGES += init.msm7630.rc
PRODUCT_PACKAGES += init.usb.rc
PRODUCT_PACKAGES += charger
PRODUCT_PACKAGES += init.trace.rc
PRODUCT_PACKAGES += e2fsk.sh
PRODUCT_PACKAGES += nv_set
PRODUCT_PACKAGES += resize2fs_s
PRODUCT_PACKAGES += rmt_storage_recovery
PRODUCT_PACKAGES += fstab.qcom
PRODUCT_PACKAGES += fstab.msm7627a

LOCAL_KERNEL := device/coolpad/Coolpad5890/kernel
PRODUCT_COPY_FILES += $(LOCAL_KERNEL):kernel

PRODUCT_NAME := Coolpad5890
PRODUCT_DEVICE := Coolpad5890
PRODUCT_MODEL := Coolpad 5890
PRODUCT_BRAND := Coolpad
