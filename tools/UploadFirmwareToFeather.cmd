@echo off
setlocal

set firmwarePath="bin\%1"
echo %firmwarePath%

for /f "tokens=1* delims==" %%I in ('wmic path win32_pnpentity get caption  /format:list ^| find "COM5"') do (
    ::call :resetCOM "%%~J"
    set currentCOM=%%~J
)

goto :resetCOM

:continue

timeout /t 2

:: wmic /format:list strips trailing spaces (at least for path win32_pnpentity)
for /f "tokens=1* delims==" %%I in ('wmic path win32_pnpentity get caption  /format:list ^| find "USB Serial Device ("') do (
    ::call :setCOM "%%~J"
    set currentPORT=%%~J
)

goto :setCOM

:: end main batch
goto :flash

:resetCOM <WMIC_output_line>
:: sets _COM#=line
setlocal
set "str=%currentCOM%"
echo %str%
set "num=%str:*(COM=%"
set "num=%num:)=%"
set port=COM%num%
echo %port%
mode %port%: BAUD=1200 parity=N data=8 stop=1
goto :continue

:setCOM <WMIC_output_line>
:: sets _COM#=line
setlocal
set "str=%currentPORT%"
echo %str%
set "num=%str:*(COM=%"
set "num=%num:)=%"
set port=COM%num%
echo %port%
goto :flash

:flash
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avrdude" -v -CC:"\Program Files (x86)\Arduino\hardware\tools\avr\etc\avrdude.conf" -patmega32u4 -cavr109 -P%port% -b57600 -D -Uflash:w:%firmwarePath%:i
