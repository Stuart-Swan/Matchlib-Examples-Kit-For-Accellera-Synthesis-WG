

#include "dut.h"

#include <mc_scverify.h>

#include "sc_named.h"

class Top : public sc_module {
 public:
  using T_data = dut::T_data;
  using T_addr = dut::T_addr;
  static constexpr unsigned int MEM_SIZE = 256;
  CCS_DESIGN(dut) SC_NAMED(dut1);
  sc_clock clk;
  SC_SIG(bool, rst_n);

  SC_SIG(bool, start);
  SC_SIG(bool, done);

  SC_SIG(bool, mem1_write_en);
  SC_SIG(bool, mem1_read_en);
  SC_SIG(T_addr, mem1_addr);
  SC_SIG(T_data, mem1_write_data);
  SC_SIG(T_data, mem1_read_data);
  SC_SIG(bool, mem1_stall);

  SC_SIG(bool, mem2_write_en);
  SC_SIG(bool, mem2_read_en);
  SC_SIG(T_addr, mem2_addr);
  SC_SIG(T_data, mem2_write_data);
  SC_SIG(T_data, mem2_read_data);
  SC_SIG(bool, mem2_stall);

  mem_with_stall_memory<T_data, T_addr, MEM_SIZE> SC_NAMED(mem1);
  mem_with_stall_memory<T_data, T_addr, MEM_SIZE> SC_NAMED(mem2);

  SC_CTOR(Top) : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_n(rst_n);
    dut1.start(start);
    dut1.done(done);

    sc_in<bool> stall;
    sc_out<bool> read_en;
    sc_out<bool> write_en;
    sc_out<T_addr> address;
    sc_out<T_data> write_data;
    sc_in<T_data> read_data;
    dut1.mem1.stall(mem1_stall);
    dut1.mem1.read_en(mem1_read_en);
    dut1.mem1.write_en(mem1_write_en);
    dut1.mem1.address(mem1_addr);
    dut1.mem1.write_data(mem1_write_data);
    dut1.mem1.read_data(mem1_read_data);
    dut1.mem2.stall(mem2_stall);
    dut1.mem2.read_en(mem2_read_en);
    dut1.mem2.write_en(mem2_write_en);
    dut1.mem2.address(mem2_addr);
    dut1.mem2.write_data(mem2_write_data);
    dut1.mem2.read_data(mem2_read_data);

    mem1.debug = 1;
    // mem1.do_stall = 1;  // Set to 1 to enable stall
    mem2.debug = 1;
    // mem2.do_stall = 1;  // Set to 1 to enable stall
    mem1.clk(clk);
    mem1.rst_n(rst_n);
    mem1.stallable_mem_slave.stall(mem1_stall);
    mem1.stallable_mem_slave.read_en(mem1_read_en);
    mem1.stallable_mem_slave.write_en(mem1_write_en);
    mem1.stallable_mem_slave.address(mem1_addr);    
    mem1.stallable_mem_slave.write_data(mem1_write_data);
    mem1.stallable_mem_slave.read_data(mem1_read_data);
    mem2.clk(clk);
    mem2.rst_n(rst_n);
    mem2.stallable_mem_slave.stall(mem2_stall);
    mem2.stallable_mem_slave.read_en(mem2_read_en);
    mem2.stallable_mem_slave.write_en(mem2_write_en);
    mem2.stallable_mem_slave.address(mem2_addr);
    mem2.stallable_mem_slave.write_data(mem2_write_data);
    mem2.stallable_mem_slave.read_data(mem2_read_data);

    SC_CTHREAD(reset, clk);
    SC_CTHREAD(stim, clk);
    async_reset_signal_is(rst_n, false);
    SC_CTHREAD(resp, clk);
    async_reset_signal_is(rst_n, false);
  }

  void stim() {
    CCS_LOG("Stimulus started");
    auto mem1_array = mem1.mem;
    auto mem2_array = mem2.mem;
    for (int i = 0; i < 10; i++) {
      mem1_array[i] = 10 * (i + 1);
    }
    for (int i = 10; i < 20; ++i) {
      mem1_array[i] = -200;
    }
    for (int i = 0; i < 10; i++) {
      mem2_array[i] = i + 1;
    }
    for (int i = 10; i < 20; ++i) {
      mem2_array[i] = -100;
    }
    start.write(0);
    wait();
    CCS_LOG("Asserting start");
    start = 1;
    wait();
    CCS_LOG("De-asserting start");
    start = 0;
    wait();
  }

  void resp() {
    while (start.read() || !done.read()) {
      wait();
    }
    CCS_LOG("DONE ASSERTED");
    wait(10);

    auto mem1_array = mem1.mem;
    auto mem2_array = mem2.mem;
    for (int i = 0; i < 10; i++) {
      CCS_LOG("MEM1_ARRAY[" << std::dec << i << "] = " << mem1_array[i]);
    }
    for (int i = 0; i < 10; i++) {
      CCS_LOG("MEM2_ARRAY[" << i << "] = " << mem2_array[i]);
    }

    CCS_LOG("Write partition:");
    for (int i = 10; i < 21; i++) {
      CCS_LOG("MEM1_ARRAY[" << std::dec << i << "] = " << mem1_array[i]);
    }
    sc_stop();
    wait();
  }

  void reset() {
    rst_n.write(0);
    CCS_LOG("Asserting reset");
    wait(5);
    CCS_LOG("De-asserting reset");
    rst_n.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv) {
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
