::@Echo OFF

Set /a ProvisioningTimeoutSec=120

Set suggestedError=shani
if /I (%1)==(/interface) Goto CheckCurrentRole
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

: InitVariables
SET oper_chn=-1

:CheckCurrentRole
call rt_get_current_role.bat
set Role=%RT_CLI_RETURN%
ECHO  sta_p2p_start_group_formation.bat: current role: %Role%

:: none
if %Role% EQU 0 goto IncorrectCurrentRole
:: client
if %Role% EQU 2 goto IncorrectCurrentRole
:: go
if %Role% EQU 3 goto IncorrectCurrentRole

:: device => OK
if %Role% EQU 1 goto CheckInterface

:IncorrectCurrentRole
ECHO  sta_p2p_start_group_formation.bat: incorrect current role: %Role%, should be P2P Device (1)
Exit /B 2

:CheckInterface
if /I (%2)==(rtl8192s) Goto CheckDevId
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckDevId
if /I (%3)==(/p2pdevid) Goto ExtractDevIdStr
Set SuggestedError=/p2pdevid was not inserted or misspelled
Goto InvalidSyntax

:ExtractDevIdStr
::
:: Translate from 00:e0:4c:72:00:64 to 00 e0 4c 72 00 64
::

:: P2PDevID is the GO DevAddr if we become Client after formation
SET P2PDevID=%4

SET DevIdStr=""
SET temp2=%4

SET temp=%temp2:~0,2%
SET DevIdStr=%temp%

SET temp=%temp2:~3,2%
SET DevIdStr=%DevIdStr% %temp%

SET temp=%temp2:~6,2%
SET DevIdStr=%DevIdStr% %temp%

SET temp=%temp2:~9,2%
SET DevIdStr=%DevIdStr% %temp%

SET temp=%temp2:~12,2%
SET DevIdStr=%DevIdStr% %temp%

SET temp=%temp2:~15,2%
SET DevIdStr=%DevIdStr% %temp%

ECHO  sta_p2p_start_group_formation.bat: DevId to connect: %DevIdStr%


:CheckIntentVal
if /I (%5)==(/intent_val) goto SetIntent
Set SuggestedError=/intent_val was not inserted or misspelled
Goto InvalidSyntax

:SetIntent
call dec_to_hex.bat %6 1 > temp_groupform.txt
SET /p HexIntent=<temp_groupform.txt
del temp_groupform.txt
ECHO  sta_p2p_start_group_formation.bat: Set intent to hex value: %HexIntent%
call d.exe -s FF010202 %HexIntent%
Goto CheckInitGoNeg

:CheckInitGoNeg
if /I (%7)==(/init_go_neg) (
	set init_go_neg=%8
	goto CheckOperChn
)
Set SuggestedError=/init_go_neg was not inserted or misspelled
Goto InvalidSyntax

:CheckOperChn
:: oper_chn is optional
if /I (%9)==(/oper_chn) GOTO SetOperChn
GOTO SetInitGoNeg

:SetOperChn
:: to deal with the 10th argument
shift

set oper_chn=%9
ECHO oper_chn=%oper_chn%
call rt_set_oper_chn.bat %oper_chn%

:SetInitGoNeg
if NOT %init_go_neg% EQU 0 GOTO DoDrvAccess
:: INIT_GO_NEG==0, we just have to be ready for negotiation
SETX WFA_CLI_RETURN Result,,GroupID,,
SETX WFA_CLI_STATUS 0

:: why do we need this?
:: add delay
call rt_delay_sec.bat 5

exit /B 2

:DoDrvAccess
ECHO sta_p2p_start_group_formation.bat: Connect to: %DevIdStr%
::OID_RT_P2P_CONNECT_REQUEST
d.exe -s FF0101FC %DevIdStr%>DrvAccessRes

:: Check whether d.exe fails, probably the target is not in our scan list
findstr /I /C:"failed" DrvAccessRes
::del DrvAccessRes
if %ERRORLEVEL% EQU 0 GOTO DrvAccessFail

Set /a counter=0
ECHO  sta_p2p_start_group_formation.bat: Wait for negotiation

:WaitForNegotiation
::OID_RT_P2P_MODE: 0: none, 1: dev, 2:client, 3: go
:: Get current role
call rt_get_current_role.bat
set Role=%RT_CLI_RETURN%
Echo sta_p2p_start_group_formation.bat: current role: %Role%

