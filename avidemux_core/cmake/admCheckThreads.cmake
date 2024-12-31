
# - This module determines the thread library of the system.
# The following variables are set
#  PTHREAD_FOUND - system has pthreads
#  PTHREAD_INCLUDE_DIR - the pthreads include directory
#  PTHREAD_LIBRARIES - the libraries needed to use pthreads

INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)

MESSAGE(STATUS "Checking Thread support  ")
IF(NOT PTHREAD_CHECKED)
  MESSAGE(STATUS "Environment is ver=${MSVC_VERSION} msvc=${MSVC}")
  IF(MSVC)
    MESSAGE(STATUS "Looking for MSVC Threads4W")
    FIND_PACKAGE(PThreads4W REQUIRED)
    ADD_LIBRARY(adm_pthread INTERFACE)
    TARGET_LINK_LIBRARIES(adm_pthread INTERFACE PThreads4W::PThreads4W)
    TARGET_INCLUDE_DIRECTORIES(adm_pthread INTERFACE ${PThreads4W_INCLUDE_DIR})
  ELSE()
    MESSAGE(STATUS "Looking for Threads::Threads")
    INCLUDE(FindThreads)
    FIND_PACKAGE(Threads)
    IF(Threads_FOUND)
      MESSAGE(STATUS "Found Threads::Threads")
      SET(PTHREAD_FOUND 1)
      ADD_LIBRARY(adm_pthread INTERFACE)
      TARGET_LINK_LIBRARIES(adm_pthread INTERFACE Threads::Threads)
      IF(CMAKE_THREAD_LIBS_INIT)
        TARGET_LINK_LIBRARIES(adm_pthread INTERFACE ${CMAKE_THREAD_LIBS_INIT})
      ENDIF()
    ENDIF()
    #MESSAGE(FATAL_ERROR "STOPPING PTHREAD HERE")
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
