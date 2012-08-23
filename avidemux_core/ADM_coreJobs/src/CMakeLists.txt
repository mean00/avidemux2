SET(ADM_coreJobs_SRCS 
        ADM_coreJobs.cpp
        sqlJobs.cpp
)

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreJobs ${ADM_coreJobs_SRCS})

IF (APPLE)
	TARGET_LINK_LIBRARIES(ADM_coreJobs -Wl,-read_only_relocs,suppress)
ENDIF (APPLE)

INCLUDE_DIRECTORIES(../include ../../ADM_coreSqlLight3/include ${SQLITE_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ADM_coreJobs ADM_core6 ADM_coreSqlLight3)
ADM_INSTALL_LIB(ADM_coreJobs)
