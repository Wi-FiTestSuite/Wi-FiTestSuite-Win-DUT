@echo off

CD "C:\WFA\CLIs\Intel"

set ADAPTERPATH="C:\WFA\CLIs\Intel\devcon.exe"
set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"
set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"


%ADAPTERPATH% disable pci\cc_0280

%SLEEPPATH% 2

%ADAPTERPATH% enable pci\cc_0280


%SLEEPPATH% 4

echo %EXPPATH% sta_set_p2p /interface wlan /p2p_mode listen /listen_chn 1
%EXPPATH% sta_set_p2p /interface wlan /p2p_mode listen /listen_chn 1


%SLEEPPATH% 2

echo Discover target device before starting group formation...
echo %EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover

%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
%SLEEPPATH% 4



echo %EXPPATH% sta_wps_read_pin /interface wlan0

%EXPPATH% sta_wps_read_pin /interface wlan0


%SLEEPPATH% 7

echo %EXPPATH% sta_p2p_start_group_formation /interface wlan /p2pdevid 00:15:00:5B:50:91 /intent_val 7 /init_go_neg 1
%EXPPATH% sta_p2p_start_group_formation /interface wlan /p2pdevid 00:15:00:5B:50:91 /intent_val 7 /init_go_neg 1