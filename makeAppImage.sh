#!/bin/bash
fail()
{
       echo "FAIL $@"
       exit 1
}
bash bootStrap.bash --rebuild --deb --without-cli || fail main
cp appImage/AppRun install
cp appImage/avidemux.png install
cp appImage/avidemux.desktop install
bash appImage/deploy.sh
