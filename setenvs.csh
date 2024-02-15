

unsetenv MGC_HOME
unsetenv CATAPULT_HOME
setenv CXX "g++ -std=c++11"
setenv SYSTEMC_HOME `pwd`/systemc-2.3.3
setenv SYSTEMC_HOME_SRC $SYSTEMC_HOME/src
setenv CONNECTIONS_HOME  `pwd`/matchlib_connections-master
setenv AC_SIMUTILS `pwd`/ac_simutils-master
setenv AC_TYPES `pwd`/ac_types-master
setenv AC_MATH `pwd`/ac_math-master
setenv MATCHLIB_HOME `pwd`/matchlib-main
setenv RAPIDJSON_HOME `pwd`/rapidjson-master
setenv BOOST_HOME `pwd`/Boost-Preprocessor-master
setenv LD_LIBRARY_PATH $SYSTEMC_HOME/lib-linux64:$LD_LIBRARY_PATH
