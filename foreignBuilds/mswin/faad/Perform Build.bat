@echo off

echo MSYS build for FAAD
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

set package=faad2-2.7.tar.gz
set sourceFolder=faac-2.7-%BuildBits%
set tarFolder=faad2-2.7
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://downloads.sourceforge.net/faac/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfz "%package%" -C "%devDir%/%sourceFolder%"
if errorlevel 1 goto end

cd "%devDir%\%sourceFolder%"

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
  move "%CD%\%tarFolder%\%%a" "%CD%"
)

copy ".\include\*.h" "%usrLocalDir%\include"
cd libfaad
gcc -s -O3 -DHAVE_MEMCPY=1 -DHAVE_STRING_H=1 -DHAVE_STDINT_H=1 -I../include -I"." *.c -shared -o "%usrLocalDir%/bin/libfaad2.dll" -Wl,--out-implib,"%usrLocalDir%/lib/libfaad.a"
if errorlevel 1 goto end

copy "%usrLocalDir%\bin\libfaad2.dll" "%admBuildDir%"
goto end

:error
echo Error

:end
pause