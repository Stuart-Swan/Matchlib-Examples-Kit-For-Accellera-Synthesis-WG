
#!/bin/bash

set -e

echo Testing Error Detection in msg_lib.h

for i in connections*
do
 (cd $i; sh clean.sh ; sh build.sh ; ./sim_sc ; sh clean.sh)
done
