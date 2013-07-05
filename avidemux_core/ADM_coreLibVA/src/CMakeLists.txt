
SET(ADM_libva_SRCS
ADM_coreLibVA.cpp
)	
#*************************************************
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreLibVA6 ${ADM_libva_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ADM_coreLibVA/include)
TARGET_LINK_LIBRARIES(ADM_coreLibVA6 va va-x11)
TARGET_LINK_LIBRARIES(ADM_coreLibVA6 ADM_core6 ADM_coreUI6 )
ADM_INSTALL_LIB(ADM_coreLibVA6)
