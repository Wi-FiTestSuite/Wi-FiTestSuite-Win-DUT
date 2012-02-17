@Echo OFF
Set ERRORLEVEL=0
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForValue
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForValue
if (%3)==(/action) Goto CheckActionValue
Set SuggestedError=/action was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if /I (%4)==(off) Set val=0
if /I (%4)==(fast) Set val=3
if /I (%4)==(off) Goto SetRegKey
if /I (%4)==(fast) Goto SetRegKey
if /I (%4)==(on) Goto ValueNotSupported
Set SuggestedError=Powersave value was not inserted or was invalid
Goto InvalidSyntax


:SetRegKey
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp

REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v PowerIndex /t REG_DWORD /d %val% /f

del temp

if /I (%5)==(/NoDisable) Goto Success
if "%5"=="" GOTO DisEnaDriver
Set SuggestedError=/NoDisable extension was misspelled
Goto InvalidSyntax


:DisEnaDriver
set myOutput2=
Devcon restart *1101* > temp2
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
echo:Example = Try - "STA_SET_POWER_SAVE /interface <interface name> /action <on/fast/off> [/NoDisable]"
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


:ValueNotSupported
echo: power save modes: on (PS-Poll mode) is not supported
SET WFA_CLI_STATUS=1
Exit /B 2

:Success
echo:
echo: Power save mode is now %4%
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2