@Echo OFF
Set suggestedError=shani
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForAction
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForAction
SET MCS=Error
SET McsValue=0
SET BW=0
SET BwValue=0
if /I (%3)==(/Fixedrate) Goto CheckFixedRateValue

if /I (%3)==(/mcs32) SET McsValue=%4
if /I (%3)==(/mcs32) SET BW=%5
if /I (%3)==(/mcs32) SET BwValue=%6
if /I (%3)==(/mcs32) Goto DoMcs

Set SuggestedError=/Fixedrate or /mcs32 were not inserted or misspelled
Goto InvalidSyntax

:CheckFixedRateValue
SET MCS=%4

if /I (%5)==(/mcs32) SET McsValue=%6
if /I (%5)==(/mcs32) SET BW=%7
if /I (%5)==(/mcs32) SET BwValue=%8
if /I (%5)==(/mcs32) Goto DoMcs
SET BW=%5
SET BwValue=%6
Goto CheckMcsValue


:DoMcs
if /I %McsValue%==Enable SET MCS=32
if /I %McsValue%==Enable Goto CheckMcsValue
if /I %McsValue%==Disable Goto CheckMcsValue
Set SuggestedError=/mcs32 value was not inserted or misspelle
Goto InvalidSyntax


:CheckMcsValue
if (%MCS%) GEQ (0) Goto aboveZeroOk
Set SuggestedError=MCS should be an Integer between 0-23
Goto InvalidSyntax


:aboveZeroOk
SET ANT=0
if %MCS% LEQ 7 SET ANT=a
if %MCS% LEQ 7 Goto width
if %MCS% LEQ 15 SET ANT=ab
if %MCS% LEQ 15 Goto width
if %MCS% LEQ 23 SET ANT=abc
if %MCS% LEQ 23 Goto Width
if %MCS%==32 SET ANT=a
if %MCS%==32 SET FAT=1
if %MCS%==32 Goto DoIwl

Set SuggestedError=MCS is not an Integer between 0-23 or /Fixedrare is not declared and /mcs32 is disable (if /mcs32 is disabled - /Fixedrare should be declared)
Goto InvalidSyntax

:Width
if "%BW%"=="" Set Fat=0
if "%BW%"=="" Goto DoIwl
if /I (%BW%)==(/BW) goto CheckWidth
Set suggestedError=/action type "BW" was misspelled
Goto InvalidSyntax

:CheckWidth
if /I (%BwValue%)==(40) set Fat=1
if /I (%BwValue%)==(20) set Fat=0
if /I (%BwValue%)==(40) Goto DoIwl
if /I (%BwValue%)==(20) Goto DoIwl
Set suggestedError=please make sure to choose BW is 20 or 40 
Goto InvalidSyntax


:DoIwl
iwl.exe -fixed -mcs %MCS% -ant %ANT% -fat %FAT%  > temp2
findstr /C:"Unable to perform action" temp2 > temp3
set /p myOutput2=<temp3

del temp2
del temp3
if "%myOutput2%"=="Unable to perform action" Goto IwlFail
echo:
echo: Fixed MCS %MCS% was set
SET WFA_CLI_STATUS=0
Exit /B 2



:InvalidSyntax
echo:syntax Error
echo:
echo:Example = Try - "SET_MCS /interface <interface name> /Fixedrate [0-23] /mcs32 [Enable,Disable] /BW [20,40]"
echo:
echo:/BW extension is only required for 40Mhz usage, default is 20Mhz
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

