SET(ADM_LIB ADM_dialogGtk)

SET(${ADM_LIB}_SRCS
DIA_about.cpp    
 DIA_calculator.cpp  
 DIA_preview.cpp    
 DIA_xvid4.cpp
 DIA_encoding.cpp  
 DIA_ocr.cpp      
 DIA_properties.cpp 
 DIA_xvid.cpp
 DIA_idx_pg.cpp    
 #DIA_partial.cpp  
 DIA_quota.cpp      
 DIA_vobsub.cpp
 DIA_jobs.cpp      
 DIA_working.cpp
 DIA_busy.cpp     
 toolkit_dialog.cpp
 TLK_filesel.cpp
)
SET(OBSOLETE_SRCS
 DIA_exLame.cpp
 DIA_pipe.cpp
 DIA_threshold.cpp
 DIA_SwissArmyKnife.cpp
 DIA_particle.cpp
 DIA_eraser.cpp

 DIA_hue.cpp       
 DIA_srt.cpp     
 DIA_crop.cpp      
DIA_eq2.cpp       
DIA_asharp.cpp   
 DIA_contrast.cpp    
 DIA_audioTracks.cpp


)
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
ADD_DEFINITIONS( -DADM_SUBVERSION=\"${ADM_SUBVERSION}\")
