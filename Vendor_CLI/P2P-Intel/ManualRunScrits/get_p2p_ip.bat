FOR /F "tokens=2 delims==" %%i in ('findstr DEVICE c:\wfa\WfaEndpoint\sigma_settings.txt') Do set DEVICE=%%i
sta_get_p2p_ip_config /interface %DEVICE% /groupid %1 %2