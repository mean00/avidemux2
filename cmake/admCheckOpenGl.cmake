########################################
# OpenGL
########################################
OPTION(OPENGL "" ON)

MESSAGE(STATUS "Checking for OpenGL")
MESSAGE(STATUS "*******************")

IF (OPENGL)
       IF (QT4_FOUND AND QT_VERSION_MINOR GREATER 5)
               IF (QT_QTOPENGL_FOUND)
                       MESSAGE(STATUS "Found QtOpenGL")
                       FIND_PACKAGE(OpenGL)
                       PRINT_LIBRARY_INFO("OpenGL" OPENGL_FOUND "${OPENGL_INCLUDE_DIR}" "${OPENGL_LIBRARIES}")
               ELSE (QT_QTOPENGL_FOUND)
                       MESSAGE(STATUS "QtOpenGL was not found")
               ENDIF (QT_QTOPENGL_FOUND)
       ELSE (QT4_FOUND AND QT_VERSION_MINOR GREATER 5)
               MESSAGE(STATUS "OpenGL is only available for Qt 4.6 or later ${QT_VERSION_MINOR}")
       ENDIF (QT4_FOUND AND QT_VERSION_MINOR GREATER 5)

       IF (OPENGL_FOUND)
               SET(USE_OPENGL 1)
       ENDIF (OPENGL_FOUND)
ELSE (OPENGL)
       MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (OPENGL)

MESSAGE("")


