SET(ADM_script_SRCS
	ADM_AvidemuxAudio.cpp  ADM_AvidemuxVideo.cpp    ADM_JSAvidemux.cpp       ADM_JSDirectorySearch.cpp  ADM_JSGlobal.cpp
	ADM_Avidemux.cpp       ADM_JSAvidemuxAudio.cpp  ADM_JSAvidemuxVideo.cpp  ADM_JSFunctions.cpp        DirectorySearch.cpp
        ADM_JSFunctionsNonRegFactory.cpp)

ADD_LIBRARY(ADM_script STATIC ${ADM_script_SRCS})
ADD_TARGET_CFLAGS(ADM_script "-DJS_THREADSAFE -DXP_UNIX")
