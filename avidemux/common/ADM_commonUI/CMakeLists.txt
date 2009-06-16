SET(ADM_LIB ADM_commonUI)

SET(${ADM_LIB}_SRCS
DIA_resizeWiz.cpp  
DIA_v2v.cpp
DIA_audioFilter.cpp   
DIA_gototime.cpp    
DIA_postproc.cpp  
DIA_bitrateHisto.cpp  
DIA_prefs.cpp     
DIA_SVCD.cpp       
DIA_builtin.cpp       
DIA_lavDecoder.cpp  
DIA_requant.cpp   
#DIA_ocr.cpp             
DIA_tsSub.cpp        
DIA_jobs_save.cpp
DIA_flv1.cpp            
DIA_plugins.cpp
)

SET(OBSOLETE_SRCS
DIA_mjpeg.cpp     
DIA_DVDff.cpp           
DIA_lavcodec.cpp    
DIA_flyThreshold.cpp 
DIA_flyParticle.cpp 
DIA_flyEraser.cpp 
DIA_flySwissArmyKnife.cpp
DIA_flyContrast.cpp  
DIA_flySrtPos.cpp   
DIA_srt.cpp        
DIA_flyCrop.cpp      
DIA_flyAsharp.cpp    
DIA_flyHue.cpp       
DIA_flyEq2.cpp        

)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})

