#include <mc_connections.h>
#include <ac_sysc_trace.h>
#include <stable_random.h>
#include <memory.h>


#ifdef SINGLE_PROCESS
#include "mat_mul_single_process.h"
#endif

#ifdef MULTI_PROCESS
#include "mat_mul_multi_process.h"
#endif

#ifdef BANKED_MULTI_PROCESS
#include "mat_mul_banked_multi_process.h"
#endif

#include <ac_bank_array.h>

#include <mc_scverify.h>

class Top : public sc_module {
 public:
  sc_clock clk;
  SC_SIG(bool, rstn);
  Connections::Combinational<ac_int<8> > SC_NAMED(A);
  Connections::Combinational<ac_int<8> > SC_NAMED(B);
  Connections::Combinational<ac_int<8+8+3> > SC_NAMED(C);

  CCS_DESIGN(matrixMultiply) SC_NAMED(dut);
  ac_bank_array_2D<int8, 8, 8> A_ref;
  ac_bank_array_2D<int8, 8, 8> B_ref;
  ac_bank_array_2D<int19, 8, 8> C_ref;

  SC_CTOR(Top)
    :   clk("clk", 2, SC_NS, 1,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut.clk(clk);
    dut.rstn(rstn);
    dut.A(A);
    dut.B(B);
    dut.C(C);

    SC_THREAD(reset);
    sensitive << clk.posedge_event();

    SC_THREAD(stimA);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);

    SC_THREAD(stimB);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);
  }

  void stimA() {
    CCS_LOG("Stimulus reset started");
    A.ResetWrite();
    wait();
    
    stable_random gen;

    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++) {
        A_ref[i][j] = gen.get();
      }
    }
    for (int k=0; k<2; k++) 
      for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
          A.Push(A_ref[i][j]);
        }
      }
    wait();
  }

  void stimB() {
    B.ResetWrite();
    wait();

    stable_random gen;

    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++) {
        B_ref[i][j] = gen.get();
      }
    }

    for (int k=0; k<2; k++){
      for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
          B.Push(B_ref[i][j]);
        }
      }
      wait(2);//add some time between matrices
    } 
    wait();
  }

  void resp() {
    C.ResetRead();
    wait(5);
    // Reference matrix multiply
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        C_ref[i][j] = 0;
        for (int k = 0; k < 8; k++) {
          C_ref[i][j] += A_ref[i][k] * B_ref[k][j];
        }
      }
    }
    int err = 0;
    for (int k=0; k<2; k++) //run twice
      for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
          ac_int<8+8+3> res = C.Pop();
          CCS_LOG("TB resp sees: " << res);
          if (res != C_ref[i][j]) {
            printf("ERROR expected %d, recieved %d\n",C_ref[i][j].to_int(), res.to_int());
            err++;
          }
        }
      }
    if (err==0)
    { printf("Testbench passed with zero errors\n"); }

    sc_stop();
  }

  void reset() {
    rstn.write(0);
    wait(5);
    rstn.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv) {
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  auto top = std::make_shared<Top>("top");
  trace_hierarchy(top.get(), trace_file_ptr);

  channel_logs logs;

  logs.enable("my_log",true);
  logs.log_hierarchy(*top);

  sc_start();
  return 0;
}

