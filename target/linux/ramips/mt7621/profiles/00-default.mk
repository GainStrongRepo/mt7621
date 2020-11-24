#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/MT7621
	NAME:=Default Profile
	PACKAGES:=\
		-swconfig -rt2x00 \
		ated hwnat regs gpio btnd switch ethstt uci2dat mii_mgr watchdog 8021xd \
		nvram qdma luci luci-app-samba luci-app-mtk mtk-base-files \
		wireless-tools block-mount fstools kmod-scsi-generic \
		kmod-usb-core kmod-usb3 kmod-usb-storage kmod-serial-8250 kmod-serial-8250-rt288x \
		kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 \
		kmod-nls-cp437 kmod-nls-cp850 kmod-nls-iso8859-1 kmod-nls-iso8859-15 kmod-nls-cp950 \
		kmod-8021q kmod-nf-sc kmod-serial-8250-rt288x kmod-mtk-gdma kmod-hw_nat
endef


define Profile/Default/Description
	Basic MT7621 SoC support
endef
$(eval $(call Profile,MT7621))
