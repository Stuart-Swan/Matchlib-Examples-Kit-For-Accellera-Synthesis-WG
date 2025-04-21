

cd Boost*master
rm -rf include
mkdir include
cp -rp boost include
cd ..

export CXX="g++ -std=c++11"
cd $SYSTEMC_HOME
find . -name 'configure.ac' -exec touch aclocal.m4 configure Makefile.in {} +
rm -rf objdir
mkdir objdir
cd objdir
../configure --enable-pthreads --disable-maintainer-mode
make
make install
