
MACRO(checkQt5)
  IF(NOT QT5_CHECKED)
    OPTION(QT5 "" ON)

    MESSAGE(STATUS "Checking for Qt5")
    MESSAGE(STATUS "****************")

    IF(QT5)

      IF(CROSS)
        SET(CROSS5 ${QT_HOME}/lib/cmake)
        MESSAGE(STATUS "Cross-compiling override for Qt5: \"${CROSS5}\"")
        SET(CMAKE_MODULE_PATH
            ${CROSS5}/Qt5
            ${CROSS5}/Qt5Core
            ${CROSS5}/Qt5Gui
            ${CROSS5}/Qt5Network
            ${CROSS5}/Qt5Widgets
            ${CROSS5}/Qt5WinExtras
            ${CMAKE_MODULE_PATH})
        MESSAGE(STATUS "CMake search path: \"${CMAKE_MODULE_PATH}\"")
        SET(QT_BINARY_DIR ${QT_HOME}/bin)
        SET(QT_LIBRARY_DIR ${QT_HOME}/lib ${QT_HOME}/bin)
        LINK_DIRECTORIES(${QT_LIBRARY_DIR})
      ENDIF()

      MESSAGE(STATUS "Checking for Qt5Core")
      FIND_PACKAGE(Qt5Core REQUIRED)
      MESSAGE(STATUS "Checking for Qt5Widgets")
      FIND_PACKAGE(Qt5Widgets REQUIRED)
      MESSAGE(STATUS "Checking for Qt5Network")
      FIND_PACKAGE(Qt5Network)
      IF(WIN32)
        FIND_PACKAGE(Qt5WinExtras)
      ENDIF()
      IF(Qt5Core_FOUND AND Qt5Widgets_FOUND AND Qt5Network_FOUND)
        MESSAGE(STATUS "Qt5 found ")
        SET(QT5_FOUND 1)
        SET(QT_VERSION_MINOR ${Qt5Core_VERSION_MINOR})
        SET(QT_INCLUDES ${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
        SET(QT_INCLUDE_DIR ${QT_INCLUDES})
        SET(QT_QTCORE_LIBRARY ${Qt5Core_LIBRARIES})
        SET(QT_QTNETWORK_LIBRARY ${Qt5Network_LIBRARIES})
        SET(QT_QTGUI_LIBRARY ${Qt5Widgets_LIBRARIES})
        SET(QT_DEFINITIONS ${Qt5Core_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS})
        IF(Qt5WinExtras_FOUND)
          SET(QT_QTGUI_LIBRARY ${QT_QTGUI_LIBRARY} ${Qt5WinExtras_LIBRARIES})
          SET(QT_DEFINITIONS ${QT_DEFINITIONS} -DQT_HAS_WINEXTRA)
        ENDIF()
        STRING(REGEX REPLACE "[\\]" "/" QT_INCLUDES "${QT_INCLUDES}")    # backslashes aren't taken care of properly on Windows
        MESSAGE(STATUS "Qt5 includes     : ${QT_INCLUDES}")
        MESSAGE(STATUS "Qt5 definitions  : ${QT_DEFINITIONS}")
        MESSAGE(STATUS "Qt5 libs         : ${QT_QTCORE_LIBRARY} : ${QT_QTGUI_LIBRARY} : ${QT_QTNETWORK_LIBRARY}")
        MARK_AS_ADVANCED(LRELEASE_EXECUTABLE)
        MARK_AS_ADVANCED(QT_MKSPECS_DIR)
        MARK_AS_ADVANCED(QT_QMAKE_EXECUTABLE)
        SET(QT_EXTENSION qt5)
        SET(QT_LIBRARY_EXTENSION QT5)
        SET(ADM_QT_VERSION 5)
        IF(Qt5_POSITION_INDEPENDENT_CODE)
          SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
        ENDIF()
        get_target_property(QMAKE_EXECUTABLE Qt5::qmake LOCATION)

        execute_process(COMMAND "${QMAKE_EXECUTABLE}" -query QT_INSTALL_PLUGINS
                            RESULT_VARIABLE return_code
                            OUTPUT_VARIABLE QT_PLUGINS_DIR
                            OUTPUT_STRIP_TRAILING_WHITESPACE)

        MESSAGE(STATUS "plugin dir = \"${QT_PLUGINS_DIR}\"")

          # Do we have qtScript also ?
          #FIND_PACKAGE(Qt5Script)
          #MESSAGE(STATUS "  Checking for Qt5Script")
          #IF(Qt5Script_FOUND)
          #MESSAGE(STATUS "   Qt5Script found")
          #SET(QT_QTSCRIPT_FOUND 1)
          #SET(QT_QTSCRIPT_LIBRARY ${Qt5Script_LIBRARIES})
          #ELSE()
          #MESSAGE(STATUS "   Qt5Script NOT found")
          #ENDIF()
          # ----------------------------------

      ELSE()
        MESSAGE(STATUS "Some Qt5 components are missing")
      ENDIF()

    ELSE()
      MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF()

    SET(QT5_CHECKED 1)

    MESSAGE("")
  ENDIF()
ENDMACRO()
