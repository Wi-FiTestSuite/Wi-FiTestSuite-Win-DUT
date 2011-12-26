@ECHO off

Set SsidStr=%1

echo %SsidStr%>temp.txt
FOR %%A IN (temp.txt) DO SET SsidLen=%%~zA
set /A SsidLen=%SsidLen%-2
DEL temp.txt
echo %0: ssid: %SsidStr%
echo %0: ssid len: %SsidLen%

call dec_to_hex.bat %SsidLen% 1 > temp2.txt
SET /p HexSsidLen=<temp2.txt
del temp2.txt
set counter=0
set HexAsciiStr=%HexSsidLen%

::
:: For each char in the ssid str, translate to hex value
::
:loop
CALL SET temp=%%SsidStr:~%counter%,1%%
::echo %temp%

:: result is in %asc%
call asc.bat %temp%
::echo %asc%

:: translate from decimal to hex
call dec_to_hex.bat %asc% 1 >temp.txt
SET /p HexAscVal=<temp.txt
del temp.txt
::echo %HexAscVal%

set HexAsciiStr=%HexAsciiStr% %HexAscVal%
::echo HexAsciiStr: %HexAsciiStr%

set /a counter=%counter%+1
::echo counter=%counter%
::echo SsidLen=%SsidLen%
if %counter% LSS %SsidLen% goto loop

echo %HexAsciiStr%

