@echo off

:: Get SSID string
d.exe -gossid g > temp1
findstr /I /C:"GO SSID:" temp1> temp2

:: Get string length
FOR %%A IN (temp2) DO SET SsidLen=%%~zA
set /A SsidLen=%SsidLen%-9

SET /p temp=<temp2
CALL SET SsidStr=%%temp:~9,%SsidLen%%%
::ECHO %0: SSID Str: %SsidStr%
::ECHO %0: SSID Len: %SsidLen%


:: if no ssid, print nothing (if we use "echo.", it prints a new line, if we don't echo anything,it prints "echo is off")
::if %SsidLen% EQU 0 <nul (set/p anyvariable=) 
::if %SsidLen% NEQ 0 ECHO %SsidStr%

set RT_CLI_RETURN=%SsidStr%
