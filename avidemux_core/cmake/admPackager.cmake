#
# Set ADM_SUBVERSION either using subversion or git SVN to
#  get the revision
#
MACRO(admPackager _prog)
  IF(NOT AVIDEMUX_PACKAGER)
    SET(AVIDEMUX_PACKAGER "none" CACHE STRING "")
  ELSE()
    SET(AVIDEMUX_PACKAGER "${AVIDEMUX_PACKAGER}" CACHE STRING "")
    MESSAGE(STATUS "Packager=${AVIDEMUX_PACKAGER}, valid choices= {deb,rpm,tgz,none}")
  ENDIF()

  IF("${AVIDEMUX_PACKAGER}" STREQUAL "rpm")
    INCLUDE("./${_prog}Rpm.cmake")
  ELSEIF("${AVIDEMUX_PACKAGER}" STREQUAL "deb")
    SET(CPACK_DEB_COMPONENT_INSTALL ON)
    #
    INCLUDE("./${_prog}Debian.cmake")
    #
  ELSEIF("${AVIDEMUX_PACKAGER}" STREQUAL "tgz")
    INCLUDE("./${_prog}TarGz.cmake")
  ELSE()
    MESSAGE(STATUS "No packaging... (package=${_package})")
  ENDIF()
ENDMACRO()
