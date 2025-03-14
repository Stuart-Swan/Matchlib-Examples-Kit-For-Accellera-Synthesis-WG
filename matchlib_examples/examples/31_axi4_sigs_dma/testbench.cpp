// INSERT_EULA_COPYRIGHT: 2020

#include "dma.h"
#include "ram.h"
#include <mc_scverify.h>
#include <memory.h>

#include "axi4_signal_transactor.h"


#include "wrap_dma_axi4_sigs.h"
#include "wrap_ram_axi4_sigs.h"

class Top : public sc_module, public local_axi_sig_cfg
{
public:
  wrap_ram_axi4_sigs             SC_NAMED(ram1);
  CCS_DESIGN(wrap_dma_axi4_sigs) SC_NAMED(dma1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);

  w_master<>    SC_NAMED(tb_w_master);
  r_master<>    SC_NAMED(tb_r_master);
  w_chan<> SC_NAMED(dma_w_chan_slave);
  r_chan<> SC_NAMED(dma_r_chan_slave);
  axi4_signal_w_master_xactor<>    SC_NAMED(tb_signal_w_master_xactor);
  axi4_signal_r_master_xactor<>    SC_NAMED(tb_signal_r_master_xactor);

  axi4_signal_w_chan SC_NAMED(dma_w_slave);
  axi4_signal_r_chan SC_NAMED(dma_r_slave);
  axi4_signal_w_chan SC_NAMED(dma_w_master);
  axi4_signal_r_chan SC_NAMED(dma_r_master);

  Connections::Combinational<bool>        SC_NAMED(dma_done);
  Connections::Combinational<sc_uint<32>> SC_NAMED(dma_dbg);

  SC_CTOR(Top)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {

    tb_w_master(dma_w_chan_slave);
    tb_r_master(dma_r_chan_slave);

    tb_signal_w_master_xactor(dma_w_slave);
    tb_signal_w_master_xactor.aw_port(dma_w_chan_slave.aw);
    tb_signal_w_master_xactor.w_port(dma_w_chan_slave.w);
    tb_signal_w_master_xactor.b_port(dma_w_chan_slave.b);

    tb_signal_r_master_xactor(dma_r_slave);
    tb_signal_r_master_xactor.ar_port(dma_r_chan_slave.ar);
    tb_signal_r_master_xactor.r_port(dma_r_chan_slave.r);

    ram1.clk(clk);
    ram1.rst_bar(rst_bar);
    ram1.r_slave0_ports(dma_r_master);
    ram1.w_slave0_ports(dma_w_master);

    dma1.clk(clk);
    dma1.rst_bar(rst_bar);
    dma1.r_master0_ports(dma_r_master);
    dma1.w_master0_ports(dma_w_master);
    dma1.r_slave0_ports(dma_r_slave);
    dma1.w_slave0_ports(dma_w_slave);
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

  void stim() {
    CCS_LOG("Stimulus started");
    tb_w_master.reset();
    tb_r_master.reset();
    wait();

    tb_w_master.single_write(offsetof(dma_address_map, ar_addr), source_addr);
    tb_w_master.single_write(offsetof(dma_address_map, aw_addr), target_addr);
    tb_w_master.single_write(offsetof(dma_address_map, len),     beats - 1);  // axi encoding: 0 means 1 beat
    tb_w_master.single_write(offsetof(dma_address_map, start),   0x1);
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

  auto top = std::make_shared<Top>("top");
  trace_hierarchy(top.get(), trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(*top);

  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}

