# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7603
PKG_VERSION:=gerrit
PKG_SOURCE:=MT7603e_LinuxAP_V4.1.2.1_20190503-ac1d3fab.tar.bz2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7603
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  MENU:=1
  TITLE:=MT7603 wifi driver
  DEPENDS:=+mt7603-scripts +wifi-l1profile +wifi_services
endef

define KernelPackage/mt7603-in
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7603e BUILD_IN
  DEPENDS:=+mt7603-scripts +wifi-l1profile +mtk-base-files +wifi_services
  FILES:=
  AUTOLOAD:=
  KCONFIG:= \
    CONFIG_WIFI_MT7603E=y \
    CONFIG_MT7603E_RALINK_MT7603E=y \
    CONFIG_MT7603E_RLT_WIFI=y \
    CONFIG_MT7603E_MT_MAC=y \
    CONFIG_MT7603E_RLT_AP_SUPPORT=y
endef

define KernelPackage/mt7603-ko
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7603e BUILD_KO
  DEPENDS:=+mt7603-scripts
  FILES:=$(LINUX_DIR)/drivers/net/wireless/mtk/mt7603/mt7603_wifi_ap/mt7603.ko
  AUTOLOAD:=$(call AutoProbe,mt7603)
  KCONFIG:= \
    CONFIG_WIFI_MT7603E=y \
    CONFIG_MT7603E_RALINK_MT7603E=y \
    CONFIG_MT7603E_RLT_WIFI=y \
    CONFIG_MT7603E_MT_MAC=y \
    CONFIG_MT7603E_RLT_AP_SUPPORT=m
endef

define KernelPackage/mt7603/config
	if PACKAGE_kmod-mt7603
	source "$(SOURCE)/config.in"
	endif
endef

define Build/Prepare
	@echo "this package is built with kernel"
endef

define Build/Compile
	@echo "this package is built with kernel"
endef

define Package/mt7603-scripts
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=mt7603 scripts
  DEPENDS:=+mtk-base-files
  HIDDEN:=1
endef

define Package/mt7603-scripts/install
	$(INSTALL_DIR) $(1)/lib/wifi/
	-if [ "$$(CONFIG_WIFI_SCRIPT_LUA)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7603.lua $(1)/lib/wifi/; \
	else \
	$(INSTALL_BIN) ./files/mt7603.sh $(1)/lib/wifi/; \
	fi
	$(INSTALL_DIR) $(1)/etc/wireless/mt7603/
	if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7603.map.dat $(1)/etc/wireless/mt7603/mt7603.dat; \
	else \
	$(INSTALL_BIN) ./files/mt7603.dat $(1)/etc/wireless/mt7603/mt7603.dat; \
	fi
	-$(INSTALL_BIN) ./files/mt7603.eeprom.bin $(1)/etc/wireless/mt7603/
	echo $(PKG_VERSION) > $(1)/etc/wireless/mt7603/version
endef

define Package/mt7603-scripts/conffiles
/etc/wireless/mt7603/
endef

$(eval $(call KernelPackage,mt7603))
$(eval $(call KernelPackage,mt7603-ko))
$(eval $(call KernelPackage,mt7603-in))
$(eval $(call BuildPackage,mt7603-scripts))
