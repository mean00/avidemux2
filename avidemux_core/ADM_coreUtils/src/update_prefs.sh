#!/bin/sh
echo "Updating prefs"
python pref_gen.py prefs2.conf
python ../../../cmake/admSerialization.py prefs2.conf

