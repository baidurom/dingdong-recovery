include $(MTK_PATH_BUILD)/common.mk
#Fix-me: modified for early porting
#$(call all-subdir-src-or-makefile)
obj-y   += uart/
obj-y   += gpio/
obj-y   += power/
obj-y   += power_gs/
obj-y   += usb20/
obj-y   += m4u/
obj-y   += smi/
obj-y   += keypad/
obj-y   += video/
obj-y   += dispsys/
obj-y   += i2c/
obj-y   += mmc-host/
obj-y   += pmic_wrap/
obj-y   += pmt/
obj-y   += accdet/
obj-y   += vibrator/
obj-y   += leds/
obj-y   += sound/
obj-y   += spi/
obj-$(CONFIG_MTK_SYSTRACKER)   += systracker/
obj-$(CONFIG_MTK_BTIF)  += btif/
ifeq ($(CONFIG_MTK_COMBO),m)
obj-y   += wmt/
endif
obj-y   += mrdump/

ifneq ($(CONFIG_EARLY_LINUX_PORTING),y)
obj-y   += pwm/
obj-y   += dual_ccci/
obj-y   += btcvsd/
obj-y   += mmc-host/
obj-y   += jpeg/

obj-y   += rtc/
obj-y   += wdt/
obj-y   += devinfo/
obj-y   += devapc/
obj-y   += auxadc/
obj-y   += ldvt/
obj-y   += videocodec/
obj-y   += mjc/
obj-y   += masp/
obj-y   += thermal/
obj-y   += cpu_stress/
ifeq ($(HAVE_CMMB_FEATURE),yes)
obj-m += cmmbdrv/
endif
#obj-y   += nand/
#obj-m   += mdrvb/
#obj-y   += touchpanel/
#obj-y   += ccci/
#
#obj-y   += rtc/
#
#obj-y   += jpeg/
#obj-y   += gdma/
#
endif
ifneq   ($(MTK_GPU_SUPPORT),no)
obj-m   += gpu/
endif
obj-y	+= $(call all-subdirs-with-makefile,met)
