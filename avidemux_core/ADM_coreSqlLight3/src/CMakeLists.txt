SET(ADM_coreSqlLight3_SRCS 
        Database.cpp
        Query.cpp
        StderrLog.cpp
        SysLog.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreSqlLight3 ${ADM_coreSqlLight3_SRCS})

IF (APPLE)
	TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 -Wl,-read_only_relocs,suppress)
ENDIF (APPLE)

IF (UNIX)
	TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 dl)
ENDIF (UNIX)

INCLUDE_DIRECTORIES(../include ${SQLITE_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 ADM_core6 ${SQLITE_LIBRARY_DIR})
ADM_INSTALL_LIB(ADM_coreSqlLight3)
