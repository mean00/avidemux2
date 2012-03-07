@echo off

%~d0
cd "%~dp0"

echo MSYS build for SpiderMonkey
echo ===========================
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

set package=js-1.7.0.tar.gz
set sourceFolder=js-1.7.0-%BuildBits%
set tarFolder=js
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://ftp.mozilla.org/pub/mozilla.org/js/%package%
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
echo Patching
copy "%curDir%\WINNT6.1.mk" .\src\config
patch -p0 -i "%curDir%\Makefile.ref.patch"
patch -p0 -i "%curDir%\jsnum.c.patch"

echo.
cd src
if "%BuildBits%" == "32" windres -i js3240.rc -o jsres.o -O coff -F pe-i386
if "%BuildBits%" == "64" windres -i js3240.rc -o jsres.o -O coff

set CFLAGS=%CFLAGS% -I"%usrLocalDir%/include/nspr"
make -f Makefile.ref JS_DIST="%usrLocalDir%" BUILD_OPT=1 JS_HAS_FILE_OBJECT=1 XLDFLAGS="%LDFLAGS% jsres.o -L%usrLocalDir%/lib -lnspr4"
if errorlevel 1 goto end
echo.

strip "WINNT6.1_OPT.OBJ\libjs.dll"
copy "WINNT6.1_OPT.OBJ\libjs.dll" "%usrLocalDir%\bin"
copy "%usrLocalDir%\bin\libjs.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause