@echo off

echo MSYS build for Xvid
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

set package=xvidcore-1.3.2.tar.gz
set sourceFolder=xvidcore-1.3.2-%BuildBits%
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://downloads.xvid.org/downloads/%package%
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

for /f "delims=" %%a in ('dir /b xvidcore') do (
  move "%CD%\xvidcore\%%a" "%CD%"
)

echo.
echo Patching
patch -p0 -i "%curDir%\configure.patch"
patch -p0 -i "%curDir%\nasm.inc.patch"

echo.
echo Configuring
cd build\generic

if "%BuildBits%" == "32" sh ./configure --prefix="%usrLocalDir%"
if "%BuildBits%" == "64" sh ./configure --prefix="%usrLocalDir%" --build=x86_64-pc-mingw32
if errorlevel 1 goto end
echo.
pause

make LDFLAGS="%LDFLAGS% -Wl,-s"
if errorlevel 1 goto end

make install
if errorlevel 1 goto end

move "%usrLocalDir%\lib\xvidcore.dll" "%usrLocalDir%\bin"
copy "%usrLocalDir%\bin\xvidcore.dll" "%admBuildDir%"
copy ".\=build\xvidcore.dll.a" "%usrLocalDir%\lib"
goto end

:error
echo Error

:end
pause