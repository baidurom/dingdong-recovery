#
# config.mk
#
# Product-specific compile-time definitions.
#
# add kernel header files path by lidaqing 
QCOM_KERNEL_HEADER_PATH := device/qcom/msm8625
include device/qcom/msm7627a/BoardConfig.mk

-include vendor/qcom/proprietary/common/msm8625/BoardConfigVendor.mk
