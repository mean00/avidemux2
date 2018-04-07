SET(ADM_LIB ADM_commonUI6)
IF(USE_VIDEOTOOLBOX)
    ADD_DEFINITIONS("-DUSE_VIDEOTOOLBOX")
ENDIF(USE_VIDEOTOOLBOX)

SET(${ADM_LIB}_SRCS
DIA_audioFilter.cpp   
DIA_gototime.cpp    
DIA_postproc.cpp  
DIA_bitrateHisto.cpp  
DIA_prefs.cpp     
DIA_builtin.cpp       
DIA_jobs_save.cpp
DIA_plugins.cpp
)


ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})

