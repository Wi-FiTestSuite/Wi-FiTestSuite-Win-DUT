@Echo OFF
Set ERRORLEVEL=0
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6200) Goto CheckForAction
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForAction
if (%3)==(/action) Goto CheckActionValue
Set SuggestedError=/action was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if (%4)==(long) GOTO SetLongPREAMBLE
if (%4)==(short) GOTO SetShortPREAMBLE
Set SuggestedError=action value was not inserted or misspelled
Goto InvalidSyntax

:SetLongPREAMBLE
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Advanced-N 6200 AGN" > temp
set /P myOutput=<temp

REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v FixedLongPreamble /t REG_DWORD /d 1 /f


del temp

if /I (%5)==(/NoDisable) Goto Success
if "%5"=="" GOTO DisEnaDriver
Set SuggestedError=/NoDisable extension was misspelled
Goto InvalidSyntax

:SetShortPREAMBLE
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Advanced-N 6200 AGN" > temp
set /P myOutput=<temp

REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v FixedLongPreamble /t REG_DWORD /d 0 /f


del temp

if /I (%5)==(/NoDisable) Goto Success
if "%5"=="" GOTO DisEnaDriver
Set SuggestedError=/NoDisable extension was misspelled
Goto InvalidSyntax

:DisEnaDriver
set myOutput2=
Devcon restart *422c* > temp2
findstr /C:"1 device(s) restarted" temp2 > temp3
set /p myOutput2=<temp3
del temp2
del temp3

if "%myOutput2%"=="" Goto DevconFail
echo:
echo:Device restarted succesfully 
Goto Success



:InvalidSyntax
echo: syntax Error
echo:
echo:Example = Try - "SET_PREAMBLE /interface <interface name> /action <Value>"
echo:
echo:Suggested Error: %SuggestedError%
SET WFA_CLI_STATUS=2
Exit /B 2


:DevconFail
echo:
echo:Device Disable/Enable failed
echo: 
SET WFA_CLI_STATUS=1
Exit /B 2

:Success
echo:
echo:Preamble is now %4%
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2