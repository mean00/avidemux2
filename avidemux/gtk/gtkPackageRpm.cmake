##############################                                                                                                                                                     
# RPM                                                                                                                                                                              
##############################                                                                                                                                                     
SET(CPACK_SET_DESTDIR "ON")
SET(CPACK_RPM_PACKAGE_NAME "avidemux3-gtk")
SET (CPACK_GENERATOR "RPM")
# ARCH                                                                                                                                                                             
IF (X86_64_SUPPORTED)
SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
ELSE (X86_64_SUPPORTED)
SET(CPACK_RPM_PACKAGE_ARCHITECTURE "i386")
ENDIF (X86_64_SUPPORTED)
# Mandatory
SET(CPACK_RPM_PACKAGE_VERSION "${AVIDEMUX_VERSION}")
SET(CPACK_RPM_PACKAGE_RELEASE "1.r${ADM_SUBVERSION}.bootstrap")
SET(CPACK_RPM_PACKAGE_LICENSE "GPLv2+")                                                                                                                                     
SET(CPACK_RPM_PACKAGE_VENDOR "mean")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "Simple video editor,main program gtk version ")
SET(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
# Some more infos  
SET(CPACK_RPM_PACKAGE_SUMMARY "GTK interface for avidemux")
SET(CPACK_RPM_PACKAGE_URL "http://www.avidemux.org")
SET(CPACK_RPM_PACKAGE_REQUIRES "avidemux3-core%{?_isa} >= ${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}")
SET(CPACK_RPM_PACKAGE_PROVIDES "avidemux3-gui = ${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}")

SET(CPACK_PACKAGE_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}-${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")

SET(CPACK_PACKAGE_RELOCATABLE "false")
include(CPack)

