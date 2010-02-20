INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_DEFINITIONS(-DHAVE_AV_CONFIG_H -fomit-frame-pointer -fno-math-errno)
# Compiler flags
###########################""
IF (UNIX)
	# CMake automatically adds -fPIC so remove it
	STRING(REPLACE "-fPIC" "" CMAKE_SHARED_LIBRARY_C_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
	STRING(REPLACE "-fPIC" "" CMAKE_SHARED_LIBRARY_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
ENDIF (UNIX)

IF (ADM_CPU_X86_64 OR ADM_CPU_PPC)
	ADD_DEFINITIONS(-fPIC -DPIC)
ELSEIF (APPLE)
	ADD_DEFINITIONS(-mdynamic-no-pic)
ENDIF (ADM_CPU_X86_64 OR ADM_CPU_PPC)
include_directories(ffmpeg_config)
# Linker flags
IF (APPLE)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-single_module -Wl,-read_only_relocs,suppress")
ENDIF (APPLE)

IF (UNIX AND NOT APPLE)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -export-dynamic -Wl,-Bsymbolic")
ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-image-base")
ENDIF (WIN32)
#
#
ADD_SUBDIRECTORY(libavcodec)
ADD_SUBDIRECTORY(libavformat)
ADD_SUBDIRECTORY(libavutil)
ADD_SUBDIRECTORY(libswscale)
ADD_SUBDIRECTORY(libpostproc)
