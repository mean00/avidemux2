#/bin/bash
export bash_path=$(cygpath -ua $(dirname $0))
export src_path=`cygpath -u $1`
export run_path="packages/org.avidemux.run/data/"
export dev_path="packages/org.avidemux.dev/data/" 

if [ ! -d "$src_path" ]; then
	echo "Source path does not exist"
	exit 1
fi

echo "Cleaning"
rm -Rf $run_path
rm -Rf $dev_path
mkdir -p $run_path $dev_path

echo "Copying"

cp -Rap $src_path/* $run_path/
cp -Rap $bash_path/packages/org.avidemux.run/data/scripts  $run_path/
echo "Moving"
mv $run_path/include $dev_path
mv $run_path/*.lib $dev_path
rm $run_path/plugins/*/*.lib
rm -f $run_path/vsscript.dll
rm -f $run_path/vapoursynth.dll
echo "Done"
