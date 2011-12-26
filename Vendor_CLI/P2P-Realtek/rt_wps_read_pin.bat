@echo off

call RtlUI2.exe -wps_read_pin > temp.txt
findstr /I /R /C:"^[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]" temp.txt>temp2.txt
set /p RT_CLI_RETURN=<temp2.txt
echo %0: PIN=%RT_CLI_RETURN%

del temp.txt
del temp2.txt