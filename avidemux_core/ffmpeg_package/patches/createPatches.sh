#!/bin/bash

export curDir=$PWD
export ffmpegPath=$1/core/ffmpeg/source

echo Updating patches in $ffmpegPath

function updatePatch {
	cd $ffmpegPath
	cp $1/$2 $1/$2.new
	svn revert $1/$2
	unix2dos $1/$2
	unix2dos $1/$2.new
	mv $1/$2 $1/$2.old
	mv $1/$2.new $1/$2
	diff -u $1/$2.old $1/$2 > $curDir/$1_$2.patch
	rm $1/$2.old
	cd $curDir
	dos2unix $1_$2.patch
}

updatePatch libavcodec avcodec.h
updatePatch libavcodec h263dec.c
updatePatch libavcodec h264_parser.c
updatePatch libavcodec mpeg12.c
updatePatch libavcodec mpeg12enc.c
updatePatch libavcodec mpegvideo.c
updatePatch libavcodec mpegvideo_enc.c
updatePatch libavcodec utils.c
updatePatch libavformat file.c
updatePatch libavformat isom.c
updatePatch libavformat matroskaenc.c
updatePatch libavutil avutil.h
updatePatch libavutil lfg.c
updatePatch libavutil lfg.h
updatePatch libswscale swscale_template.c