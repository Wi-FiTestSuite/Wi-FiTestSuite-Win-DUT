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
if (%3)==(/set) Goto CheckActionValue
Set SuggestedError=/set was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if /I (%4)==(11n) Goto SetRegKeyDefault
if /I (%4)==(NoOptional) Goto setRegKeyNoOptional
Set SuggestedError=Set mode can only be 11n or NoOptional
Goto InvalidSyntax

:SetRegKeyDefault
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ChannelWidth24 /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ChannelWidth52 /t REG_SZ /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v fatChannelIntolerant /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v tlcAutoAggregation /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v rxAddbaDisable /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ShortGICapabilities /t REG_DWORD /d 3 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v GreenFieldCapable /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 2 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 2 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v TGeEnableBits /t REG_DWORD /d 27 /f


Set Status=Device was set back to default 11n settings
del temp
Goto DisEnaDriver

:SetRegKeyNoOptional
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Advanced-N 6200 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ChannelWidth24 /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ChannelWidth52 /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v fatChannelIntolerant /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v tlcAutoAggregation /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v rxAddbaDisable /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ShortGICapabilities /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v GreenFieldCapable /t REG_DWORD /d 0 /f
Set Status=Device was set to disable all optional features
del temp
Goto DisEnaDriver


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
echo:Example = Try - "RESET_DEFAULT /interface <interface name> /set <11n,NoOptional>"
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
echo:%status%
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2