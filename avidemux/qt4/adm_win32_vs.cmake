#
MACRO(ADM_PREBUILD)
      	LIST(APPEND PlatformLibs  Qt5::WinMain)
ENDMACRO(ADM_PREBUILD)
#
MACRO(ADM_MAIN_APP)
        ADD_EXECUTABLE(avidemux3_${QT_EXTENSION} WIN32 ${ADM_EXE_SRCS})
        set_target_properties(avidemux3_${QT_EXTENSION} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /STACK:4000000")
	set_target_properties(avidemux3_${QT_EXTENSION} PROPERTIES WIN32_EXECUTABLE True)
        set_property(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux)
ENDMACRO(ADM_MAIN_APP)
#
MACRO(ADM_POSTBUILD)
	ADM_INSTALL_BIN(avidemux3_${QT_EXTENSION})
        include(./installMsvcRunTime.cmake)
        include(FindBourne)
        IF(RELEASE)
                configure_file(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/ChangeLog.release 
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/ChangeLog.html
                                COPYONLY)
                configure_file(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css.release
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css
                                COPYONLY)
        ELSE(RELEASE)
                execute_process(
                        COMMAND ${BASH_EXECUTABLE} genlog.sh
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/
                        )
                configure_file(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css.devbuild
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css
                                COPYONLY)
        ENDIF(RELEASE)
ENDMACRO(ADM_POSTBUILD)
