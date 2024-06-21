// Auto generated on: Thu Jun 20 07:42:01 2024


// This file shows the SystemVerilog input/output declarations for the exported wrapped SC model.
// This file is only for documentation purposes.

typedef struct packed {
  reg  [7:0] len; // width: 8
  reg  [31:0] addr; // width: 32
  reg  [3:0] id; // width: 4
} r_master0_ar_dat_type; // width: 44

typedef struct packed {
  reg  [0:0] last; // width: 1
  reg  [1:0] resp; // width: 2
  reg  [63:0] data; // width: 64
  reg  [3:0] id; // width: 4
} r_master0_r_dat_type; // width: 71

typedef struct packed {
  reg  [7:0] len; // width: 8
  reg  [31:0] addr; // width: 32
  reg  [3:0] id; // width: 4
} w_master0_aw_dat_type; // width: 44

typedef struct packed {
  reg  [7:0] wstrb; // width: 8
  reg  [0:0] last; // width: 1
  reg  [63:0] data; // width: 64
} w_master0_w_dat_type; // width: 73

typedef struct packed {
  reg  [1:0] resp; // width: 2
  reg  [3:0] id; // width: 4
} w_master0_b_dat_type; // width: 6

typedef struct packed {
  reg  [7:0] len; // width: 8
  reg  [31:0] addr; // width: 32
  reg  [3:0] id; // width: 4
} r_slave0_ar_dat_type; // width: 44

typedef struct packed {
  reg  [0:0] last; // width: 1
  reg  [1:0] resp; // width: 2
  reg  [63:0] data; // width: 64
  reg  [3:0] id; // width: 4
} r_slave0_r_dat_type; // width: 71

typedef struct packed {
  reg  [7:0] len; // width: 8
  reg  [31:0] addr; // width: 32
  reg  [3:0] id; // width: 4
} w_slave0_aw_dat_type; // width: 44

typedef struct packed {
  reg  [7:0] wstrb; // width: 8
  reg  [0:0] last; // width: 1
  reg  [63:0] data; // width: 64
} w_slave0_w_dat_type; // width: 73

typedef struct packed {
  reg  [1:0] resp; // width: 2
  reg  [3:0] id; // width: 4
} w_slave0_b_dat_type; // width: 6


module dma(
  clk
, rst_bar
, r_master0_ar_rdy
, r_master0_ar_vld
, r_master0_ar_dat
, r_master0_r_rdy
, r_master0_r_vld
, r_master0_r_dat
, w_master0_aw_rdy
, w_master0_aw_vld
, w_master0_aw_dat
, w_master0_w_rdy
, w_master0_w_vld
, w_master0_w_dat
, w_master0_b_rdy
, w_master0_b_vld
, w_master0_b_dat
, r_slave0_ar_rdy
, r_slave0_ar_vld
, r_slave0_ar_dat
, r_slave0_r_rdy
, r_slave0_r_vld
, r_slave0_r_dat
, w_slave0_aw_rdy
, w_slave0_aw_vld
, w_slave0_aw_dat
, w_slave0_w_rdy
, w_slave0_w_vld
, w_slave0_w_dat
, w_slave0_b_rdy
, w_slave0_b_vld
, w_slave0_b_dat
, dma_done_rdy
, dma_done_vld
, dma_done_dat
, dma_dbg_rdy
, dma_dbg_vld
, dma_dbg_dat
);
  input [0:0] clk;
  input [0:0] rst_bar;
  input  r_master0_ar_rdy;
  output r_master0_ar_vld;
  output r_master0_ar_dat_type r_master0_ar_dat;
  output r_master0_r_rdy;
  input  r_master0_r_vld;
  input  r_master0_r_dat_type r_master0_r_dat;
  input  w_master0_aw_rdy;
  output w_master0_aw_vld;
  output w_master0_aw_dat_type w_master0_aw_dat;
  input  w_master0_w_rdy;
  output w_master0_w_vld;
  output w_master0_w_dat_type w_master0_w_dat;
  output w_master0_b_rdy;
  input  w_master0_b_vld;
  input  w_master0_b_dat_type w_master0_b_dat;
  output r_slave0_ar_rdy;
  input  r_slave0_ar_vld;
  input  r_slave0_ar_dat_type r_slave0_ar_dat;
  input  r_slave0_r_rdy;
  output r_slave0_r_vld;
  output r_slave0_r_dat_type r_slave0_r_dat;
  output w_slave0_aw_rdy;
  input  w_slave0_aw_vld;
  input  w_slave0_aw_dat_type w_slave0_aw_dat;
  output w_slave0_w_rdy;
  input  w_slave0_w_vld;
  input  w_slave0_w_dat_type w_slave0_w_dat;
  input  w_slave0_b_rdy;
  output w_slave0_b_vld;
  output w_slave0_b_dat_type w_slave0_b_dat;
  input  dma_done_rdy;
  output dma_done_vld;
  output [0:0] dma_done_dat;
  input  dma_dbg_rdy;
  output dma_dbg_vld;
  output [31:0] dma_dbg_dat;
endmodule;
