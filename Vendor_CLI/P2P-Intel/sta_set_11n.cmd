@echo off

set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"
set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"

#set 11n params in driver root
%EXPPATH% sta_set_11n %*

#Disable enable driver.
devcon disable PCI\CC_0280
%SLEEPPATH% 2

devcon enable PCI\CC_0280
%SLEEPPATH% 6


