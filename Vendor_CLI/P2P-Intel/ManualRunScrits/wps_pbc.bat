FOR /F "tokens=2 delims==" %%i in ('findstr DEVICE c:\wfa\WfaEndpoint\sigma_settings.txt') Do set DEVICE=%%i
sta_set_wps_pbc /interface %DEVICE%
