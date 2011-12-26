#!/bin/sh

# Start DUT
xterm -geometry +0+250 -sb -si +sk -sl 1000 -e "echo WFA Sigma DUT; ./wfa_dut lo 8000" &
sleep 2

# Start CA
xterm -geometry +500+250 -sb -si +sk -sl 1000 -e "echo WFA Sigma CA; ip addr show dev eth0; echo CA port 9000; WFA_ENV_AGENT_IPADDR=127.0.0.1 WFA_ENV_AGENT_PORT=8000 ./wfa_ca eth0 9000" &

sleep 1

exit 0


# Start shell for CA/CLI commands
WFA_ENV_CA_IPADDR=127.0.0.1 WFA_ENV_CA_PORT=9000 xterm -geometry +0+350 -e "PATH=$PWD:$PATH bash; killall wfa_dut; killall wfa_ca" &
