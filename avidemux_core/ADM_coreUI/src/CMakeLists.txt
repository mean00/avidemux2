SET(ADM_coreUI_SRCS 
	DIA_factory.cpp
	DIA_fileSel.cpp
        DIA_coreToolkit.cpp
        DIA_working.cpp
        DIA_encoding.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreUI6 ${ADM_coreUI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreUI6 ADM_core6)

ADM_INSTALL_LIB(ADM_coreUI6)
