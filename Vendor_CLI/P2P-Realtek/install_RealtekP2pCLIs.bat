@echo off

set REALTEK_CLI_VER=RT8192-1
set Input=Realtek

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

netsh wlan set ho key=12345678 keyUsage=persistent
goto :EOF
