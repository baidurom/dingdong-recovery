###################################################################
# flag
###################################################################

MOD_SRC  = 
MOD_OBJ  = $(D_ROOT)/out

###################################################################
# source files
# - add your file here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
###################################################################

MOD_SRC  := \
	circbuf.c \
	dramc_dqs_gw.c  \
	dramc_dle_cal.c \
	dramc_calib.c \
	mmc_common_inter.c \
	mmc_core.c \
	mmc_test.c \
	msdc.c \
	msdc_utils.c \
	mt_i2c.c \
	mtk_key.c \
	mt6577_pdn.c \
	mt_usbd.c \
	mt_usbphy.c \
	mt_usbtty.c \
	mt_emi.c \
	mt6577_cpu_power.c \
	mtk_timer.c \
	mtk_wdt.c \
	partition_mt.c \
	platform.c \
	mtk_pll.c \
	device_apc.c \
	gpio.c \
	uart.c \
 	mt_pmic_wrap_init.c \
	mtk_pmic_6320.c \
	mt_rtc.c \

ifneq ("$(MTK_EMMC_SUPPORT)","yes")
	MOD_SRC  += 	mtk_nand.c 
	MOD_SRC  +=	bmt.c
endif
 	
ifeq ("$(MTK_MT8193_SUPPORT)","yes")
  MOD_SRC  += mt8193_init.c
	MOD_SRC  += mt8193_ckgen.c
	MOD_SRC  += mt8193_i2c.c
endif

###################################################################
# target
###################################################################

include $(COMMON_FILE_MK)
