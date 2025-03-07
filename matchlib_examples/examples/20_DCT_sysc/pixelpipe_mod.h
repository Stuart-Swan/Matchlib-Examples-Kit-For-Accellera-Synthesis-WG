// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include "jpeg.h"
#include "dct.h"
#include "mc_fifo.h"
#include <mc_scverify.h>

SC_MODULE(pixelpipe_mod)
{
  public:
  sc_in <bool>                        SC_NAMED(clk);
  sc_in <bool>                        SC_NAMED(rstn);
  Connections::In<ac_int<2, false > > SC_NAMED(blocktype);
  Connections::In<rgb_t >             SC_NAMED(rgb);
  Connections::Out<codes_t>           SC_NAMED(codes);

#ifdef TOP_PIXELPIPE_MOD
  dct SC_NAMED(dct_inst);
#else
  CCS_DESIGN(dct) SC_NAMED(dct_inst);
#endif

  SC_CTOR(pixelpipe_mod) {
    SC_THREAD(broadcast_exec);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
    SC_THREAD(convert_exec);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
    SC_THREAD(quantize_exec);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
    SC_THREAD(encode_exec);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

    //DCT instance port bindings
    dct_inst.clk(clk);
    dct_inst.rstn(rstn);
    dct_inst.input(converted);
    dct_inst.output(transformed);
    dct_inst.sync_out(sync_out);

    //DCT output memory port bindings
    transformed.CK(clk);

    //fifo clock and reset port bindings

    quantized.clk(clk);
    quantized.rst_bar(rstn);
    quantized.in1(quantized_in);
    quantized.out1(quantized_out);

    param1.clk(clk);
    param1.rst_bar(rstn);
    param1.in1(param1_in);
    param1.out1(param1_out);

    param2.clk(clk);
    param2.rst_bar(rstn);
    param2.in1(param2_in);
    param2.out1(param2_out);

    param3.clk(clk);
    param3.rst_bar(rstn);
    param3.in1(param3_in);
    param3.out1(param3_out);
  }

  void broadcast();
  void convert();
  void quantize(bool &pp);
  void encode();

  private:

  void broadcast_exec() {
    blocktype.Reset();
    param1_in.ResetWrite();
    param2_in.ResetWrite();
    param3_in.ResetWrite();
    wait();                                 // WAIT
    while (1) {
      broadcast();
    }
  }

  // convert 8x8 RGB block to 8x8 YCbCr
  void convert_exec() {
    rgb.Reset();
    param1_out.ResetRead();
    converted.ResetWrite();
    wait();                                 // WAIT
    while (1) { convert(); }
  }

  // zig-zag and quantize the results
  void quantize_exec() {
    bool pp = false;
    param2_out.ResetRead();
    transformed.reset_read();
    quantized_in.ResetWrite();
    sync_out.reset_sync_in();
    last_non_zero_index.ResetWrite();
    wait();                                 // WAIT
    while (1) { quantize(pp); }
  }

  // run-length and Huffman encode
  void encode_exec() {
    codes.Reset();
    quantized_out.ResetRead();
    param3_out.ResetRead();
    last_non_zero_index.ResetRead();
    wait();                                 // WAIT
    while (1) { encode(); }
  }

  Connections::Combinational<ac_int<8>> SC_NAMED(converted);
  RAM_1R1W_model<>::mem<ac_int<16>,128>  SC_NAMED(transformed);
  Connections::SyncChannel SC_NAMED(sync_out);
  Connections::Combinational<uint6> SC_NAMED(last_non_zero_index);

  Connections::FifoModule<ac_int<16, true>, 65> SC_NAMED(quantized);
  Connections::Combinational<ac_int<16, true>> SC_NAMED(quantized_in);
  Connections::Combinational<ac_int<16, true>> SC_NAMED(quantized_out);

  Connections::FifoModule<uint2, 8>  SC_NAMED(param1);
  Connections::Combinational<uint2>  SC_NAMED(param1_in);
  Connections::Combinational<uint2>  SC_NAMED(param1_out);

  Connections::FifoModule<uint2, 8>  SC_NAMED(param2);
  Connections::Combinational<uint2>  SC_NAMED(param2_in);
  Connections::Combinational<uint2>  SC_NAMED(param2_out);

  Connections::FifoModule<uint2, 8>  SC_NAMED(param3);
  Connections::Combinational<uint2>  SC_NAMED(param3_in);
  Connections::Combinational<uint2>  SC_NAMED(param3_out);

};

