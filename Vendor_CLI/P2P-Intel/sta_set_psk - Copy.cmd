
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
::%EXPPATH% sta_set_psk %*

call :getargc argc %*

set INTERFACE=%1
set INTERFACEVALUE=%2

set SSID=%3
set SSIDVALUE=%4


SET ARGUMENTCOUNT=%argc%


::////////////////////////////////////////////Let's do it Dans way/////////////////////////


	IF %SSIDVALUE% EQU "5.1.6" goto profile5.1.6
	IF %SSIDVALUE% EQU "5.1.7" goto profile5.1.7

        echo ERROR: Unsupported arg count!
	goto eof

		echo %EXPPATH% sta_set_psk %*

:profile5.1.6
		netsh wlan add profile filename="C:\WFA\CLIs\Intel\Wireless Network Connection-autobat_pan_test_5.1.6.xml"
		SETX WFA_CLI_STATUS 0
		endlocal
:profile5.1.7
		netsh wlan add profile filename="C:\WFA\CLIs\Intel\Wireless Network Connection-autobat_pan_test_5.1.7.xml"
		SETX WFA_CLI_STATUS 0		
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





