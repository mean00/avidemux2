@echo off

echo MSYS build for x264
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

set PATH=%PATH%;%~d0\Dev\MSYS\bin;%devDir%\Git\bin
set curDir=%CD%

del "%usrLocalDir%\bin\libx264-*.dll"
del "%usrLocalDir%\include\x264.h"
cd "%devDir%"

set sourceFolder=x264-%BuildBits%
rm -r -f %sourceFolder%
if errorlevel 1 goto end

echo Downloading from git
call git clone git://git.videolan.org/x264.git %sourceFolder%
if errorlevel 1 goto end

cd "%devDir%/%sourceFolder%"

echo.
echo Patching
if "%BuildBits%" == "32" patch -p0 -i "%curDir%\configure32.patch"

echo.
echo Configuring
if "%BuildBits%" == "32" sh ./configure --prefix=%usrLocalDir% --enable-shared --enable-win32thread
if "%BuildBits%" == "64" sh ./configure --prefix=%usrLocalDir% --enable-shared --enable-win32thread --host=x86_64-pc-mingw32

if errorlevel 1 goto end

make install
if errorlevel 1 goto end

del "%admBuildDir%\libx264-*.dll"
copy "%usrLocalDir%/bin/libx264-*.dll" "%admBuildDir%"

:end
pause