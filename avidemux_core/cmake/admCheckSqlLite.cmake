# Outputs:
#   SQLITE3_INCLUDEDIR
#   SQLITE3_LINK_LIBRARIES

MACRO(checkSqlite3)
  IF(NOT SQLITE3_CHECKED)

    MESSAGE(STATUS "Checking for Sqlite3")
    MESSAGE(STATUS "********************")
    IF(MSVC)
      FIND_PACKAGE(unofficial-sqlite3 CONFIG REQUIRED)
      ADD_LIBRARY(adm_sqlite3 INTERFACE)
      TARGET_LINK_LIBRARIES(adm_sqlite3 INTERFACE unofficial::sqlite3::sqlite3)
    ELSE()
      FIND_PACKAGE(SQLite3)
      IF(SQLite3_FOUND)
        PRINT_LIBRARY_INFO("Sqlite3" SQLite3_FOUND "${SQLite3_INCLUDE_DIRS}" "${SQLite3_LIBRARIES}")

        SET(SQLITE3_CHECKED 1)
        MESSAGE("")
        ADD_LIBRARY(adm_sqlite3 INTERFACE)
        TARGET_INCLUDE_DIRECTORIES(adm_sqlite3 INTERFACE ${SQLite3_INCLUDE_DIRS})
        TARGET_LINK_LIBRARIES(adm_sqlite3 INTERFACE ${SQLite3_LIBRARIES})
      ELSE()
        MESSAGE(STATUS "SQLite3 not found")
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO()
