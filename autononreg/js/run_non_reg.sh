#!/bin/bash
export PRG=adm2_s
export SRC=.
echo "Running tests..."
rm -f allog
rm -f out/* /tmp/md5
#*****************
cd functiontest
perl ../foreach.pl $SRC/*.js "echo %f && adm2_s    --run %f --quit > /dev/null"
cd ..
#*****************
echo "## ALL done checking...##"
md5sum out/*  > /tmp/md5
		
