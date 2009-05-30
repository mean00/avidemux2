SET(ADM_filter_SRCS 
ADM_vidDummyFilter.cpp  
filter.cpp  
filter_declaration.cpp  
filter_saveload.cpp  
filter_autoFilter.cpp
vidVCD.cpp)

ADD_LIBRARY(ADM_filter STATIC ${ADM_filter_SRCS})
ADD_TARGET_CFLAGS(ADM_filter -I${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS})
