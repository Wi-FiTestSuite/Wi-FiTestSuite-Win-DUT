@echo off

SET Target=%1

SET SsidStr1=
SET SsidStr2=
SET SsidStr3=
SET SsidStr4=

if /I (%Target%)==(go) goto GetGOSsid
if /I (%Target%)==(client) goto GetClientSsid
goto InvalidSyntax

:GetGOSsid
call netsh wlan show ho > temp.txt
findstr /I /C:"SSID" temp.txt>temp2.txt
SET /p SsidStr1=<temp2.txt
:: Remove unnecessary chars
SET SsidStr2=%SsidStr1:SSID name=%
SET SsidStr3=%SsidStr2::=%
SET SsidStr4=%SsidStr3:"=%
SET SsidStr5=%SsidStr4: =%
echo %0: SSID: %SsidStr5%

del temp.txt
del temp2.txt

SET RT_CLI_RETURN=%SsidStr5%

goto exit

:GetClientSsid
call netsh wlan show interface > temp.txt
findstr /I /C:"SSID" temp.txt>temp2.txt

:: Count file size of temp2.txt
FOR %%A IN (temp2.txt) DO SET FileSize=%%~zA
::echo FileSize=%FileSize%

if %FileSize% EQU 0 (
	SET RT_CLI_RETURN=
	goto GetClientSsid_cleanup)
	
if %FileSize% NEQ 0 goto ParseClientSsid

:ParseClientSsid
SET /p SsidStr1=<temp2.txt

SET SsidStr2=%SsidStr1:SSID=%
SET SsidStr3=%SsidStr2: =%
SET SsidStr4=%SsidStr3::=%
SET RT_CLI_RETURN=%SsidStr4%
echo RT_CLI_RETURN=%RT_CLI_RETURN%

::echo SsidStr1=%SsidStr1%
:GetClientSsid_cleanup
del temp.txt
del temp2.txt

goto exit

:InvalidSyntax
echo:syntax Error
echo:
echo:Example = Try - "%0 <go|client>"
echo:
Exit /B 2

:Exit
Exit /B 2

:: =======================Backup=======================

:: Get SSID string
d.exe -ssid g > temp1
findstr /I /C:"SSID: " temp1> temp2

:: Get string length
FOR %%A IN (temp2) DO SET SsidLen=%%~zA
set /A SsidLen=%SsidLen%-2
set /A SsidLen=%SsidLen%-6

SET /p temp=<temp2
CALL SET SsidStr=%%temp:~6,%SsidLen%%%
::ECHO %0: SSID Str: %SsidStr%
::ECHO %0: SSID Len: %SsidLen%

:: if no ssid, print nothing (if we use "echo.", it prints a new line, if we don't echo anything,it prints "echo is off")
if %SsidLen% EQU 0 <nul (set/p anyvariable=) 
if %SsidLen% NEQ 0 ECHO %SsidStr% 
