#!/bin/bash

set -e

echo --------------------------------------------------
echo TEST PREHLS GROUP START: `pwd`
echo --------------------------------------------------

set -v

for i in */build.sh
do
 (
        cd "$(dirname "$i")" && sh build.sh && ./sim_sc && sh clean.sh
 )
done

echo --------------------------------------------------
echo TEST PREHLS GROUP FINISH: `pwd`
echo --------------------------------------------------
