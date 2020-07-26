SET(ADM_core_SRCS
	ADM_cpuCap.cpp  ADM_memsupport.cpp  ADM_threads.cpp  ADM_win32.cpp  ADM_misc.cpp  ADM_debug.cpp
	TLK_clock.cpp  ADM_fileio.cpp  ADM_dynamicLoading.cpp  ADM_queue.cpp  ADM_benchmark.cpp
        ADM_coreTranslator.cpp
        ADM_prettyPrint.cpp
)
IF (MINGW)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_mingw.cpp ADM_folder_win32.cpp ADM_folder_mingw.cpp ADM_win32_mingw.cpp )
ELSEIF (APPLE)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_apple.cpp ADM_folder_mac.cpp)
    IF(CREATE_BUNDLE)
       ADD_DEFINITIONS(-DCREATE_BUNDLE)
    ENDIF(CREATE_BUNDLE)
ELSEIF (UNIX)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_unix.cpp ADM_folder_linux.cpp)
ELSEIF (MSVC)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_vs.cpp ADM_folder_win32.cpp ADM_folder_vs.cpp ADM_win32_vs.cpp)
ELSE (MINGW)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_crashdump_other.cpp ADM_folder_linux.cpp) # Default
ENDIF (MINGW)


IF (NOT APPLE)
	SET(ADM_core_SRCS ${ADM_core_SRCS} ADM_memcpy.cpp)
ENDIF (NOT APPLE)

if (ADM_DEBUG AND FIND_LEAKS)
	set(ADM_core_SRCS ${ADM_core_SRCS} ADM_nvwa/debug_new.cpp)

	if (MINGW)
		set(ADM_core_SRCS ${ADM_core_SRCS} ADM_nvwa/addr2line.cpp)
	endif (MINGW)
endif (ADM_DEBUG AND FIND_LEAKS)

set(ADM_core_SRCS ${ADM_core_SRCS})

YASMIFY(asmFiles cpuid adm_emms)
MESSAGE(STATUS "List of asm files ${asmFiles}")
ADM_ADD_SHARED_LIBRARY(ADM_core6 ${ADM_core_SRCS} ${asmFiles})
include_directories("${PTHREAD_INCLUDE_DIR}")

IF (WIN32)
	target_link_libraries(ADM_core6 imagehlp ws2_32)
ELSEIF (APPLE)
	target_link_libraries(ADM_core6 "-framework Carbon")
elseif (UNIX)
	target_link_libraries(ADM_core6 dl m)
ENDIF (WIN32)

if (HAVE_EXECINFO)
	include_directories("${EXECINFO_INCLUDE_DIR}")
	target_link_libraries(ADM_core6 "${EXECINFO_LIBRARY_DIR}")
endif (HAVE_EXECINFO)

if (ADM_DEBUG AND FIND_LEAKS)
	add_definitions(-D_DEBUG_NEW_ERROR_ACTION=NULL -DPACKAGE=1 -DPACKAGE_VERSION=1)

	if (UNIX)
		add_definitions(-fPIC)
	endif (UNIX)

	if (MINGW)
		target_link_libraries(ADM_core6  bfd  iberty  imagehlp)
	endif (MINGW)
endif (ADM_DEBUG AND FIND_LEAKS)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)

target_link_libraries(ADM_core6  ADM_libavutil) # To get the CPU functions of libavutil
ADM_LINK_THREAD(ADM_core6)
ADM_INSTALL_LIB(ADM_core6)
