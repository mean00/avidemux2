SET(ADM_LIB ADM_mplex)

SET(${ADM_LIB}_SRCS 
	ac3strm_in.cpp      ADM_mplexout.cpp   audiostrm_out.cpp  dtsstrm_in.cpp  lpcmstrm_in.cpp   multiplexor.cpp   stream_params.cpp  videostrm_in.cpp
	ADM_mplexin.cpp     ADM_mthread.cpp    bits.cpp           inputstrm.cpp   padstrm.cpp       systems.cpp       videostrm_out.cpp
	ADM_mplexmuxer.cpp  ADM_transfert.cpp  interact.cpp       mpastrm_in.cpp  stillsstream.cpp  mpegconsts.c      mjpeg_logging.c    decodebufmodel.cpp
	yuv4mpeg.c          cpu_accel.c)

ADD_ADM_LIB_ALL_TARGETS(${ADM_LIB} ${${ADM_LIB}_SRCS})
ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-I${CMAKE_CURRENT_SOURCE_DIR} -DHAVE_FMAX -DRETSIGTYPE=void")

IF (NOT APPLE)
    ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_MEMALIGN")
ENDIF (NOT APPLE)

IF (ADM_CPU_X86)
    ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_X86CPU")
ELSEIF (ADM_CPU_ALTIVEC)
    ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_ALTIVEC")
ENDIF (ADM_CPU_X86)