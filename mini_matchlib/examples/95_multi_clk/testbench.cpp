
#include "dut.h"
#include <memory>

class Top : public sc_module
{
public:
  dut SC_NAMED(dut1);

  sc_clock clk1;
  sc_clock clk5;
  sc_signal<bool> SC_NAMED(rst_bar);

  Connections::Combinational<uint32_t>        SC_NAMED(out1);
  Connections::Combinational<uint32_t>        SC_NAMED(in1);


  SC_CTOR(Top) : 
    clk1("clk1", 1, SC_NS, 0.5,0,SC_NS,true),
    clk5("clk5", 5, SC_NS, 0.5,0,SC_NS,true) 
  {
    dut1.clk5(clk5);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);

    SC_CTHREAD(reset, clk1);

    SC_THREAD(stim);
    sensitive << clk5.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk1.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }

  void stim() {
    std::cout << sc_time_stamp() << " Stimulus started\n";
    in1.ResetWrite();
    wait();

    for (int i = 0; i < 10; i++) { in1.Push(i); }

    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    // wait(233, SC_PS); // enable this to test checking that Push/Pop are done on clock edges

    while (1) { 
     auto v = out1.Pop();
     std::cout << sc_time_stamp() << " TB resp sees: " << std::hex << v << "\n";
    }
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait(20);
    // we are simulating a "dynamic" reset here, occurs at 25 ns, duration 10 ns
    std::cout << sc_time_stamp() << " starting dynamic reset\n";
    rst_bar.write(0);
    wait(10);
    std::cout << sc_time_stamp() << " ending dynamic reset\n";
    rst_bar.write(1);
  }
};

sc_trace_file *trace_file_ptr;

int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  trace_file_ptr = sc_create_vcd_trace_file("trace");

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

