@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"

call :getargc argc %*


set INTERFACE=%1
set INTERFACEVALUE=%2
set PIN=%3
set PINVALUE=%4


set GROUPID=%5


set BSSID=%6
echo The GO MAC is %BSSID%

set SSID=%7
echo The Group SSID is %SSID%

SET GROUPIDVALUE="%BSSID% %SSID%"
echo The Group ID is %GROUPIDVALUE%

SET ARGUMENTCOUNT=%argc%

echo %ARGUMENTCOUNT%

::////////////////////////////////////////////////////////

	IF %ARGUMENTCOUNT% EQU 7 goto group
	IF %ARGUMENTCOUNT% EQU 4 goto normal

        echo ERROR: Unsupported arg count!
	goto eof
        
:group
	echo %EXPPATH% sta_wps_enter_pin %INTERFACE% %INTERFACEVALUE% %PIN% %PINVALUE% %GROUPID% %GROUPIDVALUE%
	%EXPPATH% sta_wps_enter_pin %INTERFACE% %INTERFACEVALUE% %PIN% %PINVALUE% %GROUPID% %GROUPIDVALUE%
	endlocal
goto :eof


:normal
	echo %EXPPATH% sta_wps_enter_pin %INTERFACE% %INTERFACEVALUE% %PIN% %PINVALUE%
	%EXPPATH% sta_wps_enter_pin %INTERFACE% %INTERFACEVALUE% %PIN% %PINVALUE%
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









