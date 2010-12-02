cd "%sourceDir%\%buildCoreFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux_core

if errorlevel 1 goto error
pause

cd "%sourceDir%\%buildCliFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux/cli

if errorlevel 1 goto error
pause

cd "%sourceDir%\%buildGtkFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux/gtk

if errorlevel 1 goto error
pause

cd "%sourceDir%\%buildQtFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" %DebugFlags% ../../avidemux/qt4

if errorlevel 1 goto error
pause

set msysSourceDir=%sourceDir:\=/%
cd "%sourceDir%\%buildPluginFolder%"
cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DAVIDEMUX_SOURCE_DIR="%msysSourceDir%" -DPLUGIN_UI=ALL %DebugFlags% ../../avidemux_plugins

if errorlevel 1 goto error
pause

cd "%sourceDir%\%buildCoreFolder%"
make install

if errorlevel 1 goto error

cd "%sourceDir%\%buildCliFolder%"
make install

if errorlevel 1 goto error

cd "%sourceDir%\%buildGtkFolder%"
make install

if errorlevel 1 goto error

cd "%sourceDir%\%buildQtFolder%"
make install

if errorlevel 1 goto error

cd "%sourceDir%\%buildPluginFolder%"
rmdir /s /q "%buildDir%\plugins" 2> NUL
make install

if errorlevel 1 goto error
goto :EOF

:error
exit /b 1