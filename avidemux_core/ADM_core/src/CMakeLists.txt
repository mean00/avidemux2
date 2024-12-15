#
# This is the top core library
# *everything* should link to it
#
SET(ADM_LIB ADM_core6)
ADM_ADD_SHARED_LIBRARY(ADM_core6)

# This should be the only place we directly link/include pthread
find_package(Threads REQUIRED)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC Threads::Threads)

TARGET_INCLUDE_DIRECTORIES(ADM_core6 PUBLIC "${PTHREAD_INCLUDE_DIR}")
ADM_LINK_THREAD(ADM_core6)
TARGET_LINK_LIBRARIES(ADM_core6 PUBLIC ADM_libavutil)
TARGET_LINK_LIBRARIES(ADM_core6 PUBLIC adm_gettext)

TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_cpuCap.cpp  ADM_memsupport.cpp  ADM_threads.cpp
  ADM_win32.cpp  ADM_misc.cpp ADM_debug.cpp TLK_clock.cpp  ADM_fileio.cpp
  ADM_dynamicLoading.cpp  ADM_queue.cpp
  ADM_benchmark.cpp ADM_coreTranslator.cpp ADM_prettyPrint.cpp
)

MACRO(ADDSRC xfiles)
  TARGET_SOURCES(ADM_core6 PRIVATE ${xfiles} ${ARGN})
  #MESSAGE(STATUS " ---Adding ${xfiles}")
ENDMACRO()

IF(MINGW)
  ADDSRC(ADM_crashdump_mingw.cpp ADM_folder_win32.cpp ADM_folder_mingw.cpp ADM_win32_mingw.cpp )
ELSEIF(APPLE)
  ADDSRC(ADM_crashdump_apple.cpp ADM_folder_mac.cpp)
  IF(CREATE_BUNDLE)
    TARGET_ADD_DEFINITIONS(${ADM_LIB} PRIVATE CREATE_BUNDLE)
  ENDIF()
ELSEIF(UNIX)
  ADDSRC(ADM_crashdump_unix.cpp ADM_folder_linux.cpp)
ELSEIF(MSVC)
  ADDSRC(ADM_crashdump_vs.cpp ADM_folder_win32.cpp ADM_folder_vs.cpp ADM_win32_vs.cpp)
ELSE()
  ADDSRC(ADM_crashdump_other.cpp ADM_folder_linux.cpp) # Default
ENDIF()

IF(NOT APPLE)
  ADDSRC(ADM_memcpy.cpp)
ENDIF()

IF(ADM_DEBUG AND FIND_LEAKS)
  ADDSRC(ADM_nvwa/debug_new.cpp)
  IF(MINGW)
    ADDSRC(ADM_nvwa/addr2line.cpp)
  ENDIF()
ENDIF()

YASMIFY(asmFiles cpuid adm_emms)
MESSAGE(STATUS "List of asm files ${asmFiles}")
ADDSRC(${asmFiles})

IF(WIN32)
  TARGET_LINK_LIBRARIES(ADM_core6 PUBLIC imagehlp ws2_32 powrprof)
ELSEIF(APPLE)
  TARGET_LINK_LIBRARIES(ADM_core6 PUBLIC "-framework Carbon")
ELSE()
  TARGET_LINK_LIBRARIES(ADM_core6 PUBLIC dl m)
ENDIF()

IF(HAVE_EXECINFO)
  TARGET_INCLUDE_DIRECTORIES(ADM_core6 PRIVATE "${EXECINFO_INCLUDE_DIR}")
  TARGET_LINK_LIBRARIES(ADM_core6 PRIVATE "${EXECINFO_LIBRARY_DIR}")
ENDIF()

IF(ADM_DEBUG AND FIND_LEAKS)
  TARGET_COMPILE_DEFINITIONS(ADM_core6 PUBLIC _DEBUG_NEW_ERROR_ACTION=NULL PACKAGE=1 PACKAGE_VERSION=1)

  IF(UNIX)
    TARGET_COMPILE_OPTIONS(ADM_core6 PUBLIC -fPIC)
  ENDIF()

  IF(MINGW)
    TARGET_LINK_LIBRARIES(ADM_core6  PUBLIC bfd  iberty  imagehlp)
  ENDIF()
ENDIF()

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)

# To get the CPU functions of libavutil
ADM_LINK_THREAD(ADM_core6)
ADM_INSTALL_LIB(ADM_core6)
