subdirs (PREORDER ADM_ffmpeg)
subdirs (PREORDER ADM_libmpeg2enc)
#subdirs (PREORDER ADM_lvemux)
#subdirs (PREORDER ADM_mplex)
subdirs (PREORDER ADM_smjs)
subdirs (PREORDER ADM_xvidratectl)

if (ADM_DEBUG AND FIND_LEAKS)
	subdirs (PREORDER ADM_nvwa)
endif (ADM_DEBUG AND FIND_LEAKS)

if (APPLE AND USE_SDL)
	subdirs (PREORDER ADM_libsdl)
endif (APPLE AND USE_SDL)
