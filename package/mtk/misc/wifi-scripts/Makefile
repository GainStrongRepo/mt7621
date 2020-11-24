#
# Copyright (C) 2016 MediaTek
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=wifi-l1profile
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/wifi_services
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  DEPENDS:=+wifi-l1profile
  TITLE:=wifi services
  SUBMENU:=Drivers
endef


define Package/wifi-l1profile
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  DEPENDS:=+wifi-kconfig-hack
  TITLE:=Config l1profile.dat
  VERSION:=$(PKG_RELEASE)-$(REVISION)
  MENU:=1
endef

define Package/wifi-l1profile/description
  This package helps to build l1profile on demand.
endef

define Package/wifi-l1profile/config
	if PACKAGE_wifi-l1profile
	source "$(SOURCE)/l1profile.config.in"
	endif
endef


define Package/wifi-kconfig-hack
  HIDDEN:=1
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Misc
  DEPENDS:=
  TITLE:=Kconfig Hack
  VERSION:=1
endef

define Package/wifi-kconfig-hack/config
	if PACKAGE_wifi-kconfig-hack
	source "$(SOURCE)/wifi-kconfig-hack.in"
	endif
endef


define Build/Prepare
endef

define Build/Compile
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	python ./make-l1profile.py $(TOPDIR)/.config $(PKG_BUILD_DIR)/.l1profile.dat.tmp
endef

define Package/wifi_services/install
	$(INSTALL_DIR) $(1)/lib/wifi/
ifeq ($(CONFIG_WIFI_SCRIPT_LUA),y)
	$(INSTALL_BIN) ./files/wifi_services.lua $(1)/lib/wifi/
else
	$(INSTALL_BIN) ./files/wifi_services_sh.lua $(1)/lib/wifi/
endif
endef

define Package/wifi-l1profile/install
	$(INSTALL_DIR) $(1)/etc/wireless/
	python ./make-l1profile.py $(TOPDIR)/.config $(1)/etc/wireless/l1profile.dat
endef

$(eval $(call BuildPackage,wifi-kconfig-hack))
$(eval $(call BuildPackage,wifi-l1profile))
$(eval $(call BuildPackage,wifi_services))
