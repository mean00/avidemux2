SET(ADM_coreJobs_SRCS 
        ADM_coreJobs.cpp
        sqlite3.c
        libjobdb.cpp
        Database.cpp
        Query.cpp
        StderrLog.cpp
        SysLog.cpp
)

ADD_LIBRARY(ADM_coreJobs SHARED ${ADM_coreJobs_SRCS})
IF (APPLE)
	TARGET_LINK_LIBRARIES(ADM_coreJobs -Wl,-read_only_relocs,suppress)
ENDIF (APPLE)
INCLUDE_DIRECTORIES(../include)
TARGET_LINK_LIBRARIES(ADM_coreJobs ADM_core6 )
ADM_INSTALL_LIB(ADM_coreJobs)
