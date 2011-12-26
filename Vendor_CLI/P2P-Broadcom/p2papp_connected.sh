#
# This script is called by p2papp after a connection is established between
# the two peers.  This script:
# - sets up the network interface for the connection
# - verifies the connection by doing a ping and a file transfer.
# When this script exits, p2papp will tear down the connection.
#
# $Copyright Broadcom Corporation$
#
# $Id: p2papp_connected.sh,v 1.28 2010/07/31 02:20:55 johnvb Exp $
#

enable_ping=$3

echo === Kill existing dhcpd:
#dheth1_pid=`ps aux | grep dhcpd | awk '{ print $2 }'`
dheth1_pid=`ps -e | /bin/grep dhcpd | awk '{ print $1 }'`
if [ -n "${dheth1_pid}" ]
then
   echo kill -9 $dheth1_pid
   kill -9 $dheth1_pid
fi

echo === Clearing dhcpd leases
dhcpd_leases_file="/var/lib/dhcpd/dhcpd.leases"
if [ -a ${dhcpd_leases_file} ]
then   
   echo cp -f ${dhcpd_leases_file} ${dhcpd_leases_file}.bak
   cp -f ${dhcpd_leases_file} ${dhcpd_leases_file}.bak
   echo rm -f ${dhcpd_leases_file}
   rm -f ${dhcpd_leases_file}
fi
if [ -a ${dhcpd_leases_file}~ ]
then
   echo rm -f ${dhcpd_leases_file}~
   rm -f ${dhcpd_leases_file}~
fi
echo touch ${dhcpd_leases_file}
touch ${dhcpd_leases_file}

echo === Kill existing dhclient:
dheth1_pid=`ps -e | /bin/grep dhclient | awk '{ print $1 }'`
if [ -n "${dheth1_pid}" ]
then
   echo kill -9 $dheth1_pid
   kill -9 $dheth1_pid
fi

echo === Clearing dhclient leases
dhclient_leases_file="/var/lib/dhclient/dhclient.leases"
if [ -a ${dhclient_leases_file} ]
then   
   cp -f ${dhclient_leases_file} ${dhclient_leases_file}.bak
   echo rm -f ${dhclient_leases_file}
   rm -f ${dhclient_leases_file}
fi

#
# Actions for a standlone Group Owner 
#
if [ $1 == standalone_go ]; then

echo === Running DHCP Server
echo /sbin/ifconfig $2
/sbin/ifconfig $2
echo /usr/sbin/dhcpd -cf ./dhcpd.conf $2
/usr/sbin/dhcpd -cf ./dhcpd.conf $2

fi



#
# Actions for the AP peer in a P2P connection
#
if [ $1 == ap ]; then

# On the AP peer, our IP address is statically assigned by the P2P Library
# to the IP address required by the DHCP server.
echo
echo === Running DHCP Server
echo
#echo ifconfig $2
#/sbin/ifconfig $2
echo /usr/sbin/dhcpd -cf ./dhcpd.conf $2
/usr/sbin/dhcpd -cf ./dhcpd.conf $2

#sleep 1

#echo
#echo === Check that we can ping the peer:
#echo
#echo ping -c 5 192.168.16.21
#ping -c 3 192.168.16.21

#echo
#echo === Testing a file transfer to the peer...
#echo
#echo scp testfile root@192.168.16.21:/tmp
#scp testfile root@192.168.16.21:/tmp
#echo

#echo
#echo === Press ENTER twice to start the iperf server.
#echo === Do this FIRST, before starting the iperf client on the peer.
#read abc
#echo ./iperf -u -s -P 1 -i 1
#echo ./iperf -s -P 1 -i 1
#echo
#echo ===
#echo === NOTE: When the iperf client is finished the test, 
#echo ===       press ctrl-C here to exit the iperf server.
#echo ===
#echo
#./iperf -u -s -P 1 -i 1
#./iperf -s -P 1 -i 1

fi



#
# Actions for the AP side of the connection in Auto Responder mode
#
if [ $1 == auto_ap ]; then

# On the AP peer, our IP address is statically assigned by the P2P Library
# to the IP address required by the DHCP server.  Show this address.
echo ifconfig $2
ifconfig $2

if [ $enable_ping == "1" ]; then
   # Check that we can ping the peer:
   echo ping -c 3 192.168.16.212
   ping -c 3 192.168.16.212
fi   

fi



#
# Actions for the STA peer in a P2P connection
#
if [ $1 == sta ]; then

echo /sbin/ifconfig $2 up
/sbin/ifconfig $2 up

# wait for DHCP server
sleep 3
echo
echo === Running DHCP Client
echo
echo /sbin/dhclient $2
/sbin/dhclient $2
echo === DCHP Client obtained IP address: 
ifconfig $2


if [ $enable_ping == "1" ]; then
   echo
   echo === Check that we can ping the peer:
   echo
   echo ping -c 3 192.168.16.1
   ping -c 3 192.168.16.1
fi   

#echo === TEMP: test STA powersave
#echo ./wl PM 1
#./wl PM 1
#echo ping -c 3 192.168.16.212

#if [ -e /tmp/testfile ]; then
#rm -v /tmp/testfile
#fi

#echo === Waiting up to 100 seconds for peer to initiate a file transfer...
#if [ -e /tmp/testfile ]; then
#  echo rm -v /tmp/testfile
#  rm -v /tmp/testfile
#fi
#for (( i = 0 ; i <= 100 ; i++ ))
#do
#  sleep 1
#  if [ -e /tmp/testfile ]; then
#    echo
#    echo === Received file:
#    ls -l /tmp/testfile
#    echo
#    exit 0
#  fi
#done

#echo
#echo === Press ENTER twice to start the iperf client.
#echo === Do this SECOND, after starting the iperf server on the peer.
#read abc
#echo ./iperf -u -c 192.168.16.1 -t 10 -i 1
#./iperf -u -c 192.168.16.1 -t 10 -i 1
#echo ./iperf -c 192.168.16.1 -t 10 -i 1
#./iperf -c 192.168.16.1 -t 10 -i 1

fi



#
# Actions for the STA side of the connection in Auto Responder mode
#
if [ $1 == auto_sta ]; then

# For now, statically assign ourself an IP address.
# In the future, invoke a DHCP client to get an IP address from
# the DHCP server running on the AP peer.
echo
echo === Configuring STA static IP address:
echo
echo ifconfig $2 192.168.16.212 up
ifconfig $2 192.168.16.212 up
ifconfig $2

if [ $enable_ping == "1" ]; then
   echo
   echo === Check that we can ping the peer:
   echo
   echo ping -c 3 192.168.16.1
   ping -c 3 192.168.16.1
fi

# For the Set Top Box only:
# Start the STB media player if not already running
# RUNNING=`ps | grep browsertest | grep -v grep`
#
#echo media_player_running=$RUNNING
#if [ -n "$RUNNING" ]; then
#    echo Media player already running.
#else
#    echo Starting media player.
#	sh player.sh &
#fi


fi
