::@echo off

set DcToSet=%1

::
:: If the capability has already been disabled, ignore the action
::
call rt_is_dc_enabled.bat %DcToSet%
if (%RT_CLI_RETURN%)==(0) goto exit

::
:: Get the current capability and OR it with the capability to set
::
call rt_sta_get_p2p_cap.bat
set /A dc=%dc%-%DcToSet%
echo dc updated to:%dc%

call dec_to_hex.bat %dc% 1 > temp_rt_disable_dc.txt
SET  /p HexDc=<temp_rt_disable_dc.txt
del temp_rt_disable_dc.txt

call dec_to_hex.bat %dc% 1 > temp_rt_disable_dc.txt
SET /p HexDc=<temp_rt_disable_dc.txt
del temp_rt_disable_dc.txt

echo hexdc:%HexDc%, hexgc:%HexGc%
d.exe -s FF010205 %HexDc% %HexGc%

:exit
