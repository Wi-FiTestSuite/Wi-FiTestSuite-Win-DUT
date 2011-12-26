@Echo OFF

Set suggestedError=shani
if /I (%1)==(/interface) Goto CheckInterface
Set SuggestedError=/interface was not inserted or misspelled
Goto InvalidSyntax


:CheckInterface
if /I (%2)==(rtl8192s) Goto CheckInterval
Set SuggestedError=interface value was not inserted or misspelled
Goto InvalidSyntax


:CheckInterval
if /I (%3)==(/interval) Goto GetInterval
Set SuggestedError=/interval was not inserted or misspelled
Goto InvalidSyntax

:GetInterval
set interval=%4

call dec_to_hex.bat %interval% 2 > temp_stasendprobereq
SET /p hex_interval=<temp_stasendprobereq
del temp_stasendprobereq
echo hex_interval=%hex_interval%

:CheckPeriod
if /I (%5)==(/period) Goto GetPeriod
Set SuggestedError=/period was not inserted or misspelled
Goto InvalidSyntax

:GetPeriod
set period=%6

call dec_to_hex.bat %period% 2 > temp_stasendprobereq
SET /p hex_period=<temp_stasendprobereq
del temp_stasendprobereq
echo hex_period=%hex_period%


:DoDrvAccess
:: OID_RT_P2P_EXTENDED_LISTEN_TIMING
d.exe -s ff01020a 01 %hex_interval% %hex_period%


:PrepareReturnStr
SETX WFA_CLI_STATUS 0
Exit /B 2


:InvalidSyntax
echo:syntax Error
echo:
echo:Example = Try - "%0: /interface <interface name>"
echo:
echo:Suggested Error: %SuggestedError%
SETX WFA_CLI_STATUS 2
Exit /B 2

