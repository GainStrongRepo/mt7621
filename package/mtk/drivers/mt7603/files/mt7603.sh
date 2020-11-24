#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7603_up() {
	echo "mt7603_up($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_up ${devname} "mt7603"
}


mt7603_down() {
	echo "mt7603_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_down ${devname} "mt7603"
}


mt7603_reload() {
	echo "mt7603_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_reload ${devname} "mt7603"
}


mt7603_restart() {
	echo "mt7603_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7603"
}

mt7603_detect() {
	if [ -e /etc/config/wireless ]; then
		uci2dat -l /etc/config/wireless -u /etc/wireless/l1profile.dat
	else
		dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7603
	fi
}


mt7603_status() {
	iwconfig
}

mt7603_hello() {
	echo "mt7603_hello($@)"
}

#=====================================================

cmd=$1
dev=$2

if [ "${cmd}" != "" ]; then
	mt7603_${cmd} $dev;
fi

