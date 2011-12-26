@echo off

set GcToSet=%1

::
:: If the capability has already been disabled, ignore the action
::
call rt_is_gc_enabled.bat %GcToSet%
if (%RT_CLI_RETURN%)==(0) goto exit

::
:: Get the current capability and OR it with the capability to set
::
call rt_sta_get_p2p_cap.bat
set /A gc=%gc%-%GcToSet%
echo gc updated to:%gc%

call dec_to_hex.bat %gc% 1 > temp_rt_disable_gc.txt
SET /p HexGc=<temp_rt_disable_gc.txt
del temp_rt_disable_gc.txt

call dec_to_hex.bat %dc% 1 > temp_rt_disable_gc.txt
SET /p HexDc=<temp_rt_disable_gc.txt
del temp_rt_disable_gc.txt

echo hexdc:%HexDc%, hexgc:%HexGc%
d.exe -s FF010205 %HexDc% %HexGc%

:exit
