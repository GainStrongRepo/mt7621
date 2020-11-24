#
# 2018 Hua Shao <hua.shao@mediatek.com>
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uci2dat
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

define Package/uci2dat
	SECTION:=MTK Properties
	CATEGORY:=MTK Properties
	DEPENDS:=+libuci
	TITLE:=Translate uci config into ralink wifi dat format
	SUBMENU:=Applications
  MENU:=1
endef

define Package/uci2dat/description
	Translate uci config into ralink wifi dat format
endef

define Package/uci2dat/config
	if PACKAGE_uci2dat
		config UCI2DAT_LUA
			bool "Use lua implementation instead of C"
			default y
	endif
endef

define Build/Compile/Lua
	@echo "will use lua uci2dat, no need to build c code.";
	@echo "will use lua uci2dat, no need to build c code.";
	@echo "will use lua uci2dat, no need to build c code.";
	@echo "will use lua uci2dat, no need to build c code.";
	@echo "will use lua uci2dat, no need to build c code.";
endef

define Build/Compile/C
	$(call Build/Compile/Default)
endef

ifdef CONFIG_UCI2DAT_LUA
Build/Compile = $(Build/Compile/Lua)
else
Build/Compile = $(Build/Compile/C)
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -luci -Wall
TARGET_CFLAGS += -Wno-error=format-security 


define Package/uci2dat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib/lua
	if [ "$$(CONFIG_UCI2DAT_LUA)" = "y" ]; then \
	$(INSTALL_BIN) ./files/uci2dat.lua $(1)/usr/bin/uci2dat; \
	$(LN) uci2dat $(1)/usr/bin/dat2uci; \
	$(LN) uci2dat $(1)/usr/bin/datdiff; \
	$(LN) uci2dat $(1)/usr/bin/l1prof; \
	$(INSTALL_BIN) ./files/shuci.lua $(1)/usr/lib/lua/shuci.lua; \
	else \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/uci2dat $(1)/usr/bin; \
	fi
endef


$(eval $(call BuildPackage,uci2dat))

