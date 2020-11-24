#
# Jing Chang@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=RoamApp
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7621 RALINK_MT7628
PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/RoamApp
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=RoamApp Daemon
  SUBMENU:=Applications
  DEPENDS:= +libpthread +librt
endef

define Package/air_monitor/description
  This is a sample daemon for test and verify function for Roam in Driver.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

#TARGET_CFLAGS += \
#	CFLAGS="$(TARGET_CFLAGS)"

define Build/Configure
endef

define Package/RoamApp/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/RoamApp $(1)/bin/RoamApp
endef


$(eval $(call BuildPackage,RoamApp))

