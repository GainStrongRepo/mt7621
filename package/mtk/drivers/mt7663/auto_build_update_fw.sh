if [ -d mt_wifi ]; then
    echo "7613 mt_wifi autobuild update fw"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7663
    RT28xx_MODE=AP
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    make -C mt_wifi/embedded build_tools
    ./mt_wifi/embedded/tools/bin2h
    make -C mt_wifi/embedded build_sku_tables
    ./mt_wifi/txpwr/dat2h
else
    exit 1
fi
