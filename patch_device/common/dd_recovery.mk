ifneq ($(DD_PRODUCT), )
ifneq ($(DD_KERNEL), )
dd_recovery_out := out/patch_device
dd_recovery_product := $(dd_recovery_out)/$(DD_PRODUCT)
dd_recovery_ramdisk := $(dd_recovery_product)/ramdisk-recovery.img
dd_recovery_target := $(dd_recovery_product)/recovery.img
dd_recovery_binary := $(call intermediates-dir-for,EXECUTABLES,recovery)/recovery
dd_recovery_sbin := $(TARGET_ROOT_OUT)/sbin
dd_recovery_resource := src/res
dd_recovery_root := $(dd_recovery_product)/root
dd_recovery_icon := $(dd_recovery_root)/res/icons
dd_recovery_icon_mdpi := $(dd_recovery_root)/res/icons-mdpi
dd_recovery_icon_hdpi := $(dd_recovery_root)/res/icons-hdpi
dd_recovery_icon_xhdpi := $(dd_recovery_root)/res/icons-xhdpi
dd_recovery_icon_xxhdpi := $(dd_recovery_root)/res/icons-xxhdpi

ifneq ($(DD_DEVICE_SCREEN_TYPE),)
	dd_recovery_screen_type := $(subst ",,$(DD_DEVICE_SCREEN_TYPE))
else
	dd_recovery_screen_type := HDPI
endif

dd_recoveryimage_args := \
	--kernel $(DD_KERNEL) \
	--ramdisk $(dd_recovery_ramdisk)

ifneq ($(DD_KERNEL_CMDLINE),)
	dd_recoveryimage_args += --cmdline "$(DD_KERNEL_CMDLINE)"
endif

ifneq ($(DD_KERNEL_BASE),)
	dd_recoveryimage_args += --base $(DD_KERNEL_BASE)
endif

ifneq ($(DD_KERNEL_PAGESIZE),)
	dd_recoveryimage_args += --pagesize $(DD_KERNEL_PAGESIZE)
endif
ifneq ($(DD_BOOTRAMDISK_OFFSET),)
	dd_recoveryimage_args += --ramdisk_offset $(DD_BOOTRAMDISK_OFFSET)
endif
ifneq ($(DD_RAMDISK_OFFSET),)
	dd_recoveryimage_args += --ramdisk_offset $(DD_RAMDISK_OFFSET)
endif
ifneq ($(DD_TAGS_OFFSET),)
	dd_recoveryimage_args += --tags_offset $(DD_TAGS_OFFSET)
endif
ifneq ($(DD_DTIMG),)
	dd_recoveryimage_args += --dt $(DD_DTIMG)
endif


#BUILD_RECOVERY_BEFORE := $(filter $(TARGET_ROOT_OUT)/sbin/%, $(ALL_DEFAULT_INSTALLED_MODULES))
.PHONY: $(DD_PRODUCT)
$(DD_PRODUCT): dd_recovery_out := $(dd_recovery_out)
$(DD_PRODUCT): dd_recovery_product := $(dd_recovery_product)
$(DD_PRODUCT): dd_recovery_ramdisk := $(dd_recovery_ramdisk)
$(DD_PRODUCT): dd_recovery_target := $(dd_recovery_target)
$(DD_PRODUCT): dd_recovery_binary := $(dd_recovery_binary)
$(DD_PRODUCT): dd_recovery_sbin := $(dd_recovery_sbin)
$(DD_PRODUCT): dd_recovery_resource := $(dd_recovery_resource)
$(DD_PRODUCT): dd_recovery_root := $(dd_recovery_root)
$(DD_PRODUCT): dd_recoveryimage_args := $(dd_recoveryimage_args)
$(DD_PRODUCT): dd_recovery_icon := $(dd_recovery_icon)
$(DD_PRODUCT): dd_recovery_icon_mdpi := $(dd_recovery_icon_mdpi)
$(DD_PRODUCT): dd_recovery_icon_hdpi := $(dd_recovery_icon_hdpi)
$(DD_PRODUCT): dd_recovery_icon_xhdpi := $(dd_recovery_icon_xhdpi)
$(DD_PRODUCT): dd_recovery_icon_xxhdpi := $(dd_recovery_icon_xxhdpi)
$(DD_PRODUCT): dd_recovery_screen_type := $(dd_recovery_screen_type)
$(DD_PRODUCT): DD_PRODUCT := $(DD_PRODUCT)
$(DD_PRODUCT): DD_KERNEL := $(DD_KERNEL)
$(DD_PRODUCT): DD_PRODUCT_ROOT := $(DD_PRODUCT_ROOT)
$(DD_PRODUCT): DD_DEVICE_CONFIG := $(DD_DEVICE_CONFIG)
$(DD_PRODUCT): RECOVERY_LINKS := \
	$(addprefix $(dd_recovery_root)/sbin/, busybox)
$(DD_PRODUCT): BUSYBOX_LINKS := $(shell cat src/busybox/busybox-minimal.links)
$(DD_PRODUCT): exclude := tune2fs mke2fs
$(DD_PRODUCT): RECOVERY_BUSYBOX_SYMLINKS := \
	$(addprefix $(dd_recovery_root)/sbin/,$(filter-out $(exclude),$(notdir $(BUSYBOX_LINKS))))
$(DD_PRODUCT): $(MKBOOTFS) $(MINIGZIP) \
		$(MKBOOTIMG) \
		recoveryimage
	@echo make $(DD_PRODUCT) 
	rm -rf $(dd_recovery_product)
	mkdir -p $(dd_recovery_out)
	mkdir -p $(dd_recovery_product)
	mkdir -p $(dd_recovery_root)
	mkdir -p out/release
	mkdir -p out/upload
	cp -rf $(DD_PRODUCT_ROOT) $(dd_recovery_product)/
	cp -rf $(DD_KERNEL) $(dd_recovery_product)/
	cp -rf $(dd_recovery_sbin) $(dd_recovery_root)/
	cp -f $(dd_recovery_binary) $(dd_recovery_root)/sbin/
	cp -rf $(dd_recovery_resource) $(dd_recovery_root)/
	-cp -rf $(DD_PRODUCT_ROOT)/sbin/adbd $(dd_recovery_root)/sbin/
ifneq ($(DD_DEVICE_CONFIG),)
	-cp -f $(DD_DEVICE_CONFIG) $(dd_recovery_root)/res/
endif

ifeq ($(dd_recovery_screen_type), XHDPI)
	mv $(dd_recovery_icon_xhdpi) $(dd_recovery_icon)
	rm -rf $(dd_recovery_icon_mdpi)
	rm -rf $(dd_recovery_icon_hdpi)
	rm -rf $(dd_recovery_icon_xxhdpi)
endif

ifeq ($(dd_recovery_screen_type), MDPI)
	mv $(dd_recovery_icon_mdpi) $(dd_recovery_icon)
	rm -rf $(dd_recovery_icon_hdpi)
	rm -rf $(dd_recovery_icon_xhdpi)
	rm -rf $(dd_recovery_icon_xxhdpi)
endif

ifeq ($(dd_recovery_screen_type), XXHDPI)
	mv $(dd_recovery_icon_xxhdpi) $(dd_recovery_icon)
	rm -rf $(dd_recovery_icon_mdpi)
	rm -rf $(dd_recovery_icon_hdpi)
	rm -rf $(dd_recovery_icon_xhdpi)
endif

ifneq ($(dd_recovery_screen_type), XHDPI)
ifneq ($(dd_recovery_screen_type), XXHDPI)
ifneq ($(dd_recovery_screen_type), MDPI)
	mv $(dd_recovery_icon_hdpi) $(dd_recovery_icon)
	rm -rf $(dd_recovery_icon_mdpi)
	rm -rf $(dd_recovery_icon_xhdpi)
	rm -rf $(dd_recovery_icon_xxhdpi)
endif
endif
endif

	$(hide) $(foreach item, $(RECOVERY_LINKS), \
		echo "Symlink: $(item) -> recovery"; \
		rm -f $(item) > /dev/null; \
		ln -sf recovery $(item) > /dev/null;)

	$(hide) $(foreach item, $(RECOVERY_BUSYBOX_SYMLINKS), \
		echo "Symlink: $(item) -> busybox"; \
		rm -f $(item) > /dev/null; \
		ln -sf busybox $(item) > /dev/null;)


	@echo make recovery image $(dd_recovery_target)
	$(MKBOOTFS) $(dd_recovery_root) | $(MINIGZIP) > $(dd_recovery_ramdisk)
	$(MKBOOTIMG) $(dd_recoveryimage_args) --output $(dd_recovery_target)
	$(hide) $(call assert-max-image-size, $(dd_recovery_target), $(BOARD_RECOVERYIMAGE_PARTITION_SIZE), raw)
	cp $(dd_recovery_target) out/upload/recovery-$(DD_PRODUCT).img
	cp $(dd_recovery_target) out/release/dingdongrecovery-$(DD_RECOVERY_VERSION)-$(DD_PRODUCT).img

DD_PRODUCT_RELEASE := $(DD_PRODUCT)_release
.PHONY:$(DD_PRODUCT_RELEASE)
$(DD_PRODUCT_RELEASE): dd_recovery_target := $(dd_recovery_target)
$(DD_PRODUCT_RELEASE): DD_PRODUCT := $(DD_PRODUCT)
$(DD_PRODUCT_RELEASE):$(DD_PRODUCT)
	mkdir -p out
	mkdir -p out/release
	mkdir -p out/upload
	cp $(dd_recovery_target) out/release/dingdongrecovery-$(DD_RECOVERY_VERSION)-$(DD_PRODUCT).img
	cp $(dd_recovery_target) out/upload/recovery-$(DD_PRODUCT).img
#else for DD_KERNEL is NULL
else
$(DD_PRODUCT):
	@echo do nothing in make $(DD_PRODUCT),because no DD_KERNEL
endif #end of DD_KERNEL judgement
endif #end of DD_PRODUCT judgement


