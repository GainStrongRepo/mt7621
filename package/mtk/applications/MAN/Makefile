#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ManDaemon
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/ManDaemon
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=ManDaemon
  SUBMENU:=Applications
  DEPENDS:=+libpthread +librt
endef

define Package/ManDaemon/description
  MAN program.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Package/ManDaemon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ManDaemon $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_ez $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN)  ./files/etc/init.d/man $(1)/etc/init.d/
endef


$(eval $(call BuildPackage,ManDaemon))

