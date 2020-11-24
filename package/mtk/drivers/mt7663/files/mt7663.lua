#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path

local function esc(x)
   return (x:gsub('%%', '%%%%')
            :gsub('^%^', '%%^')
            :gsub('%$$', '%%$')
            :gsub('%(', '%%(')
            :gsub('%)', '%%)')
            :gsub('%.', '%%.')
            :gsub('%[', '%%[')
            :gsub('%]', '%%]')
            :gsub('%*', '%%*')
            :gsub('%+', '%%+')
            :gsub('%-', '%%-')
            :gsub('%?', '%%?'))
end

function add_vif_into_lan(vif)
    local mtkwifi = require("mtkwifi")
    local brvifs = mtkwifi.__trim( mtkwifi.read_pipe("uci get network.lan.ifname"))
    if not string.match(brvifs, esc(vif)) then
        nixio.syslog("debug", "add "..vif.." into lan")
        -- brvifs = brvifs.." "..vif
        -- os.execute("uci set network.lan.ifname=\""..brvifs.."\"") -- netifd will down vif form /etc/config/network
        -- os.execute("uci commit")
        -- os.execute("ubus call network.interface.lan add_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
        os.execute("brctl addif br-lan "..vif) -- double insurance for rare failure
    end
end

function del_vif_from_lan(vif)
    local mtkwifi = require("mtkwifi")
    local brvifs = mtkwifi.__trim(mtkwifi.read_pipe("uci get network.lan.ifname"))
    if string.match(brvifs, esc(vif)) then
        -- brvifs = mtkwifi.__trim(string.gsub(brvifs, esc(vif), ""))
        nixio.syslog("debug", "del "..vif.." from lan")
        -- os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
        -- os.execute("uci commit")
        -- os.execute("ubus call network.interface.lan remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
        os.execute("brctl delif br-lan "..vif)
    end
end

function mt7663_up(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local wifi_services_exist = false
    if  mtkwifi.exists("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mt7663_up called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    assert(l1parser, "failed to parse l1profile!")
    dev = devs.devname_ridx[devname]
    if not dev then
        nixio.syslog("err", "mt7663_up: dev "..devname.." not found!")
        return
    end
    local profile = mtkwifi.search_dev_and_profile()[devname]
    local cfgs = mtkwifi.load_profile(profile)
    -- we have to bring up main_ifname first, main_ifname will create all other vifs.
    if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
        nixio.syslog("debug", "mt7663_up: ifconfig "..dev.main_ifname.." up")
        os.execute("ifconfig "..dev.main_ifname.." up")
        add_vif_into_lan(dev.main_ifname)
        if wifi_services_exist then
            miniupnpd_chk(devname, dev.main_ifname, true)
            d8021xd_chk(devname, dev.ext_ifname, dev.main_ifname, true)
        end
    else
        nixio.syslog("err", "mt7663_up: main_ifname "..dev.main_ifname.." missing, quit!")
        return
    end
    for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
    do
        if vif ~= dev.main_ifname and (
           string.match(vif, esc(dev.ext_ifname).."[0-9]+")
        or (string.match(vif, esc(dev.apcli_ifname).."[0-9]+") and
                cfgs.ApCliEnable ~= "0" and cfgs.ApCliEnable ~= "")
        -- or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
        or string.match(vif, esc(dev.mesh_ifname).."[0-9]+"))
        then
            nixio.syslog("debug", "mt7663_up: ifconfig "..vif.."0 up")
            os.execute("ifconfig "..vif.." up")
            add_vif_into_lan(vif)
            if wifi_services_exist and string.match(vif, esc(dev.ext_ifname).."[0-9]+") then
                miniupnpd_chk(devname, vif, true)
                d8021xd_chk(devname, dev.ext_ifname, vif, true)
            end
        -- else nixio.syslog("debug", "mt7663_up: skip "..vif..", prefix not match "..pre)
        end
    end
    os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=1")

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
        os.execute("/etc/init.d/man start")
    end
end

function mt7663_down(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local wifi_services_exist = false
    if  mtkwifi.exists("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mt7663_down called!")

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
    end

    local devs, l1parser = mtkwifi.__get_l1dat()
    assert(l1parser, "failed to parse l1profile!")
    dev = devs.devname_ridx[devname]
    if not dev then
        nixio.syslog("err", "mt7663_down: dev "..devname.." not found!")
        return
    end
    os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=0")
    for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
    do
        if vif == dev.main_ifname
        or string.match(vif, esc(dev.ext_ifname).."[0-9]+")
        or string.match(vif, esc(dev.apcli_ifname).."[0-9]+")
        or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
        or string.match(vif, esc(dev.mesh_ifname).."[0-9]+")
        then
            if wifi_services_exist then
                if vif == dev.main_ifname
                or string.match(vif, esc(dev.ext_ifname).."[0-9]+") then
                    miniupnpd_chk(devname, vif)
                    d8021xd_chk(devname, dev.ext_ifname, vif)
                end
            end
            nixio.syslog("debug", "mt7663_down: ifconfig "..vif.." down")
            os.execute("ifconfig "..vif.." down")
            del_vif_from_lan(vif)
        -- else nixio.syslog("debug", "mt7663_down: skip "..vif..", prefix not match "..pre)
        end
    end
end

function mt7663_reload(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local devs, l1parser = mtkwifi.__get_l1dat()
    nixio.syslog("debug", "mt7663_reload called!")
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                -- merge mapddat file to dat
                local profile = mtkwifi.search_dev_and_profile()[devname]
                local tmpdat = "/etc/map/"..string.match(profile, "([^/]+)\.dat")..".tmpdat"
                if mtkwifi.exists(tmpdat) then
                    mtkwifi.update_profile(profile, tmpdat)
                    os.remove(tmpdat)
                end
                mt7663_down(devname)
            end
        end
    else
        for devname, dev in pairs(devs.devname_ridx) do
            local profile = mtkwifi.search_dev_and_profile()[devname]
            local tmpdat = "/etc/map/"..string.match(profile, "([^/]+)\.dat")..".tmpdat"
            if mtkwifi.exists(tmpdat) then
                mtkwifi.update_profile(profile, tmpdat)
                os.remove(tmpdat)
            end
            mt7663_down(devname)
        end
    end
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mt7663_up(devname)
            end
        end
    else
       for devname, dev in pairs(devs.devname_ridx) do
           mt7663_up(devname)
       end
    end
end

function mt7663_restart(devname)
    local nixio = require("nixio")
    local uci  = require "luci.model.uci".cursor()
    local mtkwifi = require("mtkwifi")
    local devs, l1parser = mtkwifi.__get_l1dat()
    local hwnat_en = uci:get("hwnat","global","enabled")
    nixio.syslog("debug", "mt7663_restart called!")

    --if wifi driver is built-in, it's necessary action to reboot the device
    if mtkwifi.exists("/sys/module/mt7663") == false then
        os.execute("echo reboot_required > /tmp/mtk/wifi/reboot_required")
        return
    end

    -- All interface should be down when we rmmod the mt_wifi.ko
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                 mt7663_down(devname)
            end
        end
    else
         for devname, dev in pairs(devs.devname_ridx) do
             mt7663_down(devname)
         end
    end
    os.execute("rmmod wifi_forward")
    os.execute("rmmod mt7663")
    if hwnat_en then
        os.execute("rmmod hw_nat")
        os.execute("insmod /lib/modules/ralink/hw_nat.ko")
    end
    os.execute("modprobe mt7663")
    os.execute("modprobe wifi_forward")
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mt7663_up(devname)
            end
        end
    else
        for devname, dev in pairs(devs.devname_ridx) do
            mt7663_up(devname)
        end
    end
end

function mt7663_reset(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7663_reset called!")
    if mtkwifi.exists("/rom/etc/wireless/mt7663/") then
        os.execute("rm -rf /etc/wireless/mt7663/")
        os.execute("cp -rf /rom/etc/wireless/mt7663/ /etc/wireless/")
        mt7663_reload(devname)
    else
        nixio.syslog("debug", "mt7663_reset: /rom"..profile.." missing, unable to reset!")
    end
end

function mt7663_status(devname)
    return wifi_common_status()
end

function mt7663_hello(devname)
    print("hello from mt7663, devname="..devname)
end

function mt7663_detect(devname)
--[=[
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7663_detect called!")

    for _,dev in ipairs(mtkwifi.get_all_devs()) do
        local relname = string.format("%s%d%d",dev.maindev,dev.mainidx,dev.subidx)
        print([[
config wifi-device ]]..relname.."\n"..[[
    option type mt7663
    option vendor ralink
]])
        for _,vif in ipairs(dev.vifs) do
            print([[
config wifi-iface
    option device ]]..relname.."\n"..[[
    option ifname ]]..vif.vifname.."\n"..[[
    option network lan
    option mode ap
    option ssid ]]..vif.__ssid.."\n")
        end
    end
]=]
end
