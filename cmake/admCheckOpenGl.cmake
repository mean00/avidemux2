########################################
# OpenGL
########################################
OPTION(OPENGL "" ON)

MESSAGE(STATUS "Checking for OpenGL")
MESSAGE(STATUS "*******************")

IF (OPENGL)
        IF (QT5_FOUND AND QT_VERSION_MINOR GREATER 4)
                MESSAGE(STATUS "Found QOpenGL")
                IF(CROSS)
                        IF(NOT APPLE)
                                SET(OPENGL_FOUND 1)
                                SET(OPENGL_INCLUDE_DIR  "${MINGW}/include")
                                SET(OPENGL_LIBRARIES "-lopengl32 -lglu32")
                        ELSE(NOT APPLE)
                                SET(OPENGL_INCLUDE_DIR  "${MINGW}/include")
                                SET(OPENGL_LIBRARIES "-framework OpenGL")
                        ENDIF(NOT APPLE)
                        MESSAGE(STATUS "Cross compilation override, Skipping openGl search")
                ELSE(CROSS)
                        if (MINGW)
                                find_path(OPENGL_INCLUDE_DIR GL/gl.h)
                        endif (MINGW)

                        FIND_PACKAGE(OpenGL)
                        PRINT_LIBRARY_INFO("OpenGL" OPENGL_FOUND "${OPENGL_INCLUDE_DIR}" "${OPENGL_LIBRARIES}")
                ENDIF(CROSS)
        ELSE (QT5_FOUND AND QT_VERSION_MINOR GREATER 4)
                MESSAGE(STATUS "QOpenGL was not found")
                MESSAGE(STATUS "OpenGL is only available for Qt 5.5 or later ${QT_VERSION_MINOR}")
        ENDIF (QT5_FOUND AND QT_VERSION_MINOR GREATER 4)

        IF (OPENGL_FOUND)
                SET(USE_OPENGL 1)
        ENDIF (OPENGL_FOUND)
ELSE (OPENGL)
        MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (OPENGL)

APPEND_SUMMARY_LIST("Miscellaneous" "OpenGL" "${USE_OPENGL}")

MESSAGE("")


