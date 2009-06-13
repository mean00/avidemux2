
SET(ADMaudioParser_SRCS
ADM_aacinfo.cpp 
ADM_mp3info.cpp  
ADM_a52info.cpp 
ADM_dcainfo.cpp  
)	
#*************************************************
ADD_DEFINITIONS( "-DADM_LEGACY_PROGGY")
ADD_DEFINITIONS( "-DHAVE_LRINT -DHAVE_LRINTF ")
ADD_DEFINITIONS( "-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0")
ADD_DEFINITIONS( "-DPACKAGE=`"SRC`"")
ADD_DEFINITIONS( "-DVERSION=`"0.1.4`"")
#*************************************************
ADD_LIBRARY(ADM_audioParser SHARED ${ADMaudioParser_SRCS})
TARGET_LINK_LIBRARIES(ADM_audioParser ADM_core ADM_coreUI)

INSTALL(TARGETS ADM_audioParser RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
