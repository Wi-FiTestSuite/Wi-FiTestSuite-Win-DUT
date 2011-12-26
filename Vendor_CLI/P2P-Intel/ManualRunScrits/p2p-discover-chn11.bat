FOR /F "tokens=2 delims==" %%i in ('findstr DEVICE c:\wfa\WfaEndpoint\sigma_settings.txt') Do set DEVICE=%%i
sta_set_p2p /interface %DEVICE% /p2p_mode discover /listen_chn 11 /persistent 0