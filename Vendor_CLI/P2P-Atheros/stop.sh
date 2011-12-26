#!/bin/sh

echo Stopping Atheros P2P
cd /home/atheros/Atheros-P2P
sudo ./stop-wpas.sh
./stop-dut.sh
