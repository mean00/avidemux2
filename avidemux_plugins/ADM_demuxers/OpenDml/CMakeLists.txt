include(dm_plugin)
SET(ADM_openDML_SRCS 
ADM_odml_audio.cpp 
ADM_odml_regular.cpp 
ADM_openDMLDepack.cpp   
ADM_odml_odml.cpp  
ADM_openDML.cpp      
ADM_riff.cpp
ADM_openDMLPlugin.cpp
)
ADD_DEMUXER(ADM_dm_opendml ${ADM_openDML_SRCS})

INIT_DEMUXER(ADM_dm_opendml)
INSTALL_DEMUXER(ADM_dm_opendml)
