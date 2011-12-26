REM Start the services

set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"
devcon enable PCI\CC_0280
%SLEEPPATH% 6
