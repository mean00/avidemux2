#!/bin/bash
fail()
{
       echo "FAIL $@"
       exit 1
}
export CXXFLAGS="$CXXFLAGS -std=c++11"
bash bootStrap.bash --rebuild --deb --without-cli || fail main
bash appImage/deploy.sh
