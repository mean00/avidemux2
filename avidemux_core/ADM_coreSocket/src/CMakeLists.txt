SET(ADMcoreSocket_SRCS
        ADM_coreSocket.cpp
        ADM_coreCommandSocket.cpp)	

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreSocket6 ${ADMcoreSocket_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreSocket6  ADM_core6 ADM_coreUtils6)
INCLUDE_DIRECTORIES(../include "${PTHREAD_INCLUDE_DIR}")

if (WIN32)
	target_link_libraries(ADM_coreSocket6 wsock32)
endif (WIN32)

ADM_INSTALL_LIB(ADM_coreSocket6)