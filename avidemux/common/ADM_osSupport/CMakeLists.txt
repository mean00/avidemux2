SET(ADM_osSupport_SRCS
ADM_crashHook.cpp
ADM_misc.cpp
)

ADD_LIBRARY(ADM_osSupport6 STATIC ${ADM_osSupport_SRCS})
TARGET_LINK_LIBRARIES(ADM_osSupport6 PRIVATE  ADM_editor6_interface)

