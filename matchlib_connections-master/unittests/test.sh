
for i in connections*
do
 (cd $i; make clean; make build; ./sim_sc)
done
