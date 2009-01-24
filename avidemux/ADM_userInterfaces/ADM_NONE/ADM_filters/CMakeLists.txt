SET(ADM_LIB ADM_dialogFiltersNone)

SET(${ADM_LIB}_SRCS 
	filter_none.cpp)

ADD_ADM_LIB_CLI_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})