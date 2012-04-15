set(ADM_nvwa_SRCS debug_new.cpp)

if (MINGW)
	set(ADM_nvwa_SRCS ${ADM_nvwa_SRCS} addr2line.cpp)
endif (MINGW)

add_library(ADM_nvwa STATIC ${ADM_nvwa_SRCS})
add_definitions(-D_DEBUG_NEW_ERROR_ACTION=NULL)

if (UNIX)
	add_definitions(-fPIC)
endif (UNIX)

if (MINGW)
	target_link_libraries(ADM_nvwa  bfd  iberty)
endif (MINGW)
