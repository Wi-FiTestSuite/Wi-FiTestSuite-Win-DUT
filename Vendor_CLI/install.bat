@echo off
echo "Install CLIs for DUT device"
echo "Please input the Device/Company Name of DUT (Note: input 'WMM_STAUT' for WMMPS):"
set /p DeviceName=Device:
IF /i "%DeviceName%"=="WMM_STAUT" (goto WMM_STAUT)
echo "Please input the Firmware version number of DUT:"
set /p FirmwareVersion=Version:
set /p ask=Do you have the implementation of Vender's CLIs? (y/n)
IF %ask%==y ( 
echo "Please input the local path of your CLIs (end with \)"
set /p CLIPath=Local Path:
) ELSE (
goto :EOF
)

IF EXIST C:\WFA\CLIs\%DeviceName% (
rmdir /S /Q  C:\WFA\CLIs\%DeviceName%
mkdir C:\WFA\CLIs\%DeviceName%
) ELSE (
mkdir C:\WFA\CLIs\%DeviceName%
)
xcopy /S /R /Y %CLIPath%*.* C:\WFA\CLIs\%DeviceName%\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\%DeviceName%" > C:\WFA\WFAEndpoint\wts_settings.txt
netsh wlan show interfaces > C:\WFA\Temp\temp.txt
FOR /F "tokens=2 delims=: " %%i IN ('findstr GUID C:\Wfa\Temp\temp.txt') DO @echo DEVICE={%%i} >> C:\WFA\WfaEndpoint\wts_settings.txt
echo VERSION=%FirmwareVersion% >> C:\WFA\WfaEndpoint\wts_settings.txt

cls
IF %ask%==y ( 
IF EXIST %CLIPath% (
echo "CLIs has been installed!"
) ELSE ( 
echo "Install failed: The Path is invalid!"
)
)

:WMM_STAUT
mkdir C:\WFA\CLIs\WMM_STAUT
echo WMM_STAUT > c:\install.out
xcopy /S /R /Y WMM_STAUT\*.* C:\WFA\CLIs\WMM_STAUT\

echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\WMM_STAUT" >> C:\WFA\WfaEndpoint\wts_settings.txt
echo DEVICE=WMM_STAUT >> C:\WFA\WfaEndpoint\\wts_settings.txt
echo VERSION=%WMM_STAUT_CLI_VER% >> C:\WFA\WfaEndpoint\wts_settings.txt
echo #end >> C:\WFA\WfaEndpoint\\wts_settings.txt
echo WMM_STAUT

echo Install WMM_STAUT OK, press any key to exit.

pause

goto :EOF




