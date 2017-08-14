MACRO(ADM_PREBUILD)
        LIST(APPEND PlatformLibs   "-lm -lstdc++")
        LIST(APPEND PlatformLibs   "winmm -mwindows -Wl,--export-all-symbols")
ENDMACRO(ADM_PREBUILD)
#
MACRO(ADM_MAIN_APP)
        
        ADD_EXECUTABLE(avidemux3_${QT_EXTENSION} WIN32 ${ADM_EXE_SRCS})
        set_property(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux)

        if(CROSS)
                TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_UI_${QT_LIBRARY_EXTENSION}6)
                TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_UI_${QT_LIBRARY_EXTENSION}6)
                TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_render6_${QT_LIBRARY_EXTENSION})
                TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_coreUtils6)
                TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_core6)
                TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_core6)
        endif(CROSS)


ENDMACRO(ADM_MAIN_APP)
#
MACRO(ADM_POSTBUILD)
	ADM_INSTALL_BIN(avidemux3_${QT_EXTENSION})
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
