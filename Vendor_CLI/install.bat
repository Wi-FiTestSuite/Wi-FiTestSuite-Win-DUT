@echo off

set RALINK_CLI_VER=RA3592-1
set INTEL_CLI_VER=6200-1
set REALTEK_CLI_VER=RT8192-1
set RALINK_PMF_CLI_VER=RA3593-1

echo "Install CLI's For STA (INTEL-P2P/RALINK-P2P/Realtek/RALINK-PMF/INTEL-6300/INTEL-6200)?"
set /p Input=
if /i "%Input%"=="Intel-P2P" (goto Intel-P2P)
if /i "%Input%"=="Realtek" (goto Realtek)
if /i "%Input%"=="RALINK-P2P" (goto Ralink-P2P)
if /i "%Input%"=="RALINK-PMF" (goto Ralink-PMF)
if /i "%Input%"=="Intel-6300" (goto Intel-6300)
if /i "%Input%"=="Intel-6200" (goto Intel-6200)
else exit /b


:Intel-P2P
echo Intel-P2P
IF EXIST C:\WFA\CLIs\Intel (
rmdir /S /Q  C:\WFA\CLIs\Intel
mkdir C:\WFA\CLIs\Intel
) ELSE (
mkdir C:\WFA\CLIs\Intel
)
xcopy /S /R /Y Intel-P2P\*.* C:\WFA\CLIs\Intel\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Intel" > C:\WFA\WFAEndpoint\sigma_settings.txt
netsh wlan show interfaces > C:\WFA\Temp\temp.txt
FOR /F "tokens=2 delims=: " %%i IN ('findstr GUID C:\Wfa\Temp\temp.txt') DO @echo DEVICE={%%i} >> C:\WFA\WfaEndpoint\sigma_settings.txt
echo VERSION=%INTEL_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt
goto :EOF

:Intel-6300
echo Intel-6300
IF EXIST C:\WFA\CLIs\Intel (
rmdir /S /Q  C:\WFA\CLIs\Intel
mkdir C:\WFA\CLIs\Intel
) ELSE (
mkdir C:\WFA\CLIs\Intel
)
xcopy /S /R /Y Intel-6300\*.* C:\WFA\CLIs\Intel\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Intel" > C:\WFA\WFAEndpoint\sigma_settings.txt
echo DEVICE=6300>> C:\WFA\WFA_Endpoint\sigma_settings.txt
echo VERSION=%INTEL_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt
goto :EOF

:Intel-6200
echo Intel-6200
IF EXIST C:\WFA\CLIs\Intel (
rmdir /S /Q  C:\WFA\CLIs\Intel
mkdir C:\WFA\CLIs\Intel
) ELSE (
mkdir C:\WFA\CLIs\Intel
)
xcopy /S /R /Y Intel-6200\*.* C:\WFA\CLIs\Intel\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Intel" > C:\WFA\WFAEndpoint\sigma_settings.txt
echo DEVICE=6200>> C:\WFA\WFA_Endpoint\sigma_settings.txt
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

:Ralink-P2P
echo P2P Ralink
IF EXIST C:\WFA\CLIs\Ralink (
rmdir /S /Q  C:\WFA\CLIs\Ralink
mkdir C:\WFA\CLIs\Ralink
) ELSE (
mkdir C:\WFA\CLIs\Ralink
)
xcopy /S /R /Y Ralink-P2P\*.* C:\WFA\CLIs\Ralink\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Ralink" > C:\WFA\WfaEndpoint\sigma_settings.txt
netsh wlan show interfaces > C:\WFA\Temp\temp.txt
FOR /F "tokens=2 delims=: " %%i IN ('findstr GUID C:\Wfa\Temp\temp.txt') DO @echo DEVICE={%%i} >> C:\WFA\WfaEndpoint\sigma_settings.txt
echo VERSION=%Ralink_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt


:Ralink-PMF
echo PMF Ralink
IF EXIST C:\WFA\CLIs\Ralink (
rmdir /S /Q  C:\WFA\CLIs\Ralink
mkdir C:\WFA\CLIs\Ralink
) ELSE (
mkdir C:\WFA\CLIs\Ralink
)
xcopy /S /R /Y Ralink-PMF\*.* C:\WFA\CLIs\Ralink\
echo WFA_CLI_STA_DEVICE="C:\WFA\CLIs\Ralink" > C:\WFA\WfaEndpoint\sigma_settings.txt
netsh wlan show interfaces > C:\WFA\Temp\temp.txt
FOR /F "tokens=2 delims=: " %%i IN ('findstr GUID C:\Wfa\Temp\temp.txt') DO @echo DEVICE={%%i} >> C:\WFA\WfaEndpoint\sigma_settings.txt
echo VERSION=%Ralink_PMF_CLI_VER% >> C:\WFA\WfaEndpoint\sigma_settings.txt


