@echo off

call dec_to_hex.bat %1 1 >temp_set_listen_chn.txt
SET /p HexListenChnl=<temp_set_listen_chn.txt
ECHO Set listen channel to hex value: %HexListenChnl%
d.exe -s FF010209 %HexListenChnl%
del temp_set_listen_chn.txt
