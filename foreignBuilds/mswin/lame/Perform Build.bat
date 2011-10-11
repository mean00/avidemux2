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

set package=lame-3.98.4.tar.gz
set sourceFolder=lame-3.98.4-%BuildBits%
set tarFolder=lame-3.98.4
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://sourceforge.net/projects/lame/files/lame/3.98.4/%package%/download/
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
echo Configuring
set CFLAGS=%CFLAGS% -O2

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