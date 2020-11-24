include $(TOPDIR)/rules.mk

PKG_NAME:=igmpproxy
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


ifeq ($(CONFIG_TARGET_ramips_mt7620),y)
	TARGET_CFLAGS += -DCONFIG_RALINK_MT7620=y
	MAKE_FLAGS += CONFIG_RALINK_MT7620=y LINUX_DIR=$(LINUX_DIR)
endif

ifeq ($(CONFIG_TARGET_ramips_mt7621),y)
	TARGET_CFLAGS += -DCONFIG_RALINK_MT7621=y
	MAKE_FLAGS += CONFIG_RALINK_MT7621=y LINUX_DIR=$(LINUX_DIR)
endif

ifeq ($(CONFIG_TARGET_ramips_mt7628),y)
	TARGET_CFLAGS += -DCONFIG_RALINK_MT7628=y
	MAKE_FLAGS += CONFIG_RALINK_MT7628=y LINUX_DIR=$(LINUX_DIR)
endif


define Package/igmpproxy
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=igmpproxy
  SUBMENU:=Applications
endef

define Package/igmpproxy/description
  igmpproxy
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))	

define Build/Configure
endef

define Package/igmpproxy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/igmpproxy $(1)/usr/bin
	$(INSTALL_BIN) ./src/igmpproxy.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,igmpproxy))
