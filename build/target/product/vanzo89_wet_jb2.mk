
PRODUCT_PACKAGES := \
#    FMRadio
#    MyTube \
#    VideoPlayer


$(call inherit-product, $(SRC_TARGET_DIR)/product/common.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/telephony.mk)

### add by baidu
#$(call inherit-product, device/baidu/common/common.mk)
$(call inherit-product, device/baidu/common/rom.mk)
$(call inherit-product, mediatek/config/vanzo89_wet_jb2/baidudsdsfeature.mk)

# Overrides
PRODUCT_BRAND  := alps
PRODUCT_NAME   := $(TARGET_PRODUCT)
PRODUCT_DEVICE := $(TARGET_PRODUCT)

# This is for custom project language configuration.
PRODUCT_LOCALES := \
        en_US \
        es_ES \
        zh_CN \
        zh_TW \
        ru_RU \
        pt_BR \
        fr_FR \
        de_DE \
        tr_TR \
        it_IT \
        in_ID \
        ms_MY \
        vi_VN \
        ar_EG \
        hi_IN \
        th_TH \
        bn_IN \
        pt_PT \
        ur_PK \
        fa_IR \
        nl_NL \
        xhdpi \
        hdpi

##############################################################################
##############################################################################
########Below Setting is from vanzo89_wet_jb2.mk.custom, dont edit directly########
##############################################################################
##############################################################################

PRODUCT_BRAND  := UMI_X1S
PRODUCT_LOCALES :=  \
        zh_CN \
        ar_EG \
        bn_IN \
        de_DE \
        en_US \
        es_ES \
        fa_IR \
        fr_FR \
        hi_IN \
        in_ID \
        it_IT \
        ms_MY \
        pt_PT \
        ru_RU \
        th_TH \
        vi_VN \
        zh_TW \
        xhdpi \
        hdpi
PRODUCT_PROPERTY_OVERRIDES += \
        qemu.hw.mainkeys=1

# for baidu ota
PRODUCT_PROPERTY_OVERRIDES += \
    ro.baidu.recovery.verify=1 \
    ro.baidu.build.hardware=fanzhuo-umix1s \
    ro.baidu.build.hardware.version=1.0
