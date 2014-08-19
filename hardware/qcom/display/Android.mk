#Enables the listed display HAL modules
#Libs to be built for all targets (including SDK)
ifneq ($(TARGET_HTC_MSM7X27A),true)
display-hals := libqcomui libtilerenderer

#libs to be built for QCOM targets only
ifeq ($(call is-vendor-board-platform,QCOM),true)
display-hals += libhwcomposer liboverlay libgralloc libgenlock libcopybit
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm7x30)
display-hals += libhwcomposer liboverlay libgralloc
endif

include $(call all-named-subdir-makefiles,$(display-hals))
endif