set /a counter=%counter%+1

:: Wait until 8 retries or Role becomes something that is not device address
if %counter% EQU 24 goto NegotiationFailed
if %Role% EQU 1 (
	call rt_delay_sec.bat 1
	goto WaitForNegotiation)
if %Role% GEQ 2 goto NegotiationDone
	
:NegotiationFailed	
ECHO sta_p2p_start_group_formation.bat: Negotiation failed or timeout
goto NegotiationFailed


:NegotiationDone
Set /a counter=0

ECHO sta_p2p_start_group_formation.bat: Wait for provisioning
:WaitForProvisioning
::
:: We decide whether provisioning is done by checking whether SSID is available
::

:: Timeout mechanism
set /a bProvisioningTimeout=0
set /a counter=%counter%+1
if %counter% EQU %ProvisioningTimeoutSec% (
	set /a bProvisioningTimeout=1
	goto ProvisioningDone
)

:: Get role for calling rt_get_ssid.bat
if %Role% EQU 2 SET RoleStr=client
if %Role% EQU 3 SET RoleStr=go

call rt_get_ssid.bat %RoleStr%
set SsidStr=%RT_CLI_RETURN%


if (%SsidStr%)==() (
	call rt_delay_sec.bat 1
	GOTO WaitForProvisioning)

echo SsidStr=%SsidStr%
	
goto ProvisioningDone

:ProvisioningDone
if %bProvisioningTimeout% EQU 1 goto ProvisioningTimeout
echo Provisioning Done

::
:: Return: [go, client] [GO DevAddr] [SSID]
:: Eg: result,go,groupid,00:e0:4c:74:00:08 DIRECT-xx
::
Echo sta_p2p_start_group_formation.bat: Negotiation Done, role: %Role%

SET ReturnStr=

:: Role
SET ReturnStr=Result,%RoleStr%
echo 1: %ReturnStr%

:: GO DevAddr

:: if we are Client, the GO DevAddr would be P2PDevID
if /I (%RoleStr%)==(client) (
	SET ReturnStr=%ReturnStr%,GroupID,%P2PDevID%
	goto AppendSsidStr
)

:: if we are GO, the GO DevAddr would be our own DevAddr
if /I (%RoleStr%)==(go) goto GetGODevAddr


:GetGODevAddr
call sta_get_p2p_dev_address.bat /interface rtl8192s
echo sta_get_p2p_dev_address returns:%RT_CLI_RETURN%
SET ReturnStr=%ReturnStr%,GroupID,%RT_CLI_RETURN%
	
:AppendSsidStr

SET ReturnStr=%ReturnStr% %SsidStr%
echo WFA_CLI_RETURN: %ReturnStr%

SETX WFA_CLI_RETURN "%ReturnStr%"
SETX WFA_CLI_STATUS 0
ECHO sta_p2p_start_group_formation.bat: return str: %WFA_CLI_RETURN%

:: add delay
call rt_delay_sec.bat 5

Exit /B 2


:InvalidSyntax
echo:syntax Error
echo:
echo:Example = Try - "sta_p2p_start_group_formation.bat /interface <interface name> /p2pdevid <P2P Device ID> /intent_val <Intent Value> /oper_chn <channel>"
echo:
echo:Suggested Error: %SuggestedError%
SETX WFA_CLI_STATUS 2
Exit /B 2


:DrvAccessFail
echo:
echo:
echo: Driver Access Operation fail
echo: 
SETX WFA_CLI_STATUS 1
Exit /B 2

:NegotiationFailed
echo:
echo:
echo: Negotiation Failed
echo: 
:: 'FAIL' is returned only in the case where GO Nego fails due to 2 STAs having the GO intent value 15
SETX WFA_CLI_RETURN Result,FAIL
SETX WFA_CLI_STATUS 1
Exit /B 2

:ProvisioningTimeout
echo:
echo:
echo: Provisioning Timeout
echo: 
:: 'FAIL' is returned only in the case where GO Nego fails due to 2 STAs having the GO intent value 15
SETX WFA_CLI_RETURN Result,FAIL
SETX WFA_CLI_STATUS 1
Exit /B 2
