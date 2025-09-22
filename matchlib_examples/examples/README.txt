Matchlib Examples Summary
=========================

Below is a brief summary of each example.
See the README files in each example directory for more information.

01_and_gate:
  SystemC combinational AND gate showing how Catapult maps a trivial
  process to RTL.

02_flop:
  Single flip-flop example linking SystemC state to generated RTL.

03_crossed_flops:
  Crossed flop pair verifying sequential semantics match between
  SystemC and Catapult RTL.

04_ready_valid:
  Exposes the ready/valid/data handshake in Matchlib push/pop with 1
  transaction every 2 clocks.

05_push_pop:
  Push/pop latency example where pre- and post-HLS models sustain
  single-cycle throughput.

06_thruput_accurate:
  Throughput-accurate multi-input/output design demonstrating 1-cycle
  initiation interval across interfaces.

07_car_factory:
  Car factory model comparing three architectural approaches (see
  matchlib_training.pdf).

08_dma:
  Throughput-accurate DMA showcasing auto_gen_wrapper to emit a
  Verilog wrapper.

09_axi_fabric:
  Throughput-accurate AXI4 fabric plus FAST_SIM and AddressSanitizer
  build options.

10_fifo_channel_hier:
  Demonstrates hierarchical FIFO instantiation and wiring between
  Matchlib modules.

11_fifo_channel_flat:
  Shows push/pop access to a FIFO from processes within a flat module.

12_ping_pong_mem:
  Models ping-pong shared memory with ac_shared<> and ties into the
  memory logging methodology.

13_toggle_protocol:
  Bridges Matchlib ready/valid protocol to a SystemC toggle-style
  signal interface.

15_native_ram_fifo:
  Implements a RAM-backed FIFO by mapping a native C array into an HLS
  RAM.

16_ram_based_fifo:
  Wraps sc_fifo with Matchlib connections and maps storage to RAM via
  HLS directives.

17_fifo_status:
  FIFO with status signals (is_full/is_empty/etc.) demonstrating 1 ns
  push/pop throughput.

18_scatter_gather:
  Throughput-accurate scatter-gather DMA; see scatter_gather_dma.pdf
  for burst segmentation details.

20_DCT_sysc:
  SystemC/Matchlib DCT block synthesized through Catapult (see
  supporting documentation).

21_pixelpipe_mod:
  Variant of the DCT design using the pixelpipe_mod top level.

22_edge_detector:
  Matchlib edge detector with optional -DHAVE_DISPLAY build to
  visualize images.

23_auto_gen_field_methods:
  Uses AUTO_GEN_FIELD_METHODS to supply marshal/compare helpers for
  custom transactions.

24_vlog_split_wrap:
  Generates a Verilog wrapper that splits struct/class fields into
  discrete ports for RTL integration.

30_tlm2_dma:
  Wraps DMA and RAM with TLM-2.0 adapters for FAST_SIM virtual
  platform integration.

31_axi4_sigs_dma:
  AXI DMA exposing every bus field on discrete ports via signal-level
  transactors.

32_dat_vld:
  Interfaces a dat/vld-only handshake with an II=2 pipeline while
  preserving push/pop semantics.

33_wr_mask_array_sc:
  SystemC-flow example of ac_wr_mask_array_1D<> write-mask RAM
  modeling.

34_wr_mask_array_cpp:
  C++-flow counterpart using ac_wr_mask_array_1D<> for write-mask
  memories.

35_wrap_ram_to_multi_port:
  Wraps a 1R1W RAM to present a 2R2W interface through
  Connections::Combinational channels.

36_ac_bank_array_sc:
  SystemC-flow demonstration of ac_bank_array<> for banked memories
  (see memory methodology doc).

37_ac_bank_array_cpp:
  C++-flow use of ac_bank_array<> mirroring the SystemC example.

38_matrix_mult:
  Matrix multiplication sample—refer to matrix_mult_example.pdf for
  the walkthrough.

