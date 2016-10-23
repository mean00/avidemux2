
SET(ADM_dxva2_SRCS
ADM_coreDxva2.cpp
)	
#*************************************************
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coredxva26 ${ADM_dxva2_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coredxva2/include)
TARGET_LINK_LIBRARIES(ADM_coredxva26 ADM_core6 ADM_coreUI6 dxva2)
ADM_INSTALL_LIB(ADM_coredxva26)
