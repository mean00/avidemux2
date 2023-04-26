# - Extract information from a subversion working copy
include(admTimeStamp)
###################################################################
#
# Look for git-svn-id in the logs to detect if it is git-svn
#
###################################################################
MACRO(admIsGitSvn _dir _isSvn)
    #MESSAGE(STATUS "Git on folder ${_dir}")
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} log -n 50 ${_dir}
            COMMAND grep "git-svn-id"
            COMMAND wc -l
            WORKING_DIRECTORY ${_dir}
                RESULT_VARIABLE result 
                OUTPUT_VARIABLE output
            )
    if(${output} EQUAL 0)
        MESSAGE(STATUS "This is not git-svn")
        SET(${_isSvn} 0)
    else(${output} EQUAL 0)
        MESSAGE(STATUS "This is git-svn")
        SET(${_isSvn} 1)
    endif(${output} EQUAL 0)
    #MESSAGE(STATUS "Dir : ${_dir} Output : ${output} result:${result} svn:${${_isSvn}}")    
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
        set(svn 0)
       MESSAGE(STATUS "Getting git-svn version from ${_dir}")
       EXECUTE_PROCESS(
               COMMAND ${GIT_EXECUTABLE} rev-parse  --show-toplevel ${_dir}
               WORKING_DIRECTORY ${_dir} 
               RESULT_VARIABLE result 
               OUTPUT_VARIABLE topdir
               )
       STRING(STRIP "${topdir}" topdir)
       MESSAGE(STATUS "Top dir is <${topdir}>")
       #MESSAGE(STATUS "COMMAND  ${GIT_EXECUTABLE} log --format=oneline -1   ${topdir}")
       EXECUTE_PROCESS(
               COMMAND echo  log --format=oneline --no-abbrev -1   ${topdir}
               COMMAND xargs ${GIT_EXECUTABLE} 
               COMMAND head -c 11
               WORKING_DIRECTORY ${_dir} 
               RESULT_VARIABLE result 
               OUTPUT_VARIABLE output
               )

       #MESSAGE(STATUS "git last entry is ${output} -- ${result}")
       ADM_TIMESTAMP(date)
       STRING(STRIP "${output}" output)
       SET( ${_rev} "${date}_${output}")
       MESSAGE(STATUS "<${output}><${date}>==> ${${_rev}}")
                
ELSE(GIT_EXECUTABLE)
                SET(ADM_GIT_SVN_REVISION 0)
        ENDIF(GIT_EXECUTABLE)

ENDMACRO(admGetGitRevision _dir _rev)

# FindSubversion.cmake ends here.
