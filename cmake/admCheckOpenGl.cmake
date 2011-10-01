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
                       IF(CROSS)
				IF(NOT APPLE)
                                	SET(OPENGL_FOUND 1)
                                	SET(OPENGL_INCLUDE_DIR  "${MINGW}/include" )
                                	SET(OPENGL_LIBRARIES "-lopengl32 -lglu32")
				ELSE(NOT APPLE)
                                	SET(OPENGL_INCLUDE_DIR  "${MINGW}/include" )
                                	SET(OPENGL_LIBRARIES "-framework OpenGL ")
				ENDIF(NOT APPLE)
                                MESSAGE(STATUS "Cross compilation override, Skipping openGl search")
                       ELSE(CROSS)
                       	        FIND_PACKAGE(OpenGL)
                                PRINT_LIBRARY_INFO("OpenGL" OPENGL_FOUND "${OPENGL_INCLUDE_DIR}" "${OPENGL_LIBRARIES}")
                       ENDIF(CROSS)
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


