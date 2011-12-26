
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
::%EXPPATH% sta_associate %*



call :getargc argc %*

set INTERFACE=%1
set INTERFACEVALUE=%2

set SSID=%3
set SSIDVALUE=%4

SET ARGUMENTCOUNT=%argc%


::////////////////////////////////////////////Let's do it Dans way/////////////////////////



		echo %EXPPATH% sta_associate %INTERFACE% %INTERFACEVALUE% %SSID% %SSIDVALUE% 
		netsh wlan connect name=%SSIDVALUE%  ssid=%SSIDVALUE%
		endlocal
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





