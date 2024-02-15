// INSERT_EULA_COPYRIGHT: 2020

#include <mc_scverify.h>

#include "tlm2_axi4_adapters.h"

typedef axi::axi4_segment<axi::cfg::standard> local_axi;

#include "wrap_ram_tlm2.h"
#include "wrap_dma_tlm2.h"

class Top : public sc_module, public local_axi
{
public:
  wrap_ram_tlm2 CCS_INIT_S1(ram1);
  wrap_dma_tlm2 CCS_INIT_S1(dma1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  Connections::Combinational<bool>        CCS_INIT_S1(dma_done);
  Connections::Combinational<sc_uint<32>> CCS_INIT_S1(dma_dbg);
  tlm_utils::simple_initiator_socket<Top> tb_tlm2_initiator;

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    ram1.clk(clk);
    ram1.rst_bar(rst_bar);

    dma1.clk(clk);
    dma1.rst_bar(rst_bar);
    dma1.tlm2_initiator(ram1.tlm2_target);
    tb_tlm2_initiator(dma1.tlm2_target);

    dma1.dma_done(dma_done);
    dma1.dma_dbg(dma_dbg);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(dbg_mon);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(done_mon);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    sc_object_tracer<sc_clock> trace_clk(clk);
  }

  sc_time start_time, end_time;
  int beats = 0x40;
  int source_addr = 0x1000;
  int target_addr = 0x4000;

  void single_write(uint64_t addr, uint64_t data) {
    tlm::tlm_generic_payload trans;
    sc_time zero_time(SC_ZERO_TIME);

    trans.set_write();
    trans.set_address(addr);
    trans.set_data_ptr((unsigned char*)&data);
    trans.set_data_length(sizeof(data));
    trans.set_byte_enable_ptr(0);
    trans.set_byte_enable_length(0);
    trans.set_streaming_width(sizeof(data));
    tb_tlm2_initiator->b_transport(trans, zero_time);
  }

  void stim() {
    CCS_LOG("Stimulus started");

    wait(10); // let DUT come out of reset fully

    single_write(offsetof(dma_address_map, ar_addr), source_addr);
    single_write(offsetof(dma_address_map, aw_addr), target_addr);
    single_write(offsetof(dma_address_map, len),     beats - 1);  // axi encoding: 0 means 1 beat
    single_write(offsetof(dma_address_map, start),   0x1);
    start_time = sc_time_stamp();

    wait(2000, SC_NS);
    CCS_LOG("stopping sim due to testbench timeout");
    sc_stop();
    wait();
  }

  void done_mon() {
    dma_done.ResetRead();
    wait();

    while (1) {
      bool v = dma_done.Pop();
      end_time = sc_time_stamp();

      CCS_LOG("dma_done detected. " << v);
      CCS_LOG("start_time: " << start_time << " end_time: " << end_time);
      CCS_LOG("axi beats (dec): " << std::dec << beats );
      sc_time elapsed =  end_time - start_time;
      CCS_LOG("elapsed time: " << elapsed);
      CCS_LOG("beat rate: " << (elapsed / beats));
      CCS_LOG("clock period: " << sc_time(1, SC_NS));

      for (int i=0; i < beats; i++) {
        int s = ram1.ram1.debug_read_addr(source_addr + (i * axi_cfg::dataWidth/8));
        int t = ram1.ram1.debug_read_addr(target_addr + (i * axi_cfg::dataWidth/8));
        if (s != t) {
          CCS_LOG("ram source and target data mismatch! Beat#: " << i << " " <<  std::hex << " s:" << s << " t: " << t);
        }
      }

      sc_stop();
    }
  }

  void dbg_mon() {
    dma_dbg.ResetRead();
    wait();

    while (1) {
      uint32_t v = dma_dbg.Pop();
      CCS_LOG("dma_dbg val: " << std::hex << v);
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

  Top top("top");
  trace_hierarchy(&top, trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(top);

  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}

