obj-  := dummy.o

ifeq ($(CONFIG_THERMAL), y)
else
    obj-y += mtk_thermal_dummy.o
endif

obj-$(CONFIG_THERMAL) += mtk_thermal_platform.o
obj-$(CONFIG_THERMAL) += mtk_ts_pa_thput.o
#obj-$(CONFIG_THERMAL) += mtk_ts_skin.o

ifneq ($(CONFIG_MTK_COMBO_WIFI),)
    obj-$(CONFIG_THERMAL) += mtk_ts_wmt.o
endif

# Only included in non-EVB projects
ifeq ($(CONFIG_POWER_EXT), y)
else
    ifeq ($(CONFIG_MTK_SMART_BATTERY), y)
        # Always align the Makefile in mediatek/platform/mt6592/kernel/drivers/power/
        ifeq ($(MTK_NCP1851_SUPPORT),yes)
            obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
        else
            ifeq ($(MTK_BQ24196_SUPPORT),yes)
                obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
            else
                # FAN5405 or PMIC6320 pulse charging or new charger IC
                obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
            endif
        endif
    endif
endif

obj-y += mtk_ts_cpu.o
obj-$(CONFIG_THERMAL) += mtk_ts_pa.o
obj-$(CONFIG_THERMAL) += mtk_ts_battery.o
obj-$(CONFIG_THERMAL) += mtk_ts_pmic.o
#obj-$(CONFIG_THERMAL) += mtk_ts_battery2.o
obj-$(CONFIG_THERMAL) += mtk_ts_AP.o
#obj-$(CONFIG_THERMAL) += mtk_ts_xtal.o
obj-$(CONFIG_THERMAL) += mtk_ts_6333buck.o

ifeq ($(CONFIG_THERMAL),y)
    obj-y += mtk_ts_abb.o
endif


ifeq ($(MTK_TMP103_SUPPORT),yes)
#obj-$(CONFIG_THERMAL) += mtk_ts_pcb.o
endif
