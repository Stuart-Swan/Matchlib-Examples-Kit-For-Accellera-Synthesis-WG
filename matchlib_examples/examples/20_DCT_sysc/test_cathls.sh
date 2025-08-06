#!/bin/bash

set -e

echo --------------------------------------------------
echo CATHLS TEST: `pwd`
echo --------------------------------------------------

echo skipping..
exit 0

set -v

if command -v catapult > /dev/null 2>&1; then
  catapult -shell -f go_hls.tcl
fi

make clean
