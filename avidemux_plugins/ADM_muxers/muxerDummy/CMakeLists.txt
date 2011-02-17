include(mx_plugin)
SET(ADM_dummy_SRCS 
	ADM_dummy.cpp
	ADM_dummyPlugin.cpp
)

ADD_MUXER(ADM_mx_dummy ${ADM_dummy_SRCS})

INIT_MUXER(ADM_mx_dummy)
INSTALL_MUXER(ADM_mx_dummy)
