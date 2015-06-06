#
# Set ADM_SUBVERSION either using subversion or git SVN to 
#  get the revision
#
MACRO(admPackager _prog)
        IF(NOT AVIDEMUX_PACKAGER)
                SET(AVIDEMUX_PACKAGER "none" CACHE STRING "")
        ELSE(NOT AVIDEMUX_PACKAGER)
                SET(AVIDEMUX_PACKAGER "${AVIDEMUX_PACKAGER}" CACHE STRING "")
                MESSAGE(STATUS "Packager=${AVIDEMUX_PACKAGER}, valid choices= {deb,rpm,tgz,none}")
        ENDIF(NOT AVIDEMUX_PACKAGER)
        
        if("${AVIDEMUX_PACKAGER}" STREQUAL "rpm")
                include("./${_prog}Rpm.cmake")
        elseif("${AVIDEMUX_PACKAGER}" STREQUAL "deb")
                SET(CPACK_DEB_COMPONENT_INSTALL ON)
                #
                include("./${_prog}Debian.cmake")
                #
        elseif("${AVIDEMUX_PACKAGER}" STREQUAL "tgz")
                include("./${_prog}TarGz.cmake")
        else("${AVIDEMUX_PACKAGER}" STREQUAL "rpm")
                MESSAGE(STATUS "No packaging... (package=${_package})")
        endif("${AVIDEMUX_PACKAGER}" STREQUAL "rpm")
ENDMACRO(admPackager _prog )
