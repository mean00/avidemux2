cd "%sourceDir%\%buildCoreFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux_core

if errorlevel 1 goto error
pause

make install
if errorlevel 1 goto error

cd "%sourceDir%\%buildCliFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux/cli

if errorlevel 1 goto error
pause

make install
if errorlevel 1 goto error

cd "%sourceDir%\%buildGtkFolder%"
rem cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux/gtk

if errorlevel 1 goto error
rem pause

rem make install
if errorlevel 1 goto error

cd "%sourceDir%\%buildQtFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux/qt4

if errorlevel 1 goto error
pause

make install
if errorlevel 1 goto error

set msysSourceDir=%sourceDir:\=/%
cd "%sourceDir%\%buildPluginFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DAVIDEMUX_SOURCE_DIR="%msysSourceDir%" -DPLUGIN_UI=ALL %DebugFlags% ../../avidemux_plugins

if errorlevel 1 goto error
pause

rmdir /s /q "%buildDir%\plugins" 2> NUL
make install
if errorlevel 1 goto error
goto :EOF

:error
exit /b 1