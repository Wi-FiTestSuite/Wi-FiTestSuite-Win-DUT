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
if (%3)==(/powersave) Goto CheckActionValue
Set SuggestedError=/set was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if /I (%4)==(On) Goto setPSPOLL
if /I (%4)==(Fast) Goto setPSNonPOLL
if /I (%4)==(Off) Goto setPSOff
Set SuggestedError=Set Powersave has a wrong value - On/Off/Fast
Goto InvalidSyntax

:setPSNonPOLL
Set temp1=
c:\Windows\System32\powercfg.exe -L | findstr /C:"*"| findstr /C:"GUID">temp1
FOR /F "tokens=4" %%i in (temp1) do set temp11=%%i
echo:%temp1%

c:\Windows\System32\powercfg.exe -Q | findstr /C:"(Wireless Adapter Settings)">temp2
FOR /F "tokens=3" %%i in (temp2) do set temp22=%%i

c:\Windows\System32\powercfg.exe -Q | findstr /C:"(Power Saving Mode)">temp3
FOR /F "tokens=4" %%i in (temp3) do set temp33=%%i

c:\Windows\System32\powercfg.exe -SETACVALUEINDEX %temp11% %temp22% %temp33% 003
c:\Windows\System32\powercfg.exe -SETDCVALUEINDEX %temp11% %temp22% %temp33% 003

Set Status=Device was set to PSNONPOLL
del temp1 temp2 temp3
Goto Success


:setPSOff
Set temp1=
c:\Windows\System32\powercfg.exe -L | findstr /C:"*"| findstr /C:"GUID">temp1
FOR /F "tokens=4" %%i in (temp1) do set temp11=%%i
echo:%temp1%

c:\Windows\System32\powercfg.exe -Q | findstr /C:"(Wireless Adapter Settings)">temp2
FOR /F "tokens=3" %%i in (temp2) do set temp22=%%i

c:\Windows\System32\powercfg.exe -Q | findstr /C:"(Power Saving Mode)">temp3
FOR /F "tokens=4" %%i in (temp3) do set temp33=%%i

c:\Windows\System32\powercfg.exe -SETACVALUEINDEX %temp11% %temp22% %temp33% 000
c:\Windows\System32\powercfg.exe -SETDCVALUEINDEX %temp11% %temp22% %temp33% 000

Set Status=Device was set to PowerSave OFF
del temp1 temp2 temp3
Goto Success


:setPSPOLL
Set Status=Device does not support PS POLL
Goto Success 


:InvalidSyntax
echo: syntax Error
echo:
echo:Example = Try - "SET_PowerSave /interface <interface name> /powersave <On/Off/Fast>"
echo:
echo:Suggested Error: %SuggestedError%
SET WFA_CLI_STATUS=2
Exit /B 2


:Success
echo:
echo:%Status%
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2