@echo off

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"

echo Discover target device before sending PD Request...
%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
%SLEEPPATH% 4
%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
%SLEEPPATH% 4

%EXPPATH% sta_send_p2p_provision_dis_req %*



