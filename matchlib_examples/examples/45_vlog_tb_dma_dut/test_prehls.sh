#!/bin/bash

set -e

echo --------------------------------------------------
echo PREHLS TEST: `pwd`
echo --------------------------------------------------

set -v

if [[ $CXX == *"clang"* ]]; then
    exit 0
fi

exit 0  # vsim version on Rocky8 needs update..

if command -v vsim > /dev/null 2>&1; then
  make build
  make clean
fi

