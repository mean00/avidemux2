# - Extract information from a subversion working copy
FIND_PROGRAM(GIT_EXECUTABLE git
  DOC "git command line client")
MARK_AS_ADVANCED(GIT_EXECUTABLE)

IF(GIT_EXECUTABLE)
        
        #SET(EXE  "cd ${AVIDEMUX_TOP_SOURCE_DIR}&& ${GIT_EXECUTABLE} svn log | head -2  | grep '^r' | sed 's/ .*$//g'" )
        MESSAGE(STATUS "Getting git-svn version from ${AVIDEMUX_TOP_SOURCE_DIR}")

      EXECUTE_PROCESS(COMMAND /usr/bin/git svn log --oneline --limit=1   
        WORKING_DIRECTORY ${AVIDEMUX_TOP_SOURCE_DIR} 
        RESULT_VARIABLE result 
        OUTPUT_VARIABLE output
        )
        STRING(REGEX REPLACE " .*$" "" ADM_GIT_SVN_REVISION "${output}")
        STRING(REGEX REPLACE "^r" "" ADM_GIT_SVN_REVISION "${ADM_GIT_SVN_REVISION}")
        MESSAGE(STATUS "Git Svn Revision : ${ADM_GIT_SVN_REVISION}")
ELSE(GIT_EXECUTABLE)
        SET(ADM_GIT_SVN_REVISION 0)
ENDIF(GIT_EXECUTABLE)

# FindSubversion.cmake ends here.