39_array_subrange:
  Highlights ac_array_subrange<> for modeling array subranges.

40_scratchpad:
  Scratchpad module providing banked memory without arbitration; user
  must avoid bank conflicts.

41_scratchpad_class:
  ScratchpadClass version synthesizable inline without a separate
  module.

42_arbitrated_scratchpad:
  Scratchpad variant with arbitration/queuing, introducing possible
  bank conflicts and backpressure.

43_scratchpad_class_cpp_flow:
  C++-flow version of ScratchpadClass to match the SystemC example.

44_arb_scratchpad_cpp_flow:
  C++-flow version of the arbitrated scratchpad.

45_vlog_tb_dma_dut:
  Verilog testbench instantiating the SystemC DMA DUT using the same
  stimulus as 08_dma.

46_sc_tb_wrap_rtl_dut:
  Shows how Matchlib-generated RTL wrappers let Catapult RTL plug back
  into a SystemC testbench.

48_chisel:
  Chain-of-flops example illustrating Chisel-like dynamic structure in
  SystemC/Catapult.

49_chisel:
  Simplified dynamic structure construction mirroring Chisel patterns.

51_context_switch:
  Demonstrates reliable pipeline context switching with SyncChannel-
  managed state save/restore.

52_apb:
  Configurable APB DMA with a synthesizeable APB transactor and
  protocol references.

53_transactor_modeling:
  Supporting material in transactor_modeling.pdf for building Matchlib
  transactors.

54_mem_with_stall:
  Custom memory protocol translating push/pop into a stall-aware RAM
  interface (see custom_memory_protocols.pdf).

55_mixed_axi:
  DMA using 64-bit AXI4 slave and 16-bit master ports to illustrate
  mixed bus widths.

57_thruput_latency_control:
  Companion to thruput_latency_control_tutorial.pdf for managing
  latency/throughput trade-offs.

59_idle_signal:
  Explores automatic versus manual idle signal generation for coarse-
  grain clock gating.

60_rand_stall:
  Random-stall debugging scenario explained in
  matchlib_soc_debug_tutorial.pdf and memory logging guide.

61_direct_input_sync:
  Stress test for Catapult direct input scheduling; compares standard
  versus DIRECT_INPUT_SYNC flows.

63_peek_arbiter:
  Peek-capable arbiter maintaining 1 ns throughput with push/pop
  semantics.

64_peek_arbiter_comb:
  Combinational variant of the peek arbiter with the same throughput
  profile.

66_basic_round_robin_arbiter:
  Low-level round-robin arbiter coded in SystemC for synthesis
  efficiency.

67_merge_comb:
  Purely combinational merger of three Connections::In<> channels into
  one output.

70_python_matlab_integration:
  Embeds Matchlib simulation inside Python or MATLAB via ctypes/MEX
  using sc_start and sc_pause.

71_annotate_simple:
  Latency/capacity backannotation example driven by top.input.json
  edits.

72_annotate_reconverge:
  Bagel shop reconvergence case showing stutter reproduction through
  latency annotations.

73_stutter_fix:
  Diagnoses and resolves stuttering across interfaces and uses
  assert_no_x() to catch X propagation.

80_mem_log_pre_post_hls:
  Demonstrates pre- and post-HLS memory logging, including custom
  mc_transactors instrumentation.

85_cdc_fifo:
  Clock-domain-crossing FIFO using ac_blackbox to drop in a Verilog
  model and drive a multi-clock DUT.

87_axi4_lite:
  DMA combining AXI4-Lite control and full AXI4 data interfaces.

88_stub_off_ports:
  Shows how to cleanly stub intentionally unused Matchlib ports
  without warnings.

94_event_pulse:
  Matchlib push/pop example creating single-cycle pulses via
  p2p_event.

95_multi_clk:
  Multi-clock, dynamic-reset design exercised before and after HLS.

98_hls_block_size:
  Demonstrates templated hls_block_size pragmas for banked memory
  modeling.
