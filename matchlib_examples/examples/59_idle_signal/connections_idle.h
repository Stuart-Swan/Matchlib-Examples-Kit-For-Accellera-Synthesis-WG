
#pragma once

// utilities for generation of idle signals in matchlib connections models.


#define COUNTER_THREAD(x) \
  void x ## _thread () {  \
    x ## _cnt = 0;        \
    wait();               \
    while (1) {           \
      if (x .rdy.read() && x .vld.read())        \
        x ## _cnt = x ## _cnt.read() + 1;        \
      wait();                                    \
    }                                            \
  }

#define IN_STALLED(x) ((x.rdy.read() == 1) && (x.vld.read() == 0))
#define OUT_STALLED(x) ((x.rdy.read() == 0) && (x.vld.read() == 1))
#define EQUAL_COUNTS(x, y) (x ## _cnt.read() == y ## _cnt.read())
