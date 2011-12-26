
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"


call :getargc argc %*


set INTERFACE=%1
set INTERFACEVALUE=%2

set DEVICEID=%3
set DEVICEIDVALUE=%4

set GROUPID=%5

set BSSID=%6
echo The GO MACis %BSSID%

set SSID=%7
echo The Group SSID is %SSID%

SET GROUPIDVALUE="%BSSID% %SSID%"
echo The group id is %GROUPIDVALUE%


SET ARGUMENTCOUNT=%argc%

echo Discover target device before starting connection...
%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
%SLEEPPATH% 4
%EXPPATH% sta_set_p2p /interface wlan /p2p_mode discover
%SLEEPPATH% 4


::////////////////////////////////////////////////////////


	echo %EXPPATH% sta_p2p_connect %INTERFACE% %INTERFACEVALUE% %DEVICEID% %DEVICEIDVALUE% %GROUPID% %GROUPIDVALUE%

	%EXPPATH% sta_p2p_connect %INTERFACE% %INTERFACEVALUE% %DEVICEID% %DEVICEIDVALUE% %GROUPID% %GROUPIDVALUE%

	endlocal
goto :eof


goto :eof

:getargc
		set getargc_v0=%1
		set /a "%getargc_v0%=0"
		:getargc_l0
			if not x%2x==xx (
						shift
						set /a "%getargc_v0%=%getargc_v0% + 1"
						goto :getargc_l0
					)
		set getargc_v0=
		goto :eof


