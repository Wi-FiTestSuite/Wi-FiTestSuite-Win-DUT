@Echo OFF
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax

:CheckInterface
if(%2)==(6300) Goto CheckForAction
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax

:CheckForAction
if /I (%3)==(/intolerant) Goto IntValue
Set SuggestedError=action type "/intolerant" was not inserted or misspelled
Goto InvalidSyntax

:IntValue
if /i (%4)==(Enable) Set into=1
if /i (%4)==(Disable) Set into=0
if /i (%4)==(Enable) goto 20Req
if /i (%4)==(Disable) goto 20Req
Set SuggestedError=Intolerant action value can be only Disable or Enable
Goto InvalidSyntax

:20Req
if /i (%5)==(/20req) Goto 20ReqValue
Set SuggestedError=action type "/20req" was not inserted or misspelled
Goto InvalidSyntax

:20ReqValue
if /i (%6)==(Enable) Set ReqValue=1
if /i (%6)==(Disable) Set ReqValue=0
if /i (%6)==(Enable) goto RegReport
if /i (%6)==(Disable) goto RegReport
Set SuggestedError=20Req value can only be Disable or Enable
Goto InvalidSyntax


:RegReport
if /i (%7)==(/RegulatoryReport) goto RegReportValue
if /i "%7"=="" goto DoIwlNoReg
Set SuggestedError=action type "/RegulatoryReport" was misspelled
Goto InvalidSyntax


:RegReportValue
if /i "%8"=="" Set SuggestedError=syntax of Regulatory report should be as followed: Class:Channel
if /i "%8"=="" Goto InvalidSyntax
set ReportValue=%8
Goto DoIwl

:DOIwl
iwl -infoReq 0 -intolerant %into% -20Req %ReqValue% -regulatory %ReportValue% > temp2
findstr /C:"Unable to perform action" temp2 > temp3
set /p myOutput2=<temp3
del temp2
del temp3
if "%myOutput2%"=="Unable to perform action" Goto IwlFail
echo:
echo: 40Mhz Coex action frame was sent succesfully
SET WFA_CLI_STATUS=0
Exit /B 2


:DOIwlNoReg
echo: iwl -infoReq 0 -intolerant %into% -20Req %ReqValue%
iwl -infoReq 0 -intolerant %into% -20Req %ReqValue% > temp2
findstr /C:"Unable to perform action" temp2 > temp3
set /p myOutput2=<temp3
del temp2
del temp3
if "%myOutput2%"=="Unable to perform action" Goto IwlFail
SET WFA_CLI_STATUS=0
Exit /B 2



:InvalidSyntax
echo: 
echo:syntax Error 
echo:
echo:Example = Try - "SET_COEXIST_MGMT /interface <interface name> /intolerant <Enable,Disable> /20req <Enable,Disable> /RegulatoryReport <Class:Channel>"
echo: 
echo: RegulatoryReport is optional, should be used only if channel report list is needed
echo:
echo: Suggested error: %SuggestedError%
SET WFA_CLI_STATUS=2
Exit /B 2


:IwlFail
echo:
echo:
echo:IWL Operation fail, make sure you're associated and Winpcap is installed, reboot may be required
echo: 
SET WFA_CLI_STATUS=1
Exit /B 2

