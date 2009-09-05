include(mx_plugin)
SET(ADM_LIB ADM_mx_mplex)

SET(${ADM_LIB}_SRCS 
	ADM_mplexMuxer.cpp  
        ADM_mplexInput.cpp   
	ADM_mplexOutput.cpp     
	ac3strm_in.cpp      
        audiostrm_out.cpp  dtsstrm_in.cpp  lpcmstrm_in.cpp   multiplexor.cpp   stream_params.cpp  videostrm_in.cpp
        ADM_mthread.cpp    bits.cpp           inputstrm.cpp   padstrm.cpp       systems.cpp       videostrm_out.cpp
        ADM_transfert.cpp  interact.cpp       mpastrm_in.cpp  stillsstream.cpp  mpegconsts.c      mjpeg_logging.c    decodebufmodel.cpp
	yuv4mpeg.c          cpu_accel.c)

include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
ADD_DEFINITIONS(${ADM_LIB} "-I${CMAKE_CURRENT_SOURCE_DIR} -DHAVE_FMAX -DRETSIGTYPE=void")

IF (NOT APPLE)
    ADD_DEFINITIONS(${ADM_LIB} "-DHAVE_MEMALIGN")
ENDIF (NOT APPLE)

IF (ADM_CPU_X86)
    ADD_DEFINITIONS(${ADM_LIB} "-DHAVE_X86CPU")
    
    IF (ADM_CPU_X86_64)
		ADD_DEFINITIONS(${ADM_LIB} "-DHAVE_X86_64CPU")
    ENDIF (ADM_CPU_X86_64)
ELSEIF (ADM_CPU_ALTIVEC)
    ADD_DEFINITIONS(${ADM_LIB} "-DHAVE_ALTIVEC")
ENDIF (ADM_CPU_X86)

INIT_MUXER(${ADM_LIB})
INSTALL_MUXER(${ADM_LIB})
