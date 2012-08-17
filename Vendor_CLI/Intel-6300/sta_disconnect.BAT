@Echo OFF
Set ERRORLEVEL=0
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto StaDisconnect
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax


:StaDisconnect
netsh wlan delete profile name="*"


if /I (%3)==(/NoDisable) Goto Success
if "%3"=="" GOTO DisEnaDriver
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
echo:Example = Try - "sta_disconnect /interface <interface name> [/NoDisable]"
echo:
echo:Suggested Error: %SuggestedError%
SETX WFA_CLI_STATUS 2
Exit /B 2


:DevconFail
echo:
echo:Device Disable/Enable failed
echo: 
SETX WFA_CLI_STATUS 1
Exit /B 2

:Success
echo:
echo: STA is disconnected
echo: 
SETX WFA_CLI_STATUS 0
goto :eof

Exit /B 2