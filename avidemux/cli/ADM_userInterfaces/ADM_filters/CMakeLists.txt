SET(ADM_LIB ADM_filtersCli6)

SET(${ADM_LIB}_SRCS 
	filter_none.cpp)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
