#!/bin/bash
fail()
{
       echo "FAIL $@"
       exit 1
}
bash bootStrap.bash --rebuild --deb --without-cli || fail main
bash appImage/deploy.sh
