#
# jie.shen@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=1905daemon
PKG_REVISION:=25018687
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7621 RALINK_MT7628 MACH_MT7623 GE1_SGMII_FORCE_2500 P5_RGMII_TO_MT7530_MODE
MAKE_FLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$c)))
	
TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c))) 	
	
TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT

ifeq ($(CONFIG_LINUX_4_4),y)
TARGET_CFLAGS += \
	-I$(LINUX_DIR)/drivers/net/ethernet/raeth 
else
TARGET_CFLAGS +=  \
	-I$(LINUX_DIR)/drivers/net/raeth
endif

MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y

define Package/1905daemon
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=1905 daemon
  SUBMENU:=Applications
  DEPENDS:=+wapp
endef

define Package/1905daemon/description
  wifi app daemon
endef


define Package/1905daemon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/p1905_managerd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/1905ctrl $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/map_cfg.txt $(1)/etc
endef

$(eval $(call BuildPackage,1905daemon))

