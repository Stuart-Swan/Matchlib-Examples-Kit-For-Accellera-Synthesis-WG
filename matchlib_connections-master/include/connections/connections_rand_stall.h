
#pragma once

// new_connections.h random stall implementation
// Stuart Swan 
// Siemens EDA
// 17 Oct 2025

#include <cstdlib>

#include <connections/new_connections.h>

#include <random>
#include <functional>
#include <cstdlib>

namespace Connections {

// This class provides stable random number generation in a SystemC simulation with multiple threads.
// Each thread has separate generators, and generators are initialized with a seed
// based on the name of thread (which is stable between different runs/compiles/ etc).

class stable_random {
public:
  std::hash<std::string> hasher;
  std::mt19937 generator;
  std::uniform_int_distribution<unsigned> distribution;
  sc_process_handle handle;
  bool handle_set{0};
  std::string pathname;

  stable_random() {}

  void do_seed()
  {
    unsigned int seed = 0;
#ifdef NVHLS_RAND_SEED
    seed = (NVHLS_RAND_SEED);
#endif
    const char* env_rand_seed = std::getenv("NVHLS_RAND_SEED");
    if (env_rand_seed != NULL) 
      seed = atoi(env_rand_seed);

    // when generator is coupled with a structural object (e.g. channel), we use pathname
    // when generator is coupled with a process, we use process handle name

    if (pathname != "")
      generator.seed(seed + hasher(clean_name(pathname)));
    else
      generator.seed(seed + hasher(clean_name(handle.name())));
  }

  unsigned get() {
    if (!handle_set) {
      handle = sc_get_current_process_handle();
      handle_set = 1;
      do_seed();
    } else if (sc_get_current_process_handle() != handle) {
        SC_REPORT_ERROR("CONNECTIONS-402", "stable_random::get() called from 2 different processes");
    }

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


class pacer_v2 : public rand_force_if
{
protected:
  static const unsigned precision = 1000;

  int stall_prob;
  int hold_stall_prob;
  bool stalled;

  virtual bool random(const unsigned &prob) {
    return (gen1.get() % (precision + 1) < prob);
  }

public:
  pacer_v2(const float &stall_prob_, const float &hold_stall_prob_)
    : stall_prob(stall_prob_ * static_cast<float>(precision)),
      hold_stall_prob(hold_stall_prob_ * static_cast<float>(precision)),
      stalled(false) {}

  stable_random gen1;

  virtual void reset() { stalled = false; }

  virtual void set_stall_prob(float &newProb) { stall_prob=(static_cast<float>(newProb) * precision); }
  virtual void set_hold_stall_prob(float &newProb) { hold_stall_prob=(static_cast<float>(newProb) * precision); }

   virtual bool force_zero() { return tic(); }

  // return whether we should stall this cycle
  virtual bool tic() {
#ifdef DISABLE_PACER
    return false;
#endif
    if (stalled) {
      if (!random(hold_stall_prob)) {
        stalled = false;
      }
    } else {
      if (random(stall_prob)) {
        stalled = true;
      }
    }
    return stalled;
  }

};

struct rand_stall {

  static void set_helper(sc_object *obj, stable_random& gen, bool chatty ) {
#ifdef CONNECTIONS_SIM_ONLY
    if ( Connections::set_rand_force_if *p = dynamic_cast<Connections::set_rand_force_if*>(obj) ) {
      double x = gen.get() % 100;
      double y = gen.get() % 100;
 
      auto pacer = std::make_shared<pacer_v2>(x/100, y/100);
      pacer->gen1.pathname = obj->name();
      p->set_rand_force(pacer);

      if (chatty)
        std::cout << "prob stall% hold% " << x << " " << y << " " << obj->name() << "\n";
    }
    std::vector<sc_object *> children = obj->get_child_objects();
    for ( unsigned i = 0; i < children.size(); i++ ) {
      if ( children[i] ) {
        set_helper(children[i], gen, chatty);
      }
    }
#endif
  }

  static void set(sc_object &sc_obj, bool chatty = 0 ) {
#ifdef CONNECTIONS_SIM_ONLY
    stable_random gen;
    set_helper(&sc_obj, gen, chatty);
#endif
  }
};

} // namespace connections
