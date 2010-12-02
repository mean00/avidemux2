set PATH=%msysDir%\bin;%PATH%

echo -- Generating Change Log.html --
cd "..\..\installer"
sh genlog.sh

echo -- Generating Package Notes.html --
cd "%curDir%\Tools"
sh gennotes.sh

echo -- Generating Touch Files.html --
sh gentouch.sh

goto end

:error
exit /b 1

:end
pause