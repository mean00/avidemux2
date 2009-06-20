SET(ADM_coreUI_SRCS 
	DIA_factory.cpp
	DIA_fileSel.cpp
        DIA_coreToolkit.cpp
        DIA_working.cpp
        DIA_encoding.cpp
)

ADD_LIBRARY(ADM_coreUI SHARED ${ADM_coreUI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreUI ADM_core)

ADM_INSTALL_LIB(ADM_coreUI)
