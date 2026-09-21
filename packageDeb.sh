#!/bin/sh
set -x
rm -f A*.debs.tar.gz
cd pkgs && ls && pwd &&
  tar -czlf "../Avidemux-2604_x86_64_$(date +%Y%m%d).debs.tar.gz" *runtime*.deb *plugins*.deb *settings*.deb
