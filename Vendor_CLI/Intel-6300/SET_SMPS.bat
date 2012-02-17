@Echo OFF
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForAction
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForAction
if /I (%3)==(/mode) Goto CheckActionValue
Set SuggestedError=/action type "TID" was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue

set mimoPs=error
if /i (%4)==(Dynamic) Set mimoPs=dynamic
if /i (%4)==(Static) Set mimoPs=static
if /i (%4)==(NoLimit) Set mimoPs=full
if /i %mimoPs%==error goto InvalidSyntax
Goto DoIwl


:DOIwl
iwl -mimops %mimoPs% > temp2
findstr /C:"Unable to perform action" temp2 > temp3
set /p myOutput2=<temp3
del temp2
del temp3
if "%myOutput2%"=="Unable to perform action" Goto IwlFail
echo: 
echo:Spatial Multiplexing action frame with a value of %mimoPs% was sent
SET WFA_CLI_STATUS=0
Exit /B 2


:InvalidSyntax
echo: 
echo:syntax Error
echo:
echo:Example = Try - "SET_SMPS /interface <interface name> /mode <Dynamic,Static,NoLimit>"
SET WFA_CLI_STATUS=2
Exit /B 2


:IwlFail
echo:
echo:
echo:IWL Operation fail, make sure you're associated and Winpcap is installed, reboot may be required
echo: 
SET WFA_CLI_STATUS=1
Exit /B 2

