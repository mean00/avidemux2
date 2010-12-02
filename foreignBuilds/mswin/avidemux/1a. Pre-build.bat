cd "%sourceDir%"
echo Cleaning build directory (%CD%\%buildFolder%)
rmdir /s /q "%buildFolder%" 2> NUL

echo Cleaning SDK directory (%sdkBuildDir%)
rmdir /s /q "%sdkBuildDir%" 2> NUL

if exist "%buildFolder%" goto removalFailure
if exist "%sdkBuildDir%" goto removalFailure

mkdir "%buildCoreFolder%"
mkdir "%buildCliFolder%"
mkdir "%buildGtkFolder%"
mkdir "%buildQtFolder%"
mkdir "%buildPluginFolder%"
mkdir "%sdkBuildDir%"

echo.
echo Performing Subversion update
svn up .
echo.

goto :EOF

:removalFailure
echo.
echo ERROR - build directories could not be fully deleted.
echo Aborting.

:error
exit /b 1