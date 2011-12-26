
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"
set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"

%EXPPATH% sta_set_ip_config %*

SETX WFA_CLI_STATUS 0