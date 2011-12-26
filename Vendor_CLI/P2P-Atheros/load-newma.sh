#!/bin/sh

cd /home/atheros/Atheros-P2P

MODULE_PATH=./newma
insmod $MODULE_PATH/adf.ko
insmod $MODULE_PATH/asf.ko
insmod $MODULE_PATH/ath_hal.ko
insmod $MODULE_PATH/ath_rate_atheros.ko
insmod $MODULE_PATH/ath_dev.ko
insmod $MODULE_PATH/umac.ko

./newma/wlanconfig wlan create wlandev wifi0 wlanmode sta
./newma/wlanconfig wlan create wlandev wifi0 wlanmode p2pdev
