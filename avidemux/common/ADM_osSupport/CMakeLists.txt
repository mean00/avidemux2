SET(ADM_osSupport_SRCS 
ADM_crashHook.cpp 
ADM_misc.cpp 
ADM_queue.cpp 
)

ADD_LIBRARY(ADM_osSupport6 STATIC ${ADM_osSupport_SRCS})

