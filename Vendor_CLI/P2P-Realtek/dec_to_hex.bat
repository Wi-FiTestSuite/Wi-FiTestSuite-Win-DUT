@ECHO OFF

:: Windows NT / 2000 / XP only
IF NOT "%OS%"=="Windows_NT" GOTO Syntax

:: Two command line parameter only
IF     "%~1"=="" GOTO Syntax
IF     "%~2"=="" GOTO Syntax

:: ResultDataLength is in number of bytes
set ResultDataByteLen=%2

:: Display help if requested
ECHO "%~1" | FIND "?" >NUL
IF NOT ERRORLEVEL 1 GOTO Syntax
ECHO "%~1" | FIND "/" >NUL
IF NOT ERRORLEVEL 1 GOTO Syntax

:: Enable delayed variable expansion
SETLOCAL ENABLEDELAYEDEXPANSION
:: Check if command line parameter is a valid number
SET /A X = %1 >NUL 2>&1
IF NOT "%X%"=="%~1" (
	ENDLOCAL
	GOTO Syntax
)
IF %1 LSS 0 (
	ENDLOCAL
	GOTO Syntax
)

SET Hex=
SET Cvt=0123456789ABCDEF

SET /A Tmp0 = %1

:Loop
:: Get the last (least significant) digit
SET /A Tmp1 = %Tmp0% / 16
SET /A Tmp2 = %Tmp0% - 16 * %Tmp1%
:: Convert that last digit (still decimal) to hexadecimal
SET Tmp2=!Cvt:~%Tmp2%,1!
:: Prepend it to the intermediate hexadecimal result
SET Hex=%Tmp2%%Hex%
:: Remove the last digit from the original number
SET Tmp0=%Tmp1%
:: If there are any digits left, loop once more
IF %Tmp0% GTR 0 GOTO Loop

::ECHO xxx:%Hex%

:LengthenResultIfNeeded

:CountCurrentHexLen
echo len:%Hex%>temp_dec_to_hex.txt
FOR %%A IN (temp_dec_to_hex.txt) DO SET HexLen=%%~zA
set /A HexLen=%HexLen%-6
DEL temp_dec_to_hex.txt
::ECHO %0: hex len: %HexLen%

:CountNumZerosToPrefix
SET /A ResultDataCharLen=%ResultDataByteLen%*2
:: Error: Overflow
IF %ResultDataCharLen% LSS %HexLen% (
	SET HexStr=
	ECHO Overflow: ResultDataCharLen=%ResultDataCharLen%, HexLen=%HexLen%
	GOTO Done
)
SET /A NumZerosToPrefix=%ResultDataCharLen%-%HexLen%
::ECHO NumZerosToPrefix: %NumZerosToPrefix%

:: No need to prefix zeros
IF %NumZerosToPrefix% EQU 0 (
		SET HexStr=%Hex%
		GOTO TranslateToLittleEndianAndInsertBlanks
)

:PrefixWithZeros
SET Hex=0%Hex%
SET /A NumZerosToPrefix=%NumZerosToPrefix%-1
IF NOT %NumZerosToPrefix% EQU 0 GOTO PrefixWithZeros
::ECHO After prefix zeros: %Hex%

:TranslateToLittleEndianAndInsertBlanks
SET HexStr=""
SET temp2=%Hex%
SET /A startchar=%ResultDataCharLen%-2
SET /A endchar=0

CALL SET temp=%%temp2:~%startchar%,2%%
SET HexStr=%temp%
set /A startchar=%startchar%-2

if %startchar% LSS %endchar% goto Done

:Loop2
CALL SET temp=%%temp2:~%startchar%,2%%
SET HexStr=%HexStr% %temp%
::ECHO HexStr: %HexStr%
set /A startchar=%startchar%-2
if %startchar% GEQ %endchar% goto Loop2

:Done
ECHO %HexStr%
ENDLOCAL

Exit /B 2

:Syntax
Echo %0: Syntax error