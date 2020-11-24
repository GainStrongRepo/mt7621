#
# Copyright (C) 2018 Hua Shao <hua.shao@mediatek.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

default: prepare

prepare:
	@echo =========================== $@
	python $(TOPDIR)/scripts/mtkwifi.py --linuxdir=$(LINUX_DIR) --topdir=$(TOPDIR) clean prepare

compile:
	@echo =========================== $@
	python $(TOPDIR)/scripts/mtkwifi.py --linuxdir=$(LINUX_DIR) --topdir=$(TOPDIR) clean prepare

install:
	@echo =========================== $@
	python $(TOPDIR)/scripts/mtkwifi.py --linuxdir=$(LINUX_DIR) --topdir=$(TOPDIR) clean prepare

clean: FORCE
	@echo =========================== $@
	python $(TOPDIR)/scripts/mtkwifi.py --linuxdir=$(LINUX_DIR) --topdir=$(TOPDIR) clean

.PHONY: all clean prepare compile install
