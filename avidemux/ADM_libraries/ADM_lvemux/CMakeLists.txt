SET(ADM_LIB ADM_lvemux)

SET(${ADM_LIB}_SRCS 
	ADM_muxer.cpp
	mux_out.c)

ADD_ADM_LIB_ALL_TARGETS(${ADM_LIB} ${${ADM_LIB}_SRCS})