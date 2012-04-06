@echo off

echo MSYS build for Qt
echo =================
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

set version=4.8.1
set package=qt-everywhere-opensource-src-%version%.tar.gz
set sourceBaseFolder=Qt%BuildBits%
set sourceFolder=%sourceBaseFolder%\\%version%
set tarFolder=qt-everywhere-opensource-src-%version%
set curDir=%CD%
rem set PATH=%PATH%;%~d0\Dev\MSYS\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://download.qt.nokia.com/qt/source/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rmdir /s /q "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

if not exist "%devDir%\%sourceBaseFolder%" mkdir "%devDir%\%sourceBaseFolder%"
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
patch -p0 -i "%curDir%\qmake%BuildBits%.conf.patch"

echo.
echo Configuring
configure -confirm-license -opensource -release -system-zlib -no-stl -no-qt3support -no-phonon-backend -no-webkit -no-multimedia -no-style-cleanlooks -no-style-plastique -no-style-motif -no-style-cde -qt-style-windowsxp -qt-style-windowsvista -no-xmlpatterns -nomake demos -nomake examples -platform win32-g++ -mmx -sse -sse2 -3dnow -I %CMAKE_INCLUDE_PATH% -L %CMAKE_LIBRARY_PATH%

if errorlevel 1 goto end
echo.
pause

mingw32-make
if errorlevel 1 goto end

copy "%devDir%\%sourceFolder%\bin\QtCore4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtGui4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtOpenGL4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtScript4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtScriptTools4.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause