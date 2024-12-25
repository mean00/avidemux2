#
# Set ADM_SUBVERSION either using subversion or git SVN to
#  get the revision
#
MACRO(admGetRevision _dir _rev)
  IF(EXISTS "${_dir}/.svn")
    MESSAGE(STATUS "Seems to be SVN...")
    FIND_PACKAGE(Subversion)
    Subversion_WC_INFO(${_dir} ADM_SVN)
    SET(${_rev} ${ADM_SVN_WC_LAST_CHANGED_REV})
  ELSE()
    IF(EXISTS "${_dir}/.git" OR EXISTS "${_dir}/../.git" OR EXISTS "${_dir}/../../.git")
      MESSAGE(STATUS "Seems to be git or git-svn...")
      INCLUDE(FindGitSvn)
      admGetGitRevision(${_dir} ADM_GIT_SVN_REVISION)
      SET(${_rev} ${ADM_GIT_SVN_REVISION})
    ELSE()
      MESSAGE(STATUS "Dont know what SCM is used")
      SET(${_rev} "0")
    ENDIF()
  ENDIF()
  #MESSAGE( STATUS "revision : ${${_rev}}" )
ENDMACRO()
