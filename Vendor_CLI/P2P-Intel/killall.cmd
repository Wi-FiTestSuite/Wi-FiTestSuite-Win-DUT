@echo on
REM - Vista OS

REM - Close all the services
REM - Close all the exes
set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"
netsh wlan disconnect interface=* 
netsh wlan delete profile name=* interface=*
net stop MyWiFiDHCPDNS

devcon disable PCI\CC_0280

%SLEEPPATH% 1

taskkill /F /IM WLANExt.exe
taskkill /F /IM ifrmewrk.exe

%SLEEPPATH% 1