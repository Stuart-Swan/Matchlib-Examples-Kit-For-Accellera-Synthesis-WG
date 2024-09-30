#!/bin/bash

set -e

echo --------------------------------------------------
echo PREHLS TEST: `pwd`
echo --------------------------------------------------

set -v

make build
./sim_sc
make clean

make marshall_port_sc
./marshall_port_sc
make clean

make direct_port_sc
./direct_port_sc
make clean

