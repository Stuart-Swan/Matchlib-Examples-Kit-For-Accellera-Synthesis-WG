
(cd $SYSTEMC_HOME ; rm -rf objdir ; rm -rf lib-linux64)

cd matchlib_examples/examples
for i in *
do
 if [ -f $i/Makefile ]; then
  (cd $i; make clean)
 fi
done
