#
# Copyright (c) 2017-2024, NVIDIA CORPORATION.  All rights reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include ../../../cmod_Makefile

all: sim_reg sim_csreg

sim_reg: $(wildcard *.h) $(wildcard *.cpp) $(wildcard $(CWD)/../../../include/axi/*.h)
	$(CC) -o $@ $(CFLAGS) $(USER_FLAGS) -I../../../include $(wildcard *.cpp) $(BOOSTLIBS) $(LIBS)

sim_csreg: $(wildcard *.h) $(wildcard *.cpp) $(wildcard $(CWD)/../../../include/axi/*.h)
	$(CC) -o $@ $(CFLAGS) $(USER_FLAGS) -DSTATUS_REG -I../../../include $(wildcard *.cpp) $(BOOSTLIBS) $(LIBS)

run:
	./sim_reg
	./sim_csreg

sim_clean:
	rm -rf *.o sim_*
