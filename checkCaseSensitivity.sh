#!/bin/bash

isCaseSensitive()
{
    local test_dir=casetest-$RANDOM$RANDOM
    mkdir "$test_dir" || { echo "Read-only directory, cannot probe case-sensitivity." && exit 1; }
    touch "$test_dir/casetest"
    touch "$test_dir/caseTEST"
    local check_num=$(find ./"$test_dir" -type f -name 'case*' | wc -l)
    rm -rf "$test_dir"

    if [ "$check_num" -eq 2 ]; then
        return 0
    else
        return 1
    fi
}
