#
# dragon@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kv_tst_app
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7621 RALINK_MT7628
PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/kv_tst_app
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=11kv test app
  SUBMENU:=Applications
  DEPENDS:= +libpthread +librt
endef

define Package/kv_tst_app/description
  Band Steering  Daemon For support MT7615 New Band Steering
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))

MAKE_FLAGS += LINUX_DIR=$(LINUX_DIR)

#MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)"

define Package/kv_tst_app/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kv $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kvctrl $(1)/bin
endef


$(eval $(call BuildPackage,kv_tst_app))

