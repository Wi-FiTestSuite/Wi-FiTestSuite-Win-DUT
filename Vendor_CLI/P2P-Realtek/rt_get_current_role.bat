@echo off
d.exe -g FF0101F9 > rtgetcurrentroletemp.txt
findstr /I /R /C:"^[0-9A-F][0-9A-F]" rtgetcurrentroletemp.txt > rtgetcurrentroletemp2.txt
set /p rtgetcurrentroletemp3=<rtgetcurrentroletemp2.txt
SET rtgetcurrentroletemp4=%rtgetcurrentroletemp3:~0,2%
SET RT_CLI_RETURN=%rtgetcurrentroletemp4%

if %RT_CLI_RETURN% EQU 0 Set RoleStr=NONE
if %RT_CLI_RETURN% EQU 1 Set RoleStr=P2P Device
if %RT_CLI_RETURN% EQU 2 Set RoleStr=P2P Client
if %RT_CLI_RETURN% EQU 3 Set RoleStr=P2P GO

::ECHO %0: Current Role: %RoleStr% (%RT_CLI_RETURN%)

del rtgetcurrentroletemp.txt
del rtgetcurrentroletemp2.txt

:exit
