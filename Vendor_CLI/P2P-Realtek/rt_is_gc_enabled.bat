@echo off
call rt_sta_get_p2p_cap.bat
::echo dc: %dc%
::echo gc: %gc%

set GcToTest=%1
set /A temp="%gc%&%GcToTest%"
if NOT %temp%==0 (
	set RT_CLI_RETURN=1
	goto exit
)

set RT_CLI_RETURN=0

:exit
