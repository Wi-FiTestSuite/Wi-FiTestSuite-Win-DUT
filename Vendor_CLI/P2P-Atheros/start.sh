#!/bin/sh

echo Starting Atheros P2P
cd /home/atheros/Atheros-P2P

if [ ! -e w.conf ]; then
    cp wpas-default.conf w.conf
fi
if [ ! -e sta.conf ]; then
    cp wpas-default-sta.conf sta.conf
fi

if [ ! -e Logs ]; then
    mkdir Logs
fi

if modinfo ath9k > /dev/null 2>&1; then
    echo Loading ath9k
    sudo ./start-wpas.sh
else
    echo Loading newma
    sudo ./start-wpas-newma.sh
fi
./start-dut.sh
sleep 1
echo Atheros P2P started
sleep 2
