INCLUDE(vf_plugin)
ADD_CORE_INCLUDE(ADM_coreLibVA)

FIND_PATH(VPP va/va_vpp.h PATHS /usr/include)
IF(VPP)
        MESSAGE(STATUS "Checking libva is recent enough..")
        TRY_COMPILE(hasHeader
		  ${CMAKE_BINARY_DIR}
		  ${CMAKE_CURRENT_SOURCE_DIR}/testHeader.c 
		  CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${_include}" "-DLINK_LIBRARIES:STRING=va"
		  COMPILE_DEFINITIONS ""
		  OUTPUT_VARIABLE foo)

        IF(hasHeader)
                MESSAGE(STATUS "yes")
                SET(ADM_vf_vaapiFilter_SRCS ADM_vidVaapiFilter.cpp)
                SET(ADM_vf_vaapiFilterDeint_SRCS ADM_vidVaapiFilterDeint.cpp)
                ADD_VIDEO_FILTER(ADM_vf_vaapiFilter ${ADM_vf_vaapiFilter_SRCS})
                ADD_VIDEO_FILTER(ADM_vf_vaapiFilterDeint ${ADM_vf_vaapiFilterDeint_SRCS})
                TARGET_LINK_LIBRARIES( ADM_vf_vaapiFilter ADM_coreLibVA6)
                TARGET_LINK_LIBRARIES( ADM_vf_vaapiFilter va)
                TARGET_LINK_LIBRARIES( ADM_vf_vaapiFilterDeint ADM_coreLibVA6)
                TARGET_LINK_LIBRARIES( ADM_vf_vaapiFilterDeint va)
                INIT_VIDEO_FILTER(ADM_vf_vaapiFilter)
                INIT_VIDEO_FILTER(ADM_vf_vaapiFilterDeint)
                INSTALL_VIDEO_FILTER(ADM_vf_vaapiFilter)
                INSTALL_VIDEO_FILTER(ADM_vf_vaapiFilterDeint)
        ELSE(hasHeader)
                MESSAGE(STATUS "no ")
        ENDIF(hasHeader)
ENDIF(VPP)
