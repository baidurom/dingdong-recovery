############same as android###############

DD_PRODUCTS := $(shell cat patch_device/devices.list)
DD_RECOVERY_VERSION := 1.0.0

.PHONY: all
all: 
	make $(DD_PRODUCTS)

.PHONY: usage
usage:
	@echo " "
	@echo "Now support the following target:"
	@echo "$(DD_PRODUCTS)"
	@echo " "
	@echo "NOTE:"
	@echo ". build/envsetup.sh   must be executed at the beginning."
	@echo "lunch                 maybe executed if needed, full_maguro-userdebug defaulted."
	@echo " "
	@echo "USAGE:"
	@echo "  make [target]       for making signal target, as, make maguro."
	@echo "  make all            for making all support targets."
	@echo "  make release        for making all support targets at out/realese/."
	@echo "  make clean          for cleaning everything that build out."

##include main.mk ,load make system#######
include build/core/main.mk
##########################################
.PHONY:release
release:
	make clean;
	make recoveryimage -j4;
	$(foreach product, $(DD_PRODUCTS),make $(product)_release;)

