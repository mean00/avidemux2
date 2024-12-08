SET(ADM_imageLoader_SRCS
ADM_imageLoader.cpp
)
ADM_ADD_SHARED_LIBRARY(ADM_coreImageLoader6 ${ADM_imageLoader_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreImageLoader6 PUBLIC
                                          ADM_core6
                                          ADM_coreImage6
                                          ADM_coreUI6
                                          ADM_coreUtils6
                                          ADM_coreVideoCodec6)
TARGET_INCLUDE_DIRECTORIES(ADM_coreImageLoader6 PRIVATE ../include)
ADM_INSTALL_LIB(ADM_coreImageLoader6)
