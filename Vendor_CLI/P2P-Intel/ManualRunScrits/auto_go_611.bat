FOR /F "tokens=2 delims==" %%i in ('findstr DEVICE c:\wfa\WfaEndpoint\sigma_settings.txt') Do set DEVICE=%%i
sta_start_autonomous_go /interface %DEVICE% /oper_chn 6 /SSID 6.1.1