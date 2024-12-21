@echo off

echo MSYS build for expat
echo ====================
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

set version=2.1.0
set package=expat-%version%.tar.gz
set sourceFolder=expat-%version%-%BuildBits%
set tarFolder=expat-%version%
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://sourceforge.net/projects/expat/files/expat/%version%/%package%/download
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

echo.
echo Configuring

sh ./configure --prefix="%usrLocalDir%" --disable-static

if errorlevel 1 goto end
echo.
pause

make CFLAGS="%CFLAGS% -O3" install
if errorlevel 1 goto end

strip "%usrLocalDir%\bin\libexpat-1.dll"
copy "%usrLocalDir%\bin\libexpat-1.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause