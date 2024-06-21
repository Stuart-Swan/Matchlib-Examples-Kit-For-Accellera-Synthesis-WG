// INSERT_EULA_COPYRIGHT: 2020


`timescale 1ns / 1ps


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

module testbench ();

  reg clk;
  reg rst_bar;
  wire r_master0_ar_dat_type r_master0_ar_dat;
  wire r_master0_ar_vld;
  wire r_master0_ar_rdy;
  wire r_master0_r_dat_type r_master0_r_dat;
  wire r_master0_r_vld;
  wire r_master0_r_rdy;
  wire w_master0_aw_dat_type w_master0_aw_dat;
  wire w_master0_aw_vld;
  wire w_master0_aw_rdy;
  wire w_master0_w_dat_type w_master0_w_dat;
  wire w_master0_w_vld;
  wire w_master0_w_rdy;
  wire w_master0_b_dat_type w_master0_b_dat;
  wire w_master0_b_vld;
  wire w_master0_b_rdy;
  wire r_slave0_ar_dat_type r_slave0_ar_dat;
  wire r_slave0_ar_vld;
  wire r_slave0_ar_rdy;
  wire r_slave0_r_dat_type r_slave0_r_dat;
  wire r_slave0_r_vld;
  wire r_slave0_r_rdy;
  w_slave0_aw_dat_type w_slave0_aw_dat;
  reg w_slave0_aw_vld;
  wire w_slave0_aw_rdy;
  w_slave0_w_dat_type w_slave0_w_dat;
  reg w_slave0_w_vld;
  wire w_slave0_w_rdy;
  wire w_slave0_b_dat_type w_slave0_b_dat;
  wire w_slave0_b_vld;
  wire w_slave0_b_rdy;
  wire dma_done_dat;
  wire dma_done_vld;
  wire dma_done_rdy;
  wire [31:0] dma_dbg_dat;
  wire dma_dbg_vld;
  wire dma_dbg_rdy;


  assign dma_dbg_rdy = 1'b1;
  assign dma_done_rdy = 1'b1;
  assign w_slave0_b_rdy = 1'b1;
  assign r_slave0_ar_dat = 73'h0000000000000000000;


    parameter clock_period = 10;

    initial begin
        rst_bar <= 1'b0;
        #clock_period
        rst_bar <= 1'b1;
    end

    initial
    begin
        clk <= 1'b1;
    end

    always
    begin : Clock_Generator
        #(clock_period / 2) clk <= ~clk;
    end

    initial begin
        #clock_period;
        #clock_period;
        w_slave0_aw_dat = 44'h0;
        w_slave0_aw_vld = 1'b1;
          #clock_period;
        while (w_slave0_aw_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_aw_vld = 1'b0;

        w_slave0_w_dat = 73'h0010000000000001000;
        w_slave0_w_vld = 1'b1;
          #clock_period;
        while (w_slave0_w_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_w_vld = 1'b0;

        w_slave0_aw_dat = 44'h80;
        w_slave0_aw_vld = 1'b1;
          #clock_period;
        while (w_slave0_aw_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_aw_vld = 1'b0;

        w_slave0_w_dat = 73'h0010000000000004000;
        w_slave0_w_vld = 1'b1;
          #clock_period;
        while (w_slave0_w_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_w_vld = 1'b0;

        w_slave0_aw_dat = 44'h100;
        w_slave0_aw_vld = 1'b1;
          #clock_period;
        while (w_slave0_aw_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_aw_vld = 1'b0;

        w_slave0_w_dat = 73'h001000000000000003F;
        w_slave0_w_vld = 1'b1;
          #clock_period;
        while (w_slave0_w_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_w_vld = 1'b0;

        w_slave0_aw_dat = 44'h180;
        w_slave0_aw_vld = 1'b1;
          #clock_period;
        while (w_slave0_aw_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_aw_vld = 1'b0;

        w_slave0_w_dat = 73'h0010000000000000001;
        w_slave0_w_vld = 1'b1;
          #clock_period;
        while (w_slave0_w_rdy != 1'b1) begin
          #clock_period;
        end
        w_slave0_w_vld = 1'b0;

        while (dma_done_vld != 1'b1) begin
          #clock_period;
        end
          #clock_period;
          #clock_period;
        $stop();

    end



    dma_wrap dma_INST ( 
           .clk(clk)
         , .rst_bar(rst_bar)
         , .r_master0_ar_dat(r_master0_ar_dat)
         , .r_master0_ar_vld(r_master0_ar_vld)
         , .r_master0_ar_rdy(r_master0_ar_rdy)
         , .r_master0_r_dat(r_master0_r_dat)
         , .r_master0_r_vld(r_master0_r_vld)
         , .r_master0_r_rdy(r_master0_r_rdy)
         , .w_master0_aw_dat(w_master0_aw_dat)
         , .w_master0_aw_vld(w_master0_aw_vld)
         , .w_master0_aw_rdy(w_master0_aw_rdy)
         , .w_master0_w_dat(w_master0_w_dat)
         , .w_master0_w_vld(w_master0_w_vld)
         , .w_master0_w_rdy(w_master0_w_rdy)
         , .w_master0_b_dat(w_master0_b_dat)
         , .w_master0_b_vld(w_master0_b_vld)
         , .w_master0_b_rdy(w_master0_b_rdy)
         , .r_slave0_ar_dat(r_slave0_ar_dat)
         , .r_slave0_ar_vld(r_slave0_ar_vld)
         , .r_slave0_ar_rdy(r_slave0_ar_rdy)
         , .r_slave0_r_dat(r_slave0_r_dat)
         , .r_slave0_r_vld(r_slave0_r_vld)
         , .r_slave0_r_rdy(r_slave0_r_rdy)
         , .w_slave0_aw_dat(w_slave0_aw_dat)
         , .w_slave0_aw_vld(w_slave0_aw_vld)
         , .w_slave0_aw_rdy(w_slave0_aw_rdy)
         , .w_slave0_w_dat(w_slave0_w_dat)
         , .w_slave0_w_vld(w_slave0_w_vld)
         , .w_slave0_w_rdy(w_slave0_w_rdy)
         , .w_slave0_b_dat(w_slave0_b_dat)
         , .w_slave0_b_vld(w_slave0_b_vld)
         , .w_slave0_b_rdy(w_slave0_b_rdy)
         , .dma_done_dat(dma_done_dat)
         , .dma_done_vld(dma_done_vld)
         , .dma_done_rdy(dma_done_rdy)
         , .dma_dbg_dat(dma_dbg_dat)
         , .dma_dbg_vld(dma_dbg_vld)
         , .dma_dbg_rdy(dma_dbg_rdy)
      );

    ram_wrap ram_INST ( 
           .clk(clk)
         , .rst_bar(rst_bar)
         , .r_slave0_ar_dat(r_master0_ar_dat)
         , .r_slave0_ar_vld(r_master0_ar_vld)
         , .r_slave0_ar_rdy(r_master0_ar_rdy)
         , .r_slave0_r_dat(r_master0_r_dat)
         , .r_slave0_r_vld(r_master0_r_vld)
         , .r_slave0_r_rdy(r_master0_r_rdy)
         , .w_slave0_aw_dat(w_master0_aw_dat)
         , .w_slave0_aw_vld(w_master0_aw_vld)
         , .w_slave0_aw_rdy(w_master0_aw_rdy)
         , .w_slave0_w_dat(w_master0_w_dat)
         , .w_slave0_w_vld(w_master0_w_vld)
         , .w_slave0_w_rdy(w_master0_w_rdy)
         , .w_slave0_b_dat(w_master0_b_dat)
         , .w_slave0_b_vld(w_master0_b_vld)
         , .w_slave0_b_rdy(w_master0_b_rdy)
      );

endmodule

