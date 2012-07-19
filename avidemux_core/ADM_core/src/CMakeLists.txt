SET(ADM_core_SRCS
	ADM_cpuCap.cpp  ADM_memsupport.cpp  ADM_threads.cpp  ADM_win32.cpp
	ADM_memory.cpp  ADM_misc.cpp  TLK_clock.cpp  ADM_fileio.cpp ADM_dynamicLoading.cpp
        ADM_debug.cpp
        ADM_queue.cpp
        ADM_benchmark.cpp
)

IF (MINGW)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_mswin.cpp)
ELSEIF (APPLE)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_apple.cpp)
ELSE (MINGW)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_unix.cpp)
ENDIF (MINGW)

IF (NOT APPLE)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_memcpy.cpp)
ENDIF (NOT APPLE)

ADM_ADD_SHARED_LIBRARY(ADM_core6 ${ADM_core_SRCS})
include_directories("${PTHREAD_INCLUDE_DIR}")

IF (MINGW)
	TARGET_LINK_LIBRARIES(ADM_core6 "${PTHREAD_LIBRARIES}" imagehlp ws2_32)
ELSEIF (APPLE)
	target_link_libraries(ADM_core6 "-framework Carbon")
ENDIF (MINGW)

if (HAVE_EXECINFO)
	include_directories("${LIBEXECINFO_INCLUDE_DIR}")
	target_link_libraries(ADM_core6 "${LIBEXECINFO_LIBRARY_DIR}")
endif (HAVE_EXECINFO)

if (UNIX)
	target_link_libraries(ADM_core6 dl m)
endif (UNIX)

if (ADM_DEBUG AND FIND_LEAKS)
	add_subdirectory(ADM_nvwa)
	target_link_libraries(ADM_core6 ADM_nvwa)
endif (ADM_DEBUG AND FIND_LEAKS)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)

ADM_LINK_THREAD(ADM_core6)
ADM_INSTALL_LIB(ADM_core6)
