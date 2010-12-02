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

if errorlevel 1 goto error

set PATH=%PATH%;%devDir%\Git\bin

del "%usrLocalDir%\bin\libx264-*.dll"
del "%usrLocalDir%\include\x264.h"
cd "%devDir%"

if "%BuildBits%" == "32" (
	set sourceFolder=x264
	set buildFolder=build
)

if "%BuildBits%" == "64" (
	set sourceFolder=x264-64
	set buildFolder=build64
	set configFlags=--host=x86_64-pc-mingw32
)

rm -r -f %sourceFolder%

echo Downloading from git
git clone git://git.videolan.org/x264.git %sourceFolder%
if errorlevel 1 goto end

cd "%devDir%/%sourceFolder%"
sh ./configure --prefix=/usr/local --enable-shared %configFlags%
if errorlevel 1 goto end

make install
if errorlevel 1 goto end

del "%devDir%\avidemux_2.5_%buildFolder%\libx264-*.dll"
copy "%usrLocalDir%/bin/libx264-*.dll" "%devDir%\avidemux_2.5_%buildFolder%"

:end
pause