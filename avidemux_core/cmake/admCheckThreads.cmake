INCLUDE(FindThreads)

# - This module determines the thread library of the system.
# The following variables are set
#  PTHREAD_FOUND - system has pthreads
#  PTHREAD_INCLUDE_DIR - the pthreads include directory
#  PTHREAD_LIBRARIES - the libraries needed to use pthreads

INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)

IF(NOT PTHREAD_CHECKED)

  FIND_PACKAGE(Threads)
  IF(Threads_FOUND)
    SET(PTHREAD_FOUND 1)
    ADD_LIBRARY(adm_pthread INTERFACE)
    TARGET_LINK_LIBRARIES(adm_pthread INTERFACE Threads::Threads)
    IF(CMAKE_THREAD_LIBS_INIT)
      TARGET_LINK_LIBRARIES(adm_pthread INTERFACE ${CMAKE_THREAD_LIBS_INIT})
    ENDIF()
  ELSE() # Mostly windows
    FIND_HEADER_AND_LIB(raw_PTHREAD pthread.h pthreads)
    IF(NOT raw_PTHREAD_LIBRARIES)
      FIND_HEADER_AND_LIB(raw_PTHREAD pthread.h pthread)
      IF(NOT raw_PTHREAD_LIBRARIES)
        FIND_HEADER_AND_LIB(raw_PTHREAD pthread.h libwinpthread)
        IF(NOT raw_PTHREAD_LIBRARIES)
          FIND_HEADER_AND_LIB(raw_PTHREAD pthread.h pthreadGC2)
        ENDIF()
      ENDIF()
    ENDIF()

    IF(NOT raw_PTHREAD_LIBRARIES)
      MESSAGE(FATAL_ERROR "Cannot locate pthread")
    ENDIF()

    SET(PTHREAD_INCLUDE_DIR ${raw_PTHREAD_INCLUDE_DIR})
    SET(PTHREAD_LIBRARIES ${raw_PTHREAD_LIBRARIES})

    MARK_AS_ADVANCED(PTHREAD_LIBRARIES)

    SET(PTHREAD_FOUND 1)
    ADD_LIBRARY(adm_pthread INTERFACE)
    MESSAGE(STATUS "Using pthread Lib=${PTHREAD_LIBRARIES} Inc=${PTHREAD_INCLUDE_DIR}")
    TARGET_LINK_LIBRARIES(adm_pthread INTERFACE ${PTHREAD_LIBRARIES})
    TARGET_INCLUDE_DIRECTORIES(adm_pthread INTERFACE ${PTHREAD_INCLUDE_DIR})
  ENDIF()
  SET(PTHREAD_CHECKED 1)
ENDIF()

MACRO(ADM_LINK_THREAD target)
  TARGET_LINK_LIBRARIES(${target} PRIVATE adm_pthread)
  #IF(CMAKE_THREAD_LIBS_INIT)
  #TARGET_LINK_LIBRARIES(${target} ${CMAKE_THREAD_LIBS_INIT})
  #ELSE()
  #IF(WIN32 OR (UNIX AND PTHREAD_FOUND))
  #TARGET_LINK_LIBRARIES(${target} PUBLIC "${PTHREAD_LIBRARIES}")
  #ENDIF()
  #ENDIF()
ENDMACRO()
