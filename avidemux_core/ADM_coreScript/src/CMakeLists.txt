SET(admScript_SRCS
	ADM_scriptDFInteger.cpp
	ADM_scriptDFMenu.cpp
	ADM_scriptDFToggle.cpp
	ADM_scriptDialogFactory.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreScript ${admScript_SRCS})
ADM_INSTALL_LIB(ADM_coreScript)

include_directories(../include)

target_link_libraries(ADM_coreScript ADM_coreUtils6)