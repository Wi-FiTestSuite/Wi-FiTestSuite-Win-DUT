@echo off

d.exe -g FF010202 > temp_rt_get_intent.txt
findstr /I /R /C:"^[0-9A-F][0-9A-F]" temp_rt_get_intent.txt>temp2_rt_get_intent.txt

set temp3=

set /p temp3=<temp2_rt_get_intent.txt
::echo temp3=%temp3%

:: Get first two char
SET temp4=%temp3:~0,2%
::echo temp4=%temp4%

:: To decimal
SET /A temp5 = 0x%temp4%
::echo temp5=%temp5%

SET RT_CLI_RETURN=%temp5%
ECHO %0: intent=%RT_CLI_RETURN%

del temp_rt_get_intent.txt
del temp2_rt_get_intent.txt