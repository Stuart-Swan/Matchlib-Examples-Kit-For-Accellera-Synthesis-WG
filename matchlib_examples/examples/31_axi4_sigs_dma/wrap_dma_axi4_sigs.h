// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include "axi4_signal_transactor.h"
#include "dma.h"

// This module wraps the DMA which uses Connections::In/Out interfaces to use port
// interfaes where each AXI4 field is a separate port

typedef axi::axi4_signal_transactor<axi::cfg::standard> local_axi_sig_cfg;

class wrap_dma_axi4_sigs : public sc_module , public local_axi_sig_cfg {
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);
  Connections::Out<bool> CCS_INIT_S1(dma_done);
  Connections::Out<sc_uint<32>> CCS_INIT_S1(dma_dbg);

  axi4_signal_r_slave_ports<> CCS_INIT_S1(r_slave0_ports);
  axi4_signal_w_slave_ports<> CCS_INIT_S1(w_slave0_ports);
  axi4_signal_r_slave_xactor<> CCS_INIT_S1(r_slave0_xactor);
  axi4_signal_w_slave_xactor<> CCS_INIT_S1(w_slave0_xactor);

  axi4_signal_r_master_ports<> CCS_INIT_S1(r_master0_ports);
  axi4_signal_w_master_ports<> CCS_INIT_S1(w_master0_ports);
  axi4_signal_r_master_xactor<> CCS_INIT_S1(r_master0_xactor);
  axi4_signal_w_master_xactor<> CCS_INIT_S1(w_master0_xactor);

  dma CCS_INIT_S1(dma1);

  local_axi_sig_cfg::r_chan<> CCS_INIT_S1(dma_slave_r_chan);
  local_axi_sig_cfg::w_chan<> CCS_INIT_S1(dma_slave_w_chan);

  local_axi_sig_cfg::r_chan<> CCS_INIT_S1(dma_master_r_chan);
  local_axi_sig_cfg::w_chan<> CCS_INIT_S1(dma_master_w_chan);
  
  SC_CTOR(wrap_dma_axi4_sigs) {
    dma1.clk(clk);
    dma1.rst_bar(rst_bar);
    dma1.dma_done(dma_done);
    dma1.dma_dbg(dma_dbg);
    dma1.r_slave0(dma_slave_r_chan);
    dma1.w_slave0(dma_slave_w_chan);
    dma1.r_master0(dma_master_r_chan);
    dma1.w_master0(dma_master_w_chan);

    r_slave0_xactor(r_slave0_ports);
    r_slave0_xactor.ar_port(dma_slave_r_chan.ar);
    r_slave0_xactor.r_port(dma_slave_r_chan.r);

    w_slave0_xactor(w_slave0_ports);
    w_slave0_xactor.aw_port(dma_slave_w_chan.aw);
    w_slave0_xactor.w_port(dma_slave_w_chan.w);
    w_slave0_xactor.b_port(dma_slave_w_chan.b);

    r_master0_xactor(r_master0_ports);
    r_master0_xactor.ar_port(dma_master_r_chan.ar);
    r_master0_xactor.r_port(dma_master_r_chan.r);

    w_master0_xactor(w_master0_ports);
    w_master0_xactor.aw_port(dma_master_w_chan.aw);
    w_master0_xactor.w_port(dma_master_w_chan.w);
    w_master0_xactor.b_port(dma_master_w_chan.b);

  }
};
