
MACRO(checkQt6)
    IF (NOT QT6_CHECKED)
        OPTION(QT6 "" ON)

        MESSAGE(STATUS "Checking for Qt6")
        MESSAGE(STATUS "******************")
        IF(CROSS)
            MESSAGE(STATUS "Qt6 is supported only for native builds at the moment")
        ELSE(CROSS)
            IF (QT6)
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

                    MACRO(ADM_QT_WRAP_UI a)
                        qt_wrap_ui(${a} ${ARGN})
                    ENDMACRO(ADM_QT_WRAP_UI a)

                    MACRO(ADM_QT_WRAP_CPP a)
                        qt_wrap_cpp(${a} ${ARGN})
                    ENDMACRO(ADM_QT_WRAP_CPP a)

                    MACRO(ADM_QT_ADD_RESOURCES a)
                        qt_add_resources(${a} ${ARGN})
                    ENDMACRO(ADM_QT_ADD_RESOURCES a)

                    SET(QT_EXTENSION qt6)
                    SET(QT_LIBRARY_EXTENSION QT6)
                    SET(ADM_QT_VERSION 6)

                    IF(Qt6_POSITION_INDEPENDENT_CODE)
                        SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
                    ENDIF(Qt6_POSITION_INDEPENDENT_CODE)

                    get_target_property(QMAKE_EXECUTABLE Qt6::qmake LOCATION)
                    exec_program(${QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_PLUGINS" RETURN_VALUE return_code OUTPUT_VARIABLE QT_PLUGINS_DIR )
                    MESSAGE(STATUS "plugin dir = ${QT_PLUGINS_DIR}")

                ELSE(Qt6Core_FOUND AND Qt6Gui_FOUND AND Qt6Widgets_FOUND AND Qt6OpenGLWidgets_FOUND AND Qt6Network_FOUND)
                    MESSAGE(STATUS "Some Qt6 components are missing")
                ENDIF(Qt6Core_FOUND AND Qt6Gui_FOUND AND Qt6Widgets_FOUND AND Qt6OpenGLWidgets_FOUND AND Qt6Network_FOUND)
            ELSE (QT6)
                MESSAGE("${MSG_DISABLE_OPTION}")
            ENDIF (QT6)
        ENDIF(CROSS)

        SET(QT6_CHECKED 1)

        MESSAGE("")
    ENDIF (NOT QT6_CHECKED)
ENDMACRO(checkQt6)
