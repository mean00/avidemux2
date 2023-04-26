SET(ADM_coreSqlLight3_SRCS 
        Database.cpp
        Query.cpp
        StderrLog.cpp
        SysLog.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreSqlLight3 ${ADM_coreSqlLight3_SRCS})

IF (APPLE)
    IF (ADM_CPU_X86)
        TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 -Wl)
    ELSE (ADM_CPU_X86)
        TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 -Wl,-read_only_relocs,suppress)
    ENDIF (ADM_CPU_X86)
ELSE (APPLE)
    IF (UNIX)
        TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 dl)
    ENDIF (UNIX)
ENDIF (APPLE)

INCLUDE_DIRECTORIES(../include ${SQLITE3_INCLUDEDIR})
TARGET_LINK_LIBRARIES(ADM_coreSqlLight3 ADM_core6 ${SQLITE3_LDFLAGS})
ADM_INSTALL_LIB(ADM_coreSqlLight3)
