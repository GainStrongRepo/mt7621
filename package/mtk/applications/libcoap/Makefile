#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcoap
PKG_VERSION:=2018-03-12
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/obgm/libcoap.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=a674334afc29ecc0ca6d8f3418f4384a2b446198
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/libcoap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A CoAP (RFC 7252) implementation in C
  URL:=https://libcoap.net/
endef

define Package/libcoap/description
	libcoap implements a lightweight application-protocol for devices that are
	constrained their resources such as computing power, RF range,
	memory, bandwith, or network packet sizes. This protocol, CoAP was
	standardized in the IETF as RFC 7252.
endef

CONFIGURE_ARGS += \
	--disable-examples \
	--disable-dtls

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) -a $(PKG_INSTALL_DIR)/usr/include/coap $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcoap-1.{a,so*} $(1)/usr/lib/
endef

define Package/libcoap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcoap-1.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcoap))
