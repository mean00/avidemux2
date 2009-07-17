SET(ADM_coreUI_SRCS 
	DIA_factory.cpp
	DIA_fileSel.cpp
        DIA_coreToolkit.cpp
        DIA_working.cpp
        DIA_encoding.cpp
)

ADD_LIBRARY(ADM_coreUI6 SHARED ${ADM_coreUI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreUI6 ADM_core6)

ADM_INSTALL_LIB(ADM_coreUI6)
