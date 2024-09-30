
# this scripts builds SystemC simulator with clang++ rather than g++

cd Boost*master
rm -rf include
mkdir include
cp -rp boost include
cd ..

export CXX=`pwd`/clang_fix.sh
cd $SYSTEMC_HOME
rm -rf objdir
mkdir objdir
cd objdir
../configure
make
make install
