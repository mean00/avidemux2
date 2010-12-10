#
doPatch()
{
	src=$1
	dts=$1
	for i in patches/$src/*.patch ; do
		echo $i
       	 	patch -p3   <   $i
	done
	
}
doPatch avcodec
doPatch avformat
doPatch avutils


