@Echo OFF
Set ERRORLEVEL=0
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForAction
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForAction
if (%3)==(/action) Goto CheckActionValue
Set SuggestedError=/action was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if /I (%4)==(Enable) Set flag=1
if /I (%4)==(Disable) Set flag=0
if /I (%4)==(Enable) Goto SetRegKey
if /I (%4)==(Disable) Goto SetRegKey
Set SuggestedError=action value was not inserted or misspelled
Goto InvalidSyntax

:SetRegKey
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp

REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v fatChannelIntolerant /t REG_SZ /d %flag% /f

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
echo:Example = Try - "SET_40_INTOLERANT /interface <interface name> /action <Enable/Disable> [/NoDisable]"
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
echo:Fat Channel Intolerant is now %4%
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2