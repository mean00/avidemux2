SET(ADM_coreUI_SRCS 
	DIA_factory.cpp
	DIA_fileSel.cpp
        DIA_coreToolkit.cpp
        DIA_working.cpp
        DIA_encoding.cpp
)

ADD_LIBRARY(ADM_coreUI SHARED ${ADM_coreUI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreUI ADM_core)

INSTALL(TARGETS ADM_coreUI RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
