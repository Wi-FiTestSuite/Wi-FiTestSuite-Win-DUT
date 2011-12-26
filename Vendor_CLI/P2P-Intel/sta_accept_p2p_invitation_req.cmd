
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"

call :getargc argc %*


set INTERFACE=%1
set INTERFACEVALUE=%2

set DEVICEID=%3
set DEVICEIDVALUE=%4

set GROUPID=%5

set BSSID=%6
echo The GO MAC is %BSSID%

set SSID=%7
echo The Group SSID is %SSID%

set GROUPIDVALUE="%BSSID% %SSID%"
echo The group id is %GROUPIDVALUE%


set REINVOKE=%8
set REINVOKEVALUE=%9

SET ARGUMENTCOUNT=%argc%



::////////////////////////////////////////////////////////


	echo %EXPPATH% sta_accept_p2p_invitation_req %INTERFACE% %INTERFACEVALUE% %DEVICEID% %DEVICEIDVALUE% %GROUPID% %GROUPIDVALUE% %REINVOKE% %REINVOKEVALUE%

	%EXPPATH% sta_accept_p2p_invitation_req %INTERFACE% %INTERFACEVALUE% %DEVICEID% %DEVICEIDVALUE% %GROUPID% %GROUPIDVALUE% %REINVOKE% %REINVOKEVALUE%

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


