@echo off

echo MSYS build for opencore-amr
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

set package=opencore-amr-0.1.2.tar.gz
set sourceFolder=opencore-amr-0.1.2-%BuildBits%
set tarFolder=opencore-amr-0.1.2
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://sourceforge.net/projects/opencore-amr/files/opencore-amr/0.1.2/%package%/download
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
patch -p0 -i "%curDir%\amrnb_Makefile.in"
patch -p0 -i "%curDir%\amrwb_Makefile.in"

echo.
echo Configuring

sh ./configure --prefix="%usrLocalDir%" --disable-static

if errorlevel 1 goto end
echo.
pause

make install-strip
if errorlevel 1 goto end

copy "%usrLocalDir%\bin\libopencore-amrnb-0.dll" "%admBuildDir%"
copy "%usrLocalDir%\bin\libopencore-amrwb-0.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause