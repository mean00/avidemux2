if not exist "..\..\..\installer\revision.nsh" goto end

copy "Revision No template.bat" + "..\..\..\installer\revision.nsh" temp.bat > NUL

call temp.bat
del temp.bat

set revisionNo=%revisionNo:~17,4%

:end