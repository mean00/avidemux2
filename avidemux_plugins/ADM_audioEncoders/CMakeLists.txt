#######################################
include (admCheckAudioEncoderLibs)
ADD_SUBDIRECTORY(twolame)
ADD_SUBDIRECTORY(pcm)
ADD_SUBDIRECTORY(lavcodec)

if(USE_LAME)
ADD_SUBDIRECTORY(lame)
endif(USE_LAME)

if(USE_AFTEN)
ADD_SUBDIRECTORY(aften)
endif(USE_AFTEN)

if(USE_VORBIS)
ADD_SUBDIRECTORY(vorbis)
endif(USE_VORBIS)

if(USE_FAAC)
ADD_SUBDIRECTORY(faac)
endif(USE_FAAC)
MESSAGE("***  Audio Encoder ***")
ADM_DISPLAY("Aften     " "${USE_AFTEN}")
ADM_DISPLAY("LAME      " "${USE_LAME}")
ADM_DISPLAY("FAAC      " "${USE_FAAC}")
ADM_DISPLAY("Vorbis    " "${USE_VORBIS}")
MESSAGE("***  /Audio Encoder  ***")



