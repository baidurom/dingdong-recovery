include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).a

COBJS := mt6575.o \
	mt6575_board.o \
	mt65xx_leds.o \
	boot_mode.o \
	load_image.o \
	addr_trans.o \
	mmc_core.o \
	mmc_test.o \
	msdc_utils.o \
	msdc.o \
	mtk_serial.o \
	mtk_timer.o \
	mtk_nand.o \
	mt6575_meta.o \
	mtk_key.o \
	mt_rtc.o \
	mt6575_recovery.o \
	mtk_factory.o \
	mt6575_auxadc.o\
	mtk_wdt.o\
	mt6575_gpio.o \
	mt6575_gpio_init.o \
	mt6575_pwm.o \
	mt6589_i2c.o \
	mt65xx_disp_drv.o \
	disp_drv.o \
 	disp_assert_layer.o \
	disp_drv_dbi.o \
   	lcd_drv.o \
  	disp_drv_dsi.o \
 	dsi_drv.o \
	disp_drv_dpi.o \
 	dpi_drv.o \
    mt65xx_logo.o \
    mt6589_pll.o \
    mt6589_sleep.o \
  bmt.o \
  partition_mt.o \
  irq.o \
  mt_pmic_wrap_uboot.o \
  mtk_pmic_6320.o

###	custom/$(MTK_PROJECT)/mt65xx_partition.o \
			
SOBJS	:= lowlevel_init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
