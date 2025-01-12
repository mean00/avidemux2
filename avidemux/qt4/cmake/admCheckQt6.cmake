MACRO(checkQt6)
  IF(NOT QT6_CHECKED)
    OPTION(QT6 "" ON)

    IF(QT6)
      MESSAGE(STATUS "Checking for Qt6")
      MESSAGE(STATUS "******************")

      IF(CROSS)
        #MESSAGE(STATUS "Qt6 is supported only for native builds at the moment")
        SET(CROSS6 ${QT_HOME}/lib/cmake)
        MESSAGE(STATUS "Cross-compiling override for Qt6: \"${CROSS6}\"")
        SET(CMAKE_MODULE_PATH
            ${CROSS6}/Qt6
            ${CROSS6}/Qt6Core
            ${CROSS6}/Qt6Gui
            ${CROSS6}/Qt6Network
            ${CROSS6}/Qt6OpenGLWidgets
            ${CROSS6}/Qt6Widgets
            ${CMAKE_MODULE_PATH})
        MESSAGE(STATUS "CMake search path: \"${CMAKE_MODULE_PATH}\"")
        SET(QT_BINARY_DIR ${QT_HOME}/bin)
        SET(QT_LIBRARY_DIR ${QT_HOME}/lib ${QT_HOME}/bin)
      ENDIF()

      MESSAGE(STATUS "Checking for Qt6Core")
      FIND_PACKAGE(Qt6 COMPONENTS Core)

      MESSAGE(STATUS "Checking for Qt6Gui")
      FIND_PACKAGE(Qt6 COMPONENTS Gui)

      MESSAGE(STATUS "Checking for Qt6Widgets")
      FIND_PACKAGE(Qt6 COMPONENTS Widgets)

      MESSAGE(STATUS "Checking for OpenGLWidgets")
      FIND_PACKAGE(Qt6 COMPONENTS OpenGLWidgets)

      MESSAGE(STATUS "Checking for Qt6Network")
      FIND_PACKAGE(Qt6 COMPONENTS Network)

      IF(Qt6Core_FOUND AND Qt6Gui_FOUND AND Qt6Widgets_FOUND AND Qt6OpenGLWidgets_FOUND AND Qt6Network_FOUND)
        MESSAGE(STATUS "Qt6 found")
        SET(QT6_FOUND 1)

        SET(QT_VERSION_MINOR ${Qt6Core_VERSION_MINOR})
        SET(QT_INCLUDES ${Qt6Core_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS} ${Qt6OpenGLWidgets_INCLUDE_DIRS} ${Qt6Network_INCLUDE_DIRS})
        SET(QT_INCLUDE_DIR ${QT_INCLUDES})
        SET(QT_HEADERS_DIR ${QT_INCLUDE_DIR})

        SET(QT_QTCORE_LIBRARY ${Qt6Core_LIBRARIES})
        SET(QT_QTNETWORK_LIBRARY ${Qt6Network_LIBRARIES})
        SET(QT_QTGUI_LIBRARY ${Qt6Gui_LIBRARIES} ${Qt6Widgets_LIBRARIES} ${Qt6OpenGLWidgets_LIBRARIES})

        SET(QT_DEFINITIONS ${Qt6Core_DEFINITIONS} ${Qt6Widgets_DEFINITIONS} ${Qt6OpenGLWidgets_DEFINITIONS} ${Qt6Network_DEFINITIONS})

        MARK_AS_ADVANCED(LRELEASE_EXECUTABLE)
        MARK_AS_ADVANCED(QT_MKSPECS_DIR)
        MARK_AS_ADVANCED(QT_QMAKE_EXECUTABLE)

        SET(QT_EXTENSION qt6)
        SET(QT_LIBRARY_EXTENSION QT6)
        SET(ADM_QT_VERSION 6)

        IF(Qt6_POSITION_INDEPENDENT_CODE)
          SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
        ENDIF()

        GET_TARGET_PROPERTY(QMAKE_EXECUTABLE Qt6::qmake LOCATION)
        EXECUTE_PROCESS(COMMAND "${QMAKE_EXECUTABLE}" -query QT_INSTALL_PLUGINS
                        RESULT_VARIABLE return_code
                        OUTPUT_VARIABLE QT_PLUGINS_DIR
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        MESSAGE(STATUS "plugin dir = \"${QT_PLUGINS_DIR}\"")

      ELSE()
        MESSAGE(STATUS "Some Qt6 components are missing")
      ENDIF()
    ELSE()
      MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF()

    SET(QT6_CHECKED 1)

    MESSAGE("")
  ENDIF()
ENDMACRO()
