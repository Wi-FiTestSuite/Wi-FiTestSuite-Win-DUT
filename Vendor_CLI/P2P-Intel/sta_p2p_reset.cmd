@echo off

pushd %~dp0
cls
set BINPLATFORM=Win32
set BINCONFIG=Release


set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
::call sta_p2p_dissovle.cmd /interface wlan /groupid "00:00:00:00:00:00 DIRECT-XX"

%EXPPATH% sta_set_11n /addba_reject disable /ampdu disable

call killall.cmd

call startall.cmd


%EXPPATH% sta_p2p_reset %*

SETX WFA_CLI_STATUS 0




