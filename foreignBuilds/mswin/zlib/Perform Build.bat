@echo off

echo MSYS build for zlib
echo ===================
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

set version=1.2.7
set package=zlib-%version%.tar.gz
set sourceFolder=zlib-%version%-%BuildBits%
set tarFolder=zlib-%version%
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://zlib.net/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfz "%package%" -C "%devDir%\\%sourceFolder%"
if errorlevel 1 goto end
cd "%devDir%\%sourceFolder%"

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
  move "%CD%\%tarFolder%\%%a" "%CD%"
)

if "%BuildBits%" == "32" set RC=RC="windres -F pe-i386"
if "%BuildBits%" == "64" set RC=RC="windres -F pe-x86-64"
mingw32-make -f win32/Makefile.gcc CFLAGS="%CFLAGS%" LDFLAGS="%LDFLAGS%" %RC%
if errorlevel 1 goto end

copy zlib1.dll "%usrLocalDir%\bin"
copy zconf.h "%usrLocalDir%\include"
copy zlib.h "%usrLocalDir%\include"
copy libz.a "%usrLocalDir%\lib"
copy libz.dll.a "%usrLocalDir%\lib"

copy "%usrLocalDir%\bin\zlib1.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause