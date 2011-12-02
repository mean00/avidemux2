@echo off

echo MSYS build for LAME
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

set version=3.99
set package=lame-%version%.tar.gz
set sourceFolder=lame-%version%-%BuildBits%
set tarFolder=lame-%version%
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://sourceforge.net/projects/lame/files/lame/%version%/%package%/download/
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfz "%package%" -C "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

cd "%devDir%\%sourceFolder%"

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
  move "%CD%\%tarFolder%\%%a" "%CD%"
)

echo.
echo Patching
patch -p0 -i "%curDir%\vbrquantize.c.patch"

echo.
echo Configuring
set CFLAGS=%CFLAGS% -O3

if "%BuildBits%" == "32" sh ./configure --prefix="%usrLocalDir%" --disable-static --enable-nasm
if "%BuildBits%" == "64" sh ./configure --prefix="%usrLocalDir%" --disable-static

if errorlevel 1 goto end
echo.
pause

make install
if errorlevel 1 goto end

strip "%usrLocalDir%\bin\libmp3lame-0.dll"
copy "%usrLocalDir%\bin\libmp3lame-0.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause