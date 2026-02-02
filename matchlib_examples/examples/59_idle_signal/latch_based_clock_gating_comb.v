module clock_gate_module (  
    input clk_in,         // Original clock signal  
    input idle_in,      // Enable signal  
    output clk_out   // Gated clock output  
);  
  
  wire latch_out;  

  // Level-sensitive latch  
  assign latch_out = (!clk_in) ? idle_in : latch_out;  

  // Gated clock logic  
  assign clk_out = latch_out ? 1'b1 : clk_in;  
  
endmodule  