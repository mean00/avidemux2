@echo off

echo MSYS build for win-iconv
echo ========================
echo 1. 32-bit build
echo 2. 64-bit build
echo X. Exit
echo.

choice /c 12x

if errorlevel 1 set BuildBits=32
if errorlevel 2 set BuildBits=64
if errorlevel 3 goto :eof

verify >nul
call "../Set Common Environment Variables"
if errorlevel 1 goto end

set version=20100912
set package=win-iconv-dev_tml-%version%_win64.zip
set sourceFolder=win-iconv-%version%-%BuildBits%
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://ftp.gnome.org/pub/gnome/binaries/win64/dependencies/%package%/
)

if errorlevel 1 goto error

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto error

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto error

cd "%devDir%\%sourceFolder%"
unzip "%curDir%\%package%"
if errorlevel 1 goto error

gcc %CFLAGS% -O3 -o win_iconv.win64.o -c src/tml/win_iconv/win_iconv.c
if errorlevel 1 goto error

ar crv "%usrLocalDir%/lib/libiconv.a" win_iconv.win64.o
if errorlevel 1 goto error

copy src\tml\win_iconv\iconv.h "%usrLocalDir%/include"
if errorlevel 1 goto error

goto end

:error
echo Error

:end
pause