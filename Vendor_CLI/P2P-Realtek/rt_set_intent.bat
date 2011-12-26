@echo off

call dec_to_hex.bat %1 1 > temp.txt
SET /p HexIntent=<temp.txt
ECHO Set intent to hex value: %HexIntent%
d.exe -s FF010202 %HexIntent%
