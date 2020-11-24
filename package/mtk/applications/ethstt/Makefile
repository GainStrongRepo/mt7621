#
# Copyright (C) 2010-2013 hua.shao@mediatek.com
#
# Ralink Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ethstt
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/ethstt
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=Ethernet port status
  SUBMENU:=Applications
  DEPENDS:=@TARGET_ramips +libc
endef

define Package/ethstt/description
  Ethernet port status
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
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

TARGET_CFLAGS += \
	-Wall \
	-I$(LINUX_DIR)/drivers/net/raeth


define Package/ethstt/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ethstt $(1)/usr/sbin
endef


$(eval $(call BuildPackage,ethstt))

