SET(ADMcoreSocket_SRCS
        ADM_coreSocket.cpp
        ADM_coreCommandSocket.cpp
)	
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreSocket6 ${ADMcoreSocket_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreSocket6  ADM_core6 ADM_coreUtils6)
INCLUDE_DIRECTORIES(../include)
if(WIN32)
        target_link_libraries(ADM_coreSocket6 wsock32)
endif(WIN32)
ADM_INSTALL_LIB(ADM_coreSocket6)


