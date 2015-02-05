OPTION(QTSCRIPT "" ON)

if (QTSCRIPT)
	include(admCheckQt)
	checkQt()
endif (QTSCRIPT)

if (QT_QTSCRIPT_FOUND)
    APPEND_SUMMARY_LIST("Scripting" "QtScript" "${QT_QTSCRIPT_FOUND}")
    ADD_SUBDIRECTORY(src)
endif (QT_QTSCRIPT_FOUND)
