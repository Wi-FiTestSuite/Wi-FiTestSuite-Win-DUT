FOR /F "tokens=2 delims==" %%i in ('findstr DEVICE c:\wfa\WfaEndpoint\sigma_settings.txt') Do set DEVICE=%%i
sta_p2p_start_group_formation /interface %DEVICE% /p2pdevid %1 /intent_val 15 /init_go_neg 1 /oper_chn 11
