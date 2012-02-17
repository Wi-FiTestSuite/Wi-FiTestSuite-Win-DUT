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
if /I (%3)==(/Mode) Goto CheckActionValue
Set SuggestedError=/Mode was not inserted or misspelled
Goto InvalidSyntax

:CheckActionValue
if /I (%4)==(BO) Goto SetBonly
if /I (%4)==(BG) Goto SetBGonly
if /I (%4)==(AO) Goto SetAonly
if /I (%4)==(NA) Goto SetNAonly
if /I (%4)==(NG) Goto SetNGonly
if /I (%4)==(NL) Goto SetNL
Set SuggestedError=Mode type was not inserted or misspelled
Goto InvalidSyntax

:SetBonly
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v WirelessMode /t REG_SZ /d 0 /f
Goto CheckForDisable

:SetBGonly
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 2 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v WirelessMode /t REG_SZ /d 32 /f
Goto CheckForDisable

:SetAonly
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v WirelessMode /t REG_SZ /d 17 /f
Goto CheckForDisable

:SetNAonly
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v WirelessMode /t REG_SZ /d 17 /f
Goto CheckForDisable

:SetNGonly
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 0 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 2 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v WirelessMode /t REG_SZ /d 32 /f
Goto CheckForDisable

:SetNL
Set temp=
RegEditSigma.exe "Intel(R) Centrino(R) Ultimate-N 6300 AGN" > temp
set /P myOutput=<temp
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v BandType /t REG_DWORD /d 2 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v ModulationType /t REG_DWORD /d 2 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v IEEE11nMode /t REG_SZ /d 1 /f
REG ADD HKEY_LOCAL_MACHINE\%myOutput% /v WirelessMode /t REG_SZ /d 34 /f
Goto CheckForDisable

:CheckForDisable
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
echo:Example = Try - "STA_SET_MODE /interface <interface name> /Mode <BO/BG/AO/NA/NG/NL> [/NoDisable]"
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
echo:Band mode was set to %4%
echo: 
SET WFA_CLI_STATUS=0
goto :eof

Exit /B 2