# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7615
PKG_VERSION:=gerrit
PKG_SOURCE:=MT7622_7615_SoftAP_5.0.4.0_bb5ba3_20190503.tar.xz

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7615
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7615 wifi driver
  DEPENDS:=+mt7615-scripts +wifi-l1profile +mtk-base-files +wifi_services
  MENU:=1
endef

define KernelPackage/mt7615-in
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7615e BUILD_IN
  DEPENDS:=+mt7615-scripts
  FILES:=
  AUTOLOAD:=
  KCONFIG:= \
    CONFIG_WIFI_MT7615E=y \
    CONFIG_MT7615E_CHIP_MT7615E=y \
    CONFIG_MT7615E_MT_WIFI=y \
    CONFIG_MT7615E_MT_AP_SUPPORT=y \
    CONFIG_MT7615E_WIFI_MODE_AP=y
endef

define KernelPackage/mt7615-ko
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7615e BUILD_KO
  DEPENDS:=+mt7615-scripts
  FILES:=$(LINUX_DIR)/drivers/net/wireless/mtk/mt7615/mt_wifi_ap/mt_wifi.ko
  AUTOLOAD:=$(call AutoProbe,mt7615)
  KCONFIG:= \
    CONFIG_WIFI_MT7615E=y \
    CONFIG_MT7615E_CHIP_MT7615E=y \
    CONFIG_MT7615E_MT_WIFI=m \
    CONFIG_MT7615E_MT_AP_SUPPORT=m \
    CONFIG_MT7615E_WIFI_MODE_AP=m
endef

define KernelPackage/mt7615/config
	if PACKAGE_kmod-mt7615
	source "$(SOURCE)/config.in"
	endif
endef

define Package/mt7615-scripts
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=mt7615 scripts
  DEPENDS:=+mtk-base-files
  HIDDEN:=1
endef

define Build/Prepare
	@echo "this package is built with kernel"
endef

define Build/Compile
	@echo "this package is built with kernel"
endef

define Package/mt7615-scripts/install
	$(INSTALL_DIR) $(1)/lib/wifi/
	$(INSTALL_DIR) $(1)/etc/wireless/mt7615/
	$(INSTALL_BIN) ./files/mt7615-sku.dat $(1)/etc/wireless/mt7615/
	$(INSTALL_BIN) ./files/mt7615-sku-bf.dat $(1)/etc/wireless/mt7615/
	echo $(PKG_VERSION) > $(1)/etc/wireless/mt7615/version
	-if [ "$$(CONFIG_WIFI_SCRIPT_LUA)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7615.lua $(1)/lib/wifi/; \
	else \
	$(INSTALL_BIN) ./files/mt7615.sh $(1)/lib/wifi/; \
	fi

	if [ "$$(CONFIG_KCONFIG_FIRST_IF_MT7615E)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615*.bin $(1)/etc/wireless/mt7615/; \
	fi
	if [ "$$(CONFIG_FIRST_MT7615E_NORMAL)" = "y" ]; then \
		if [ "$$(CONFIG_KCONFIG_SECOND_IF_MT7603E)" = "y" ]; then \
			if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
			$(INSTALL_BIN) ./files/mt7615.2.map.dat $(1)/etc/wireless/mt7615/mt7615.2.dat; \
			else \
			$(INSTALL_BIN) ./files/mt7615.2.dat $(1)/etc/wireless/mt7615/; \
			fi \
		else \
			if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
			$(INSTALL_BIN) ./files/mt7615.1.map.dat $(1)/etc/wireless/mt7615/mt7615.1.dat; \
			else \
			$(INSTALL_BIN) ./files/mt7615.1.dat $(1)/etc/wireless/mt7615/; \
			fi \
		fi \
	fi
	if [ "$$(CONFIG_FIRST_MT7615E_DBDC)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615.1.2G.map.dat $(1)/etc/wireless/mt7615/mt7615.1.2G.dat; \
		$(INSTALL_BIN) ./files/mt7615.1.5G.map.dat $(1)/etc/wireless/mt7615/mt7615.1.5G.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7615.1.2G.dat $(1)/etc/wireless/mt7615/; \
		$(INSTALL_BIN) ./files/mt7615.1.5G.dat $(1)/etc/wireless/mt7615/; \
		fi \
	fi

	if [ "$$(CONFIG_KCONFIG_SECOND_IF_MT7615E)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615*.bin $(1)/etc/wireless/mt7615/; \
	fi
	if [ "$$(CONFIG_SECOND_MT7615E_NORMAL)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615.2.map.dat $(1)/etc/wireless/mt7615/mt7615.2.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7615.2.dat $(1)/etc/wireless/mt7615/; \
		fi \
	fi
	if [ "$$(CONFIG_SECOND_MT7615E_DBDC)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615.2.2G.map.dat $(1)/etc/wireless/mt7615/mt7615.2.2G.dat ; \
		$(INSTALL_BIN) ./files/mt7615.2.5G.map.dat $(1)/etc/wireless/mt7615/mt7615.2.5G.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7615.2.2G.dat $(1)/etc/wireless/mt7615/; \
		$(INSTALL_BIN) ./files/mt7615.2.5G.dat $(1)/etc/wireless/mt7615/; \
		fi \
	fi

	if [ "$$(CONFIG_KCONFIG_THIRD_IF_MT7615E)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615*.bin $(1)/etc/wireless/mt7615/; \
	fi
	if [ "$$(CONFIG_THIRD_MT7615E_NORMAL)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615.3.map.dat $(1)/etc/wireless/mt7615/mt7615.3.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7615.3.dat $(1)/etc/wireless/mt7615/; \
		fi \
	fi
	if [ "$$(CONFIG_THIRD_MT7615E_DBDC)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7615.3.2G.map.dat $(1)/etc/wireless/mt7615/mt7615.3.2G.dat; \
		$(INSTALL_BIN) ./files/mt7615.3.5G.map.dat $(1)/etc/wireless/mt7615/mt7615.3.5G.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7615.3.2G.dat $(1)/etc/wireless/mt7615/; \
		$(INSTALL_BIN) ./files/mt7615.3.5G.dat $(1)/etc/wireless/mt7615/; \
		fi \
	fi
endef

define Package/mt7615-scripts/conffiles
/etc/wireless/mt7615/
endef

$(eval $(call KernelPackage,mt7615))
$(eval $(call KernelPackage,mt7615-ko))
$(eval $(call KernelPackage,mt7615-in))
$(eval $(call BuildPackage,mt7615-scripts))
