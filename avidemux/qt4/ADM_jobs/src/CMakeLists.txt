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
SET(UI uiJobs.ui uiProgress.ui)
SET(headers ../include/T_jobs.h ../include/T_progress.h)
QT4_ADD_RESOURCES(resource  jobs.rsc)
QT4_WRAP_UI(ADM_JOBS_headers ${UI})
QT4_WRAP_CPP(ADM_JOBS_source ${headers}
                ADM_jobControl.cpp)
SET(ADM_JOB_SRCS 
                ADM_jobs.cpp
                ADM_jobControl.cpp
                ADM_runOneJob.cpp
		ADM_jobProgress.cpp
                ${ADM_JOBS_headers}
                ${ADM_JOBS_source}
               	${resource} 
        )

###########################################
# Version Info
###########################################
if (WIN32)
	# add icon and version info
	SET(FILEVERSION_STRING "${AVIDEMUX_VERSION}")
	SET(PRODUCTVERSION_STRING "${AVIDEMUX_VERSION}")
	STRING(REPLACE "." "," FILEVERSION ${FILEVERSION_STRING})
	STRING(REPLACE "." "," PRODUCTVERSION ${PRODUCTVERSION_STRING})
	SET(PRODUCTVERSION "${PRODUCTVERSION},0")

	IF (ADM_CPU_X86_64)
		SET(WIN_RES_TARGET "pe-x86-64")
	ELSE (ADM_CPU_X86_64)
		SET(WIN_RES_TARGET "pe-i386")
	ENDIF (ADM_CPU_X86_64)

	SET(AVIDEMUX_ICON "adm.ico")
	CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/admWin32.rc.in" ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc IMMEDIATE)

	if (MINGW)
		SET(ADM_WIN_RES "adm.obj")
		SET(ADM_JOB_SRCS ${ADM_JOB_SRCS} ${ADM_WIN_RES})
        include(admWindRes)
		ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} COMMAND ${WINDRES} -F ${WIN_RES_TARGET} -i ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc -o ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} -O coff --define VS_VERSION_INFO=1)
	endif (MINGW)

	if (MSVC)
		SET(ADM_JOB_SRCS ${ADM_JOB_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc)
	endif (MSVC)
endif (WIN32)

###########################################
# Executable
###########################################
include(admCoreIncludes)
include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_EXECUTABLE(avidemux3_jobs ${ADM_JOB_SRCS})
ADD_SOURCE_CFLAGS(ADM_jobs.cpp "-DADM_SUBVERSION='\"${ADM_SUBVERSION}\"'")

###########################################
TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_coreSocket6)
TARGET_LINK_LIBRARIES(avidemux3_jobs ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
TARGET_LINK_LIBRARIES(avidemux3_jobs ${PTHREAD_LIBRARIES})
###########################################
# OS Specific
###########################################
if (WIN32 OR APPLE)
	set_property(TARGET avidemux3_jobs PROPERTY OUTPUT_NAME avidemux_jobs)
endif (WIN32 OR APPLE)

IF (MINGW)
	target_link_libraries(avidemux3_jobs winmm -mwindows)
ENDIF (MINGW)

IF (APPLE)
	target_link_libraries(avidemux3_jobs "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")

	# for Leopard but it doesn't hurt Tiger
	target_link_libraries(avidemux3_jobs "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (APPLE)

TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_coreJobs)
TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_coreUtils6)
TARGET_LINK_LIBRARIES(avidemux3_jobs ADM_core6)
#
target_link_libraries(avidemux3_jobs "-lm -lstdc++")
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
