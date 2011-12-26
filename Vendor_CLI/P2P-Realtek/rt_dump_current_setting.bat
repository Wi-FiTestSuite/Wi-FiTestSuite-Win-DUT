@echo off

:: Role
call rt_get_current_role.bat
set CurRole=%RT_CLI_RETURN%

:: Intent
call rt_get_intent.bat
set CurIntent=%RT_CLI_RETURN%

:: GO SSID
call rt_get_go_ssid.bat
set CurGOSsid=%RT_CLI_RETURN%

:: listen chn
call rt_get_listen_chn.bat
set CurListenChn=%RT_CLI_RETURN%

:: oper chn
call rt_get_oper_chn.bat
set CurOperChn=%RT_CLI_RETURN%

ECHO.
ECHO %0: Current Settings:
ECHO Role: %CurRole%
ECHO Intent: %CurIntent%
ECHO GOSsid: %CurGOSsid%
ECHO ListenChn: %CurListenChn%
ECHO OperChn: %CurOperChn%
call rt_dump_cap.bat
ECHO.