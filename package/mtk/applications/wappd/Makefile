#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wappd
PKG_REVISION:=93098478
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y

define Package/wapp
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=wapp daemon
  SUBMENU:=Applications
  DEPENDS:= +libpthread +librt
endef

define Package/wapp/description
  wifi app daemon
endef


#MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)"

define Package/wapp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wappctrl $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wapp $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wapp_openwrt.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_config_save.lua $(1)/usr/bin/wifi_config_save
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mbo_nr.sh $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc
	cp -r $(PKG_BUILD_DIR)/config_and_icon_files/* $(1)/etc/
endef

$(eval $(call BuildPackage,wapp))

