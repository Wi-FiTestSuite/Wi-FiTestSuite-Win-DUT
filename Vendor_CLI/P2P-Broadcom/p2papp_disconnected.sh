#
# This script is called by p2papp after tearing down a connection between
# the two peers.  This script de-initializes the network interface for the
# connection.
#
# $Copyright Broadcom Corporation$
#
# $Id: p2papp_disconnected.sh,v 1.12 2010/06/23 02:05:11 dlo Exp $
#


echo === Kill existing dhcpd:
#dheth1_pid=`ps aux | grep dhcpd | awk '{ print $2 }'`
dheth1_pid=`ps -e | /bin/grep dhcpd | awk '{ print $1 }'`
if [ -n "${dheth1_pid}" ]
then
   echo kill -9 $dheth1_pid
   kill -9 $dheth1_pid
fi

echo === Kill existing dhclient:
dheth1_pid=`ps -e | /bin/grep dhclient | awk '{ print $1 }'`
if [ -n "${dheth1_pid}" ]
then
   echo kill -9 $dheth1_pid
   kill -9 $dheth1_pid
fi   


#
# Actions for the AP peer in a P2P connection
#
if [ $1 == ap ]; then

#echo
#echo === Check that we can no longer ping the peer:
#echo
#echo ping -c 1 -W 3 192.168.16.202
#ping -c 1 -W 3 192.168.16.202

echo ifconfig $2 0.0.0.0
/sbin/ifconfig $2 0.0.0.0

kill $(ps -e | grep dhcpd | awk '{ print $1 }')

fi



#
# Actions for the STA peer in a P2P connection
#
if [ $1 == sta ]; then

#echo
#echo === Check that we can no longer ping the peer:
#echo
#echo ping -c 1 -W 3 192.168.16.1
#ping -c 1 -W 3 192.168.16.1

echo ifconfig $2 0.0.0.0
/sbin/ifconfig $2 0.0.0.0

kill $(ps -e | grep dhclient | awk '{ print $1 }')

fi

date
