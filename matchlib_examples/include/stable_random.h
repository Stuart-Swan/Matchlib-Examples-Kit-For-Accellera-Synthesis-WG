
#pragma once

#include <mc_connections.h>
#include <connections/connections_utils.h>

#include <random>
#include <functional>
#include <cstdlib>

// This class provides stable random number generation in a SystemC simulation with multiple threads.
// Each thread has separate generators, and generators are initialized with a seed
// based on the name of thread (which is stable between different runs/compiles/ etc).

class stable_random {
public:
  std::hash<std::string> hasher;
  std::mt19937 generator;
  std::uniform_int_distribution<unsigned> distribution;
  sc_process_handle handle{sc_get_current_process_handle()};

  stable_random()
  {
    unsigned int seed = 0;
#ifdef NVHLS_RAND_SEED
    seed = (NVHLS_RAND_SEED);
#endif
    const char* env_rand_seed = std::getenv("NVHLS_RAND_SEED");
    if (env_rand_seed != NULL) seed = atoi(env_rand_seed);


    generator.seed(seed + hasher(clean_name(handle.name())));
  }

  unsigned get() {
    CONNECTIONS_ASSERT_MSG(sc_get_current_process_handle() == handle, "stable_random called from two processes");
    return distribution(generator);
  }

  std::string clean_name(std::string str) {
    // Questa messes up SC pathnames, we clean them up if needed here..
    std::string substr("sc_main/");
    size_t pos = str.find(substr);

    if (pos != std::string::npos) {
        str.erase(pos, substr.length());
    }

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '/') {
            str[i] = '.'; 
        }
    }

    // std::cout << "PROC: " << str << "\n";

    return str;
  }
};
