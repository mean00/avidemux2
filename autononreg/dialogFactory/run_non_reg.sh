#!/bin/bash
export PRG=~/workspace/qt4/avidemux3_qt4
export SRC=.
echo "Running tests..."
#*****************
cd functiontest
perl ../foreach.pl $SRC/*.js "echo %f &&  $PRG    --run %f --quit > /dev/null"
cd ..
#*****************
echo "## ALL done checking...##"
		
