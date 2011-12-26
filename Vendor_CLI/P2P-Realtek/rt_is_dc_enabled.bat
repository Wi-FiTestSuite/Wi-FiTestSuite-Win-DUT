@echo off
call rt_sta_get_p2p_cap.bat
::echo dc: %dc%
::echo gc: %gc%

set DcToTest=%1
set /A temp="%dc%&%DcToTest%"
if NOT %temp%==0 (
	set RT_CLI_RETURN=1
	goto exit
)

set RT_CLI_RETURN=0

:exit
