#!/bin/bash
fail() {
  echo "FAIL $@"
  exit 1
}
export CXXFLAGS="$CXXFLAGS -std=c++11"
bash bootStrap.bash --without-cli || fail main
bash appImage/deploy_qt6.sh $PWD/install $PWD/appDir
bash appImage/pack.sh
