HCSR04_SERVER_VERSION = 1.0
HCSR04_SERVER_SITE = $(BR2_EXTERNAL_HCSR04_DRIVER_PATH)/package/hcsr04-server/src
HCSR04_SERVER_SITE_METHOD = local

# Build step: Execute the compilation process
# $(TARGET_MAKE_ENV): Sets up the environment (like PATH) to point to the cross-compilation tools
# $(MAKE): Invokes the make command
# $(TARGET_CONFIGURE_OPTS): Injects the cross-compiler variables (CC, CFLAGS, LDFLAGS, etc.)
# -C $(@D): Tells make to run inside the build directory (@D is the location where sources were extracted)
define HCSR04_SERVER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

# Installation step: Copy the executable to the target filesystem
# $(INSTALL): A cross-platform friendly version of 'cp' used by Buildroot
# -D: Create leading directories if they don't exist
# -m 0755: Set permissions to rwxr-xr-x (owner can execute, others can read/execute)
# $(@D)/hcsr04_server: The source binary located in the build directory
# $(TARGET_DIR)/usr/bin/: The destination path on the final SD card image
define HCSR04_SERVER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hcsr04_server $(TARGET_DIR)/usr/bin/hcsr04_server
endef

# Systemd integration: Install the service unit file
# /usr/lib/systemd/system/: The standard location where systemd looks for available unit files
# Buildroot will automatically enable this service by creating the necessary symlinks
define HCSR04_SERVER_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 644 $(BR2_EXTERNAL_HCSR04_DRIVER_PATH)/package/hcsr04-server/hcsr04-server.service \
		$(TARGET_DIR)/usr/lib/systemd/system/hcsr04-server.service
endef

$(eval $(generic-package))