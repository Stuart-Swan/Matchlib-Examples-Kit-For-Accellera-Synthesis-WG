
g++ -std=c++11 -g -I../../include -I$SYSTEMC_HOME/include testbench.cpp -L$SYSTEMC_HOME/lib-linux64 -o sim_sc -lsystemc -lpthread
