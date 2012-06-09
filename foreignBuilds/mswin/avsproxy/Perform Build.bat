@echo off

echo MSVC build for avsproxy
echo =======================
echo.

set curDir=%CD%

call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
echo.

cd ..\..\..\addons\avisynthproxy
if errorlevel 1 goto end

vcupgrade -overwrite -nologo avsproxy.vcproj
if errorlevel 1 goto end

echo.
msbuild avsproxy.vcxproj /p:Configuration=Release /t:rebuild
if errorlevel 1 goto end

set BuildBits=32
call "%curDir%\..\Set Common Environment Variables"
copy "avsproxy.exe" "%admBuildDir%"

set BuildBits=64
call "%curDir%\..\Set Common Environment Variables"
copy "avsproxy.exe" "%admBuildDir%"

:end
pause