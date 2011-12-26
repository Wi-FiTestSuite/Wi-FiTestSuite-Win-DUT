@echo off

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
#set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"

echo Discover target device before starting group formation...
%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
%SLEEPPATH% 4
#%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
#%SLEEPPATH% 4

%EXPPATH% sta_p2p_start_group_formation %*