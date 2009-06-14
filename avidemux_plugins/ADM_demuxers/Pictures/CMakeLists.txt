# Demuxer for picture
# PNG etc...

include(dm_plugin)

SET(ADM_pic_SRCS 
	ADM_picPlugin.cpp
	ADM_pics.cpp
)

ADD_LIBRARY(ADM_dm_pic SHARED ${ADM_pic_SRCS})

INIT_DEMUXER(ADM_dm_pic)
INSTALL_DEMUXER(ADM_dm_pic)

