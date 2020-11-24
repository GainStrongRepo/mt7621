#
# Guangbin.Zhong@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=andlink
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)



include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/andlink
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:= AndLink Daemon
  DEPENDS:=+libpthread +libcoap +libnvram +libopenssl
  SUBMENU:=Applications
endef

define Package/andlink/description
   AndLink Daemon
endef


define Build/Compile
  echo "compile"
endef

define Build/Configure
endef

define Package/andlink/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/andlink
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/andlink $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/files/andlink.conf $(1)/etc/andlink
	$(CP) $(PKG_BUILD_DIR)/files/fw_upgrade.sh $(1)/usr/sbin
endef


$(eval $(call BuildPackage,andlink))

