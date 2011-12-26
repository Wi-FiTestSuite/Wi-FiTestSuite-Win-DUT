@echo off

call dec_to_hex.bat %1 1 > setoperchntemp.txt
SET /p HexOperChnl=<setoperchntemp.txt
ECHO Set operating channel to hex value: %HexOperChnl%
d.exe -s FF010208 %HexOperChnl%
del setoperchntemp.txt
