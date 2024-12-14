# # # # # # # # # # # # # # # # # # # # # # # # # # # ###############
# Config
# # # # # # # # # # # # # # # # # # # # # # # # # # # ###############
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)

IF(MSVC)
  ADD_DEFINITIONS(-D_WINSOCKAPI_)
ENDIF()

SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX qt4)
INCLUDE(admWindRes)
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
# Executable
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
#
SET(UI uiJobs.ui uiProgress.ui)
SET(headers ../include/T_jobs.h ../include/T_progress.h)
IF(APPLE OR WIN32)
  ADM_QT_ADD_RESOURCES(resource jobs.qrc)
ELSE()
  ADM_QT_ADD_RESOURCES(resource jobs_linux.qrc)
ENDIF()
ADM_QT_WRAP_UI(ADM_JOBS_headers ${UI})
ADM_QT_WRAP_CPP(ADM_JOBS_source ${headers})
SET(ADM_JOB_SRCS
  ADM_jobs.cpp
  ADM_jobControl.cpp
  ADM_runOneJob.cpp
  ADM_jobProgress.cpp
  ../../ADM_userInterfaces/translator.cpp
  ${ADM_JOBS_headers}
  ${ADM_JOBS_source}
  ${resource}
)

# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
# Version Info
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
IF(WIN32)
  WINDRESIFY(mainJobs "${CMAKE_CURRENT_SOURCE_DIR}/../../../common/xpm/adm.ico" src "avidemux_jobs" "Avidemux Jobs")
  SET(ADM_JOB_SRCS ${ADM_JOB_SRCS} ${src})
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
# Executable
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
INCLUDE(admCoreIncludes)

IF(WIN32)
  ADD_EXECUTABLE(avidemux3_jobs_${QT_EXTENSION} WIN32 ${ADM_JOB_SRCS})
  IF(MSVC)
    SET_TARGET_PROPERTIES(avidemux3_jobs_${QT_EXTENSION} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    SET_TARGET_PROPERTIES(avidemux3_jobs_${QT_EXTENSION} PROPERTIES WIN32_EXECUTABLE True)
  ELSE()
    TARGET_LINK_LIBRARIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE winmm -mwindows)
  ENDIF()
ELSE()
  ADD_EXECUTABLE(avidemux3_jobs_${QT_EXTENSION} ${ADM_JOB_SRCS})
ENDIF()

TARGET_INCLUDE_DIRECTORIES(avidemux3_jobs_${QT_EXTENSION} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_INCLUDE_DIRECTORIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

ADD_SOURCE_CFLAGS(ADM_jobs.cpp "-DADM_VERSION=\"${AVIDEMUX_VERSION}\"")

# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
TARGET_LINK_LIBRARIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE ADM_coreSocket6 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
# OS Specific
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
IF(WIN32 OR APPLE)
  SET_PROPERTY(TARGET avidemux3_jobs_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux_jobs)
ENDIF()

IF(APPLE)
  add_source_cflags(ADM_runOneJob.cpp "-DAVIDEMUX_MAJOR_MINOR=\"${AVIDEMUX_MAJOR_MINOR}\"")
  TARGET_LINK_LIBRARIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")
  # for Leopard but it doesn't hurt Tiger
  TARGET_LINK_LIBRARIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF()

TARGET_LINK_LIBRARIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE ADM_coreJobs ADM_coreUtils6 ADM_core6)
#
TARGET_LINK_LIBRARIES(avidemux3_jobs_${QT_EXTENSION} PRIVATE "-lm -lstdc++")
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
# Install
# # # # # # # # # # # # # # # # # # # # # # # # # # # ################
ADM_INSTALL_BIN(avidemux3_jobs_${QT_EXTENSION})
#
