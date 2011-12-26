
kill $(ps -e | grep dhcpd | awk '{ print $1 }')
kill $(ps -e | grep dhclient | awk '{ print $1 }')

rmmod wl
insmod wl.ko
#ifconfig eth1 up

./wl channel 11
./wl apsta 1
#wl up
./wl assert_type 2
