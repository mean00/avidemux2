#!/bin/bash

isCaseSensitive()
{
        test_dir=casetest-$RANDOM$RANDOM
        mkdir "$test_dir"
        touch "$test_dir/casetest"
        touch "$test_dir/caseTEST"
        check_num=$(find ./"$test_dir" -type f -name 'case*' | wc -l)

         if [ "$check_num" -eq 2 ]; then
            rm -rf "$test_dir"
            return 0
         else
            rm -rf "$test_dir"
            return 1
        fi
}