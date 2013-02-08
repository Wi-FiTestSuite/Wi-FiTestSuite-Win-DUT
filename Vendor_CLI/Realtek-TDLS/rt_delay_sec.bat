@echo off

:: this does not work on win7
::PING 1.1.1.1 -n 1 -w %1 >NUL
set /a count=%1+1
ping 127.0.0.1 -n %count% > NUL