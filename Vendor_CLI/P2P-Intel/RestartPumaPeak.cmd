@echo off
pushd %~dp0
cls
echo *** Stopping Event engine service...
net stop "EvtEng"

echo.
echo *** Stopping Puma Peak NIC (6300 and 6200)...
controlnic /disable 6300
controlnic /disable 6200
devcon disable PCI\CC_0280
sleep 2
echo.
echo *** Killing pan tray (ignore error if there is one)...
taskkill /F /IM ifrmewrk.exe

echo.
echo *** Waiting for a moment...
sleep 1

echo.
echo *** Starting Puma Peak NIC...
controlnic /enable 6300
controlnic /enable 6200
devcon enable PCI\CC_0280
echo.
echo *** Waiting for Virtual WiFi to start...
sleep 5

echo.
echo *** Starting Event engine service...
net start "EvtEng"
