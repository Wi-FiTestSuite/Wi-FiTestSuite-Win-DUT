@echo off
echo Installing Sigma Testbed Station Endpoint
echo -----------------------------------------
echo Searching for Testbed Station... Please wait... This may take few minutes

set DEST_PATH=C:\WFA\WfaEndpoint
set TEMP_PATH=C:\WFA\Temp
set WFA_LOG_PATH=C:\WFA\Logs

sc stop WfaControlAgent 
sc stop WfaDutAgent

IF EXIST %WFA_LOG_PATH% GOTO STEP0

mkdir %WFA_LOG_PATH%

:STEP0


IF EXIST C:\WFA\WfaEndpoint GOTO STEP1

mkdir %DEST_PATH%

:STEP1


xcopy /S /R /Y *.* %DEST_PATH%
cd %DEST_PATH%


setx PATH "%PATH%;%CD%" -m

sc stop WfaControlAgent 
sleep 5
sc delete WfaControlAgent 

sc stop WfaDutAgent
sleep 5
sc delete WfaDutAgent



IF EXIST C:\WFA\Temp GOTO STEP2 

mkdir %TEMP_PATH%

:STEP2

IF EXIST %DEST_PATH%\Interface.txt GOTO STEP3 

del /Q /F %DEST_PATH%\Interface.txt

:STEP3


netsh wlan show interfaces > %TEMP_PATH%\temp.txt

FOR /F "tokens=2,3,4,5 delims=: " %%i IN ('findstr Name %%TEMP_PATH%%\temp.txt') DO ( 
if  [%%l]==[] ( 
@echo IFNAME="%%i %%j %%k"> %DEST_PATH%\Interface.txt 
) else  ( 
@echo IFNAME="%%i %%j %%k %%l"> %DEST_PATH%\Interface.txt
)
)

IF EXIST %DEST_PATH%\Interface.txt GOTO STEP4 

echo "FATAL ERROR : No Wi-Fi Card Found - Installation failed"
PAUSE
Goto :EOF

:STEP4


WfaDutAgent -i
sc start WfaDutAgent 
sleep 5

WfaControlAgent -i
sc start WfaControlAgent 

echo "WFA STA Service installation completed...Restarting the machine"
PAUSE
#shutdown -r
exit
