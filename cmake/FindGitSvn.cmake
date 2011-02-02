# - Extract information from a subversion working copy
MACRO(admIsGitSvn _dir _svn)
	EXECUTE_PROCESS(COMMAND grep svn ${_dir}/.git/config  
			COMMAND wc -l
        	RESULT_VARIABLE result 
        	OUTPUT_VARIABLE output
        	)
	#MESSAGE(STATUS "r:${result} o:${output}")
	if(${output} EQUAL 0)
	else(${output} EQUAL 0)
		SET(${_svn} 1)
	endif(${output} EQUAL 0)
	
ENDMACRO(admIsGitSvn _dir _svn)
#
#
#
MACRO(admGetGitRevision _dir _rev)

FIND_PROGRAM(GIT_EXECUTABLE git
  DOC "git command line client")
MARK_AS_ADVANCED(GIT_EXECUTABLE)

IF(GIT_EXECUTABLE)
        
        #SET(EXE  "cd ${_dir}&& ${GIT_EXECUTABLE} svn log | head -2  | grep '^r' | sed 's/ .*$//g'" )
	admIsGitSvn(${_dir} svn)
	if(${svn})
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
	else(${svn})
        	MESSAGE(STATUS "Getting git version from ${_dir}")
	endif(${svn})
	
ELSE(GIT_EXECUTABLE)
        SET(ADM_GIT_SVN_REVISION 0)
ENDIF(GIT_EXECUTABLE)

ENDMACRO(admGetGitRevision _dir _rev)

# FindSubversion.cmake ends here.
