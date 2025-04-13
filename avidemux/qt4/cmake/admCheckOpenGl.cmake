########################################
# OpenGL
########################################
OPTION(OPENGL "" ON)

MESSAGE(STATUS "Checking for OpenGL")
MESSAGE(STATUS "*******************")

IF(OPENGL)
        IF(QT6_FOUND OR (QT5_FOUND AND QT_VERSION_MINOR GREATER 4) )
                MESSAGE(STATUS "Found QOpenGL")
                IF(CROSS)
                        IF(NOT APPLE)
                                SET(OPENGL_FOUND 1)
                                SET(OPENGL_INCLUDE_DIR  "${MINGW}/include")
                                SET(OPENGL_LIBRARIES "-lopengl32 -lglu32")
                        ELSE()
                                SET(OPENGL_INCLUDE_DIR  "${MINGW}/include")
                                SET(OPENGL_LIBRARIES "-framework OpenGL")
                        ENDIF()
                        MESSAGE(STATUS "Cross compilation override, Skipping openGl search")
                ELSE()
                        IF(MINGW)
                                find_path(OPENGL_INCLUDE_DIR GL/gl.h)
                        ENDIF()

                        FIND_PACKAGE(OpenGL)
                        PRINT_LIBRARY_INFO("OpenGL" OPENGL_FOUND "${OPENGL_INCLUDE_DIR}" "${OPENGL_LIBRARIES}")
                ENDIF()
        ELSE()
                MESSAGE(STATUS "QOpenGL was not found")
                MESSAGE(STATUS "OpenGL is only available for Qt 5.5 or later ${QT_VERSION_MINOR}")
        ENDIF()

        IF(OPENGL_FOUND)
                SET(USE_OPENGL 1)
        ENDIF()
ELSE()
        MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF()

APPEND_SUMMARY_LIST("Miscellaneous" "OpenGL" "${USE_OPENGL}")

MESSAGE("")


