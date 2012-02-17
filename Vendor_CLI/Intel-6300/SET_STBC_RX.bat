@Echo OFF
Set ERRORLEVEL=0
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForStreams
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForStreams
if /I (%3)==(/streams) Goto CheckStreamsValue
Set SuggestedError=/streams was not inserted or misspelled
Goto InvalidSyntax

:CheckStreamsValue
if not (%4) GEQ (0) Goto StreamsSyntaxError
if (%4) LEQ (1) Goto SetRegKey
if (%4) LEQ (4) Goto NotSupportedNumOfStreams

:StreamsSyntaxError
Set SuggestedError=Streams value was not inserted or misspelled
Goto InvalidSyntax

:SetRegKey
rem Set flag = %4  	rem disable for 0 streams and enable for 1 stream
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp

REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v rxStbc /t REG_DWORD /d %4 /f

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
echo:Example = Try - "SET_STBC_RX /interface <interface name> /Streams <0-4> [/NoDisable]"
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

:NotSupportedNumOfStreams
echo:2 or more streams are not supported 
SET WFA_CLI_STATUS=1
Exit /B 2

:Success
echo:
echo:Rx STBC capabilities are now set to %4% Antenna(s)
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2