CXXFLAGS += -g -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-label

CATAPULT_HOME ?= $(MGC_HOME)

ifneq "$(CATAPULT_HOME)" ""
ifneq "$(SYSTEMC_HOME)" ""
$(warning - Warning: SYSTEMC_HOME and MGC_HOME/CATAPULT_HOME are both set. Using SystemC from MGC_HOME/CATAPULT_HOME)
endif
SYSTEMC_HOME := $(CATAPULT_HOME)/shared
CONNECTIONS_HOME ?= $(CATAPULT_HOME)/shared
MATCHLIB_HOME ?= $(CATAPULT_HOME)/shared/pkgs/matchlib
BOOST_HOME ?= $(CATAPULT_HOME)/shared/pkgs/boostpp/pp
RAPIDJSON_HOME ?= $(CATAPULT_HOME)/shared
AC_TYPES ?= $(CATAPULT_HOME)/shared
AC_SIMUTILS ?= $(CATAPULT_HOME)/shared
CXX := $(CATAPULT_HOME)/bin/g++
LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):$(CATAPULT_HOME)/lib:$(CATAPULT_HOME)/shared/lib,$(CATAPULT_HOME)/lib:$(CATAPULT_HOME)/shared/lib)
export LD_LIBRARY_PATH
LIBDIRS += -L$(CATAPULT_HOME)/lib -L$(CATAPULT_HOME)/shared/lib
else
ifndef SYSTEMC_HOME
$(error - Environment variable SYSTEMC_HOME must be defined)
endif
ifndef CONNECTIONS_HOME
$(error - Environment variable CONNECTIONS_HOME must be defined)
endif
ifndef MATCHLIB_HOME
$(error - Environment variable MATCHLIB_HOME must be defined)
endif
ifndef BOOST_HOME
$(error - Environment variable BOOST_HOME must be defined)
endif
ifndef RAPIDJSON_HOME
$(error - Environment variable RAPIDJSON_HOME must be defined)
endif
ifndef AC_TYPES
$(error - Environment variable AC_TYPES must be defined)
endif
ifndef AC_SIMUTILS
$(error - Environment variable AC_SIMUTILS must be defined)
endif
CXX ?= g++
LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64,$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64)
export LD_LIBRARY_PATH
LIBDIRS += -L$(SYSTEMC_HOME)/lib -L$(SYSTEMC_HOME)/lib-linux64
endif

export CATAPULT_HOME SYSTEMC_HOME CONNECTIONS_HOME MATCHLIB_HOME BOOST_HOME RAPIDJSON_HOME AC_TYPES AC_SIMUTILS

PWD := $(shell pwd)
ifndef TEST_SOURCE_DIR
THIS_TEST_MAKEFILE := $(lastword $(MAKEFILE_LIST))
TEST_SOURCE_DIR := $(dir $(abspath $(THIS_TEST_MAKEFILE)))
endif
SOURCE_DIR := $(TEST_SOURCE_DIR)

INCDIRS := -I$(SOURCE_DIR) -I$(SOURCE_DIR)/../../include -I$(SOURCE_DIR)/../common
INCDIRS += -I$(SYSTEMC_HOME)/include -I$(SYSTEMC_HOME)/src
INCDIRS += -I$(CONNECTIONS_HOME)/include
INCDIRS += -I$(MATCHLIB_HOME)/cmod/include
INCDIRS += -I$(BOOST_HOME)/include
INCDIRS += -I$(RAPIDJSON_HOME)/include
INCDIRS += -I$(AC_TYPES)/include
INCDIRS += -I$(AC_SIMUTILS)/include

CPPFLAGS += $(INCDIRS)
LIBS += -lsystemc -lpthread

.PHONY: all build run clean sim_clean

all: build
build: sim_sc

run: build
	./sim_sc

sim_sc: $(wildcard $(SOURCE_DIR)/*.cpp) $(wildcard $(SOURCE_DIR)/*.h) $(wildcard $(SOURCE_DIR)/*.hpp)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIBDIRS) $(filter %.cpp,$^) -o $@ $(LIBS)

sim_clean:
	@rm -rf sim_*

clean: sim_clean
	@rm -f sim_sc *.vcd *.wlf test.log
