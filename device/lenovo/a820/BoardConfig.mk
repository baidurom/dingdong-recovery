# config.mk
#
# Product-specific compile-time definitions.
#
#LOCAL_PATH:= $(call my-dir)


# add to choose existing libs
USE_BAIDU_EXISTING_LIBS := true

# add for recovery
BOARD_NO_RGBX_8888 := true
TARGET_RECOVERY_UI_LIB := librecovery_ui_a820

TARGET_PROVIDES_INIT_RC := true

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi

TARGET_CPU_SMP := true

# eMMC support
ifeq ($(MTK_EMMC_SUPPORT),yes)
TARGET_USERIMAGES_USE_EXT4:=true
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := false
endif

USE_CAMERA_STUB := true

TARGET_NO_FACTORYIMAGE := true

# for migrate build system
# temporarily open this two options
HAVE_HTC_AUDIO_DRIVER := true
#BOARD_USES_GENERIC_AUDIO := true
 
BOARD_USES_MTK_AUDIO := true

BOARD_EGL_CFG := device/lenovo/$(TARGET_DEVICE)/official_jb/system/lib/egl/egl.cfg

BOARD_MTK_LIBSENSORS_NAME :=
BOARD_MTK_LIB_SENSOR :=

# MTK, Baochu Wang, 20101130, Add A-GPS {
ifeq ($(MTK_AGPS_APP), yes)
   BOARD_AGPS_SUPL_LIBRARIES := true
else
   BOARD_AGPS_SUPL_LIBRARIES := false
endif
# MTK, Baochu Wang, 20101130, Add A-GPS }

ifeq ($(MTK_GPS_SUPPORT), yes)
  BOARD_GPS_LIBRARIES := true
else
  BOARD_GPS_LIBRARIES := false
endif

# MTK, Infinity, 20090720, Add WiFi {
ifeq ($(MTK_WLAN_SUPPORT), yes)
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
BOARD_P2P_SUPPLICANT_DRIVER := NL80211
HAVE_CUSTOM_WIFI_DRIVER_2 := true
HAVE_INTERNAL_WPA_SUPPLICANT_CONF := true
HAVE_CUSTOM_WIFI_HAL := mediatek
WPA_SUPPLICANT_VERSION := VER_0_6_X
P2P_SUPPLICANT_VERSION := VER_0_8_X
endif
# MTK, Infinity, 20090720, Add WiFi }

#TARGET_KMODULES := true

TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon

ifeq ($(strip $(MTK_NAND_PAGE_SIZE)), 4K)
  BOARD_NAND_PAGE_SIZE := 4096 -s 128
else
  BOARD_NAND_PAGE_SIZE := 2048 -s 64   # default 2K
endif

COMMON_GLOBAL_CFLAGS += -DMTK_MEDIA_SUPPORT_4_1
COMMON_GLOBAL_CFLAGS += -DMTK_AUDIO_SUPPORT_V0

# add to support flashing 3rd party ROMs
#PRODUCT_DEVICE_ALIAS := a820


