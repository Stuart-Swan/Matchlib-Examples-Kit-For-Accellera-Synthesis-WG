// INSERT_EULA_COPYRIGHT: 2020

#include "dut.h"
#include <mc_scverify.h>
#include <memory.h>


class Top : public sc_module
{
public:
  CCS_DESIGN(dut) SC_NAMED(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<mem_with_stall_req<addr_t, data_t>> SC_NAMED(req_chan);
  Connections::Combinational<mem_with_stall_rsp<data_t>> SC_NAMED(rsp_chan);
  mem_with_stall_chan<addr_t, data_t> SC_NAMED(mem_chan);
  mem_with_stall_memory<addr_t, data_t, 256> SC_NAMED(memory1);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.in1(req_chan);
    dut1.out1(rsp_chan);
    dut1.mem1(mem_chan);

    memory1.clk(clk);
    memory1.rst_bar(rst_bar);
    memory1.mem_with_stall_in1(mem_chan);
    memory1.debug = 1;
    memory1.do_stall = 1;

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }


  void stim() {
    CCS_LOG("Stimulus started");
    req_chan.ResetWrite(); 
    wait();

    for (int i=0; i < 10; i++) {
      mem_with_stall_req<addr_t, data_t> req;

      req.write_en = 1;
 
      req.address = i;
      req.write_data = i;
      req_chan.Push(req);
    }

    for (int i=0; i < 10; i++) {
      mem_with_stall_req<addr_t, data_t> req;

      req.read_en = 1;
 
      req.address = i;
      req.write_data = 0;
      req_chan.Push(req);
    }


    wait(10);
    sc_stop();
    wait();
  }

  void resp() {
    rsp_chan.ResetRead();
    wait();
 
    while (1) {
      auto rsp = rsp_chan.Pop();
      CCS_LOG("TB read rsp is: " << (int)rsp.read_data);
    }
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  auto top = std::make_shared<Top>("top");
  trace_hierarchy(top.get(), trace_file_ptr);
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}
