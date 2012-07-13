SET(ADM_script_SRCS
	ADM_script.cpp
)

include_directories(../include)
include_directories(../../ADM_editor)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScript/include)

ADD_LIBRARY(ADM_script6 STATIC ${ADM_script_SRCS})
target_link_libraries(ADM_script6 ADM_coreScript)