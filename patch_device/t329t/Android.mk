LOCAL_PATH := $(call my-dir)
include $(DD_CLEAR)
DD_PRODUCT := t329t
DD_KERNEL := $(LOCAL_PATH)/kernel

DD_KERNEL_BASE := 0x000000
DD_KERNEL_PAGESIZE := 2048
DD_KERNEL_CMDLINE := cachepolicy=writealloc noinitrd init=init board_id= vmalloc=128M logo. startup_graphics= mpcore_wdt.mpcore_margin=359 root=/dev/ram0 rw rootwait mem=128M@0 hwmem=100M@128M mem=155M@228M mem_issw=1M@383M mem=127M@384M ram_console=1M@511M
DD_DEVICE_SCREEN_TYPE := "HDPI"

DD_PRODUCT_ROOT := $(LOCAL_PATH)/root
DD_DEVICE_CONFIG := $(LOCAL_PATH)/*.conf
include $(DD_RECOVERY)
