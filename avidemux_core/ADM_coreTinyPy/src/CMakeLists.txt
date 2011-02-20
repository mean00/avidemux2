SET(ADM_coreTinyPy_SRCS 
        tinypy.cpp
        ADM_tinypy.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreTinyPy6 ${ADM_coreTinyPy_SRCS})
ADD_DEFINITIONS(-DCPYTHON_MOD)
IF (APPLE)
	TARGET_LINK_LIBRARIES(ADM_coreTinyPy6 -Wl,-read_only_relocs,suppress)
ENDIF (APPLE)
INCLUDE_DIRECTORIES(../include)
TARGET_LINK_LIBRARIES(ADM_coreTinyPy6 ADM_core6 ADM_coreUI6  ADM_coreUtils6)
ADM_INSTALL_LIB(ADM_coreTinyPy6)
