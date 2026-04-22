# Paths
BR_DIR          := $(CURDIR)/buildroot
EXTERNAL_DIR    := $(CURDIR)/hcsr04-driver

DEFCONFIG       := stm32mp157c_dk2_defconfig
THREADS         := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu)

# Detection of Qt6 path from Brew (MacOS)
QT6_PATH        := $(shell brew --prefix qt)

.PHONY: all config build clean help ihm-config ihm-build

all: help

# --- Buildroot targets (Target: STM32) ---
config:
	@echo "--- Configuring Buildroot ---"
	$(MAKE) -C $(BR_DIR) BR2_EXTERNAL=$(EXTERNAL_DIR) $(DEFCONFIG)

build: config
	@echo "--- Starting Buildroot compilation ---"
	$(MAKE) -C $(BR_DIR) -j$(THREADS)

# --- Global targets ---
clean:
	@echo "--- Cleaning everything ---"
	$(MAKE) -C $(BR_DIR) clean 2>/dev/null || true
	rm -rf $(IHM_BUILD_DIR)

help:
	@echo "Available commands:"
	@echo "  make config    : Configure Buildroot"
	@echo "  make build     : Start full Buildroot compilation"
	@echo "  make clean     : Clean everything"