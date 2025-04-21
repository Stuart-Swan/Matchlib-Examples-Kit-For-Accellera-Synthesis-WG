

cd Boost*master
rm -rf include
mkdir include
cp -rp boost include
cd ..

export CXX="g++ -std=c++11"
cd $SYSTEMC_HOME

touch configure.ac
sleep 2
touch aclocal.m4 configure Makefile.cin
find . -name 'Makefile.in' -exec touch {} \;
find . -name 'config.h.in' -exec touch {} \;

rm -rf objdir
mkdir objdir
cd objdir
../configure --enable-pthreads 
make
make install
