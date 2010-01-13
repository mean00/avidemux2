SET(ADM_imageLoader_SRCS 
ADM_imageLoader.cpp
)

ADD_LIBRARY(ADM_coreImageLoader6 SHARED ${ADM_imageLoader_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreImageLoader6 
                                          ADM_core6
                                          ADM_coreImage6 
                                          ADM_coreUI6
                                          ADM_coreUtils6
                                          ADM_coreVideoCodec6)
include_directories(../include)
ADM_INSTALL_LIB(ADM_coreImageLoader6)
