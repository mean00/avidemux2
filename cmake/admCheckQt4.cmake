MACRO(checkQt4)
	IF (NOT QT4_CHECKED)
		OPTION(QT4 "" ON)

		MESSAGE(STATUS "Checking for Qt 4")
		MESSAGE(STATUS "*****************")
                IF(CROSS)
                        MESSAGE(STATUS "Cross compiling override for QT4")
                        SET(QT_QTOPENGL_FOUND 1)
                        SET(QT_VERSION_MINOR 7)

                        SET(QT_QTOPENGL_LIBRARY ${QT_HOME}/lib/libQtOpenGL4.a)
                        SET(QT_QTOPENGL_INCLUDE_DIR ${QT_HOME}/include/QtOpenGL)

                        SET(QT_HEADERS_DIR "${QT_HOME}/include/")

                        SET(QT_INCLUDES "-I${QT_HOME}/include")
                        SET(QT_INCLUDE_DIR "${QT_HOME}/include")
                        SET(QT_BINARY_DIR ${QT_HOME}/bin)
                        SET(QT_LIBRARY_DIR ${QT_HOME}/lib)
                        SET(QT4_FOUND 1)
                        SET(QT_RCC_EXECUTABLE rcc)
                        SET(QT_MOC_EXECUTABLE moc-qt4)
                        SET(QT_UIC_EXECUTABLE uic-qt4)
                        SET(QT_QTCORE_LIBRARY ${QT_HOME}/lib/libQtCore4.a)
                        SET(QT_QTGUI_LIBRARY ${QT_HOME}/lib/libQtGui4.a)
                        MESSAGE(STATUS "[QT4Include] ${QT_INCLUDES}")
                        MESSAGE(STATUS "[QT4IncludeDir] ${QT_INCLUDE_DIR}")
                        include(admCrossQt4)
                        
                               

                ELSE(CROSS)
		IF (QT4)
			FIND_PACKAGE(Qt4)	
			STRING(REGEX REPLACE "[\\]" "/" QT_INCLUDES "${QT_INCLUDES}")	# backslashes aren't taken care of properly on Windows
			PRINT_LIBRARY_INFO("Qt 4" QT4_FOUND "${QT_INCLUDES} ${QT_DEFINITIONS}" "${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}")

			MARK_AS_ADVANCED(LRELEASE_EXECUTABLE)
			MARK_AS_ADVANCED(QT_MKSPECS_DIR)
			MARK_AS_ADVANCED(QT_PLUGINS_DIR)
			MARK_AS_ADVANCED(QT_QMAKE_EXECUTABLE)
		ELSE (QT4)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (QT4)
                ENDIF(CROSS)

		SET(QT4_CHECKED 1)

		MESSAGE("")
	ENDIF (NOT QT4_CHECKED)
ENDMACRO(checkQt4)
