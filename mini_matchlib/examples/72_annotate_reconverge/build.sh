
g++ -DCONN_BACK_ANNOTATE -I../../../rapidjson-master/include -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++11 -g -I../../include -I$SYSTEMC_HOME/{include,src} testbench.cpp -L$SYSTEMC_HOME/lib-linux64 -o sim_sc -lsystemc -lpthread
