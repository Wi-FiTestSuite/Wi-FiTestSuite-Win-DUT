@Echo OFF
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForAction
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForAction
if /I (%3)==(/TID) Goto CheckActionValue
Set SuggestedError=/action type "TID" was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if (%4) GEQ (0) Goto aboveZeroOk
Set SuggestedError=TID should be an Integer between 0-7, seem like it is below the limits
Goto InvalidSyntax

:aboveZeroOk
if (%4) LEQ (7) SET TID=%4%
if (%4) LEQ (7) GOTO DOIwl
Set SuggestedError=TID should be an Integer between 0-7, seem like it is above the limits, or was not typed right
Goto InvalidSyntax

:DOIwl
iwl.exe -addba -tid %TID% > temp2
findstr /C:"Unable to perform action" temp2 > temp3
set /p myOutput2=<temp3
del temp2
del temp3
if "%myOutput2%"=="Unable to perform action" Goto IwlFail
echo:
echo: ADDBA With TID %TID% was sent succesfully
SET WFA_CLI_STATUS=0
Exit /B 2



:InvalidSyntax
echo:syntax Error
echo:
echo:Example = Try - "SEND_ADDBA /interface <interface name> /TID [0-7]"
echo:
echo:Suggested Error: %SuggestedError%
SET WFA_CLI_STATUS=2
Exit /B 2


:IwlFail
echo:
echo:
echo:IWL Operation fail, make sure you're associated and Winpcap is installed, reboot may be required
echo: 
SET WFA_CLI_STATUS=1
Exit /B 2

