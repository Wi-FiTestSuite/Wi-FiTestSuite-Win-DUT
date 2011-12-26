rem @echo off
pushd %~dp0
cls

echo.
echo *** Replacing installed binaries with new versions... ***
echo Press CTRL+C to abort or
pause


::Start Section Stop service

echo Stopping PROSet services...
net stop "EvtEng"
net stop "RegSrvc"
net stop "WLAN AutoConfig"
net stop "EapHost"
net stop "MyWifiDHCPDNS"
net stop "wscsvc"
net stop "sharedaccess"
net stop "iphlpsvc"
net stop "winmgmt"

::End Section Stop service


echo Killing PROSet processes...
taskkill /F /IM ifrmewrk.exe
taskkill /F /IM PanUi.exe
taskkill /F /IM pantester.exe
rem taskkill /F /IM Wmiprvse.exe
taskkill /F /IM Wlanext.exe
taskkill /F /IM FileTransfer.exe



::Start Section Copy files

set COPYOPTION=/Y

echo Copying files...
copy "..\Sigmapackage14July\WFA\CLIs\Intel\*.*" "C:\WFA\CLIs\Intel"
copy "..\Sigmapackage14July\IWMSSvc.dll" "C:\WINDOWS\system32" %COPYOPTION%
copy "..\Sigmapackage14July\NETwNs32.sys" "C:\WINDOWS\system32\drivers" %COPYOPTION%
copy "..\Sigmapackage14July\wfa_cli.txt" "C:\WFA\WfaEndpoint" %COPYOPTION%
copy "..\Sigmapackage14July\PanApi.dll" "C:\Program Files\Intel\WiFi\bin" %COPYOPTION%
copy "..\Sigmapackage14July\P2PConsole.exe" "C:\Program Files\Intel\WiFi\bin" %COPYOPTION%


::End Section Copy files

::Start Section Start service

echo Restarting PROSet services...
net start "winmgmt"
net start "iphlpsvc"
rem net start "sharedaccess"
net start "wscsvc"
net start "EapHost"
net start "WLAN AutoConfig"
net start "EvtEng"
net start "RegSrvc"

echo Restarting tray icon...
start "" "C:\Program Files\Common Files\Intel\WirelessCommon\iFrmewrk.exe" /tf Intel Wireless Tray

::End Section Start service