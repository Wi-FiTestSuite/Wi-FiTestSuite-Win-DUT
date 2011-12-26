#!/bin/sh

./disable-nm-wireless.sh
stop avahi-daemon
killall dnsmasq
rmmod ath9k
sleep 1
modprobe ath9k
sleep 1
./disable-nm-wireless.sh
sleep 1
./wpa_supplicant -Dnl80211 -iwlan0 -c w.conf -ddKt > Logs/`date +%s` &
sleep 1
./wpa_cli -a$PWD/p2p-action.sh -B
