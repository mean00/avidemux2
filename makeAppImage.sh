#!/bin/bash
bash bootStrap.bash --rebuild
cp appImage/AppRun install
cp appImage/avidemux.png install
cp appImage/avidemux.desktop install
bash appImage/deploy.sh
