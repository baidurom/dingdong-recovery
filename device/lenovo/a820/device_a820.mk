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


ORIGINAL_PATH := device/lenovo/a820/official_jb

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
#$(call inherit-product, $(SRC_TARGET_DIR)/product/full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# stone: add while ready

# Discard inherited values and use our own instead.
PRODUCT_DEVICE := a820
# Huawei manufacturer infomations
PRODUCT_MANUFACTURER := Baidu
PRODUCT_BRAND := CloudROM
PRODUCT_NAME := a820
PRODUCT_MODEL := Lenovo A820


# linchunliang: add for TARGET_ROOT_OUT
INIT += meta_init.project.rc
INIT += init.protect.rc
INIT += factory_init.rc
INIT += fstab
INIT += factory_init.project.rc
INIT += init.usb.rc
INIT += init.trace.rc
INIT += init.charging.rc
INIT += init.factory.rc
INIT += init.aee.customer.rc
INIT += init.project.rc
INIT += init.xlog.rc
INIT += advanced_meta_init.project.rc
INIT += advanced_meta_init.rc
INIT += meta_init.rc
INIT += init
INIT += ueventd.rc
PRODUCT_PACKAGES += $(INIT)


# for off-mode charging
PRODUCT_PACKAGES += \
    ipod \
    libipod

# proprietary side of the device
#$(call inherit-product-if-exists, device/baidu/a820/mtk-special.mk)

DISABLE_DEXPREOPT := false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=240

PRODUCT_PROPERTY_OVERRIDES += \
    ro.call.record=1

# atlas40 uses high-density artwork where available
PRODUCT_LOCALES += hdpi

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# This should not be needed but on-screen keyboard uses the wrong density without it.
PRODUCT_PROPERTY_OVERRIDES += \
    qemu.sf.lcd_density=240

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
    persist.sys.usb.config=mass_storage,adb \
    ro.baidu.build.hardware.version=1.0 \
    ro.baidu.build.hardware=a820

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.device.alias=a820,A820,GT-N7102

PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.dsds_mode=umts_gsm

PRODUCT_PROPERTY_OVERRIDES += \
    ro.camera.sound.forced=0

PRODUCT_PROPERTY_OVERRIDES += \
    ro.baidu.2nd_storage.format=enable

# add by tangliuxiang01@baidu.com to support a820's camera recording
PRODUCT_PROPERTY_OVERRIDES += \
    ro.camera.record_hint.disabled=1

# only for mtk
#$(call inherit-product, $(SRC_TARGET_DIR)/product/common.mk)
#$(call inherit-product, $(SRC_TARGET_DIR)/product/telephony.mk)
# only for mtk
