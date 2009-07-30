# - Extract information from a subversion working copy
MACRO(admGetGitRevision _dir _rev)

FIND_PROGRAM(GIT_EXECUTABLE git
  DOC "git command line client")
MARK_AS_ADVANCED(GIT_EXECUTABLE)

IF(GIT_EXECUTABLE)
        
        #SET(EXE  "cd ${_dir}&& ${GIT_EXECUTABLE} svn log | head -2  | grep '^r' | sed 's/ .*$//g'" )
        MESSAGE(STATUS "Getting git-svn version from ${_dir}")

      EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} log -n 50 ${_dir}
        COMMAND grep git-svn-id
	COMMAND head -1
        WORKING_DIRECTORY ${_dir} 
        RESULT_VARIABLE result 
        OUTPUT_VARIABLE output
        )
        #MESSAGE(STATUS "Git Svn Revision : <${output}>")
        STRING(REGEX REPLACE ".*branch_.*@" "" rev "${output}")
        STRING(REGEX REPLACE " .*$" "" rev "${rev}")
        MESSAGE(STATUS "Git Svn Revision : <${rev}>")
        SET( ${_rev} ${rev})
ELSE(GIT_EXECUTABLE)
        SET(ADM_GIT_SVN_REVISION 0)
ENDIF(GIT_EXECUTABLE)

ENDMACRO(admGetGitRevision _dir _rev)

# FindSubversion.cmake ends here.
