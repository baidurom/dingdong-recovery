PRODUCT_PROPERTY_OVERRIDES += \
        dalvik.vm.heapstartsize=5m \
        dalvik.vm.heapgrowthlimit=36m \
        dalvik.vm.heapsize=128m

$(call inherit-product, device/qcom/common/common.mk)

PRODUCT_NAME := msm7630_surf
PRODUCT_DEVICE := msm7630_surf

# Bluetooth configuration files
PRODUCT_COPY_FILES += \
    system/bluetooth/data/main.conf:system/etc/bluetooth/main.conf \
