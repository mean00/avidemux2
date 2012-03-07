SET(ADM_scriptDF_SRCS
        ADM_JSDFMenu.cpp
        ADM_JSDFToggle.cpp
        ADM_JSDFInteger.cpp
        ADM_JSDialogFactory.cpp
        ADM_JSDialogFactory_if.cpp 
        
)

ADD_LIBRARY(ADM_scriptDF STATIC ${ADM_scriptDF_SRCS})
include_directories(../../../ADM_coreScript/include)

if (USE_SYSTEM_SPIDERMONKEY)
	include_directories(${SPIDERMONKEY_INCLUDE_DIR})
	target_link_libraries(ADM_scriptDF ${SPIDERMONKEY_LIBRARY_DIR})
else (USE_SYSTEM_SPIDERMONKEY)
	include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_smjs)
	target_link_libraries(ADM_scriptDF ADM_smjs6)
endif (USE_SYSTEM_SPIDERMONKEY)