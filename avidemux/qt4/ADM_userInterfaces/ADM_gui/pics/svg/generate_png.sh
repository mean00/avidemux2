#!/bin/bash

convertsvg()
{
    flatpak run org.inkscape.Inkscape --export-height=$1 --export-type=png --export-background-opacity=0 --export-filename=$2 $3
}

for i in *.svg; do
    [ -f "$i" ] || break
    convertsvg 24 "${i%.*}.png" $i
    convertsvg 48 "${i%.*}@2x.png" $i
done

convertsvg 16 volume.png volume.svg
convertsvg 32 volume@2x.png volume.svg
convertsvg 16 volume_off.png volume_off.svg
convertsvg 32 volume_off@2x.png volume_off.svg

