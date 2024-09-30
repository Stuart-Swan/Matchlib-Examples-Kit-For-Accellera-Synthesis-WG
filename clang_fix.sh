#!/bin/bash

HAS_C=0

for arg in "$@"; do
    if [[ "$arg" == *.c ]]; then
        HAS_C=1
        break 
    fi
done

if [ $HAS_C -eq 1 ]; then
    clang++ "$@"
else
    clang++ -std=c++11 "$@"
fi

