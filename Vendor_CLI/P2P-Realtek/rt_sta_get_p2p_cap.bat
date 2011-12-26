@echo off

:: Get GC and DC
d -g ff010205 > temp 
findstr /I /R /C:"^[0-9A-F][0-9A-F] [0-9A-F][0-9A-F]" temp>temp2
set /p temp3=<temp2
::echo %temp3%


set hexdc=%temp3:~0,2%
set hexgc=%temp3:~3,2%

:: Convert from hex to decimal
set /A gc=0x%hexgc%
set /A dc=0x%hexdc%

::echo rt_sta_get_p2p_cap: dc: %dc%
::echo rt_sta_get_p2p_cap: gc: %gc%

del temp
del temp2