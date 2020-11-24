#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mii_mgr
PKG_REVISION:=ef06bdb2
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.gz
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/mii_mgr
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=mii_mgr
  SUBMENU:=Applications
endef

define Package/mii_mgr/description
  An mdio read/write program.
endef


ifeq ($(CONFIG_TARGET_ramips_mt7620),y)
TARGET_CFLAGS += -DCONFIG_RALINK_MT7620=y
endif

ifeq ($(CONFIG_TARGET_ramips_mt7621),y)
TARGET_CFLAGS += -DCONFIG_RALINK_MT7621=y
endif

ifeq ($(CONFIG_TARGET_ramips_mt7628),y)
TARGET_CFLAGS += -DCONFIG_RALINK_MT7628=y
endif

TARGET_CFLAGS += -I$(LINUX_DIR)/drivers/net/raeth
TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT

define Build/Configure
endef

define Package/mii_mgr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mii_mgr $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mii_mgr_cl45 $(1)/usr/bin
endef


$(eval $(call BuildPackage,mii_mgr))

