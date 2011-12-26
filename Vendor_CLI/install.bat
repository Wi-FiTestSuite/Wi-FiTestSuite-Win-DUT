@echo off

set RALINK_CLI_VER=RA3592-1
set INTEL_CLI_VER=6200-1
set REALTEK_CLI_VER=RT8192-1

echo "Install CLI's For STA (INTEL/RALINK/Realtek)?"
set /p Input=
if /i "%Input%"=="Intel" (goto Intel)
if /i "%Input%"=="Realtek" (goto Realtek)
if /i "%Input%"=="RALINK" (goto Ralink)
else exit /b


:Intel
echo Intel
IF EXIST C:\WFA\CLIs\Intel (
rmdir /S /Q  C:\WFA\CLIs\Intel
mkdir C:\WFA\CLIs\Intel
) ELSE (
mkdir C:\WFA\CLIs\Intel
)
xcopy /S /R /Y Intel\*.* C:\WFA\CLIs\Intel\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Intel" > C:\WFA\WFAEndpoint\sigma_settings.txt
netsh wlan show interfaces > C:\WFA\Temp\temp.txt
FOR /F "tokens=2 delims=: " %%i IN ('findstr GUID C:\Wfa\Temp\temp.txt') DO @echo DEVICE={%%i} >> C:\WFA\WfaEndpoint\sigma_settings.txt
echo VERSION=%INTEL_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt
goto :EOF

:Realtek
echo Realtek
IF EXIST C:\WFA\CLIs\Realtek (
rmdir /S /Q  C:\WFA\CLIs\Realtek
mkdir C:\WFA\CLIs\Realtek
) ELSE (
mkdir C:\WFA\CLIs\Realtek
)
xcopy /S /R /Y Realtek\*.* C:\WFA\CLIs\Realtek\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Realtek" > C:\WFA\WfaEndpoint\sigma_settings.txt
echo DEVICE=rtl8192s >> C:\WFA\WfaEndpoint\sigma_settings.txt
echo VERSION=%REALTEK_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt
goto :EOF

:Ralink
echo Ralink
IF EXIST C:\WFA\CLIs\Ralink (
rmdir /S /Q  C:\WFA\CLIs\Ralink
mkdir C:\WFA\CLIs\Ralink
) ELSE (
mkdir C:\WFA\CLIs\Ralink
)
xcopy /S /R /Y Ralink\*.* C:\WFA\CLIs\Ralink\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Ralink" > C:\WFA\WfaEndpoint\sigma_settings.txt
netsh wlan show interfaces > C:\WFA\Temp\temp.txt
FOR /F "tokens=2 delims=: " %%i IN ('findstr GUID C:\Wfa\Temp\temp.txt') DO @echo DEVICE={%%i} >> C:\WFA\WfaEndpoint\sigma_settings.txt
echo VERSION=%Ralink_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt
