MACRO(XX4 supp arch)
        IF(${supp})
                SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${arch})
        ENDIF()
ENDMACRO()

MACRO(SET_DEBIAN_ARCH)
        XX4(X86_64_SUPPORTED "amd64")
        XX4(X86_32_SUPPORTED "i386")
        # Cause problem on raspberry, armel vs armhf XX4(ARMEL_SUPPORTED  "armel")
ENDMACRO()

