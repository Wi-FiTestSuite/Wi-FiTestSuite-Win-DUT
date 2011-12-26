#!/bin/sh

cd /home/atheros/Atheros-P2P

## ALL VAPs must be down before ANY can be destroyed
## Do this in two stages

VAPLIST=`cat /proc/net/dev | cut -f1 -d: | tr -d ' ' | grep ^wlan`

    for i in $VAPLIST
    do
        ifconfig $i down
    done

    sleep 1
    for i in $VAPLIST
    do
        ./newma/wlanconfig $i destroy
    done

    sleep 3

    rmmod umac.ko
    rmmod ath_dev.ko
    rmmod ath_rate_atheros.ko
    rmmod ath_hal.ko
    rmmod asf.ko
    rmmod adf.ko
