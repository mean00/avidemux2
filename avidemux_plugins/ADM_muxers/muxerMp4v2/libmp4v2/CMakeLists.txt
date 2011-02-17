SET(ADM_LIB ADM_libmp4v2)

SET(${ADM_LIB}_SRCS 
	./src/log.cpp
	./src/3gp.cpp
	./src/atom_ac3.cpp
	./src/atom_amr.cpp
	./src/atom_avc1.cpp
	./src/atom_avcC.cpp
	./src/atom_chpl.cpp
	./src/atom_colr.cpp
	./src/atom_d263.cpp
	./src/atom_dac3.cpp
	./src/atom_damr.cpp
	./src/atom_dref.cpp
	./src/atom_elst.cpp
	./src/atom_enca.cpp
	./src/atom_encv.cpp
	./src/atom_free.cpp
	./src/atom_ftab.cpp
	./src/atom_ftyp.cpp
	./src/atom_gmin.cpp
	./src/atom_hdlr.cpp
	./src/atom_hinf.cpp
	./src/atom_hnti.cpp
	./src/atom_href.cpp
	./src/atom_mdat.cpp
	./src/atom_mdhd.cpp
	./src/atom_meta.cpp
	./src/atom_mp4s.cpp
	./src/atom_mp4v.cpp
	./src/atom_mvhd.cpp
	./src/atom_nmhd.cpp
	./src/atom_ohdr.cpp
	./src/atom_pasp.cpp
	./src/atom_root.cpp
	./src/atom_rtp.cpp
	./src/atom_s263.cpp
	./src/atom_sdp.cpp
	./src/atom_sdtp.cpp
	./src/atom_smi.cpp
	./src/atom_sound.cpp
	./src/atom_standard.cpp
	./src/atom_stbl.cpp
	./src/atom_stdp.cpp
	./src/atom_stsc.cpp
	./src/atom_stsd.cpp
	./src/atom_stsz.cpp
	./src/atom_stz2.cpp
	./src/atom_text.cpp
	./src/atom_tfhd.cpp
	./src/atom_tkhd.cpp
	./src/atom_treftype.cpp
	./src/atom_trun.cpp
	./src/atom_tx3g.cpp
	./src/atom_udta.cpp
	./src/atom_url.cpp
	./src/atom_urn.cpp
	./src/atom_uuid.cpp
	./src/atom_video.cpp
	./src/atom_vmhd.cpp
	./src/bmff/typebmff.cpp
	./src/cmeta.cpp
	./src/descriptors.cpp
	./src/exception.cpp
	./src/isma.cpp
	./src/itmf/CoverArtBox.cpp
	./src/itmf/generic.cpp
	./src/itmf/Tags.cpp
	./src/itmf/type.cpp
	./src/mp4.cpp
	./src/mp4atom.cpp
	./src/mp4container.cpp
	./src/mp4descriptor.cpp
	./src/mp4file.cpp
	./src/mp4file_io.cpp
	./src/mp4info.cpp
	#./src/mp4meta.cpp
	./src/mp4property.cpp
	./src/mp4track.cpp
	./src/mp4util.cpp
	./src/ocidescriptors.cpp
	./src/odcommands.cpp
	./src/qosqualifiers.cpp
	./src/qtff/coding.cpp
	./src/qtff/ColorParameterBox.cpp
	./src/qtff/PictureAspectRatioBox.cpp
	./src/rtphint.cpp
	./src/text.cpp	)
# Platform files
if(WIN32)
	SET(PLATFORM
		libplatform/io/File_win32.cpp
		libplatform/io/FileSystem_win32.cpp
		libplatform/number/random_win32.cpp
		libplatform/process/process_win32.cpp
		libplatform/time/time_win32.cpp
		libplatform/platform_win32.cpp
 	)
else(WIN32)
		SET(PLATFORM
			libplatform/io/File_posix.cpp
			libplatform/io/FileSystem_posix.cpp
			libplatform/number/random_posix.cpp
			libplatform/process/process_posix.cpp
			libplatform/time/time_posix.cpp
	
 		)
endif(WIN32)

SET(PLATFORM ${PLATFORM}
	libplatform/io/File.cpp
	libplatform/io/FileSystem.cpp
	libplatform/prog/option.cpp
	libplatform/sys/error.cpp
	libplatform/time/time.cpp
)


ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS} ${PLATFORM})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)


IF (UNIX)
	ADD_TARGET_CFLAGS(${ADM_LIB} -fPIC)
ENDIF (UNIX)


