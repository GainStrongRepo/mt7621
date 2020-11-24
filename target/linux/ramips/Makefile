#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ramips
BOARDNAME:=MTK/Ralink APSoC (MIPS)
SUBTARGETS:=mt7620 mt7621 mt7628
FEATURES:=squashfs gpio

LINUX_VERSION:=3.10.108
KERNEL_REVISION:=58e72ec3

ifeq ($(MTK_RELEASE_VERSION),trunk) 
	MTK_PROJECT_DIR:=$(TOPDIR)/..
	MTK_GATEWAY_APP_DIR:=$(MTK_PROJECT_DIR)/app/gateway
	MTK_WLAN_APP_DIR:=$(MTK_PROJECT_DIR)/app/wlan_daemon
	MTK_WLAN_TOOL_DIR:=$(MTK_PROJECT_DIR)/app/wlan_tool
	MTK_LIB_DIR:=$(MTK_PROJECT_DIR)/lib
endif 

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += 

define Target/Description
	Build firmware images for MTK/Ralink APSoC based boards.
endef

$(eval $(call BuildTarget))
