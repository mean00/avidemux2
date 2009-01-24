#######################################
include (admCheckAudioEncoderLibs)
include (admConfigSummary)
MESSAGE("***  Audio Codec  ***")
ADM_DISPLAY("Aften     " "${USE_AFTEN}")
ADM_DISPLAY("LAME      " "${USE_LAME}")
ADM_DISPLAY("FAAC      " "${USE_FAAC}")
ADM_DISPLAY("Vorbis    " "${USE_VORBIS}")
MESSAGE("***  /Audio Codec  ***")


ADD_SUBDIRECTORY(twolame)
ADD_SUBDIRECTORY(pcm)
ADD_SUBDIRECTORY(lavcodec)

if(USE_LAME)
ADD_SUBDIRECTORY(lame)
endif(USE_LAME)

if(USE_AFTEN)
# not tested ADD_SUBDIRECTORY(aften)
endif(USE_AFTEN)

if(USE_VORBIS)
ADD_SUBDIRECTORY(vorbis)
endif(USE_VORBIS)

if(USE_FAAC)
ADD_SUBDIRECTORY(faac)
endif(USE_FAAC)
