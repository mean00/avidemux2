# Demuxer for picture
# PNG etc...

include(dm_plugin)

SET(ADM_pic_SRCS 
	ADM_picPlugin.cpp
	ADM_pics.cpp
)

ADD_DEMUXER(ADM_dm_pic ${ADM_pic_SRCS})
TARGET_LINK_LIBRARIES(ADM_dm_pic ADM_coreImageLoader6)
INIT_DEMUXER(ADM_dm_pic)
INSTALL_DEMUXER(ADM_dm_pic)

