SET(ADM_LIB ADM_xvidratectl)

SET(${ADM_LIB}_SRCS 
	ADM_oldxvid.cpp  ADM_xvidratectl.cpp  ADM_xvidratectlVBV.cpp)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
