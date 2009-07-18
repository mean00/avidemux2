#
# Set ADM_SUBVERSION either using subversion or git SVN to 
#  get the revision
#
MACRO(admGetRevision)
if (EXISTS "${AVIDEMUX_TOP_SOURCE_DIR}/.svn")
        MESSAGE(STATUS "Seems to be SVN...")
        FIND_PACKAGE( Subversion)
        Subversion_WC_INFO( ${AVIDEMUX_TOP_SOURCE_DIR} ADM_SVN)
        SET(ADM_SUBVERSION "r${ADM_SVN_WC_REVISION}")
else (EXISTS "${AVIDEMUX_TOP_SOURCE_DIR}/.svn")
        if (EXISTS "${AVIDEMUX_TOP_SOURCE_DIR}/.git")
                MESSAGE(STATUS "Seems to be git svn...")
                include( FindGitSvn)
                SET(ADM_SUBVERSION ${ADM_GIT_SVN_REVISION})
        else (EXISTS "${AVIDEMUX_TOP_SOURCE_DIR}/.git")
                MESSAGE(STATUS "Dont know what SCM is used")
                SET(ADM_SUBVERSION "0")
        endif (EXISTS "${AVIDEMUX_TOP_SOURCE_DIR}/.git")
endif (EXISTS "${AVIDEMUX_TOP_SOURCE_DIR}/.svn")
                MESSAGE( STATUS "revision : ${ADM_SUBVERSION}" )
ENDMACRO(admGetRevision)
