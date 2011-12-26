#!/bin/sh

kill_daemon() {
    NAME=$1
    PF=$2

    if [ ! -r $PF ]; then
	return
    fi

    PID=`cat $PF`
    if [ $PID -gt 0 ]; then
	if ps $PID | grep -q $NAME; then
	    kill $PID
	fi
    fi
    rm $PF
}

wpa_cli term

GIFNAME=wlan0
kill_daemon dhclient /var/run/dhclient.pid-$GIFNAME
kill_daemon dnsmasq /var/run/dnsmasq.pid-$GIFNAME
