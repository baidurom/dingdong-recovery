PRODUCT_PROPERTY_OVERRIDES += \
       dalvik.vm.heapstartsize=5m \
       dalvik.vm.heapgrowthlimit=48m \
       dalvik.vm.heapsize=128m

PRODUCT_COPY_FILES += device/qcom/msm7627a/media/media_profiles_7627a.xml:system/etc/media_profiles.xml \
                      device/qcom/msm7627a/media/media_codecs_7627a.xml:system/etc/media_codecs.xml

$(call inherit-product, device/qcom/common/common.mk)

#add more msm7627a packages
PRODUCT_PACKAGES += cdrom_install.iso

PRODUCT_NAME := msm7627a
PRODUCT_DEVICE := msm7627a

DEVICE_PACKAGE_OVERLAYS := device/qcom/msm7627a/overlay

#Bluetooth configuration files
PRODUCT_COPY_FILES += \
   system/bluetooth/data/main.le.conf:system/etc/bluetooth/main.conf
PRODUCT_PACKAGES += fstab.msm7627a
