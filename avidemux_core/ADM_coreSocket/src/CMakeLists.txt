SET(ADMcoreSocket_SRCS
  ADM_coreSocket.cpp
  ADM_coreCommandSocket.cpp
  ADM_avsProtocol.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreSocket6 ${ADMcoreSocket_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreSocket6  PUBLIC ADM_core6 ADM_coreUtils6)
TARGET_INCLUDE_DIRECTORIES(ADM_coreSocket6 PRIVATE ../include)

IF(WIN32)
  TARGET_LINK_LIBRARIES(ADM_coreSocket6 PUBLIC wsock32)
ENDIF()

ADM_INSTALL_LIB(ADM_coreSocket6)
