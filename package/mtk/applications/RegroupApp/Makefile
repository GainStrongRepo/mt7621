#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=RegroupApp
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/RegroupApp
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=RegroupApp
  SUBMENU:=Applications
  DEPENDS:=+libpthread +librt
endef

define Package/RegroupApp/description
  MAN program.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Package/RegroupApp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/RegroupApp $(1)/usr/bin
endef


$(eval $(call BuildPackage,RegroupApp))

