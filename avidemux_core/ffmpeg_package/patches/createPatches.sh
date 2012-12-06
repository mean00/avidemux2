#!/bin/bash

export curDir=$PWD
export ffmpegPath=$PWD/source
export origFfmpegPath=$PWD/ffmpeg

echo Updating patches in $ffmpegPath

function updatePatch {
	#cd $ffmpegPath
	#cp $1/$2 $1/$2.new
	#svn revert $1/$2
	#unix2dos $1/$2
	#unix2dos $1/$2.new
	#mv $1/$2 $1/$2.old
	#mv $1/$2.new $1/$2
	#diff -u $1/$2.old $1/$2 > $curDir/$1_$2.patch
	#rm $1/$2.old
	#cd $curDir
	#dos2unix $1_$2.patch

	cd $ffmpegPath
	unix2dos $1/$2
	unix2dos $origFfmpegPath/$1/$2
	diff -u $origFfmpegPath/$1/$2 $1/$2 > $curDir/${1//\//_}_$2.patch
	cd $curDir
	dos2unix ${1//\//_}_$2.patch
}

updatePatch libavcodec avcodec.h
updatePatch libavcodec golomb.h
updatePatch libavcodec h263dec.c
updatePatch libavcodec h264_parser.c
updatePatch libavcodec libavcodec.v
updatePatch libavcodec mathops.h
updatePatch libavcodec mpeg12enc.c
updatePatch libavcodec mpegvideo_enc.c
updatePatch libavcodec put_bits.h
updatePatch libavcodec vdpau.h
updatePatch libavcodec/x86 fmtconvert_init.c
updatePatch libavformat isom.c
updatePatch libavformat matroskaenc.c
updatePatch libavformat mpegtsenc.c
updatePatch libavutil avutil.h
updatePatch libavutil common.h
updatePatch libavutil lfg.c
updatePatch libavutil lfg.h