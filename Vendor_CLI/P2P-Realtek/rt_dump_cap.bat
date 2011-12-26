@echo off

set dc=
set gc=

call rt_sta_get_p2p_cap.bat
::echo dc: %dc%
::echo gc: %gc%



::
:: some of the bits such as go in group cap is set on the fly,
:: so they are not correct if we use oid to query the cap.
::

echo ======== Device Capability ========
echo SUPPORTED:

set /A temp2="%dc%&1"
if NOT %temp2%==0 echo service discovery (1)

set /A temp2="%dc%&2"
if NOT %temp2%==0 echo p2p client discoverability (2)

set /A temp2="%dc%&4"
if NOT %temp2%==0 echo concurrent operation (4)

set /A temp2="%dc%&8"
if NOT %temp2%==0 echo p2p infrastructure managed (8)

set /A temp2="%dc%&16"
if NOT %temp2%==0 echo p2p device limit (16)

set /A temp2="%dc%&32"
if NOT %temp2%==0 echo p2p invitation procedure (32)

echo. 
echo UNSUPPORTED:

set /A temp2="%dc%&1"
if %temp2%==0 echo service discovery (1)

set /A temp2="%dc%&2"
if %temp2%==0 echo p2p client discoverability (2)

set /A temp2="%dc%&4"
if %temp2%==0 echo concurrent operation (4)

set /A temp2="%dc%&8"
if %temp2%==0 echo p2p infrastructure managed (8)

set /A temp2="%dc%&16"
if %temp2%==0 echo p2p device limit (16)

set /A temp2="%dc%&32"
if %temp2%==0 echo p2p invitation procedure (32)

echo.
echo ======== Group Capability ========
echo SUPPORTED:

set /A temp="%gc%&1"
if NOT %temp%==0 echo acting as GO (1)

set /A temp="%gc%&2"
if NOT %temp%==0 echo persistent (2)

set /A temp="%gc%&4"
if NOT %temp%==0 echo group limit (4)

set /A temp="%gc%&8"
if NOT %temp%==0 echo intra-bss distribution (8)

set /A temp="%gc%&16"
if NOT %temp%==0 echo cross connection (16)

set /A temp="%gc%&32"
if NOT %temp%==0 echo persistent reconnect (32)

set /A temp="%gc%&64"
if NOT %temp%==0 echo group formation (64)

echo.
echo UNSUPPORTED:

set /A temp="%gc%&1"
if  %temp%==0 echo acting as GO (1)

set /A temp="%gc%&2"
if  %temp%==0 echo persistent (2)

set /A temp="%gc%&4"
if  %temp%==0 echo group limit (4)

set /A temp="%gc%&8"
if  %temp%==0 echo intra-bss distribution (8)

set /A temp="%gc%&16"
if  %temp%==0 echo cross connection (16)

set /A temp="%gc%&32"
if  %temp%==0 echo persistent reconnect (32)

set /A temp="%gc%&64"
if  %temp%==0 echo group formation (64)

echo.
