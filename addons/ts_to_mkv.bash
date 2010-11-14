#!/bin/bash

INDIR=/capture/links
OUTDIR=/capture/linksMkv
SCRIPT=/tmp/script
mkdir -p $OUTDIR
for i in $INDIR/*.mpg; do
echo "Processing $i"
infile="$i"
name=` echo $i | sed 's/^.*\///g'`
outfile=` echo $name | sed 's/mpg$/mkv/g'`
echo "$infile -> $outfile"
if [ -f "$OUTDIR/$outfile" ] ; then 
   echo "   $outfile already present, skipping"
else
   echo "Creating script file..."
        rm -f $SCRIPT
        echo "adm=Avidemux()" > $SCRIPT
        echo "adm.loadVideo(\"$infile\")" >> $SCRIPT
        echo "adm.setContainer(\"MKV\")" >> $SCRIPT
        echo "adm.save(\"$OUTDIR"/"$outfile\")" >> $SCRIPT
        cat $SCRIPT
        echo "Running script..."
        avidemux3_cli --runpy $SCRIPT --quit
# >& /tmp/log_ts2mkv
fi
done
