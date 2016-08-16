#
# Set ADM_SUBVERSION either using subversion or git SVN to 
#  get the revision
#
MACRO(admGetRevision _dir _rev)
if (EXISTS "${_dir}/.svn")
        MESSAGE(STATUS "Seems to be SVN...")
        FIND_PACKAGE( Subversion)
        Subversion_WC_INFO( ${_dir} ADM_SVN)
        SET(${_rev} ${ADM_SVN_WC_LAST_CHANGED_REV})
else (EXISTS "${_dir}/.svn")
        if (EXISTS "${_dir}/.git" OR EXISTS "${_dir}/../.git" OR EXISTS "${_dir}/../../.git")
                MESSAGE(STATUS "Seems to be git or git-svn...")
                include( FindGitSvn)
                admGetGitRevision( ${_dir} ADM_GIT_SVN_REVISION)
                SET(${_rev} ${ADM_GIT_SVN_REVISION})
        ELSE (EXISTS "${_dir}/.git" OR EXISTS "${_dir}/../.git" OR EXISTS "${_dir}/../../.git")
                MESSAGE(STATUS "Dont know what SCM is used")
                SET(${_rev} "0")
        ENDIF (EXISTS "${_dir}/.git" OR EXISTS "${_dir}/../.git" OR EXISTS "${_dir}/../../.git")
endif (EXISTS "${_dir}/.svn")
                #MESSAGE( STATUS "revision : ${${_rev}}" )
ENDMACRO(admGetRevision _dir _rev)
