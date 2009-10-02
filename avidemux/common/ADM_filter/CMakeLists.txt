SET(ADM_filter_SRCS 
filter.cpp  
filter_declaration.cpp  
filter_saveload.cpp  
filter_autoFilter.cpp
vidVCD.cpp)

ADD_LIBRARY(ADM_filter6 STATIC ${ADM_filter_SRCS})
ADD_TARGET_CFLAGS(ADM_filter6 -I${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS})
