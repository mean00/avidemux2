#!/bin/bash
bash bootStrap.bash --rebuild
cp appImage/AppRun install
cp appImage/avidemux.png install
bash deploy.sh
