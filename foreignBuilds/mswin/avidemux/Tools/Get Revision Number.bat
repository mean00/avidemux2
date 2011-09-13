if not exist "..\..\..\..\avidemux\wininstaller\revision.nsh" goto end

copy "Revision No template.bat" + "..\..\..\..\avidemux\wininstaller\revision.nsh" temp.bat > NUL

call temp.bat
del temp.bat

set revisionNo=%revisionNo:~17,4%

:end