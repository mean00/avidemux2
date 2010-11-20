##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX qt4)
include_directories(../include ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
###########################################
# Executable
###########################################
#
SET(UI uiJobs.ui)
SET(headers ../include/T_jobs.h)
QT4_WRAP_UI(ADM_JOBS_headers ${UI})
QT4_WRAP_CPP(ADM_JOBS_source ${headers}
                ADM_jobControl.cpp)
SET(ADM_JOB_SRCS 
                ADM_jobs.cpp
                ADM_jobControl.cpp
                ADM_runOneJob.cpp
                ${ADM_JOBS_headers}
                ${ADM_JOBS_source}
                
        )

###########################################
# Executable
###########################################
include(admCoreIncludes)
ADD_EXECUTABLE(avidemux3_jobs ${ADM_JOB_SRCS})
ADD_SOURCE_CFLAGS(ADM_jobs.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")

###########################################
TARGET_LINK_LIBRARIES(avidemux3_jobs ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

###########################################
# OS Specific
###########################################
IF (WIN32)
#	target_link_libraries(avidemux3_jobs ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux3_jobs -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux3_jobs winmm -mwindows)
ENDIF (WIN32)

IF (APPLE)
	ADD_TARGET_LDFLAGS(avidemux3_jobs "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")

	# for Leopard but it doesn't hurt Tiger
	ADD_TARGET_LDFLAGS(avidemux3_jobs "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (APPLE)
TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_coreJobs)
TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_coreUtils6)
TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_core6)
#
#


###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_jobs)
ADM_INSTALL_BIN(avidemux3_jobs)
#
# Packaging
#
#include(admPackager)
#admPackager(qt4Package)
