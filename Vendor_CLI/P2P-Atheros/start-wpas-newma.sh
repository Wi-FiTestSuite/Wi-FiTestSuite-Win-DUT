#!/bin/sh

./disable-nm-wireless.sh
stop network-manager
sleep 1
killall wpa_supplicant
stop avahi-daemon
killall dnsmasq
./unload-newma.sh
sleep 1
./load-newma.sh
sleep 1
./disable-nm-wireless.sh
sleep 1
mkdir -p Logs
./wpa_supplicant -Dathr -i wlan0 -c sta.conf -N -Dathr -i wlan1 -c w.conf -ddKt > Logs/`date +%s` &
sleep 3
./wpa_cli -iwlan1 -a$PWD/p2p-action.sh -B
