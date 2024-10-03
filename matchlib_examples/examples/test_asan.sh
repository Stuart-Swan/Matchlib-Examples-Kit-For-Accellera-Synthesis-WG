#!/bin/bash

set -e

echo
echo Note: SystemC must be built with pthreads to use AddressSanitizer.
echo See ../../pthreads_build_systemc.sh
echo

echo --------------------------------------------------
echo TEST ASAN PREHLS GROUP START: `pwd`
echo --------------------------------------------------

export CXX="g++ -std=c++11 -O0 -fsanitize=address -fno-omit-frame-pointer"

set -v

for i in */test_prehls.sh
do
 (
        cd "$(dirname "$i")" && ./$(basename "$i")
 )
done

echo --------------------------------------------------
echo TEST ASAN PREHLS GROUP FINISH: `pwd`
echo --------------------------------------------------
