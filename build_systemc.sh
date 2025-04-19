

cd Boost*master
rm -rf include
mkdir include
cp -rp boost include
cd ..

export CXX="g++ -std=c++11"
cd $SYSTEMC_HOME
rm -rf objdir
mkdir objdir
cd objdir
../configure
make
make install
