include(se_plugin)

add_subdirectory(qtScript)
IF(DO_COMMON)
        add_subdirectory(spiderMonkey)
        add_subdirectory(tinyPy)
ENDIF(DO_COMMON)
