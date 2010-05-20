SET(ADM_coreTinyPy_SRCS 
        tinypy.c
        ADM_tinypy.cpp
)

ADD_LIBRARY(ADM_coreTinyPy6 SHARED ${ADM_coreTinyPy_SRCS})
IF (APPLE)
	TARGET_LINK_LIBRARIES(ADM_coreTinyPy6 -Wl,-read_only_relocs,suppress)
ENDIF (APPLE)
INCLUDE_DIRECTORIES(../include)
TARGET_LINK_LIBRARIES(ADM_coreTinyPy6 ADM_core6 ADM_coreUI6  ADM_coreUtils6)
ADM_INSTALL_LIB(ADM_coreTinyPy6)
