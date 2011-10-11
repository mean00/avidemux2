@echo off

echo MSYS build for libvpx
echo =====================
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

set package=libvpx-v0.9.7-p1.tar.bz2
set sourceFolder=libvpx-0.9.7-p1-%BuildBits%
set tarFolder=libvpx-v0.9.7-p1
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://webm.googlecode.com/files/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfj "%package%" -C "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

cd "%devDir%\%sourceFolder%"

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
  move "%CD%\%tarFolder%\%%a" "%CD%"
)

echo.
echo Patching
patch -p0 -i "%curDir%\configure.patch"
patch -p0 -i "%curDir%\configure.sh.patch"

echo.
echo Configuring

if "%BuildBits%" == "32" sh ./configure --prefix="%usrLocalDir%" --disable-vp8-encoder --target=x86-win32-gcc
if "%BuildBits%" == "64" sh ./configure --prefix="%usrLocalDir%" --disable-vp8-encoder --target=x86_64-win64-gcc

if errorlevel 1 goto end
echo.
pause

make install
if errorlevel 1 goto end

goto end

:error
echo Error

:end
pause