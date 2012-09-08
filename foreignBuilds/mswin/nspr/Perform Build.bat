@echo off

%~d0
cd "%~dp0"

echo MSYS build for nspr
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

set version=4.9.2
set package=nspr-%version%.tar.gz
set sourceFolder=nspr-%version%-%BuildBits%
set tarFolder=nspr-%version%
set PATH=%PATH%;%~d0\Dev\MSYS\bin
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v%version%/src/%package%
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

echo.
echo Configuring
cd mozilla\nsprpub
set LDFLAGS=-shared-libgcc
sh ./configure --prefix="%usrLocalDir%" --enable-strip --enable-win32-target=WIN95 --enable-optimize="-O3 %CFLAGS%" --disable-debug

if errorlevel 1 goto end
echo.
pause

if "%BuildBits%" == "32" set RC=RC="windres -F pe-i386" CC="gcc -m32"
make %RC%
if errorlevel 1 goto end

make install
if errorlevel 1 goto end

move "%usrLocalDir%\lib\nspr4.dll" "%usrLocalDir%\bin"
copy "%usrLocalDir%\bin\nspr4.dll" "%admBuildDir%"

pexports "%usrLocalDir%/bin/nspr4.dll" > nspr4.def
if "%BuildBits%" == "32" dlltool -d nspr4.def -l "%usrLocalDir%/lib/nspr4.dll.a" -m i386 --as-flags=--32
if "%BuildBits%" == "64" dlltool -d nspr4.def -l "%usrLocalDir%/lib/nspr4.dll.a"

goto end

:error
echo Error

:end
pause