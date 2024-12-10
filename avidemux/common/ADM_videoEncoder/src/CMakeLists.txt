SET(ADM_internalVideoEncoder_SRCS
 ADM_VideoEncoders.cpp
        ADM_dynVideoEncoder.cpp
)
ADD_LIBRARY(ADM_videoEncoder6_interface INTERFACE)
TARGET_INCLUDE_DIRECTORIES(ADM_videoEncoder6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
ADD_LIBRARY(ADM_videoEncoder6 STATIC ${ADM_internalVideoEncoder_SRCS})
TARGET_LINK_LIBRARIES(ADM_videoEncoder6 PRIVATE ADM_videoEncoder6_interface ADM_videoFilter6_interface)
