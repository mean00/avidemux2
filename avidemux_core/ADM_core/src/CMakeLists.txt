SET(ADM_core_SRCS 
	ADM_cpuCap.cpp  ADM_memcpy.cpp  ADM_memsupport.cpp  ADM_threads.cpp  ADM_win32.cpp
	ADM_memory.cpp  ADM_misc.cpp  TLK_clock.cpp  ADM_crashdump.cpp  ADM_fileio.cpp ADM_dynamicLoading.cpp
        ADM_debug.cpp 
)

ADD_LIBRARY(ADM_core6 SHARED ${ADM_core_SRCS})
ADD_TARGET_DEFINITIONS(ADM_core6 ADM_DEBUG FIND_LEAKS)

IF (MINGW)
	TARGET_LINK_LIBRARIES(ADM_core6 "${PTHREAD_LIBRARIES}" imagehlp ws2_32)
	ADD_TARGET_CFLAGS(ADM_core6 "-I${PTHREAD_INCLUDE_DIR}")
ELSEIF (APPLE)
	ADD_TARGET_LDFLAGS(ADM_core6 "-framework Carbon")
ENDIF (MINGW)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)

# pthreads

ADM_LINK_THREAD(ADM_core6)
ADM_INSTALL_LIB(ADM_core6)
