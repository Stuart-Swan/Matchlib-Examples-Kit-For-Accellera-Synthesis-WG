#!/bin/bash

set -e

echo --------------------------------------------------
echo TEST CLANG PREHLS GROUP START: `pwd`
echo --------------------------------------------------

export CXX="clang++ -std=c++11"

set -v

for i in */test_prehls.sh
do
 (
        cd "$(dirname "$i")" && ./$(basename "$i")
 )
done

echo --------------------------------------------------
echo TEST CLANG PREHLS GROUP FINISH: `pwd`
echo --------------------------------------------------
