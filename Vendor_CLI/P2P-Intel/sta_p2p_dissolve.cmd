
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"

call :getargc argc %*


set INTERFACE=%1
set INTERFACEVALUE=%2

set GROUPID=%3

set BSSID=%4
echo The GO MACis %BSSID%

set SSID=%5
echo The Group SSID is %SSID%

SET GROUPIDVALUE="%BSSID% %SSID%"
echo The group id is %GROUPIDVALUE%


SET ARGUMENTCOUNT=%argc%


::////////////////////////////////////////////Let's do it Dans way//////////

%EXPPATH% sta_p2p_dissolve %INTERFACE% %INTERFACEVALUE% %GROUPID% %GROUPIDVALUE%

