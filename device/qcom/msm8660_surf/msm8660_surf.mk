$(call inherit-product, device/qcom/common/common.mk)

PRODUCT_NAME := msm8660_surf
PRODUCT_DEVICE := msm8660_surf

# Wallpapers
PRODUCT_PACKAGES += Galaxy4
PRODUCT_PACKAGES += HoloSpiralWallpaper
PRODUCT_PACKAGES += MagicSmokeWallpapers
PRODUCT_PACKAGES += NoiseField
PRODUCT_PACKAGES += PhaseBeam
#fstab.msm8660
PRODUCT_PACKAGES += fstab.msm8660
# Bluetooth configuration files
PRODUCT_COPY_FILES += \
    system/bluetooth/data/main.conf:system/etc/bluetooth/main.conf \
