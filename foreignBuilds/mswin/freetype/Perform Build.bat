@echo off

echo MSYS build for FreeType
echo =======================
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

set version=2.4.9
set package=freetype-%version%.tar.gz
set sourceFolder=freetype-%version%-%BuildBits%
set tarFolder=freetype-%version%
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://sourceforge.net/projects/freetype/files/freetype2/%version%/%package%/download
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

set CFLAGS=%CFLAGS% -O3
sh ./configure --prefix="%usrLocalDir%" --disable-static

if errorlevel 1 goto end
echo.
pause

make install
if errorlevel 1 goto end

strip "%usrLocalDir%\bin\libfreetype-6.dll"
copy "%usrLocalDir%\bin\libfreetype-6.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause