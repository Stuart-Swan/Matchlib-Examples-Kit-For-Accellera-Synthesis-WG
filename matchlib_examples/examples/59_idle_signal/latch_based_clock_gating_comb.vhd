library IEEE;  
use IEEE.STD_LOGIC_1164.ALL;  
  
entity clock_gate_module is  
    Port (  
        clk_in     : in  STD_LOGIC; -- Original clock signal  
        idle_in    : in  STD_LOGIC; -- Enable signal  
        clk_out : out STD_LOGIC  -- Gated clock output  
    );  
end clock_gate_module;  
  
architecture Behavioral of clock_gate_module is  
    signal latch_out : STD_LOGIC; -- Internal signal for the latch  
begin  
    -- Level-sensitive latch  
    process(clk_in, idle_in)  
    begin  
        if clk_in = '0' then  
            latch_out <= idle_in;  
        end if;  
    end process;  
  
    -- Gated clock logic  
    clk_out <= '1' when latch_out = '1' else clk_in;  
  
end Behavioral;  

 