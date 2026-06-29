
#!/bin/bash

set -e

echo Testing Error Detection in sc_hls::msg_lib

for i in connections*
do
 (cd $i; sh clean.sh ; sh build.sh ; ./sim_sc ; sh clean.sh)
done
