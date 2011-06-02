#!/bin/sh
cp License.txt.in License.txt
find . -name "admPackage*.txt" | xargs cat >> License.txt
echo "Done"
