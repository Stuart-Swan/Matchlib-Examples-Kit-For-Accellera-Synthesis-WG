/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Adder3.h"
#include <systemc.h>
#include <mc_scverify.h>

#include <connections/annotate.h>

#include <deque>
using namespace::std;

// Manually set RAND_SEED since we're dependent on
// Pacer() behavior to get proper cycle count for this
// test
#define RAND_SEED 1

#include <connections/Pacer.h>

typedef deque<int> Fifo;

SC_MODULE (Source)
{
  Connections::Out<Adder3::Data> x_out;

  sc_in <bool> clk;
  sc_in <bool> rst;
  const int start_val;
  Pacer pacer;

  Fifo fifo;

  void run() {
    x_out.Reset();
    pacer.reset();
    fifo.clear();

    Adder3::Data x = start_val;

    // Wait for initial reset.
    wait(20.0, SC_NS);

    wait();

    while (1) {
      //cout << "@" << sc_time_stamp() << "\t" << name() << " sending X=" << x << endl ;
      x_out.Push(x);
      //cout << "@" << sc_time_stamp() << "\t" << name() << " DONE" << endl ;
      fifo.push_back(x);
      ++x;

      wait();
      while (pacer.tic()) {
        //cout << "@" << sc_time_stamp() << "\t" << name() << " STALL" << endl ;
        wait();
      }
    }
  }

  SC_HAS_PROCESS(Source);

  Source(sc_module_name name_, const int &start_val_, const Pacer& pacer_) :
    sc_module(name_),
    x_out("x_out"),
    clk("clk"),
    rst("rst"),
    start_val(start_val_),
    pacer(pacer_) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rst,false);
  }
};

SC_MODULE (Dest)
{
  Connections::In<Adder3::Data> sum_in;

  sc_in <bool> clk;
  sc_in <bool> rst;

  Fifo &fifo_a;
  Fifo &fifo_b;

  Pacer pacer;

  unsigned long &count;

  void run() {
    sum_in.Reset();
    pacer.reset();
    Adder3::Data sum;

    // Wait for initial reset.
    wait(20.0, SC_NS);

    wait();

    while (1) {
      //cout << "@" << sc_time_stamp() << "\t\t\t\t\t\t Dest checking for result" << endl ;
      sum = sum_in.Pop();
      //cout << "@" << sc_time_stamp() << "\t\t\t\t\t\t       GOT " << sum << endl;
      count++;

      while (fifo_a.empty() || fifo_b.empty()) {
        wait();
      }

      if (sum == ((fifo_a.front() + fifo_b.front()) & 0xffff)) {
        fifo_b.pop_front();
      } else {
        assert(sum == (fifo_a.front() & 0xffff));
      }
      fifo_a.pop_front();
      wait();

      while (pacer.tic()) {
        //cout << "@" << sc_time_stamp() << "\t" << name() << " STALL" << endl ;
        wait();
      }
    }
  }

  SC_HAS_PROCESS(Dest);
  Dest(sc_module_name name_, Fifo& fifo_a_, Fifo& fifo_b_, const Pacer& pacer_, unsigned long &count_) : 
    sc_module(name_),
    sum_in("sum_in"),
    clk("clk"),
    rst("rst"),
    fifo_a(fifo_a_),
    fifo_b(fifo_b_),
    pacer(pacer_),
    count(count_) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rst,false);
  }
};


SC_MODULE (testbench)
{
  CCS_DESIGN(Adder3) adder;
  Source srca,srcb;
  Dest dest;

  Connections::Combinational<Adder3::Data> a,b,sum;

  sc_clock clk;
  sc_signal<bool> rst;

  unsigned long count;

  SC_CTOR(testbench) :
    adder("adder"),
    srca("srca", 7, Pacer(0.3, 0.7)),
    srcb("srcb", 13, Pacer(0.2, 0.5)),
    dest("dest", srca.fifo, srcb.fifo, Pacer(0.2, 0.5), count),  //using a member inside constrction sequence could lead to a runtime bug
    a("a"),
    b("b"),
    sum("sum"),
    clk("clk", 1, SC_NS, 0.5,0,SC_NS,true),
    rst("rst"),
    count(0) {
    adder.clk(clk);
    adder.rst(rst);

    srca.clk(clk);
    srca.rst(rst);

    srcb.clk(clk);
    srcb.rst(rst);

    dest.clk(clk);
    dest.rst(rst);

    srca.x_out(a);
    srcb.x_out(b);

    adder.a_in(a);
    adder.b_in(b);
    adder.sum_out(sum);

    dest.sum_in(sum);

    SC_THREAD(run);
  }

  void run() {
    //reset
    rst = 1;
    wait(10.5, SC_NS);
    rst = 0;
    //cout << "@" << sc_time_stamp() << " Asserting Reset " << endl ;
    wait(1, SC_NS);
    //cout << "@" << sc_time_stamp() << " Deasserting Reset " << endl ;
    rst = 1;
    wait(1000,SC_NS);
    //cout << "@" << sc_time_stamp() << " Stop " << endl ;
    sc_stop();
    cout << "Final count = " << count << endl;
#ifdef CCS_SYSC
    unsigned int correct_count = 176; // TODO - why mismatch?
#else
    unsigned int correct_count = 175;
#endif
    if (count != correct_count) {
      std::ostringstream msg;
      msg << "Error: Count does not match expected value (" << correct_count << ")!";
      SC_REPORT_ERROR("Testbench", msg.str().c_str());
    }
  }
};



int sc_main(int argc, char *argv[])
{
#ifdef CONN_RAND_STALL
  cout << "Error: This unit test doesn't support CONN_RAND_STALL since it looks for specific cycle times! Please re-run without it enabled." << endl;
  assert(0);
#endif
  testbench my_testbench("my_testbench");
  std::string base_name = "";
  if ( sc_argc() >= 2 ) {
    base_name = sc_argv()[1];
  }
  std::string input_dir_path = "";
  if ( sc_argc() >= 3 ) {
    input_dir_path = sc_argv()[2];
  }

  Connections::annotate_design(my_testbench, base_name, input_dir_path);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_start();
  int retv = sc_report_handler::get_count(SC_ERROR);
  if ( retv != 0 ) {
    cout << "CMODEL FAILED" << endl;
  } else {
    cout << "CMODEL PASS" << endl;
  }
  return retv;
};

