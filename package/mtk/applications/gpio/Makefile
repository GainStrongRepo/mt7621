#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gpio
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/gpio
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=Command to config gpio (ramips)
  DEPENDS:=@TARGET_ramips
  SUBMENU:=Applications
endef

define Package/gpio/description
  An program to config gpio.
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
	-I$(LINUX_DIR)/arch/mips/include \
	-I$(LINUX_DIR)/include \
	-I$(LINUX_DIR)/drivers/char

define Build/Configure
endef

define Package/gpio/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpio $(1)/usr/bin
endef


$(eval $(call BuildPackage,gpio))

