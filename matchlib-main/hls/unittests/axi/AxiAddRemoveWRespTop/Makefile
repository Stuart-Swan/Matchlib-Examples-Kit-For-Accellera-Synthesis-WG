# Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
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

ROOT            := ../../../..
SRC_PATH				:= $(ROOT)/cmod/unittests/axi

include $(ROOT)/hls/hls_Makefile

# Without write responses there is no way to prevent a bad seed
# that will stall writes in the TB so that reads overtake them.
COMPILER_FLAGS += NVHLS_RAND_SEED=1000
