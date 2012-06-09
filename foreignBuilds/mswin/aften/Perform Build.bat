@echo off

echo MSVC build for Aften
echo ====================
echo 1. 32-bit build
echo 2. 64-bit build
echo X. Exit
echo.

choice /c 12x

if errorlevel 1 (
	set BuildBits=32
	set VcvarFlags=x86 )
if errorlevel 2 (
	set BuildBits=64
	set VcvarFlags=x86_amd64 )
if errorlevel 3 goto :eof

verify >nul
call "../Set Common Environment Variables"
if errorlevel 1 goto end

set curDir=%CD%
set PATH=%PATH%;%msysdir%\bin;%devDir%\Git\bin
set CFLAGS=
set CXXFLAGS=
set LDFLAGS=

call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" %VcvarFlags%
echo.

set sourceFolder=%devDir%/aften-%BuildBits%
rm -r -f %sourceFolder%
if errorlevel 1 goto end

echo Downloading from git
call git clone git://aften.git.sourceforge.net/gitroot/aften/aften %sourceFolder%
if errorlevel 1 goto end

cd "%sourceFolder%"
call git checkout 89aee3d496bb2a89f046025402626ee12a12969f
if errorlevel 1 goto end

mkdir build%BuildBits%
if errorlevel 1 goto end

cd build%BuildBits%
cmake -G"NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%usrLocalDir% -DSHARED=ON -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /Ob2 /D NDEBUG" ..
if errorlevel 1 goto end

nmake install
if errorlevel 1 goto end

copy "%usrLocalDir%\bin\aften.dll" "%admBuildDir%"

:end
pause