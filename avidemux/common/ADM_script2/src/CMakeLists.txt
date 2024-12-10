SET(ADM_script_SRCS
 ADM_script.cpp
)

include_directories(../include)
include_directories(../../ADM_editor)

ADD_LIBRARY(ADM_script6 STATIC ${ADM_script_SRCS})
TARGET_LINK_LIBRARIES(ADM_script6 PRIVATE ADM_coreScript ADM_editor6_interface)
