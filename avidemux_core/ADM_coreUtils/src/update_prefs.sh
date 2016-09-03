#!/bin/sh
echo "Updating prefs"
python ../../../cmake/admSerialization.py prefs2.conf
python pref_gen.py prefs2.conf

