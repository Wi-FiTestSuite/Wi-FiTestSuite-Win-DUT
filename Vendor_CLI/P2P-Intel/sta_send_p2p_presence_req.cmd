
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"

call :getargc argc %*


set INTERFACE=%1
set INTERFACEVALUE=%2



set GROUPID=%3

set BSSID=%4
echo The GO MAC is %BSSID%

set SSID=%5
echo The Group SSID is %SSID%

set GROUPIDVALUE="%BSSID% %SSID%"
echo The group id is %GROUPIDVALUE%

set DURATION=%6
set DURATIONVALUE=%7

set INTERVAL=%8
set INTERVALVALUE=%9




SET ARGUMENTCOUNT=%argc%



::////////////////////////////////////////////////////////


	echo %EXPPATH% sta_send_p2p_presence_req %INTERFACE% %INTERFACEVALUE% %DURATION% %DURATIONVALUE% %INTERVAL% %INTERVALVALUE% %GROUPID% %GROUPIDVALUE%

	%EXPPATH% sta_send_p2p_presence_req  %INTERFACE% %INTERFACEVALUE% %GROUPID% %GROUPIDVALUE% %DURATION% %DURATIONVALUE% %INTERVAL% %INTERVALVALUE%

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


