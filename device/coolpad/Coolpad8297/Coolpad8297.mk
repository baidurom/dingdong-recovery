# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file is the build configuration for a full Android
# build for sapphire hardware. This cleanly combines a set of
# device-specific aspects (drivers) with a device-agnostic
# product configuration (apps).
#


ORIGINAL_PATH := device/coolpad/Coolpad8297/official_jb
# Get the long list of APNs
PRODUCT_COPY_FILES := $(ORIGINAL_PATH)/system/etc/apns-conf.xml:system/etc/apns-conf.xml

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/common.mk)
#$(call inherit-product, $(SRC_TARGET_DIR)/product/full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# for baidu ota
PRODUCT_PROPERTY_OVERRIDES += \
    ro.baidu.recovery.verify=1 \
    ro.baidu.build.hardware=Coolpad8297 \
    ro.baidu.build.hardware.version=1.0

# Discard inherited values and use our own instead.
PRODUCT_DEVICE := Coolpad8297
# Huawei manufacturer infomations
PRODUCT_MANUFACTURER := Baidu
PRODUCT_BRAND := Coolpad
PRODUCT_NAME := Coolpad8297
PRODUCT_MODEL := Coolpad 8297

# This is for custom project language configuration.
PRODUCT_LOCALES := $(MTK_PRODUCT_LOCALES)
PRODUCT_LOCALES += $(MTK_PRODUCT_AAPT_CONFIG)

# linchunliang: add for TARGET_ROOT_OUT
INIT += fstab
INIT += init
PRODUCT_PACKAGES += $(INIT)

PRODUCT_PACKAGES += \
    audio.primary.msm7627a \
    audio_policy.msm7627a \
    gralloc.msm7627a.so \
    copybit.msm7627a.so \
    hwcomposer.msm7627a.so \
    liboverlay.so \
    libmemalloc.so \
    libgenlock.so \
    BaiduDualCardSetting

# for FM
PRODUCT_PACKAGES += \
    BaiduFM

# for BaiduDualCardSettings
PRODUCT_PACKAGES += \
    CellConnService

# for off-mode charging
PRODUCT_PACKAGES += \
    ipod \
    libipod

# proprietary side of the device
$(call inherit-product-if-exists, device/coolpad/Coolpad8297/mtk-special.mk)

DISABLE_DEXPREOPT := false

# linchunliang: add bootanimation
PRODUCT_COPY_FILES += \
    device/baidu/Coolpad8297/bootanimation.zip:system/media/bootanimation.zip

# Install the features available on this device.
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml

# stone: remove temp
# frameworks/base/data/etc/baidu.software.sip.voip.xml:system/etc/permissions/baidu.software.sip.voip.xml

PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true \
    ro.com.android.dateformat=dd-MM-yyyy \
    ro.ril.hsxpa=1 \
    ro.ril.gprsclass=10 \
    ro.media.dec.jpeg.memcap=10000000

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.supplicant_scan_interval=15 \
    ro.com.android.dataroaming=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=320

PRODUCT_PROPERTY_OVERRIDES += \
    ro.call.record=1

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# This should not be needed but on-screen keyboard uses the wrong density without it.
PRODUCT_PROPERTY_OVERRIDES += \
    qemu.sf.lcd_density=320

PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true \
    ro.com.android.dateformat=dd-MM-yyyy \
    ro.ril.hsxpa=2 \
    ro.ril.gprsclass=10 \
    ro.telephony.default_network=0 \
    ro.telephony.call_ring.multiple=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.setupwizard.enable_bypass=1 \
    ro.media.dec.jpeg.memcap=20000000 \
    dalvik.vm.lockprof.threshold=500 \
    dalvik.vm.dexopt-flags=m=y \
    dalvik.vm.execution-mode=int:jit \
    dalvik.vm.dexopt-data-only=1 \
    ro.opengles.version=131072  \
    ro.compcache.default=0 \
    ro.data.media.minsize=50m \
    ro.config.oem_storage=1

PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp,adb \
    ro.baidu.build.hardware.version=1.0 \
    ro.baidu.build.hardware=Coolpad8297

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.device.alias=Coolpad8297

PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.dsds_mode=umts_gsm

# linchunliang@baiyi-mobile.com: fm speaker support
PRODUCT_PROPERTY_OVERRIDES += \
    ro.fm.speaker=true

PRODUCT_PROPERTY_OVERRIDES += \
    ro.camera.sound.forced=0

PRODUCT_PROPERTY_OVERRIDES += \
    ro.baidu.2nd_storage.format=enable

# add by tangliuxiang01@baidu.com to support Coolpad8297's camera recording
PRODUCT_PROPERTY_OVERRIDES += \
    ro.camera.record_hint.disabled=1

# only for mtk
#$(call inherit-product, $(SRC_TARGET_DIR)/product/common.mk)
#$(call inherit-product, $(SRC_TARGET_DIR)/product/telephony.mk)
# only for mtk

