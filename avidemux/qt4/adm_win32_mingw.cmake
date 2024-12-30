MACRO(ADM_PREBUILD)
  LIST(APPEND PlatformLibs   "-lm -lstdc++")
  LIST(APPEND PlatformLibs   "winmm -mwindows -Wl,--export-all-symbols")
ENDMACRO()
#
MACRO(ADM_MAIN_APP)

  ADD_EXECUTABLE(avidemux3_${QT_EXTENSION} WIN32 ${ADM_EXE_SRCS})
  SET_PROPERTY(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux)

  IF(CROSS)
    TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_UI_${QT_LIBRARY_EXTENSION}6)
    TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_UI_${QT_LIBRARY_EXTENSION}6)
    TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_render6)
    TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_coreUtils6)
    TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_core6)
    TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_core6)
  ENDIF()
ENDMACRO()
#
MACRO(ADM_POSTBUILD)
  ADM_INSTALL_BIN(avidemux3_${QT_EXTENSION})
  INCLUDE(FindBourne)
  IF(RELEASE)
    CONFIGURE_FILE(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/ChangeLog.release
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/ChangeLog.html
                                COPYONLY)
    CONFIGURE_FILE(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css.release
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css
                                COPYONLY)
  ELSE()
    EXECUTE_PROCESS(
                        COMMAND ${BASH_EXECUTABLE} genlog.sh
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/
                        )
    CONFIGURE_FILE(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css.devbuild
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/change.css
                                COPYONLY)
  ENDIF()
ENDMACRO()
