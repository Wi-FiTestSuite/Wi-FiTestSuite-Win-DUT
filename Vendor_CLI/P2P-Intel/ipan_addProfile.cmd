@echo off

netsh wlan add profile filename="C:\WFA\CLIs\Intel\Wireless Network Connection-autobat_pan_test_5.1.6.xml" >> C:\Results.txt


REM set EXPPATH="C:\Program Files\Intel\WiFi\bin\mPROCmd.exe"
REM %EXPPATH% add_prof -prof_name Sweet_Auto99g -NetWorkName Sweet_Auto99g -Authentication WPA2_PSK -WEP aes_ccmp -PskPassphrase 12345678 >> C:\Results.txt
REM netsh wlan add profile filename="C:\WFA\CLIs\Intel\Wireless Network Connection-autobat_pan_test.xml" interface="Wireless Network Connection" user=all >> C:\Results.txt

findstr /m "added" C:\Results.txt
if %errorlevel%==0 (
echo Found SUCCESS in file setting enviornment variable to true
SETX WFA_CLI_STATUS 0
SETX WFAL_CLI_RETURN "Add profile Test PASSED"

)

if %errorlevel%==1 (
echo Couldn't find SUCCESS in file setting enviornment variable to false
SETX WFA_CLI_STATUS 1
SETX WFAL_CLI_RETURN "Add profile Test FAILED"
)

if %errorlevel%==2 (
echo Couldn't find correct parameters in file setting enviornment variable to false
SETX WFA_CLI_STATUS 2
SETX WFAL_CLI_RETURN "Add profile Test FAILED"
)

rem pause

del C:\Results.txt 