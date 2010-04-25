
if (ADM_DEBUG AND FIND_LEAKS)
	subdirs (PREORDER ADM_nvwa)
endif (ADM_DEBUG AND FIND_LEAKS)

if (APPLE AND USE_SDL)
	subdirs (PREORDER ADM_libsdl)
endif (APPLE AND USE_SDL)
