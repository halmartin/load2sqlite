#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=load2sqlite
PKG_VERSION:=1.0.1
PKG_RELEASE:=5
PKG_MAINTAINER:=Hal Martin <hal.martin@gmail.com>
PKG_LICENSE:=GPL-2
PKG_CONFIG_DEPENDS:=libsqlite3 libconfig

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

TARGET_LDFLAGS+= \
  -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
  -Wl,-rpath-link=$(STAGING_DIR)/usr/lib/libconfig/lib \
  -Wl,-rpath-link=$(STAGING_DIR)/usr/lib/sqlite/lib

define Package/load2sqlite
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libsqlite3 +libconfig
  TITLE:=SQLite example program, creates or opens a user defined SQLite database
  URL:=https://github.com/halmartin/load2sqlite
  MENU:=1
endef

define Package/load2sqlite/description
 Example SQLite is a sample program built using libsqlite3 and libconfig
 which creates or opens a user-defined SQLite3 database and performs some
 simple verification checks on the file to ensure that the target table (readings)
 exists, and if not creates the table, then inserts a row with the current system
 time, and the load (1 minute, 5 minute, 15 minute).
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/load2sqlite/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/load2sqlite $(1)/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/load2sqlite.conf $(1)/etc/config/load2sqlite
endef

$(eval $(call BuildPackage,load2sqlite))

